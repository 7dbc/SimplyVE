#ifndef SBRK_D
#define SBRK_D

#include "../libc/stdlib.h"

int brk (void *addr);
void *sbrk (intptr_t increment);

#endif
