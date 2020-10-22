//
// Created by ben on 28/12/16.
//

#ifndef PROJECT_UTILS_H
#define PROJECT_UTILS_H

#include "stdlib.h"
#include <stdio.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type, member) );})

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

#define ASSERT_FAIL() do { \
        printf("ASSERT: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
        kernel_exit(); \
    } while (0)

#define ASSERT(condition) VERIFY(condition)

#define VERIFY(condition) do { if (!(condition)) ASSERT_FAIL(); } while (0)

#define TRANSFER_FLAG(x, bit1, bit2) (x & (uint64_t)bit1 ? (uint64_t)bit2 : 0)
#define TRANSFER_INV_FLAG(x, bit1, bit2) (!(x & (uint64_t)bit1) ? (uint64_t)bit2 : 0)

void disassemble_address(uint64_t addr, size_t num_inst);

#endif //PROJECT_UTILS_H
