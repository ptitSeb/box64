// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <string.h>
#include "box64ec_private.h"
#include "custommem.h"
#include "os.h"
#include "wine/compiler.h"

int Box64EC_IsEcCode(uintptr_t address)
{
    uint64_t* map;
    uint64_t word;
    uintptr_t page;
    void* peb = *(void**)((char*)NtCurrentTeb() + 0x60);

    if (!peb || !address || address >= Box64EC_EcCodeBitmapLimit) {
        return 0;
    }
    map = *(uint64_t**)((char*)peb + 0x368);
    if (!map) {
        return 0;
    }
    page = address >> 12;
    word = map[page >> 6];
    {
        int result = (int)((word >> (page & 63)) & 1);
        return result;
    }
}

// Box64's own mappings are not reported back to it, see the Notify hooks
void Box64EC_InternalMapEnter(void)
{
    box64ec_thr_t* thread = Box64EC_GetThreadState();
    if (thread)
        ++thread->internal_map_depth;
}

void Box64EC_InternalMapLeave(void)
{
    box64ec_thr_t* thread = Box64EC_GetThreadState();
    if (thread && thread->internal_map_depth)
        --thread->internal_map_depth;
}

// whole allocation of an address: AllocationBase and the following regions sharing it
static int get_allocation_range(void* address, uintptr_t* start, size_t* size)
{
    MEMORY_BASIC_INFORMATION info;
    uintptr_t base, end;

    if (NtQueryVirtualMemory(NtCurrentProcess(), address, MemoryBasicInformation, &info, sizeof(info), NULL) || !info.AllocationBase)
        return 0;
    base = (uintptr_t)info.AllocationBase;
    end = (uintptr_t)info.BaseAddress + info.RegionSize;
    while (!NtQueryVirtualMemory(NtCurrentProcess(), (void*)end, MemoryBasicInformation, &info, sizeof(info), NULL) &&
           (uintptr_t)info.AllocationBase == base)
        end += info.RegionSize;
    *start = base;
    *size = end - base;
    return 1;
}

// Blocks of freed memory must go before the address range is reused
static void drop_blocks(uintptr_t address, size_t size)
{
    uintptr_t start = address & ~(uintptr_t)0xfff;
    size_t length = ((address + size + 0xfff) & ~(uintptr_t)0xfff) - start;
#ifdef DYNAREC
    cleanDBFromAddressRange(start, length, 1);
#endif
    freeProtection(start, length);
}

void WINAPI NotifyMemoryFree(void* address, SIZE_T size, ULONG type, BOOL is_post, NTSTATUS status)
{
    box64ec_thr_t* thread = Box64EC_GetThreadState();
    uintptr_t start = (uintptr_t)address;
    size_t length = size;

    if (is_post || !thread || thread->internal_map_depth)
        return;
    if (!size && !get_allocation_range(address, &start, &length))
        return;
    drop_blocks(start, length);
}

#ifdef DYNAREC
// the pre hooks run before Wine checks the range the guest passed
static int user_range(const void* address, SIZE_T size)
{
    return (uintptr_t)address < (1ull << 47) && size <= (1ull << 47) - (uintptr_t)address;
}
#endif

void WINAPI NotifyMemoryProtect(void* address, SIZE_T size, ULONG protection, BOOL is_post, NTSTATUS status)
{
    box64ec_thr_t* thread = Box64EC_GetThreadState();
    uint32_t prot = 0;

    if (!thread || thread->internal_map_depth)
        return;
#ifdef DYNAREC
    // Wine returns the old protection and VirtualQuery the new one: the guest must not see the write
    // protection of its blocks there, so lift it before (the blocks are checked again when they run)
    if (!is_post) {
        if (user_range(address, size))
            unprotectDB((uintptr_t)address, size, 1);
        return;
    }
#endif
    if (!is_post || status)
        return;
    if (!(protection & PAGE_GUARD))
        switch (protection & 0xff) {
            case PAGE_READONLY: prot = PROT_READ; break;
            case PAGE_READWRITE:
            case PAGE_WRITECOPY: prot = PROT_READ | PROT_WRITE; break;
            case PAGE_EXECUTE:
            case PAGE_EXECUTE_READ: prot = PROT_READ | PROT_EXEC; break;
            case PAGE_EXECUTE_READWRITE:
            case PAGE_EXECUTE_WRITECOPY: prot = PROT_READ | PROT_WRITE | PROT_EXEC; break;
        }
#ifdef DYNAREC
    // the blocks of a no-access range are freed, which takes mutex_dyndump: do it before updateProtection takes mutex_prot
    if (!prot)
        drop_blocks((uintptr_t)address, size);
#endif
    updateProtection((uintptr_t)address, size, prot);
}

// the kernel side of these cannot take a write fault on a page protected for its blocks
void WINAPI BTCpu64NotifyReadFile(HANDLE handle, void* address, SIZE_T size, BOOL is_post, NTSTATUS status)
{
#ifdef DYNAREC
    if (!is_post && user_range(address, size))
        unprotectDB((uintptr_t)address, size, 1);
#endif
}

void WINAPI BTCpu64FlushInstructionCache(const void* address, SIZE_T size)
{
#ifdef DYNAREC
    if (user_range(address, size))
        unprotectDB((uintptr_t)address, size, 1);
#endif
}

void WINAPI BTCpu64NotifyMemoryDirty(void* address, SIZE_T size)
{
#ifdef DYNAREC
    unprotectDB((uintptr_t)address, size, 1);
#endif
}

void WINAPI NotifyUnmapViewOfSection(void* address, BOOL is_post, NTSTATUS status)
{
    box64ec_thr_t* thread = Box64EC_GetThreadState();
    uintptr_t start;
    size_t length;

    if (is_post || !thread || thread->internal_map_depth)
        return;
    if (get_allocation_range(address, &start, &length))
        drop_blocks(start, length);
}
