//
// Created by ben on 17/10/16.
//

#include <stdio.h>
#include "../h/gdt.h"
#include "../h/tss.h"
#include "../../common/paging.h"
#include "../h/cpu.h"
#include "../h/kernel_as.h"

void fill_segment_descriptor(uint64_t *dt, struct gdt_segment *seg)
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
    printf("i %p\n", dt[index+1]);
}

void gdt_init(uint64_t gdt_page){
    gdt.limit = PAGE_SIZE - 1;
    gdt.base = (void *)gdt_page;
    gdt_load((uint64_t)&gdt);

    /*struct gdt_segment seg = {
            .selector = CPU_S,
            .type = 3,
            .base = (uint64_t)&cpu,
            .limit = sizeof(cpu_t) - 1,
            .present = 1,
            .dpl = 3,
            .s = 0,
            .avl = 0,
            .l = 1,
            .db = 0,
            .g = 0
    };

    printf("gdt %p\n", seg.base);

    fill_segment_descriptor((void*)gdt_page, &seg);*/
}