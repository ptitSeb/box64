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

int Run67(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    uint32_t tmp32u;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;

    opcode = F8;

    // REX prefix before the 67 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }

    switch(opcode) {

    case 0x89:                    /* MOV Ed,Gd */
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w) {
            ED->q[0] = GD->q[0];
        } else {
            //if ED is a reg, than the opcode works like movzx
            if(MODREG)
                ED->q[0] = GD->dword[0];
            else
                ED->dword[0] = GD->dword[0];
        }
        break;

    case 0x8D:                      /* LEA Gd,M */
        nextop = F8;
        GETED32(0);
        GETGD;
        if(rex.w)
            GD->q[0] = (uint64_t)ED;
        else
            GD->q[0] = (uint32_t)(uintptr_t)ED;
        break;

    case 0xC1:                      /* GRP2 Ed,Ib */
        nextop = F8;
        GETED32(1);
        tmp8u = F8/* & 0x1f*/; // masking done in each functions
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

    case 0xE0:                      /* LOOPNZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX && !ACCESS_FLAG(F_ZF))
            R_RIP += tmp8s;
        break;
    case 0xE1:                      /* LOOPZ */
        CHECK_FLAGS(emu);
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX && ACCESS_FLAG(F_ZF))
            R_RIP += tmp8s;
        break;
    case 0xE2:                      /* LOOP */
        tmp8s = F8S;
        --R_ECX; // don't update flags
        if(R_ECX)
            R_RIP += tmp8s;
        break;
    case 0xE3:              /* JECXZ Ib */
        tmp8s = F8S;
        if(!R_ECX)
            R_RIP += tmp8s;
        break;

    case 0xF7:                      /* GRP3 Ed(,Id) */
        nextop = F8;
        tmp8u = (nextop>>3)&7;
        GETED((tmp8u<2)?4:0);
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
                    emu->regs[_AX].dword[1] = 0;
                    emu->regs[_DX].dword[1] = 0;
                    break;
                case 7:                 /* IDIV Ed */
                    idiv32(emu, ED->dword[0]);
                    emu->regs[_AX].dword[1] = 0;
                    emu->regs[_DX].dword[1] = 0;
                    break;
            }
        }
        break;
            
    default:
        return 1;
    }
    return 0;
}