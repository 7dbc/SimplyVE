        .global bootstrap, bootstrap_end
        .code32
bootstrap:

        // Set cr0.pg
        movl %cr0, %eax
        orl $0x80000000, %eax
        movl %eax, %cr0

        // We are now in ia32e compatibility mode. Switch to 64-bit
	    // code segment
        ljmp $0x10, $1f
        .code64
1:
        jmp *%rdi
bootstrap_end:
