#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <setjmp.h>
#ifndef ANDROID
#include <execinfo.h>
#endif

#include "x64_signals.h"
#include "box64context.h"
#include "custommem.h"
#include "debug.h"
#include "elfloader.h"
#include "emit_signals.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"
#include "regs.h"
#include "x64emu.h"
#include "signals.h"
#include "libtools/signal_private.h"
#include "box64cpu_util.h"

void my_sigactionhandler_oldcode(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db, uintptr_t x64pc);
void EmitSignal(x64emu_t* emu, int sig, void* addr, int code)
{
    siginfo_t info = { 0 };
    info.si_signo = sig;
    info.si_errno = (sig == X64_SIGSEGV) ? 0x1234 : 0; // Mark as a sign this is a #GP(0) (like privileged instruction)
    info.si_code = code;
    if (sig == X64_SIGSEGV && code == 0xbad0) {
        info.si_errno = 0xbad0;
        info.si_code = 0;
    } else if (sig == X64_SIGSEGV && code == 0xecec) {
        info.si_errno = 0xecec;
        info.si_code = SEGV_ACCERR;
    } else if (sig == X64_SIGSEGV && code == 0xb09d) {
        info.si_errno = 0xb09d;
        info.si_code = 0;
    }
    info.si_addr = addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if (BOX64ENV(log) > LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if (elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "%04d|Emit Signal %d at IP=%p(%s / %s) / addr=%p, code=0x%x\n", GetTID(), sig, (void*)R_RIP, x64name ? x64name : "???", elfname ? elfname : "?", addr, code);
        if(sig==5)
            printf_log(LOG_INFO, "\t Opcode (%02X %02X %02X %02X) %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                Peek(emu, -4), Peek(emu, -3), Peek(emu, -2), Peek(emu, -1),
                Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
                Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7),
                Peek(emu, 8), Peek(emu, 9), Peek(emu,10), Peek(emu,11),
                Peek(emu,12), Peek(emu,13), Peek(emu,14));

        print_rolling_log(LOG_INFO);

        if ((BOX64ENV(showbt) || sig == X64_SIGABRT) && BOX64ENV(log) >= LOG_INFO) {
            // show native bt
            #define BT_BUF_SIZE 100
            int nptrs;
            void* buffer[BT_BUF_SIZE];
            char** strings;

            #ifndef ANDROID
            nptrs = backtrace(buffer, BT_BUF_SIZE);
            strings = backtrace_symbols(buffer, nptrs);
            if (strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(LOG_INFO, "NativeBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(LOG_INFO, "NativeBT: none (%d/%s)\n", errno, strerror(errno));
            #endif
            extern int my_backtrace_ip(x64emu_t * emu, void** buffer, int size); // in wrappedlibc
            extern char** my_backtrace_symbols(x64emu_t * emu, uintptr_t * buffer, int size);
            // save and set real RIP/RSP
            nptrs = my_backtrace_ip(emu, buffer, BT_BUF_SIZE);
            strings = my_backtrace_symbols(emu, (uintptr_t*)buffer, nptrs);
            if (strings) {
                for (int j = 0; j < nptrs; j++)
                    printf_log(LOG_INFO, "EmulatedBT: %s\n", strings[j]);
                free(strings);
            } else
                printf_log(LOG_INFO, "EmulatedBT: none\n");
        }
        printf_log(LOG_NONE, DumpCPURegs(emu, R_RIP, emu->segs[_CS] == 0x23));
        printf_log_prefix(0, LOG_NONE, "Emu Stack: %p 0x%lx%s\n", emu->init_stack, emu->size_stack, emu->stack2free ? " owned" : "");
        // if(!elf) {
        //     FILE* f = fopen("/proc/self/maps", "r");
        //     if(f) {
        //         char line[1024];
        //         while(!feof(f)) {
        //             char* ret = fgets(line, sizeof(line), f);
        //             printf_log(LOG_NONE, "\t%s", ret);
        //         }
        //         fclose(f);
        //     }
        // }
        if (sig == X64_SIGILL) {
            uint8_t* mem = (uint8_t*)R_RIP;
            printf_log(LOG_NONE, "SIGILL: Opcode at ip is %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", mem[0], mem[1], mem[2], mem[3], mem[4], mem[5]);
        }
    }
    my_sigactionhandler_oldcode(emu, sig, 0, &info, NULL, NULL, NULL, R_RIP);
}

void CheckExec(x64emu_t* emu, uintptr_t addr)
{
    if (box64_pagesize != 4096)
        return; // disabling the test, 4K pagesize simlation isn't good enough for this
    while ((getProtection/*_fast*/(addr) & (PROT_EXEC | PROT_READ)) != (PROT_EXEC | PROT_READ)) {
        R_RIP = addr; // incase there is a slight difference
        EmitSignal(emu, X64_SIGSEGV, (void*)addr, 0xecec);
    }
}

void EmitInterruption(x64emu_t* emu, int num, void* addr)
{
    siginfo_t info = { 0 };
    info.si_signo = X64_SIGSEGV;
    info.si_errno = 0xdead;
    info.si_code = num;
    info.si_addr = NULL; // addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if (BOX64ENV(log) > LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if (elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "Emit Interruption 0x%x at IP=%p(%s / %s) / addr=%p\n", num, (void*)R_RIP, x64name ? x64name : "???", elfname ? elfname : "?", addr);
    }
    my_sigactionhandler_oldcode(emu, X64_SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
}

void EmitDiv0(x64emu_t* emu, void* addr, int code)
{
    siginfo_t info = { 0 };
    info.si_signo = X64_SIGSEGV;
    info.si_errno = 0xcafe;
    info.si_code = code;
    info.si_addr = addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if (BOX64ENV(log) > LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if (elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "Emit Divide by 0 at IP=%p(%s / %s) / addr=%p\n", (void*)R_RIP, x64name ? x64name : "???", elfname ? elfname : "?", addr);
    }
    my_sigactionhandler_oldcode(emu, X64_SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
}

void EmitWineInt(x64emu_t* emu, int num, void* addr)
{
    siginfo_t info = { 0 };
    info.si_signo = X64_SIGSEGV;
    info.si_errno = 0xdead;
    info.si_code = num;
    info.si_addr = NULL; // addr;
    const char* x64name = NULL;
    const char* elfname = NULL;
    if (BOX64ENV(log) > LOG_INFO || BOX64ENV(dynarec_dump) || BOX64ENV(showsegv)) {
        x64name = getAddrFunctionName(R_RIP);
        elfheader_t* elf = FindElfAddress(my_context, R_RIP);
        if (elf)
            elfname = ElfName(elf);
        printf_log(LOG_NONE, "Emit Interruption 0x%x at IP=%p(%s / %s) / addr=%p\n", num, (void*)R_RIP, x64name ? x64name : "???", elfname ? elfname : "?", addr);
    }
    if(box64_is32bits)
        my_sigactionhandler_oldcode(emu, X64_SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
    else {
        uintptr_t frame = R_RSP;
        int sig = X64_SIGSEGV;
        // stack tracking
        x64_stack_t *new_ss = my_context->onstack[sig]?sigstack_getstack():NULL;
        int used_stack = 0;
        if(new_ss) {
            if(new_ss->ss_flags == SS_ONSTACK) { // already using it!
                frame = ((uintptr_t)emu->regs[_SP].q[0] - 128) & ~0x0f;
            } else {
                frame = (uintptr_t)(((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~0x0f);
                used_stack = 1;
                new_ss->ss_flags = SS_ONSTACK;
            }
        } else {
            frame -= 0x200; // redzone
        }

        // TODO: do I need to really setup 2 stack frame? That doesn't seems right!
        // setup stack frame
        frame -= 512+64+16*16;
        void* xstate = (void*)frame;
        frame -= sizeof(siginfo_t);
        siginfo_t* info2 = (siginfo_t*)frame;
        memcpy(info2, &info, sizeof(siginfo_t));
        // try to fill some sigcontext....
        frame -= sizeof(x64_ucontext_t);
        x64_ucontext_t   *sigcontext = (x64_ucontext_t*)frame;
        // get general register
        sigcontext->uc_mcontext.gregs[X64_R8] = R_R8;
        sigcontext->uc_mcontext.gregs[X64_R9] = R_R9;
        sigcontext->uc_mcontext.gregs[X64_R10] = R_R10;
        sigcontext->uc_mcontext.gregs[X64_R11] = R_R11;
        sigcontext->uc_mcontext.gregs[X64_R12] = R_R12;
        sigcontext->uc_mcontext.gregs[X64_R13] = R_R13;
        sigcontext->uc_mcontext.gregs[X64_R14] = R_R14;
        sigcontext->uc_mcontext.gregs[X64_R15] = R_R15;
        sigcontext->uc_mcontext.gregs[X64_RAX] = R_RAX;
        sigcontext->uc_mcontext.gregs[X64_RCX] = R_RCX;
        sigcontext->uc_mcontext.gregs[X64_RDX] = R_RDX;
        sigcontext->uc_mcontext.gregs[X64_RDI] = R_RDI;
        sigcontext->uc_mcontext.gregs[X64_RSI] = R_RSI;
        sigcontext->uc_mcontext.gregs[X64_RBP] = R_RBP;
        sigcontext->uc_mcontext.gregs[X64_RSP] = R_RSP;
        sigcontext->uc_mcontext.gregs[X64_RBX] = R_RBX;
        sigcontext->uc_mcontext.gregs[X64_RIP] = R_RIP;
        // flags
        ResetFlags(emu);
        sigcontext->uc_mcontext.gregs[X64_EFL] = emu->eflags.x64;
        // get segments
        sigcontext->uc_mcontext.gregs[X64_CSGSFS] = ((uint64_t)(R_CS)) | (((uint64_t)(R_GS))<<16) | (((uint64_t)(R_FS))<<32) | (((uint64_t)(R_SS))<<48);
        if(R_CS==0x23) {
            // trucate regs to 32bits, just in case
            #define GO(R)   sigcontext->uc_mcontext.gregs[X64_R##R]&=0xFFFFFFFF
            GO(AX);
            GO(CX);
            GO(DX);
            GO(DI);
            GO(SI);
            GO(BP);
            GO(SP);
            GO(BX);
            GO(IP);
            #undef GO
        }
        // get FloatPoint status
        sigcontext->uc_mcontext.fpregs = xstate;//(struct x64_libc_fpstate*)&sigcontext->xstate;
        fpu_xsave_mask(emu, xstate, 0, 0b111);
        memcpy(&sigcontext->xstate, xstate, sizeof(sigcontext->xstate));
        ((struct x64_fpstate*)xstate)->res[12] = 0x46505853;   // magic number to signal an XSTATE type of fpregs
        ((struct x64_fpstate*)xstate)->res[13] = 0; // offset to xstate after this?
        // get signal mask
        if(new_ss) {
            sigcontext->uc_stack.ss_sp = new_ss->ss_sp;
            sigcontext->uc_stack.ss_size = new_ss->ss_size;
            sigcontext->uc_stack.ss_flags = new_ss->ss_flags;
        } else
            sigcontext->uc_stack.ss_flags = SS_DISABLE;        
        // prepare info2
        info2->si_errno = 0;
        info2->si_code = 128;
        info2->si_addr = NULL;
        sigcontext->uc_mcontext.gregs[X64_TRAPNO] = 13;
        sigcontext->uc_mcontext.gregs[X64_ERR] = 0x02|(num<<3);
        int exits = 0;
        int ret;
        ret = RunFunctionHandler(emu, &exits, 2, sigcontext, my_context->signals[info2->si_signo], 3, info2->si_signo, info2, sigcontext);
        if(used_stack)  // release stack
            new_ss->ss_flags = 0;
        // restore values
        // general regs
        R_R8 = sigcontext->uc_mcontext.gregs[X64_R8];
        R_R9 = sigcontext->uc_mcontext.gregs[X64_R9];
        R_R10 = sigcontext->uc_mcontext.gregs[X64_R10];
        R_R11 = sigcontext->uc_mcontext.gregs[X64_R11];
        R_R12 = sigcontext->uc_mcontext.gregs[X64_R12];
        R_R13 = sigcontext->uc_mcontext.gregs[X64_R13];
        R_R14 = sigcontext->uc_mcontext.gregs[X64_R14];
        R_R15 = sigcontext->uc_mcontext.gregs[X64_R15];
        R_RAX = sigcontext->uc_mcontext.gregs[X64_RAX];
        R_RCX = sigcontext->uc_mcontext.gregs[X64_RCX];
        R_RDX = sigcontext->uc_mcontext.gregs[X64_RDX];
        R_RDI = sigcontext->uc_mcontext.gregs[X64_RDI];
        R_RSI = sigcontext->uc_mcontext.gregs[X64_RSI];
        R_RBP = sigcontext->uc_mcontext.gregs[X64_RBP];
        R_RSP = sigcontext->uc_mcontext.gregs[X64_RSP];
        R_RBX = sigcontext->uc_mcontext.gregs[X64_RBX];
        R_RIP = sigcontext->uc_mcontext.gregs[X64_RIP];
        // flags
        emu->eflags.x64 = sigcontext->uc_mcontext.gregs[X64_EFL];
        // get segments
        R_CS = sigcontext->uc_mcontext.gregs[X64_CSGSFS]&0xffff;
        R_GS = (sigcontext->uc_mcontext.gregs[X64_CSGSFS]>>16)&0xffff;
        R_FS = (sigcontext->uc_mcontext.gregs[X64_CSGSFS]>>32)&0xffff;
        R_SS = (sigcontext->uc_mcontext.gregs[X64_CSGSFS]>>48)&0xffff;
        // fpu
        fpu_xrstor_mask(emu, xstate, 0, 0b111);
    }
}

