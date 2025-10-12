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
#include "emit_signals.h"
#include "bridge.h"
#ifdef DYNAREC
#include "custommem.h"
#endif

#include "modrm.h"
#include "x64compstrings.h"

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
uintptr_t Test660F(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t Run660F(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int16_t tmp16s;
    uint16_t tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s, i64[4];
    float tmpf;
    double tmpd;
    int is_nan;
    int mask_nan[4];
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1, *opex2, eax2;
    mmx87_regs_t *opem, *opgm;
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
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    switch(opcode) {

    case 0x10:                      /* MOVUPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(GX, EX, 16); // unaligned...
        break;
    case 0x11:                      /* MOVUPD Ex, Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(EX, GX, 16); // unaligned...
        break;
    case 0x12:                      /* MOVLPD Gx, Eq */
        nextop = F8;
        if(!MODREG) {
            GETE8(0);
            GETGX;
            GX->q[0] = ED->q[0];
        }
        break;
    case 0x13:                      /* MOVLPD Eq, Gx */
        nextop = F8;
        if(!MODREG) {
            GETE8(0);
            GETGX;
            ED->q[0] = GX->q[0];
        }
        break;
    case 0x14:                      /* UNPCKLPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[1] = EX->q[0];
        break;
    case 0x15:                      /* UNPCKHPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = GX->q[1];
        GX->q[1] = EX->q[1];
        break;
    case 0x16:                      /* MOVHPD Gx, Ed */
        nextop = F8;
        if(!MODREG) {
            GETE8(0);
            GETGX;
            GX->q[1] = ED->q[0];
        }
        break;
    case 0x17:                      /* MOVHPD Ed, Gx */
        nextop = F8;
        if(!MODREG) {
            GETE8(0);
            GETGX;
            ED->q[0] = GX->q[1];
        }
        break;

    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:                      /* NOP (multi-byte) */
        nextop = F8;
        FAKEED(0);
        break;

    case 0x28:                      /* MOVAPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        GX->q[1] = EX->q[1];
        break;
    case 0x29:                      /* MOVAPD Ex, Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->q[0] = GX->q[0];
        EX->q[1] = GX->q[1];
        break;
    case 0x2A:                      /* CVTPI2PD Gx, Em */
        nextop = F8;
        GETEM(0);
        GETGX;
        GX->d[0] = EM->sd[0];
        GX->d[1] = EM->sd[1];
        break;
    case 0x2B:                      /* MOVNTPD Ex, Gx */
        nextop = F8;
        if(!MODREG) {
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
        }
        break;
    case 0x2C:                      /* CVTTPD2PI Gm, Ex */
        nextop = F8;
        GETEX(0);
        GETGM;
        tmp64s = EX->d[0];
        if (tmp64s==(int32_t)tmp64s && !isnan(EX->d[0]))
            GM->sd[0] = (int32_t)tmp64s;
        else
            GM->sd[0] = INT32_MIN;
        tmp64s = EX->d[1];
        if (tmp64s==(int32_t)tmp64s && !isnan(EX->d[1]))
            GM->sd[1] = (int32_t)tmp64s;
        else
            GM->sd[1] = INT32_MIN;
        break;
    case 0x2D:                      /* CVTPD2PI Gm, Ex */
        nextop = F8;
        GETEX(0);
        GETGM;
        switch(emu->mxcsr.f.MXCSR_RC) {
            case ROUND_Nearest: {
                int round = fegetround();
                fesetround(FE_TONEAREST);
                i64[0] = nearbyint(EX->d[0]);
                i64[1] = nearbyint(EX->d[1]);
                fesetround(round);
                break;
            }
            case ROUND_Down:
                i64[0] = floor(EX->d[0]);
                i64[1] = floor(EX->d[1]);
                break;
            case ROUND_Up:
                i64[0] = ceil(EX->d[0]);
                i64[1] = ceil(EX->d[1]);
                break;
            case ROUND_Chop:
                i64[0] = EX->d[0];
                i64[1] = EX->d[1];
                break;
        }
        for(int i=0; i<2; ++i)
            if (i64[i]==(int32_t)i64[i] && !isnan(EX->d[i]))
                GM->sd[i] = (int32_t)i64[i];
            else
                GM->sd[i] = INT32_MIN;

        break;
    case 0x2E:                      /* UCOMISD Gx, Ex */
        // no special check...
    case 0x2F:                      /* COMISD Gx, Ex */
        RESET_FLAGS(emu);
        nextop = F8;
        GETEX(0);
        GETGX;
        if(isnan(GX->d[0]) || isnan(EX->d[0])) {
            SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
        } else if(isgreater(GX->d[0], EX->d[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        } else if(isless(GX->d[0], EX->d[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
        } else {
            SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        }
        CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
        break;

    case 0x38:  // SSSE3 opcodes
        opcode = F8;
        switch(opcode) {
            case 0x00:  /* PSHUFB */
                nextop = F8;
                GETEX(0);
                GETGX;
                eax1 = *GX;
                for (int i=0; i<16; ++i) {
                    if(EX->ub[i]&128)
                        GX->ub[i] = 0;
                    else
                        GX->ub[i] = eax1.ub[EX->ub[i]&15];
                }
                break;
            case 0x01:  /* PHADDW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i)
                    GX->sw[i] = GX->sw[i*2+0]+GX->sw[i*2+1];
                if(GX == EX) {
                    GX->q[1] = GX->q[0];
                } else {
                    for (int i=0; i<4; ++i)
                        GX->sw[4+i] = EX->sw[i*2+0] + EX->sw[i*2+1];
                }
                break;
            case 0x02:  /* PHADDD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                GX->sd[0] += GX->sd[1];
                GX->sd[1] = GX->sd[2] + GX->sd[3];
                if(GX == EX) {
                    GX->q[1] = GX->q[0];
                } else {
                    GX->sd[2] = EX->sd[0] + EX->sd[1];
                    GX->sd[3] = EX->sd[2] + EX->sd[3];
                }
                break;
            case 0x03:  /* PHADDSW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i) {
                    tmp32s = GX->sw[i*2+0]+GX->sw[i*2+1];
                    GX->sw[i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                }
                if(GX == EX) {
                    GX->q[1] = GX->q[0];
                } else {
                    for (int i=0; i<4; ++i) {
                        tmp32s = EX->sw[i*2+0] + EX->sw[i*2+1];
                        GX->sw[4+i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                    }
                }
                break;
            case 0x04:  /* PMADDUBSW Gx,Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                if(GX==EX) {
                    eax1 = *EX;
                    EX=&eax1;
                }
                for (int i=0; i<8; ++i) {
                    tmp32s = (int32_t)(GX->ub[i*2+0])*EX->sb[i*2+0] + (int32_t)(GX->ub[i*2+1])*EX->sb[i*2+1];
                    GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                }
                break;
            case 0x05:  /* PHSUBW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i)
                    GX->sw[i] = GX->sw[i*2+0] - GX->sw[i*2+1];
                if(GX == EX) {
                    GX->q[1] = GX->q[0];
                } else {
                    for (int i=0; i<4; ++i)
                        GX->sw[4+i] = EX->sw[i*2+0] - EX->sw[i*2+1];
                }
                break;
            case 0x06:  /* PHSUBD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<2; ++i)
                    GX->sd[i] = GX->sd[i*2+0] - GX->sd[i*2+1];
                if(GX == EX) {
                    GX->q[1] = GX->q[0];
                } else {
                    for (int i=0; i<2; ++i)
                        GX->sd[2+i] = EX->sd[i*2+0] - EX->sd[i*2+1];
                }
                break;
            case 0x07:  /* PHSUBSW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i) {
                    tmp32s = GX->sw[i*2+0] - GX->sw[i*2+1];
                    GX->sw[i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                }
                if(GX == EX) {
                    GX->q[1] = GX->q[0];
                } else {
                    for (int i=0; i<4; ++i) {
                        tmp32s = EX->sw[i*2+0] - EX->sw[i*2+1];
                        GX->sw[4+i] = (tmp32s<-32768)?-32768:((tmp32s>32767)?32767:tmp32s);
                    }
                }
                break;
            case 0x08:  /* PSIGNB Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<16; ++i)
                    GX->sb[i] *= (EX->sb[i]<0)?-1:((EX->sb[i]>0)?1:0);
                break;
            case 0x09:  /* PSIGNW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<8; ++i)
                    GX->sw[i] *= (EX->sw[i]<0)?-1:((EX->sw[i]>0)?1:0);
                break;
            case 0x0A:  /* PSIGND Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i)
                    GX->sd[i] *= (EX->sd[i]<0)?-1:((EX->sd[i]>0)?1:0);
                break;
            case 0x0B:  /* PMULHRSW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<8; ++i) {
                    tmp32s = ((((int32_t)(GX->sw[i])*(int32_t)(EX->sw[i]))>>14) + 1)>>1;
                    GX->uw[i] = tmp32s&0xffff;
                }
                break;

            case 0x10:  /* PBLENDVB Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<16; ++i) {
                    if(emu->xmm[0].ub[i]&0x80)
                        GX->ub[i] = EX->ub[i];
                }
                break;

            case 0x14:  /* BLENDVPS Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i) {
                    if(emu->xmm[0].ud[i]&0x80000000)
                        GX->ud[i] = EX->ud[i];
                }
                break;
            case 0x15:  /* BLENDVPD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<2; ++i) {
                    if(emu->xmm[0].q[i]&0x8000000000000000LL)
                        GX->q[i] = EX->q[i];
                }
                break;

            case 0x17:      // PTEST GX, EX
                nextop = F8;
                GETEX(0);
                GETGX;
                RESET_FLAGS(emu);
                CONDITIONAL_SET_FLAG(!(GX->u128&EX->u128), F_ZF);
                CONDITIONAL_SET_FLAG(!((~GX->u128)&EX->u128), F_CF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_PF);
                break;

            case 0x1C:  /* PABSB Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<16; ++i) {
                    GX->ub[i] = abs(EX->sb[i]);
                }
                break;
            case 0x1D:  /* PABSW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<8; ++i) {
                    GX->uw[i] = abs(EX->sw[i]);
                }
                break;
            case 0x1E:  /* PABSD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i) {
                    GX->ud[i] = abs(EX->sd[i]);
                }
                break;

            case 0x20:  /* PMOVSXBW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=7; i>=0; --i)
                    GX->sw[i] = EX->sb[i];
                break;
            case 0x21:  /* PMOVSXBD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=3; i>=0; --i)
                    GX->sd[i] = EX->sb[i];
                break;
            case 0x22:  /* PMOVSXBQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->sq[i] = EX->sb[i];
                break;
            case 0x23:  /* PMOVSXWD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=3; i>=0; --i)
                    GX->sd[i] = EX->sw[i];
                break;
            case 0x24:  /* PMOVSXWQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->sq[i] = EX->sw[i];
                break;
            case 0x25:  /* PMOVSXDQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->sq[i] = EX->sd[i];
                break;

            case 0x28:  /* PMULDQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                GX->sq[1] = ((int64_t)GX->sd[2])*(int64_t)EX->sd[2];
                GX->sq[0] = ((int64_t)GX->sd[0])*(int64_t)EX->sd[0];
                break;
            case 0x29:  /* PCMPEQQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->sq[i] = (GX->sq[i]==EX->sq[i])?-1LL:0LL;
                break;
            case 0x2A:  /* MOVNTDQA Gx, Ex */
                nextop = F8;
                if(!MODREG) {
                    GETEX(0);
                    GETGX;
                    GX->q[0] = EX->q[0];
                    GX->q[1] = EX->q[1];
                }
                break;
            case 0x2B:  /* PACKUSDW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<4; ++i)
                    GX->uw[i] = (GX->sd[i]<0)?0:((GX->sd[i]>65535)?65535:GX->sd[i]);
                if(GX==EX)
                    GX->q[1] = GX->q[0];
                else
                    for(int i=0; i<4; ++i)
                        GX->uw[i+4] = (EX->sd[i]<0)?0:((EX->sd[i]>65535)?65535:EX->sd[i]);
                break;

            case 0x30: /* PMOVZXBW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=7; i>=0; --i)
                    GX->uw[i] = EX->ub[i];
                break;
            case 0x31: /* PMOVZXBD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=3; i>=0; --i)
                    GX->ud[i] = EX->ub[i];
                break;
            case 0x32: /* PMOVZXBQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->q[i] = EX->ub[i];
                break;
            case 0x33: /* PMOVZXWD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=3; i>=0; --i)
                    GX->ud[i] = EX->uw[i];
                break;
            case 0x34: /* PMOVZXWQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->q[i] = EX->uw[i];
                break;
            case 0x35: /* PMOVZXDQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->q[i] = EX->ud[i];
                break;

            case 0x37: /* PCMPGTQ Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=1; i>=0; --i)
                    GX->sq[i] = (GX->sq[i]>EX->sq[i])?-1LL:0LL;
                break;
            case 0x38:  /* PMINSB Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<16; ++i)
                    if(GX->sb[i]>EX->sb[i])
                        GX->sb[i] = EX->sb[i];
                break;
            case 0x39:  /* PMINSD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<4; ++i)
                    if(GX->sd[i]>EX->sd[i])
                        GX->sd[i] = EX->sd[i];
                break;
            case 0x3A:  /* PMINUW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<8; ++i)
                    if(GX->uw[i]>EX->uw[i])
                        GX->uw[i] = EX->uw[i];
                break;
            case 0x3B:  /* PMINUD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<4; ++i)
                    if(GX->ud[i]>EX->ud[i])
                        GX->ud[i] = EX->ud[i];
                break;
            case 0x3C:  /* PMAXSB Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<16; ++i)
                    if(GX->sb[i]<EX->sb[i])
                        GX->sb[i] = EX->sb[i];
                break;
            case 0x3D:  /* PMAXSD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<4; ++i)
                    if(GX->sd[i]<EX->sd[i])
                        GX->sd[i] = EX->sd[i];
                break;
            case 0x3E:  /* PMAXUW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<8; ++i)
                    if(GX->uw[i]<EX->uw[i])
                        GX->uw[i] = EX->uw[i];
                break;
            case 0x3F:  /* PMAXUD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<4; ++i)
                    if(GX->ud[i]<EX->ud[i])
                        GX->ud[i] = EX->ud[i];
                break;
            case 0x40:  /* PMULLD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for(int i=0; i<4; ++i)
                    GX->ud[i] *= EX->ud[i];
                break;
            case 0x41:  /* PHMINPOSUW Gx, Ex */
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
                break;

            case 0x82:  /* INVPCID */
                nextop = F8;
                GETED(0);
                // this is a privilege opcode...
                #ifndef TEST_INTERPRETER
                EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
                #endif
                break;

            case 0xDB:  /* AESIMC Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
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
                break;
            case 0xDC:  /* AESENC Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                //STATE ← SRC1;
                //RoundKey ← SRC2;
                //STATE ← ShiftRows( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = GX->ub[shiftrows[i]];
                //STATE ← SubBytes( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = subbytes[eax1.ub[i]];
                //STATE ← MixColumns( STATE );
                for(int j=0; j<4; ++j) {
                    eax2.ub[0+j*4] = ff_mult(0x02, eax1.ub[0+j*4]) ^ ff_mult(0x03, eax1.ub[1+j*4]) ^               eax1.ub[2+j*4]  ^               eax1.ub[3+j*4] ;
                    eax2.ub[1+j*4] =               eax1.ub[0+j*4]  ^ ff_mult(0x02, eax1.ub[1+j*4]) ^ ff_mult(0x03, eax1.ub[2+j*4]) ^               eax1.ub[3+j*4] ;
                    eax2.ub[2+j*4] =               eax1.ub[0+j*4]  ^               eax1.ub[1+j*4]  ^ ff_mult(0x02, eax1.ub[2+j*4]) ^ ff_mult(0x03, eax1.ub[3+j*4]);
                    eax2.ub[3+j*4] = ff_mult(0x03, eax1.ub[0+j*4]) ^               eax1.ub[1+j*4]  ^               eax1.ub[2+j*4]  ^ ff_mult(0x02, eax1.ub[3+j*4]);
                }
                //DEST[127:0] ← STATE XOR RoundKey;
                GX->q[0] = eax2.q[0] ^ EX->q[0];
                GX->q[1] = eax2.q[1] ^ EX->q[1];
                break;
            case 0xDD:  /* AESENCLAST Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                //STATE ← SRC1;
                //RoundKey ← SRC2;
                //STATE ← ShiftRows( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = GX->ub[shiftrows[i]];
                //STATE ← SubBytes( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = subbytes[eax1.ub[i]];
                //DEST[127:0] ← STATE XOR RoundKey;
                GX->q[0] = eax1.q[0] ^ EX->q[0];
                GX->q[1] = eax1.q[1] ^ EX->q[1];
                break;
            case 0xDE:  /* AESDEC Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                //STATE ← SRC1;
                //RoundKey ← SRC2;
                //STATE ← InvShiftRows( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = GX->ub[invshiftrows[i]];
                //STATE ← InvSubBytes( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = invsubbytes[eax1.ub[i]];
                //STATE ← InvMixColumns( STATE );
                for(int j=0; j<4; ++j) {
                    eax2.ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
                    eax2.ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
                    eax2.ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
                    eax2.ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
                }
                //DEST[127:0] ← STATE XOR RoundKey;
                GX->q[0] = eax2.q[0] ^ EX->q[0];
                GX->q[1] = eax2.q[1] ^ EX->q[1];
                break;
            case 0xDF:  /* AESDECLAST Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                //STATE ← SRC1;
                //RoundKey ← SRC2;
                //STATE ← InvShiftRows( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = GX->ub[invshiftrows[i]];
                //STATE ← InvSubBytes( STATE );
                for(int i=0; i<16; ++i)
                    eax1.ub[i] = invsubbytes[eax1.ub[i]];
                //DEST[127:0] ← STATE XOR RoundKey;
                GX->q[0] = eax1.q[0] ^ EX->q[0];
                GX->q[1] = eax1.q[1] ^ EX->q[1];
                break;
            case 0xF0: /* MOVBE Gw, Ew */
                nextop = F8;
                GETED(0);
                GETGD;
                GD->word[0] = __builtin_bswap16(ED->word[0]);
                break;
            case 0xF1: /* MOVBE Ew, Gw */
                nextop = F8;
                GETED(0);
                GETGD;
                ED->word[0] = __builtin_bswap16(GD->word[0]);
                break;

            case 0xF6: /* ADCX Gd, Rd */
                nextop = F8;
                GETED(0);
                GETGD;
                CHECK_FLAGS(emu);
                if(rex.w) {
                    if (ACCESS_FLAG(F_CF)) {
                        tmp64u = 1 + (GD->q[0] & 0xFFFFFFFF) + (ED->q[0] & 0xFFFFFFFF);
                        tmp64u2 = 1 + GD->q[0] + ED->q[0];
                        }
                    else {
                        tmp64u = (GD->q[0] & 0xFFFFFFFF) + (ED->q[0] & 0xFFFFFFFF);
                        tmp64u2 = GD->q[0] + ED->q[0];
                        }
                    tmp64u = (tmp64u >> 32) + (GD->q[0] >> 32) + (ED->q[0] >> 32);
                    CONDITIONAL_SET_FLAG(tmp64u & 0x100000000LL, F_CF);
                    GD->q[0] = tmp64u2;
                } else {
                    if (ACCESS_FLAG(F_CF))
                        GD->q[0] = 1LL + GD->dword[0] + ED->dword[0];
                    else
                        GD->q[0] = (uint64_t)GD->dword[0] + ED->dword[0];
                    CONDITIONAL_SET_FLAG(GD->q[0] & 0x100000000LL, F_CF);
                    GD->dword[1] = 0;
                }
                break;
            default:
                return 0;
        }
        break;

    case 0x3A:  // these are some SSE3 & SSE4.x opcodes
        opcode = F8;
        switch(opcode) {
            case 0x08:          // ROUNDPS Gx, Ex, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8; // ignoring bit 3 interupt thingy
                if(tmp8u&4)
                    tmp8u = emu->mxcsr.f.MXCSR_RC;
                else
                    tmp8u &= 3;
                switch(tmp8u) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        for(int i=0; i<4; ++i)
                            GX->f[i] = nearbyintf(EX->f[i]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        for(int i=0; i<4; ++i)
                            GX->f[i] = floorf(EX->f[i]);
                        break;
                    case ROUND_Up:
                        for(int i=0; i<4; ++i)
                            GX->f[i] = ceilf(EX->f[i]);
                        break;
                    case ROUND_Chop:
                        for(int i=0; i<4; ++i)
                            GX->f[i] = truncf(EX->f[i]);
                        break;
                }
                break;
            case 0x09:          // ROUNDPD Gx, Ex, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8; // ignoring bit 3 interupt thingy
                if(tmp8u&4)
                    tmp8u = emu->mxcsr.f.MXCSR_RC;
                else
                    tmp8u &= 3;
                switch(tmp8u) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GX->d[0] = nearbyint(EX->d[0]);
                        GX->d[1] = nearbyint(EX->d[1]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GX->d[0] = floor(EX->d[0]);
                        GX->d[1] = floor(EX->d[1]);
                        break;
                    case ROUND_Up:
                        GX->d[0] = ceil(EX->d[0]);
                        GX->d[1] = ceil(EX->d[1]);
                        break;
                    case ROUND_Chop:
                        GX->d[0] = trunc(EX->d[0]);
                        GX->d[1] = trunc(EX->d[1]);
                        break;
                }
                break;
            case 0x0A:          // ROUNDSS Gx, Ex, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8; // ignoring bit 3 interupt thingy
                if(tmp8u&4)
                    tmp8u = emu->mxcsr.f.MXCSR_RC;
                else
                    tmp8u &= 3;
                switch(tmp8u) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GX->f[0] = nearbyintf(EX->f[0]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GX->f[0] = floorf(EX->f[0]);
                        break;
                    case ROUND_Up:
                        GX->f[0] = ceilf(EX->f[0]);
                        break;
                    case ROUND_Chop:
                        GX->f[0] = truncf(EX->f[0]);
                        break;
                }
                break;
            case 0x0B:          // ROUNDSD Gx, Ex, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8; // ignoring bit 3 interupt thingy
                if(tmp8u&4)
                    tmp8u = emu->mxcsr.f.MXCSR_RC;
                else
                    tmp8u &= 3;
                switch(tmp8u) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GX->d[0] = nearbyint(EX->d[0]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GX->d[0] = floor(EX->d[0]);
                        break;
                    case ROUND_Up:
                        GX->d[0] = ceil(EX->d[0]);
                        break;
                    case ROUND_Chop:
                        GX->d[0] = trunc(EX->d[0]);
                        break;
                }
                break;

            case 0x0C: /* BLENDPS Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                for (int i=0; i<4; ++i) {
                    if(tmp8u&(1<<i))
                        GX->ud[i] = EX->ud[i];
                }
                break;
            case 0x0D: /* BLENDPD Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                for (int i=0; i<2; ++i) {
                    if(tmp8u&(1<<i))
                        GX->q[i] = EX->q[i];
                }
                break;
            case 0x0E:  /* PBLENDW Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                for (int i=0; i<8; ++i) {
                    if(tmp8u&(1<<i))
                        GX->uw[i] = EX->uw[i];
                }
                break;
            case 0x0F:          // PALIGNR GX, EX, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                if(tmp8u>31)
                    {GX->q[0] = GX->q[1] = 0;}
                else
                {
                    for (int i=0; i<16; ++i, ++tmp8u)
                        eax1.ub[i] = (tmp8u>15)?((tmp8u>31)?0:GX->ub[tmp8u-16]):EX->ub[tmp8u];
                    GX->q[0] = eax1.q[0];
                    GX->q[1] = eax1.q[1];
                }
                break;

            case 0x14:      // PEXTRB ED, GX, u8
                nextop = F8;
                GETED(1);
                GETGX;
                tmp8u = F8;
                if(MODREG)
                    ED->q[0] = GX->ub[tmp8u&0x0f];
                else
                    ED->byte[0] = GX->ub[tmp8u&0x0f];
                break;
            case 0x15:      // PEXTRW Ew,Gx,Ib
                nextop = F8;
                GETED(1);
                GETGX;
                tmp8u = F8;
                if(MODREG)
                    ED->q[0] = GX->uw[tmp8u&7];  // 16bits extract, 0 extended
                else
                    ED->word[0] = GX->uw[tmp8u&7];
                break;
            case 0x16:      // PEXTRD/Q ED, GX, u8
                nextop = F8;
                GETED(1);
                GETGX;
                tmp8u = F8;
                if(rex.w) {
                    ED->q[0] = GX->q[tmp8u&1];
                } else {
                    if(MODREG)
                        ED->q[0] = GX->ud[tmp8u&3];
                    else
                        ED->dword[0] = GX->ud[tmp8u&3];
                }
                break;
            case 0x17:      // EXTRACTPS ED, GX, u8
                nextop = F8;
                GETED(1);
                GETGX;
                tmp8u = F8;
                ED->dword[0] = GX->ud[tmp8u&3];
                if(MODREG) ED->dword[1] = 0;
                break;

            case 0x20:      // PINSRB GX, ED, u8
                nextop = F8;
                GETED(1);   // It's ED, and not EB
                GETGX;
                tmp8u = F8;
                GX->ub[tmp8u&0xf] = ED->byte[0];
                break;
            case 0x21:      // INSERTPS GX, EX, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                if(MODREG) tmp8s = (tmp8u>>6)&3; else tmp8s = 0;
                GX->ud[(tmp8u>>4)&3] = EX->ud[tmp8s];
                for(int i=0; i<4; ++i)
                    if(tmp8u&(1<<i))
                        GX->ud[i] = 0;
                break;
            case 0x22:      // PINSRD GX, ED, u8
                nextop = F8;
                GETED(1);
                GETGX;
                tmp8u = F8;
                if(rex.w)
                    GX->q[tmp8u&0x1] = ED->q[0];
                else
                    GX->ud[tmp8u&0x3] = ED->dword[0];
                break;

            case 0x40:  /* DPPS Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                tmpf = 0.0f;
                for(int i=0; i<4; ++i)
                    if(tmp8u&(1<<(i+4)))
                        tmpf += GX->f[i]*EX->f[i];
                for(int i=0; i<4; ++i)
                    GX->f[i] = (tmp8u&(1<<i))?tmpf:0.0f;
                break;
            case 0x41:  /* DPPD Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                tmpd = 0.0;
                if(tmp8u&(1<<(4+0)))
                    tmpd += GX->d[0]*EX->d[0];
                if(tmp8u&(1<<(4+1)))
                    tmpd += GX->d[1]*EX->d[1];
                GX->d[0] = (tmp8u&(1<<(0)))?tmpd:0.0;
                GX->d[1] = (tmp8u&(1<<(1)))?tmpd:0.0;
                break;
            case 0x42:  /* MPSADBW Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                if(GX==EX) {
                    eax1 = *EX;
                    EX=&eax1;
                }
                tmp8u = F8;
                {
                    int src = tmp8u&3;
                    int dst = (tmp8u>>2)&1;
                    int b[11];
                    for (int i=0; i<11; ++i)
                        b[i] = GX->ub[dst*4+i];
                    for(int i=0; i<8; ++i) {
                        int tmp = abs(b[i+0]-EX->ub[src*4+0]);
                        tmp += abs(b[i+1]-EX->ub[src*4+1]);
                        tmp += abs(b[i+2]-EX->ub[src*4+2]);
                        tmp += abs(b[i+3]-EX->ub[src*4+3]);
                        GX->uw[i] = tmp;
                    }
                }
                break;

            case 0x44:  /* PCLMULQDQ Gx, Ex, Ib */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                {
                    int g = (tmp8u&1)?1:0;
                    int e = (tmp8u&0b10000)?1:0;
                    __int128 result = 0;
                    __int128 op2 = EX->q[e];
                    for (int i=0; i<64; ++i)
                        if(GX->q[g]&(1LL<<i))
                            result ^= (op2<<i);

                    GX->q[0] = result&0xffffffffffffffffLL;
                    GX->q[1] = (result>>64)&0xffffffffffffffffLL;
                }
                break;

            case 0x60:  /* PCMPESTRM */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                tmp32u = sse42_compare_string_explicit_len(emu, EX, R_EDX, GX, R_EAX, tmp8u);
                if(tmp8u&0b1000000) {
                    switch(tmp8u&1) {
                        case 0: for(int i=0; i<16; ++i) emu->xmm[0].ub[i] = ((tmp32u>>i)&1)?0xff:0x00; break;
                        case 1: for(int i=0; i<8; ++i) emu->xmm[0].uw[i] = ((tmp32u>>i)&1)?0xffff:0x0000; break;
                    }
                } else {
                    emu->xmm[0].q[1] = emu->xmm[0].q[0] = 0;
                    emu->xmm[0].uw[0] = tmp32u;
                }
                break;
            case 0x61:  /* PCMPESTRI */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                tmp32u = sse42_compare_string_explicit_len(emu, EX, R_EDX, GX, R_EAX, tmp8u);
                if(!tmp32u)
                    R_RCX = (tmp8u&1)?8:16;
                else if(tmp8u&0b1000000)
                    R_RCX = 31-__builtin_clz(tmp32u);
                else
                    R_RCX = __builtin_ffs(tmp32u) - 1;
                break;
            case 0x62:  /* PCMPISTRM */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                tmp32u = sse42_compare_string_implicit_len(emu, EX, GX, tmp8u);
                if(tmp8u&0b1000000) {
                    switch(tmp8u&1) {
                        case 0: for(int i=0; i<16; ++i) emu->xmm[0].ub[i] = ((tmp32u>>i)&1)?0xff:0x00; break;
                        case 1: for(int i=0; i<8; ++i) emu->xmm[0].uw[i] = ((tmp32u>>i)&1)?0xffff:0x0000; break;
                    }
                } else {
                    emu->xmm[0].q[1] = emu->xmm[0].q[0] = 0;
                    emu->xmm[0].uw[0] = tmp32u;
                }
                break;
            case 0x63:  /* PCMPISTRI */
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8;
                tmp32u = sse42_compare_string_implicit_len(emu, EX, GX, tmp8u);
                if(!tmp32u)
                    R_RCX = (tmp8u&1)?8:16;
                else if(tmp8u&0b1000000)
                    R_RCX = 31-__builtin_clz(tmp32u);
                else
                    R_RCX = __builtin_ffs(tmp32u) - 1;
                break;

            case 0xDF:      // AESKEYGENASSIST Gx, Ex, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp32u = F8;
                for (int i = 4; i < 8; ++i)
                    GX->ub[i] = subbytes[EX->ub[i]];
                for (int i = 12; i < 16; ++i)
                    GX->ub[i] = subbytes[EX->ub[i]];
                GX->ud[0] = GX->ud[1];
                tmp8u = GX->ub[4];
                GX->ud[1] = GX->ud[1] >> 8;
                GX->ub[7] = tmp8u;
                GX->ud[1] ^= tmp32u;
                GX->ud[2] = GX->ud[3];
                tmp8u = GX->ub[12];
                GX->ud[3] = GX->ud[3] >> 8;
                GX->ub[15] = tmp8u;
                GX->ud[3] ^= tmp32u;
                break;

            default:
                return 0;
        }
        break;

    GOCOND(0x40
        , nextop = F8;
        CHECK_FLAGS(emu);
        GETEW(0);
        GETGW;
        , if(rex.w) GW->q[0] = EW->q[0]; else GW->word[0] = EW->word[0];
        ,,
    )                               /* 0x40 -> 0x4F CMOVxx Gw,Ew */ // conditional move, no sign

    case 0x50:                      /* MOVMSKPD Gd, Ex */
        nextop = F8;
        GETEX(0);
        GETGD;
        GD->q[0] = 0;
        for(int i=0; i<2; ++i)
            GD->dword[0] |= ((EX->q[i]>>63)&1)<<i;
        break;
    case 0x51:                      /* SQRTPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<2; ++i) {
            if(EX->d[i]<0.0)            // on x86, default nan are negative
                GX->d[i] = -NAN;        // but input NAN are not touched (so sqrt(+nan) -> +nan)
            else if(isnan(EX->d[i]))
                GX->d[i] = EX->d[i];
            else
                GX->d[i] = sqrt(EX->d[i]);
        }
        break;

    case 0x54:                      /* ANDPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] &= EX->q[0];
        GX->q[1] &= EX->q[1];
        break;
    case 0x55:                      /* ANDNPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = (~GX->q[0]) & EX->q[0];
        GX->q[1] = (~GX->q[1]) & EX->q[1];
        break;
    case 0x56:                      /* ORPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] |= EX->q[0];
        GX->q[1] |= EX->q[1];
        break;
    case 0x57:                      /* XORPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] ^= EX->q[0];
        GX->q[1] ^= EX->q[1];
        break;
    case 0x58:                      /* ADDPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_VD_2(GX, EX);
        for(int i=0; i<2; ++i) {
            GX->d[i] += EX->d[i];
        }
        CHECK_NAN_VD(GX);
        break;
    case 0x59:                      /* MULPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_VD_2(GX, EX);
        for(int i=0; i<2; ++i) {
            GX->d[i] *= EX->d[i];
        }
        CHECK_NAN_VD(GX);
        break;
    case 0x5A:                      /* CVTPD2PS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->f[0] = EX->d[0];
        GX->f[1] = EX->d[1];
        GX->q[1] = 0;
        break;
    case 0x5B:                      /* CVTPS2DQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<4; ++i) {
            if(isnanf(EX->f[i]))
                tmp64s = INT32_MIN;
            else
                switch(emu->mxcsr.f.MXCSR_RC) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        tmp64s = nearbyintf(EX->f[i]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        tmp64s = floorf(EX->f[i]);
                        break;
                    case ROUND_Up:
                        tmp64s = ceilf(EX->f[i]);
                        break;
                    case ROUND_Chop:
                        tmp64s = EX->f[i];
                        break;
                }
            if (tmp64s==(int32_t)tmp64s) {
                GX->sd[i] = (int32_t)tmp64s;
            } else {
                GX->sd[i] = INT32_MIN;
            }
        }
        break;
    case 0x5C:                      /* SUBPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_VD_2(GX, EX);
        for(int i=0; i<2; ++i) {
            GX->d[i] -= EX->d[i];
        }
        CHECK_NAN_VD(GX);
        break;
    case 0x5D:                      /* MINPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || islessequal(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        if (isnan(GX->d[1]) || isnan(EX->d[1]) || islessequal(EX->d[1], GX->d[1]))
            GX->d[1] = EX->d[1];
        break;
    case 0x5E:                      /* DIVPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<2; ++i) {
            is_nan = isnan(GX->d[i]) || isnan(EX->d[i]);
            GX->d[i] /= EX->d[i];
            if(!is_nan && isnan(GX->d[i]))
                GX->d[i] = -NAN;
        }
        break;
    case 0x5F:                      /* MAXPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || isgreaterequal(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        if (isnan(GX->d[1]) || isnan(EX->d[1]) || isgreaterequal(EX->d[1], GX->d[1]))
            GX->d[1] = EX->d[1];
        break;
    case 0x60:  /* PUNPCKLBW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=7; i>=0; --i) {
            GX->ub[2 * i] = GX->ub[i];
            GX->ub[2 * i + 1] = EX->ub[i];
        }
        break;
    case 0x61:  /* PUNPCKLWD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=3; i>=0; --i) {
            GX->uw[2 * i] = GX->uw[i];
            GX->uw[2 * i + 1] = EX->uw[i];
        }
        break;
    case 0x62:  /* PUNPCKLDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->ud[3] = EX->ud[1];
        GX->ud[2] = GX->ud[1];
        GX->ud[1] = EX->ud[0];
        break;
    case 0x63:  /* PACKSSWB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(GX==EX) {
            for(int i=0; i<8; ++i)
                GX->sb[i] = (EX->sw[i]<-128)?-128:((EX->sw[i]>127)?127:EX->sw[i]);
            GX->q[1] = GX->q[0];
        } else {
            for(int i=0; i<8; ++i)
                GX->sb[i] = (GX->sw[i]<-128)?-128:((GX->sw[i]>127)?127:GX->sw[i]);
            for(int i=0; i<8; ++i)
                GX->sb[8+i] = (EX->sw[i]<-128)?-128:((EX->sw[i]>127)?127:EX->sw[i]);
        }
        break;
    case 0x64:  /* PCMPGTB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i)
            GX->ub[i] = (GX->sb[i]>EX->sb[i])?0xFF:0x00;
        break;
    case 0x65:  /* PCMPGTW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i)
            GX->uw[i] = (GX->sw[i]>EX->sw[i])?0xFFFF:0x0000;
        break;
    case 0x66:  /* PCMPGTD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<4; ++i)
            GX->ud[i] = (GX->sd[i]>EX->sd[i])?0xFFFFFFFF:0x00000000;
        break;
    case 0x67:  /* PACKUSWB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(GX==EX) {
            for(int i=0; i<8; ++i)
                GX->ub[i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
            GX->q[1] = GX->q[0];
        } else {
            for(int i=0; i<8; ++i)
                GX->ub[i] = (GX->sw[i]<0)?0:((GX->sw[i]>0xff)?0xff:GX->sw[i]);
            for(int i=0; i<8; ++i)
                GX->ub[8+i] = (EX->sw[i]<0)?0:((EX->sw[i]>0xff)?0xff:EX->sw[i]);
        }
        break;
    case 0x68:  /* PUNPCKHBW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            GX->ub[2 * i] = GX->ub[i + 8];
            GX->ub[2 * i + 1] = EX->ub[i + 8];
        }
        break;
    case 0x69:  /* PUNPCKHWD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<4; ++i) {
            GX->uw[2 * i] = GX->uw[i + 4];
            GX->uw[2 * i + 1] = EX->uw[i + 4];
        }
        break;
    case 0x6A:  /* PUNPCKHDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        // no copy needed if GX==EX
        GX->ud[0] = GX->ud[2];
        GX->ud[1] = EX->ud[2];
        GX->ud[2] = GX->ud[3];
        GX->ud[3] = EX->ud[3];
        break;
    case 0x6B:  /* PACKSSDW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(GX==EX) {
            for(int i=0; i<4; ++i)
                GX->sw[i] = (EX->sd[i]<-32768)?-32768:((EX->sd[i]>32767)?32767:EX->sd[i]);
            GX->q[1] = GX->q[0];
        } else {
            for(int i=0; i<4; ++i)
                GX->sw[i] = (GX->sd[i]<-32768)?-32768:((GX->sd[i]>32767)?32767:GX->sd[i]);
            for(int i=0; i<4; ++i)
                GX->sw[4+i] = (EX->sd[i]<-32768)?-32768:((EX->sd[i]>32767)?32767:EX->sd[i]);
        }
        break;
    case 0x6C:  /* PUNPCKLQDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[1] = EX->q[0];
        break;
    case 0x6D:  /* PUNPCKHQDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = GX->q[1];
        GX->q[1] = EX->q[1];
        break;
    case 0x6E:                      /* MOVD Gx, Ed */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            GX->q[0] = ED->q[0];
        else
            GX->q[0] = ED->dword[0];
        GX->q[1] = 0;
        break;
    case 0x6F:                      /* MOVDQA Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        GX->q[1] = EX->q[1];
        break;
    case 0x70:  /* PSHUFD Gx,Ex,Ib */
        nextop = F8;
        GETEX(1);
        GETGX;
        tmp8u = F8;
        if(EX==GX) {eax1 = *GX; EX = &eax1;}   // copy is needed
        for (int i=0; i<4; ++i)
            GX->ud[i] = EX->ud[(tmp8u>>(i*2))&3];
        break;
    case 0x71:  /* GRP */
        nextop = F8;
        GETEX(1);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLW Ex, Ib */
                tmp8u = F8;
                if(tmp8u>15)
                    {EX->q[0] = EX->q[1] = 0;}
                else
                    for (int i=0; i<8; ++i) EX->uw[i] >>= tmp8u;
                break;
            case 4:                 /* PSRAW Ex, Ib */
                tmp8u = F8;
                if(tmp8u>15) tmp8u=15;
                for (int i=0; i<8; ++i) EX->sw[i] >>= tmp8u;
                break;
            case 6:                 /* PSLLW Ex, Ib */
                tmp8u = F8;
                if(tmp8u>15)
                    {EX->q[0] = EX->q[1] = 0;}
                else
                    for (int i=0; i<8; ++i) EX->uw[i] <<= tmp8u;
                break;
            default:
                return 0;
        }
        break;
    case 0x72:  /* GRP */
        nextop = F8;
        GETEX(1);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLD Ex, Ib */
                tmp8u = F8;
                if(tmp8u>31)
                    {EX->q[0] = EX->q[1] = 0;}
                else
                    for (int i=0; i<4; ++i) EX->ud[i] >>= tmp8u;
                break;
            case 4:                 /* PSRAD Ex, Ib */
                tmp8u = F8;
                if(tmp8u>31) tmp8u=31;
                for (int i=0; i<4; ++i) EX->sd[i] >>= tmp8u;
                break;
            case 6:                 /* PSLLD Ex, Ib */
                tmp8u = F8;
                if(tmp8u>31)
                    {EX->q[0] = EX->q[1] = 0;}
                else
                    for (int i=0; i<4; ++i) EX->ud[i] <<= tmp8u;
                break;
            default:
                return 0;
        }
        break;
    case 0x73:  /* GRP */
        nextop = F8;
        GETEX(1);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLQ Ex, Ib */
                tmp8u = F8;
                if(tmp8u>63)
                    {EX->q[0] = EX->q[1] = 0;}
                else
                    {EX->q[0] >>= tmp8u; EX->q[1] >>= tmp8u;}
                break;
            case 3:                 /* PSRLDQ Ex, Ib */
                tmp8u = F8;
                if(tmp8u>15)
                    EX->u128 = 0;
                else if (tmp8u)
                    EX->u128 >>= (tmp8u<<3);
                break;
            case 6:                 /* PSLLQ Ex, Ib */
                tmp8u = F8;
                if(tmp8u>63)
                    {EX->q[0] = EX->q[1] = 0;}
                else
                    {EX->q[0] <<= tmp8u; EX->q[1] <<= tmp8u;}
                break;
            case 7:                 /* PSLLDQ Ex, Ib */
                tmp8u = F8;
                if(tmp8u>15)
                    EX->u128 = 0;
                else if (tmp8u)
                    EX->u128 <<= (tmp8u<<3);
                break;
            default:
                return 0;
        }
        break;
    case 0x74:  /* PCMPEQB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<16; ++i)
            GX->ub[i] = (GX->ub[i]==EX->ub[i])?0xff:0;
        break;
    case 0x75:  /* PCMPEQW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<8; ++i)
            GX->uw[i] = (GX->uw[i]==EX->uw[i])?0xffff:0;
        break;
    case 0x76:  /* PCMPEQD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<4; ++i)
            GX->ud[i] = (GX->ud[i]==EX->ud[i])?0xffffffff:0;
        break;
    
    case 0x78:  /* EXTRQ Ex, ib, ib */
        // AMD only
        nextop = F8;
        if(!BOX64ENV(cputype) || (nextop&0xC0)>>3) {
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
        } else {
            //TODO: test /0
            GETEX(2);
            tmp8s = F8&0x3f;
            tmp8u = F8&0x3f;
            EX->q[0]>>=tmp8u;
            EX->q[0]&=((1<<(tmp8s+1))-1);
        }
        break;
    case 0x79:  /* EXTRQ Ex, Gx */
        // AMD only
        nextop = F8;
        if(!BOX64ENV(cputype) || !(MODREG)) {
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
        } else {
            //TODO: test/r
            GETGX;
            GETEX(2);
            tmp8s = EX->ub[0]&0x3f;
            tmp8u = EX->ub[1]&0x3f;
            GX->q[0]>>=tmp8u;
            GX->q[0]&=((1<<(tmp8s+1))-1);
        }
        break;

    case 0x7C:  /* HADDPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        is_nan = isnan(GX->d[0]) || isnan(GX->d[1]);
        GX->d[0] += GX->d[1];
        if(!is_nan && isnan(GX->d[0]))
            GX->d[0] = -NAN;
        if(EX==GX) {
            GX->d[1] = GX->d[0];
        } else {
            is_nan = isnan(EX->d[0]) || isnan(EX->d[1]);
            GX->d[1] = EX->d[0] + EX->d[1];
            if(!is_nan && isnan(GX->d[1]))
                GX->d[1] = -NAN;
        }
        break;
    case 0x7D:  /* HSUBPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        is_nan = isnan(GX->d[0]) || isnan(GX->d[1]);
        GX->d[0] -= GX->d[1];
        if(!is_nan && isnan(GX->d[0]))
            GX->d[0] = -NAN;
        if(EX==GX) {
            GX->d[1] = GX->d[0];
        } else {
            is_nan = isnan(EX->d[0]) || isnan(EX->d[1]);
            GX->d[1] = EX->d[0] - EX->d[1];
            if(!is_nan && isnan(GX->d[1]))
                GX->d[1] = -NAN;
        }
        break;
    case 0x7E:                      /* MOVD Ed, Gx */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            ED->q[0] = GX->q[0];
        else {
            if(MODREG)
                ED->q[0] = GX->ud[0];
            else
                ED->dword[0] = GX->ud[0];
        }
        break;
    case 0x7F:  /* MOVDQA Ex,Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->q[0] = GX->q[0];
        EX->q[1] = GX->q[1];
        break;

    case 0xA3:                      /* BT Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        tmp32s = rex.w?GW->sdword[0]:GW->sword[0];
        tmp8u=tmp32s&(rex.w?63:15);
        tmp32s >>= (rex.w?6:4);
        if(!MODREG)
        {
            #ifdef TEST_INTERPRETER
            test->memaddr=((test->memaddr)+(tmp32s<<(rex.w?3:1)));
            if(rex.w)
                *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
            else
                *(uint16_t*)test->mem = *(uint16_t*)test->memaddr;
            #else
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
            #endif
        }
        if(rex.w) {
            if(EW->q[0] & (1LL<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
        } else {
            if(EW->word[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
        }
        break;
    case 0xA4:                      /* SHLD Ew,Gw,Ib */
    case 0xA5:                      /* SHLD Ew,Gw,CL */
        nextop = F8;
        GETEW((nextop==0xA4)?1:0);
        GETGW;
        if(opcode==0xA4)
            tmp8u = F8;
        else
            tmp8u = R_CL;
        if(rex.w)
            EW->q[0] = shld64(emu, EW->q[0], GW->q[0], tmp8u);
        else
            EW->word[0] = shld16(emu, EW->word[0], GW->word[0], tmp8u);
        break;

    case 0xAB:                      /* BTS Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        tmp32s = rex.w?GW->sdword[0]:GW->sword[0];
        tmp8u=tmp32s&(rex.w?63:15);
        tmp32s >>= (rex.w?6:4);
        if(!MODREG)
        {
            #ifdef TEST_INTERPRETER
            test->memaddr=((test->memaddr)+(tmp32s<<(rex.w?3:1)));
            if(rex.w)
                *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
            else
                *(uint16_t*)test->mem = *(uint16_t*)test->memaddr;
            #else
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
            #endif
        }
        if(rex.w) {
            if(EW->q[0] & (1LL<<tmp8u))
                SET_FLAG(F_CF);
            else {
                EW->q[0] |= (1LL<<tmp8u);
                CLEAR_FLAG(F_CF);
            }
        } else {
            if(EW->word[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else {
                EW->word[0] |= (1<<tmp8u);
                CLEAR_FLAG(F_CF);
            }
        }
        break;
    case 0xAC:                      /* SHRD Ew,Gw,Ib */
    case 0xAD:                      /* SHRD Ew,Gw,CL */
        nextop = F8;
        GETEW((nextop==0xAC)?1:0);
        GETGW;
        if(opcode==0xAC)
            tmp8u = F8;
        else
            tmp8u = R_CL;
        if(rex.w)
            EW->q[0] = shrd64(emu, EW->q[0], GW->q[0], tmp8u);
        else
            EW->word[0] = shrd16(emu, EW->word[0], GW->word[0], tmp8u);
        break;
    case 0xAE:                      /* Grp Ed (SSE) */
        nextop = F8;
        if(MODREG)
            switch(nextop) {
                default:
                    return 0;
            }
        else
        switch((nextop>>3)&7) {
            case 6:                 /* CLWB Ed */
                // same code and CLFLUSH, is it ok?
                _GETED(0);
                #if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                if(BOX64ENV(dynarec))
                    cleanDBFromAddressRange((uintptr_t)ED, 8, 0);
                #endif
                break;
            case 7:                 /* CLFLUSHOPT Ed */
                // same code and CLFLUSH, is it ok?
                _GETED(0);
                #if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                if(BOX64ENV(dynarec))
                    cleanDBFromAddressRange((uintptr_t)ED, 8, 0);
                #endif
                break;
            default:
                return 0;
        }
        break;
    case 0xAF:                      /* IMUL Gw,Ew */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            GW->q[0] = imul64(emu, GW->q[0], EW->q[0]);
        else
            GW->word[0] = imul16(emu, GW->word[0], EW->word[0]);
        break;

    case 0xB1:                      /* CMPXCHG Ew,Gw */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w) {
            cmp64(emu, R_RAX, EW->q[0]);
            if(ACCESS_FLAG(F_ZF)) {
                EW->q[0] = GW->q[0];
            } else {
                R_RAX = EW->q[0];
            }
        } else {
            cmp16(emu, R_AX, EW->word[0]);
            if(ACCESS_FLAG(F_ZF)) {
                EW->word[0] = GW->word[0];
            } else {
                R_AX = EW->word[0];
            }
        }
        break;

    case 0xB3:                      /* BTR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        tmp32s = rex.w?GW->sdword[0]:GW->sword[0];
        tmp8u=tmp32s&(rex.w?63:15);
        tmp32s >>= (rex.w?6:4);
        if(!MODREG)
        {
            #ifdef TEST_INTERPRETER
            test->memaddr=((test->memaddr)+(tmp32s<<(rex.w?3:1)));
            if(rex.w)
                *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
            else
                *(uint16_t*)test->mem = *(uint16_t*)test->memaddr;
            #else
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
            #endif
        }
        if(rex.w) {
            if(EW->q[0] & (1LL<<tmp8u)) {
                SET_FLAG(F_CF);
                EW->q[0] ^= (1LL<<tmp8u);
            } else
                CLEAR_FLAG(F_CF);
        } else {
            if(EW->word[0] & (1<<tmp8u)) {
                SET_FLAG(F_CF);
                EW->word[0] ^= (1<<tmp8u);
            } else
                CLEAR_FLAG(F_CF);
        }
        break;

    case 0xB6:                      /* MOVZX Gw,Eb */
        nextop = F8;
        GETEB(0);
        GETGW;
        if(rex.w)
            GW->q[0] = EB->byte[0];
        else
            GW->word[0] = EB->byte[0];
        break;
    case 0xB7:                      /* MOVZX Gw,Ew */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            GW->q[0] = EW->word[0];
        else
            GW->word[0] = EW->word[0];
        break;

    case 0xBA:
        nextop = F8;
        switch((nextop>>3)&7) {
            case 4:                 /* BT Ew,Ib */
                CHECK_FLAGS(emu);
                GETEW(1);
                GETGW;
                tmp8u = F8;
                if(rex.w) {
                    tmp8u&=63;
                    if(ED->q[0] & (1LL<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                } else {
                    tmp8u&=15;
                    if(EW->word[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                }
                break;
            case 5:             /* BTS Ew, Ib */
                CHECK_FLAGS(emu);
                GETEW(1);
                GETGW;
                tmp8u = F8;
                if(rex.w) {
                    tmp8u&=63;
                    if(ED->q[0] & (1LL<<tmp8u)) {
                        SET_FLAG(F_CF);
                    } else {
                        ED->q[0] ^= (1LL<<tmp8u);
                        CLEAR_FLAG(F_CF);
                    }
                } else {
                    tmp8u&=15;
                    if(EW->word[0] & (1<<tmp8u)) {
                        SET_FLAG(F_CF);
                    } else {
                        EW->word[0] ^= (1<<tmp8u);
                        CLEAR_FLAG(F_CF);
                    }
                }
                break;
            case 6:             /* BTR Ew, Ib */
                CHECK_FLAGS(emu);
                GETEW(1);
                GETGW;
                tmp8u = F8;
                if(rex.w) {
                    tmp8u&=63;
                    if(ED->q[0] & (1LL<<tmp8u)) {
                        SET_FLAG(F_CF);
                        ED->q[0] ^= (1LL<<tmp8u);
                    } else
                        CLEAR_FLAG(F_CF);
                } else {
                    tmp8u&=15;
                    if(EW->word[0] & (1<<tmp8u)) {
                        SET_FLAG(F_CF);
                        EW->word[0] ^= (1<<tmp8u);
                    } else
                        CLEAR_FLAG(F_CF);
                }
                break;
            case 7:             /* BTC Ew, Ib */
                CHECK_FLAGS(emu);
                GETEW(1);
                GETGW;
                tmp8u = F8;
                if(rex.w) {
                    tmp8u&=63;
                    if(ED->q[0] & (1LL<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    ED->q[0] ^= (1LL<<tmp8u);
                } else {
                    tmp8u&=15;
                    if(EW->word[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    EW->word[0] ^= (1<<tmp8u);
                }
                break;
            default:
                return 0;
        }
        break;
    case 0xBB:                      /* BTC Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        tmp32s = rex.w?GW->sdword[0]:GW->sword[0];
        tmp8u=tmp32s&(rex.w?63:15);
        tmp32s >>= (rex.w?6:4);
        if(!MODREG)
        {
            #ifdef TEST_INTERPRETER
            test->memaddr=((test->memaddr)+(tmp32s<<(rex.w?3:1)));
            if(rex.w)
                *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
            else
                *(uint16_t*)test->mem = *(uint16_t*)test->memaddr;
            #else
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
            #endif
        }
        if(rex.w) {
            if(EW->q[0] & (1LL<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            EW->q[0] ^= (1LL<<tmp8u);
        } else {
            if(EW->word[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            EW->word[0] ^= (1<<tmp8u);
        }
        break;
    case 0xBC:                      /* BSF Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        tmp8u = 0;
        if(rex.w) {
            tmp64u = EW->q[0];
            if(tmp64u) {
                CLEAR_FLAG(F_ZF);
                while(!(tmp64u&(1LL<<tmp8u))) ++tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            if(tmp64u || !MODREG)
                GW->q[0] = tmp8u;
        } else {
            tmp16u = EW->word[0];
            if(tmp16u) {
                CLEAR_FLAG(F_ZF);
                while(!(tmp16u&(1<<tmp8u))) ++tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            if(tmp16u || !MODREG)
                GW->word[0] = tmp8u;
        }
        if(!BOX64ENV(cputype)) {
            CONDITIONAL_SET_FLAG(PARITY(tmp8u), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CLEAR_FLAG(F_SF);
            CLEAR_FLAG(F_OF);
        }
        break;
    case 0xBD:                      /* BSR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        tmp8u = 0;
        if(rex.w) {
            tmp64u = EW->q[0];
            if(tmp64u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 63;
                while(!(tmp64u&(1LL<<tmp8u))) --tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            if(tmp64u || !MODREG)
                GW->q[0] = tmp8u;
        } else {
            tmp16u = EW->word[0];
            if(tmp16u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 15;
                while(!(tmp16u&(1<<tmp8u))) --tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            if(tmp16u || !MODREG)
                GW->word[0] = tmp8u;
        }
        if(!BOX64ENV(cputype)) {
            CONDITIONAL_SET_FLAG(PARITY(tmp8u), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CLEAR_FLAG(F_SF);
            CLEAR_FLAG(F_OF);
        }
        break;
    case 0xBE:                      /* MOVSX Gw,Eb */
        nextop = F8;
        GETEB(0);
        GETGW;
        if(rex.w)
            GW->sq[0] = EB->sbyte[0];
        else
            GW->sword[0] = EB->sbyte[0];
        break;
    case 0xBF:                      /* MOVSX Gw,Ew */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            GW->sq[0] = EW->sword[0];
        else
            GW->sword[0] = EW->sword[0];
        break;

    case 0xC1:                      /* XADD Gw,Ew */ // Xchange and Add
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w) {
            tmp64u = add64(emu, EW->q[0], GW->q[0]);
            GW->q[0] = EW->q[0];
            EW->q[0] = tmp64u;
        } else {
            tmp16u = add16(emu, EW->word[0], GW->word[0]);
            GW->word[0] = EW->word[0];
            EW->word[0] = tmp16u;
        }
        break;
    case 0xC2:                      /* CMPPD Gx, Ex, Ib */
        nextop = F8;
        GETEX(1);
        GETGX;
        tmp8u = F8;
        for(int i=0; i<2; ++i) {
            tmp8s = 0;
            switch(tmp8u&7) {
                case 0: tmp8s=(GX->d[i] == EX->d[i]); break;
                case 1: tmp8s=isless(GX->d[i], EX->d[i]); break;
                case 2: tmp8s=islessequal(GX->d[i], EX->d[i]); break;
                case 3: tmp8s=isnan(GX->d[i]) || isnan(EX->d[i]); break;
                case 4: tmp8s=isnan(GX->d[i]) || isnan(EX->d[i]) || (GX->d[i] != EX->d[i]); break;
                case 5: tmp8s=isnan(GX->d[i]) || isnan(EX->d[i]) || isgreaterequal(GX->d[i], EX->d[i]); break;
                case 6: tmp8s=isnan(GX->d[i]) || isnan(EX->d[i]) || isgreater(GX->d[i], EX->d[i]); break;
                case 7: tmp8s=!isnan(GX->d[i]) && !isnan(EX->d[i]); break;
            }
            GX->q[i]=(tmp8s)?0xffffffffffffffffLL:0LL;
        }
        break;

    case 0xC4:  /* PINSRW Gx,Ew,Ib */
        nextop = F8;
        GETED(1);
        GETGX;
        tmp8u = F8;
        GX->uw[tmp8u&7] = ED->word[0];   // only low 16bits
        break;
    case 0xC5:  /* PEXTRW Gw,Ex,Ib */
        nextop = F8;
        GETEX(1);
        GETGD;
        tmp8u = F8;
        GD->q[0] = EX->uw[tmp8u&7];  // 16bits extract, 0 extended
        break;
    case 0xC6:  /* SHUFPD Gx, Ex, Ib */
        nextop = F8;
        GETEX(1);
        GETGX;
        tmp8u = F8;
        eax1.q[0] = GX->q[tmp8u&1];
        eax1.q[1] = EX->q[(tmp8u>>1)&1];
        GX->q[0] = eax1.q[0];
        GX->q[1] = eax1.q[1];
        break;

    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:                  /* BSWAP reg16 */
        tmp8u = (opcode&7)+(rex.b<<3);
        if(rex.w) {
            emu->regs[tmp8u].q[0] = __builtin_bswap64(emu->regs[tmp8u].q[0]);
        } else {
            emu->regs[tmp8u].word[0] = __builtin_bswap16(emu->regs[tmp8u].word[0]);
        }
        break;
    case 0xD0:  /* ADDSUBPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_VD_2(GX, EX);
        GX->d[0] -= EX->d[0];
        GX->d[1] += EX->d[1];
        CHECK_NAN_VD(GX);
        break;
    case 0xD1:  /* PSRLW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->q[0]>15)
            {GX->q[0] = GX->q[1] = 0;}
        else
            {tmp8u=EX->ub[0]; for (int i=0; i<8; ++i) GX->uw[i] >>= tmp8u;}
        break;
    case 0xD2:  /* PSRLD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->q[0]>31)
            {GX->q[0] = GX->q[1] = 0;}
        else
            {tmp8u=EX->ub[0]; for (int i=0; i<4; ++i) GX->ud[i] >>= tmp8u;}
        break;
    case 0xD3:  /* PSRLQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->q[0]>63)
            {GX->q[0] = GX->q[1] = 0;}
        else
            {tmp8u=EX->ub[0]; for (int i=0; i<2; ++i) GX->q[i] >>= tmp8u;}
        break;
    case 0xD4:  /* PADDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->sq[0] += EX->sq[0];
        GX->sq[1] += EX->sq[1];
        break;
    case 0xD5:  /* PMULLW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32s = (int32_t)GX->sw[i] * EX->sw[i];
            GX->sw[i] = tmp32s&0xffff;
        }
        break;
    case 0xD6:                      /* MOVQ Ex,Gx */
        nextop = F8;
        GETEX8(0);
        GETGX;
        EX->q[0] = GX->q[0];
        if(MODREG)
            EX->q[1] = 0;
        break;
    case 0xD7:  /* PMOVMSKB Gd,Ex */
        nextop = F8;
        if(MODREG) {
            GETEX(0);
            GETGD;
            GD->q[0] = 0;
            for (int i=0; i<16; ++i)
                if(EX->ub[i]&0x80)
                    GD->dword[0] |= (1<<i);
        } else
            return 0;
        break;
    case 0xD8:  /* PSUBUSB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i) {
            tmp16s = (int16_t)GX->ub[i] - EX->ub[i];
            GX->ub[i] = (tmp16s<0)?0:tmp16s;
        }
        break;
    case 0xD9:  /* PSUBUSW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32s = (int32_t)GX->uw[i] - EX->uw[i];
            GX->uw[i] = (tmp32s<0)?0:tmp32s;
        }
        break;
    case 0xDA:  /* PMINUB Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<16; ++i)
            if(EX->ub[i]<GX->ub[i]) GX->ub[i] = EX->ub[i];
        break;
    case 0xDB:  /* PAND Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] &= EX->q[0];
        GX->q[1] &= EX->q[1];
        break;
    case 0xDC:  /* PADDUSB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i) {
            tmp16s = (int16_t)GX->ub[i] + EX->ub[i];
            GX->ub[i] = (tmp16s>255)?255:tmp16s;
        }
        break;
    case 0xDD:  /* PADDUSW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32s = (int32_t)GX->uw[i] + EX->uw[i];
            GX->uw[i] = (tmp32s>65535)?65535:tmp32s;
        }
        break;
    case 0xDE:  /* PMAXUB Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<16; ++i)
            if(EX->ub[i]>GX->ub[i]) GX->ub[i] = EX->ub[i];
        break;
    case 0xDF:  /* PANDN Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = (~(GX->q[0])) & EX->q[0];
        GX->q[1] = (~(GX->q[1])) & EX->q[1];
        break;
    case 0xE0:  /* PAVGB Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<16; ++i)
            GX->ub[i] = ((uint16_t)GX->ub[i] + EX->ub[i] + 1)>>1;
        break;
    case 0xE1:  /* PSRAW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        tmp8u=(EX->q[0]>15)?15:EX->ub[0];
        for (int i=0; i<8; ++i)
            GX->sw[i] >>= tmp8u;
        break;
    case 0xE2:  /* PSRAD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        tmp8u=(EX->q[0]>31)?31:EX->ub[0];
        for (int i=0; i<4; ++i)
            GX->sd[i] >>= tmp8u;
        break;
    case 0xE3:  /* PAVGW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<8; ++i)
            GX->uw[i] = ((uint32_t)GX->uw[i] + EX->uw[i] + 1)>>1;
        break;
    case 0xE4:  /* PMULHUW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32u = (uint32_t)GX->uw[i] * EX->uw[i];
            GX->uw[i] = (tmp32u>>16)&0xffff;
        }
        break;
    case 0xE5:  /* PMULHW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32s = (int32_t)GX->sw[i] * EX->sw[i];
            GX->uw[i] = (tmp32s>>16)&0xffff;
        }
        break;
    case 0xE6:  /* CVTTPD2DQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>(double)0x7fffffff)
            GX->ud[0] = 0x80000000;
        else
            GX->sd[0] = EX->d[0];
        if(isnan(EX->d[1]) || isinf(EX->d[1]) || EX->d[1]>(double)0x7fffffff)
            GX->ud[1] = 0x80000000;
        else
            GX->sd[1] = EX->d[1];
        GX->q[1] = 0;
        break;
    case 0xE7:   /* MOVNTDQ Ex, Gx */
        nextop = F8;
        if(!MODREG) {
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
        }
        break;
    case 0xE8:  /* PSUBSB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i) {
            tmp16s = (int16_t)GX->sb[i] - EX->sb[i];
            GX->sb[i] = (tmp16s<-128)?-128:((tmp16s>127)?127:tmp16s);
        }
        break;
    case 0xE9:  /* PSUBSW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32s = (int32_t)GX->sw[i] - EX->sw[i];
            GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
        }
        break;
    case 0xEA:  /* PMINSW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<8; ++i)
            GX->sw[i] = (GX->sw[i]<EX->sw[i])?GX->sw[i]:EX->sw[i];
        break;
    case 0xEB:  /* POR Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] |= EX->q[0];
        GX->q[1] |= EX->q[1];
        break;
    case 0xEC:  /* PADDSB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i) {
            tmp16s = (int16_t)GX->sb[i] + EX->sb[i];
            GX->sb[i] = (tmp16s>127)?127:((tmp16s<-128)?-128:tmp16s);
        }
        break;
    case 0xED:  /* PADDSW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i) {
            tmp32s = (int32_t)GX->sw[i] + EX->sw[i];
            GX->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
        }
        break;
    case 0xEE:  /* PMAXSW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i)
            GX->sw[i] = (GX->sw[i]>EX->sw[i])?GX->sw[i]:EX->sw[i];
        break;
    case 0xEF:                      /* PXOR Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] ^= EX->q[0];
        GX->q[1] ^= EX->q[1];
        break;

    case 0xF1:  /* PSLLW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->q[0]>15)
            {GX->q[0] = GX->q[1] = 0;}
        else
            {tmp8u=EX->ub[0]; for (int i=0; i<8; ++i) GX->uw[i] <<= tmp8u;}
        break;
    case 0xF2:  /* PSLLD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->q[0]>31)
            {GX->q[0] = GX->q[1] = 0;}
        else
            {tmp8u=EX->ub[0]; for (int i=0; i<4; ++i) GX->ud[i] <<= tmp8u;}
        break;
    case 0xF3:  /* PSLLQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->q[0]>63)
            {GX->q[0] = GX->q[1] = 0;}
        else
            {tmp8u=EX->ub[0]; for (int i=0; i<2; ++i) GX->q[i] <<= tmp8u;}
        break;
    case 0xF4:  /* PMULUDQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[1] = (uint64_t)EX->ud[2]*GX->ud[2];
        GX->q[0] = (uint64_t)EX->ud[0]*GX->ud[0];
        break;
    case 0xF5:  /* PMADDWD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for (int i=0; i<4; ++i)
            GX->sd[i] = (int32_t)(GX->sw[i*2+0])*EX->sw[i*2+0] + (int32_t)(GX->sw[i*2+1])*EX->sw[i*2+1];
        break;
    case 0xF6:  /* PSADBW Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        tmp32u = 0;
        for (int i=0; i<8; ++i)
            tmp32u += (GX->ub[i]>EX->ub[i])?(GX->ub[i] - EX->ub[i]):(EX->ub[i] - GX->ub[i]);
        GX->q[0] = tmp32u;
        tmp32u = 0;
        for (int i=8; i<16; ++i)
            tmp32u += (GX->ub[i]>EX->ub[i])?(GX->ub[i] - EX->ub[i]):(EX->ub[i] - GX->ub[i]);
        GX->q[1] = tmp32u;
        break;
    case 0xF7:  /* MASKMOVDQU Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        opex2 = (sse_regs_t *)(R_RDI);
        for (int i=0; i<16; ++i) {
            if(EX->ub[i]&0x80)
                opex2->ub[i] = GX->ub[i];
        }
        break;
    case 0xF8:  /* PSUBB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i)
            GX->sb[i] -= EX->sb[i];
        break;
    case 0xF9:  /* PSUBW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i)
            GX->sw[i] -= EX->sw[i];
        break;
    case 0xFA:  /* PSUBD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->sd[0] -= EX->sd[0];
        GX->sd[1] -= EX->sd[1];
        GX->sd[2] -= EX->sd[2];
        GX->sd[3] -= EX->sd[3];
        break;
    case 0xFB:  /* PSUBQ Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->sq[0] -= EX->sq[0];
        GX->sq[1] -= EX->sq[1];
        break;
    case 0xFC:  /* PADDB Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<16; ++i)
            GX->sb[i] += EX->sb[i];
        break;
    case 0xFD:  /* PADDW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<8; ++i)
            GX->sw[i] += EX->sw[i];
        break;
    case 0xFE:  /* PADDD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->sd[0] += EX->sd[0];
        GX->sd[1] += EX->sd[1];
        GX->sd[2] += EX->sd[2];
        GX->sd[3] += EX->sd[3];
        break;

    default:
        return 0;
    }
    return addr;
}
