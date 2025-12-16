#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "x64trace.h"
#include "dynablock.h"
#include "dynarec_native.h"
#include "dynablock_private.h"
#include "custommem.h"
#include "x64test.h"
#include "pe_tools.h"

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

uintptr_t native_pass(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max)
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
    #ifdef ARM64
    dyn->f = status_unk;
    #else
    dyn->f.pending = 0;
    dyn->f.dfnone = 0;
    #endif
    dyn->forward = 0;
    dyn->forward_to = 0;
    dyn->forward_size = 0;
    dyn->forward_ninst = 0;
    dyn->ymm_zero = 0;
    int dynarec_dirty = BOX64ENV(dynarec_dirty);
    #if STEP == 0
    memset(&dyn->insts[ninst], 0, sizeof(instruction_native_t));
    #ifdef ARM64
    dyn->have_purge = BOX64ENV(dynarec_purge);
    #endif
    #endif
    fpu_reset(dyn);
    ARCH_INIT();
    int reset_n = -1; // -1 no reset; -2 reset to 0; else reset to the state of reset_n
    dyn->last_ip = (alternate || (dyn->insts && dyn->insts[0].pred_sz))?0:ip;  // RIP is always set at start of block unless there is a predecessor!
    int stopblock = 2 + !IsAddrElfOrFileMapped(addr);                          // if block is in elf memory or file mapped memory, it can be extended with BOX64DRENV(dynarec_bigblock)==2, else it needs 3
    // ok, go now
    INIT;
    #if STEP == 0
    uintptr_t cur_page = (addr)&~(box64_pagesize-1);
    #endif
    while(ok) {
        #if STEP == 0
        if(cur_page != ((addr)&~(box64_pagesize-1))) {
            cur_page = (addr)&~(box64_pagesize-1);
            uint32_t prot = getProtection(addr);
            if(!(prot&PROT_READ) || checkInHotPage(addr) || (addr>dyn->end)) {
                dynarec_log(LOG_INFO, "Stopping dynablock because of protection, hotpage or mmap crossing at %p -> %p inst=%d\n", (void*)dyn->start, (void*)addr, ninst);
                need_epilog = 1;
                break;
            }
            if(prot&PROT_NEVERCLEAN)
                dyn->always_test = 1;
        }
        // This test is here to prevent things like TABLE64 to be out of range
        // native_size is not exact at this point, but it should be larger, not smaller, and not by a huge margin anyway
        // so it's good enough to avoid overflow in relative to PC data fectching
        if(ninst >= inst_max) {
            need_epilog = 1;
            break;
        }
        #endif
        if(!ninst && dyn->need_x87check) {
            NATIVE_RESTORE_X87PC();
        }
        ip = addr;
        #ifdef ARM64
        if(!ninst) {
            if(dyn->have_purge)
                doEnterBlock(dyn, 0, x1, x2, x3);
            if(dyn->insts[0].preload_xmmymm)
                doPreload(dyn, 0);
            ENDPREFIX;
        }
        #endif
        fpu_propagate_stack(dyn, ninst);
        if (reset_n!=-1) {
            dyn->last_ip = 0;
            if(reset_n==-2) {
                MESSAGE(LOG_DEBUG, "Reset Caches to zero\n");
                #ifdef ARM64
                dyn->f = status_unk;
                #else
                dyn->f.dfnone = 0;
                dyn->f.pending = 0;
                #endif
                fpu_reset(dyn);
                ARCH_RESET();
            } else {
                fpu_reset_cache(dyn, ninst, reset_n);
                dyn->f = dyn->insts[reset_n].f_exit;
                if(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT) {
                    MESSAGE(LOG_DEBUG, "Apply Barrier Float\n");
                    fpu_reset(dyn);
                }
                if(dyn->insts[ninst].x64.barrier&BARRIER_FLAGS) {
                    MESSAGE(LOG_DEBUG, "Apply Barrier Flags\n");
                    #ifdef ARM64
                    dyn->f = status_unk;
                    #else
                    dyn->f.dfnone = 0;
                    dyn->f.pending = 0;
                    #endif
                }
            }
            reset_n = -1;
        }
        #if STEP > 0
        else if(ninst && (dyn->insts[ninst].pred_sz>1 || (dyn->insts[ninst].pred_sz==1 && dyn->insts[ninst].pred[0]!=ninst-1)))
            dyn->last_ip = 0;   // reset IP if some jump are coming here
        #endif
        NEW_INST;
        MESSAGE(LOG_DUMP, "New Instruction %s:%p, native:%p\n", is32bits?"x86":"x64",(void*)addr, (void*)dyn->block);
        #ifdef ARCH_NOP
        if(dyn->insts[ninst].x64.alive && dyn->insts[ninst].x64.self_loop)
            CALLRET_LOOP();
        #endif
        if(!ninst) {
            GOTEST(x1, x2);
        }
        if(dyn->insts[ninst].pred_sz>1) {SMEND();}
        #if STEP > 1
        if (dyn->insts[ninst].lock) {
            WILLWRITELOCK(dyn->insts[ninst].lock);
        } else if (dyn->insts[ninst].will_write) {
            WILLWRITE();
        }

        int is_opcode_volatile = /*box64_wine &&*/ VolatileRangesContains(ip) && VolatileOpcodesHas(ip);
        if (is_opcode_volatile && !dyn->insts[ninst].lock)
            DMB_ISHST();
        #endif
        if((dyn->insts[ninst].x64.need_before&~X_PEND) && !ninst) {
            READFLAGS(dyn->insts[ninst].x64.need_before&~X_PEND);
        }
        if(BOX64ENV(dynarec_test) && (!BOX64ENV(dynarec_test_end) || (ip>=BOX64ENV(dynarec_test_start) && ip<BOX64ENV(dynarec_test_end)))) {
            MESSAGE(LOG_DUMP, "TEST STEP ----\n");
            extcache_native_t save;
            fpu_save_and_unwind(dyn, ninst, &save);
            fpu_reflectcache(dyn, ninst, x1, x2, x3);
            GO_TRACE(x64test_step, 1, x5);
            fpu_unreflectcache(dyn, ninst, x1, x2, x3);
            fpu_unwind_restore(dyn, ninst, &save);
            MESSAGE(LOG_DUMP, "----------\n");
        }
        if (BOX64DRENV(dynarec_dump) && (!BOX64ENV(dynarec_dump_range_end) || (ip >= BOX64ENV(dynarec_dump_range_start) && ip < BOX64ENV(dynarec_dump_range_end)))) {
            dyn->need_dump = BOX64DRENV(dynarec_dump);
        }
        #ifdef HAVE_TRACE
        else if(my_context->dec && BOX64ENV(dynarec_trace)) {
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

        uint8_t pk = PK(0);
        
        rex.rex = 0;
        rex.seg = 0;
        rex.offset = 0;
        rex.is32bits = is32bits;
        rex.is66 = 0;
        rex.is67 = 0;
        rex.isf0 = 0;
        rex.rep = 0;
        while((pk==0xF2) || (pk==0xF3) || (pk==0xf0)
            || (pk==0x3E) || (pk==0x26) || (pk==0x2e) || (pk==0x36) 
            || (pk==0x64) || (pk==0x65) || (pk==0x66) || (pk==0x67)
            || (!is32bits && (pk>=0x40 && pk<=0x4f))) {
            switch (pk) {
                case 0xF0: rex.isf0 = 1; rex.rex = 0; break;
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
                case 0x40 ... 0x4F: rex.rex = pk; break;
            }
            ++addr;
            pk = PK(0);
        }
        if(rex.isf0) {
            if(rex.is66 && !rex.w)
                addr = dynarec64_66F0(dyn, addr, ip, ninst, rex, &ok, &need_epilog);
            else
                addr = dynarec64_F0(dyn, addr, ip, ninst, rex, &ok, &need_epilog);
        } else if(rex.is66)
            addr = dynarec64_66(dyn, addr, ip, ninst, rex, &ok, &need_epilog);
        else
            addr = dynarec64_00(dyn, addr, ip, ninst, rex, &ok, &need_epilog);
        if(dyn->abort)
            return ip;
        INST_EPILOG;

        #if STEP > 1
        if (is_opcode_volatile || dyn->insts[ninst].lock)
            DMB_ISH();
        #endif
        #ifdef ARM64
        if(dyn->insts[ninst].x64.has_next && dyn->insts[ninst+1].preload_xmmymm) {
            doPreload(dyn, ninst+1);
        }
        #endif

        fpu_reset_scratch(dyn);
        int next = ninst+1;
        #if STEP > 0
        if(dyn->insts[ninst].x64.has_next && dyn->insts[next].x64.barrier) {
            if(dyn->insts[next].x64.barrier&BARRIER_FLOAT) {
                #if defined (RV64) || defined(LA64)
                uint8_t tmp1, tmp2, tmp3;
                if(dyn->insts[next].nat_flags_fusion) get_free_scratch(dyn, next, &tmp1, &tmp2, &tmp3, x1, x2, x3, x4, x5);
                else { tmp1=x1; tmp2=x2; tmp3=x3; }
                fpu_purgecache(dyn, ninst, 0, tmp1, tmp2, tmp3);
                #else
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3, 0);
                #endif
            }
            if(dyn->insts[next].x64.barrier&BARRIER_FLAGS) {
                #ifdef ARM64
                dyn->f = status_unk;
                #else
                dyn->f.pending = 0;
                dyn->f.dfnone = 0;
                #endif
                dyn->last_ip = 0;
            }
        }
        #endif
        #ifndef PROT_READ
        #define PROT_READ 1
        #endif
        #if STEP == 0
        if(dynarec_dirty && ok && is_addr_autosmc(ip)) {
            // this is the last opcode, because it will write in current block if not stopped
            ok = 0;
            need_epilog = 1;
        }
        #endif
        #if STEP != 0
        if(!ok && !need_epilog && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
            // we use the 1st predecessor here
            if((ninst+1)<dyn->size && !dyn->insts[ninst+1].x64.alive) {
                // reset fpu value...
                #ifdef ARM64
                dyn->f = status_unk;
                #else
                dyn->f.dfnone = 0;
                dyn->f.pending = 0;
                #endif
                fpu_reset(dyn);
                while((ninst+1)<dyn->size && !dyn->insts[ninst+1].x64.alive) {
                    // may need to skip opcodes to advance
                    ++ninst;
                    NEW_INST;
                    MESSAGE(LOG_DEBUG, "Skipping unused opcode\n");
                    INST_NAME("Skipped opcode");
                    addr += dyn->insts[ninst].x64.size;
                    INST_EPILOG;
                }
            }
            if((dyn->insts[ninst+1].x64.barrier&BARRIER_FULL)==BARRIER_FULL)
                reset_n = -2;    // hack to say Barrier!
            else {
                reset_n = getNominalPred(dyn, ninst+1);  // may get -1 if no predecessor are available
                if(reset_n==-1) {
                    reset_n = -2;
                    if(!dyn->insts[ninst].x64.has_callret) {
                        MESSAGE(LOG_DEBUG, "Warning, Reset Caches mark not found\n");
                    }
                }
            }
        }
        #else
        // check if block need to be stopped, because it's a 00 00 opcode (unreadeable is already checked earlier)
        if((ok>0) && !dyn->forward && (!(getProtection(addr+3)&PROT_READ) || !(*(uint32_t*)addr))) {
            if (dyn->need_dump) dynarec_log(LOG_NONE, "Stopping block at %p reason: %s\n", (void*)addr, "Next opcode is 00 00 00 00");
            ok = 0;
            need_epilog = 1;
            dyn->insts[ninst].x64.need_after |= X_PEND;
        }
        if(dyn->forward) {
            if(dyn->forward_to == addr && !need_epilog && ok>=0) {
                // we made it!
                reset_n = get_first_jump_addr(dyn, addr);
                if (dyn->need_dump) dynarec_log(LOG_NONE, "Forward extend block for %d bytes %s%p -> %p (ninst %d - %d)\n", dyn->forward_to - dyn->forward, dyn->insts[dyn->forward_ninst].x64.has_callret ? "(opt. call) " : "", (void*)dyn->forward, (void*)dyn->forward_to, reset_n, ninst);
                if(dyn->insts[dyn->forward_ninst].x64.has_callret && !dyn->insts[dyn->forward_ninst].x64.has_next)
                    dyn->insts[dyn->forward_ninst].x64.has_next = 1;  // this block actually continue
                dyn->forward = 0;
                dyn->forward_to = 0;
                dyn->forward_size = 0;
                dyn->forward_ninst = 0;
                ok = 1; // in case it was 0
            } else if ((dyn->forward_to < addr) || ok<=0) {
                // something when wrong! rollback
                if (dyn->need_dump) dynarec_log(LOG_NONE, "Could not forward extend block for %d bytes %p -> %p\n", dyn->forward_to - dyn->forward, (void*)dyn->forward, (void*)dyn->forward_to);
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
        } else if (!ok && !need_epilog && BOX64DRENV(dynarec_bigblock) && (getProtection(addr + 3) & ~PROT_READ))
            if(*(uint32_t*)addr!=0) {   // check if need to continue (but is next 4 bytes are 0, stop)
                uintptr_t next = get_closest_next(dyn, addr);
                if(next && (
                    (((next-addr)<15) && is_nops(dyn, addr, next-addr))))
                {
                    ok = 1;
                    if(dyn->insts[ninst].x64.has_callret && !dyn->insts[ninst].x64.has_next) {
                        dyn->insts[ninst].x64.has_next = 1;  // this block actually continue
                    } else {
                        // need to find back that instruction to copy the caches, as previous version cannot be used anymore
                        // and pred table is not ready yet
                        reset_n = get_first_jump_addr(dyn, next);
                    }
                    if (dyn->need_dump) dynarec_log(LOG_NONE, "Extend block %p, %s%p -> %p (ninst=%d, jump from %d)\n", dyn, dyn->insts[ninst].x64.has_callret ? "(opt. call) " : "", (void*)addr, (void*)next, ninst + 1, dyn->insts[ninst].x64.has_callret ? ninst : reset_n);
                } else if (next && (int)(next - addr) < BOX64ENV(dynarec_forward) && (getProtection(next) & PROT_READ) /*BOX64DRENV(dynarec_bigblock)>=stopblock*/) {
                    if (!((BOX64DRENV(dynarec_bigblock) < stopblock) && !isJumpTableDefault64((void*)next))) {
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
        PURGE_YMM();
        ++ninst;
        #if STEP == 0
        memset(&dyn->insts[ninst], 0, sizeof(instruction_native_t));
        if ((ok > 0) && (((BOX64DRENV(dynarec_bigblock) < stopblock) && !isJumpTableDefault64((void*)addr)) || (addr >= BOX64ENV(nodynarec_start) && addr < BOX64ENV(nodynarec_end))))
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
            MESSAGE(LOG_DEBUG, "Stopping block %p (%d / %d)\n",(void*)init_addr, ninst, dyn->size);
            if (!dyn->need_dump && addr >= BOX64ENV(nodynarec_start) && addr < BOX64ENV(nodynarec_end))
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
            #if defined (RV64) || defined(LA64)
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            #else
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3, 0);
            #endif
            jump_to_next(dyn, addr, 0, ninst, rex.is32bits);
            ok=0; need_epilog=0;
        }
    }
    if(need_epilog) {
        NOTEST(x3);
        #if defined (RV64) || defined(LA64)
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
        #else
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3, 0);
        #endif
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknown instruction
    }
    FINI;
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d)\n", dyn->size);
    return addr;
}
