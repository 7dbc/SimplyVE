//
// Created by ben on 09/10/16.
//

#include <stdlib.h>
#include <inttypes.h>
#include "../common/paging.h"
#include "vm.h"

uint32_t pd_addr = 0xFFEFF000;
uint32_t pt_start_addr = 0xFFB00000;
//reserve first few pages for bootstrap
uint64_t page_counter = 10;
uint64_t pml4_addr;

uint64_t kbit(uint64_t addr, size_t k) {
    return (addr >> k) & 1;
}

uint64_t un_sign_extend(uint64_t addr) {
    if (kbit(addr, 63) == 0) return addr;

    size_t extension_bits = 0;
    for (int32_t i = 62; i >= 0; i--) {
        if (kbit(addr, i) == 1) extension_bits++;
        else break;
    }
    //handle exception for how 32/16-bit pointers are zero extended
    if (extension_bits > 32)
        extension_bits = 32;

    return (addr & (0xFFFFFFFFFFFFFFFF >> extension_bits));
}

virt_addr_info_t get_virt_addr_info(uint64_t addr) {
    virt_addr_info_t pginf;

    pginf.phys_offset = addr & 0x0000000000000FFF;
    pginf.pg_tbl_offset = (addr & 0x00000000001FF000) >> 12;
    pginf.pg_dir_offset = (addr & 0x000000003fe00000) >> 21;
    pginf.pg_dir_ptr_offset = (addr & 0x0000007fc0000000) >> 30;
    pginf.version = (addr & 0x0000ff8000000000) >> 39;

    return pginf;
}

int read_virtual_cstr(uint64_t virtual_addr, char **buffer) {
    uint64_t phys_addr;
    size_t page_bytes;
    size_t bytes_read = 0;

    while (true) {
        if (!get_phys_addr(virtual_addr + bytes_read, &phys_addr)) return 0;

        page_bytes = PAGE_SIZE - ((virtual_addr + bytes_read) % PAGE_SIZE);

        for (size_t i = 0; i < page_bytes; i++) {
            if ((vm.mem + phys_addr)[bytes_read++] == 0) {
                bytes_read++;
                *buffer = malloc(bytes_read);
                return read_virtual_addr(virtual_addr, bytes_read, *buffer);
            }
        }
    }
}

int write_virtual_addr(uint64_t virtual_addr, char *source, size_t length) {
    uint64_t phys_addr;
    size_t page_bytes;
    size_t bytes_written = 0;
    size_t bytes_left = length;

    while (bytes_left > 0) {
        if (!get_phys_addr(virtual_addr + bytes_written, &phys_addr)) return 0;

        page_bytes = PAGE_SIZE - ((virtual_addr + bytes_written) % PAGE_SIZE);
        if (page_bytes > bytes_left) page_bytes = bytes_left;

        memcpy(vm.mem + phys_addr, source + bytes_written, page_bytes);

        bytes_written += page_bytes;
        bytes_left -= page_bytes;
    }

    return 1;
}

int read_virtual_addr(uint64_t virtual_addr, size_t size, void *buffer) {

    uint64_t phys_addr;
    size_t page_bytes;
    size_t bytes_read = 0;
    size_t bytes_left = size;

    while (bytes_left > 0) {
        if (!get_phys_addr(virtual_addr + bytes_read, &phys_addr)) return 0;

        page_bytes = PAGE_SIZE - ((virtual_addr + bytes_read) % PAGE_SIZE);
        if (page_bytes > bytes_left) page_bytes = bytes_left;

        memcpy(buffer + bytes_read, vm.mem + phys_addr, page_bytes);

        bytes_read += page_bytes;
        bytes_left -= page_bytes;
    }

    return 1;
}

int is_vpage_present(uint64_t virtual_addr) {
    return get_phys_addr(virtual_addr, NULL);
}

int get_phys_addr(uint64_t virtual_addr, uint64_t *phys_addr) {
    uint64_t page_addr;

    virt_addr_info_t info = get_virt_addr_info(virtual_addr);

    uint64_t *pml4 = (void *) (vm.mem + pml4_addr);

    if (!get_page_entry(pml4, 0, &page_addr))
        return 0;

    uint64_t *pdpt = (void *) (vm.mem + page_addr);
    if (!get_page_entry(pdpt, info.pg_dir_ptr_offset, &page_addr))
        return 0;

    uint64_t *pd = (void *) (vm.mem + page_addr);
    if (!get_page_entry(pd, info.pg_dir_offset, &page_addr))
        return 0;

    uint64_t *pd2 = (void *) (vm.mem + page_addr);
    if (!get_page_entry(pd2, info.pg_tbl_offset, &page_addr))
        return 0;

    if (phys_addr != NULL)
        *phys_addr = page_addr + info.phys_offset;

    return 1;
}

/**
start_addr should be page aligned
*/
void
load_address_space(uint64_t start_addr, size_t mem_size, char *elf_seg_start, size_t elf_seg_size, uint64_t flags) {
    size_t size_left_mem = mem_size;
    size_t size_left_elf = elf_seg_size;

    for (uint64_t p = start_addr, i = 0; p < start_addr + mem_size; p += 0x1000, i += 0x1000) {

        uint64_t phy_addr = allocate_pages(1, false);

        memset(vm.mem + phy_addr, 0x0, 0x1000);
        memcpy(vm.mem + phy_addr, elf_seg_start + i, size_left_elf < 0x1000 ? size_left_elf : 0x1000);

        //printf("Loaded page: %p %p, %#04hhx\n", (void*) p, (void*) phy_addr, mem_offset[phy_addr+0xc0a]);
        map_physical_pages(p, phy_addr, flags, 1, 0);

        size_left_mem -= 0x1000;
        size_left_elf -= 0x1000;
    }
}

