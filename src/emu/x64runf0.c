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
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int RunF0(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint32_t tmp32u;
    int64_t tmp64s;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

        case 0x0f:
            opcode = F8;
            switch (opcode) { 

                case 0xB1:                      /* CMPXCHG Ed,Gd */
                    nextop = F8;
                    GETED;
                    GETGD;
#ifdef DYNAREC
                    if(((uintptr_t)ED)&3) {
                        do {
                            tmp32u = ED->dword[0] & ~0xff;
                            tmp32u |= arm_lock_read_b(ED);
                            cmp32(emu, R_EAX, tmp32u);
                            if(ACCESS_FLAG(F_ZF)) {
                                tmp32s = arm_lock_write_b(ED, GD->dword[0] & 0xff);
                                if(!tmp32s)
                                    ED->dword[0] = GD.dword[0];
                            } else {
                                R_EAX = tmp32u;
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                    } else {
                        do {
                            tmp32u = arm_lock_read_d(ED);
                            cmp32(emu, R_EAX, tmp32u);
                            if(ACCESS_FLAG(F_ZF)) {
                                tmp32s = arm_lock_write_d(ED, GD.dword[0]);
                            } else {
                                R_EAX = tmp32u;
                                tmp32s = 0;
                            }
                        } while(tmp32s);
                    }
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
                    GETED;
                    GETGD;
#ifdef DYNAREC
                    if(((uintptr_t)ED)&3) {
                        do {
                            tmp32u = ED->dword[0] & ~0xff;
                            tmp32u |= arm_lock_read_b(ED);
                            tmp32u2 = add32(emu, tmp32u, GD.dword[0]);
                        } while(arm_lock_write_b(ED, tmp32u2&0xff));
                        ED->dword[0] = tmp32u2;
                    } else {
                        do {
                            tmp32u = arm_lock_read_d(ED);
                            tmp32u2 = add32(emu, tmp32u, GD.dword[0]);
                        } while(arm_lock_write_d(ED, tmp32u2));
                    }
                    GD.dword[0] = tmp32u;
#else
                    pthread_mutex_lock(&emu->context->mutex_lock);
                    if(rex.w) {
                        tmp64u = add64(emu, ED->q[0], GD->q[0]);
                        GD->q[0] = ED->q[0];
                        ED->q[0] = tmp64u;
                    } else {
                        tmp32u = add32(emu, ED->dword[0], GD->dword[0]);
                        GD->dword[0] = ED->dword[0];
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
            GETED;
            if(opcode==0x83) {
                tmp64s = F8S;
                tmp64u = (uint64_t)tmp64s;
            } else
                tmp64u = F32S64;
#ifdef DYNAREC
            switch((nextop>>3)&7) {
                case 0: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, add32(emu, tmp32u2, tmp32u))); break;
                case 1: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED,  or32(emu, tmp32u2, tmp32u))); break;
                case 2: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, adc32(emu, tmp32u2, tmp32u))); break;
                case 3: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, sbb32(emu, tmp32u2, tmp32u))); break;
                case 4: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, and32(emu, tmp32u2, tmp32u))); break;
                case 5: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, sub32(emu, tmp32u2, tmp32u))); break;
                case 6: do { tmp32u2 = arm_lock_read_d(ED);} while(arm_lock_write_d(ED, xor32(emu, tmp32u2, tmp32u))); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
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
                switch((nextop>>3)&7) {
                    case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp64u&0xffffffff); break;
                    case 7:                cmp32(emu, ED->dword[0], tmp64u&0xffffffff); break;
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