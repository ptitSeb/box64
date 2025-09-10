#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/personality.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include <malloc.h>

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

const char* GetBridgeName(void* p)
{
    return getBridgeName(p);
}

const char* GetNativeName(void* p)
{
    static char buff[500] = { 0 };
    {
        const char* n = GetBridgeName(p);
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
    /*struct rlimit l;
    if(getrlimit(RLIMIT_DATA, &l)<0) return;  // failed
    if(l.rlim_max>3*1024*1024*1024LL) {
        l.rlim_cur = 3*1024*1024*1024LL;
        setrlimit(RLIMIT_DATA, &l);
    }*/
    // setting 32bits malloc options
    mallopt(M_ARENA_TEST, 2);
    mallopt(M_ARENA_MAX, 2);
    mallopt(M_MMAP_THRESHOLD, 128*1024);
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

extern FILE* ftrace;
extern char* ftrace_name;

static void checkFtrace()
{
    int fd = fileno(ftrace);
    if (fd < 0 || lseek(fd, 0, SEEK_CUR) == (off_t)-1) {
        ftrace = fopen(ftrace_name, "a");
        printf_log(LOG_INFO, "%04d|Recreated trace because fd was invalid\n", GetTID());
    }
}

void PrintfFtrace(int prefix, const char* fmt, ...)
{
    if (ftrace_name) {
        checkFtrace();
    }

    static const char* names[2] = { "BOX64", "BOX32" };

    if (prefix && ftrace == stdout) {
        if (prefix > 1) {
            fprintf(ftrace, "[\033[31m%s\033[0m] ", names[box64_is32bits]);
        } else {
            fprintf(ftrace, "[%s] ", names[box64_is32bits]);
        }
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(ftrace, fmt, args);
    fflush(ftrace);
    va_end(args);
}

void* GetEnv(const char* name)
{
    return getenv(name);
}

int FileExist(const char* filename, int flags)
{
    struct stat sb;
    if (stat(filename, &sb) == -1)
        return 0;
    if (flags == -1)
        return 1;
    // check type of file? should be executable, or folder
    if (flags & IS_FILE) {
        if (!S_ISREG(sb.st_mode))
            return 0;
    } else if (!S_ISDIR(sb.st_mode))
        return 0;

    if (flags & IS_EXECUTABLE) {
        if ((sb.st_mode & S_IXUSR) != S_IXUSR)
            return 0; // nope
    }
    return 1;
}

int MakeDir(const char* folder)
{
    int ret = mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(!ret || ret==EEXIST)
        return 1;
    return 0;
}

size_t FileSize(const char* filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1)
        return 0;
    // check type of file? should be executable, or folder
    if (!S_ISREG(sb.st_mode))
        return 0;
    return sb.st_size;
}