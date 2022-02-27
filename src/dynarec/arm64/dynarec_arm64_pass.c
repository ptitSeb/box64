#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

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
#include "dynablock.h"
#include "dynarec_native.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

#ifndef STEP
#error No STEP defined
#endif

uintptr_t arm_pass(dynarec_arm_t* dyn, uintptr_t addr)
{
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uintptr_t init_addr = addr;
    rex_t rex;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix
    int need_epilog = 1;
    dyn->sons_size = 0;
    // Clean up (because there are multiple passes)
    dyn->state_flags = 0;
    dyn->dfnone = 0;
    dyn->last_ip = ip;  // RIP is always set at start of block!
    MAYUSE(init_addr);
    fpu_reset(dyn, ninst);
    // ok, go now
    INIT;
    while(ok) {
        ip = addr;
        if((dyn->insts[ninst].x64.barrier==1)) {
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
                STORE_XEMU_CALL(xRIP);
                MOV32w(x2, 1);
                CALL(PrintTrace, -1);
                LOAD_XEMU_CALL(xRIP);
                MESSAGE(LOG_DUMP, "----------\n");
            }
        }
#endif

        rep = 0;
        uint8_t pk = PK(0);
        while((pk==0xF2) || (pk==0xF3)) {
            rep = pk-0xF1;
            ++addr;
            pk = PK(0);
        }
        while(pk==0x3E) {   //Branch Taken Hint ignored
            ++addr;
            pk = PK(0);
        }
        rex.rex = 0;
        while(pk>=0x40 && pk<=0x4f) {
            rex.rex = pk;
            ++addr;
            pk = PK(0);
        }

        addr = dynarec64_00(dyn, addr, ip, ninst, rex, rep, &ok, &need_epilog);

        INST_EPILOG;

        if(dyn->insts[ninst+1].x64.barrier) {
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            if(dyn->insts[ninst+1].x64.barrier!=2) {
                dyn->state_flags = 0;
                dyn->dfnone = 0;
            }
        }
        #if STEP == 0
        if(!ok && !need_epilog && box64_dynarec_bigblock && getProtection(addr+3)&~PROT_CUSTOM && !IsInHotPage(addr+3))
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
        #else
        if(!ok && !need_epilog && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
        }
        #endif
        if(ok<0)  {ok = 0; need_epilog=1;}
        ++ninst;
        #if STEP == 0
        if(ok && !isJumpTableDefault64((void*)addr))
        #else
        if(ok && (ninst==dyn->size))
        #endif
        {
            #if STEP == 3
            dynarec_log(LOG_DEBUG, "Stopping block %p (%d / %d)\n",(void*)init_addr, ninst, dyn->size); 
            #endif
            BARRIER(2);
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            jump_to_next(dyn, addr, 0, ninst);
            ok=0; need_epilog=0;
        }
    }
    if(need_epilog) {
        fpu_purgecache(dyn, ninst, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    }
    FINI;
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d, %d sons)\n", dyn->size, dyn->sons_size);
    return addr;
}