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
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int Run660F(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    int32_t tmp32s;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;

    opcode = F8;

    switch(opcode) {

    case 0x1F:                      /* NOP (multi-byte) */
        nextop = F8;
        GETED(0);
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
        GX->sd[0] = EX->f[0];
        GX->sd[1] = EX->f[1];
        GX->sd[2] = EX->f[2];
        GX->sd[3] = EX->f[3];
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

    case 0xD6:                      /* MOVQ Ex,Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        EX->q[0] = GX->q[0];
        if(MODREG)
            EX->q[1] = 0;
        break;

    case 0xEF:                      /* PXOR Gx,Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] ^= EX->q[0];
        GX->q[1] ^= EX->q[1];
        break;

    default:
        return 1;
    }
    return 0;
}