#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
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
#include "custommem.h"
#endif
#ifdef HAVE_TRACE
#include "elfloader.h"
#endif

#ifdef DYNAREC
uintptr_t getX64Address(dynablock_t* db, uintptr_t arm_addr);

void* LinkNext(x64emu_t* emu, uintptr_t addr, void* x2, uintptr_t* x3)
{
    int is32bits = (R_CS == 0x23);
    #ifdef HAVE_TRACE
    if(!addr) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_NONE, "Warning, jumping to NULL address from %p (db=%p, x64addr=%p/%s)\n", x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    } else if(addr<0x10000) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_NONE, "Warning, jumping to low address %p from %p (db=%p, x64addr=%p/%s)\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    }
    #endif
    void * jblock;
    dynablock_t* block = NULL;
    if(hasAlternate((void*)addr)) {
        printf_log(LOG_DEBUG, "Jmp address has alternate: %p", (void*)addr);
        if(box64_log<LOG_DEBUG) dynarec_log(LOG_INFO, "Jmp address has alternate: %p", (void*)addr);
        uintptr_t old_addr = addr;
        addr = (uintptr_t)getAlternate((void*)addr);    // set new address
        R_RIP = addr;   // but also new RIP!
        *x3 = addr; // and the RIP in x27 register
        printf_log(LOG_DEBUG, " -> %p\n", (void*)addr);
        block = DBAlternateBlock(emu, old_addr, addr, is32bits);
    } else
        block = DBGetBlock(emu, addr, 1, is32bits);
    if(!block) {
        #ifdef HAVE_TRACE
        if(LOG_INFO<=box64_dynarec_log) {
            if(checkInHotPage(addr)) {
                dynarec_log(LOG_INFO, "Not trying to run a block from a Hotpage at %p\n", (void*)addr);
            } else {
                dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
                elfheader_t* h = FindElfAddress(my_context, (uintptr_t)x2-4);
                dynarec_log(LOG_INFO, "Warning, jumping to a no-block address %p from %p (db=%p, x64addr=%p(elf=%s), RIP=%p)\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, h?ElfName(h):"(none)", (void*)*x3);
            }
        }
        #endif
        //tableupdate(native_epilog, addr, table);
        return native_epilog;
    }
    if(!block->done) {
        // not finished yet... leave linker
        #ifdef HAVE_TRACE
        if(box64_dynarec_log && !block->isize) {
            dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
            printf_log(LOG_NONE, "Warning, NULL block at %p from %p (db=%p, x64addr=%p/%s)\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
        }
        #endif
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

void DynaCall(x64emu_t* emu, uintptr_t addr)
{
    uint64_t old_rsp = R_RSP;
    uint64_t old_rbx = R_RBX;
    uint64_t old_rdi = R_RDI;
    uint64_t old_rsi = R_RSI;
    uint64_t old_rbp = R_RBP;
    uint64_t old_rip = R_RIP;
    // save defered flags
    deferred_flags_t old_df = emu->df;
    multiuint_t old_op1 = emu->op1;
    multiuint_t old_op2 = emu->op2;
    multiuint_t old_res = emu->res;
    multiuint_t old_op1_sav= emu->op1_sav;
    multiuint_t old_res_sav= emu->res_sav;
    deferred_flags_t old_df_sav= emu->df_sav;
    // uc_link
    x64_ucontext_t* old_uc_link = emu->uc_link;
    emu->uc_link = NULL;

    PushExit(emu);
    R_RIP = addr;
    emu->df = d_none;
    DynaRun(emu);
    emu->quit = 0;  // reset Quit flags...
    emu->df = d_none;
    emu->uc_link = old_uc_link;
    if(emu->flags.quitonlongjmp && emu->flags.longjmp) {
        if(emu->flags.quitonlongjmp==1)
            emu->flags.longjmp = 0;   // don't change anything because of the longjmp
    } else {
        // restore defered flags
        emu->df = old_df;
        emu->op1 = old_op1;
        emu->op2 = old_op2;
        emu->res = old_res;
        emu->op1_sav = old_op1_sav;
        emu->res_sav = old_res_sav;
        emu->df_sav = old_df_sav;
        // and the old registers
        R_RBX = old_rbx;
        R_RDI = old_rdi;
        R_RSI = old_rsi;
        R_RBP = old_rbp;
        R_RSP = old_rsp;
        R_RIP = old_rip;  // and set back instruction pointer
    }
}

int my_setcontext(x64emu_t* emu, void* ucp);
void DynaRun(x64emu_t* emu)
{
    // prepare setjump for signal handling
    JUMPBUFF jmpbuf[1] = {0};
    int skip = 0;
    JUMPBUFF *old_jmpbuf = emu->jmpbuf;
    #ifdef RV64
    uintptr_t old_savesp = emu->xSPSave;
    #endif
    emu->flags.jmpbuf_ready = 0;

    while(!(emu->quit)) {
        if(!emu->jmpbuf || (emu->flags.need_jmpbuf && emu->jmpbuf!=jmpbuf)) {
            emu->jmpbuf = jmpbuf;
            #ifdef RV64
            emu->old_savedsp = emu->xSPSave;
            #endif
            emu->flags.jmpbuf_ready = 1;
            #ifdef ANDROID
            if((skip=sigsetjmp(*(JUMPBUFF*)emu->jmpbuf, 1)))
            #else
            if((skip=sigsetjmp(emu->jmpbuf, 1)))
            #endif
            {
                printf_log(LOG_DEBUG, "Setjmp DynaRun, fs=0x%x\n", emu->segs[_FS]);
                #ifdef DYNAREC
                if(box64_dynarec_test) {
                    if(emu->test.clean)
                        x64test_check(emu, R_RIP);
                    emu->test.clean = 0;
                }
                #endif
            }
        }
        if(emu->flags.need_jmpbuf)
            emu->flags.need_jmpbuf = 0;

#ifdef DYNAREC
        if(!box64_dynarec)
#endif
            Run(emu, 0);
#ifdef DYNAREC
        else {
            int is32bits = (emu->segs[_CS]==0x23);
            dynablock_t* block = (skip)?NULL:DBGetBlock(emu, R_RIP, 1, is32bits);
            if(!block || !block->block || !block->done || ACCESS_FLAG(F_TF)) {
                skip = 0;
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "%04d|Running Interpreter @%p, emu=%p\n", GetTID(), (void*)R_RIP, emu);
                if(box64_dynarec_test)
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Running DynaRec Block @%p (%p) of %d x64 insts (hash=0x%x) emu=%p\n", GetTID(), (void*)R_RIP, block->block, block->isize, block->hash, emu);
                // block is here, let's run it!
                native_prolog(emu, block->block);
                extern int running32bits;
                if(emu->segs[_CS]==0x23)
                    running32bits = 1;
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = x64emu_fork(emu, forktype);
            }
            if(emu->quit && emu->uc_link) {
                emu->quit = 0;
                my_setcontext(emu, emu->uc_link);
            }
        }
#endif
        if(emu->flags.need_jmpbuf)
            emu->quit = 0;
    }
    // clear the setjmp
    emu->jmpbuf = old_jmpbuf;
    #ifdef RV64
    emu->xSPSave = old_savesp;
    #endif
}
