#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "os.h"

#define HandleToULong(h) ((ULONG)(ULONG_PTR)(h))

NTSTATUS WINAPI NtYieldExecution(void);

static HANDLE myGetProcessHeap(void)
{
    return ((HANDLE**)NtCurrentTeb())[12][6];
}


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

void* GetSeg43Base()
{
    return NULL;
}

void* GetSegmentBase(uint32_t desc)
{
    // FIXME
    return NULL;
}

void EmuInt3(void* emu, void* addr) { }
void* EmuFork(void* emu, int forktype) { return NULL; }


void EmuX64Syscall(void* emu)
{
    // FIXME
}

void EmuX86Syscall(void* emu)
{
    // FIXME
}

const char* GetNativeName(void* p)
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

#define NtCurrentProcess() ((HANDLE) ~(ULONG_PTR)0)

NTSTATUS WINAPI NtProtectVirtualMemory(HANDLE, PVOID*, SIZE_T*, ULONG, ULONG*);
NTSTATUS WINAPI NtAllocateVirtualMemory(HANDLE, PVOID*, ULONG_PTR, SIZE_T*, ULONG, ULONG);
PVOID WINAPI RtlReAllocateHeap(HANDLE, ULONG, PVOID, SIZE_T);
NTSTATUS WINAPI NtFreeVirtualMemory(HANDLE, PVOID*, SIZE_T*, ULONG);

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
    ret = RtlAllocateHeap(myGetProcessHeap(), 0, size);
    return ret;
}

void* WinRealloc(void* ptr, size_t size)
{
    void* ret;
    if (!ptr)
        return WinMalloc(size);
    ret = RtlReAllocateHeap(myGetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size);
    return ret;
}

void* WinCalloc(size_t nmemb, size_t size)
{
    void* ret;
    ret = RtlAllocateHeap(myGetProcessHeap(), HEAP_ZERO_MEMORY, nmemb * size);
    return ret;
}

void WinFree(void* ptr)
{
    RtlFreeHeap(myGetProcessHeap(), 0, ptr);
}

void free(void* ptr)
{
    RtlFreeHeap(myGetProcessHeap(), 0, ptr);
}

int VolatileRangesContains(uintptr_t addr)
{
    return 0;
}
