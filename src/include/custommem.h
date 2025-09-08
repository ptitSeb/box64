#ifndef __CUSTOM_MEM__H_
#define __CUSTOM_MEM__H_
#include <unistd.h>
#include <stdint.h>


typedef struct box64context_s box64context_t;

void* customMalloc(size_t size);
void* customMalloc32(size_t size);
void* customCalloc(size_t n, size_t size);
void* customCalloc32(size_t n, size_t size);
void* customRealloc(void* p, size_t size);
void* customRealloc32(void* p, size_t size);
void* customMemAligned(size_t align, size_t size);
void* customMemAligned32(size_t align, size_t size);
void customFree(void* p);
void customFree32(void* p);
size_t customGetUsableSize(void* p);

#define kcalloc     customCalloc
#define kmalloc     customMalloc
#define krealloc    customRealloc
#define kfree       customFree

#define ALIGN(p) (((p)+box64_pagesize-1)&~(box64_pagesize-1))

#ifndef MAP_32BIT
#define MAP_32BIT       0x40
#endif

#ifdef DYNAREC
typedef struct dynablock_s dynablock_t;
typedef struct mmaplist_s mmaplist_t;
typedef struct DynaCacheBlock_s DynaCacheBlock_t;
// custom protection flag to mark Page that are Write protected for Dynarec purpose
uintptr_t AllocDynarecMap(uintptr_t x64_addr, size_t size, int is_new);
void FreeDynarecMap(uintptr_t addr);
mmaplist_t* NewMmaplist();
void DelMmaplist(mmaplist_t* list);
int MmaplistHasNew(mmaplist_t* list, int clear);
int MmaplistNBlocks(mmaplist_t* list);
void MmaplistFillBlocks(mmaplist_t* list, DynaCacheBlock_t* blocks);
void MmaplistAddNBlocks(mmaplist_t* list, int nblocks);
int MmaplistAddBlock(mmaplist_t* list, int fd, off_t offset, void* orig, size_t size, intptr_t delta_map, uintptr_t mapping_start);

void addDBFromAddressRange(uintptr_t addr, size_t size);
// Will return 1 if at least 1 db in the address range
int cleanDBFromAddressRange(uintptr_t addr, size_t size, int destroy);

dynablock_t* getDB(uintptr_t idx);
int getNeedTest(uintptr_t idx);
int addJumpTableIfDefault64(void* addr, void* jmp); // return 1 if write was succesfull
int setJumpTableIfRef64(void* addr, void* jmp, void* ref); // return 1 if write was succesfull
void setJumpTableDefault64(void* addr);
void setJumpTableDefaultRef64(void* addr, void* jmp);
int isJumpTableDefault64(void* addr);
uintptr_t getJumpTable64(void);
uintptr_t getJumpTable48(void);
uintptr_t getJumpTable32(void);
uintptr_t getJumpTableAddress64(uintptr_t addr);
uintptr_t getJumpAddress64(uintptr_t addr);

#ifdef SAVE_MEM
#define JMPTABL_SHIFTMAX   JMPTABL_SHIFT4
#define JMPTABL_SHIFT4 16
#define JMPTABL_SHIFT3 16
#define JMPTABL_SHIFT2 12
#define JMPTABL_SHIFT1 12
#define JMPTABL_SHIFT0 10
#define JMPTABL_START4 (JMPTABL_START3+JMPTABL_SHIFT3)
#define JMPTABL_START3 (JMPTABL_START2+JMPTABL_SHIFT2)
#define JMPTABL_START2 (JMPTABL_START1+JMPTABL_SHIFT1)
#define JMPTABL_START1 (JMPTABL_START0+JMPTABL_SHIFT0)
#define JMPTABL_START0 0
#define JMPTABLE_MASK4 ((1<<JMPTABL_SHIFT4)-1)
#define JMPTABLE_MASK3 ((1<<JMPTABL_SHIFT3)-1)
#define JMPTABLE_MASK2 ((1<<JMPTABL_SHIFT2)-1)
#define JMPTABLE_MASK1 ((1<<JMPTABL_SHIFT1)-1)
#define JMPTABLE_MASK0 ((1<<JMPTABL_SHIFT0)-1)
#else
#define JMPTABL_SHIFTMAX   JMPTABL_SHIFT3
#define JMPTABL_SHIFT3 16
#define JMPTABL_SHIFT2 16
#define JMPTABL_SHIFT1 18
#define JMPTABL_SHIFT0 14
#define JMPTABL_START3 (JMPTABL_START2+JMPTABL_SHIFT2)
#define JMPTABL_START2 (JMPTABL_START1+JMPTABL_SHIFT1)
#define JMPTABL_START1 (JMPTABL_START0+JMPTABL_SHIFT0)
#define JMPTABL_START0 0
#define JMPTABLE_MASK3 ((1<<JMPTABL_SHIFT3)-1)
#define JMPTABLE_MASK2 ((1<<JMPTABL_SHIFT2)-1)
#define JMPTABLE_MASK1 ((1<<JMPTABL_SHIFT1)-1)
#define JMPTABLE_MASK0 ((1<<JMPTABL_SHIFT0)-1)
#endif //SAVE_MEM
#endif

