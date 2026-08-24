#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "os.h"
#include "debug.h"
#include "env.h"
#include "wine/compiler.h"
#include "wine/debug.h"

#define HandleToULong(h) ((ULONG)(ULONG_PTR)(h))

NTSTATUS WINAPI NtYieldExecution(void);

int box64_isAddressSpace32 = 0;

enum {
    CPU_FEATURE_VME   = 0x00000005,
    CPU_FEATURE_TSC   = 0x00000002,
    CPU_FEATURE_CMOV  = 0x00000008,
    CPU_FEATURE_PGE   = 0x00000014,
    CPU_FEATURE_PSE   = 0x00000024,
    CPU_FEATURE_MTRR  = 0x00000040,
    CPU_FEATURE_CX8   = 0x00000080,
    CPU_FEATURE_MMX   = 0x00000100,
    CPU_FEATURE_X86   = 0x00000200,
    CPU_FEATURE_PAT   = 0x00000400,
    CPU_FEATURE_FXSR  = 0x00000800,
    CPU_FEATURE_SEP   = 0x00001000,
    CPU_FEATURE_SSE   = 0x00002000,
    CPU_FEATURE_SSSE3 = 0x00008000,
    CPU_FEATURE_SSE2  = 0x00010000,
    CPU_FEATURE_SSE3  = 0x00080000,
    CPU_FEATURE_CX128 = 0x00100000,
    CPU_FEATURE_PAE   = 0x00200000,
    CPU_FEATURE_DAZ   = 0x00400000,
    CPU_FEATURE_XSAVE = 0x00800000,
    CPU_FEATURE_SSE41 = 0x01000000,
    CPU_FEATURE_SSE42 = 0x02000000,
    CPU_FEATURE_RDFS  = 0x10000000,
    CPU_FEATURE_NX    = 0x20000000,
    CPU_FEATURE_AVX   = 0x40000000,
    CPU_FEATURE_AVX2  = 0x80000000u,
};

static ULONG wine_processor_feature_bits(void)
{
    ULONG features = CPU_FEATURE_VME | CPU_FEATURE_TSC | CPU_FEATURE_CMOV |
                     CPU_FEATURE_PGE | CPU_FEATURE_PSE | CPU_FEATURE_MTRR |
                     CPU_FEATURE_CX8 | CPU_FEATURE_MMX | CPU_FEATURE_X86 |
                     CPU_FEATURE_PAT | CPU_FEATURE_FXSR | CPU_FEATURE_SEP |
                     CPU_FEATURE_SSE | CPU_FEATURE_SSSE3 |
                     CPU_FEATURE_SSE2 | CPU_FEATURE_SSE3 | CPU_FEATURE_CX128 |
                     CPU_FEATURE_PAE | CPU_FEATURE_DAZ | CPU_FEATURE_SSE41 |
                     CPU_FEATURE_RDFS | CPU_FEATURE_NX;

    if (BOX64ENV(sse42))
        features |= CPU_FEATURE_SSE42;
    if (BOX64ENV(avx))
        features |= CPU_FEATURE_XSAVE | CPU_FEATURE_AVX;
    if (BOX64ENV(avx) == 2)
        features |= CPU_FEATURE_AVX2;
    return features;
}

BOOLEAN Box64WineIsProcessorFeaturePresent(UINT feature)
{
    static const ULONGLONG features =
        (1ull << PF_COMPARE_EXCHANGE_DOUBLE) |
        (1ull << PF_MMX_INSTRUCTIONS_AVAILABLE) |
        (1ull << PF_XMMI_INSTRUCTIONS_AVAILABLE) |
        (1ull << PF_RDTSC_INSTRUCTION_AVAILABLE) |
        (1ull << PF_PAE_ENABLED) |
        (1ull << PF_XMMI64_INSTRUCTIONS_AVAILABLE) |
        (1ull << PF_SSE_DAZ_MODE_AVAILABLE) |
        (1ull << PF_NX_ENABLED) |
        (1ull << PF_SSE3_INSTRUCTIONS_AVAILABLE) |
        (1ull << PF_COMPARE_EXCHANGE128) |
        (1ull << PF_RDWRFSGSBASE_AVAILABLE) |
        (1ull << PF_FASTFAIL_AVAILABLE) |
        (1ull << PF_RDTSCP_INSTRUCTION_AVAILABLE) |
        (1ull << PF_SSSE3_INSTRUCTIONS_AVAILABLE) |
        (1ull << PF_SSE4_1_INSTRUCTIONS_AVAILABLE);

    if (feature == PF_SSE4_2_INSTRUCTIONS_AVAILABLE)
        return BOX64ENV(sse42) != 0;
    if (feature == PF_XSAVE_ENABLED || feature == PF_AVX_INSTRUCTIONS_AVAILABLE)
        return BOX64ENV(avx) != 0;
    if (feature == PF_AVX2_INSTRUCTIONS_AVAILABLE)
        return BOX64ENV(avx) == 2;
    return feature < 64 && (features & (1ull << feature));
}

void Box64WineUpdateProcessorInformation(SYSTEM_CPU_INFORMATION* info,
                                         USHORT architecture)
{
    if (!info)
        return;
    info->ProcessorArchitecture = architecture;
    if (BOX64ENV(cputype)) {
        info->ProcessorLevel = 23;
        info->ProcessorRevision = 0x710c;
    } else {
        info->ProcessorLevel = 6;
        info->ProcessorRevision = 0x4601;
    }
    info->ProcessorFeatureBits = wine_processor_feature_bits();
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

const char* GetNativeName(void* p, int lib)
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