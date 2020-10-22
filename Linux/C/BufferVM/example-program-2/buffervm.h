//
// Created by ben on 22/01/17.
//

#include <stdint.h>

#ifndef EX2_BUFFERVM_H
#define EX2_BUFFERVM_H

void *set_version(uint64_t *pointer, size_t length, uint64_t version);
uint8_t get_version(uint64_t *addr) ;

#endif //EX2_BUFFERVM_H
