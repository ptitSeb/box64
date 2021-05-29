#ifndef __LIBRARY_H_
#define __LIBRARY_H_
#include <stdint.h>
#include "symbols.h"

typedef struct library_s       library_t;
typedef struct lib_s           lib_t;
typedef struct kh_symbolmap_s  kh_symbolmap_t;
typedef struct box64context_s  box64context_t;
typedef struct x64emu_s        x64emu_t;
typedef struct needed_libs_s   needed_libs_t;

library_t *NewLibrary(const char* path, box64context_t* box64);
int AddSymbolsLibrary(lib_t* maplib, library_t* lib, x64emu_t* emu);
int FinalizeLibrary(library_t* lib, lib_t* local_maplib, x64emu_t* emu);
int ReloadLibrary(library_t* lib, x64emu_t* emu);
void InactiveLibrary(library_t* lib);
void Free1Library(library_t **lib, x64emu_t* emu);

char* GetNameLib(library_t *lib);
int IsSameLib(library_t* lib, const char* path);    // check if lib is same (path -> name)
int GetLibSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int local);
int GetLibNoWeakSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int local);
int GetLibLocalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int local);
void fillGLProcWrapper(box64context_t* context);
void freeGLProcWrapper(box64context_t* context);
void fillALProcWrapper(box64context_t* context);
void freeALProcWrapper(box64context_t* context);
needed_libs_t* GetNeededLibs(library_t* lib);
int GetNeededLibN(library_t* lib);
library_t* GetNeededLib(library_t* lib, int idx);
lib_t* GetMaplib(library_t* lib);

int GetElfIndex(library_t* lib);    // -1 if no elf (i.e. native)
void* GetHandle(library_t* lib);    // NULL if not native

#endif //__LIBRARY_H_
