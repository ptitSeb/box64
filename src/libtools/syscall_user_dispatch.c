#define _GNU_SOURCE
#include <errno.h>
#include <elf.h>
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>

#ifdef __linux__
#include <linux/audit.h>
#endif

#include "x64_signals.h"
#include "os.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "debug.h"
#include "custommem.h"
#include "signals.h"
#include "syscall_user_dispatch.h"

#ifndef SYS_USER_DISPATCH
#define SYS_USER_DISPATCH 2
#endif

#ifndef __AUDIT_ARCH_64BIT
#define __AUDIT_ARCH_64BIT 0x80000000U
#endif

#ifndef __AUDIT_ARCH_LE
#define __AUDIT_ARCH_LE 0x40000000U
#endif

#ifndef AUDIT_ARCH_X86_64
#define AUDIT_ARCH_X86_64 (EM_X86_64 | __AUDIT_ARCH_64BIT | __AUDIT_ARCH_LE)
#endif

#ifndef AUDIT_ARCH_I386
#define AUDIT_ARCH_I386 (EM_386 | __AUDIT_ARCH_LE)
#endif

int my_sigactionhandler_oldcode_64(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void* ucntx, int* old_code, void* cur_db);
#ifdef BOX32
int my_sigactionhandler_oldcode_32(x64emu_t* emu, int32_t sig, int simple, siginfo_t* info, void* ucntx, int* old_code, void* cur_db);
#endif

static void sud_emit_sigsys(x64emu_t* emu, uintptr_t call_addr, long syscall_nr, int is32bits)
{
    siginfo_t info = {0};

    info.si_signo = X64_SIGSYS;
    info.si_errno = 0;
    info.si_code = SYS_USER_DISPATCH;
    info.si_call_addr = (void*)call_addr;
    info.si_syscall = (int)syscall_nr;
    info.si_arch = is32bits ? AUDIT_ARCH_I386 : AUDIT_ARCH_X86_64;

#ifdef BOX32
    if(is32bits) {
        my_sigactionhandler_oldcode_32(emu, X64_SIGSYS, 0, &info, NULL, NULL, NULL);
        return;
    }
#else
    (void)is32bits;
#endif
    my_sigactionhandler_oldcode_64(emu, X64_SIGSYS, 0, &info, NULL, NULL, NULL);
}

long my_syscall_user_dispatch_prctl(x64emu_t* emu, unsigned long op, unsigned long offset, unsigned long len, void* selector)
{
    if(!emu) return -EINVAL;

    switch(op) {
        case PR_SYS_DISPATCH_OFF:
            if(offset || len || selector)
                return -EINVAL;
            emu->sud_enabled = 0;
            emu->sud_selector = NULL;
            emu->sud_offset = 0;
            emu->sud_len = 0;
            printf_log(LOG_DEBUG, "Disabled syscall user dispatch\n");
            return 0;

        case PR_SYS_DISPATCH_ON:
            if(offset && offset + len <= offset)
                return -EINVAL;
            break;

        case PR_SYS_DISPATCH_INCLUSIVE_ON:
            if(!len || offset + len <= offset)
                return -EINVAL;
            offset += len;
            len = -len;
            break;

        default:
            return -EINVAL;
    }

    if(selector && (!memExist((uintptr_t)selector) || !(getProtection((uintptr_t)selector) & PROT_READ))) {
        return -EFAULT;
    }

    emu->sud_selector = (uint8_t*)selector;
    emu->sud_offset = offset;
    emu->sud_len = len;
    emu->sud_enabled = 1;
    printf_log(LOG_DEBUG, "Enabled syscall user dispatch: allowed=%p-%p selector=%p\n",
               (void*)offset, (void*)(offset + len), selector);
    return 0;
}

int my_syscall_user_dispatch(x64emu_t* emu, uintptr_t call_addr, long syscall_nr, int is32bits)
{
    uint8_t state;

    if(!emu || !emu->sud_enabled)
        return 0;

    if(call_addr - emu->sud_offset < emu->sud_len)
        return 0;

    if(emu->sud_selector) {
        if(!memExist((uintptr_t)emu->sud_selector) || !(getProtection((uintptr_t)emu->sud_selector) & PROT_READ)) {
            printf_log(LOG_NONE, "Warning: invalid syscall user dispatch selector %p\n", emu->sud_selector);
            return 0;
        }

        state = *emu->sud_selector;
        if(state == SYSCALL_DISPATCH_FILTER_ALLOW)
            return 0;

        if(state != SYSCALL_DISPATCH_FILTER_BLOCK) {
            printf_log(LOG_NONE, "Warning: invalid syscall user dispatch selector value %u at %p\n", state, emu->sud_selector);
            return 0;
        }
    }

    printf_log(LOG_DEBUG, "%04d|Syscall user dispatch: syscall=%ld call_addr=%p arch=%s\n",
               GetTID(), syscall_nr, (void*)call_addr, is32bits ? "i386" : "x86_64");
    sud_emit_sigsys(emu, call_addr, syscall_nr, is32bits);
    return 1;
}
