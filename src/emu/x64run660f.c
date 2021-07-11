#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include "bridge.h"

#include "modrm.h"

int Run660F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int16_t tmp16s;
    uint16_t tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1, *opex2;
    mmx87_regs_t *opem, *opgm;

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
        GETED(0);
        GETGX;
        GX->q[0] = ED->q[0];
        break;
    case 0x13:                      /* MOVLPD Eq, Gx */
        nextop = F8;
        GETED(0);
        GETGX;
        ED->q[0] = GX->q[0];
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
        GETED(0);
        GETGX;
        GX->q[1] = ED->q[0];
        break;
    case 0x17:                      /* MOVHPD Ed, Gx */
        nextop = F8;
        GETED(0);
        GETGX;
        ED->q[0] = GX->q[1];
        break;

    case 0x1F:                      /* NOP (multi-byte) */
        nextop = F8;
        GETED(0);
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

    case 0x2C:                      /* CVTTPD2PI Gm, Ex */
        nextop = F8;
        GETEX(0);
        GETGM;
        GM->sd[0] = EX->d[0];
        GM->sd[1] = EX->d[1];
        break;
    case 0x2D:                      /* CVTPD2PI Gm, Ex */
        nextop = F8;
        GETEX(0);
        GETGM;
        switch((emu->mxcsr>>13)&3) {
            case ROUND_Nearest:
                GM->sd[0] = floor(EX->d[0]+0.5);
                GM->sd[1] = floor(EX->d[1]+0.5);
                break;
            case ROUND_Down:
                GM->sd[0] = floor(EX->d[0]);
                GM->sd[1] = floor(EX->d[1]);
                break;
            case ROUND_Up:
                GM->sd[0] = ceil(EX->d[0]);
                GM->sd[1] = ceil(EX->d[1]);
                break;
            case ROUND_Chop:
                GM->sd[0] = EX->d[0];
                GM->sd[1] = EX->d[1];
                break;
        }
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

            case 0x17:      // PTEST GX, EX
                nextop = F8;
                GETEX(0);
                GETGX;
                CONDITIONAL_SET_FLAG(!(GX->u128&EX->u128), F_ZF);
                CONDITIONAL_SET_FLAG(!((~GX->u128)&EX->u128), F_CF);
                break;

            case 0x1C:  /* PABSB Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<16; ++i) {
                    GX->sb[i] = abs(EX->sb[i]);
                }
                break;
            case 0x1D:  /* PABSW Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<8; ++i) {
                    GX->sw[i] = abs(EX->sw[i]);
                }
                break;
            case 0x1E:  /* PABSD Gx, Ex */
                nextop = F8;
                GETEX(0);
                GETGX;
                for (int i=0; i<4; ++i) {
                    GX->sd[i] = abs(EX->sd[i]);
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

            default:
                return 1;
        }
        break;

    case 0x3A:  // these are some SSE3 opcodes
        opcode = F8;
        switch(opcode) {
            case 0x0B:          // ROUNDSD Gx, Ex, u8
                nextop = F8;
                GETEX(1);
                GETGX;
                tmp8u = F8; // ignoring bit 3 interupt thingy
                if(tmp8u&4)
                    tmp8u = (emu->mxcsr>>13)&3;
                else
                    tmp8u &= 3;
                switch(tmp8u) {
                    case ROUND_Nearest:
                        GX->d[0] = floor(EX->d[0]+0.5);
                        break;
                    case ROUND_Down:
                        GX->d[0] = floor(EX->d[0]);
                        break;
                    case ROUND_Up:
                        GX->d[0] = ceil(EX->d[0]);
                        break;
                    case ROUND_Chop:
                        GX->d[0] = EX->d[0];
                        break;
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

            case 0x17:      // EXTRACTPS ED, GX, u8
                nextop = F8;
                GETED(1);
                GETGX;
                tmp8u = F8;
                ED->dword[0] = GX->ud[tmp8u&3];
                if(MODREG && rex.w) ED->dword[1] = 0;
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

            default:
                return 1;
        }
        break;
        
    GOCOND(0x40
        , nextop = F8;
        CHECK_FLAGS(emu);
        GETEW(0);
        GETGW;
        , if(rex.w) GW->q[0] = EW->q[0]; else GW->word[0] = EW->word[0];
        ,
    )                               /* 0x40 -> 0x4F CMOVxx Gw,Ew */ // conditional move, no sign

    case 0x50:                      /* MOVMSKPD Gd, Ex */
        nextop = F8;
        GETEX(0);
        GETGD;
        GD->dword[0] = 0;
        for(int i=0; i<2; ++i)
            GD->dword[0] |= ((EX->q[i]>>63)&1)<<i;
        break;
    case 0x51:                      /* SQRTPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] = sqrt(EX->d[0]);
        GX->d[1] = sqrt(EX->d[1]);
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
        GX->d[0] += EX->d[0];
        GX->d[1] += EX->d[1];
        break;
    case 0x59:                      /* MULPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] *= EX->d[0];
        GX->d[1] *= EX->d[1];
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
        switch((emu->mxcsr>>13)&3) {
            case ROUND_Nearest:
                GX->sd[0] = floorf(EX->f[0]+0.5f);
                GX->sd[1] = floorf(EX->f[1]+0.5f);
                GX->sd[2] = floorf(EX->f[2]+0.5f);
                GX->sd[3] = floorf(EX->f[3]+0.5f);
                break;
            case ROUND_Down:
                GX->sd[0] = floorf(EX->f[0]);
                GX->sd[1] = floorf(EX->f[1]);
                GX->sd[2] = floorf(EX->f[2]);
                GX->sd[3] = floorf(EX->f[3]);
                break;
            case ROUND_Up:
                GX->sd[0] = ceilf(EX->f[0]);
                GX->sd[1] = ceilf(EX->f[1]);
                GX->sd[2] = ceilf(EX->f[2]);
                GX->sd[3] = ceilf(EX->f[3]);
                break;
            case ROUND_Chop:
                GX->sd[0] = EX->f[0];
                GX->sd[1] = EX->f[1];
                GX->sd[2] = EX->f[2];
                GX->sd[3] = EX->f[3];
                break;
        }
        break;
    case 0x5C:                      /* SUBPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] -= EX->d[0];
        GX->d[1] -= EX->d[1];
        break;
    case 0x5D:                      /* MINPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || isless(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        if (isnan(GX->d[1]) || isnan(EX->d[1]) || isless(EX->d[1], GX->d[1]))
            GX->d[1] = EX->d[1];
        break;
    case 0x5E:                      /* DIVPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] /= EX->d[0];
        GX->d[1] /= EX->d[1];
        break;
    case 0x5F:                      /* MAXPD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || isgreater(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        if (isnan(GX->d[1]) || isnan(EX->d[1]) || isgreater(EX->d[1], GX->d[1]))
            GX->d[1] = EX->d[1];
        break;
        
    case 0x60:  /* PUNPCKLBW Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=7; i>0; --i)  // 0 is untouched
            GX->ub[2 * i] = GX->ub[i];
        if(GX==EX)
            for(int i=0; i<8; ++i)
                GX->ub[2 * i + 1] = GX->ub[2 * i];
        else 
            for(int i=0; i<8; ++i)
                GX->ub[2 * i + 1] = EX->ub[i];
        break;
    case 0x61:  /* PUNPCKLWD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=3; i>0; --i)
            GX->uw[2 * i] = GX->uw[i];
        if(GX==EX)
            for(int i=0; i<4; ++i)
                GX->uw[2 * i + 1] = GX->uw[2 * i];
        else
            for(int i=0; i<4; ++i)
                GX->uw[2 * i + 1] = EX->uw[i];
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
        for(int i=0; i<8; ++i)
            GX->ub[2 * i] = GX->ub[i + 8];
        if(GX==EX)
            for(int i=0; i<8; ++i)
                GX->ub[2 * i + 1] = GX->ub[2 * i];
        else
            for(int i=0; i<8; ++i)
                GX->ub[2 * i + 1] = EX->ub[i + 8];
        break;
    case 0x69:  /* PUNPCKHWD Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<4; ++i)
            GX->uw[2 * i] = GX->uw[i + 4];
        if(GX==EX)
            for(int i=0; i<4; ++i)
                GX->uw[2 * i + 1] = GX->uw[2 * i];
        else
            for(int i=0; i<4; ++i)
                GX->uw[2 * i + 1] = EX->uw[i + 4];
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
                return 1;
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
                if(tmp8u>31) {
                    for (int i=0; i<4; ++i) EX->sd[i] = (EX->sd[i]<0)?-1:0;
                } else
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
                return 1;
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
                    {EX->q[0] = EX->q[1] = 0;}
                else {
                    tmp8u*=8;
                    if (tmp8u < 64) {
                        EX->q[0] = (EX->q[0] >> tmp8u) | (EX->q[1] << (64 - tmp8u));
                        EX->q[1] = (EX->q[1] >> tmp8u);
                    } else {
                        EX->q[0] = EX->q[1] >> (tmp8u - 64);
                        EX->q[1] = 0;
                    }                    
                }
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
                    {EX->q[0] = EX->q[1] = 0;}
                else {
                    tmp8u*=8;
                    if (tmp8u < 64) {
                        EX->q[1] = (EX->q[1] << tmp8u) | (EX->q[0] >> (64 - tmp8u));
                        EX->q[0] = (EX->q[0] << tmp8u);
                    } else {
                        EX->q[1] = EX->q[0] << (tmp8u - 64);
                        EX->q[0] = 0;
                    }
                }
                break;
            default:
                return 1;
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
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
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
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
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
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
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
        GW->word[0] = EB->byte[0];
        break;

    case 0xBA:                      
        nextop = F8;
        switch((nextop>>3)&7) {
            case 4:                 /* BT Ew,Ib */
                CHECK_FLAGS(emu);
                GETEW(0);
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
                GETEW(0);
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
                GETEW(0);
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
                GETEW(0);
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
                return 1;
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
            EW=(reg64_t*)(((uintptr_t)(EW))+(tmp32s<<(rex.w?3:1)));
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
        if(rex.w) {
            tmp64u = EW->q[0];
            if(tmp64u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 0;
                while(!(tmp64u&(1LL<<tmp8u))) ++tmp8u;
                GW->q[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
        } else {
            tmp16u = EW->word[0];
            if(tmp16u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 0;
                while(!(tmp16u&(1<<tmp8u))) ++tmp8u;
                GW->word[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
        }
        break;
    case 0xBD:                      /* BSR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w) {
            tmp64u = EW->q[0];
            if(tmp64u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 63;
                while(!(tmp64u&(1LL<<tmp8u))) --tmp8u;
                GW->q[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
        } else {
            tmp16u = EW->word[0];
            if(tmp16u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 15;
                while(!(tmp16u&(1<<tmp8u))) --tmp8u;
                GW->word[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
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
        GD->dword[0] = EX->uw[tmp8u&7];  // 16bits extract, 0 extended
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
        GETEX(0);
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
            return 1;
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
        tmp8u=(EX->q[0]>15)?16:EX->ub[0];
        for (int i=0; i<8; ++i) 
            GX->sw[i] >>= tmp8u;
        break;
    case 0xE2:  /* PSRAD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        tmp8u=(EX->q[0]>31)?32:EX->ub[0];
        if(tmp8u>31) 
            for (int i=0; i<4; ++i)
                GX->sd[i] = (GX->sd[i]<0)?-1:0;
        else
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
        GX->sd[0] = EX->d[0];
        GX->sd[1] = EX->d[1];
        GX->q[1] = 0;
        break;
    case 0xE7:   /* MOVNTDQ Ex, Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->q[0] = GX->q[0];
        EX->q[1] = GX->q[1];
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
        GX->ud[0] |= EX->ud[0];
        GX->ud[1] |= EX->ud[1];
        GX->ud[2] |= EX->ud[2];
        GX->ud[3] |= EX->ud[3];
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
            {tmp8u=EX->q[0]; for (int i=0; i<2; ++i) GX->q[i] <<= tmp8u;}
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
        return 1;
    }
    return 0;
}