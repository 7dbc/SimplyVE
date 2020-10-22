#include "../common/paging.h"
#include "elf.h"
#include "vm.h"
#include "../common/elf.h"
#include <unistd.h>

struct vm_t vm;

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 3) {
        fprintf(stderr, "%s kernel.elf binary\n", argv[0]);
        return 1;
    }

    /*char **argvc = argv;
    printf("\nargv:");
    while(argc-- > 0)
        printf(" %s",*argvc++);
    printf("\n");

    printf("envp:");
    while(*envp)
        printf(" %s",*envp++);
    printf("\n\n");*/


    int kernel_binary_fd;
    if (!(kernel_binary_fd = read_binary(argv[1]))) {
        return 1;
    }

    struct vcpu_t vcpu;

    vm_init(&vm, PHYSICAL_HEAP_SIZE);
    vcpu_init(&vm, &vcpu);

    run(&vm, &vcpu, kernel_binary_fd, argc - 2, &argv[2], envp);

    close(kernel_binary_fd);

    return 0;
}
