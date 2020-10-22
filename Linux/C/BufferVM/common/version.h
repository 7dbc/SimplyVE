//
// Created by ben on 07/12/16.
//

#ifndef PROJECT_VERSION_H
#define PROJECT_VERSION_H

#include "../libc/stdlib.h"

void *set_version(uint64_t *pointer, size_t length, uint64_t version);
uint8_t get_version(uint64_t *addr) ;

#endif //PROJECT_VERSION_H
