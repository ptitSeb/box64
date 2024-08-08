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
        case 0x10:
            INST_NAME("MOVUPD Gx,Ex");
            nextop = F8;
            GETG;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                VOR_V(v0, v1, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VLD(v0, ed, fixedaddress);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            v1 = fpu_get_scratch(dyn);
            FLD_D(v1, wback, fixedaddress);
            VEXTRINS_D(v0, v1, 0);
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                FLD_D(v1, ed, fixedaddress);
            }
            VEXTRINS_D(v0, v1, 0x10);
            break;
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
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("COMISD Gx, Ex");
            } else {
                INST_NAME("UCOMISD Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGX(d0, 0);
            GETEXSD(v0, 0, 0);

            CLEAR_FLAGS(x3);
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FCMP_D(fcc0, d0, v0, cUN);
                BCEQZ_MARK(fcc0);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_MARK3_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FCMP_D(fcc1, d0, v0, cLT);
                BCEQZ_MARK2(fcc1);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_MARK3_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FCMP_D(fcc2, d0, v0, cEQ);
                BCEQZ_MARK3(fcc2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            MARK3;
            SPILL_EFLAGS();
            break;
        case 0x38: // SSSE3 opcodes
            nextop = F8;
            switch (nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VLDI(v0, 0b0000010001111); // broadcast 0b10001111 as byte
                    VAND_V(v0, v0, q1);
                    VMINI_BU(v0, v0, 0x1f);
                    VXOR_V(v1, v1, v1);
                    VSHUF_B(q0, v1, q0, v0);
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VADD_H(q0, v0, v1);
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_W(v0, q1, q0);
                    VPICKOD_W(v1, q1, q0);
                    VADD_W(q0, v0, v1);
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VSADD_H(q0, v0, v1);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gx,Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VEXT2XV_HU_BU(v0, q0);
                    VEXT2XV_H_B(v1, q1);
                    XVMUL_H(v0, v0, v1);
                    XVPERMI_Q(v1, v0, 1); // v1[127:0] = v0[255:128];
                    VPICKEV_H(q0, v1, v0);
                    VPICKOD_H(v0, v1, v0);
                    VSADD_H(q0, v0, q0);
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VSUB_H(q0, v0, v1);
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_W(v0, q1, q0);
                    VPICKOD_W(v1, q1, q0);
                    VSUB_D(q0, v0, v1);
                    break;
                case 0x07:
                    INST_NAME("PHSUBSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn);
                    v1 = fpu_get_scratch(dyn);
                    VPICKEV_H(v0, q1, q0);
                    VPICKOD_H(v1, q1, q0);
                    VSSUB_H(q0, v0, v1);
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    VSIGNCOV_B(q0, q1, q0);
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    VSIGNCOV_H(q0, q1, q0);
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    VSIGNCOV_W(q0, q1, q0);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x3A: // these are some more SSSE3+ opcodes
            opcode = F8;
            switch (opcode) {
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 1);
                    u8 = F8;
                    if (u8 > 31) {
                        VXOR_V(q0, q0, q0);
                    } else if (u8 > 15) {
                        VBSRL_V(q0, q0, u8 - 16);
                    } else if (!u8) {
                        VOR_V(q0, q1, q1);
                    } else {
                        d0 = fpu_get_scratch(dyn);
                        VBSLL_V(q0, q0, 16 - u8);
                        VBSRL_V(d0, q1, u8);
                        VOR_V(q0, q0, d0);
                    }
                    break;
                case 0x16:
                    if (rex.w) {
                        INST_NAME("PEXTRQ Ed, Gx, Ib");
                    } else {
                        INST_NAME("PEXTRD Ed, Gx, Ib");
                    }
                    nextop = F8;
                    GETGX(q0, 0);
                    d0 = fpu_get_scratch(dyn);
                    if (MODREG) {
                        ed = TO_LA64((nextop & 7) + (rex.b << 3));
                        u8 = F8;
                        if (rex.w) {
                            VBSRL_V(d0, q0, (u8 & 1) * 8);
                            MOVFR2GR_D(ed, d0);
                        } else {
                            VBSRL_V(d0, q0, (u8 & 3) * 4);
                            MOVFR2GR_S(ed, d0);
                            ZEROUP(ed);
                        }
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 1, 1);
                        u8 = F8;
                        if (rex.w) {
                            VBSRL_V(d0, q0, (u8 & 1) * 8);
                            FST_D(d0, ed, fixedaddress);
                        } else {
                            VBSRL_V(d0, q0, (u8 & 3) * 4);
                            FST_S(d0, ed, fixedaddress);
                        }
                        SMWRITE2();
                    }
                    break;
                case 0x22:
                    INST_NAME("PINSRD Gx, ED, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETED(1);
                    u8 = F8;
                    d0 = fpu_get_scratch(dyn);
                    if (rex.w) {
                        MOVGR2FR_D(d0, ed);
                        VEXTRINS_D(q0, d0, (u8 & 1) << 4);
                    } else {
                        MOVGR2FR_W(d0, ed);
                        VEXTRINS_W(q0, d0, (u8 & 3) << 4);
                    }
                    break;
                case 0x44:
                    INST_NAME("PCLMULQDQ Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x5, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL(native_pclmul, -1);
                    break;
                case 0xDF:
                    INST_NAME("AESKEYGENASSIST Gx, Ex, Ib"); // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if (MODREG) {
                        ed = (nextop & 7) + (rex.b << 3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0); // p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 0, 1);
                        if (ed != x3) {
                            MV(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL(native_aeskeygenassist, -1);
                    break;
                default:
                    DEFAULT;
            }
            break;

        #define GO(GETFLAGS, NO, YES, F, I)                                                          \
            READFLAGS(F);                                                                            \
            if (la64_lbt) {                                                                          \
                X64_SETJ(x1, I);                                                                     \
            } else {                                                                                 \
                GETFLAGS;                                                                            \
            }                                                                                        \
            nextop = F8;                                                                             \
            GETGD;                                                                                   \
            if (MODREG) {                                                                            \
                ed = TO_LA64((nextop & 7) + (rex.b << 3));                                           \
            } else {                                                                                 \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0); \
                LD_HU(x4, ed, fixedaddress);                                                         \
                ed = x4;                                                                             \
            }                                                                                        \
            if (la64_lbt)                                                                            \
                BEQZ(x1, 4 + 4);                                                                     \
            else                                                                                     \
                B##NO(x1, 4 + 4);                                                                    \
            BSTRINS_D(gd, ed, 15, 0);

            GOCOND(0x40, "CMOV", "Gd, Ed");
        #undef GO

        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VAND_V(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VANDN_V(v0, v0, q0);
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VOR_V(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPD Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VXOR_V(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VXOR_V(q0, q0, q1);
            }
            break;
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!box64_dynarec_fastnan) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_D(v0, q0, q1, cUN);
            }
            VFADD_D(q1, q1, q0);
            if (!box64_dynarec_fastnan) {
                VFCMP_D(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111000); // broadcast 0xFFFFFFFFFFFFFFF8
                VSLLI_D(v1, v1, 48);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!box64_dynarec_fastnan) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_D(v0, q0, q1, cUN);
            }
            VFMUL_D(q1, q1, q0);
            if (!box64_dynarec_fastnan) {
                VFCMP_D(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111000); // broadcast 0xFFFFFFFFFFFFFFF8
                VSLLI_D(v1, v1, 48);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            // TODO: !box64_dynarec_fastround
            q0 = fpu_get_scratch(dyn);
            VFCVT_S_D(q0, v1, v1);
            VXOR_V(v0, v0, v0);
            VEXTRINS_D(v0, q0, 0);
            break;
        case 0x5B:
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            u8 = sse_setround(dyn, ninst, x6, x4);
            if (v0 == v1 && !box64_dynarec_fastround) {
                v1 = fpu_get_scratch(dyn);
                VOR_V(v1, v0, v0);
            }
            VFTINT_W_S(v0, v1);
            if (!box64_dynarec_fastround) {
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
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!box64_dynarec_fastnan) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_D(v0, q0, q1, cUN);
            }
            VFSUB_D(q1, q1, q0);
            if (!box64_dynarec_fastnan) {
                VFCMP_D(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111000); // broadcast 0xFFFFFFFFFFFFFFF8
                VSLLI_D(v1, v1, 48);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if (!box64_dynarec_fastnan) {
                v0 = fpu_get_scratch(dyn);
                v1 = fpu_get_scratch(dyn);
                VFCMP_D(v0, q0, q1, cUN);
            }
            VFDIV_D(q1, q1, q0);
            if (!box64_dynarec_fastnan) {
                VFCMP_D(v1, q1, q1, cUN);
                VANDN_V(v0, v0, v1);
                VLDI(v1, 0b011111111000); // broadcast 0xFFFFFFFFFFFFFFF8
                VSLLI_D(v1, v1, 48);
                VAND_V(v1, v0, v1);
                VANDN_V(v0, v0, q1);
                VOR_V(q1, v0, v1);
            }
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VILVL_B(v0, q0, v0);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VILVL_H(v0, q0, v0);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VILVL_W(v0, q0, v0);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            if (v0 == v1) {
                VSAT_H(v0, v0, 7);
                VPICKEV_B(v0, v0, v0);
            } else {
                VSAT_H(v0, v0, 7);
                VSAT_H(q0, v1, 7);
                VPICKEV_B(v0, q0, v0);
            }
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSLT_B(v0, v1, v0);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSLT_H(v0, v1, v0);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSLT_W(v0, v1, v0);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn);
            VLDI(q0, 0b0010011111111); // broadcast 0xff as 16-bit elements to all lanes
            if (v0 == v1) {
                VMAXI_H(v0, v0, 0);
                VMIN_H(v0, v0, q0);
                VPICKEV_B(v0, v0, v0);
            } else {
                q1 = fpu_get_scratch(dyn);
                VMAXI_H(v0, v0, 0);
                VMAXI_H(q1, v1, 0);
                VMIN_H(v0, v0, q0);
                VMIN_H(q1, q1, q0);
                VPICKEV_B(v0, q1, v0);
            }
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VILVH_B(q0, q1, q0);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VILVH_H(q0, q1, q0);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VILVH_W(q0, q1, q0);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            VOR_V(d0, v1, v1);
            VSSRANI_H_W(d0, v0, 0);
            VOR_V(v0, d0, d0);
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
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 8, 0);
                FLD_D(v1, ed, fixedaddress + 8);
                VEXTRINS_D(v1, v1, 0x10);
            }
            VILVH_D(v0, v1, v0);
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
        case 0x70:
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            VSHUF4I_W(v0, v1, u8);
            break;
        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            VXOR_V(q0, q0, q0);
                        } else if (u8) {
                            VSRLI_H(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        VSRAI_H(q0, q0, u8);
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            VXOR_V(q0, q0, q0);
                        } else {
                            VSLLI_H(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            VXOR_V(q0, q0, q0);
                        } else if (u8) {
                            VSRLI_W(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        VSRAI_W(q0, q0, u8);
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            VXOR_V(q0, q0, q0);
                        } else {
                            VSLLI_W(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            VXOR_V(q0, q0, q0);
                        } else {
                            VSRLI_D(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XVOR_V(q0, q0, q0);
                        } else {
                            VBSRL_V(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            XVOR_V(q0, q0, q0);
                        } else {
                            VSLLI_D(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            VXOR_V(q0, q0, q0);
                        } else if (u8 > 0) {
                            VBSLL_V(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSEQ_B(v0, v0, q0);
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSEQ_H(v0, v0, q0);
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSEQ_W(v0, v0, q0);
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
        case 0x7F:
            INST_NAME("MOVDQA Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xAF:
            INST_NAME("IMUL Gw,Ew");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETSEW(x1, 0);
            GETSGW(x2);
            MUL_W(x2, x2, x1);
            UFLAG_RES(x2);
            BSTRPICK_D(x2, x2, 15, 0);
            GWBACK;
            UFLAG_DF(x1, d_imul16);
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
        case 0xC4:
            INST_NAME("PINSRW Gx, Ed, Ib");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                u8 = (F8) & 7;
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 1, 1);
                u8 = (F8) & 7;
                ed = x3;
                LD_HU(ed, wback, fixedaddress);
            }
            d0 = fpu_get_scratch(dyn);
            MOVGR2FR_D(d0, ed);
            VEXTRINS_H(v0, d0, (u8 << 4));
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd, Ex, Ib");
            nextop = F8;
            GETGD;
            if (MODREG) {
                GETEX(v0, 0, 1);
                u8 = (F8) & 7;
                v1 = fpu_get_scratch(dyn);
                VBSRL_V(v1, v0, (u8 << 1));
                MOVFR2GR_D(gd, v1);
                BSTRPICK_D(gd, gd, 15, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x4, &fixedaddress, rex, NULL, 1, 1);
                u8 = (F8) & 7;
                LD_HU(gd, wback, (u8 << 1));
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            VSHUF4I_D(v0, v1, 0x8 | (u8 & 1) | ((u8 & 2) << 1));
            break;
        case 0xD2:
            INST_NAME("PSRLD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            VSAT_DU(v0, q1, 31);
            VREPLVEI_W(v0, v0, 0);
            VLDI(v1, 0b1000000011111); // broadcast 31 as uint32
            VSLT_WU(v1, v1, v0);
            VMINI_WU(v0, v0, 31);
            VSRL_W(q0, q0, v0);
            VSRL_W(q0, q0, v1);
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn);
            v1 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            VREPLVEI_D(v0, q1, 0);
            VLDI(v1, 0b0110000111111); // broadcast 63 as uint64
            VMIN_DU(d0, v0, v1);
            VSLT_DU(v1, v1, v0);
            VSRL_D(q0, q0, d0);
            VSRL_D(q0, q0, v1);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADD_D(v0, v0, q0);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VMUL_H(q0, q0, q1);
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                if (v0 == v1) {
                    // clear upper bits
                    q0 = fpu_get_scratch(dyn);
                    VXOR_V(q0, q0, q0);
                    VEXTRINS_D(q0, v0, 0);
                    VOR_V(v1, q0, q0);
                } else {
                    VXOR_V(v1, v1, v1);
                    VEXTRINS_D(v1, v0, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            v0 = fpu_get_scratch(dyn);
            VMSKLTZ_B(v0, q0);
            MOVFR2GR_D(x1, v0);
            BSTRPICK_D(gd, x1, 15, 0);
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VSSUB_HU(q0, q0, q1);
            break;
        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VAND_V(v0, v0, q0);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VSADD_BU(q0, q0, q1);
            break;
        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VANDN_V(v0, v0, q0);
            break;
        case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VAVGR_BU(v0, v0, v1);
            break;
        case 0xE1:
            INST_NAME("PSRAW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn);
            VMINI_HU(v0, q1, 15);
            VREPLVEI_H(v0, v0, 0);
            VSRA_H(q0, q0, v0);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VAVGR_HU(v0, v0, v1);
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMUH_HU(v0, v0, v1);
            break;
        case 0xE5:
            INST_NAME("PMULHW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMUH_H(v0, v0, v1);
            break;
        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                VOR_V(v1, v0, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                VST(v0, ed, fixedaddress);
            }
            break;
        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VOR_V(v0, v0, q0);
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
        case 0xF4:
            INST_NAME("PMULUDQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMULWEV_D_WU(v0, v0, v1);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            VABSD_BU(q0, q0, q1);

            // 8bit -> 16bit merge
            VPICKEV_B(d0, q0, q0);
            VPICKOD_B(d1, q0, q0);
            VEXTH_HU_BU(d0, d0);
            VEXTH_HU_BU(d1, d1);
            VADD_H(q0, d0, d1);

            // 16bit to 32bit merge
            VPICKEV_H(d0, q0, q0);
            VPICKOD_H(d1, q0, q0);
            VEXTH_WU_HU(d0, d0);
            VEXTH_WU_HU(d1, d1);
            VADD_W(q0, d0, d1);

            // 32bit to 64bit merge
            VPICKEV_W(d0, q0, q0);
            VPICKOD_W(d1, q0, q0);
            VEXTH_DU_WU(d0, d0);
            VEXTH_DU_WU(d1, d1);
            VADD_D(q0, d0, d1);
            break;
        case 0xF8:
            INST_NAME("PSUBB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUB_B(v0, v0, q0);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUB_H(v0, v0, q0);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUB_W(v0, v0, q0);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUB_D(v0, v0, q0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADD_B(v0, v0, q0);
            break;
        case 0xFD:
            INST_NAME("PADDW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADD_H(v0, v0, q0);
            break;
        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADD_W(v0, v0, q0);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
