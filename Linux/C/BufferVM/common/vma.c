//
// Created by ben on 30/12/17.
//

#include <stdlib.h>
#include <stdio.h>
#include "vma.h"
#include "utils.h"
#include "paging.h"

bool vma_contiguous(vm_area_t *vma) {
#ifdef VM
    uint64_t old_phys, new_phys, start, end, i;

    start = PAGE_ALIGN_DOWN(vma->start_addr);
    end = PAGE_ALIGN(vma->end_addr);

    for (i = start; i < end; i += PAGE_SIZE) {
        if (!get_phys_addr(i,  &new_phys)){
            printf("Missing page\n");
            return (false);
        }

        if (i != start && old_phys + PAGE_SIZE != new_phys){
            printf("PA %lx not next to %lx\n", old_phys, new_phys);
            return (false);
        }

        old_phys = new_phys;
    }

    return (true);
#else
    return (false);
#endif
}

void vma_print_node(vm_area_t *vma, bool follow, size_t page_count) {
    if (vma == NULL){
        printf("-------------------------------------\n");
        printf("Total 4kb pages: %ld/%ld 2mb pages (est): %ld/%ld bytes: %lx/%ld\n",
               page_count, PHYSICAL_HEAP_PAGES,
               page_count / 500, PHYSICAL_HEAP_PAGES / 500,
               page_count * PAGE_SIZE, PHYSICAL_HEAP_PAGES * PAGE_SIZE);
        return;
    }

    size_t pages = PAGE_DIFFERENCE(vma->end_addr, vma->start_addr);
    printf("VMA addr: %p end: %p file: %d pages: %ld grows: %d faulted: %d updated: %d contiguous: %d\n",
           (void *) vma->start_addr, (void *) vma->end_addr, vma->file_info.fd, pages,
           (int)((vma->flags & VMA_GROWS) == VMA_GROWS),
           (int)((vma->flags & VMA_IS_PREFAULTED) == VMA_IS_PREFAULTED),
           (int)((vma->flags & VMA_HAS_RESIZED) == VMA_HAS_RESIZED),
           vma_contiguous(vma));

    if (follow)
        vma_print_node(vma_ptr(vma->next), true, page_count + pages);
}

void vma_print() {
    vm_area_t *start;
    printf("\nVMAs:\n");
    printf("-------------------------------------\n");

    start = vma_ptr(*vma_list_start_ptr);

    if (start != NULL) {
        vma_print_node(start, true, 0);
        printf("\n");
    } else {
        printf("No VMAs allocated\n");
    }
}

vm_area_t *vma_find_phys(uint64_t addr) {
    vm_area_t *vma = vma_ptr(*vma_list_start_ptr);

    while (vma) {

        if (addr >= vma->start_addr && addr < vma->end_addr) {
            return (vma);
        }
        vma = vma->next;
    }

    return (NULL);
}

vm_area_t *vma_find(uint64_t addr) {
    rb_node_t *rb_node;
    vm_area_t *vma = NULL;

    rb_node = vma_rb_ptr(vma_rb_root_ptr->rb_node);

    while (rb_node) {
        vm_area_t *tmp;

        tmp = container_of(rb_node, vm_area_t, rb_node);

        if (tmp->end_addr > addr) {
            vma = tmp;
            if (tmp->start_addr <= addr) {
                break;
            }
            rb_node = vma_rb_ptr(rb_node->rb_left);
        } else
            rb_node = vma_rb_ptr(rb_node->rb_right);
    }

    return vma;
}
