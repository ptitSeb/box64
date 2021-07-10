#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>

#include "box64context.h"
#include "elfloader.h"
#include "debug.h"
#include "x64trace.h"
#include "x64emu.h"
#include "librarian.h"
#include "bridge.h"
#include "library.h"
#include "callback.h"
#include "wrapper.h"
#include "threads.h"
#include "x64trace.h"
#include "signals.h"
#include <sys/mman.h>
#include "custommem.h"
#include "khash.h"
#include "threads.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynarec/arm64_lock.h"

//#define USE_MMAP

// init inside dynablocks.c
KHASH_MAP_INIT_INT64(dynablocks, dynablock_t*)
static dynablocklist_t***  dynmap123[1<<DYNAMAP_SHIFT]; // 64bits.. in 4x16bits array
static pthread_mutex_t     mutex_mmap;
static mmaplist_t          *mmaplist;
static size_t              mmapsize;
static kh_dynablocks_t     *dblist_oversized;      // store the list of oversized dynablocks (normal sized are inside mmaplist)
static uintptr_t***        box64_jmptbl3[1<<JMPTABL_SHIFT];
static uintptr_t**         box64_jmptbldefault2[1<<JMPTABL_SHIFT];
static uintptr_t*          box64_jmptbldefault1[1<<JMPTABL_SHIFT];
static uintptr_t           box64_jmptbldefault0[1<<JMPTABL_SHIFT];
#endif
#define MEMPROT_SHIFT 12
#define MEMPROT_SHIFT2 (32-MEMPROT_SHIFT)
#define MEMPROT_SIZE (1<<(32-MEMPROT_SHIFT))
static pthread_mutex_t     mutex_prot;
KHASH_MAP_INIT_INT(memprot, uint8_t*)
static kh_memprot_t        *memprot;
static int inited = 0;

typedef struct blocklist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
} blocklist_t;

#define MMAPSIZE (256*1024)      // allocate 256kb sized blocks

static pthread_mutex_t     mutex_blocks;
static int                 n_blocks = 0;       // number of blocks for custom malloc
static blocklist_t*        p_blocks = NULL;    // actual blocks for custom malloc

typedef union mark_s {
    struct {
        unsigned int    fill:1;
        unsigned int    size:31;
    };
    uint32_t            x32;
} mark_t;
typedef struct blockmark_s {
    mark_t  prev;
    mark_t  next;
} blockmark_t;


// get first subblock free in block. Return NULL if no block, else first subblock free (mark included), filling size
static void* getFirstBlock(void* block, size_t maxsize, size_t* size)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)block;
    while(m->next.x32) {    // while there is a subblock
        if(!m->next.fill && m->next.size>=maxsize+sizeof(blockmark_t)) {
            *size = m->next.size-sizeof(blockmark_t);
            return m;
        }
        m = (blockmark_t*)((uintptr_t)m + m->next.size);
    }

    return NULL;
}

static size_t getMaxFreeBlock(void* block, size_t block_size)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)((uintptr_t)block+block_size-sizeof(blockmark_t)); // start with the end
    int maxsize = 0;
    while(m->prev.x32) {    // while there is a subblock
        if(!m->prev.fill && m->prev.size>maxsize) {
            maxsize = m->prev.size;
            if((uintptr_t)block+maxsize>(uintptr_t)m)
                return (maxsize>=sizeof(blockmark_t))?(maxsize-sizeof(blockmark_t)):0; // no block large enough left...
        }
        m = (blockmark_t*)((uintptr_t)m - m->prev.size);
    }
    return (maxsize>=sizeof(blockmark_t))?(maxsize-sizeof(blockmark_t)):0;
}

