#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "emit_signals.h"
#include "x64shaext.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

static uint8_t ff_mult(uint8_t a, uint8_t b)
{
    int retval = 0;

    for(int i = 0; i < 8; i++) {
        if((b & 1) == 1)
            retval ^= a;

        if((a & 0x80)) {
            a <<= 1;
            a  ^= 0x1b;
        } else {
            a <<= 1;
        }

        b >>= 1;
    }

    return retval;
}

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_660F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_660F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u, u8;
    int8_t tmp8s;
    int16_t tmp16s;
    uint16_t tmp16u;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s;
    reg64_t *oped, *opgd, *opvd;
    sse_regs_t *opex, *opgx, *opvx, eax1, eax2;
    sse_regs_t *opey, *opgy, *opvy, eay1, eay2;
    // AES opcodes constants
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  F  K  P  E  J  O  D  I  N  C  H  M  B  G  L
    const uint8_t shiftrows[] = {0, 5,10,15, 4, 9,14, 3, 8,13, 2, 7,12, 1, 6,11};
    const uint8_t subbytes[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
    };
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  N  K  H  E  B  O  L  I  F  C  P  M  J  G  D
    const uint8_t invshiftrows[] = {0,13,10, 7, 4, 1,14,11, 8, 5, 2,15,12, 9, 6, 3};
    const uint8_t invsubbytes[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
    };


