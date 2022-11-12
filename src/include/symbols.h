#ifndef __SYMBOLS_PRIVATE_H_
#define __SYMBOLS_PRIVATE_H_
#include <stdint.h>

#include "custommem.h"
#include "khash.h"

typedef struct versymbols_s versymbols_t;

KHASH_MAP_DECLARE_STR(mapsymbols, versymbols_t)

kh_mapsymbols_t* NewMapSymbols();
void FreeMapSymbols(kh_mapsymbols_t** map);

// replace if already there
void AddSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz, int ver, const char* vername);
uintptr_t FindSymbol(kh_mapsymbols_t *mapsymbols, const char* name, int ver, const char* vername, int local, const char* defver);
// don't add if already there

void AddUniqueSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz, int ver, const char* vername);
int GetSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end, int ver, const char* vername, int local, const char* defver);
int GetSizedSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int ver, const char* vername, int local, const char* defver);
const char* GetSymbolName(kh_mapsymbols_t* mapsymbols, void* p, uintptr_t* offs, uint32_t* sz, const char** vername);

// default version handling
KHASH_MAP_DECLARE_STR(defaultversion, const char*)
kh_defaultversion_t* NewDefaultVersion();
void FreeDefaultVersion(kh_defaultversion_t** def);

void AddDefaultVersion(kh_defaultversion_t* def, const char* symname, const char* vername);
const char* GetDefaultVersion(kh_defaultversion_t* def, const char* symname);

#endif //__SYMBOLS_PRIVATE_H_