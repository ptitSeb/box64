#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>
#include <elf.h>

typedef struct elfheader_s elfheader_t;
typedef struct lib_s lib_t;
typedef struct library_s library_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct box64context_s box64context_t;
typedef struct x64emu_s x64emu_t;
typedef struct needed_libs_s needed_libs_t;
typedef struct kh_defaultversion_s kh_defaultversion_t;
#ifdef DYNAREC
typedef struct dynablock_s dynablock_t;
#endif

// Define for handling .relr.dyn section (since glibc 2.36)
// See Also https://lists.gnu.org/archive/html/info-gnu/2022-08/msg00000.html
//
// if glibc 2.36 is widely used in the future, this part can be removed
#ifndef DT_RELR

// Copy from (glibc 2.36) elf.h

#define SHT_RELR	  19            /* RELR relative relocations */

typedef Elf32_Word	Elf32_Relr;
typedef Elf64_Xword	Elf64_Relr;

#define DT_RELRSZ	35		/* Total size of RELR relative relocations */
#define DT_RELR		36		/* Address of RELR relative relocations */
#define DT_RELRENT	37		/* Size of one RELR relative relocaction */
#endif // DT_RELR

// Open an elfheader. Transfert control of f to elfheader also!
elfheader_t* LoadAndCheckElfHeader(FILE* f, const char* name, int exec); // exec : 0 = lib, 1 = exec
void FreeElfHeader(elfheader_t** head);
const char* ElfName(elfheader_t* head);
const char* ElfPath(elfheader_t* head);
void ElfAttachLib(elfheader_t* head, library_t* lib);

// return 0 if OK
int CalcLoadAddr(elfheader_t* head);
int AllocLoadElfMemory(box64context_t* context, elfheader_t* head, int mainbin);
void FreeElfMemory(elfheader_t* head);
int isElfHasNeededVer(elfheader_t* head, const char* libname, elfheader_t* verneeded);
int RelocateElf(lib_t *maplib, lib_t* local_maplib, int bindnow, int deepbind, elfheader_t* head);
int RelocateElfPlt(lib_t *maplib, lib_t* local_maplib, int bindnow, int deepbind, elfheader_t* head);
void CalcStack(elfheader_t* h, uint64_t* stacksz, size_t* stackalign);
uintptr_t GetEntryPoint(lib_t* maplib, elfheader_t* h);
uintptr_t GetLastByte(elfheader_t* h);
void AddSymbols(lib_t *maplib, elfheader_t* h);
int NeededLibs(elfheader_t* h);
int LoadNeededLibs(elfheader_t* h, lib_t *maplib, int local, int bindnow, int deepbind, box64context_t *box64, x64emu_t* emu);
needed_libs_t* GetELfNeededLibs(elfheader_t* h);
uintptr_t GetElfInit(elfheader_t* h);
uintptr_t GetElfFini(elfheader_t* h);
void RefreshElfTLS(elfheader_t* h, x64emu_t* emu);
void RunElfInit(elfheader_t* h, x64emu_t *emu);
void RunElfFini(elfheader_t* h, x64emu_t *emu);
void RunDeferredElfInit(x64emu_t *emu);
void MarkElfInitDone(elfheader_t* h);
void* GetBaseAddress(elfheader_t* h);
void* GetElfDelta(elfheader_t* h);
uint32_t GetBaseSize(elfheader_t* h);
int IsAddressInElfSpace(const elfheader_t* h, uintptr_t addr);
elfheader_t* FindElfAddress(box64context_t *context, uintptr_t addr);
const char* FindNearestSymbolName(elfheader_t* h, void* p, uintptr_t* start, uint64_t* sz);
int32_t GetTLSBase(elfheader_t* h);
uint32_t GetTLSSize(elfheader_t* h);
void* GetTLSPointer(x64emu_t* emu, elfheader_t* h);
void* GetDTatOffset(x64emu_t* emu, unsigned long int index, unsigned long int offset);
void ResetSpecialCaseMainElf(elfheader_t* h);
void CreateMemorymapFile(box64context_t* context, int fd);
void* GetDynamicSection(elfheader_t* h);

int ElfCheckIfUseTCMallocMinimal(elfheader_t* h);   // return 1 if tcmalloc is used

const char* GetSymbolVersion(elfheader_t* h, int version);
const char* GetParentSymbolVersion(elfheader_t* h, int index);
const char* VersionedName(const char* name, int ver, const char* vername);
int SameVersionedSymbol(const char* name1, int ver1, const char* vername1, int veropt1, const char* name2, int ver2, const char* vername2, int veropt2);
int GetVersionIndice(elfheader_t* h, const char* vername);
int GetNeededVersionCnt(elfheader_t* h, const char* libname);
const char* GetNeededVersionString(elfheader_t* h, const char* libname, int idx);
int GetNeededVersionForLib(elfheader_t* h, const char* libname, const char* ver);

void* ElfGetLocalSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *sz, const char* symname, int* ver, const char** vername, int local, int* veropt);
void* ElfGetGlobalSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *sz, const char* symname, int* ver, const char** vername, int local, int* veropt);
void* ElfGetWeakSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *sz, const char* symname, int* ver, const char** vername, int local, int* veropt);
void* ElfGetSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt);
int ElfGetSymTabStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname);
int ElfGetSymTabStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname);
int ElfGetSymTabStartEnd64(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname);

void* GetNativeSymbolUnversioned(void* lib, const char* name);

void AddMainElfToLinkmap(elfheader_t* lib);

void PltResolver32(x64emu_t* emu);
void PltResolver64(x64emu_t* emu);

const char* getAddrFunctionName(uintptr_t addr);

#endif //__ELF_LOADER_H_
