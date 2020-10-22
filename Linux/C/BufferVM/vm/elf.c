//
// Created by ben on 09/10/16.
//

#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <memory.h>
#include <sysexits.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <gelf.h>

#include "elf.h"
#include "../common/elf.h"
#include "../common/paging.h"

int load_elf_binary(int fd, void **elf_entry, elf_info_t *elf_info, bool load_only) {
    Elf *e;
    size_t n;
    GElf_Ehdr ehdr;

    if (elf_version(EV_CURRENT) == EV_NONE)
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));

    if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
        errx(EX_SOFTWARE, "elf_begin () failed: %s.", elf_errmsg(-1));

    if (elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "Not an ELF object.");

    if (gelf_getehdr(e, &ehdr) == NULL)
        errx(EX_SOFTWARE, "getehdr () failed: %s.", elf_errmsg(-1));

    elf_info->entry_addr = (void *) ehdr.e_entry;

    if (elf_getphdrnum(e, &n) != 0)
        errx(EX_DATAERR, "elf_getphdrnum () failed: %s.", elf_errmsg(-1));

    GElf_Phdr phdr;
    uint64_t start;
    uint64_t taddr;
    uint64_t min_addr = UINT64_MAX;
    uint64_t max_addr = 0;
    uint64_t section_max_addr;

    for (size_t i = 0; i < n; i++) {

        if (gelf_getphdr(e, i, &phdr) != &phdr)
            errx(EX_SOFTWARE, "getphdr () failed: %s.", elf_errmsg(-1));

        if (phdr.p_type != PT_LOAD) {
            continue;
        }
        if (phdr.p_filesz > phdr.p_memsz) {
            fprintf(stderr, "load_elf_binary:: p_filesz > p_memsz\n");
            //TODO
            //munmap(exec, size);
            return 1;
        }
        if (!phdr.p_filesz) {
            continue;
        }

        // p_filesz can be smaller than p_memsz,
        // the difference is zeroe'd out.
        start = (uint64_t) elf_rawfile(e, NULL) + phdr.p_offset;
        taddr = phdr.p_vaddr;

        uint64_t flags = 0;

        //Writeable.
        if (phdr.p_flags & PF_W)
            flags |= PDE64_WRITEABLE;

        // Executable.
        if (!(phdr.p_flags & PF_X))
            flags |= PDE64_NO_EXE;

        load_address_space(taddr, phdr.p_memsz, (char *) start, phdr.p_filesz, flags);

        section_max_addr = taddr + phdr.p_memsz;

        if (taddr < min_addr)
            min_addr = taddr;

        if (section_max_addr > max_addr)
            max_addr = section_max_addr;

        printf("Loaded header at %p of size: %" PRIx64 " at file offset: %" PRIu64 "\n", (void *) taddr,
               (uint64_t) phdr.p_memsz, (uint64_t) phdr.p_offset);
    }

    // Align to page size
    elf_info->min_page_addr = (uint64_t) P2ALIGN(min_addr, PAGE_SIZE);
    elf_info->max_page_addr = (uint64_t) P2ROUNDUP(max_addr, PAGE_SIZE);

    elf_end(e);

    return 0;

}

int read_binary(char *name) {
    int file;

    //Open file
    file = open(name, O_RDONLY, 0);
    if (file < 0) {
        fprintf(stderr, "Unable to open file %s\n", name);
        return 0;
    }

    return file;
}
