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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "dynarec_rv64_helper.h"

uintptr_t dynarec64_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb2;
    uint8_t eb1, eb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    switch(opcode) {

        case 0x01:
            INST_NAME("FAKE xgetbv");
            nextop = F8;
            addr = fakeed(dyn, addr, ninst, nextop);
            SETFLAGS(X_ALL, SF_SET);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL();
            CALL(native_ud, -1);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x05:
            INST_NAME("SYSCALL");
            SMEND();
            GETIP(addr);
            STORE_XEMU_CALL();
            CALL_S(x64Syscall, -1);
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LW(w1, xEmu, offsetof(x64emu_t, quit));
            CBZ_NEXT(w1);
            MARK;
            LOAD_XEMU_REM();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;

        case 0x09:
            INST_NAME("WBINVD");
            SETFLAGS(X_ALL, SF_SET);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL();
            CALL(native_ud, -1);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x0B:
            INST_NAME("UD2");
            SETFLAGS(X_ALL, SF_SET);    // Hack to set flags in "don't care" state
            GETIP(ip);
            STORE_XEMU_CALL();
            CALL(native_ud, -1);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;


        case 0x18:
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                INST_NAME("NOP (multibyte)");
            } else
            switch((nextop>>3)&7) {
                case 0:
                    DEFAULT;
                    break;
                case 1:
                    DEFAULT;
                    break;
                case 2:
                    DEFAULT;
                    break;
                case 3:
                    DEFAULT;
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

        case 0x31:
            INST_NAME("RDTSC");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            CALL(ReadTSC, xRAX);   // will return the u64 in xEAX
            SRLI(xRDX, xRAX, 32);
            ZEROUP(xRAX);   // wipe upper part
            break;


        #define GO(GETFLAGS, NO, YES, F)            \
            READFLAGS(F);                           \
            GETFLAGS;                               \
            nextop=F8;                              \
            GETGD;                                  \
            if(MODREG) {                            \
                ed = xRAX+(nextop&7)+(rex.b<<3);    \
                B##NO(x1, 8);                       \
                MV(gd, ed);                         \
            } else {                                \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x4, &fixedaddress, rex, NULL, 1, 0); \
                B##NO(x1, 8);                       \
                LDxw(gd, ed, fixedaddress);         \
                if(!rex.w) {ZEROUP(gd);}            \
            }

        GOCOND(0x40, "CMOV", "Gd, Ed");
        #undef GO

        case 0x77:
            INST_NAME("EMMS");
            // empty MMX, FPU now usable
            mmx_purgecache(dyn, ninst, 0, x1);
            /*emu->top = 0;
            emu->fpu_stack = 0;*/ //TODO: Check if something is needed here?
            break;

        #define GO(GETFLAGS, NO, YES, F)   \
            READFLAGS(F);                                               \
            i32_ = F32S;                                                \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i32_, 1);                                         \
            GETFLAGS;                                                   \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                B##NO(x1, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i32_, 0, ninst);             \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);   \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                B##YES(x1, i32);                                        \
            }

        GOCOND(0x80, "J", "Id");
        #undef GO

        #define GO(GETFLAGS, NO, YES, F)                \
            READFLAGS(F);                               \
            GETFLAGS;                                   \
            nextop=F8;                                  \
            S##YES(x3, x1);                             \
            if(MODREG) {                                \
                if(rex.rex) {                           \
                    eb1= xRAX+(nextop&7)+(rex.b<<3);    \
                    eb2 = 0;                            \
                } else {                                \
                    ed = (nextop&7);                    \
                    eb2 = (ed>>2)*8;                    \
                    eb1 = xRAX+(ed&3);                  \
                }                                       \
                if (eb2) {                              \
                    LUI(x1, 0xffffffffffff0);           \
                    ORI(x1, x1, 0xff);                  \
                    AND(eb1, eb1, x1);                  \
                } else {                                \
                    ANDI(eb1, eb1, 0xf00);              \
                }                                       \
                OR(eb1, eb1, x3);                       \
            } else {                                    \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress,rex, NULL, 1, 0); \
                SB(x3, ed, fixedaddress);               \
                SMWRITE();                              \
            }

        GOCOND(0x90, "SET", "Eb");
        #undef GO
            
        case 0xA2:
            INST_NAME("CPUID");
            MV(A1, xRAX);
            CALL_(my_cpuid, -1, 0);
            // BX and DX are not synchronized durring the call, so need to force the update
            LD(xRDX, xEmu, offsetof(x64emu_t, regs[_DX]));
            LD(xRBX, xEmu, offsetof(x64emu_t, regs[_BX]));
            break;

        case 0xAE:
            nextop = F8;
            if((nextop&0xF8)==0xE8) {
                INST_NAME("LFENCE");
                SMDMB();
            } else
            if((nextop&0xF8)==0xF0) {
                INST_NAME("MFENCE");
                SMDMB();
            } else
            if((nextop&0xF8)==0xF8) {
                INST_NAME("SFENCE");
                SMDMB();
            } else {
                switch((nextop>>3)&7) {
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization?\n");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        if(wback!=A1) {
                            MV(A1, wback);
                        }
                        CALL_(native_clflush, -1, 0);
                        break;
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    MULH(x3, gd, ed);
                    MUL(gd, gd, ed);
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul64);
                } else {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    MUL(gd, gd, ed);
                    UFLAG_RES(gd);
                    SRLI(x3, gd, 32);
                    UFLAG_OP1(x3);
                    UFLAG_DF(x3, d_imul32);
                    SEXT_W(gd, gd);
                } else {
                    MULxw(gd, gd, ed);
                }
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    eb1 = xRAX+(nextop&7)+(rex.b<<3);
                    eb2 = 0;                \
                } else {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                }
                if (eb2) {
                    SRLI(gd, eb1, 8);
                    ANDI(gd, gd, 0xff);
                } else {
                    ANDI(gd, eb1, 0xff);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LBU(gd, ed, fixedaddress);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                SLLI(gd, ed, 48);
                SRLI(gd, gd, 48);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHU(gd, ed, fixedaddress);
            }
            break;

        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    wback = xRAX+(nextop&7)+(rex.b<<3);
                    wb2 = 0;
                } else {
                    wback = (nextop&7);
                    wb2 = (wback>>2)*8;
                    wback = xRAX+(wback&3);
                }
                SLLI(gd, wback, 56-wb2);
                SRAI(gd, gd, 56);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LB(gd, ed, fixedaddress);
            }
            if(!rex.w)
                ZEROUP(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                SLLI(gd, ed, 48);
                SRLI(gd, gd, 48);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(gd, ed, fixedaddress);
            }
            if(!rex.w)
                ZEROUP(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
