// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include "box64ec_private.h"
#include "box64context.h"
#include "custommem.h"
#include "debug.h"
#include "env.h"
#include "os.h"
#include "volatile_fences.h"
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

int Box64EC_IsExecutable(uintptr_t address)
{
    MEMORY_BASIC_INFORMATION info;

    if (NtQueryVirtualMemory(NtCurrentProcess(), (void*)address, MemoryBasicInformation, &info, sizeof(info), NULL) ||
        info.State != MEM_COMMIT || (info.Protect & PAGE_GUARD))
        return 0;
    switch (info.Protect & 0xff) {
        case PAGE_EXECUTE_READ:
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
            return 1;
    }
    return 0;
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

#ifdef DYNAREC
extern void arm64_ec_fast_forward_direct(void);
extern void arm64_ec_native_exit_direct(void);

// The jump table remembers native targets: forget them with the memory that held them
static void forget_native_targets(uintptr_t address, size_t size)
{
    setJumpTableDefaultRangeRef64(address, size, arm64_ec_fast_forward_direct, arm64_ec_native_exit_direct);
}
#endif

// Blocks of freed memory must go before the address range is reused
static void drop_blocks(uintptr_t address, size_t size)
{
    uintptr_t start = address & ~(uintptr_t)0xfff;
    size_t length = ((address + size + 0xfff) & ~(uintptr_t)0xfff) - start;
#ifdef DYNAREC
    forget_native_targets(start, length);
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
        if (user_range(address, size)) {
            forget_native_targets((uintptr_t)address, size);
            unprotectDB((uintptr_t)address, size, 1);
        }
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
    if (!is_post && user_range(address, size)) {
        forget_native_targets((uintptr_t)address, size);
        unprotectDB((uintptr_t)address, size, 1);
    }
#endif
}

void WINAPI BTCpu64FlushInstructionCache(const void* address, SIZE_T size)
{
#ifdef DYNAREC
    if (user_range(address, size)) {
        forget_native_targets((uintptr_t)address, size);
        unprotectDB((uintptr_t)address, size, 1);
    }
#endif
}

void WINAPI BTCpu64NotifyMemoryDirty(void* address, SIZE_T size)
{
#ifdef DYNAREC
    forget_native_targets((uintptr_t)address, size);
    unprotectDB((uintptr_t)address, size, 1);
#endif
}

NTSYSAPI PIMAGE_NT_HEADERS WINAPI RtlImageNtHeader(HMODULE);
NTSYSAPI void* WINAPI RtlImageDirectoryEntryToData(HMODULE, BOOLEAN, USHORT, ULONG*);

// IMAGE_VOLATILE_METADATA, missing in the llvm-mingw headers
typedef struct {
    DWORD size, version;
    DWORD access_rva, access_size;      // DWORD RVAs
    DWORD range_rva, range_size;        // DWORD RVA and size pairs
} volatile_metadata_t;

static int in_image(const IMAGE_NT_HEADERS* nt, uint64_t rva, uint64_t size)
{
    return rva <= nt->OptionalHeader.SizeOfImage && size <= nt->OptionalHeader.SizeOfImage - rva;
}

// The volatile metadata of a mapped image: Box64EC has no file to read it from, as ParseVolatileMetadata does
void Box64EC_RegisterVolatileMetadata(void* image, int relocated)
{
    PIMAGE_NT_HEADERS nt = RtlImageNtHeader(image);
    uintptr_t base = (uintptr_t)image;
    size_t config_size = offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, VolatileMetadataPointer) + sizeof(ULONGLONG);
    ULONG size;

    if (!nt || nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        return;
    IMAGE_LOAD_CONFIG_DIRECTORY64* config = RtlImageDirectoryEntryToData(image, TRUE, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, &size);
    if (!config || !in_image(nt, (uintptr_t)config - base, config_size) || config->Size < config_size || !config->VolatileMetadataPointer)
        return;
    // the loader relocates the pointer without updating ImageBase, and Wine reports a mapping before that
    uint64_t rva = config->VolatileMetadataPointer - (relocated ? base : nt->OptionalHeader.ImageBase);
    if (!in_image(nt, rva, sizeof(volatile_metadata_t)))
        return;
    const volatile_metadata_t* metadata = (const volatile_metadata_t*)(base + rva);
    if (!in_image(nt, metadata->access_rva, metadata->access_size) || !in_image(nt, metadata->range_rva, metadata->range_size))
        return;
    printf_log(LOG_INFO, "Parsing volatile metadata of image loaded at %p\n", image);
    const DWORD* access = (const DWORD*)(base + metadata->access_rva);
    for (DWORD i = 0; i < metadata->access_size / sizeof(DWORD); ++i)
        if (in_image(nt, access[i], 1))
            VolatileFenceAddMetadataPoint(base + access[i]);
    const DWORD* range = (const DWORD*)(base + metadata->range_rva);
    for (DWORD i = 0; i < metadata->range_size / (2 * sizeof(DWORD)); ++i, range += 2)
        if (in_image(nt, range[0], range[1]))     // a range past the image would outlive its unmapping
            VolatileFenceAddRange(VOLATILE_RANGE_METADATA, base + range[0], base + range[0] + range[1]);
}

// Wine calls this for each image it maps: it carries volatile metadata, and UnityPlayer.dll needs the BOX64_UNITY memory ordering hack
NTSTATUS WINAPI NotifyMapViewOfSection(void* unknown1, void* address, void* unknown2, SIZE_T size,
                                       ULONG allocation, ULONG protection)
{
    struct {
        UNICODE_STRING name;
        WCHAR path[1024];     // deep game install paths
    } mapped = {0};
    const WCHAR* base;
    const WCHAR* end;

    if (BOX64ENV(dynarec_volatile_metadata)) {
        mutex_lock(&my_context->mutex_dyndump);    // the dynarec reads the ranges while filling a block under this lock
        Box64EC_RegisterVolatileMetadata(address, 0);
        mutex_unlock(&my_context->mutex_dyndump);
    }
    if (!BOX64ENV(unityplayer) || BOX64ENV(unity) ||
        NtQueryVirtualMemory(NtCurrentProcess(), address, MemoryMappedFilenameInformation, &mapped, sizeof(mapped), NULL))
        return STATUS_SUCCESS;
    base = mapped.name.Buffer;
    end = base + mapped.name.Length / sizeof(WCHAR);
    for (const WCHAR* c = base; c < end; ++c)
        if (*c == L'\\')
            base = c + 1;
    if (end - base == 15 && !_wcsnicmp(base, L"unityplayer.dll", 15)) {
        SET_BOX64ENV(unity, 1);
        printf_log(LOG_INFO, "Detected UnityPlayer.dll\n");
    }
    return STATUS_SUCCESS;
}

void WINAPI NotifyUnmapViewOfSection(void* address, BOOL is_post, NTSTATUS status)
{
    box64ec_thr_t* thread = Box64EC_GetThreadState();
    uintptr_t start;
    size_t length;

    if (is_post || !thread || thread->internal_map_depth)
        return;
    if (get_allocation_range(address, &start, &length)) {
        drop_blocks(start, length);
        if (BOX64ENV(dynarec_volatile_metadata)) {
            mutex_lock(&my_context->mutex_dyndump);
            VolatileFenceRemoveMetadataRange(start, start + length);
            mutex_unlock(&my_context->mutex_dyndump);
        }
    }
}
