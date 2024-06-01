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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_AVX_66_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u, tmp64u2;
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

    /* Remember to not create a new fpu_scratch after some GY/VY/EY is created, because Y can be in the scratch area and might overlap (and scratch will win) */

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if(box64_dynarec_fastround) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                VFRINTISQ(v0, v1);
                if(!vex.l) x87_restoreround(dyn, ninst, u8);
                VFCVTZSQS(v0, v0);
            } else {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                MOV32w(x4, 0x80000000);
                d0 = fpu_get_scratch(dyn, ninst);
                for(int i=0; i<4; ++i) {
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    VMOVeS(d0, 0, v1, i);
                    FRINTIS(d0, d0);
                    VFCVTZSs(d0, d0);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ(x5, FPSR_IOC, 4+4);
                    VMOVQSfrom(d0, 0, x4);
                    VMOVeS(v0, i, d0, 0);
                }
                if(!vex.l) x87_restoreround(dyn, ninst, u8);
            }
            if(vex.l) {
                GETGY_empty_EY(v0, v1);
                if(box64_dynarec_fastround) {
                    VFRINTISQ(v0, v1);
                    x87_restoreround(dyn, ninst, u8);
                    VFCVTZSQS(v0, v0);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    MOV32w(x4, 0x80000000);
                    d0 = fpu_get_scratch(dyn, ninst);
                    for(int i=0; i<4; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        VMOVeS(d0, 0, v1, i);
                        FRINTIS(d0, d0);
                        VFCVTZSs(d0, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        VMOVQSfrom(d0, 0, x4);
                        VMOVeS(v0, i, d0, 0);
                    }
                    x87_restoreround(dyn, ninst, u8);
                }
            } else YMM0(gd);
            break;

        case 0x64:
            INST_NAME("VPCMPGTB Gx,Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VCMGTQ_8(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VCMGTQ_8(v0, v2, v1);
            } else YMM0(gd);
            break;
        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VCMGTQ_16(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VCMGTQ_16(v0, v2, v1);
            } else YMM0(gd);
            break;
        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETGX_empty_VXEX(v0, v2, v1, 0);
            VCMGTQ_32(v0, v2, v1);
            if(vex.l) {
                GETGY_empty_VYEY(v0, v2, v1);
                VCMGTQ_32(v0, v2, v1);
            } else YMM0(gd);
            break;

        case 0x6B:
            INST_NAME("PACKSSDW Gx,Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                if(v0==v1) {
                    q0 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(q0, v0);
                }
                SQXTN_16(v0, v2);
                if(v2==v1) {
                    VMOVeD(v0, 1, v0, 0);
                } else {
                    SQXTN2_16(v0, (v0==v1)?q0:v1);
                }
            } 
            if(!vex.l) YMM0(gd);
            break;
        case 0x6C:
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VXEX(v0, v2, v1, 0);
                    } else {
                        GETGY_empty_VYEY(v0, v2, v1);
                    }
                    VMOVeD(v0, 1, v1, 0);
                    if(v0!=v2) VMOVeD(v0, 0, v2, 0);
                }
            } else {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VX(v0, v2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                    } else {
                        GETGY_empty_VY(v0, v2, 0, -1, -1);
                        ADDx_U12(x3, ed, 16);
                        ed = x3;
                    }
                    VLD1_64(v0, 1, ed);
                    if(v0!=v2) VMOVeD(v0, 0, v2, 0);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x6D:
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            if(MODREG) {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VXEX(v0, v2, v1, 0);
                    } else {
                        GETGY_empty_VYEY(v0, v2, v1);
                    }
                    VMOVeD(v0, 0, v2, 1);
                    if(v0!=v1) VMOVeD(v0, 1, v1, 1);
                }
            } else {
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        GETGX_empty_VX(v0, v2, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                        ADDx_U12(x1, ed, 8);
                    } else {
                        GETGY_empty_VY(v0, v2, -1, -1, -1);
                        ADDx_U12(x1, ed, 16+8);
                    }
                    VMOVeD(v0, 0, v2, 1);
                    VLD1_64(v0, 1, x1);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x6E:
            INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETGX_empty(v0);
            GETED(0);
            if(rex.w) {
                FMOVDx(v0, ed);
            } else {
                VEORQ(v0, v0, v0); // RAZ vector
                VMOVQSfrom(v0, 0, ed);
            }
            YMM0(gd);
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VMOVQ(v0, v1);
                if(vex.l) {
                    GETGY_empty_EY(v0, v1);
                    VMOVQ(v0, v1);
                }
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                VLDR128_U12(v0, ed, fixedaddress);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VLDR128_U12(v0, ed, fixedaddress+16);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0x70:
            INST_NAME("VPSHUFD Gx,Ex,Ib");
            nextop = F8;
            if(MODREG) {
                u8 = F8;
                d0 = fpu_get_scratch(dyn, ninst);
                for(int l=0; l<1+vex.l; ++l) {
                    if(!l) {
                        v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                        GETGX_empty(v0);
                    } else {
                        GETGY_empty_EY(v0, v1);
                    }
                    if(u8==0x4E) {
                        if(v0==v1) {
                            VEXTQ_8(v0, v0, v0, 8);
                        } else {
                            VMOVeD(v0, 0, v1, 1);
                            VMOVeD(v0, 1, v1, 0);
                        }
                    } else if(u8==0x00) {
                        VDUPQ_32(v0, v1, 0);
                    } else if(u8==0x55) {
                        VDUPQ_32(v0, v1, 1);
                    } else if(u8==0xAA) {
                        VDUPQ_32(v0, v1, 2);
                    } else if(u8==0xFF) {
                        VDUPQ_32(v0, v1, 3);
                    } else if(u8==0x44) {
                        VDUPQ_64(v0, v1, 0);
                    } else if(u8==0xEE) {
                        VDUPQ_64(v0, v1, 1);
                    } else if(u8==0xB1) {
                        VREV64Q_32(v0, v1);
                    } else if(v0!=v1) {
                        VMOVeS(v0, 0, v1, (u8>>(0*2))&3);
                        VMOVeS(v0, 1, v1, (u8>>(1*2))&3);
                        VMOVeS(v0, 2, v1, (u8>>(2*2))&3);
                        VMOVeS(v0, 3, v1, (u8>>(3*2))&3);
                    } else {
                        if(!l) {
                            uint64_t swp[4] = {
                                (0)|(1<<8)|(2<<16)|(3<<24),
                                (4)|(5<<8)|(6<<16)|(7<<24),
                                (8)|(9<<8)|(10<<16)|(11<<24),
                                (12)|(13<<8)|(14<<16)|(15<<24)
                            };
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
                        }
                        VTBLQ1_8(v0, v1, d0);
                    }
                }
            } else {
                SMREAD();
                for(int l=0; l<1+vex.l; ++l) {
                    i32 = -1;
                    if(!l) {
                        GETGX_empty(v0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 1);
                        u8 = F8;
                    } else {
                        GETGY_empty(v0, -1, -1, -1);
                        ADDx_U12(x3, ed, 16);
                        ed = x3;
                    }
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
                        VLDQ1R_32(v0, ed);
                    }
                }
            }
            if(!vex.l)  YMM0(gd);
            break;

        case 0x72:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("VPSRLD Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETVX_empty_EX(v0, v1,  1);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>31) {
                                VEORQ(v0, v0, v0);
                            } else if(u8) {
                                VSHRQ_32(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 4:
                    INST_NAME("VPSRAD Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETVX_empty_EX(v0, v1, 1);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8>31) u8=31;
                        if(u8) {
                            VSSHRQ_32(v0, v1, u8);
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 6:
                    INST_NAME("VPSLLD Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETVX_empty_EX(v0, v1, 1);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>31) {
                                VEORQ(v0, v0, v0);
                            } else {
                                VSHLQ_32(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("VPSRLQ Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>63) {
                                VEORQ(v0, v0, v0);
                            } else if(u8) {
                                VSHRQ_64(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 3:
                    INST_NAME("VPSRLDQ Vx, Ex, Ib");
                    q1 = fpu_get_scratch(dyn, ninst);
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if(u8>15) {
                                VEORQ(v0, v0, v0);
                            } else {
                                if(!l) VEORQ(q1, q1, q1);
                                VEXTQ_8(v0, v1, q1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 6:
                    INST_NAME("VPSLLQ Vx, Ex, Ib");
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if (u8>63) {
                                VEORQ(v0, v0, v0);
                            } else {
                                VSHLQ_64(v0, v1, u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                case 7:
                    INST_NAME("VPSLLDQ Vx, Ex, Ib");
                    q1 = fpu_get_scratch(dyn, ninst);
                    for(int l=0; l<1+vex.l; ++l) {
                        if(!l) {
                            GETEX_Y(v1, 0, 1);
                            GETVX_empty(v0);
                            u8 = F8;
                        } else {
                            GETVY_empty_EY(v0, v1);
                        }
                        if(u8) {
                            if(u8>15) {
                                VEORQ(v0, v0, v0);
                            } else if(u8>0) {
                                if(!l) VEORQ(q1, q1, q1);
                                VEXTQ_8(v0, q1, v1, 16-u8);
                            }
                        } else if(v0!=v1)
                            VMOVQ(v0, v1);
                    }
                    if(!vex.l) YMM0(vex.v);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1);
                VMOVQ(v1, v0);
                if(vex.l) {
                    GETGYEY(v1, v0);
                    VMOVQ(v1, v0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                VSTR128_U12(v0, ed, fixedaddress);
                if(vex.l) {
                    GETGY(v0, 0, -1, -1, -1);
                    VSTR128_U12(v0, ed, fixedaddress+16);
                }
                SMWRITE2();
            }
            break;

        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQSUBQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQSUBQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UMINQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                VANDQ(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UQADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDE:
            INST_NAME("VPMAXUB Gx,  Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                UMAXQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                VBICQ(v0, v1, v2);
            }
            if(!vex.l) YMM0(gd);
            break;
         case 0xE0:
            INST_NAME("VPAVGB Gx,  Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                URHADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQSUBQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQSUBQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SMINQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                VORRQ(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SQADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) {
                    GETGX_empty_VXEX(v0, v2, v1, 0);
                } else {
                    GETGY_empty_VYEY(v0, v2, v1);
                }
                SMAXQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETG;
            if(MODREG && ((nextop&7)+(rex.b<<3)==vex.v)) {
                // special case for PXOR Gx, Gx
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VEORQ(q0, q0, q0);
                if(vex.l) {
                    q0 = ymm_get_reg_empty(dyn, ninst, x1, gd, -1, -1,-1);
                    VEORQ(q0, q0, q0);
                }
            } else {
                GETGX_empty_VXEX(v0, v2, v1, 0);
                VEORQ(v0, v1, v2);
                if(vex.l) {
                    GETGY_empty_VYEY(v0, v2,v1);
                    VEORQ(v0, v1, v2);
                }
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VSUBQ_64(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_8(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_16(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;
        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            for(int l=0; l<1+vex.l; ++l) {
                if(!l) { GETGX_empty_VXEX(v0, v2, v1, 0); } else { GETGY_empty_VYEY(v0, v2, v1); }
                VADDQ_32(v0, v2, v1);
            }
            if(!vex.l) YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
