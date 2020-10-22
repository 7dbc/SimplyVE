//
// Created by ben on 30/12/17.
//

#include "../h/vma.h"
#include "../h/vma_heap.h"
#include "../../common/paging.h"
#include "../../common/utils.h"
#include "../h/host.h"
#include "../h/cpu.h"

inline vm_area_t *vma_ptr(vm_area_t *vma) {
    return (vma);
}

inline rb_node_t *vma_rb_ptr(rb_node_t *vma){
    return (vma);
}

int vma_find_links(uint64_t addr,
                   uint64_t end, vm_area_t **pprev,
                   rb_node_t ***rb_link, rb_node_t **rb_parent) {
    rb_node_t **__rb_link, *__rb_parent, *rb_prev;

    __rb_link = &vma_rb_root.rb_node;
    rb_prev = __rb_parent = NULL;

    while (*__rb_link) {
        vm_area_t *vma_tmp;

        __rb_parent = *__rb_link;
        vma_tmp = container_of(__rb_parent, vm_area_t, rb_node);

        if (vma_tmp->end_addr > addr) {
            /* Fail if an existing vma overlaps the area */
            if (vma_tmp->start_addr < end)
                return -ENOMEM;
            __rb_link = &__rb_parent->rb_left;
        } else {
            rb_prev = __rb_parent;
            __rb_link = &__rb_parent->rb_right;
        }
    }

    *pprev = NULL;
    if (rb_prev)
        *pprev = container_of(rb_prev, vm_area_t, rb_node);
    *rb_link = __rb_link;
    *rb_parent = __rb_parent;
    return 0;
}

static void vma_link_list(vm_area_t *vma,
                          vm_area_t *prev, rb_node_t *rb_parent) {
    vm_area_t *next;

    vma->prev = prev;
    if (prev) {
        next = prev->next;
        prev->next = vma;
    } else {
        //keep pointer to list start, for walking purposes
        vma_list_start = vma;
        if (rb_parent)
            next = container_of(rb_parent, vm_area_t, rb_node);
        else
            next = NULL;
    }
    vma->next = next;
    if (next)
        next->prev = vma;
}

static long vma_compute_subtree_gap(vm_area_t *vma) {
    uint64_t max, subtree_gap;
    max = vma->start_addr;
    if (vma->prev)
        max -= vma->prev->end_addr;
    if (vma->rb_node.rb_left) {
        subtree_gap = container_of(vma->rb_node.rb_left, vm_area_t, rb_node)->rb_subtree_gap;
        if (subtree_gap > max)
            max = subtree_gap;
    }
    if (vma->rb_node.rb_right) {
        subtree_gap = container_of(vma->rb_node.rb_right, vm_area_t, rb_node)->rb_subtree_gap;
        if (subtree_gap > max)
            max = subtree_gap;
    }
    return max;
}

//rbtree stubs

inline void
vma_rb_gap_propagate(rb_node_t *rb, rb_node_t *stop) {
    while (rb != stop) {
        vm_area_t *node = container_of(rb, vm_area_t, rb_node);
        uint64_t augmented = vma_compute_subtree_gap(node);
        if (node->rb_subtree_gap == augmented)
            break;
        node->rb_subtree_gap = augmented;
        rb = rb_parent(&node->rb_node);
    }
}

void
vma_rb_gap_rotate(rb_node_t *rb_old, rb_node_t *rb_new) {
    vm_area_t *old = container_of(rb_old, vm_area_t, rb_node);
    vm_area_t *new = container_of(rb_new, vm_area_t, rb_node);
    new->rb_subtree_gap = old->rb_subtree_gap;
    old->rb_subtree_gap = vma_compute_subtree_gap(old);
}

inline void
vma_rb_gap_copy(rb_node_t *rb_old, rb_node_t *rb_new) {
    vm_area_t *old = container_of(rb_old, vm_area_t, rb_node);
    vm_area_t *new = container_of(rb_new, vm_area_t, rb_node);
    new->rb_subtree_gap = old->rb_subtree_gap;
}

//rbtree state functions

void vma_gap_update(vm_area_t *vma) {
    vma_rb_gap_propagate(&vma->rb_node, NULL);
}

void vma_link_rb(vm_area_t *vma, rb_node_t **rb_link, rb_node_t *rb_parent) {
    /* Update tracking information for the gap following the new vma. */
    if (vma->next)
        vma_gap_update(vma->next);
    else
        vma_highest_addr = vma->end_addr;

    /*
     * vma->prev wasn't known when we followed the rbtree to find the
     * correct insertion point for that vma. As a result, we could not
     * update the vma rb_node parents rb_subtree_gap values on the way down.
     * So, we first insert the vma with a zero rb_subtree_gap value
     * (to be consistent with what we did on the way down), and then
     * immediately update the gap to the correct value. Finally we
     * rebalance the rbtree after all augmented values have been set.
     */
    rb_link_node(&vma->rb_node, rb_parent, rb_link);
    vma->rb_subtree_gap = 0;
    vma_gap_update(vma);
    rb_insert(&vma->rb_node, &vma_rb_root);
}

