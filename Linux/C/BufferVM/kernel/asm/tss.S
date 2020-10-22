#include "../h/kernel_as.h"

    .section .text
    .global tss_load
    .global gdt_load
tss_load:
    movw $(TSS_S | 3), %ax
    ltr %ax
    ret
gdt_load:
    lgdt (%rdi)
    ret