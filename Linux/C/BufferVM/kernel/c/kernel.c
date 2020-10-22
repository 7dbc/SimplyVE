//
// Created by ben on 13/10/16.
//

#include <string.h>

#include "../h/cpu.h"
#include "../h/idt.h"
#include "../h/tss.h"
#include "../h/kernel.h"
#include "../h/syscall.h"
#include "../../common/paging.h"
#include "../h/host.h"

#include "../../intelxed/kit/include/xed/xed-interface.h"
#include "../h/stack.h"
#include "../../common/utils.h"
#include "../h/gdt.h"

uint64_t kernel_min_address;

tss_entry_t *tss;

uint64_t kernel_stack;
uint64_t user_heap_start;
uint64_t user_version_start;
uint64_t user_version_end;

void xed_user_abort_function(const char *msg, const char *file, int line, void *other) {
    printf("abort thing needs writing!\n");
    abort();
}

void *stack_alloc(uint64_t *sp, size_t length) {
    *sp -= length;
    //grow_stack(user_stack_vma, *sp);

    return (void *) *sp;
}

uint64_t stack_round(uint64_t *sp) {
    return (*sp = (((uint64_t) (*sp)) & ~15UL));
}

#define MAX_ENTRIES 10

void new_aux_entry(uint64_t *entries, uint64_t *num_entries, uint64_t id, uint64_t value) {
    if (*num_entries + 2 > MAX_ENTRIES * 2) {
        printf("AUX entry table is full.\n");
        return;
    }
    entries[(*num_entries)++] = id;
    entries[(*num_entries)++] = value;
}

void
kernel_main(void *kernel_entry, uint64_t _kernel_stack_max, uint64_t _tss_start, int argc, char *argv[], char *envp[]) {

    kernel_min_address = _tss_start;
    kernel_stack = _kernel_stack_max;
    tss = (tss_entry_t *) _tss_start;

    //TODO - add some assert mechanism to see if enough space (will this be needed?)

    uint64_t gdt_page = kernel_min_address - PAGE_SIZE;
    map_physical_pages(gdt_page, 0x1000, PDE64_WRITEABLE, 1, true);
    kernel_min_address = gdt_page;

    gdt_init(gdt_page);

    //unmap bootstrap and original gdt mapping
    unmap_physical_page(0x0000);
    unmap_physical_page(0x1000);

    cpu_init();
    tss_init(kernel_stack);
    idt_init(true);
    syscall_init();

    vma_init(1000);

    user_version_end = P2ALIGN(kernel_min_address, 2 * PAGE_SIZE);
    kernel_min_address = user_version_start = user_version_end / 2;

    user_stack_init(user_version_start, 16000);

    printf("\n-------------------\nKERNEL START\n-------------------\n\n");

    xed_register_abort_function(&xed_user_abort_function, 0);
    xed_tables_init();

    ASSERT(argc > 0);

    printf("Kernel stack: %p\n", kernel_stack);
    printf("Version store start: %p\n", user_version_start);
    printf("User stack start: %p\n", user_stack_start);
    printf("User stack min: %p\n", user_stack_min);
    printf("User binary location: %s\n", argv[0]);

    elf_info_t user_elf_info;
    void *elf_entry;
    int user_bin_fd = read_binary(argv[0]);
    load_elf_binary(user_bin_fd, &elf_entry, &user_elf_info, false);
    kernel_close(user_bin_fd);
    user_heap_start = user_elf_info.max_page_addr;

    printf("Heap stack region: %p %p\n", user_version_start, user_heap_start);

    vma_print();

    /*char **argvc = argv;
    printf("\nargv:");
    while(argc-- > 0)
        printf(" %s",*argvc++);
    printf("\n");

    printf("envp:");
    while(*envp)
        printf(" %s",*envp++);
    printf("\n\n");*/

    //User stuff
    load_user_land(user_stack_start, elf_entry, &user_elf_info, argc, argv, envp);
}

