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

#ifdef TEST_INTERPRETER
uintptr_t Test64(x64test_t *test, rex_t rex, int seg, uintptr_t addr)
#else
uintptr_t Run64(x64emu_t *emu, rex_t rex, int seg, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;   (void)tmp8u;
    int16_t tmp16s;  (void)tmp16s;
    uint16_t tmp16u; (void)tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;
    int rep;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    uintptr_t tlsdata = GetSegmentBaseEmu(emu, seg);

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    if(!rex.is32bits)
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }
    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }
    if(!rex.is32bits)
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }

    switch(opcode) {
        #define GO(B, OP)                                   \
        case B+0:                                           \
            nextop = F8;                                    \
            GETEB_OFFS(0, tlsdata);                         \
            GETGB;                                          \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);      \
            break;                                          \
        case B+1:                                           \
            nextop = F8;                                    \
            GETED_OFFS(0, tlsdata);                         \
            GETGD;                                          \
            if(rex.w)                                       \
                ED->q[0] = OP##64(emu, ED->q[0], GD->q[0]); \
            else {                                          \
                if(MODREG)                                  \
                    ED->q[0] = OP##32(emu, ED->dword[0], GD->dword[0]);     \
                else                                                        \
                    ED->dword[0] = OP##32(emu, ED->dword[0], GD->dword[0]); \
            }                                               \
            break;                                          \
        case B+2:                                           \
            nextop = F8;                                    \
            GETEB_OFFS(0, tlsdata);                         \
            GETGB;                                          \
            GB = OP##8(emu, GB, EB->byte[0]);               \
            break;                                          \
        case B+3:                                           \
            nextop = F8;                                    \
            GETED_OFFS(0, tlsdata);                         \
            GETGD;                                          \
            if(rex.w)                                       \
                GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]); \
            else                                            \
                GD->q[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
            break;                                          \
        case B+4:                                           \
            R_AL = OP##8(emu, R_AL, F8);                    \
            break;                                          \
        case B+5:                                           \
            if(rex.w)                                       \
                R_RAX = OP##64(emu, R_RAX, F32S64);         \
            else                                            \
                R_RAX = OP##32(emu, R_EAX, F32);            \
            break;

        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO
        case 0x0F:
            opcode = F8;
            switch(opcode) {

                case 0x10:
                    switch(rep) {
                        case 0: /* MOVUPS Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->u128 = EX->u128;
                            break;
                        case 1: /* MOVSD Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->q[0] = EX->q[0];
                            if(!MODREG) {
                                // EX is not a register
                                GX->q[1] = 0;
                            }
                            break;
                        case 2: /* MOVSS Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->ud[0] = EX->ud[0];
                            if(!MODREG) {
                                // EX is not a register (reg to reg only move 31:0)
                                GX->ud[1] = GX->ud[2] = GX->ud[3] = 0;
                            }
                            break;
                        default:
                            return 0;
                    }
                    break;
                case 0x11:
                    switch(rep) {
                        case 0: /* MOVUPS FS:Ex, Gx */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->u128 = GX->u128;
                            break;
                        case 1: /* MOVSD Ex, Gx */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->q[0] = GX->q[0];
                            break;
                        case 2: /* MOVSS FS:Ex, Gx */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->ud[0] = GX->ud[0];
                            break;
                        default:
                            return 0;
                    }
                    break;
                case 0x28:
                    switch(rep) {
                        case 0: /* MOVAPS Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->u128 = EX->u128;
                            break;
                        default:
                            return 0;
                    }
                    break;
                case 0x29:                      /* MOVAPS FS:Ex,Gx */
                    switch(rep) {
                        case 0:
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->q[0] = GX->q[0];
                            EX->q[1] = GX->q[1];
                            break;
                        default:
                            return 0;
                    }
                    break;

                case 0x59:
                    switch(rep) {
                        case 2: /* MULSS Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->f[0] *= EX->f[0];
                            break;

                        default:
                            return 0;
                    }
                    break;
                case 0x5A:
                    switch(rep) {
                        case 2:  /* CVTSS2SD Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->d[0] = EX->f[0];
                            break;

                        default:
                            return 0;
                    }
                    break;

                case 0x6F:
                    switch(rep) {
                        case 2: /* MOVDQU Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            memcpy(GX, EX, 16);    // unaligned...
                            break;
                            
                        default:
                            return 0;
                    }
                    break;

                case 0xAF:
                    switch(rep) {
                        case 0: /* IMUL Gd, FS:Ed */
                            nextop = F8;
                            GETED_OFFS(0, tlsdata);
                            GETGD;
                            if(rex.w)
                                GD->q[0] = imul64(emu, GD->q[0], ED->q[0]);
                            else
                                GD->q[0] = imul32(emu, GD->dword[0], ED->dword[0]);
                            break;
                        default:
                            return 0;
                    }
                    break;

                case 0xB6:
                    switch(rep) {
                        case 0: /* MOVZX Gd, FS:Eb */
                            nextop = F8;
                            GETEB_OFFS(0, tlsdata);
                            GETGD;
                            GD->q[0] = EB->byte[0];
                            break;
                        default:
                            return 0;
                    }
                    break;
                case 0xB7:
                    switch(rep) {
                        case 0: /* MOVZX Gd, FS:Ew */
                            nextop = F8;
                            GETEW_OFFS(0, tlsdata);
                            GETGD;
                            GD->q[0] = EW->word[0];
                            break;
                        default:
                            return 0;
                    }
                    break;

                case 0xBA:                      
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 4:                 /* BT Ed,Ib */
                            CHECK_FLAGS(emu);
                            GETED_OFFS(1, tlsdata);
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
                            GETED_OFFS(1, tlsdata);
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
                            GETED_OFFS(1, tlsdata);
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
                            GETED_OFFS(1, tlsdata);
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

                default:
                    return 0;
            }
            break;

        case 0x38:
            nextop = F8;
            GETEB_OFFS(0, tlsdata);
            GETGB;
            cmp8(emu, EB->byte[0], GB);
            break;
        case 0x39:
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                cmp64(emu, ED->q[0], GD->q[0]);
            else
                cmp32(emu, ED->dword[0], GD->dword[0]);
            break;

        case 0x3B:
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                cmp64(emu, GD->q[0], ED->q[0]);
            else
                cmp32(emu, GD->dword[0], ED->dword[0]);
            break;


        case 0x63:                      /* MOVSXD Gd, FS:Ed */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.is32bits) {
                // ARPL here
                // faking to always happy...
                SET_FLAG(F_ZF);
            } else {
                if(rex.w)
                    GD->sq[0] = ED->sdword[0];
                else
                    if(MODREG)
                        GD->q[0] = ED->dword[0];    // not really a sign extension
                    else
                        GD->sdword[0] = ED->sdword[0];  // meh?
            }
            break;
        case 0x64:                      /* FS: prefix */
            #ifdef TEST_INTERPRETER
            return Test64(test, rex, _FS, addr);
            #else
            return Run64(emu, rex, _FS, addr);
            #endif
            break;
        case 0x65:                      /* GS: prefix */
            #ifdef TEST_INTERPRETER
            return Test64(test, rex, _GS, addr);
            #else
            return Run64(emu, rex, _GS, addr);
            #endif
            break;
        case 0x66:
            return Run6664(emu, rex, seg, addr);
        case 0x67:
            if(rex.is32bits)
                return Run6764_32(emu, rex, seg, seg, addr);
            else
                return 0;

        case 0x80:                      /* GRP Eb,Ib */
            nextop = F8;
            GETEB_OFFS(1, tlsdata);
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
                case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
                case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0x81:                      /* GRP Ed,Id */
        case 0x83:                      /* GRP Ed,Ib */
            nextop = F8;
            GETED_OFFS((opcode==0x81)?4:1, tlsdata);
            if(opcode==0x81) {
                tmp32s = F32S;
            } else {
                tmp32s = F8S;
            }
            if(rex.w) {
                tmp64u = (uint64_t)(int64_t)tmp32s;
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = add64(emu, ED->q[0], tmp64u); break;
                    case 1: ED->q[0] =  or64(emu, ED->q[0], tmp64u); break;
                    case 2: ED->q[0] = adc64(emu, ED->q[0], tmp64u); break;
                    case 3: ED->q[0] = sbb64(emu, ED->q[0], tmp64u); break;
                    case 4: ED->q[0] = and64(emu, ED->q[0], tmp64u); break;
                    case 5: ED->q[0] = sub64(emu, ED->q[0], tmp64u); break;
                    case 6: ED->q[0] = xor64(emu, ED->q[0], tmp64u); break;
                    case 7:            cmp64(emu, ED->q[0], tmp64u); break;
                }
            } else {
                tmp32u = (uint32_t)tmp32s;
                if(MODREG)
                    switch((nextop>>3)&7) {
                        case 0: ED->q[0] = add32(emu, ED->dword[0], tmp32u); break;
                        case 1: ED->q[0] =  or32(emu, ED->dword[0], tmp32u); break;
                        case 2: ED->q[0] = adc32(emu, ED->dword[0], tmp32u); break;
                        case 3: ED->q[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                        case 4: ED->q[0] = and32(emu, ED->dword[0], tmp32u); break;
                        case 5: ED->q[0] = sub32(emu, ED->dword[0], tmp32u); break;
                        case 6: ED->q[0] = xor32(emu, ED->dword[0], tmp32u); break;
                        case 7:            cmp32(emu, ED->dword[0], tmp32u); break;
                    }
                else
                    switch((nextop>>3)&7) {
                        case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                        case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                        case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                        case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                        case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                        case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                        case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                        case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
                    }
            }
            break;

        case 0x86:                      /* XCHG Eb,Gb */
            nextop = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            GETEB_OFFS(0, tlsdata);
            GETGB;
            if(MODREG) { // reg / reg: no lock
                tmp8u = GB;
                GB = EB->byte[0];
                EB->byte[0] = tmp8u;
            } else {
                GB = native_lock_xchg_b(EB, GB);
            }
            // dynarec use need it's own mecanism
#else
            GETEB_OFFS(0, tlsdata);
            GETGB;
            if(!MODREG)
                pthread_mutex_lock(&my_context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp8u = GB;
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            if(!MODREG)
                pthread_mutex_unlock(&my_context->mutex_lock);
#endif                
            break;

        case 0x88:                      /* MOV FS:Eb,Gb */
            nextop = F8;
            GETEB_OFFS(0, tlsdata);
            GETGB;
            EB->byte[0] = GB;
            break;
        case 0x89:                    /* MOV FS:Ed,Gd */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w) {
                ED->q[0] = GD->q[0];
            } else {
                if(MODREG)
                    ED->q[0] = GD->dword[0];
                else
                    ED->dword[0] = GD->dword[0];
            }
            break;
        case 0x8A:                      /* MOV Gb, FS:Eb */
            nextop = F8;
            GETEB_OFFS(0, tlsdata);
            GETGB;
            GB = EB->byte[0];
            break;
        case 0x8B:                      /* MOV Gd, FS:Ed */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                GD->q[0] = ED->q[0];
            else
                GD->q[0] = ED->dword[0];
            break;

        case 0x8D:                      /* LEA Gd,M */
            nextop = F8;
            GETGD;
            tmp64u = GETEA(0);
            if(rex.w)
                GD->q[0] = tmp64u;
            else
                GD->q[0] = tmp64u&0xffffffff;
            break;
        case 0x8E:                      /* MOV Seg, Seg:Ew */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            emu->segs[((nextop&0x38)>>3)] = ED->word[0];
            emu->segs_serial[((nextop&0x38)>>3)] = 0;
            break;
        case 0x8F:                      /* POP FS:Ed */
            nextop = F8;
            if(MODREG) {
                emu->regs[(nextop&7)+(rex.b<<3)].q[0] = Pop64(emu);
            } else {
                if(rex.is32bits) {
                    tmp32u = Pop32(emu);  // this order allows handling POP [ESP] and variant
                    GETED_OFFS(0, tlsdata);
                    R_ESP -= 4; // to prevent issue with SEGFAULT
                    ED->dword[0] = tmp32u;
                    R_ESP += 4;
                } else {
                    tmp64u = Pop64(emu);  // this order allows handling POP [ESP] and variant
                    GETED_OFFS(0, tlsdata);
                    R_RSP -= sizeof(void*); // to prevent issue with SEGFAULT
                    ED->q[0] = tmp64u;
                    R_RSP += sizeof(void*);
                }
            }
            break;
        case 0x90:                      /* NOP */
            break;

        case 0xA1:                      /* MOV EAX,FS:Od */
            if(rex.is32bits) {
                tmp32s = F32S;
                R_EAX = *(uint32_t*)(tlsdata+tmp32s);
            } else {
                tmp64u = F64;
                if(rex.w)
                    R_RAX = *(uint64_t*)(tlsdata+tmp64u);
                else
                    R_RAX = *(uint32_t*)(tlsdata+tmp64u);
            }
            break;

        case 0xA3:                      /* MOV FS:Od,EAX */
            if(rex.is32bits) {
                tmp32s = F32S;
                *(uint32_t*)(uintptr_t)(tlsdata+tmp32s) = R_EAX;
            } else {
                tmp64u = F64;
                if(rex.w)
                    *(uint64_t*)(tlsdata+tmp64u) = R_RAX;
                else
                    *(uint32_t*)(tlsdata+tmp64u) = R_EAX;
            }
            break;

        case 0xC6:                      /* MOV FS:Eb, Ib */
            nextop = F8;
            GETEB_OFFS(1, tlsdata);
            EB->byte[0] = F8;
            break;
        case 0xC7:                      /* MOV FS:Ed, Id */
            nextop = F8;
            GETED_OFFS(4, tlsdata);
            if(rex.w)
                ED->q[0] = F32S64;
            else
                if(MODREG)
                    ED->q[0] = F32;
                else
                    ED->dword[0] = F32;
            break;

        case 0xD1:                      /* GRP2 Ed,1 */
        case 0xD3:                      /* GRP2 Ed,CL */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            tmp8u = (opcode==0xD1)?1:R_CL;
            if(rex.w) {
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = rol64(emu, ED->q[0], tmp8u); break;
                    case 1: ED->q[0] = ror64(emu, ED->q[0], tmp8u); break;
                    case 2: ED->q[0] = rcl64(emu, ED->q[0], tmp8u); break;
                    case 3: ED->q[0] = rcr64(emu, ED->q[0], tmp8u); break;
                    case 4: 
                    case 6: ED->q[0] = shl64(emu, ED->q[0], tmp8u); break;
                    case 5: ED->q[0] = shr64(emu, ED->q[0], tmp8u); break;
                    case 7: ED->q[0] = sar64(emu, ED->q[0], tmp8u); break;
                }
            } else {
                if(MODREG)
                    switch((nextop>>3)&7) {
                        case 0: ED->q[0] = rol32(emu, ED->dword[0], tmp8u); break;
                        case 1: ED->q[0] = ror32(emu, ED->dword[0], tmp8u); break;
                        case 2: ED->q[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                        case 3: ED->q[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                        case 4: 
                        case 6: ED->q[0] = shl32(emu, ED->dword[0], tmp8u); break;
                        case 5: ED->q[0] = shr32(emu, ED->dword[0], tmp8u); break;
                        case 7: ED->q[0] = sar32(emu, ED->dword[0], tmp8u); break;
                    }
                else
                    switch((nextop>>3)&7) {
                        case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                        case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                        case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                        case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                        case 4: 
                        case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                        case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                        case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
                    }
            }
            break;

        case 0xEB:                      /* JMP Ib */
            tmp32s = F8S; // jump is relative
            addr += tmp32s;
            break;

        case 0xF7:                      /* GRP3 Ed(,Id) */
            nextop = F8;
            tmp8u = (nextop>>3)&7;
            GETED_OFFS((tmp8u<2)?4:0, tlsdata);
            if(rex.w) {
                switch(tmp8u) {
                    case 0: 
                    case 1:                 /* TEST Ed,Id */
                        tmp64u = F32S64;
                        test64(emu, ED->q[0], tmp64u);
                        break;
                    case 2:                 /* NOT Ed */
                        ED->q[0] = not64(emu, ED->q[0]);
                        break;
                    case 3:                 /* NEG Ed */
                        ED->q[0] = neg64(emu, ED->q[0]);
                        break;
                    case 4:                 /* MUL RAX,Ed */
                        mul64_rax(emu, ED->q[0]);
                        break;
                    case 5:                 /* IMUL RAX,Ed */
                        imul64_rax(emu, ED->q[0]);
                        break;
                    case 6:                 /* DIV Ed */
                        div64(emu, ED->q[0]);
                        break;
                    case 7:                 /* IDIV Ed */
                        idiv64(emu, ED->q[0]);
                        #ifdef TEST_INTERPRETER
                        test->notest = 1;
                        #endif
                        break;
                }
            } else {
                switch(tmp8u) {
                    case 0: 
                    case 1:                 /* TEST Ed,Id */
                        tmp32u = F32;
                        test32(emu, ED->dword[0], tmp32u);
                        break;
                    case 2:                 /* NOT Ed */
                        if(MODREG)
                            ED->q[0] = not32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = not32(emu, ED->dword[0]);
                        break;
                    case 3:                 /* NEG Ed */
                        if(MODREG)
                            ED->q[0] = neg32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = neg32(emu, ED->dword[0]);
                        break;
                    case 4:                 /* MUL EAX,Ed */
                        mul32_eax(emu, ED->dword[0]);
                        emu->regs[_AX].dword[1] = 0;
                        break;
                    case 5:                 /* IMUL EAX,Ed */
                        imul32_eax(emu, ED->dword[0]);
                        emu->regs[_AX].dword[1] = 0;
                        break;
                    case 6:                 /* DIV Ed */
                        div32(emu, ED->dword[0]);
                        //emu->regs[_AX].dword[1] = 0;  // already put high regs to 0
                        //emu->regs[_DX].dword[1] = 0;
                        break;
                    case 7:                 /* IDIV Ed */
                        idiv32(emu, ED->dword[0]);
                        //emu->regs[_AX].dword[1] = 0;
                        //emu->regs[_DX].dword[1] = 0;
                        break;
                }
            }
            break;
            
        case 0xFF:                      /* GRP 5 Ed */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    if(rex.w)
                        ED->q[0] = inc64(emu, ED->q[0]);
                    else {
                        if(MODREG)
                            ED->q[0] = inc32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = inc32(emu, ED->dword[0]);
                    }
                    break;
                case 1:                 /* DEC Ed */
                    if(rex.w)
                        ED->q[0] = dec64(emu, ED->q[0]);
                    else {
                        if(MODREG)
                            ED->q[0] = dec32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = dec32(emu, ED->dword[0]);
                    }
                    break;
                case 2:                 /* CALL NEAR Ed */
                    if(rex.is32bits) {
                        tmp64u = (uintptr_t)getAlternate((void*)(uintptr_t)ED->dword[0]);
                        Push32(emu, addr);
                    } else {
                        tmp64u = (uintptr_t)getAlternate((void*)ED->q[0]);
                        Push64(emu, addr);
                    }
                    addr = tmp64u;
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(MODREG) {
                        printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    } else {
                        if(rex.is32bits || !rex.w) {
                            Push32(emu, R_CS);
                            Push32(emu, addr);
                            R_RIP = addr = ED->dword[0];
                            R_CS = ED->word[2];
                        } else {
                            Push64(emu, R_CS);
                            Push64(emu, addr);
                            R_RIP = addr = ED->q[0];
                            R_CS = (ED+1)->word[0];
                        }
                        return 0;  // exit loop to recompute new CS...
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    if(rex.is32bits)
                        addr = (uintptr_t)getAlternate((void*)(uintptr_t)ED->dword[0]);
                    else
                        addr = (uintptr_t)getAlternate((void*)ED->q[0]);
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(MODREG) {
                        printf_log(LOG_NONE, "Illegal Opcode %p: 0x%02X 0x%02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    } else {
                        if(rex.is32bits || !rex.w) {
                            R_RIP = addr = ED->dword[0];
                            R_CS = ED->word[2];
                        } else {
                            R_RIP = addr = ED->q[0];
                            R_CS = (ED+1)->word[0];
                        }
                    }
                    break;
                case 6:                 /* Push Ed */
                    if(rex.is32bits) {
                        tmp32u = ED->dword[0];
                        Push32(emu, tmp32u);  // avoid potential issue with push [esp+...]
                    } else {
                        tmp64u = ED->q[0];  // rex.w ignored
                        Push64(emu, tmp64u);  // avoid potential issue with push [esp+...]
                    }
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X %02X %02X\n",(void*)R_RIP, opcode, nextop, PK(2), PK(3), PK(4), PK(5));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    return 0;
            }
            break;
        default:
            return 0;
    }
    return addr;
}
