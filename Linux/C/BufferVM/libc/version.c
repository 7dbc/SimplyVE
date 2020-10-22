//
// Created by ben on 30/12/17.
//

#include "version.h"


#define VER_NUM_BITS 8
#define VER_NUM_BITMASK ((1 << VER_NUM_BITS) - 1)

inline void *set_version_ptr(uint64_t version, void *pointer) {
    return (void *) ((uint64_t) normalise_version_ptr(pointer) | ((version & 0xFF) << (48 - VER_NUM_BITS)));
}

inline uint64_t get_version_ptr(uint64_t *pointer) {
    return (uint64_t) pointer >> (48 - VER_NUM_BITS);
}

inline void *normalise_version_ptr(void *addr) {
    return (void *) ((uint64_t) addr & ~((uint64_t) VER_NUM_BITMASK << (48 - VER_NUM_BITS)));
}
