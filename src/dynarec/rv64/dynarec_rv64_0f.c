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
#include "bitutils.h"

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
    uint8_t wback, wb2, gback;
    uint8_t eb1, eb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
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
            NOTEST(x1);
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


        case 0x10:
            INST_NAME("MOVUPS Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, wback, fixedaddress+0);
            LD(x4, wback, fixedaddress+8);
            SD(x3, gback, 0);
            SD(x4, gback, 8);
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, gback, 0);
            LD(x4, gback, 8);
            SD(x3, wback, fixedaddress+0);
            SD(x4, wback, fixedaddress+8);
            if(!MODREG)
                SMWRITE2();
            break;
        case 0x12:
            nextop = F8;
            if(MODREG) {
                INST_NAME("MOVHLPS Gx,Ex");
                GETGX(x1);
                GETEX(x2, 0);
                LD(x3, wback, fixedaddress+8);
                SD(x3, gback, 0);
            } else {
                INST_NAME("MOVLPS Gx,Ex");
                GETEXSD(v0, 0);
                GETGXSD_empty(v1);
                FMVD(v1, v0);
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LWU(x5, gback, 1*4);
            LWU(x3, wback, fixedaddress+0);
            LWU(x4, wback, fixedaddress+4);
            SW(x4, gback, 3*4);
            SW(x5, gback, 2*4);
            SW(x3, gback, 1*4);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LWU(x3, wback, fixedaddress+2*4);
            LWU(x4, wback, fixedaddress+3*4);
            LWU(x5, gback, 2*4);
            LWU(x6, gback, 3*4);
            SW(x5, gback, 0*4);
            SW(x3, gback, 1*4);
            SW(x6, gback, 2*4);
            SW(x4, gback, 3*4);
            break;
        case 0x16:
            nextop = F8;
            if(MODREG) {
                INST_NAME("MOVLHPS Gx,Ex");
            } else {
                INST_NAME("MOVHPS Gx,Ex");
                SMREAD();
            }
            GETGX(x1);
            GETEX(x2, 0);
            LD(x4, wback, fixedaddress+0);
            SD(x4, gback, 8);
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("MOVHPS Ex,Gx");
            GETGX(x1);
            GETEX(x2, 0);
            LD(x4, gback, 8);
            SD(x4, wback, fixedaddress+0);
            if(!MODREG)
                SMWRITE2();
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

        case 0x28:
            INST_NAME("MOVAPS Gx,Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_MV_Q(x3);
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_MV_Q2(x3);
            if(!MODREG)
                SMWRITE2();
            break;

        case 0x2B:
            INST_NAME("MOVNTPS Ex,Gx");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            LD(x3, gback, 0);
            LD(x4, gback, 8);
            SD(x3, wback, fixedaddress+0);
            SD(x4, wback, fixedaddress+8);
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISS Gx, Ex");} else {INST_NAME("UCOMISS Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE();
            nextop = F8;
            GETGXSS(d0);
            GETEXSS(v0, 0);
            CLEAR_FLAGS();
            // if isnan(d0) || isnan(v0)
            IFX(X_ZF | X_PF | X_CF) {
                FEQS(x3, d0, d0);
                FEQS(x2, v0, v0);
                AND(x2, x2, x3);
                BNE_MARK(x2, xZR);
                ORI(xFlags, xFlags, (1<<F_ZF) | (1<<F_PF) | (1<<F_CF));
                B_NEXT_nocond;
            }
            MARK;
            // else if isless(d0, v0)
            IFX(X_CF) {
                FLTS(x2, d0, v0);
                BEQ_MARK2(x2, xZR);
                ORI(xFlags, xFlags, 1<<F_CF);
                B_NEXT_nocond;
            }
            MARK2;
            // else if d0 == v0
            IFX(X_ZF) {
                FEQS(x2, d0, v0);
                CBZ_NEXT(x2);
                ORI(xFlags, xFlags, 1<<F_ZF);
            }
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
            }                                       \
            if(!rex.w) ZEROUP(gd);

        GOCOND(0x40, "CMOV", "Gd, Ed");
        #undef GO
        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            GETGD;
            GETEX(x1, 0);
            XOR(gd, gd, gd);
            for(int i=0; i<4; ++i) {
                LWU(x2, wback, fixedaddress+i*4);
                SRLI(x2, x2, 31-i);
                if (i>0) ANDI(x2, x2, 1<<i);
                OR(gd, gd, x2);
            }
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(!(MODREG && gd==(nextop&7)+(rex.b<<3))) {
                GETGX(x1);
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, AND(x3, x3, x4));
            }
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            SSE_LOOP_Q(x3, x4, NOT(x3, x3); AND(x3, x3, x4));
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            gd = ((nextop&0x38)>>3)+(rex.r<<3);
            if(!(MODREG && gd==(nextop&7)+(rex.b<<3))) {
                GETGX(x1);
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, OR(x3, x3, x4));
            }
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            //TODO: it might be possible to check if SS or SD are used and not purge them to optimize a bit
            GETGX(x1);
            if(MODREG && gd==(nextop&7)+(rex.b<<3))
            {
                // just zero dest
                SD(xZR, x1, 0);
                SD(xZR, x1, 8);
            } else {
                GETEX(x2, 0);
                SSE_LOOP_Q(x3, x4, XOR(x3, x3, x4));
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for(int i=0; i<4; ++i) {
                // GX->f[i] += EX->f[i];
                FLW(s0, wback, fixedaddress+i*4);
                FLW(s1, gback, i*4);
                FADDS(s1, s1, s0);
                FSW(s1, gback, i*4);
            }
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for(int i=0; i<4; ++i) {
                // GX->f[i] *= EX->f[i];
                FLW(s0, wback, fixedaddress+i*4);
                FLW(s1, gback, i*4);
                FMULS(s1, s1, s0);
                FSW(s1, gback, i*4);
            }
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            FLW(s0, wback, fixedaddress);
            FLW(s1, wback, fixedaddress+4);
            FCVTDS(s0, s0);
            FCVTDS(s1, s1);
            FSD(s0, gback, 0);
            FSD(s1, gback, 8);
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            for (int i=0; i<4; ++i) {
                LW(x3, wback, fixedaddress+i*4);
                FCVTSW(s0, x3, RD_RNE);
                FSW(s0, gback, i*4);
            }
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for(int i=0; i<4; ++i) {
                // GX->f[i] -= EX->f[i];
                FLW(s0, wback, fixedaddress+i*4);
                FLW(s1, gback, i*4);
                FSUBS(s1, s1, s0);
                FSW(s1, gback, i*4);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 0);
            s0 = fpu_get_scratch(dyn);
            s1 = fpu_get_scratch(dyn);
            for(int i=0; i<4; ++i) {
                // GX->f[i] /= EX->f[i];
                FLW(s0, wback, fixedaddress+i*4);
                FLW(s1, gback, i*4);
                FDIVS(s1, s1, s0);
                FSW(s1, gback, i*4);
            }
            break;
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
                B##NO##_safe(x1, i32);                                  \
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
                B##YES##_safe(x1, i32);                                 \
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
            NOTEST(x1);
            MV(A1, xRAX);
            CALL_(my_cpuid, -1, 0);
            // BX and DX are not synchronized durring the call, so need to force the update
            LD(xRDX, xEmu, offsetof(x64emu_t, regs[_DX]));
            LD(xRBX, xEmu, offsetof(x64emu_t, regs[_BX]));
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAIxw(x1, gd, 5+rex.w); // r1 = (gd>>5)
                SLLI(x1, x1, 2+rex.w);
                ADD(x3, wback, x1); //(&ed)+=r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            ANDI(x2, gd, rex.w?0x3f:0x1f);
            SRL(x4, ed, x2);
            ANDI(x4, x4, 1);
            ANDI(xFlags, xFlags, ~1);   //F_CF is 1
            OR(xFlags, xFlags, x4);
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETED(1);
            GETGD;
            u8 = F8;
            emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4, x5);
            WBACK;
            break;
        case 0xAB:
            INST_NAME("BTS Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAI(x1, gd, 5+rex.w);
                SLLI(x1, x1, 2+rex.w);
                ADD(x3, wback, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if (rex.w) {
                ANDI(x2, gd, 0x3f);
            } else {
                ANDI(x2, gd, 0x1f);
            }
            SRL(x4, ed, x2);
            ANDI(x4, x4, 1); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x3, xZR, 1);
            SLL(x3, x3, x2);
            OR(ed, ed, x3);
            if(wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETED(1);
            GETGD;
            u8 = F8;
            u8&=(rex.w?0x3f:0x1f);
            emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
            WBACK;
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
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if(MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                            if(ed!=x1) {MV(x1, ed);}
                            CALL(rex.w?((void*)fpu_fxsave64):((void*)fpu_fxsave32), -1);
                        }
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if(MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                            if(ed!=x1) {MV(x1, ed);}
                            CALL(rex.w?((void*)fpu_fxrstor64):((void*)fpu_fxrstor32), -1);
                        }
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        SW(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if(box64_sse_flushto0) {
                            // TODO: applyFlushTo0 also needs to add RISC-V support.
                        }
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        LWU(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        SW(x4, wback, fixedaddress);
                        break;
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
                } else {
                    MULxw(gd, gd, ed);
                }
                SLLI(gd, gd, 32);
                SRLI(gd, gd, 32);
            }
            break;
        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAI(x1, gd, 5+rex.w);
                SLLI(x1, x1, 2+rex.w);
                ADD(x3, wback, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if (rex.w) {
                ANDI(x2, gd, 0x3f);
            } else {
                ANDI(x2, gd, 0x1f);
            }
            SRL(x4, ed, x2);
            ANDI(x4, x4, 1); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x3, xZR, 1);
            SLL(x3, x3, x2);
            NOT(x3, x3);
            AND(ed, ed, x3);
            if(wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
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
        case 0xBA:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8&=rex.w?0x3f:0x1f;
                    SRLIxw(x3, ed, u8);
                    ANDI(x3, x3, 1); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8&=rex.w?0x3f:0x1f;
                    SRLIxw(x3, ed, u8);
                    ANDI(x3, x3, 1); // F_CF is 1
                    ANDI(xFlags, xFlags, ~1);
                    OR(xFlags, xFlags, x3);
                    if (u8 <= 10) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    if(wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = xRAX+(nextop&7)+(rex.b<<3);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                SRAI(x1, gd, 5+rex.w);
                SLLI(x1, x1, 2+rex.w);
                ADD(x3, wback, x1);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if (rex.w) {
                ANDI(x2, gd, 0x3f);
            } else {
                ANDI(x2, gd, 0x1f);
            }
            SRL(x4, ed, x2);
            ANDI(x4, x4, 1); // F_CF is 1
            ANDI(xFlags, xFlags, ~1);
            OR(xFlags, xFlags, x4);
            ADDI(x3, xZR, 1);
            SLL(x3, x3, x2);
            XOR(ed, ed, x3);
            if(wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if(!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1<<F_ZF);
            B_NEXT_nocond;
            MARK;
            NEG(x2, ed);
            AND(x2, x2, ed);
            TABLE64(x3, 0x03f79d71b4ca8b09ULL);
            MUL(x2, x2, x3);
            SRLI(x2, x2, 64-6);
            TABLE64(x1, (uintptr_t)&deBruijn64tab);
            ADD(x1, x1, x2);
            LBU(gd, x1, 0);
            ANDI(xFlags, xFlags, ~(1<<F_ZF));
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            if(!rex.w && MODREG) {
                AND(x4, ed, xMASK);
                ed = x4;
            }
            BNE_MARK(ed, xZR);
            ORI(xFlags, xFlags, 1<<F_ZF);
            B_NEXT_nocond;
            MARK;
            ANDI(xFlags, xFlags, ~(1<<F_ZF));
            if(ed!=gd)
                u8 = gd;
            else
                u8 = x1;
            ADDI(u8, xZR, 0);
            if(rex.w) {
                MV(x2, ed);
                SRLI(x3, x2, 32);
                BEQZ(x3, 4+2*4);
                ADDI(u8, u8, 32);
                MV(x2, x3);
            } else {
                AND(x2, ed, xMASK);
            }
            SRLI(x3, x2, 16);
            BEQZ(x3, 4+2*4);
            ADDI(u8, u8, 16);
            MV(x2, x3);
            SRLI(x3, x2, 8);
            BEQZ(x3, 4+2*4);
            ADDI(u8, u8, 8);
            MV(x2, x3);
            SRLI(x3, x2, 4);
            BEQZ(x3, 4+2*4);
            ADDI(u8, u8, 4);
            MV(x2, x3);
            ANDI(x2, x2, 0b1111); 
            TABLE64(x3, (uintptr_t)&lead0tab);
            ADD(x3, x3, x2);
            LBU(x2, x3, 0);
            ADD(gd, u8, x2);
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
                SRAI(gd, gd, 48);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LH(gd, ed, fixedaddress);
            }
            if(!rex.w)
                ZEROUP(gd);
            break;
        case 0xC6: // TODO: Optimize this!
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(x1);
            GETEX(x2, 1);
            u8 = F8;
            int32_t idx;

            idx = (u8>>(0*2))&3;
            LWU(x3, gback, idx*4);
            idx = (u8>>(1*2))&3;
            LWU(x4, gback, idx*4);
            idx = (u8>>(2*2))&3;
            LWU(x5, wback, fixedaddress+idx*4);
            idx = (u8>>(3*2))&3;
            LWU(x6, wback, fixedaddress+idx*4);

            SW(x3, gback, 0*4);
            SW(x4, gback, 1*4);
            SW(x5, gback, 2*4);
            SW(x6, gback, 3*4);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:                  /* BSWAP reg */
            INST_NAME("BSWAP Reg");
            gd = xRAX+(opcode&7)+(rex.b<<3);
            MOV_U12(x1, 0xff);
            SLLI(x4, x1, 8); // mask 0xff00
            if (rex.w) {
                SLLI(x5, x1, 16); // mask 0xff0000
                SLLI(x6, x1, 24); // mask 0xff000000

                SRLI(x2, gd, 56);

                SRLI(x3, gd, 40);
                AND(x3, x3, x4);
                OR(x2, x2, x3);

                SRLI(x3, gd, 24);
                AND(x3, x3, x5);
                OR(x2, x2, x3);

                SRLI(x3, gd, 8);
                AND(x3, x3, x6);
                OR(x2, x2, x3);

                AND(x3, gd, x6);
                SLLI(x3, x3, 8);
                OR(x2, x2, x3);

                AND(x3, gd, x5);
                SLLI(x3, x3, 24);
                OR(x2, x2, x3);

                AND(x3, gd, x4);
                SLLI(x3, x3, 40);
                OR(x2, x2, x3);

                SLLI(x3, x3, 56);
                OR(gd, x2, x3);
            } else {
                SRLIW(x2, gd, 24);

                SRLIW(x3, gd, 8);
                AND(x3, x3, x4);
                OR(x2, x2, x3);

                AND(x3, gd, x4);
                SLLI(x3, x3, 8);
                OR(x2, x2, x3);

                AND(x3, gd, x1);
                SLLI(x3, x3, 24);
                OR(gd, x2, x3);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
