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
#include "box64cpu_util.h"
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
#include "freq.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test0F(x64test_t *test, rex_t rex, uintptr_t addr, int *step)
#else
uintptr_t Run0F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1;
    mmx87_regs_t *opem, *opgm, eam1;
    uint8_t maskps[4];

#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    switch(opcode) {

        case 0x00:
            if(rex.is32bits) {          /* VERx Ed */
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0:                 /* SLDT Ew */
                        GETEW(0);
                        EW->word[0] = 0;
                        if(MODREG)
                            EW->word[1] = 0;
                        break;
                    case 1:                 /* STR Ew */
                        GETEW(0);
                        EW->word[0] = 0x7f; // dummy return
                        if(MODREG)
                            EW->word[1] = 0;
                        break;
                    case 4: //VERR
                    case 5: //VERW
                        GETEW(0);
                        if(!EW->word[0])
                            CLEAR_FLAG(F_ZF);
                        else
                            SET_FLAG(F_ZF); // should test if selector is ok
                        break;
                    default:
                        return 0;
                }
            } else
                return 0;
            break;
        case 0x01:                      /* XGETBV, SGDT, etc... */
            nextop = F8;
            GETED(0);
            if(MODREG)
            switch(nextop) {
                case 0xC8:  /* MONITOR */
                    // this is a privilege opcode...
                    #ifndef TEST_INTERPRETER
                    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
                    #endif
                    break;
                case 0xC9:  /* MWAIT */
                    // this is a privilege opcode...
                    #ifndef TEST_INTERPRETER
                    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
                    #endif
                    break;
                case 0xD0:
                    if(R_RCX) {
                        #ifndef TEST_INTERPRETER
                        EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                        #endif
                    } else {
                        R_RAX = 0b111;   // x87 & SSE & AVX for now
                        R_RDX = 0;
                    }
                    break;
                case 0xE0:
                case 0xE1:
                case 0xE2:
                case 0xE3:
                case 0xE4:
                case 0xE5:
                case 0xE6:
                case 0xE7:  /* SMSW Ew */
                    ED->word[0] = (1<<0) | (1<<4); // only PE and ET set...
                    break;
                case 0xF9:  /* RDTSCP */
                    tmp64u = ReadTSC(emu);
                    if(box64_rdtsc_shift)
                        tmp64u<<=box64_rdtsc_shift;
                    R_RAX = tmp64u & 0xffffffff;
                    R_RDX = tmp64u >> 32;
                    R_RCX = helper_getcpu(emu);
                    break;
                default:
                    return 0;
            } else
                switch((nextop>>3)&7) {
                    case 0:                 /* SGDT Ed */
                        ED->word[0] = 0x7f;    // dummy return...
                        if(rex.is32bits) {
                            *(uint32_t*)(&ED->word[1]) = 0x3000;
                        } else {
                            *(uint64_t*)(&ED->word[1]) = 0xfffffe0000077000LL;
                        }
                        break;
                    case 1:                 /* SIDT Ed */
                        ED->word[0] = 0xfff;    // dummy return, like "disabled"
                        if(rex.is32bits) {
                            *(uint32_t*)(&ED->word[1]) = 0x0000;
                        } else {
                            *(uint64_t*)(&ED->word[1]) = 0xfffffe0000000000LL;
                        }
                        break;
                    case 4:                 /* SMSW Ew */
                        // dummy for now... Do I need to track CR0 state?
                        ED->word[0] = (1<<0) | (1<<4); // only PE and ET set...
                        break;
                    default:
                        return 0;
            }
            break;

        case 0x03:                      /* LSL */
            nextop = F8;
            GETED(0);
            GETGD;
            CHECK_FLAGS(emu);
            tmp8u = ED->word[0]>>3;
            tmp8s = !!(ED->word[0]&2);
            if (tmp8u>0x10 || !tmp8s?emu->segldt[tmp8u].present:my_context->seggdt[tmp8u].present) {
                CLEAR_FLAG(F_ZF);
            } else {
                GD->dword[0] = tmp8s?emu->segldt[tmp8u].limit:my_context->seggdt[tmp8u].limit;
                SET_FLAG(F_ZF);
            }
            break;

        case 0x05:                      /* SYSCALL */
            #ifndef TEST_INTERPRETER
            R_RIP = addr;
            EmuX64Syscall(emu);
            #endif
            break;
        case 0x06:                      /* CLTS */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
            #endif
            break;

        case 0x08:                      /* INVD */
        case 0x09:                      /* WBINVD */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
            #endif
            break;

        case 0x0B:                      /* UD2 */
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
            break;

        case 0x0D:
            nextop = F8;
            GETED(0);
            switch((nextop>>3)&7) {
                case 0: //PREFETCH?
                    __builtin_prefetch((void*)ED, 0, 0);
                    break;
                case 1: //PREFETCHW
                    __builtin_prefetch((void*)ED, 1, 0);
                    break;
                case 2: //PREFETCHWT1
                    __builtin_prefetch((void*)ED, 1, 0);
                    break;
                default:    //NOP
                    break;
            }
            break;
        case 0x0E:                      /* FEMMS */
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
            break;

        case 0x10:                      /* MOVUPS Gx,Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->u128 = EX->u128;
            break;
        case 0x11:                      /* MOVUPS Ex,Gx */
            nextop = F8;
            GETEX(0);
            GETGX;
            EX->u128 = GX->u128;
            break;
        case 0x12:
            nextop = F8;
            GETEX(0);
            GETGX;
            if(MODREG)    /* MOVHLPS Gx, Ex */
                GX->q[0] = EX->q[1];
            else
                GX->q[0] = EX->q[0];    /* MOVLPS Gx, Ex */
            break;
        case 0x13:                      /* MOVLPS Ex, Gx */
            nextop = F8;
            if(!MODREG) {
                GETEX(0);
                GETGX;
                EX->q[0] = GX->q[0];
            }
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
        case 0x18:                       /* PREFETCHh Ed */
            nextop = F8;
            _GETED(0);
            if(MODREG) {
            } else
            switch((nextop>>3)&7) {
                case 0: //PREFETCHnta
                case 1: //PREFETCH1
                case 2: //PREFETCH2
                case 3: //PREFETCH3
                    __builtin_prefetch((void*)ED, 0, 0); // ignoring wich level of cache
                    break;
                default:    //NOP
                    break;
            }
            break;

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
        case 0x20:                      /* MOV REG, crX */
        case 0x21:                      /* MOV REG, drX */
        case 0x22:                      /* MOV cxR, REG */
        case 0x23:                      /* MOV drX, REG */
            // this is a privilege opcode...
            nextop = F8;
            #ifndef TEST_INTERPRETER
            tmp8u = (rex.r*8)+(nextop>>3&7);
            if((((opcode==20) || (opcode==22)) && ((tmp8u==1) || (tmp8u==5) || (tmp8u==6) || (tmp8u==7) || (tmp8u>8))) || (((opcode==0x21) || (opcode==0x23) && rex.r))) {
                EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            } else
                EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
            #endif
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
            tmp64s = EX->f[1];
            if (tmp64s==(int32_t)tmp64s && !isnanf(EX->f[1]))
                GM->sd[1] = (int32_t)tmp64s;
            else
                GM->sd[1] = INT32_MIN;
            tmp64s = EX->f[0];
            if (tmp64s==(int32_t)tmp64s && !isnanf(EX->f[0]))
                GM->sd[0] = (int32_t)tmp64s;
            else
                GM->sd[0] = INT32_MIN;
            break;
        case 0x2D:                      /* CVTPS2PI Gm, Ex */
            // rounding should be done; and indefinite integer should also be assigned if overflow or NaN/Inf
            nextop = F8;
            GETEX(0);
            GETGM;
            for(int i=1; i>=0; --i) {
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
                if (tmp64s==(int32_t)tmp64s)
                    GM->sd[i] = (int32_t)tmp64s;
                else
                    GM->sd[i] = INT32_MIN;
            }
            break;
        case 0x2E:                      /* UCOMISS Gx, Ex */
            // same for now
        case 0x2F:                      /* COMISS Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GETEX(0);
            GETGX;
            if(isnanf(GX->f[0]) || isnanf(EX->f[0])) {
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
        case 0x30:                      /* WRMSR */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0);
            #endif
            break;
        case 0x31:                   /* RDTSC */
            tmp64u = ReadTSC(emu);
            if(box64_rdtsc_shift)
                tmp64u<<=box64_rdtsc_shift;
            R_RDX = tmp64u>>32;
            R_RAX = tmp64u&0xFFFFFFFF;
            break;
        case 0x32:                   /* RDMSR */
            // priviledge instruction
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
            STEP;
            #endif
            break;

        case 0x34:                  /* SYSENTER */
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
            STEP;
            #endif
            break;
        case 0x35:                  /* SYSEXIT */
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
            STEP;
            #endif
            break;

        case 0x38:  // these are some SSE3 opcodes
            opcode = F8;
            switch(opcode) {
                case 0x00:  /* PSHUFB */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    eam1 = *GM;
                    for (int i=0; i<8; ++i) {
                        if(EM->ub[i]&128)
                            GM->ub[i] = 0;
                        else
                            GM->ub[i] = eam1.ub[EM->ub[i]&7];
                    }
                    break;
                case 0x01: /* PHADDW Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 2; ++i)
                        GM->sw[i] = GM->sw[i * 2 + 0] + GM->sw[i * 2 + 1];
                    if (GM == EM) {
                        GM->sd[1] = GM->sd[0];
                    } else {
                        for (int i = 0; i < 2; ++i)
                            GM->sw[2 + i] = EM->sw[i * 2 + 0] + EM->sw[i * 2 + 1];
                    }
                    break;
                case 0x02: /* PHADDD Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    GM->sd[0] = GM->sd[0] + GM->sd[1];
                    if (GM == EM) {
                        GM->sd[1] = GM->sd[0];
                    } else {
                        GM->sd[1] = EM->sd[0] + EM->sd[1];
                    }
                    break;
                case 0x03: /* PHADDSW Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 2; ++i) {
                        tmp32s = GM->sw[i * 2 + 0] + GM->sw[i * 2 + 1];
                        GM->sw[i] = (tmp32s < -32768) ? -32768 : ((tmp32s > 32767) ? 32767 : tmp32s);
                    }
                    if (GM == EM) {
                        GM->sd[1] = GM->sd[0];
                    } else {
                        for (int i = 0; i < 2; ++i) {
                            tmp32s = EM->sw[i * 2 + 0] + EM->sw[i * 2 + 1];
                            GM->sw[2 + i] = (tmp32s < -32768) ? -32768 : ((tmp32s > 32767) ? 32767 : tmp32s);
                        }
                    }
                    break;
                case 0x04:  /* PMADDUBSW Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    if(GM==EM) {
                        eam1 = *EM;
                        EM = &eam1;
                    }
                    for (int i=0; i<4; ++i) {
                        tmp32s = (int32_t)(GM->ub[i*2+0])*EM->sb[i*2+0] + (int32_t)(GM->ub[i*2+1])*EM->sb[i*2+1];
                        GM->sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                    }
                    break;
                case 0x05: /* PHSUBW Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 2; ++i)
                        GM->sw[i] = GM->sw[i * 2 + 0] - GM->sw[i * 2 + 1];
                    if (GM == EM) {
                        GM->sd[1] = GM->sd[0];
                    } else {
                        for (int i = 0; i < 2; ++i)
                            GM->sw[2 + i] = EM->sw[i * 2 + 0] - EM->sw[i * 2 + 1];
                    }
                    break;
                case 0x06: /* PHSUBD Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    GM->sd[0] = GM->sd[0] - GM->sd[1];
                    if (GM == EM) {
                        GM->sd[1] = GM->sd[0];
                    } else {
                        GM->sd[1] = EM->sd[0] - EM->sd[1];
                    }
                    break;
                case 0x07: /* PHSUBSW Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 2; ++i) {
                        tmp32s = GM->sw[i * 2 + 0] - GM->sw[i * 2 + 1];
                        GM->sw[i] = (tmp32s < -32768) ? -32768 : ((tmp32s > 32767) ? 32767 : tmp32s);
                    }
                    if (GM == EM) {
                        GM->sd[1] = GM->sd[0];
                    } else {
                        for (int i = 0; i < 2; ++i) {
                            tmp32s = EM->sw[i * 2 + 0] - EM->sw[i * 2 + 1];
                            GM->sw[2 + i] = (tmp32s < -32768) ? -32768 : ((tmp32s > 32767) ? 32767 : tmp32s);
                        }
                    }
                    break;
                case 0x08: /* PSIGNB Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 8; ++i)
                        GM->sb[i] *= (EM->sb[i] < 0) ? -1 : ((EM->sb[i] > 0) ? 1 : 0);
                    break;
                case 0x09: /* PSIGNW Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 4; ++i)
                        GM->sw[i] *= (EM->sw[i] < 0) ? -1 : ((EM->sw[i] > 0) ? 1 : 0);
                    break;
                case 0x0A: /* PSIGND Gm,Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i = 0; i < 2; ++i)
                        GM->sd[i] *= (EM->sd[i] < 0) ? -1 : ((EM->sd[i] > 0) ? 1 : 0);
                    break;
                case 0x0B:  /* PMULHRSW Gm, Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i=0; i<4; ++i) {
                        tmp32s = ((((int32_t)(GM->sw[i])*(int32_t)(EM->sw[i]))>>14) + 1)>>1;
                        GM->uw[i] = tmp32s&0xffff;
                    }
                    break;

                case 0x1C:  /* PABSB Gm, Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i=0; i<8; ++i) {
                        GM->ub[i] = abs(EM->sb[i]);
                    }
                    break;
                case 0x1D:  /* PABSW Gm, Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i=0; i<4; ++i) {
                        GM->uw[i] = abs(EM->sw[i]);
                    }
                    break;
                case 0x1E:  /* PABSD Gm, Em */
                    nextop = F8;
                    GETEM(0);
                    GETGM;
                    for (int i=0; i<2; ++i) {
                        GM->ud[i] = abs(EM->sd[i]);
                    }
                    break;

                case 0xC8:  /* SHA1NEXTE Gx, Ex */
                    nextop = F8;
                    GETGX;
                    GETEX(0);
                    sha1nexte(emu, GX, EX);
                    break;
                case 0xC9:  /* SHA1MSG1 Gx, Ex */
                    nextop = F8;
                    GETGX;
                    GETEX(0);
                    sha1msg1(emu, GX, EX);
                    break;
                case 0xCA:  /* SHA1MSG2 Gx, Ex */
                    nextop = F8;
                    GETGX;
                    GETEX(0);
                    sha1msg2(emu, GX, EX);
                    break;
                case 0xCB:  /* SHA256RNDS2 Gx, Ex (, XMM0) */
                    nextop = F8;
                    GETGX;
                    GETEX(0);
                    sha256rnds2(emu, GX, EX);
                    break;
                case 0xCC:  /* SHA256MSG1 Gx, Ex */
                    nextop = F8;
                    GETGX;
                    GETEX(0);
                    sha256msg1(emu, GX, EX);
                    break;
                case 0xCD:  /* SHA256MSG2 Gx, Ex */
                    nextop = F8;
                    GETGX;
                    GETEX(0);
                    sha256msg2(emu, GX, EX);
                    break;

                case 0xF0: /* MOVBE Gd, Ed*/
                    nextop = F8;
                    GETGD;
                    GETED(0);
                    if(rex.w)
                        GD->q[0] = __builtin_bswap64(ED->q[0]);
                    else
                        GD->q[0] = __builtin_bswap32(ED->dword[0]);
                    break;
                case 0xF1: /* MOVBE Ed, Gd*/
                    nextop = F8;
                    GETGD;
                    GETED(0);
                    if(rex.w)
                        ED->q[0] = __builtin_bswap64(GD->q[0]);
                    else {
                        if(MODREG)
                            ED->q[0] = __builtin_bswap32(GD->dword[0]);
                        else
                            ED->dword[0] = __builtin_bswap32(GD->dword[0]);
                    }
                    break;

                default:
                    return 0;
            }
            break;

        case 0x3A:
            opcode = F8;
            switch(opcode) {
                case 0xF: /* PALIGNR Gm, Em, Ib */
                    nextop = F8;
                    GETEM(1);
                    GETGM;
                    tmp8u = F8;
                    tmp64u = EM->q;
                    if (tmp8u >= 16) {
                        GM->q = 0;
                    } else if (tmp8u >= 8) {
                        tmp8u -= 8;
                        GM->q >>= tmp8u * 8;
                    } else if (tmp8u == 0) {
                        GM->q = tmp64u;
                    } else {
                        GM->q <<= (8 - tmp8u) * 8;
                        GM->q |= (tmp64u >> tmp8u * 8);
                    }
                    break;

                case 0xCC:  /* SHA1RNDS4 Gx, Ex, Ib */
                    nextop = F8;
                    GETGX;
                    GETEX(1);
                    tmp8u = F8;
                    sha1rnds4(emu, GX, EX, tmp8u);
                    break;

                default:
                    return 0;
            }
            break;


        case 0x3F:
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            #endif
            break;
        GOCOND(0x40
            , nextop = F8;
            GETED(0);
            GETGD;
            CHECK_FLAGS(emu);
            , if(rex.w) {GD->q[0] = ED->q[0]; } else {GD->q[0] = ED->dword[0];}
            , if(!rex.w) GD->dword[1] = 0;
            ,
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign

        case 0x50:                      /* MOVMSKPS Gd, Ex */
            nextop = F8;
            GETEX(0);
            GETGD;
            GD->q[0] = 0;
            for(int i=0; i<4; ++i)
                GD->dword[0] |= ((EX->ud[i]>>31)&1)<<i;
            break;
        case 0x51:                      /* SQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else
                    GX->f[i] = (EX->f[i] < 0) ? (-NAN) : sqrtf(EX->f[i]);
            break;
        case 0x52:                      /* RSQRTPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                if(EX->f[i]==0)
                    GX->f[i] = 1.0f/EX->f[i];
                else if (EX->f[i]<0)
                    GX->f[i] = -NAN;
                else if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else if (isinf(EX->f[i]))
                    GX->f[i] = 0.0;
                else
                    GX->f[i] = 1.0f/sqrtf(EX->f[i]);
            }
            break;
        case 0x53:                      /* RCPPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i)
                if (isnan(EX->f[i]))
                    GX->f[i] = EX->f[i];
                else
                    GX->f[i] = 1.0f / EX->f[i];
            break;
        case 0x54:                      /* ANDPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->u128 &= EX->u128;
            break;
        case 0x55:                      /* ANDNPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            GX->u128 = (~GX->u128) & EX->u128;
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
            for(int i=0; i<4; ++i) {
                maskps[i] = isnanf(GX->f[i]) || isnanf(EX->f[i]);
                GX->f[i] += EX->f[i];
                if(isnanf(GX->f[i]) && !maskps[i]) GX->ud[i] |= 0x80000000;
            }
            break;
        case 0x59:                      /* MULPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                maskps[i] = isnanf(GX->f[i]) || isnanf(EX->f[i]);
                GX->f[i] *= EX->f[i];
                if(isnanf(GX->f[i]) && !maskps[i]) GX->ud[i] |= 0x80000000;
            }
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
            for(int i=0; i<4; ++i) {
                maskps[i] = isnanf(GX->f[i]) || isnanf(EX->f[i]);
                GX->f[i] -= EX->f[i];
                if(isnanf(GX->f[i]) && !maskps[i]) GX->ud[i] |= 0x80000000;
            }
            break;
        case 0x5D:                      /* MINPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX->f[i]) || isnan(EX->f[i]) || islessequal(EX->f[i], GX->f[i]))
                    GX->f[i] = EX->f[i];
            }
            break;
        case 0x5E:                      /* DIVPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                maskps[i] = isnanf(GX->f[i]) || isnanf(EX->f[i]);
                GX->f[i] /= EX->f[i];
                if(isnanf(GX->f[i]) && !maskps[i]) GX->ud[i] |= 0x80000000;
            }
            break;
        case 0x5F:                      /* MAXPS Gx, Ex */
            nextop = F8;
            GETEX(0);
            GETGX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX->f[i]) || isnan(EX->f[i]) || isgreaterequal(EX->f[i], GX->f[i]))
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
        case 0x70:                       /* PSHUFW Gm, Em, Ib */
            nextop = F8;
            GETEM(1);
            GETGM;
            tmp8u = F8;
            if(EM==GM) {eam1 = *GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<4; ++i)
                GM->uw[i] = EM->uw[(tmp8u>>(i*2))&3];
            break;
        case 0x71:  /* GRP */
            nextop = F8;
            GETEM(1);
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLW Em, Ib */
                    if(tmp8u>15)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<4; ++i) EM->uw[i] >>= tmp8u;
                    break;
                case 4:                 /* PSRAW Em, Ib */
                    if(tmp8u>15) tmp8u = 15;
                    for (int i=0; i<4; ++i) EM->sw[i] >>= tmp8u;
                    break;
                case 6:                 /* PSLLW Em, Ib */
                    if(tmp8u>15)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<4; ++i) EM->uw[i] <<= tmp8u;
                    break;
                default:
                    return 0;
            }
            break;
        case 0x72:  /* GRP */
            nextop = F8;
            GETEM(1);
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLD Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>31)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<2; ++i) EM->ud[i] >>= tmp8u;
                    break;
                case 4:                 /* PSRAD Em, Ib */
                    tmp8u = F8;
                    if (tmp8u > 31) tmp8u = 31;
                    for (int i=0; i<2; ++i) EM->sd[i] >>= tmp8u;
                    break;
                case 6:                 /* PSLLD Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>31)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<2; ++i) EM->ud[i] <<= tmp8u;
                    break;
                default:
                    return 0;
            }
            break;
        case 0x73:  /* GRP */
            nextop = F8;
            GETEM(1);
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLQ Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>63)
                        {EM->q = 0;}
                    else
                        {EM->q >>= tmp8u;}
                    break;
                case 6:                 /* PSLLQ Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>63)
                        {EM->q = 0;}
                    else
                        {EM->q <<= tmp8u;}
                    break;
                default:
                    return 0;
            }
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
            else {
                if(MODREG)
                    ED->q[0] = GM->ud[0];
                else
                    ED->dword[0] = GM->ud[0];
            }
            break;
        case 0x7F:                      /* MOVQ Em, Gm */
            nextop = F8;
            GETEM(0);
            GETGM;
            EM->q = GM->q;
            break;
        GOCOND(0x80
            , tmp32s = F32S; CHECK_FLAGS(emu);
            , addr += tmp32s;
            ,,STEP3
        )                               /* 0x80 -> 0x8F Jxx */ //STEP3
        GOCOND(0x90
            , nextop = F8; CHECK_FLAGS(emu);
            GETEB(0);
            , EB->byte[0]=1;
            , EB->byte[0]=0;
            ,
        )                               /* 0x90 -> 0x9F SETxx Eb */
        case 0xA0:                      /* PUSH FS */
            if(rex.is32bits)
                Push32(emu, emu->segs[_FS]);
            else
                Push64(emu, emu->segs[_FS]);
            break;
        case 0xA1:                      /* POP FS */
            if(rex.is32bits)
                emu->segs[_FS] = Pop32(emu);
            else
                emu->segs[_FS] = Pop64(emu);
            emu->segs_serial[_FS] = 0;
            break;
        case 0xA2:                      /* CPUID */
            tmp32u = R_EAX;
            my_cpuid(emu, tmp32u);
            break;
        case 0xA3:                      /* BT Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp64s = rex.w?GD->sq[0]:GD->sdword[0];
            tmp8u=tmp64s&(rex.w?63:31);
            tmp64s >>= (rex.w?6:5);
            if(!MODREG)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+(tmp64s<<(rex.w?3:2)));
                if(rex.w)
                    *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                else
                    *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg64_t*)(((uintptr_t)(ED))+(tmp64s<<(rex.w?3:2)));
                #endif
            }
            if(rex.w) {
                if(ED->q[0] & (1LL<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
            } else {
                if(ED->dword[0] & (1<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
            }
            break;
        case 0xA4:                      /* SHLD Ed,Gd,Ib */
            nextop = F8;
            GETED(1);
            GETGD;
            tmp8u = F8;
            if(rex.w)
                ED->q[0] = shld64(emu, ED->q[0], GD->q[0], tmp8u);
            else {
                if(MODREG)
                    ED->q[0] = shld32(emu, ED->dword[0], GD->dword[0], tmp8u);
                else
                    ED->dword[0] = shld32(emu, ED->dword[0], GD->dword[0], tmp8u);
            }
            break;
        case 0xA5:                      /* SHLD Ed,Gd,CL */
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = R_CL;
            if(rex.w)
                ED->q[0] = shld64(emu, ED->q[0], GD->q[0], tmp8u);
            else {
                if(MODREG)
                    ED->q[0] = shld32(emu, ED->dword[0], GD->dword[0], tmp8u);
                else
                    ED->dword[0] = shld32(emu, ED->dword[0], GD->dword[0], tmp8u);
            }
            break;

        case 0xA8:                      /* PUSH GS */
            if(rex.is32bits)
                Push32(emu, emu->segs[_GS]);
            else
                Push64(emu, emu->segs[_GS]);
            break;
        case 0xA9:                      /* POP GS */
            if(rex.is32bits)
                emu->segs[_GS] = Pop32(emu);
            else
                emu->segs[_GS] = Pop64(emu);
            emu->segs_serial[_FS] = 0;
            break;

        case 0xAB:                      /* BTS Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp64s = rex.w?GD->sq[0]:GD->sdword[0];
            tmp8u=tmp64s&(rex.w?63:31);
            tmp64s >>= (rex.w?6:5);
            if(!MODREG)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+(tmp64s<<(rex.w?3:2)));
                if(rex.w)
                    *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                else
                    *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg64_t*)(((uintptr_t)(ED))+(tmp64s<<(rex.w?3:2)));
                #endif
            }
            if(rex.w) {
                if(ED->q[0] & (1LL<<tmp8u))
                    SET_FLAG(F_CF);
                else {
                    ED->q[0] |= (1LL<<tmp8u);
                    CLEAR_FLAG(F_CF);
                }
            } else {
                if(ED->dword[0] & (1<<tmp8u))
                    SET_FLAG(F_CF);
                else {
                    ED->dword[0] |= (1<<tmp8u);
                    CLEAR_FLAG(F_CF);
                }
                if(MODREG)
                    ED->dword[1] = 0;
            }
            if (BOX64ENV(dynarec_test)) {
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_PF);
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
            else {
                if(MODREG)
                    ED->q[0] = shrd32(emu, ED->dword[0], GD->dword[0], tmp8u);
                else
                    ED->dword[0] = shrd32(emu, ED->dword[0], GD->dword[0], tmp8u);
            }
            break;
        case 0xAE:                      /* Grp Ed (SSE) */
            nextop = F8;
            if(MODREG)
                switch(nextop) {
                    case 0xE8:
                        return addr;            /* LFENCE */
                    case 0xF0:
                        return addr;            /* MFENCE */
                    case 0xF8:
                        return addr;            /* SFENCE */
                    default:
                        return 0;
                }
            else
            switch((nextop>>3)&7) {
                case 0:                 /* FXSAVE Ed */
                    _GETED(0);
                    #ifdef TEST_INTERPRETER
                    emu->sw.f.F87_TOP = emu->top&7;
                    #else
                    if(rex.is32bits)
                        fpu_fxsave32(emu, ED);
                    else
                        fpu_fxsave64(emu, ED);
                    #endif
                    break;
                case 1:                 /* FXRSTOR Ed */
                    _GETED(0);
                    if(rex.is32bits)
                        fpu_fxrstor32(emu, ED);
                    else
                        fpu_fxrstor64(emu, ED);
                    break;
                case 2:                 /* LDMXCSR Md */
                    GETED(0);
                    emu->mxcsr.x32 = ED->dword[0];
                    #ifndef TEST_INTERPRETER
                    if(BOX64ENV(sse_flushto0))
                        applyFlushTo0(emu);
                    #endif
                    break;
                case 3:                 /* STMXCSR Md */
                    GETED(0);
                    ED->dword[0] = emu->mxcsr.x32;
                    break;
                case 4:                 /* XSAVE Ed */
                    _GETED(0);
                    #ifdef TEST_INTERPRETER
                    emu->sw.f.F87_TOP = emu->top&7;
                    #else
                    fpu_xsave(emu, ED, rex.w?0:1);
                    #endif
                    break;
                case 5:                 /* XRSTOR Ed */
                    _GETED(0);
                    fpu_xrstor(emu, ED, rex.w?0:1);
                    break;
                case 7:                 /* CLFLUSH Ed */
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
        case 0xAF:                      /* IMUL Gd,Ed */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                GD->q[0] = imul64(emu, GD->q[0], ED->q[0]);
            else
                GD->q[0] = imul32(emu, GD->dword[0], ED->dword[0]);
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
            tmp64s = rex.w?GD->sq[0]:GD->sdword[0];
            tmp8u=tmp64s&(rex.w?63:31);
            tmp64s >>= (rex.w?6:5);
            if(!MODREG)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+(tmp64s<<(rex.w?3:2)));
                if(rex.w)
                    *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                else
                    *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg64_t*)(((uintptr_t)(ED))+(tmp64s<<(rex.w?3:2)));
                #endif
            }
            if(rex.w) {
                if(ED->q[0] & (1LL<<tmp8u)) {
                    SET_FLAG(F_CF);
                    ED->q[0] ^= (1LL<<tmp8u);
                } else
                    CLEAR_FLAG(F_CF);
            } else {
                if(ED->dword[0] & (1<<tmp8u)) {
                    SET_FLAG(F_CF);
                    ED->dword[0] ^= (1<<tmp8u);
                } else
                    CLEAR_FLAG(F_CF);
                if(MODREG)
                    ED->dword[1] = 0;
            }
            if (BOX64ENV(dynarec_test)) {
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_PF);
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
                        if(MODREG)
                            ED->dword[1] = 0;
                    }
                    break;
                case 6:             /* BTR Ed, Ib */
                    CHECK_FLAGS(emu);
                    GETED(1);
                    tmp8u = F8;
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
                case 7:             /* BTC Ed, Ib */
                    CHECK_FLAGS(emu);
                    GETED(1);
                    tmp8u = F8;
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

                default:
                    return 0;
            }
            break;
        case 0xBB:                      /* BTC Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp64s = rex.w?GD->sq[0]:GD->sdword[0];
            tmp8u=tmp64s&(rex.w?63:31);
            tmp64s >>= (rex.w?6:5);
            if(!MODREG)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+(tmp64s<<(rex.w?3:2)));
                if(rex.w)
                    *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                else
                    *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg64_t*)(((uintptr_t)(ED))+(tmp64s<<(rex.w?3:2)));
                #endif
            }
            if(rex.w) {
                if(ED->q[0] & (1LL<<tmp8u))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
                ED->q[0] ^= (1LL<<tmp8u);
            } else {
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
            RESET_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = 0;
            if(rex.w) {
                tmp64u = ED->q[0];
                if(tmp64u) {
                    CLEAR_FLAG(F_ZF);
                    while(!(tmp64u&(1LL<<tmp8u))) ++tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
                if(tmp64u || !MODREG)
                    GD->q[0] = tmp8u;
            } else {
                tmp32u = ED->dword[0];
                if(tmp32u) {
                    CLEAR_FLAG(F_ZF);
                    while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
                if(tmp32u || !MODREG)
                    GD->q[0] = tmp8u;
            }
            if(!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG(PARITY(tmp8u), F_PF);
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_OF);
            }
            break;
        case 0xBD:                      /* BSR Ed,Gd */
            RESET_FLAGS(emu);
            nextop = F8;
            GETED(0);
            GETGD;
            tmp8u = 0;
            if(rex.w) {
                tmp64u = ED->q[0];
                if(tmp64u) {
                    CLEAR_FLAG(F_ZF);
                    tmp8u = 63;
                    while(!(tmp64u&(1LL<<tmp8u))) --tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                }
                if(tmp64u || !MODREG)
                    GD->q[0] = tmp8u;
            } else {
                tmp32u = ED->dword[0];
                if(tmp32u) {
                    CLEAR_FLAG(F_ZF);
                    tmp8u = 31;
                    while(!(tmp32u&(1<<tmp8u))) --tmp8u;
                    GD->q[0] = tmp8u;
                } else {
                    SET_FLAG(F_ZF);
                    if(!MODREG)
                        GD->q[0] = tmp8u;
                }
            }
            if(!BOX64ENV(cputype)) {
                CONDITIONAL_SET_FLAG(PARITY(tmp8u), F_PF);
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_OF);
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
        case 0xC3:                      /* MOVNTI Ed, Gd */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                ED->q[0] = GD->q[0];
            else {
                if(MODREG)
                    ED->q[0] = GD->dword[0];
                else
                    ED->dword[0] = GD->dword[0];
            }
            break;
        case 0xC4:                      /* PINSRW Gm,Ew,Ib */
            nextop = F8;
            GETED(1);
            GETGM;
            tmp8u = F8;
            GM->uw[tmp8u&3] = ED->word[0];   // only low 16bits
            break;
        case 0xC5:                       /* PEXTRW Gw,Em,Ib */
            nextop = F8;
            GETEM(1);
            GETGD;
            tmp8u = F8;
            GD->q[0] = EM->uw[tmp8u&3];  // 16bits extract, 0 extended
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
        case 0xC7:
            CHECK_FLAGS(emu);
            nextop = F8;
            GETE8xw(0);
            switch((nextop>>3)&7) {
                case 1:     /* CMPXCHG8B Eq */
                    if(rex.w) {
                        tmp64u = ED->q[0];
                        tmp64u2= ED->q[1];
                        if(R_RAX == tmp64u && R_RDX == tmp64u2) {
                            SET_FLAG(F_ZF);
                            ED->q[0] = R_RBX;
                            ED->q[1] = R_RCX;
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
                case 6:     /* RDRAND Ed */
                    RESET_FLAGS(emu);
                    CLEAR_FLAG(F_OF);
                    CLEAR_FLAG(F_SF);
                    CLEAR_FLAG(F_PF);
                    CLEAR_FLAG(F_ZF);
                    CLEAR_FLAG(F_AF);
                    SET_FLAG(F_CF);
                    if(rex.w)
                        ED->q[0] = get_random64();
                    else {
                        ED->dword[0] = get_random32();
                        if(MODREG)
                            ED->dword[1] = 1;
                    }
                    break;
                case 7:     /* RDPID Ed */
                    ED->q[0] = helper_getcpu(emu);
                    break;
                default:
                    return 0;
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
                GM->q = 0;
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
        case 0xD4:                   /* PADDQ Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->sq += EM->sq;
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

        case 0xD7:                   /* PMOVMSKB Gd,Em */
            nextop = F8;
            GETEM(0);
            GETGD;
            GD->q[0] = 0;
            for (int i=0; i<8; ++i)
                if(EM->ub[i]&0x80)
                    GD->dword[0] |= (1<<i);
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
        case 0xDA:                   /* PMINUB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i=0; i<8; ++i)
                GM->ub[i] = (GM->ub[i]<EM->ub[i])?GM->ub[i]:EM->ub[i];
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
        case 0xDE:                   /* PMAXUB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i=0; i<8; ++i)
                GM->ub[i] = (GM->ub[i]>EM->ub[i])?GM->ub[i]:EM->ub[i];
            break;
        case 0xDF:                   /* PANDN Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = (~GM->q) & EM->q;
            break;
        case 0xE0:                   /* PAVGB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i)
                    GM->ub[i] = ((uint32_t)GM->ub[i]+EM->ub[i]+1)>>1;
            break;
        case 0xE1:                   /* PSRAW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>15)
                tmp8u = 15;
            else
                tmp8u = EM->ub[0];
            for(int i=0; i<4; ++i)
                GM->sw[i] >>= tmp8u;
            break;
        case 0xE2:                   /* PSRAD Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            if(EM->q>31)
                tmp8u = 31;
            else
                tmp8u = EM->ub[0];
            for(int i=0; i<2; ++i)
                GM->sd[i] >>= tmp8u;
            break;
        case 0xE3:                   /* PAVGW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->uw[i] = ((uint32_t)GM->uw[i]+EM->uw[i]+1)>>1;
            break;
        case 0xE4:                   /* PMULHUW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32u = (uint32_t)GM->uw[i] * (uint32_t)EM->uw[i];
                GM->uw[i] = (tmp32u>>16)&0xffff;
            }
            break;
        case 0xE5:                   /* PMULHW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM->sw[i] * (int32_t)EM->sw[i];
                GM->sw[i] = (tmp32s>>16)&0xffff;
            }
            break;
        case 0xE7:                   /* MOVNTQ Em,Gm */
            nextop = F8;
            if(MODREG)
                return 0;
            GETEM(0);
            GETGM;
            EM->q = GM->q;
            break;
        case 0xE8:                   /* PSUBSB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM->sb[i] - (int32_t)EM->sb[i];
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
        case 0xEA:                  /* PMINSW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i=0; i<4; ++i)
                GM->sw[i] = (GM->sw[i]<EM->sw[i])?GM->sw[i]:EM->sw[i];
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
        case 0xEE:                  /* PMAXSW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i=0; i<4; ++i)
                GM->sw[i] = (GM->sw[i]>EM->sw[i])?GM->sw[i]:EM->sw[i];
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
                    GM->uw[i] <<= tmp8u;
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
                    GM->ud[i] <<= tmp8u;
            }
            break;
        case 0xF3:                   /* PSLLQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = (EM->q > 63) ? 0L : (GM->q << EM->ub[0]);
            break;
        case 0xF4:                   /* PMULUDQ Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->q = (uint64_t)GM->ud[0] * (uint64_t)EM->ud[0];
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
        case 0xF6:                   /* PSADBW Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            tmp32u = 0;
            for (int i=0; i<8; ++i)
                tmp32u += (GM->ub[i]>EM->ub[i])?(GM->ub[i] - EM->ub[i]):(EM->ub[i] - GM->ub[i]);
            GM->q = tmp32u;
            break;
        case 0xF7:                   /* MASKMOVQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for (int i = 0; i < 8; i++) {
                if (EM->ub[i] & 0x80) {
                   ((reg64_t*)(emu->regs[_DI].q[0]))->byte[i] = GM->ub[i];
                }
            }
            break;
        case 0xF8:                   /* PSUBB Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i)
                GM->ub[i] -= EM->ub[i];
            break;
        case 0xF9:                   /* PSUBW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->uw[i] -= EM->uw[i];
            break;
        case 0xFA:                   /* PSUBD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<2; ++i)
                GM->ud[i] -= EM->ud[i];
            break;
        case 0xFB:                   /* PSUBQ Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            GM->sq -= EM->sq;
            break;
        case 0xFC:                   /* PADDB Gm, Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<8; ++i)
                GM->ub[i] += EM->ub[i];
            break;
        case 0xFD:                   /* PADDW Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<4; ++i)
                GM->uw[i] += EM->uw[i];
            break;
        case 0xFE:                   /* PADDD Gm,Em */
            nextop = F8;
            GETEM(0);
            GETGM;
            for(int i=0; i<2; ++i)
                GM->ud[i] += EM->ud[i];
            break;

        default:
            return 0;
    }
    return addr;
}
