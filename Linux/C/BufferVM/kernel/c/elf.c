//
// Created by ben on 03/01/17.
//

#include <string.h>

#include "../h/host.h"
#include "../h/syscall.h"
#include "../../common/elf.h"
#include "../../common/paging.h"
#include "../../common/utils.h"
#include "../h/kernel.h"

bool is_elf(int fd) {
    unsigned char e_ident[EI_NIDENT];
    kernel_read(fd, (char *) &e_ident, sizeof(e_ident));
    kernel_lseek(fd, 0, SEEK_SET); // go to the beginning

    return memcmp(e_ident, "\177ELF", 4) == 0;
}

bool is_binary_64bit(int fd) {
    elf64_hdr_t elf_hdr;
    kernel_read(fd, (char *) &elf_hdr, sizeof(elf64_hdr_t));
    kernel_lseek(fd, 0, SEEK_SET); // go to the beginning

    return elf_hdr.e_machine == EM_X86_64;
}

int pad_zero(uint64_t elf_bss) {
    uint64_t nbyte;

    nbyte = PAGE_OFFSET(elf_bss);
    if (nbyte) {
        nbyte = PAGE_SIZE - nbyte;
        if (memset((void *) elf_bss, 0, nbyte))
            return -EFAULT;
    }
    return 0;
}

static uint64_t total_mapping_size(elf64_phdr_t *phdrs, uint16_t phnum)
{
    int i;
    int first_phdr = -1, last_phdr = -1;

    for (i = 0; i < phnum; i++) {

        if (phdrs[i].p_type == PT_LOAD) {
            last_phdr = i;
            if (first_phdr == -1){
                first_phdr = i;
            }
        }
    }
    if (first_phdr == -1)
        return 0;

    return (phdrs[last_phdr].p_vaddr + phdrs[last_phdr].p_memsz - PAGE_ALIGN_DOWN(phdrs[first_phdr].p_vaddr));
}

static uint64_t elf_map(int fd, uint64_t addr,
                        elf64_phdr_t *eppnt, uint64_t prot, uint64_t flags,
                        uint64_t total_size)
{
    uint64_t map_addr;
    uint64_t size = eppnt->p_filesz + PAGE_OFFSET(eppnt->p_vaddr);
    uint64_t off = eppnt->p_offset - PAGE_OFFSET(eppnt->p_vaddr);
    addr = PAGE_ALIGN_DOWN(addr);
    size = PAGE_ALIGN(size);

    /* mmap() will return -EINVAL if given a zero size, but a
     * segment with zero filesize is perfectly valid */
    if (!size)
        return addr;

    /*
    * total_size is the size of the ELF (interpreter) image.
    * The _first_ mmap needs to know the full size, otherwise
    * randomization might put this image into an overlapping
    * position with the ELF binary image. (since size < total_size)
    * So we first map the 'big' image - and unmap the remainder at
    * the end. (which unmap is needed for ELF images with holes.)
    */
    if (total_size) {
        total_size = PAGE_ALIGN(total_size);
        map_addr = syscall_mmap(addr, total_size, prot, flags, fd, off);
        if (map_addr < kernel_min_address)
            syscall_munmap(map_addr+size, total_size-size);
    } else {
        map_addr = syscall_mmap(addr, size, prot, flags, fd, off);
    }

    return(map_addr);
}

