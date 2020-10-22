//
// Created by ben on 07/12/16.
//

#include <stdio.h>

#include "../h/host.h"
#include "../../libc/version.h"
#include "../../common/version.h"
#include "../h/kernel.h"
#include "../../common/paging.h"
#include "../../intelxed/kit/include/xed/xed-interface.h"

bool on_same_page(void *addr1, void *addr2) {
    return P2ALIGN((uint64_t) addr1, PAGE_SIZE) == P2ALIGN((uint64_t) addr2, PAGE_SIZE);
}

void *set_version(uint64_t *addr, size_t length, uint64_t version) {
    uint8_t *version_buf = (uint8_t *) user_version_start + (uint64_t) normalise_version_ptr(addr);

    for (size_t i = 0; i < length; i++) {
        version_buf[i] = (uint8_t) version;
    }

    return set_version_ptr(version, addr);
}

uint8_t get_version(uint64_t *addr) {
    uint8_t *version_buf = (uint8_t *) user_version_start + (uint64_t) normalise_version_ptr(addr);

    return (*version_buf);
}

bool check_version_instruction(uint64_t *addr, uint64_t *rip, size_t offset, uint64_t ptr_ver) {
    xed_error_enum_t xed_error;
    xed_bool_t ok;
    xed_decoded_inst_t xedd;
    uint64_t inst_len, m_ver;

    xed_decoded_inst_zero(&xedd);

    xed_decoded_inst_set_mode(&xedd, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
    xed_error = xed_decode(&xedd,
                           XED_REINTERPRET_CAST(const xed_uint8_t*, rip),
                           15);

    if (xed_error == XED_ERROR_NONE) {
        inst_len = xed_decoded_inst_get_operand_width(&xedd) / 8;
        if (inst_len <= offset)
            return (true);
    }
    m_ver = get_version(addr + offset);

    printf("VADDR: %p, RIP: %p PVer: %d, MVer: %d, Byte (in operand): %d\n", addr, rip, ptr_ver, m_ver, offset);

    return (false);
}

bool check_version(void *addr, uint64_t *rip) {

    //printf("Checking version of VA %p at RIP: %p\n", addr, rip);
    uint64_t ptr_ver = get_version_ptr(addr);
    void *norm_addr = normalise_version_ptr(addr);
    for (size_t i = 0; i < 10; i++) {
        void *off_ptr_nor = normalise_version_ptr(addr + i);
        if (!on_same_page(norm_addr, off_ptr_nor) && is_vpage_present((uint64_t) off_ptr_nor)) {
            //check to see if rip size was bigger than i
            //if so, this would have likely failed on execution, but we caught it here
            return check_version_instruction(addr, rip, i, ptr_ver);
        } else if (get_version(addr + i) != ptr_ver) {
            return check_version_instruction(addr, rip, i, ptr_ver);
        }
    }
    return (true);
}
