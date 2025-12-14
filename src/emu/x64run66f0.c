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
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "bridge.h"
#ifdef DYNAREC
#include "dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test66F0(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t Run66F0(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u, tmp8u2;
    int16_t tmp16s;
    uint16_t tmp16u, tmp16u2;
    int32_t tmp32s;
    uint32_t tmp32u, tmp32u2;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;
    #ifdef USE_CAS
    uint64_t tmpcas;
    #endif
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    switch(opcode) {
        
        case 0x0F:
            nextop = F8;
            switch(nextop) {

                case 0xB1:                      /* CMPXCHG Ew,Gw */
                    nextop = F8;
                    if(MODREG) {
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    }
                    GETEW(0);
                    GETGW;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp16u = native_lock_read_h(EW);
                        cmp16(emu, R_AX, tmp16u);
                        if(ACCESS_FLAG(F_ZF)) {
                            tmp32s = native_lock_write_h(EW, GW->word[0]);
                        } else {
                            R_AX = tmp16u;
                            tmp32s = 0;
                        }
                    } while(tmp32s);
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    cmp16(emu, R_AX, EW->word[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        EW->word[0] = GW->word[0];
                    } else {
                        R_AX = EW->word[0];
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;

                case 0xBA: /* BTS Ew, Ib */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    if(MODREG) {
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    }
                    GETEW(0);
                    tmp8u = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    tmp8u &= 15;
                    if ((uintptr_t)ED & 1) {
                        do {
                            tmp32u = native_lock_read_b(ED + (tmp8u >> 3));
                            if (tmp32u & (1 << (tmp8u & 7))) {
                                SET_FLAG(F_CF);
                                tmp32s = 0;
                            } else {
                                tmp32u ^= (1 << (tmp8u & 7));
                                tmp32s = native_lock_write_b(ED + (tmp8u >> 3), tmp32u);
                                CLEAR_FLAG(F_CF);
                            }
                        } while (tmp32s);
                    } else {
                        do {
                            tmp32u = native_lock_read_h(ED);
                            if (tmp32u & (1 << tmp8u)) {
                                SET_FLAG(F_CF);
                                tmp32s = 0;
                            } else {
                                tmp32u ^= (1 << tmp8u);
                                tmp32s = native_lock_write_h(ED, tmp32u);
                                CLEAR_FLAG(F_CF);
                            }
                        } while (tmp32s);
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    tmp8u &= 15;
                    if (ED->dword[0] & (1 << tmp8u)) {
                        SET_FLAG(F_CF);
                    } else {
                        ED->dword[0] ^= (1 << tmp8u);
                        CLEAR_FLAG(F_CF);
                    }
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    if (BOX64ENV(dynarec_test)) {
                        CLEAR_FLAG(F_OF);
                        CLEAR_FLAG(F_SF);
                        CLEAR_FLAG(F_AF);
                        CLEAR_FLAG(F_PF);
                    }
                    break;

                case 0xC1:                      /* XADD Gw,Ew */
                    nextop = F8;
                    if(MODREG) {
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    }
                    GETEW(0);
                    GETGW;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(((uintptr_t)ED)&1) {
                        do {
                            tmp16u = ED->word[0] & ~0xff;
                            tmp16u |= native_lock_read_h(ED);
                            tmp16u2 = add16(emu, tmp16u, GD->word[0]);
                        } while(native_lock_write_h(ED, tmp16u2&0xff));
                        ED->word[0] = tmp16u2;
                    } else {
                        do {
                            tmp16u = native_lock_read_h(ED);
                            tmp16u2 = add16(emu, tmp16u, GD->word[0]);
                        } while(native_lock_write_h(ED, tmp16u2));
                    }
                    GD->word[0] = tmp16u;
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    tmp16u = add16(emu, ED->word[0], GD->word[0]);
                    GD->word[0] = ED->word[0];
                    ED->word[0] = tmp16u;
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;

                default:
                    return 0;
            }
            break;

#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
        #define GO(B, OP)                                           \
        case B+1:                                                   \
            nextop = F8;                                            \
            if(MODREG) {                                            \
                emu->error |= ERR_ILLEGAL;                          \
                return 0;                                           \
            }                                                       \
            GETEW(0);                                               \
            GETGW;                                                  \
            do {                                                    \
                tmp16u = native_lock_read_h(ED);                    \
                tmp16u = OP##16(emu, tmp16u, GW->word[0]);          \
            } while (native_lock_write_h(ED, tmp16u));              \
            break;
#else
        #define GO(B, OP)                                           \
        case B+1:                                                   \
            nextop = F8;                                            \
            if(MODREG) {                                            \
                emu->error |= ERR_ILLEGAL;                          \
                return 0;                                           \
            }                                                       \
            GETEW(0);                                               \
            GETGW;                                                  \
            pthread_mutex_lock(&my_context->mutex_lock);            \
            EW->word[0] = OP##16(emu, EW->word[0], GW->word[0]);    \
            pthread_mutex_unlock(&my_context->mutex_lock);          \
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

        case 0x81:              /* GRP Ew,Iw */
        case 0x83:              /* GRP Ew,Ib */
            nextop = F8;
            if(MODREG) {
                emu->error |= ERR_ILLEGAL;
                return 0;
            }
            GETEW((opcode==0x83)?1:2);
            tmp16s = (opcode==0x83)?(F8S):(F16S);
            tmp16u = (uint16_t)tmp16s;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            switch((nextop>>3)&7) {
                case 0: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = add16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 1: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 =  or16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 2: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = adc16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 3: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = sbb16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 4: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = and16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 5: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = sub16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 6: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = xor16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                case 7: emu->error |= ERR_ILLEGAL; return 0;
            }
#else
            pthread_mutex_lock(&my_context->mutex_lock);
            switch((nextop>>3)&7) {
                case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
                case 1: EW->word[0] =  or16(emu, EW->word[0], tmp16u); break;
                case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
                case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
                case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
                case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
                case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
                case 7: emu->error |= ERR_ILLEGAL; return 0;
            }
            pthread_mutex_unlock(&my_context->mutex_lock);
#endif
            break;

        case 0xF7:                      /* GRP3 Ed(,Id) */
            nextop = F8;
            if(MODREG) {
                emu->error |= ERR_ILLEGAL;
                return 0;
            }
            GETEW((((nextop>>3)&7)<2)?2:0);
            switch((nextop>>3)&7) {
                case 2:                 /* NOT Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(rex.w)
                        do {
                            tmp64u = native_lock_read_dd(ED); 
                            tmp64u = not64(emu, tmp64u);
                        } while(native_lock_write_dd(ED, tmp64u));
                    else {
                        do {
                            tmp16u = native_lock_read_h(ED); 
                            tmp16u = not16(emu, tmp16u);
                        } while(native_lock_write_h(ED, tmp16u));
                    }
#else
                    if(rex.w) {
                        pthread_mutex_lock(&my_context->mutex_lock);
                        ED->q[0] = not64(emu, ED->q[0]);
                        pthread_mutex_unlock(&my_context->mutex_lock);
                    } else {
                        pthread_mutex_lock(&my_context->mutex_lock);
                        ED->word[0] = not16(emu, ED->dword[0]);
                        pthread_mutex_unlock(&my_context->mutex_lock);
                    }
#endif
                    break;
                default:
                    return 0;
            }
            break;

        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            if(MODREG) {
                emu->error |= ERR_ILLEGAL;
                return 0;
            }
            GETEW(0);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(((uintptr_t)EW)&1) { 
                        //meh.
                        do {
                            tmp16u = EW->word[0];
                            tmp16u &=~0xff;
                            tmp16u |= native_lock_read_b(EW);
                            tmp16u = inc16(emu, tmp16u);
                        } while(native_lock_write_b(EW, tmp16u&0xff));
                        EW->word[0] = tmp16u;
                    } else {
                        do {
                            tmp16u = native_lock_read_h(EW);
                            tmp16u = inc16(emu, tmp16u);
                        } while(native_lock_write_h(EW, tmp16u));
                    }
                    #else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    tmp64u2 = EW->word[0];
                    tmp16u = EW->word[0] = inc16(emu, EW->word[0]);
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp16u = native_lock_read_h(EW);
                        tmp16u = dec16(emu, tmp16u);
                    } while(native_lock_write_h(EW, tmp16u));
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    ED->word[0] = dec16(emu, ED->word[0]);
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                default:
                    emu->error |= ERR_ILLEGAL;
                    return 0;
            }
            break;
       default:
            return 0;
    }
    return addr;
}
