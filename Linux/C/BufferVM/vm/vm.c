//
// Created by ben on 09/10/16.
//

#include <stdio.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <err.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <signal.h>
#include <errno.h>
#include <sys/uio.h>

#include "vm.h"
#include "../common/paging.h"
#include "elf.h"
#include "gdt.h"
#include "../common/elf.h"
#include "../common/syscall.h"
#include "vma.h"
#include "host.h"

extern const unsigned char bootstrap[], bootstrap_end[];

static size_t phy_mem_size;

void vm_init(struct vm_t *vm, size_t mem_size)
{
    int api_ver;
    struct kvm_userspace_memory_region memreg;

    phy_mem_size = mem_size;

    vm->sys_fd = open("/dev/kvm", O_RDWR);
    if (vm->sys_fd < 0) {
        perror("open /dev/kvm");
        exit(1);
    }

    api_ver = ioctl(vm->sys_fd, KVM_GET_API_VERSION, 0);
    if (api_ver < 0) {
        perror("KVM_GET_API_VERSION");
        exit(1);
    }

    if (api_ver != KVM_API_VERSION) {
        fprintf(stderr, "Got KVM api version %d, expected %d\n",
                api_ver, KVM_API_VERSION);
        exit(1);
    }

    vm->fd = ioctl(vm->sys_fd, KVM_CREATE_VM, 0);
    if (vm->fd < 0) {
        perror("KVM_CREATE_VM");
        exit(1);
    }

    vm->mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (vm->mem == MAP_FAILED) {
        perror("mmap mem");
        exit(1);
    }

    madvise(vm->mem, mem_size, MADV_MERGEABLE);

    printf("VM memory: %p to %p\n", vm->mem, vm->mem + mem_size);

    memreg.slot = 0;
    memreg.flags = 0;
    memreg.guest_phys_addr = 0;
    memreg.memory_size = mem_size;
    memreg.userspace_addr = (unsigned long)vm->mem;
    if (ioctl(vm->fd, KVM_SET_USER_MEMORY_REGION, &memreg) < 0) {
        perror("KVM_SET_USER_MEMORY_REGION");
        exit(1);
    }
}

