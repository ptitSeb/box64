#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#ifndef ANDROID
#include <execinfo.h>
#endif

#include "box64context.h"
#include "custommem.h"
#include "debug.h"
#include "elfloader.h"
#include "emit_signals.h"
#include "emu/x64emu_private.h"
#include "regs.h"
#include "signals.h"
#include "x64emu.h"

void my_sigactionhandler_oldcode(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void * ucntx, int* old_code, void* cur_db, uintptr_t x64pc);
void EmitSignal(x64emu_t* emu, int sig, void* addr, int code)
{
    siginfo_t info = { 0 };
    info.si_signo = sig;
    info.si_errno = (sig == SIGSEGV) ? 0x1234 : 0; // Mark as a sign this is a #GP(0) (like privileged instruction)
    info.si_code = code;
    if (sig == SIGSEGV && code == 0xbad0) {
        info.si_errno = 0xbad0;
        info.si_code = 0;
    } else if (sig == SIGSEGV && code == 0xecec) {
        info.si_errno = 0xecec;
        info.si_code = SEGV_ACCERR;
    } else if (sig == SIGSEGV && code == 0xb09d) {
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
        printf_log(LOG_NONE, "Emit Signal %d at IP=%p(%s / %s) / addr=%p, code=0x%x\n", sig, (void*)R_RIP, x64name ? x64name : "???", elfname ? elfname : "?", addr, code);
        print_rolling_log(LOG_INFO);

        if ((BOX64ENV(showbt) || sig == SIGABRT) && BOX64ENV(log) >= LOG_INFO) {
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
        if (sig == SIGILL) {
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
    while ((getProtection_fast(addr) & (PROT_EXEC | PROT_READ)) != (PROT_EXEC | PROT_READ)) {
        R_RIP = addr; // incase there is a slight difference
        EmitSignal(emu, SIGSEGV, (void*)addr, 0xecec);
    }
}

void EmitInterruption(x64emu_t* emu, int num, void* addr)
{
    siginfo_t info = { 0 };
    info.si_signo = SIGSEGV;
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
    my_sigactionhandler_oldcode(emu, SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
}

void EmitDiv0(x64emu_t* emu, void* addr, int code)
{
    siginfo_t info = { 0 };
    info.si_signo = SIGSEGV;
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
    my_sigactionhandler_oldcode(emu, SIGSEGV, 0, &info, NULL, NULL, NULL, R_RIP);
}
