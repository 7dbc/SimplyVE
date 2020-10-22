#include "../common/syscall_as.h"

.text
.globl get_version
.globl set_version

get_version:
    movq    $(SYSCALL_MAX-1), %rax
    syscall                 # call kernel.

	retq

set_version:
    movq    $(SYSCALL_MAX-2), %rax
    syscall                 # call kernel.

	retq