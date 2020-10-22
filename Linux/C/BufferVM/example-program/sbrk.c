#include "sbrk.h"

void *__curbrk = NULL;

/*int brk(void *addr) {
    void *newbrk = _brk(addr);

    __curbrk = newbrk;

    if (newbrk < addr) {
        return -1;
    }

    return 0;
}*/

void *sbrk(intptr_t increment) {
    char *end;
    char *new_brk;
    char *old_brk;

    if (__curbrk == NULL){
        __curbrk = _brk(0);
    }

    if (increment == 0)
        return __curbrk;

    old_brk = __curbrk;
    end = __curbrk + increment;

    new_brk = (char *) _brk(end);

    if (new_brk == (void *)-1)
        return new_brk;
    else if (new_brk < end)
    {
        return (void *)-1;
    }

    __curbrk = new_brk;
    return old_brk;
}
