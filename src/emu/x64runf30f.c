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

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestF30F(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t RunF30F(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1;
    mmx87_regs_t *opem;
    #ifdef TEST_INTERPRETER
    x64emu_t*emu = test->emu;
    #endif
    int is_nan;

    opcode = F8;

    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = F8;
        GETEX4(0);
        GETGX;
        GX->ud[0] = EX->ud[0];
        if(!MODREG) {
            // EX is not a register (reg to reg only move 31:0)
            GX->ud[1] = GX->ud[2] = GX->ud[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = F8;
        GETEX4(0);
        GETGX;
        EX->ud[0] = GX->ud[0];
        break;
    case 0x12:  /* MOVSLDUP Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->ud[1] = GX->ud[0] = EX->ud[0];
        GX->ud[3] = GX->ud[2] = EX->ud[2];
        break;

    case 0x16:  /* MOVSHDUP Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->ud[1] = GX->ud[0] = EX->ud[1];
        GX->ud[3] = GX->ud[2] = EX->ud[3];
        break;

    case 0x1E:  /* ENDBR64 */
        nextop = F8;
        GETED(0);
        break;

    case 0x2A:  /* CVTSI2SS Gx, Ed */
        nextop = F8;
        GETED(0);
        GETGX;
        if(rex.w)
            GX->f[0] = ED->sq[0];
        else
            GX->f[0] = ED->sdword[0];
        break;
    case 0x2B:  /* MOVNTSS Ex Gx */
        nextop = F8;
        if(!MODREG) {
            GETEX4(0);
            GETGX;
            EX->ud[0] = GX->ud[0];
        }
        break;
    case 0x2C:  /* CVTTSS2SI Gd, Ex */
        nextop = F8;
        GETEX(0);
        GETGD;
        if (rex.w) {
            if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>=(float)0x7fffffffffffffffLL)
                GD->q[0] = 0x8000000000000000LL;
            else
                GD->sq[0] = EX->f[0];
        } else {
            if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>=(float)0x7fffffff)
                GD->dword[0] = 0x80000000;
            else
                GD->sdword[0] = EX->f[0];
            GD->dword[1] = 0;
        }
        break;
    case 0x2D:  /* CVTSS2SI Gd, Ex */
        nextop = F8;
        GETEX(0);
        GETGD;
        if(rex.w) {
            if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>=(float)0x7fffffffffffffffLL)
                GD->q[0] = 0x8000000000000000LL;
            else
                switch(emu->mxcsr.f.MXCSR_RC) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        GD->sq[0] = nearbyintf(EX->f[0]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        GD->sq[0] = floorf(EX->f[0]);
                        break;
                    case ROUND_Up:
                        GD->sq[0] = ceilf(EX->f[0]);
                        break;
                    case ROUND_Chop:
                        GD->sq[0] = EX->f[0];
                        break;
                }
        } else {
            if(isnanf(EX->f[0]))
                tmp64s = INT32_MIN;
            else
                switch(emu->mxcsr.f.MXCSR_RC) {
                    case ROUND_Nearest: {
                        int round = fegetround();
                        fesetround(FE_TONEAREST);
                        tmp64s = nearbyintf(EX->f[0]);
                        fesetround(round);
                        break;
                    }
                    case ROUND_Down:
                        tmp64s = floorf(EX->f[0]);
                        break;
                    case ROUND_Up:
                        tmp64s = ceilf(EX->f[0]);
                        break;
                    case ROUND_Chop:
                        tmp64s = EX->f[0];
                        break;
                }
            if (tmp64s==(int32_t)tmp64s)
                GD->sdword[0] = (int32_t)tmp64s;
            else
                GD->sdword[0] = INT32_MIN;
            GD->dword[1] = 0;
        }
        break;

    case 0x38:  /* MAP 0F38 */
        opcode = F8;
        switch(opcode) {

            case 0xF6: /* ADOX Gd, Rd */
                nextop = F8;
                GETED(0);
                GETGD;
                CHECK_FLAGS(emu);
                if(rex.w) {
                    if (ACCESS_FLAG(F_OF)) {
                        tmp64u = 1 + (GD->q[0] & 0xFFFFFFFF) + (ED->q[0] & 0xFFFFFFFF);
                        tmp64u2 = 1 + GD->q[0] + ED->q[0];
                    } else {
                        tmp64u = (GD->q[0] & 0xFFFFFFFF) + (ED->q[0] & 0xFFFFFFFF);
                        tmp64u2 = GD->q[0] + ED->q[0];
                    }
                    tmp64u = (tmp64u >> 32) + (GD->q[0] >> 32) + (ED->q[0] >> 32);
                    CONDITIONAL_SET_FLAG(tmp64u & 0x100000000LL, F_OF);
                    GD->q[0] = tmp64u2;
                } else {
                    if (ACCESS_FLAG(F_OF))
                        GD->q[0] = 1LL + GD->dword[0] + ED->dword[0];
                    else
                        GD->q[0] = (uint64_t)GD->dword[0] + ED->dword[0];
                    CONDITIONAL_SET_FLAG(GD->q[0] & 0x100000000LL, F_OF);
                    GD->dword[1] = 0;
                }
                break;

            default:
                return 0;
        }
        break;

    case 0x51:  /* SQRTSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (EX->f[0]<0)
            GX->f[0] = -NAN;
        else if (isnanf(EX->f[0]))
            GX->f[0] = EX->f[0];
        else
            GX->f[0] = sqrtf(EX->f[0]);
        break;
    case 0x52:  /* RSQRTSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(EX->f[0]==0)
            GX->f[0] = 1.0f/EX->f[0];
        else if (EX->f[0]<0)
            GX->f[0] = -NAN;
        else if (isnan(EX->f[0]))
            GX->f[0] = EX->f[0];
        else if (isinf(EX->f[0]))
            GX->f[0] = 0.0;
        else
            GX->f[0] = 1.0f/sqrtf(EX->f[0]);
        break;
    case 0x53:  /* RCPSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        NAN_PROPAGATION(GX->f[0], EX->f[0], break);
        GX->f[0] = 1.0f/EX->f[0];
        break;

    case 0x58:  /* ADDSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_F_2(GX, EX);
        NAN_PROPAGATION(GX->f[0], EX->f[0], break);
        GX->f[0] += EX->f[0];
        CHECK_NAN_F(GX);
        break;
    case 0x59:  /* MULSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_F_2(GX, EX);
        NAN_PROPAGATION(GX->f[0], EX->f[0], break);
        GX->f[0] *= EX->f[0];
        CHECK_NAN_F(GX);
        break;
    case 0x5A:  /* CVTSS2SD Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->d[0] = EX->f[0];
        break;
    case 0x5B:  /* CVTTPS2DQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        for(int i=0; i<4; ++i) {
            if(isnanf(EX->f[i]))
                tmp64s = INT32_MIN;
            else
                tmp64s = EX->f[i];
            if (tmp64s==(int32_t)tmp64s) {
                GX->sd[i] = (int32_t)tmp64s;
            } else {
                GX->sd[i] = INT32_MIN;
            }
        }
        break;
    case 0x5C:  /* SUBSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        NAN_PROPAGATION(GX->f[0], EX->f[0], break);
        GX->f[0] -= EX->f[0];
        break;
    case 0x5D:  /* MINSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if(isnan(GX->f[0]) || isnan(EX->f[0]) || islessequal(EX->f[0], GX->f[0]))
            GX->f[0] = EX->f[0];
        break;
    case 0x5E:  /* DIVSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        MARK_NAN_F_2(GX, EX);
        NAN_PROPAGATION(GX->f[0], EX->f[0], break);
        GX->f[0] /= EX->f[0];
        CHECK_NAN_F(GX);
        break;
    case 0x5F:  /* MAXSS Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        if (isnan(GX->f[0]) || isnan(EX->f[0]) || isgreaterequal(EX->f[0], GX->f[0]))
            GX->f[0] = EX->f[0];
        break;

    case 0x6F:  /* MOVDQU Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(GX, EX, 16);    // unaligned...
        break;
    case 0x70:  /* PSHUFHW Gx, Ex, Ib */
        nextop = F8;
        GETEX(1);
        GETGX;
        tmp8u = F8;
        if(GX==EX) {
            for (int i=0; i<4; ++i)
                eax1.uw[4+i] = EX->uw[4+((tmp8u>>(i*2))&3)];
            GX->q[1] = eax1.q[1];
        } else {
            for (int i=0; i<4; ++i)
                GX->uw[4+i] = EX->uw[4+((tmp8u>>(i*2))&3)];
            GX->q[0] = EX->q[0];
        }
        break;

    case 0x7E:  /* MOVQ Gx, Ex */
        nextop = F8;
        GETEX(0);
        GETGX;
        GX->q[0] = EX->q[0];
        GX->q[1] = 0;
        break;
    case 0x7F:  /* MOVDQU Ex, Gx */
        nextop = F8;
        GETEX(0);
        GETGX;
        memcpy(EX, GX, 16);    // unaligned...
        break;

    case 0xAE:
        nextop = F8;
        switch((nextop>>3)&7) {
            case 0: /* RDFSBASE */
            case 1: /* RDGSBASE */
                if(!rex.is32bits && MODREG) {
                    GETED(0);
                    int seg = _FS+((nextop>>3)&7);
                    uintptr_t addr = emu->segs_offs[seg];
                    if(rex.w)
                        ED->q[0] = addr;
                    else {
                        ED->dword[0] = addr;
                        ED->dword[1] = 0;
                    }
                } else {
                    return 0;
                }
                break;
            case 2: /* WRFSBASE */
            case 3: /* WRGSBASE */
                if(!rex.is32bits && MODREG) {
                    GETED(0);
                    int seg = _FS+((nextop>>3)&7)-2;
                    uintptr_t base = rex.w?ED->q[0]:ED->dword[0];
                    emu->segs_offs[seg] = base;
                } else {
                    return 0;
                }
                break;
            default: return 0;
        }
        break;

    case 0xB8:  /* POPCNT Gd,Ed */
        nextop = F8;
        GETED(0);
        GETGD;
        if(rex.w)
            GD->q[0] = __builtin_popcountl(ED->q[0]);
        else
            GD->q[0] = __builtin_popcount(ED->dword[0]);
        RESET_FLAGS(emu);
        CLEAR_FLAG(F_OF);
        CLEAR_FLAG(F_SF);
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CONDITIONAL_SET_FLAG(GD->q[0]==0, F_ZF);
        break;

    case 0xBC:  /* TZCNT Ed,Gd */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETED(0);
        GETGD;
        if(rex.w) {
            tmp64u = ED->q[0];
            if(tmp64u) {
                tmp8u = 0;
                while(!(tmp64u&(1LL<<tmp8u))) ++tmp8u;
                GD->q[0] = tmp8u;
                CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
                CLEAR_FLAG(F_CF);
            } else {
                CLEAR_FLAG(F_ZF);
                SET_FLAG(F_CF);
                GD->q[0] = 64;
            }
        } else {
            tmp32u = ED->dword[0];
            if(tmp32u) {
                tmp8u = 0;
                while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                GD->q[0] = tmp8u;
                CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
                CLEAR_FLAG(F_CF);
            } else {
                CLEAR_FLAG(F_ZF);
                SET_FLAG(F_CF);
                GD->q[0] = 32;
            }
        }
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_SF);
        CLEAR_FLAG(F_PF);
        if(!BOX64ENV(cputype))
            CLEAR_FLAG(F_OF);
    break;
    case 0xBD:  /* LZCNT Ed,Gd */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETED(0);
        GETGD;
        if(rex.w) {
            tmp64u = ED->q[0];
            tmp8u = (tmp64u)?__builtin_clzl(tmp64u):64;
            CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u==64, F_CF);
        } else {
            tmp32u = ED->dword[0];
            tmp8u = (tmp32u)?__builtin_clz(tmp32u):32;
            CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u==32, F_CF);
        }
        GD->q[0] = tmp8u;
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_SF);
        if(!BOX64ENV(cputype))
            CLEAR_FLAG(F_OF);
        break;

    case 0xC2:  /* CMPSS Gx, Ex, Ib */
        nextop = F8;
        GETEX(1);
        GETGX;
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(GX->f[0] == EX->f[0]); break;
            case 1: tmp8s=isless(GX->f[0], EX->f[0]) && !(isnan(GX->f[0]) || isnan(EX->f[0])); break;
            case 2: tmp8s=islessequal(GX->f[0], EX->f[0]) && !(isnan(GX->f[0]) || isnan(EX->f[0])); break;
            case 3: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]); break;
            case 4: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]) || (GX->f[0] != EX->f[0]); break;
            case 5: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]) || isgreaterequal(GX->f[0], EX->f[0]); break;
            case 6: tmp8s=isnan(GX->f[0]) || isnan(EX->f[0]) || isgreater(GX->f[0], EX->f[0]); break;
            case 7: tmp8s=!isnan(GX->f[0]) && !isnan(EX->f[0]); break;
        }
        GX->ud[0]=(tmp8s)?0xffffffff:0;
        break;

    case 0xD6:  /* MOVQ2DQ Gx, Em */
        nextop = F8;
        GETEM(0);
        GETGX;
        GX->q[0] = EM->q;
        GX->q[1] = 0;
        break;

    case 0xE6:  /* CVTDQ2PD Gx, Ex */
        nextop = F8;
        GETEX8(0);
        GETGX;
        GX->d[1] = EX->sd[1];
        GX->d[0] = EX->sd[0];
        break;

    default:
        return 0;
    }
    return addr;
}