int load_elf64_pg_hdrs(int fd, elf64_hdr_t *elf_hdr, uint64_t *elf_bias, elf_info_t *elf_info, bool is_interp) {

    uint64_t start_addr, total_size;
    uint64_t min_addr = UINT64_MAX;

    uint64_t section_max_maddr = 0;
    uint64_t section_max_faddr = 0;

    uint64_t max_faddr = 0;
    uint64_t max_maddr = 0;

    uint64_t load_offset = 0;

    uint64_t load_addr = 0;

    bool found_entry = false;
    elf64_phdr_t phdr[elf_hdr->e_phnum];

    kernel_lseek(fd, elf_hdr->e_phoff, SEEK_SET);
    kernel_read(fd, (char *) &phdr, sizeof(elf64_phdr_t) * elf_hdr->e_phnum);

    if (is_interp) {
        total_size = total_mapping_size(phdr, elf_hdr->e_phnum);
        VERIFY(total_size);
    }

    for (size_t i = 0; i < elf_hdr->e_phnum; i++) {
        uint64_t v_addr;

        v_addr = phdr[i].p_vaddr;

        //TODO - size > file_offset

        if (/*load_only && */phdr[i].p_type != PT_LOAD) {
            continue;
        }

        if (!is_interp) {
            total_size = 0;
        }

        uint64_t prot = 0, flags = MAP_PRIVATE;

        //Readable.
        if (phdr[i].p_flags & PF_R)
            prot |= PROT_READ;

        //Writeable.
        if (phdr[i].p_flags & PF_W)
            prot |= PROT_WRITE;

        // Executable.
        if (phdr[i].p_flags & PF_X)
            prot |= PROT_EXEC;
        if (elf_hdr->e_type == ET_EXEC || found_entry)
            flags |= MAP_FIXED;
        else if (is_interp && elf_info->base_addr && elf_hdr->e_type == ET_DYN) {
            //load_offset = (uint64_t)-v_addr;
        } else if (!is_interp && elf_hdr->e_type == ET_DYN) {
            load_offset = PAGE_ALIGN_DOWN((kernel_min_address / 3 * 2) - v_addr);
            total_size = total_mapping_size(phdr, elf_hdr->e_phnum);

            VERIFY(total_size);
        }

        start_addr = elf_map(fd, v_addr + load_offset, &phdr[i], prot, flags, total_size);

        if (is_interp) {
            total_size = 0;
        }

        //Will handle error codes too - debugging point
        //printf("Error %p\n", -(uint64_t) start_addr);
        VERIFY(!IS_ERR_VALUE(start_addr) && start_addr < kernel_min_address);
        VERIFY(!(flags & MAP_FIXED) || start_addr == PAGE_ALIGN_DOWN(v_addr + load_offset));

        if (!found_entry) {
            found_entry = true;
            if (is_interp && elf_hdr->e_type == ET_DYN) {
                load_offset = start_addr - PAGE_ALIGN_DOWN(v_addr);
            } else {
                load_addr = phdr[i].p_vaddr - phdr[i].p_offset;

                if (elf_hdr->e_type == ET_DYN) {
                    load_offset += start_addr - PAGE_ALIGN_DOWN(load_offset + v_addr);
                    load_addr += load_offset;
                }
            }
        }

        section_max_maddr = phdr[i].p_vaddr + phdr[i].p_memsz;
        section_max_faddr = phdr[i].p_vaddr + phdr[i].p_filesz;

        if (is_interp) {
            section_max_maddr += load_offset;
            section_max_faddr += load_offset;
        }

        if (start_addr < min_addr)
            min_addr = start_addr;

        //mem
        if (section_max_maddr > max_maddr)
            max_maddr = section_max_maddr;

        //file
        if (section_max_faddr > max_faddr)
            max_faddr = section_max_faddr;

        printf("Loaded header at %p of size: %d file offset %d\n", (void *) phdr[i].p_vaddr, phdr[i].p_memsz, phdr[i].p_filesz);
    }

    if (!is_interp) {
        max_maddr += load_offset;
        max_faddr += load_offset;
        elf_info->load_addr = load_addr;
    }

    if (!pad_zero(max_faddr)) {
        printf("Zeroed end of bss\n");
    }

    if (is_interp) {
        max_faddr = (uint64_t)PAGE_ALIGN(max_faddr);
        max_maddr = (uint64_t)PAGE_ALIGN(max_maddr);

        if (max_maddr > max_faddr) {
            VERIFY(!do_brk(max_faddr, max_maddr - max_faddr, VMA_ALLOC_ZEROED));
        }
    }

    // Align to page size
    elf_info->min_page_addr = (uint64_t) PAGE_ALIGN(min_addr);
    elf_info->max_page_addr = (uint64_t) PAGE_ALIGN(max_maddr);

    if (elf_bias) *elf_bias = load_offset;

    return 0;
}

