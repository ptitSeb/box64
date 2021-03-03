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
void FreeLibrarian(lib_t **maplib);
dlprivate_t *NewDLPrivate();
void FreeDLPrivate(dlprivate_t **lib);

box64context_t* GetLibrarianContext(lib_t* maplib);
kh_mapsymbols_t* GetMapSymbol(lib_t* maplib);
kh_mapsymbols_t* GetWeakSymbol(lib_t* maplib);
kh_mapsymbols_t* GetLocalSymbol(lib_t* maplib);
kh_mapsymbols_t* GetGlobalData(lib_t* maplib);
int AddNeededLib(lib_t* maplib, needed_libs_t* neededlibs, int local, const char* path, box64context_t* box86, x64emu_t* emu); // 0=success, 1=error
library_t* GetLibMapLib(lib_t* maplib, const char* name);
library_t* GetLibInternal(const char* name);
uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name);
int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self);
int GetSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self);
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end);
int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end);
int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self);
int GetNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self);
elfheader_t* GetGlobalSymbolElf(lib_t *maplib, const char* name);
int IsGlobalNoWeakSymbolInNative(lib_t *maplib, const char* name);

void AddSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz); // replace if already there
uintptr_t FindSymbol(kh_mapsymbols_t *mapsymbols, const char* name);
void AddWeakSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz); // don't add if already there
int GetSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end);
const char* GetSymbolName(kh_mapsymbols_t* mapsymbols, void* p, uintptr_t* offs, uint32_t* sz);

const char* FindSymbolName(lib_t *maplib, void* p, void** start, uint32_t* sz, const char** libname, void** base);

void AddOffsetSymbol(lib_t *maplib, void* offs, const char* name);
const char* GetNameOffset(lib_t *maplib, void* offs);

#endif //__LIBRARIAN_H_