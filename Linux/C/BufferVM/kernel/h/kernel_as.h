//
// Created by ben on 14/10/16.
//

#ifndef KERNEL_KERNEL_AS_H
#define KERNEL_KERNEL_AS_H

#define KERNEL_CS	0x10
#define KERNEL_DS	0x18
#define USER_CS	    0x30
#define USER_DS	    0x28

#define TSS_S	    0x40
#define CPU_S       0x50

#define CPU_TMP_R15       0
#define CPU_TMP_RSP       8
#define CPU_TMP_DSP       16
#define CPU_TMP_DCNTR       24
#define CPU_IN_KERN       32

#define CPU_REG_BASE    40
#define CPU_REG_RAX       CPU_REG_BASE
#define CPU_REG_RDI  CPU_REG_RAX+8
#define CPU_REG_RSI  CPU_REG_RDI+8
#define CPU_REG_RDX  CPU_REG_RSI+8
#define CPU_REG_RCX  CPU_REG_RDX+8
#define CPU_REG_RBX  CPU_REG_RCX+8
#define CPU_REG_RSP  CPU_REG_RBX+8
#define CPU_REG_RBP  CPU_REG_RSP+8
#define CPU_REG_R8  CPU_REG_RBP+8
#define CPU_REG_R9  CPU_REG_R8+8
#define CPU_REG_R10  CPU_REG_R9+8
#define CPU_REG_R11  CPU_REG_R10+8
#define CPU_REG_R12  CPU_REG_R11+8
#define CPU_REG_R13  CPU_REG_R12+8
#define CPU_REG_R14  CPU_REG_R13+8
#define CPU_REG_R15  CPU_REG_R14+8

/* AMD64 MSR values */
#define AMD64_MSR_EFER			0xC0000080
#define AMD64_MSR_STAR			0xC0000081
#define AMD64_MSR_LSTAR			0xC0000082
#define AMD64_MSR_CSTAR			0xC0000083
#define AMD64_MSR_SFMASK	0xC0000084

#define SINGLE_STEP_F 0x0100

#define MSR_FS_BASE		0xc0000100 /* 64bit FS base */
#define MSR_GS_BASE		0xc0000101 /* 64bit GS base */
#define MSR_KERNEL_GS_BASE	0xc0000102 /* SwapGS GS shadow - set this in kernel to effect userland */

#define SINGLE_STEP_DEBUG 0

#if (SINGLE_STEP_DEBUG == 1)
#define check_ss_mode(flags) \
    orl $SINGLE_STEP_F, flags;
#else
#define check_ss_mode(flags)
#endif

#define pushScratchRegs() \
    pushq %rax; \
    pushq %rdi; \
    pushq %rsi; \
    pushq %rdx; \
    pushq %r8; \
    pushq %r9; \
    pushq %r10; \
    pushq %r11;

#define pushAllRegs(sp_reg) \
    movq %rax, %gs:CPU_REG_RAX; \
    movq %rdi, %gs:CPU_REG_RDI; \
    movq %rsi, %gs:CPU_REG_RSI; \
    movq %rdx, %gs:CPU_REG_RDX; \
    movq %rcx, %gs:CPU_REG_RCX; \
    movq %rbx, %gs:CPU_REG_RBX; \
    movq sp_reg, %rbx; \
    movq %rbx, %gs:CPU_REG_RSP; \
    movq %rbp, %gs:CPU_REG_RBP; \
    movq %r8, %gs:CPU_REG_R8; \
    movq %r9, %gs:CPU_REG_R9; \
    movq %r10, %gs:CPU_REG_R10; \
    movq %r11, %gs:CPU_REG_R11; \
    movq %r12, %gs:CPU_REG_R12; \
    movq %r13, %gs:CPU_REG_R13; \
    movq %r14, %gs:CPU_REG_R14; \
    movq %r15, %gs:CPU_REG_R15;

#define popScratchRegs() \
    popq %r11; \
    popq %r10; \
    popq %r9; \
    popq %r8; \
    popq %rdx; \
    popq %rsi; \
    popq %rdi; \
    popq %rax;

#define popAllRegs() \
    movq %gs:CPU_REG_RAX, %rax; \
    movq %gs:CPU_REG_RDI, %rdi; \
    movq %gs:CPU_REG_RSI, %rsi; \
    movq %gs:CPU_REG_RDX, %rdx; \
    movq %gs:CPU_REG_RCX, %rcx; \
    movq %gs:CPU_REG_RBX, %rbx; \
    movq %gs:CPU_REG_RBP, %rbp; \
    movq %gs:CPU_REG_R8, %r8; \
    movq %gs:CPU_REG_R9, %r9; \
    movq %gs:CPU_REG_R10, %r10; \
    movq %gs:CPU_REG_R11, %r11; \
    movq %gs:CPU_REG_R12, %r12; \
    movq %gs:CPU_REG_R13, %r13; \
    movq %gs:CPU_REG_R14, %r14; \
    movq %gs:CPU_REG_R15, %r15;

#endif //KERNEL_KERNEL_AS_H
