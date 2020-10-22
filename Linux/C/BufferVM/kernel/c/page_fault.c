//
// Created by ben on 27/12/16.
//

#include "../../libc/stdlib.h"
#include "../h/stack.h"
#include "../../common/utils.h"
#include "../h/vma_heap.h"
#include "../h/host.h"
#include "../../common/paging.h"
#include "../h/kernel.h"
#include "../../common/vma.h"

#define PF_PROT         (1<<0)
#define PF_WRITE        (1<<1)
#define PF_USER         (1<<2)
#define PF_RSVD         (1<<3)
#define PF_INSTR        (1<<4)

void handle_segfault(uint64_t addr, uint64_t rip) {

    vma_print();

    printf("\nSEGFAULT ERROR at addr: %p\n", addr);
    printf("RIP: %p\n", rip);
    disassemble_address(rip, 5);
    kernel_exit();
}

int handle_kernel_page_fault(uint64_t addr, uint32_t error_code, uint64_t rip) {
    vm_area_t *vma;
    uint64_t user_addr;

    if (addr >= user_version_start && addr < user_version_end + PAGE_SIZE) {

        user_addr = addr - user_version_start;

        if (CHECK_BIT(error_code, 2)) {
            vma = vma_find(user_addr);
            if (vma == NULL || !(vma->flags & VMA_IS_VERSIONED)) {
                printf("VA %p is either not mapped or does not support versioning. %lx %lx\n", user_addr, error_code, rip);
                kernel_exit();
            }
        }

        map_physical_pages(PAGE_ALIGN_DOWN(addr), -1, PDE64_NO_EXE | PDE64_WRITEABLE,
                           1, MAP_ZERO_PAGES);
        return (0);

    } else {
        printf("Kernel Page fault - VA: %p PC: %p Error: %d RIP: %p\n", addr, error_code, rip);
        kernel_exit();
    }
}

int handle_user_page_fault(uint64_t addr, uint64_t error_code, uint64_t rip) {

    vm_area_t *vma;
    bool grown = false;

    /*bool p, w, u, r, i;

    p = (bool)(error_code & PF_PROT);
    w = (bool)(error_code & PF_WRITE);
    u = (bool)(error_code & PF_USER);
    r = (bool)(error_code & PF_RSVD);
    i = (bool)(error_code & PF_INSTR);


    disassemble_address(rip, 5);

    printf("P: %d W: %d U: %d R: %d I: %d\n", p, w, u, r ,i);*/

    //disassemble_address(rip, 5);

    printf("page fault at %p error: %d rip %p\n", addr, error_code, rip);

    if (!(error_code & PF_PROT)) {

        vma = vma_find(addr);

        if (!vma) {
            printf("1\n");
            goto seg_fault;
        }

        if (vma->start_addr <= addr) {
            goto handle_paging;
        }

        if (!(vma->flags & VMA_GROWS)) {
            printf("2 %p %p %p\n", vma->start_addr, vma->end_addr, addr);
            goto seg_fault;
        }

        if (grow_stack(vma, addr)) {
            printf("3\n");
            goto seg_fault;
        } else {
            grown = true;
        }

handle_paging:
        VERIFY(grown || !(vma->flags & VMA_IS_PREFAULTED));
        printf("Loading page at addr: %p\n", addr);

        return 1;
    }
    printf("4\n");

    seg_fault:
    handle_segfault(addr, rip);
    return 0;
}

/*int64_t page_do_fault(vm_area_t *vma, size_t num_pages, bool continuous){
    int64_t phys_addr;

    phys_addr = map_physical_pages(vma->start_addr,
                                   -1, vma_prot_to_pg(vma->page_prot) | PDE64_USER,
                                   num_pages,
                                   MAP_CONTINUOUS, 0);

    return phys_addr;
}*/
