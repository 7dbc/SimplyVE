//
// Created by ben on 20/10/16.
//

#include "../../libc/stdlib.h"

#ifndef PROJECT_CPU_H
#define PROJECT_CPU_H

typedef struct regs {
    uint64_t rax;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} __attribute__
    ((packed))regs_t;;

typedef struct cpu {
    uint64_t rtmp_r15;
    uint64_t rtmp_rsp;
    uint64_t rtmp_dsp;
    uint64_t rtmp_dcntr;
    uint64_t in_kernel;
    regs_t regs;
} __attribute__
    ((packed))cpu_t;

extern cpu_t cpu_info;

extern void cpu_init(void);

extern void invlpg(uint64_t);

#endif //PROJECT_CPU_H
