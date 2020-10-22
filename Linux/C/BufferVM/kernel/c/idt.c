#include "../h/idt.h"
#include "../h/kernel_as.h"
#include "../h/tss.h"

void idt_set_gate(uint8_t num, uint64_t handler, uint8_t dpl)
{
    /* Set the ISR's base address */
    idt[num].handler_low = (handler & 0xFFFF);
    idt[num].handler_medium = (handler >> 16) & 0xFFFF;
    idt[num].handler_high = (handler >> 32) & 0xFFFFFFFF;

    /* Set the ISR's code segment selector */
    idt[num].selector = KERNEL_CS;
    idt[num].ist = 0;
    idt[num].zero = 0;

    /* Build the entry's access flags */
    idt[num].flags = 0xE;                 // interrupt gate
    idt[num].flags |= (dpl & 0b11) << 5;  // dpl
    idt[num].flags |= (1 << 7); // present
}

void idt_init(bool bsp)
{

    size_t i;
    for (i = 0; i < IDT_MAX; ++i) {
        idt_set_gate(i, (uint64_t) &idt_null_handler, 0x0);
    }

    idt_set_gate(14, (uint64_t) &idt_page_fault_handler, 0x0);
    idt_set_gate(1, (uint64_t) &idt_debug_handler, 0x0);
    //idt_set_gate(0x06, (uint64_t) &idt_page_fault_handler, 0x0);
    idt_set_gate(0x06, (uint64_t) &idt_invalid_opcode_handler, 0x0);
    idt_set_gate(0x0D, (uint64_t) &idt_gpf_handler, 0x0);

    /* Running on the bootstrap processor */
    if (bsp)
    {
        /* Set up the IDT register structure */
        idtr.limit = (sizeof(struct idt_entry) * IDT_MAX) - 1;
        idtr.base = (uintptr_t) &idt;
    }

    /* Load our new IDT */
    idt_load((uint64_t)&idtr);
}