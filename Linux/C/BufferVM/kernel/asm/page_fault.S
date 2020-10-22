#include "../h/kernel_as.h"

.section .text
.global idt_page_fault_handler
.type idt_page_fault_handler, @function
idt_page_fault_handler:
    cli

    btq $2, (%rsp)
    jnc .kernel_page_fault

    swapgs
1:
    sets %gs:CPU_IN_KERN
    movq %rcx, %gs:CPU_TMP_R15

    jc .page_fault_user_check
.page_fault_user_exit:

    //enter c page fault handler

.handle_page_fault:
    pushScratchRegs()

    movq %cr2, %rdi
    movq 64(%rsp), %rsi
    movq 72(%rsp), %rdx
    call handle_user_page_fault
    testq %rax, %rax
    je .unhandled_page_fault

    popScratchRegs()

    movq %gs:CPU_TMP_R15, %rcx

    btq $2, (%rsp)
    jnc .page_fault_exit
    swapgs
.page_fault_exit:
    addq $8, %rsp
    iretq
.kernel_page_fault:
    pushScratchRegs()
    movq %cr2, %rdi
    movl 80(%rsp), %esi
    movq 72(%rsp), %rdx
    call handle_kernel_page_fault

    popScratchRegs()

    jmp .page_fault_exit
.unhandled_page_fault:
    //all bets are off here - clobber the fuck out of everything

    popScratchRegs()

    movabs $LC0, %rdi
    movq %cr2, %rsi
    movq 8(%rsp), %rdx
    movq (%rsp), %rcx
    movq 24(%rsp), %r8
    clr %rax       # needed for printf
    call printf
    movq 8(%rsp), %rdi
    movq $5, %rsi
    call disassemble_address
    call kernel_exit
.page_fault_user_check:
    movq (%rsp), %rcx
    btq $0, %rcx
    jc .page_fault_version_check
    jmp .page_fault_user_exit
.page_fault_version_check:
    //check if pointer same as last fault, and in single step mode - if so not version fault!
    //get pointer version - check against lookup
    //if OK, set single step mode and store current pc - clear flag on page entry
    //jump to pc

    pushScratchRegs()

    movq %cr2, %rdi
    movq 72(%rsp), %rsi
    call check_version
    testq %rax, %rax
    je .version_error

    popScratchRegs()

    //clone return address on stack for retq
    //we replace error value currently on stack
    movq 8(%rsp), %rcx
    movq %rcx, (%rsp)

    movl $2, %gs:CPU_TMP_DCNTR
    movq 32(%rsp), %rcx
    movq %rcx, %gs:CPU_TMP_DSP

    movq 24(%rsp), %rcx
    orq $SINGLE_STEP_F, %rcx
    movq %rcx, (%rsp)
    //restore register
    movq %gs:CPU_TMP_R15, %rcx
    swapgs
    popf

    #allows us to jump without clobbering register
    retq

.version_error:
    movabs $LC1, %rdi
    clr %rax       # needed for printf
    call printf

    call kernel_exit

.section .data
LC0:
        .ascii "Unhandled Page fault - VA: %p PC: %p Error: %d SP: %p\n\0"
LC1:
    .ascii "Version error\n\0"
