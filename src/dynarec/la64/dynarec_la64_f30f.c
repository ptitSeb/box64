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

uintptr_t dynarec64_F30F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, gback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                v1 = fpu_get_scratch(dyn);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                VXOR_V(v0, v0, v0);
                FLD_S(v1, ed, fixedaddress);
            }
            VEXTRINS_W(v0, v1, 0);
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VEXTRINS_W(q0, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FST_S(v0, ed, fixedaddress);
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
            GETGX(v0, 1);
            GETED(0);
            d1 = fpu_get_scratch(dyn);
            MOVGR2FR_D(d1, ed);
            if(rex.w) {
                FFINT_S_L(d1, d1);
            } else {
                FFINT_S_W(d1, d1);
            }
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            if (!box64_dynarec_fastround) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINTRZ_L_S(d1, d0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINTRZ_W_S(d1, d0);
                MOVFR2GR_S(gd, d1);
            }
            if (!rex.w) ZEROUP(gd);
            if (!box64_dynarec_fastround) {
                MOVFCSR2GR(x5, FCSR2); // get back FPSR to check
                MOV32w(x3, (1 << FR_V) | (1 << FR_O));
                AND(x5, x5, x3);
                CBZ_NEXT(x5);
                if (rex.w) {
                    MOV64x(gd, 0x8000000000000000LL);
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FSQRT_S(d1, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FADD_S(d1, v0, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FMUL_S(d1, v0, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            d1 = fpu_get_scratch(dyn);
            FCVT_D_S(d1, v1);
            VEXTRINS_D(v0, d1, 0);
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FSUB_S(d1, v0, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGX(d0, 1);
            GETEXSS(d1, 0, 0);
            FCMP_S(fcc0, d0, d1, cUN);
            BCNEZ_MARK(fcc0);
            FCMP_S(fcc1, d1, d0, cLT);
            BCEQZ_MARK2(fcc1);
            MARK;
            v1 = fpu_get_scratch(dyn);
            FMOV_S(v1, d1);
            VEXTRINS_W(d0, v1, 0);
            MARK2;
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FDIV_S(d1, v0, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGX(d0, 1);
            GETEXSS(d1, 0, 0);
            FCMP_S(fcc0, d0, d1, cUN);
            BCNEZ_MARK(fcc0);
            FCMP_S(fcc1, d0, d1, cLT);
            BCEQZ_MARK2(fcc1);
            MARK;
            v1 = fpu_get_scratch(dyn);
            FMOV_S(v1, d1);
            VEXTRINS_W(d0, v1, 0);
            MARK2;
            break;
        case 0x6F:
            INST_NAME("MOVDQU Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                VOR_V(v0, v1, v1);
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            GETGX_empty(v0);
            VXOR_V(v0, v0, v0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                FLD_D(v1, ed, fixedaddress);
            }
            VEXTRINS_D(v0, v1, 0); // v0[63:0] = v1[63:0]
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 1);
            u8 = F8;
            switch (u8 & 7) {
                case 0: FCMP_S(fcc0, v0, v1, cEQ); break;  // Equal
                case 1: FCMP_S(fcc0, v0, v1, cLT); break;  // Less than
                case 2: FCMP_S(fcc0, v0, v1, cLE); break;  // Less or equal
                case 3: FCMP_S(fcc0, v0, v1, cUN); break;  // NaN
                case 4: FCMP_S(fcc0, v0, v1, cUNE); break; // Not Equal or unordered
                case 5: FCMP_S(fcc0, v1, v0, cULE); break; // Greater or equal or unordered
                case 6: FCMP_S(fcc0, v1, v0, cULT); break; // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                case 7: FCMP_S(fcc0, v0, v1, cOR); break;  // not NaN
            }
            MOVCF2GR(x2, fcc0);
            NEG_D(x2, x2);
            q1 = fpu_get_scratch(dyn);
            MOVGR2FR_W(q1, x2);
            VEXTRINS_W(v0, q1, 0);
            break;
        default:
            DEFAULT;
    }
    return addr;
}