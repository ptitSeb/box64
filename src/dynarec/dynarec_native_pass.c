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
    dyn->sons_size = 0;
    // Clean up (because there are multiple passes)
    dyn->f.pending = 0;
    dyn->f.dfnone = 0;
    fpu_reset(dyn);
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
                MESSAGE(LOG_DEBUG, "Reset Caches with %d\n",reset_n);
                #if STEP > 1
                // for STEP 2 & 3, just need to refrest with current, and undo the changes (push & swap)
                dyn->n = dyn->insts[ninst].n;
                neoncacheUnwind(&dyn->n);
                #ifdef HAVE_TRACE
                if(box64_dynarec_dump)
                    if(memcmp(&dyn->n, &dyn->insts[reset_n].n, sizeof(neon_cache_t))) {
                        MESSAGE(LOG_DEBUG, "Warning, difference in neoncache: reset=");
                        for(int i=0; i<24; ++i)
                            if(dyn->insts[reset_n].n.neoncache[i].v)
                                MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].n.neoncache[i].t, dyn->insts[reset_n].n.neoncache[i].n));
                        if(dyn->insts[reset_n].n.combined1 || dyn->insts[reset_n].n.combined2)
                            MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].n.swapped?"SWP":"CMB", dyn->insts[reset_n].n.combined1, dyn->insts[reset_n].n.combined2);
                        if(dyn->insts[reset_n].n.stack_push || dyn->insts[reset_n].n.stack_pop)
                            MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].n.stack_push, -dyn->insts[reset_n].n.stack_pop);
                        MESSAGE(LOG_DEBUG, " ==> ");
                        for(int i=0; i<24; ++i)
                            if(dyn->insts[ninst].n.neoncache[i].v)
                                MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].n.neoncache[i].t, dyn->insts[ninst].n.neoncache[i].n));
                        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
                            MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
                        if(dyn->insts[ninst].n.stack_push || dyn->insts[ninst].n.stack_pop)
                            MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
                        MESSAGE(LOG_DEBUG, " -> ");
                        for(int i=0; i<24; ++i)
                            if(dyn->n.neoncache[i].v)
                                MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->n.neoncache[i].t, dyn->n.neoncache[i].n));
                        if(dyn->n.combined1 || dyn->n.combined2)
                            MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->n.swapped?"SWP":"CMB", dyn->n.combined1, dyn->n.combined2);
                        if(dyn->n.stack_push || dyn->n.stack_pop)
                            MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->n.stack_push, -dyn->n.stack_pop);
                        MESSAGE(LOG_DEBUG, "\n");
                    }
                #endif //HAVE_TRACE
                #else
                dyn->n = dyn->insts[reset_n].n;
                #endif
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
        } else if(ninst && (dyn->insts[ninst].pred_sz!=1 || dyn->insts[ninst].pred[0]!=ninst-1))
            dyn->last_ip = 0;   // reset IP if some jump are comming here
        // propagate ST stack state, especial stack pop that are defered
        if(dyn->n.stack_pop) {
            for(int j=0; j<24; ++j)
                if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D || dyn->n.neoncache[j].t == NEON_CACHE_ST_F)) {
                    if(dyn->n.neoncache[j].n<dyn->n.stack_pop)
                        dyn->n.neoncache[j].v = 0;
                    else
                        dyn->n.neoncache[j].n-=dyn->n.stack_pop;
                }
            dyn->n.stack_pop = 0;
        }
        dyn->n.stack = dyn->n.stack_next;
        dyn->n.news = 0;
        dyn->n.stack_push = 0;
        dyn->n.swapped = 0;
        NEW_INST;
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
        if(!ok && !need_epilog && box64_dynarec_bigblock && getProtection(addr+3)&~PROT_CUSTOM)
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
                } else if(next && (next-addr)<30) {
                    if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Cannot extend block %p -> %p (%02X %02X %02X %02X %02X %02X %02X %02x)\n", (void*)addr, (void*)next, PK(0), PK(1), PK(2), PK(3), PK(4), PK(5), PK(6), PK(7));
                }
            }
        #endif
        if(ok<0)  {ok = 0; need_epilog=1;}
        ++ninst;
        #if STEP == 0
        if(ok && (((box64_dynarec_bigblock<stopblock) && !isJumpTableDefault64((void*)addr)) 
            || (addr>=box64_nodynarec_start && addr<box64_nodynarec_end)))
        #else
        if(ok && (ninst==dyn->size))
        #endif
        {
            int j32;
            MAYUSE(j32);
            MESSAGE(LOG_DEBUG, "Stopping block %p (%d / %d)\n",(void*)init_addr, ninst, dyn->size); 
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
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d, %d sons)\n", dyn->size, dyn->sons_size);
    return addr;
}
