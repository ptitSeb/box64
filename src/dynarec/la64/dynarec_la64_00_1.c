#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "la64_mapping.h"
#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "../dynablock_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_00_1(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed, tmp1, tmp2, tmp3;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    uint8_t v0, v1, v2;
    int unscaled;
    int lock;
    int cacheupd = 0;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(wb);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            INST_NAME("INC Reg (32bits)");
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
            gd = TO_NAT(opcode & 7);
            MARKREGsd(gd);
            emit_inc32(dyn, ninst, rex, gd, x1, x2, x3, x4);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            INST_NAME("DEC Reg (32bits)");
            SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
            gd = TO_NAT(opcode & 7);
            MARKREGsd(gd);
            emit_dec32(dyn, ninst, rex, gd, x1, x2, x3, x4);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            INST_NAME("PUSH reg");
            SCRATCH_USAGE(0);
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            MARKREGsz(gd);
            if (!rex.is32bits) UP32_READ(xRSP);
            PUSH1mz(gd);
            SMWRITE();
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            INST_NAME("POP reg");
            SCRATCH_USAGE(0);
            SMREAD();
            gd = TO_NAT((opcode & 0x07) + (rex.b << 3));
            if (!rex.is32bits) UP32_READ(xRSP);
            MARKREGdz(gd);
            POP1mz(gd);
            break;
        case 0x60:
            if (rex.is32bits) {
                INST_NAME("PUSHAD");
                MV(x1, xRSP);
                PUSH1_32(xRAX);
                PUSH1_32(xRCX);
                PUSH1_32(xRDX);
                PUSH1_32(xRBX);
                PUSH1_32(x1);
                PUSH1_32(xRBP);
                PUSH1_32(xRSI);
                PUSH1_32(xRDI);
                SMWRITE();
            } else {
                INST_NAME("Illegal 60");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x61:
            if (rex.is32bits) {
                INST_NAME("POPAD");
                SMREAD();
                POP1_32(xRDI);
                POP1_32(xRSI);
                POP1_32(xRBP);
                POP1_32(x1);
                POP1_32(xRBX);
                POP1_32(xRDX);
                POP1_32(xRCX);
                POP1_32(xRAX);
            } else {
                INST_NAME("Illegal 61");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x62:
            if (rex.is32bits) {
                // BOUND here
                DEFAULT;
            } else {
                INST_NAME("Illegal 62");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0x63:
            if (rex.is32bits) {
                INST_NAME("ARPL Ew, Gw");
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                SET_DFNONE();
                nextop = F8;
                GETEW(x1, 0);
                GETGW(x2);
                ANDI(x6, ed, 3);
                ANDI(x4, gd, 3);
                SLTU(x5, x6, x4);
                BSTRINS_D(xFlags, x5, F_ZF, F_ZF);
                if (cpuext.lbt) X64_SET_EFLAGS(xFlags, X_ZF);
                BEQZ_MARK(x5);
                BSTRINS_D(ed, gd, 1, 0);
                EWBACK;
                MARK;
            } else {
                INST_NAME("MOVSXD Gd, Ed");
                nextop = F8;
                GETGDd;
                SCRATCH_USAGE(0);
                if (rex.w) {
                    if (MODREG) { // reg <= reg
                        GETED(0);
                        ADDI_W(gd, ed, 0);
                    } else { // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                        LD_W(gd, ed, fixedaddress);
                    }
                } else {
                    if (MODREG) { // reg <= reg
                        GETED(0);
                        ZEROUP2(gd, ed);
                    } else { // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                        LD_WU(gd, ed, fixedaddress);
                    }
                }
            }
            break;
        case 0x68:
            INST_NAME("PUSH Id");
            if (!rex.is32bits) UP32_READ(xRSP);
            i64 = F32S;
            if (PK(0) == 0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                TABLE64(x3, addr - 4);
                LD_W(x1, x3, 0);
                PUSH1mz(x1);
                SMWRITE();
            } else {
                if (!i64) {
                    PUSH1mz(xZR);
                } else {
                    MOV64z(x3, i64);
                    PUSH1mz(x3);
                }
                SMWRITE();
            }
            break;
        case 0x69:
            INST_NAME("IMUL Gd, Ed, Id");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGDd;
            GETED(4);
            i64 = F32S;
            MOV64x(x4, i64);
            CLEAR_FLAGS(x3);
            if (rex.w) {
                UFLAG_IF {
                    MULH_D(x3, ed, x4);
                    MUL_D(gd, ed, x4);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRAI_D(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MUL_D(gd, ed, x4);
                }
            } else {
                UFLAG_IF {
                    SLLI_W(x3, ed, 0);
                    MUL_D(x5, x3, x4);
                    SRLI_D(x3, x5, 32);
                    SLLI_W(gd, x5, 0);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        XOR(x3, gd, x5);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MUL_W(gd, ed, x4);
                }
                if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);
            }
            IFX (X_SF) {
                SRLI_D(x3, gd, rex.w ? 63 : 31);
                BSTRINS_D(xFlags, x3, F_SF, F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            IFX (X_ALL) SPILL_EFLAGS();
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            if (!rex.is32bits) UP32_READ(xRSP);
            i64 = F8S;
            if (!i64) {
                PUSH1mz(xZR);
            } else {
                MOV64z(x3, i64);
                PUSH1mz(x3);
            }
            SMWRITE();
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGDd;
            GETED(1);
            i64 = F8S;
            MOV64x(x4, i64);
            CLEAR_FLAGS(x3);
            if (rex.w) {
                UFLAG_IF {
                    MULH_D(x3, ed, x4);
                    MUL_D(gd, ed, x4);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRAI_D(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MUL_D(gd, ed, x4);
                }
            } else {
                UFLAG_IF {
                    SLLI_W(x3, ed, 0);
                    MUL_D(x5, x3, x4);
                    SRLI_D(x3, x5, 32);
                    SLLI_W(gd, x5, 0);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        XOR(x3, gd, x5);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                } else {
                    MUL_W(gd, ed, x4);
                }
                if (NEED_ZEROUP(gd)) ZEROUP_RESULT(gd);
            }
            IFX (X_SF) {
                SRLI_D(x3, gd, rex.w ? 63 : 31);
                BSTRINS_D(xFlags, x3, F_SF, F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            IFX (X_ALL) SPILL_EFLAGS();
            break;
        case 0x6C:
        case 0x6D:
            INST_NAME(opcode == 0x6C ? "INSB" : "INSD");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL();
            CALL(const_native_priv, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x6E:
        case 0x6F:
            INST_NAME(opcode == 0x6E ? "OUTSB" : "OUTSD");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL();
            CALL(const_native_priv, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                                 \
    COMIS_JCC(I);                                                                                  \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                       \
    i8 = F8S;                                                                                      \
    JUMP(addr + i8, 1);                                                                            \
    if (!COMIS_FUSED() && !dyn->insts[ninst].nat_flags_fusion) {                                   \
        if (cpuext.lbt) {                                                                          \
            X64_SETJ(tmp1, I);                                                                     \
        } else {                                                                                   \
            GETFLAGS;                                                                              \
        }                                                                                          \
    }                                                                                              \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                                  \
        /* out of block */                                                                         \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                       \
        if (COMIS_FUSED()) {                                                                       \
            COMIS_BRANCH_NOT_TAKEN(i32, ninst);                                                    \
        } else if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATNO, i32, ninst);                                                    \
        } else if (cpuext.lbt)                                                                     \
            BEQZ_safe(tmp1, i32, ninst);                                                           \
        else                                                                                       \
            B##NO##_safe(tmp1, i32, ninst);                                                        \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                               \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                                  \
                fpu_purgecache(dyn, ninst, 1, tmp1, tmp2, tmp3);                                   \
            jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                                  \
        } else {                                                                                   \
            CacheTransform(dyn, ninst, cacheupd, tmp1, tmp2, tmp3);                                \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].branch_address - (dyn->native_size); \
            B(i32);                                                                                \
        }                                                                                          \
    } else {                                                                                       \
        /* inside the block */                                                                     \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].branch_address - (dyn->native_size);     \
        if (COMIS_FUSED()) {                                                                       \
            COMIS_BRANCH_TAKEN(i32, dyn->insts[ninst].x64.jmp_insts);                              \
        } else if (dyn->insts[ninst].nat_flags_fusion) {                                           \
            NATIVEJUMP_safe(NATYES, i32, dyn->insts[ninst].x64.jmp_insts);                         \
        } else if (cpuext.lbt)                                                                     \
            BNEZ_safe(tmp1, i32, dyn->insts[ninst].x64.jmp_insts);                                 \
        else                                                                                       \
            B##YES##_safe(tmp1, i32, dyn->insts[ninst].x64.jmp_insts);                             \
    }

            GOCOND(0x70, "J", "ib");

#undef GO

        default:
            DEFAULT;
    }

    return addr;
}
