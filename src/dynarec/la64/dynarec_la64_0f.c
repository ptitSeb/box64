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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"
#include "bitutils.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

uintptr_t dynarec64_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x01:
            // TODO:, /0 is SGDT. While 0F 01 D0 is XGETBV, etc...
            nextop = F8;
            if(MODREG) {
                switch(nextop) {
                    case 0xD0:
                        INST_NAME("FAKE xgetbv");
                        nextop = F8;
                        addr = fakeed(dyn, addr, ninst, nextop);
                        SETFLAGS(X_ALL, SF_SET); // Hack to set flags in "don't care" state
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
            } else {
                switch((nextop>>3)&7) {
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr);
            STORE_XEMU_CALL();
            CALL_S(x64Syscall, -1);
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LD_W(w1, xEmu, offsetof(x64emu_t, quit));
            CBZ_NEXT(w1);
            MARK;
            LOAD_XEMU_REM();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;
        case 0x10:
            INST_NAME("MOVUPS Gx,Ex");
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
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
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
        case 0x12:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVHLPS Gx,Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                VEXTRINS_D(v0, v1, 1);
            } else {
                INST_NAME("MOVLPS Gx,Ex");
                GETGX(v0, 1);
                SMREAD();
                GETEX(q1, 0, 0);
                VEXTRINS_D(v0, q1, 0);
            }
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("MOVLPS Ex,Gx");
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VEXTRINS_D(v1, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FST_D(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx, Ex");
            nextop = F8;
            SMREAD();
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VILVL_W(v0, q0, v0);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx, Ex");
            nextop = F8;
            SMREAD();
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VILVH_W(v0, q0, v0);
            break;
        case 0x16:
            nextop = F8;
            if (MODREG) {
                INST_NAME("MOVLHPS Gx,Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
            } else {
                INST_NAME("MOVHPS Gx,Ex");
                SMREAD();
                GETGX(v0, 1);
                v1 = fpu_get_scratch(dyn);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                FLD_D(v1, ed, fixedaddress);
            }
            VILVL_D(v0, v1, v0); // v0[127:64] = v1[63:0]
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("MOVHPS Ex,Gx");
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VEXTRINS_D(v1, v0, 0x01);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                VEXTRINS_D(v1, v0, 0x01);
                FST_D(v1, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x18:
            nextop = F8;
            if ((nextop & 0xC0) == 0xC0) {
                INST_NAME("NOP (multibyte)");
            } else
                switch ((nextop >> 3) & 7) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        INST_NAME("PREFETCHh Ed");
                        FAKEED;
                        break;
                    default:
                        INST_NAME("NOP (multibyte)");
                        FAKEED;
                }
            break;
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x28:
            INST_NAME("MOVAPS Gx,Ex");
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
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
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
                INST_NAME("COMISS Gx, Ex");
            } else {
                INST_NAME("UCOMISS Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGX(d0, 0);
            GETEXSS(v0, 0, 0);
            CLEAR_FLAGS(x2);
            // if isnan(d0) || isnan(v0)
            IFX (X_ZF | X_PF | X_CF) {
                FCMP_S(fcc0, d0, v0, cUN);
                BCEQZ_MARK(fcc0);
                ORI(xFlags, xFlags, (1 << F_ZF) | (1 << F_PF) | (1 << F_CF));
                B_MARK3_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX (X_CF) {
                FCMP_S(fcc1, d0, v0, cLT);
                BCEQZ_MARK2(fcc1);
                ORI(xFlags, xFlags, 1 << F_CF);
                B_MARK3_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX (X_ZF) {
                FCMP_S(fcc2, d0, v0, cEQ);
                BCEQZ_MARK3(fcc2);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            MARK3;
            IFX (X_ALL) {
                SPILL_EFLAGS();
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
                if (la64_lbt)                                                                        \
                    BEQZ(x1, 8);                                                                     \
                else                                                                                 \
                    B##NO(x1, 8);                                                                    \
                MV(gd, ed);                                                                          \
            } else {                                                                                 \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0); \
                if (la64_lbt)                                                                        \
                    BEQZ(x1, 8);                                                                     \
                else                                                                                 \
                    B##NO(x1, 8);                                                                    \
                LDxw(gd, ed, fixedaddress);                                                          \
            }                                                                                        \
            if (!rex.w) ZEROUP(gd);

            GOCOND(0x40, "CMOV", "Gd, Ed");

        #undef GO

        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(v0);
            VFSQRT_S(v0, q0);
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            SKIPTEST(x1);
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            VFRSQRT_S(q1, q0);
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VAND_V(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VANDN_V(v0, v0, q0);
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VOR_V(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPS Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VXOR_V(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VXOR_V(q0, q0, q1);
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VFADD_S(v0, v0, q0);
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VFMUL_S(v0, v0, q0);
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            VFCVTL_D_S(q1, q0);
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VFSUB_S(v0, v0, q0);
            break;

        #define GO(GETFLAGS, NO, YES, F, I)                                                         \
            if (box64_dynarec_test == 2) { NOTEST(x1); }                                            \
            READFLAGS(F);                                                                           \
            i32_ = F32S;                                                                            \
            BARRIER(BARRIER_MAYBE);                                                                 \
            JUMP(addr + i32_, 1);                                                                   \
            if (la64_lbt) {                                                                         \
                X64_SETJ(x1, I);                                                                    \
            } else {                                                                                \
                GETFLAGS;                                                                           \
            }                                                                                       \
            if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
                /* out of the block */                                                              \
                i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
                if (la64_lbt)                                                                       \
                    BEQZ_safe(x1, i32);                                                             \
                else                                                                                \
                    B##NO##_safe(x1, i32);                                                          \
                if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
                    if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
                    jump_to_next(dyn, addr + i32_, 0, ninst, rex.is32bits);                         \
                } else {                                                                            \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
                    B(i32);                                                                         \
                }                                                                                   \
            } else {                                                                                \
                /* inside the block */                                                              \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
                if (la64_lbt)                                                                       \
                    BNEZ_safe(x1, i32);                                                             \
                else                                                                                \
                    B##YES##_safe(x1, i32);                                                         \
            }

            GOCOND(0x80, "J", "Id");

        #undef GO


        #define GO(GETFLAGS, NO, YES, F, I)                                                          \
            READFLAGS(F);                                                                            \
            if (la64_lbt) {                                                                          \
                X64_SETJ(x3, I);                                                                     \
            } else {                                                                                 \
                GETFLAGS;                                                                            \
                S##YES(x3, x1);                                                                      \
            }                                                                                        \
            nextop = F8;                                                                             \
            if (MODREG) {                                                                            \
                if (rex.rex) {                                                                       \
                    eb1 = TO_LA64((nextop & 7) + (rex.b << 3));                                      \
                    eb2 = 0;                                                                         \
                } else {                                                                             \
                    ed = (nextop & 7);                                                               \
                    eb2 = (ed >> 2) * 8;                                                             \
                    eb1 = TO_LA64(ed & 3);                                                           \
                }                                                                                    \
                BSTRINS_D(eb1, x3, eb2 + 7, eb2);                                                    \
            } else {                                                                                 \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0); \
                ST_B(x3, ed, fixedaddress);                                                          \
                SMWRITE();                                                                           \
            }

            GOCOND(0x90, "SET", "Eb");
        #undef GO

        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            MV(A1, xRAX);
            CALL_(my_cpuid, -1, 0);
            // BX and DX are not synchronized durring the call, so need to force the update
            LD_D(xRDX, xEmu, offsetof(x64emu_t, regs[_DX]));
            LD_D(xRBX, xEmu, offsetof(x64emu_t, regs[_BX]));
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAIxw(x1, gd, 5 + rex.w);        // r1 = (gd>>5)
                ALSL_D(x3, wback, x1, 2 + rex.w); // (&ed) += r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRLxw(x4, ed, x2);
            if (la64_lbt)
                X64_SET_EFLAGS(x4, X_CF);
            else
                BSTRINS_D(xFlags, x4, F_CF, F_CF);
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            if (box64_dynarec_test) {
                // avoid noise during test
                CLEAR_FLAGS(x3);
            }
            if (rex.w) {
                // 64bits imul
                UFLAG_IF {
                    if (la64_lbt) {
                        X64_MUL_D(gd, ed);
                    }
                    MULH_D(x3, gd, ed);
                    MUL_D(gd, gd, ed);
                    IFX (X_PEND) {
                        UFLAG_OP1(x3);
                        UFLAG_RES(gd);
                        UFLAG_DF(x3, d_imul64);
                    } else {
                        SET_DFNONE();
                    }
                    IFXA (X_CF | X_OF, !la64_lbt) {
                        SRAI_D(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) {
                            BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        }
                        IFX (X_OF) {
                            BSTRINS_D(xFlags, x3, F_OF, F_OF);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    if (la64_lbt) {
                        X64_MUL_W(gd, ed);
                    }
                    MUL_D(gd, gd, ed);
                    SRLI_D(x3, gd, 32);
                    SLLI_W(gd, gd, 0);
                    IFX (X_PEND) {
                        UFLAG_RES(gd);
                        UFLAG_OP1(x3);
                        UFLAG_DF(x4, d_imul32);
                    } else IFX (X_CF | X_OF) {
                        SET_DFNONE();
                    }
                    IFXA (X_CF | X_OF, !la64_lbt) {
                        SRAI_W(x4, gd, 31);
                        SUB_D(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) {
                            BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        }
                        IFX (X_OF) {
                            BSTRINS_D(xFlags, x3, F_OF, F_OF);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
                ZEROUP(gd);
            }
            break;
        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_LA64((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_LA64(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;   // L or H
                }
                BSTRPICK_D(gd, eb1, eb2 * 8 + 7, eb2 * 8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_BU(gd, ed, fixedaddress);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                BSTRPICK_D(gd, ed, 15, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_HU(gd, ed, fixedaddress);
            }
            break;
        case 0xBA:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    BSTRPICK_D(x4, ed, u8, u8);
                    if (la64_lbt)
                        X64_SET_EFLAGS(x4, X_CF);
                    else
                        BSTRINS_D(xFlags, x4, 0, 0);
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    BSTRPICK_D(x4, ed, u8, u8);
                    if (la64_lbt)
                        X64_SET_EFLAGS(x4, X_CF);
                    else
                        BSTRINS_D(xFlags, x4, 0, 0);
                    ADDI_D(x4, xZR, -1);
                    BSTRINS_D(ed, x4, u8, u8);
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK;
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    BSTRPICK_D(x4, ed, u8, u8);
                    if (la64_lbt)
                        X64_SET_EFLAGS(x4, X_CF);
                    else
                        BSTRINS_D(xFlags, x4, 0, 0);
                    BSTRINS_D(ed, xZR, u8, u8);
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            if (la64_lbt) {
                ADDI_D(x3, xZR, 1 << F_ZF);
                X64_SET_EFLAGS(x3, X_ZF);
            } else {
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            B_NEXT_nocond;
            MARK;
            // gd is undefined if ed is all zeros, don't worry.
            if (rex.w)
                CTZ_D(gd, ed);
            else
                CTZ_W(gd, ed);
            if (la64_lbt) {
                X64_SET_EFLAGS(xZR, X_ZF);
            } else {
                ADDI_D(x3, xZR, ~(1 << F_ZF));
                AND(xFlags, xFlags, x3);
            }
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            if (la64_lbt) {
                ADDI_D(x3, xZR, 1 << F_ZF);
                X64_SET_EFLAGS(x3, X_ZF);
            } else {
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            B_NEXT_nocond;
            MARK;
            if (la64_lbt) {
                X64_SET_EFLAGS(xZR, X_ZF);
            } else {
                ADDI_D(x3, xZR, ~(1 << F_ZF));
                AND(xFlags, xFlags, x3);
            }
            if (rex.w)
                CLZ_D(gd, ed);
            else
                CLZ_W(gd, ed);
            ADDI_D(x1, xZR, rex.w ? 63 : 31);
            SUB_D(gd, x1, gd);
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_LA64((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2) * 8;
                    wback = TO_LA64(wback & 3);
                }
                BSTRPICK_D(gd, wback, wb2 + 7, wb2);
                EXT_W_B(gd, gd);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_B(gd, ed, fixedaddress);
            }
            if (!rex.w) ZEROUP(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_LA64((nextop & 7) + (rex.b << 3));
                EXT_W_H(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LD_H(gd, ed, fixedaddress);
            }
            if (!rex.w) ZEROUP(gd);
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            if (v0 == v1) {
                VSHUF4I_W(v0, v0, u8);
            } else {
                q1 = fpu_get_scratch(dyn);
                VSHUF4I_W(v0, v0, u8);
                VSHUF4I_W(q1, v1, u8);
                VEXTRINS_D(v0, q1, 0x11); // v0[127:64] = q1[127:64]
            }
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("BSWAP Reg");
            gd = TO_LA64((opcode & 7) + (rex.b << 3));
            REVBxw(gd, gd);
            break;
        default:
            DEFAULT;
    }
    return addr;
}
