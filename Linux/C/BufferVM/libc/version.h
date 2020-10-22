//
// Created by ben on 30/12/17.
//

#ifndef LIBC_VERSION_H
#define LIBC_VERSION_H

#include <stdint.h>

void *set_version_ptr(uint64_t version, void *pointer);
uint64_t get_version_ptr(uint64_t *pointer);
void *normalise_version_ptr(void *addr);

#endif //LIBC_VERSION_H
