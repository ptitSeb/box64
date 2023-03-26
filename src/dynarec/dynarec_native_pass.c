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
#include "elfloader.h"

#include "dynarec_arch.h"
#include "dynarec_helper.h"

#ifndef STEP
#error No STEP defined
#endif

uintptr_t native_pass(dynarec_native_t* dyn, uintptr_t addr)
{
    int ok = 1;
    int ninst = 0;
    int j64;
    uintptr_t ip = addr;
    uintptr_t init_addr = addr;
    rex_t rex;
    int rep;    // 0 none, 1=F2 prefix, 2=F3 prefix
    int need_epilog = 1;
    // Clean up (because there are multiple passes)
    dyn->f.pending = 0;
    dyn->f.dfnone = 0;
    dyn->forward = 0;
    dyn->forward_to = 0;
    dyn->forward_size = 0;
    dyn->forward_ninst = 0;
    fpu_reset(dyn);
    ARCH_INIT();
    int reset_n = -1;
    dyn->last_ip = (dyn->insts && dyn->insts[0].pred_sz)?0:ip;  // RIP is always set at start of block unless there is a predecessor!
    int stopblock = 2+(FindElfAddress(my_context, addr)?0:1); // if block is in elf_memory, it can be extended with bligblocks==2, else it needs 3
    // ok, go now
    INIT;
    while(ok) {
        ip = addr;
        if (reset_n!=-1) {
            dyn->last_ip = 0;
            if(reset_n==-2) {
                MESSAGE(LOG_DEBUG, "Reset Caches to zero\n");
                dyn->f.dfnone = 0;
                dyn->f.pending = 0;
                fpu_reset(dyn);
            } else {
                fpu_reset_cache(dyn, ninst, reset_n);
                dyn->f = dyn->insts[reset_n].f_exit;
                if(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT) {
                    MESSAGE(LOG_DEBUG, "Apply Barrier Float\n");
                    fpu_reset(dyn);
                }
                if(dyn->insts[ninst].x64.barrier&BARRIER_FLAGS) {
                    MESSAGE(LOG_DEBUG, "Apply Barrier Flags\n");
                    dyn->f.dfnone = 0;
                    dyn->f.pending = 0;
                }
            }
            reset_n = -1;
        } else if(ninst && (dyn->insts[ninst].pred_sz>1 || (dyn->insts[ninst].pred_sz==1 && dyn->insts[ninst].pred[0]!=ninst-1)))
            dyn->last_ip = 0;   // reset IP if some jump are comming here
        fpu_propagate_stack(dyn, ninst);
        NEW_INST;
        if(dyn->insts[ninst].pred_sz>1) {SMSTART();}
        fpu_reset_scratch(dyn);
        if((dyn->insts[ninst].x64.need_before&~X_PEND) && !dyn->insts[ninst].pred_sz) {
            READFLAGS(dyn->insts[ninst].x64.need_before&~X_PEND);
        }
#ifdef HAVE_TRACE
        if(my_context->dec && box64_dynarec_trace) {
        if((trace_end == 0) 
            || ((ip >= trace_start) && (ip < trace_end)))  {
                MESSAGE(LOG_DUMP, "TRACE ----\n");
                fpu_reflectcache(dyn, ninst, x1, x2, x3);
                GO_TRACE();
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
        while(pk==0x3E || pk==0x26) {   //Branch Taken Hint ignored, same for ES: prefix
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

        int next = ninst+1;
        #if STEP > 0
        if(!dyn->insts[ninst].x64.has_next && dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts!=-1)
            next = dyn->insts[ninst].x64.jmp_insts;
        #endif
        if(dyn->insts[ninst].x64.has_next && dyn->insts[next].x64.barrier) {
            if(dyn->insts[next].x64.barrier&BARRIER_FLOAT)
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            if(dyn->insts[next].x64.barrier&BARRIER_FLAGS) {
                dyn->f.pending = 0;
                dyn->f.dfnone = 0;
                dyn->last_ip = 0;
            }
        }
        #ifndef PROT_READ
        #define PROT_READ 1
        #endif
        #if STEP != 0
        if(!ok && !need_epilog && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
            // we use the 1st predecessor here
            int ii = ninst+1;
            while(ii<dyn->size && !dyn->insts[ii].pred_sz) 
                ++ii;
            if((dyn->insts[ii].x64.barrier&BARRIER_FULL)==BARRIER_FULL)
                reset_n = -2;    // hack to say Barrier!
            else {
                reset_n = getNominalPred(dyn, ii);  // may get -1 if no predecessor are availble
                if(reset_n==-1) {
                    reset_n = -2;
                    MESSAGE(LOG_DEBUG, "Warning, Reset Caches mark not found\n");
                }
            }
        }
        #else
        if(dyn->forward) {
            if(dyn->forward_to == addr && !need_epilog) {
                // we made it!
                if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Forward extend block for %d bytes %p -> %p\n", dyn->forward_to-dyn->forward, (void*)dyn->forward, (void*)dyn->forward_to);
                dyn->forward = 0;
                dyn->forward_to = 0;
                dyn->forward_size = 0;
                dyn->forward_ninst = 0;
                ok = 1; // in case it was 0
            } else if ((dyn->forward_to < addr) || !ok) {
                // something when wrong! rollback
                if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Could not forward extend block for %d bytes %p -> %p\n", dyn->forward_to-dyn->forward, (void*)dyn->forward, (void*)dyn->forward_to);
                ok = 0;
                dyn->size = dyn->forward_size;
                ninst = dyn->forward_ninst;
                addr = dyn->forward;
                dyn->forward = 0;
                dyn->forward_to = 0;
                dyn->forward_size = 0;
                dyn->forward_ninst = 0;
            }
            // else just continue
        } else if(!ok && !need_epilog && box64_dynarec_bigblock && (getProtection(addr+3)&~PROT_READ))
            if(*(uint32_t*)addr!=0) {   // check if need to continue (but is next 4 bytes are 0, stop)
                uintptr_t next = get_closest_next(dyn, addr);
                if(next && (
                    (((next-addr)<15) && is_nops(dyn, addr, next-addr)) 
                    /*||(((next-addr)<30) && is_instructions(dyn, addr, next-addr))*/ ))
                {
                    ok = 1;
                    // need to find back that instruction to copy the caches, as previous version cannot be used anymore
                    reset_n = -2;
                    for(int ii=0; ii<ninst; ++ii)
                        if(dyn->insts[ii].x64.jmp == next) {
                            reset_n = ii;
                            ii=ninst;
                        }
                    if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Extend block %p, %p -> %p (ninst=%d, jump from %d)\n", dyn, (void*)addr, (void*)next, ninst, reset_n);
                } else if(next && (next-addr)<box64_dynarec_forward && (getProtection(next)&PROT_READ)/*box64_dynarec_bigblock>=stopblock*/) {
                    if(!((box64_dynarec_bigblock<stopblock) && !isJumpTableDefault64((void*)next))) {
                        dyn->forward = addr;
                        dyn->forward_to = next;
                        dyn->forward_size = dyn->size;
                        dyn->forward_ninst = ninst;
                        reset_n = -2;
                        ok = 1;
                    }
                }
            }
        #endif
        if(ok<0)  {
            ok = 0; need_epilog=1; 
            #if STEP == 0
            if(ninst) {
                --ninst;
                if(!dyn->insts[ninst].x64.barrier) {
                    BARRIER(BARRIER_FLOAT);
                }
                dyn->insts[ninst].x64.need_after |= X_PEND;
                ++ninst;
            }
            #endif
        }
        ++ninst;
        #if STEP == 0
        if(ok && (((box64_dynarec_bigblock<stopblock) && !isJumpTableDefault64((void*)addr)) 
            || (addr>=box64_nodynarec_start && addr<box64_nodynarec_end)))
        #else
        if(ok && (ninst==dyn->size))
        #endif
        {
            #if STEP == 0
            if(dyn->forward) {
                // stopping too soon
                dyn->size = dyn->forward_size;
                ninst = dyn->forward_ninst+1;
                addr = dyn->forward;
                dyn->forward = 0;
                dyn->forward_to = 0;
                dyn->forward_size = 0;
                dyn->forward_ninst = 0;
            }
            #endif
            int j32;
            MAYUSE(j32);
            MESSAGE(LOG_DEBUG, "Stopping block %p (%d / %d)\n",(void*)init_addr, ninst, dyn->size);
            if(!box64_dynarec_dump && addr>=box64_nodynarec_start && addr<box64_nodynarec_end)
                dynarec_log(LOG_INFO, "Stopping block in no-dynarec zone\n");
            --ninst;
            if(!dyn->insts[ninst].x64.barrier) {
                BARRIER(BARRIER_FLOAT);
            }
            #if STEP == 0
            dyn->insts[ninst].x64.need_after |= X_PEND;
            #endif
            ++ninst;
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            jump_to_next(dyn, addr, 0, ninst);
            ok=0; need_epilog=0;
        }
    }
    if(need_epilog) {
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    }
    FINI;
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d)\n", dyn->size);
    return addr;
}
