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
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
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
        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            GETGXSD(v0);
            GETED(0);
            if(rex.w) {
                FCVTDL(v0, ed, RD_RNE);
            } else {
                FCVTDW(v0, ed, RD_RNE);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(v0, 0);
            // TODO: fastnan handling
            FCVTLDxw(gd, v0, RD_RTZ);
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
        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            // TODO: fastnan handling
            GETGXSD(v0);
            GETEXSD(v1, 0);
            FADDD(v0, v0, v1);
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            //TODO: fastnan handling
            GETGXSD(v0);
            GETEXSD(v1, 0);
            FMULD(v0, v0, v1);
            break;
        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            //TODO: fastnan handling
            GETGXSD(v0);
            GETEXSD(v1, 0);
            FSUBD(v0, v0, v1);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            FEQD(x2, v0, v0);
            FEQD(x3, v1, v1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLED(x2, v1, v0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVD(v0, v1);
            MARK2;
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGXSD(v0);
            GETEXSD(v1, 0);
            if(!box64_dynarec_fastnan) {
                FEQD(x3, v0, v0);
                FEQD(x4, v1, v1);
            }
            FDIVD(v0, v0, v1);
            if(!box64_dynarec_fastnan) {
                AND(x3, x3, x4);
                CBZ_NEXT(x3);
                FEQD(x3, v0, v0);
                CBNZ_NEXT(x3);
                FNEGD(v0, v0);
            }
            break;
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGXSD(d0);
            GETEXSD(d1, 1);
            u8 = F8;
            if ((u8&7) == 0) {                                      // Equal
                FEQD(x2, d0, d1);
            } else if ((u8&7) == 4) {                               // Not Equal or unordered
                FEQD(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQD(x3, d0, d0);
                FEQD(x2, d1, d1);
                AND(x2, x2, x3);

                switch(u8&7) {
                case 1: BEQ_MARK(x2, xZR); FLTD(x2, d0, d1); break; // Less than
                case 2: BEQ_MARK(x2, xZR); FLED(x2, d0, d1); break; // Less or equal
                case 3: XORI(x2, x2, 1); break;                     // NaN
                case 5: {                                           // Greater or equal or unordered
                    BEQ_MARK2(x2, xZR);
                    FLED(x2, d1, d0);
                    B_MARK_nocond;
                    break;
                }
                case 6: {                                           // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                    BEQ_MARK2(x2, xZR);
                    FLTD(x2, d1, d0);
                    B_MARK_nocond;
                    break;
                }
                case 7: break;                                      // Not NaN
                }
                
                MARK2;
                if ((u8&7) == 5 || (u8&7) == 6) {
                    MOV32w(x2, 1);
                }
                MARK;
            }
            NEG(x2, x2);
            FMVDX(d0, x2);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
