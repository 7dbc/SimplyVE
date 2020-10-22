//
// Created by ben on 29/12/16.
//

#ifndef PROJECT_VMA_HEAP_H
#define PROJECT_VMA_HEAP_H

#include "../../common/stdlib_inc.h"
#include "../../libc/stdlib.h"
#include "../../common/rbtree.h"
#include "../../common/syscall.h"
#include "../../common/vma.h"

//do typedef earlier due to recursive reference
typedef struct vma_node vma_node_t;

struct vma_node {
    vma_node_t *next_freed;
    vm_area_t vma;
};

void vma_init(size_t max_entries);

vm_area_t *vma_alloc();

vm_area_t *vma_zalloc();

void vma_free(vm_area_t *addr);

void vma_gap_update(vm_area_t *vma);

uint64_t vma_prot_to_pg(uint64_t prot);

uint64_t
mmap_region(vm_file_t *file_info, uint64_t addr, uint64_t length, uint64_t vma_flags, uint64_t vma_prot, uint64_t offset,
            vm_area_t **vma_out);

#endif //PROJECT_VMA_HEAP_H
