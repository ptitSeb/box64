#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64stack.h"
#include "box64cpu_util.h"
#include "x64emu.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "alternate.h"
#include "emit_signals.h"
#include "mysignal.h"
#ifdef DYNAREC
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
int RunTest(x64test_t *test)
#else
int running32bits = 0;
int Run(x64emu_t *emu, int step, int need_tf)
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
    rex_t rex = {0};
    int unimp = 0;
    int is32bits = (emu->segs[_CS]==0x23);
    int no_tf = 0;
    int tf = ACCESS_FLAG(F_TF);

    if(emu->quit)
        return 0;
    if(addr==0) {
        // Some programs, like VB6 VARA.exe, need to trigger that segfault to actually run... (ticket #830 in box86)
        printf_log(LOG_INFO, "%04d|Ask to run at NULL, will segfault\n", GetTID());
    }
    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p is32bits=%d\n", emu, (void*)addr, (void*)R_RSP, is32bits);

#ifdef TEST_INTERPRETER
    test->memsize = 0;
#else
    CheckExec(emu, R_RIP);
x64emurun:
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

        #ifndef TEST_INTERPRETER
        // check the TRACE flag before going to next
        if(tf) {
            if(no_tf)
                no_tf = 0;
            else {
                R_RIP = addr;
                EmitSignal(emu, X64_SIGTRAP, (void*)addr, 1);
                if(emu->quit) goto fini;
            }
        }
