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
    int32_t tmp32s;
    uint32_t tmp32u;
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
                    
            default:
                return 1;
            }
            break;

        default:
            return 1;
    }
    return 0;
}