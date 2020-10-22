//
// Created by ben on 21/12/16.
//

#ifndef COMMON_TSS_H
#define COMMON_TSS_H

#include "stdlib_inc.h"

#define TSS_KVM 0xfffbd000
#define TSS_START 0x7FFFFFD000
#define TSS_SIZE P2ROUNDUP(sizeof (tss_entry_t), PAGE_SIZE)

typedef struct tss_entry {
    uint32_t prev_tss;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserve2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserve3;
    uint16_t reserve4;
    uint16_t iomap_base;
} __attribute__ ((packed))
        tss_entry_t;

#endif //COMMON_TSS_H
