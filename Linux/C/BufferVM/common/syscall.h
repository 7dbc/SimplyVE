//
// Created by ben on 28/12/16.
//

#ifndef COMMON_SYSCALL_H
#define COMMON_SYSCALL_H

#include <stddef.h>

#include "syscall_as.h"

#define MAX_ERRNO	4095
#define IS_ERR_VALUE(x) ((x) >= (unsigned long)-MAX_ERRNO)

typedef struct vm_file {
    int fd;
    uint64_t dev;
    uint64_t inode;
} vm_file_t;

typedef struct vm_iovec
{
    void *iov_base;	/* BSD uses caddr_t (1003.1g requires void *) */
    size_t iov_len; /* Must be size_t (1003.1g) */
} vm_iovec_t;

typedef struct vm_timespec
{
    int64_t tv_sec;		/* Seconds.  */
    int64_t tv_nsec;	/* Nanoseconds.  */
} vm_timespec_t;

typedef struct vm_stat {
    uint64_t  st_dev;     /* ID of device containing file */
    uint64_t  st_ino;     /* inode number */
    uint64_t  st_nlink;   /* number of hard links */
    uint32_t   st_mode;    /* protection */
    uint32_t     st_uid;     /* user ID of owner */
    uint32_t     st_gid;     /* group ID of owner */
    unsigned int		__pad0;
    uint64_t     st_rdev;    /* device ID (if special file) */
    int64_t    st_size;    /* total size, in bytes */
    int64_t st_blksize; /* blocksize for file system I/O */
    int64_t  st_blocks;  /* number of 512B blocks allocated */
    vm_timespec_t    st_vm_atime;   /* time of last access */
    vm_timespec_t    st_vm_mtime;   /* time of last modification */
    vm_timespec_t    st_vm_ctime;   /* time of last status change */
} vm_stat_t;

#define UTSNAME_LENGTH 65

typedef struct vm_utsname
{
    /* Name of the implementation of the operating system.  */
    char sysname[UTSNAME_LENGTH];

    /* Name of this node on the network.  */
    char nodename[UTSNAME_LENGTH];

    /* Current release level of this implementation.  */
    char release[UTSNAME_LENGTH];
    /* Current version level of this release.  */
    char version[UTSNAME_LENGTH];

    /* Name of the hardware type the system is running on.  */
    char machine[UTSNAME_LENGTH];

    char domainname[UTSNAME_LENGTH];
} utsname_t;

//arch_prctl

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

//mmap

#define PROT_READ       0x1             /* page can be read */
#define PROT_WRITE      0x2             /* page can be written */
#define PROT_EXEC       0x4             /* page can be executed */
#define PROT_SEM        0x8             /* page may be used for atomic ops */
#define PROT_NONE       0x0             /* page can not be accessed */
#define PROT_GROWSDOWN  0x01000000      /* mprotect flag: extend change to start of growsdown vma */
#define PROT_GROWSUP    0x02000000      /* mprotect flag: extend change to end of growsup vma */

#ifndef VM

#define MAP_SHARED      0x01            /* Share changes */
#define MAP_PRIVATE     0x02            /* Changes are private */
#define MAP_TYPE        0x0f            /* Mask for type of mapping */
#define MAP_FIXED       0x10            /* Interpret addr exactly */
#define MAP_ANONYMOUS   0x20            /* don't use a file */

#define MAP_GROWSDOWN   0x0100          /* stack-like segment */

#define MAP_POPULATE	0x8000		/* populate (prefault) pagetables */
#define MAP_NONBLOCK	0x10000		/* do not block on IO */

#define MAP_FAILED ((void*)-1)

//file

#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02

#define SEEK_SET        0       /* seek relative to beginning of file */
#define SEEK_CUR        1       /* seek relative to current file position */
#define SEEK_END        2       /* seek relative to end of file */
#define SEEK_DATA       3       /* seek to the next data */
#define SEEK_HOLE       4       /* seek to the next hole */
//errno

#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define E2BIG            7      /* Argument list too long */
#define ENOEXEC          8      /* Exec format error */
#define EBADF            9      /* Bad file number */
#define ECHILD          10      /* No child processes */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define ENOTBLK         15      /* Block device required */
#define EBUSY           16      /* Device or resource busy */
#define EEXIST          17      /* File exists */
#define EXDEV           18      /* Cross-device link */
#define ENODEV          19      /* No such device */
#define ENOTDIR         20      /* Not a directory */
#define EISDIR          21      /* Is a directory */
#define EINVAL          22      /* Invalid argument */
#define ENFILE          23      /* File table overflow */
#define EMFILE          24      /* Too many open files */
#define ENOTTY          25      /* Not a typewriter */
#define ETXTBSY         26      /* Text file busy */
#define EFBIG           27      /* File too large */
#define ENOSPC          28      /* No space left on device */
#define ESPIPE          29      /* Illegal seek */
#define EROFS           30      /* Read-only file system */
#define EMLINK          31      /* Too many links */
#define EPIPE           32      /* Broken pipe */
#define EDOM            33      /* Math argument out of domain of func */
#define ERANGE          34      /* Math result not representable */

#endif

#endif //COMMON_SYSCALL_H
