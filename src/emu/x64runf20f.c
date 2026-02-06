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
#include "bridge.h"
#include "emit_signals.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestF20F(x64test_t *test, rex_t rex, uintptr_t addr, int *step)
#else
uintptr_t RunF20F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    int32_t tmp32s;
    int64_t tmp64s0, tmp64s1;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1;
    mmx87_regs_t *opgm;
    int mask_nan[4];
    int is_nan;
    #ifdef TEST_INTERPRETER
    x64emu_t*emu = test->emu;
    #endif

    opcode = F8;

    switch(opcode) {

    case 0x10:  /* MOVSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        if(!MODREG) {
            // EX is not a register
            GX->q[1] = 0;
        }
        break;
    case 0x11:  /* MOVSD Ex, Gx */
        nextop = F8;
        GETEX8(0);
        GETGX;
        EX->q[0] = GX->q[0];
        break;
    case 0x12:  /* MOVDDUP Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        GX->q[1] = GX->q[0] = EX->q[0];
        break;

    case 0x1E:  /* NOP */
        nextop = F8;
        _GETED(0);
        break;

    case 0x2A:  /* CVTSI2SD Gx, Ed */
        nextop = F8;
        _GETED(0);
        GETGX;
        if(rex.w) {
            GX->d[0] = ED->sq[0];
        } else {
            GX->d[0] = ED->sdword[0];
        }
        break;
    case 0x2B:  /* MOVNTSD Ex, Gx */
        nextop = F8;
        if(!MODREG) {
            GETEX8(0);
            GETGX;
            EX->q[0] = GX->q[0];
        }
        break;
    case 0x2C:  /* CVTTSD2SI Gd, Ex */
        nextop = F8;
        _GETEX(0);
        GETGD;
        if(rex.w)
            if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>=(double)0x7fffffffffffffffLL)
                GD->q[0] = 0x8000000000000000LL;
            else
                GD->sq[0] = EX->d[0];
        else {
            if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>(double)0x7fffffff)
                GD->dword[0] = 0x80000000;
            else
                GD->sdword[0] = EX->d[0];
            GD->dword[1] = 0;
        }
        break;
    case 0x2D:  /* CVTSD2SI Gd, Ex */
        nextop = F8;
        _GETEX(0);
        GETGD;
        if(rex.w) {
            if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>=(double)0x7fffffffffffffffLL)
                GD->q[0] = 0x8000000000000000LL;
            else
                switch(emu->mxcsr.f.MXCSR_RC) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GD->sq[0] = nearbyint(EX->d[0]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GD->sq[0] = floor(EX->d[0]);
                        break;
                    case ROUND_Up:
                        GD->sq[0] = ceil(EX->d[0]);
                        break;
                    case ROUND_Chop:
                        GD->sq[0] = EX->d[0];
                        break;
                }
        } else {
            if(isnan(EX->d[0]) || isinf(EX->d[0]) || EX->d[0]>(double)0x7fffffff)
                GD->dword[0] = 0x80000000;
            else
                switch(emu->mxcsr.f.MXCSR_RC) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GD->sdword[0] = nearbyint(EX->d[0]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GD->sdword[0] = floor(EX->d[0]);
                        break;
                    case ROUND_Up:
                        GD->sdword[0] = ceil(EX->d[0]);
                        break;
                    case ROUND_Chop:
                        GD->sdword[0] = EX->d[0];
                        break;
                }
            GD->dword[1] = 0;
        }
        break;

    case 0x38:  // more opcodes
            opcode = F8;
            switch(opcode) {

                case 0xF0:  // CRC32 Gd, Eb
                    nextop = F8;
                    _GETEB(0);
                    GETGD;
                    GD->dword[0] ^=  EB->byte[0];
                    for (int i = 0; i < 8; i++) {
                        if (GD->dword[0] & 1)
                            GD->dword[0] = (GD->dword[0] >> 1) ^ 0x82f63b78;
                        else
                            GD->dword[0] = (GD->dword[0] >> 1);
                    }
                    GD->dword[1] = 0;
                    break;
                case 0xF1:  // CRC32 Gd, Ed
                    nextop = F8;
                    _GETED(0);
                    GETGD;
                    for(int j=0; j<4*(rex.w+1); ++j) {
                        GD->dword[0] ^=  ED->byte[j];
                        for (int i = 0; i < 8; i++) {
                            if (GD->dword[0] & 1)
                                GD->dword[0] = (GD->dword[0] >> 1) ^ 0x82f63b78;
                            else
                                GD->dword[0] = (GD->dword[0] >> 1);
                        }
                    }
                    GD->dword[1] = 0;
                    break;

                default:
                    return 0;
            }
        break;
        
    case 0x51:  /* SQRTSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        if(EX->d[0]<0.0 )
            GX->d[0] = -NAN;
        else if(isnan(EX->d[0]))
            GX->d[0] = EX->d[0];
        else
            GX->d[0] = sqrt(EX->d[0]);
        break;

    case 0x58:  /* ADDSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        MARK_NAN_D_2(GX, EX);
        {
            NAN_PROPAGATION(GX->d[0], EX->d[0], break);
            GX->d[0] += EX->d[0];
        }
        CHECK_NAN_D(GX);
        break;
    case 0x59:  /* MULSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        // mul generate a -NAN only if doing (+/-)inf * (+/-)0
        MARK_NAN_D_2(GX, EX);
        {
            NAN_PROPAGATION(GX->d[0], EX->d[0], break);
            GX->d[0] *= EX->d[0];
        }
        CHECK_NAN_D(GX);
        break;
    case 0x5A:  /* CVTSD2SS Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        GX->f[0] = EX->d[0];
        break;

    case 0x5C:  /* SUBSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        MARK_NAN_D_2(GX, EX);
        {
            NAN_PROPAGATION(GX->d[0], EX->d[0], break);
            GX->d[0] -= EX->d[0];
        }
        CHECK_NAN_D(GX);
        break;
    case 0x5D:  /* MINSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || islessequal(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        break;
    case 0x5E:  /* DIVSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        MARK_NAN_D_2(GX, EX);
        NAN_PROPAGATION(GX->d[0], EX->d[0], break);
        GX->d[0] /= EX->d[0];
        CHECK_NAN_D(GX);
        break;
    case 0x5F:  /* MAXSD Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        if (isnan(GX->d[0]) || isnan(EX->d[0]) || isgreaterequal(EX->d[0], GX->d[0]))
            GX->d[0] = EX->d[0];
        break;

    case 0x70:  /* PSHUFLW Gx, Ex, Ib */
        nextop = F8;
        _GETEX(1);
        GETGX;
        tmp8u = F8;
        if(GX==EX) {
            for (int i=0; i<4; ++i)
                eax1.uw[i] = EX->uw[(tmp8u>>(i*2))&3];
            GX->q[0] = eax1.q[0];
        } else {
            for (int i=0; i<4; ++i)
                GX->uw[i] = EX->uw[(tmp8u>>(i*2))&3];
            GX->q[1] = EX->q[1];
        }
        break;

    case 0x78:  /* INSERTQ Gx, Ex, ib, ib */
        // AMD only
        nextop = F8;
        if(!BOX64ENV(cputype) || !(MODREG)) {
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
        } else {
            //TODO: test /r
            GETGX;
            GETEX(2);
            tmp8s = F8&0x3f;
            tmp8u = F8&0x3f;
            tmp64u = (tmp8s==63)?~0ULL:((1ULL<<(tmp8s+1))-1);
            GX->q[0] &=~(tmp64u<<tmp8u);
            GX->q[0] |= (EX->q[0]&tmp64u)<<tmp8u;
        }
        break;
    case 0x79:  /* INSERTQ Gx, Ex */
        // AMD only
        nextop = F8;
        if(!BOX64ENV(cputype) || !(MODREG)) {
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
        } else {
            //TODO: test /r
            GETGX;
            GETEX(2);
            tmp8u = EX->ub[8]&0x3f;
            tmp8s = EX->ub[9]&0x3f;
            tmp64u = (tmp8s==63)?~0ULL:((1ULL<<(tmp8s+1))-1);
            GX->q[0] &=~(tmp64u<<tmp8u);
            GX->q[0] |= (EX->q[0]&tmp64u)<<tmp8u;
        }
        break;

    case 0x7C:  /* HADDPS Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        mask_nan[0] = isnanf(GX->f[0]) || isnanf(GX->f[1]);
        mask_nan[1] = isnanf(GX->f[2]) || isnanf(GX->f[3]);
        GX->f[0] += GX->f[1];
        GX->f[1] = GX->f[2] + GX->f[3];
        if(EX==GX) {
            GX->q[1] = GX->q[0];
            mask_nan[2] = mask_nan[0];
            mask_nan[3] = mask_nan[1];
        } else {
            mask_nan[2] = isnanf(EX->f[0]) || isnanf(EX->f[1]);
            mask_nan[3] = isnanf(EX->f[2]) || isnanf(EX->f[3]);
            GX->f[2] = EX->f[0] + EX->f[1];
            GX->f[3] = EX->f[2] + EX->f[3];
        }
        CHECK_NAN_VF(GX);
        break;
    case 0x7D:  /* HSUBPS Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        mask_nan[0] = isnanf(GX->f[0]) || isnanf(GX->f[1]);
        mask_nan[1] = isnanf(GX->f[2]) || isnanf(GX->f[3]);
        GX->f[0] -= GX->f[1];
        GX->f[1] = GX->f[2] - GX->f[3];
        if(EX==GX) {
            GX->q[1] = GX->q[0];
            mask_nan[2] = mask_nan[0];
            mask_nan[3] = mask_nan[1];
        } else {
            mask_nan[2] = isnanf(EX->f[0]) || isnanf(EX->f[1]);
            mask_nan[3] = isnanf(EX->f[2]) || isnanf(EX->f[3]);
            GX->f[2] = EX->f[0] - EX->f[1];
            GX->f[3] = EX->f[2] - EX->f[3];
        }
        CHECK_NAN_VF(GX);
        break;

    GOCOND(0x80
        , tmp32s = F32S; CHECK_FLAGS(emu);
        , addr += tmp32s;
        ,,STEP3
    )                               /* 0x80 -> 0x8F Jxx */

    case 0xA5:  // ignore F2 prefix
    case 0xB7:
    case 0xBA:
    case 0xBC:  // this one is still BSR, not TZCNT
        #ifdef TEST_INTERPRETER 
        return Test0F(test, rex, addr-1, step);
        #else
        return Run0F(emu, rex, addr-1, step);
        #endif

    case 0xC2:  /* CMPSD Gx, Ex, Ib */
        nextop = F8;
        _GETEX(1);
        GETGX;
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(GX->d[0] == EX->d[0]); break;
            case 1: tmp8s=isless(GX->d[0], EX->d[0]) && !(isnan(GX->d[0]) || isnan(EX->d[0])); break;
            case 2: tmp8s=islessequal(GX->d[0], EX->d[0]) && !(isnan(GX->d[0]) || isnan(EX->d[0])); break;
            case 3: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]); break;
            case 4: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]) || (GX->d[0] != EX->d[0]); break;
            case 5: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]) || isgreaterequal(GX->d[0], EX->d[0]); break;
            case 6: tmp8s=isnan(GX->d[0]) || isnan(EX->d[0]) || isgreater(GX->d[0], EX->d[0]); break;
            case 7: tmp8s=!isnan(GX->d[0]) && !isnan(EX->d[0]); break;
        }
        GX->q[0]=(tmp8s)?0xffffffffffffffffLL:0LL;
        break;

    case 0xD0:  /* ADDSUBPS Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        MARK_NAN_VF_2(GX, EX);
        GX->f[0] -= EX->f[0];
        GX->f[1] += EX->f[1];
        GX->f[2] -= EX->f[2];
        GX->f[3] += EX->f[3];
        CHECK_NAN_VF(GX);
        break;

    case 0xD6:  /* MOVDQ2Q Gm, Ex */
        nextop = F8;
        _GETEX(0);
        GETGM;
        GM->q = EX->q[0];
        break;

    case 0xE6:  /* CVTPD2DQ Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        switch(emu->mxcsr.f.MXCSR_RC) {
            case ROUND_Nearest: {
                int round = fegetround();
                fesetround(FE_TONEAREST);
                tmp64s0 = nearbyint(EX->d[0]);
                tmp64s1 = nearbyint(EX->d[1]);
                fesetround(round);
                break;
            }
            case ROUND_Down:
                tmp64s0 = floor(EX->d[0]);
                tmp64s1 = floor(EX->d[1]);
                break;
            case ROUND_Up:
                tmp64s0 = ceil(EX->d[0]);
                tmp64s1 = ceil(EX->d[1]);
                break;
            case ROUND_Chop:
                tmp64s0 = EX->d[0];
                tmp64s1 = EX->d[1];
                break;
        }
        if (tmp64s0==(int32_t)tmp64s0 && !isnan(EX->d[0])) {
            GX->sd[0] = (int32_t)tmp64s0;
        } else {
            GX->sd[0] = INT32_MIN;
        }
        if (tmp64s1==(int32_t)tmp64s1 && !isnan(EX->d[1])) {
            GX->sd[1] = (int32_t)tmp64s1;
        } else {
            GX->sd[1] = INT32_MIN;
        }

        GX->q[1] = 0;
        break;

    case 0xF0:  /* LDDQU Gx, Ex */
        nextop = F8;
        _GETEX(0);
        GETGX;
        memcpy(GX, EX, 16);
        break;

   default:
        return 0;
    }
    return addr;
}
