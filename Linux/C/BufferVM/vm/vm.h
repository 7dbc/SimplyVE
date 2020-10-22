//
// Created by ben on 09/10/16.
//

#ifndef BUFFERVM_VM_H
#define BUFFERVM_VM_H

#include <stdint.h>
#include "../common/tss.h"

#define CPU_START 0xFFFBC000

/* CR0 bits */
#define CR0_PE 1
#define CR0_MP (1 << 1)
#define CR0_EM (1 << 2)
#define CR0_TS (1 << 3)
#define CR0_ET (1 << 4)
#define CR0_NE (1 << 5)
#define CR0_WP (1 << 16)
#define CR0_AM (1 << 18)
#define CR0_NW (1 << 29)
#define CR0_CD (1 << 30)
#define CR0_PG (1 << 31)

/* CR4 bits */
#define CR4_VME 1
#define CR4_PVI (1 << 1)
#define CR4_TSD (1 << 2)
#define CR4_DE (1 << 3)
#define CR4_PSE (1 << 4)
#define CR4_PAE (1 << 5)
#define CR4_MCE (1 << 6)
#define CR4_PGE (1 << 7)
#define CR4_PCE (1 << 8)
#define CR4_OSFXSR (1 << 9)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_UMIP (1 << 11)
#define CR4_VMXE (1 << 13)
#define CR4_SMXE (1 << 14)
#define CR4_FSGSBASE (1 << 16)
#define CR4_PCIDE (1 << 17)
#define CR4_OSXSAVE (1 << 18)
#define CR4_SMEP (1 << 20)
#define CR4_SMAP (1 << 21)

#define EFER_SCE 1
#define EFER_LME (1 << 8)
#define EFER_LMA (1 << 10)
#define EFER_NXE (1 << 11)

typedef struct kvm_segment kvm_segment_t;

typedef struct cpu_t {
    uint64_t rtmp_r15;
    uint64_t rtmp_rsp;
    uint64_t rtmp_dsp;
    uint64_t rtmp_dcntr;
} __attribute__
((packed))cpu_t;

struct vm_t {
    int sys_fd;
    int fd;
    char *mem;
};

struct vcpu_t {
    int fd;
    struct kvm_run *kvm_run;
};

extern struct vm_t vm;

void vm_init(struct vm_t *vm, size_t mem_size);

void vcpu_init(struct vm_t *vm, struct vcpu_t *vcpu);

void kvm_show_regs(struct vm_t *vm);

void run(struct vm_t *vm, struct vcpu_t *vcpu, int kernel_binary_fd, int argc, char *argv[], char *envp[]);

#endif //BUFFERVM_VM_H
