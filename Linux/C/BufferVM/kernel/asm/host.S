.text

#define HOSTCALL_DEF(PREFIX, NAME)  \
.globl PREFIX ## _ ## NAME;  \
PREFIX ## _ ## NAME:   \
    movq $(__COUNTER__), %rax;   \
    hlt; \
    ret;

#define APPCALL_DEF(NAME)    HOSTCALL_DEF(app, NAME)

#define APPCALL_DEF2(NAME, arg1, arg2, arg3, arg4, arg5, arg6, ret)    APPCALL_DEF(NAME)

#define APPCALL_DEF3(NAME)    APPCALL_DEF(NAME)

#define KERNELCALL_DEF(NAME)    HOSTCALL_DEF(kernel, NAME)

#define KERNELCALL_DEF2(NAME)  \
.globl NAME;  \
NAME:   \
    movq $(__COUNTER__), %rax;   \
    hlt; \
    ret;

#define KERNELCALL_DEF_CUSTOM(NAME)

#include "../../common/host.h"

.globl unmap_physical_page
unmap_physical_page:
    pushq %rdi
    movq $(__COUNTER__), %rax
    hlt
    popq %rdi
    invlpg	(%rdi)
    ret