#endif

        opcode = F8;
        
        rex.rex = 0;
        rex.seg = 0;
        rex.offset = 0;
        rex.is32bits = is32bits;
        rex.is66 = 0;
        rex.is67 = 0;
        rex.rep = 0;
        while((opcode==0xF2) || (opcode==0xF3) 
            || (opcode==0x3E) || (opcode==0x26) || (opcode==0x2e) || (opcode==0x36) 
            || (opcode==0x64) || (opcode==0x65) || (opcode==0x66) || (opcode==0x67)
            || (!is32bits && (opcode>=0x40 && opcode<=0x4f))) {
            switch (opcode) {
                case 0xF2: rex.rep = 1; rex.rex = 0; break;
                case 0xF3: rex.rep = 2; rex.rex = 0; break;
                case 0x26: /* ES: */
                case 0x2E: /* CS: */
                case 0x36: /* SS; */
                case 0x3E: /* DS; */ 
                           rex.seg =   0; rex.rex = 0; break;
                case 0x64: rex.seg = _FS; rex.rex = 0; break;
                case 0x65: rex.seg = _GS; rex.rex = 0; break;
                case 0x66: rex.is66 = 1; rex.rex = 0; break;
                case 0x67: rex.is67 = 1; rex.rex = 0; break;
                case 0x40 ... 0x4F: rex.rex = opcode; break;
            }
            opcode = F8;
        }

        if(rex.seg)
            rex.offset = GetSegmentBaseEmu(emu, rex.seg);
        
        if(rex.is66) {
            /* 16bits prefix */
            #ifdef TEST_INTERPRETER
            if(!(addr = Test66(test, rex, addr-1)))
                unimp = 1;
            #else
            if(!(addr = Run66(emu, rex, addr-1))) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit) {
                R_RIP = addr;
                goto fini;
            }
            tf = ACCESS_FLAG(F_TF);
            #endif
        } else
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
        case 0x06:                      /* PUSH ES */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            Push32(emu, emu->segs[_ES]);  // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x07:                      /* POP ES */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            emu->segs[_ES] = Pop32(emu);    // no check, no use....
            emu->segs_serial[_ES] = 0;
            break;
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        case 0x0E:                      /* PUSH CS */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            Push32(emu, emu->segs[_CS]);  // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x0F:                      /* More instructions */
            switch(rex.rep) {
                case 1:
                    #ifdef TEST_INTERPRETER 
                    if(!(addr = TestF20F(test, rex, addr, &step)))
                        unimp = 1;
                    #else
                    if(!(addr = RunF20F(emu, rex, addr, &step))) {
                        unimp = 1;
                        goto fini;
                    }
                    if(step==2) {
                        STEP2;
                    }
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
                    if(step==2) {
                        STEP2;
                    }
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

        case 0x16:                      /* PUSH SS */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            Push32(emu, emu->segs[_SS]);  // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x17:                      /* POP SS */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            emu->segs[_SS] = Pop32(emu);    // no check, no use....
            emu->segs_serial[_SS] = 0;
            tf = 0;
            break;

        case 0x1E:                      /* PUSH DS */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            Push32(emu, emu->segs[_DS]);  // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0x1F:                      /* POP DS */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            emu->segs[_DS] = Pop32(emu);    // no check, no use....
            emu->segs_serial[_DS] = 0;
            break;

        case 0x27:                  /* DAA */
            if(rex.is32bits) {
                R_AL = daa8(emu, R_AL);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x2F:                  /* DAS */
            if(rex.is32bits) {
                R_AL = das8(emu, R_AL);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x2E:          /* segments are ignored */
        case 0x26:
        case 0x36:          /* SS: (ignored) */
            break;
        case 0x37:                  /* AAA */
            if(rex.is32bits) {
                R_AX = aaa16(emu, R_AX);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x38:
            nextop = F8;
            _GETEB(0);
            GETGB;
            cmp8(emu, EB->byte[0], GB);
            break;
        case 0x39:
            nextop = F8;
            _GETED(0);
            GETGD;
            if(rex.w)
                cmp64(emu, ED->q[0], GD->q[0]);
            else
                cmp32(emu, ED->dword[0], GD->dword[0]);
            break;
        case 0x3A:
            nextop = F8;
            _GETEB(0);
            GETGB;
            cmp8(emu, GB, EB->byte[0]);
            break;
        case 0x3B:
            nextop = F8;
            _GETED(0);
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

        case 0x3F:                  /* AAS */
            if(rex.is32bits) {
                R_AX = aas16(emu, R_AX);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:                      /* INC Reg (32bits only)*/
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = inc32(emu, emu->regs[tmp8u].dword[0]);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:                      /* DEC Reg (32bits only)*/
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = dec32(emu, emu->regs[tmp8u].dword[0]);
            break;
        case 0x54:                      /* PUSH ESP */
            if(rex.b)
                Push64(emu, R_R12);
            else {
                if(rex.is32bits) {
                    tmp32u = R_ESP;
                    Push32(emu, tmp32u);
                } else {
                    tmp64u = R_RSP;
                    Push64(emu, tmp64u);
                }
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
            if(rex.is32bits)
                Push32(emu, emu->regs[tmp8u].dword[0]);
            else
                Push64(emu, emu->regs[tmp8u].q[0]);
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
            emu->regs[tmp8u].q[0] = rex.is32bits?Pop32(emu):Pop64(emu);
            break;
        case 0x60:                      /* PUSHAD */
            if(rex.is32bits) {
                tmp32u = R_ESP;
                Push32(emu, R_EAX);
                Push32(emu, R_ECX);
                Push32(emu, R_EDX);
                Push32(emu, R_EBX);
                Push32(emu, tmp32u);
                Push32(emu, R_EBP);
                Push32(emu, R_ESI);
                Push32(emu, R_EDI);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x61:                      /* POPAD */
            if(rex.is32bits) {
                R_EDI = Pop32(emu);
                R_ESI = Pop32(emu);
                R_EBP = Pop32(emu);
                R_ESP+=4;   // POP ESP
                R_EBX = Pop32(emu);
                R_EDX = Pop32(emu);
                R_ECX = Pop32(emu);
                R_EAX = Pop32(emu);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x62:                  /* BOUND Gd, Ed */
            nextop = F8;
            if(rex.is32bits && MODREG) {
                GETGD;
                int* bounds = (int*)GETEA(0);
                if(bounds[0]<GD->dword[0] || bounds[1]>GD->dword[0])
                    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xb09d);
            } else {
                unimp = 2;
                goto fini;
            }
            break;
        case 0x63:                      /* MOVSXD Gd,Ed */
            nextop = F8;
            GETE4(0);
            GETGD;
            if(rex.is32bits) {
                // ARPL here
                // faking to always happy...
                SET_FLAG(F_ZF);
            } else {
                if(rex.w)
                    GD->sq[0] = ED->sdword[0];
                else
                    if(MODREG)
                        GD->q[0] = ED->dword[0];    // not really a sign extension
                    else
                        GD->sdword[0] = ED->sdword[0];  // meh?
            }
            break;

        case 0x68:                      /* Push Id */
            if(rex.is32bits)
                Push32(emu, F32);
            else
                Push64(emu, F32S64);
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
            if(rex.is32bits) {
                tmp32s = F8S;
                Push32(emu, (uint32_t)tmp32s);
            } else {
                tmp64s = F8S;
                Push64(emu, (uint64_t)tmp64s);
            }
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
        case 0x6D:                      /* INSD DX */
        case 0x6E:                      /* OUTSB DX */
        case 0x6F:                      /* OUTSD DX */
#ifndef TEST_INTERPRETER
            if(rex.is32bits && BOX64ENV(ignoreint3))
            {
            } else {
                EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
            }
            STEP;
            #endif
            break;

        GOCOND(0x70
            ,   tmp8s = F8S; CHECK_FLAGS(emu);
            ,   addr += tmp8s;
            ,,STEP2
            )                           /* Jxx Ib */
        
        case 0x82:
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            // fallthru
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
                        // not aligned, dont't try to "LOCK"
                        tmp64u = ED->q[0];
                        ED->q[0] = GD->q[0];
                        GD->q[0] = tmp64u;
                    } else
                        GD->q[0] = native_lock_xchg_dd(ED, GD->q[0]);
                } else {
                    if((uintptr_t)ED&3) {
                        // not aligned, dont't try to "LOCK"
                        tmp32u = ED->dword[0];
                        ED->dword[0] = GD->dword[0];
                        GD->q[0] = tmp32u;
                    } else
                        GD->q[0] = native_lock_xchg_d(ED, GD->dword[0]);
                }
            }
#else
            GETED(0);
            GETGD;
            if(!MODREG)
                pthread_mutex_lock(&my_context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
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
                pthread_mutex_unlock(&my_context->mutex_lock);
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
            _GETEB(0);
            GETGB;
            GB = EB->byte[0];
            break;
        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            _GETED(0);
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
        case 0x8E:                      /* MOV Seg, Ew */
            nextop = F8;
            GETED(0);
            emu->segs[((nextop&0x38)>>3)] = ED->word[0];
            emu->segs_serial[((nextop&0x38)>>3)] = 0;
            if(((nextop&0x38)>>3)==_SS && tf)   // disable trace when SS is accessed
                no_tf = 1;
            break;
        case 0x8F:                      /* POP Ed */
            nextop = F8;
            if(MODREG) {
                emu->regs[(nextop&7)+(rex.b<<3)].q[0] = rex.is32bits?Pop32(emu):Pop64(emu);
            } else {
                if(rex.is32bits) {
                    tmp32u = Pop32(emu);  // this order allows handling POP [ESP] and variant
                    GETED(0);
                    R_ESP -= 4; // to prevent issue with SEGFAULT
                    ED->dword[0] = tmp32u;
                    R_ESP += 4;
                } else {
                    tmp64u = Pop64(emu);  // this order allows handling POP [ESP] and variant
                    GETED(0);
                    R_RSP -= sizeof(void*); // to prevent issue with SEGFAULT
                    ED->q[0] = tmp64u;
                    R_RSP += sizeof(void*);
                }
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
        case 0x9A:                      /* CALL FAR seg:off*/
            if(is32bits) {
                uint16_t new_cs = F16;
                uint32_t new_addr = F32;
                Push32(emu, emu->segs[_CS]);
                Push32(emu, addr);
                #ifndef TEST_INTERPRETER
                if((new_cs&3)!=3) {
                    // R_RIP doesn't advance
                    printf_log(LOG_INFO, "Warning, unexpected new_cs=0x%x\n", new_cs);
                    R_RSP-=(rex.w?4:8)*2;
                    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0); // GP if trying to change priv level
                    goto fini;
                }
                emu->segs[_CS] = new_cs;
                emu->segs_serial[_CS] = 0;
                R_RIP = new_addr;
                if(is32bits!=(emu->segs[_CS]==0x23)) {
                    is32bits = (emu->segs[_CS]==0x23);
                    if(is32bits) {
                        // Zero upper part of the 32bits regs
                        R_RAX = R_EAX;
                        R_RBX = R_EBX;
                        R_RCX = R_ECX;
                        R_RDX = R_EDX;
                        R_RSP = R_ESP;
                        R_RBP = R_EBP;
                        R_RSI = R_ESI;
                        R_RDI = R_EDI;
                    }
                    #ifndef TEST_INTERPRETER
                    if(is32bits)
                        running32bits = 1;
                    #endif
                }
                #endif
            } else {
                unimp = 2;
                goto fini;
            }
            STEP;
        case 0x9B:                      /* FWAIT */
            break;
        case 0x9C:                      /* PUSHF */
            CHECK_FLAGS(emu);
            if(rex.is32bits)
                Push32(emu, emu->eflags.x64);
            else
                Push64(emu, emu->eflags.x64);
            break;
        case 0x9D:                      /* POPF */
            emu->eflags.x64 = (((rex.is32bits?Pop32(emu):Pop64(emu)) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x202; // mask off res2 and res3 and on res1
            RESET_FLAGS(emu);
            tf = ACCESS_FLAG(F_TF);
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
            if(rex.is32bits && rex.is67)
                R_AL = *(uint8_t*)(uintptr_t)(ptr_t)(rex.offset+F16S);
            else if(rex.is32bits || rex.is67)
                R_AL = *(uint8_t*)(uintptr_t)(ptr_t)(F32+rex.offset);
            else
                R_AL = *(uint8_t*)(F64+rex.offset);
            break;
        case 0xA1:                      /* MOV EAX,Od */
            if(rex.is32bits && rex.is67)
                R_EAX = *(uint32_t*)(uintptr_t)(ptr_t)(rex.offset+F16S);
            else if(rex.is32bits || rex.is67) {
                if(rex.w)
                    R_RAX = *(int64_t*)(uintptr_t)(ptr_t)(F32+rex.offset);
                else {
                    R_EAX = *(int32_t*)(uintptr_t)(ptr_t)(F32+rex.offset);
                    if(!rex.is32bits)
                        R_RAX = R_EAX;
                }
            } else {
                if(rex.w)
                    R_RAX = *(uint64_t*)(F64+rex.offset);
                else
                    R_RAX = *(uint32_t*)(F64+rex.offset);
            }
            break;
        case 0xA2:                      /* MOV Ob,AL */
            if(rex.is32bits && rex.is67)
                *(uint8_t*)(uintptr_t)(ptr_t)(rex.offset+F16S) = R_AL;
            else if(rex.is32bits || rex.is67)
                *(uint8_t*)(uintptr_t)(ptr_t)(F32+rex.offset) = R_AL;
            else
                *(uint8_t*)(F64+rex.offset) = R_AL;
            break;
        case 0xA3:                      /* MOV Od,EAX */
            if(rex.is32bits && rex.is67)
                *(uint32_t*)(uintptr_t)(ptr_t)(rex.offset+F16S) = R_EAX;
            else if(rex.is32bits || rex.is67) {
                if(rex.w)
                    *(uint64_t*)(uintptr_t)(ptr_t)(F32+rex.offset) = R_RAX;
                else
                    *(uint32_t*)(uintptr_t)(ptr_t)(F32+rex.offset) = R_EAX;
            } else {
                if(rex.w)
                    *(uint64_t*)(F64+rex.offset) = R_RAX;
                else
                    *(uint32_t*)(F64+rex.offset) = R_EAX;
            }
            break;
        case 0xA4:                      /* MOVSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rex.rep)?R_RCX:1L;
            while(tmp64u) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_RDI = *(uint8_t*)R_RSI;
                #endif
                R_RDI += tmp8s;
                R_RSI += tmp8s;
                --tmp64u;
            }
            if(rex.rep)
                R_RCX = tmp64u;
            break;
        case 0xA5:              /* (REP) MOVSD */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rex.rep)?R_RCX:1L;
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
            if(rex.rep)
                R_RCX = tmp64u;
            break;
        case 0xA6:                      /* (REPZ/REPNE) CMPSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            switch(rex.rep) {
                case 1:
                    if(R_RCX) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp8u  = *(uint8_t*)R_RDI;
                            tmp8u2 = *(uint8_t*)R_RSI;
                            R_RDI += tmp8s;
                            R_RSI += tmp8s;
                            if(tmp8u==tmp8u2)
                                break;
                        }
                        cmp8(emu, tmp8u2, tmp8u);
                    }
                    break;
                case 2:
                    if(R_RCX) {
                        while(R_RCX) {
                            --R_RCX;
                        tmp8u  = *(uint8_t*)R_RDI;
                        tmp8u2 = *(uint8_t*)R_RSI;
                        R_RDI += tmp8s;
                        R_RSI += tmp8s;
                        if(tmp8u!=tmp8u2)
                            break;
                        }
                        cmp8(emu, tmp8u2, tmp8u);
                    }
                    break;
                default:
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
            switch(rex.rep) {
                case 1:
                    if(R_RCX) {
                        if(rex.w) {
                            while(R_RCX) {
                                --R_RCX;
                                tmp64u3 = *(uint64_t*)R_RDI;
                                tmp64u2 = *(uint64_t*)R_RSI;
                                R_RDI += tmp8s;
                                R_RSI += tmp8s;
                                if(tmp64u3==tmp64u2)
                                    break;
                            }
                            cmp64(emu, tmp64u2, tmp64u3);
                        } else {
                            while(R_RCX) {
                                --R_RCX;
                                tmp32u  = *(uint32_t*)R_RDI;
                                tmp32u2 = *(uint32_t*)R_RSI;
                                R_RDI += tmp8s;
                                R_RSI += tmp8s;
                                if(tmp32u==tmp32u2)
                                    break;
                            }
                            cmp32(emu, tmp32u2, tmp32u);
                        }
                    }
                    break;
                case 2:
                    if(R_RCX) {
                        if(rex.w) {
                            while(R_RCX) {
                                --R_RCX;
                                tmp64u3 = *(uint64_t*)R_RDI;
                                tmp64u2 = *(uint64_t*)R_RSI;
                                R_RDI += tmp8s;
                                R_RSI += tmp8s;
                                if(tmp64u3!=tmp64u2)
                                    break;
                            }
                            cmp64(emu, tmp64u2, tmp64u3);
                        } else {
                            while(R_RCX) {
                                --R_RCX;
                                tmp32u  = *(uint32_t*)R_RDI;
                                tmp32u2 = *(uint32_t*)R_RSI;
                                R_RDI += tmp8s;
                                R_RSI += tmp8s;
                                if(tmp32u!=tmp32u2)
                                    break;
                            }
                            cmp32(emu, tmp32u2, tmp32u);
                        }
                    }
                    break;
                default:
                    if(rex.w) {
                        tmp64u  = *(uint64_t*)R_RDI;
                        tmp64u2 = *(uint64_t*)R_RSI;
                        R_RDI += tmp8s;
                        R_RSI += tmp8s;
                        cmp64(emu, tmp64u2, tmp64u);
                    } else {
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
            tmp64u = (rex.rep)?R_RCX:1L;
            while(tmp64u) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_RDI = R_AL;
                #endif
                R_RDI += tmp8s;
                --tmp64u;
            }
            if(rex.rep)
                R_RCX = tmp64u;
            break;
        case 0xAB:                      /* (REP) STOSD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            tmp64u = (rex.rep)?R_RCX:1L;
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
            if(rex.rep)
                R_RCX = tmp64u;
            break;
        case 0xAC:                      /* LODSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp64u = (rex.rep)?R_RCX:1L;
            while(tmp64u) {
                R_AL = *(uint8_t*)R_RSI;
                R_RSI += tmp8s;
                --tmp64u;
            }
            if(rex.rep)
                R_RCX = tmp64u;
            break;
        case 0xAD:                      /* (REP) LODSD */
            if(rex.w)
                tmp8s = ACCESS_FLAG(F_DF)?-8:+8;
            else
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
            tmp64u = (rex.rep)?R_RCX:1L;
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
            if(rex.rep)
                R_RCX = tmp64u;
            break;
        case 0xAE:                      /* (REPZ/REPNE) SCASB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            switch(rex.rep) {
                case 1:
                    if(R_RCX) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp8u = *(uint8_t*)R_RDI;
                            R_RDI += tmp8s;
                            if(R_AL==tmp8u)
                                break;
                        }
                        cmp8(emu, R_AL, tmp8u);
                    }
                    break;
                case 2:
                    if(R_RCX) {
                        while(R_RCX) {
                            --R_RCX;
                            tmp8u = *(uint8_t*)R_RDI;
                            R_EDI += tmp8s;
                            if(R_AL!=tmp8u)
                                break;
                        }
                        if(R_RCX) cmp8(emu, R_AL, tmp8u);
                    }
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
            switch(rex.rep) {
                case 1:
                    if(R_RCX) {
                        if(rex.w) {
                            while(R_RCX) {
                                --R_RCX;
                                tmp64u2 = *(uint64_t*)R_RDI;
                                R_RDI += tmp8s;
                                if(R_RAX==tmp64u2)
                                    break;
                            }
                            cmp64(emu, R_RAX, tmp64u2);
                        } else {
                            while(R_RCX) {
                                --R_RCX;
                                tmp32u = *(uint32_t*)R_RDI;
                                R_RDI += tmp8s;
                                if(R_EAX==tmp32u)
                                    break;
                            }
                            cmp32(emu, R_EAX, tmp32u);
                        }
                    }
                    break;
                case 2:
                    if(R_RCX) {
                        if(rex.w) {
                            while(R_RCX) {
                                --R_RCX;
                                tmp64u2 = *(uint64_t*)R_RDI;
                                R_RDI += tmp8s;
                                if(R_RAX!=tmp64u2)
                                    break;
                            }
                            cmp64(emu, R_RAX, tmp64u2);
                        } else {
                            while(R_RCX) {
                                --R_RCX;
                                tmp32u = *(uint32_t*)R_RDI;
                                R_RDI += tmp8s;
                                if(R_EAX!=tmp32u)
                                    break;
                            }
                            cmp32(emu, R_EAX, tmp32u);
                        }
                    }
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
            addr = rex.is32bits?Pop32(emu):Pop64(emu);
            R_RSP += tmp16u;
            STEP2
            break;
        case 0xC3:                      /* RET */
            addr = rex.is32bits?Pop32(emu):Pop64(emu);
            STEP2
            break;
        case 0xC4:                      /* LES Gd,Ed */
            nextop = F8;
            if(rex.is32bits && !(MODREG)) {
                GETED(0);
                GETGD;
                emu->segs[_ES] = *(uint16_t*)(((char*)ED) + 4);
                emu->segs_serial[_ES] = 0;
                GD->dword[0] = *(uint32_t*)ED;
            } else {
                vex_t vex = {0};
                vex.rex = rex;
                tmp8u = nextop;
                vex.m = tmp8u&0b00011111;
                vex.rex.b = (tmp8u&0b00100000)?0:1;
                vex.rex.x = (tmp8u&0b01000000)?0:1;
                vex.rex.r = (tmp8u&0b10000000)?0:1;
                tmp8u = F8;
                vex.p = tmp8u&0b00000011;
                vex.l = (tmp8u>>2)&1;
                vex.v = ((~tmp8u)>>3)&0b1111;
                vex.rex.w = (tmp8u>>7)&1;
                #ifdef TEST_INTERPRETER 
                if(!(addr = TestAVX(test, vex, addr, &step)))
                    unimp = 1;
                #else
                if(!(addr = RunAVX(emu, vex, addr, &step))) {
                    unimp = 1;
                    goto fini;
                }
                if(step==2) {
                    STEP2;
                }
                #endif
            }
            break;
        case 0xC5:                      /* LDS Gd,Ed */
            nextop = F8;
            if(rex.is32bits && !(MODREG)) {
                GETED(0);
                GETGD;
                emu->segs[_DS] = *(uint16_t*)(((char*)ED) + 4);
                emu->segs_serial[_DS] = 0;
                GD->dword[0] = *(uint32_t*)ED;
            } else {
                vex_t vex = {0};
                vex.rex = rex;
                tmp8u = nextop;
                vex.p = tmp8u&0b00000011;
                vex.l = (tmp8u>>2)&1;
                vex.v = ((~tmp8u)>>3)&0b1111;
                vex.rex.r = (tmp8u&0b10000000)?0:1;
                vex.rex.b = 0;
                vex.rex.x = 0;
                vex.rex.w = 0;
                vex.m = VEX_M_0F;
                #ifdef TEST_INTERPRETER 
                if(!(addr = TestAVX(test, vex, addr, &step)))
                    unimp = 1;
                #else
                if(!(addr = RunAVX(emu, vex, addr, &step))) {
                    unimp = 1;
                    goto fini;
                }
                if(step==2) {
                    STEP2;
                }
                #endif
            }
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
            if(rex.is32bits) {
                tmp64u = R_EBP;
                Push32(emu, R_EBP);
                R_EBP = R_ESP;
                if (tmp8u) {
                    for (tmp8u2 = 1; tmp8u2 < tmp8u; tmp8u2++) {
                        tmp64u -= 4;
                        Push32(emu, *((uint32_t*)tmp64u));
                    }
                    Push32(emu, R_EBP);
                }
            } else {
                tmp64u = R_RBP;
                Push64(emu, R_RBP);
                R_RBP = R_RSP;
                if (tmp8u) {
                    for (tmp8u2 = 1; tmp8u2 < tmp8u; tmp8u2++) {
                        tmp64u -= sizeof(void*);
                        Push64(emu, *((uintptr_t*)tmp64u));
                    }
                    Push64(emu, R_RBP);
                }
            }
            R_RSP -= tmp16u;
            break;
        case 0xC9:                      /* LEAVE */
            R_RSP = R_RBP;
            R_RBP = rex.is32bits?Pop32(emu):Pop64(emu);
            break;
        case 0xCA:                      /* FAR RETN */
            tmp16u = F16;
            if(rex.is32bits) {
                addr = Pop32(emu);
                emu->segs[_CS] = Pop32(emu);    // no check, no use....
            } else {
                addr = Pop64(emu);
                emu->segs[_CS] = Pop64(emu);    // no check, no use....
            }
            emu->segs_serial[_CS] = 0;
            R_RSP += tmp16u;
            is32bits = (R_CS==0x23);    // checking if CS changed
            #ifndef TEST_INTERPRETER
            if(is32bits)
                running32bits = 1;
            #endif
            STEP2;
            break;
        case 0xCB:                      /* FAR RET */
            if(rex.is32bits) {
                addr = Pop32(emu);
                emu->segs[_CS] = Pop32(emu);    // no check, no use....
            } else {
                addr = Pop64(emu);
                emu->segs[_CS] = Pop64(emu);    // no check, no use....
            }
            emu->segs_serial[_CS] = 0;
            is32bits = (R_CS==0x23);    // checking if CS changed
            #ifndef TEST_INTERPRETER
            if(is32bits)
                running32bits = 1;
            #endif
            STEP2;
            break;
        case 0xCC:                      /* INT 3 */
            R_RIP = addr;   // update RIP
            #ifndef TEST_INTERPRETER
            EmuInt3(emu, &addr);
            if(emu->quit) goto fini;    // R_RIP is up to date when returning from x64Int3
            addr = R_RIP;
            #endif
            break;
        case 0xCD:                      /* INT n */
            tmp8u = F8;
            #ifdef _WIN32
            #ifndef TEST_INTERPRETER
            EmitInterruption(emu, tmp8u, (void*)R_RIP);
            STEP;
            addr = R_RIP;
            #endif
            #else
            // this is a privilege opcode...
            if(box64_wine && tmp8u==0x2D) {
                // lets ignore the INT 2D
                printf_log(LOG_DEBUG, "INT 2D called\n");
                EmitInterruption(emu, 0x2d, (void*)R_RIP);
            } else if(box64_wine && tmp8u==0x2c) {
                printf_log(LOG_DEBUG, "INT 2c called\n");
                EmitInterruption(emu, 0x2c, (void*)R_RIP);
            } else if(box64_wine && tmp8u==0x29) {
                // INT 29 is __fastfail
                printf_log(LOG_DEBUG, "INT 29 called => __fastfail(0x%x)\n", R_ECX);
                EmitInterruption(emu, 0x29, (void*)R_RIP);
            } else if (tmp8u==0x80) {
                R_RIP = addr;
                if(tf) no_tf = 1;
                // 32bits syscall
                #ifndef TEST_INTERPRETER
                EmuX86Syscall(emu);
                STEP2;
                #endif
            } else if (tmp8u==0x03) {
                R_RIP = addr;
                #ifndef TEST_INTERPRETER
                EmitSignal(emu, X64_SIGTRAP, NULL, 3);
                STEP2;
                #endif
            } else {
                if(rex.is32bits && tmp8u==0x04) {
                    R_RIP = addr;
                }
                #ifndef TEST_INTERPRETER
                EmitInterruption(emu, tmp8u, (void*)R_RIP);
                STEP2;
                #endif
            }
            #endif
            break;
        case 0xCE:                      /* INTO */
            if(!rex.is32bits) {
                unimp = 2;
                goto fini;
            }
            emu->old_ip = R_RIP;
            R_RIP = addr;
            #ifndef TEST_INTERPRETER
            CHECK_FLAGS(emu);
            if(ACCESS_FLAG(F_OF))
                EmitInterruption(emu, 4, (void*)R_RIP);
            STEP2;
            #endif
            break;
        case 0xCF:                      /* IRET */
            {
                uintptr_t new_addr = (!rex.w)?Pop32(emu):Pop64(emu);
                uint32_t new_cs = ((!rex.w)?Pop32(emu):Pop64(emu))&0xffff; 
                #ifndef TEST_INTERPRETER
                if((new_cs&3)!=3) {
                    // R_RIP doesn't advance
                    printf_log(LOG_INFO, "Warning, unexpected new_cs=0x%x\n", new_cs);
                    R_RSP-=(rex.w?4:8)*2;
                    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0); // GP if trying to change priv level
                    goto fini;
                }
                #endif
                RESET_FLAGS(emu);
                uint64_t new_flags = ((((!rex.w)?Pop32(emu):Pop64(emu)) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
                if(!is32bits || (is32bits && (new_cs!=0x23))) {
                    uintptr_t new_sp = (!rex.w)?Pop32(emu):Pop64(emu);
                    uint32_t new_ss = ((!rex.w)?Pop32(emu):Pop64(emu))&0xffff;
                    if(!new_ss) {
                        // R_RIP doesn't advance
                        printf_log(LOG_INFO, "Warning, unexpected new_cs=0x%x\n", new_cs);
                        R_RSP-=(rex.w?4:8)*5;
                        EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0); // GPF
                        goto fini;
                    }
                    R_RSP = new_sp;
                    emu->segs[_SS] = new_sp;
                    emu->segs_serial[_SS] = 0;
                }
                emu->eflags.x64 = new_flags;
                tf = ACCESS_FLAG(F_TF);
                emu->segs[_CS] = new_cs;
                emu->segs_serial[_CS] = 0;
                addr = new_addr;
                R_RIP = addr;
                if(is32bits!=(emu->segs[_CS]==0x23)) {
                    is32bits = (emu->segs[_CS]==0x23);
                    if(is32bits) {
                        // Zero upper part of the 32bits regs
                        R_RAX = R_EAX;
                        R_RBX = R_EBX;
                        R_RCX = R_ECX;
                        R_RDX = R_EDX;
                        R_RSP = R_ESP;
                        R_RBP = R_EBP;
                        R_RSI = R_ESI;
                        R_RDI = R_EDI;
                    }
                    #ifndef TEST_INTERPRETER
                    if(is32bits)
                        running32bits = 1;
                    #endif
                }
            }
            STEP;
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

        case 0xD4:                      /* AAM Ib */
            if(rex.is32bits) {
                R_AX = aam16(emu, R_AL, F8);
            } else {
                unimp = 2;
                goto fini;
            };
            break;
        case 0xD5:                      /* AAD Ib */
            if(rex.is32bits) {
                R_AX = aad16(emu, R_AX, F8);
            } else {
                unimp = 2;
                goto fini;
            };
            break;
        case 0xD6:                      /* SALC */
            if(rex.is32bits) {
                CHECK_FLAGS(emu);
                R_AL = ACCESS_FLAG(F_CF)?0xff:0x00;
            } else {
                unimp = 2;
                goto fini;
            };
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
            if(rex.is32bits && rex.is67) {
                --R_CX; // don't update flags
                if(R_CX && !ACCESS_FLAG(F_ZF))
                    addr += tmp8s;
            } else if(rex.is32bits || rex.is67) {
                --R_ECX; // don't update flags
                if(rex.is67) emu->regs[_CX].dword[1] = 0;
                if(R_ECX && !ACCESS_FLAG(F_ZF))
                    addr += tmp8s;
            } else {
                --R_RCX; // don't update flags
                if(R_RCX && !ACCESS_FLAG(F_ZF))
                    addr += tmp8s;
            }
            STEP2
            break;
        case 0xE1:                      /* LOOPZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            if(rex.is32bits && rex.is67) {
                --R_CX; // don't update flags
                if(R_CX && ACCESS_FLAG(F_ZF))
                    addr += tmp8s;
            } else if(rex.is32bits || rex.is67) {
                --R_ECX; // don't update flags
                if(rex.is67) emu->regs[_CX].dword[1] = 0;
                if(R_ECX && ACCESS_FLAG(F_ZF))
                    addr += tmp8s;
            } else {
                --R_RCX; // don't update flags
                if(R_RCX && ACCESS_FLAG(F_ZF))
                    addr += tmp8s;
            }
            STEP2
            break;
        case 0xE2:                      /* LOOP */
            tmp8s = F8S;
            if(rex.is32bits && rex.is67) {
                --R_CX; // don't update flags
                if(R_CX)
                    addr += tmp8s;
            } else if(rex.is32bits || rex.is67) {
                --R_ECX; // don't update flags
                if(rex.is67) emu->regs[_CX].dword[1] = 0;
                if(R_ECX)
                    addr += tmp8s;
            } else {
                --R_RCX; // don't update flags
                if(R_RCX)
                    addr += tmp8s;
            }
            STEP2
            break;
        case 0xE3:                      /* JRCXZ */
            tmp8s = F8S;
            if(rex.is32bits && rex.is67) {
                if(!R_CX)
                    addr += tmp8s;
            } else if(rex.is32bits || rex.is67) {
                if(!R_ECX)
                    addr += tmp8s;
            } else {
                if(!R_RCX)
                    addr += tmp8s;
            }
            STEP2
            break;
        case 0xE4:                      /* IN AL, XX */
        case 0xE5:                      /* IN EAX, XX */
        case 0xE6:                      /* OUT XX, AL */
        case 0xE7:                      /* OUT XX, EAX */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            F8;
            if(rex.is32bits && BOX64ENV(ignoreint3))
            {} else
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
            STEP;
            #endif
            break;
        case 0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            if(rex.is32bits)
                Push32(emu, addr);
            else
                Push64(emu, addr);
            if(rex.is32bits)
                addr = (uint32_t)(addr+tmp32s);
            else
                addr += tmp32s;
            addr = (uintptr_t)getAlternate((void*)addr);
            STEP2
            break;
        case 0xE9:                      /* JMP Id */
            tmp32s = F32S; // jmp is relative
            if(rex.is32bits)
                addr = (uint32_t)(addr+tmp32s);
            else
                addr += tmp32s;
            addr = (uintptr_t)getAlternate((void*)addr);
            STEP2
            break;
        case 0xEA:                      /* JMP FAR seg:off*/
            if(is32bits) {
                uint16_t new_cs = F16;
                uint32_t new_addr = F32;
                #ifndef TEST_INTERPRETER
                if((new_cs&3)!=3) {
                    // R_RIP doesn't advance
                    printf_log(LOG_INFO, "Warning, unexpected new_cs=0x%x\n", new_cs);
                    R_RSP-=(rex.w?4:8)*2;
                    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0); // GP if trying to change priv level
                    goto fini;
                }
                emu->segs[_CS] = new_cs;
                emu->segs_serial[_CS] = 0;
                R_RIP = new_addr;
                if(is32bits!=(emu->segs[_CS]==0x23)) {
                    is32bits = (emu->segs[_CS]==0x23);
                    if(is32bits) {
                        // Zero upper part of the 32bits regs
                        R_RAX = R_EAX;
                        R_RBX = R_EBX;
                        R_RCX = R_ECX;
                        R_RDX = R_EDX;
                        R_RSP = R_ESP;
                        R_RBP = R_EBP;
                        R_RSI = R_ESI;
                        R_RDI = R_EDI;
                    }
                    #ifndef TEST_INTERPRETER
                    if(is32bits)
                        running32bits = 1;
                    #endif
                }
                #endif
            } else {
                unimp = 2;
                goto fini;
            }
            STEP;
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
            if(rex.is32bits && BOX64ENV(ignoreint3))
            {} else
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
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
        case 0xF1:                      /* INT1 */
            emu->old_ip = R_RIP;
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 128);
            #endif
            break;

        case 0xF4:                      /* HLT */
            // this is a privilege opcode...
            #ifndef TEST_INTERPRETER
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
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
                    #ifndef TEST_INTERPRETER
                    if(!EB->byte[0])
                        EmitDiv0(emu, (void*)R_RIP, 1);
                    #endif
                    div8(emu, EB->byte[0]);
                    break;
                case 7:                 /* IDIV Eb */
                    #ifndef TEST_INTERPRETER
                    if(!EB->byte[0])
                        EmitDiv0(emu, (void*)R_RIP, 1);
                    #endif
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
                        #ifndef TEST_INTERPRETER
                        if(!ED->q[0])
                            EmitDiv0(emu, (void*)R_RIP, 1);
                        #endif
                        div64(emu, ED->q[0]);
                        break;
                    case 7:                 /* IDIV Ed */
                        #ifndef TEST_INTERPRETER
                        if(!ED->q[0])
                            EmitDiv0(emu, (void*)R_RIP, 1);
                        #endif
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
                        #ifndef TEST_INTERPRETER
                        if(!ED->dword[0])
                            EmitDiv0(emu, (void*)R_RIP, 1);
                        #endif
                        div32(emu, ED->dword[0]);
                        //emu->regs[_AX].dword[1] = 0;  // already put high regs to 0
                        //emu->regs[_DX].dword[1] = 0;
                        break;
                    case 7:                 /* IDIV Ed */
                        #ifndef TEST_INTERPRETER
                        if(!ED->dword[0])
                            EmitDiv0(emu, (void*)R_RIP, 1);
                        #endif
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
            // this is a privilege opcode
            if(rex.is32bits && BOX64ENV(ignoreint3))
            {} else
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
            STEP;
            break;
        case 0xFB:                      /* STI */
            // this is a privilege opcode
            if(rex.is32bits && BOX64ENV(ignoreint3))
            {} else
            EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
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
                    if(rex.is32bits) {
                        tmp64u = (uintptr_t)ED->dword[0];
                        tmp64u = (uintptr_t)getAlternate((void*)tmp64u);
                        Push32(emu, addr);
                    } else {
                        tmp64u = ED->q[0];
                        tmp64u = (uintptr_t)getAlternate((void*)tmp64u);
                        Push64(emu, addr);
                    }
                    addr = tmp64u;
                    STEP2
                    break;
                case 3:                 /* CALL FAR Ed */
                    GETET(0);
                    if(MODREG) {
                        printf_log(LOG_NONE, "Illegal Opcode %p: (%02X %02X %02X %02X) %02X %02X %02X %02X\n", (void*)R_RIP, PK(-6), PK(-5), PK(-4), PK(-3), opcode, nextop, PK(0), PK(1));
                        EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                        goto fini;
                    } else {
                        if(rex.is32bits || !rex.w) {
                            Push32(emu, R_CS);
                            Push32(emu, addr);
                            addr = (uintptr_t)ED->dword[0];
                            R_CS = ED->word[2];
                        } else {
                            Push64(emu, R_CS);
                            Push64(emu, addr);
                            addr = ED->q[0];
                            R_CS = (ED+1)->word[0];
                        }
                        addr = (uintptr_t)getAlternate((void*)addr);
                        STEP2;
                        if(is32bits!=(emu->segs[_CS]==0x23)) {
                            is32bits = (emu->segs[_CS]==0x23);
                            if(is32bits) {
                                // Zero upper part of the 32bits regs
                                R_RAX = R_EAX;
                                R_RBX = R_EBX;
                                R_RCX = R_ECX;
                                R_RDX = R_EDX;
                                R_RSP = R_ESP;
                                R_RBP = R_EBP;
                                R_RSI = R_ESI;
                                R_RDI = R_EDI;
                            }
                            #ifndef TEST_INTERPRETER
                            if(is32bits)
                                running32bits = 1;
                            #endif
                        }
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    GETE8(0);
                    if(rex.is32bits)
                        addr = (uintptr_t)ED->dword[0];
                    else
                        addr = (uintptr_t)ED->q[0];
                    addr = (uintptr_t)getAlternate((void*)addr);
                    STEP2
                    break;
                case 5:                 /* JMP FAR Ed */
                    GETET(0);
                    if(MODREG) {
                        printf_log(LOG_NONE, "Illegal Opcode %p: (%02X %02X %02X %02X) %02X %02X %02X %02X\n", (void*)R_RIP, PK(-6), PK(-5), PK(-4), PK(-3), opcode, nextop, PK(0), PK(1));
                        EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                        goto fini;
                    } else {
                        if(rex.is32bits || !rex.w) {
                            addr = (uintptr_t)ED->dword[0];
                            R_CS = ED->word[2];
                        } else {
                            addr = ED->q[0];
                            R_CS = (ED+1)->word[0];
                        }
                        addr = (uintptr_t)getAlternate((void*)addr);
                        STEP2;
                        if(is32bits!=(emu->segs[_CS]==0x23)) {
                            is32bits = (emu->segs[_CS]==0x23);
                            if(is32bits) {
                                // Zero upper part of the 32bits regs
                                R_RAX = R_EAX;
                                R_RBX = R_EBX;
                                R_RCX = R_ECX;
                                R_RDX = R_EDX;
                                R_RSP = R_ESP;
                                R_RBP = R_EBP;
                                R_RSI = R_ESI;
                                R_RDI = R_EDI;
                            }
                            #ifndef TEST_INTERPRETER
                            if(is32bits)
                                running32bits = 1;
                            #endif
                        }
                    }
                    break;
                case 6:                 /* Push Ed */
                    _GETED(0);
                    if(rex.is32bits) {
                        tmp32u = ED->dword[0];
                        Push32(emu, tmp32u);  // avoid potential issue with push [esp+...]
                    } else {
                        tmp64u = ED->q[0];  // rex.w ignored
                        Push64(emu, tmp64u);  // avoid potential issue with push [esp+...]
                    }
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode %p: (%02X %02X %02X %02X) %02X %02X %02X %02X %02X %02X\n", (void*)R_RIP, PK(-6), PK(-5), PK(-4), PK(-3), opcode, nextop, PK(0), PK(1), PK(2), PK(3));
                    EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
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
    // check the TRACE flag before going to out, in case it's a step by step scenario
    if(!emu->quit && !emu->fork && ACCESS_FLAG(F_TF)) {
        R_RIP = addr;
        EmitSignal(emu, X64_SIGTRAP, (void*)addr, 1);
        if(emu->quit) goto fini;
    }
#endif
if(emu->segs[_CS]!=0x33 && emu->segs[_CS]!=0x23) printf_log(LOG_NONE, "Warning, CS is not default value: 0x%x\n", emu->segs[_CS]);
#ifndef TEST_INTERPRETER
    printf_log(LOG_DEBUG, "End of X86 run (%p), RIP=%p, Stack=%p, unimp=%d, emu->fork=%d, emu->quit=%d\n", emu, (void*)R_RIP, (void*)R_RSP, unimp, emu->fork, emu->quit);
    if(unimp) {
        //emu->quit = 1;
        if(unimp==1)
            UnimpOpcode(emu, is32bits);
        EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
    }
    // fork handling
    if(emu->fork) {
        addr = R_RIP;
        int forktype = emu->fork;
        emu->quit = 0;
        emu->fork = 0;
        emu = EmuFork(emu, forktype);
        if(step)
            return 0;
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
