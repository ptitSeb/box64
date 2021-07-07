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
#include "dynarec/arm64_lock.h"

#include "modrm.h"

int Run66F0(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint16_t tmp16u, tmp16u2;
    int32_t tmp32s;
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

    if(rex.w) return RunF0(emu, rex);

    switch(opcode) {
        
        case 0x0F:
            nextop = F8;
            switch(nextop) {

                case 0xB1:                      /* CMPXCHG Ew,Gw */
                    nextop = F8;
                    GETEW(0);
                    GETGW;
#ifdef DYNAREC
                    do {
                        tmp16u = arm64_lock_read_h(EW);
                        cmp16(emu, R_AX, tmp16u);
                        if(ACCESS_FLAG(F_ZF)) {
                            tmp32s = arm64_lock_write_h(EW, GW->word[0]);
                        } else {
                            R_AX = tmp16u;
                            tmp32s = 0;
                        }
                    } while(tmp32s);
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    cmp16(emu, R_AX, EW->word[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        EW->word[0] = GW->word[0];
                    } else {
                        R_AX = EW->word[0];
                    }
                    pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
                    break;

                default:
                    return 1;
            }
            break;

        case 0x81:              /* GRP Ew,Iw */
        case 0x83:              /* GRP Ew,Ib */
            nextop = F8;
            GETED((opcode==0x83)?1:2);
            tmp64s = (opcode==0x83)?(F8S):(F16S);
            tmp64u = (uint64_t)tmp64s;
#ifdef DYNAREC
            if(MODREG)
                switch((nextop>>3)&7) {
                    case 0: ED->word[0] = add16(emu, ED->word[0], tmp64u); break;
                    case 1: ED->word[0] =  or16(emu, ED->word[0], tmp64u); break;
                    case 2: ED->word[0] = adc16(emu, ED->word[0], tmp64u); break;
                    case 3: ED->word[0] = sbb16(emu, ED->word[0], tmp64u); break;
                    case 4: ED->word[0] = and16(emu, ED->word[0], tmp64u); break;
                    case 5: ED->word[0] = sub16(emu, ED->word[0], tmp64u); break;
                    case 6: ED->word[0] = xor16(emu, ED->word[0], tmp64u); break;
                    case 7:            cmp16(emu, ED->word[0], tmp64u); break;
                }
            else
                switch((nextop>>3)&7) {
                    case 0: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 = add16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 1: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 =  or16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 2: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 = adc16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 3: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 = sbb16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 4: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 = and16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 5: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 = sub16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 6: do { tmp16u2 = arm64_lock_read_h(ED); tmp16u2 = xor16(emu, tmp16u2, tmp64u);} while(arm64_lock_write_h(ED, tmp16u2)); break;
                    case 7:                                                 cmp16(emu, ED->word[0], tmp64u); break;
                }
#else
            pthread_mutex_lock(&emu->context->mutex_lock);
            switch((nextop>>3)&7) {
                case 0: ED->word[0] = add16(emu, ED->word[0], tmp64u); break;
                case 1: ED->word[0] =  or16(emu, ED->word[0], tmp64u); break;
                case 2: ED->word[0] = adc16(emu, ED->word[0], tmp64u); break;
                case 3: ED->word[0] = sbb16(emu, ED->word[0], tmp64u); break;
                case 4: ED->word[0] = and16(emu, ED->word[0], tmp64u); break;
                case 5: ED->word[0] = sub16(emu, ED->word[0], tmp64u); break;
                case 6: ED->word[0] = xor16(emu, ED->word[0], tmp64u); break;
                case 7:               cmp16(emu, ED->word[0], tmp64u); break;
            }
            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;

       default:
            return 1;
    }
    return 0;
}