static void* allocBlock(void* block, void *sub, size_t size)
{
    (void)block;

    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = (blockmark_t*)((uintptr_t)s + s->next.size);

    s->next.fill = 1;
    s->next.size = size+sizeof(blockmark_t);
    blockmark_t *m = (blockmark_t*)((uintptr_t)s + s->next.size);   // this is new n
    m->prev.fill = 1;
    m->prev.size = s->next.size;
    if(n!=m) {
        // new mark
        m->prev.fill = 1;
        m->prev.size = s->next.size;
        m->next.fill = 0;
        m->next.size = (uintptr_t)n - (uintptr_t)m;
        n->prev.fill = 0;
        n->prev.size = m->next.size;
    }

    return (void*)((uintptr_t)sub + sizeof(blockmark_t));
}
static void freeBlock(void *block, void* sub)
{
    blockmark_t *m = (blockmark_t*)block;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = (blockmark_t*)((uintptr_t)s + s->next.size);
    if(block!=sub)
        m = (blockmark_t*)((uintptr_t)s - s->prev.size);
    s->next.fill = 0;
    n->prev.fill = 0;
    // check if merge with previous
    if (s->prev.x32 && !s->prev.fill) {
        // remove s...
        m->next.size += s->next.size;
        n->prev.size = m->next.size;
        s = m;
    }
    // check if merge with next
    if(n->next.x32 && !n->next.fill) {
        blockmark_t *n2 = (blockmark_t*)((uintptr_t)n + n->next.size);
        //remove n
        s->next.size += n->next.size;
        n2->prev.size = s->next.size;
    }
}
// return 1 if block has been expanded to new size, 0 if not
static int expandBlock(void* block, void* sub, size_t newsize)
{
    (void)block;

    newsize = (newsize+3)&~3;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = (blockmark_t*)((uintptr_t)s + s->next.size);
    if(s->next.fill)
        return 0;   // next block is filled
    // unsigned bitfield of this length gets "promoted" to *signed* int...
    if((size_t)(s->next.size + n->next.size) < newsize)
        return 0;   // free space too short
    // ok, doing the alloc!
    s->next.size = newsize+sizeof(blockmark_t);
    blockmark_t *m = (blockmark_t*)((uintptr_t)s + s->next.size);   // this is new n
    m->prev.fill = 1;
    m->prev.size = s->next.size;
    if(n!=m) {
        // new mark
        m->prev.fill = 1;
        m->prev.size = s->next.size;
        m->next.fill = 0;
        m->next.size = (uintptr_t)n - (uintptr_t)m;
        n->prev.fill = 0;
        n->prev.size = m->next.size;
    }
    return 1;
}
// return size of block
static size_t sizeBlock(void* sub)
{
    blockmark_t *s = (blockmark_t*)sub;
    return s->next.size;
}

void* customMalloc(size_t size)
{
    // look for free space
    void* sub = NULL;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, size, &rsize);
            if(sub) {
                void* ret = allocBlock(p_blocks[i].block, sub, size);
                if(rsize==p_blocks[i].maxfree)
                    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size);
                pthread_mutex_unlock(&mutex_blocks);
                return ret;
            }
        }
    }
    // add a new block
    int i = n_blocks++;
    p_blocks = (blocklist_t*)realloc(p_blocks, n_blocks*sizeof(blocklist_t));
    size_t allocsize = MMAPSIZE;
    if(size+2*sizeof(blockmark_t)>allocsize)
        allocsize = size+2*sizeof(blockmark_t);
    #ifdef USE_MMAP
    void* p = mmap(NULL, allocsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    memset(p, 0, allocsize);
    #else
    void* p = calloc(1, allocsize);
    #endif
    p_blocks[i].block = p;
    p_blocks[i].size = allocsize;
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = allocsize-sizeof(blockmark_t);
    m = (blockmark_t*)(p+allocsize-sizeof(blockmark_t));
    m->next.x32 = 0;
    m->prev.fill = 0;
    m->prev.size = allocsize-sizeof(blockmark_t);
    // alloc 1st block
    void* ret  = allocBlock(p_blocks[i].block, p, size);
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size);
    pthread_mutex_unlock(&mutex_blocks);
    return ret;
}
void* customCalloc(size_t n, size_t size)
{
    size_t newsize = n*size;
    void* ret = customMalloc(newsize);
    memset(ret, 0, newsize);
    return ret;
}
void* customRealloc(void* p, size_t size)
{
    if(!p)
        return customMalloc(size);
    uintptr_t addr = (uintptr_t)p;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            if(expandBlock(p_blocks[i].block, sub, size)) {
                p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size);
                pthread_mutex_unlock(&mutex_blocks);
                return p;
            }
            pthread_mutex_unlock(&mutex_blocks);
            void* newp = customMalloc(size);
            memcpy(newp, p, sizeBlock(sub));
            customFree(p);
            return newp;
        }
    }
    pthread_mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for realloc, malloc'ing again without free\n", (void*)addr);
    return customMalloc(size);
}
void customFree(void* p)
{
    if(!p)
        return;
    uintptr_t addr = (uintptr_t)p;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            freeBlock(p_blocks[i].block, sub);
            p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size);
            pthread_mutex_unlock(&mutex_blocks);
            return;
        }
    }
    pthread_mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for Free\n", (void*)addr);
}

#ifdef DYNAREC
typedef struct mmaplist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
    kh_dynablocks_t*    dblist;
    uint8_t*            helper;
} mmaplist_t;

