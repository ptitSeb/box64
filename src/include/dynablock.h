#ifndef __DYNABLOCK_H_
#define __DYNABLOCK_H_

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;

uint32_t X31_hash_code(void* addr, int len);
void FreeDynablock(dynablock_t* db, int need_lock, int need_remove);
void MarkDynablock(dynablock_t* db);
void MarkRangeDynablock(dynablock_t* db, uintptr_t addr, uintptr_t size);
int FreeRangeDynablock(dynablock_t* db, uintptr_t addr, uintptr_t size);
void FreeInvalidDynablock(dynablock_t* db, int need_lock);
dynablock_t* InvalidDynablock(dynablock_t* db, int need_lock);

dynablock_t* FindDynablockFromNativeAddress(void* addr);    // defined in box64context.h

// Handling of Dynarec block (i.e. an exectable chunk of x64 translated code)
dynablock_t* DBGetBlock(x64emu_t* emu, uintptr_t addr, int create, int is32bits);   // return NULL if block is not found / cannot be created. Don't create if create==0
dynablock_t* DBAlternateBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr, int is32bits);

// for use in signal handler
void cancelFillBlock(void);

// clear instruction cache on a range
void ClearCache(void* start, size_t len);

uintptr_t getX64Address(dynablock_t* db, uintptr_t native_addr);
int getX64AddressInst(dynablock_t* db, uintptr_t x64pc);
uintptr_t getX64InstAddress(dynablock_t* db, int inst);

#endif //__DYNABLOCK_H_