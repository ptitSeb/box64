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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    int64_t j64;
    uint64_t tmp64u, tmp64u2;
    int mask;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    #if STEP > 1
    static const int8_t round_round[] = { 0, 2, 1, 3};
    #endif

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVUPD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX(v0, 1);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VLD1_64(v0, 0, ed);
            break;
        case 0x13:
            INST_NAME("MOVLPD Eq, Gx");
            nextop = F8;
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            GETGX(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VST1_64(v0, 0, ed);
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                VMOVeD(v0, 1, v1, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(v0, 1, ed);
            }
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            VMOVeD(v0, 0, v0, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                VMOVeD(v0, 1, v1, 1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                v1 = fpu_get_scratch(dyn, ninst);
                ADDx_U12(x1, ed, 8);
                VLD1_64(v0, 1, x1);
            }
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VLD1_64(v0, 1, ed);
            break;
        case 0x17:
            INST_NAME("MOVHPD Eq, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
            VST1_64(v0, 1, ed);
            SMWRITE2();
            break;

        case 0x18:
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

        case 0x28:
            INST_NAME("MOVAPD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
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
            INST_NAME("CVTPI2PD Gx,Em");
            nextop = F8;
            GETGX(v0, 1);
            GETEM(q1, 0);
            SXTL_32(v0, q1);
            SCVTQFD(v0, v0);
            break;
        case 0x2B:
            INST_NAME("MOVNTPD Ex,Gx");
            nextop = F8;
            if(MODREG) {
                DEFAULT;
            } else {
                GETGX(v0, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTPD2PI Gm,Ex");
            nextop = F8;
            GETGM(q0);
            GETEX(v1, 0, 0);
            if(BOX64ENV(dynarec_fastround)) {
                VFCVTZSQD(q0, v1);
                SQXTN_32(q0, q0);
            } else {
                if(cpuext.frintts) {
                    VFRINT32ZDQ(q0, v1);
                    VFCVTZSQD(q0, q0);
                    SQXTN_32(q0, q0);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    ORRw_mask(x2, xZR, 1, 0);    //0x80000000
                    d0 = fpu_get_scratch(dyn, ninst);
                    for (int i=0; i<2; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        if (i) {
                            VMOVeD(d0, 0, v1, i);
                            FRINTZD(d0, d0);
                        } else {
                            FRINTZD(d0, v1);
                        }
                        FCVTZSwD(x1, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        MOVw_REG(x1, x2);
                        VMOVQSfrom(q0, i, x1);
                    }
                }
            }
            break;
        case 0x2D:
            INST_NAME("CVTPD2PI Gm,Ex");
            nextop = F8;
            GETGM(q0);
            GETEX(v1, 0, 0);
            if(BOX64ENV(dynarec_fastround)) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                VFRINTIDQ(q0, v1);
                FCVTXN(q0, q0);
                x87_restoreround(dyn, ninst, u8);
                VFCVTZSS(q0, q0);
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                if(cpuext.frintts) {
                    VFRINT32XDQ(q0, v1);
                    VFCVTZSQD(q0, q0);
                    SQXTN_32(q0, q0);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    ORRw_mask(x2, xZR, 1, 0);    //0x80000000
                    d0 = fpu_get_scratch(dyn, ninst);
                    for (int i=0; i<2; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        if (i) {
                            VMOVeD(d0, 0, v1, i);
                            FRINTID(d0, d0);
                        } else {
                            FRINTID(d0, v1);
                        }
                        FCVTZSwD(x1, d0);
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
            if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGX(v0, 0);
            GETEXSD(q0, 0, 0);
            IFX(X_CF|X_PF|X_ZF) {
                FCMPD(v0, q0);
            }
            FCOMI(x1, x2);
            break;

        case 0x38:  // SSSE3 opcodes
            nextop = F8;
            switch(nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    d0 = fpu_get_scratch(dyn, ninst);
                    MOVIQ_8(d0, 0b10001111);
                    VANDQ(d0, d0, q1);  // mask the index
                    VTBLQ1_8(q0, q0, d0);
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    VADDPQ_16(q0, q0, q1);
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    VADDPQ_32(q0, q0, q1);
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP1Q_16(v0, q0, q1);
                    VUZP2Q_16(q0, q0, q1);
                    SQADDQ_16(q0, q0, v0);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(q0==q1)
                        d0 = fpu_get_scratch(dyn, ninst);
                    else
                        d0 = q0;
                    UXTL_8(v0, q0);   // this is unsigned, so 0 extended
                    SXTL_8(v1, q1);   // this is signed
                    VMULQ_16(v0, v0, v1);
                    SADDLPQ_16(v1, v0);
                    UXTL2_8(v0, q0);   // this is unsigned
                    SQXTN_16(d0, v1);   // SQXTN reset the vector so need to grab the high part first
                    SXTL2_8(v1, q1);   // this is signed
                    VMULQ_16(v0, v0, v1);
                    SADDLPQ_16(v0, v0);
                    SQXTN2_16(d0, v0);
                    if(q0!=d0)
                        VMOVQ(q0, d0);
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP2Q_16(v0, q0, q1);
                    VUZP1Q_16(q0, q0, q1);
                    VSUBQ_16(q0, q0, v0);
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP2Q_32(v0, q0, q1);
                    VUZP1Q_32(q0, q0, q1);
                    VSUBQ_32(q0, q0, v0);
                    break;
                case 0x07:
                    INST_NAME("PHSUBSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    VUZP2Q_16(v0, q0, q1);
                    VUZP1Q_16(q0, q0, q1);
                    SQSUBQ_16(q0, q0, v0);
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v1 = fpu_get_scratch(dyn, ninst);
                    v0 = fpu_get_scratch(dyn, ninst);
                    NEGQ_8(v0, q0);     // get NEG
                    CMLTQ_0_8(v1, q1);  // calculate mask
                    VBIFQ(v0, q0, v1);  // put back positive values
                    CMEQQ_0_8(v1, q1);  // handle case where Ex is 0
                    VBICQ(q0, v0, v1);
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v1 = fpu_get_scratch(dyn, ninst);
                    v0 = fpu_get_scratch(dyn, ninst);
                    NEGQ_16(v0, q0);     // get NEG
                    CMLTQ_0_16(v1, q1);  // calculate mask
                    VBIFQ(v0, q0, v1);   // put back positive values
                    CMEQQ_0_16(v1, q1);  // handle case where Ex is 0
                    VBICQ(q0, v0, v1);
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v1 = fpu_get_scratch(dyn, ninst);
                    v0 = fpu_get_scratch(dyn, ninst);
                    NEGQ_32(v0, q0);     // get NEG
                    CMLTQ_0_32(v1, q1);  // calculate mask
                    VBIFQ(v0, q0, v1);   // put back positive values
                    CMEQQ_0_32(v1, q1);  // handle case where Ex is 0
                    VBICQ(q0, v0, v1);
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = fpu_get_scratch(dyn, ninst);
                    v1 = fpu_get_scratch(dyn, ninst);
                    VSMULL_16(v0, q0, q1);
                    VSMULL2_16(v1, q0, q1);
                    SRSHRQ_32(v0, v0, 15);
                    SRSHRQ_32(v1, v1, 15);
                    XTN_16(q0, v0);
                    XTN2_16(q0, v1);
                    break;

                case 0x10:
                    INST_NAME("PBLENDVB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = sse_get_reg(dyn, ninst, x1, 0, 0);
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(q0!=q1) {
                        VSSHRQ_8(v1, v0, 7);    // bit[7]-> bit[7..0]
                        VBITQ(q0, q1, v1);
                    }
                    break;

                case 0x14:
                    INST_NAME("BLENDVPS Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = sse_get_reg(dyn, ninst, x1, 0, 0);
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(q0!=q1) {
                        VSSHRQ_32(v1, v0, 31);    // bit[31]-> bit[31..0]
                        VBITQ(q0, q1, v1);
                    }
                    break;
                case 0x15:
                    INST_NAME("BLENDVPD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 0);
                    v0 = sse_get_reg(dyn, ninst, x1, 0, 0);
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(q0!=q1) {
                        VSSHRQ_64(v1, v0, 63);    // bit[63]-> bit[63..0]
                        VBITQ(q0, q1, v1);
                    }
                    break;

                case 0x17:
                    INST_NAME("PTEST Gx, Ex");
                    nextop = F8;
                    SETFLAGS(X_ALL, SF_SET);
                    GETGX(v0, 0);
                    GETEX(v1, 0, 0);
                    q0 = fpu_get_scratch(dyn, ninst);
                    IFX(X_CF) {
                        VBICQ(q0, v1, v0);
                        CMEQQ_0_64(q0, q0);
                        UQXTN_32(q0, q0);
                        VMOVQDto(x2, q0, 0);
                        ADDSx_U12(xZR, x2, 1);
                        CSETw(x2, cEQ);
                        BFIw(xFlags, x2, F_CF, 1);
                    }
                    IFX(X_ZF) {
                        VANDQ(q0, v0, v1);
                        CMEQQ_0_64(q0, q0);
                        UQXTN_32(q0, q0);
                        VMOVQDto(x2, q0, 0);
                        ADDSx_U12(xZR, x2, 1);
                        IFNATIVE(NF_EQ) {} else {
                            CSETw(x2, cEQ);
                            BFIw(xFlags, x2, F_ZF, 1);
                        }
                    }
                    IFX(X_PF|X_AF|X_OF|X_SF) {
                        MOV32w(x1, (1<<F_PF)|(1<<F_AF)|(1<<F_OF)|(1<<F_SF));
                        BICw_REG(xFlags, xFlags, x1);
                    }
                    SET_DFNONE();
                    break;

                case 0x1C:
                    INST_NAME("PABSB Gx, Ex");
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX_empty(q0);
                    ABSQ_8(q0, q1);
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gx, Ex");
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX_empty(q0);
                    ABSQ_16(q0, q1);
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gx, Ex");
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX_empty(q0);
                    ABSQ_32(q0, q1);
                    break;

                case 0x20:
                    INST_NAME("PMOVSXBW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX64(q1, 0, 0);
                    GETGX_empty(q0);
                    SXTL_8(q0, q1);     // 8bits->16bits
                    break;
                case 0x21:
                    INST_NAME("PMOVSXBD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX32(q1, 0, 0);
                    GETGX_empty(q0);
                    SXTL_8(q0, q1);     // 8bits->16bits
                    SXTL_16(q0, q0);    //16bits->32bits
                    break;
                case 0x22:
                    INST_NAME("PMOVSXBQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX16(q1, 0, 0);
                    GETGX_empty(q0);
                    SXTL_8(q0, q1);     // 8bits->16bits
                    SXTL_16(q0, q0);    //16bits->32bits
                    SXTL_32(q0, q0);    //32bits->64bits
                    break;
                case 0x23:
                    INST_NAME("PMOVSXWD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX64(q1, 0, 0);
                    GETGX_empty(q0);
                    SXTL_16(q0, q1);     // 16bits->32bits
                    break;
                case 0x24:
                    INST_NAME("PMOVSXWQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX32(q1, 0, 0);
                    GETGX_empty(q0);
                    SXTL_16(q0, q1);     // 16bits->32bits
                    SXTL_32(q0, q0);     // 32bits->64bits
                    break;
                case 0x25:
                    INST_NAME("PMOVSXDQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX64(q1, 0, 0);
                    GETGX_empty(q0);
                    SXTL_32(q0, q1);     // 32bits->64bits
                    break;

                case 0x28:
                    INST_NAME("PMULDQ Gx, Ex");
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    VUZP1Q_32(q0, q0, q0);   // needs elem 0 and 2 in lower part
                    if(q0==q1) {
                        v0 = q0;
                    } else {
                        if(MODREG)
                            v0 = fpu_get_scratch(dyn, ninst);
                        else
                            v0 = q1;
                        VUZP1Q_32(v0, q1, q1);
                    }
                    VSMULL_32(q0, q0, v0);
                    break;
                case 0x29:
                    INST_NAME("PCMPEQQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    VCMEQQ_64(q0, q0, q1);
                    break;
                case 0x2A:
                    INST_NAME("MOVNTDQA Gx, Ex");
                    nextop = F8;
                    if (MODREG) {
                        DEFAULT;
                    } else {
                        GETGX_empty(v0);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff << 4, 15, rex, NULL, 0, 0);
                        VLD128(v0, ed, fixedaddress);
                    }
                    break;
                case 0x2B:
                    INST_NAME("PACKUSDW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    SQXTUN_16(q0, q0);
                    if(q0==q1) {
                        VMOVeD(q0, 1, q0, 0);
                    } else {
                        SQXTUN2_16(q0, q1);
                    }
                    break;

                case 0x30:
                    INST_NAME("PMOVZXBW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX64(q1, 0, 0);
                    GETGX_empty(q0);
                    UXTL_8(q0, q1);     // 8bits->16bits
                    break;
                case 0x31:
                    INST_NAME("PMOVZXBD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX32(q1, 0, 0);
                    GETGX_empty(q0);
                    UXTL_8(q0, q1);     // 8bits->16bits
                    UXTL_16(q0, q0);    //16bits->32bits
                    break;
                case 0x32:
                    INST_NAME("PMOVZXBQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX16(q1, 0, 0);
                    GETGX_empty(q0);
                    UXTL_8(q0, q1);     // 8bits->16bits
                    UXTL_16(q0, q0);    //16bits->32bits
                    UXTL_32(q0, q0);    //32bits->64bits
                    break;
                case 0x33:
                    INST_NAME("PMOVZXWD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX64(q1, 0, 0);
                    GETGX_empty(q0);
                    UXTL_16(q0, q1);     // 16bits->32bits
                    break;
                case 0x34:
                    INST_NAME("PMOVZXWQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX32(q1, 0, 0);
                    GETGX_empty(q0);
                    UXTL_16(q0, q1);     // 16bits->32bits
                    UXTL_32(q0, q0);     // 32bits->64bits
                    break;
                case 0x35:
                    INST_NAME("PMOVZXDQ Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX64(q1, 0, 0);
                    GETGX_empty(q0);
                    UXTL_32(q0, q1);     // 32bits->64bits
                    break;

                case 0x37:
                    INST_NAME("PCMPGTQ Gx, Ex"); // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    VCMGTQ_64(q0, q0, q1);
                    break;
                case 0x38:
                    INST_NAME("PMINSB Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    SMINQ_8(q0, q0, q1);
                    break;
                case 0x39:
                    INST_NAME("PMINSD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    SMINQ_32(q0, q0, q1);
                    break;
                case 0x3A:
                    INST_NAME("PMINUW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    UMINQ_16(q0, q0, q1);
                    break;
                case 0x3B:
                    INST_NAME("PMINUD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    UMINQ_32(q0, q0, q1);
                    break;
                case 0x3C:
                    INST_NAME("PMAXSB Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    SMAXQ_8(q0, q0, q1);
                    break;
                case 0x3D:
                    INST_NAME("PMAXSD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    SMAXQ_32(q0, q0, q1);
                    break;
                case 0x3E:
                    INST_NAME("PMAXUW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    UMAXQ_16(q0, q0, q1);
                    break;
                case 0x3F:
                    INST_NAME("PMAXUD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    UMAXQ_32(q0, q0, q1);
                    break;
                case 0x40:
                    INST_NAME("PMULLD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0, 0);
                    GETGX(q0, 1);
                    VMULQ_32(q0, q0, q1);
                    break;

                case 0xDB:
                    INST_NAME("AESIMC Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(cpuext.aes) {
                        GETEX(q1, 0, 0);
                        GETGX_empty(q0);
                        AESIMC(q0, q1);
                    } else {
                        GETEX(q1, 0, 0);
                        GETGX_empty(q0);
                        if(q0!=q1) {
                            VMOVQ(q0, q1);
                        }
                        sse_forget_reg(dyn, ninst, gd);
                        MOV32w(x1, gd);
                        CALL(const_native_aesimc, -1);
                    }
                    break;
                case 0xDC:
                    INST_NAME("AESENC Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(cpuext.aes) {
                        GETEX(q1, 0, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESE(v0, q1);
                        AESMC(v0, v0);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        GETEX(q1, 0, 0);
                        if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                            d0 = fpu_get_scratch(dyn, ninst);
                            VMOVQ(d0, q1);
                        } else d0 = -1;
                        sse_forget_reg(dyn, ninst, gd);
                        MOV32w(x1, gd);
                        CALL(const_native_aese, -1);
                        GETGX(q0, 1);
                        VEORQ(q0, q0, (d0!=-1)?d0:q1);
                    }
                    break;
                case 0xDD:
                    INST_NAME("AESENCLAST Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(cpuext.aes) {
                        GETEX(q1, 0, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESE(v0, q1);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        GETEX(q1, 0, 0);
                        if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                            d0 = fpu_get_scratch(dyn, ninst);
                            VMOVQ(d0, q1);
                        } else d0 = -1;
                        sse_forget_reg(dyn, ninst, gd);
                        MOV32w(x1, gd);
                        CALL(const_native_aeselast, -1);
                        GETGX(q0, 1);
                        VEORQ(q0, q0, (d0!=-1)?d0:q1);
                    }
                    break;
                case 0xDE:
                    INST_NAME("AESDEC Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(cpuext.aes) {
                        GETEX(q1, 0, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESD(v0, q1);
                        AESIMC(v0, v0);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        GETEX(q1, 0, 0);
                        if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                            d0 = fpu_get_scratch(dyn, ninst);
                            VMOVQ(d0, q1);
                        } else d0 = -1;
                        sse_forget_reg(dyn, ninst, gd);
                        MOV32w(x1, gd);
                        CALL(const_native_aesd, -1);
                        GETGX(q0, 1);
                        VEORQ(q0, q0, (d0!=-1)?d0:q1);
                    }
                    break;
                case 0xDF:
                    INST_NAME("AESDECLAST Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(cpuext.aes) {
                        GETEX(q1, 0, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch(dyn, ninst);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESD(v0, q1);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        GETEX(q1, 0, 0);
                        if(MODREG && (gd==(nextop&7)+(rex.b<<3))) {
                            d0 = fpu_get_scratch(dyn, ninst);
                            VMOVQ(d0, q1);
                        } else d0 = -1;
                        sse_forget_reg(dyn, ninst, gd);
                        MOV32w(x1, gd);
                        CALL(const_native_aesdlast, -1);
                        GETGX(q0, 1);
                        VEORQ(q0, q0, (d0!=-1)?d0:q1);
                    }
                    break;
                case 0xF0:
                    INST_NAME("MOVBE Gw, Ew");
                    nextop=F8;
                    gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        REV16x(x1, ed);
                        BFIx(gd, x1, 0, 16);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, (1<<1)-1, rex, NULL, 0, 0);
                        LDH(x1, ed, fixedaddress);
                        REV16x(x1, x1);
                        BFIx(gd, x1, 0, 16);
                    }
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ew, Gw");
                    nextop=F8;
                    gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        REV16x(x1, gd);
                        BFIx(ed, x1, 0, 16);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff<<1, (1<<1)-1, rex, NULL, 0, 0);
                        REV16x(x1, gd);
                        STH(x1, ed, fixedaddress);
                        SMWRITE();
                    }
                    break;

                case 0xF6:
                    INST_NAME("ADCX Gd, Ed");
                    nextop = F8;
                    READFLAGS(X_CF);
                    SETFLAGS(X_CF, SF_SUBSET);
                    GETED(0);
                    GETGD;
                    IFNATIVE_BEFORE(NF_CF) {
                        if(INVERTED_CARRY_BEFORE) {
                            if(cpuext.flagm)
                                CFINV();
                            else {
                                MRS_nzcv(x3);
                                EORx_mask(x3, x3, 1, 35, 0);  //mask=1<<NZCV_C
                                MSR_nzcv(x3);
                            }
                        }
                    } else {
                        MRS_nzcv(x3);
                        BFIx(x3, xFlags, 29, 1); // set C
                        MSR_nzcv(x3);      // load CC into ARM CF
                    }
                    IFX(X_CF) {
                        ADCSxw_REG(gd, gd, ed);
                        IFNATIVE(NF_CF) {} else {
                            CSETw(x3, cCS);
                            BFIw(xFlags, x3, F_CF, 1);
                        }
                    } else {
                        ADCxw_REG(gd, gd, ed);
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x3A:  // these are some more SSSE3+ opcodes
            opcode = F8;
            switch(opcode) {
                case 0x08:
                    INST_NAME("ROUNDPS Gx, Ex, Ib");
                    nextop = F8;
                    GETEX(q1, 0, 1);
                    GETGX_empty(q0);
                    u8 = F8;
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(u8&4) {
                        u8 = sse_setround(dyn, ninst, x1, x2, x3);
                        VFRINTISQ(q0, q1);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        VFRINTRSQ(q0, q1, round_round[u8&3]);
                    }
                    break;
                case 0x09:
                    INST_NAME("ROUNDPD Gx, Ex, Ib");
                    nextop = F8;
                    GETEX(q1, 0, 1);
                    GETGX_empty(q0);
                    u8 = F8;
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(u8&4) {
                        u8 = sse_setround(dyn, ninst, x1, x2, x3);
                        VFRINTIDQ(q0, q1);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        VFRINTRDQ(q0, q1, round_round[u8&3]);
                    }
                    break;
                case 0x0A:
                    INST_NAME("ROUNDSS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEXSS(q1, 0, 1);
                    u8 = F8;
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(u8&4) {
                        u8 = sse_setround(dyn, ninst, x1, x2, x3);
                        FRINTXS(v1, q1);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FRINTRRS(v1, q1, round_round[u8&3]);
                    }
                    VMOVeS(q0, 0, v1, 0);
                    break;
                case 0x0B:
                    INST_NAME("ROUNDSD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEXSD(q1, 0, 1);
                    u8 = F8;
                    v1 = fpu_get_scratch(dyn, ninst);
                    if(u8&4) {
                        u8 = sse_setround(dyn, ninst, x1, x2, x3);
                        FRINTXD(v1, q1);
                        x87_restoreround(dyn, ninst, u8);
                    } else {
                        FRINTRRD(v1, q1, round_round[u8&3]);
                    }
                    VMOVeD(q0, 0, v1, 0);
                    break;
                case 0x0C:
                    INST_NAME("BLENDPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 1);
                    u8 = F8&0b1111;
                    if((u8&0b0011)==0b0011) {
                        VMOVeD(q0, 0, q1, 0);
                        u8&=~0b0011;
                    }
                    if((u8&0b1100)==0b1100) {
                        VMOVeD(q0, 1, q1, 1);
                        u8&=~0b1100;
                    }
                    for(int i=0; i<4; ++i)
                        if(u8&(1<<i)) {
                            VMOVeS(q0, i, q1, i);
                        }
                    break;
                case 0x0D:
                    INST_NAME("BLENDPD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 1);
                    u8 = F8&0b11;
                    if(u8==0b01) {
                        VMOVeD(q0, 0, q1, 0);
                    } else if(u8==0b10) {
                        VMOVeD(q0, 1, q1, 1);
                    } else if(u8==0b11) {
                        VMOVQ(q0, q1);
                    }
                    break;
                case 0x0E:
                    INST_NAME("PBLENDW Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(v0, 1);
                    GETEX(v1, 0, 1);
                    u8 = F8;
                    q0 = fpu_get_scratch(dyn, ninst);
                    MOVI_64(q0, u8);
                    SXTL_8(q0, q0);    // expand 8bits to 16bits...
                    VBITQ(v0, v1, q0);
                    break;
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 1);
                    u8 = F8;
                    if(u8>31) {
                        VEORQ(q0, q0, q0);
                    } else if(u8>15) {
                        d0 = fpu_get_scratch(dyn, ninst);
                        VEORQ(d0, d0, d0);
                        VEXTQ_8(q0, q0, d0, u8-16);
                    } else {
                        VEXTQ_8(q0, q1, q0, u8);
                    }
                    break;

                case 0x14:
                    INST_NAME("PEXTRB Ed, Gx, Ib");
                    nextop = F8;
                    GETGX(q0, 0);
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8;
                        VMOVBto(ed, q0, (u8&15));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        u8 = F8;
                        VST1_8(q0, (u8&15), wback);
                        SMWRITE2();
                    }
                    break;
                case 0x15:
                    INST_NAME("PEXTRW Ed, Gx, Ib");
                    nextop = F8;
                    GETGX(q0, 0);
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8;
                        VMOVHto(ed, q0, (u8&7));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        u8 = F8;
                        VST1_16(q0, (u8&7), wback);
                        SMWRITE2();
                    }
                    break;
                case 0x16:
                    if(rex.w) {INST_NAME("PEXTRQ Ed, Gx, Ib");} else {INST_NAME("PEXTRD Ed, Gx, Ib");}
                    nextop = F8;
                    GETGX(q0, 0);
                    if(MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8;
                        if(rex.w) {
                            VMOVQDto(ed, q0, (u8&1));
                        } else {
                            VMOVSto(ed, q0, (u8&3));
                        }
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        u8 = F8;
                        if(rex.w) {
                            VST1_64(q0, (u8&1), wback);
                        } else {
                            VST1_32(q0, (u8&3), wback);
                        }
                        SMWRITE2();
                    }
                    break;
                case 0x17:
                    INST_NAME("EXTRACTPS Ew, Gx, Ib");
                    nextop = F8;
                    GETGX(q0, 0);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        u8 = F8&0b11;
                        VMOVSto(ed, q0, u8);    // will zero extend
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 1);
                        u8 = F8&0b11;
                        VMOVSto(x1, q0, u8);
                        STW(x1, wback, fixedaddress);
                        SMWRITE2();
                    }
                    break;
                case 0x20:
                    INST_NAME("PINSRB Gx, ED, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    if (MODREG) {
                        ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 1);
                        LDB(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    VMOVQBfrom(q0, (u8&15), ed);
                    break;
                case 0x21:
                    INST_NAME("INSERTPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    d0 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(d0, q0);
                    if (MODREG) {
                        q1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                        u8 = F8;
                        VMOVeS(q0, (u8>>4)&3, q1, (u8>>6)&3);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 1);
                        u8 = F8;
                        LDW(x2, wback, fixedaddress);
                        VMOVQSfrom(q0, (u8>>4)&3, x2);
                    }
                    uint8_t zmask = u8 & 0xf;
                    for (uint8_t i=0; i<4; i++) {
                        if (zmask & (1<<i)) {
                            VMOVQSfrom(q0, i, wZR);
                        }
                    }
                    break;
                case 0x22:
                    INST_NAME("PINSRD Gx, ED, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETED(1);
                    u8 = F8;
                    if(rex.w) {
                        VMOVQDfrom(q0, (u8&1), ed);
                    } else {
                        VMOVQSfrom(q0, (u8&3), ed);
                    }
                    break;

                case 0x40:
                    INST_NAME("DPPS Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 1);
                    u8 = F8;
                    v0 = fpu_get_scratch(dyn, ninst);
                    VFMULQS(v0, q0, q1);
                    // mask some (considering masking is rare)
                    for(int i=0; i<4; ++i)
                        if(!(u8&(1<<(4+i)))) {
                            VMOVQSfrom(v0, i, xZR);
                        }
                    VFADDPQS(v0, v0, v0);
                    FADDPS(v0, v0);
                    VDUPQ_32(q0, v0, 0);
                    for(int i=0; i<4; ++i)
                        if(!(u8&(1<<i))) {
                            VMOVQSfrom(q0, i, xZR);
                        }
                    break;

                case 0x41:
                    INST_NAME("DPPD Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0, 1);
                    u8 = F8;
                    v0 = fpu_get_scratch(dyn, ninst);
                    VFMULQD(v0, q0, q1);
                    // mask some, duplicate all, mask some
                    for(int i=0; i<2; ++i)
                        if(!(u8&(1<<(4+i)))) {
                            VMOVQDfrom(v0, i, xZR);
                        }
                    FADDPD(v0, v0);
                    VDUPQ_64(q0, v0, 0);
                    for(int i=0; i<2; ++i)
                        if(!(u8&(1<<i))) {
                            VMOVQDfrom(q0, i, xZR);
                        }
                    break;

                case 0x44:
                    INST_NAME("PCLMULQDQ Gx, Ex, Ib");
                    nextop = F8;
                    if(cpuext.pmull) {
                        GETGX(q0, 1);
                        GETEX(q1, 0, 1);
                        u8 = F8;
                        switch (u8&0b00010001) {
                            case 0b00000000:
                                PMULL_128(q0, q0, q1);
                                break;
                            case 0b00010001:
                                PMULL2_128(q0, q0, q1);
                                break;
                            case 0b00000001:
                                VEXTQ_8(q0, q0, q0, 8); // Swap Up/Lower 64bits parts
                                PMULL_128(q0, q0, q1);
                                break;
                            case 0b00010000:
                                VEXTQ_8(q0, q0, q0, 8); // Swap Up/Lower 64bits parts
                                PMULL2_128(q0, q0, q1);
                                break;
                        }
                    } else {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd);
                        MOV32w(x1, gd); // gx
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            sse_forget_reg(dyn, ninst, ed);
                            MOV32w(x2, ed);
                            MOV32w(x3, 0);  //p = NULL
                        } else {
                            MOV32w(x2, 0);
                            addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                            if(ed!=x3) {
                                MOVx_REG(x3, ed);
                            }
                        }
                        u8 = F8;
                        MOV32w(x4, u8);
                        CALL(const_native_pclmul, -1);
                    }
                    break;

                case 0x60:
                    INST_NAME("PCMPESTRM Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF);
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDx_U12(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if(MODREG) {
                        ed = (nextop&7)+(rex.b<<3);
                        if(ed>7)
                            sse_reflect_reg(dyn, ninst, ed);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                    }
                    MOVx_REG(x2, xRDX);
                    MOVx_REG(x4, xRAX);
                    u8 = F8;
                    MOV32w(x5, u8);
                    CALL(const_sse42_compare_string_explicit_len, x1);
                    q0 = sse_get_reg_empty(dyn, ninst, x2, 0);
                    if(u8&0b1000000) {
                        q1 = fpu_get_scratch(dyn, ninst);
                        switch(u8&1) {
                            case 0b00:
                                VDUPQB(q0, x1); // load the low 8bits of the mask
                                LSRw_IMM(x1, x1, 8);
                                VDUPQB(q1, x1); // load the high 8bits of the mask
                                VEXTQ_8(q0, q0, q1, 8); // low and hig bits mask
                                TABLE64C(x2, const_8b_7_6_5_4_3_2_1_0);
                                VLDR64_U12(q1, x2, 0);     // load shift
                                VDUPQ_64(q1, q1, 0);
                                USHLQ_8(q0, q0, q1); // extract 1 bit
                                MOVIQ_8(q1, 0x80);   // load mask
                                VANDQ(q0, q0, q1);
                                VSSHRQ_8(q0, q0, 7);    // saturate the mask
                                break;
                            case 0b01:
                                VDUPQH(q0, x1); // load the 8bits of the mask
                                TABLE64C(x2, const_8b_15_14_13_12_11_10_9_8);
                                VLDR64_U12(q1, x2, 0);     // load shift
                                UXTL_8(q1, q1);     // extend mask to 16bits
                                USHLQ_16(q0, q0, q1); // extract 1 bit
                                MOVIQ_16(q1, 0x80, 1);   // load mask
                                VANDQ(q0, q0, q1);
                                VSSHRQ_16(q0, q0, 15);    // saturate the mask
                        }
                    } else {
                        VEORQ(q0, q0, q0);
                        VMOVQHfrom(q0, 0, x1);
                    }
                    break;
                case 0x61:
                    INST_NAME("PCMPESTRI Gx, Ex, Ib");
                    nextop = F8;
                    GETG;
                    u8 = geted_ib(dyn, addr, ninst, nextop);
                    if((u8&0b1100)==0b1000) {
                        SETFLAGS(X_ALL, SF_SET);
                        // this case is (un)signed word, equal each
                        GETGX(v0, 0);
                        GETEX(v1, 0, 1);
                        u8 = F8;
                        q0 = fpu_get_scratch(dyn, ninst);
                        if(u8&1) {
                            //16bits
                            VCMEQQ_16(q0, v0, v1);   // equal => mask regs
                            XTN_8(q0, q0);          // 8 bits mask, in lower 64bits
                            // transform that a mask in x1
                            q1 = fpu_get_scratch(dyn, ninst);
                            VSHL_8(q0, q0, 7);  // keep only bit 0x80
                            TABLE64C(x1, const_8b_m7_m6_m5_m4_m3_m2_m1_0);
                            VLDR64_U12(q1, x1, 0);     // load shift
                            USHL_8(q0, q0, q1); // shift
                            UADDLV_8(q0, q0);   // accumulate
                            VMOVBto(x1, q0, 0);
                        } else {
                            //8 bits
                            VCMEQQ_8(q0, v0, v1);   // equal => mask regs
                            // transform that a mask in x1
                            q1 = fpu_get_scratch(dyn, ninst);
                            d0 = fpu_get_scratch(dyn, ninst);
                            VSHL_8(d0, q0, 7);  // keep only bit 0x80
                            TABLE64C(x1, const_8b_m7_m6_m5_m4_m3_m2_m1_0);
                            VLDR64_U12(q1, x1, 0);     // load shift
                            USHL_8(d0, d0, q1); // shift
                            UADDLV_8(d0, d0);   // accumulate
                            VMOVBto(x1, d0, 0);
                            // high part
                            VMOVeD(d0, 0, q0, 1);
                            VSHL_8(d0, d0, 7);  // keep only bit 0x80
                            USHL_8(d0, d0, q1); // shift
                            UADDLV_8(d0, d0);   // accumulate
                            VMOVBto(x2, d0, 0);
                            BFIw(x1, x2, 8, 8); // insert
                        }
                        // get abs of eax / edx and find min
                        ADDSxw_U12(x2, xRAX, 0);
                        Bcond(cPL, 4+4);
                        NEGxw_REG(x2, x2);
                        ADDSxw_U12(x3, xRDX, 0);
                        Bcond(cPL, 4+4);
                        NEGxw_REG(x3, x3);
                        MOV32w(x4, (u8&1)?8:16);
                        CMPSw_REG(x3, x4);
                        CSELw(x3, x3, x4, cLT); // x3 is lmem
                        CMPSw_REG(x2, x4);
                        CSELw(x6, x2, x4, cLT); // x6 is lreg
                        CMPSw_REG(x6, x3);
                        CSELw(x5, x3, x6, cLT); // x5 is max(lmem, lreg)
                        CSELw(x2, x6, x3, cLT); // x2 is min(lmem, lreg)
                        // x2 is min length 0-n_packed
                        MVNw_REG(x4, xZR);
                        LSLw_REG(x87pc, x4, x2);
                        BICw_REG(x1, x1, x87pc);
                        LSLw_REG(x4, x4, x5);
                        ORRw_REG(x1, x1, x4);
                        ANDw_mask(x1, x1, 0, (u8&1)?7:15);
                        // x1 is intres1, transform to intres2
                        switch((u8>>4)&3) {
                            case 0b01:
                                MOV32w(x4, (1<<((u8&1)?8:16))-1);
                                EORw_REG(x1, x1, x4);
                                break;
                            case 0b11:
                                MOV32w(x4, 1);
                                LSLw_REG(x4, x4, x3);
                                SUBw_U12(x4, x4, 1);
                                EORw_REG(x1, x1, x4);
                        }
                        // flags
                        IFX(X_ALL) {
                            SET_DFNONE();
                            IFX(X_CF) {
                                CMPSw_REG(x1, xZR);
                                CSETw(x4, cNE);
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                            IFX(X_ZF|X_SF) {
                                MOV32w(x4, (u8&1)?8:16);
                                IFX(X_ZF) {
                                    CMPSw_REG(x3, x4);
                                    CSETw(x5, cLT);
                                    BFIw(xFlags, x5, F_ZF, 1);
                                }
                                IFX(X_SF) {
                                    CMPSw_REG(x6, x4);
                                    CSETw(x5, cLT);
                                    BFIw(xFlags, x5, F_SF, 1);
                                }
                            }
                            IFX(X_OF) {
                                BFIw(xFlags, x1, F_OF, 1);
                            }
                            IFX(X_AF) {
                                BFCw(wFlags, F_AF, 1);
                                /*CMPSw_U12(x1, 0);
                                CSETw(x4, cEQ);
                                CMPSw_U12(x3, (u8&1)?8:16);
                                CSETw(x5, cEQ);
                                ANDw_REG(x4, x4, x5);
                                BFIw(xFlags, x4, F_AF, 1);*/
                            }
                            IFX(X_PF) {
                                BFCw(xFlags, F_PF, 1);
                            }
                        }
                        NATIVE_RESTORE_X87PC();
                    } else {
                        SETFLAGS(X_ALL, SF_SET_DF);
                        if(gd>7)    // no need to reflect cache as xmm0-xmm7 will be saved before the function call anyway
                            sse_reflect_reg(dyn, ninst, gd);
                        ADDx_U12(x3, xEmu, offsetof(x64emu_t, xmm[gd]));
                        if(MODREG) {
                            ed = (nextop&7)+(rex.b<<3);
                            if(ed>7)
                                sse_reflect_reg(dyn, ninst, ed);
                            ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                            if(ed!=x1) {
                                MOVx_REG(x1, ed);
                            }
                        }
                        MOVw_REG(x2, xRDX);
                        MOVw_REG(x4, xRAX);
                        u8 = F8;
                        MOV32w(x5, u8);
                        CALL(const_sse42_compare_string_explicit_len, x1);
                    }
                    if(u8&0b1000000) {
                        CBNZw_MARK(x1);
                        MOV32w(xRCX, (u8&1)?8:16);
                        B_NEXT_nocond;
                        MARK;
                        CLZw(xRCX, x1);
                        MOV32w(x2, 31);
                        SUBw_REG(xRCX, x2, xRCX);
                    } else {
                        ORRw_mask(xRCX, x1, (u8&1)?0b011000:0b010000,0);
                        RBITw(xRCX, xRCX);
                        CLZw(xRCX, xRCX);
                    }
                    break;
                case 0x62:
                    INST_NAME("PCMPISTRM Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF);
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if(MODREG) {
                        ed = (nextop&7)+(rex.b<<3);
                        if(ed>7)
                            sse_reflect_reg(dyn, ninst, ed);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x3, u8);
                    CALL(const_sse42_compare_string_implicit_len, x1);
                    q0 = sse_get_reg_empty(dyn, ninst, x2, 0);
                    if(u8&0b1000000) {
                        q1 = fpu_get_scratch(dyn, ninst);
                        switch(u8&1) {
                            case 0b00:
                                VDUPQB(q0, x1); // load the low 8bits of the mask
                                LSRw_IMM(x1, x1, 8);
                                VDUPQB(q1, x1); // load the high 8bits of the mask
                                VEXTQ_8(q0, q0, q1, 8); // low and hig bits mask
                                TABLE64C(x2, const_8b_7_6_5_4_3_2_1_0);
                                VLDR64_U12(q1, x2, 0);     // load shift
                                VDUPQ_64(q1, q1, 0);
                                USHLQ_8(q0, q0, q1); // extract 1 bit
                                MOVIQ_8(q1, 0x80);   // load mask
                                VANDQ(q0, q0, q1);
                                VSSHRQ_8(q0, q0, 7);    // saturate the mask
                                break;
                            case 0b01:
                                VDUPQH(q0, x1); // load the 8bits of the mask
                                TABLE64C(x2, const_8b_15_14_13_12_11_10_9_8);
                                VLDR64_U12(q1, x2, 0);     // load shift
                                UXTL_8(q1, q1);     // extend mask to 16bits
                                USHLQ_16(q0, q0, q1); // extract 1 bit
                                MOVIQ_16(q1, 0x80, 1);   // load mask
                                VANDQ(q0, q0, q1);
                                VSSHRQ_16(q0, q0, 15);    // saturate the mask
                        }
                    } else {
                        VEORQ(q0, q0, q0);
                        VMOVQHfrom(q0, 0, x1);
                    }
                    break;
                case 0x63:
                    INST_NAME("PCMPISTRI Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF);
                    nextop = F8;
                    GETG;
                    if(gd>7)
                        sse_reflect_reg(dyn, ninst, gd);
                    ADDx_U12(x2, xEmu, offsetof(x64emu_t, xmm[gd]));
                    if(MODREG) {
                        ed = (nextop&7)+(rex.b<<3);
                        if(ed>7)
                            sse_reflect_reg(dyn, ninst, ed);
                        ADDx_U12(x1, xEmu, offsetof(x64emu_t, xmm[ed]));
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x3, u8);
                    CALL(const_sse42_compare_string_implicit_len, x1);
                    CBNZw_MARK(x1);
                    MOV32w(xRCX, (u8&1)?8:16);
                    B_NEXT_nocond;
                    MARK;
                    if(u8&0b1000000) {
                        CLZw(xRCX, x1);
                        MOV32w(x2, 31);
                        SUBw_REG(xRCX, x2, xRCX);
                    } else {
                        RBITxw(xRCX, x1);
                        CLZw(xRCX, xRCX);
                    }
                    break;

                case 0xDF:
                    INST_NAME("AESKEYGENASSIST Gx, Ex, Ib");  // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd);
                    MOV32w(x1, gd); // gx
                    if(MODREG) {
                        ed = (nextop&7)+(rex.b<<3);
                        sse_forget_reg(dyn, ninst, ed);
                        MOV32w(x2, ed);
                        MOV32w(x3, 0);  //p = NULL
                    } else {
                        MOV32w(x2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        if(ed!=x3) {
                            MOVx_REG(x3, ed);
                        }
                    }
                    u8 = F8;
                    MOV32w(x4, u8);
                    CALL(const_native_aeskeygenassist, -1);
                    break;

                default:
                    DEFAULT;
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)                                                                                   \
            READFLAGS(F);                                                                                                  \
            GETFLAGS;                                                                                                      \
            nextop = F8;                                                                                                   \
            GETGD;                                                                                                         \
            if (MODREG) {                                                                                                  \
                ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                  \
            } else {                                                                                                       \
                SMREAD();                                                                                                  \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff << 1, 1, rex, NULL, 0, 0); \
                LDH(x1, ed, fixedaddress);                                                                                 \
                ed = x1;                                                                                                   \
            }                                                                                                              \
            Bcond(NO, +8);                                                                                                 \
            BFIx(gd, ed, 0, 16);

        GOCOND(0x40, "CMOV", "Gw, Ew");
        #undef GO

        case 0x50:
            nextop = F8;
            INST_NAME("MOVMSKPD Gd, Ex");
            GETEX(q0, 0, 0);
            GETGD;
            VMOVQDto(x1, q0, 0);
            VMOVQDto(gd, q0, 1);
            LSRx(gd, gd, 62);
            BFXILx(gd, x1, 63, 1);
            break;
        case 0x51:
            INST_NAME("SQRTPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX_empty(q1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFCMEQQD(v0, q0, q0);    // 0 if NAN, 1 if not NAN
                VFSQRTQD(q1, q0);
                VFCMEQQD(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            } else {
                VFSQRTQD(q1, q0);
            }
            break;

        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VANDQ(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VBICQ(v0, q0, v0);
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            VORRQ(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG && ((nextop&7)+(rex.b<<3)==gd)) {
                // special case for XORPD Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VEORQ(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VEORQ(q0, q0, q1);
            }
            break;
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQD(v0, q0, q1);    // propagate NAN
                VFCMEQQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFADDQD(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQD(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQD(v0, q0, q1);    // propagate NAN
                VFCMEQQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFMULQD(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQD(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if(BOX64ENV(dynarec_fastround)==2) {
                FCVTXN(v0, v1);
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                FCVTN(v0, v1);
                x87_restoreround(dyn, ninst, u8);
            }
            break;
        case 0x5B:
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if(BOX64ENV(dynarec_fastround)) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                VFRINTISQ(v0, v1);
                x87_restoreround(dyn, ninst, u8);
                VFCVTZSQS(v0, v0);
            } else {
                if(cpuext.frintts) {
                    u8 = sse_setround(dyn, ninst, x1, x2, x3);
                    VFRINT32XSQ(v0, v1); // handle overflow
                    VFCVTZSQS(v0, v0);
                } else {
                    MRS_fpsr(x5);
                    u8 = sse_setround(dyn, ninst, x1, x2, x3);
                    MOV32w(x4, 0x80000000);
                    d0 = fpu_get_scratch(dyn, ninst);
                    for(int i=0; i<4; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        if(i) {
                            VMOVeS(d0, 0, v1, i);
                            FRINTIS(d0, d0);
                        } else {
                            FRINTIS(d0, v1);
                        }
                        VFCVTZSs(d0, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        VMOVQSfrom(d0, 0, x4);
                        VMOVeS(v0, i, d0, 0);
                    }
                }
                x87_restoreround(dyn, ninst, u8);
            }
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQD(v0, q0, q1);    // propagate NAN
                VFCMEQQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFSUBQD(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQD(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x5D:
            INST_NAME("MINPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // FMIN/FMAX wll not copy the value if v0[x] is NaN
            // but x86 will copy if either v0[x] or v1[x] is NaN, so lets force a copy if source is NaN
            q0 = fpu_get_scratch(dyn, ninst);
            VFCMGTQD(q0, v1, v0);   // 0 is NaN or v1 GT v0, so invert mask for copy
            VBIFQ(v0, v1, q0);
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(q1, 1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQD(v0, q0, q1);    // propagate NAN
                VFCMEQQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFDIVQD(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQD(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x5F:
            INST_NAME("MAXPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // FMIN/FMAX wll not copy the value if v0[x] is NaN
            // but x86 will copy if either v0[x] or v1[x] is NaN, or if values are equals, so lets force a copy if source is NaN
            q0 = fpu_get_scratch(dyn, ninst);
            VFCMGTQD(q0, v0, v1);   // 0 is NaN or v0 GT v1, so invert mask for copy
            VBIFQ(v0, v1, q0);
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VZIP1Q_8(v0, v0, q0);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VZIP1Q_16(v0, v0, q0);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VZIP1Q_32(v0, v0, q0);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            SQXTN_8(q0, q0);
            if(q0==q1) {
                VMOVeD(q0, 1, q0, 0);
            } else {
                SQXTN2_8(q0, q1);
            }
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMGTQ_8(v0, v0, v1);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMGTQ_16(v0, v0, v1);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMGTQ_32(v0, v0, v1);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            SQXTUN_8(v0, v0);
            if(v0==v1) {
                VMOVeD(v0, 1, v0, 0);
            } else {
                SQXTUN2_8(v0, v1);
            }
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VZIP2Q_8(q0, q0, q1);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VZIP2Q_16(q0, q0, q1);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VZIP2Q_32(q0, q0, q1);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            SQXTN_16(v0, v0);
            if(v0==v1) {
                VMOVeD(v0, 1, v0, 0);
            } else {
                SQXTN2_16(v0, v1);
            }
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                VMOVeD(v0, 1, v1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLD1_64(v0, 1, ed);
            }
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            VMOVeD(v0, 0, v0, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                if(v0!=v1) {
                    VMOVeD(v0, 1, v1, 1);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                ADDSx_U12(x1, ed, 8);
                VLD1_64(v0, 1, x1);
            }
            break;
        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            GETGX_empty(v0);
            GETED(0);
            if(rex.w) {
                FMOVDx(v0, ed);
            } else {
                VEORQ(v0, v0, v0); // RAZ vector
                VMOVQSfrom(v0, 0, ed);
            }
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx, Ex");
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
            INST_NAME("PSHUFD Gx, Ex,Ib");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                v1 = fpu_get_scratch(dyn, ninst);      
            }
            u8 = F8;
            GETGX_empty(v0);
            switch(u8) {
            case 0x4E: // Swap Up/Lower 64bits parts
                if(!MODREG)
                    VLDR128_U12(v1, wback, 0);
                VEXTQ_8(v0, v1, v1, 8);
                break;
            case 0x00: // duplicate lower 32bits to all spot
                if(MODREG)
                    VDUPQ_32(v0, v1, 0);
                else
                    VLDQ1R_32(v0, wback);
                break;
            case 0x55: // duplicate slot 1 to all spot
                if(MODREG)
                    VDUPQ_32(v0, v1, 1);
                else {
                    ADDx_U12(x2, wback, 4);
                    VLDQ1R_32(v0, x2);
                }
                break;
            case 0xAA: // duplicate slot 2 to all spot
                if(MODREG)
                    VDUPQ_32(v0, v1, 2);
                else {
                    ADDx_U12(x2, wback, 8);
                    VLDQ1R_32(v0, x2);
                }
                break;
            case 0xFF: // duplicate slot 3 to all spot
                if(MODREG)
                    VDUPQ_32(v0, v1, 3);
                else {
                    ADDx_U12(x2, wback, 12);
                    VLDQ1R_32(v0, x2);
                }
                break;
            case 0x44: // duplicate slot 0/1 to all spot
                if(MODREG)
                    VDUPQ_64(v0, v1, 0);
                else
                    VLDQ1R_64(v0, wback);
                break;
            case 0xEE: // duplicate slot 2/3 to all spot
                if(MODREG)
                    VDUPQ_64(v0, v1, 1);
                else {
                    ADDx_U12(x2, wback, 8);
                    VLDQ1R_64(v0, x2);
                }
                break;
            case 0xB1: // invert 0/1 and 2/3
                if(!MODREG)
                    VLDR128_U12(v1, wback, 0);
                VREV64Q_32(v0, v1);
                break;
            case 0x39: // 0 3 2 1: ror 32 bits
                if(!MODREG)
                    VLDR128_U12(v1, wback, 0);
                VEXTQ_8(v0, v1, v1, 4);
                break;
            case 0x93: // 2 1 0 3: 0 3 2 1: ror 32 bits then invert low/high 64bits
                if(!MODREG)
                    VLDR128_U12(v1, wback, 0);
                VEXTQ_8(v0, v1, v1, 4);
                VEXTQ_8(v0, v0, v0, 8);
                break;
            default:
                if(!MODREG) {
                    i32 = -1;
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            ADDx_U12(x2, wback, idx*4);
                            i32 = idx;
                        }
                        VLD1_32(v0, i, x2);
                    }
                } else if(v0!=v1) {
                    VMOVeS(v0, 0, v1, (u8>>(0*2))&3);
                    VMOVeS(v0, 1, v1, (u8>>(1*2))&3);
                    VMOVeS(v0, 2, v1, (u8>>(2*2))&3);
                    VMOVeS(v0, 3, v1, (u8>>(3*2))&3);
                } else {
                    uint64_t swp[4] = {
                        (0)|(1<<8)|(2<<16)|(3<<24),
                        (4)|(5<<8)|(6<<16)|(7<<24),
                        (8)|(9<<8)|(10<<16)|(11<<24),
                        (12)|(13<<8)|(14<<16)|(15<<24)
                    };
                    d0 = fpu_get_scratch(dyn, ninst);
                    tmp64u = swp[(u8>>(0*2))&3] | (swp[(u8>>(1*2))&3]<<32);
                    MOV64x(x2, tmp64u);
                    VMOVQDfrom(d0, 0, x2);
                    tmp64u2 = swp[(u8>>(2*2))&3] | (swp[(u8>>(3*2))&3]<<32);
                    if(tmp64u2==tmp64u) {
                        VMOVQDfrom(d0, 1, x2);
                    } else {
                        MOV64x(x3, tmp64u2);
                        VMOVQDfrom(d0, 1, x3);
                    }
                    VTBLQ1_8(v0, v1, d0);
                }
            }
            break;
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_16(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8>15) u8=15;
                    if(u8) {
                        VSSHRQ_16(q0, q0, u8);
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_16(q0, q0, u8);
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
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_32(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8>31) u8=31;
                    if(u8) {
                        VSSHRQ_32(q0, q0, u8);
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_32(q0, q0, u8);
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
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_64(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if(u8>15) {
                            VEORQ(q0, q0, q0);
                        } else {
                            q1 = fpu_get_scratch(dyn, ninst);
                            VEORQ(q1, q1, q1);
                            VEXTQ_8(q0, q0, q1, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_64(q0, q0, u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if(u8) {
                        if(u8>15) {
                            VEORQ(q0, q0, q0);
                        } else if(u8>0) {
                            q1 = fpu_get_scratch(dyn, ninst);
                            VEORQ(q1, q1, q1);
                            VEXTQ_8(q0, q1, q0, 16-u8);
                        }
                        PUTEX(q0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x74:
            INST_NAME("PCMPEQB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VCMEQQ_8(v0, v0, q0);
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VCMEQQ_16(v0, v0, q0);
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VCMEQQ_32(v0, v0, q0);
            break;

        case 0x7C:
            INST_NAME("HADDPD Gx, Ex");
            nextop = F8;
            GETGX(q1, 1);
            GETEX(q0, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXPQD(v0, q1, q0);    // propagate NAN
                VFCMEQQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFADDPQD(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQD(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x7D:
            INST_NAME("HSUBPD Gx, Ex");  // SSE4 opcode!
            nextop = F8;
            GETEX(q1, 0, 0);
            GETGX(q0, 1);
            v0 = fpu_get_scratch(dyn, ninst);
            VUZP1Q_64(v0, q0, q1);
            VUZP2Q_64(q0, q0, q1);
            if(!BOX64ENV(dynarec_fastnan)) {
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQD(v1, v0, q0);    // propagate NAN
                VFCMEQQD(v1, v1, v1);    // 0 if NAN, 1 if not NAN
            }
            VFSUBQD(q0, v0, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQD(v0, q0, q0);    // 0 => out is NAN
                VBICQ(v1, v1, v0);      // forget it in any input was a NAN alreavy
                VSHLQ_64(v1, v1, 63);   // only keep the sign bit
                VORRQ(q0, q0, v1);      // NAN -> -NAN
            }
            break;
        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(rex.w) {
                if(MODREG) {
                    ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                    VMOVQDto(ed, v0, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                    VST64(v0, ed, fixedaddress);
                    SMWRITE2();
                }
            } else {
                if(MODREG) {
                    ed = TO_NAT((nextop & 0x07) + (rex.b << 3));
                    VMOVSto(ed, v0, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<2, 3, rex, NULL, 0, 0);
                    VST32(v0, ed, fixedaddress);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0xA3:
            INST_NAME("BT Ew, Gw");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); // GETGD
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<2, (1<<2)-1, rex, NULL, 0, 0);
                SBFXx(x1, gd, 4, 12);   // r1 = (gw>>4)
                ADDx_REG_LSL(x3, wback, x1, 1); //(&ed)+=r1*2;
                LDH(x1, x3, fixedaddress);
                ed = x1;
            }
            IFX(X_CF) {
                ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
                LSRw_REG(x1, ed, x2);
                BFIw(xFlags, x1, F_CF, 1);
            }
            break;
        case 0xA4:
            INST_NAME("SHLD Ew, Gw, Ib");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
            if(u8) {
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETEW(x1, 1);
                GETGW(x2);
                u8 = F8;
                emit_shld16c(dyn, ninst, ed, gd, u8, x4, x5);
                EWBACK;
            } else {
                FAKEED;
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ew, Gw, CL");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING);
            GETGW(x2);
            GETEW(x1, 0);
            ANDw_mask(x4, xRCX, 0, 0b00100);  //mask=0x00000001f
            UFLAG_IF {
                CBZw_NEXT(x4);
            }
            emit_shld16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); // GETGD
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<2, (1<<2)-1, rex, NULL, 0, 0);
                SBFXx(x4, gd, 4, 12);   // r1 = (gw>>4)
                ADDx_REG_LSL(x3, wback, x4, 1); //(&ed)+=r1*2;
                LDH(x4, x3, fixedaddress);
                wback = x3;
                ed = x4;
            }
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            IFX(X_CF) {
                LSRw_REG(x1, ed, x2);
                BFIw(xFlags, x1, F_CF, 1);
            }
            MOV32w(x1, 1);
            LSLw_REG(x1, x1, x2);
            ORRx_REG(ed, ed, x1);
            if(wback) {
                STH(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ew, Gw, Ib");
            u8 = geted_ib(dyn, addr, ninst, nextop)&0x1f;
            if(u8) {
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETEW(x1, 1);
                GETGW(x2);
                u8 = F8;
                emit_shrd16c(dyn, ninst, ed, gd, u8, x4, x5);
                EWBACK;
            } else {
                FAKEED;
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ew, Gw, CL");
            if(BOX64DRENV(dynarec_safeflags)>1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING);
            GETGW(x2);
            GETEW(x1, 0);
            ANDw_mask(x4, xRCX, 0, 0b00100);  //mask=0x00000001f
            UFLAG_IF {
                CBZw_NEXT(x4);
            } else {
            }
            emit_shrd16(dyn, ninst, ed, gd, x4, x5, x6);
            EWBACK;
            break;
        case 0xAE:
            nextop = F8;
            if(MODREG)
                switch (nextop) {
                    default:
                        DEFAULT;
                }
            else
                switch((nextop>>3)&7) {
                    case 6:
                        INST_NAME("CLWB Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (CLWB)?\n");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                        CALL_(const_native_clflush, -1, 0);
                        break;
                    case 7:
                        INST_NAME("CLFLUSHOPT Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization (CLFLUSHOPT)?\n");
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
            INST_NAME("IMUL Gw,Ew");
            if(BOX64ENV(dynarec_safeflags) && BOX64ENV(cputype)) {
                SETFLAGS(X_OF|X_CF, SF_SET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            nextop = F8;
            GETSEW(x1, 0);
            GETSGW(x2);
            MULw(x2, x2, x1);
            GWBACK;
            SET_DFNONE();
            IFX(X_CF|X_OF) {
                ASRw(x1, x2, 15);
                CMPSw_REG_ASR(x1, x2, 31);
                CSETw(x3, cNE);
                IFX(X_CF) {
                    BFIw(xFlags, x3, F_CF, 1);
                }
                IFX(X_OF) {
                    BFIw(xFlags, x3, F_OF, 1);
                }
            }
            IFX2(X_AF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_AF, 1);}
            IFX2(X_ZF, && !BOX64ENV(cputype)) {BFCw(xFlags, F_ZF, 1);}
            IFX2(X_SF, && !BOX64ENV(cputype)) {
                LSRxw(x3, gd, 15);
                BFIw(xFlags, x3, F_SF, 1);
            }
            IFX2(X_PF, && !BOX64ENV(cputype)) emit_pf(dyn, ninst, gd, x3);
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); // GETGD
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<2, (1<<2)-1, rex, NULL, 0, 0);
                SBFXx(x4, gd, 4, 12);   // r1 = (gw>>4)
                ADDx_REG_LSL(x3, wback, x4, 1); //(&ed)+=r1*2;
                LDH(x4, x3, fixedaddress);
                wback = x3;
                ed = x4;
            }
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            BICx_REG(ed, ed, x1);
            if(wback) {
                STH(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gw, Eb");
            nextop = F8;
            if(MODREG) {
                if(rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop&7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                }
                UBFXxw(x1, eb1, eb2*8, 8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 0);
                LDB(x1, ed, fixedaddress);
            }
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); // GETGW
            BFIx(gd, x1, 0, 16);        // insert in Gw
            break;
        case 0xB7:
            INST_NAME("MOVZX Gw, Ew");
            nextop = F8;
            if(MODREG) {
                eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                UBFXxw(x1, eb1, 0, 16);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff>>1, 1, rex, NULL, 0, 0);
                LDH(x1, ed, fixedaddress);
            }
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); // GETGW
            BFIx(gd, x1, 0, 16);        // insert in Gw
            break;


        case 0xBA:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                    INST_NAME("BT Ew, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    gd = x2;
                    GETEW(x1, 1);
                    u8 = F8;
                    u8&=rex.w?0x3f:0x0f;
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    break;
                case 5:
                    INST_NAME("BTS Ew, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    GETEW(x1, 1);
                    u8 = F8;
                    u8&=(rex.w?0x3f:0x0f);
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    mask = convert_bitmask_xw(1<<u8);
                    ORRxw_mask(ed, ed, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                    EWBACK;
                    break;
                case 6:
                    INST_NAME("BTR Ew, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    GETEW(x1, 1);
                    u8 = F8;
                    u8&=(rex.w?0x3f:0x0f);
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    BFCxw(ed, u8, 1);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("BTC Ew, Ib");
                    if(!BOX64ENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                    } else {
                        SETFLAGS(X_CF, SF_SUBSET);
                    }
                    GETEW(x1, 1);
                    u8 = F8;
                    u8&=(rex.w?0x3f:0x0f);
                    IFX(X_CF) {
                        BFXILxw(xFlags, ed, u8, 1);  // inject 1 bit from u8 to F_CF (i.e. pos 0)
                    }
                    mask = convert_bitmask_xw(1<<u8);
                    EORxw_mask(ed, ed, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                    EWBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            if(!BOX64ENV(dynarec_safeflags)) {
                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_CF, SF_SUBSET);
            }
            nextop = F8;
            gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); // GETGD
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff<<2, (1<<2)-1, rex, NULL, 0, 0);
                SBFXx(x4, gd, 4, 12);   // r1 = (gw>>4)
                ADDx_REG_LSL(x3, wback, x4, 1); //(&ed)+=r1*2;
                LDH(x4, x3, fixedaddress);
                wback = x3;
                ed = x4;
            }
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            IFX(X_CF) {
                LSRw_REG(x1, ed, x2);
                BFIw(xFlags, x1, F_CF, 1);
            }
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORx_REG(ed, ed, x1);
            if(wback) {
                STH(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gw,Ew");
            if(!BOX64ENV(dynarec_safeflags) || BOX64ENV(cputype)) {
                SETFLAGS(X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            SET_DFNONE();
            nextop = F8;
            GETGD;
            GETEW(x1, 0);  // Get EW
            IFX(X_ZF) {
                TSTw_REG(x1, x1);
                B_MARK(cEQ);
            } else {
                CBZw_MARK(x1);
            }
            RBITw(x1, x1);   // reverse
            CLZw(x1, x1);    // x2 gets leading 0 == BSF
            if(!MODREG) MARK;   // value gets written on 0 input only if input is a memory it seems
            BFIx(gd, x1, 0, 16);
            if(MODREG) MARK;
            IFX(X_ZF) {
                IFNATIVE(NF_EQ) {} else {
                    CSETw(x2, cEQ);    //ZF not set
                    BFIw(xFlags, x2, F_ZF, 1);
                }
            }
            if(BOX64ENV(dynarec_safeflags) && !BOX64ENV(cputype)) {
                IFX(X_CF) BFCw(xFlags, F_CF, 1);
                IFX(X_AF) BFCw(xFlags, F_AF, 1);
                IFX(X_SF) BFCw(xFlags, F_SF, 1);
                IFX(X_OF) BFCw(xFlags, F_OF, 1);
                IFX(X_PF) emit_pf(dyn, ninst, x1, x2);
            }
            break;
        case 0xBD:
            INST_NAME("BSR Gw,Ew");
            if(!BOX64ENV(dynarec_safeflags) || BOX64ENV(cputype)) {
                SETFLAGS(X_ZF, SF_SUBSET);
            } else {
                SETFLAGS(X_ALL, SF_SET);
            }
            SET_DFNONE();
            nextop = F8;
            GETGD;
            GETEW(x1, 0);  // Get EW
            IFX(X_ZF) {
                TSTw_REG(x1, x1);   // Don't use CBZ here, as the flag is reused later
                B_MARK(cEQ);
            } else {
                CBZw_MARK(x1);
            }
            LSLw(x1, x1, 16);   // put bits on top
            CLZw(x2, x1);       // x2 gets leading 0
            SUBw_U12(x2, x2, 15);
            NEGw_REG(x1, x2);   // complement
            if(!MODREG) MARK;
            BFIx(gd, x1, 0, 16);
            if(MODREG) MARK;
            IFX(X_ZF) {
                IFNATIVE(NF_EQ) {} else {
                    CSETw(x2, cEQ);    //ZF not set
                    BFIw(xFlags, x2, F_ZF, 1);
                }
            }
            if(BOX64ENV(dynarec_safeflags) && !BOX64ENV(cputype)) {
                IFX(X_CF) BFCw(xFlags, F_CF, 1);
                IFX(X_AF) BFCw(xFlags, F_AF, 1);
                IFX(X_SF) BFCw(xFlags, F_SF, 1);
                IFX(X_OF) BFCw(xFlags, F_OF, 1);
                IFX(X_PF) emit_pf(dyn, ninst, x1, x2);
            }
            break;
        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb1=ed;
                    eb2=0;
                } else {
                    ed = (nextop&7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                }
                SBFXw(x1, eb1, eb2*8, 8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, 0);
                LDSBw(x1, ed, fixedaddress);
            }
            BFIx(gd, x1, 0, 16);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gw, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                SXTHxw(x1, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, &unscaled, 0xfff<<1, 1, rex, NULL, 0, 0);
                LDSHxw(x1, ed, fixedaddress);
            }
            BFIx(gd, x1, 0, 16);
            break;

        case 0xC1:
            INST_NAME("XADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2, 0);
            BFIx(TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)), ed, 0, 16);
            emit_add16(dyn, ninst, ed, gd, x4, x5);
            EWBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            switch(u8&7) {
                // the inversion of the params in the comparison is there to handle NaN the same way SSE does
                case 0: VFCMEQQD(v0, v0, v1); break;   // Equal
                case 1: VFCMGTQD(v0, v1, v0); break;   // Less than
                case 2: VFCMGEQD(v0, v1, v0); break;   // Less or equal
                case 3: if(v0!=v1) {
                            VFMAXQD(v0, v0, v1);    // propagate NAN
                        }
                        VFCMEQQD(v0, v0, v0);
                        VMVNQ(v0, v0);
                        break;   // NaN (NaN is not equal to himself)
                case 4: VFCMEQQD(v0, v0, v1); VMVNQ(v0, v0); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: VFCMGTQD(v0, v1, v0); VMVNQ(v0, v0); break;   // Greater or equal or unordered
                case 6: VFCMGEQD(v0, v1, v0); VMVNQ(v0, v0); break;   // Greater or unordered
                case 7: if(v0!=v1) {
                            VFMAXQD(v0, v0, v1);    // propagate NAN
                        }
                        VFCMEQQD(v0, v0, v0);
                        break;   // not NaN
            }
            break;

        case 0xC4:
            INST_NAME("PINSRW Gx,Ed,Ib");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                u8 = (F8)&7;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                VMOVQHfrom(v0, u8, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = (F8)&7;
                VLD1_16(v0, u8, wback);
            }
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Ex,Ib");
            nextop = F8;
            GETGD;
            if(MODREG) {
                GETEX(v0, 0, 1);
                u8 = (F8)&7;
                VMOVHto(gd, v0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                u8 = (F8)&7;
                LDRH_U12(gd, wback, u8*2);
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            if(v0==v1 && u8==0) {
                VMOVeD(v0, 1, v0, 0);
            } else {
                if(v0==v1)
                    q0 = fpu_get_scratch(dyn, ninst);
                else
                    q0 = v0;
                VMOVeD(q0, 0, v0, (u8&1));
                VMOVeD(q0, 1, v1, ((u8>>1)&1));
                if(v0==v1) {
                    VMOVQ(v0, q0);
                }
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
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            if(rex.w) {
                REV64x(gd, gd);
            } else {
                REV16w(x1, gd);
                BFIx(gd, x1, 0, 16);
            }
            break;

        case 0xD0:
            INST_NAME("ADDSUBPD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            VFSUBQD(v0, q0, q1);
            VFADDQD(q0, q0, q1);
            VMOVeD(q0, 0, v0, 0);
            break;
        case 0xD1:
            INST_NAME("PSRLW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 16);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +16 values
            NEG_32(v0, v0);         // neg to do shr
            VDUPQ_16(v0, v0, 0);    // only the low 8bits will be used anyway
            USHLQ_16(q0, q0, v0);   // SHR x8
            break;
        case 0xD2:
            INST_NAME("PSRLD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 32);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +32 values
            NEG_32(v0, v0);         // neg to do shr
            VDUPQ_32(v0, v0, 0);    // only the low 8bits will be used anyway
            USHLQ_32(q0, q0, v0);   // SHR x4
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 64);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +64 values
            NEG_32(v0, v0);         // neg to do shr
            VDUPQ_32(v0, v0, 0);    // only the low 8bits will be used anyway
            USHLQ_64(q0, q0, v0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADDQ_64(v0, v0, q0);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            VMULQ_16(q0, q0, q1);
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                FMOVD(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0xD7:
            nextop = F8;
            if(MODREG) {
                INST_NAME("PMOVMSKB Gd, Ex");
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                q1 = fpu_get_scratch(dyn, ninst);
                GETEX(q0, 0, 0);
                GETGD;
                TABLE64_(x2, 0x0706050403020100LL);
                VDUPQD(v0, x2);
                VSHRQ_8(q1, q0, 7);
                USHLQ_8(q1, q1, v0); // shift
                UADDLV_8(v1, q1);   // accumalte
                VMOVBto(gd, v1, 0);
                // and now the high part
                VMOVeD(q1, 0, q1, 1);
                UADDLV_8(q1, q1);   // accumalte
                VMOVBto(x2, q1, 0);
                BFIw(gd, x2, 8, 8);
            } else {
                DEFAULT;
            }
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            UQSUBQ_8(q0, q0, q1);
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            UQSUBQ_16(q0, q0, q1);
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            UMINQ_8(q0, q0, q1);
            break;
        case 0xDB:
            INST_NAME("PAND Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VANDQ(v0, v0, q0);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            UQADDQ_8(q0, q0, q1);
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            UQADDQ_16(q0, q0, q1);
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            UMAXQ_8(q0, q0, q1);
            break;
        case 0xDF:
            INST_NAME("PANDN Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VBICQ(v0, q0, v0);
            break;
         case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            URHADDQ_8(v0, v0, v1);
            break;
        case 0xE1:
            INST_NAME("PSRAW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 15);
            UMIN_32(v0, v0, v1);    // limit to -15 .. +15 values
            NEG_32(v0, v0);
            VDUPQ_16(v0, v0, 0);    // only the low 8bits will be used anyway
            SSHLQ_16(q0, q0, v0);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 31);
            UMIN_32(v0, v0, v1);        // limit to 0 .. +31 values
            NEG_32(v0, v0);
            VDUPQ_32(v0, v0, 0);    // only the low 8bits will be used anyway
            SSHLQ_32(q0, q0, v0);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            URHADDQ_16(v0, v0, q0);
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            VUMULL_16(q0, v0, v1);
            VUMULL2_16(q1, v0, v1);
            UQSHRN_16(v0, q0, 16);  // 16bits->16bits: no saturation
            UQSHRN2_16(v0, q1, 16);
            break;
        case 0xE5:
            INST_NAME("PMULHW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            VSMULL_16(q0, v0, v1);
            VSMULL2_16(q1, v0, v1);
            SQSHRN_16(v0, q0, 16);
            SQSHRN2_16(v0, q1, 16);
            break;
        case 0xE6:
            INST_NAME("CVTTPD2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if(BOX64ENV(dynarec_fastround)) {
                VFCVTZSQD(v0, v1);  // convert double -> int64
                SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
            } else {
                if(cpuext.frintts) {
                    VFRINT32ZDQ(v0, v1); // handle overflow
                    VFCVTZSQD(v0, v0);  // convert double -> int64
                    SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
                } else {
                    MRS_fpsr(x5);
                    ORRw_mask(x4, xZR, 1, 0);    //0x80000000
                    d0 = fpu_get_scratch(dyn, ninst);
                    for(int i=0; i<2; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        if(i) {
                            VMOVeD(d0, 0, v1, i);
                            FCVTZSwD(x1, d0);
                        } else {
                            FCVTZSwD(x1, v1);
                        }
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        MOVw_REG(x1, x4);
                        VMOVQSfrom(v0, i, x1);
                    }
                    VMOVQDfrom(v0, 1, xZR);
                }
            }
            break;
        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
            nextop = F8;
            if(MODREG) {
                DEFAULT;
            } else {
                GETGX(v0, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VST128(v0, ed, fixedaddress);
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            SQSUBQ_8(v0, v0, q0);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            SQSUBQ_16(v0, v0, q0);
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            SMINQ_16(v0, v0, q0);
            break;
        case 0xEB:
            INST_NAME("POR Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VORRQ(v0, v0, q0);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            SQADDQ_8(v0, v0, q0);
            break;
        case 0xED:
            INST_NAME("PADDSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            SQADDQ_16(v0, v0, q0);
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            SMAXQ_16(v0, v0, q0);
            break;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG && ((nextop&7)+(rex.b<<3)==gd)) {
                // special case for PXOR Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VEORQ(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                VEORQ(q0, q0, q1);
            }
            break;

        case 0xF1:
            INST_NAME("PSLLW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 16);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +16 values
            VDUPQ_16(v0, v0, 0);    // only the low 8bits will be used anyway
            USHLQ_16(q0, q0, v0);
            break;
        case 0xF2:
            INST_NAME("PSLLD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 32);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +32 values
            VDUPQ_32(v0, v0, 0);    // only the low 8bits will be used anyway
            USHLQ_32(q0, q0, v0);
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            v1 = fpu_get_scratch(dyn, ninst);
            UQXTN_32(v0, q1);
            MOVI_32(v1, 64);
            UMIN_32(v0, v0, v1);    // limit to 0 .. +64 values
            VDUPQ_64(v0, v0, 0);    // only the low 8bits will be used anyway
            USHLQ_64(q0, q0, v0);
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            VUZP1Q_32(q0, v0, v0);  //A3 A2 A1 A0 -> A3 A1 A2 A0
            if(MODREG) {
                q1 = fpu_get_scratch(dyn, ninst);
            } else {
                q1 = v1;
            }
            VUZP1Q_32(q1, v1, v1);
            VUMULL_32(v0, q0, q1);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            q1 = fpu_get_scratch(dyn, ninst);
            VSMULL_16(q0, v0, v1);
            VSMULL2_16(q1, v0, v1);
            VADDPQ_32(v0, q0, q1);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0, 0);
            d0 = fpu_get_scratch(dyn, ninst);
            d1 = fpu_get_scratch(dyn, ninst);
            VEOR(d1, d1, d1);   // is it necessary?
            UABDL_8(d0, q0, q1);
            UADDLVQ_16(d1, d0);
            VMOVeD(q0, 0, d1, 0);
            UABDL2_8(d0, q0, q1);
            UADDLVQ_16(d1, d0);
            VMOVeD(q0, 1, d1, 0);
            break;
        case 0xF7:
            INST_NAME("MASKMOVDQU Gx, Ex");
            nextop = F8;
            GETGX(q0, 0);
            GETEX(q1, 0, 0);
            v0 = fpu_get_scratch(dyn, ninst);
            VLDR128_U12(v0, xRDI, 0);
            if(MODREG)
                v1 = fpu_get_scratch(dyn, ninst); // need to preserve the register
            else
                v1 = q1;
            VSSHRQ_8(v1, q1, 7);  // get the mask
            VBITQ(v0, q0, v1);
            VSTR128_U12(v0, xRDI, 0);  // put back
            break;
        case 0xF8:
            INST_NAME("PSUBB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUBQ_8(v0, v0, q0);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUBQ_16(v0, v0, q0);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUBQ_32(v0, v0, q0);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VSUBQ_64(v0, v0, q0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADDQ_8(v0, v0, q0);
            break;
        case 0xFD:
            INST_NAME("PADDW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADDQ_16(v0, v0, q0);
            break;
        case 0xFE:
            INST_NAME("PADDD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            VADDQ_32(v0, v0, q0);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
