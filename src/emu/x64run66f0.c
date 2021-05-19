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
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;
    uintptr_t tlsdata = GetFSBaseEmu(emu);

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GETED(1);
            tmp64s = F8S;
            tmp64u = (uint64_t)tmp64s;
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
                        case 0: ED->word[0] = add16(emu, ED->word[0], tmp64u); break;
                        case 1: ED->word[0] =  or16(emu, ED->word[0], tmp64u); break;
                        case 2: ED->word[0] = adc16(emu, ED->word[0], tmp64u); break;
                        case 3: ED->word[0] = sbb16(emu, ED->word[0], tmp64u); break;
                        case 4: ED->word[0] = and16(emu, ED->word[0], tmp64u); break;
                        case 5: ED->word[0] = sub16(emu, ED->word[0], tmp64u); break;
                        case 6: ED->word[0] = xor16(emu, ED->word[0], tmp64u); break;
                        case 7:               cmp16(emu, ED->word[0], tmp64u); break;
                    }
                else
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
            }
            pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;

       default:
            return 1;
    }
    return 0;
}
