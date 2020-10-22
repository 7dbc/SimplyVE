#include "../common/syscall_as.h"

.text
.globl read
.globl write
.globl open
.globl close
.globl mmap
.globl munmap
.globl _brk
.globl get_version
.globl set_version
.type read, @function
.type write, @function
.type open, @function
.type close, @function
.type munmap, @function
.type mmap, @function
.type _brk, @function
.type get_version, @function
.type set_version, @function


read:
    # third argument: message length.
    # second argument: pointer to message to write.
    # first argument: file handle.
    movq    $0,%rax	        # system call number (read).
    syscall                 # call kernel.

	# return already in rax
	retq

write:
    # third argument: message length.
    # second argument: pointer to message to write.
    # first argument: file handle.
    movq    $1,%rax	        # system call number (sys_write).
    syscall                 # call kernel.

	# return already in rax
	retq

open:
    # third argument: mode
    # second argument: flags
    # first argument: file name.
    movq    $2,%rax	        # system call number (sys_write).
    syscall                 # call kernel.

    # return already in rax
    retq

close:
    # first argument: file handle.
    movq    $3,%rax	        # system call number (sys_write).
    syscall                 # call kernel.

    # return already in rax
    retq

mmap:
    # first argument: file handle.
    movq    $9,%rax	        # system call number (sys_mmap).
    movq %rcx, %r10         # swap to match syscall abi
    syscall                 # call kernel.

    # return already in rax
    retq

munmap:
    # first argument: file handle.
    movq    $11,%rax	        # system call number (sys_mmap).
    syscall                 # call kernel.

    # return already in rax
    retq

_brk:
	push    %rbp            # create stack frame
    movq    %rsp, %rbp

    # first argument: file handle (stdout).
    movq    $12,%rax	    # system call number (sys_brk).
    syscall                 # call kernel.*/

	pop     %rbp            # restore the base pointer
	retq

get_version:
    movq    $(SYSCALL_MAX), %rax
    syscall                 # call kernel.

	retq

set_version:
    movq    $(SYSCALL_MAX-1), %rax
    syscall                 # call kernel.

	retq