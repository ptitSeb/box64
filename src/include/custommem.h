#ifndef __CUSTOM_MEM__H_
#define __CUSTOM_MEM__H_
#include <unistd.h>
#include <stdint.h>


typedef struct box64context_s box64context_t;

void* customMalloc(size_t size);
void* customCalloc(size_t n, size_t size);
void* customRealloc(void* p, size_t size);
void customFree(void* p);

#define kcalloc     customCalloc
#define kmalloc     customMalloc
#define krealloc    customRealloc
#define kfree       customFree

#ifdef DYNAREC
typedef struct dynablock_s dynablock_t;
typedef struct dynablocklist_s dynablocklist_t;
// custom protection flag to mark Page that are Write protected for Dynarec purpose
uintptr_t AllocDynarecMap(dynablock_t* db, size_t size);
void FreeDynarecMap(dynablock_t* db, uintptr_t addr, size_t size);

void addDBFromAddressRange(uintptr_t addr, size_t size);
void cleanDBFromAddressRange(uintptr_t addr, size_t size, int destroy);

dynablocklist_t* getDB(uintptr_t idx);
void addJumpTableIfDefault64(void* addr, void* jmp);
void setJumpTableDefault64(void* addr);
int isJumpTableDefault64(void* addr);
uintptr_t getJumpTable64();
uintptr_t getJumpTableAddress64(uintptr_t addr);
#endif

#define PROT_DYNAREC    0x80
#define PROT_CUSTOM     (PROT_DYNAREC)

void updateProtection(uintptr_t addr, size_t size, uint32_t prot);
void setProtection(uintptr_t addr, size_t size, uint32_t prot);
void freeProtection(uintptr_t addr, size_t size);
uint32_t getProtection(uintptr_t addr);
void loadProtectionFromMap();
#ifdef DYNAREC
void protectDB(uintptr_t addr, size_t size);
void unprotectDB(uintptr_t addr, size_t size);
int isprotectedDB(uintptr_t addr, size_t size);
#endif
void* find32bitBlock(size_t size);
void* findBlockNearHint(void* hint, size_t size);
void* find47bitBlock(size_t size);
void* find47bitBlockNearHint(void* hint, size_t size);

// unlock mutex that are locked by current thread (for signal handling). Return a mask of unlock mutex
int unlockCustommemMutex();
// relock the muxtex that were unlocked
void relockCustommemMutex(int locks);

void init_custommem_helper(box64context_t* ctx);
void fini_custommem_helper(box64context_t* ctx);

#ifdef DYNAREC
// ---- StrongMemoryModel
void addLockAddress(uintptr_t addr);    // add an address to the list of "LOCK"able
int isLockAddress(uintptr_t addr);  // return 1 is the address is used as a LOCK, 0 else
#endif

#endif //__CUSTOM_MEM__H_
