//
// Created by ben on 30/12/17.
//

#include "vma_heap.h"

#ifndef PROJECT_VMA_H
#define PROJECT_VMA_H

int vma_split(vm_area_t *, uint64_t, int);

void vma_link(vm_area_t *, vm_area_t *, rb_node_t **, rb_node_t *);

void vma_list_remove(vm_area_t *);

int vma_find_links(uint64_t,
                   uint64_t, vm_area_t **,
                   rb_node_t ***, rb_node_t **);

vm_area_t *
vma_merge(vm_area_t *, uint64_t, uint64_t, uint64_t, vm_file_t *, uint64_t);

void detach_vmas_to_be_unmapped(vm_area_t *, vm_area_t *, uint64_t);

void unmap_vma(vm_area_t *vma);

#endif //PROJECT_VMA_H
