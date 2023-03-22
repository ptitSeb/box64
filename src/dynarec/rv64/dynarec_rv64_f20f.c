#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wback;
    uint8_t u8;
    uint64_t u64, j64;
    int v0, v1;
    int q0;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(v0);
    MAYUSE(v1);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                FMVD(v0, d0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLD(v0, ed, fixedaddress);
                // reset upper part
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                FMVD(d0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FSD(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0x38:  // these are some more SSSE4.2+ opcodes
            opcode = F8;
            switch(opcode) {

                case 0xF0:
                    INST_NAME("(unsupported) CRC32 Gd, Eb)");
                    nextop = F8;
                    addr = fakeed(dyn, addr, ninst, nextop);
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags in "don't care" state
                    GETIP(ip);
                    STORE_XEMU_CALL();
                    CALL(native_ud, -1);
                    LOAD_XEMU_CALL();
                    jump_to_epilog(dyn, 0, xRIP, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 0xF1:
                    INST_NAME("(unsupported) CRC32 Gd, Ed)");
                    nextop = F8;
                    addr = fakeed(dyn, addr, ninst, nextop);
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags in "don't care" state
                    GETIP(ip);
                    STORE_XEMU_CALL();
                    CALL(native_ud, -1);
                    LOAD_XEMU_CALL();
                    jump_to_epilog(dyn, 0, xRIP, ninst);
                    *need_epilog = 0;
                    *ok = 0;
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
