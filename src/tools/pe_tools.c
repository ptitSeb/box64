#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include "debug.h"
#include "khash.h"
#include "rbtree.h"

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint64_t ULONGLONG;

#define IMAGE_DOS_SIGNATURE               0x5A4D
#define IMAGE_NT_SIGNATURE                0x00004550
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC     0x10B
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC     0x20B

typedef struct _IMAGE_DOS_HEADER {
    WORD e_magic;
    WORD e_cblp;
    WORD e_cp;
    WORD e_crlc;
    WORD e_cparhdr;
    WORD e_minalloc;
    WORD e_maxalloc;
    WORD e_ss;
    WORD e_sp;
    WORD e_csum;
    WORD e_ip;
    WORD e_cs;
    WORD e_lfarlc;
    WORD e_ovno;
    WORD e_res[4];
    WORD e_oemid;
    WORD e_oeminfo;
    WORD e_res2[10];
    LONG e_lfanew;
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    WORD Machine;
    WORD NumberOfSections;
    DWORD TimeDateStamp;
    DWORD PointerToSymbolTable;
    DWORD NumberOfSymbols;
    WORD SizeOfOptionalHeader;
    WORD Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD VirtualAddress;
    DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    WORD Magic;
    BYTE MajorLinkerVersion;
    BYTE MinorLinkerVersion;
    DWORD SizeOfCode;
    DWORD SizeOfInitializedData;
    DWORD SizeOfUninitializedData;
    DWORD AddressOfEntryPoint;
    DWORD BaseOfCode;
    ULONGLONG ImageBase;
    DWORD SectionAlignment;
    DWORD FileAlignment;
    WORD MajorOperatingSystemVersion;
    WORD MinorOperatingSystemVersion;
    WORD MajorImageVersion;
    WORD MinorImageVersion;
    WORD MajorSubsystemVersion;
    WORD MinorSubsystemVersion;
    DWORD Win32VersionValue;
    DWORD SizeOfImage;
    DWORD SizeOfHeaders;
    DWORD CheckSum;
    WORD Subsystem;
    WORD DllCharacteristics;
    ULONGLONG SizeOfStackReserve;
    ULONGLONG SizeOfStackCommit;
    ULONGLONG SizeOfHeapReserve;
    ULONGLONG SizeOfHeapCommit;
    DWORD LoaderFlags;
    DWORD NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS64 {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_SECTION_HEADER {
    BYTE Name[8];
    union {
        DWORD PhysicalAddress;
        DWORD VirtualSize;
    } Misc;
    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD NumberOfRelocations;
    WORD NumberOfLinenumbers;
    DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _IMAGE_LOAD_CONFIG_DIRECTORY64 {
    DWORD Size;
    DWORD TimeDateStamp;
    WORD MajorVersion;
    WORD MinorVersion;
    DWORD GlobalFlagsClear;
    DWORD GlobalFlagsSet;
    DWORD CriticalSectionDefaultTimeout;
    ULONGLONG DeCommitFreeBlockThreshold;
    ULONGLONG DeCommitTotalFreeThreshold;
    ULONGLONG LockPrefixTable;
    ULONGLONG MaximumAllocationSize;
    ULONGLONG VirtualMemoryThreshold;
    ULONGLONG ProcessAffinityMask;
    DWORD ProcessHeapFlags;
    WORD CSDVersion;
    WORD DependentLoadFlags;
    ULONGLONG EditList;
    ULONGLONG SecurityCookie;
    ULONGLONG SEHandlerTable;
    ULONGLONG SEHandlerCount;
    ULONGLONG GuardCFCheckFunctionPointer;
    ULONGLONG GuardCFDispatchFunctionPointer;
    ULONGLONG GuardCFFunctionTable;
    ULONGLONG GuardCFFunctionCount;
    DWORD GuardFlags;
    ULONGLONG CodeIntegrity;
    ULONGLONG GuardAddressTakenIatEntryTable;
    ULONGLONG GuardAddressTakenIatEntryCount;
    ULONGLONG GuardLongJumpTargetTable;
    ULONGLONG GuardLongJumpTargetCount;
    ULONGLONG DynamicValueRelocTable;
    ULONGLONG CHPEMetadataPointer;
    ULONGLONG GuardRFFailureRoutine;
    ULONGLONG GuardRFFailureRoutineFunctionPointer;
    DWORD DynamicValueRelocTableOffset;
    WORD DynamicValueRelocTableSection;
    WORD Reserved2;
    ULONGLONG GuardRFVerifyStackPointerFunctionPointer;
    DWORD HotPatchTableOffset;
    DWORD Reserved3;
    ULONGLONG EnclaveConfigurationPointer;
    ULONGLONG VolatileMetadataPointer;
    ULONGLONG GuardEHContinuationTable;
    ULONGLONG GuardEHContinuationCount;
    ULONGLONG GuardXFGCheckFunctionPointer;
    ULONGLONG GuardXFGDispatchFunctionPointer;
    ULONGLONG GuardXFGTableDispatchFunctionPointer;
    ULONGLONG CastGuardOsDeterminedFailureMode;
    ULONGLONG GuardMemcpyFunctionPointer;
} IMAGE_LOAD_CONFIG_DIRECTORY64, *PIMAGE_LOAD_CONFIG_DIRECTORY64;

typedef struct _IMAGE_VOLATILE_METADATA {
    DWORD Size;
    DWORD Version;
    DWORD VolatileAccessTable;
    DWORD VolatileAccessTableSize;
    DWORD VolatileInfoRangeTable;
    DWORD VolatileInfoRangeTableSize;
} IMAGE_VOLATILE_METADATA, *PIMAGE_VOLATILE_METADATA;

typedef struct _IMAGE_VOLATILE_RVA_METADATA {
    ULONG Rva;
} IMAGE_VOLATILE_RVA_METADATA, *PIMAGE_VOLATILE_RVA_METADATA;

typedef struct _IMAGE_VOLATILE_RANGE_METADATA {
    ULONG Rva;
    ULONG Size;
} IMAGE_VOLATILE_RANGE_METADATA, *PIMAGE_VOLATILE_RANGE_METADATA;

KHASH_SET_INIT_STR(string);
KHASH_SET_INIT_INT64(volatileopcode);

static kh_volatileopcode_t* volatileOpcodes = NULL; // never freed
static rbtree_t* volatileRanges = NULL; // never freed

DWORD RVAToFileOffset(PIMAGE_SECTION_HEADER sections, DWORD numberOfSections, DWORD rva, BYTE* fileBuffer, size_t fileSize)
{
    for (DWORD i = 0; i < numberOfSections; i++) {
        PIMAGE_SECTION_HEADER section = &sections[i];
        if (rva >= section->VirtualAddress && rva < section->VirtualAddress + section->SizeOfRawData) {
            DWORD offset = rva - section->VirtualAddress + section->PointerToRawData;
            if (offset < fileSize) {
                return offset;
            }
        }
    }
    return 0;
}

void ParseVolatileMetadata(char* filename, void* addr)
{
    if (!volatileOpcodes) volatileOpcodes = kh_init(volatileopcode);
    if (!volatileRanges) volatileRanges = rbtree_init("volatileRanges");

    char* baseName = strrchr(filename, '/');
    if (!baseName)
        return;
    else
        baseName++;

    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)addr;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return;

    PIMAGE_NT_HEADERS64 ntHeaders64 = (PIMAGE_NT_HEADERS64)(addr + dosHeader->e_lfanew);
    if (ntHeaders64->Signature != IMAGE_NT_SIGNATURE || ntHeaders64->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) return;

    int numberOfSections = ntHeaders64->FileHeader.NumberOfSections;
    if (numberOfSections <= 0) {
        return;
    }
    IMAGE_DATA_DIRECTORY loadConfigDir = ntHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
    if (loadConfigDir.VirtualAddress == 0 || loadConfigDir.Size == 0) {
        return;
    }

    PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)((void*)ntHeaders64 + sizeof(IMAGE_NT_HEADERS64)); // immediately follows the optional header, if any.
    DWORD loadConfigOffset = RVAToFileOffset(sectionHeaders, numberOfSections, loadConfigDir.VirtualAddress, (BYTE*)addr, ntHeaders64->OptionalHeader.SizeOfImage);
    if (loadConfigOffset == 0) {
        return;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size);
    if (!buffer) {
        fclose(file);
        return;
    }

    if ((long)fread(buffer, 1, size, file) != size) {
        free(buffer);
        fclose(file);
        return;
    }
    fclose(file);

    PIMAGE_LOAD_CONFIG_DIRECTORY64 loadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY64)(buffer + loadConfigOffset);
    if (loadConfig->Size < offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, VolatileMetadataPointer) + sizeof(ULONGLONG)) {
        free(buffer);
        return;
    }
    DWORD volatileMetadataPointer = (DWORD)(loadConfig->VolatileMetadataPointer - ntHeaders64->OptionalHeader.ImageBase);
    if (volatileMetadataPointer == 0) {
        free(buffer);
        return;
    }

    DWORD volatileMetadataOffset = RVAToFileOffset(sectionHeaders, numberOfSections, volatileMetadataPointer, (BYTE*)buffer, ntHeaders64->OptionalHeader.SizeOfImage);
    if (volatileMetadataOffset == 0) {
        free(buffer);
        return;
    }

    PIMAGE_VOLATILE_METADATA volatileMetadata = (PIMAGE_VOLATILE_METADATA)(buffer + volatileMetadataOffset);
    if (volatileMetadata->VolatileAccessTable && volatileMetadata->VolatileAccessTableSize) {
        printf_log(LOG_INFO, "Parsing volatile metadata of file %s loaded at %p\n", baseName, addr);

        DWORD volatileAccessTableOffset = RVAToFileOffset(sectionHeaders, numberOfSections, volatileMetadata->VolatileAccessTable, (BYTE*)buffer, ntHeaders64->OptionalHeader.SizeOfImage);
        if (volatileAccessTableOffset == 0) {
            free(buffer);
            return;
        }

        DWORD numEntries = volatileMetadata->VolatileAccessTableSize / sizeof(IMAGE_VOLATILE_RVA_METADATA);
        PIMAGE_VOLATILE_RVA_METADATA volatileAccessTable = (PIMAGE_VOLATILE_RVA_METADATA)(buffer + volatileAccessTableOffset);

        for (DWORD i = 0; i < numEntries; i++) {
            ULONGLONG entry = volatileAccessTable[i].Rva + (ULONGLONG)addr;
            int ret;
            khint_t _ = kh_put(volatileopcode, volatileOpcodes, entry, &ret);
            printf_log(LOG_DEBUG, "Volatile access table [%d]: %08lx\n", i, entry);
        }
    }

    if (volatileMetadata->VolatileInfoRangeTable && volatileMetadata->VolatileInfoRangeTableSize) {
        DWORD volatileInfoRangeTableOffset = RVAToFileOffset(sectionHeaders, numberOfSections, volatileMetadata->VolatileInfoRangeTable, (BYTE*)buffer, ntHeaders64->OptionalHeader.SizeOfImage);
        if (volatileInfoRangeTableOffset == 0) {
            free(buffer);
            return;
        }

        DWORD numEntries = volatileMetadata->VolatileInfoRangeTableSize / sizeof(IMAGE_VOLATILE_RANGE_METADATA);
        PIMAGE_VOLATILE_RANGE_METADATA volatileRangeMetadata = (PIMAGE_VOLATILE_RANGE_METADATA)(buffer + volatileInfoRangeTableOffset);

        for (DWORD i = 0; i < numEntries; i++) {
            ULONGLONG rva = volatileRangeMetadata[i].Rva + (ULONGLONG)addr;
            ULONGLONG size = volatileRangeMetadata[i].Size;
            rb_set(volatileRanges, rva, rva + size, 1);
            printf_log(LOG_DEBUG, "Volatile range metadata [%d]: %08lx-%08lx\n", i, rva, rva + size);
        }
    }
    free(buffer);
    return;
}

int VolatileRangesContains(uintptr_t addr)
{
    if (!volatileRanges) return 0;
    return rb_get(volatileRanges, addr) != 0;
}

int VolatileOpcodesHas(uintptr_t addr)
{
    if (!volatileOpcodes) return 0;
    return kh_get(volatileopcode, volatileOpcodes, addr) != kh_end(volatileOpcodes);
}
