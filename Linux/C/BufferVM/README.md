# BufferVM

A hardware based virtual machine for sand-boxing Linux userland processes on AMD64. Built on top of Linux KVM, utilising the Intel VT/AMD-V virtualisation extensions supported on modern AMD64 hardware. Can be used to catch heap based memory bugs (use after free, use after realloc, heap overflow etc) with minimal overhead compared to other solutions (i.e. Valgrind). The application is ran on top of a shim kernel that intercepts system calls to forward them up to the host kernel, along with managing its own virtual memory allocations (key to segregating them).

Currently implements a small number of syscalls, and is capable of loading ld-linux and glibc appications.
