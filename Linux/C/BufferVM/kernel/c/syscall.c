//
// Created by ben on 17/10/16.
//

#include "../../libc/stdlib.h"
#include "../h/kernel.h"
#include "../h/syscall.h"
#include "../h/host.h"
#include "../../common/paging.h"
#include "../../common/version.h"
#include "../h/vma_heap.h"
#include "../../common/utils.h"
#include "../h/vma.h"

uint64_t curr_brk;

void syscall_read(uint32_t fd, const char *buf, size_t count) {
    app_read(fd, buf, count);
}

void syscall_write(uint32_t fd, const char *buf, size_t count) {
    app_write(fd, buf, count);
}

int syscall_open(const char *filename, int32_t flags, uint16_t mode) {
    printf("Opening file: %s\n", filename);
    int fd = app_open(filename, flags, mode);
    printf("open fd: %d\n", fd);

    return fd;
}

int syscall_openat(int dfd, const char *filename, int32_t flags, uint16_t mode) {
    printf("Opening file: %s at fd: %d\n", filename, dfd);
    int fd = app_openat(dfd, filename, flags, mode);
    printf("open fd: %d\n", fd);

    return fd;
}

int syscall_close(int32_t fd) {
    printf("closing fd: %d\n", fd);
    return app_close(fd);
}

int syscall_ioctl(uint32_t fd, uint64_t request, void *argp) {
    printf("syscall_ioctl fd: %d\n", fd);
    return 0;
    //return app_fstat(fd, stats);
}

int syscall_stat(const char *path, vm_stat_t *stats) {
    printf("syscall_stat path: %s\n", path);
    return app_stat(path, stats);
}

int syscall_fstat(uint32_t fd, vm_stat_t *stats) {
    printf("syscall_fstat fd: %d\n", fd);
    return app_fstat(fd, stats);
}

int syscall_access(const char *pathname, int mode) {
    return app_access(pathname, mode);
}

ssize_t syscall_writev(uint64_t fd, const vm_iovec_t *vec, uint64_t vlen, int flags) {
    return app_writev(fd, vec, vlen, flags);
}

void syscall_exit_group(int status) {
    printf("Exit group status: %d\n", status);
    kernel_exit();
}

void syscall_exit() {
    kernel_exit();
}

uint64_t syscall_arch_prctl(int code, uint64_t addr) {
    uint64_t ret = 0;

    printf("Setup thread local storage at %p?\n", addr);

    //TODO - make this safe with interrupt handler etc
    switch (code) {
        case ARCH_SET_GS:
            ret = write_msr(MSR_KERNEL_GS_BASE, addr);
            break;
        case ARCH_SET_FS:
            write_msr(MSR_FS_BASE, addr);
            break;
        case ARCH_GET_GS:
            ret = read_msr(MSR_KERNEL_GS_BASE);
            break;
        case ARCH_GET_FS:
            ret = read_msr(MSR_FS_BASE);
            break;
        default:
            ret = -EINVAL;
    }

    return ret;
}

vm_area_t *user_heap_vma;

uint64_t syscall_brk(uint64_t brk) {
    uint64_t new_brk;
    size_t num_pages;
    uint64_t addr;

    if (brk == 0 && curr_brk == 0) {

        curr_brk = user_heap_start;

        addr = mmap_region(NULL, curr_brk, PAGE_SIZE, VMA_GROWS, VMA_WRITE | VMA_READ | VMA_IS_VERSIONED, 0,
                           &user_heap_vma);

        ASSERT(user_heap_vma != NULL && addr == curr_brk);

        printf("Starting heap at: %p\n", user_heap_start);
    } else {
        if (brk < curr_brk) {
            //TODO - shrinking brk
            return curr_brk;
        }

        new_brk = PAGE_ALIGN(brk);

        if (new_brk <= user_heap_vma->start_addr || (user_heap_vma->next && new_brk > user_heap_vma->next->start_addr))
            return curr_brk;

        user_heap_vma->end_addr = new_brk;
        vma_gap_update(user_heap_vma);

        //unmap_vma(user_heap_vma);
        kernel_map_vma(user_heap_vma);

        printf("New brk at: %p\n", new_brk);

        curr_brk = new_brk;
    }

    return curr_brk;
}

int syscall_uname(utsname_t *buf){
    return app_uname(buf);
}

/* Syscall table and parameter info */

void *syscall_table[SYSCALL_MAX] = {
        [0 ... SYSCALL_MAX - 1] = &syscall_null_handler,
        [0] = &syscall_read,
        [1] = &syscall_write,
        [2] = &syscall_open,
        [3] = &syscall_close,
        [4] = &syscall_stat,
        [5] = &syscall_fstat,
        [9] = &syscall_mmap,
        [10] = &syscall_mprotect,
        [11] = &syscall_munmap,
        [12] = &syscall_brk,
        [16] = &syscall_ioctl,
        [20] = &syscall_writev,
        [21] = &syscall_access,
        [60] = &syscall_exit,
        [63] = &syscall_uname,
        [158] = &syscall_arch_prctl,
        [231] = &syscall_exit_group,
        [257] = &syscall_openat,
        [SYSCALL_MAX - 1] = &get_version,
        [SYSCALL_MAX - 2] = &set_version
};

void syscall_init() {
    syscall_setup();

    curr_brk = 0;
}