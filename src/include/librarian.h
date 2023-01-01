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
dlprivate_t *NewDLPrivate();
void FreeDLPrivate(dlprivate_t **lib);

box64context_t* GetLibrarianContext(lib_t* maplib);
kh_mapsymbols_t* GetGlobalData(lib_t* maplib);
int AddNeededLib(lib_t* maplib, int local, int bindnow, needed_libs_t* needed, box64context_t* box64, x64emu_t* emu); // 0=success, 1=error
library_t* GetLibMapLib(lib_t* maplib, const char* name);
library_t* GetLibInternal(const char* name);
uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name, int version, const char* vername);
int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self, size_t size, int version, const char* vername);
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self, int version, const char* vername);
int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername);
int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self, int version, const char* vername);
elfheader_t* GetGlobalSymbolElf(lib_t *maplib, const char* name, int version, const char* vername);
int IsGlobalNoWeakSymbolInNative(lib_t *maplib, const char* name, int version, const char* vername);

const char* FindSymbolName(lib_t *maplib, void* p, void** start, uint64_t* sz, const char** libname, void** base, library_t** lib);

void AddOffsetSymbol(lib_t *maplib, void* offs, const char* name);
const char* GetNameOffset(lib_t *maplib, void* offs);

#endif //__LIBRARIAN_H_
