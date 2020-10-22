//
// Created by ben on 17/10/16.
//

#ifndef PROJECT_TSS_H
#define PROJECT_TSS_H

#include "../../common/tss.h"

tss_entry_t* tss;

extern void tss_load(void);
extern void gdt_load(uint64_t gdt);

void tss_init(uint64_t kernel_stack);

#endif //PROJECT_TSS_H