uintptr_t FindFreeDynarecMap(dynablock_t* db, size_t size)
{
    // look for free space
    void* sub = NULL;
    for(size_t i=0; i<mmapsize; ++i) {
        if(mmaplist[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(mmaplist[i].block, size, &rsize);
            if(sub) {
                uintptr_t ret = (uintptr_t)allocBlock(mmaplist[i].block, sub, size);
                if(rsize==mmaplist[i].maxfree)
                    mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size);
                kh_dynablocks_t *blocks = mmaplist[i].dblist;
                if(!blocks) {
                    blocks = mmaplist[i].dblist = kh_init(dynablocks);
                    kh_resize(dynablocks, blocks, 64);
                }
                khint_t k;
                int r;
                k = kh_put(dynablocks, blocks, (uintptr_t)ret, &r);
                kh_value(blocks, k) = db;
                for(size_t j=0; j<size; ++j)
                    mmaplist[i].helper[(uintptr_t)ret-(uintptr_t)mmaplist[i].block+j] = (j<256)?j:255;
                return ret;
            }
        }
    }
    return 0;
}

uintptr_t AddNewDynarecMap(dynablock_t* db, size_t size)
{
    size_t i = mmapsize++;
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%zu\n", mmapsize);
    mmaplist = (mmaplist_t*)realloc(mmaplist, mmapsize*sizeof(mmaplist_t));
    #ifndef USE_MMAP
    void *p = NULL;
    if(posix_memalign(&p, box64_pagesize, MMAPSIZE)) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%zu\n", MMAPSIZE, i);
        --mmapsize;
        return 0;
    }
    mprotect(p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    #else
    void* p = mmap(NULL, MMAPSIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(p==(void*)-1) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%zu\n", MMAPSIZE, i);
        --mmapsize;
        return 0;
    }
    #endif
    setProtection((uintptr_t)p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    mmaplist[i].block = p;
    mmaplist[i].size = MMAPSIZE;
    mmaplist[i].helper = (uint8_t*)calloc(1, MMAPSIZE);
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = MMAPSIZE-sizeof(blockmark_t);
    m = (blockmark_t*)(p+MMAPSIZE-sizeof(blockmark_t));
    m->next.x32 = 0;
    m->prev.fill = 0;
    m->prev.size = MMAPSIZE-sizeof(blockmark_t);
    // alloc 1st block
    uintptr_t sub  = (uintptr_t)allocBlock(mmaplist[i].block, p, size);
    mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size);
    kh_dynablocks_t *blocks = mmaplist[i].dblist = kh_init(dynablocks);
    kh_resize(dynablocks, blocks, 64);
    khint_t k;
    int ret;
    k = kh_put(dynablocks, blocks, (uintptr_t)sub, &ret);
    kh_value(blocks, k) = db;
    for(size_t j=0; j<size; ++j)
        mmaplist[i].helper[(uintptr_t)sub-(uintptr_t)mmaplist[i].block + j] = (j<256)?j:255;
    return sub;
}

