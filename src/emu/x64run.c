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
//#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
//#include "my_cpuid.h"
#include "bridge.h"
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

int my_setcontext(x64emu_t* emu, void* ucp);

int Run(x64emu_t *emu, int step)
{
    uint8_t opcode;
    uint8_t nextop;
    reg64_t *oped, *opgd;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    uint64_t tmp64u, tmp64u2, tmp64u3;
    int32_t tmp32s, tmp32s2;
    int64_t tmp64s, tmp64s2;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opex, eax1;
    mmx_regs_t *opem, eam1;
    rex_t rex;
    int unimp = 0;

    if(emu->quit)
        return 0;

    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p\n", emu, (void*)R_RIP, emu->context->stack);
#define F8      *(uint8_t*)(R_RIP++)
#define F8S     *(int8_t*)(R_RIP++)
#define F16     *(uint16_t*)(R_RIP+=2, R_RIP-2)
#define F32     *(uint32_t*)(R_RIP+=4, R_RIP-4)
#define F32S    *(int32_t*)(R_RIP+=4, R_RIP-4)
#define F64     *(uint64_t*)(R_RIP+=8, R_RIP-8)
#define F64S    *(int64_t*)(R_RIP+=8, R_RIP-8)
#define PK(a)   *(uint8_t*)(R_RIP+a)
#ifdef DYNAREC
#define STEP if(step) return 0;
#else
#define STEP
#endif

#define GETED oped=GetEd(emu, rex, nextop)
#define GETGD opgd=GeG(emu, rex, nextop)
#define ED  oped
#define GD  opgd

x64emurun:

//#include "modrm.h"
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
        if(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        } else
            rex.rex = 0;

        switch(opcode) {

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:      /* Another REX */
            --R_RIP;
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
//    if(emu->fork) {
//        if(step)
//            return 0;
//        int forktype = emu->fork;
//        emu->quit = 0;
//        emu->fork = 0;
//        emu = x64emu_fork(emu, forktype);
//        goto x64emurun;
//    }
    // setcontext handling
//    else if(emu->uc_link) {
//        emu->quit = 0;
//        my_setcontext(emu, emu->uc_link);
//        goto x64emurun;
//    }
    return 0;
}
