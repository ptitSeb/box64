#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "threads.h"
#include "alternate.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynablock_private.h"
#include "bridge.h"
#include "dynarec_next.h"
#include "custommem.h"
#include "x64test.h"
#endif
#ifdef HAVE_TRACE
#include "elfloader.h"
#endif

extern int running32bits;
#ifdef DYNAREC
void* LinkNext(x64emu_t* emu, uintptr_t addr, void* x2, uintptr_t* x3)
{
    int is32bits = (R_CS == 0x23);
    if(!running32bits && is32bits) running32bits=1;
    #ifndef HAVE_ALTJUMP
    // inefficient way to handle alternate without ALTJUMP
    uintptr_t new_addr = (uintptr_t)getAlternate((void*)addr);
    if(new_addr!=addr) {
        *x3 = new_addr;
        addr = new_addr;
    }
    #endif
    #ifdef HAVE_TRACE
    if(!addr) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_INFO, "Warning, jumping to NULL address from %p (db=%p, x64addr=%p/%s)\n", x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    } else if(addr<0x10000) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_INFO, "Warning, jumping to low address %p from %p (db=%p, x64addr=%p/%s)\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    #ifdef BOX32
    } else if(emu->segs[_CS]==0x23 && addr>0x100000000LL) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_INFO, "Warning, jumping to high address %p from %p (db=%p, x64addr=%p/%s)\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    #endif
    } else if(!memExist(addr)) {
        dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
        printf_log(LOG_INFO, "Warning, jumping to an unmapped address %p from %p (db=%p, x64addr=%p/%s)\n", (void*)addr, x2-4, db, db?(void*)getX64Address(db, (uintptr_t)x2-4):NULL, db?getAddrFunctionName(getX64Address(db, (uintptr_t)x2-4)):"(nil)");
    }
    #endif
    void * jblock;
    dynablock_t* block = NULL;
    block = DBGetBlock(emu, addr, 1, is32bits);
    if(!block) {
        #ifdef HAVE_TRACE
        if(LOG_INFO<=BOX64ENV(dynarec_log)) {
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
        if(BOX64ENV(dynarec_log) && !block->isize) {
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
    if(block->sep_size && (uintptr_t)block->x64_addr!=addr) {
        jblock = NULL;
        for(int i=0; i<block->sep_size && !jblock; ++i) {
            if(addr==(uintptr_t)block->x64_addr + block->sep[i].x64_offs)
                jblock = block->block + block->sep[i].nat_offs;
        }
        if(!jblock) {
            printf_log(LOG_NONE, "Warning, cannot find Secondary Entry Point %p in dynablock %p\n", addr, block);
            return native_epilog;
        }
    }
    //dynablock_t *father = block->father?block->father:block;
    return jblock;
}
#endif

void DynaCall(x64emu_t* emu, uintptr_t addr, int no_alt)
{
    uint64_t old_rsp = R_RSP;
    uint64_t old_rbx = R_RBX;
    uint64_t old_rdi = R_RDI;
    uint64_t old_rsi = R_RSI;
    uint64_t old_rbp = R_RBP;
    uint64_t old_rip = R_RIP;
    x64flags_t old_eflags = emu->eflags;
    // save defered flags
    deferred_flags_t old_df = emu->df;
    multiuint_t old_op1 = emu->op1;
    multiuint_t old_op2 = emu->op2;
    multiuint_t old_res = emu->res;

    #ifdef BOX32
    if(box64_is32bits)
        PushExit_32(emu);
    else
    #endif
        PushExit(emu);
    R_RIP = addr;
    emu->df = d_none;
    if(emu->flags.quitonlongjmp)
        emu->flags.need_jmpbuf = 1;
    EmuRun(emu, 1, no_alt);
    emu->quit = 0;  // reset Quit flags...
    emu->df = d_none;
    if(emu->flags.quitonlongjmp && emu->flags.longjmp) {
        if(emu->flags.quitonlongjmp==1)
            emu->flags.longjmp = 0;   // don't change anything because of the longjmp
    } else {
        // restore defered flags
        emu->df = old_df;
        emu->op1 = old_op1;
        emu->op2 = old_op2;
        emu->res = old_res;
        // and the old registers
        emu->eflags = old_eflags;
        R_RBX = old_rbx;
        R_RDI = old_rdi;
        R_RSI = old_rsi;
        R_RBP = old_rbp;
        R_RSP = old_rsp;
        R_RIP = old_rip;  // and set back instruction pointer
    }
}

#ifdef DYNAREC
static dynablock_t* fastDBGetBlock(x64emu_t* emu, uintptr_t addr, int create, int is32bits)
{
    dynablock_t* ret = getDBnoTest(R_RIP);
    if(!ret)
        ret = DBGetBlock(emu, R_RIP, 1, is32bits);
    return ret;
}
#endif

void EmuRun(x64emu_t* emu, int use_dynarec, int no_alt)
{
    // prepare setjump for signal handling
    JUMPBUFF jmpbuf[1] = {0};
    int skip = 0;
    JUMPBUFF *old_jmpbuf = emu->jmpbuf;
    #if defined(RV64) || defined(PPC64LE)
    uintptr_t old_savesp = emu->xSPSave;
    #endif
    int is32bits = (emu->segs[_CS]==0x23);
    while(!(emu->quit)) {
        if(!emu->jmpbuf || (emu->flags.need_jmpbuf && emu->jmpbuf!=jmpbuf)) {
            emu->jmpbuf = jmpbuf;
            #if defined(RV64) || defined(PPC64LE)
            emu->old_savedsp = emu->xSPSave;
            #endif
            emu->flags.jmpbuf_ready = 1;
            #ifdef ANDROID
            if ((skip = SigSetJmp(*(JUMPBUFF*)emu->jmpbuf, 1)))
            #else
            if ((skip = SigSetJmp(emu->jmpbuf, 1)))
            #endif
            {
                dynarec_log(LOG_DEBUG, "Setjmp EmuRun, fs=0x%x will %sskip dynarec next\n", emu->segs[_FS], (skip==3)?"not ":"");
                #ifdef DYNAREC
                if(BOX64ENV(dynarec_test)) {
                    if(emu->test.clean)
                        x64test_check(emu, R_RIP);
                    emu->test.clean = 0;
                }
                #endif
                if(skip==3)
                    skip = 0;
            }
        }
        if(emu->flags.need_jmpbuf)
            emu->flags.need_jmpbuf = 0;
        if(no_alt)
            { no_alt = 0; skip = 1;}    // the Dynarec will shadow the Entry Point, so using Intperter to enter the function
        else
            R_RIP = (uintptr_t)getAlternate((void*)R_RIP);
#ifdef DYNAREC
        if(!BOX64ENV(dynarec) || !use_dynarec)
#endif
            Run(emu, 0);
#ifdef DYNAREC
        else {
            int newis32bits = (emu->segs[_CS]==0x23);
            if(newis32bits!=is32bits) {
                is32bits = newis32bits;
                if(is32bits) {
                    // Zero upper part of the 32bits regs
                    R_RAX = R_EAX;
                    R_RBX = R_EBX;
                    R_RCX = R_ECX;
                    R_RDX = R_EDX;
                    R_RSP = R_ESP;
                    R_RBP = R_EBP;
                    R_RSI = R_ESI;
                    R_RDI = R_EDI;
                    running32bits = 1;
                }
            }
            dynablock_t* block = (skip || ACCESS_FLAG(F_TF))?NULL:fastDBGetBlock(emu, R_RIP, 1, is32bits);
            if(!block || !block->block || !block->done || ACCESS_FLAG(F_TF)) {
                // no block, or block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                if(BOX64ENV(dynarec_log)) {
                    if(ACCESS_FLAG(F_TF))
                        dynarec_log(LOG_INFO, "%04d|Running Interpreter @%p, emu=%p because TF is on\n", GetTID(), (void*)R_RIP, emu);
                    else
                        dynarec_log(LOG_DEBUG, "%04d|Running Interpreter @%p, emu=%p (skip=%d)\n", GetTID(), (void*)R_RIP, emu, skip);
                }
                skip = 0;
                if (BOX64ENV(dynarec_test))
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Running DynaRec Block @%p (%p) of %d x64 insts (hash=0x%x) emu=%p\n", GetTID(), (void*)R_RIP, block->block, block->isize, block->hash, emu);
                if(!BOX64ENV(dynarec_df)) {
                    CHECK_FLAGS(emu);
                }
                // block is here, let's run it!
                void* jblock = block->block;
                if(block->sep_size && R_RIP!=(uintptr_t)block->x64_addr) {
                    jblock = NULL;
                    for(int i=0; i<block->sep_size && !jblock; ++i) {
                        if(R_RIP==(uintptr_t)block->x64_addr + block->sep[i].x64_offs)
                            jblock = block->block + block->sep[i].nat_offs;
                    }
                }
                if(!jblock) {
                    printf_log(LOG_NONE, "Warning, cannot find Secondary Entry Point %p in dynablock %p\n", (void*)R_RIP, block);
                    skip = 1;
                } else
                    native_prolog(emu, jblock);
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = EmuFork(emu, forktype);
            }
        }
#endif
        if(emu->flags.need_jmpbuf)
            emu->quit = 0;
    }
    // clear the setjmp
    emu->jmpbuf = old_jmpbuf;
    #if defined(RV64) || defined(PPC64LE)
    emu->xSPSave = old_savesp;
    #endif
}

void DynaRun(x64emu_t *emu)
{
    EmuRun(emu, 1, 0);
}
