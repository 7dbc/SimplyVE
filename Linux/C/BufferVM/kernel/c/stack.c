//
// Created by ben on 27/12/16.
//

#include "../h/stack.h"
#include "../../common/paging.h"
#include "../h/vma_heap.h"
#include "../../common/utils.h"
#include "../h/host.h"
#include "../h/syscall.h"
#include "../h/vma.h"

/**
 * Stack start address
 */
uint64_t user_stack_start;
/**
 * Current stack page address
 */
uint64_t user_stack_page;
/**
 * Stack min address. mmap VMAs sits below here.
 */
uint64_t user_stack_min;
/**
 * Stack page limit - max number of stack pages
 */
uint64_t user_stack_page_limit;
/**
 * Stack page limit - max number of stack pages
 */
vm_area_t *user_stack_vma;

void user_stack_init(uint64_t _user_stack_start, const uint64_t stack_page_limit){

    uint64_t addr;

    user_stack_start = _user_stack_start;

    user_stack_page = _user_stack_start - PAGE_SIZE;

    user_stack_min = _user_stack_start - stack_page_limit * PAGE_SIZE;

    user_stack_page_limit = stack_page_limit;

    addr = mmap_region(NULL, user_stack_page, PAGE_SIZE, VMA_GROWS, VMA_WRITE | VMA_READ, 0, &user_stack_vma);

    ASSERT(addr == user_stack_page);
}

int grow_stack(vm_area_t *vma, uint64_t addr){

    uint64_t new_start = PAGE_ALIGN(addr) - PAGE_SIZE;

    if (PAGE_DIFFERENCE(user_stack_start, new_start) > user_stack_page_limit)
        return 1;

    vma->start_addr = new_start;
    user_stack_page = new_start;

    vma_gap_update(vma);

    //Remap VMA, faulting mappings
    kernel_map_vma(vma);

    return 0;
}