//
// Created by ben on 14/10/16.
//

#ifndef KERNEL_HOST_H
#define KERNEL_HOST_H

#include <stdlib.h>

#include "../../common/syscall.h"
#include "../../common/vma.h"

//app

ssize_t app_write(uint32_t fd, const char *buf, size_t count);

ssize_t app_read(uint32_t fd, const char *buf, size_t count);

int app_open(const char *filename, int32_t flags, uint16_t mode);

int app_openat(int dfd, const char *filename, int32_t flags, uint16_t mode);

int app_close(uint32_t fd);

int app_dup(uint32_t fd);

int app_stat(const char *path, vm_stat_t *stats);

int app_fstat(uint32_t fd, vm_stat_t *stats);

int app_access(const char *pathname, int mode);

int app_uname(utsname_t *buf);

void *app_mmap(void *addr, size_t length, uint64_t prot, uint64_t flags, int fd, uint64_t offset);

int64_t app_lseek(int fd, int64_t offset, int whence);

ssize_t app_writev(int fd, const vm_iovec_t *vec, int vlen, int flags);

//kernel

ssize_t kernel_write(uint32_t fd, const char *buf, size_t count);

ssize_t kernel_read(uint32_t fd, const char *buf, size_t count);

int kernel_open(const char *filename, int32_t flags, uint16_t mode);

int kernel_stat(const char *path, vm_stat_t *stats);

int kernel_fstat(uint32_t fd, vm_stat_t *stats);

#define kernel_close app_close
#define kernel_dup app_dup
#define kernel_lseek app_lseek

void kernel_exit();

int kernel_set_vma_heap(uint64_t vma_heap_addr, vm_area_t **vma_list_start, rb_root_t *vma_rb_root);

void kernel_map_vma(vm_area_t *vma);

void kernel_unmap_vma(vm_area_t *vma);

#endif //KERNEL_HOST_H
