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

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

uintptr_t dynarec64_F20F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t u8;
    uint64_t u64, j64;
    int v0, v1;
    int q0, q1;
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
                ed = (nextop & 7) + (rex.b << 3);
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                v1 = fpu_get_scratch(dyn);
                VXOR_V(v0, v0, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 8, 0);
                FLD_D(v1, ed, fixedaddress);
            }
            VEXTRINS_D(v0, v1, 0); // v0[63:0] = v1[63:0]
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                VEXTRINS_D(d0, v0, 0); // d0[63:0] = v0[63:0]
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            GETGX(v0, 1);
            GETED(0);
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                MOVGR2FR_D(d1, ed);
                FFINT_D_L(d1, d1);
            } else {
                MOVGR2FR_W(d1, ed);
                FFINT_D_W(d1, d1);
            }
            VEXTRINS_D(v0, d1, 0);
            break;
        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(q0, 0, 0);
            if (!box64_dynarec_fastround) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINTRZ_L_D(d1, q0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINTRZ_W_D(d1, q0);
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
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(q0, 0, 0);
            if (!box64_dynarec_fastround) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            u8 = sse_setround(dyn, ninst, x2, x3);
            if (rex.w) {
                FTINT_L_D(d1, q0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINT_W_D(d1, q0);
                MOVFR2GR_S(gd, d1);
            }
            x87_restoreround(dyn, ninst, u8);
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
        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FADD_D(d0, v0, v1);
            if (!box64_dynarec_fastnan) {
                FCMP_D(fcc0, v0, v1, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            VEXTRINS_D(v0, d0, 0); // v0[63:0] = d0[63:0]
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FMUL_D(d0, v0, v1);
            if (!box64_dynarec_fastnan) {
                FCMP_D(fcc0, v0, v1, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            VEXTRINS_D(v0, d0, 0); // v0[63:0] = d0[63:0]
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            FCVT_S_D(d1, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FSUB_D(d0, v0, v1);
            if (!box64_dynarec_fastnan) {
                FCMP_D(fcc0, v0, v1, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            VEXTRINS_D(v0, d0, 0); // v0[63:0] = d0[63:0]
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FDIV_D(d0, v0, v1);
            if (!box64_dynarec_fastnan) {
                FCMP_D(fcc0, v0, v1, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            VEXTRINS_D(v0, d0, 0); // v0[63:0] = d0[63:0]
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            FCMP_D(fcc0, v0, v1, cUN);
            BCNEZ_MARK(fcc0);
            FCMP_D(fcc1, v0, v1, cLE);
            BCEQZ_MARK2(fcc1);
            MARK;
            VEXTRINS_D(v0, v1, 0);
            MARK2;
            break;
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 1);
            u8 = F8;
            switch (u8 & 7) {
                case 0: FCMP_D(fcc0, v0, v1, cEQ); break;  // Equal
                case 1: FCMP_D(fcc0, v0, v1, cLT); break;  // Less than
                case 2: FCMP_D(fcc0, v0, v1, cLE); break;  // Less or equal
                case 3: FCMP_D(fcc0, v0, v1, cUN); break;  // NaN
                case 4: FCMP_D(fcc0, v0, v1, cUNE); break; // Not Equal or unordered
                case 5: FCMP_D(fcc0, v1, v0, cULE); break; // Greater or equal or unordered
                case 6: FCMP_D(fcc0, v1, v0, cULT); break; // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                case 7: FCMP_D(fcc0, v0, v1, cOR); break;  // not NaN
            }
            MOVCF2GR(x2, fcc0);
            NEG_D(x2, x2);
            q1 = fpu_get_scratch(dyn);
            MOVGR2FR_D(q1, x2);
            VEXTRINS_D(v0, q1, 0);
            break;
        default:
            DEFAULT;
    }
    return addr;
}