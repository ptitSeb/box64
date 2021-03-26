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
#include "../dynarec/arm64_lock.h"
#endif

#include "modrm.h"

int Run0F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1;
    mmx87_regs_t *opem, *opgm;

    opcode = F8;

    switch(opcode) {

        case 0x05:                      /* SYSCALL */
            x64Syscall(emu);
            break;

        case 0x10:                      /* MOVUPS Gx,Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            memcpy(GX, EX, 16); // unaligned, so carreful
            break;
        case 0x11:                      /* MOVUPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            memcpy(EX, GX, 16); // unaligned, so carreful
            break;
        case 0x12:                      
            nextop = F8;
            GETEX(0);
            GETGX;
            if((nextop&0xC0)==0xC0)    /* MOVHLPS Gx,Ex */
                GX->q[0] = EX->q[1];
            else
                GX->q[0] = EX->q[0];    /* MOVLPS Gx,Ex */
            break;
        case 0x13:                      /* MOVLPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            break;
        case 0x14:                      /* UNPCKLPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->ud[3] = EX->ud[1];
            GX->ud[2] = GX->ud[1];
            GX->ud[1] = EX->ud[0];
            break;
        case 0x15:                      /* UNPCKHPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->ud[0] = GX->ud[2];
            GX->ud[1] = EX->ud[2];
            GX->ud[2] = GX->ud[3];
            GX->ud[3] = EX->ud[3];
            break;
        case 0x16:                      /* MOVHPS Gx,Ex */
            nextop = F8;               /* MOVLHPS Gx,Ex (Ex==reg) */
            GETEX(0);
            GETGX;
            GX->q[1] = EX->q[0];
            break;
        case 0x17:                      /* MOVHPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[1];
            break;

        case 0x1F:                      /* NOP (multi-byte) */
            nextop = F8;
            GETED(0);
            break;

        case 0x28:                      /* MOVAPS Gx,Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->q[0] = EX->q[0];
            GX->q[1] = EX->q[1];
            break;
        case 0x29:                      /* MOVAPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            break;
        case 0x2A:                      /* CVTPI2PS Gx, Em */
            nextop = F8;
            GETEM(0);
            GETGX;
            GX->f[0] = EM->sd[0];
            GX->f[1] = EM->sd[1];
            break;
        case 0x2B:                      /* MOVNTPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            break;
        case 0x2C:                      /* CVTTPS2PI Gm, Ex */
            nextop = F8;
            GETEX(0);
            GETGM;
            GM->sd[1] = EX->f[1];
            GM->sd[0] = EX->f[0];
            break;
        case 0x2D:                      /* CVTPS2PI Gm, Ex */
            // rounding should be done; and indefinite integer should also be assigned if overflow or NaN/Inf
            nextop = F8;
            GETEX(0);
            GETGM;
            switch((emu->mxcsr>>13)&3) {
                case ROUND_Nearest:
                    GM->sd[1] = floorf(EX->f[1]+0.5f);
                    GM->sd[0] = floorf(EX->f[0]+0.5f);
                    break;
                case ROUND_Down:
                    GM->sd[1] = floorf(EX->f[1]);
                    GM->sd[0] = floorf(EX->f[0]);
                    break;
                case ROUND_Up:
                    GM->sd[1] = ceilf(EX->f[1]);
                    GM->sd[0] = ceilf(EX->f[0]);
                    break;
                case ROUND_Chop:
                    GM->sd[1] = EX->f[1];
                    GM->sd[0] = EX->f[0];
                    break;
            }
            break;
        case 0x2E:                      /* UCOMISS Gx, Ex */
            // same for now
        case 0x2F:                      /* COMISS Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GETEX(0);
            GETGX;
            if(isnan(GX->f[0]) || isnan(EX->f[0])) {
                SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
            } else if(isgreater(GX->f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            } else if(isless(GX->f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
            } else {
                SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            }
            CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
            break;


        GOCOND(0x40
            , nextop = F8;
            GETED(0);
            GETGD;
            CHECK_FLAGS(emu);
            , if(rex.w) {GD->q[0] = ED->q[0]; } else {GD->q[0] = ED->dword[0];}
            , if(!rex.w) GD->dword[1] = 0;
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign
        
        case 0x50:                      /* MOVMSKPS Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            GD->dword[0] = 0;
            for(int i=0; i<4; ++i)
                GD->dword[0] |= ((EX->ud[i]>>31)&1)<<i;
            break;
        case 0x51:                      /* SQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] = sqrtf(EX->f[i]);
            break;
        case 0x52:                      /* RSQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] = 1.0f/sqrtf(EX->f[i]);
            break;
        case 0x53:                      /* RCPPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] = 1.0f/EX->f[i];
            break;
        case 0x54:                      /* ANDPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->ud[i] &= EX->ud[i];
            break;
        case 0x55:                      /* ANDNPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->ud[i] = (~GX->ud[i]) & EX->ud[i];
            break;
        case 0x56:                      /* ORPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->ud[i] |= EX->ud[i];
            break;
        case 0x57:                      /* XORPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->ud[i] ^= EX->ud[i];
            break;
        case 0x58:                      /* ADDPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] += EX->f[i];
            break;
        case 0x59:                      /* MULPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] *= EX->f[i];
            break;
        case 0x5A:                      /* CVTPS2PD Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->d[1] = EX->f[1];
            GX->d[0] = EX->f[0];
            break;
        case 0x5B:                      /* CVTDQ2PS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->f[0] = EX->sd[0];
            GX->f[1] = EX->sd[1];
            GX->f[2] = EX->sd[2];
            GX->f[3] = EX->sd[3];
            break;
        case 0x5C:                      /* SUBPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] -= EX->f[i];
            break;
        case 0x5D:                      /* MINPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX->f[i]) || isnan(EX->f[i]) || isless(EX->f[i], GX->f[i]))
                    GX->f[i] = EX->f[i];
            }
            break;
        case 0x5E:                      /* DIVPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                GX->f[i] /= EX->f[i];
            break;
        case 0x5F:                      /* MAXPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX->f[i]) || isnan(EX->f[i]) || isgreater(EX->f[i], GX->f[i]))
                    GX->f[i] = EX->f[i];
            }
            break;
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
        case 0x64:                       /* PCMPGTB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 8; i++) {
                GM->ub[i] = (GM->sb[i] > EM->sb[i]) ? 0xFF : 0;
            }
            break;
        case 0x65:                       /* PCMPGTW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 4; i++) {
                GM->uw[i] = (GM->sw[i] > EM->sw[i]) ? 0xFFFF : 0;
            }
            break;
        case 0x66:                       /* PCMPGTD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 2; i++) {
                GM->ud[i] = (GM->sd[i] > EM->sd[i]) ? 0xFFFFFFFF : 0;
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

        case 0x6E:                      /* MOVD Gm, Ed */
            nextop = F8;
            GETED(0);
            GETGM;
            if(rex.w)
                GM->q = ED->q[0];
            else
                GM->q = ED->dword[0];    // zero extended
            break;
        case 0x6F:                      /* MOVQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = EM->q;
            break;

        case 0x74:                       /* PCMPEQB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 8; i++) {
                GM->ub[i] = (GM->sb[i] == EM->sb[i]) ? 0xFF : 0;
            }
            break;
        case 0x75:                       /* PCMPEQW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 4; i++) {
                GM->uw[i] = (GM->sw[i] == EM->sw[i]) ? 0xFFFF : 0;
            }
            break;
        case 0x76:                       /* PCMPEQD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 2; i++) {
                GM->ud[i] = (GM->sd[i] == EM->sd[i]) ? 0xFFFFFFFF : 0;
            }
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
        case 0xA3:                      /* BT Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = GD->byte[0];
            if(!MODREG)
            {
                ED=(reg64_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            if(rex.w) {
                tmp8u&=63;
                if(ED->q[0] & (1LL<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
            } else {
                tmp8u&=31;
                if(ED->dword[0] & (1<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
            }
            break;
        case 0xA4:                      /* SHLD Ed,Gd,Ib */
        case 0xA5:                      /* SHLD Ed,Gd,CL */
            nextop = F8;
            GETED((nextop==0xA4)?1:0);
            GETGD;
            if(opcode==0xA4)
                tmp8u = F8;
            else
                tmp8u = R_CL;
            if(rex.w)
                ED->q[0] = shld64(emu, ED->q[0], GD->q[0], tmp8u);
            else
                ED->q[0] = shld32(emu, ED->dword[0], GD->dword[0], tmp8u);
            break;

        case 0xAB:                      /* BTS Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = GD->byte[0];
            if(!MODREG)
            {
                ED=(reg64_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            if(rex.w) {
                tmp8u&=63;
                if(ED->q[0] & (1LL<<tmp8u))
                    SET_FLAG(F_CF);
                else {
                    ED->q[0] |= (1LL<<tmp8u);
                    CLEAR_FLAG(F_CF);
                }
            } else {
                tmp8u&=31;
                if(ED->dword[0] & (1<<tmp8u))
                    SET_FLAG(F_CF);
                else {
                    ED->dword[0] |= (1<<tmp8u);
                    CLEAR_FLAG(F_CF);
                }
                if(MODREG)
                    ED->dword[1] = 0;
            }
            break;
        case 0xAC:                      /* SHRD Ed,Gd,Ib */
        case 0xAD:                      /* SHRD Ed,Gd,CL */
            nextop = F8;
            GETED((nextop==0xAC)?1:0);
            GETGD;
            tmp8u = (opcode==0xAC)?(F8):R_CL;
            if(rex.w)
                ED->q[0] = shrd64(emu, ED->q[0], GD->q[0], tmp8u);
            else
                ED->q[0] = shrd32(emu, ED->dword[0], GD->dword[0], tmp8u);
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
                case 0:                 /* FXSAVE Ed */
                    if(rex.w)
                        fpu_fxsave64(emu, ED);
                    else
                        fpu_fxsave32(emu, ED);
                    break;
                case 1:                 /* FXRSTOR Ed */
                    if(rex.w)
                        fpu_fxrstor64(emu, ED);
                    else
                        fpu_fxrstor32(emu, ED);
                    break;
                case 2:                 /* LDMXCSR Md */
                    emu->mxcsr = ED->dword[0];
                    break;
                case 3:                 /* STMXCSR Md */
                    ED->dword[0] = emu->mxcsr;
                    break;
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
        case 0xB0:                      /* CMPXCHG Eb,Gb */
            nextop = F8;
            GETEB(0);
            GETGB;
            cmp8(emu, R_AL, EB->byte[0]);
            if(ACCESS_FLAG(F_ZF)) {
                EB->byte[0] = GB;
            } else {
                R_AL = EB->byte[0];
            }
            break;
        case 0xB1:                      /* CMPXCHG Ed,Gd */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w) {
                cmp64(emu, R_RAX, ED->q[0]);
                if(ACCESS_FLAG(F_ZF)) {
                    ED->q[0] = GD->q[0];
                } else {
                    R_RAX = ED->q[0];
                }
            } else {
                cmp32(emu, R_EAX, ED->dword[0]);
                if(ACCESS_FLAG(F_ZF)) {
                    if(MODREG)
                        ED->q[0] = GD->dword[0];
                    else
                        ED->dword[0] = GD->dword[0];
                } else {
                    R_RAX = ED->dword[0];
                }
            }
            break;
        case 0xB3:                      /* BTR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = GD->byte[0];
            if(!MODREG)
            {
                ED=(reg64_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            if(rex.w) {
                tmp8u&=63;
                if(ED->q[0] & (1LL<<tmp8u)) {
                    SET_FLAG(F_CF);
                    ED->q[0] ^= (1LL<<tmp8u);
                } else
                    CLEAR_FLAG(F_CF);
            } else {
                tmp8u&=31;
                if(ED->dword[0] & (1<<tmp8u)) {
                    SET_FLAG(F_CF);
                    ED->dword[0] ^= (1<<tmp8u);
                } else
                    CLEAR_FLAG(F_CF);
                if(MODREG)
                    ED->dword[1] = 0;
            }
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

        case 0xBA:                      
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:                 /* BT Ed,Ib */
                    CHECK_FLAGS(emu);
                    GETED(1);
                    tmp8u = F8;
                    if(!MODREG)
                        ED=(reg64_t*)(((uintptr_t*)(ED))+(tmp8u>>5));
                    if(rex.w) {
                        tmp8u&=63;
                        if(ED->q[0] & (1LL<<tmp8u))
                            SET_FLAG(F_CF);
                        else
                            CLEAR_FLAG(F_CF);
                    } else {
                        tmp8u&=31;
                        if(ED->dword[0] & (1<<tmp8u))
                            SET_FLAG(F_CF);
                        else
                            CLEAR_FLAG(F_CF);
                    }
                    break;
                case 5:             /* BTS Ed, Ib */
                    CHECK_FLAGS(emu);
                    GETED(1);
                    tmp8u = F8;
                    if(!MODREG)
                        ED=(reg64_t*)(((uintptr_t*)(ED))+(tmp8u>>5));
                    if(rex.w) {
                        tmp8u&=63;
                        if(ED->q[0] & (1LL<<tmp8u)) {
                            SET_FLAG(F_CF);
                        } else {
                            ED->q[0] ^= (1LL<<tmp8u);
                            CLEAR_FLAG(F_CF);
                        }
                    } else {
                        tmp8u&=31;
                        if(ED->dword[0] & (1<<tmp8u)) {
                            SET_FLAG(F_CF);
                        } else {
                            ED->dword[0] ^= (1<<tmp8u);
                            CLEAR_FLAG(F_CF);
                        }
                    }
                    break;
                case 6:             /* BTR Ed, Ib */
                    CHECK_FLAGS(emu);
                    GETED(1);
                    tmp8u = F8;
                    if(!MODREG)
                        ED=(reg64_t*)(((uintptr_t*)(ED))+(tmp8u>>5));
                    if(rex.w) {
                        tmp8u&=63;
                        if(ED->q[0] & (1LL<<tmp8u)) {
                            SET_FLAG(F_CF);
                            ED->q[0] ^= (1LL<<tmp8u);
                        } else
                            CLEAR_FLAG(F_CF);
                    } else {
                        tmp8u&=31;
                        if(ED->dword[0] & (1<<tmp8u)) {
                            SET_FLAG(F_CF);
                            ED->dword[0] ^= (1<<tmp8u);
                        } else
                            CLEAR_FLAG(F_CF);
                    }
                    break;
                case 7:             /* BTC Ed, Ib */
                    CHECK_FLAGS(emu);
                    GETED(1);
                    tmp8u = F8;
                    if(!MODREG)
                        ED=(reg64_t*)(((uintptr_t*)(ED))+(tmp8u>>5));
                    if(rex.w) {
                        tmp8u&=63;
                        if(ED->q[0] & (1LL<<tmp8u))
                            SET_FLAG(F_CF);
                        else
                            CLEAR_FLAG(F_CF);
                        ED->q[0] ^= (1LL<<tmp8u);
                    } else {
                        tmp8u&=31;
                        if(ED->dword[0] & (1<<tmp8u))
                            SET_FLAG(F_CF);
                        else
                            CLEAR_FLAG(F_CF);
                        ED->dword[0] ^= (1<<tmp8u);
                    }
                    break;

                default:
                    return 1;
            }
            break;
        case 0xBB:                      /* BTC Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = GD->byte[0];
            if(!MODREG)
            {
                if(rex.w)
                    ED=(reg64_t*)(((uint64_t*)(ED))+(tmp8u>>6));
                else
                    ED=(reg64_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            if(rex.w) {
                tmp8u&=63;
                if(ED->q[0] & (1LL<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
                ED->q[0] ^= (1LL<<tmp8u);
            } else {
                tmp8u&=31;
                if(ED->dword[0] & (1<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
                ED->dword[0] ^= (1<<tmp8u);
                if(MODREG)
                    ED->dword[1] = 0;
            }
            break;
        case 0xBC:                      /* BSF Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w) {
                tmp64u = ED->q[0];
                if(tmp64u) {
                    CLEAR_FLAG(F_ZF);
                    tmp8u = 0;
                    while(!(tmp64u&(1LL<<tmp8u))) ++tmp8u;
                    GD->q[0] = tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
            } else {
                tmp32u = ED->dword[0];
                if(tmp32u) {
                    CLEAR_FLAG(F_ZF);
                    tmp8u = 0;
                    while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                    GD->q[0] = tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
            }
            break;
        case 0xBD:                      /* BSR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w) {
                tmp64u = ED->q[0];
                if(tmp64u) {
                    CLEAR_FLAG(F_ZF);
                    tmp8u = 63;
                    while(!(tmp64u&(1LL<<tmp8u))) --tmp8u;
                    GD->q[0] = tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
            } else {
                tmp32u = ED->dword[0];
                if(tmp32u) {
                    CLEAR_FLAG(F_ZF);
                    tmp8u = 31;
                    while(!(tmp32u&(1<<tmp8u))) --tmp8u;
                    GD->q[0] = tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
            }
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
        case 0xC0:                      /* XADD Gb,Eb */
            nextop = F8;
            GETEB(0);
            GETGB;
            tmp8u = add8(emu, EB->byte[0], GB);
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            break;
        case 0xC1:                      /* XADD Gd,Ed */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w) {
                tmp64u = add64(emu, ED->q[0], GD->q[0]);
                GD->q[0] = ED->q[0];
                ED->q[0] = tmp64u;
            } else {
                tmp32u = add32(emu, ED->dword[0], GD->dword[0]);
                GD->q[0] = ED->dword[0];
                if(MODREG)
                    ED->q[0] = tmp32u;
                else
                    ED->dword[0] = tmp32u;
            }
            break;
        case 0xC2:                      /* CMPPS Gx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            tmp8u = F8;
            for(int i=0; i<4; ++i) {
                tmp8s = 0;
                switch(tmp8u&7) {
                    case 0: tmp8s=(GX->f[i] == EX->f[i]); break;
                    case 1: tmp8s=isless(GX->f[i], EX->f[i]); break;
                    case 2: tmp8s=islessequal(GX->f[i], EX->f[i]); break;
                    case 3: tmp8s=isnan(GX->f[i]) || isnan(EX->f[i]); break;
                    case 4: tmp8s=(GX->f[i] != EX->f[i]); break;
                    case 5: tmp8s=isnan(GX->f[i]) || isnan(EX->f[i]) || isgreaterequal(GX->f[i], EX->f[i]); break;
                    case 6: tmp8s=isnan(GX->f[i]) || isnan(EX->f[i]) || isgreater(GX->f[i], EX->f[i]); break;
                    case 7: tmp8s=!isnan(GX->f[i]) && !isnan(EX->f[i]); break;
                }
                GX->ud[i]=(tmp8s)?0xffffffff:0;
            }
            break;

        case 0xC6:                      /* SHUFPS Gx, Ex, Ib */
            nextop = F8;
            GETEX(1);
            GETGX;
            tmp8u = F8;
            for(int i=0; i<2; ++i) {
                eax1.ud[i] = GX->ud[(tmp8u>>(i*2))&3];
            }
            for(int i=2; i<4; ++i) {
                eax1.ud[i] = EX->ud[(tmp8u>>(i*2))&3];
            }
            GX->q[0] = eax1.q[0];
            GX->q[1] = eax1.q[1];
            break;
        case 0xC7:                      /* CMPXCHG8B Eq */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            if(rex.w) {
                tmp64u = ED->q[0];
                tmp64u2= ED->q[1];
                if(R_RAX == tmp64u && R_RDX == tmp64u2) {
                    SET_FLAG(F_ZF);
                    ED->q[0] = R_EBX;
                    ED->q[1] = R_ECX;
                } else {
                    CLEAR_FLAG(F_ZF);
                    R_RAX = tmp64u;
                    R_RDX = tmp64u2;
                }
            } else {
                tmp32u = ED->dword[0];
                tmp32u2= ED->dword[1];
                if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                    SET_FLAG(F_ZF);
                    ED->dword[0] = R_EBX;
                    ED->dword[1] = R_ECX;
                } else {
                    CLEAR_FLAG(F_ZF);
                    R_RAX = tmp32u;
                    R_RDX = tmp32u2;
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
            tmp8u = (opcode&7)+(rex.b<<3);
            if(rex.w)
                emu->regs[tmp8u].q[0] = __builtin_bswap64(emu->regs[tmp8u].q[0]);
            else
                emu->regs[tmp8u].q[0] = __builtin_bswap32(emu->regs[tmp8u].dword[0]);
            break;

        case 0xD1:                   /* PSRLW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>15)
                GM->q=0; 
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<4; ++i)
                    GM->uw[i] >>= tmp8u;
            }
            break;
        case 0xD2:                   /* PSRLD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>31)
                GM->q=0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM->ud[i] >>= tmp8u;
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

        case 0xE1:                   /* PSRAW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>15)
                tmp8u = 16;
            else
                tmp8u = EM->ub[0];
            for(int i=0; i<4; ++i)
                GM->sw[i] >>= tmp8u;
            break;
        case 0xE2:                   /* PSRAD Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>31) {
                for(int i=0; i<2; ++i)
                    GM->sd[i] = (GM->sd[i]<0)?-1:0;
            } else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM->sd[i] >>= tmp8u;
            }
            break;
        case 0xE3:                   /* PSRAQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>63)
                tmp8u = 64;
            else
                tmp8u = EM->ub[0];
            GM->sq >>= tmp8u;
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

        case 0xF1:                   /* PSLLW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>15)
                GM->q = 0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<4; ++i)
                    GM->sw[i] <<= tmp8u;
            }
            break;
        case 0xF2:                   /* PSLLD Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>31)
                GM->q = 0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM->sd[i] <<= tmp8u;
            }
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