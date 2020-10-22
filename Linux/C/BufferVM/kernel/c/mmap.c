//
// Created by ben on 27/12/16.
//

#include "../../libc/stdlib.h"
#include "../../common/syscall.h"
#include "../../common/paging.h"
#include "../h/kernel.h"
#include "../h/vma_heap.h"
#include "../h/stack.h"
#include "../../common/utils.h"
#include "../h/host.h"
#include "../h/vma.h"
#include "../../common/vma.h"


static uint64_t find_next_gap(uint64_t low_range, uint64_t high_range, uint64_t length) {
    vm_area_t *vma;
    uint64_t input_low_range, input_high_range, gap_start, gap_end;

    input_low_range = low_range;
    input_high_range = high_range;

    gap_end = high_range;
    if (gap_end < length)
        return -ENOMEM;
    high_range = gap_end - length;

    if (low_range > high_range)
        return -ENOMEM;
    low_range = low_range + length;

    //search for gap, starting at highest address and working down
    gap_start = vma_highest_addr;
    if (gap_start <= high_range)
        goto found_highest;

    //check root node
    if (vma_rb_root.rb_node == NULL)
        return -ENOMEM;

    vma = container_of(vma_rb_root.rb_node, vm_area_t, rb_node);
    if (vma->rb_subtree_gap < length)
        return -ENOMEM;

    while (true) {
        gap_start = vma->prev ? vma->prev->end_addr : 0;
        if (gap_start <= high_range && vma->rb_node.rb_right) {
            vm_area_t *right =
                    container_of(vma->rb_node.rb_right, vm_area_t, rb_node);
            if (right->rb_subtree_gap >= length) {
                vma = right;
                continue;
            }
        }

        check_current:
        //is there a gap at the current node?
        gap_end = vma->start_addr;
        if (gap_end < low_range)
            return -ENOMEM;
        if (gap_start <= high_range && gap_end - gap_start >= length)
            goto found;

        if (vma->rb_node.rb_left) {
            vm_area_t *left =
                    container_of(vma->rb_node.rb_left, vm_area_t, rb_node);
            if (left->rb_subtree_gap >= length) {
                vma = left;
                continue;
            }
        }

        //find next node back up rbtree
        while (true) {
            rb_node_t *prev = &vma->rb_node;
            if (!rb_parent(prev))
                return -ENOMEM;
            vma = container_of(rb_parent(prev), vm_area_t, rb_node);
            if (prev == vma->rb_node.rb_right) {
                gap_start = vma->prev ?

                            vma->prev->end_addr : 0;
                goto check_current;
            }
        }
    }

    found:
    if (gap_end > input_high_range)
        gap_end = input_high_range;

    found_highest:
    //calc highest gap address
    gap_end -= length;

    ASSERT(gap_end >= input_low_range);
    ASSERT(gap_end >= gap_start);

    return gap_end;
}

static uint64_t get_unmapped_area_unchecked(uint64_t addr, uint64_t length, uint64_t offset, uint64_t flags) {
    vm_area_t *vma;

    if (!(flags & MAP_FIXED)) {
        //address hint present
        if (addr) {
            addr = PAGE_ALIGN(addr);
            vma = vma_find(addr);
            if (kernel_min_address - length >= addr &&
                (!vma || addr + length <= vma->start_addr))
                return addr;
        } else {
            return find_next_gap(0, user_stack_min, length);
        }
    } else
        return addr;
}

static uint64_t get_unmapped_area(uint64_t addr, uint64_t length, uint64_t offset, uint64_t flags) {

    if (flags & MAP_FIXED)
        return addr;

    addr = get_unmapped_area_unchecked(addr, length, offset, flags);

    if (addr > kernel_min_address - length)
        return -ENOMEM;

    if (addr & ~PAGE_MASK)
        return -EINVAL;

    return addr;
}

int syscall_munmap(uint64_t addr, size_t length) {

    uint64_t end;
    vm_area_t *vma, *prev, *last;

    if (addr & ~PAGE_MASK || addr > kernel_min_address || length > kernel_min_address - addr || length == 0)
        return -EINVAL;

    //align length to page interval like mmap
    length = PAGE_ALIGN(length);

    vma = vma_find(addr);

    if (!vma)
        return 0;

    prev = vma->prev;

    end = addr + length;

    /*
     * Confirm bounds
     * TODO - move into vma_find?
     */
    if (vma->start_addr >= end)
        return 0;

    //is the address past the start vma (of defined region)
    if (addr > vma->start_addr) {
        int error;

        //check for sysctl_max_map_count
        //switch for null check on allocation for split.
        /*if (end < vma->end_addr)
            return -ENOMEM;*/

        error = vma_split(vma, addr, 0);
        if (error)
            return error;
        prev = vma;
    }

    //is the end past the end vma
    last = vma_find(end);
    if (last && end > last->start_addr) {
        int error = vma_split(last, end, 1);
        if (error)
            return error;
    }
    vma = prev ? prev->next : vma_list_start;

    detach_vmas_to_be_unmapped(vma, prev, end);
    //now we can walk the unlinked list of VMAs to remove/free any resources
    vma_list_remove(vma);

    return 0;


}