void ActuallyFreeDynarecMap(dynablock_t* db, uintptr_t addr, size_t size)
{
    (void)db;
    if(!addr || !size)
        return;
    for(size_t i=0; i<mmapsize; ++i) {
        if ((addr>(uintptr_t)mmaplist[i].block) 
         && (addr<((uintptr_t)mmaplist[i].block+mmaplist[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            freeBlock(mmaplist[i].block, sub);
            mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size);
            kh_dynablocks_t *blocks = mmaplist[i].dblist;
            if(blocks) {
                khint_t k = kh_get(dynablocks, blocks, (uintptr_t)sub);
                if(k!=kh_end(blocks))
                    kh_del(dynablocks, blocks, k);
                for(size_t j=0; j<size; ++j)
                    mmaplist[i].helper[(uintptr_t)sub-(uintptr_t)mmaplist[i].block+j] = 0;
            }
            return;
        }
    }
    if(mmapsize)
        dynarec_log(LOG_NONE, "Warning, block %p (size %zu) not found in mmaplist for Free\n", (void*)addr, size);
}

dynablock_t* FindDynablockFromNativeAddress(void* addr)
{
    // look in actual list
    for(size_t i=0; i<mmapsize; ++i) {
        if ((uintptr_t)addr>=(uintptr_t)mmaplist[i].block 
        && ((uintptr_t)addr<(uintptr_t)mmaplist[i].block+mmaplist[i].size)) {
            if(!mmaplist[i].helper)
                return FindDynablockDynablocklist(addr, mmaplist[i].dblist);
            else {
                uintptr_t p = (uintptr_t)addr - (uintptr_t)mmaplist[i].block;
                while(mmaplist[i].helper[p]) p -= mmaplist[i].helper[p];
                khint_t k = kh_get(dynablocks, mmaplist[i].dblist, (uintptr_t)mmaplist[i].block + p);
                if(k!=kh_end(mmaplist[i].dblist))
                    return kh_value(mmaplist[i].dblist, k);
                return NULL;
            }
        }
    }
    // look in oversized
    return FindDynablockDynablocklist(addr, dblist_oversized);
}

uintptr_t AllocDynarecMap(dynablock_t* db, size_t size)
{
    if(!size)
        return 0;
    if(size>MMAPSIZE-2*sizeof(blockmark_t)) {
        #ifndef USE_MMAP
        pthread_mutex_lock(&mutex_mmap);
        void *p = NULL;
        if(posix_memalign(&p, box64_pagesize, size)) {
            dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes\n", size);
            return 0;
        }
        mprotect(p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
        #else
        void* p = mmap(NULL, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
        if(p==(void*)-1) {
            dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes\n", size);
            return 0;
        }
        #endif
        setProtection((uintptr_t)p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
        kh_dynablocks_t *blocks = dblist_oversized;
        if(!blocks) {
            blocks = dblist_oversized = kh_init(dynablocks);
            kh_resize(dynablocks, blocks, 64);
        }
        khint_t k;
        int ret;
        k = kh_put(dynablocks, blocks, (uintptr_t)p, &ret);
        kh_value(blocks, k) = db;
        pthread_mutex_unlock(&mutex_mmap);
        return (uintptr_t)p;
    }
    
    pthread_mutex_lock(&mutex_mmap);

    uintptr_t ret = FindFreeDynarecMap(db, size);
    if(!ret)
        ret = AddNewDynarecMap(db, size);

    pthread_mutex_unlock(&mutex_mmap);

    return ret;
}

void FreeDynarecMap(dynablock_t* db, uintptr_t addr, size_t size)
{
    if(!addr || !size)
        return;
    if(size>MMAPSIZE-2*sizeof(blockmark_t)) {
        pthread_mutex_lock(&mutex_mmap);
        #ifndef USE_MMAP
        free((void*)addr);
        #else
        munmap((void*)addr, size);
        #endif
        kh_dynablocks_t *blocks = dblist_oversized;
        if(blocks) {
            khint_t k = kh_get(dynablocks, blocks, addr);
            if(k!=kh_end(blocks))
                kh_del(dynablocks, blocks, k);
        }
        pthread_mutex_unlock(&mutex_mmap);
        return;
    }
    pthread_mutex_lock(&mutex_mmap);
    ActuallyFreeDynarecMap(db, addr, size);
    pthread_mutex_unlock(&mutex_mmap);
}

dynablocklist_t* getDB(uintptr_t idx)
{
    // already 16bits shifted
    uintptr_t idx3 = (idx>>32)&((1<<DYNAMAP_SHIFT)-1);
    uintptr_t idx2 = (idx>>16)&((1<<DYNAMAP_SHIFT)-1);
    uintptr_t idx1 = (idx    )&((1<<DYNAMAP_SHIFT)-1);

    if(!dynmap123[idx3])
        return NULL;
    if(!dynmap123[idx3][idx2])
        return NULL;
    return dynmap123[idx3][idx2][idx1];
}

// each dynmap is 64k of size

void addDBFromAddressRange(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "addDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        int idx3 = (i>>32)&((1<<DYNAMAP_SHIFT)-1);
        int idx2 = (i>>16)&((1<<DYNAMAP_SHIFT)-1);
        int idx1 = (i    )&((1<<DYNAMAP_SHIFT)-1);
        if(!dynmap123[idx3]) {
            dynablocklist_t*** p = (dynablocklist_t***)calloc(1<<DYNAMAP_SHIFT, sizeof(dynablocklist_t**));
            if(arm64_lock_storeifnull(&dynmap123[idx3], p)!=p)
                free(p);
        }
        if(!dynmap123[idx3][idx2]) {
            dynablocklist_t** p = (dynablocklist_t**)calloc(1<<DYNAMAP_SHIFT, sizeof(dynablocklist_t*));
            if(arm64_lock_storeifnull(&dynmap123[idx3][idx2], p)!=p)
                free(p);
        }
        if(!dynmap123[idx3][idx2][idx1]) {
            dynablocklist_t* p = NewDynablockList(i<<DYNAMAP_SHIFT, 1<<DYNAMAP_SHIFT, 0);
            if(arm64_lock_storeifnull(&dynmap123[idx3][idx2][idx1], p)!=p)
                FreeDynablockList(&p);
        }
    }
}

static int dynmapempty(void** mem)
{
    for (int i=0; i<(1<<DYNAMAP_SHIFT); ++i)
        if(mem[i])
            return 0;
    return 1;
}

void cleanDBFromAddressRange(uintptr_t addr, size_t size, int destroy)
{
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p -> %p %s\n", (void*)addr, (void*)(addr+size-1), destroy?"destroy":"mark");
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        int idx3 = (i>>32)&((1<<DYNAMAP_SHIFT)-1);
        int idx2 = (i>>16)&((1<<DYNAMAP_SHIFT)-1);
        int idx1 = (i    )&((1<<DYNAMAP_SHIFT)-1);
        if(dynmap123[idx3] && dynmap123[idx3][idx2]) {
            dynablocklist_t* dblist = dynmap123[idx3][idx2][idx1];
            if(dblist) {
                if(destroy) {
                    if(FreeRangeDynablock(dblist, addr, size) && 0) {    // dblist is empty, check if we can delete more...
                        // disabling this for now. It seems to cause random crash in Terraria
                        if(!arm64_lock_storeifref(&dynmap123[idx3][idx2][idx1], NULL, dblist)) {
                            dynablocklist_t** p = dynmap123[idx3][idx2];
                            if(dynmapempty((void**)p)) {
                                if(!arm64_lock_storeifref(&dynmap123[idx3][idx2], NULL, p)) {
                                    dynablocklist_t*** p2 = dynmap123[idx3];
                                    if(dynmapempty((void**)p2)) {
                                        if(!arm64_lock_storeifref(&dynmap123[idx3], NULL, p2)) {
                                            free(p2);
                                        }
                                    }
                                    free(p);
                                }
                            }
                            FreeDynablockList(&dblist);
                        }
                    }
                } else
                    MarkRangeDynablock(dblist, addr, size);
            }
        }
    }
}

#ifdef ARM64
void arm64_next(void);
#endif

void addJumpTableIfDefault64(void* addr, void* jmp)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(arm64_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(arm64_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm64_next;
        if(arm64_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            free(tbl);
    }

    arm64_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1][idx0], jmp, arm64_next);
}
void setJumpTableDefault64(void* addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return;
    if(box64_jmptbl3[idx3][idx2][idx1][idx0]==(uintptr_t)arm64_next)
        return;
    box64_jmptbl3[idx3][idx2][idx1][idx0] = (uintptr_t)arm64_next;
}
int isJumpTableDefault64(void* addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return 1;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return 1;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return 1;
    if(box64_jmptbl3[idx3][idx2][idx1][idx0]==(uintptr_t)arm64_next)
        return 1;
    return (box64_jmptbl3[idx3][idx2][idx1][idx0]==(uintptr_t)arm64_next)?1:0;
}
uintptr_t getJumpTable64()
{
    return (uintptr_t)box64_jmptbl3;
}

uintptr_t getJumpTableAddress64(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = ((addr)>>48)&0xffff;
    idx2 = ((addr)>>32)&0xffff;
    idx1 = ((addr)>>16)&0xffff;
    idx0 = ((addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(arm64_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(arm64_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm64_next;
        if(arm64_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            free(tbl);
    }

    return (uintptr_t)&box64_jmptbl3[idx3][idx2][idx1][idx0];
}

// Remove the Write flag from an adress range, so DB can be executed safely
void protectDBnolock(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "protectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    int ret;
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>MEMPROT_SHIFT2)&0xffffffff;
        khint_t k = kh_put(memprot, memprot, key, &ret);
        if(ret) {
            uint8_t *m = (uint8_t*)calloc(1, MEMPROT_SIZE);
            kh_value(memprot, k) = m;
        }
        const uintptr_t ii = i&(MEMPROT_SIZE-1);
        uint8_t prot = kh_value(memprot, k)[ii];
        if(!(prot&PROT_DYNAREC)) {
            if(!prot)
                prot = PROT_READ | PROT_WRITE;    // comes from malloc & co, so should not be able to execute
            kh_value(memprot, k)[ii] = prot|PROT_DYNAREC;
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~(PROT_WRITE|PROT_CUSTOM));
        }
    }
}

void protectDB(uintptr_t addr, size_t size)
{
    pthread_mutex_lock(&mutex_prot);
    protectDBnolock(addr, size);
    pthread_mutex_unlock(&mutex_prot);
}

void lockDB()
{
    pthread_mutex_lock(&mutex_prot);
}

void unlockDB()
{
    pthread_mutex_unlock(&mutex_prot);
}

// Add the Write flag from an adress range, and mark all block as dirty
// no log, as it can be executed inside a signal handler
void unprotectDB(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "unprotectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    int ret;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>MEMPROT_SHIFT2)&0xffffffff;
        khint_t k = kh_put(memprot, memprot, key, &ret);
        if(ret) {
            uint8_t *m = (uint8_t*)calloc(1, MEMPROT_SIZE);
            kh_value(memprot, k) = m;
        }
        const uintptr_t ii = i&(MEMPROT_SIZE-1);
        uint8_t prot = kh_value(memprot, k)[ii];
        if(prot&PROT_DYNAREC) {
            kh_value(memprot, k)[ii] = prot&~PROT_DYNAREC;
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_CUSTOM);
            cleanDBFromAddressRange((i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, 0);
        }
    }
    pthread_mutex_unlock(&mutex_prot);
}

#endif

void updateProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "updateProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    int ret;
    uintptr_t last = idx<<MEMPROT_SHIFT;
    uint8_t oldprot = 0xff;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>MEMPROT_SHIFT2)&0xffffffff;
        khint_t k = kh_put(memprot, memprot, key, &ret);
        if(ret) {
            uint8_t *m = (uint8_t*)calloc(1, MEMPROT_SIZE);
            kh_value(memprot, k) = m;
        }
        const uintptr_t start = i&(MEMPROT_SIZE-1);
        const uintptr_t finish = (((i|(MEMPROT_SIZE-1))<end)?(MEMPROT_SIZE-1):end)&(MEMPROT_SIZE-1);
        uint8_t* block = kh_value(memprot, k);
        for(uintptr_t ii = start; ii<=finish; ++ii) {
            uint32_t dyn = block[ii]&PROT_DYNAREC;
            if(dyn && (prot&PROT_WRITE)) {   // need to remove the write protection from this block
                if(oldprot!=prot) {
                    if(oldprot!=0xff)
                        mprotect((void*)last, (i<<MEMPROT_SHIFT)-last, oldprot&~PROT_CUSTOM); // need to optimize
                    last = i<<MEMPROT_SHIFT;
                    oldprot = prot;
                }
            } else if(prot!=0xff) {
                mprotect((void*)last, (i<<MEMPROT_SHIFT)-last, oldprot&~PROT_CUSTOM); // need to optimize
                last = i << MEMPROT_SHIFT;
                oldprot = 0xff;
            }
            block[ii] = prot|dyn|PROT_ALLOC;
        }
        i+=finish-start;    // +1 from the "for" loop
    }
    if(oldprot!=0xff)
        mprotect((void*)last, (end<<MEMPROT_SHIFT)-last, oldprot&~PROT_CUSTOM); // need to optimize
    pthread_mutex_unlock(&mutex_prot);
}

void setProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "setProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    int ret;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>MEMPROT_SHIFT2)&0xffffffff;
        khint_t k = kh_put(memprot, memprot, key, &ret);
        if(ret) {
            uint8_t *m = (uint8_t*)calloc(1, MEMPROT_SIZE);
            kh_value(memprot, k) = m;
        }
        const uintptr_t start = i&(MEMPROT_SIZE-1);
        const uintptr_t finish = (((i|(MEMPROT_SIZE-1))<end)?(MEMPROT_SIZE-1):end)&(MEMPROT_SIZE-1);
        memset(kh_value(memprot, k)+start, prot|PROT_ALLOC, finish-start+1);
        i+=finish-start;    // +1 from the "for" loop
    }
    pthread_mutex_unlock(&mutex_prot);
}

void allocProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "allocProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    int ret;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>MEMPROT_SHIFT2)&0xffffffff;
        khint_t k = kh_put(memprot, memprot, key, &ret);
        if(ret) {
            uint8_t *m = (uint8_t*)calloc(1, MEMPROT_SIZE);
            kh_value(memprot, k) = m;
        }
        const uintptr_t start = i&(MEMPROT_SIZE-1);
        const uintptr_t finish = (((i|(MEMPROT_SIZE-1))<end)?(MEMPROT_SIZE-1):end)&(MEMPROT_SIZE-1);
        uint8_t* block = kh_value(memprot, k);
        for(uintptr_t ii = start; ii<=finish; ++ii) {
            if(!block[ii])
                block[ii] = prot;
        }
        i+=finish-start;    // +1 from the "for" loop
    }
    pthread_mutex_unlock(&mutex_prot);
}

