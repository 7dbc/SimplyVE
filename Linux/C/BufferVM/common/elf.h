//
// Created by ben on 03/01/17.
//

#include "stdlib_inc.h"

#ifndef COMMON_ELF_H
#define COMMON_ELF_H

/* Legal values for p_type (segment type).  */

#define	PT_NULL		0		/* Program header table entry unused */
#define PT_LOAD		1		/* Loadable program segment */
#define PT_DYNAMIC	2		/* Dynamic linking information */
#define PT_INTERP	3		/* Program interpreter */
#define PT_NOTE		4		/* Auxiliary information */
#define PT_SHLIB	5		/* Reserved */
#define PT_PHDR		6		/* Entry for header table itself */
#define PT_TLS		7		/* Thread-local storage segment */
#define	PT_NUM		8		/* Number of defined types */

/* Legal values for p_flags (segment flags).  */

#ifdef KERNEL

#define PF_X		(1 << 0)	/* Segment is executable */
#define PF_W		(1 << 1)	/* Segment is writable */
#define PF_R		(1 << 2)	/* Segment is readable */

#define EI_NIDENT       16

#define ET_EXEC 2
#define ET_DYN  3

#define EM_386          3
#define EM_X86_64       62

typedef struct elf32_hdr{
    unsigned char e_ident[EI_NIDENT];
    uint16_t    e_type;
    uint16_t    e_machine;
    uint32_t    e_version;
    uint32_t    e_entry;
    uint32_t    e_phoff;
    uint32_t    e_shoff;
    uint32_t    e_flags;
    uint16_t    e_ehsize;
    uint16_t    e_phentsize;
    uint16_t    e_phnum;
    uint16_t    e_shentsize;
    uint16_t    e_shnum;
    uint16_t    e_shstrndx;
} elf32_hdr_t;

typedef struct elf64_hdr {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf64_hdr_t;

typedef struct elf64_phdr
{
    uint32_t	p_type;			/* Segment type */
    uint32_t	p_flags;		/* Segment flags */
    uint64_t	p_offset;		/* Segment file offset */
    uint64_t	p_vaddr;		/* Segment virtual address */
    uint64_t	p_paddr;		/* Segment physical address */
    uint64_t	p_filesz;		/* Segment size in file */
    uint64_t	p_memsz;		/* Segment size in memory */
    uint64_t	p_align;		/* Segment alignment */
} elf64_phdr_t;

#endif

typedef struct elf_info {
    void *entry_addr;
    uint64_t base_addr;
    uint64_t load_addr;
    uint64_t min_page_addr;
    uint64_t max_page_addr;
    uint64_t phdr_off;
    uint64_t phdr_num;
} elf_info_t;

int load_elf_binary(int fd, void **elf_entry, elf_info_t *elf_info, bool load_only);
int read_binary(char *name);

#endif //COMMON_ELF_H