uint64_t
mmap_region(vm_file_t *file_info, uint64_t addr, uint64_t length, uint64_t vma_flags, uint64_t vma_prot, uint64_t offset,
            vm_area_t **vma_out) {

    vm_area_t *vma, *prev, *next;
    rb_node_t **rb_link, *rb_parent;
    vm_file_t null_file_info = {-1, -1, -1};

    if (file_info == NULL)
        file_info = &null_file_info;

    while (vma_find_links(addr, addr + length, &prev, &rb_link,
                          &rb_parent)) {
        if (syscall_munmap(addr, length)) {
            *vma_out = NULL;
            return -ENOMEM;
        }
    }

    //lets try and merge with an applicable region
    //see can_vma_merge_before and can_vma_merge_after for criteria
    vma = vma_merge(prev, addr, addr + length, vma_flags, file_info, offset);

    if (vma == NULL) {
        vma = vma_zalloc();

        vma->start_addr = addr;
        vma->end_addr = addr + length;
        vma->flags = vma_flags;
        vma->page_offset = offset;
        vma->page_prot = vma_prot;
        vma->file_info = *file_info;
        vma->phys_page_start = -1;

        vma_link(vma, prev, rb_link, rb_parent);
    }

    *vma_out = vma;

    return addr;
}

/*
 * Combine the mmap "flags" argument into "flags" used internally.
 */
static inline uint64_t
flag_to_vma(uint64_t flags) {
    return TRANSFER_FLAG(flags, MAP_GROWSDOWN, VMA_GROWS);
}

uint64_t syscall_mmap(uint64_t addr, size_t length, uint64_t prot, uint64_t flags, int fd, uint64_t offset) {

    //should anonymous mappings be done like sbrk?

    //check for map_fixed
    //if not, largely ignore the hint
    //if hint is null, ignore completely
    //else, force it past the end of the heap region for user (MAX_HEAP anyone?)
    //should force any hint to be bigger than brk (unless MAP_FIXED)

    //hack for now until mprotect
    //prot |= PROT_READ | PROT_WRITE | PROT_EXEC;

    vm_area_t *vma;
    int64_t phys_addr;
    uint64_t host_mmap_ret;
    uint64_t vma_flags = 0;
    uint64_t vma_prot = 0;
    vm_file_t file_info = {-1, -1, -1};
    int64_t org_length = length;

    if (offset & ~PAGE_MASK)
        return -EINVAL;

    //First check inputted length
    if (!length)
        return -EINVAL;

    //then check page aligned length
    length = PAGE_ALIGN(length);
    if (!length)
        return -EINVAL;

    addr = get_unmapped_area(addr, length, offset, flags);

    vma_flags |= flag_to_vma(flags);
    vma_prot |= prot_to_vma(prot) | TRANSFER_FLAG(flags, MAP_ANONYMOUS, VMA_IS_VERSIONED);

    if (fd != -1) {

        fd = kernel_dup(fd);

        //ASSERT(!IS_ERR_VALUE(fd));

        vm_stat_t stats;
        kernel_fstat(fd, &stats);

        //store inode for comparison later
        //add extra stuff here as needed?

        file_info.fd = fd;
        file_info.dev = stats.st_dev;
        file_info.inode = stats.st_ino;

        switch (flags & MAP_TYPE) {
            case MAP_SHARED:
                vma_flags |= VMA_SHARED;
            case MAP_PRIVATE:
                break;
            default:
                return -EINVAL;
        }
    } else {
        switch (flags & MAP_TYPE) {
            case MAP_SHARED:
                if (vma_flags & VMA_GROWS)
                    return -EINVAL;

                offset = 0;
                vma_flags |= VMA_SHARED;
                break;
            case MAP_PRIVATE:
                //nothing here
                //offset = addr >> PAGE_SHIFT;

                break;
            default:
                return -EINVAL;
        }
    }

    addr = mmap_region(&file_info, addr, length, vma_flags, vma_prot, offset, &vma);

    return addr;
}

uint64_t do_brk(uint64_t addr, uint64_t request, uint64_t flags) {
    uint64_t error, len, prot, pgoff = addr >> PAGE_SHIFT;
    vm_area_t *vma, *prev;
    rb_node_t **rb_link, *rb_parent;
    vm_file_t null_file_info = {-1, -1, -1};

    len = (uint64_t)PAGE_ALIGN(request);

    if (len < request)
        return (uint64_t)-ENOMEM;
    if (!len)
        return 0;

    prot  = VMA_READ | VMA_WRITE | VMA_EXEC;

    error = get_unmapped_area(addr, len, 0, MAP_FIXED);
    if (PAGE_OFFSET(error))
        return error;

    while (vma_find_links(addr, addr + len, &prev, &rb_link,
                          &rb_parent)) {
        if (syscall_munmap(addr, len))
            return (uint64_t)-ENOMEM;
    }

    //lets try and merge with an applicable region
    //see can_vma_merge_before and can_vma_merge_after for criteria
    vma = vma_merge(prev, addr, addr + len, flags, &null_file_info, pgoff);

    if (vma == NULL) {
        vma = vma_zalloc();

        vma->start_addr = addr;
        vma->end_addr = addr + len;
        vma->flags = flags;
        vma->page_offset = pgoff;
        vma->page_prot = prot;
        vma->file_info = null_file_info;
        vma->phys_page_start = -1;

        vma_link(vma, prev, rb_link, rb_parent);
    }

    return 0;
}

int syscall_mprotect(void *addr, size_t len, int prot){
    //TODO
    printf("mprotect %p of len %d with prot %x\n", addr, len, prot);
    return 0;
}
