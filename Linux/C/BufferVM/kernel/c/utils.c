//
// Created by ben on 27/12/16.
//
#include "../../common/utils.h"
#include "../../intelxed/kit/include/xed/xed-interface.h"
#include "../h/cpu.h"
#include "../h/vma_heap.h"

#define BUFLEN  100

uint64_t get_reg_value(xed_reg_enum_t reg, regs_t *regs){
    switch (reg){
        case XED_REG_AX:
        case XED_REG_EAX:
        case XED_REG_RAX:
            return regs->rax;
    }

    return 0;
}

uint64_t disassemble_instr_helper(uint64_t addr, char *buffer, xed_decoded_inst_t *xedd, regs_t *regs){

    xed_bool_t ok;
    xed_error_enum_t xed_error;

    xed_decoded_inst_zero(xedd);


    xed_decoded_inst_set_mode(xedd, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
    xed_error = xed_decode(xedd,
                           XED_REINTERPRET_CAST(const xed_uint8_t*, addr),
                           15);

    if (xed_error == XED_ERROR_NONE) {
        ok = xed_format_context(XED_SYNTAX_ATT, xedd, buffer, BUFLEN, 0, 0, 0);
        if (ok) {
            if (regs != NULL) printf("  rax: %x, rdi: %x, rsi: %x, rdx: %x, rcx: %x, rbx: %x, rbp: %x, rsp: %x, r8: %x, r9: %x, r10: %x,"
                           " r11: %x, r12: %x, r13: %x, r14: %x, r15: %x\n",
                   cpu_info.regs.rax,
                   cpu_info.regs.rdi,
                   cpu_info.regs.rsi,
                   cpu_info.regs.rdx,
                   cpu_info.regs.rcx,
                   cpu_info.regs.rbx,
                   cpu_info.regs.rbp,
                   cpu_info.regs.rsp,
                   cpu_info.regs.r8,
                   cpu_info.regs.r9,
                   cpu_info.regs.r10,
                   cpu_info.regs.r11,
                   cpu_info.regs.r12,
                   cpu_info.regs.r13,
                   cpu_info.regs.r14,
                   cpu_info.regs.r15

            );
            printf("  %x: %s\n", addr, buffer);
        } else
            printf("Error disassembling instruction at %p\n", addr);
        addr += xed_decoded_inst_get_length (xedd);
    }else{
        printf("Error decoding instruction at %p\n", addr);
    }

    return addr;
}

void disassemble_instr(uint64_t addr, regs_t *regs){
    char buffer[BUFLEN];
    xed_decoded_inst_t xedd;

    disassemble_instr_helper(addr, buffer, &xedd, regs);
}

void disassemble_address(uint64_t addr, size_t num_inst){
    uint64_t inst_start = addr;
    char buffer[BUFLEN];
    xed_decoded_inst_t xedd;

    for (size_t i = 0; i < num_inst; i++){
        inst_start = disassemble_instr_helper(inst_start, buffer, &xedd, NULL);
    }
}