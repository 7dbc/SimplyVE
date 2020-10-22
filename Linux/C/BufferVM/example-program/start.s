.text
.globl _start

_start:
	call main

	# exit
	movq $60, %rax
	xorq %rdi, %rdi
	syscall

.section .data
LC0:
    .ascii "heap pos: %p %p\n\0"
        #          2^-1  2^-2  2^-3   2^-4    2^-5     2^-6      2^-7       2^-8
        v1: .float 0.50, 0.25, 0.125, 0.0625, 0.03125, 0.015625, 0.0078125, 0.00390625
        v2: .float 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0
        v3: .float 512.0, 1024.0, 2048.0, 8192.0, 16384.0, 32768.0, 65536.0, 131072.0
        v4: .float 0,0,0,0,0,0,0,0
