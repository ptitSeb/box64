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
    int16_t tmp16s;
    uint16_t tmp16u, tmp16u2;
    int32_t tmp32s;
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
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    uintptr_t addr_entry = addr;
    if(!rex.is32bits)
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }

    if(rex.w) return RunF0(emu, rex, addr_entry);

    switch(opcode) {
        
        case 0x0F:
            nextop = F8;
            switch(nextop) {

                case 0xB1:                      /* CMPXCHG Ew,Gw */
                    nextop = F8;
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

                case 0xC1:                      /* XADD Gw,Ew */
                    nextop = F8;
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
            GETEW(0);                                               \
            GETGW;                                                  \
            do {                                                    \
                tmp16u = native_lock_read_h(ED);                    \
                tmp16u = OP##16(emu, tmp16u, GW->word[0]);          \
            } while (native_lock_write_h(ED, tmp16u));              \
            break;                                                  \
        case B+3:                                                   \
            nextop = F8;                                            \
            GETEW(0);                                               \
            GETGW;                                                  \
            GW->word[0] = OP##16(emu, GW->word[0], EW->word[0]);    \
        case B+5:                                                   \
            R_AX = OP##16(emu, R_AX, F16);                          \
            break;
#else
        #define GO(B, OP)                                           \
        case B+1:                                                   \
            nextop = F8;                                            \
            GETEW(0);                                               \
            GETGW;                                                  \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            EW->word[0] = OP##16(emu, EW->word[0], GW->word[0]);    \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
            break;                                                  \
        case B+3:                                                   \
            nextop = F8;                                            \
            GETEW(0);                                               \
            GETGW;                                                  \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            GW->word[0] = OP##16(emu, GW->word[0], EW->word[0]);    \
            pthread_mutex_unlock(&my_context->mutex_lock);        \
            break;                                                  \
        case B+5:                                                   \
            pthread_mutex_lock(&my_context->mutex_lock);          \
            R_AX = OP##16(emu, R_AX, F16);                          \
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

        case 0x81:              /* GRP Ew,Iw */
        case 0x83:              /* GRP Ew,Ib */
            nextop = F8;
            GETEW((opcode==0x83)?1:2);
            tmp16s = (opcode==0x83)?(F8S):(F16S);
            tmp16u = (uint16_t)tmp16s;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            if(MODREG)
                switch((nextop>>3)&7) {
                    case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
                    case 1: EW->word[0] =  or16(emu, EW->word[0], tmp16u); break;
                    case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
                    case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
                    case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
                    case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
                    case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
                    case 7:               cmp16(emu, EW->word[0], tmp16u); break;
                }
            else
                switch((nextop>>3)&7) {
                    case 0: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = add16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 1: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 =  or16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 2: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = adc16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 3: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = sbb16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 4: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = and16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 5: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = sub16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 6: do { tmp16u2 = native_lock_read_h(EW); tmp16u2 = xor16(emu, tmp16u2, tmp16u);} while(native_lock_write_h(ED, tmp16u2)); break;
                    case 7:                                                  cmp16(emu, EW->word[0], tmp16u); break;
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
                case 7:               cmp16(emu, EW->word[0], tmp16u); break;
            }
            pthread_mutex_unlock(&my_context->mutex_lock);
#endif
            break;

        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            GETEW(0);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if((uintptr_t)EW&1) { 
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
                        } while(native_lock_write_h(EW, inc16(emu, tmp16u)));
                    }
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    EW->word[0] = inc16(emu, EW->word[0]);
                    pthread_mutex_unlock(&my_context->mutex_lock);
#endif
                    break;
                case 1:                 /* DEC Ed */
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    do {
                        tmp16u = native_lock_read_h(EW);
                    } while(native_lock_write_h(EW, dec16(emu, tmp16u)));
#else
                    pthread_mutex_lock(&my_context->mutex_lock);
                    ED->word[0] = dec16(emu, ED->word[0]);
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
