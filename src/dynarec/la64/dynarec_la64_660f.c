#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "la64_emitter.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "bitutils.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

uintptr_t dynarec64_660F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8, s8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int v0, v1;
    int q0, q1;
    int d0, d1, d2;
    int64_t fixedaddress, gdoffset;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);

    switch (opcode) {
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x28:
            INST_NAME("MOVAPD Gx,Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VOR_V(v0, v1, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VILVL_H(v0, q0, v0);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VILVH_H(q0, q1, q0);
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FLD_D(v1, ed, fixedaddress);
            }
            VILVL_D(v0, v1, v0); // v0[127:64] = v1[63:0]
            break;
        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            GETGX_empty(v0);
            v1 = fpu_get_scratch(dyn);
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MOVGR2FR_D(v1, ed);
                } else {
                    MOVGR2FR_W(v1, ed);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                FLDxw(v1, ed, fixedaddress);
            }
            VXOR_V(v0, v0, v0);
            if (rex.w) {
                VEXTRINS_D(v0, v1, 0); // v0[63:0] = v1[63:0]
            } else {
                VEXTRINS_W(v0, v1, 0); // v0[31:0] = v1[31:0]
            }
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                VOR_V(v0, v1, v1);
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (rex.w) {
                if (MODREG) {
                    ed = TO_LA64((nextop & 7) + (rex.b << 3));
                    MOVFR2GR_D(ed, v0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    FST_D(v0, ed, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    ed = TO_LA64((nextop & 7) + (rex.b << 3));
                    MOVFR2GR_S(ed, v0);
                    ZEROUP(ed);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    FST_S(v0, ed, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    ed = TO_LA64((nextop & 7) + (rex.b << 3));
                    eb1 = ed;
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_LA64(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;   // L or H
                }
                if (eb2) {
                    SRLI_D(x1, eb1, eb2 * 8);
                    EXT_W_B(x1, x1);
                } else {
                    EXT_W_B(x1, eb1);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0);
                LD_B(x1, ed, fixedaddress);
            }
            BSTRINS_D(gd, x1, 15, 0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADD_D(v0, v0, q0);
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VXOR_V(v1, v1, v1);
                VEXTRINS_D(v1, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VAND_V(v0, v0, q0);
            break;
        case 0xEF:
            INST_NAME("PXOR Gx,Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for PXOR Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VXOR_V(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VXOR_V(q0, q0, q1);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
