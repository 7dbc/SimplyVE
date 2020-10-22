//
// Created by ben on 18/10/16.
//

#include "kernel_as.h"
#include "../../common/elf.h"

#ifndef PROJECT_KERNEL_H
#define PROJECT_KERNEL_H

#define AT_NULL   0	/* end of vector */
#define AT_IGNORE 1	/* entry should be ignored */
#define AT_EXECFD 2	/* file descriptor of program */
#define AT_PHDR   3	/* program headers for program */
#define AT_PHENT  4	/* size of program header entry */
#define AT_PHNUM  5	/* number of program headers */
#define AT_PAGESZ 6	/* system page size */
#define AT_BASE   7	/* base address of interpreter */
#define AT_FLAGS  8	/* flags */
#define AT_ENTRY  9	/* entry point of program */
#define AT_NOTELF 10	/* program is not ELF */
#define AT_UID    11	/* real uid */
#define AT_EUID   12	/* effective uid */
#define AT_GID    13	/* real gid */
#define AT_EGID   14	/* effective gid */
#define AT_PLATFORM 15  /* string identifying CPU for optimizations */
#define AT_HWCAP  16    /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK 17	/* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23   /* secure mode boolean */
#define AT_BASE_PLATFORM 24	/* string identifying real platform, may
				 * differ from AT_PLATFORM. */
#define AT_RANDOM 25	/* address of 16 random bytes */
#define AT_HWCAP2 26	/* extension of AT_HWCAP */

#define AT_EXECFN  31	/* filename of program */

extern uint64_t kernel_min_address;

extern uint64_t user_heap_start;
extern uint64_t user_version_start;
extern uint64_t user_version_end;

void load_user_land(uint64_t esp, void *elf_entry, elf_info_t *user_elf_info, int argc, char *argv[], char *envp[]);
void switch_user_land(void *entry, uint64_t stack_entry);

static inline uint64_t read_msr(uint32_t msr)
{
    uint64_t low, high;

    asm volatile("rdmsr":"=a"(low),"=d"(high):"c"(msr));

    return ((low) | (high) << 32);
}
static inline uint64_t write_msr(uint32_t msr, uint64_t value)
{

    uint64_t low, high;

    high = value >> 32;
    low = value & 0xFFFFFFFF;

    asm volatile ("wrmsr" \
			  : /* no outputs */ \
			  : "c" (msr), "a" (low), "d" (high));

    return value;
}

#endif //PROJECT_KERNEL_H
