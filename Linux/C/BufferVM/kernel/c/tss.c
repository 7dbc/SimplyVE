//
// Created by ben on 17/10/16.
//

#include <string.h>

#include "../h/tss.h"
#include "../h/host.h"

void tss_init(uint64_t kernel_stack){
    tss_load();
    memset((void *) tss, 0, sizeof(tss_entry_t));
    tss->rsp0 = kernel_stack;
}
