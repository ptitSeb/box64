#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_F20F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

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

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
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
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                VEXTRINS_D(d0, v0, 0); // d0[63:0] = v0[63:0]
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VREPLVE_D(v0, d0, xZR);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                VLDREPL_D(v0, ed, 0);
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
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINTRZ_L_D(d1, q0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINTRZ_W_D(d1, q0);
                MOVFR2GR_S(gd, d1);
                ZEROUP(gd);
            }
            if (!rex.w) ZEROUP(gd);
            if (!BOX64ENV(dynarec_fastround)) {
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
            if (!BOX64ENV(dynarec_fastround)) {
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
                ZEROUP(gd);
            }
            x87_restoreround(dyn, ninst, u8);
            if (!BOX64ENV(dynarec_fastround)) {
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

        case 0x38: // these are some more SSSE4.2+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0xF0:
                    INST_NAME("CRC32 Gd, Eb");
                    nextop = F8;
                    GETEB(x1, 0);
                    GETGD;
                    CRCC_W_B_W(gd, gd, ed);
                    ZEROUP(gd);
                    break;
                case 0xF1:
                    INST_NAME("CRC32 Gd, Ed");
                    nextop = F8;
                    GETED(0);
                    GETGD;
                    if (rex.w) {
                        CRCC_W_D_W(gd, gd, ed);
                    } else {
                        CRCC_W_W_W(gd, gd, ed);
                    }
                    ZEROUP(gd);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x51:
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSD(d0, 0, 0);
            FSQRT_D(d1, d0);
            if (!BOX64ENV(dynarec_fastnan)) {
                v1 = fpu_get_scratch(dyn);
                MOVGR2FR_D(v1, xZR);
                FCMP_D(fcc0, d0, v1, cLT);
                BCEQZ(fcc0, 4 + 4);
                FNEG_D(d1, d1);
            }
            VEXTRINS_D(v0, d1, 0);
            break;
        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FADD_D(d0, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
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
            if (!BOX64ENV(dynarec_fastnan)) {
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
            if (!BOX64ENV(dynarec_fastnan)) {
                FCMP_D(fcc0, v0, v1, cUN);
                BCNEZ_MARK(fcc0);
                FCMP_D(fcc1, d0, d0, cOR);
                BCNEZ_MARK(fcc1);
                FNEG_D(d0, d0);
            }
            MARK;
            VEXTRINS_D(v0, d0, 0); // v0[63:0] = d0[63:0]
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            FCMP_D(fcc0, v0, v1, cUN);
            BCNEZ_MARK(fcc0);
            FCMP_D(fcc1, v1, v0, cLE);
            BCEQZ_MARK2(fcc1);
            MARK;
            VEXTRINS_D(v0, v1, 0);
            MARK2;
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FDIV_D(d0, v0, v1);
            if (!BOX64ENV(dynarec_fastnan)) {
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
        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX(v0, 1);
            u8 = F8;
            if (v0 != v1) {
                VSHUF4I_H(v0, v1, u8);
                VEXTRINS_D(v0, v1, 0x11); // v0[127:64] = v1[127:64]
            } else {
                q0 = fpu_get_scratch(dyn);
                VSHUF4I_H(q0, v1, u8);
                VEXTRINS_D(v0, q0, 0x0); // v0[63:0] = q0[63:0]
            }
            break;
        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VPICKEV_W(v0, q1, q0);
            VPICKOD_W(v1, q1, q0);
            VFADD_S(q0, v0, v1);
            break;
        case 0x7D:
            INST_NAME("HSUBPS Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VPICKEV_W(v0, q1, q0);
            VPICKOD_W(v1, q1, q0);
            VFSUB_S(q0, v0, v1);
            break;
        case 0xAE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 6:
                    INST_NAME("(unsupported) UWAIT Ed");
                    FAKEED;
                    UDF();
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 1);
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
        case 0xD0:
            INST_NAME("ADDSUBPS Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn);
            VFSUB_S(v0, q0, q1);
            VFADD_S(q0, q0, q1);
            VEXTRINS_W(q0, v0, 0);
            VEXTRINS_W(q0, v0, 0b00100010);
            break;
        case 0xD6:
            INST_NAME("MOVDQ2Q Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEXSD(v1, 0, 0);
            FMOV_D(v0, v1);
            break;
        case 0xE6:
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            u8 = sse_setround(dyn, ninst, x1, x2);
            if (BOX64ENV(dynarec_fastround)) {
                VFTINT_W_D(v0, v1, v1);
            } else {
                d0 = fpu_get_scratch(dyn);
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                VFTINT_W_D(d0, v1, v1);
                VLDI(q0, 0b1001110000000); // broadcast 32bit 0x80000000 to all
                LU52I_D(x5, xZR, 0x41e);
                VREPLGR2VR_D(q1, x5);
                VFCMP_D(q1, q1, v1, cULE);
                VSHUF4I_W(q1, q1, 0b00001000);
                VBITSEL_V(v0, d0, q0, q1);
            }
            x87_restoreround(dyn, ninst, u8);
            VINSGR2VR_D(v0, xZR, 1);
            break;
        case 0xF0:
            INST_NAME("LDDQU Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VOR_V(v0, v1, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        default:
            DEFAULT;
    }
    return addr;
}
