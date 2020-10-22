//
// Created by ben on 09/10/16.
//

#ifndef BUFFERVM_PAGING_H
#define BUFFERVM_PAGING_H

//
// Created by ben on 09/10/16.
//

#include "stdlib_inc.h"

/* 64-bit page * entry bits */
#define PDE64_PRESENT 1
#define PDE64_WRITEABLE (1 << 1)
#define PDE64_NO_EXE (UINT64_C(1) << 63)
#define PDE64_USER (1 << 2)
#define PDE64_WRITE_MEM (1 << 3)
#define PDE64_DISABLE_CACHE (1 << 4)
#define PDE64_ACCESSED (1 << 5)
#define PDE64_DIRTY (1 << 6)
#define PDE64_PS (1 << 7)
#define PDE64_G (1 << 8)

#define PAGE_CREATE -1

#define MIN_ALIGN   8

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
//#define PAGE_SIZE 0x1000UL
#define PAGE_MASK (~(PAGE_SIZE-1))

#define P2ALIGN(x, align)    ((x) & -(align))
#define P2ROUNDUP(x, align) (-(-(x) & -(align)))

#define PAGE_ALIGN(addr) P2ROUNDUP(addr, PAGE_SIZE)
#define PAGE_ALIGN_DOWN(addr) P2ALIGN(addr, PAGE_SIZE)

#define PAGE_OFFSET(addr) (addr & ~PAGE_MASK)
#define PAGE_DIFFERENCE(left, right) ((left - right) >> PAGE_SHIFT)

#define PHYSICAL_HEAP_SIZE 0xFFF00000UL
#define PHYSICAL_HEAP_PAGES (PHYSICAL_HEAP_SIZE / PAGE_SIZE)

typedef struct {
    uint64_t version;
    uint64_t pg_dir_ptr_offset;
    uint64_t pg_dir_offset;
    uint64_t pg_tbl_offset;
    uint64_t phys_offset;
} virt_addr_info_t;

extern uint32_t pd_addr;
extern uint32_t pt_start_addr;
extern uint64_t page_counter;
extern uint64_t pml4_addr;

int read_virtual_cstr(uint64_t virtual_addr, char **buffer);

int write_virtual_addr(uint64_t virtual_addr, char *source, size_t length);

int read_virtual_addr(uint64_t virtual_addr, size_t size, void *buffer);

uint64_t un_sign_extend(uint64_t addr);

virt_addr_info_t get_virt_addr_info(uint64_t addr);

int is_vpage_present(uint64_t virtual_addr);

int get_phys_addr(uint64_t virtual_addr, uint64_t *phys_addr);

int get_page_entry(uint64_t *table, size_t index, uint64_t *page);

void build_page_tables();

int64_t allocate_pages(size_t num_pages, bool zero_page);

uint64_t map_page_entry(uint64_t *table, size_t index, uint64_t flags, int64_t page);

void unmap_page_entry(uint64_t *table, size_t index);

void load_address_space(uint64_t start_addr, size_t mem_size, char *elf_seg_start, size_t elf_seg_size, uint64_t flags);

#define MAP_CONTINUOUS 1
#define MAP_NO_OVERWRITE (1 << 1)
#define MAP_ZERO_PAGES (1 << 2)

int64_t map_physical_pages(uint64_t virtual_page_addr, int64_t physical_page_addr, uint64_t page_prot, size_t num_pages,
                           uint64_t flags);

void unmap_physical_page(uint64_t virtual_page_addr);

#endif //BUFFERVM_PAGING_H
