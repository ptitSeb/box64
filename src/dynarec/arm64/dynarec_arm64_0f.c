#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "my_cpuid.h"
#include "freq.h"
#include "emu/x87emu_private.h"
#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"
#ifndef __WIN32
#include "elfloader.h"
#endif

uintptr_t dynarec64_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0;
    int mask;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    switch(opcode) {
        case 0x00:
            if(rex.is32bits) {
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0:
                        INST_NAME("SLDT EW");
                        if(MODREG) {
                            ed = TO_NAT((nextop & 7) + (rex.b << 3));
                            MOV32w(ed, 0);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                            STRH_U12(xZR, wback, 0);
                        }
                        break;
                    default:
                        DEFAULT;
                }
            } else {
                DEFAULT;
            }
            break;
        case 0x01:
            // TODO:, /0 is SGDT. While 0F 01 D0 is XGETBV, etc...
            nextop = F8;
            if(MODREG)
            switch(nextop) {
                case 0xD0:
                    INST_NAME("XGETBV");
                    CBZw_MARK(xRCX);
                    UDF(0);
                    MARK;
                    MOV32w(xRAX, 0b111);
                    MOV32w(xRDX, 0);
                    break;
                case 0xE0:
                case 0xE1:
                case 0xE2:
                case 0xE3:
                case 0xE4:
                case 0xE5:
                case 0xE6:
                case 0xE7:
                    INST_NAME("SMSW Ed");
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    MOV32w(ed, (1<<0) | (1<<4)); // only PE and ET set...
                    break;
                case 0xF9:
                    INST_NAME("RDTSCP");
                    NOTEST(x1);
                    if(box64_rdtsc) {
                        CALL_(const_readtsc, x1, x3);
                    } else {
                        MRS_cntvct_el0(x1);
                    }
                    if(box64_rdtsc_shift) {
                        LSLx(x1, x1, box64_rdtsc_shift);
                    }
                    LSRx(xRDX, x1, 32);
                    MOVw_REG(xRAX, x1);   // wipe upper part
                    CALL_(const_helper_getcpu, x1, x3);
                    MOVw_REG(xRCX, x1);    // IA32_TSC, 0 for now
                    break;
                default:
                    DEFAULT;
            } else
                switch((nextop>>3)&7) {
                    case 0: // SGDT
                        INST_NAME("SGDT Ed");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        MOV32w(x1, 0x7f);
                        STURH_I9(x1, wback, 0);
                        if(rex.is32bits) {
                            MOV32w(x1, 0x3000);
                            STURw_I9(x1, wback, 2);
                        } else {
                            MOV64x(x1, 0xfffffe0000077000LL);
                            STURx_I9(x1, wback, 2);
                        }
                        break;
                    case 1:
                        INST_NAME("SIDT Ed");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        MOV32w(x1, 0xfff);
                        STURH_I9(x1, wback, 0);
                        if(rex.is32bits) {
                            STURw_I9(xZR, wback, 2);
                        } else {
                            MOV64x(x1, 0xfffffe0000000000LL);
                            STURx_I9(x1, wback, 2);
                        }
                        break;
                    case 4:
                        INST_NAME("SMSW Ew");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        // dummy for now... Do I need to track CR0 state?
                        MOV32w(x1, (1<<0) | (1<<4)); // only PE and ET set...
                        STURH_I9(x1, wback, 0);
                        break;
                    default:
                        DEFAULT;
            }
            break;

        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr);
            STORE_XEMU_CALL(xRIP);
            #ifndef __WIN32
            if(!box64_wine || FindElfAddress(my_context, ip)) {
                CALL_S(const_x64syscall_linux, -1);
            } else
            #endif
            {
                CALL_S(const_x64syscall, -1);
            }
            LOAD_XEMU_CALL(xRIP);
            TABLE64(x3, addr); // expected return address
            SUBx_REG(x3, x3, xRIP);
            CBNZx_MARK(x3);
            LDRw_U12(w1, xEmu, offsetof(x64emu_t, quit));
            CBZw_NEXT(w1);
            MARK;
            LOAD_XEMU_REM();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;

        case 0x09:
            INST_NAME("WBINVD");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            BARRIER(BARRIER_FLOAT);
            GETIP(ip);
            UDF(0);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x0B:
            INST_NAME("UD2");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            BARRIER(BARRIER_FLOAT);
            GETIP(ip);
            UDF(0);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x0D:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("PREFETCH");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PLD_L1_STREAM_U12(ed, fixedaddress);
                    break;
                case 1:
                    INST_NAME("PREFETCHW");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PST_L1_STREAM_U12(ed, fixedaddress);
                    break;
                case 2:
                    INST_NAME("PREFETCHWT1");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PST_L1_STREAM_U12(ed, fixedaddress);
                    break;
                default:    //NOP
                    FAKEED;
                    break;
            }
            break;
        case 0x0E:
            INST_NAME("femms");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL_S(const_native_ud, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x10:
            INST_NAME("MOVUPS Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);   // no alignment issue with ARMv8 NEON :)
            }
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
            } else {
                IF_UNALIGNED(ip) {
                    MESSAGE(LOG_DEBUG, "\tUnaligned path");
                    addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    if(wback!=x1) {
                        MOVx_REG(x1, wback);
                        wback = x1;
                    }
                    for(int i=0; i<16; ++i) {
                        VST1_8(v0, i, wback);
                        ADDx_U12(wback, wback, 1);
                    }
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                    VST128(v0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x12:
            nextop = F8;
            if(MODREG) {
                INST_NAME("MOVHLPS Gx, Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                VMOVeD(v0, 0, v1, 1);
            } else {
                INST_NAME("MOVLPS Gx, Ex");
                GETGX(v0, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(v0, 0, ed);
            }
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("MOVLPS Ex, Gx");
            GETGX(v0, 0);
            if(MODREG) {
                DEFAULT;
                return addr;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VST1_64(v0, 0, ed);  // better to use VST1 than VSTR_64, to avoid NEON->VFPU transfert I assume
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VZIP1Q_32(v0, v0, q0);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VZIP2Q_32(v0, v0, q0);
            break;
        case 0x16:
            nextop = F8;
            if(MODREG) {
                INST_NAME("MOVLHPS Gx, Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                VMOVeD(v0, 1, v1, 0);
            } else {
                INST_NAME("MOVHPS Gx, Ex");
                SMREAD();
                GETGX(v0, 1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(v0, 1, ed);
            }
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("MOVHPS Ex, Gx");
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1);
                VMOVeD(v1, 0, v0, 1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VST1_64(v0, 1, ed);
                SMWRITE2();
            }
            break;
        case 0x18:
            nextop = F8;
            if (MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("PREFETCHh Ed");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PLD_L1_STREAM_U12(ed, fixedaddress);
                    break;
                case 1:
                    INST_NAME("PREFETCHh Ed");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PLD_L1_KEEP_U12(ed, fixedaddress);
                    break;
                case 2:
                    INST_NAME("PREFETCHh Ed");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PLD_L2_KEEP_U12(ed, fixedaddress);
                    break;
                case 3:
                    INST_NAME("PREFETCHh Ed");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xfff, 7, rex, NULL, 0, 0);
                    PLD_L3_KEEP_U12(ed, fixedaddress);
                    break;
                default:
                    INST_NAME("NOP (multibyte)");
                    FAKEED;
                }
            break;

        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x20:                      /* MOV REG, crX */
        case 0x21:                      /* MOV REG, drX */
        case 0x22:                      /* MOV cxR, REG */
        case 0x23:                      /* MOV drX, REG */
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            nextop = F8;
            GETIP(ip);
            u8 = (rex.r*8)+(nextop>>3&7);
            if((((opcode==0x20) || (opcode==0x22)) && ((u8==1) || (u8==5) || (u8==6) || (u8==7) || (u8>8))) || (((opcode==0x21) || (opcode==0x23) && rex.r))) {
                INST_NAME("Illegal 0F 20..23");
                UDF(0);
            } else {
                INST_NAME(opcode==0x20?"MOV REG, crX":(opcode==0x21?"MOV REG, drX":(opcode==0x22?"MOV cxR, REG":"MOV dxR, REG")));
                STORE_XEMU_CALL(xRIP);
                CALL_S(const_native_priv, -1);
                LOAD_XEMU_CALL(xRIP);
            }
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0x28:
            INST_NAME("MOVAPS Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x2A:
            INST_NAME("CVTPI2PS Gx,Em");
            nextop = F8;
            GETGX(v0, 1);
            GETEM(q1, 0);
            d0 = fpu_get_scratch(dyn, ninst);
            u8 = sse_setround(dyn, ninst, x1, x2, x3);
            SCVTFS(d0, q1);
            x87_restoreround(dyn, ninst, u8);
            VMOVeD(v0, 0, d0, 0);
            break;
        case 0x2B:
            INST_NAME("MOVNTPS Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                DEFAULT;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTPS2PI Gm,Ex");
            nextop = F8;
            GETGM(q0);
            GETEXSD(v1, 0, 0);
            if (BOX64ENV(dynarec_fastround)) {
                VFCVTZSS(q0, v1);
            } else {
                if(cpuext.frintts) {
                    VFRINT32ZS(q0, v1);
                    VFCVTZSS(q0, q0);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    ORRw_mask(x2, xZR, 1, 0);    //0x80000000
                    d0 = fpu_get_scratch(dyn, ninst);
                    for (int i=0; i<2; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        if (i) {
                            VMOVeS(d0, 0, v1, i);
                            FRINTZS(d0, d0);
                        } else {
                            FRINTZS(d0, v1);
                        }
                        FCVTZSwS(x1, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        MOVw_REG(x1, x2);
                        VMOVQSfrom(q0, i, x1);
                    }
                }
            }
            break;
        case 0x2D:
            INST_NAME("CVTPS2PI Gm, Ex");
            nextop = F8;
            GETGM(q0);
            GETEXSD(v1, 0, 0);
            if (BOX64ENV(dynarec_fastround)) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                VFRINTIS(q0, v1);
                x87_restoreround(dyn, ninst, u8);
                VFCVTZSS(q0, q0);
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                if(cpuext.frintts) {
                    VFRINT32XS(q0, v1);
                    VFCVTZSS(q0, q0);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    ORRw_mask(x2, xZR, 1, 0);    //0x80000000
                    d0 = fpu_get_scratch(dyn, ninst);
                    for (int i=0; i<2; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        if (i) {
                            VMOVeS(d0, 0, v1, i);
                            FRINTIS(d0, d0);
                        } else {
                            FRINTIS(d0, v1);
                        }
                        FCVTZSwS(x1, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        MOVw_REG(x1, x2);
                        VMOVQSfrom(q0, i, x1);
                    }
                }
                x87_restoreround(dyn, ninst, u8);
            }
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISS Gx, Ex");} else {INST_NAME("UCOMISS Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET_NODF);
            nextop = F8;
            GETGX(v0, 0);
            GETEXSS(s0, 0, 0);
            IFX(X_CF|X_PF|X_ZF) {
                FCMPS(v0, s0);
            }
            FCOMI(x1, x2);
            break;
        case 0x30:
            INST_NAME("WRMSR");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            }
            GETIP(ip);
            STORE_XEMU_CALL(xRIP);
            CALL_S(const_native_priv, -1);
            LOAD_XEMU_CALL(xRIP);
            jump_to_epilog(dyn, 0, xRIP, ninst);
            // continue the block
            break;
        case 0x31:
            INST_NAME("RDTSC");
            NOTEST(x1);
            if(box64_rdtsc) {
                CALL_(const_readtsc, x1, x3);
            } else {
                MRS_cntvct_el0(x1);
            }
            if(box64_rdtsc_shift) {
                LSLx(x1, x1, box64_rdtsc_shift);
            }
            LSRx(xRDX, x1, 32);
            MOVw_REG(xRAX, x1);   // wipe upper part
            break;

        case 0x38:
            //SSE3
            nextop=F8;
            switch(nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    d0 = fpu_get_scratch(dyn, ninst);
                    MOVI_8(d0, 0b10000111);
                    VAND(d0, d0, q1);  // mask the index
                    VTBL1_8(q0, q0, d0);
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    VADDP_16(q0, q0, q1);
                    break;
                case 0x2:
                    INST_NAME("PHADDD Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    VADDP_32(q0, q0, q1);
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gm, Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP1_16(v0, q0, q1);
                    VUZP2_16(q0, q0, q1);
                    SQADD_16(q0, q0, v0);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    v1 = fpu_get_scratch(dyn, ninst);
                    UXTL_8(v0, q0);   // this is unsigned, so 0 extended
                    SXTL_8(v1, q1);   // this is signed
                    VMULQ_16(v0, v0, v1);
                    SADDLPQ_16(v1, v0);
                    SQXTN_16(q0, v1);
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP1_16(v0, q0, q1);
                    VUZP2_16(q0, q0, q1);
                    VSUB_16(q0, v0, q0);
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP1_32(v0, q0, q1);
                    VUZP2_32(q0, q0, q1);
                    VSUB_32(q0, v0, q0);
                    break;
                case 0x07:
                    INST_NAME("PHSUBSW Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP1_16(v0, q0, q1);
                    VUZP2_16(q0, q0, q1);
                    SQSUB_16(q0, v0, q0);
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    v1 = fpu_get_scratch(dyn, ninst);
                    NEG_8(v0, q0);     // get NEG
                    CMLT_0_8(v1, q1);  // calculate mask
                    VBIF(v0, q0, v1);  // put back positive values
                    CMEQ_0_8(v1, q1);  // handle case where Ex is 0
                    VBIC(q0, v0, v1);
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    v1 = fpu_get_scratch(dyn, ninst);
                    NEG_16(v0, q0);     // get NEG
                    CMLT_0_16(v1, q1);  // calculate mask
                    VBIF(v0, q0, v1);   // put back positive values
                    CMEQ_0_16(v1, q1);  // handle case where Ex is 0
                    VBIC(q0, v0, v1);
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    v1 = fpu_get_scratch(dyn, ninst);
                    NEG_32(v0, q0);     // get NEG
                    CMLT_0_32(v1, q1);  // calculate mask
                    VBIF(v0, q0, v1);   // put back positive values
                    CMEQ_0_32(v1, q1);  // handle case where Ex is 0
                    VBIC(q0, v0, v1);
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VSMULL_16(v0, q0, q1);
                    SRSHRQ_32(v0, v0, 15);
                    XTN_16(q0, v0);
                    break;
                case 0x1C:
                    INST_NAME("PABSB Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    ABS_8(q0, q1);
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    ABS_16(q0, q1);
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gm,Em");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 0);
                    ABS_32(q0, q1);
                    break;

                case 0xC8:
                    INST_NAME("SHA1NEXTE Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VEORQ(v0, v0, v0);
                    if(cpuext.sha1) {
                        v1 = fpu_get_scratch(dyn, ninst);
                        VMOVeS(v1, 0, q0, 3);
                        SHA1H(v1, v1);
                        VMOVeS(v0, 3, v1, 0);
                    } else {
                        VMOVSto(x1, q0, 3);
                        RORw(x1, x1, 2);    // i.e. ROL 30
                        VMOVQSfrom(v0, 3, x1);
                    }
                    VADDQ_32(q0, v0, q1);
                    break;
                case 0xC9:
                    INST_NAME("SHA1MSG1 Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VEXTQ_8(v0, q1, q0, 8);
                    VEORQ(q0, q0, v0);
                    break;
                case 0xCA:
                    INST_NAME("SHA1MSG2 Gx, Ex");
                    nextop = F8;
                    if(cpuext.sha1) {
                        GETGX(q0, 1);
                        GETEX(q1, 0, 0);
                        VEXTQ_8(q0, q0, q0, 8);
                        VREV64Q_32(q0, q0);
                        if(MODREG) {
                            if(q0==q1)
                                v0 = q0;
                            else {
                                v0 = fpu_get_scratch(dyn, ninst);
                                VEXTQ_8(v0, q1, q1, 8);
                                VREV64Q_32(v0, v0);
                            }
                        } else {
                            v0 = q1;
                            VEXTQ_8(v0, q1, q1, 8);
                            VREV64Q_32(v0, v0);
                        }
                        SHA1SU1(q0, v0);
                        VEXTQ_8(q0, q0, q0, 8);
                        VREV64Q_32(q0, q0);
                    } else {
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_reflect_reg(dyn, ninst, ed);
                            ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        } else {
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                            if(wback!=x2) {
                                MOVx_REG(x2, wback);
                            }
                        }
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                        CALL(const_sha1msg2, -1);
                    }
                    break;
                case 0xCB:
                    INST_NAME("SHA256RNDS2 Gx, Ex (, XMM0)");
                    nextop = F8;
                    if(cpuext.sha2) {
                        GETGX(q0, 1);
                        GETEX(q1, 0, 0);
                        d0 = sse_get_reg(dyn, ninst, x1, 0, 0);
                        v0 = fpu_get_scratch(dyn, ninst);
                        d1 = fpu_get_scratch(dyn, ninst);
                        if(MODREG) {
                            v1 = fpu_get_scratch(dyn, ninst);
                        } else
                            v1 = q1;
                        VREV64Q_32(q0, q0);
                        if(q0==q1) {
                            VMOVQ(v1, q0);
                        } else {
                            VREV64Q_32(v1, q1);
                        }
                        VZIP1Q_64(v0, v1, q0);
                        VZIP2Q_64(v1, v1, q0);
                        SHA256H(v1, v0, d0);
                        VREV64Q_32(d1, q1);
                        VZIP2Q_64(d1, d1, q0);
                        SHA256H2(v0, d1, d0);
                        VZIP2Q_64(q0, v0, v1);
                        VREV64Q_32(q0, q0);
                    } else {
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_reflect_reg(dyn, ninst, ed);
                            ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        } else {
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                            if(wback!=x2) {
                                MOVx_REG(x2, wback);
                            }
                        }
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                        sse_reflect_reg(dyn, ninst, 0);
                        CALL(const_sha256rnds2, -1);
                    }
                    break;
                case 0xCC:
                    INST_NAME("SHA256MSG1 Gx, Ex");
                    nextop = F8;
                    if(cpuext.sha2) {
                        GETGX(q0, 1);
                        GETEX(q1, 0, 0);
                        SHA256SU0(q0, q1);
                    } else {
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_reflect_reg(dyn, ninst, ed);
                            ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        } else {
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                            if(wback!=x2) {
                                MOVx_REG(x2, wback);
                            }
                        }
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                        CALL(const_sha256msg1, -1);
                    }
                    break;
                case 0xCD:
                    INST_NAME("SHA256MSG2 Gx, Ex");
                    nextop = F8;
                    if(cpuext.sha2) {
                        GETGX(q0, 1);
                        GETEX(q1, 0, 0);
                        v0 = fpu_get_scratch(dyn, ninst);
                        v1 = fpu_get_scratch(dyn, ninst);
                        d0 = fpu_get_scratch(dyn, ninst);
                        VEORQ(v1, v1, v1);
                        VMOVQ(v0, q0);
                        SHA256SU1(v0, v1, q1);  // low v0 are ok and also need to be feed again SHA256SU1 to get the high part
                        VTRNQ1_64(d0, v0, v0);  // duplicate low to hi
                        VEXTQ_8(q0, q0, q0, 8); // invert low/high now
                        SHA256SU1(q0, v1, d0);  // low is destination high
                        VEXTQ_8(q0, d0, q0, 8);
                    } else {
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_reflect_reg(dyn, ninst, ed);
                            ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        } else {
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                            if(wback!=x2) {
                                MOVx_REG(x2, wback);
                            }
                        }
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                        CALL(const_sha256msg2, -1);
                    }
                    break;

                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop=F8;
                    GETGD;
                    if(MODREG) {
                        REVxw(gd, TO_NAT((nextop & 7) + (rex.b << 3)));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                        LDxw(gd, ed, fixedaddress);
                        REVxw(gd, gd);
                    }
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop=F8;
                    GETGD;
                    if(MODREG) {   // reg <= reg
                        REVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
                    } else {                    // mem <= reg
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                        REVxw(x1, gd);
                        STxw(x1, ed, fixedaddress);
                        SMWRITE2();
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x3A:  // these are some more SSSE3 opcodes
            opcode = F8;
            switch(opcode) {
                case 0x0F:
                    INST_NAME("PALIGNR Gm, Em, Ib");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1, 1);
                    u8 = F8;
                    if(u8>15) {
                        VEOR(q0, q0, q0);
                    } else if(u8>7) {
                        d0 = fpu_get_scratch(dyn, ninst);
                        VEOR(d0, d0, d0);
                        VEXT_8(q0, q0, d0, u8-8);
                    } else {
                        VEXT_8(q0, q1, q0, u8);
                    }
                    break;

                case 0xCC:
                    INST_NAME("SHA1RNDS4 Gx, Ex, Ib");
                    nextop = F8;
                    if(cpuext.sha1) {
                        GETGX(q0, 1);
                        GETEX(q1, 0, 1);
                        u8 = F8&3;
                        d0 = fpu_get_scratch(dyn, ninst);
                        d1 = fpu_get_scratch(dyn, ninst);
                        v0 = fpu_get_scratch(dyn, ninst);
                        VEXTQ_8(v0, q0, q0, 8);
                        VREV64Q_32(v0, v0);
                        VEORQ(d1, d1, d1);
                        if(MODREG) {
                            v1 = fpu_get_scratch(dyn, ninst);
                        } else
                            v1 = q1;
                        if(v1!=v0) {
                            VEXTQ_8(v1, q1, q1, 8);
                            VREV64Q_32(v1, v1);
                        }
                        switch(u8) {
                            case 0:
                                MOV32w(x1, 0x5A827999);
                                VDUPQS(d0, x1);
                                VADDQ_32(v1, v1, d0);
                                SHA1C(v0, d1, v1);
                                break;
                            case 1:
                                MOV32w(x1, 0x6ED9EBA1);
                                VDUPQS(d0, x1);
                                VADDQ_32(v1, v1, d0);
                                SHA1P(v0, d1, v1);
                                break;
                            case 2:
                                MOV32w(x1, 0X8F1BBCDC);
                                VDUPQS(d0, x1);
                                VADDQ_32(v1, v1, d0);
                                SHA1M(v0, d1, v1);
                                break;
                            case 3:
                                MOV32w(x1, 0xCA62C1D6);
                                VDUPQS(d0, x1);
                                VADDQ_32(v1, v1, d0);
                                SHA1P(v0, d1, v1);
                                break;
                        }
                        VREV64Q_32(v0, v0);
                        VEXTQ_8(q0, v0, v0, 8);
                        break;
                    } else {
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_reflect_reg(dyn, ninst, ed);
                            ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[ed]));
                        } else {
                            SMREAD();
                            addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                            if(wback!=x2) {
                                MOVx_REG(x2, wback);
                            }
                        }
                        u8 = F8;
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[gd]));
                        MOV32w(x3, u8);
                        CALL(const_sha1rnds4, -1);
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)                                                                                                                  \
            READFLAGS(F);                                                                                                                                 \
            GETFLAGS;                                                                                                                                     \
            nextop = F8;                                                                                                                                  \
            GETGD;                                                                                                                                        \
            if (MODREG) {                                                                                                                                 \
                ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                                                 \
                CSELxw(gd, ed, gd, YES);                                                                                                                  \
            } else {                                                                                                                                      \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff << (2 + rex.w), (1 << (2 + rex.w)) - 1, rex, NULL, 0, 0); \
                Bcond(NO, +8);                                                                                                                            \
                LDxw(gd, ed, fixedaddress);                                                                                                               \
                if (!rex.w) { MOVw_REG(gd, gd); }                                                                                                         \
            }

        GOCOND(0x40, "CMOV", "Gd, Ed");
        #undef GO
        case 0x50:
            INST_NAME("MOVMSKPS Gd, Ex");
            nextop = F8;
            GETGD;
            MOV32w(gd, 0);
            if(MODREG) {
                // EX is an xmm reg
                q0 = fpu_get_scratch(dyn, ninst);
                GETEX(v0, 0, 0);
                SQXTN_16(q0, v0);   // reduces the 4 32bits to 4 16bits
                VMOVQDto(x1, q0, 0);
                LSRx(x1, x1, 15);
                BFIx(gd, x1, 0, 1);
                LSRx(x1, x1, 16);
                BFIx(gd, x1, 1, 1);
                LSRx(x1, x1, 16);
                BFIx(gd, x1, 2, 1);
                LSRx(x1, x1, 16);
                BFIx(gd, x1, 3, 1);
            } else {
                // EX is memory
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0x3f<<3, 7, rex, NULL, 1, 0);
                LDPx_S7_offset(x1, x2, ed, fixedaddress);
                LSRx(x1, x1, 31);
                BFIx(gd, x1, 0, 1);
                LSRx(x1, x1, 32);
                BFIx(gd, x1, 1, 1);
                LSRx(x1, x2, 31);
                BFIx(gd, x1, 2, 1);
                LSRx(x1, x1, 32);
                BFIx(gd, x1, 3, 1);
            }
            break;
        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFCMEQQS(v0, q0, q0);    // 0 if NAN, 1 if not NAN
                VFSQRTQS(q1, q0);
                VFCMEQQS(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_32(v1, v1, 31);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            } else {
                VFSQRTQS(q1, q0);
            }
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            v0 = fpu_get_scratch(dyn, ninst);
            if(!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn, ninst);
                d1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFCMEQQS(d0, q0, q0);    // 0 if NAN, 1 if not NAN
            }
            VFMOVSQ_8(v0, 0b01110000);    //1.0f
            VFSQRTQS(q1, q0);
            VFDIVQS(q1, v0, q1);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(d1, q1, q1);    // 0 => out is NAN
                VBICQ(d1, d0, d1);      // forget it in any input was a NAN already
                VSHLQ_32(d1, d1, 31);   // only keep the sign bit
                VORRQ(q1, q1, d1);      // NAN -> -NAN
            }
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            #if 0
            SKIPTEST(x1);
            if(q0 == q1)
                v1 = fpu_get_scratch(dyn, ninst);
            else
                v1 = q1;
            v0 = fpu_get_scratch(dyn, ninst);
            VFRECPEQS(v0, q0);
            VFRECPSQS(v1, v0, q0);
            VFMULQS(q1, v0, v1);
            #else
            v0 = fpu_get_scratch(dyn, ninst);
            VFMOVSQ_8(v0, 0b01110000);    //1.0f
            VFDIVQS(q1, v0, q0);
            #endif
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VANDQ(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VBICQ(v0, q0, v0);
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VORRQ(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG && ((nextop&7)+(rex.b<<3)==gd)) {
                // special case for XORPS Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VEORQ(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VEORQ(q0, q0, q1);
            }
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQS(v0, q0, q1);    // propagate NAN
                VFCMEQQS(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFADDQS(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_32(v1, v1, 31);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQS(v0, q0, q1);    // propagate NAN
                VFCMEQQS(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFMULQS(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_32(v1, v1, 31);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETEXSD(q0, 0, 0);
            GETGX_empty(q1);
            FCVTL(q1, q0);
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            SCVTQFS(q1, q0);
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQS(v0, q0, q1);    // propagate NAN
                VFCMEQQS(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFSUBQS(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_32(v1, v1, 31);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // FMIN/FMAX wll not copy the value if v0[x] is NaN
            // but x86 will copy if either v0[x] or v1[x] is NaN, so lets force a copy if source is NaN
            q0 = fpu_get_scratch(dyn, ninst);
            VFCMGTQS(q0, v1, v0);   // 0 is NaN or v1 GT v0, so invert mask for copy
            VBIFQ(v0, v1, q0);
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQS(v0, q0, q1);    // propagate NAN
                VFCMEQQS(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFDIVQS(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_32(v1, v1, 31);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // FMIN/FMAX wll not copy the value if v0[x] is NaN
            // but x86 will copy if either v0[x] or v1[x] is NaN, or if values are equals, so lets force a copy if source is NaN
            q0 = fpu_get_scratch(dyn, ninst);
            VFCMGTQS(q0, v0, v1);   // 0 is NaN or v0 GT v1, so invert mask for copy
            VBIFQ(v0, v1, q0);
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VZIP1_8(d0, d0, d1);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VZIP1_16(d0, d0, d1);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VZIP1_32(d0, d0, d1);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            VMOVeD(q0, 0, d0, 0);
            VMOVeD(q0, 1, d1, 0);
            SQXTN_8(d0, q0);
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMGT_8(v0, v0, v1);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMGT_16(v0, v0, v1);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMGT_32(v0, v0, v1);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gm, Em");
            nextop = F8;
            GETGM(v0);
            q0 = fpu_get_scratch(dyn, ninst);
            VMOVeD(q0, 0, v0, 0);
            if(MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, (nextop&7));
                VMOVeD(q0, 1, v1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(q0, 1, ed);
            }
            SQXTUN_8(v0, q0);
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gm,Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VZIP2_8(q0, q0, q1);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gm,Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VZIP2_16(q0, q0, q1);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gm,Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VZIP2_32(q0, q0, q1);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gm,Em");
            nextop = F8;
            GETGM(v0);
            q0 = fpu_get_scratch(dyn, ninst);
            VMOVeD(q0, 0, v0, 0);
            if(MODREG) {
                GETEM(v1, 0);
                VMOVeD(q0, 1, v1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(q0, 1, ed);
            }
            SQXTN_16(v0, q0);
            break;

        case 0x6E:
            INST_NAME("MOVD Gm, Ed");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            if(MODREG) {
                ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                if(rex.w) {
                    FMOVDx(v0, ed);
                } else {
                    VEOR(v0, v0, v0);
                    FMOVSw(v0, ed);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                if(rex.w) {
                    VLD64(v0, ed, fixedaddress);
                } else {
                    VLD32(v0, ed, fixedaddress);
                }
            }
            break;
        case 0x6F:
            INST_NAME("MOVQ Gm, Em");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7); // no rex.b on MMX
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                VMOVeD(v0, 0, v1, 0);
            } else {
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v0, ed, fixedaddress);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFW Gm,Em,Ib");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                u8 = F8;
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, (nextop&7));
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                if(u8==0x4E) {
                    if(v0==v1) {
                        VEXT_8(v0, v0, v0, 4); // Swap Up/Lower 32bits parts
                    } else {
                        VMOVeS(v0, 0, v1, 1);
                        VMOVeS(v0, 1, v1, 0);
                    }
                } else if(u8==0x00) {
                    // duplicate lower 16bits to all spot
                    VDUP_16(v0, v1, 0);
                } else if(u8==0x55) {
                    // duplicate 16bits slot 1 to all spot
                    VDUP_16(v0, v1, 1);
                } else if(u8==0xAA) {
                    // duplicate 16bits slot 2 to all spot
                    VDUP_16(v0, v1, 2);
                } else if(u8==0xFF) {
                    // duplicate 16bits slot 3 to all spot
                    VDUP_16(v0, v1, 3);
                } else if(v0!=v1) {
                    VMOVeH(v0, 0, v1, (u8>>(0*2))&3);
                    VMOVeH(v0, 1, v1, (u8>>(1*2))&3);
                    VMOVeH(v0, 2, v1, (u8>>(2*2))&3);
                    VMOVeH(v0, 3, v1, (u8>>(3*2))&3);
                } else {
                    uint64_t swp[4] = {
                        (0)|(1<<8),
                        (2)|(3<<8),
                        (4)|(5<<8),
                        (6)|(7<<8)
                    };
                    d0 = fpu_get_scratch(dyn, ninst);
                    tmp64u = swp[(u8>>(0*2))&3] | (swp[(u8>>(1*2))&3]<<16);
                    tmp64u |= (swp[(u8>>(2*2))&3]<<32) | (swp[(u8>>(3*2))&3]<<48);
                    MOV64x(x2, tmp64u);
                    VMOVQDfrom(d0, 0, x2);
                    VTBL1_8(v0, v1, d0);
                }
            } else {
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = F8;
                if (u8) {
                    i32 = -1;
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            ADDx_U12(x2, ed, idx*2);
                            i32 = idx;
                        }
                        VLD1_16(v0, i, x2);
                    }
                } else {
                    VLD1R_16(v0, ed);
                }
            }
            break;
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLW Em, Ib");
                    GETEM(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEOR(q0, q0, q0);
                        } else if(u8) {
                            VSHR_16(q0, q0, u8);
                        }
                        PUTEM(q0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Em, Ib");
                    GETEM(q0, 1);
                    u8 = F8;
                    if(u8>15) u8=15;
                    if(u8) {
                        VSSHR_16(q0, q0, u8);
                    }
                    PUTEM(q0);
                    break;
                case 6:
                    INST_NAME("PSLLW Em, Ib");
                    GETEM(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEOR(q0, q0, q0);
                        } else {
                            VSHL_16(q0, q0, u8);
                        }
                        PUTEM(q0);
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLD Em, Ib");
                    GETEM(d0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEOR(d0, d0, d0);
                        } else if(u8) {
                            VSHR_32(d0, d0, u8);
                        }
                        PUTEM(d0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Em, Ib");
                    GETEM(d0, 1);
                    u8 = F8;
                    if(u8>31) u8=31;
                    if(u8) {
                        VSSHR_32(d0, d0, u8);
                    }
                    PUTEM(d0);
                    break;
                case 6:
                    INST_NAME("PSLLD Em, Ib");
                    GETEM(d0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEOR(d0, d0, d0);
                        } else {
                            VSHL_32(d0, d0, u8);
                        }
                        PUTEM(d0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLQ Em, Ib");
                    GETEM(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEOR(q0, q0, q0);
                        } else if(u8) {
                            USHR_64(q0, q0, u8);
                        }
                        PUTEM(q0);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Em, Ib");
                    GETEM(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEOR(q0, q0, q0);
                        } else {
                            SHL_64(q0, q0, u8);
                        }
                        PUTEM(q0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            VCMEQ_8(d0, d0, d1);
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            VCMEQ_16(v0, v0, q0);
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VCMEQ_32(v0, v0, v1);
            break;
        case 0x77:
            INST_NAME("EMMS");
            // empty MMX, FPU now usable
            mmx_purgecache(dyn, ninst, 0, x1);
            /*emu->top = 0;
            emu->fpu_stack = 0;*/ //TODO: Check if something is needed here?
            break;

        case 0x7E:
            INST_NAME("MOVD Ed, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                if(rex.w) {
                    FMOVxD(ed, v0);
                } else {
                    FMOVwS(ed, v0);
                    MOVxw_REG(ed, ed);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                if(rex.w) {
                    VST64(v0, ed, fixedaddress);
                } else {
                    VST32(v0, ed, fixedaddress);
                }
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if(MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop&7);
                VMOV(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)   \
            READFLAGS(F);                                               \
            i32_ = F32S;                                                \
            if(rex.is32bits)                                            \
                j64 = (uint32_t)(addr+i32_);                            \
            else                                                        \
                j64 = addr+i32_;                                        \
            JUMP(j64, 1);                                               \
            GETFLAGS;                                                   \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, j64, 0, ninst, rex.is32bits);     \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd);               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                    SKIP_SEVL(i32);                                     \
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                SKIP_SEVL(i32);                                         \
                Bcond(YES, i32);                                        \
            }

        GOCOND(0x80, "J", "Id");
        #undef GO

        #define GO(GETFLAGS, NO, YES, F)                                                                              \
            READFLAGS(F);                                                                                             \
            GETFLAGS;                                                                                                 \
            nextop = F8;                                                                                              \
            CSETw(x3, YES);                                                                                           \
            if (MODREG) {                                                                                             \
                if (rex.rex) {                                                                                        \
                eb1 = TO_NAT((nextop & 7) + (rex.b << 3));                                                            \
                eb2 = 0;                                                                                              \
                } else {                                                                                              \
                ed = (nextop & 7);                                                                                    \
                eb2 = (ed >> 2) * 8;                                                                                  \
                eb1 = TO_NAT (ed & 3);                                                                                \
                }                                                                                                     \
                BFIx(eb1, x3, eb2, 8);                                                                                \
            } else {                                                                                                  \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 0); \
                STB(x3, ed, fixedaddress);                                                                            \
                SMWRITE();                                                                                            \
            }

        GOCOND(0x90, "SET", "Eb");
        #undef GO
        case 0xA0:
            INST_NAME("PUSH FS");
            LDRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_FS]));
            PUSH1z(x2);
            break;
        case 0xA1:
            INST_NAME("POP FS");
            POP1z(x2);
            STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_FS]));
            CBZw_NEXT(x2);
            MOV32w(x1, _FS);
            CALL(const_getsegmentbase, -1);
            break;
        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            MOVx_REG(x1, xRAX);
            CALL_(const_cpuid, -1, 0);
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                ASRxw(x1, gd, 5+rex.w); // r1 = (gd>>5)
                if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                ADDz_REG_LSL(x3, wback, x1, 2+rex.w); //(&ed)+=r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            IFX(X_CF) {
                if(rex.w) {
                    ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
                } else {
                    ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
                }
                LSRxw_REG(x4, ed, x2);
                BFIw(xFlags, x4, F_CF, 1);
            }
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            if(geted_ib(dyn, addr, ninst, nextop)&(rex.w?63:31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETED(1);
                GETGD;
                u8 = F8&(rex.w?63:31);
                emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { MOVw_REG(ed, ed); }
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING);
            GETGD;
            GETED(0);
            if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
            if(rex.w) {
                ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
            }
            CBZw_NEXT(x3);
            emit_shld32(dyn, ninst, rex, ed, gd, x3, x5, x4);
            WBACK;
            break;

        case 0xA8:
            INST_NAME("PUSH GS");
            LDRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_GS]));
            PUSH1z(x2);
            break;
        case 0xA9:
            INST_NAME("POP GS");
            POP1z(x2);
            STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_GS]));
            CBZw_NEXT(x2);
            MOV32w(x1, _GS);
            CALL(const_getsegmentbase, -1);
            break;

        case 0xAB:
            INST_NAME("BTS Ed, Gd");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                ASRxw(x1, gd, 5+rex.w); // r1 = (gd>>5)
                if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                ADDz_REG_LSL(x3, wback, x1, 2+rex.w); //(&ed)+=r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            IFX(X_CF) {
                LSRxw_REG(x4, ed, x2);
                BFIw(xFlags, x4, F_CF, 1);
            }
            MOV32w(x4, 1);
            LSLxw_REG(x4, x4, x2);
            ORRxw_REG(ed, ed, x4);
            if(wback) {
                STxw(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            if(geted_ib(dyn, addr, ninst, nextop)&(rex.w?63:31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETED(1);
                GETGD;
                u8 = F8&(rex.w?63:31);
                emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { MOVw_REG(ed, ed); }
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING);
            GETGD;
            GETED(0);
            if(!rex.w && !rex.is32bits && MODREG) {MOVw_REG(ed, ed);}
            if(rex.w) {
                ANDx_mask(x3, xRCX, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x3, xRCX, 0, 0b00100);  //mask=0x00000001f
            }
            CBZw_NEXT(x3);
            emit_shrd32(dyn, ninst, rex, ed, gd, x3, x5, x4);
            WBACK;
            break;

        case 0xAE:
            nextop = F8;
            if(MODREG)
                switch (nextop) {
                    case 0xE8:
                        INST_NAME("LFENCE");
                        SMDMB();
                        break;
                    case 0xF0:
                        INST_NAME("MFENCE");
                        SMDMB();
                        break;
                    case 0xF8:
                        INST_NAME("SFENCE");
                        SMDMB();
                        break;
                    default:
                        DEFAULT;
                }
            else
                switch((nextop>>3)&7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (FXSAVE)\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        CALL(rex.is32bits?const_fpu_fxsave32:const_fpu_fxsave64, -1);
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (FXRSTOR)\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        CALL(rex.is32bits?const_fpu_fxrstor32:const_fpu_fxrstor64, -1);
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        STRw_U12(ed, xEmu, offsetof(x64emu_t, mxcsr));
                        if(BOX64ENV(sse_flushto0)) {
                            MRS_fpcr(x2);                   // get fpscr
                            LSRw_IMM(x3, ed, 15);           // get FZ bit
                            BFIw(x2, x3, 24, 1);            // inject FZ bit
                            EORw_REG_LSR(x3, x3, ed, 1);    // FZ xor DAZ
                            BFIw(x2, x3, 1, 1);             // inject AH bit
                            MSR_fpcr(x2);                   // put new fpscr
                        }
                        if(BOX64ENV(sse_flushto0)) {
                            // try to sync mxcsr with fpsr on the flag side
                            /* mapping is
                                ARM <-> X86
                                 0  <->  0  // Invalid operation
                                 1  <->  2  // Divide by 0
                                 2  <->  3  // Overflow
                                 3  <->  4  // underflow
                                 4  <->  5  // Inexact
                                 5  <->  1  // denormal
                            */
                            // doing X86 -> ARM here, 543210 => 432150
                            if(ed!=x1)
                                MOVw_REG(x1, ed);   // x1 = 543210
                            UBFXw(x2, x1, 1, 5);   // x2 = 54321
                            BFIw(x1, x2, 2, 4); // x1 = 432110
                            LSRw(x2, x2, 4);    // x2 = 5
                            BFIw(x1, x2, 1, 1); // x1 = 432150
                            MRS_fpsr(x2);
                            BFIx(x2, x1, 0, 6);
                            MSR_fpsr(x2);
                        }
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                        LDRw_U12(x4, xEmu, offsetof(x64emu_t, mxcsr));
                        if(BOX64ENV(sse_flushto0)) {
                            // sync with fpsr, with mask from mxcsr
                            MRS_fpsr(x1);
                            // doing ARM -> X86 here,  0 1 2 3 4 5 -> 0 2 3 4 5 1
                            RORw(x3, x1, 2);    // x3 = 10.....5432
                            BFIw(x1, x3, 1, 4); // x1 = 54320
                            RORw(x3, x3, 32-1); // x3 = 0.....54321
                            BFIw(x1, x3, 5, 1); // x1 = 154320
                            //BFXILw(x3, x4, 7, 6); // this would the mask, but let's ignore that for now
                            BFIw(x4, x1, 0, 6); // inject back the flags
                        }
                        STW(x4, ed, fixedaddress);
                        break;
                    case 4:
                        INST_NAME("XSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (XSAVE)\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        MOV32w(x2, rex.w?0:1);
                        CALL(const_fpu_xsave, -1);
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (XRSTOR)\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {MOVx_REG(x1, ed);}
                        MOV32w(x2, rex.w?0:1);
                        CALL(const_fpu_xrstor, -1);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (CLFLUSH)?\n");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                        CALL_(const_native_clflush, -1, 0);
                        break;
                    default:
                        DEFAULT;
                }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            if(BOX64ENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                SETFLAGS(X_OF|X_CF, SF_SET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            nextop = F8;
            GETGD;
            GETED(0);
            if(rex.w) {
                // 64bits imul
                UFLAG_IF {
                    SMULH(x3, gd, ed);
                    MULx(gd, gd, ed);
                    SET_DFNONE();
                    IFX(X_CF|X_OF) {
                        CMPSx_REG_ASR(x3, gd, 63);
                        CSETw(x3, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x3, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
            } else {
                // 32bits imul
                UFLAG_IF {
                    SMULL(gd, gd, ed);
                    LSRx(x3, gd, 32);
                    MOVw_REG(gd, gd);
                    SET_DFNONE();
                    IFX(X_CF|X_OF) {
                        CMPSw_REG_ASR(x3, gd, 31);
                        CSETw(x3, cNE);
                        IFX(X_CF) {
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                        IFX(X_OF) {
                            BFIw(xFlags, x3, F_OF, 1);
                        }
                    }
                } else {
                    MULxw(gd, gd, ed);
                }
            }
            IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
            IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
            IFX2(X_SF, && !BOX64ENV(cputype)) {
                LSRxw(x3, gd, rex.w?63:31);
                BFIw(xFlags, x3, F_SF, 1);
            }
            IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, gd, x3);
            break;

        case 0xB1:
            // rep has no impact on this opcode
            INST_NAME("CMPXCHG Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
                UFLAG_IF {
                    emit_cmp32(dyn, ninst, rex, xRAX, ed, x3, x4, x5);
                } else {
                    CMPSxw_REG(xRAX, ed);
                }
                MOVxw_REG(x1, ed); // save value
                Bcond(cNE, 4 + (rex.w ? 4 : 8));
                MOVxw_REG(ed, gd);
                if (!rex.w) { B_NEXT_nocond; }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                LDxw(x1, wback, fixedaddress);
                UFLAG_IF {
                    emit_cmp32(dyn, ninst, rex, xRAX, x1, x3, x4, x5);
                }
                SUBxw_REG(x4, xRAX, x1);
                CBNZxw_MARK(x4);
                // EAX == Ed
                STxw(gd, wback, fixedaddress);
                if (!rex.w) { B_NEXT_nocond; }
                MARK;
            }
            MOVxw_REG(xRAX, x1);
            break;

        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                ASRxw(x1, gd, 5+rex.w); // r1 = (gd>>5)
                if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                ADDz_REG_LSL(x3, wback, x1, 2+rex.w); //(&ed)+=r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            IFX(X_CF) {
                LSRxw_REG(x4, ed, x2);
                BFIw(xFlags, x4, F_CF, 1);
            }
            MOV32w(x4, 1);
            LSLxw_REG(x4, x4, x2);
            BICxw_REG(ed, ed, x4);
            if(wback) {
                STxw(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;                \
                } else {
                    ed = (nextop&7);
                    eb1 = TO_NAT(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                }
                UBFXxw(gd, eb1, eb2*8, 8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 0);
                LDB(gd, ed, fixedaddress);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                UBFXxw(gd, ed, 0, 16);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                LDH(gd, ed, fixedaddress);
            }
            break;

        case 0xBA:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    gd = x2;
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 1);
                        LDxw(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=rex.w?0x3f:0x1f;
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 1);
                        LDxw(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=(rex.w?0x3f:0x1f);
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    mask = convert_bitmask_xw(1LL<<u8);
                    ORRxw_mask(ed, ed, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                    if(wback) {
                        STxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 1);
                        LDxw(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=(rex.w?0x3f:0x1f);
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    BFCxw(ed, u8, 1);
                    if(wback) {
                        STxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 1);
                        LDxw(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=(rex.w?0x3f:0x1f);
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    mask = convert_bitmask_xw(1LL<<u8);
                    EORxw_mask(ed, ed, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                    if(wback) {
                        STxw(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                ASRxw(x1, gd, 5+rex.w); // r1 = (gd>>5)
                if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                ADDz_REG_LSL(x3, wback, x1, 2+rex.w); //(&ed)+=r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            if(rex.w) {
                ANDx_mask(x2, gd, 1, 0, 0b00101);  //mask=0x000000000000003f
            } else {
                ANDw_mask(x2, gd, 0, 0b00100);  //mask=0x00000001f
            }
            IFX(X_CF) {
                LSRxw_REG(x4, ed, x2);
                BFIw(xFlags, x4, F_CF, 1);
            }
            MOV32w(x4, 1);
            LSLxw_REG(x4, x4, x2);
            EORxw_REG(ed, ed, x4);
            if(wback) {
                STxw(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            if(!BOX64ENV(dynarec_safeflags) || BOX64ENV(cputype)) {
                SETFLAGS(X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            TSTxw_REG(ed, ed);
            CSELxw(gd, xZR, gd, cEQ);   // set gd to 0 if input is 0 (UD behaviour that seems universal)
            B_MARK(cEQ);
            RBITxw(x1, ed);   // reverse
            CLZxw(gd, x1);    // x2 gets leading 0 == BSF
            MARK;
            IFX(X_ZF) {
                IFNATIVE(NF_EQ) {} else {
                    CSETw(x2, cEQ);    //other flags are undefined
                    BFIw(xFlags, x2, F_ZF, 1);
                }
            }
            if(BOX64ENV(dynarec_safeflags) && !BOX64ENV(cputype)) {
                IFX(X_CF) BFCw(xFlags, F_CF, 1);
                IFX(X_AF) BFCw(xFlags, F_AF, 1);
                IFX(X_SF) BFCw(xFlags, F_SF, 1);
                IFX(X_OF) BFCw(xFlags, F_OF, 1);
                IFX(X_PF) emit_pf(dyn, ninst, gd, x2);
            }
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            if(!BOX64ENV(dynarec_safeflags) || BOX64ENV(cputype)) {
                SETFLAGS(X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            SET_DFNONE();
            nextop = F8;
            GETED(0);
            GETGD;
            TSTxw_REG(ed, ed);
            CSELxw(gd, xZR, gd, cEQ);   // set gd to 0 if input is 0 (UD behaviour that seems universal)
            B_MARK(cEQ);
            CLZxw(gd, ed);    // x2 gets leading 0
            SUBxw_U12(gd, gd, rex.w?63:31);
            NEGxw_REG(gd, gd);   // complement
            MARK;
            IFX(X_ZF) {
                IFNATIVE(NF_EQ) {} else {
                    CSETw(x2, cEQ);    //other flags are undefined
                    BFIw(xFlags, x2, F_ZF, 1);
                }
            }
            if(BOX64ENV(dynarec_safeflags) && !BOX64ENV(cputype)) {
                IFX(X_CF) BFCw(xFlags, F_CF, 1);
                IFX(X_AF) BFCw(xFlags, F_AF, 1);
                IFX(X_SF) BFCw(xFlags, F_SF, 1);
                IFX(X_OF) BFCw(xFlags, F_OF, 1);
                IFX(X_PF) emit_pf(dyn, ninst, gd, x2);
            }
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop&7);
                    wb2 = (wback>>2)*8;
                    wback = TO_NAT(wback&3);
                }
                SBFXxw(gd, wback, wb2, 8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 0);
                LDSBxw(gd, ed, fixedaddress);
            }
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                SXTHxw(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                LDSHxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC0:
            INST_NAME("XADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGB(x1);
            GETEB(x2, 0);
            gd = x2; ed = x1;    // swap gd/ed
            emit_add8(dyn, ninst, x1, x2, x4, x5);
            GBBACK; // gb gets x2 (old ed)
            EBBACK; // eb gets x1 (sum)
            break;
        case 0xC1:
            INST_NAME("XADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED(0);
            MOVxw_REG(x3, ed);
            MOVxw_REG(ed, gd);
            MOVxw_REG(gd, x3);
            emit_add32(dyn, ninst, rex, ed, gd, x4, x5);
            WBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            switch(u8&7) {
                // the inversion of the params in the comparison is there to handle NaN the same way SSE does
                case 0: VFCMEQQS(v0, v0, v1); break;   // Equal
                case 1: VFCMGTQS(v0, v1, v0); break;   // Less than
                case 2: VFCMGEQS(v0, v1, v0); break;   // Less or equal
                case 3: if(v0!=v1) {
                            VFMAXQS(v0, v0, v1);    // propagate NAN
                        }
                        VFCMEQQS(v0, v0, v0);
                        VMVNQ(v0, v0);
                        break;   // NaN (NaN is not equal to himself)
                case 4: VFCMEQQS(v0, v0, v1); VMVNQ(v0, v0); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: VFCMGTQS(v0, v1, v0); VMVNQ(v0, v0); break;   // Greater or equal or unordered
                case 6: VFCMGEQS(v0, v1, v0); VMVNQ(v0, v0); break;   // Greater or unordered
                case 7: if(v0!=v1) {
                            VFMAXQS(v0, v0, v1);    // propagate NAN
                        }
                        VFCMEQQS(v0, v0, v0);
                        break;   // not NaN
            }
            break;
        case 0xC3:
            INST_NAME("MOVNTI Ed, Gd");
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOVxw_REG(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                STxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gm,Ed,Ib");
            nextop = F8;
            GETGM(v0);
            if(MODREG) {
                u8 = (F8)&3;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                VMOVQHfrom(v0, u8, ed);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = (F8)&3;
                VLD1_16(v0, u8, wback);
            }
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Em,Ib");
            nextop = F8;
            GETGD;
            if(MODREG) {
                GETEM(v0, 1);
                u8 = (F8)&3;
                VMOVHto(gd, v0, u8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = (F8)&3;
                LDRH_U12(gd, wback, u8*2);
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            if(!MODREG) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                v1 = -1; // to avoid a warning
            } else
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            u8 = F8;
            if(v0==v1 && (u8&0x3)==((u8>>2)&3) && (u8&0xf)==((u8>>4)&0xf)) {
                VDUPQ_32(v0, v0, u8&3);
            } else if(v0==v1 && (u8==0xe0)) {   // easy special case
                VMOVeS(v0, 1, v0, 0);
            } else if(v0==v1 && (u8==0xe5)) {   // easy special case
                VMOVeS(v0, 0, v0, 1);
            } else if(v0==v1 && (u8==0x39)) {
                VEXTQ_8(v0, v0, v0, 4);
            } else if(u8==0x4E && MODREG) {
                VEXTQ_8(v0, v0, v1, 8);
            } else {
                d0 = fpu_get_scratch(dyn, ninst);
                // first two elements from Gx
                if((u8&0xf)==0x04)
                    VMOVeD(d0, 0, v0, 0);
                else if((u8&0xf)==0x0e)
                    VMOVeD(d0, 0, v0, 1);
                else
                    for(int i=0; i<2; ++i) {
                        VMOVeS(d0, i, v0, (u8>>(i*2))&3);
                }
                // second two from Ex
                if(MODREG) {
                    if((u8&0xf0)==0x40)
                        VMOVeD(d0, 1, v1, 0);
                    else if((u8&0xf0)==0xe0)
                        VMOVeD(d0, 1, v1, 1);
                    else
                        for(int i=2; i<4; ++i) {
                            VMOVeS(d0, i, v1, (u8>>(i*2))&3);
                    }
                } else {
                    SMREAD();
                    for(int i=2; i<4; ++i) {
                        ADDx_U12(x2, ed, ((u8>>(i*2))&3)*4);
                        VLD1_32(d0, i, x2);
                    }
                }
                VMOVQ(v0, d0);
            }
            break;
        case 0xC7:
            // rep has no impact here
            nextop = F8;
            if(MODREG) switch((nextop>>3)&7) {
            case 6:
                INST_NAME("RDRAND Ed");
                SETFLAGS(X_ALL, SF_SET);
                SET_DFNONE();
                GETED(0);
                IFX(X_OF|X_SF|X_ZF|X_PF|X_AF) {
                    MOV32w(x1, (1<<F_OF)|(1<<F_SF)|(1<<F_ZF)|(1<<F_PF)|(1<<F_AF));
                    BICw(xFlags, xFlags, x1);
                }
                if(cpuext.rndr) {
                    MRS_rndr(x1);
                    IFX(X_CF) { CSETw(x3, cNE); }
                } else {
                    CALL(rex.w?const_random64:const_random32, x1);
                    IFX(X_CF) { MOV32w(x3, 1); }
                }
                IFX(X_CF) { BFIw(xFlags, x3, F_CF, 1); }
                MOVxw_REG(ed, x1);
                break;
            case 7:
                INST_NAME("RDPID Ed");
                GETED(0);
                CALL_(const_helper_getcpu, ed, x2);
                break;
            default:
                DEFAULT;
            } else switch((nextop>>3)&7) {
            case 1:
                INST_NAME("CMPXCHG8B Gq, Eq");
                SETFLAGS(X_ZF, SF_SUBSET);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                LDPxw_S7_offset(x2, x3, wback, 0);
                CMPSxw_REG(xRAX, x2);
                CCMPxw(xRDX, x3, 0, cEQ);
                B_MARK(cNE);    // EAX!=ED[0] || EDX!=Ed[1]
                STPxw_S7_offset(xRBX, xRCX, wback, 0);
                UFLAG_IF {
                    IFNATIVE(NF_EQ) {} else {MOV32w(x1, 1);}
                }
                B_MARK3_nocond;
                MARK;
                MOVxw_REG(xRAX, x2);
                MOVxw_REG(xRDX, x3);
                UFLAG_IF {
                    IFNATIVE(NF_EQ) {} else {MOV32w(x1, 0);}
                }
                MARK3;
                UFLAG_IF {
                    IFNATIVE(NF_EQ) {} else {
                        BFIw(xFlags, x1, F_ZF, 1);
                    }
                }
                SMWRITE();
                break;
            case 4:
                INST_NAME("Unsupported XSAVEC Ed");
                FAKEED;
                BARRIER(BARRIER_FLOAT);
                GETIP(ip);
                UDF(0);
                break;
            case 6:
                INST_NAME("RDRAND Ed");
                SETFLAGS(X_ALL, SF_SET);
                SET_DFNONE();
                IFX(X_OF|X_SF|X_ZF|X_PF|X_AF) {
                    MOV32w(x1, (1<<F_OF)|(1<<F_SF)|(1<<F_ZF)|(1<<F_PF)|(1<<F_AF));
                    BICw(xFlags, xFlags, x1);
                }
                if(cpuext.rndr) {
                    MRS_rndr(x1);
                    IFX(X_CF) { CSETw(x3, cNE); }
                } else {
                    CALL(rex.w?const_random64:const_random32, x1);
                    IFX(X_CF) { MOV32w(x3, 1); }
                }
                IFX(X_CF) { BFIw(xFlags, x3, F_CF, 1); }
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, NULL, 0, 0);
                STxw(x1, wback, fixedaddress);
                break;
            default:
                DEFAULT;
            }
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
            gd = TO_NAT((opcode&7)+(rex.b<<3));
            REVxw(gd, gd);
            break;
        case 0xD1:
            INST_NAME("PSRLW Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 16);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +15 values
            VDUP_16(v0, v0, 0);    // only the low 8bits will be used anyway
            NEG_16(v0, v0);
            USHL_16(d0, d0, v0);
            break;
        case 0xD2:
            INST_NAME("PSRLD Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 32);
            UMIN_32(v0, v0, v1);        // limit to 0 .. +31 values
            VDUP_32(v0, v0, 0);         // only the low 8bits will be used anyway
            NEG_32(v0, v0);
            USHL_32(d0, d0, v0);
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v1, d1);
            MOVI_32(v0, 64);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +64 values
            NEG_32(v0, v0);
            UXTL_32(v0, v0);
            USHL_R_64(d0, d0, v0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            ADD_64(v0, v0, q0);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            VMUL_16(q0, q0, q1);
            break;

        case 0xD7:
            nextop = F8;
            INST_NAME("PMOVMSKB Gd, Em");
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            GETEM(q0, 0);
            GETGD;
            TABLE64C(x1, const_8b_m7_m6_m5_m4_m3_m2_m1_0);
            VLDR64_U12(v0, x1, 0);     // load shift
            MOVI_8(v1, 0x80);   // load mask
            VAND(q1, v1, q0);
            USHL_8(q1, q1, v0); // shift
            UADDLV_8(q1, q1);   // accumalte
            VMOVBto(gd, q1, 0);
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            UQSUB_8(q0, q0, q1);
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            UQSUB_16(q0, q0, q1);
            break;
        case 0xDA:
            INST_NAME("PMINUB Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            UMIN_8(d0, d0, d1);
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VAND(v0, v0, v1);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gm,Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            UQADD_8(q0, q0, q1);
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gm,Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            UQADD_16(q0, q0, q1);
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            UMAX_8(d0, d0, d1);
            break;
         case 0xDF:
            INST_NAME("PANDN Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VBIC(v0, v1, v0);
            break;
         case 0xE0:
            INST_NAME("PAVGB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            URHADD_8(v0, v0, v1);
            break;
        case 0xE1:
            INST_NAME("PSRAW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 15);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +15 values
            NEG_32(v0, v0);
            VDUP_16(v0, v0, 0);    // only the low 8bits will be used anyway
            SSHL_16(d0, d0, v0);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 31);
            UMIN_32(v0, v0, v1);        // limit to 0 .. +31 values
            NEG_32(v0, v0);
            VDUP_32(v0, v0, 0);         // only the low 8bits will be used anyway
            SSHL_32(d0, d0, v0);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            URHADD_16(d0, d0, d1);
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            VUMULL_16(q0, v0, v1);
            UQSHRN_16(v0, q0, 16);  // saturation will never happens as only 16bits remain and fits in 16bits
            break;
        case 0xE5:
            INST_NAME("PMULHW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            VSMULL_16(q0, v0, v1);
            SQSHRN_16(v0, q0, 16);
            break;

        case 0xE7:
            INST_NAME("MOVNTQ Em, Gm");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                DEFAULT;
            } else {
                v0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v0, ed, fixedaddress);
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            SQSUB_8(v0, v0, q0);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            SQSUB_16(v0, v0, q0);
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx,Ex");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            SMIN_16(v0, v0, q0);
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VORR(v0, v0, v1);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            SQADD_8(d0, d0, d1);
            break;
        case 0xED:
            INST_NAME("PADDSW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            SQADD_16(d0, d0, d1);
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0, 0);
            SMAX_16(v0, v0, q0);
            break;
        case 0xEF:
            INST_NAME("PXOR Gm,Em");
            nextop = F8;
            gd = ((nextop&0x38)>>3);
            if(MODREG && ((nextop&7))==gd) {
                // special case for PXOR Gm, Gm
                q0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                VEOR(q0, q0, q0);
            } else {
                q0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
                GETEM(q1, 0);
                VEOR(q0, q0, q1);
            }
            break;
        case 0xF1:
            INST_NAME("PSLLW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 16);
            UMIN_32(v0, v0, v1); // limit to 0 .. +16 values
            VDUP_16(v0, v0, 0);
            USHL_16(d0, d0, v0);
            break;
        case 0xF2:
            INST_NAME("PSLLD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 32);
            UMIN_32(v0, v0, v1); // limit to 0 .. +32 values
            VDUPQ_32(v0, v0, 0);
            SSHL_32(d0, d0, v0);
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, d1);
            MOVI_32(v1, 64);
            UMIN_32(v0, v0, v1); // limit to 0 .. +64 values
            USHL_R_64(d0, d0, v0);
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VUMULL_32(v0, v0, v1);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            VSMULL_16(q0, v0, v1);
            VADDPQ_32(q0, q0, q0); //ADDP from Q to non-Q?
            VMOV(v0, q0);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            d0 = fpu_get_scratch(dyn, ninst);
            d1 = fpu_get_scratch(dyn, ninst);
            UABDL_8(d0, q0, q1);    // 8bits absolute difference in 16bits
            UADDLVQ_16(d1, d0); // sum accros vector on the 16bits value inside a 32bits scalar
            UXTL_32(d1, d1);    // 32bits -> 64bits
            VMOV(q0, d1);
            break;
        case 0xF7:
            INST_NAME("MASKMOVQ Gm, Em");
            nextop = F8;
            GETGM(q0);
            GETEM(q1, 0);
            d0 = fpu_get_scratch(dyn, ninst);
            d1 = fpu_get_scratch(dyn, ninst);
            VSSHR_8(d1, q1, 7); // d1 = byte slection mask
            VLDR64_U12(d0, xRDI, 0);
            VBIT(d0, q0, d1);
            VSTR64_U12(d0, xRDI, 0);
            break;
        case 0xF8:
            INST_NAME("PSUBB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_8(v0, v0, v1);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_16(v0, v0, v1);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VSUB_32(v0, v0, v1);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            SUB_64(v0, v0, v1);
            break;
        case 0xFC:
            INST_NAME("PADDB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_8(v0, v0, v1);
            break;
        case 0xFD:
            INST_NAME("PADDW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_16(v0, v0, v1);
            break;
        case 0xFE:
            INST_NAME("PADDD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1, 0);
            VADD_32(v0, v0, v1);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
