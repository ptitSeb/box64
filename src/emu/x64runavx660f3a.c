#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#include "x64shaext.h"
#include "x64compstrings.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

static __int128_t pclmul_helper(uint64_t X, uint64_t Y)
{
    __int128 result = 0;
    __int128 op2 = Y;
    for (int i=0; i<64; ++i)
        if(X&(1LL<<i))
            result ^= (op2<<i);

    return result;
}

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_660F3A(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_660F3A(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u, u8;
    int8_t tmp8s;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s;
    reg64_t *oped, *opgd;
    float tmpf;
    double tmpd;
    sse_regs_t *opex, *opgx, *opvx, eax1,eax2;
    sse_regs_t *opey, *opgy, *opvy, eay1,eay2;
    // AES opcodes constants
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


#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x00:  /* VPERMQ Gx, Ex, Imm8 */
        case 0x01:  /* VPERMPD Gx, Ex, Imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETGY;
            GETEY;
            u8 = F8;
            if(!vex.l)  emit_signal(emu, SIGILL, (void*)R_RIP, 0);
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
                eay1 = *EY;
                EY = &eay1;
            }
            for(int i=0; i<2; ++i)
                GX->q[i] = (((u8>>(i*2))&3)>1)?EY->q[(u8>>(i*2))&1]:EX->q[(u8>>(i*2))&1];
            for(int i=2; i<4; ++i)
                GY->q[i-2] = (((u8>>(i*2))&3)>1)?EY->q[(u8>>(i*2))&1]:EX->q[(u8>>(i*2))&1];
            break;
        case 0x02:      /* VPBLENDD Gx, Vx, Ex, u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (tmp8u&(1<<i))?EX->ud[i]:VX->ud[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (tmp8u&(1<<(i+4)))?EY->ud[i]:VY->ud[i];
            } else
                GY->u128 = 0;
            break;

        case 0x04:  /* VPERMILPS Gx, Ex, Imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETGY;
            GETEY;
            u8 = F8;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            for(int i=0; i<4; ++i)
                GX->ud[i] = EX->ud[(u8>>(i*2))&3];
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                for(int i=0; i<4; ++i)
                    GY->ud[i] = EY->ud[(u8>>(i*2))&3];
            } else
                GY->u128 = 0;
            break;
        case 0x05:  /* VPERMILD Gx, Ex, Imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETGY;
            GETEY;
            u8 = F8;
            if(GX==EX) {
                eax1 = *EX;
                EX = &eax1;
            }
            for(int i=0; i<2; ++i)
                GX->q[i] = EX->q[(u8>>i)&1];
            if(vex.l) {
                if(GY==EY) {
                    eay1 = *EY;
                    EY = &eay1;
                }
                for(int i=0; i<2; ++i)
                    GY->q[i] = EY->q[(u8>>(i+2))&1];
            } else
                GY->u128 = 0;
            break;
        case 0x06:  /* VPERM2F128 Gx, Vx, Ex, Imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETEY;
            GETGY;
            GETVY;
            u8 = F8;
            if(!vex.l) emit_signal(emu, SIGILL, (void*)R_RIP, 0);
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
            switch(u8&0x0f) {
                case 0 : GX->u128 = VX->u128; break;
                case 1 : GX->u128 = VY->u128; break;
                case 2 : GX->u128 = EX->u128; break;
                case 3 : GX->u128 = EY->u128; break;
                default: GX->u128 = 0; break;
            }
            switch((u8>>4)&0x0f) {
                case 0 : GY->u128 = VX->u128; break;
                case 1 : GY->u128 = VY->u128; break;
                case 2 : GY->u128 = EX->u128; break;
                case 3 : GY->u128 = EY->u128; break;
                default: GY->u128 = 0; break;
            }
            break;

        case 0x08:          // VROUNDPS Gx, Ex, u8
            nextop = F8;
            GETEX(1);
            GETGX; GETGY;
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
            if(vex.l) {
                GETEY;
                switch(tmp8u) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        for(int i=0; i<4; ++i)
                            GY->f[i] = nearbyintf(EY->f[i]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        for(int i=0; i<4; ++i)
                            GY->f[i] = floorf(EY->f[i]);
                        break;
                    case ROUND_Up:
                        for(int i=0; i<4; ++i)
                            GY->f[i] = ceilf(EY->f[i]);
                        break;
                    case ROUND_Chop:
                        for(int i=0; i<4; ++i)
                            GY->f[i] = truncf(EY->f[i]);
                        break;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x09:          // VROUNDPD Gx, Ex, u8
            nextop = F8;
            GETEX(1);
            GETGX; GETGY;
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
            if(vex.l) {
                GETEY;
                switch(tmp8u) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GY->d[0] = nearbyint(EY->d[0]);
                        GY->d[1] = nearbyint(EY->d[1]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GY->d[0] = floor(EY->d[0]);
                        GY->d[1] = floor(EY->d[1]);
                        break;
                    case ROUND_Up:
                        GY->d[0] = ceil(EY->d[0]);
                        GY->d[1] = ceil(EY->d[1]);
                        break;
                    case ROUND_Chop:
                        GY->d[0] = trunc(EY->d[0]);
                        GY->d[1] = trunc(EY->d[1]);
                        break;
                }
            } else
                GY->u128 = 0;
            break;
        case 0x0A:          // VROUNDSS Gx, Vx, Ex, u8
            nextop = F8;
            GETEX(1);
            GETGX; GETVX; GETGY;
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
            if(GX!=VX) {
                GX->ud[1] = VX->ud[1];
                GX->q[1] = VX->q[1];
            }
            GY->u128 = 0;
            break;
        case 0x0B:          // VROUNDSD Gx, Vx, Ex, u8
            nextop = F8;
            GETEX(1);
            GETGX; GETVX; GETGY;
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
            GX->q[1] = VX->q[1];
            GY->u128 = 0;
            break;
        case 0x0C:      /* VBLENDPS Gx, Vx, Ex, u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (tmp8u&(1<<i))?EX->ud[i]:VX->ud[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (tmp8u&(1<<(i+4)))?EY->ud[i]:VY->ud[i];
            } else
                GY->u128 = 0;
            break;
        case 0x0D:      /* VBLENDPD Gx, Vx, Ex, u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<2; ++i)
                GX->q[i] = (tmp8u&(1<<i))?EX->q[i]:VX->q[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    GY->q[i] = (tmp8u&(1<<(i+2)))?EY->q[i]:VY->q[i];
            } else
                GY->u128 = 0;
            break;
        case 0x0E:      /* VPBLENDW Gx, Vx, Ex, u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            for(int i=0; i<8; ++i)
                GX->uw[i] = (tmp8u&(1<<i))?EX->uw[i]:VX->uw[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<8; ++i)
                    GY->uw[i] = (tmp8u&(1<<i))?EY->uw[i]:VY->uw[i];
            } else
                GY->u128 = 0;
            break;
        case 0x0F:      /* VPALIGNR GX, VX, EX, u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            u8 = tmp8u = F8;
            if(tmp8u>31)
                {GX->q[0] = GX->q[1] = 0;}
            else
            {
                for (int i=0; i<16; ++i, ++tmp8u)
                    eax1.ub[i] = (tmp8u>15)?((tmp8u>31)?0:VX->ub[tmp8u-16]):EX->ub[tmp8u];
                GX->q[0] = eax1.q[0];
                GX->q[1] = eax1.q[1];
            }
            if(vex.l) {
                GETEY;
                GETVY;
                if(u8>31)
                    {GY->u128 = 0;}
                else
                {
                    for (int i=0; i<16; ++i, ++u8)
                        eax1.ub[i] = (u8>15)?((u8>31)?0:VY->ub[u8-16]):EY->ub[u8];
                    GY->q[0] = eax1.q[0];
                    GY->q[1] = eax1.q[1];
                }
            } else
                GY->u128 = 0;
            break;

        case 0x14:      // VPEXTRB ED, GX, u8
            nextop = F8;
            GETED(1);
            GETGX;
            tmp8u = F8;
            if(MODREG)
                ED->q[0] = GX->ub[tmp8u&0x0f];
            else
                ED->byte[0] = GX->ub[tmp8u&0x0f];
            break;
        case 0x15:      // VPEXTRW Ew,Gx,Ib
            nextop = F8;
            GETED(1);
            GETGX;
            tmp8u = F8;
            if(MODREG)
                ED->q[0] = GX->uw[tmp8u&7];  // 16bits extract, 0 extended
            else
                ED->word[0] = GX->uw[tmp8u&7];
            break;
        case 0x16:      // VPEXTRD/Q ED, GX, u8
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
        case 0x17:      // VEXTRACTPS ED, GX, u8
            nextop = F8;
            GETED(1);
            GETGX;
            tmp8u = F8;
            ED->dword[0] = GX->ud[tmp8u&3];
            if(MODREG) ED->dword[1] = 0;
            break;
        case 0x18:  /* VINSERTF128 Gx, Vx, Ex, imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            tmp8u = F8;
            if(tmp8u&1) {
                GY->u128 = EX->u128;
                if(GX!=VX)
                    GX->u128 = VX->u128;
            } else {
                GX->u128 = EX->u128;
                if(GY!=VY)
                    GY->u128 = VY->u128;
            }
            break;
        case 0x19:  /* VEXTRACT128 Ex, Gx, imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETGY;
            tmp8u = F8;
            EX->u128 = (tmp8u&1)?GY->u128:GX->u128;
            if(MODREG) {
                GETEY;
                EY->u128 = 0;
            }
            break;

        case 0x1D:  /* VCVTPS2PH Ex, Gx, u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            u8 = F8;
            if(u8&4)
                u8 = emu->mxcsr.f.MXCSR_RC;
            else
                u8 = u8&3;
            for(int i=0; i<4; ++i)
                EX->uw[i] = cvtf32_16(GX->ud[i], u8);
            if(vex.l) {
                GETGY;
                for(int i=0; i<4; ++i)
                    EX->uw[4+i] = cvtf32_16(GY->ud[i], u8);
            }
            if(MODREG) {
                if(!vex.l) EX->q[1] = 0;
                GETEY;
                EY->u128 = 0;
            }
            break;


        case 0x20:      // VPINSRB GX, Vx, ED, u8
            nextop = F8;
            GETED(1);   // It's ED, and not EB
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            if(GX!=VX)
                GX->u128 = VX->u128;
            GX->ub[tmp8u&0xf] = ED->byte[0];
            GY->u128 = 0;
            break;
        case 0x21:  /* VINSERTPS Gx, Vx, Ex, imm8 */
            nextop = F8;
            GETGX;
            GETEX(1);
            GETVX;
            GETGY;
            tmp8u = F8;
            if(MODREG) {
                tmp32u = EX->ud[(tmp8u>>6)&3];
            } else
                tmp32u = EX->ud[0];
            for(int i=0; i<4; ++i)
                GX->ud[i] = (tmp8u&(1<<i))?0:((i==((tmp8u>>4)&3))?tmp32u:VX->ud[i]);
            GY->u128 = 0;
            break;
        case 0x22:      // VPINSRD Gx, Vx, ED, u8
            nextop = F8;
            GETED(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            if(GX!=VX)
                GX->u128 = VX->u128;
            if(rex.w)
                GX->q[tmp8u&0x1] = ED->q[0];
            else
                GX->ud[tmp8u&0x3] = ED->dword[0];
            GY->u128 = 0;
            break;

        case 0x38:  /* VINSERTI128 Gx, Ex, imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            GETVY;
            tmp8u = F8;
            if(tmp8u&1) {
                GY->u128 = EX->u128;
                if(GX!=VX)
                    GX->u128 = VX->u128;
            } else {
                GX->u128 = EX->u128;
                if(GY!=VY)
                    GY->u128 = VY->u128;
            }
            break;
        case 0x39:  /* VEXTRACTI128 Ex, Gx, Ib */
            nextop = F8;
            GETGX;
            GETEX(1);
            GETGY;
            tmp8u = F8;
            EX->u128 = (tmp8u&1)?GY->u128:GX->u128;
            if(MODREG) {
                GETEY;
                EY->u128 = 0;
            }
            break;

        case 0x40:  /* VDPPS Gx, VX, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            tmpf = 0.0f;
            for(int i=0; i<4; ++i)
                if(tmp8u&(1<<(i+4)))
                    tmpf += VX->f[i]*EX->f[i];
            for(int i=0; i<4; ++i)
                GX->f[i] = (tmp8u&(1<<i))?tmpf:0.0f;
            if(vex.l) {
                GETEY;
                GETVY;
                tmpf = 0.0f;
                for(int i=0; i<4; ++i)
                    if(tmp8u&(1<<(i+4)))
                        tmpf += VY->f[i]*EY->f[i];
                for(int i=0; i<4; ++i)
                    GY->f[i] = (tmp8u&(1<<i))?tmpf:0.0f;
            } else
                GY->u128 = 0;
            break;
        case 0x41:  /* VDPPD Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            tmpd = 0.0;
            if(tmp8u&(1<<(4+0)))
                tmpd += VX->d[0]*EX->d[0];
            if(tmp8u&(1<<(4+1)))
                tmpd += VX->d[1]*EX->d[1];
            GX->d[0] = (tmp8u&(1<<(0)))?tmpd:0.0;
            GX->d[1] = (tmp8u&(1<<(1)))?tmpd:0.0;
            if(vex.l) {
                GETEY;
                GETVY;
                tmpd = 0.0;
                if(tmp8u&(1<<(4+0)))
                    tmpd += VY->d[0]*EY->d[0];
                if(tmp8u&(1<<(4+1)))
                    tmpd += VY->d[1]*EY->d[1];
                GY->d[0] = (tmp8u&(1<<(0)))?tmpd:0.0;
                GY->d[1] = (tmp8u&(1<<(1)))?tmpd:0.0;
            } else
                GY->u128 = 0;
            break;
        case 0x42:  /* VMPSADBW Gx, Vx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX; GETVX; GETGY; GETVY; GETEY;
            if(GX==EX) {
                eax1 = *EX;
                EX=&eax1;
            }
            if(GX==VX) {
                eay1 = *VX;
                VX=&eay1;
            }
            tmp8u = F8;
            {
                int src = tmp8u&3;
                int dst = (tmp8u>>2)&1;
                int b[11];
                for (int i=0; i<11; ++i)
                    b[i] = VX->ub[dst*4+i];
                for(int i=0; i<8; ++i) {
                    int tmp = abs(b[i+0]-EX->ub[src*4+0]);
                    tmp += abs(b[i+1]-EX->ub[src*4+1]);
                    tmp += abs(b[i+2]-EX->ub[src*4+2]);
                    tmp += abs(b[i+3]-EX->ub[src*4+3]);
                    GX->uw[i] = tmp;
                }
            }
            if(vex.l) {
                if(GY==EY) {
                    eax1 = *EY;
                    EY=&eax1;
                }
                if(GY==VY) {
                    eay1 = *VY;
                    VY=&eay1;
                }
                {
                    int src = (tmp8u>>3)&3;
                    int dst = (tmp8u>>5)&1;
                    int b[11];
                    for (int i=0; i<11; ++i)
                        b[i] = VY->ub[dst*4+i];
                    for(int i=0; i<8; ++i) {
                        int tmp = abs(b[i+0]-EY->ub[src*4+0]);
                        tmp += abs(b[i+1]-EY->ub[src*4+1]);
                        tmp += abs(b[i+2]-EY->ub[src*4+2]);
                        tmp += abs(b[i+3]-EY->ub[src*4+3]);
                        GY->uw[i] = tmp;
                    }
                }
            } else
                GY->u128 = 0;
            break;
                
        case 0x44:    /* VPCLMULQDQ Gx, Vx, Ex, imm8 */
            nextop = F8;
            GETGX;
            GETEX(1);
            GETVX;
            GETGY;
            tmp8u = F8;
            GX->u128 = pclmul_helper(VX->q[tmp8u&1], EX->q[(tmp8u>>4)&1]);
            if(vex.l) {
                GETVY;
                GETEY;
                GY->u128 = pclmul_helper(VY->q[tmp8u&1], EY->q[(tmp8u>>4)&1]);
            } else
                GY->u128 = 0;
            break;

        case 0x46:  /* VPERM2I128 Gx, Vx, Ex, Imm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETEY;
            GETGY;
            GETVY;
            u8 = F8;
            if(!vex.l) emit_signal(emu, SIGILL, (void*)R_RIP, 0);
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
            switch(u8&0x0f) {
                case 0 : GX->u128 = VX->u128; break;
                case 1 : GX->u128 = VY->u128; break;
                case 2 : GX->u128 = EX->u128; break;
                case 3 : GX->u128 = EY->u128; break;
                default: GX->u128 = 0; break;
            }
            switch((u8>>4)&0x0f) {
                case 0 : GY->u128 = VX->u128; break;
                case 1 : GY->u128 = VY->u128; break;
                case 2 : GY->u128 = EX->u128; break;
                case 3 : GY->u128 = EY->u128; break;
                default: GY->u128 = 0; break;
            }
            break;

        case 0x4A:      /* VBLENDVPS Gx, Vx, Ex, XMMImm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = (F8)>>4;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (emu->xmm[tmp8u].ud[i]>>31)?EX->ud[i]:VX->ud[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<4; ++i)
                    GY->ud[i] = (emu->ymm[tmp8u].ud[i]>>31)?EY->ud[i]:VY->ud[i];
            } else
                GY->u128 = 0;
            break;
        case 0x4B:      /* VBLENDVPD Gx, Vx, Ex, XMMImm8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = (F8)>>4;
            for(int i=0; i<2; ++i)
                GX->q[i] = (emu->xmm[tmp8u].q[i]>>63)?EX->q[i]:VX->q[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<2; ++i)
                    GY->q[i] = (emu->ymm[tmp8u].q[i]>>63)?EY->q[i]:VY->q[i];
            } else
                GY->u128 = 0;
            break;
        case 0x4C:      /* VBLENDPVB Gx, Vx, Ex, XMM/u8 */
            nextop = F8;
            GETEX(1);
            GETGX;
            GETVX;
            GETGY;
            tmp8u = F8;
            tmp8u>>=4;
            if(rex.is32bits)
                tmp8u&=7;
            for(int i=0; i<16; ++i)
                GX->ub[i] = (emu->xmm[tmp8u].ub[i]&0x80)?EX->ub[i]:VX->ub[i];
            if(vex.l) {
                GETEY;
                GETVY;
                for(int i=0; i<16; ++i)
                    GY->ub[i] = (emu->ymm[tmp8u].ub[i]&0x80)?EY->ub[i]:VY->ub[i];
            } else
                GY->u128 = 0;
            break;

        case 0x60:  /* VPCMPESTRM */
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
                emu->ymm[0].u128 = 0;
            }
            break;
        case 0x61:  /* VPCMPESTRI */
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
        case 0x62:  /* VPCMPISTRM */
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
                emu->ymm[0].u128 = 0;
            }
            break;
        case 0x63:  /* VPCMPISTRI */
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

        case 0xDF:      // VAESKEYGENASSIST Gx, Ex, u8
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
            GETGY;
            GY->u128 = 0;
            break;

        default:
            return 0;
    }
    return addr;
}
