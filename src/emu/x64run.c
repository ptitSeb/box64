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
#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

int my_setcontext(x64emu_t* emu, void* ucp);

int Run(x64emu_t *emu, int step)
{
    uint8_t opcode;
    uint8_t nextop;
    reg64_t *oped, *opgd;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint32_t tmp32u;
    uint64_t tmp64u;
    int32_t tmp32s;
    rex_t rex;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix
    int unimp = 0;

    if(emu->quit)
        return 0;

    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p\n", emu, (void*)R_RIP, emu->context->stack);

x64emurun:

    while(1) {
#ifdef HAVE_TRACE
        __builtin_prefetch((void*)R_RIP, 0, 0); 
        emu->prev2_ip = emu->old_ip;
        if(my_context->dec && (
            (trace_end == 0) 
            || ((R_RIP >= trace_start) && (R_RIP < trace_end))) )
                PrintTrace(emu, R_RIP, 0);
#endif
        emu->old_ip = R_RIP;

        opcode = F8;
        
        rep = 0;
        while((opcode==0xF2) || (opcode==0xF3)) {
            rep = opcode-0xF1;
            opcode = F8;
        }
        rex.rex = 0;
        while(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        }

        switch(opcode) {

        #define GO(B, OP)                                   \
        case B+0:                                           \
            nextop = F8;                                    \
            GETEB(0);                                       \
            GETGB;                                          \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);      \
            break;                                          \
        case B+1:                                           \
            nextop = F8;                                    \
            GETED(0);                                       \
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
            GETEB(0);                                       \
            GETGB;                                          \
            GB = OP##8(emu, GB, EB->byte[0]);               \
            break;                                          \
        case B+3:                                           \
            nextop = F8;                                    \
            GETED(0);                                       \
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
        case 0x0F:                      /* More instructions */
            switch(rep) {
                case 1:
                    if(RunF20F(emu, rex)) {
                        unimp = 1;
                        goto fini;
                    }
                    break;
                case 2:
                    if(RunF30F(emu, rex)) {
                        unimp = 1;
                        goto fini;
                    }
                    break;
                default:
                    if(Run0F(emu, rex)) {
                        unimp = 1;
                        goto fini;
                    }
                    break;
            }
            if(emu->quit)
                goto fini;
            break;
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO

        case 0x38:
            nextop = F8;
            GETEB(0);
            GETGB;
            cmp8(emu, EB->byte[0], GB);
            break;
        case 0x39:
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                cmp64(emu, ED->q[0], GD->q[0]);
            else
                cmp32(emu, ED->dword[0], GD->dword[0]);
            break;
        case 0x3A:
            nextop = F8;
            GETEB(0);
            GETGB;
            cmp8(emu, GB, EB->byte[0]);
            break;
        case 0x3B:
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                cmp64(emu, GD->q[0], ED->q[0]);
            else
                cmp32(emu, GD->dword[0], ED->dword[0]);
            break;
        case 0x3C:
            cmp8(emu, R_AL, F8);
            break;
        case 0x3D:
            if(rex.w)
                cmp64(emu, R_RAX, F32S64);
            else
                cmp32(emu, R_EAX, F32);
            break;

        case 0x54:                      /* PUSH ESP */
            if(rex.b)
                Push(emu, R_R12);
            else {
                tmp64u = R_RSP;
                Push(emu, tmp64u);
            }
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x55:
        case 0x56:
        case 0x57:                      /* PUSH Reg */
            tmp8u = (opcode&7)+(rex.b<<3);
            Push(emu, emu->regs[tmp8u].q[0]);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:                      /* POP ESP */
        case 0x5D:
        case 0x5E:
        case 0x5F:                      /* POP Reg */
            tmp8u = (opcode&7)+(rex.b<<3);
            emu->regs[tmp8u].q[0] = Pop(emu);
            break;

        case 0x63:                      /* MOVSXD Gd,Ed */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                GD->sq[0] = ED->sdword[0];
            else
                if(MODREG)
                    GD->q[0] = ED->sdword[0];
                else
                    GD->sdword[0] = ED->sdword[0];  // meh?
            break;
        case 0x64:                      /* FS: prefix */
            if(Run64(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0x66:                      /* 16bits prefix */
            if(Run66(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0x68:                      /* Push Id */
            Push(emu, F32S64);
            break;
        case 0x69:                      /* IMUL Gd,Ed,Id */
            nextop = F8;
            GETED(4);
            GETGD;
            tmp64u = F32S64;
            if(rex.w)
                GD->q[0] = imul64(emu, ED->q[0], tmp64u);
            else
                if((nextop&0xC0)==0xC0)
                    GD->q[0] = imul32(emu, ED->dword[0], tmp64u);
                else
                    GD->dword[0] = imul32(emu, ED->dword[0], tmp64u);
            break;

        GOCOND(0x70
            ,   tmp8s = F8S; CHECK_FLAGS(emu);
            ,   R_RIP += tmp8s;
            ,
            )                           /* Jxx Ib */
        
        case 0x80:                      /* GRP Eb,Ib */
            nextop = F8;
            GETEB(1);
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
            GETED((opcode==0x81)?4:1);
            if(opcode==0x81) {
                tmp32s = F32S;
            } else {
                tmp32s = F8S;
            }
            if(rex.w) {
                tmp64u = (uint64_t)tmp32s;
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

        case 0x84:                      /* TEST Eb,Gb */
            nextop = F8;
            GETEB(0);
            GETGB;
            test8(emu, EB->byte[0], GB);
            break;
        case 0x85:                      /* TEST Ed,Gd */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                test64(emu, ED->q[0], GD->q[0]);
            else
                test32(emu, ED->dword[0], GD->dword[0]);
            break;

        case 0x87:                      /* XCHG Ed,Gd */
            nextop = F8;
#ifdef DYNAREC
            GET_ED;
            if((nextop&0xC0)==0xC0) {
                tmp32u = GD.dword[0];
                GD.dword[0] = ED->dword[0];
                ED->dword[0] = tmp32u;
            } else {
                if(((uintptr_t)ED)&3)
                {
                    // not aligned, dont't try to "LOCK"
                    tmp32u = ED->dword[0];
                    ED->dword[0] = GD.dword[0];
                    GD.dword[0] = tmp32u;
                } else {
                    // XCHG is supposed to automaticaly LOCK memory bus
                    GD.dword[0] = arm_lock_xchg(ED, GD.dword[0]);
                }
            }
#else
            GETED(0);
            GETGD;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            if(rex.w) {
                tmp64u = GD->q[0];
                GD->q[0] = ED->q[0];
                ED->q[0] = tmp64u;
            } else {
                tmp32u = GD->dword[0];
                GD->dword[0] = ED->dword[0];
                ED->dword[0] = tmp32u;
            }
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;
        case 0x88:                      /* MOV Eb,Gb */
            nextop = F8;
            GETEB(0);
            GETGB;
            EB->byte[0] = GB;
            break;
        case 0x89:                    /* MOV Ed,Gd */
            nextop = F8;
            GETED(0);
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

        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                GD->q[0] = ED->q[0];
            else
                GD->q[0] = ED->dword[0];
            break;

        case 0x8D:                      /* LEA Gd,M */
            nextop = F8;
            GETED(0);
            GETGD;
            if(rex.w)
                GD->q[0] = (uint64_t)ED;
            else
                GD->q[0] = (uint32_t)(uintptr_t)ED;
            break;

        case 0x90:                      /* NOP */
            break;

        case 0x98:                      /* CWDE */
            if(rex.w)
                emu->regs[_AX].sq[0] = emu->regs[_AX].sdword[0];
            else
                emu->regs[_AX].sdword[0] = emu->regs[_AX].sword[0];
            break;
        case 0x99:                      /* CDQ */
            if(rex.w)
                R_RDX=(R_RAX & 0x8000000000000000L)?0xFFFFFFFFFFFFFFFFL:0x0000000000000000L;
            else
                R_RDX=(R_EAX & 0x80000000)?0xFFFFFFFFFFFFFFFFL:0x0000000000000000L;
            break;

        case 0xA8:                      /* TEST AL, Ib */
            test8(emu, R_AL, F8);
            break;
        case 0xA9:                      /* TEST EAX, Id */
            if(rex.w)
                test64(emu, R_RAX, F32S64);
            else
                test32(emu, R_EAX, F32);
            break;

        case 0xAB:                      /* (REP) STOSD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            tmp64u = (rep)?R_RCX:1L;
            if((rex.w))
                while(tmp64u) {
                    *(uint64_t*)R_RDI = R_RAX;
                    R_RDI += tmp8s;
                    --tmp64u;
                }
            else
                while(tmp64u) {
                    *(uint32_t*)R_RDI = R_EAX;
                    R_RDI += tmp8s;
                    --tmp64u;
                }
            if(rep)
                R_RCX = tmp64u;
            break;

        case 0xB0:                      /* MOV AL,Ib */
        case 0xB1:                      /* MOV CL,Ib */
        case 0xB2:                      /* MOV DL,Ib */
        case 0xB3:                      /* MOV BL,Ib */
            if(rex.rex)
                emu->regs[(opcode&7)+(rex.b<<3)].byte[0] = F8;
            else
                emu->regs[opcode&3].byte[0] = F8;
            break;
        case 0xB4:                      /* MOV AH,Ib */
        case 0xB5:                      /*    ...    */
        case 0xB6:
        case 0xB7:
            if(rex.rex)
                emu->regs[(opcode&7)+(rex.b<<3)].byte[0] = F8;
            else
                emu->regs[opcode&3].byte[1] = F8;
            break;
        case 0xB8:                      /* MOV EAX,Id */
        case 0xB9:                      /* MOV ECX,Id */
        case 0xBA:                      /* MOV EDX,Id */
        case 0xBB:                      /* MOV EBX,Id */
        case 0xBC:                      /*    ...     */
        case 0xBD:
        case 0xBE:
        case 0xBF:
            if(rex.w)
                emu->regs[(opcode&7)+(rex.b<<3)].q[0] = F64;
            else
                emu->regs[(opcode&7)+(rex.b<<3)].q[0] = F32;
            break;

        case 0xC1:                      /* GRP2 Ed,Ib */
            nextop = F8;
            GETED(1);
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

        case 0xC3:                      /* RET */
            R_RIP = Pop(emu);
            STEP
            break;

        case 0xC6:                      /* MOV Eb,Ib */
            nextop = F8;
            GETEB(1);
            EB->byte[0] = F8;
            break;
        case 0xC7:                      /* MOV Ed,Id */
            nextop = F8;
            GETED(4);
            if(rex.w)
                ED->q[0] = F32S64;
            else
                if(MODREG)
                    ED->q[0] = F32;
                else
                    ED->dword[0] = F32;
            break;

        case 0xC9:                      /* LEAVE */
            R_RSP = R_RBP;
            R_RBP = Pop(emu);
            break;

        case 0xCC:                      /* INT 3 */
            x64Int3(emu);
            if(emu->quit) goto fini;
            break;

        case 0xD1:                      /* GRP2 Ed,1 */
        case 0xD3:                      /* GRP2 Ed,CL */
            nextop = F8;
            GETED(0);
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

        case 0xD8:                      /* x87 opcodes */
            if(RunD8(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;
        case 0xD9:                      /* x87 opcodes */
            if(RunD9(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0xDB:                      /* x87 opcodes */
            if(RunDB(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0xDD:                      /* x87 opcodes */
            if(RunDD(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0xDF:                      /* x87 opcodes */
            if(RunDF(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            Push(emu, R_RIP);
            R_RIP += tmp32s;
            STEP
            break;
        case 0xE9:                      /* JMP Id */
            tmp32s = F32S; // jmp is relative
            R_RIP += tmp32s;
            STEP
            break;

        case 0xEB:                      /* JMP Ib */
            tmp32s = F8S; // jump is relative
            R_RIP += tmp32s;
            STEP
            break;

        case 0xF0:                      /* LOCK prefix */
            if(RunF0(emu, rex)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;

        case 0xF6:                      /* GRP3 Eb(,Ib) */
            nextop = F8;
            tmp8u = (nextop>>3)&7;
            GETEB((tmp8u<2)?1:0);
            switch(tmp8u) {
                case 0: 
                case 1:                 /* TEST Eb,Ib */
                    tmp8u = F8;
                    test8(emu, EB->byte[0], tmp8u);
                    break;
                case 2:                 /* NOT Eb */
                    EB->byte[0] = not8(emu, EB->byte[0]);
                    break;
                case 3:                 /* NEG Eb */
                    EB->byte[0] = neg8(emu, EB->byte[0]);
                    break;
                case 4:                 /* MUL AL,Eb */
                    mul8(emu, EB->byte[0]);
                    break;
                case 5:                 /* IMUL AL,Eb */
                    imul8(emu, EB->byte[0]);
                    break;
                case 6:                 /* DIV Eb */
                    div8(emu, EB->byte[0]);
                    break;
                case 7:                 /* IDIV Eb */
                    idiv8(emu, EB->byte[0]);
                    break;
            }
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
                        ED->dword[0] = not32(emu, ED->dword[0]);
                        break;
                    case 3:                 /* NEG Ed */
                        ED->dword[0] = neg32(emu, ED->dword[0]);
                        break;
                    case 4:                 /* MUL EAX,Ed */
                        mul32_eax(emu, ED->dword[0]);
                        break;
                    case 5:                 /* IMUL EAX,Ed */
                        imul32_eax(emu, ED->dword[0]);
                        break;
                    case 6:                 /* DIV Ed */
                        div32(emu, ED->dword[0]);
                        break;
                    case 7:                 /* IDIV Ed */
                        idiv32(emu, ED->dword[0]);
                        break;
                }
            }
            break;

        case 0xFF:                      /* GRP 5 Ed */
            nextop = F8;
            GETED(0);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    if(rex.w)
                        ED->q[0] = inc64(emu, ED->q[0]);
                    else
                        ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    if(rex.w)
                        ED->q[0] = dec64(emu, ED->q[0]);
                    else
                        ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                case 2:                 /* CALL NEAR Ed */
                    tmp64u = (uintptr_t)getAlternate((void*)ED->q[0]);
                    Push(emu, R_RIP);
                    R_RIP = tmp64u;
                    STEP
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xC0) {
                        R_RIP = emu->old_ip;
                        printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        Push16(emu, R_CS);
                        Push(emu, R_RIP);
                        R_RIP = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                        goto fini;  // exit loop to recompute new CS...
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    R_RIP = (uintptr_t)getAlternate((void*)ED->q[0]);
                    STEP
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        R_RIP = emu->old_ip;
                        printf_log(LOG_NONE, "Illegal Opcode %p: 0x%02X 0x%02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        R_RIP = ED->q[0];
                        R_CS = (ED+1)->word[0];
                        STEP
                        goto fini;  // exit loop to recompute CS...
                    }
                    break;
                case 6:                 /* Push Ed */
                    tmp64u = ED->q[0];  // rex.w ignored
                    Push(emu, tmp64u);  // avoid potential issue with push [esp+...]
                    break;
                default:
                    R_RIP = emu->old_ip;
                    printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X %02X %02X\n",(void*)R_RIP, opcode, nextop, PK(2), PK(3), PK(4), PK(5));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    goto fini;
            }
            break;
        default:
            unimp = 1;
            goto fini;
        }
    }


fini:
    if(unimp) {
        R_RIP = emu->old_ip;
        emu->quit = 1;
        UnimpOpcode(emu);
    }
    // fork handling
    if(emu->fork) {
        if(step)
            return 0;
        int forktype = emu->fork;
        emu->quit = 0;
        emu->fork = 0;
        emu = x64emu_fork(emu, forktype);
        goto x64emurun;
    }
    // setcontext handling
    else if(emu->uc_link) {
        emu->quit = 0;
        my_setcontext(emu, emu->uc_link);
        goto x64emurun;
    }
    return 0;
}
