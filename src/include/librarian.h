#ifndef __LIBRARIAN_H_
#define __LIBRARIAN_H_
#include <stdint.h>

typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;
typedef struct library_s library_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct dlprivate_s dlprivate_t;
typedef struct box64context_s  box64context_t;
typedef struct x64emu_s x64emu_t;
typedef struct elfheader_s elfheader_t;
typedef struct needed_libs_s needed_libs_t;
typedef struct kh_mapoffsets_s kh_mapoffsets_t;
typedef char* cstr_t;

lib_t *NewLibrarian(box64context_t* context, int ownlibs);
void FreeLibrarian(lib_t **maplib, x64emu_t* emu);
dlprivate_t *NewDLPrivate(void);
void FreeDLPrivate(dlprivate_t **lib);

box64context_t* GetLibrarianContext(lib_t* maplib);
kh_mapsymbols_t* GetGlobalData(lib_t* maplib);
int AddNeededLib(lib_t* maplib, int local, int bindnow, needed_libs_t* needed, box64context_t* box64, x64emu_t* emu); // 0=success, 1=error
void RemoveNeededLib(lib_t* maplib, int local, needed_libs_t* needed, box64context_t* box64, x64emu_t* emu);
library_t* GetLibMapLib(lib_t* maplib, const char* name);
library_t* GetLibInternal(const char* name);
void promoteLocalLibGlobal(library_t* lib);
int isLibLocal(library_t* lib);
uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name, int version, const char* vername);
int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, size_t size, int version, const char* vername, const char* globdefver, const char* weakdefver);
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self, int version, const char* vername, const char* globdefver, const char* weakdefver);
int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, const char* defver);
int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self, int version, const char* vername, const char* globdefver, const char* weakdefver);
elfheader_t* GetGlobalSymbolElf(lib_t *maplib, const char* name, int version, const char* vername);
int IsGlobalNoWeakSymbolInNative(lib_t *maplib, const char* name, int version, const char* vername, const char* defver);

void MapLibRemoveLib(lib_t* maplib, library_t* lib);

const char* GetMaplibDefaultVersion(lib_t *maplib, lib_t *local_maplib, int isweak, const char* symname);

const char* FindSymbolName(lib_t *maplib, void* p, void** start, uint64_t* sz, const char** libname, void** base, library_t** lib);

void AddOffsetSymbol(lib_t *maplib, void* offs, const char* name);
const char* GetNameOffset(lib_t *maplib, void* offs);

#endif //__LIBRARIAN_H_