void vma_link(vm_area_t *vma, vm_area_t *prev, rb_node_t **rb_link, rb_node_t *rb_parent) {
    vma_link_list(vma, prev, rb_parent);
    vma_link_rb(vma, rb_link, rb_parent);

    //unmap_vma(vma);
    kernel_map_vma(vma);
}

void
detach_vmas_to_be_unmapped(vm_area_t *vma,
                           vm_area_t *prev, uint64_t end) {
    vm_area_t **insertion_point;
    vm_area_t *tail_vma = NULL;

    insertion_point = (prev ? &prev->next : &vma_list_start);
    vma->prev = NULL;
    do {
        rb_erase(&vma->rb_node, &vma_rb_root);
        tail_vma = vma;
        vma = vma->next;
    } while (vma && vma->start_addr < end);
    *insertion_point = vma;
    if (vma) {
        vma->prev = prev;
        vma_gap_update(vma);
    } else
        vma_highest_addr = prev ? prev->end_addr : 0;
    tail_vma->next = NULL;
}

static void vma_insert(vm_area_t *vma) {
    vm_area_t *prev;
    rb_node_t **rb_link, *rb_parent;

    if (vma_find_links(vma->start_addr, vma->end_addr,
                       &prev, &rb_link, &rb_parent))
        ASSERT(false);

    vma_link(vma, prev, rb_link, rb_parent);
    //mm->map_count++;
}

static inline void
vma_unlink(vm_area_t *vma, vm_area_t *prev) {
    vm_area_t *next;

    rb_erase(&vma->rb_node, &vma_rb_root);
    prev->next = next = vma->next;
    if (next)
        next->prev = prev;
}

void unmap_vma(vm_area_t *vma) {
    kernel_unmap_vma(vma);

    size_t pages = PAGE_DIFFERENCE(vma->end_addr, vma->start_addr);
    for (size_t i = 0; i < pages; i++){
        invlpg(vma->start_addr+i*PAGE_SIZE);
    }
}

static vm_area_t *vma_remove(vm_area_t *vma) {
    if (vma->file_info.fd != -1)
        kernel_close(vma->file_info.fd);

    unmap_vma(vma);
    vma_free(vma);
    return vma->next;
}

void vma_list_remove(vm_area_t *vma) {
    do {
        vma = vma_remove(vma);
    } while (vma);
}

int vma_adjust(vm_area_t *vma, uint64_t start, uint64_t end, uint64_t pgoff, vm_area_t *insert) {
    vm_area_t *next = vma->next;
    bool start_changed = false, end_changed = false;
    long adjust_next = 0;
    int remove_next = 0;

    if (next && !insert) {
        if (end >= next->end_addr) {
            /*
             * vma expands, overlapping all the next, and
             * perhaps the one after too (mprotect case 6).
             */
            remove_next = 1 + (end > next->end_addr);
            end = next->end_addr;

        } else if (end > next->start_addr) {
            /*
             * vma expands, overlapping part of the next:
             * mprotect case 5 shifting the boundary up.
             */
            adjust_next = PAGE_DIFFERENCE(end, next->start_addr);
        } else if (end < vma->end_addr) {
            /*
             * vma shrinks, and !insert tells it's not
             * split_vma inserting another: so it must be
             * mprotect case 4 shifting the boundary down.
             */
            adjust_next = -(PAGE_DIFFERENCE(vma->end_addr, end));
        }
    }
    again:

    if (start != vma->start_addr) {
        vma->start_addr = start;
        start_changed = true;
    }
    if (end != vma->end_addr) {
        vma->end_addr = end;
        end_changed = true;
    }
    vma->page_offset = pgoff;
    if (adjust_next) {
        next->start_addr += adjust_next << PAGE_SHIFT;
        next->page_offset += adjust_next;

        //unmap_vma(next);
        kernel_map_vma(next);
    }

    if (remove_next) {
        /*
         * vma_merge has merged next into vma, and needs
         * us to remove next before dropping the locks.
         */
        vma_unlink(next, vma);
    } else if (insert) {
        /*
         * split_vma has split insert from vma, and needs
         * us to insert it before dropping the locks
         * (it may either follow vma or precede it).
         */
        vma_insert(insert);
    } else {
        if (start_changed)
            vma_gap_update(vma);
        if (end_changed) {
            if (!next)
                vma_highest_addr = end;
            else if (!adjust_next)
                vma_gap_update(next);
        }
    }

    if (start_changed || end_changed){
        //unmap_vma(vma);
        kernel_map_vma(vma);
    }

    if (remove_next) {

        vma_remove(next);
        /*
         * In mprotect's case 6 (see comments on vma_merge),
         * we must remove another next too. It would clutter
         * up the code too much to do both in one go.
         */
        next = vma->next;
        if (remove_next == 2) {
            remove_next = 1;
            end = next->end_addr;
            goto again;
        } else if (next)
            vma_gap_update(next);
        else
            vma_highest_addr = end;
    }

    return 0;
}

