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
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

int my_setcontext(x64emu_t* emu, void* ucp);

#ifdef TEST_INTERPRETER
int RunTest(x64test_t *test)
#else
int Run(x64emu_t *emu, int step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    reg64_t *oped, *opgd;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u, tmp32u2;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2, tmp64u3;
    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    int step = 0;
    #endif
    uintptr_t addr = R_RIP;
    rex_t rex;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix
    int unimp = 0;

    if(emu->quit)
        return 0;
    if(addr==0) {
        emu->quit = 1;
        printf_log(LOG_INFO, "%04d|Ask to run at NULL, quit silently\n", GetTID());
        print_cycle_log(LOG_INFO);
        return 0;
    }
    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p\n", emu, (void*)addr, (void*)R_RSP);

x64emurun:
#ifndef TEST_INTERPRETER
    while(1) 
#endif
    {
#if defined(HAVE_TRACE)
        __builtin_prefetch((void*)addr, 0, 0); 
        emu->prev2_ip = emu->old_ip;
        if(my_context->dec && (
            (trace_end == 0) 
            || ((addr >= trace_start) && (addr < trace_end))) )
                PrintTrace(emu, addr, 0);
#endif
        emu->old_ip = addr;

        opcode = F8;
        
        rep = 0;
        while((opcode==0xF2) || (opcode==0xF3)) {
            rep = opcode-0xF1;
            opcode = F8;
        }
        while((opcode==0x3E))   //Branch Taken Hint ignored
            opcode = F8;
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
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = TestF20F(test, rex, addr, &step)))
                        unimp = 1;
                    #else
                    if(!(addr = RunF20F(emu, rex, addr, &step))) {
                        unimp = 1;
                        goto fini;
                    }
                    if(step==2) STEP2;
                    #endif
                    break;
                case 2:
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = TestF30F(test, rex, addr)))
                        unimp = 1;
                    #else
                    if(!(addr = RunF30F(emu, rex, addr))) {
                        unimp = 1;
                        goto fini;
                    }
                    #endif
                    break;
                default:
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = Test0F(test, rex, addr, &step)))
                        unimp = 1;
                    #else
                    if(!(addr = Run0F(emu, rex, addr, &step))) {
                        unimp = 1;
                        goto fini;
                    }
                    if(step==2) STEP2;
                    #endif
                    break;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            break;
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO

	    case 0x2E:	    /* segments are ignored */
        case 0x36:          /* SS: (ignored) */
            break;

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
                    GD->q[0] = ED->dword[0];    // not really a sign extension
                else
                    GD->sdword[0] = ED->sdword[0];  // meh?
            break;
        case 0x64:                      /* FS: prefix */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test64(test, rex, _FS, addr)))
                unimp = 1;
            #else
            if(!(addr = Run64(emu, rex, _FS, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x65:                      /* GS: prefix */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test64(test, rex, _GS, addr)))
                unimp = 1;
            #else
            if(!(addr = Run64(emu, rex, _GS, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x66:                      /* 16bits prefix */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test66(test, rex, rep, addr)))
                unimp = 1;
            #else
            if(!(addr = Run66(emu, rex, rep, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0x67:                      /* reduce EASize prefix */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test67(test, rex, rep, addr)))
                unimp = 1;
            #else
            if(!(addr = Run67(emu, rex, rep, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
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
                GD->q[0] = imul32(emu, ED->dword[0], tmp64u);
            break;
        case 0x6A:                      /* Push Ib */
            tmp64s = F8S;
            Push(emu, (uint64_t)tmp64s);
            break;
        case 0x6B:                      /* IMUL Gd,Ed,Ib */
            nextop = F8;
            GETED(1);
            GETGD;
            tmp64s = F8S;
            if(rex.w)
                GD->q[0] = imul64(emu, ED->q[0], (uint64_t)tmp64s);
            else
                GD->q[0] = imul32(emu, ED->dword[0], (uint32_t)tmp64s);
            break;
        case 0x6C:                      /* INSB DX */
        case 0x6D:                      /* INSL DX */
        case 0x6E:                      /* OUTSB DX */
        case 0x6F:                      /* OUTSL DX */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETOR
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            #endif
            break;

        GOCOND(0x70
            ,   tmp8s = F8S; CHECK_FLAGS(emu);
            ,   addr += tmp8s;
            ,,STEP2
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
                tmp64u = (uint64_t)(int64_t)tmp32s;
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
                GB = native_lock_xchg_b(EB, GB);
            }
            // dynarec use need it's own mecanism
#else
            GETEB(0);
            GETGB;
            if(!MODREG)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp8u = GB;
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            if(!MODREG)
                pthread_mutex_unlock(&emu->context->mutex_lock);
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
                    GD->q[0] = native_lock_xchg_dd(ED, GD->q[0]);
                } else {
                    GD->q[0] = native_lock_xchg_d(ED, GD->dword[0]);
                }
            }
#else
            GETED(0);
            GETGD;
            if(!MODREG)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
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
            if(!MODREG)
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
        case 0x8A:                      /* MOV Gb,Eb */
            nextop = F8;
            GETEB(0);
            GETGB;
            GB = EB->byte[0];
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
        case 0x8C:                      /* MOV Ed, Seg */
            nextop = F8;
            GETED(0);
            if(MODREG)
                ED->q[0] = emu->segs[((nextop&0x38)>>3)];
            else
                ED->word[0] = emu->segs[((nextop&0x38)>>3)];
            break;
        case 0x8D:                      /* LEA Gd,M */
            nextop = F8;
            GETGD;
            tmp64u = GETEA(0);
            if(rex.w)
                GD->q[0] = tmp64u;
            else
                GD->q[0] = tmp64u&0xffffffff;
            break;

        case 0x8F:                      /* POP Ed */
            nextop = F8;
            if(MODREG) {
                emu->regs[(nextop&7)+(rex.b<<3)].q[0] = Pop(emu);
            } else {
                tmp64u = Pop(emu);  // this order allows handling POP [ESP] and variant
                GETED(0);
                R_ESP -= sizeof(void*); // to prevent issue with SEGFAULT
                ED->q[0] = tmp64u;
                R_ESP += sizeof(void*);
            }
            break;
        case 0x90:                      /* NOP or XCHG R8, RAX*/
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:                      /* XCHG reg,EAX */
            tmp8u = _AX+(opcode&7)+(rex.b<<3);
            if(tmp8u!=_AX) {
                if(rex.w) {
                    tmp64u = R_RAX;
                    R_RAX = emu->regs[tmp8u].q[0];
                    emu->regs[tmp8u].q[0] = tmp64u;
                } else {
                    tmp64u = R_EAX;
                    R_RAX = emu->regs[tmp8u].dword[0];
                    emu->regs[tmp8u].q[0] = tmp64u;
                }
            }
            break;

        case 0x98:                      /* CWDE */
            if(rex.w)
                emu->regs[_AX].sq[0] = emu->regs[_AX].sdword[0];
            else {
                emu->regs[_AX].sdword[0] = emu->regs[_AX].sword[0];
                emu->regs[_AX].dword[1] = 0;
            }
            break;
        case 0x99:                      /* CDQ */
            if(rex.w)
                R_RDX=(R_RAX & 0x8000000000000000LL)?0xFFFFFFFFFFFFFFFFLL:0x0000000000000000LL;
            else
                R_RDX=(R_EAX & 0x80000000)?0x00000000FFFFFFFFLL:0x0000000000000000LL;
            break;

        case 0x9B:                      /* FWAIT */
            break;
        case 0x9C:                      /* PUSHF */
            CHECK_FLAGS(emu);
            Push(emu, emu->eflags.x64);
            break;
        case 0x9D:                      /* POPF */
            emu->eflags.x64 = ((Pop(emu) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
            RESET_FLAGS(emu);
            break;
        case 0x9E:                      /* SAHF */
            CHECK_FLAGS(emu);
            tmp8u = emu->regs[_AX].byte[1];
            CONDITIONAL_SET_FLAG(tmp8u&0x01, F_CF);
            CONDITIONAL_SET_FLAG(tmp8u&0x04, F_PF);
            CONDITIONAL_SET_FLAG(tmp8u&0x10, F_AF);
            CONDITIONAL_SET_FLAG(tmp8u&0x40, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u&0x80, F_SF);
            RESET_FLAGS(emu);
            break;
        case 0x9F:                      /* LAHF */
            CHECK_FLAGS(emu);
            R_AH = (uint8_t)emu->eflags.x64;
            break;
        case 0xA0:                      /* MOV AL,Ob */
            R_AL = *(uint8_t*)F64;
            break;
        case 0xA1:                      /* MOV EAX,Od */
            if(rex.w)
                R_RAX = *(uint64_t*)F64;
            else
                R_RAX = *(uint32_t*)F64;
            break;
        case 0xA2:                      /* MOV Ob,AL */
            *(uint8_t*)F64 = R_AL;
            break;
        case 0xA3:                      /* MOV Od,EAX */
            if(rex.w)
                *(uint64_t*)F64 = R_RAX;
            else
                *(uint32_t*)F64 = R_EAX;
            break;
        case 0xA4:                      /* MOVSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rep)?R_RCX:1L;
            while(tmp64u) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_RDI = *(uint8_t*)R_RSI;
                #endif
                R_RDI += tmp8s;
                R_RSI += tmp8s;
                --tmp64u;
            }
            if(rep)
                R_RCX = tmp64u;
            break;
        case 0xA5:              /* (REP) MOVSD */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rep)?R_RCX:1L;
            if(rex.w) {
                tmp8s *= 8;
                while(tmp64u) {
                    --tmp64u;
                    #ifndef TEST_INTERPRETER
                    *(uint64_t*)R_RDI = *(uint64_t*)R_RSI;
                    #endif
                    R_RDI += tmp8s;
                    R_RSI += tmp8s;
                }
            } else {
                tmp8s *= 4;
                while(tmp64u) {
                    --tmp64u;
                    #ifndef TEST_INTERPRETER
                    *(uint32_t*)R_RDI = *(uint32_t*)R_RSI;
                    #endif
                    R_RDI += tmp8s;
                    R_RSI += tmp8s;
                }
            }
            if(rep)
                R_RCX = tmp64u;
            break;
        case 0xA6:                      /* (REPZ/REPNE) CMPSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            switch(rep) {
                case 1:
                    tmp64u = R_RCX;
                    while(tmp64u) {
                        --tmp64u;
                        tmp8u  = *(uint8_t*)R_RDI;
                        tmp8u2 = *(uint8_t*)R_RSI;
                        R_RDI += tmp8s;
                        R_RSI += tmp8s;
                        if(tmp8u==tmp8u2)
                            break;
                    }
                    if(R_RCX) cmp8(emu, tmp8u2, tmp8u);
                    R_RCX = tmp64u;
                    break;
                case 2:
                    tmp64u = R_RCX;
                    while(tmp64u) {
                        --tmp64u;
                    tmp8u  = *(uint8_t*)R_RDI;
                    tmp8u2 = *(uint8_t*)R_RSI;
                    R_RDI += tmp8s;
                    R_RSI += tmp8s;
                    if(tmp8u!=tmp8u2)
                        break;
                    }
                    if(R_RCX) cmp8(emu, tmp8u2, tmp8u);
                    R_RCX = tmp64u;
                    break;
                default:
                    tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                    tmp8u  = *(uint8_t*)R_RDI;
                    tmp8u2 = *(uint8_t*)R_RSI;
                    R_RDI += tmp8s;
                    R_RSI += tmp8s;
                    cmp8(emu, tmp8u2, tmp8u);
            }
            break;
        case 0xA7:                      /* (REPZ/REPNE) CMPSD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            switch(rep) {
                case 1:
                    tmp64u = R_RCX;
                    if(rex.w) {
                        while(tmp64u) {
                            --tmp64u;
                            tmp64u3 = *(uint64_t*)R_RDI;
                            tmp64u2 = *(uint64_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp64u3==tmp64u2)
                                break;
                        }
                        if(R_RCX) cmp64(emu, tmp64u2, tmp64u3);
                    } else {
                        while(tmp64u) {
                            --tmp64u;
                            tmp32u  = *(uint32_t*)R_RDI;
                            tmp32u2 = *(uint32_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp32u==tmp32u2)
                                break;
                        }
                        if(R_RCX) cmp32(emu, tmp32u2, tmp32u);
                    }
                    R_RCX = tmp64u;
                    break;
                case 2:
                    tmp64u = R_RCX;
                    if(rex.w) {
                        while(tmp64u) {
                            --tmp64u;
                            tmp64u3 = *(uint64_t*)R_RDI;
                            tmp64u2 = *(uint64_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp64u3!=tmp64u2)
                                break;
                        }
                        if(R_RCX) cmp64(emu, tmp64u2, tmp64u3);
                    } else {
                        while(tmp64u) {
                            --tmp64u;
                            tmp32u  = *(uint32_t*)R_RDI;
                            tmp32u2 = *(uint32_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp32u!=tmp32u2)
                                break;
                        }
                        if(R_RCX) cmp32(emu, tmp32u2, tmp32u);
                    }
                    R_RCX = tmp64u;
                    break;
                default:
                    if(rex.w) {
                        tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
                        tmp64u  = *(uint64_t*)R_RDI;
                        tmp64u2 = *(uint64_t*)R_RSI;
                        R_RDI += tmp8s;
                        R_RSI += tmp8s;
                        cmp64(emu, tmp64u2, tmp64u);
                    } else {
                        tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                        tmp32u  = *(uint32_t*)R_RDI;
                        tmp32u2 = *(uint32_t*)R_RSI;
                        R_RDI += tmp8s;
                        R_RSI += tmp8s;
                        cmp32(emu, tmp32u2, tmp32u);
                    }
            }
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

        case 0xAA:                      /* (REP) STOSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rep)?R_RCX:1L;
            while(tmp64u) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_RDI = R_AL;
                #endif
                R_RDI += tmp8s;
                --tmp64u;
            }
            if(rep)
                R_RCX = tmp64u;
            break;
        case 0xAB:                      /* (REP) STOSD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            tmp64u = (rep)?R_RCX:1L;
            if((rex.w))
                while(tmp64u) {
                    #ifndef TEST_INTERPRETER
                    *(uint64_t*)R_RDI = R_RAX;
                    #endif
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
        case 0xAC:                      /* LODSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rep)?R_RCX:1L;
            while(tmp64u) {
                R_AL = *(uint8_t*)R_RSI;
                R_RSI += tmp8s;
                --tmp64u;
            }
            if(rep)
                R_RCX = tmp64u;
            break;
        case 0xAD:                      /* (REP) LODSD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            tmp64u = (rep)?R_RCX:1L;
            if((rex.w))
                while(tmp64u) {
                    R_RAX = *(uint64_t*)R_RSI;
                    R_RSI += tmp8s;
                    --tmp64u;
                }
            else
                while(tmp64u) {
                    R_RAX = *(uint32_t*)R_RSI;
                    R_RSI += tmp8s;
                    --tmp64u;
                }
            if(rep)
                R_RCX = tmp64u;
            break;
        case 0xAE:                      /* (REPZ/REPNE) SCASB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            switch(rep) {
                case 1:
                    tmp64u = R_RCX;
                    while(tmp64u) {
                        --tmp64u;
                        tmp8u = *(uint8_t*)R_RDI;
                        R_RDI += tmp8s;
                        if(R_AL==tmp8u)
                            break;
                    }
                    if(R_RCX) cmp8(emu, R_AL, tmp8u);
                    R_RCX = tmp64u;
                    break;
                case 2:
                    tmp64u = R_RCX;
                    while(tmp64u) {
                        --tmp64u;
                        tmp8u = *(uint8_t*)R_RDI;
                        R_EDI += tmp8s;
                        if(R_AL!=tmp8u)
                            break;
                    }
                    if(R_RCX) cmp8(emu, R_AL, tmp8u);
                    R_RCX = tmp64u;
                    break;
                default:
                    cmp8(emu, R_AL, *(uint8_t*)R_RDI);
                    R_RDI += tmp8s;
            }
            break;
        case 0xAF:                      /* (REPZ/REPNE) SCASD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            switch(rep) {
                case 1:
                    tmp64u = R_RCX;
                    if(rex.w) {
                        while(tmp64u) {
                            --tmp64u;
                            tmp64u2 = *(uint64_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_RAX==tmp64u2)
                                break;
                        }
                        if(R_RCX) cmp64(emu, R_RAX, tmp64u2);
                    } else {
                        while(tmp64u) {
                            --tmp64u;
                            tmp32u = *(uint32_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_EAX==tmp32u)
                                break;
                        }
                        if(R_RCX) cmp32(emu, R_EAX, tmp32u);
                    }
                    R_RCX = tmp64u;
                    break;
                case 2:
                    tmp64u = R_RCX;
                    if(rex.w) {
                        while(tmp64u) {
                            --tmp64u;
                            tmp64u2 = *(uint64_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_RAX!=tmp64u2)
                                break;
                        }
                        if(R_RCX) cmp64(emu, R_RAX, tmp64u2);
                    } else {
                        while(tmp64u) {
                            --tmp64u;
                            tmp32u = *(uint32_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_EAX!=tmp32u)
                                break;
                        }
                        if(R_RCX) cmp32(emu, R_EAX, tmp32u);
                    }
                    R_RCX = tmp64u;
                    break;
                default:
                    if(rex.w)
                        cmp64(emu, R_RAX, *(uint64_t*)R_RDI);
                    else
                        cmp32(emu, R_EAX, *(uint32_t*)R_RDI);
                    R_RDI += tmp8s;
            }
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
        case 0xC0:                      /* GRP2 Eb,Ib */
            nextop = F8;
            GETEB(1);
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4:
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
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
        case 0xC2:                      /* RETN Iw */
            tmp16u = F16;
            addr = Pop(emu);
            R_RSP += tmp16u;
            STEP2
            break;
        case 0xC3:                      /* RET */
            addr = Pop(emu);
            STEP2
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
        case 0xC8:                      /* ENTER Iw,Ib */
            tmp16u = F16;
            tmp8u = (F8) & 0x1f;
            tmp64u = R_RBP;
            Push(emu, R_RBP);
            R_RBP = R_RSP;
            if (tmp8u) {
                for (tmp8u2 = 1; tmp8u2 < tmp8u; tmp8u2++) {
                    tmp64u -= sizeof(void*);
                    Push(emu, *((uintptr_t*)tmp64u));
                }
                Push(emu, R_RBP);
            }
            R_RSP -= tmp16u;
            break;
        case 0xC9:                      /* LEAVE */
            R_RSP = R_RBP;
            R_RBP = Pop(emu);
            break;

        case 0xCC:                      /* INT 3 */
            #ifndef TEST_INTERPRETER
            x64Int3(emu, &addr);
            if(emu->quit) goto fini;    // R_RIP is up to date when returning from x64Int3
            addr = R_RIP;
            #endif
            break;
        case 0xCD:                      /* INT n */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            #endif
            break;


        case 0xCF:                      /* IRET */
            addr = Pop(emu);
            emu->segs[_CS] = Pop(emu)&0xffff;
            emu->segs_serial[_CS] = 0;
            emu->eflags.x64 = ((Pop(emu) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
            tmp64u = Pop(emu);  //RSP
            emu->segs[_SS] = Pop(emu)&0xffff;
            emu->segs_serial[_SS] = 0;
            R_RSP= tmp64u;
            RESET_FLAGS(emu);
            R_RIP = addr;
            goto fini;      // exit, to recompute CS if needed
            break;
        case 0xD0:                      /* GRP2 Eb,1 */
        case 0xD2:                      /* GRP2 Eb,CL */
            nextop = F8;
            GETEB(0);
            tmp8u = (opcode==0xD0)?1:R_CL;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4: 
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
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

        case 0xD7:                      /* XLAT */
            R_AL = *(uint8_t*)(R_RBX + R_AL);
            break;
        case 0xD8:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestD8(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunD8(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xD9:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestD9(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunD9(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDA:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDA(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDA(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDB:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDB(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDB(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDC:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDC(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDC(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDD:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDD(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDD(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDE:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDE(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDE(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xDF:                      /* x87 opcodes */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestDF(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunDF(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;
        case 0xE0:                      /* LOOPNZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_RCX; // don't update flags
            if(R_RCX && !ACCESS_FLAG(F_ZF))
                addr += tmp8s;
            STEP2
            break;
        case 0xE1:                      /* LOOPZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_RCX; // don't update flags
            if(R_RCX && ACCESS_FLAG(F_ZF))
                addr += tmp8s;
            STEP2
            break;
        case 0xE2:                      /* LOOP */
            tmp8s = F8S;
            --R_RCX; // don't update flags
            if(R_RCX)
                addr += tmp8s;
            STEP2
            break;
        case 0xE3:                      /* JRCXZ */
            tmp8s = F8S;
            if(!R_RCX)
                addr += tmp8s;
            STEP2
            break;
        case 0xE4:                      /* IN AL, XX */
        case 0xE5:                      /* IN EAX, XX */
        case 0xE6:                      /* OUT XX, AL */
        case 0xE7:                      /* OUT XX, EAX */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            #endif
            break;
        case 0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            Push(emu, addr);
            addr += tmp32s;
            STEP2
            break;
        case 0xE9:                      /* JMP Id */
            tmp32s = F32S; // jmp is relative
            addr += tmp32s;
            STEP2
            break;

        case 0xEB:                      /* JMP Ib */
            tmp32s = F8S; // jump is relative
            addr += tmp32s;
            STEP2
            break;
        case 0xEC:                      /* IN AL, DX */
        case 0xED:                      /* IN EAX, DX */
        case 0xEE:                      /* OUT DX, AL */
        case 0xEF:                      /* OUT DX, EAX */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            #endif
            break;
        case 0xF0:                      /* LOCK prefix */
            #ifdef TEST_INTERPRETER
            if(!(addr = TestF0(test, rex, addr)))
                unimp = 1;
            #else
            if(!(addr = RunF0(emu, rex, addr))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            #endif
            break;

        case 0xF4:                      /* HLT */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            #endif
            break;
        case 0xF5:                      /* CMC */
            CHECK_FLAGS(emu);
            CONDITIONAL_SET_FLAG(!ACCESS_FLAG(F_CF), F_CF);
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
                        emu->regs[_DX].dword[1] = 0;
                        break;
                    case 5:                 /* IMUL EAX,Ed */
                        imul32_eax(emu, ED->dword[0]);
                        emu->regs[_AX].dword[1] = 0;
                        emu->regs[_DX].dword[1] = 0;
                        break;
                    case 6:                 /* DIV Ed */
                        div32(emu, ED->dword[0]);
                        //emu->regs[_AX].dword[1] = 0;  // already put high regs to 0
                        //emu->regs[_DX].dword[1] = 0;
                        break;
                    case 7:                 /* IDIV Ed */
                        idiv32(emu, ED->dword[0]);
                        //emu->regs[_AX].dword[1] = 0;
                        //emu->regs[_DX].dword[1] = 0;
                        break;
                }
            }
            break;
        case 0xF8:                      /* CLC */
            CHECK_FLAGS(emu);
            CLEAR_FLAG(F_CF);
            break;
        case 0xF9:                      /* STC */
            CHECK_FLAGS(emu);
            SET_FLAG(F_CF);
            break;
        case 0xFA:                      /* CLI */
            // this is a privilege opcode...
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            break;
        case 0xFB:                      /* STI */
            // this is a privilege opcode...
            emit_signal(emu, SIGSEGV, (void*)R_RIP, 0);
            STEP;
            break;
        case 0xFC:                      /* CLD */
            CLEAR_FLAG(F_DF);
            break;
        case 0xFD:                      /* STD */
            SET_FLAG(F_DF);
            break;
        case 0xFE:                      /* GRP 5 Eb */
            nextop = F8;
            GETEB(0);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Eb */
                    ED->byte[0] = inc8(emu, ED->byte[0]);
                    break;
                case 1:                 /* DEC Ed */
                    ED->byte[0] = dec8(emu, ED->byte[0]);
                    break;
                default:
                    unimp = 1;
                    goto fini;
            }
            break;
        case 0xFF:                      /* GRP 5 Ed */
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    GETED(0);
                    if(rex.w)
                        ED->q[0] = inc64(emu, ED->q[0]);
                    else {
                        if(MODREG)
                            ED->q[0] = inc32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = inc32(emu, ED->dword[0]);
                    }
                    break;
                case 1:                 /* DEC Ed */
                    GETED(0);
                    if(rex.w)
                        ED->q[0] = dec64(emu, ED->q[0]);
                    else {
                        if(MODREG)
                            ED->q[0] = dec32(emu, ED->dword[0]);
                        else
                            ED->dword[0] = dec32(emu, ED->dword[0]);
                    }
                    break;
                case 2:                 /* CALL NEAR Ed */
                    GETE8(0);
                    tmp64u = (uintptr_t)getAlternate((void*)ED->q[0]);
                    Push(emu, addr);
                    addr = tmp64u;
                    STEP2
                    break;
                case 3:                 /* CALL FAR Ed */
                    GETET(0);
                    if(MODREG) {
                        printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        Push(emu, R_CS);
                        Push(emu, addr);
                        R_RIP = addr = (uintptr_t)getAlternate((void*)ED->q[0]);   // check CS?
                        R_CS = (ED+1)->word[0];
                        goto fini;  // exit loop to recompute new CS...
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    GETE8(0);
                    addr = (uintptr_t)getAlternate((void*)ED->q[0]);
                    STEP2
                    break;
                case 5:                 /* JMP FAR Ed */
                    GETET(0);
                    if(MODREG) {
                        printf_log(LOG_NONE, "Illegal Opcode %p: 0x%02X 0x%02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        R_RIP = (uintptr_t)getAlternate((void*)ED->q[0]);   //check CS?
                        R_CS = (ED+1)->word[0];
                        goto fini;  // exit loop to recompute CS...
                    }
                    break;
                case 6:                 /* Push Ed */
                    GETE8(0);
                    tmp64u = ED->q[0];  // rex.w ignored
                    #ifdef TEST_INTERPRETER
                    R_RSP -=8;
                    if(test->memsize!=8)
                        *(uint64_t*)test->mem = *(uint64_t*)test->memaddr;
                    test->memsize = 8;
                    test->memaddr = R_RSP;
                    #else
                    Push(emu, tmp64u);  // avoid potential issue with push [esp+...]
                    #endif
                    break;
                default:
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
        R_RIP = addr;
    }


fini:
#ifndef TEST_INTERPRETER
    printf_log(LOG_DEBUG, "End of X86 run (%p), RIP=%p, Stack=%p, unimp=%d, emu->fork=%d, emu->uc_link=%p, emu->quit=%d\n", emu, (void*)R_RIP, (void*)R_RSP, unimp, emu->fork, emu->uc_link, emu->quit);
    if(unimp) {
        emu->quit = 1;
        UnimpOpcode(emu);
    }
    // fork handling
    if(emu->fork) {
        addr = R_RIP;
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
        addr = R_RIP;
        goto x64emurun;
    }
#else
    if(unimp) {
        printf_log(LOG_INFO, "Warning, inimplemented opcode in Test Interpreter\n");
    } else
        addr = R_RIP;
#endif
    return 0;
}
