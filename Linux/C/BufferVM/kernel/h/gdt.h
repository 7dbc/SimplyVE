//
// Created by ben on 16/01/17.
//

#ifndef PROJECT_GDT_H
#define PROJECT_GDT_H

#include "../../libc/stdlib.h"

struct gdt_segment {
    uint64_t base;
    uint32_t limit;
    uint16_t selector;
    uint8_t  type;
    uint8_t  present, dpl, db, s, l, g, avl;
    uint8_t  unusable;
    uint8_t  padding;
};

struct gdt
{
    uint16_t limit;
    uintptr_t base;
} __attribute__((packed));

static struct gdt gdt;

void gdt_init(uint64_t gdt_page);

#endif //PROJECT_GDT_H
