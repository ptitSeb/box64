#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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

#if STEP == 0
#ifndef PROT_READ
#define PROT_READ 0x1
#endif
#endif

uintptr_t native_pass(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits)
{
    int ok = 1;
    int ninst = 0;
    int j64;
    uintptr_t ip = addr;
    uintptr_t init_addr = addr;
    rex_t rex;
    int rep = 0;    // 0 none, 1=F2 prefix, 2=F3 prefix
    int need_epilog = 1;
    // Clean up (because there are multiple passes)
    dyn->f.pending = 0;
    dyn->f.dfnone = 0;
    dyn->forward = 0;
    dyn->forward_to = 0;
    dyn->forward_size = 0;
    dyn->forward_ninst = 0;
    #if STEP == 0
    memset(&dyn->insts[ninst], 0, sizeof(instruction_native_t));
    #endif
    fpu_reset(dyn);
    ARCH_INIT();
    int reset_n = -1;
    dyn->last_ip = (alternate || (dyn->insts && dyn->insts[0].pred_sz))?0:ip;  // RIP is always set at start of block unless there is a predecessor!
    int stopblock = 2+(FindElfAddress(my_context, addr)?0:1); // if block is in elf_memory, it can be extended with box64_dynarec_bigblock==2, else it needs 3
    // ok, go now
    INIT;
    #if STEP == 0
    uintptr_t cur_page = (addr)&~box64_pagesize;
    #endif
    while(ok) {
        #if STEP == 0
        if(cur_page != ((addr)&~box64_pagesize)) {
            cur_page = (addr)&~box64_pagesize;
            if(!(getProtection(addr)&PROT_READ)) {
                need_epilog = 1;
                break;
            }
        }
        // This test is here to prevent things like TABLE64 to be out of range
        // native_size is not exact at this point, but it should be larger, not smaller, and not by a huge margin anyway
        // so it's good enough to avoid overflow in relative to PC data fectching
        if((dyn->native_size >= MAXBLOCK_SIZE) || (ninst >= MAX_INSTS)) {
            need_epilog = 1;
            break;
        }
        #endif
        fpu_propagate_stack(dyn, ninst);
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
        }
        #if STEP > 0
        else if(ninst && (dyn->insts[ninst].pred_sz>1 || (dyn->insts[ninst].pred_sz==1 && dyn->insts[ninst].pred[0]!=ninst-1)))
            dyn->last_ip = 0;   // reset IP if some jump are coming here
        #endif
        NEW_INST;
        #if STEP == 0
        if(ninst && dyn->insts[ninst-1].x64.barrier_next) {
            BARRIER(dyn->insts[ninst-1].x64.barrier_next);
        }
        #endif
        if(!ninst) {
            GOTEST(x1, x2);
        }
        if(dyn->insts[ninst].pred_sz>1) {SMSTART();}
        fpu_reset_scratch(dyn);
        if((dyn->insts[ninst].x64.need_before&~X_PEND) && !dyn->insts[ninst].pred_sz) {
            READFLAGS(dyn->insts[ninst].x64.need_before&~X_PEND);
        }
        if(box64_dynarec_test) {
            MESSAGE(LOG_DUMP, "TEST STEP ----\n");
            fpu_reflectcache(dyn, ninst, x1, x2, x3);
            GO_TRACE(x64test_step, 1, x5);
            fpu_unreflectcache(dyn, ninst, x1, x2, x3);
            MESSAGE(LOG_DUMP, "----------\n");
        }
#ifdef HAVE_TRACE
        else if(my_context->dec && box64_dynarec_trace) {
        if((trace_end == 0)
            || ((ip >= trace_start) && (ip < trace_end)))  {
                MESSAGE(LOG_DUMP, "TRACE ----\n");
                fpu_reflectcache(dyn, ninst, x1, x2, x3);
                GO_TRACE(PrintTrace, 1, x5);
                fpu_unreflectcache(dyn, ninst, x1, x2, x3);
                MESSAGE(LOG_DUMP, "----------\n");
            }
        }
#endif

        rep = 0;
        uint8_t pk = PK(0);
        while((pk==0xF2) || (pk==0xF3) || (pk==0x3E) || (pk==0x26)) {
            switch(pk) {
                case 0xF2: rep = 1; break;
                case 0xF3: rep = 2; break;
                case 0x3E:
                case 0x26: /* ignored */ break;
            }
            ++addr;
            pk = PK(0);
        }
        rex.rex = 0;
        rex.is32bits = is32bits;
        if(!rex.is32bits)
            while(pk>=0x40 && pk<=0x4f) {
                rex.rex = pk;
                ++addr;
                pk = PK(0);
            }

        addr = dynarec64_00(dyn, addr, ip, ninst, rex, rep, &ok, &need_epilog);
        if(dyn->abort)
            return ip;
        INST_EPILOG;

        int next = ninst+1;
        #if STEP > 0
        if(!dyn->insts[ninst].x64.has_next && dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts!=-1)
            next = dyn->insts[ninst].x64.jmp_insts;
        if(dyn->insts[ninst].x64.has_next && dyn->insts[next].x64.barrier) {
            if(dyn->insts[next].x64.barrier&BARRIER_FLOAT) {
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            }
            if(dyn->insts[next].x64.barrier&BARRIER_FLAGS) {
                dyn->f.pending = 0;
                dyn->f.dfnone = 0;
                dyn->last_ip = 0;
            }
        }
        #endif
        #ifndef PROT_READ
        #define PROT_READ 1
        #endif
        #if STEP != 0
        if(!ok && !need_epilog && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
            // we use the 1st predecessor here
            int ii = ninst+1;
            if(ii<dyn->size && !dyn->insts[ii].x64.alive) {
                while(ii<dyn->size && !dyn->insts[ii].x64.alive) {
                    // may need to skip opcodes to advance
                    ++ninst;
                    NEW_INST;
                    MESSAGE(LOG_DEBUG, "Skipping unused opcode\n");
                    INST_NAME("Skipped opcode");
                    INST_EPILOG;
                    addr += dyn->insts[ii].x64.size;
                    ++ii;
                }
            }
            if((dyn->insts[ii].x64.barrier&BARRIER_FULL)==BARRIER_FULL)
                reset_n = -2;    // hack to say Barrier!
            else {
                reset_n = getNominalPred(dyn, ii);  // may get -1 if no predecessor are available
                if(reset_n==-1) {
                    reset_n = -2;
                    MESSAGE(LOG_DEBUG, "Warning, Reset Caches mark not found\n");
                }
            }
        }
        #else
        // check if block need to be stopped, because it's a 00 00 opcode (unreadeable is already checked earlier)
        if((ok>0) && !dyn->forward && !(*(uint8_t*)addr) && !(*(uint8_t*)(addr+1))) {
            if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Stopping block at %p reason: %s\n", (void*)addr, "Next opcode is 00 00");
            ok = 0;
            need_epilog = 1;
        }
        if(dyn->forward) {
            if(dyn->forward_to == addr && !need_epilog && ok>=0) {
                // we made it!
                reset_n = get_first_jump(dyn, addr);
                if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Forward extend block for %d bytes %s%p -> %p (ninst %d - %d)\n", dyn->forward_to-dyn->forward, dyn->insts[dyn->forward_ninst].x64.has_callret?"(opt. call) ":"", (void*)dyn->forward, (void*)dyn->forward_to, reset_n, ninst);
                if(dyn->insts[dyn->forward_ninst].x64.has_callret && !dyn->insts[dyn->forward_ninst].x64.has_next)
                    dyn->insts[dyn->forward_ninst].x64.has_next = 1;  // this block actually continue
                dyn->forward = 0;
                dyn->forward_to = 0;
                dyn->forward_size = 0;
                dyn->forward_ninst = 0;
                ok = 1; // in case it was 0
            } else if ((dyn->forward_to < addr) || ok<=0) {
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
                    if(dyn->insts[ninst].x64.has_callret && !dyn->insts[ninst].x64.has_next) {
                        dyn->insts[ninst].x64.has_next = 1;  // this block actually continue
                    } else {
                        // need to find back that instruction to copy the caches, as previous version cannot be used anymore
                        // and pred table is not ready yet
                        reset_n = get_first_jump(dyn, next);
                    }
                    if(box64_dynarec_dump) dynarec_log(LOG_NONE, "Extend block %p, %s%p -> %p (ninst=%d, jump from %d)\n", dyn, dyn->insts[ninst].x64.has_callret?"(opt. call) ":"", (void*)addr, (void*)next, ninst+1, dyn->insts[ninst].x64.has_callret?ninst:reset_n);
                } else if(next && (next-addr)<box64_dynarec_forward && (getProtection(next)&PROT_READ)/*box64_dynarec_bigblock>=stopblock*/) {
                    if(!((box64_dynarec_bigblock<stopblock) && !isJumpTableDefault64((void*)next))) {
                        if(dyn->forward) {
                            if(next<dyn->forward_to)
                                dyn->forward_to = next;
                            reset_n = -2;
                            ok = 1;
                        } else {
                            dyn->forward = addr;
                            dyn->forward_to = next;
                            dyn->forward_size = dyn->size;
                            dyn->forward_ninst = ninst;
                            reset_n = -2;
                            ok = 1;
                        }
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
        }
        if((ok>0) && dyn->insts[ninst].x64.has_callret)
            reset_n = -2;
        ++ninst;
        #if STEP == 0
        memset(&dyn->insts[ninst], 0, sizeof(instruction_native_t));
        if((ok>0) && (((box64_dynarec_bigblock<stopblock) && !isJumpTableDefault64((void*)addr))
            || (addr>=box64_nodynarec_start && addr<box64_nodynarec_end)))
        #else
        if((ok>0) && (ninst==dyn->size))
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
            NOTEST(x3);
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            jump_to_next(dyn, addr, 0, ninst, rex.is32bits);
            ok=0; need_epilog=0;
        }
    }
    if(need_epilog) {
        NOTEST(x3);
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknown instruction
    }
    FINI;
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d)\n", dyn->size);
    return addr;
}
