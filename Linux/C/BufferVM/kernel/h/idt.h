//
// Created by ben on 13/10/16.
//

#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include "../../libc/stdlib.h"

#define IDT_MAX 256

struct idt_entry
{
    uint16_t handler_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t handler_medium;
    uint32_t handler_high;
    uint32_t zero;
} __attribute__((packed));

struct idtr
{
    uint16_t limit;
    uintptr_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_MAX];
static struct idtr idtr;

extern void idt_null_handler(void);
extern void idt_gpf_handler(void);
extern void idt_page_fault_handler(void);
extern void idt_debug_handler(void);
extern void idt_invalid_opcode_handler(void);

extern void idt_load(uint64_t idtr);

void idt_init(bool bsp);
void idt_set_gate(uint8_t num, uint64_t handler, uint8_t dpl);

#endif //KERNEL_IDT_H
