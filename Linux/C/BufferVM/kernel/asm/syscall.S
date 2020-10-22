#include "../h/kernel_as.h"
#include "../../common/syscall_as.h"

    .section .text
    .global syscall_entry
    .global syscall_setup
    .global syscall_null_handler
syscall_setup:
    # Set the kernel EFLAGS mask value
    movl $(SINGLE_STEP_F), %eax
    movl $0x00, %edx
	movl $(AMD64_MSR_SFMASK), %ecx
	wrmsr
    # Set the kernel segment registers
    movl $(KERNEL_CS | ((USER_DS - 8) << 16)), %edx
    movl $(AMD64_MSR_STAR), %ecx
    wrmsr
    # Set the kernel 64-bit entry point
    movabs $syscall_entry, %rcx
    movl %ecx, %eax
    shrq $32, %rcx
    movl %ecx, %edx
    movl $(AMD64_MSR_LSTAR), %ecx
    wrmsr
    # Finally enable syscall
    movl $(AMD64_MSR_EFER), %ecx
    rdmsr
    orl $1, %eax
    wrmsr
    #return
    ret
syscall_entry:
    swapgs
    #use %r11 so we do not trash syscall args
    #we cannot trash %rcx and %r11 as they are ret pointer and RFLAGs
    movq %r15, %gs:CPU_TMP_R15
    movq %rsp, %gs:CPU_TMP_RSP

    movabs $kernel_stack, %rsp
    movq (%rsp), %rsp

    pushq %rbp
    pushq %rcx
    pushq %r11

    #bounds check
    cmpq $0, %rax
    jl syscall_entry_bail
    cmpq $(SYSCALL_MAX), %rax
    jge syscall_entry_bail

    #swap to match C amd64 abi
    movq %r10, %rcx

    movabs $syscall_table, %r15
    movq (%r15,%rax,8), %r15
    call *%r15

    movq %rcx, %r10

syscall_entry_bail:
    popq %r11
    popq %rcx
    popq %rbp

    movq %gs:CPU_TMP_R15, %r15
    movq %gs:CPU_TMP_RSP, %rsp

    swapgs
    sysretq
syscall_null_handler:
    movabs $LC0, %rdi
    movq %rax, %rsi
    clr %rax       # needed for printf
    call printf
    movq $-1, %rax #return -1
    ret
.section .data
LC0:
        .ascii "Syscall: %ld\n\0"
