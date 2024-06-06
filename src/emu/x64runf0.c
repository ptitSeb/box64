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
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestF0(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t RunF0(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u, tmp8u2;      (void)tmp8u2;
    int32_t tmp32s;             (void)tmp32s;
    uint32_t tmp32u, tmp32u2;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;
    #ifdef USE_CAS
    uint64_t tmpcas;
    #endif
    #ifdef TEST_INTERPRETER
    x64emu_t*emu = test->emu;
    #endif

    opcode = F8;
    while((opcode==0x36) || (opcode==0x2E) || (opcode==0x3E) || (opcode==0x26))
        opcode = F8;

    // REX prefix before the F0 are ignored
    rex.rex = 0;
    if(!rex.is32bits)
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }

    switch(opcode) {
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
        #define GO(B, OP)                                           \
        case B+0:                                                   \
            nextop = F8;                                            \
            GETEB(0);                                               \
            GETGB;                                                  \
            do {                                                    \
                tmp8u = native_lock_read_b(EB);                     \
                tmp8u = OP##8(emu, tmp8u, GB);                      \
            } while (native_lock_write_b(EB, tmp8u));               \
            break;                                                  \
        case B+1:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            if(((uintptr_t)ED)&(3<<rex.w)) {                        \
                if(rex.w) {                                             \
                    do {                                                \
                        tmp8u = native_lock_read_b(ED);                 \
                        tmp64u = ED->q[0];                              \
                        tmp64u = OP##64(emu, tmp64u, GD->q[0]);         \
                    } while (native_lock_write_b(ED, tmp8u));           \
                    ED->q[0] = tmp64u;                                  \
                } else {                                                \
                    do {                                                \
                        tmp8u = native_lock_read_b(ED);                 \
                        tmp32u = ED->dword[0];                          \
                        tmp32u = OP##32(emu, tmp32u, GD->dword[0]);     \
                    } while (native_lock_write_b(ED, tmp8u));           \
                    ED->dword[0] = tmp32u;                              \
                    if(MODREG)                                          \
                        ED->dword[1] = 0;                               \
                }                                                       \
            } else {                                                \
            if(rex.w) {                                             \
                do {                                                \
                    tmp64u = native_lock_read_dd(ED);               \
                    tmp64u = OP##64(emu, tmp64u, GD->q[0]);         \
                } while (native_lock_write_dd(ED, tmp64u));         \
            } else {                                                \
                do {                                                \
                    tmp32u = native_lock_read_d(ED);                \
                    tmp32u = OP##32(emu, tmp32u, GD->dword[0]);     \
                } while (native_lock_write_d(ED, tmp32u));          \
                if(MODREG)                                          \
                    ED->dword[1] = 0;                               \
            }                                                       \
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
            pthread_mutex_lock(&my_context->mutex_lock);            \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);              \
            pthread_mutex_unlock(&my_context->mutex_lock);          \
            break;                                                  \
        case B+1:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            pthread_mutex_lock(&my_context->mutex_lock);            \
            if(rex.w)                                               \
                ED->q[0] = OP##64(emu, ED->q[0], GD->q[0]);         \
            else                                                    \
                if(MODREG)                                          \
                    ED->q[0] = OP##32(emu, ED->dword[0], GD->dword[0]);     \
                else                                                        \
                    ED->dword[0] = OP##32(emu, ED->dword[0], GD->dword[0]); \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
            break;                                                  \
        case B+2:                                                   \
            nextop = F8;                                            \
            GETEB(0);                                               \
            GETGB;                                                  \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            GB = OP##8(emu, GB, EB->byte[0]);                       \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
            break;                                                  \
        case B+3:                                                   \
            nextop = F8;                                            \
            GETED(0);                                               \
            GETGD;                                                  \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            if(rex.w)                                               \
                GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]);         \
            else                                                    \
                GD->q[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
            break;                                                  \
        case B+4:                                                   \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            R_AL = OP##8(emu, R_AL, F8);                            \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
            break;                                                  \
        case B+5:                                                   \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            if(rex.w)                                               \
                R_RAX = OP##64(emu, R_RAX, F32S64);                 \
            else                                                    \
                R_RAX = OP##32(emu, R_EAX, F32);                    \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
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
                    test->memaddr=((test->memaddr)+(tmp32s<<(rex.w?3:2)));
                    if(rex.w)
                        *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                    else
                        *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                    #else
                    ED=(reg64_t*)(((uintptr_t)(ED))+(tmp64s<<(rex.w?3:2)));
                    #endif
                }
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                if(rex.w) {
                    tmp8u&=63;
                    if(MODREG) {
                        if(ED->q[0] & (1LL<<tmp8u))
                            SET_FLAG(F_CF);
                        else {
                            ED->q[0] |= (1LL<<tmp8u);
                            CLEAR_FLAG(F_CF);
                        }
                    } else
                        do {
                            tmp64u = native_lock_read_dd(ED);
                            if(tmp64u & (1LL<<tmp8u)) {
                                SET_FLAG(F_CF);
                                tmp32s = 0;
                            } else {
                                tmp64u |= (1LL<<tmp8u);
                                CLEAR_FLAG(F_CF);
                                tmp32s = native_lock_write_dd(ED, tmp64u);
                            }
                        } while(tmp32s);
                } else {
                    tmp8u&=31;
                    if(MODREG) {
                        if(ED->dword[0] & (1<<tmp8u))
                            SET_FLAG(F_CF);
                        else {
                            ED->dword[0] |= (1<<tmp8u);
                            CLEAR_FLAG(F_CF);
                        }
                        ED->dword[1] = 0;
                    } else
                        do {
                            tmp32u = native_lock_read_d(ED);
                            if(tmp32u & (1<<tmp8u)) {
                                SET_FLAG(F_CF);
                                tmp32s = 0;
                            } else {
                                tmp32u |= (1<<tmp8u);
                                CLEAR_FLAG(F_CF);
                                tmp32s = native_lock_write_d(ED, tmp32u);
                            }
                        } while(tmp32s);
                }
