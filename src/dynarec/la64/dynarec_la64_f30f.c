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
#include "bitutils.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

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
    uint8_t tmp1, tmp2, tmp3;
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
            if (MODREG) {
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
        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            GETEX(q1, 0, 0);
            GETGX_empty(q0);
            VPACKEV_W(q0, q1, q1);
            break;
        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            GETEX(q1, 0, 0);
            GETGX_empty(q0);
            VPACKOD_W(q0, q1, q1);
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
            if (rex.w) {
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
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINTRZ_L_S(d1, d0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINTRZ_W_S(d1, d0);
                MOVFR2GR_S(gd, d1);
                ZEROUP(gd);
            }
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
            INST_NAME("CVTSS2SI Gd, Ex");
            if (addr >= 0x1033f98d && addr <= 0x1033f98d + 8)
                EMIT(0);
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            if (!BOX64ENV(dynarec_fastround)) {
                MOVGR2FCSR(FCSR2, xZR); // reset all bits
            }
            u8 = sse_setround(dyn, ninst, x5, x6);
            d1 = fpu_get_scratch(dyn);
            if (rex.w) {
                FTINT_L_S(d1, d0);
                MOVFR2GR_D(gd, d1);
            } else {
                FTINT_W_S(d1, d0);
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
        case 0x38:  /* MAP */
            opcode = F8;
            switch(opcode) {
                case 0xF6:
                    INST_NAME("ADOX Gd, Ed");
                    nextop = F8;
                    READFLAGS(X_OF);
                    SETFLAGS(X_OF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    GETGD;
                    if(cpuext.lbt) {
                        X64_GET_EFLAGS(x3, X_OF);
                        SRLI_D(x3, x3, F_OF);
                    } else {
                        BSTRPICK_D(x3, xFlags, F_OF, F_OF);
                    }
                    IFX(X_OF) {
                        if(rex.w) {
                            ADD_D(x4, gd, ed);
                            SLTU(x5, x4, gd);
                            ADD_D(gd, x4, x3);
                            SLTU(x6, gd, x4);
                        } else {
                            ADD_W(x4, gd, ed);
                            ZEROUP(x4);
                            ZEROUP(gd);
                            SLTU(x5, x4, gd);
                            ADD_W(gd, x4, x3);
                            ZEROUP(gd);
                            SLTU(x6, gd, x4);
                        }
                        OR(x5, x5, x6);
                        if(cpuext.lbt) {
                            SLLI_D(x5, x5, F_OF);
                            X64_SET_EFLAGS(x5, X_OF);
                        } else {
                            BSTRINS_D(xFlags, x5, F_OF, F_OF);
                        }
                    } else {
                        ADDxw(x5, gd, ed);
                        ADDxw(gd, x5, x3);
                        if (!rex.w) ZEROUP(gd);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                               \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                     \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                                   \
        if (cpuext.lbt) {                                                                        \
            X64_SETJ(tmp1, I);                                                                   \
        } else {                                                                                 \
            GETFLAGS;                                                                            \
        }                                                                                        \
    }                                                                                            \
    nextop = F8;                                                                                 \
    GETGD;                                                                                       \
    if (MODREG) {                                                                                \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEJUMP(NATNO, 8);                                                                \
        } else {                                                                                 \
            if (cpuext.lbt)                                                                      \
                BEQZ(tmp1, 8);                                                                   \
            else                                                                                 \
                B##NO(tmp1, 8);                                                                  \
        }                                                                                        \
        MV(gd, ed);                                                                              \
        if (!rex.w) ZEROUP(gd);                                                                  \
    } else {                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, 1, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                                \
            NATIVEJUMP(NATNO, 8);                                                                \
        } else {                                                                                 \
            if (cpuext.lbt)                                                                      \
                BEQZ(tmp1, 8);                                                                   \
            else                                                                                 \
                B##NO(tmp1, 8);                                                                  \
        }                                                                                        \
        LDxw(gd, ed, fixedaddress);                                                              \
    }

            GOCOND(0x40, "CMOV", "Gd, Ed");

#undef GO

        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FSQRT_S(d1, d0);
            VEXTRINS_W(v0, d1, 0);
            break;
        case 0x52:
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            if (cpuext.frecipe) {
                FRSQRTE_S(q0, v1);
            } else {
                FRSQRT_S(q0, v1);
            }
            VEXTRINS_W(v0, q0, 0);
            break;
        case 0x53:
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            d1 = fpu_get_scratch(dyn);
            FRECIP_S(d1, v1);
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
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if (v0 == v1 && !BOX64ENV(dynarec_fastround)) {
                v1 = fpu_get_scratch(dyn);
                VOR_V(v1, v0, v0);
            }
            VFTINTRZ_W_S(v0, v1);
            if (!BOX64ENV(dynarec_fastround)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                VFCMP_S(q0, v1, v1, cEQ);
                VLDI(q1, 0b1001110000000); // broadcast 0x80000000
                VAND_V(v0, q0, v0);
                VANDN_V(d1, q0, q1);
                VOR_V(v0, v0, d1);
                VSUBI_WU(d1, q1, 1);
                VSEQ_W(q0, v0, d1);
                VSRLI_W(q0, q0, 31);
                VADD_W(v0, v0, q0);
            }
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
            q0 = fpu_get_scratch(dyn);
            if (BOX64ENV(dynarec_fastnan)) {
                FMIN_S(q0, d0, d1);
            } else {
                FCMP_S(fcc0, d1, d0, cULE);
                FSEL(q0, d0, d1, fcc0);
            }
            VEXTRINS_W(d0, q0, 0);
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
            q0 = fpu_get_scratch(dyn);
            if (BOX64ENV(dynarec_fastnan)) {
                FMAX_S(q0, d0, d1);
            } else {
                FCMP_S(fcc0, d1, d0, cLT);
                FSEL(q0, d1, d0, fcc0);
            }
            VEXTRINS_W(d0, q0, 0);
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
        case 0x70:
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX(v0, 1);
            u8 = F8;
            if (v0 == v1) {
                q0 = fpu_get_scratch(dyn);
                VSHUF4I_H(q0, v1, u8);
                VEXTRINS_D(v0, q0, 0x11); // v0[127:64] = q0[127:64]
            } else {
                VSHUF4I_H(v0, v1, u8);
                VEXTRINS_D(v0, v1, 0); // v0[63:0] = v1[63:0]
            }
            break;
        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                FLD_D(v1, ed, fixedaddress);
            }
            GETGX_empty(v0);
            if (v0 == v1) {
                VINSGR2VR_D(v0, xZR, 1);
            } else {
                VXOR_V(v0, v0, v0);
                VEXTRINS_D(v0, v1, 0); // v0[63:0] = v1[63:0]
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xAE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    if(rex.is32bits || !MODREG) {
                        INST_NAME("Illegal AE");
                        FAKEED;
                        UDF();
                    } else {
                        if (((nextop >> 3) & 7) == 1) {
                            INST_NAME("RDGSBASE");
                        } else {
                            INST_NAME("RDFSBASE");
                        }
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        int seg = _FS + ((nextop >> 3) & 7);
                        grab_segdata(dyn, addr, ninst, x4, seg);
                        MVxw(ed, x4);
                    }
                    break;
                case 2:
                case 3:
                    if (rex.is32bits || !MODREG) {
                        INST_NAME("Illegal AE");
                        FAKEED;
                        UDF();
                    } else {
                        if (((nextop >> 3) & 7) == 3) {
                            INST_NAME("WRGSBASE");
                        } else {
                            INST_NAME("WRFSBASE");
                        }
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        int seg = _FS + ((nextop >> 3) & 7) - 2;
                        if (!rex.w) {
                            ZEROUP2(x4, ed);
                            ST_D(x4, xEmu, offsetof(x64emu_t, segs_offs[seg]));
                        } else {
                            ST_D(ed, xEmu, offsetof(x64emu_t, segs_offs[seg]));
                        }
                    }
                    break;
                case 5:
                    INST_NAME("(unsupported) INCSSPD/INCSSPQ Ed");
                    FAKEED;
                    UDF();
                    break;
                case 6:
                    INST_NAME("(unsupported) UMONITOR Ed");
                    FAKEED;
                    UDF();
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xB8:
            INST_NAME("POPCNT Gd, Ed");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            v1 = fpu_get_scratch(dyn);
            GETGD;
            if (MODREG) {
                GETED(0);
                if (rex.w) {
                    VINSGR2VR_D(v1, ed, 0);
                } else {
                    VINSGR2VR_W(v1, ed, 0);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, 0);
                FLDxw(v1, ed, fixedaddress);
            }
            if (rex.w) {
                VPCNT_D(v1, v1);
                MOVFR2GR_D(gd, v1);
            } else {
                VPCNT_W(v1, v1);
                VPICKVE2GR_WU(gd, v1, 0);
            }
            IFX (X_ALL) {
                if (cpuext.lbt) {
                    X64_SET_EFLAGS(xZR, X_ALL);
                    BNEZ_MARK(gd);
                    ADDI_D(x3, xZR, 1 << F_ZF);
                    X64_SET_EFLAGS(x3, X_ZF);
                } else {
                    CLEAR_FLAGS(x2);
                    BNEZ_MARK(gd);
                    ORI(xFlags, xFlags, 1 << F_ZF);
                }
                MARK;
            }
            break;
        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            RESTORE_EFLAGS(x6);
            /*
                ZF is set if gd is zero, unset non-zero.
                CF is set if ed is zero, unset non-zero.
                OF, SF, PF, and AF flags are undefined
            */
            CLEAR_FLAGS(x2);
            CTZxw(gd, ed);
            BNE(gd, xZR, 4 + 4);
            ORI(xFlags, xFlags, 1 << F_ZF);
            SRLI_W(x5, gd, rex.w ? 6 : 5); // maximum value is 64/32, F_CF = 0
            OR(xFlags, xFlags, x5);
            SPILL_EFLAGS();
            break;
        case 0xBD:
            INST_NAME("LZCNT Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            RESTORE_EFLAGS(x6);
            /*
                ZF is set if gd is zero, unset non-zero.
                CF is set if ed is zero, unset non-zero.
                OF, SF, PF, and AF flags are undefined
            */
            CLEAR_FLAGS(x2);
            CLZxw(gd, ed);
            BNE(gd, xZR, 4 + 4);
            ORI(xFlags, xFlags, 1 << F_ZF);
            SRLI_W(x5, gd, rex.w ? 6 : 5); // maximum value is 64/32, F_CF = 0
            OR(xFlags, xFlags, x5);
            SPILL_EFLAGS();
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
        case 0xD6:
            INST_NAME("MOVQ2DQ Gx, Em");
            nextop = F8;
            GETGX_empty(v0);
            GETEM(v1, 0);
            VXOR_V(v0, v0, v0);
            VEXTRINS_D(v0, v1, VEXTRINS_IMM_4_0(0, 0));
            break;
        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            GETEXSD(v1, 0, 0);
            GETGX_empty(v0);
            VFFINTL_D_W(v0, v1);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