#define PROT_NEVERCLEAN 0x100
#define PROT_DYNAREC    0x80
#define PROT_DYNAREC_R  0x40
#define PROT_NOPROT     0x20
#define PROT_DYN        (PROT_DYNAREC | PROT_DYNAREC_R | PROT_NOPROT | PROT_NEVERCLEAN)
#define PROT_CUSTOM     (PROT_DYNAREC | PROT_DYNAREC_R | PROT_NOPROT | PROT_NEVERCLEAN)
#define PROT_NEVERPROT  (PROT_NOPROT | PROT_NEVERCLEAN)
#define PROT_WAIT       0xFF

void updateProtection(uintptr_t addr, size_t size, uint32_t prot);
void setProtection(uintptr_t addr, size_t size, uint32_t prot);
void setProtection_mmap(uintptr_t addr, size_t size, uint32_t prot);
void setProtection_box(uintptr_t addr, size_t size, uint32_t prot);
void setProtection_stack(uintptr_t addr, size_t size, uint32_t prot);
void setProtection_elf(uintptr_t addr, size_t size, uint32_t prot);
void freeProtection(uintptr_t addr, size_t size);
void refreshProtection(uintptr_t addr);
uint32_t getProtection(uintptr_t addr);
uint32_t getProtection_fast(uintptr_t addr);
int getMmapped(uintptr_t addr);
int memExist(uintptr_t addr);
void loadProtectionFromMap(void);
#ifdef DYNAREC
void protectDB(uintptr_t addr, size_t size);
void protectDBJumpTable(uintptr_t addr, size_t size, void* jump, void* ref);
void unprotectDB(uintptr_t addr, size_t size, int mark);    // if mark==0, the blocks are not marked as potentially dirty
void neverprotectDB(uintptr_t addr, size_t size, int mark);
void unneverprotectDB(uintptr_t addr, size_t size);
int isprotectedDB(uintptr_t addr, size_t size);
#endif
void* find32bitBlock(size_t size);
void* find31bitBlockNearHint(void* hint, size_t size, uintptr_t mask);
void* find47bitBlock(size_t size);
void* find47bitBlockNearHint(void* hint, size_t size, uintptr_t mask); // mask can be 0 for default one (0xffff)
void* find47bitBlockElf(size_t size, int mainbin, uintptr_t mask);
void* find31bitBlockElf(size_t size, int mainbin, uintptr_t mask);
int isBlockFree(void* hint, size_t size);

// relock the muxtex that were unlocked
void relockCustommemMutex(int locks);

void init_custommem_helper(box64context_t* ctx);
void fini_custommem_helper(box64context_t* ctx);

#ifdef DYNAREC
// ---- StrongMemoryModel
void addLockAddress(uintptr_t addr);    // add an address to the list of "LOCK"able
int isLockAddress(uintptr_t addr);  // return 1 is the address is used as a LOCK, 0 else
int nLockAddressRange(uintptr_t start, size_t size);    // gives the number of lock address for a range
void getLockAddressRange(uintptr_t start, size_t size, uintptr_t addrs[]);   // fill in the array with the lock addresses in the range (array must be of the correct size)

void CheckHotPage(uintptr_t addr, uint32_t prot);
int isInHotPage(uintptr_t addr);
int checkInHotPage(uintptr_t addr);
#endif

// this will simulate an x86_64 version of the function (no tracking will done, but tracking will be used)
void* box_mmap(void* addr, size_t length, int prot, int flags, int fd, ssize_t offset);
// this will simulate an x86_64 version of the function (no tracking will done)
int box_munmap(void* addr, size_t length);

void reserveHighMem();

#endif //__CUSTOM_MEM__H_