void loadProtectionFromMap()
{
    char buf[500];
    FILE *f = fopen("/proc/self/maps", "r");
    if(!f)
        return;
    while(!feof(f)) {
        char* ret = fgets(buf, sizeof(buf), f);
        (void)ret;
        char r, w, x;
        uintptr_t s, e;
        if(sscanf(buf, "%lx-%lx %c%c%c", &s, &e, &r, &w, &x)==5) {
            int prot = ((r=='r')?PROT_READ:0)|((w=='w')?PROT_WRITE:0)|((x=='x')?PROT_EXEC:0);
            allocProtection(s, e-s, prot);
        }
    }
    fclose(f);
}

static int blockempty(uint8_t* mem)
{
    for (int i=0; i<(MEMPROT_SIZE); ++i)
        if(mem[i])
            return 0;
    return 1;
}

void freeProtection(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "freeProtection %p:%p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>MEMPROT_SHIFT2)&0xffffffff;
        khint_t k = kh_get(memprot, memprot, key);
        if(k!=kh_end(memprot)) {
            const uintptr_t start = i&(MEMPROT_SIZE-1);
            const uintptr_t finish = (((i|(MEMPROT_SIZE-1))<end)?(MEMPROT_SIZE-1):end)&(MEMPROT_SIZE-1);
            uint8_t *block = kh_value(memprot, k);
            memset(block+start, 0, finish-start+1);
            if(blockempty(block)) {
                free(block);
                kh_del(memprot, memprot, k);
            }
            i+=finish-start;    // +1 from the "for" loop
        }
    }
    pthread_mutex_unlock(&mutex_prot);
}

