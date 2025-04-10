#include <windows.h>

#include "os.h"

int GetTID(void)
{
    return GetCurrentThreadId();
}

int SchedYield(void)
{
    return SwitchToThread();
}

int IsBridgeSignature(char s, char c)
{
    return FALSE;
}

void PersonalityAddrLimit32Bit(void) { }

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
    ULONG old_prot;
    if (VirtualProtect(&addr, len, prot_unix_to_win32(prot), &old_prot))
        return 0;
    return -1;
}

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if (fd && fd != -1) {
        return MAP_FAILED;
    }
    if (offset) {
        return MAP_FAILED;
    }
    return VirtualAlloc(addr, length, MEM_COMMIT | MEM_RESERVE, prot_unix_to_win32(prot));
}

int munmap(void* addr, size_t length)
{
    if (VirtualFree(addr, length, MEM_RELEASE))
        return 0;
    return -1;
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
    ret = HeapAlloc(GetProcessHeap(), 0, size);
    return ret;
}

void* WinRealloc(void* ptr, size_t size)
{
    void* ret;
    if (!ptr) return WinMalloc(size);
    ret = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size);
    return ret;
}

void* WinCalloc(size_t nmemb, size_t size)
{
    void* ret;
    ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nmemb * size);
    return ret;
}

void WinFree(void* ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}