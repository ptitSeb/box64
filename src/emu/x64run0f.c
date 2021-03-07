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
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int Run0F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;
    mmx87_regs_t *opem, *opgm;

    opcode = F8;

    switch(opcode) {

        case 0x05:                      /* SYSCALL */
            x64Syscall(emu);
            break;

        case 0x11:                      /* MOVUPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            memcpy(EX, GX, 16); // unaligned, so carreful
            break;

        case 0x1F:                      /* NOP (multi-byte) */
            nextop = F8;
            GETED(0);
            break;

        case 0x29:                      /* MOVAPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            break;


        GOCOND(0x40
            , nextop = F8;
            GETED(0);
            GETGD;
            CHECK_FLAGS(emu);
            , if(rex.w) {GD->q[0] = ED->q[0]; } else {GD->dword[0] = ED->dword[0];}
            ,
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign
        
        case 0x60:                      /* PUNPCKLBW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->ub[7] = EM->ub[3];
            GM->ub[6] = GM->ub[3];
            GM->ub[5] = EM->ub[2];
            GM->ub[4] = GM->ub[2];
            GM->ub[3] = EM->ub[1];
            GM->ub[2] = GM->ub[1];
            GM->ub[1] = EM->ub[0];
            break;
        case 0x61:                      /* PUNPCKLWD Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->uw[3] = EM->uw[1];
            GM->uw[2] = GM->uw[1];
            GM->uw[1] = EM->uw[0];
            break;
        case 0x62:                      /* PUNPCKLDQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->ud[1] = EM->ud[0];
            break;
        case 0x63:                      /* PACKSSWB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->sb[0] = (GM->sw[0] > 127) ? 127 : ((GM->sw[0] < -128) ? -128 : GM->sw[0]);
            GM->sb[1] = (GM->sw[1] > 127) ? 127 : ((GM->sw[1] < -128) ? -128 : GM->sw[1]);
            GM->sb[2] = (GM->sw[2] > 127) ? 127 : ((GM->sw[2] < -128) ? -128 : GM->sw[2]);
            GM->sb[3] = (GM->sw[3] > 127) ? 127 : ((GM->sw[3] < -128) ? -128 : GM->sw[3]);
            if(EM==GM)
                GM->ud[1] = GM->ud[0];
            else {
                GM->sb[4] = (EM->sw[0] > 127) ? 127 : ((EM->sw[0] < -128) ? -128 : EM->sw[0]);
                GM->sb[5] = (EM->sw[1] > 127) ? 127 : ((EM->sw[1] < -128) ? -128 : EM->sw[1]);
                GM->sb[6] = (EM->sw[2] > 127) ? 127 : ((EM->sw[2] < -128) ? -128 : EM->sw[2]);
                GM->sb[7] = (EM->sw[3] > 127) ? 127 : ((EM->sw[3] < -128) ? -128 : EM->sw[3]);
            }
            break;

        case 0x67:                       /* PACKUSWB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->ub[i] = (GM->sw[i]<0)?0:((GM->sw[i]>0xff)?0xff:GM->sw[i]);
            if(EM==GM)
                GM->ud[1] = GM->ud[0];
            else
                for(int i=0; i<4; ++i)
                    GM->ub[4+i] = (EM->sw[i]<0)?0:((EM->sw[i]>0xff)?0xff:EM->sw[i]);
            break;
        case 0x68:                       /* PUNPCKHBW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->ub[2 * i] = GM->ub[i + 4];
            if(EM==GM)
                for(int i=0; i<4; ++i)
                    GM->ub[2 * i + 1] = GM->ub[2 * i];
            else
                for(int i=0; i<4; ++i)
                    GM->ub[2 * i + 1] = EM->ub[i + 4];
            break;
        case 0x69:                       /* PUNPCKHWD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<2; ++i)
                GM->uw[2 * i] = GM->uw[i + 2];
            if(EM==GM)
                for(int i=0; i<2; ++i)
                    GM->uw[2 * i + 1] = GM->uw[2 * i];
            else
                for(int i=0; i<2; ++i)
                    GM->uw[2 * i + 1] = EM->uw[i + 2];
            break;
        case 0x6A:                       /* PUNPCKHDQ Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->ud[0] = GM->ud[1];
            if(EM!=GM)
                GM->ud[1] = EM->ud[1];
            break;
        case 0x6B:                       /* PACKSSDW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<2; ++i)
                GM->sw[i] = (GM->sd[i]<-32768)?-32768:((GM->sd[i]>32767)?32767:GM->sd[i]);
            if(EM==GM)
                GM->ud[1] = GM->ud[0];
            else
                for(int i=0; i<2; ++i)
                    GM->sw[2+i] = (EM->sd[i]<-32768)?-32768:((EM->sd[i]>32767)?32767:EM->sd[i]);
            break;

        case 0x6F:                      /* MOVQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = EM->q;
            break;

        case 0x77:                      /* EMMS */
            // empty MMX, FPU now usable
            emu->top = 0;
            emu->fpu_stack = 0;
            break;

        case 0x7E:                       /* MOVD Ed, Gm */
            nextop = F8;
            GETED(0);
            GETGM;
            if(rex.w)
                ED->q[0] = GM->q;
            else
                ED->q[0] = GM->ud[0];
            break;
        case 0x7F:                      /* MOVQ Em, Gm */
            nextop = F8;
            GETEM(0);
            GETGM;
            EM->q = GM->q;
            break;
        GOCOND(0x80
            , tmp32s = F32S; CHECK_FLAGS(emu);
            , R_RIP += tmp32s;
            ,
        )                               /* 0x80 -> 0x8F Jxx */
        GOCOND(0x90
            , nextop = F8; CHECK_FLAGS(emu);
            GETEB(0);
            , EB->byte[0]=1;
            , EB->byte[0]=0;
        )                               /* 0x90 -> 0x9F SETxx Eb */

        case 0xA2:                      /* CPUID */
            tmp32u = R_EAX;
            my_cpuid(emu, tmp32u);
            break;

        case 0xAE:                      /* Grp Ed (SSE) */
            nextop = F8;
            if((nextop&0xF8)==0xE8) {
                return 0;                   /* LFENCE */
            }
            if((nextop&0xF8)==0xF0) {
                return 0;                   /* MFENCE */
            }
            if((nextop&0xF8)==0xF8) {
                return 0;                   /* SFENCE */
            }
            GETED(0);
            switch((nextop>>3)&7) {
                default:
                    return 1;
            }
            break;
        case 0xAF:                      /* IMUL Gd,Ed */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                GD->q[0] = imul64(emu, GD->q[0], ED->q[0]);
            else
                GD->dword[0] = imul32(emu, GD->dword[0], ED->dword[0]);
            break;

        case 0xB6:                      /* MOVZX Gd,Eb */
            nextop = F8;
            GETEB(0);
            GETGD;
            GD->q[0] = EB->byte[0];
            break;
        case 0xB7:                      /* MOVZX Gd,Ew */
            nextop = F8;
            GETEW(0);
            GETGD;
            GD->q[0] = EW->word[0];
            break;

        case 0xBE:                      /* MOVSX Gd,Eb */
            nextop = F8;
            GETEB(0);
            GETGD;
            if(rex.w)
                GD->sq[0] = EB->sbyte[0];
            else {
                GD->sdword[0] = EB->sbyte[0];
                GD->dword[1] = 0;
            }
            break;
        case 0xBF:                      /* MOVSX Gd,Ew */
            nextop = F8;
            GETEW(0);
            GETGD;
            if(rex.w)
                GD->sq[0] = EW->sword[0];
            else {
                GD->sdword[0] = EW->sword[0];
                GD->dword[1] = 0;
            }
            break;

        case 0xD3:                   /* PSRLQ Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = (EM->q > 63) ? 0L : (GM->q >> EM->q);
            break;

        case 0xD5:                   /* PMULLW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM->sw[i] * EM->sw[i];
                GM->sw[i] = tmp32s;
            }
            break;

        case 0xD8:                   /* PSUBUSB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM->ub[i] - EM->ub[i];
                GM->ub[i] = (tmp32s < 0) ? 0 : tmp32s;
            }
            break;
        case 0xD9:                   /* PSUBUSW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM->uw[i] - EM->uw[i];
                GM->uw[i] = (tmp32s < 0) ? 0 : tmp32s;
            }
            break;

        case 0xDB:                   /* PAND Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q &= EM->q;
            break;
        case 0xDC:                   /* PADDUSB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i) {
                tmp32u = (uint32_t)GM->ub[i] + EM->ub[i];
                GM->ub[i] = (tmp32u>255) ? 255 : tmp32u;
            }
            break;
        case 0xDD:                   /* PADDUSW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32u = (uint32_t)GM->uw[i] + EM->uw[i];
                GM->uw[i] = (tmp32u>65535) ? 65535 : tmp32u;
            }
            break;

        case 0xDF:                   /* PANDN Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = (~GM->q) & EM->q;
            break;

        case 0xE4:                   /* PMULHUW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32u = (int32_t)GM->uw[i] * EM->uw[i];
                GM->uw[i] = (tmp32u>>16)&0xffff;
            }
            break;
        case 0xE5:                   /* PMULHW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM->sw[i] * EM->sw[i];
                GM->uw[i] = (tmp32s>>16)&0xffff;
            }
            break;

        case 0xE8:                   /* PSUBSB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM->sb[i] - EM->sb[i];
                GM->sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            break;
        case 0xE9:                   /* PSUBSW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM->sw[i] - EM->sw[i];
                GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            break;

        case 0xEB:                   /* POR Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q |= EM->q;
            break;
        case 0xEC:                   /* PADDSB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM->sb[i] + EM->sb[i];
                GM->sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            break;
        case 0xED:                   /* PADDSW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM->sw[i] + EM->sw[i];
                GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            break;

        case 0xEF:                   /* PXOR Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q ^= EM->q;
            break;

        case 0xF3:                   /* PSLLQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = (EM->q > 63) ? 0L : (GM->q << EM->ub[0]);
            break;

        case 0xF5:                   /* PMADDWD Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i=0; i<2; ++i) {
                int offset = i * 2;
                tmp32s = (int32_t)GM->sw[offset + 0] * EM->sw[offset + 0];
                tmp32s2 = (int32_t)GM->sw[offset + 1] * EM->sw[offset + 1];
                GM->sd[i] = tmp32s + tmp32s2;
            }
            break;

        case 0xF8:                   /* PSUBB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i)
                GM->sb[i] -= EM->sb[i];
            break;
        case 0xF9:                   /* PSUBW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->sw[i] -= EM->sw[i];
            break;
        case 0xFA:                   /* PSUBD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<2; ++i)
                GM->sd[i] -= EM->sd[i];
            break;

        case 0xFC:                   /* PADDB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i)
                GM->sb[i] += EM->sb[i];
            break;
        case 0xFD:                   /* PADDW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->sw[i] += EM->sw[i];
            break;
        case 0xFE:                   /* PADDD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<2; ++i)
                GM->sd[i] += EM->sd[i];
            break;

        default:
            return 1;
    }
    return 0;
}