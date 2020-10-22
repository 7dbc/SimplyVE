//
// Created by ben on 31/12/17.
//

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <linux/kvm.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <asm/errno.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include "../common/vma.h"
#include "../common/paging.h"
#include "vm.h"
#include "vma.h"

static bool exit_vm = false;

static inline void *vm_ptr(uint64_t addr) {
    vm_area_t *vma = vma_find(addr);

    if (vma == NULL) {
        printf("VA %p is not mapped\n", (void *)addr);
        exit(1);
    }

    return (vm.mem + vma->phys_page_start + (addr - vma->start_addr));
}

void kernel_exit() {
#ifndef VM
    __builtin_unreachable();
#endif
    printf("Exit hostcall\n");

    vma_print();

    exit_vm = true;

    return;
}

ssize_t kernel_write(uint32_t fd, const char *buf, size_t count) {
    char buffer[count];
    if (read_virtual_addr((uint64_t) buf, count, buffer)) {
        return (write(fd, buffer, count));
    } else {
        printf("Failed to write - Un-paged buffer?\n");

        return (-1);
    }
}

ssize_t kernel_read(uint32_t fd, const char *buf, size_t count) {
    char buffer[count];
    ssize_t ret = read(fd, buffer, count);
    if (!write_virtual_addr((uint64_t) buf, buffer, count))
        ret = -1;

    return (ret);
}

int kernel_open(const char *filename, int32_t flags, uint16_t mode) {
    char *buffer;
    int ret;

    if (read_virtual_cstr((uint64_t) filename, &buffer)) {
        ret = open(buffer, flags, mode);
        free(buffer);

        return (ret);
    } else {
        printf("Failed to write - Un-paged buffer?\n");

        return (-1);
    }
}

int kernel_stat(const char *path, vm_stat_t *stats) {
    struct stat s_stats;
    int ret;
    char *file;
    if (read_virtual_cstr((uint64_t)path, &file)) {
        ret = stat(file, &s_stats);
        free(file);
        if (!write_virtual_addr((uint64_t)stats, (char *) &s_stats, sizeof(s_stats)))
            ret = -1;
    } else
        ret = -1;

    return (ret);
}

int kernel_fstat(uint32_t fd, vm_stat_t *stats) {
    int ret;
    struct stat s_stats;
    ret = fstat(fd, &s_stats);
    if (!write_virtual_addr((uint64_t) stats, (char *) &s_stats, sizeof(s_stats)))
        ret = -1;

    return (ret);
}

ssize_t host_writev(int fd, const vm_iovec_t *vec, int vlen, int flags) {
    struct iovec vectors[vlen];
    struct iovec *s_vec;
    void *vecbuf;
    size_t ret = 0;

    if (vlen < 0){
        ret = -EINVAL;
    } else if (read_virtual_addr((uint64_t)vec, vlen * sizeof(struct iovec), vectors)) {
        for (int i = 0; i < vlen; i++) {
            s_vec = &vectors[i];

            vecbuf = malloc(s_vec->iov_len);
            if (!read_virtual_addr((uint64_t) s_vec->iov_base, s_vec->iov_len, vecbuf)) {
                ret = -EFAULT;
                break;
            }
            s_vec->iov_base = vecbuf;
        }
        if (!IS_ERR_VALUE(ret)) {
            ret = writev(fd, vectors, vlen);
            for (int i = 0; i < vlen; i++) {
                s_vec = &vectors[i];
                free(s_vec->iov_base);
            }
        }

    } else
        ret = -EFAULT;
    return (ret);
}

typedef struct arg_entry {
    bool a;
    bool b;
    bool c;
    bool d;
    bool e;
    bool f;
    bool ret;
} arg_entry_t;

#define ARG_ENTRY(arg1, arg2, arg3, arg4, arg5, arg6, ret)   \
    {arg1, arg2, arg3, arg4, arg5, arg6, ret},

#define ARG_ENTRY_BLANK()   ARG_ENTRY(0,0,0,0,0,0,0)

#define APPCALL_DEF(NAME)   ARG_ENTRY_BLANK()

#define APPCALL_DEF2(NAME, arg1, arg2, arg3, arg4, arg5, arg6, ret)  ARG_ENTRY(arg1, arg2, arg3, arg4, arg5, arg6, ret)

#define APPCALL_DEF3(NAME)   ARG_ENTRY_BLANK()

#define KERNELCALL_DEF(NAME)  ARG_ENTRY_BLANK()

#define KERNELCALL_DEF2(NAME)  ARG_ENTRY_BLANK()

#define KERNELCALL_DEF_CUSTOM(NAME)  ARG_ENTRY_BLANK()

static const arg_entry_t hostcall_arg_table[] = {
#include "../common/host.h"
};

typedef uint64_t (*hostcall_func_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

#undef COMMON_HOST_H
#undef APPCALL_DEF
#undef APPCALL_DEF2
#undef APPCALL_DEF3
#undef KERNELCALL_DEF
#undef KERNELCALL_DEF2
#undef KERNELCALL_DEF_CUSTOM

#define APPCALL_DEF(NAME)  \
    (hostcall_func_t)&NAME,

#define APPCALL_DEF2(NAME, arg1, arg2, arg3, arg4, arg5, arg6, ret)  \
    (hostcall_func_t)&NAME,

#define APPCALL_DEF3(NAME)  \
    (hostcall_func_t)&host_ ## NAME,

#define KERNELCALL_DEF(NAME)  \
    (hostcall_func_t)&kernel_ ## NAME,

#define KERNELCALL_DEF2(NAME)  \
    (hostcall_func_t)&NAME,

#define KERNELCALL_DEF_CUSTOM(NAME)  \
    (hostcall_func_t)&NAME,

static const hostcall_func_t hostcall_table[] = {
#include "../common/host.h"
};

#define CALL_ARG(key, val)  (arg_entry->key ? (uint64_t)vm_ptr((uint64_t)val) : val)

int handle_host_call(struct kvm_regs *regs) {
    uint64_t call_no = regs->rax;

    if (call_no < sizeof(hostcall_table) / sizeof(hostcall_func_t)) {
        //printf("Supported hostcall %" PRIu64 "\n", call_no);
        const arg_entry_t *arg_entry = &hostcall_arg_table[call_no];

        regs->rax = (uint64_t) hostcall_table[call_no](
                CALL_ARG(a, regs->rdi),
                CALL_ARG(b, regs->rsi),
                CALL_ARG(c, regs->rdx),
                CALL_ARG(d, regs->rcx),
                CALL_ARG(e, regs->r8),
                CALL_ARG(f, regs->r9));

        if (arg_entry->ret){
            printf("mmap boo\n");
            exit(1);
            vm_area_t * vma = vma_find_phys(regs->rax);
            regs->rax = regs->rax - (uint64_t)vm.mem - vma->phys_page_start + vma->start_addr;

        }

        if (exit_vm)
            return (1);
    } else {
        printf("Unsupported hostcall %" PRIu64 "\n", call_no);
        return (1);
    }

    return (0);
}

