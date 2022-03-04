#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>

typedef struct elfheader_s elfheader_t;
typedef struct lib_s lib_t;
typedef struct library_s library_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct box64context_s box64context_t;
typedef struct x64emu_s x64emu_t;
typedef struct needed_libs_s needed_libs_t;
#ifdef DYNAREC
typedef struct dynablocklist_s dynablocklist_t;
#endif

elfheader_t* LoadAndCheckElfHeader(FILE* f, const char* name, int exec); // exec : 0 = lib, 1 = exec
void FreeElfHeader(elfheader_t** head);
const char* ElfName(elfheader_t* head);
const char* ElfPath(elfheader_t* head);
void ElfAttachLib(elfheader_t* head, library_t* lib);

// return 0 if OK
int CalcLoadAddr(elfheader_t* head);
int AllocElfMemory(box64context_t* context, elfheader_t* head, int mainbin);
void FreeElfMemory(elfheader_t* head);
int LoadElfMemory(FILE* f, box64context_t* context, elfheader_t* head);
int ReloadElfMemory(FILE* f, box64context_t* context, elfheader_t* head);
int RelocateElf(lib_t *maplib, lib_t* local_maplib, int bindnow, elfheader_t* head);
int RelocateElfPlt(lib_t *maplib, lib_t* local_maplib, int bindnow, elfheader_t* head);
void CalcStack(elfheader_t* h, uint64_t* stacksz, size_t* stackalign);
uintptr_t GetEntryPoint(lib_t* maplib, elfheader_t* h);
uintptr_t GetLastByte(elfheader_t* h);
void AddSymbols(lib_t *maplib, kh_mapsymbols_t* mapsymbols, kh_mapsymbols_t* weaksymbols, kh_mapsymbols_t* localsymbols, elfheader_t* h);
int LoadNeededLibs(elfheader_t* h, lib_t *maplib, needed_libs_t* neededlibs, library_t *deplib, int local, int bindnow, box64context_t *box64, x64emu_t* emu);
uintptr_t GetElfInit(elfheader_t* h);
uintptr_t GetElfFini(elfheader_t* h);
void RunElfInit(elfheader_t* h, x64emu_t *emu);
void RunElfFini(elfheader_t* h, x64emu_t *emu);
void RunDeferedElfInit(x64emu_t *emu);
void* GetBaseAddress(elfheader_t* h);
void* GetElfDelta(elfheader_t* h);
uint32_t GetBaseSize(elfheader_t* h);
int IsAddressInElfSpace(elfheader_t* h, uintptr_t addr);
elfheader_t* FindElfAddress(box64context_t *context, uintptr_t addr);
const char* FindNearestSymbolName(elfheader_t* h, void* p, uintptr_t* start, uint64_t* sz);
int32_t GetTLSBase(elfheader_t* h);
uint32_t GetTLSSize(elfheader_t* h);
void* GetTLSPointer(box64context_t* context, elfheader_t* h);
void* GetDTatOffset(box64context_t* context, unsigned long int index, unsigned long int offset);
#ifdef DYNAREC
dynablocklist_t* GetDynablocksFromAddress(box64context_t *context, uintptr_t addr);
dynablocklist_t* GetDynablocksFromElf(elfheader_t* h);
#endif
void ResetSpecialCaseMainElf(elfheader_t* h);
void CreateMemorymapFile(box64context_t* context, int fd);
void* GetDynamicSection(elfheader_t* h);

int ElfCheckIfUseTCMallocMinimal(elfheader_t* h);   // return 1 if tcmalloc is used

const char* GetSymbolVersion(elfheader_t* h, int version);
const char* GetParentSymbolVersion(elfheader_t* h, int index);
const char* VersionnedName(const char* name, int ver, const char* vername);
int SameVersionnedSymbol(const char* name1, int ver1, const char* vername1, const char* name2, int ver2, const char* vername2);

void* GetNativeSymbolUnversionned(void* lib, const char* name);

void AddMainElfToLinkmap(elfheader_t* lib);

#endif //__ELF_LOADER_H_
