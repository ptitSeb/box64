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

uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, gback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                FMVS(v0, q0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                FLW(v0, ed, fixedaddress);
                // reset upper part
                SW(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+4);
                SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                FMVS(q0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FSW(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
            
        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            GETGXSS(v0);
            GETED(0);
            if(rex.w) {
                FCVTSL(v0, ed);
            } else {
                FCVTSW(v0, ed);
            }
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FADDS(v0, v0, d0);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FMULS(v0, v0, d0);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETEXSS(v1, 0);
            GETGXSD_empty(v0);
            FCVTDS(v0, v1);
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FSUBS(v0, v0, d0);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 0);
            FEQS(x2, d0, d0);
            FEQS(x3, d1, d1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLTS(x2, d1, d0);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGXSS(v0);
            GETEXSS(d0, 0);
            FDIVS(v0, v0, d0);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 0);
            FEQS(x2, d0, d0);
            FEQS(x3, d1, d1);
            AND(x2, x2, x3);
            BEQ_MARK(x2, xZR);
            FLTS(x2, d0, d1);
            BEQ_MARK2(x2, xZR);
            MARK;
            FMVS(d0, d1);
            MARK2;
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            // Will load Gx as SD. Is that a good choice?
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                GETGXSD_empty(v0);
                FMVD(v0, v1);
            } else {
                GETGXSD_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLD(v0, ed, fixedaddress);
            }
            SD(xZR, xEmu, offsetof(x64emu_t, xmm[gd])+8);
            break;
        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(d1, 1);
            u8 = F8;
            if ((u8&7) == 0) {                                      // Equal
                FEQS(x2, d0, d1);
            } else if ((u8&7) == 4) {                               // Not Equal or unordered
                FEQS(x2, d0, d1);
                XORI(x2, x2, 1);
            } else {
                // x2 = !(isnan(d0) || isnan(d1))
                FEQS(x3, d0, d0);
                FEQS(x2, d1, d1);
                AND(x2, x2, x3);

                switch(u8&7) {
                case 1: BEQ_MARK(x2, xZR); FLTS(x2, d0, d1); break; // Less than
                case 2: BEQ_MARK(x2, xZR); FLES(x2, d0, d1); break; // Less or equal
                case 3: XORI(x2, x2, 1); break;                     // NaN
                case 5: {                                           // Greater or equal or unordered
                    BEQ_MARK2(x2, xZR);
                    FLES(x2, d1, d0);
                    B_MARK_nocond;
                    break;
                }
                case 6: {                                           // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                    BEQ_MARK2(x2, xZR);
                    FLTS(x2, d1, d0);
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
            FMVWX(d0, x2);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