void load_elf64_interpreter(int fd, char *path, void **elf_entry, elf_info_t *elf_info, uint64_t base_addr) {
    elf_info_t interp_elf_info;
    interp_elf_info.base_addr = base_addr;

    printf("Loading interpreter: %s\n", path);

    int user_interp_fd = read_binary(path);
    load_elf_binary(user_interp_fd, elf_entry, &interp_elf_info, true);
    elf_info->base_addr = (uint64_t) *elf_entry;
    printf("Interp entry: %p\n", *elf_entry);
    *elf_entry = *elf_entry + (uint64_t) interp_elf_info.entry_addr;
}

int load_elf64(int fd, void **elf_entry, elf_info_t *elf_info, bool is_interp) {
    elf64_hdr_t elf_hdr;
    int ret;
    uint64_t elf_bias;

    kernel_read(fd, (char *) &elf_hdr, sizeof(elf64_hdr_t));

    printf("Entry point user: %p\n", elf_hdr.e_entry);
    printf("Num program headers: %d\n", elf_hdr.e_phnum);

    VERIFY(elf_hdr.e_type == ET_EXEC || elf_hdr.e_type == ET_DYN);

    elf_info->entry_addr = (void *) elf_hdr.e_entry;
    if (elf_entry) *elf_entry = elf_info->entry_addr;

    kernel_lseek(fd, elf_hdr.e_phoff, SEEK_SET);

    for (size_t i = 0; i < elf_hdr.e_phnum; i++) {
        elf64_phdr_t phdr;
        uint64_t end_addr;

        kernel_read(fd, (char *) &phdr, sizeof(elf64_phdr_t));

        if (phdr.p_type == PT_INTERP) {

            char interp_path[phdr.p_filesz];
            kernel_lseek(fd, phdr.p_offset, SEEK_SET);
            kernel_read(fd, interp_path, phdr.p_filesz);

            //load base elf image after interpreter
            if ((ret = load_elf64_pg_hdrs(fd, &elf_hdr, &elf_bias, elf_info, false))) {
                return ret;
            }

            vma_print();
            printf("Elf bias: %lx\n", elf_bias);

            load_elf64_interpreter(fd, interp_path, elf_entry, elf_info, elf_bias);

            elf_info->entry_addr = (void *) (elf_hdr.e_entry + elf_bias);
            elf_info->phdr_num = elf_hdr.e_phnum;
            elf_info->phdr_off = elf_hdr.e_phoff;

            return 0;
        }
    }

    if ((ret = load_elf64_pg_hdrs(fd, &elf_hdr, &elf_bias, elf_info, is_interp)))
        return ret;

    if (!is_interp)
        *elf_entry = (void *)(elf_hdr.e_entry + elf_bias);
    else
        *elf_entry = (void *)elf_bias;

    printf("Elf bias: %lx\n", elf_bias);

    elf_info->entry_addr = (void *) elf_hdr.e_entry;
    elf_info->phdr_num = elf_hdr.e_phnum;
    elf_info->phdr_off = elf_hdr.e_phoff;

    return 0;
}

int load_elf_binary(int fd, void **elf_entry, elf_info_t *elf_info, bool is_interp) {
    if (is_elf(fd)) {
        if (is_binary_64bit(fd)) {
            printf("64-bit bin\n");
            return load_elf64(fd, elf_entry, elf_info, is_interp);
        } else {
            printf("Un-supported binary? 32-bit not currently supported?\n");

            return 1;
        }
    } else {
        printf("Not a valid elf file\n");

        return 1;
    }
}

int read_binary(char *name) {
    int file;

    //Open file
    file = kernel_open(name, O_RDONLY, 0);
    if (file < 0) {
        printf("Unable to open file %s\n", name);
        return 0;
    }

    return file;
}