uint32_t getProtection(uintptr_t addr)
{
    const uint32_t key = (addr>>32)&0xffffffff;
    pthread_mutex_lock(&mutex_prot);
    khint_t k = kh_get(memprot, memprot, key);
    if(k==kh_end(memprot)) {
        pthread_mutex_unlock(&mutex_prot);
        return 0;
    }
    const uintptr_t idx = ((addr&0xffffffff)>>MEMPROT_SHIFT);
    uint32_t ret = kh_val(memprot, k)[idx];
    pthread_mutex_unlock(&mutex_prot);
    return ret;
}

#define LOWEST (void*)0x20000
int availableBlock(uint8_t* p, size_t n)
{
    for (size_t i=0; i<n; ++i, ++p)
        if(*p)
            return 0;
    return 1;
}
static uintptr_t nextFree(uintptr_t addr)
{
    do {
        const uint32_t key = (addr>>32)&0xffffffff;
        khint_t k = kh_get(memprot, memprot, key);
        if(k==kh_end(memprot)) {
            return addr;
        }
        uint8_t *block = kh_value(memprot, k);
        for (uintptr_t i=(addr&0xffffffffLL)>>MEMPROT_SHIFT; i<MEMPROT_SIZE; ++i)
            if(!block[i]) {
                return (addr&~0xffffffffLL)+(i<<MEMPROT_SHIFT);
            }
        addr += 0x100000000LL;
        addr &= ~0xffffffffLL;
    } while(1);
}
static uintptr_t maxFree(uintptr_t addr, uintptr_t sz)
{
    uintptr_t mfree = 0;
    do {
        const uint32_t key = (addr>>32)&0xffffffff;
        khint_t k = kh_get(memprot, memprot, key);
        if(k==kh_end(memprot)) {
            mfree+=0x100000000LL;
            if(mfree>sz) {
                return addr;
            }
        } else {
            uint8_t *block = kh_value(memprot, k);
            for (uintptr_t i=(addr&0xffffffffLL)>>MEMPROT_SHIFT; i<MEMPROT_SIZE; ++i)
                if(!block[i]) {
                    mfree+=1<<MEMPROT_SHIFT;
                } else
                    return mfree;
        }
        addr += 0x100000000LL;
        addr &= ~0xffffffffLL;
    } while(1);
}
void* find32bitBlock(size_t size)
{
    return findBlockNearHint(LOWEST, size);
}
void* find47bitBlock(size_t size)
{
    // slow iterative search... Would need something better one day
    uintptr_t addr = 0x100000000LL;
    pthread_mutex_lock(&mutex_prot);
    do {
        addr = nextFree(addr);
        uintptr_t sz = maxFree(addr, size);
        if(sz>=size) {
            pthread_mutex_unlock(&mutex_prot);
            return (void*)addr;
        }
        addr += sz;
    } while(addr<0x800000000000LL);
    // search in 32bits as a backup
    addr = (uintptr_t)LOWEST;
    do {
        addr = nextFree(addr);
        uintptr_t sz = maxFree(addr, size);
        if(sz>=size) {
            pthread_mutex_unlock(&mutex_prot);
            return (void*)addr;
        }
        addr += sz;
    } while(addr<0x100000000LL);
    pthread_mutex_unlock(&mutex_prot);
    printf_log(LOG_NONE, "Warning: cannot find a 0x%zx block in 47bits address space\n", size);
    return NULL;
}
void* find47bitBlockNearHint(void* hint, size_t size)
{
    // slow iterative search... Would need something better one day
    uintptr_t addr = (uintptr_t)hint;
    pthread_mutex_lock(&mutex_prot);
    do {
        addr = nextFree(addr);
        uintptr_t sz = maxFree(addr, size);
        if(sz>=size) {
            pthread_mutex_unlock(&mutex_prot);
            return (void*)addr;
        }
        addr += sz;
    } while(addr<0x800000000000LL);
    pthread_mutex_unlock(&mutex_prot);
    printf_log(LOG_NONE, "Warning: cannot find a 0x%zx block in 32bits address space\n", size);
    return NULL;
}
void* findBlockNearHint(void* hint, size_t size)
{
    // slow iterative search... Would need something better one day
    uintptr_t addr = (uintptr_t)hint;
    pthread_mutex_lock(&mutex_prot);
    do {
        addr = nextFree(addr);
        uintptr_t sz = maxFree(addr, size);
        if(sz>=size) {
            pthread_mutex_unlock(&mutex_prot);
            return (void*)addr;
        }
        addr += sz;
    } while(addr<0x100000000LL);
    pthread_mutex_unlock(&mutex_prot);
    printf_log(LOG_NONE, "Warning: cannot find a 0x%zx block in 32bits address space\n", size);
    return NULL;
}
#undef LOWEST