static inline int vma_can_merge(vm_area_t *vma, vm_file_t *file_info, uint64_t vm_flags) {
    if ((vma->flags ^ vm_flags) & ~(VMA_IS_PREFAULTED | VMA_HAS_RESIZED))
        return 0;
    if (vma->file_info.dev != file_info->dev || vma->file_info.inode != file_info->inode)
        return 0;
    return 1;
}


static int
can_vma_merge_before(vm_area_t *vma, uint64_t vm_flags, vm_file_t *file_info, uint64_t vm_pgoff) {
    if (vma_can_merge(vma, file_info, vm_flags)) {
        if (vma->page_offset == vm_pgoff)
            return 1;
    }
    return 0;
}

static int
can_vma_merge_after(vm_area_t *vma, uint64_t vm_flags, vm_file_t *file_info, uint64_t vm_pgoff) {
    if (vma_can_merge(vma, file_info, vm_flags)) {
        uint64_t vm_pglen;
        vm_pglen = file_info->fd != -1 ? PAGE_DIFFERENCE(vma->end_addr, vma->start_addr) : 0;
        if (vma->page_offset + vm_pglen == vm_pgoff)
            return 1;
    }
    return 0;
}

vm_area_t *
vma_merge(vm_area_t *prev, uint64_t addr, uint64_t end, uint64_t vm_flags, vm_file_t *file_info, uint64_t pgoff) {
    uint64_t pglen = file_info->fd != -1 ? PAGE_DIFFERENCE(end, addr) : 0;
    vm_area_t *area, *next;
    int err;

    if (prev)
        next = prev->next;
    else
        next = vma_list_start;
    area = next;
    if (next && next->end_addr == end)        /* cases 6, 7, 8 */
        next = next->next;

    /*
     * Can it merge with the predecessor?
     */
    if (prev && prev->end_addr == addr &&
        can_vma_merge_after(prev, vm_flags, file_info, pgoff)) {
        /*
         * OK, it can.  Can we now merge in the successor as well?
         */
        if (next && end == next->start_addr &&
            can_vma_merge_before(next, vm_flags, file_info, pgoff + pglen)) {
            /* cases 1, 6 */
            err = vma_adjust(prev, prev->start_addr, next->end_addr, prev->page_offset, NULL);
        } else                    /* cases 2, 5, 7 */
            err = vma_adjust(prev, prev->start_addr, end, prev->page_offset, NULL);
        if (err)
            return NULL;
        return prev;
    }

    /*
     * Can this new request be merged in front of next?
     */
    if (next && end == next->start_addr &&
        can_vma_merge_before(next, vm_flags,
                             file_info, pgoff + pglen)) {
        if (prev && addr < prev->end_addr)    /* case 4 */
            err = vma_adjust(prev, prev->start_addr,
                             addr, prev->page_offset, NULL);
        else                    /* cases 3, 8 */
            err = vma_adjust(area, addr, next->end_addr,
                             next->page_offset - pglen, NULL);
        if (err)
            return NULL;
        return area;
    }

    return NULL;
}

int vma_split(vm_area_t *vma, uint64_t addr, int new_below) {
    vm_area_t *new;
    int err;

    new = vma_zalloc();
    //Set default phys page value
    new->phys_page_start = -1;

    if (!new)
        return -ENOMEM;

    //copy contents
    *new = *vma;

    if (new_below)
        new->end_addr = addr;
    else {
        new->start_addr = addr;
        new->page_offset += PAGE_ALIGN_DOWN(addr - vma->start_addr);
    }

    //dup file descriptor
    new->file_info.fd = kernel_dup(new->file_info.fd);

    if (new_below)
        err = vma_adjust(vma, addr, vma->end_addr, vma->page_offset + PAGE_ALIGN_DOWN(addr - vma->start_addr), new);
    else
        err = vma_adjust(vma, vma->start_addr, addr, vma->page_offset, new);

    if (!err)
        return 0;

    //on error, we need to clean up a few things - including the dup'd fd
    if (vma->file_info.fd != -1)
        kernel_close(new->file_info.fd);
    vma_free(new);
    return err;
}