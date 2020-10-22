#include "../h/kernel_as.h"

        .section .text
        .global _start
        .global switch_user_land
_start:
	call kernel_main

	call kernel_exit
switch_user_land:
    push $(USER_DS | 3)
    push %rsi
    push $(0x200) #EFLAGS (enable ints)
    check_ss_mode((%rsp))
    push $(USER_CS | 3) #(yes that is the correct GDT entry for DPL = 3 long mode)
    push %rdi
.clear_regs:
    movw $0, %ax
    #movw %ax, %gs
    movq $0, %rax
    movq $0, %rbx
    movq $0, %rcx
    movq $0, %rdx
    movq $0, %rsi
    movq $0, %rdi
    movq $0, %rbp
    movq $0, %r8
    movq $0, %r9
    movq $0, %r10
    movq $0, %r11
    movq $0, %r12
    movq $0, %r13
    movq $0, %r14
    movq $0, %r15
    swapgs
.enter:
    iretq
