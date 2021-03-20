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
#ifdef DYNAREC
#include "../dynarec/arm64_lock.h"
#endif

#include "modrm.h"

int RunF0(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int32_t tmp32s;
    uint32_t tmp32u, tmp32u2;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {
#ifdef DYNAREC
        #define GO(B, OP)                                           \
        case B+0:                                                   \
            nextop = F8;                                            \
            GETEB(0);                                               \
            GETGB;                                                  \
            do {                                                    \
                tmp8u = arm64_lock_read_b(EB);                      \
                tmp8u = OP##8(emu, tmp8u, GB);                      \
            } while (arm64_lock_write_b(EB, tmp8u));                \
            break;                                                  \
        case B+1:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            if(rex.w) {                                             \
                do {                                                \
                    tmp64u = arm64_lock_read_dd(ED);                \
                    tmp64u = OP##64(emu, tmp64u, GD->q[0]);         \
                } while (arm64_lock_write_dd(ED, tmp64u));          \
            } else {                                                \
                do {                                                \
                    tmp32u = arm64_lock_read_d(ED);                 \
                    tmp32u = OP##32(emu, tmp32u, GD->dword[0]);     \
                } while (arm64_lock_write_d(ED, tmp32u));           \
                if(MODREG)                                          \
                    ED->dword[1] = 0;                               \
            }                                                       \
            break;                                                  \
        case B+2:                                                   \
            nextop = F8;                                            \
            GETEB(0);                                               \
            GETGB;                                                  \
            GB = OP##8(emu, GB, EB->byte[0]);                       \
            break;                                                  \
        case B+3:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            if(rex.w)                                               \
                GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]);         \
            else                                                    \
                GD->q[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
            break;                                                  \
        case B+4:                                                   \
            R_AL = OP##8(emu, R_AL, F8);                            \
            break;                                                  \
        case B+5:                                                   \
            if(rex.w)                                               \
                R_RAX = OP##64(emu, R_RAX, F32S64);                 \
            else                                                    \
                R_RAX = OP##32(emu, R_EAX, F32);                    \
            break;
#else
        #define GO(B, OP)                                           \
        case B+0:                                                   \
            nextop = F8;                                            \
            GETEB(0);                                               \
            GETGB;                                                  \
            pthread_mutex_lock(&emu->context->mutex_lock);          \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);              \
            pthread_mutex_unlock(&emu->context->mutex_lock);        \
            break;                                                  \
        case B+1:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            pthread_mutex_lock(&emu->context->mutex_lock);          \
            if(rex.w)                                               \
                ED->q[0] = OP##64(emu, ED->q[0], GD->q[0]);         \
            else                                                    \
                if(MODREG)                                          \
                    ED->q[0] = OP##32(emu, ED->dword[0], GD->dword[0]);     \
                else                                                        \
                    ED->dword[0] = OP##32(emu, ED->dword[0], GD->dword[0]); \
            pthread_mutex_unlock(&emu->context->mutex_lock);        \
            break;                                                  \
        case B+2:                                                   \
            nextop = F8;                                            \
            GETEB(0);                                               \
            GETGB;                                                  \
            pthread_mutex_lock(&emu->context->mutex_lock);          \
            GB = OP##8(emu, GB, EB->byte[0]);                       \
            pthread_mutex_unlock(&emu->context->mutex_lock);        \
            break;                                                  \
        case B+3:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            pthread_mutex_lock(&emu->context->mutex_lock);          \
            if(rex.w)                                               \
                GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]);         \
            else                                                    \
                GD->q[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
            pthread_mutex_unlock(&emu->context->mutex_lock);        \
            break;                                                  \
        case B+4:                                                   \
            pthread_mutex_lock(&emu->context->mutex_lock);          \
            R_AL = OP##8(emu, R_AL, F8);                            \
            pthread_mutex_unlock(&emu->context->mutex_lock);        \
            break;                                                  \
        case B+5:                                                   \
            pthread_mutex_lock(&emu->context->mutex_lock);          \
            if(rex.w)                                               \
                R_RAX = OP##64(emu, R_RAX, F32S64);                 \
            else                                                    \
                R_RAX = OP##32(emu, R_EAX, F32);                    \
            pthread_mutex_unlock(&emu->context->mutex_lock);        \
            break;
#endif
        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO
        case 0x0f:
            opcode = F8;
            switch (opcode) { 

                case 0xB1:                      /* CMPXCHG Ed,Gd */
                    nextop = F8;
                    GETED(0);
                    GETGD;
#ifdef DYNAREC
                    if(rex.w)
                        do {
                            tmp64u = arm64_lock_read_dd(ED);
                            cmp64(emu, R_RAX, tmp64u);
                            if(ACCESS_FLAG(F_ZF)) {
                                tmp32s = arm64_lock_write_dd(ED, GD->q[0]);
                            } else {
                                R_RAX = tmp64u;
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                    else
                        do {
                            tmp32u = arm64_lock_read_d(ED);
                            cmp32(emu, R_EAX, tmp32u);
                            if(ACCESS_FLAG(F_ZF)) {
                                tmp32s = arm64_lock_write_d(ED, GD->dword[0]);
                            } else {
                                R_RAX = tmp32u;
                                tmp32s = 0;
                            }
                        } while(tmp32s);
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
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
                            ED->dword[0] = GD->dword[0];
                        } else {
                            R_EAX = ED->dword[0];
                        }
                    }
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;

                case 0xC1:                      /* XADD Gd,Ed */
                    nextop = F8;
                    GETED(0);
                    GETGD;
#ifdef DYNAREC
                    if(rex.w) {
                        do {
                            tmp64u = arm64_lock_read_dd(ED);
                            tmp64u2 = add64(emu, tmp64u, GD->dword[0]);
                        } while(arm64_lock_write_dd(ED, tmp64u2));
                        GD->q[0] = tmp64u;
                    } else {
                        do {
                            tmp32u = arm64_lock_read_d(ED);
                            tmp32u2 = add32(emu, tmp32u, GD->dword[0]);
                        } while(arm64_lock_write_d(ED, tmp32u2));
                        GD->q[0] = tmp32u;
                        if(MODREG)
                            ED->dword[1] = 0;
                    }
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
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
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;

            default:
                return 1;
            }
            break;

        case 0x81:              /* GRP Ed,Id */
        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GETED((opcode==0x81)?4:1);
            if(opcode==0x83) {
                tmp64s = F8S;
                tmp64u = (uint64_t)tmp64s;
            } else
                tmp64u = F32S64;
#ifdef DYNAREC
            if(rex.w) {
                switch((nextop>>3)&7) {
                    case 0: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 = add64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 1: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 =  or64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 2: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 = adc64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 3: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 = sbb64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 4: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 = and64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 5: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 = sub64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 6: do { tmp64u2 = arm64_lock_read_dd(ED); tmp64u2 = xor64(emu, tmp64u2, tmp64u);} while(arm64_lock_write_dd(ED, tmp64u2)); break;
                    case 7:                cmp64(emu, ED->q[0], tmp64u); break;
                }
            } else {
                if(MODREG)
                    switch((nextop>>3)&7) {
                        case 0: ED->q[0] = add32(emu, ED->dword[0], tmp64u); break;
                        case 1: ED->q[0] =  or32(emu, ED->dword[0], tmp64u); break;
                        case 2: ED->q[0] = adc32(emu, ED->dword[0], tmp64u); break;
                        case 3: ED->q[0] = sbb32(emu, ED->dword[0], tmp64u); break;
                        case 4: ED->q[0] = and32(emu, ED->dword[0], tmp64u); break;
                        case 5: ED->q[0] = sub32(emu, ED->dword[0], tmp64u); break;
                        case 6: ED->q[0] = xor32(emu, ED->dword[0], tmp64u); break;
                        case 7:            cmp32(emu, ED->dword[0], tmp64u); break;
                    }
                else
                    switch((nextop>>3)&7) {
                        case 0: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 = add32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 1: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 =  or32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 2: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 = adc32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 3: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 = sbb32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 4: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 = and32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 5: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 = sub32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 6: do { tmp32u2 = arm64_lock_read_d(ED); tmp32u2 = xor32(emu, tmp32u2, tmp64u);} while(arm64_lock_write_d(ED, tmp32u2)); break;
                        case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
                    }
            }
#else
            pthread_mutex_lock(&emu->context->mutex_lock);
            if(rex.w) {
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
                if((nextop&0xC0)==0xC0)
                    switch((nextop>>3)&7) {
                        case 0: ED->q[0] = add32(emu, ED->dword[0], tmp64u); break;
                        case 1: ED->q[0] =  or32(emu, ED->dword[0], tmp64u); break;
                        case 2: ED->q[0] = adc32(emu, ED->dword[0], tmp64u); break;
                        case 3: ED->q[0] = sbb32(emu, ED->dword[0], tmp64u); break;
                        case 4: ED->q[0] = and32(emu, ED->dword[0], tmp64u); break;
                        case 5: ED->q[0] = sub32(emu, ED->dword[0], tmp64u); break;
                        case 6: ED->q[0] = xor32(emu, ED->dword[0], tmp64u); break;
                        case 7:            cmp32(emu, ED->dword[0], tmp64u); break;
                    }
                else
                    switch((nextop>>3)&7) {
                        case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp64u); break;
                        case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp64u); break;
                        case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp64u); break;
                        case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp64u); break;
                        case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp64u); break;
                        case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp64u); break;
                        case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp64u); break;
                        case 7:                cmp32(emu, ED->dword[0], tmp64u); break;
                    }
            }
            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;
            
        default:
            return 1;
    }
    return 0;
}