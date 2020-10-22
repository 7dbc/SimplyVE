//
// Created by ben on 15/10/16.
//

#include "gdt.h"

void fill_segment_descriptor(uint64_t *dt, struct kvm_segment *seg)
{
    uint16_t index = seg->selector >> 3;
    uint64_t limit = seg->g ? seg->limit >> 12 : seg->limit;
    dt[index] = (limit & 0xffff) /* Limit low bits 0:15 */
                | (seg->base & 0xffffff) << 16 /* Base low bits 0:23 */
                | (uint64_t)seg->type << 40
                | (uint64_t)seg->s << 44 /* system or code/data */
                | (uint64_t)seg->dpl << 45 /* Privilege level */
                | (uint64_t)seg->present << 47
                | ((limit & 0xf0000ULL) >> 16) << 48 /* Limit bits 16:19 */
                | (uint64_t)seg->avl << 52 /* Available for system software */
                | (uint64_t)seg->l << 53 /* 64-bit code segment */
                | (uint64_t)seg->db << 54 /* 16/32-bit segment */
                | (uint64_t)seg->g << 55 /* 4KB granularity */
                | ((seg->base & 0xff000000ULL) >> 24) << 56; /* Base bits 24:31 */

    dt[index+1] = (seg->base & 0xffffffff00000000ULL) >> 32;
}