void vcpu_init(struct vm_t *vm, struct vcpu_t *vcpu)
{
    int vcpu_mmap_size;

    vcpu->fd = ioctl(vm->fd, KVM_CREATE_VCPU, 0);
    if (vcpu->fd < 0) {
        perror("KVM_CREATE_VCPU");
        exit(1);
    }

    if (ioctl(vm->fd, KVM_SET_TSS_ADDR, TSS_KVM) < 0) {
        perror("KVM_SET_TSS_ADDR");
        exit(1);
    }

    vcpu_mmap_size = ioctl(vm->sys_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (vcpu_mmap_size <= 0) {
        perror("KVM_GET_VCPU_MMAP_SIZE");
        exit(1);
    }

    vcpu->kvm_run = mmap(NULL, vcpu_mmap_size, PROT_READ | PROT_WRITE,
                         MAP_SHARED, vcpu->fd, 0);
    if (vcpu->kvm_run == MAP_FAILED) {
        perror("mmap kvm_run");
        exit(1);
    }
}

static void setup_long_mode(struct vm_t *vm, struct kvm_sregs *sregs) {
    struct kvm_segment seg = {
            .base = 0,
            .limit = 0xffffffff,
            .present = 1,
            .dpl = 0,
            .db = 1,
            .s = 1, /* Code/data */
            .l = 0,
            .g = 1, /* 4KB granularity */
    };
    uint64_t *gdt;

    build_page_tables(vm->mem);

    //Page for bootstrap
    map_physical_pages(0x0000, 0x0000, PDE64_WRITEABLE | PDE64_USER, 1, 0);
    //Page for gdt
    int64_t gdt_page = map_physical_pages(0x1000, 0x1000, PDE64_WRITEABLE | PDE64_USER, 1, 0);
    printf("GDT page %" PRIx64 "\n", gdt_page);

    sregs->cr0 |= CR0_PE; /* enter protected mode */
    sregs->gdt.base = gdt_page;
    sregs->cr3 = pml4_addr;
    sregs->cr4 = CR4_PAE | CR4_OSFXSR | CR4_OSXMMEXCPT;
    sregs->cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM;
    sregs->efer = EFER_LME | EFER_NXE;

    gdt = (void *) (vm->mem + gdt_page);
    /* gdt[0] is the null segment */

    gdt[0] = 0xDEADBEEF;

    //32-bit code segment - needed for bootstrap
    sregs->gdt.limit = 3 * 8 - 1;

    seg.type = 11; /* Code: execute, read, accessed */
    seg.selector = 0x08;
    fill_segment_descriptor(gdt, &seg);
    sregs->cs = seg;

    //64-bit stuff
    sregs->gdt.limit = 18 * 8 - 1;
    seg.l = 1;

    //64-bit kernel code segment
    seg.type = 11; /* Code: execute, read, accessed */
    seg.selector = 0x10;
    seg.db = 0;
    fill_segment_descriptor(gdt, &seg);

    //64-bit kernel data segment
    seg.type = 3; /* Data: read/write, accessed */
    seg.selector = 0x18;
    fill_segment_descriptor(gdt, &seg);

    sregs->ds = sregs->es = sregs->fs = sregs->ss = seg;


    seg.dpl = 3;
    seg.db = 0;

    //64-bit user data segment
    seg.type = 3; /* Data: read/write, accessed */
    seg.selector = 0x28;
    fill_segment_descriptor(gdt, &seg);

    //64-bit user code segment
    seg.type = 11; /* Code: execute, read, accessed */
    seg.selector = 0x30;
    fill_segment_descriptor(gdt, &seg);

    //64-bit cpu kernel data segment
    /*seg.selector = 0x50;
    seg.type = 19;
    //one for each cpu in future!
    seg.limit = sizeof(cpu_t) - 1;
    seg.base = CPU_START-0x1000;
    seg.s = 0;
    seg.dpl = 0;
    seg.present = 1;
    seg.avl = 0;
    seg.l = 1;
    seg.db = 0;
    seg.g = 0;
    fill_segment_descriptor(gdt, &seg);*/

}

static uint64_t setup_kernel_tss(struct vm_t *vm, struct kvm_sregs *sregs, uint64_t kernel_min_addr){

    struct kvm_segment seg;
    uint64_t *gdt;
    uint64_t tss_start;

    gdt = (void *) (vm->mem + sregs->gdt.base);
    tss_start = kernel_min_addr - 3*PAGE_SIZE;

    //TSS segment

    seg.selector = 0x40;
    seg.limit = TSS_SIZE-1;
    seg.base = tss_start;
    seg.type = 9;
    seg.s = 0;
    seg.dpl = 3;
    seg.present = 1;
    seg.avl = 0;
    seg.l = 1;
    seg.db = 0;
    seg.g = 0;
    fill_segment_descriptor(gdt, &seg);

    //map 3 pages required for TSS

    size_t i = 0;
    for (uint64_t p = tss_start; i < 3; p += PAGE_SIZE, i++) {
        map_physical_pages(p, -1, PDE64_WRITEABLE, 1, 0);
    }

    return (tss_start);
}

void intHandler(int signal) {
    //Here to be interrupted
}

void run(struct vm_t *vm, struct vcpu_t *vcpu, int kernel_binary_fd, int argc, char *argv[], char *envp[])
{
    signal(SIGINT, intHandler);

    struct kvm_sregs sregs;
    struct kvm_regs regs;

    elf_info_t kernel_elf_info = { NULL, 0, 0, 0, 0, 0, 0 };

    uint64_t ksp_max;
    uint64_t ksp;
    uint64_t tss_start;

    size_t i;
    uint64_t p;
    size_t arg_size;
    int ret;

    char **envp_copy = envp;
    int envc = 0;
    char *kernel_argv[argc];
    uint64_t argv_start, envp_start;

    while(*envp_copy++)
        envc++;

    char *kernel_envp[envc + 1];
    kernel_envp[envc] = NULL;

    printf("Testing protected mode\n");

    if (ioctl(vcpu->fd, KVM_GET_SREGS, &sregs) < 0) {
        perror("KVM_GET_SREGS");
        exit(1);
    }

    setup_long_mode(vm, &sregs);

    load_elf_binary(kernel_binary_fd, NULL, &kernel_elf_info, true);
    printf("Entry point kernel: %p\n", kernel_elf_info.entry_addr);

    //allocate 20 stack pages for kernel stack
    ksp = kernel_elf_info.min_page_addr;
    for (p = ksp - PAGE_SIZE, i = 0; i < 20; p -= PAGE_SIZE, i++) {
        map_physical_pages(p, -1, PDE64_NO_EXE | PDE64_WRITEABLE | PDE64_USER, 1, 0);
    }

    //TODO - add protection for writing argv/envp off the end of the kernel's stack

    //copy user binary location to top of stack
    ksp_max = ksp;

    //Put argv and envp onto kernel stack

    //argv
    for (int i = argc - 1; i >= 0; i--)
    {
        arg_size = strlen(argv[i]) + 1;
        ksp -= arg_size;
        kernel_argv[i] = (char *)ksp;
        if (!write_virtual_addr(ksp, argv[i], arg_size))
        {
            perror("Error writing argv value.");
            exit(1);
        }

    }

    //envp
    for (int i = envc - 1; i >= 0; i--)
    {
        arg_size = strlen(envp[i]) + 1;
        ksp -= arg_size;
        kernel_envp[i] = (char *)ksp;
        if (!write_virtual_addr(ksp, envp[i], arg_size))
        {
            perror("Error writing envp value.");
            exit(1);
        }

    }

    ksp = P2ALIGN(ksp, MIN_ALIGN);

    //argv pointers
    ksp -= sizeof (kernel_argv);
    argv_start = ksp;
    write_virtual_addr(argv_start, (char *)kernel_argv, sizeof (kernel_argv));

    //envp pointers
    ksp -= sizeof (kernel_envp);
    envp_start = ksp;
    write_virtual_addr(envp_start, (char *)kernel_envp, sizeof (kernel_envp));

    tss_start = setup_kernel_tss(vm, &sregs, p);

    if (ioctl(vcpu->fd, KVM_SET_SREGS, &sregs) < 0) {
        perror("KVM_SET_SREGS");
        exit(1);
    }

    memset(&regs, 0, sizeof(regs));
    /* Clear all FLAGS bits, except bit 1 which is always set. */
    regs.rflags = 2;
    //bootstrap entry
    regs.rip = 0;
    //kernel entry
    regs.rdi = (uint64_t) kernel_elf_info.entry_addr;
    //kernel stack max
    regs.rsi = ksp_max;
    //kernel stack actual
    //tss start
    regs.rdx = tss_start;
    //argc
    regs.rcx = argc;
    //argv location
    regs.r8 = argv_start;
    //envp location
    regs.r9 = envp_start;

    if (ioctl(vcpu->fd, KVM_SET_REGS, &regs) < 0) {
        perror("KVM_SET_REGS");
        exit(1);
    }

    memcpy(vm->mem, bootstrap, bootstrap_end-bootstrap);
    printf("Loaded bootstrap: %" PRIu64 "\n", (uint64_t) (bootstrap_end-bootstrap));

    // Repeatedly run code and handle VM exits.
    while (1) {
        ret = ioctl(vcpu->fd, KVM_RUN, 0);
        if (ret == -1){
            perror("KVM_RUN");
            if (ioctl(vcpu->fd, KVM_GET_REGS, &regs) < 0) {
                perror("KVM_GET_REGS");
                exit(1);
            }
            printf("KVM_RUN exited with error: %d, PC: %p, Physical memory: %p, Paged memory %p, Exit reason %u\n",
                   ret, (void *)regs.rip,
                   (void *)phy_mem_size,
                   (void *)(page_counter*PAGE_SIZE),
                   vcpu->kvm_run->exit_reason);
            exit(1);
        }
        switch (vcpu->kvm_run->exit_reason) {
            case KVM_EXIT_IO:
                if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT && vcpu->kvm_run->io.size == 1 &&
                    vcpu->kvm_run->io.port == 0x3f8 &&
                    vcpu->kvm_run->io.port == 0x3f8 &&
                    vcpu->kvm_run->io.port == 0x3f8 &&
                    vcpu->kvm_run->io.port == 0x3f8 &&
                    vcpu->kvm_run->io.count == 1)
                    putchar(*(((char *) vcpu->kvm_run) + vcpu->kvm_run->io.data_offset));
                else
                    errx(1, "unhandled KVM_EXIT_IO");
                break;
            case KVM_EXIT_FAIL_ENTRY:
                errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
                     (unsigned long long) vcpu->kvm_run->fail_entry.hardware_entry_failure_reason);
                break;
            case KVM_EXIT_INTERNAL_ERROR:
                errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x", vcpu->kvm_run->internal.suberror);
            case KVM_EXIT_HLT:

                if (ioctl(vcpu->fd, KVM_GET_REGS, &regs) < 0) {
                    perror("KVM_GET_REGS");
                    exit(1);
                }

                //check for interupt

                if (handle_host_call(&regs)) {
                    return;
                }

                if (ioctl(vcpu->fd, KVM_SET_REGS, &regs) < 0) {
                    perror("KVM_SET_REGS");
                    exit(1);
                }

                break;
            case KVM_EXIT_MMIO:
            case KVM_EXIT_SHUTDOWN:

                if (ioctl(vcpu->fd, KVM_GET_REGS, &regs) < 0) {
                    perror("KVM_GET_REGS");
                    exit(1);
                }

                printf("MMIO Error: code = %d, PC: %p\n", vcpu->kvm_run->exit_reason, (void *) regs.rip);
                return;
            default:
                printf("Other exit: code = %d\n", vcpu->kvm_run->exit_reason);
                return;
        }
        continue;
    }
}
