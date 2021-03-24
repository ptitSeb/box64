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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

// Get EX as a quad
#define GETEX(a, D)                                                                                 \
    if(MODREG) {                                                                                    \
        a = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));                                     \
    } else {                                                                                        \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, D);    \
        a = fpu_get_scratch(dyn);                                                                   \
        VLDR128_U12(a, ed, fixedaddress);                                                           \
    }

#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

#define GETGX(a)                        \
    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
    a = sse_get_reg(dyn, ninst, x1, gd)

#define GETGX_empty(a)                  \
    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
    a = sse_get_reg_empty(dyn, ninst, x1, gd)

uintptr_t dynarec64_660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, j32;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    uint8_t eb1, eb2;
    int v0, v1;
    int q0, q1;
    int d0;
    int fixedaddress;

    MAYUSE(d0);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j32);
    #if 0//STEP == 3
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    switch(opcode) {


        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVeD(v0, 1, v1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                VLD1_64(v0, 1, ed);
            }
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            VMOVeD(v0, 0, v0, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVeD(v0, 1, v1, 1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                v1 = fpu_get_scratch(dyn);
                ADDx_U12(ed, ed, 8);
                VLD1_64(v0, 1, ed);
            }
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
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg(dyn, ninst, x1, ed);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex,Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                ed = (nextop&7)+(rex.b<<3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VSTR128_U12(v0, ed, fixedaddress);
            }
            break;

        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            FCMPD(v0, q0);
            FCOMI(x1, x2);
            break;

        #define GO(GETFLAGS, NO, YES, F)            \
            READFLAGS(F);                           \
            GETFLAGS;                               \
            nextop=F8;                              \
            GETGD;                                  \
            if(MODREG) {                            \
                ed = xRAX+(nextop&7)+(rex.b<<3);    \
            } else {                                \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<1, 1, rex, 0, 0); \
                LDRH_U12(x1, ed, fixedaddress);     \
                ed = x1;                            \
            }                                       \
            Bcond(NO, +8);                          \
            BFIx(gd, ed, 0, 16);

        GOCOND(0x40, "CMOV", "Gw, Ew");
        #undef GO

        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0);
            VANDQ(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0);
            VBICQ(v0, q0, v0);
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0);
            VORRQ(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0);
            VEORQ(v0, v0, q0);
            break;

        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETEX(v1, 0);
            GETGX_empty(v0);
            FCVTXN(v0, v1);
            break;
        case 0x5B:
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0);
            GETGX_empty(v0);
            // need rounding? using "toward 0 for now"
            VFCVTZSQS(v0, v1);
            break;

        case 0x60:
            INST_NAME("PUNPCKLBW Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VZIP1Q_8(v0, v0, q0);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VZIP1Q_16(v0, v0, q0);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VZIP1Q_32(v0, v0, q0);
            break;

        case 0x64:
            INST_NAME("PCMPGTB Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            VCMGTQ_8(v0, v0, v1);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            VCMGTQ_16(v0, v0, v1);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            VCMGTQ_32(v0, v0, v1);
            break;

        case 0x68:
            INST_NAME("PUNPCKHBW Gx,Ex");
            nextop = F8;
            GETGX(q0);
            GETEX(q1, 1);
            VZIP2Q_8(q0, q0, q1);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            GETGX(q0);
            GETEX(q1, 1);
            VZIP2Q_16(q0, q0, q1);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx,Ex");
            nextop = F8;
            GETGX(q0);
            GETEX(q1, 1);
            VZIP2Q_32(q0, q0, q1);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            SQXTN_16(v0, v0);
            if(v0==v1) {
                VMOVeD(v0, 1, v0, 0);
            } else {
                SQXTN2_16(v0, v1);
            }
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVeD(v0, 1, v1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                VLD1_64(v0, 1, ed);
            }
            break;

        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            GETG;
            GETED(0);
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if(rex.w) {
                FMOVDx(v0, ed);
            } else {
                VEORQ(v0, v0, v0); // RAZ vector
                VMOVQSfrom(v0, 0, ed);
            }
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            GETG;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVQ(v0, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            GETG;
            i32 = -1;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                u8 = F8;
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                if(u8==0x4E) {
                    if(v0==v1) {
                        VSWP(v0, v0+1);
                    } else {
                        VMOVeD(v0, 0, v1, 1);
                        VMOVeD(v0, 1, v1, 0);
                    }
                } else {
                    uint32_t swp[4] = {
                        (0)|(1<<8)|(2<<16)|(3<<24),
                        (4)|(5<<8)|(6<<16)|(7<<24),
                        (8)|(9<<8)|(10<<16)|(11<<24),
                        (12)|(13<<8)|(14<<16)|(15<<24)
                    };
                    d0 = fpu_get_scratch(dyn);
                    if(v0==v1) {
                        q1 = fpu_get_scratch(dyn);
                        VMOVQ(q1, v1);
                    } else
                        q1 = v1;
                    MOV32w(x2, swp[(u8>>(0*2))&3]);
                    MOV32w(x3, swp[(u8>>(1*2))&3]);
                    VMOVQSfrom(d0, 0, x2);
                    VMOVQSfrom(d0, 1, x3);
                    MOV32w(x2, swp[(u8>>(2*2))&3]);
                    MOV32w(x3, swp[(u8>>(3*2))&3]);
                    VMOVQSfrom(d0, 2, x2);
                    VMOVQSfrom(d0, 3, x3);
                    VTBLQ1_8(v0, v1, d0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 1);
                u8 = F8;
                if (u8) {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            ADDx_U12(x2, ed, idx*4);
                            i32 = idx;
                        }
                        VLD1_32(v0, i, x2);
                    }
                } else {
                    VLD1R_32(v0, ed);
                }
            }
            break;
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_16(q0, q0, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8>15) u8=15;
                    if(u8) {
                        VSSHRQ_16(q0, q0, u8);
                    }
                    if(!MODREG) {
                        VSTR128_U12(q0, ed, fixedaddress);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_16(q0, q0, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
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
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_32(q0, q0, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8>31) u8=31;
                    if(u8) {
                        VSSHRQ_32(q0, q0, u8);
                    }
                    if(!MODREG) {
                        VSTR128_U12(q0, ed, fixedaddress);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_32(q0, q0, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
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
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_64(q0, q0, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if(u8>15) {
                            VEORQ(q0, q0, q0);
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            VEORQ(q1, q1, q1);
                            VEXTQ_8(q0, q0, q1, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_64(q0, q0, u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(q0, 1);
                    u8 = F8;
                    if(u8) {
                        if(u8>15) {
                            VEORQ(q0, q0, q0);
                        } else if(u8>0) {
                            q1 = fpu_get_scratch(dyn);
                            VEORQ(q1, q1, q1);
                            VEXTQ_8(q0, q1, q0, 16-u8);
                        }
                        if(!MODREG) {
                            VSTR128_U12(q0, ed, fixedaddress);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            GETGX(v0);
            if(rex.w) {
                if(MODREG) {
                    ed = xRAX + (nextop&7) + (rex.b<<3);
                    VMOVQDto(ed, v0, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                    VSTR64_U12(v0, ed, fixedaddress);
                }
            } else {
                if(MODREG) {
                    ed = xRAX + (nextop&7) + (rex.b<<3);
                    VMOVSto(ed, v0, 0);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    VSTR32_U12(v0, ed, fixedaddress);
                }
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            GETGX(v0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3));
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<4, 15, rex, 0, 0);
                VSTR128_U12(v0, ed, fixedaddress);
            }
            break;

        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            break;
        case 0xA4:
        case 0xA5:
            nextop = F8;
            if(opcode==0xA4) {
                INST_NAME("SHLD Ew, Gw, Ib");
            } else {
                INST_NAME("SHLD Ew, Gw, CL");
                UXTBw(x3, xRCX);
            }
            SETFLAGS(X_ALL, SF_SET);
            GETEWW(x4, x1, (opcode==0xA4)?1:0);
            GETGW(x2);
            if(opcode==0xA4) {
                u8 = F8;
                MOV32w(x3, u8);
            }
            CALL_(shld16, x1, wback);
            EWBACKW(x1);
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDSw_mask(x1, x1, 0, 0);  //mask=1
            B_NEXT(cNE);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;
        case 0xAC:
        case 0xAD:
            nextop = F8;
            if(opcode==0xAC) {
                INST_NAME("SHRD Ew, Gw, Ib");
            } else {
                INST_NAME("SHRD Ew, Gw, CL");
                UXTBw(x3, xRCX);
            }
            SETFLAGS(X_ALL, SF_SET);
            GETEWW(x4, x1, (opcode==0xAC)?1:0);
            GETGW(x2);
            if(opcode==0xAC) {
                u8 = F8;
                MOV32w(x3, u8);
            }
            CALL_(shrd16, x1, wback);
            EWBACKW(x1);
            break;

        case 0xAF:
            INST_NAME("IMUL Gw,Ew");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            UFLAG_DF(x1, d_imul16);
            GETSEW(x1, 0);
            GETSGW(x2);
            MULw(x2, x2, x1);
            UFLAG_RES(x2);
            GWBACK;
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDSw_mask(x1, x1, 0, 0);  //mask=1
            B_NEXT(cEQ);
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;

        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SET);
            nextop = F8;
            gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3);    // GETGD
            GETEW(x4, 0);
            ANDw_mask(x2, gd, 0, 0b000011);  // mask=0x0f
            LSRw_REG(x1, ed, x2);
            BFIw(xFlags, x1, F_CF, 1);
            ANDw_mask(x1, x1, 0, 0);  //mask=1
            MOV32w(x1, 1);
            LSLxw_REG(x1, x1, x2);
            EORxw_REG(ed, ed, x1);
            EWBACK;
            break;
        case 0xBC:
            INST_NAME("BSF Ew,Gw");
            SETFLAGS(X_ZF, SF_SET);
            nextop = F8;
            GETGD;
            GETEW(x1, 0);  // Get EW
            TSTw_REG(x1, x1);
            B_MARK(cEQ);
            RBITw(x1, x1);   // reverse
            CLZw(x2, x1);    // x2 gets leading 0 == BSF
            BFIw(gd, x2, 0, 16);
            MARK;
            CSETw(x1, cEQ);    //ZF not set
            BFIw(xFlags, x1, F_ZF, 1);
            SET_DFNONE(x1);
            break;
        case 0xBD:
            INST_NAME("BSR Ew,Gw");
            SETFLAGS(X_ZF, SF_SET);
            nextop = F8;
            GETGD;
            GETEW(x1, 0);  // Get EW
            TSTw_REG(x1, x1);   // Don't use CBZ here, as the flag is reused later
            B_MARK(cEQ);
            LSLw(x1, x1, 16);   // put bits on top
            CLZw(x2, x1);       // x2 gets leading 0
            SUBw_U12(x2, x2, 15);
            NEGw_REG(x2, x2);   // complement
            BFIx(gd, x2, 0, 16);
            MARK;
            CSETw(x1, cEQ);    //ZF not set
            BFIw(xFlags, x1, F_ZF, 1);
            SET_DFNONE(x1);
            break;
        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                if(rex.rex) {
                    ed = xRAX+(nextop&7)+(rex.b<<3);
                    eb1=ed;
                    eb2=0;
                } else {
                    ed = (nextop&7);
                    eb1 = xRAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                }
                SBFXw(x1, eb1, eb2, 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff, 0, rex, 0, 0);
                LDRSBw_U12(x1, ed, fixedaddress);
            }
            BFIx(gd, x1, 0, 16);
            break;


        case 0xD4:
            INST_NAME("PADDQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VADDQ_64(v0, v0, q0);
            break;

        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop&7) + (rex.b<<3));
                FMOVD(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xfff<<3, 7, rex, 0, 0);
                VSTR64_U12(v0, ed, fixedaddress);
            }
            break;

        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VANDQ(v0, v0, q0);
            break;

        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VBICQ(v0, q0, v0);
            break;

        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VORRQ(v0, v0, q0);
            break;

        case 0xEF:
            INST_NAME("PXOR Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG && ((nextop&7)+(rex.b<<3)==gd)) {
                // special case for PXOR Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VEORQ(q0, q0, q0);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd);
                GETEX(q1, 0);
                VEORQ(q0, q0, q1);
            }
            break;

        case 0xF4:
            INST_NAME("PMULUDQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1, 0);
            q0 = fpu_get_scratch(dyn);
            VUZP1Q_32(q0, v0, v0);  //A3 A2 A1 A0 -> A3 A1 A2 A0
            if(MODREG) {
                q1 = fpu_get_scratch(dyn);
            } else {
                q1 = v1;
            }
            VUZP1Q_32(q1, v1, v1);
            VUMULL_32(v0, q0, q1);
            break;

        case 0xF8:
            INST_NAME("PSUBB Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VSUBQ_8(v0, v0, q0);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VSUBQ_16(v0, v0, q0);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VSUBQ_32(v0, v0, q0);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VSUBQ_64(v0, v0, q0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VADDQ_8(v0, v0, q0);
            break;
        case 0xFD:
            INST_NAME("PADDW Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VADDQ_16(v0, v0, q0);
            break;
        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0, 0);
            VADDQ_32(v0, v0, q0);
            break;

        default:
            DEFAULT;
    }
    return addr;
}

