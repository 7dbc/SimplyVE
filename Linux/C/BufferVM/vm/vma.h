//
// Created by ben on 30/12/17.
//

#ifndef PROJECT_VMA_H
#define PROJECT_VMA_H

#include <stdint.h>
#include "../common/rbtree.h"
#include "../common/vma.h"

void kernel_map_vma(vm_area_t *);

void kernel_unmap_vma(vm_area_t *);

int kernel_set_vma_heap(uint64_t, vm_area_t **, rb_root_t *);

#endif //PROJECT_VMA_H