void load_user_land(uint64_t esp, void *elf_entry, elf_info_t *user_elf_info, int argc, char *argv[], char *envp[]) {

    char *arg;
    size_t items, arg_size;
    uint64_t *usp;

    size_t argv_array_size = 0;
    uint64_t argv_array_start;

    size_t envp_array_size = 0;
    uint64_t envp_array_start;

    char **envp_copy = envp;
    int envc = 0;

    while (*envp_copy++)
        envc++;

    uint64_t *user_argv[argc];
    uint64_t *user_envp[envc];

    //Put argv and envp onto user stack

    //argv
    for (int i = argc - 1; i >= 0; i--) {
        arg_size = strlen(argv[i]) + 1;
        arg = stack_alloc(&esp, arg_size);
        user_argv[i] = (uint64_t *) arg;
        argv_array_size += arg_size;
    }

    argv_array_start = esp;

    //envp
    for (int i = envc - 1; i >= 0; i--) {
        arg_size = strlen(envp[i]) + 1;
        arg = stack_alloc(&esp, arg_size);
        user_envp[i] = (uint64_t *) arg;
        envp_array_size += arg_size;
    }

    envp_array_start = esp;

    //TODO - random bytes for userspace PRNG seeding.
    //zero currently

    char *random_bytes = stack_alloc(&esp, 16);

    uint64_t num_entries = 0;
    uint64_t entries[MAX_ENTRIES * 2];

    new_aux_entry(entries, &num_entries, AT_ENTRY, (uint64_t) user_elf_info->entry_addr);
    new_aux_entry(entries, &num_entries, AT_FLAGS, 0);
    new_aux_entry(entries, &num_entries, AT_BASE, user_elf_info->base_addr);
    new_aux_entry(entries, &num_entries, AT_PHNUM, user_elf_info->phdr_num);
    new_aux_entry(entries, &num_entries, AT_PHENT, sizeof(elf64_phdr_t));
    new_aux_entry(entries, &num_entries, AT_PHDR, (uint64_t) user_elf_info->load_addr + user_elf_info->phdr_off);
    new_aux_entry(entries, &num_entries, AT_PAGESZ, PAGE_SIZE);
    new_aux_entry(entries, &num_entries, AT_RANDOM, (uint64_t) random_bytes);
    //new_aux_entry(entries, &num_entries, AT_HWCAP, 0);
    //new_aux_entry(entries, &num_entries, AT_HWCAP2, 0);

    //then working down stack
    //envp 0..n
    //envc
    //argv 0..n
    //argc

    stack_alloc(&esp, (num_entries + 2) * sizeof(uint64_t));

    items = ((argc + 1) + (envc + 1) + 1) * sizeof(uint64_t);
    stack_alloc(&esp, items);
    stack_round(&esp);

    grow_stack(user_stack_vma, esp);

    //now we can copy stack arguments into faulted memory

    memcpy((void *) argv_array_start, *argv, argv_array_size);
    memcpy((void *) envp_array_start, *envp, envp_array_size);

    //now work up the stack, copying the dynamic linkers arguments
    usp = (uint64_t *) esp;

    //argc onto top of aligned stack
    *(usp++) = argc;

    //write argv
    memcpy(usp, user_argv, sizeof(user_argv));
    usp += sizeof(user_argv) / sizeof(uint64_t);

    //null 64-bit before envp
    *(usp++) = 0;

    //write envp
    memcpy(usp, user_envp, sizeof(user_envp));
    usp += sizeof(user_envp) / sizeof(uint64_t);

    //null 64-bit after envp
    *(usp++) = 0;

    for (size_t i = 0; i < num_entries; i++) {
        *(usp++) = entries[i];
    }

    //null aux entry
    memset(usp, 0, 2 * sizeof(uint64_t));

    printf("\n-------------------\nINITIALISED USERLAND\n-------------------\n\n");

    printf("User entry %p, stack %p\n", elf_entry, esp);
    disassemble_address((uint64_t) elf_entry, 5);

    printf("\n-------------------\nENTERING USERLAND\n-------------------\n\n");

    switch_user_land(elf_entry, esp);
}
