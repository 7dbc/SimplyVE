#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "buffervm.h"
#include "../libc/version.h"

int main(int argc, char *argv[], char *envp[]){
    printf("hello world from glibc!\n");

    printf("\nargv:");
    while(argc-- > 0)
        printf(" %s",*argv++);
    printf("\n");

    printf("envp:");
    while(*envp)
        printf(" %s",*envp++);
    printf("\n\n");

    char *hello = "this is a string literal\n";
    size_t size = strlen(hello)+1;

    char *mem = malloc(size);
    //mem = set_version((void *)mem, size, 5);

    printf("Versioned buffer from %p to %p\n", mem, mem + size - 1);

    memcpy(mem, hello, size);
    mem[size-1] = 0;
    //mem[size] = 0;

    mem = set_version_ptr(4, mem);
    mem[size-1] = 0;
    printf("here is some memory: %p %s\n", mem, mem);

    return 0;
}
