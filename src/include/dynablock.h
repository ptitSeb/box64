#ifndef __DYNABLOCK_H_
#define __DYNABLOCK_H_

typedef struct x64emu_s x64emu_t;
typedef struct dynablock_s dynablock_t;
typedef struct dynablocklist_s dynablocklist_t;
typedef struct kh_dynablocks_s  kh_dynablocks_t;

uint32_t X31_hash_code(void* addr, int len);
dynablocklist_t* NewDynablockList(uintptr_t text, int textsz, int direct);
void FreeDynablockList(dynablocklist_t** dynablocks);
void FreeDynablock(dynablock_t* db);
void MarkDynablock(dynablock_t* db);
//return 1 if Dynareblock is empty
int FreeRangeDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size);
void MarkRangeDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size);

dynablock_t* FindDynablockFromNativeAddress(void* addr);    // defined in box64context.h
dynablock_t* FindDynablockDynablocklist(void* addr, kh_dynablocks_t* dynablocks);

uintptr_t StartDynablockList(dynablocklist_t* db);
uintptr_t EndDynablockList(dynablocklist_t* db);
void MarkDirectDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size);

// Handling of Dynarec block (i.e. an exectable chunk of x64 translated code)
dynablock_t* DBGetBlock(x64emu_t* emu, uintptr_t addr, int create, dynablock_t** current);   // return NULL if block is not found / cannot be created. Don't create if create==0
dynablock_t* DBAlternateBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr);

// Create and Add an new dynablock in the list, handling direct/map
dynablock_t *AddNewDynablock(dynablocklist_t* dynablocks, uintptr_t addr, int* created);

#endif //__DYNABLOCK_H_