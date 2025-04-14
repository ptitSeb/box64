#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/personality.h>
#include <dlfcn.h>
#include <string.h>

#include "os.h"
#include "signals.h"
#include "emu/x64int_private.h"
#include "bridge.h"
#include "elfloader.h"
#include "env.h"
#include "debug.h"
#include "x64tls.h"
#include "librarian.h"

int GetTID(void)
{
    return syscall(SYS_gettid);
}

int SchedYield(void)
{
    return sched_yield();
}

int IsBridgeSignature(char s, char c)
{
    return s == 'S' && c == 'C';
}

void EmuInt3(void* emu, void* addr)
{
    return x64Int3((x64emu_t*)emu, (uintptr_t*)addr);
}

int IsNativeCall(uintptr_t addr, int is32bits, uintptr_t* calladdress, uint16_t* retn)
{
    return isNativeCallInternal(addr, is32bits, calladdress, retn);
}

void* EmuFork(void* emu, int forktype)
{
    return x64emu_fork((x64emu_t*)emu, forktype);
}

void EmuX64Syscall(void* emu)
{
    x64Syscall((x64emu_t*)emu);
}

void EmuX86Syscall(void* emu)
{
    x86Syscall((x64emu_t*)emu);
}

extern int box64_is32bits;

void* GetSeg43Base()
{
    tlsdatasize_t* ptr = getTLSData(my_context);
    return ptr->data;
}

void* GetSegmentBase(uint32_t desc)
{
    if (!desc) {
        printf_log(LOG_NONE, "Warning, accessing segment NULL\n");
        return NULL;
    }
    int base = desc >> 3;
    if (!box64_is32bits && base == 0x8 && !my_context->segtls[base].key_init)
        return GetSeg43Base();
    if (box64_is32bits && (base == 0x6))
        return GetSeg43Base();
    if (base > 15) {
        printf_log(LOG_NONE, "Warning, accessing segment unknown 0x%x or unset\n", desc);
        return NULL;
    }
    if (my_context->segtls[base].key_init) {
        void* ptr = pthread_getspecific(my_context->segtls[base].key);
        return ptr;
    }

    void* ptr = (void*)my_context->segtls[base].base;
    return ptr;
}

const char* GetNativeName(void* p)
{
    static char buff[500] = { 0 };
    {
        const char* n = getBridgeName(p);
        if (n)
            return n;
    }
    Dl_info info;
    if (dladdr(p, &info) == 0) {
        const char* ret = GetNameOffset(my_context->maplib, p);
        if (ret)
            return ret;
        sprintf(buff, "%s(%p)", "???", p);
        return buff;
    } else {
        if (info.dli_sname) {
            strcpy(buff, info.dli_sname);
            if (info.dli_fname) {
                strcat(buff, "(");
                strcat(buff, info.dli_fname);
                strcat(buff, ")");
            }
        } else {
            sprintf(buff, "%s(%s+%p)", "???", info.dli_fname, (void*)(p - info.dli_fbase));
            return buff;
        }
    }
    return buff;
}


void PersonalityAddrLimit32Bit(void)
{
    personality(ADDR_LIMIT_32BIT);
}

int IsAddrElfOrFileMapped(uintptr_t addr)
{
    return FindElfAddress(my_context, addr) || IsAddrFileMapped(addr, NULL, NULL);
}

void* InternalMmap(void* addr, unsigned long length, int prot, int flags, int fd, ssize_t offset)
{
#if 1 // def STATICBUILD
    void* ret = (void*)syscall(__NR_mmap, addr, length, prot, flags, fd, offset);
#else
    static int grab = 1;
    typedef void* (*pFpLiiiL_t)(void*, unsigned long, int, int, int, size_t);
    static pFpLiiiL_t libc_mmap64 = NULL;
    if (grab) {
        libc_mmap64 = dlsym(RTLD_NEXT, "mmap64");
    }
    void* ret = libc_mmap64(addr, length, prot, flags, fd, offset);
#endif
    return ret;
}

int InternalMunmap(void* addr, unsigned long length)
{
#if 1 // def STATICBUILD
    int ret = syscall(__NR_munmap, addr, length);
#else
    static int grab = 1;
    typedef int (*iFpL_t)(void*, unsigned long);
    static iFpL_t libc_munmap = NULL;
    if (grab) {
        libc_munmap = dlsym(RTLD_NEXT, "munmap");
    }
    int ret = libc_munmap(addr, length);
#endif
    return ret;
}