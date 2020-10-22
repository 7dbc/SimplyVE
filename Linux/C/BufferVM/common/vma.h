//
// Created by ben on 30/12/17.
//

#ifndef COMMON_VMA_H
#define COMMON_VMA_H

#include <stdint.h>
#include <stdbool.h>
#include "syscall.h"
#include "paging.h"
#include "rbtree.h"
#include "utils.h"

typedef struct vm_area vm_area_t;

struct vm_area {
    uint64_t start_addr;
    uint64_t end_addr;

    uint64_t page_prot;
    uint64_t flags;

    //information needed to resize a VMA
    int64_t phys_page_start;
    int64_t phys_page_end;

    uint64_t old_start_addr;
    uint64_t old_end_addr;

    //reference to VMA above and below
    vm_area_t *prev, *next;

    rb_node_t rb_node;

    //memory gap pre-calculated between VMA and previous, or VMA below in rbtree and its previous
    uint64_t rb_subtree_gap;

    //file page offset
    uint64_t page_offset;
    vm_file_t file_info;
};

#define VMA_READ         0x00000001
#define VMA_WRITE        0x00000002
#define VMA_EXEC         0x00000004
#define VMA_SHARED       0x00000008
#define VMA_IS_VERSIONED    0x00000010
#define VMA_ALLOC_ZEROED    0x00000012

#define VMA_GROWS   0x00000010
#define VMA_IS_PREFAULTED   0x00000020
#define VMA_HAS_RESIZED   0x00000040

static inline uint64_t prot_to_vma(uint64_t prot) {
    return TRANSFER_FLAG(prot, PROT_READ, VMA_READ) |
           TRANSFER_FLAG(prot, PROT_WRITE, VMA_WRITE) |
           TRANSFER_FLAG(prot, PROT_EXEC, VMA_EXEC);
}

static inline uint64_t vma_to_prot(uint64_t prot) {
    return TRANSFER_FLAG(prot, VMA_READ, PROT_READ) |
           TRANSFER_FLAG(prot, VMA_WRITE, PROT_WRITE)/* |
           TRANSFER_FLAG(prot, VMA_EXEC, PROT_EXEC)*/;
}

static inline uint64_t vma_prot_to_pg(uint64_t prot) {
    return TRANSFER_FLAG(prot, VMA_READ, PDE64_PRESENT) |
           TRANSFER_FLAG(prot, VMA_WRITE, PDE64_WRITEABLE) |
           TRANSFER_INV_FLAG(prot, VMA_EXEC, PDE64_NO_EXE) |
           TRANSFER_INV_FLAG(prot, VMA_IS_VERSIONED, PDE64_USER) ;
}

extern rb_root_t vma_rb_root;
extern rb_root_t *vma_rb_root_ptr;
extern uint64_t vma_highest_addr;
extern vm_area_t *vma_list_start;
extern vm_area_t **vma_list_start_ptr;
extern uint64_t vma_heap_addr;

//stubs for none common code

vm_area_t *vma_ptr(vm_area_t *);
rb_node_t *vma_rb_ptr(rb_node_t *vma);

//common functions

void vma_print_node(vm_area_t *vma, bool follow, size_t page_count);

void vma_print();

vm_area_t *vma_find(uint64_t addr);
vm_area_t *vma_find_phys(uint64_t addr);

#endif //COMMON_VMA_H
