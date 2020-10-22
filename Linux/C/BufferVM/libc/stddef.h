//
// Created by ben on 31/12/17.
//

#ifndef LIBC_STDDEF_H
#define LIBC_STDDEF_H

#include "stdint.h"

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

typedef uint64_t size_t;
typedef int64_t ssize_t;

#endif //LIBC_STDDEF_H
