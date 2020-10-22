//
// Created by ben on 18/10/16.
//

#include "../../common/syscall.h"

#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

uint64_t syscall_mmap(uint64_t addr, size_t length, uint64_t prot, uint64_t flags, int fd, uint64_t offset);
int syscall_munmap(uint64_t addr, size_t length);
int syscall_mprotect(void *addr, size_t len, int prot);
ssize_t syscall_writev(uint64_t fd, const vm_iovec_t *vec, uint64_t vlen, int flags);

uint64_t do_brk(uint64_t addr, uint64_t request, uint64_t flags);

void syscall_setup();
void syscall_init();

int syscall_null_handler();

#endif //KERNEL_SYSCALL_H
