#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <setjmp.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "tools/bridge_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "threads.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynablock_private.h"
#include "bridge.h"
#include "dynarec_next.h"
#endif
#ifdef HAVE_TRACE
#include "elfloader.h"
#endif

#ifdef DYNAREC
uintptr_t getX64Address(dynablock_t* db, uintptr_t arm_addr);

void* LinkNext(x64emu_t* emu, uintptr_t addr, void* x2, uintptr_t* x3)
{
    #ifdef HAVE_TRACE
    if(!addr) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_NONE, "Warning, jumping to NULL address from %p (db=%p, x64addr=%p/%s)\n", x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    }
    #endif
    void * jblock;
    dynablock_t* block = DBGetBlock(emu, addr, 1);
    if(!block) {
        // no block, let link table as is...
        if(hasAlternate((void*)addr)) {
            printf_log(LOG_DEBUG, "Jmp address has alternate: %p", (void*)addr);
            if(box64_log<LOG_DEBUG) dynarec_log(LOG_INFO, "Jmp address has alternate: %p", (void*)addr);
            addr = (uintptr_t)getAlternate((void*)addr);    // set new address
            R_RIP = addr;   // but also new RIP!
            *x3 = addr; // and the RIP in x27 register
            printf_log(LOG_DEBUG, " -> %p\n", (void*)addr);
            block = DBGetBlock(emu, addr, 1);
        }
        if(!block) {
            #ifdef HAVE_TRACE
            if(LOG_INFO<=box64_dynarec_log) {
                dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
                elfheader_t* h = FindElfAddress(my_context, (uintptr_t)x2-4);
                dynarec_log(LOG_INFO, "Warning, jumping to a no-block address %p from %p (db=%p, x64addr=%p(elf=%s))\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, h?ElfName(h):"(none)");
            }
            #endif
            //tableupdate(native_epilog, addr, table);
            return native_epilog;
        }
    }
    if(!block->done) {
        // not finished yet... leave linker
        return native_epilog;
    }
    if(!(jblock=block->block)) {
        // null block, but done: go to epilog, no linker here
        return native_epilog;
    }
    //dynablock_t *father = block->father?block->father:block;
    return jblock;
}
#endif

#ifdef __GNUC__
// Disable "clobbered" warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
void DynaCall(x64emu_t* emu, uintptr_t addr)
{
    // prepare setjump for signal handling
    emu_jmpbuf_t *ejb = NULL;
    int jmpbuf_reset = 0;
    int skip = 0;
    if(emu->type == EMUTYPE_MAIN) {
        ejb = GetJmpBuf();
        if(!ejb->jmpbuf_ok) {
            ejb->emu = emu;
            ejb->jmpbuf_ok = 1;
            jmpbuf_reset = 1;
            if((skip = sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1))) {
                printf_log(LOG_DEBUG, "Setjmp DynaCall, fs=0x%x\n", ejb->emu->segs[_FS]);
                addr = R_RIP;   // not sure if it should still be inside DynaCall!
                #ifdef DYNAREC
                if(box64_dynarec_test) {
                    if(emu->test.clean)
                        x64test_check(emu, R_RIP);
                    emu->test.clean = 0;
                }
                #endif
                if(skip!=2)
                    skip = 0;
            }
        }
    }
#ifdef DYNAREC
    if(!box64_dynarec)
#endif
        EmuCall(emu, addr);
