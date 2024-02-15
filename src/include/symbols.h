#ifndef __SYMBOLS_PRIVATE_H_
#define __SYMBOLS_PRIVATE_H_
#include <stdint.h>

#include "custommem.h"
#include "khash.h"

typedef struct versymbols_s versymbols_t;

KHASH_MAP_DECLARE_STR(mapsymbols, versymbols_t)

kh_mapsymbols_t* NewMapSymbols(void);
void FreeMapSymbols(kh_mapsymbols_t** map);

// replace if already there
void AddSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz, int ver, const char* vername, int veropt);
uintptr_t FindSymbol(kh_mapsymbols_t *mapsymbols, const char* name, int ver, const char* vername, int local, int veropt);
// Update addr and sz of existing symbols
void ForceUpdateSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz);
// don't add if already there
void AddUniqueSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz, int ver, const char* vername, int veropt);
int GetSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end, int ver, const char* vername, int local, int veropt);
int GetSizedSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int ver, const char* vername, int local, int veropt);
const char* GetSymbolName(kh_mapsymbols_t* mapsymbols, void* p, uintptr_t* offs, uint32_t* sz, const char** vername);

#endif //__SYMBOLS_PRIVATE_H_