int unlockCustommemMutex()
{
    int ret = 0;
    int i = 0;
    #define GO(A, B)                    \
        i = checkUnlockMutex(&A);       \
        if(i) {                         \
            ret|=(1<<B);                \
        }
    GO(mutex_blocks, 0)
    GO(mutex_prot, 1)
    #ifdef DYNAREC
    GO(mutex_mmap, 2)
    #endif
    #undef GO
    return ret;
}

void relockCustommemMutex(int locks)
{
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            pthread_mutex_lock(&A);     \

    GO(mutex_blocks, 0)
    GO(mutex_prot, 1)
    #ifdef DYNAREC
    GO(mutex_mmap, 2)
    #endif
    #undef GO
}

static void init_mutexes(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex_blocks, &attr);
    pthread_mutex_init(&mutex_prot, &attr);
#ifdef DYNAREC
    pthread_mutex_init(&mutex_mmap, &attr);
#endif

    pthread_mutexattr_destroy(&attr);
}

static void atfork_child_custommem(void)
{
    // (re)init mutex if it was lock before the fork
    init_mutexes();
}

void init_custommem_helper(box64context_t* ctx)
{
    (void)ctx;
    if(inited) // already initialized
        return;
    inited = 1;
    memprot = kh_init(memprot);
    init_mutexes();
#ifdef DYNAREC
#ifdef ARM64
    if(box64_dynarec)
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i) {
            box64_jmptbldefault0[i] = (uintptr_t)arm64_next;
            box64_jmptbldefault1[i] = box64_jmptbldefault0;
            box64_jmptbldefault2[i] = box64_jmptbldefault1;
            box64_jmptbl3[i] = box64_jmptbldefault2;
        }
#else
#error Unsupported architecture!
#endif
#endif
    pthread_atfork(NULL, NULL, atfork_child_custommem);
}

void fini_custommem_helper(box64context_t *ctx)
{
    (void)ctx;
    if(!inited)
        return;
    inited = 0;
#ifdef DYNAREC
    if(box64_dynarec) {
        dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
        for (size_t i=0; i<mmapsize; ++i) {
            if(mmaplist[i].block)
                #ifdef USE_MMAP
                munmap(mmaplist[i].block, mmaplist[i].size);
                #else
                free(mmaplist[i].block);
                #endif
            if(mmaplist[i].dblist) {
                kh_destroy(dynablocks, mmaplist[i].dblist);
                mmaplist[i].dblist = NULL;
            }
            if(mmaplist[i].helper) {
                free(mmaplist[i].helper);
                mmaplist[i].helper = NULL;
            }
        }
        if(dblist_oversized) {
            kh_destroy(dynablocks, dblist_oversized);
            dblist_oversized = NULL;
        }
        mmapsize = 0;
        dynarec_log(LOG_DEBUG, "Free dynamic Dynarecblocks\n");
        for (uintptr_t idx3=0; idx3<=0xffff; ++idx3)
            if(dynmap123[idx3]) {
                for (uintptr_t idx2=0; idx2<=0xffff; ++idx2)
                    if(dynmap123[idx3][idx2]) {
                        for (uintptr_t idx1=0; idx1<=0xffff; ++idx1)
                            if(dynmap123[idx3][idx2][idx1])
                                FreeDynablockList(&dynmap123[idx3][idx2][idx1]);
                        free(dynmap123[idx3][idx2]);
                    }
                free(dynmap123[idx3]);
            }

        free(mmaplist);
        pthread_mutex_destroy(&mutex_mmap);
        for (int i3=0; i3<(1<<DYNAMAP_SHIFT); ++i3)
            if(box64_jmptbl3[i3]!=box64_jmptbldefault2) {
                for (int i2=0; i2<(1<<DYNAMAP_SHIFT); ++i2)
                    if(box64_jmptbl3[i3][i2]!=box64_jmptbldefault1) {
                        for (int i1=0; i1<(1<<DYNAMAP_SHIFT); ++i1)
                            if(box64_jmptbl3[i3][i2][i1]!=box64_jmptbldefault0) {
                                free(box64_jmptbl3[i3][i2][i1]);
                            }
                        free(box64_jmptbl3[i3][i2]);
                    }
                free(box64_jmptbl3[i3]);
            }
    }
#endif
    uint8_t* m;
    kh_foreach_value(memprot, m,
        free(m);
    );
    kh_destroy(memprot, memprot);

    for(int i=0; i<n_blocks; ++i)
        #ifdef USE_MMAP
        munmap(p_blocks[i].block, p_blocks[i].size);
        #else
        free(p_blocks[i].block);
        #endif
    free(p_blocks);
    pthread_mutex_destroy(&mutex_prot);
    pthread_mutex_destroy(&mutex_blocks);
}