#ifdef DYNAREC
    else {
        uint64_t old_rsp = R_RSP;
        uint64_t old_rbx = R_RBX;
        uint64_t old_rdi = R_RDI;
        uint64_t old_rsi = R_RSI;
        uint64_t old_rbp = R_RBP;
        uint64_t old_rip = R_RIP;
        PushExit(emu);
        R_RIP = addr;
        emu->df = d_none;
        while(!emu->quit) {
            dynablock_t* block = (skip==2)?NULL:DBGetBlock(emu, R_RIP, 1);
            if(!block || !block->block || !block->done) {
                skip = 0;
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "%04d|Calling Interpreter @%p, emu=%p\n", GetTID(), (void*)R_RIP, emu);
                if(box64_dynarec_test)
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Calling DynaRec Block @%p (%p) of %d x64 instructions emu=%p\n", GetTID(), (void*)R_RIP, block->block, block->isize ,emu);
                CHECK_FLAGS(emu);
                // block is here, let's run it!
                native_prolog(emu, block->block);
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = x64emu_fork(emu, forktype);
                if(emu->type == EMUTYPE_MAIN) {
                    ejb = GetJmpBuf();
                    ejb->emu = emu;
                    ejb->jmpbuf_ok = 1;
                    jmpbuf_reset = 1;
                    if(sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1)) {
                        printf_log(LOG_DEBUG, "Setjmp inner DynaCall, fs=0x%x\n", ejb->emu->segs[_FS]);
                        addr = R_RIP;
                    }
                }
            }
        }
        emu->quit = 0;  // reset Quit flags...
        emu->df = d_none;
        if(emu->quitonlongjmp && emu->longjmp) {
            if(emu->quitonlongjmp==1)
                emu->longjmp = 0;   // don't change anything because of the longjmp
        } else {
            R_RBX = old_rbx;
            R_RDI = old_rdi;
            R_RSI = old_rsi;
            R_RBP = old_rbp;
            R_RSP = old_rsp;
            R_RIP = old_rip;  // and set back instruction pointer
        }
    }
#endif
    // clear the setjmp
    if(ejb && jmpbuf_reset)
        ejb->jmpbuf_ok = 0;
}

int DynaRun(x64emu_t* emu)
{
    // prepare setjump for signal handling
    emu_jmpbuf_t *ejb = NULL;
    int skip;
#ifdef DYNAREC
    int jmpbuf_reset = 1;
#endif
    if(emu->type == EMUTYPE_MAIN) {
        ejb = GetJmpBuf();
        if(!ejb->jmpbuf_ok) {
            ejb->emu = emu;
            ejb->jmpbuf_ok = 1;
#ifdef DYNAREC
            jmpbuf_reset = 1;
#endif
            if((skip=sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1)))
                printf_log(LOG_DEBUG, "Setjmp DynaRun, fs=0x%x\n", ejb->emu->segs[_FS]);
                #ifdef DYNAREC
                if(box64_dynarec_test) {
                    if(emu->test.clean)
                        x64test_check(emu, R_RIP);
                    emu->test.clean = 0;
                }
                #endif
                if(skip!=2)
                    skip = 0;
        }
    }
#ifdef DYNAREC
    if(!box64_dynarec)
#endif
        return Run(emu, 0);
#ifdef DYNAREC
    else {
        while(!emu->quit) {
            dynablock_t* block = (skip==2)?NULL:DBGetBlock(emu, R_RIP, 1);
            if(!block || !block->block || !block->done) {
                skip = 0;
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "%04d|Running Interpreter @%p, emu=%p\n", GetTID(), (void*)R_RIP, emu);
                if(box64_dynarec_test)
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Running DynaRec Block @%p (%p) of %d x64 insts emu=%p\n", GetTID(), (void*)R_RIP, block->block, block->isize, emu);
                // block is here, let's run it!
                native_prolog(emu, block->block);
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = x64emu_fork(emu, forktype);
                if(emu->type == EMUTYPE_MAIN) {
                    ejb = GetJmpBuf();
                    ejb->emu = emu;
                    ejb->jmpbuf_ok = 1;
                    jmpbuf_reset = 1;
                    if(sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1))
                        printf_log(LOG_DEBUG, "Setjmp inner DynaRun, fs=0x%x\n", ejb->emu->segs[_FS]);
                }
            }
        }
    }
    // clear the setjmp
    if(ejb && jmpbuf_reset)
        ejb->jmpbuf_ok = 0;
    return 0;
#endif
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
