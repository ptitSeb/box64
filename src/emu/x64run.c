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
    reg64_t *oped;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    uint64_t tmp64u, tmp64u2, tmp64u3;
    int32_t tmp32s, tmp32s2;
    int64_t tmp64s, tmp64s2;
    uintptr_t ip;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opex, eax1;
    mmx_regs_t *opem, eam1;

    if(emu->quit)
        return 0;

    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p\n", emu, (void*)R_RIP, emu->context->stack);
#define F8      *(uint8_t*)(ip++)
#define F8S     *(int8_t*)(ip++)
#define F16     *(uint16_t*)(ip+=2, ip-2)
#define F32     *(uint32_t*)(ip+=4, ip-4)
#define F32S    *(int32_t*)(ip+=4, ip-4)
#define PK(a)   *(uint8_t*)(ip+a)
#ifdef DYNAREC
#define STEP if(step) goto stepout;
#else
#define STEP
#endif

x64emurun:
    ip = R_RIP;

//#include "modrm.h"
    while(1) {
#ifdef HAVE_TRACE
        __builtin_prefetch((void*)ip, 0, 0); 
        emu->prev2_ip = emu->prev_ip;
        emu->prev_ip = R_RIP;
        R_RIP=ip;
        if(my_context->dec && (
            (trace_end == 0) 
            || ((ip >= trace_start) && (ip < trace_end))) )
                PrintTrace(emu, ip, 0);
#endif

        opcode = F8;
        switch(opcode) {

        default:
            emu->old_ip = R_RIP;
            R_RIP = ip;
            UnimpOpcode(emu);
            goto fini;
        }
    }
#ifdef DYNAREC
stepout:
    emu->old_ip = R_RIP;
    R_RIP = ip;
    return 0;
#endif

fini:
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
