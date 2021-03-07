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
    int32_t tmp32s;
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

        case 0xEC:                   /* PADDSB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM->sb[i] + EM->sb[i];
                GM->sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            break;

        case 0xFC:                   /* PADDB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i)
                GM->sb[i] += EM->sb[i];
            break;

        default:
            return 1;
    }
    return 0;
}