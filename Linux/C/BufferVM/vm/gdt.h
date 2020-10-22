//
// Created by ben on 15/10/16.
//

#ifndef PROJECT_GDT_H
#define PROJECT_GDT_H

#include <stdint.h>
#include <linux/kvm.h>

void fill_segment_descriptor(uint64_t *dt, struct kvm_segment *seg);

#endif //PROJECT_GDT_H
