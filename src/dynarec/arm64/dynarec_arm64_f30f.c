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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_F30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback;
    uint64_t u64;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;
    int64_t j64;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                VMOVeS(v0, 0, q0, 0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VLD32(v0, ed, fixedaddress);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                q0 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 1);
                VMOVeS(q0, 0, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                VST32(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            if(MODREG) {
                q1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLD128(q1, ed, fixedaddress);
            }
            GETGX_empty(q0);
            VTRNQ1_32(q0, q1, q1);
            break;

        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            if(MODREG) {
                q1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                q1 = fpu_get_scratch(dyn);
                VLD128(q1, ed, fixedaddress);
            }
            GETGX_empty(q0);
            VTRNQ2_32(q0, q1, q1);
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
            if(rex.w) {
                SCVTFSx(d1, ed);
            } else {
                SCVTFSw(d1, ed);
            }
            VMOVeS(v0, 0, d1, 0);
            break;

        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            FCVTZSxwS(gd, d0);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    ORRw_mask(gd, xZR, 1, 0);    //0x80000000
                }
            }
            break;
        case 0x2D:
            INST_NAME("CVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(q0, 0, 0);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            u8 = sse_setround(dyn, ninst, x1, x2, x3);
            d1 = fpu_get_scratch(dyn);
            FRINTIS(d1, q0);
            x87_restoreround(dyn, ninst, u8);
            FCVTZSxwS(gd, d1);
            if(!box64_dynarec_fastround) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    ORRw_mask(gd, xZR, 1, 0);    //0x80000000
                }
            }
            break;
        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FSQRTS(d1, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x52:
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            // so here: F32: Imm8 = abcd efgh that gives => aBbbbbbc defgh000 00000000 00000000
            // and want 1.0f = 0x3f800000
            // so 00111111 10000000 00000000 00000000
            // a = 0, b = 1, c = 1, d = 1, efgh=0
            // 0b01110000
            FMOVS_8(d0, 0b01110000);
            FSQRTS(d1, v1);
            FDIVS(d0, d0, d1);
            VMOVeS(v0, 0, d0, 0);
            break;
        case 0x53:
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FMOVS_8(d0, 0b01110000);    //1.0f
            FDIVS(d0, d0, v1);
            VMOVeS(v0, 0, d0, 0);
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FADDS(d1, v0, d0);  // the high part of the vector is erased...
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FMULS(d1, v0, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            d0 = fpu_get_scratch(dyn);
            FCVT_D_S(d0, v1);
            VMOVeD(v0, 0, d0, 0);
            break;
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0) ;
            GETGX_empty(v0);
            if(box64_dynarec_fastround) {
                VFCVTZSQS(v0, v1);
            } else {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
                ORRw_mask(x4, xZR, 1, 0);    //0x80000000
                d0 = fpu_get_scratch(dyn);
                for(int i=0; i<4; ++i) {
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    VMOVeS(d0, 0, v1, i);
                    VFCVTZSs(d0, d0);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ(x5, FPSR_IOC, 4+4);
                    VMOVQSfrom(d0, 0, x4);
                    VMOVeS(v0, i, d0, 0);
                }
            }
            break;

        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FSUBS(d1, v0, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            // MINSS: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            #if 0
            d0 = fpu_get_scratch(dyn);
            FMINNMS(d0, v0, v1);    // NaN handling may be slightly different, is that a problem?
            VMOVeS(v0, 0, d0, 0);   // to not erase uper part
            #else
            FCMPS(v0, v1);
            B_NEXT(cLS);    //Less than or equal
            VMOVeS(v0, 0, v1, 0);   // to not erase uper part
            #endif
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn);
            GETEXSS(d0, 0, 0);
            FDIVS(d1, v0, d0);
            VMOVeS(v0, 0, d1, 0);
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 0);
            // MAXSS: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            #if 0
            d0 = fpu_get_scratch(dyn);
            FMAXNMS(d0, v0, v1);    // NaN handling may be slightly different, is that a problem?
            VMOVeS(v0, 0, d0, 0);   // to not erase uper part
            #else
            FCMPS(v0, v1);
            B_NEXT(cGE);    //Greater than or equal
            VMOVeS(v0, 0, v1, 0);   // to not erase uper part
            #endif
            break;

        case 0x6F:
            INST_NAME("MOVDQU Gx,Ex");// no alignment constraint on NEON here, so same as MOVDQA
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VMOVQ(v0, v1);
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1) ;
            GETGX(v0, 1);
            u8 = F8;
            d0 = fpu_get_scratch(dyn);
            if (u8 == 0b00000000 || u8 == 0b01010101 || u8 == 0b10101010 || u8 == 0b11111111) {
                VDUPQ_16(d0, v1, (u8 & 3) + 4);
            } else {
                // only high part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
                u64 = 0;
                for (int i = 0; i < 4; ++i) {
                    u64 |= ((uint64_t)((u8 >> (i * 2)) & 3) * 2 + 8) << (i * 16 + 0);
                    u64 |= ((uint64_t)((u8 >> (i * 2)) & 3) * 2 + 9) << (i * 16 + 8);
                }
                MOV64x(x2, u64);
                VMOVQDfrom(d0, 0, x2);
                VTBL1_8(d0, v1, d0);
            }
            VMOVeD(v0, 1, d0, 0);
            if(v0!=v1) {
                VMOVeD(v0, 0, v1, 0);
            }
            break;

        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7) + (rex.b<<3), 0);
                GETGX_empty(v0);
                FMOVD(v0, v1);
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v0, ed, fixedaddress);
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0xB8:
            INST_NAME("POPCNT Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            SET_DFNONE(x1);
            nextop = F8;
            v1 = fpu_get_scratch(dyn);
            GETGD;
            if(MODREG) {
                GETED(0);
                if(rex.w)
                    VMOVQDfrom(v1, 0, ed);
                else {
                    MOVxw_REG(x1, ed);  // need to clear uper part
                    VMOVQDfrom(v1, 0, x1);
                }
            } else {
                if(rex.w) {
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VLD64(v1, ed, fixedaddress);
                } else {
                    GETED(0);   // just load and clear the upper part
                    VMOVQDfrom(v1, 0, ed);
                }
            }
            CNT_8(v1, v1);
            UADDLV_8(v1, v1);
            VMOVQDto(gd, v1, 0);
            IFX(X_ALL) {
                MOV32w(x1, (1<<F_OF) | (1<<F_SF) | (1<<F_ZF) | (1<<F_AF) | (1<<F_CF) | (1<<F_PF));
                BICw(xFlags, xFlags, x1);
                CBNZx(gd, 4+4);
                BFIw(xFlags, xFlags, F_ZF, 1);
            }
            break;

        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            SETFLAGS(X_CF|X_ZF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETED(0);
            GETGD;
            TSTxw_REG(ed, ed);
            CSETw(x3, cEQ);
            BFIw(xFlags, x3, F_CF, 1);  // CF = is source 0?
            RBITxw(x3, ed);   // reverse
            CLZxw(gd, x3);    // x2 gets leading 0 == TZCNT
            TSTxw_REG(gd, gd);
            CSETw(x3, cEQ);
            BFIw(xFlags, x3, F_ZF, 1);  // ZF = is dest 0?
            break;
        case 0xBD:
            INST_NAME("LZCNT Gd, Ed");
            SETFLAGS(X_CF|X_ZF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETED(0);
            GETGD;
            TSTxw_REG(ed, ed);
            CSETw(x3, cEQ);
            BFIw(xFlags, x3, F_CF, 1);  // CF = is source 0?
            CLZxw(gd, ed);    // x2 gets leading 0 == LZCNT
            TSTxw_REG(gd, gd);
            CSETw(x3, cEQ);
            BFIw(xFlags, x3, F_ZF, 1);  // ZF = is dest 0?
            break;

        case 0xC2:
            INST_NAME("CMPSS Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(v1, 0, 1);
            u8 = F8;
            FCMPS(v0, v1);
            switch(u8&7) {
                case 0: CSETMw(x2, cEQ); break;   // Equal
                case 1: CSETMw(x2, cCC); break;   // Less than
                case 2: CSETMw(x2, cLS); break;   // Less or equal
                case 3: CSETMw(x2, cVS); break;   // NaN
                case 4: CSETMw(x2, cNE); break;   // Not Equal or unordered
                case 5: CSETMw(x2, cCS); break;   // Greater or equal or unordered
                case 6: CSETMw(x2, cHI); break;   // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                case 7: CSETMw(x2, cVC); break;   // not NaN
            }
            VMOVQSfrom(v0, 0, x2);
            break;

        case 0xD6:
            INST_NAME("MOVQ2DQ Gx, Em");
            nextop = F8;
            GETGX_empty(v0);
            if(MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, (nextop&7));
                VEORQ(v0, v0, v0);  // usefull?
                VMOV(v0, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v0, ed, fixedaddress);
            }
            break;

        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            GETEXSD(v1, 0, 0);
            GETGX_empty(v0);
            d0 = fpu_get_scratch(dyn);
            SXTL_32(v0, v1);
            SCVTQFD(v0, v0);    // there is only I64 -> Double vector conversion, not from i32
            break;

        default:
            DEFAULT;
    }
    return addr;
}
