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
typedef struct elfheader_s     elfheader_t;

#define LIB_WRAPPED     0
#define LIB_EMULATED    1
#define LIB_UNNKNOW     -1

library_t *NewLibrary(const char* path, box64context_t* box64, elfheader_t* verneeded);
int AddSymbolsLibrary(lib_t* maplib, library_t* lib, x64emu_t* emu);
int FinalizeLibrary(library_t* lib, lib_t* local_maplib, int bindnow, int deepbind, x64emu_t* emu);

char* GetNameLib(library_t *lib);
int IsSameLib(library_t* lib, const char* path);    // check if lib is same (path -> name)
int GetLibGlobalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym);
int GetLibWeakSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym);
int GetLibLocalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym);
char** GetNeededLibsNames(library_t* lib);
int GetNeededLibsN(library_t* lib);
library_t* GetNeededLib(library_t* lib, int idx);
lib_t* GetMaplib(library_t* lib);
int GetDeepBind(library_t* lib);

int GetElfIndex(library_t* lib);    // -1 if no elf (i.e. wrapped)
elfheader_t* GetElf(library_t* lib);    // NULL if no elf (i.e. wrapped)
void* GetHandle(library_t* lib);    // NULL if not wrapped
void IncRefCount(library_t* lib, x64emu_t* emu);
int DecRefCount(library_t** lib, x64emu_t* emu);   // might unload the lib!
int GetRefCount(library_t* lib);

void SetDlOpenIdx(library_t* lib, int dlopen);
#endif //__LIBRARY_H_
