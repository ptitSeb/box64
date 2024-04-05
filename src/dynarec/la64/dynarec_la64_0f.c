#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
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

        #define GO(GETFLAGS, NO, YES, F, I)                                                         \
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
            SRLIxw(x4, ed, x2);
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
                    MULH_D(x3, gd, ed);
                    MUL_D(gd, gd, ed);
                    IFX (X_PEND) {
                        UFLAG_OP1(x3);
                        UFLAG_RES(gd);
                        UFLAG_DF(x3, d_imul64);
                    } else {
                        SET_DFNONE();
                    }
                    IFX (X_CF | X_OF) {
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
                    IFX (X_CF | X_OF) {
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
                SLLI_D(gd, gd, 32);
                SRLI_D(gd, gd, 32);
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
        default:
            DEFAULT;
    }
    return addr;
}