#else
                pthread_mutex_lock(&my_context->mutex_lock);
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
                pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                break;

                case 0xB0:                      /* CMPXCHG Eb,Gb */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GETGB;
                    GETEB(0);
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = native_lock_read_b(EB);
                        cmp8(emu, R_AL, tmp8u);
                        if(ACCESS_FLAG(F_ZF)) {
                            tmp32s = native_lock_write_b(EB, GB);
                        } else {
                            R_AL = tmp8u;
                            tmp32s = 0;
                        }
                    } while(tmp32s);
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    cmp8(emu, R_AL, EB->byte[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        EB->byte[0] = GB;
                    } else {
                        R_AL = EB->byte[0];
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                case 0xB1:                      /* CMPXCHG Ed,Gd */
                    nextop = F8;
                    GETED(0);
                    GETGD;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(rex.w)
                        if(((uintptr_t)ED)&7) {
                            do {
                                tmp64u = ED->q[0] & ~0xffLL;
                                tmp64u |= native_lock_read_b(ED);
                                cmp64(emu, R_RAX, tmp64u);
                                if(ACCESS_FLAG(F_ZF)) {
                                    tmp32s = native_lock_write_b(ED, GD->q[0]&0xff);
                                    if(!tmp32s)
                                        ED->q[0] = GD->q[0];
                                } else {
                                    R_RAX = tmp64u;
                                    tmp32s = 0;
                                }
                            } while(tmp32s);
                        } else
                            do {
                                tmp64u = native_lock_read_dd(ED);
                                cmp64(emu, R_RAX, tmp64u);
                                if(ACCESS_FLAG(F_ZF)) {
                                    tmp32s = native_lock_write_dd(ED, GD->q[0]);
                                } else {
                                    R_RAX = tmp64u;
                                    tmp32s = 0;
                                }
                            } while(tmp32s);
                    else {
                        if(((uintptr_t)ED)&3) {
                            do {
                                tmp32u = ED->q[0] & ~0xffLL;
                                tmp32u |= native_lock_read_b(ED);
                                cmp64(emu, R_RAX, tmp32u);
                                if(ACCESS_FLAG(F_ZF)) {
                                    tmp32s = native_lock_write_b(ED, GD->dword[0]&0xff);
                                    if(!tmp32s)
                                        ED->dword[0] = GD->dword[0];
                                } else {
                                    R_EAX = tmp32u;
                                    tmp32s = 0;
                                }
                            } while(tmp32s);
                        } else
                            do {
                                tmp32u = native_lock_read_d(ED);
                                cmp32(emu, R_EAX, tmp32u);
                                if(ACCESS_FLAG(F_ZF)) {
                                    tmp32s = native_lock_write_d(ED, GD->dword[0]);
                                } else {
                                    R_EAX = tmp32u;
                                    tmp32s = 0;
                                }
                            } while(tmp32s);
                        emu->regs[_AX].dword[1] = 0;
                        if(MODREG)
                            ED->dword[1] = 0;
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
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
                        emu->regs[_AX].dword[1] = 0;
                        if(MODREG)
                            ED->dword[1] = 0;
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
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
                        test->memaddr=((test->memaddr)+(tmp32s<<(rex.w?3:2)));
                        if(rex.w)
                            *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                        else
                            *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                        #else
                        ED=(reg64_t*)(((uintptr_t)(ED))+(tmp64s<<(rex.w?3:2)));
                        #endif
                    }
                    tmp8u&=rex.w?63:31;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(rex.w)
                        do {
                            tmp64u = native_lock_read_dd(ED);
                            if(tmp64u & (1LL<<tmp8u)) {
                                SET_FLAG(F_CF);
                                tmp64u ^= (1LL<<tmp8u);
                                tmp32s = native_lock_write_dd(ED, tmp64u);
                            } else {
                                CLEAR_FLAG(F_CF);
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                    else {
                        do {
                            tmp32u = native_lock_read_d(ED);
                            if(tmp32u & (1<<tmp8u)) {
                                SET_FLAG(F_CF);
                                tmp32u ^= (1<<tmp8u);
                                tmp32s = native_lock_write_d(ED, tmp32u);
                            } else {
                                CLEAR_FLAG(F_CF);
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                        if(MODREG)
                            ED->dword[1] = 0;
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    if(rex.w) {
                        if(ED->q[0] & (1<<tmp8u)) {
                            SET_FLAG(F_CF);
                            ED->q[0] ^= (1<<tmp8u);
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
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
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
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                                if(rex.w) {
                                    tmp8u&=63;
                                    do {
                                        tmp64u = native_lock_read_dd(ED);
                                        if(tmp64u & (1LL<<tmp8u)) {
                                            SET_FLAG(F_CF);
                                            tmp32s = 0;
                                        } else {
                                            tmp64u ^= (1LL<<tmp8u);
                                            tmp32s = native_lock_write_dd(ED, tmp64u);
                                            CLEAR_FLAG(F_CF);
                                        }
                                    } while(tmp32s);
                                } else {
                                    tmp8u&=31;
                                    if((uintptr_t)ED&3) {
                                        do {
                                            tmp32u = native_lock_read_b(ED+(tmp8u>>3));
                                            if(tmp32u & (1<<(tmp8u&7))) {
                                                SET_FLAG(F_CF);
                                                tmp32s = 0;
                                            } else {
                                                tmp32u ^= (1<<(tmp8u&7));
                                                tmp32s = native_lock_write_b(ED+(tmp8u>>3), tmp32u);
                                                CLEAR_FLAG(F_CF);
                                            }
                                        } while(tmp32s);
                                    } else {
                                        do {
                                            tmp32u = native_lock_read_d(ED);
                                            if(tmp32u & (1<<tmp8u)) {
                                                SET_FLAG(F_CF);
                                                tmp32s = 0;
                                            } else {
                                                tmp32u ^= (1<<tmp8u);
                                                tmp32s = native_lock_write_d(ED, tmp32u);
                                                CLEAR_FLAG(F_CF);
                                            }
                                        } while(tmp32s);
                                    }
                                }
#else
                                pthread_mutex_lock(&my_context->mutex_lock);
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
                                pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                                break;
                            case 6:             /* BTR Ed, Ib */
                                CHECK_FLAGS(emu);
                                GETED(1);
                                tmp8u = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                                if(rex.w) {
                                    do {
                                        tmp8u&=63;
                                        tmp64u = native_lock_read_dd(ED);
                                        if(tmp64u & (1LL<<tmp8u)) {
                                            SET_FLAG(F_CF);
                                            tmp64u ^= (1LL<<tmp8u);
                                            tmp32s = native_lock_write_dd(ED, tmp64u);
                                        } else {
                                            tmp32s = 0;
                                            CLEAR_FLAG(F_CF);
                                        }
                                    } while(tmp32s);
                                } else {
                                    tmp8u&=31;
                                    do {
                                        tmp32u = native_lock_read_d(ED);
                                        if(tmp32u & (1<<tmp8u)) {
                                            SET_FLAG(F_CF);
                                            tmp32u ^= (1<<tmp8u);
                                            tmp32s = native_lock_write_d(ED, tmp32u);
                                        } else {
                                            CLEAR_FLAG(F_CF);
                                            tmp32s = 0;
                                        }
                                    } while(tmp32s);
                                }
#else
                                pthread_mutex_lock(&my_context->mutex_lock);
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
                                pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                                break;
                            case 7:             /* BTC Ed, Ib */
                                CHECK_FLAGS(emu);
                                GETED(1);
                                tmp8u = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                                if(rex.w) {
                                    tmp8u&=63;
                                    do {
                                        tmp64u = native_lock_read_dd(ED);
                                        if(tmp64u & (1LL<<tmp8u))
                                            SET_FLAG(F_CF);
                                        else
                                            CLEAR_FLAG(F_CF);
                                        tmp64u ^= (1LL<<tmp8u);
                                        tmp32s = native_lock_write_dd(ED, tmp64u);
                                    } while(tmp32s);
                                } else {
                                    tmp8u&=31;
                                    do {
                                        tmp32u = native_lock_read_d(ED);
                                        if(tmp32u & (1<<tmp8u))
                                            SET_FLAG(F_CF);
                                        else
                                            CLEAR_FLAG(F_CF);
                                        tmp32u ^= (1<<tmp8u);
                                        tmp32s = native_lock_write_d(ED, tmp32u);
                                    } while(tmp32s);
                                }
#else
                                pthread_mutex_lock(&my_context->mutex_lock);
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
                                pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                                break;

                            default:
                                return 0;
                        }
                        break;

                case 0xC0:                      /* XADD Gb,Eb */
                    nextop = F8;
                    GETEB(0);
                    GETGB;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = native_lock_read_b(EB);
                        tmp8u2 = add8(emu, tmp8u, GB);
                    } while(native_lock_write_b(EB, tmp8u2));
                    GB = tmp8u;
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    tmp8u = add8(emu, EB->byte[0], GB);
                    GB = EB->byte[0];
                    EB->byte[0] = tmp8u;
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                case 0xC1:                      /* XADD Gd,Ed */
                    nextop = F8;
                    GETED(0);
                    GETGD;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(rex.w) {
                        do {
                            tmp64u = native_lock_read_dd(ED);
                            tmp64u2 = add64(emu, tmp64u, GD->q[0]);
                        } while(native_lock_write_dd(ED, tmp64u2));
                        GD->q[0] = tmp64u;
                    } else {
                        if(((uintptr_t)ED)&3) {
                            do {
                                tmp32u = ED->dword[0] & ~0xff;
                                tmp32u |= native_lock_read_b(ED);
                                tmp32u2 = add32(emu, tmp32u, GD->dword[0]);
                            } while(native_lock_write_b(ED, tmp32u2&0xff));
                            ED->dword[0] = tmp32u2;
                        } else {
                            do {
                                tmp32u = native_lock_read_d(ED);
                                tmp32u2 = add32(emu, tmp32u, GD->dword[0]);
                            } while(native_lock_write_d(ED, tmp32u2));
                        }
                        GD->q[0] = tmp32u;
                        if(MODREG)
                            ED->dword[1] = 0;
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    if(rex.w) {
                        tmp64u = ED->q[0];
                        tmp64u2 = add64(emu, tmp64u, GD->q[0]);
                        GD->q[0] = tmp64u;
                        ED->q[0] = tmp64u2;
                    } else {
                        tmp32u = ED->dword[0];
                        tmp32u2 = add32(emu, tmp32u, GD->dword[0]);
                        GD->q[0] = tmp32u;
                        if(MODREG)
                            ED->q[0] = tmp32u2;
                        else
                            ED->dword[0] = tmp32u2;
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;

                case 0xC7:                      /* CMPXCHG8B Gq */
                    nextop = F8;
                    GETE8xw(0);
                    switch((nextop>>3)&7) {
                        case 1:
                            CHECK_FLAGS(emu);
                            GETGD;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                            if (rex.w) {
#if defined(__riscv) || defined(__loongarch64)
#if defined(__loongarch64)
                                if (la64_scq) {
                                    do {
                                        native_lock_read_dq(&tmp64u, &tmp64u2, ED);
                                        if (R_RAX == tmp64u && R_RDX == tmp64u2) {
                                            SET_FLAG(F_ZF);
                                            tmp32s = native_lock_write_dq(R_RBX, R_RCX, ED);
                                        } else {
                                            CLEAR_FLAG(F_ZF);
                                            R_RAX = tmp64u;
                                            R_RDX = tmp64u2;
                                            tmp32s = 0;
                                        }
                                    } while (tmp32s);
                                } else
#endif
                                {
                                    while (native_lock_xchg_d(&emu->context->mutex_16b, 1))
                                        ; // lock
                                    tmp64u = ((uint64_t*)ED)[0];
                                    tmp64u2 = ((uint64_t*)ED)[1];
                                    if (R_RAX == tmp64u && R_RDX == tmp64u2) {
                                        SET_FLAG(F_ZF);
                                        ((uint64_t*)ED)[0] = R_RBX;
                                        ((uint64_t*)ED)[1] = R_RCX;
                                    } else {
                                        CLEAR_FLAG(F_ZF);
                                        R_RAX = tmp64u;
                                        R_RDX = tmp64u2;
                                    }
                                    native_lock_xchg_d(&emu->context->mutex_16b, 0); // unlock
                                }
#else
                                if(((uintptr_t)ED)&0xf) {
                                    do {
                                        native_lock_read_b(ED);
                                        tmp64u = ED->q[0];
                                        tmp64u2 = ED->q[1];
                                        if(R_RAX == tmp64u && R_RDX == tmp64u2) {
                                            SET_FLAG(F_ZF);
                                            tmp32s = native_lock_write_b(ED, emu->regs[_BX].byte[0]);
                                            if(!tmp32s) {
                                                ED->q[0] = R_RBX;
                                                ED->q[1] = R_RCX;
                                            }
                                        } else {
                                            CLEAR_FLAG(F_ZF);
                                            R_RAX = tmp64u;
                                            R_RDX = tmp64u2;
                                            tmp32s = 0;
                                        }
                                    } while(tmp32s);
                                } else
                                do {
                                    native_lock_read_dq(&tmp64u, &tmp64u2, ED);
                                    if(R_RAX == tmp64u && R_RDX == tmp64u2) {
                                        SET_FLAG(F_ZF);
                                        tmp32s = native_lock_write_dq(R_RBX, R_RCX, ED);
                                    } else {
                                        CLEAR_FLAG(F_ZF);
                                        R_RAX = tmp64u;
                                        R_RDX = tmp64u2;
                                        tmp32s = 0;
                                    }
                                } while(tmp32s);
#endif
                            } else
                                if(((uintptr_t)ED)&0x7) {
                                    do {
                                        native_lock_get_b(ED);
                                        tmp64u = ED->q[0];
                                        if((R_EAX == (tmp64u&0xffffffff)) && (R_EDX == ((tmp64u>>32)&0xffffffff))) {
                                            SET_FLAG(F_ZF);
                                            tmp32s = native_lock_write_b(ED, emu->regs[_BX].byte[0]);
                                            if(!tmp32s)
                                                ED->q[0] = R_EBX|(((uint64_t)R_ECX)<<32);
                                        } else {
                                            CLEAR_FLAG(F_ZF);
                                            R_RAX = tmp64u&0xffffffff;
                                            R_RDX = (tmp64u>>32)&0xffffffff;
                                            tmp32s = 0;
                                        }
                                    } while(tmp32s);
                                } else
                                do {
                                    tmp64u = native_lock_read_dd(ED);
                                    if((R_EAX == (tmp64u&0xffffffff)) && (R_EDX == ((tmp64u>>32)&0xffffffff))) {
                                        SET_FLAG(F_ZF);
                                        tmp32s = native_lock_write_dd(ED, R_EBX|(((uint64_t)R_ECX)<<32));
                                    } else {
                                        CLEAR_FLAG(F_ZF);
                                        R_RAX = tmp64u&0xffffffff;
                                        R_RDX = (tmp64u>>32)&0xffffffff;
                                        tmp32s = 0;
                                    }
                                } while(tmp32s);
#else
                            pthread_mutex_lock(&my_context->mutex_lock);
                            if(rex.w) {
                                #ifdef TEST_INTERPRETER
                                test->memsize = 16;
                                #endif
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
                                #ifdef TEST_INTERPRETER
                                test->memsize = 8;
                                #endif
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
                            pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                            break;
                        default:
                            return 0;
                    }
                    break;

            default:
                return 0;
            }
            break;

        case 0x66:
            #ifdef TEST_INTERPRETER
            return Test66F0(test, rex, addr);
            #else
            return Run66F0(emu, rex, addr);   // more opcode F0 66 and 66 F0 is the same
            #endif

        case 0x80:                      /* GRP Eb,Ib */
            nextop = F8;
            GETEB(1);
            tmp8u = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            switch((nextop>>3)&7) {
                case 0: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 = add8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 1: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 =  or8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 2: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 = adc8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 3: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 = sbb8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 4: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 = and8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 5: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 = sub8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 6: do { tmp8u2 = native_lock_read_b(EB); tmp8u2 = xor8(emu, tmp8u2, tmp8u);} while(native_lock_write_b(EB, tmp8u2)); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
#else
            pthread_mutex_lock(&my_context->mutex_lock);
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
            pthread_mutex_unlock(&my_context->mutex_lock);
#endif
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
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            if(rex.w) {
                switch((nextop>>3)&7) {
                    case 0: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 = add64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
                    case 1: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 =  or64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
                    case 2: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 = adc64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
                    case 3: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 = sbb64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
                    case 4: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 = and64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
                    case 5: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 = sub64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
                    case 6: do { tmp64u2 = native_lock_read_dd(ED); tmp64u2 = xor64(emu, tmp64u2, tmp64u);} while(native_lock_write_dd(ED, tmp64u2)); break;
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
                        case 0: if(((uintptr_t)ED)&3) {
                                    // unaligned case
                                    do { tmp32u2 = native_lock_read_b(ED); tmp32u2=ED->dword[0]; tmp32u2 = add32(emu, tmp32u2, tmp64u);} while(native_lock_write_b(ED, tmp32u2)); ED->dword[0]=tmp32u2; break;    
                                } else
                                do { tmp32u2 = native_lock_read_d(ED); tmp32u2 = add32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 1: do { tmp32u2 = native_lock_read_d(ED); tmp32u2 =  or32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 2: do { tmp32u2 = native_lock_read_d(ED); tmp32u2 = adc32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 3: do { tmp32u2 = native_lock_read_d(ED); tmp32u2 = sbb32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 4: do { tmp32u2 = native_lock_read_d(ED); tmp32u2 = and32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 5: if(((uintptr_t)ED)&3) {
                                    // unaligned case
                                    do { tmp32u2 = native_lock_read_b(ED); tmp32u2=ED->dword[0]; tmp32u2 = sub32(emu, tmp32u2, tmp64u);} while(native_lock_write_b(ED, tmp32u2)); ED->dword[0]=tmp32u2; break;    
                                } else
                                do { tmp32u2 = native_lock_read_d(ED); tmp32u2 = sub32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 6: do { tmp32u2 = native_lock_read_d(ED); tmp32u2 = xor32(emu, tmp32u2, tmp64u);} while(native_lock_write_d(ED, tmp32u2)); break;
                        case 7:                                                 cmp32(emu, ED->dword[0], tmp64u); break;
                    }
            }
#else
            pthread_mutex_lock(&my_context->mutex_lock);
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
            pthread_mutex_unlock(&my_context->mutex_lock);
#endif
            break;

        case 0x86:                      /* XCHG Eb,Gb */
            nextop = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            GETEB(0);
            GETGB;
            if(MODREG) { // reg / reg: no lock
                tmp8u = GB;
                GB = EB->byte[0];
                EB->byte[0] = tmp8u;
            } else {
                do {
                    tmp8u = native_lock_read_b(EB);
                } while(native_lock_write_b(EB, GB));
                GB = tmp8u;
            }
#else
            GETEB(0);
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
        case 0x87:                      /* XCHG Ed,Gd */
            nextop = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            GETED(0);
            GETGD;
            if(MODREG) {
                if(rex.w) {
                    tmp64u = GD->q[0];
                    GD->q[0] = ED->q[0];
                    ED->q[0] = tmp64u;
                } else {
                    tmp32u = GD->dword[0];
                    GD->q[0] = ED->dword[0];
                    ED->q[0] = tmp32u;
                }
            } else {
                if(rex.w) {
                    if((uintptr_t)ED&7) {
                        // unaligned
                        do {
                            tmp64u = ED->q[0] & 0xffffffffffffff00LL;
                            tmp64u |= native_lock_read_b(ED);
                            
                        } while(native_lock_write_b(ED, GD->byte[0]));
                        ED->q[0] = GD->q[0];
                        GD->q[0] = tmp64u;
                    } else {
                        GD->q[0] = native_lock_xchg_dd(ED, GD->q[0]);
                    }
                } else {
                    if((uintptr_t)ED&3) {
                        // unaligned
                        do {
                            tmp32u = ED->dword[0] & 0xffffff00;
                            tmp32u |= native_lock_read_b(ED);
                            
                        } while(native_lock_write_b(ED, GD->byte[0]));
                        ED->dword[0] = GD->dword[0];
                        GD->dword[0] = tmp32u;
                    } else {
                        GD->dword[0] = native_lock_xchg_d(ED, GD->dword[0]);
                    }
                }
            }
#else
            GETED(0);
            GETGD;
            pthread_mutex_lock(&my_context->mutex_lock);
            if(rex.w) {
                tmp64u = GD->q[0];
                GD->q[0] = ED->q[0];
                ED->q[0] = tmp64u;
            } else {
                tmp32u = GD->dword[0];
                GD->q[0] = ED->dword[0];
                if(MODREG)
                    ED->q[0] = tmp32u;
                else
                    ED->dword[0] = tmp32u;
            }
            pthread_mutex_unlock(&my_context->mutex_lock);
#endif
            break;            

        case 0xF6:                      /* GRP3 Eb(,Ib) */
            nextop = F8;
            tmp8u = (nextop>>3)&7;
            GETEB((tmp8u<2)?1:0);
            switch(tmp8u) {
                case 2:                 /* NOT Eb */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u2 = native_lock_read_b(EB); 
                        tmp8u2 = not8(emu, tmp8u2);
                    } while(native_lock_write_b(EB, tmp8u2));
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    EB->byte[0] = not8(emu, EB->byte[0]);
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                default:
                    return 0;
            }
            break;

        case 0xFE:              /* GRP 5 Eb */
            nextop = F8;
            GETED(0);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Eb */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = native_lock_read_b(ED);
                    } while(native_lock_write_b(ED, inc8(emu, tmp8u)));
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    ED->byte[0] = inc8(emu, ED->byte[0]);
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp8u = native_lock_read_b(ED);
                    } while(native_lock_write_b(ED, dec8(emu, tmp8u)));
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    ED->byte[0] = dec8(emu, ED->byte[0]);
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode 0xF0 0xFE 0x%02X 0x%02X\n", nextop, PK(0));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    break;
            }
            break;
        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            GETED(0);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(rex.w)
                        if(((uintptr_t)ED)&7) {
                            // unaligned
                            do {
                                tmp64u = ED->q[0] & 0xffffffffffffff00LL;
                                tmp64u |= native_lock_read_b(ED);
                                tmp64u = inc64(emu, tmp64u);
                            } while(native_lock_write_b(ED, tmp64u&0xff));
                            ED->q[0] = tmp64u;
                        }
                        else
                            do {
                                tmp64u = native_lock_read_dd(ED);
                            } while(native_lock_write_dd(ED, inc64(emu, tmp64u)));
                    else {
                        if((uintptr_t)ED&3) { 
                            //meh.
                            do {
                                tmp32u = ED->dword[0];
                                tmp32u &=~0xff;
                                tmp32u |= native_lock_read_b(ED);
                                tmp32u = inc32(emu, tmp32u);
                            } while(native_lock_write_b(ED, tmp32u&0xff));
                            ED->dword[0] = tmp32u;
                        } else {
                            do {
                                tmp32u = native_lock_read_d(ED);
                            } while(native_lock_write_d(ED, inc32(emu, tmp32u)));
                        }
                        if(MODREG) ED->dword[1] = 0;
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    if(rex.w) {
                        ED->q[0] = inc64(emu, ED->q[0]);
                    } else {
                        if(MODREG)
                            ED->q[0] = inc32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = inc32(emu, ED->dword[0]);
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(rex.w)
                        if(((uintptr_t)ED)&7) {
                            // unaligned
                            do {
                                tmp64u = ED->q[0] & 0xffffffffffffff00LL;
                                tmp64u |= native_lock_read_b(ED);
                                tmp64u = dec64(emu, tmp64u);
                            } while(native_lock_write_b(ED, tmp64u&0xff));
                            ED->q[0] = tmp64u;
                        }
                        else
                            do {
                                tmp64u = native_lock_read_dd(ED);
                            } while(native_lock_write_dd(ED, dec64(emu, tmp64u)));
                    else {
                        do {
                            tmp32u = native_lock_read_d(ED);
                        } while(native_lock_write_d(ED, dec32(emu, tmp32u)));
                        if(MODREG) ED->dword[1] = 0;
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    if(rex.w) {
                        ED->q[0] = dec64(emu, ED->q[0]);
                    } else {
                        if(MODREG)
                            ED->q[0] = dec32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = dec32(emu, ED->dword[0]);
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode 0xF0 0xFF 0x%02X 0x%02X\n", nextop, PK(0));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    break;
            }
            break;
        default:
            return 0;
    }
    return addr;
}