//TODO - make this better
int64_t allocate_pages(size_t num_pages, bool zero_page) {

    uint64_t new_page_start;

    if (num_pages == 0) return -1;

    for (size_t i = 0; i < num_pages; i++) {
        uint64_t new_page = (page_counter++) * PAGE_SIZE;
        if (i == 0) {
            new_page_start = new_page;
            //not needed for fresh anonymous pages
            /*if (zero_page)
                memset(vm.mem + new_page, 0x0, PAGE_SIZE * num_pages);*/
        }

    }

    return new_page_start;
}

void build_page_tables() {

    pml4_addr = allocate_pages(1, true);
    uint64_t *pml4 = (void *) (vm.mem + pml4_addr);

    uint64_t pdpt_addr = allocate_pages(1, true);

    for (size_t i = 0; i < 512; i++)
        pml4[i] = pdpt_addr | PDE64_PRESENT | PDE64_USER | PDE64_WRITEABLE;
}

int get_page_entry(uint64_t *table, size_t index, uint64_t *page) {

    if (table[index] & PDE64_PRESENT) {
        if (page != NULL) *page = table[index] & 0xFFFFFFFFFFF000;
        return 1;
    } else
        return 0;
}

uint64_t map_page_entry(uint64_t *table, size_t index, uint64_t flags, int64_t page) {
    if (page == PAGE_CREATE) {
        if (!(table[index] & PDE64_PRESENT)) {
            table[index] = PDE64_PRESENT | flags | allocate_pages(1, true);
        } else {
            table[index] |= flags;
        }
    } else
        table[index] = PDE64_PRESENT | flags | (page & 0xFFFFFFFFFFF000);

    return table[index] & 0xFFFFFFFFFFF000;
}

void unmap_page_entry(uint64_t *table, size_t index) {
    table[index] = 0;
}

void map_physical_page(uint64_t virtual_page_addr, int64_t physical_page_addr, uint64_t flags, bool no_overwrite) {

    virt_addr_info_t info = get_virt_addr_info(virtual_page_addr);

    uint64_t *pml4 = (void *) (vm.mem + pml4_addr);

    uint64_t pdpt_addr = map_page_entry(pml4, 0, PDE64_USER | PDE64_WRITEABLE, PAGE_CREATE);
    uint64_t *pdpt = (void *) (vm.mem + pdpt_addr);

    uint64_t pd_addr = map_page_entry(pdpt, info.pg_dir_ptr_offset, PDE64_USER | PDE64_WRITEABLE, PAGE_CREATE);
    uint64_t *pd = (void *) (vm.mem + pd_addr);

    uint64_t pd2_addr = map_page_entry(pd, info.pg_dir_offset, PDE64_USER | PDE64_WRITEABLE, PAGE_CREATE);
    uint64_t *pd2 = (void *) (vm.mem + pd2_addr);

    if (!no_overwrite || !get_page_entry(pd2, info.pg_tbl_offset, NULL))
        map_page_entry(pd2, info.pg_tbl_offset, flags | PDE64_WRITEABLE, physical_page_addr);
}

void unmap_physical_page(uint64_t virtual_page_addr) {

    virt_addr_info_t info = get_virt_addr_info(virtual_page_addr);

    uint64_t *pml4 = (void *) (vm.mem + pml4_addr);

    //TODO - maybe bail at earlier page directory - don't need to create to unmap?

    uint64_t pdpt_addr = map_page_entry(pml4, 0, PDE64_USER | PDE64_WRITEABLE, PAGE_CREATE);
    uint64_t *pdpt = (void *) (vm.mem + pdpt_addr);

    uint64_t pd_addr = map_page_entry(pdpt, info.pg_dir_ptr_offset, PDE64_USER | PDE64_WRITEABLE, PAGE_CREATE
    );
    uint64_t *pd = (void *) (vm.mem + pd_addr);

    uint64_t pd2_addr = map_page_entry(pd, info.pg_dir_offset, PDE64_USER | PDE64_WRITEABLE, PAGE_CREATE);
    uint64_t *pd2 = (void *) (vm.mem + pd2_addr);

    unmap_page_entry(pd2, info.pg_tbl_offset);
}

int64_t map_physical_pages(uint64_t virtual_page_addr, int64_t physical_page_addr, uint64_t page_prot, size_t num_pages,
                           uint64_t flags) {
    //TODO - num_pages: painful algorithm needed. Could also look at option to use bigger pages for large allocation?

    if (flags & MAP_CONTINUOUS || physical_page_addr != -1) {
        if (physical_page_addr == -1)
            physical_page_addr = allocate_pages(num_pages, flags & MAP_ZERO_PAGES);
        if (physical_page_addr == -1) return -1;

        for (size_t i = 0; i < num_pages; i++)
            map_physical_page(virtual_page_addr + PAGE_SIZE * i, physical_page_addr + PAGE_SIZE * i, page_prot,
                              flags & MAP_NO_OVERWRITE);
    } else {
        for (size_t i = 0; i < num_pages; i++) {
            physical_page_addr = allocate_pages(1, flags & MAP_ZERO_PAGES);
            if (physical_page_addr == -1) return -1;

            map_physical_page(virtual_page_addr + PAGE_SIZE * i, physical_page_addr, page_prot,
                              flags & MAP_NO_OVERWRITE);
        }
    }

    return physical_page_addr;
}