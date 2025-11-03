#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "os.h"
#include "debug.h"
#include "wine/compiler.h"
#include "wine/debug.h"

#define HandleToULong(h) ((ULONG)(ULONG_PTR)(h))

NTSTATUS WINAPI NtYieldExecution(void);

int box64_isAddressSpace32 = 0;

int GetTID(void)
{
    return HandleToULong(((HANDLE*)NtCurrentTeb())[9]);
}

int SchedYield(void)
{
    return (NtYieldExecution() != STATUS_NO_YIELD_PERFORMED);
}

int IsBridgeSignature(char s, char c)
{
    return FALSE;
}

void* GetSeg43Base(void* emu)
{
    return NULL;
}

void* GetSegmentBase(void* emu, uint32_t desc)
{
    printf_log(LOG_NONE, "GetSegmentBase does not apply to Wine dlls\n");
    return NULL;
}

void* EmuFork(void* emu, int forktype) { return NULL; }


void EmuX64Syscall(void* emu)
{
    printf_log(LOG_NONE, "EmuX64Syscall NYI\n");
}

void EmuX64Syscall_linux(void* emu)
{
    printf_log(LOG_NONE, "EmuX64Syscall_linux NYI\n");
}

void EmuX86Syscall(void* emu)
{
    printf_log(LOG_NONE, "EmuX86Syscall NYI\n");
}

const char* GetBridgeName(void* p)
{
    return NULL;
}

const char* GetNativeName(void* p)
{
    return NULL;
}

void* GetNativeFnc(uintptr_t fnc)
{
    return NULL;
}

void PersonalityAddrLimit32Bit(void)
{
}

int IsAddrElfOrFileMapped(uintptr_t addr)
{
    return 0;
}


int IsNativeCall(uintptr_t addr, int is32bits, uintptr_t* calladdress, uint16_t* retn)
{
    return 0;
}


ULONG_PTR default_zero_bits32 = 0x7fffffff;

static uint32_t prot_unix_to_win32(uint32_t unx)
{
    if ((unx & (PROT_READ | PROT_WRITE | PROT_EXEC)) == (PROT_READ | PROT_WRITE | PROT_EXEC))
        return PAGE_EXECUTE_READWRITE;
    if ((unx & (PROT_READ | PROT_EXEC)) == (PROT_READ | PROT_EXEC))
        return PAGE_EXECUTE_READ;
    if ((unx & PROT_EXEC) == PROT_EXEC)
        return PAGE_EXECUTE_READ;
    if ((unx & (PROT_READ | PROT_WRITE)) == (PROT_READ | PROT_WRITE))
        return PAGE_READWRITE;
    if ((unx & PROT_READ) == PROT_READ)
        return PAGE_READONLY;
    return 0;
}

int mprotect(void* addr, size_t len, int prot)
{
    NTSTATUS ntstatus;
    ULONG old_prot;
    SIZE_T allocsize = len;
    ntstatus = NtProtectVirtualMemory(NtCurrentProcess(), &addr, &allocsize, prot_unix_to_win32(prot), &old_prot);
    if (ntstatus != STATUS_SUCCESS) {
        return -1;
    }
    return 0;
}

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    NTSTATUS ntstatus;
    SIZE_T sz = length;
    ULONG_PTR limit;
    void* ret = NULL;

    if (addr != NULL) {
        return MAP_FAILED;
    }
    if (fd && fd != -1) {
        return MAP_FAILED;
    }
    if (offset) {
        return MAP_FAILED;
    }

    if (flags & MAP_32BIT)
        limit = default_zero_bits32;
    else
        limit = 0;

    ntstatus = NtAllocateVirtualMemory(NtCurrentProcess(), &ret, limit, &sz, MEM_COMMIT | MEM_RESERVE, prot_unix_to_win32(prot));
    return ret;
}

int munmap(void* addr, size_t length)
{
    int ret = 0;
    if (NtFreeVirtualMemory(NtCurrentProcess(), &addr, &length, MEM_RELEASE))
        ret = -1;
    return ret;
}

void* InternalMmap(void* addr, unsigned long length, int prot, int flags, int fd, ssize_t offset)
{
    return mmap(addr, length, prot, flags, fd, offset);
}

int InternalMunmap(void* addr, unsigned long length)
{
    return munmap(addr, length);
}

void* WinMalloc(size_t size)
{
    void* ret;
    ret = RtlAllocateHeap(GetProcessHeap(), 0, size);
    return ret;
}

void* WinRealloc(void* ptr, size_t size)
{
    void* ret;
    if (!ptr)
        return WinMalloc(size);
    ret = RtlReAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size);
    return ret;
}

void* WinCalloc(size_t nmemb, size_t size)
{
    void* ret;
    ret = RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, nmemb * size);
    return ret;
}

void WinFree(void* ptr)
{
    RtlFreeHeap(GetProcessHeap(), 0, ptr);
}

void free(void* ptr)
{
    RtlFreeHeap(GetProcessHeap(), 0, ptr);
}

int VolatileRangesContains(uintptr_t addr)
{
    return 0;
}

int VolatileOpcodesHas(uintptr_t addr)
{
    return 0;
}

void PrintfFtrace(int prefix, const char* fmt, ...)
{
    static char buf[1024] = { 0 };

    char* p = buf;
    p[0] = '\0';
    if (prefix) strcpy(p, prefix > 1 ? "[\033[31mBOX64\033[0m] " : "[BOX64] ");
    va_list args;
    va_start(args, fmt);
    vsprintf(p + strlen(p), fmt, args);
    va_end(args);
    __wine_dbg_output(p);
}

void* GetEnv(const char* name)
{
    static char buf[1024] = { 0 };
    int len = GetEnvironmentVariableA(name, buf, sizeof(buf));
    return len ? buf : NULL;
}

int FileExist(const char* filename, int flags)
{
    DWORD attrs = GetFileAttributesA(filename);
    if (attrs == INVALID_FILE_ATTRIBUTES) return 0;
    if (flags == -1) return 1;

    if (flags & IS_FILE) {
        if ((attrs & FILE_ATTRIBUTE_DIRECTORY) || (attrs & FILE_ATTRIBUTE_DEVICE) || (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) {
            return 0;
        }
    } else {
        if (!(attrs & FILE_ATTRIBUTE_DIRECTORY))
            return 0;
    }

    if (flags & IS_EXECUTABLE) {
        printf_log(LOG_NONE, "Warning: Executable check not implemented for Windows\n");
    }

    return 1;
}

int MakeDir(const char* folder)
{
    // TODO
    return 0;
}

size_t FileSize(const char* filename)
{
    // TODO
    return 0;
}