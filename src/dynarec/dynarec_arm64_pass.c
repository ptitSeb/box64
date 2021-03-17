#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"
#include "custommem.h"

#ifndef STEP
#error No STEP defined
#endif

void arm_pass(dynarec_arm_t* dyn, uintptr_t addr)
{
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    int need_epilog = 1;
    dyn->sons_size = 0;
    // Clean up (because there are multiple passes)
    dyn->state_flags = 0;
    dyn->dfnone = 0;
    dyn->last_ip = ip;  // RIP is always set at start of block!
    fpu_reset(dyn, ninst);
    // ok, go now
    INIT;
    while(ok) {
        if(dyn->insts && (ninst>dyn->size)) {dynarec_log(LOG_NONE, "Warning, too many inst treated (%d / %d)\n",ninst, dyn->size);}
        ip = addr;
        if(dyn->insts && (dyn->insts[ninst].x64.barrier==1)) {
            dyn->last_ip = 0;
            NEW_BARRIER_INST;
        }
        NEW_INST;
        fpu_reset_scratch(dyn);
#ifdef HAVE_TRACE
        if(my_context->dec && box64_dynarec_trace) {
        if((trace_end == 0) 
            || ((ip >= trace_start) && (ip < trace_end)))  {
                MESSAGE(LOG_DUMP, "TRACE ----\n");
                fpu_reflectcache(dyn, ninst, x1, x2, x3);
                GETIP(ip);
                MOVx_REG(x1, xRIP);
                STORE_XEMU_REGS(xRIP);
                MOV32w(x2, 1);
                CALL(PrintTrace, -1);
                LOAD_XEMU_REGS(xRIP);
                MESSAGE(LOG_DUMP, "----------\n");
            }
        }
#endif

        addr = dynarec64_00(dyn, addr, ip, ninst, &ok, &need_epilog);

        INST_EPILOG;

        if(dyn->insts && dyn->insts[ninst+1].x64.barrier) {
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            if(dyn->insts[ninst+1].x64.barrier!=2) {
                dyn->state_flags = 0;
                dyn->dfnone = 0;
            }
        }
        if(!ok && !need_epilog && dyn->insts && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
        }
        if(!ok && !need_epilog && !dyn->insts && getProtection(addr+3))
            if(*(uint32_t*)addr!=0) {   // check if need to continue (but is next 4 bytes are 0, stop)
                uintptr_t next = get_closest_next(dyn, addr);
                if(next && (
                    (((next-addr)<15) && is_nops(dyn, addr, next-addr)) 
                    ||(((next-addr)<30) && is_instructions(dyn, addr, next-addr)) ))
                {
                    dynarec_log(LOG_DEBUG, "Extend block %p, %p -> %p (ninst=%d)\n", dyn, (void*)addr, (void*)next, ninst);
                    ok = 1;
                } else if(next && (next-addr)<30) {
                    dynarec_log(LOG_DEBUG, "Cannot extend block %p -> %p (%02X %02X %02X %02X %02X %02X %02X %02x)\n", (void*)addr, (void*)next, PK(0), PK(1), PK(2), PK(3), PK(4), PK(5), PK(6), PK(7));
                }
            }
        if(ok<0)  {ok = 0; need_epilog=1;}
        ++ninst;
    }
    if(need_epilog) {
        fpu_purgecache(dyn, ninst, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    }
    FINI;
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d, %d sons)\n", dyn->size, dyn->sons_size);
}