#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {
        case 0x00:  /* VPSHUFB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            if(VX==GX) {
                eax1 = *VX;
                VX = &eax1;
            }
            for (int i=0; i<16; ++i) {
                if(EX->ub[i]&128)
                    GX->ub[i] = 0;
                else
                    GX->ub[i] = VX->ub[EX->ub[i]&15];
            }
            if(vex.l) {
                GETEY;
                if(VY==GY) {
                    eay1 = *VY;
                    VY = &eay1;
                }
                for (int i=0; i<16; ++i) {
                    if(EY->ub[i]&128)
                        GY->ub[i] = 0;
                    else
                        GY->ub[i] = VY->ub[EY->ub[i]&15];
                }
            } else
                GY->q[0] = GY->q[1] = 0;
            break;
        case 0x01:  /* VPHADDW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            u8 = (VX==EX);
            if(GX==EX) {eax1=*EX; EX=&eax1;}
            for (int i=0; i<4; ++i)
                GX->sw[i] = VX->sw[i*2+0]+VX->sw[i*2+1];
            if(u8) {
                GX->q[1] = GX->q[0];
            } else {
                for (int i=0; i<4; ++i)
                    GX->sw[4+i] = EX->sw[i*2+0] + EX->sw[i*2+1];
            }
            if(vex.l) {
                GETVY;
                if(EY==GY) {eay1=*EY; EY=&eay1;}
                for (int i=0; i<4; ++i)
                    GY->sw[i] = VY->sw[i*2+0]+VY->sw[i*2+1];
                if(u8) {
                    GY->q[1] = GY->q[0];
                } else {
                    for (int i=0; i<4; ++i)
                        GY->sw[4+i] = EY->sw[i*2+0] + EY->sw[i*2+1];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x02:  /* VPHADDD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            u8 = (VX==EX);
            if(GX==EX) {eax1=*EX; EX=&eax1;}
            GX->sd[0] = VX->sd[0] + VX->sd[1];
            GX->sd[1] = VX->sd[2] + VX->sd[3];
            if(u8) {
                GX->q[1] = GX->q[0];
            } else {
                GX->sd[2] = EX->sd[0] + EX->sd[1];
                GX->sd[3] = EX->sd[2] + EX->sd[3];
            }
            if(vex.l) {
                GETVY;
                if(EY==GY) {eay1=*EY; EY=&eay1;}
                GY->sd[0] = VY->sd[0] + VY->sd[1];
                GY->sd[1] = VY->sd[2] + VY->sd[3];
                if(u8) {
                    GY->q[1] = GY->q[0];
                } else {
                    GY->sd[2] = EY->sd[0] + EY->sd[1];
                    GY->sd[3] = EY->sd[2] + EY->sd[3];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x03:  /* VPHADDSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            u8 = (VX==EX);
            if(GX==EX) {eax1=*EX; EX=&eax1;}
            for (int i=0; i<4; ++i) {
                tmp32s = VX->sw[i*2+0]+VX->sw[i*2+1];
                GX->sw[i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
            }
            if(u8) {
                GX->q[1] = GX->q[0];
            } else {
                for (int i=0; i<4; ++i) {
                    tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                    GX->sw[4+i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                }
            }
            if(vex.l) {
                GETVY;
                if(EY==GY) {eay1=*EY; EY=&eay1;}
                for (int i=0; i<4; ++i) {
                    tmp32s = VY->sw[i*2+0]+VY->sw[i*2+1];
                    GY->sw[i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                }
                if(u8) {
                    GY->q[1] = GY->q[0];
                } else {
                    for (int i=0; i<4; ++i) {
                        tmp32s = EY->sw[i*2+0] + EY->sw[i*2+1];
                        GY->sw[4+i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                    }
                }
            } else
                GY->u128 = 0;
            break;
        case 0x04:  /* VPMADDUBSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for (int i=0; i<8; ++i) {
                tmp32s = (int32_t)(VX->ub[i*2+0])*EX->sb[i*2+0] + (int32_t)(VX->ub[i*2+1])*EX->sb[i*2+1];
                GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<8; ++i) {
                    tmp32s = (int32_t)(VY->ub[i*2+0])*EY->sb[i*2+0] + (int32_t)(VY->ub[i*2+1])*EY->sb[i*2+1];
                    GY->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                }
            } else
                GY->u128 = 0;
            break;
        case 0x05:  /* VPHSUBW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            u8 = (VX==EX);
            if(GX==EX) {eax1=*EX; EX=&eax1;}
            for (int i=0; i<4; ++i)
                GX->sw[i] = VX->sw[i*2+0] - VX->sw[i*2+1];
            if(u8) {
                GX->q[1] = GX->q[0];
            } else {
                for (int i=0; i<4; ++i)
                    GX->sw[4+i] = EX->sw[i*2+0] - EX->sw[i*2+1];
            }
            if(vex.l) {
                GETVY;
                if(EY==GY) {eay1=*EY; EY=&eay1;}
                for (int i=0; i<4; ++i)
                    GY->sw[i] = VY->sw[i*2+0] - VY->sw[i*2+1];
                if(u8) {
                    GY->q[1] = GY->q[0];
                } else {
                    for (int i=0; i<4; ++i)
                        GY->sw[4+i] = EY->sw[i*2+0] - EY->sw[i*2+1];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x06:  /* VPHSUBD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            u8 = (VX==EX);
            if(GX==EX) {eax1=*EX; EX=&eax1;}
            for (int i=0; i<2; ++i)
                GX->sd[i] = VX->sd[i*2+0] - VX->sd[i*2+1];
            if(u8) {
                GX->q[1] = GX->q[0];
            } else {
                for (int i=0; i<2; ++i)
                    GX->sd[2+i] = EX->sd[i*2+0] - EX->sd[i*2+1];
            }
            if(vex.l) {
                GETVY;
                if(EY==GY) {eay1=*EY; EY=&eay1;}
                GY->sd[0] = VY->sd[0] - VY->sd[1];
                GY->sd[1] = VY->sd[2] - VY->sd[3];
                if(u8) {
                    GY->q[1] = GY->q[0];
                } else {
                    GY->sd[2] = EY->sd[0] - EY->sd[1];
                    GY->sd[3] = EY->sd[2] - EY->sd[3];
                }
            } else
                GY->u128 = 0;
            break;
        case 0x07:  /* PHSUBSW Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            u8 = (VX==EX);
            if(GX==EX) {eax1=*EX; EX=&eax1;}
            for (int i=0; i<4; ++i) {
                tmp32s = VX->sw[i*2+0] - VX->sw[i*2+1];
                GX->sw[i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
            }
            if(u8) {
                GX->q[1] = GX->q[0];
            } else {
                for (int i=0; i<4; ++i) {
                    tmp32s = EX->sw[i*2+0] - EX->sw[i*2+1];
                    GX->sw[4+i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                }
            }
            if(vex.l) {
                GETVY;
                if(EY==GY) {eay1=*EY; EY=&eay1;}
                for (int i=0; i<4; ++i) {
                    tmp32s = VY->sw[i*2+0] - VY->sw[i*2+1];
                    GY->sw[i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                }
                if(u8) {
                    GY->q[1] = GY->q[0];
                } else {
                    for (int i=0; i<4; ++i) {
                        tmp32s = EY->sw[i*2+0] - EY->sw[i*2+1];
                        GY->sw[4+i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                    }
                }
            } else
                GY->u128 = 0;
            break;
        case 0x08:  /* VPSIGNB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for (int i=0; i<16; ++i)
                GX->sb[i] = VX->sb[i] * ((EX->sb[i]<0)?-1:((EX->sb[i]>0)?1:0));
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<16; ++i)
                    GY->sb[i] = VY->sb[i] * ((EY->sb[i]<0)?-1:((EY->sb[i]>0)?1:0));
            } else
                GY->u128 = 0;
            break;
        case 0x09:  /* VPSIGNW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for (int i=0; i<8; ++i)
                GX->sw[i] = VX->sw[i] * ((EX->sw[i]<0)?-1:((EX->sw[i]>0)?1:0));
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<8; ++i)
                    GY->sw[i] = VY->sw[i] * ((EY->sw[i]<0)?-1:((EY->sw[i]>0)?1:0));
            } else
                GY->u128 = 0;
            break;
        case 0x0A:  /* VPSIGND Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for (int i=0; i<4; ++i)
                GX->sd[i] = VX->sd[i] * ((EX->sd[i]<0)?-1:((EX->sd[i]>0)?1:0));
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<4; ++i)
                    GY->sd[i] = VY->sd[i] * ((EY->sd[i]<0)?-1:((EY->sd[i]>0)?1:0));
            } else
                GY->u128 = 0;
            break;
        case 0x0B:  /* VPMULHRSW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for (int i=0; i<8; ++i) {
                tmp32s = ((((int32_t)(VX->sw[i])*(int32_t)(EX->sw[i]))>>14) + 1)>>1;
                GX->uw[i] = tmp32s&0xffff;
            }
            if(vex.l) {
                GETEY; GETVY;
                for (int i=0; i<8; ++i) {
                    tmp32s = ((((int32_t)(VY->sw[i])*(int32_t)(EY->sw[i]))>>14) + 1)>>1;
                    GY->uw[i] = tmp32s&0xffff;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x0C:  /* VPERMILPS Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            GETVY;
            if(GX==VX) {eax1 = *VX; VX = &eax1;}
            for(int i=0; i<4; ++i)
                GX->ud[i] = VX->ud[EX->ud[i]&3];
            if(vex.l) {
                if(GY==VY) {eay1 = *VY; VY = &eay1;}
                for(int i=0; i<4; ++i)
                    GY->ud[i] = VY->ud[EY->ud[i]&3];
            } else
                GY->u128 = 0;
            break;
        case 0x0D:  /* VPERMILPD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            GETVY;
            if(GX==VX) {eax1 = *VX; VX = &eax1;}
            for(int i=0; i<2; ++i)
                GX->q[i] = VX->q[(EX->q[i]>>1)&1];
            if(vex.l) {
                if(GY==VY) {eay1 = *VY; VY = &eay1;}
                for(int i=0; i<2; ++i)
                    GY->q[i] = VY->q[(EY->q[i]>>1)&1];
            } else
                GY->u128 = 0;
            break;
        case 0x0E:  /* VTESTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            RESET_FLAGS(emu);
            u8 = 0; tmp8u = 0;
            for(int i=0; i<4; ++i) {
                if((GX->ud[i]&EX->ud[i])>>31) u8 = 1;
                if(((~GX->ud[i])&EX->ud[i])>>31) tmp8u = 1;
            }
            if(vex.l) {
                GETEY; GETGY;
                for(int i=0; i<4; ++i) {
                    if((GY->ud[i]&EY->ud[i])>>31) u8 = 1;
                    if(((~GY->ud[i])&EY->ud[i])>>31) tmp8u = 1;
                }
            }
            CONDITIONAL_SET_FLAG(!u8, F_ZF);
            CONDITIONAL_SET_FLAG(!tmp8u, F_CF);
            CLEAR_FLAG(F_AF);
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_SF);
            CLEAR_FLAG(F_PF);
            break;
        case 0x0F:  /* VTESTPD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            RESET_FLAGS(emu);
            u8 = 0; tmp8u = 0;
            for(int i=0; i<2; ++i) {
                if((GX->q[i]&EX->q[i])>>63) u8 = 1;
                if(((~GX->q[i])&EX->q[i])>>63) tmp8u = 1;
            }
            if(vex.l) {
                GETEY; GETGY;
                for(int i=0; i<2; ++i) {
                    if((GY->q[i]&EY->q[i])>>63) u8 = 1;
                    if(((~GY->q[i])&EY->q[i])>>63) tmp8u = 1;
                }
            }
            CONDITIONAL_SET_FLAG(!u8, F_ZF);
            CONDITIONAL_SET_FLAG(!tmp8u, F_CF);
            CLEAR_FLAG(F_AF);
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_SF);
            CLEAR_FLAG(F_PF);
            break;

        case 0x13:  /* VCVTPH2PS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l) {
                for(int i=3; i>=0; --i)
                    GY->ud[i] = cvtf16_32(EX->uw[4+i]);
            } else GY->u128 = 0;
            for(int i=3; i>=0; --i)
                GX->ud[i] = cvtf16_32(EX->uw[i]);
            break;

        case 0x16:  /* VPERMPS Gx, Vx, Ex */
            // same code as 0x36
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETEY;
            GETGY;
            GETVY;
            if(!vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
                eay1 = *EY;
                EY = &eay1;
            }
            if(GX==VX) {
                eax2 = *VX;
                VX = &eax2;
                eay2 = *VY;
                VY = &eay2;
            }
            for(int i=0; i<4; ++i) {
                u8 = VX->ud[i]&7;
                GX->ud[i] = (u8>3)?EY->ud[u8&3]:EX->ud[u8];
            }
            for(int i=0; i<4; ++i) {
                u8 = VY->ud[i]&7;
                GY->ud[i] = (u8>3)?EY->ud[u8&3]:EX->ud[u8];
            }
            break;
        case 0x17:      // VPTEST GX, EX
            nextop = F8;
            GETEX(0);
            GETGX;
            RESET_FLAGS(emu);
            if(vex.l) {
                GETEY; GETGY;
                CONDITIONAL_SET_FLAG(!(GY->u128&EY->u128) && !(GX->u128&EX->u128), F_ZF);
                CONDITIONAL_SET_FLAG(!((~GY->u128)&EY->u128) && !((~GX->u128)&EX->u128), F_CF);
            } else {
                CONDITIONAL_SET_FLAG(!(GX->u128&EX->u128), F_ZF);
                CONDITIONAL_SET_FLAG(!((~GX->u128)&EX->u128), F_CF);
            }
            CLEAR_FLAG(F_AF);
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_SF);
            CLEAR_FLAG(F_PF);
            break;
        case 0x18:  /* VBROADCASTSS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            tmp32u = EX->ud[0];
            for(int i=0; i<4; ++i)
                GX->ud[i] = tmp32u;
            if(vex.l) {
                for(int i=0; i<4; ++i)
                    GY->ud[i] = tmp32u;
            } else
                GY->u128 = 0;
            break;
        case 0x19:  /* VBROADCASTSD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            tmp64u = EX->q[0];
            for(int i=0; i<2; ++i)
                GX->q[i] = tmp64u;
            if(vex.l) {
                for(int i=0; i<2; ++i)
                    GY->q[i] = tmp64u;
            } else
                GY->u128 = 0;
            break;
        case 0x1A:  /* VBROADCASTF128 Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->u128 = EX->u128;
            GY->u128 = EX->u128;
            break;

        case 0x1C:  /* VPABSB Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for (int i=0; i<16; ++i) {
                GX->ub[i] = abs(EX->sb[i]);
            }
            if(vex.l) {
                GETEY;
                for (int i=0; i<16; ++i) {
                    GY->ub[i] = abs(EY->sb[i]);
                }
            } else
                GY->u128 = 0;
            break;
        case 0x1D:  /* VPABSW Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for (int i=0; i<8; ++i) {
                GX->uw[i] = abs(EX->sw[i]);
            }
            if(vex.l) {
                GETEY;
                for (int i=0; i<8; ++i) {
                    GY->uw[i] = abs(EY->sw[i]);
                }
            } else
                GY->u128 = 0;
            break;
        case 0x1E:  /* VPABSD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for (int i=0; i<4; ++i) {
                GX->ud[i] = abs(EX->sd[i]);
            }
            if(vex.l) {
                GETEY;
                for (int i=0; i<4; ++i) {
                    GY->ud[i] = abs(EY->sd[i]);
                }
            } else
                GY->u128 = 0;
            break;

        case 0x20:  /* VPMOVSXBW Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=7; i>=0; --i)
                    GY->sw[i] = EX->sb[8+i];
            else
                GY->u128 = 0;
            for(int i=7; i>=0; --i)
                GX->sw[i] = EX->sb[i];
            break;
        case 0x21:  /* VPMOVSXBD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=3; i>=0; --i)
                    GY->sd[i] = EX->sb[4+i];
            else
                GY->u128 = 0;
            for(int i=3; i>=0; --i)
                GX->sd[i] = EX->sb[i];
            break;
        case 0x22:  /* VPMOVSXBQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=1; i>=0; --i)
                    GY->sq[i] = EX->sb[2+i];
            else
                GY->u128 = 0;
            for(int i=1; i>=0; --i)
                GX->sq[i] = EX->sb[i];
            break;
        case 0x23:  /* VPMOVSXWD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=3; i>=0; --i)
                    GY->sd[i] = EX->sw[4+i];
            else
                GY->u128 = 0;
            for(int i=3; i>=0; --i)
                GX->sd[i] = EX->sw[i];
            break;
        case 0x24:  /* VPMOVSXWQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=1; i>=0; --i)
                    GY->sq[i] = EX->sw[2+i];
            else
                GY->u128 = 0;
            for(int i=1; i>=0; --i)
                GX->sq[i] = EX->sw[i];
            break;
        case 0x25:  /* VPMOVSXDQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=1; i>=0; --i)
                    GY->sq[i] = EX->sd[2+i];
            else
                GY->u128 = 0;
            for(int i=1; i>=0; --i)
                GX->sq[i] = EX->sd[i];
            break;

        case 0x28:  /* VPMULDQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GX->sq[1] = ((int64_t)VX->sd[2])*(int64_t)EX->sd[2];
            GX->sq[0] = ((int64_t)VX->sd[0])*(int64_t)EX->sd[0];
            if(vex.l) {
                GETEY; GETVY;
                GY->sq[1] = ((int64_t)VY->sd[2])*(int64_t)EY->sd[2];
                GY->sq[0] = ((int64_t)VY->sd[0])*(int64_t)EY->sd[0];
            } else
                GY->u128 = 0;
            break;
        case 0x29:  /* VPCMPEQQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=1; i>=0; --i)
                GX->sq[i] = (VX->sq[i]==EX->sq[i])?-1LL:0LL;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=1; i>=0; --i)
                    GY->sq[i] = (VY->sq[i]==EY->sq[i])?-1LL:0LL;
            } else
                GY->u128 = 0;
            break;
        case 0x2A:  /* VMOVNTDQA Gx, Ex */
            nextop = F8;
            if(!MODREG) {
                GETEX(0);
                GETGX;
                GETGY;
                GX->q[0] = EX->q[0];
                GX->q[1] = EX->q[1];
                if(vex.l) {
                    GETEY;
                    GY->q[0] = EY->q[0];
                    GY->q[1] = EY->q[1];
                } else
                    GY->u128 = 0;
            }
            break;
        case 0x2B:  /* VPACKUSDW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            GETEY;
            GETVY;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            for(int i=0; i<4; ++i)
                GX->uw[i] = (VX->sd[i]<0)?0:((VX->sd[i]>65535)?65535:VX->sd[i]);
            if(VX==EX)
                GX->q[1] = GX->q[0];
            else
                for(int i=0; i<4; ++i)
                    GX->uw[i+4] = (EX->sd[i]<0)?0:((EX->sd[i]>65535)?65535:EX->sd[i]);
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                for(int i=0; i<4; ++i)
                    GY->uw[i] = (VY->sd[i]<0)?0:((VY->sd[i]>65535)?65535:VY->sd[i]);
                if(VY==EY)
                    GY->q[1] = GY->q[0];
                else
                    for(int i=0; i<4; ++i)
                        GY->uw[i+4] = (EY->sd[i]<0)?0:((EY->sd[i]>65535)?65535:EY->sd[i]);
            } else
                GY->u128 = 0;
            break;
        case 0x2C:  /*VMASKMOVPS Gx, Vx, Ex */
            nextop = F8;
            if(MODREG)
                return 0;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (VX->ud[i]>>31)?EX->ud[i]:0;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (VY->ud[i]>>31)?EY->ud[i]:0;
            } else
                GY->u128 = 0;
            break;
        case 0x2D:  /*VMASKMOVPD Gx, Vx, Ex */
            nextop = F8;
            if(MODREG)
                return 0;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<2; ++i)
                GX->q[i] = (VX->q[i]>>63)?EX->q[i]:0;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    GY->q[i] = (VY->q[i]>>63)?EY->q[i]:0;
            } else
                GY->u128 = 0;
            break;
        case 0x2E:  /*VMASKMOVPS Ex, Vx, Gx */
            nextop = F8;
            if(MODREG)
                return 0;
            GETEX(0);
            GETGX;
            GETVX;
            for(int i=0; i<4; ++i)
                if(VX->ud[i]>>31) EX->ud[i] = GX->ud[i];
            if(vex.l) {
                GETGY;
                GETVY;
                #ifdef TEST_INTERPRETER
                if(VX->u128)
                #endif
                {
                    GETEY;
                    for(int i=0; i<4; ++i)
                        if(VY->ud[i]>>31) EY->ud[i] = GY->ud[i];
                }
            }
            break;
        case 0x2F:  /*VMASKMOVPD Ex, Vx, Gx */
            nextop = F8;
            if(MODREG)
                return 0;
            GETEX(0);
            GETGX;
            GETVX;
            for(int i=0; i<2; ++i)
                if(VX->q[i]>>63) EX->q[i] = GX->q[i];
            if(vex.l) {
                GETGY;
                GETVY;
                #ifdef TEST_INTERPRETER
                if(VX->u128)
                #endif
                {
                    GETEY;
                    for(int i=0; i<2; ++i)
                        if(VY->q[i]>>63) EY->q[i] = GY->q[i];
                }
            }
            break;
        case 0x30: /* VPMOVZXBW Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=7; i>=0; --i)
                    GY->uw[i] = EX->ub[7+1+i];
            else
                GY->u128 = 0;
            for(int i=7; i>=0; --i)
                GX->uw[i] = EX->ub[i];
            break;
        case 0x31: /* VPMOVZXBD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=3; i>=0; --i)
                    GY->ud[i] = EX->ub[3+1+i];
            else
                GY->u128 = 0;
            for(int i=3; i>=0; --i)
                GX->ud[i] = EX->ub[i];
            break;
        case 0x32: /* VPMOVZXBQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=1; i>=0; --i)
                    GY->q[i] = EX->ub[1+1+i];
            else
                GY->u128 = 0;
            for(int i=1; i>=0; --i)
                GX->q[i] = EX->ub[i];
            break;
        case 0x33: /* VPMOVZXWD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=3; i>=0; --i)
                    GY->ud[i] = EX->uw[3+1+i];
            else
                GY->u128 = 0;
            for(int i=3; i>=0; --i)
                GX->ud[i] = EX->uw[i];
            break;
        case 0x34: /* VPMOVZXWQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=1; i>=0; --i)
                    GY->q[i] = EX->uw[1+1+i];
            else
                GY->u128 = 0;
            for(int i=1; i>=0; --i)
                GX->q[i] = EX->uw[i];
            break;
        case 0x35: /* VPMOVZXDQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            if(vex.l)
                for(int i=1; i>=0; --i)
                    GY->q[i] = EX->ud[1+1+i];
            else
                GY->u128 = 0;
            for(int i=1; i>=0; --i)
                GX->q[i] = EX->ud[i];
            break;
        case 0x36:  /* VPERMD Gx, Vx, Ex */
            // same code as 0x16
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETEY;
            GETGY;
            GETVY;
            if(!vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
                eay1 = *EY;
                EY = &eay1;
            }
            if(GX==VX) {
                eax2 = *VX;
                VX = &eax2;
                eay2 = *VY;
                VY = &eay2;
            }
            for(int i=0; i<4; ++i) {
                u8 = VX->ud[i]&7;
                GX->ud[i] = (u8>3)?EY->ud[u8&3]:EX->ud[u8];
            }
            for(int i=0; i<4; ++i) {
                u8 = VY->ud[i]&7;
                GY->ud[i] = (u8>3)?EY->ud[u8&3]:EX->ud[u8];
            }
            break;
        case 0x37: /* VPCMPGTQ Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=1; i>=0; --i)
                GX->sq[i] = (VX->sq[i]>EX->sq[i])?-1LL:0LL;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=1; i>=0; --i)
                    GY->sq[i] = (VY->sq[i]>EY->sq[i])?-1LL:0LL;
            } else
                GY->u128 = 0;
            break;
        case 0x38:  /* VPMINSB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i)
                GX->sb[i] = (VX->sb[i]>EX->sb[i])?EX->sb[i]:VX->sb[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<16; ++i)
                    GY->sb[i] = (VY->sb[i]>EY->sb[i])?EY->sb[i]:VY->sb[i];
            } else
                GY->u128 = 0;
            break;
        case 0x39:  /* VPMINSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->sd[i] = (VX->sd[i]>EX->sd[i])?EX->sd[i]:VX->sd[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<4; ++i)
                    GY->sd[i] = (VY->sd[i]>EY->sd[i])?EY->sd[i]:VY->sd[i];
            } else
                GY->u128 = 0;
            break;
        case 0x3A:  /* VPMINUW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->uw[i] = (VX->uw[i]>EX->uw[i])?EX->uw[i]:VX->uw[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i)
                    GY->uw[i] = (VY->uw[i]>EY->uw[i])?EY->uw[i]:VY->uw[i];
            } else
                GY->u128 = 0;
            break;
        case 0x3B:  /* VPMINUD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (VX->ud[i]>EX->ud[i])?EX->ud[i]:VX->ud[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (VY->ud[i]>EY->ud[i])?EY->ud[i]:VY->ud[i];
            } else
                GY->u128 = 0;
            break;
        case 0x3C:  /* VPMAXSB Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<16; ++i)
                GX->sb[i] = (VX->sb[i]<EX->sb[i])?EX->sb[i]:VX->sb[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<16; ++i)
                    GY->sb[i] = (VY->sb[i]<EY->sb[i])?EY->sb[i]:VY->sb[i];
            } else
                GY->u128 = 0;
            break;
        case 0x3D:  /* VPMAXSD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->sd[i] = (VX->sd[i]<EX->sd[i])?EX->sd[i]:VX->sd[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<4; ++i)
                    GY->sd[i] = (VY->sd[i]<EY->sd[i])?EY->sd[i]:VY->sd[i];
            } else
                GY->u128 = 0;
            break;
        case 0x3E:  /* VPMAXUW Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->uw[i] = (VX->uw[i]<EX->uw[i])?EX->uw[i]:VX->uw[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<8; ++i)
                    GY->uw[i] = (VY->uw[i]<EY->uw[i])?EY->uw[i]:VY->uw[i];
            } else
                GY->u128 = 0;
            break;
        case 0x3F:  /* VPMAXUD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (VX->ud[i]<EX->ud[i])?EX->ud[i]:VX->ud[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (VY->ud[i]<EY->ud[i])?EY->ud[i]:VY->ud[i];
            } else
                GY->u128 = 0;
            break;
        case 0x40:  /* VPMULLD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = VX->ud[i] * EX->ud[i];
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = VY->ud[i] * EY->ud[i];
            } else
                GY->u128 = 0;
            break;
        case 0x41:  /* VPHMINPOSUW Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            tmp16u = EX->uw[0];
            tmp16s = 0;
            for(int i=1; i<8; ++i) {
                if(EX->uw[i]<tmp16u) {
                    tmp16u = EX->uw[i];
                    tmp16s = i;
                }
            }
            GX->q[1] = 0;
            GX->uw[0] = tmp16u;
            GX->uw[1] = tmp16s;
            GX->ud[1] = 0;
            GETGY;
            GY->u128 = 0;
            break;

        case 0x45:  /* VPSRLVD/Q Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i) {
                    tmp64u = EX->q[i];
                    GX->q[i] = (tmp64u<64)?(VX->q[i]>>tmp64u):0;
                }
                if(vex.l) {
                    GETEY; GETVY;
                    for(int i=0; i<2; ++i) {
                        tmp64u = EY->q[i];
                        GY->q[i] = (tmp64u<64)?(VY->q[i]>>tmp64u):0;
                    }
                }
            } else {
                for(int i=0; i<4; ++i) {
                    tmp32u = EX->ud[i];
                    GX->ud[i] = (tmp32u<32)?(VX->ud[i]>>tmp32u):0;
                }
                if(vex.l) {
                    GETEY; GETVY;
                    for(int i=0; i<4; ++i) {
                        tmp32u = EY->ud[i];
                        GY->ud[i] = (tmp32u<32)?(VY->ud[i]>>tmp32u):0;
                    }
                }
            }
            if(!vex.l)
                GY->u128=0;
            break;
        case 0x46:  /* VPSRAVD Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(rex.w) return 0;
            for(int i=0; i<4; ++i) {
                tmp32u = EX->ud[i]; if(tmp32u>31) tmp32u=31;
                GX->sd[i] = VX->sd[i]>>tmp32u;
            }
            if(vex.l) {
                GETEY; GETVY;
                for(int i=0; i<4; ++i) {
                    tmp32u = EY->ud[i]; if(tmp32u>31) tmp32u=31;
                    GY->sd[i] = VY->sd[i]>>tmp32u;
                }
            } else
                GY->u128=0;
            break;
        case 0x47:  /* VPSLLVD/Q Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX; GETVX; GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i) {
                    tmp64u = EX->q[i];
                    GX->q[i] = (tmp64u<64)?(VX->q[i]<<tmp64u):0;
                }
                if(vex.l) {
                    GETEY; GETVY;
                    for(int i=0; i<2; ++i) {
                        tmp64u = EY->q[i];
                        GY->q[i] = (tmp64u<64)?(VY->q[i]<<tmp64u):0;
                    }
                }
            } else {
                for(int i=0; i<4; ++i) {
                    tmp32u = EX->ud[i];
                    GX->ud[i] = (tmp32u<32)?(VX->ud[i]<<tmp32u):0;
                }
                if(vex.l) {
                    GETEY; GETVY;
                    for(int i=0; i<4; ++i) {
                        tmp32u = EY->ud[i];
                        GY->ud[i] = (tmp32u<32)?(VY->ud[i]<<tmp32u):0;
                    }
                }
            }
            if(!vex.l)
                GY->u128=0;
            break;

        case 0x58:  /* VPBROADCASTD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<4; ++i)
                GX->ud[i] = EX->ud[0];
            if(vex.l)
                for(int i=0; i<4; ++i)
                    GY->ud[i] = EX->ud[0];
            else
                GY->u128 = 0;
            break;
        case 0x59:  /* VPBROADCASTQ Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<2; ++i)
                GX->q[i] = EX->q[0];
            if(vex.l)
                for(int i=0; i<2; ++i)
                    GY->q[i] = EX->q[0];
            else
                GY->u128 = 0;
            break;
        case 0x5A:  /* VBROADCASTI128 Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            GX->u128 = EX->u128;
            GY->u128 = EX->u128;
            break;

        case 0x78:  /* VPBROADCASTB Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<16; ++i)
                GX->ub[i] = EX->ub[0];
            if(vex.l)
                for(int i=0; i<16; ++i)
                    GY->ub[i] = EX->ub[0];
            else
                GY->u128 = 0;
            break;
        case 0x79:  /* VPBROADCASTW Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            for(int i=0; i<8; ++i)
                GX->uw[i] = EX->uw[0];
            if(vex.l)
                for(int i=0; i<8; ++i)
                    GY->uw[i] = EX->uw[0];
            else
                GY->u128 = 0;
            break;

        case 0x8C:  /*VPMASKMOVD/Q Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->q[i] = (VX->q[i]>>63)?EX->q[i]:0;
                if(vex.l) {
                    GETEY;
                    GETVY;
                    for(int i=0; i<2; ++i)
                        GY->q[i] = (VY->q[i]>>63)?EY->q[i]:0;
                } else
                    GY->u128 = 0;
            } else {
                for(int i=0; i<4; ++i)
                    GX->ud[i] = (VX->ud[i]>>31)?EX->ud[i]:0;
                if(vex.l) {
                    GETEY;
                    GETVY;
                    for(int i=0; i<4; ++i)
                        GY->ud[i] = (VY->ud[i]>>31)?EY->ud[i]:0;
                } else
                    GY->u128 = 0;
            }
            break;
        case 0x8E:  /*VPMASKMOVD/Q Ex, Vx, Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    if(VX->q[i]>>63) EX->q[i] = GX->q[i];
                if(vex.l) {
                    GETGY;
                    GETEY;
                    GETVY;
                    for(int i=0; i<2; ++i)
                        if(VY->q[i]>>63) EY->q[i] = GY->q[i];
                }
            } else {
                for(int i=0; i<4; ++i)
                    if(VX->ud[i]>>31) EX->ud[i] = GX->ud[i];
                if(vex.l) {
                    GETGY;
                    GETEY;
                    GETVY;
                    for(int i=0; i<4; ++i)
                        if(VY->ud[i]>>31) EY->ud[i] = GY->ud[i];
                }
            }
            break;

        case 0x90:  /* VPGATHERDD/VPGATHERDQ Gx, VSIB, Vx */
        case 0x92:  /* VGATHERDPD/VGATHERDPS Gx, VSIB, Vx */
            nextop = F8;
            if(((nextop&7)!=4) || MODREG) {
                EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            }
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            tmp8u = F8; //SIB
            // compute base
            tmp64u = ((tmp8u&0x7)==5 && !(nextop&0xC0))?(F32S64):emu->regs[(tmp8u&0x7)+(rex.b<<3)].q[0];
            if(nextop&0x40)
                tmp64u += F8S;
            else if(nextop&0x80)
                tmp64u += F32S;
            // get vxmm
            EX = &emu->xmm[((tmp8u>>3)&7)+(rex.x<<3)];
            EY = &emu->ymm[((tmp8u>>3)&7)+(rex.x<<3)];
            u8 = tmp8u>>6;
            // prepare mask
            if(!vex.l)
                VY->u128 = 0;
            if(rex.w)
                for(int i=0; i<2; ++i)
                    VX->sq[i]>>=63;
            else
                for(int i=0; i<4; ++i)
                    VX->sd[i]>>=31;
            // go gather
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    if(VX->q[i]) {
                        uint64_t data_addr = (tmp64u + (EX->sd[i]<<u8));
                        if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                        GX->q[i] = *(uint64_t*)data_addr;
                        VX->q[i] = 0;
                    }
            } else {
                for(int i=0; i<4; ++i)
                    if(VX->ud[i]) {
                        uint64_t data_addr = (tmp64u + (EX->sd[i]<<u8));
                        if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                        GX->ud[i] = *(uint32_t*)data_addr;
                        VX->ud[i] = 0;
                    }
            }
            if(vex.l) {
                if(rex.w)
                    for(int i=0; i<2; ++i)
                        VY->sq[i]>>=63;
                else
                    for(int i=0; i<4; ++i)
                        VY->sd[i]>>=31;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        if(VY->q[i]) {
                            uint64_t data_addr = (tmp64u + (EX->sd[2+i]<<u8));
                            if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                            GY->q[i] = *(uint64_t*)data_addr;
                            VY->q[i] = 0;
                        }
                } else {
                    for(int i=0; i<4; ++i)
                        if(VY->ud[i]) {
                            uint64_t data_addr = (tmp64u + (EY->sd[i]<<u8));
                            if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                            GY->ud[i] = *(uint32_t*)data_addr;
                            VY->ud[i] = 0;
                        }
                }
            } else
                GY->u128 = 0;
            break;
        case 0x91:  /* VPGATHERQD/VPGATHERQQ Gx, VSIB, Vx */
        case 0x93:  /* VGATHERQPD/VGATHERQPS Gx, VSIB, Vx */
            nextop = F8;
            if(((nextop&7)!=4) || MODREG) {
                EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            }
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            tmp8u = F8; //SIB
            // compute base
            tmp64u = (((tmp8u&0x7)==5) && !(nextop&0xC0))?(F32S64):emu->regs[(tmp8u&0x7)+(rex.b<<3)].q[0];
            if(nextop&0x40)
                tmp64u += F8S;
            else if(nextop&0x80)
                tmp64u += F32S;
            // get vxmm
            EX = &emu->xmm[((tmp8u>>3)&7)+(rex.x<<3)];
            EY = &emu->ymm[((tmp8u>>3)&7)+(rex.x<<3)];
            u8 = tmp8u>>6;
            // prepare mask
            if(!vex.l) {
                VY->u128 = 0;
            }
            if(!vex.l || !rex.w)
                GY->u128 = 0;
            if(rex.w)
                for(int i=0; i<2; ++i)
                    VX->sq[i]>>=63;
            else
                for(int i=0; i<4; ++i)
                    VX->sd[i]>>=31;
            // go gather
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    if(VX->q[i]) {
                        uint64_t data_addr = (tmp64u + (EX->sq[i]<<u8));
                        if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                        GX->q[i] = *(uint64_t*)data_addr;
                        VX->q[i] = 0;
                    }
            } else {
                for(int i=0; i<(vex.l?4:2); ++i)
                    if(VX->ud[i]) {
                        uint64_t data_addr = (tmp64u + (((i>1)?EY->sq[i-2]:EX->sq[i])<<u8));
                        if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                        GX->ud[i] = *(uint32_t*)data_addr;
                        VX->ud[i] = 0;
                    }
            }
            if(vex.l) {
                if(rex.w)
                    for(int i=0; i<2; ++i)
                        VY->sq[i]>>=63;
                else
                    VY->u128=0;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        if(VY->q[i]) {
                            uint64_t data_addr = (tmp64u + (EY->sq[i]<<u8));
                            if (box64_is32bits) data_addr = (uintptr_t)(uint32_t)data_addr;
                            GY->q[i] = *(uint64_t*)data_addr;
                            VY->q[i] = 0;
                        }
                } else {
                    VY->u128 = 0;
                }
            }
            if(!rex.w && !vex.l) {
                GX->q[1] = 0;
                VX->q[1] = 0;
            }
            break;

        case 0x96:  /* VFMADDSUB132PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = GX->d[i]*EX->d[i] + ((i&1)?VX->d[i]:(-VX->d[i]));
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = GX->f[i]*EX->f[i] + ((i&1)?VX->f[i]:(-VX->f[i]));
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = GY->d[i]*EY->d[i] + ((i&1)?VY->d[i]:(-VY->d[i]));
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = GY->f[i]*EY->f[i] + ((i&1)?VY->f[i]:(-VY->f[i]));
                }
            } else GY->u128 = 0;
            break;
        case 0x97:  /* VFMSUBADD132PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = GX->d[i]*EX->d[i] + ((i&1)?(-VX->d[i]):VX->d[i]);
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = GX->f[i]*EX->f[i] + ((i&1)?(-VX->f[i]):VX->f[i]);
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = GY->d[i]*EY->d[i] + ((i&1)?(-VY->d[i]):VY->d[i]);
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = GY->f[i]*EY->f[i] + ((i&1)?(-VY->f[i]):VY->f[i]);
                }
            } else GY->u128 = 0;
            break;
        case 0x98:  /* VFMADD132PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = GX->d[i]*EX->d[i] + VX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = GX->f[i]*EX->f[i] + VX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = GY->d[i]*EY->d[i] + VY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = GY->f[i]*EY->f[i] + VY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0x99:  /* VFMADD132SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = GX->d[0]*EX->d[0] + VX->d[0];
            } else {
                GX->f[0] = GX->f[0]*EX->f[0] + VX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0x9A:  /* VFMSUB132PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = GX->d[i]*EX->d[i] - VX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = GX->f[i]*EX->f[i] - VX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = GY->d[i]*EY->d[i] - VY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = GY->f[i]*EY->f[i] - VY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0x9B:  /* VFMSUB132SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = GX->d[0]*EX->d[0] - VX->d[0];
            } else {
                GX->f[0] = GX->f[0]*EX->f[0] - VX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0x9C:  /* VFNMADD132PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = -GX->d[i]*EX->d[i] + VX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = -GX->f[i]*EX->f[i] + VX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = -GY->d[i]*EY->d[i] + VY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = -GY->f[i]*EY->f[i] + VY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0x9D:  /* VFNMADD132SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = -GX->d[0]*EX->d[0] + VX->d[0];
            } else {
                GX->f[0] = -GX->f[0]*EX->f[0] + VX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0x9E:  /* VFNMSUB132PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = -GX->d[i]*EX->d[i] - VX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = -GX->f[i]*EX->f[i] - VX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = -GY->d[i]*EY->d[i] - VY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = -GY->f[i]*EY->f[i] - VY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0x9F:  /* VFNMSUB132SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = -GX->d[0]*EX->d[0] - VX->d[0];
            } else {
                GX->f[0] = -GX->f[0]*EX->f[0] - VX->f[0];
            }
            GY->u128 = 0;
            break;

        case 0xA6:  /* VFMADDSUB213PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*GX->d[i] + ((i&1)?EX->d[i]:(-EX->d[i]));
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*GX->f[i] + ((i&1)?EX->f[i]:(-EX->f[i]));
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*GY->d[i] + ((i&1)?EY->d[i]:(-EY->d[i]));
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*GY->f[i] + ((i&1)?EY->f[i]:(-EY->f[i]));
                }
            } else GY->u128 = 0;
            break;
        case 0xA7:  /* VFMSUBADD213PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*GX->d[i] + ((i&1)?(-EX->d[i]):EX->d[i]);
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*GX->f[i] + ((i&1)?(-EX->f[i]):EX->f[i]);
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*GY->d[i] + ((i&1)?(-EY->d[i]):EY->d[i]);
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*GY->f[i] + ((i&1)?(-EY->f[i]):EY->f[i]);
                }
            } else GY->u128 = 0;
            break;
        case 0xA8:  /* VFMADD213PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*GX->d[i] + EX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*GX->f[i] + EX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*GY->d[i] + EY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*GY->f[i] + EY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xA9:  /* VFMADD213SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = VX->d[0]*GX->d[0] + EX->d[0];
            } else {
                GX->f[0] = VX->f[0]*GX->f[0] + EX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0xAA:  /* VFMSUB213PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*GX->d[i] - EX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*GX->f[i] - EX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*GY->d[i] - EY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*GY->f[i] - EY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xAB:  /* VFMSUB213SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = VX->d[0]*GX->d[0] - EX->d[0];
            } else {
                GX->f[0] = VX->f[0]*GX->f[0] - EX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0xAC:  /* VFNMADD213PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = -VX->d[i]*GX->d[i] + EX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = -VX->f[i]*GX->f[i] + EX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = -VY->d[i]*GY->d[i] + EY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = -VY->f[i]*GY->f[i] + EY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xAD:  /* VFNMADD213SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = -VX->d[0]*GX->d[0] + EX->d[0];
            } else {
                GX->f[0] = -VX->f[0]*GX->f[0] + EX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0xAE:  /* VFNMSUB213PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = -VX->d[i]*GX->d[i] - EX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = -VX->f[i]*GX->f[i] - EX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = -VY->d[i]*GY->d[i] - EY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = -VY->f[i]*GY->f[i] - EY->f[i];
                }
            } else GY->u128 = 0;
            break;        
        case 0xAF:  /* VFNMSUB213SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = -VX->d[0]*GX->d[0] - EX->d[0];
            } else {
                GX->f[0] = -VX->f[0]*GX->f[0] - EX->f[0];
            }
            GY->u128 = 0;
            break;

        case 0xB6:  /* VFMADDSUB231PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*EX->d[i] + ((i&1)?GX->d[i]:(-GX->d[i]));
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*EX->f[i] + ((i&1)?GX->f[i]:(-GX->f[i]));
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*EY->d[i] + ((i&1)?GY->d[i]:(-GY->d[i]));
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*EY->f[i] + ((i&1)?GY->f[i]:(-GY->f[i]));
                }
            } else GY->u128 = 0;
            break;
        case 0xB7:  /* VFMSUBADD231PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*EX->d[i] + ((i&1)?(-GX->d[i]):GX->d[i]);
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*EX->f[i] + ((i&1)?(-GX->f[i]):GX->f[i]);
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*EY->d[i] + ((i&1)?(-GY->d[i]):GY->d[i]);
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*EY->f[i] + ((i&1)?(-GY->f[i]):GY->f[i]);
                }
            } else GY->u128 = 0;
            break;
        case 0xB8:  /* VFMADD231PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*EX->d[i] + GX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*EX->f[i] + GX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*EY->d[i] + GY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*EY->f[i] + GY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xB9:  /* VFMADD231SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = VX->d[0]*EX->d[0] + GX->d[0];
            } else {
                GX->f[0] = VX->f[0]*EX->f[0] + GX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0xBA:  /* VFMSUB231PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = VX->d[i]*EX->d[i] - GX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = VX->f[i]*EX->f[i] - GX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = VY->d[i]*EY->d[i] - GY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = VY->f[i]*EY->f[i] - GY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xBB:  /* VFMSUB231SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = VX->d[0]*EX->d[0] - GX->d[0];
            } else {
                GX->f[0] = VX->f[0]*EX->f[0] - GX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0xBC:  /* VFNMADD231PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = -VX->d[i]*EX->d[i] + GX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = -VX->f[i]*EX->f[i] + GX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = -VY->d[i]*EY->d[i] + GY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = -VY->f[i]*EY->f[i] + GY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xBD:  /* VFNMADD231SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = -VX->d[0]*EX->d[0] + GX->d[0];
            } else {
                GX->f[0] = -VX->f[0]*EX->f[0] + GX->f[0];
            }
            GY->u128 = 0;
            break;
        case 0xBE:  /* VFNMSUB231PS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                for(int i=0; i<2; ++i)
                    GX->d[i] = -VX->d[i]*EX->d[i] - GX->d[i];
            } else {
                for(int i=0; i<4; ++i)
                    GX->f[i] = -VX->f[i]*EX->f[i] - GX->f[i];
            }
            if(vex.l) {
                GETEY; GETVY;
                if(rex.w) {
                    for(int i=0; i<2; ++i)
                        GY->d[i] = -VY->d[i]*EY->d[i] - GY->d[i];
                } else {
                    for(int i=0; i<4; ++i)
                        GY->f[i] = -VY->f[i]*EY->f[i] - GY->f[i];
                }
            } else GY->u128 = 0;
            break;
        case 0xBF:  /* VFNMSUB231SS/D Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            if(rex.w) {
                GX->d[0] = -VX->d[0]*EX->d[0] - GX->d[0];
            } else {
                GX->f[0] = -VX->f[0]*EX->f[0] - GX->f[0];
            }
            GY->u128 = 0;
            break;

        case 0xDB:  /* VAESIMC Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETGY;
            //STATE ← InvMixColumns( STATE );
            if (EX == GX) {
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = EX->ub[i];
                for(int j=0; j<4; ++j) {
                    GX->ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
                    GX->ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
                    GX->ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
                    GX->ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
                }
            } else {
                for(int j=0; j<4; ++j) {
                    GX->ub[0+j*4] = ff_mult(0x0E, EX->ub[0+j*4]) ^ ff_mult(0x0B, EX->ub[1+j*4]) ^ ff_mult(0x0D, EX->ub[2+j*4]) ^ ff_mult(0x09, EX->ub[3+j*4]);
                    GX->ub[1+j*4] = ff_mult(0x09, EX->ub[0+j*4]) ^ ff_mult(0x0E, EX->ub[1+j*4]) ^ ff_mult(0x0B, EX->ub[2+j*4]) ^ ff_mult(0x0D, EX->ub[3+j*4]);
                    GX->ub[2+j*4] = ff_mult(0x0D, EX->ub[0+j*4]) ^ ff_mult(0x09, EX->ub[1+j*4]) ^ ff_mult(0x0E, EX->ub[2+j*4]) ^ ff_mult(0x0B, EX->ub[3+j*4]);
                    GX->ub[3+j*4] = ff_mult(0x0B, EX->ub[0+j*4]) ^ ff_mult(0x0D, EX->ub[1+j*4]) ^ ff_mult(0x09, EX->ub[2+j*4]) ^ ff_mult(0x0E, EX->ub[3+j*4]);
                }
            }
            GY->u128 = 0;
            break;
        case 0xDC:  /* VAESENC Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            //STATE ← SRC1;
            //RoundKey ← SRC2;
            //STATE ← ShiftRows( STATE );
            //STATE ← SubBytes( STATE );
            for(int i=0; i<16; ++i)
                eax1.ub[i] = subbytes[VX->ub[shiftrows[i]]];
            //STATE ← MixColumns( STATE );
            for(int j=0; j<4; ++j) {
                eay1.ub[0+j*4] = ff_mult(0x02, eax1.ub[0+j*4]) ^ ff_mult(0x03, eax1.ub[1+j*4]) ^               eax1.ub[2+j*4]  ^               eax1.ub[3+j*4] ;
                eay1.ub[1+j*4] =               eax1.ub[0+j*4]  ^ ff_mult(0x02, eax1.ub[1+j*4]) ^ ff_mult(0x03, eax1.ub[2+j*4]) ^               eax1.ub[3+j*4] ;
                eay1.ub[2+j*4] =               eax1.ub[0+j*4]  ^               eax1.ub[1+j*4]  ^ ff_mult(0x02, eax1.ub[2+j*4]) ^ ff_mult(0x03, eax1.ub[3+j*4]);
                eay1.ub[3+j*4] = ff_mult(0x03, eax1.ub[0+j*4]) ^               eax1.ub[1+j*4]  ^               eax1.ub[2+j*4]  ^ ff_mult(0x02, eax1.ub[3+j*4]);
            }
            //DEST[127:0] ← STATE XOR RoundKey;
            GX->u128 = eay1.u128 ^ EX->u128;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = subbytes[VY->ub[shiftrows[i]]];
                for(int j=0; j<4; ++j) {
                    eay1.ub[0+j*4] = ff_mult(0x02, eax1.ub[0+j*4]) ^ ff_mult(0x03, eax1.ub[1+j*4]) ^               eax1.ub[2+j*4]  ^               eax1.ub[3+j*4] ;
                    eay1.ub[1+j*4] =               eax1.ub[0+j*4]  ^ ff_mult(0x02, eax1.ub[1+j*4]) ^ ff_mult(0x03, eax1.ub[2+j*4]) ^               eax1.ub[3+j*4] ;
                    eay1.ub[2+j*4] =               eax1.ub[0+j*4]  ^               eax1.ub[1+j*4]  ^ ff_mult(0x02, eax1.ub[2+j*4]) ^ ff_mult(0x03, eax1.ub[3+j*4]);
                    eay1.ub[3+j*4] = ff_mult(0x03, eax1.ub[0+j*4]) ^               eax1.ub[1+j*4]  ^               eax1.ub[2+j*4]  ^ ff_mult(0x02, eax1.ub[3+j*4]);
                }
                GY->u128 = eay1.u128 ^ EY->u128;
            } else
                GY->u128 = 0; 
            break;
        case 0xDD:  /* VAESENCLAST Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            //STATE ← SRC1;
            //RoundKey ← SRC2;
            //STATE ← ShiftRows( STATE );
            //STATE ← SubBytes( STATE );
            for(int i=0; i<16; ++i)
                eax1.ub[i] = subbytes[VX->ub[shiftrows[i]]];
            //DEST[127:0] ← STATE XOR RoundKey;
            GX->u128 = eax1.u128 ^ EX->u128;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = subbytes[VY->ub[shiftrows[i]]];
                GY->u128 = eax1.u128 ^ EY->u128;
            } else
                GY->u128 = 0;
            break;
        case 0xDE:  /* VAESDEC Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            //STATE ← SRC1;
            //RoundKey ← SRC2;
            //STATE ← InvShiftRows( STATE );
            for(int i=0; i<16; ++i)
                eax1.ub[i] = VX->ub[invshiftrows[i]];
            //STATE ← InvSubBytes( STATE );
            for(int i=0; i<16; ++i)
                eax1.ub[i] = invsubbytes[eax1.ub[i]];
            //STATE ← InvMixColumns( STATE );
            for(int j=0; j<4; ++j) {
                eay1.ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
                eay1.ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
                eay1.ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
                eay1.ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
            }
            //DEST[127:0] ← STATE XOR RoundKey;
            GX->u128 = eay1.u128 ^ EX->u128;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = invsubbytes[VY->ub[invshiftrows[i]]];
                for(int j=0; j<4; ++j) {
                    eay1.ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
                    eay1.ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
                    eay1.ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
                    eay1.ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
                }
                GY->u128 = eay1.u128 ^ EY->u128;
            } else
                GY->u128 = 0;
            break;
        case 0xDF:  /* VAESDECLAST Gx, Vx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GETVX;
            GETGY;
            //STATE ← SRC1;
            //RoundKey ← SRC2;
            //STATE ← InvShiftRows( STATE );
            //STATE ← InvSubBytes( STATE );
            for(int i=0; i<16; ++i)
                eax1.ub[i] = invsubbytes[VX->ub[invshiftrows[i]]];
            //DEST[127:0] ← STATE XOR RoundKey;
            GX->u128 = eax1.u128 ^ EX->u128;
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = invsubbytes[VY->ub[invshiftrows[i]]];
                GY->u128 = eax1.u128 ^ EY->u128;
            } else
                GY->u128 = 0;
            break;

        case 0xF7:  /* SHLX Gd, Ed, Vd */
            nextop = F8;
            GETED(0);
            GETGD;
            GETVD;
            if(rex.w) {
                u8 = VD->q[0] & 0x3f;
                GD->q[0] = ED->q[0] << u8;
            } else {
                u8 = VD->dword[0] & 0x1f;
                GD->q[0] = ED->dword[0] << u8;
            }
            break;

        default:
            return 0;
    }
    return addr;
}
