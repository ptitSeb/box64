#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"
#include "emu/x64compstrings.h"

uintptr_t dynarec64_66F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    #if STEP > 1
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    static const int8_t mask_string8[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
    static const int8_t mask_string16[] = { 15, 14, 13, 12, 11, 10, 9, 8 };
    static const int8_t round_round[] = { 0, 2, 1, 3};
    #endif

    switch(opcode) {

        case 0x38:  // SSE 4.x
            opcode = F8;
            switch(opcode) {

                case 0xF1:
                    INST_NAME("CRC32 Gd, Ew");
                    nextop = F8;
                    GETEW(x1, 0);
                    GETGD;
                    if(arm64_crc32) {
                        CRC32CH(gd, gd, ed);
                    } else {
                        MOV32w(x2, 0x82f63b78);
                        for(int j=0; j<2; ++j) {
                            UBFXxw(x3, ed, 8*j, 8);
                            EORw_REG(gd, gd, x3);
                            for(int i=0; i<8; ++i) {
                                LSRw_IMM((i&1)?gd:x4, (i&1)?x4:gd, 1);
                                TBZ((i&1)?x4:gd, 0, 4+4);
                                EORw_REG((i&1)?gd:x4, (i&1)?gd:x4, x2);
                            }
                        }
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
