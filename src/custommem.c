#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

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
#include "dynarec/native_lock.h"
#include "dynarec/dynarec_next.h"

//#define USE_MMAP

// init inside dynablocks.c
KHASH_MAP_INIT_INT64(dynablocks, dynablock_t*)
static mmaplist_t          *mmaplist = NULL;
static size_t              mmapsize = 0;
static size_t              mmapcap = 0;
static kh_dynablocks_t     *dblist_oversized;      // store the list of oversized dynablocks (normal sized are inside mmaplist)
static uintptr_t***        box64_jmptbl3[1<<JMPTABL_SHIFT];
static uintptr_t**         box64_jmptbldefault2[1<<JMPTABL_SHIFT];
static uintptr_t*          box64_jmptbldefault1[1<<JMPTABL_SHIFT];
static uintptr_t           box64_jmptbldefault0[1<<JMPTABL_SHIFT];
// lock addresses
KHASH_SET_INIT_INT64(lockaddress)
static kh_lockaddress_t    *lockaddress = NULL;
#endif
static pthread_mutex_t     mutex_prot;
#if defined(PAGE64K)
#define MEMPROT_SHIFT 16
#define MEMPROT_SHIFT2 (16+16)
#elif defined(PAGE16K)
#define MEMPROT_SHIFT 14
#define MEMPROT_SHIFT2 (16+14)
#elif defined(PAGE8K)
#define MEMPROT_SHIFT 13
#define MEMPROT_SHIFT2 (16+13)
#else
#define MEMPROT_SHIFT 12
#define MEMPROT_SHIFT2 (16+12)
#endif
#define MEMPROT_SIZE (1<<16)
#define MEMPROT_SIZE0 (48-MEMPROT_SHIFT2)
typedef struct memprot_s
{
    uint8_t* prot;
    uint8_t* hot;
} memprot_t;
static memprot_t memprot[1<<MEMPROT_SIZE0];    // x86_64 mem is 48bits, page is 12bits, so memory is tracked as [20][16][page protection]
static uint8_t   memprot_default[MEMPROT_SIZE];
static int inited = 0;

typedef struct mapmem_s {
    uintptr_t begin, end;
    struct mapmem_s *next;
} mapmem_t;

static mapmem_t *mapmem = NULL;

typedef struct blocklist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
    void*               first;
} blocklist_t;

#define MMAPSIZE (256*1024)      // allocate 256kb sized blocks

static pthread_mutex_t     mutex_blocks;
static int                 n_blocks = 0;       // number of blocks for custom malloc
static int                 c_blocks = 0;       // capacity of blocks for custom malloc
static blocklist_t*        p_blocks = NULL;    // actual blocks for custom malloc

typedef union mark_s {
    struct {
        unsigned int    size:31;
        unsigned int    fill:1;
    };
    uint32_t            x32;
} mark_t;
typedef struct blockmark_s {
    mark_t  prev;
    mark_t  next;
} blockmark_t;

#define NEXT_BLOCK(b) (blockmark_t*)((uintptr_t)(b) + (b)->next.size + sizeof(blockmark_t))
#define PREV_BLOCK(b) (blockmark_t*)(((uintptr_t)(b) - (b)->prev.size) - sizeof(blockmark_t))
#define LAST_BLOCK(b, s) (blockmark_t*)(((uintptr_t)(b)+(s))-sizeof(blockmark_t))

static void printBlock(blockmark_t* b, void* start)
{
    printf_log(LOG_INFO, "========== Block is:\n");
    do {
        printf_log(LOG_INFO, "%c%p, fill=%d, size=0x%x (prev=%d/0x%x)\n", b==start?'*':' ', b, b->next.fill, b->next.size, b->prev.fill, b->prev.size);
        b = NEXT_BLOCK(b);
    } while(b->next.x32);
    printf_log(LOG_INFO, "===================\n");
}

// get first subblock free in block. Return NULL if no block, else first subblock free (mark included), filling size
static void* getFirstBlock(void* block, size_t maxsize, size_t* size, void* start)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)((start)?start:block);
    while(m->next.x32) {    // while there is a subblock
        if(!m->next.fill && m->next.size>=maxsize) {
            *size = m->next.size;
            return m;
        }
        m = NEXT_BLOCK(m);
    }

    return NULL;
}

static void* getNextFreeBlock(void* block)
{
    blockmark_t *m = (blockmark_t*)block;
    while (m->next.fill) {
         m = NEXT_BLOCK(m);
    };
    return m;
}
static void* getPrevFreeBlock(void* block)
{
    blockmark_t *m = (blockmark_t*)block;
    do {
         m = PREV_BLOCK(m);
    } while (m->next.fill);
    return m;
}

static size_t getMaxFreeBlock(void* block, size_t block_size, void* start)
{
    // get start of block
    if(start) {
        blockmark_t *m = (blockmark_t*)start;
        int maxsize = 0;
        while(m->next.x32) {    // while there is a subblock
            if(!m->next.fill && m->next.size>maxsize) {
                maxsize = m->next.size;
            }
            m = NEXT_BLOCK(m);
        }
        return (maxsize>=sizeof(blockmark_t))?maxsize:0;
    } else {
        blockmark_t *m = LAST_BLOCK(block, block_size); // start with the end
        int maxsize = 0;
        while(m->prev.x32) {    // while there is a subblock
            if(!m->prev.fill && m->prev.size>maxsize) {
                maxsize = m->prev.size;
                if((uintptr_t)block+maxsize>(uintptr_t)m)
                    return (maxsize>=sizeof(blockmark_t))?maxsize:0; // no block large enough left...
            }
            m = PREV_BLOCK(m);
        }
        return (maxsize>=sizeof(blockmark_t))?maxsize:0;
    }
}

static void* allocBlock(void* block, void *sub, size_t size, void** pstart)
{
    (void)block;

    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);

    s->next.fill = 1;
    // check if a new mark is worth it
    if(s->next.size>size+2*sizeof(blockmark_t))
        s->next.size = size;
    blockmark_t *m = NEXT_BLOCK(s);   // this is new n
    m->prev.fill = 1;
    if(n!=m) {
        // new mark
        m->prev.size = s->next.size;
        m->next.fill = 0;
        m->next.size = ((uintptr_t)n - (uintptr_t)m) - sizeof(blockmark_t);
        n->prev.fill = 0;
        n->prev.size = m->next.size;
    }

    if(pstart && sub==*pstart) {
        // get the next free block
        m = (blockmark_t*)*pstart;
        while(m->next.fill)
            m = NEXT_BLOCK(m);
        *pstart = (void*)m;
    }
    return (void*)((uintptr_t)sub + sizeof(blockmark_t));
}
static size_t freeBlock(void *block, void* sub, void** pstart)
{
    blockmark_t *m = (blockmark_t*)block;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);
    if(block!=sub)
        m = PREV_BLOCK(s);
    s->next.fill = 0;
    n->prev.fill = 0;
    // check if merge with previous
    if (s->prev.x32 && !s->prev.fill) {
        // remove s...
        m->next.size += s->next.size + sizeof(blockmark_t);
        n->prev.size = m->next.size;
        s = m;
    }
    // check if merge with next
    if(n->next.x32 && !n->next.fill) {
        blockmark_t *n2 = NEXT_BLOCK(n);
        //remove n
        s->next.size += n->next.size + sizeof(blockmark_t);
        n2->prev.size = s->next.size;
    }
    if(pstart && (uintptr_t)*pstart>(uintptr_t)sub) {
        *pstart = (void*)s;
    }
    // return free size at current block (might be bigger)
    return s->next.size;
}
// return 1 if block has been expanded to new size, 0 if not
static int expandBlock(void* block, void* sub, size_t newsize)
{
    (void)block;

    newsize = (newsize+3)&~3;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);
    if(s->next.size>=newsize)
        // big enough, no shrinking...
        return 1;
    if(s->next.fill)
        return 0;   // next block is filled
    // unsigned bitfield of this length gets "promoted" to *signed* int...
    if((size_t)(s->next.size + n->next.size + sizeof(blockmark_t)) < newsize)
        return 0;   // free space too short
    // ok, doing the alloc!
    if((s->next.size+n->next.size+sizeof(blockmark_t))-newsize<2*sizeof(blockmark_t))
        s->next.size += n->next.size+sizeof(blockmark_t);
    else
        s->next.size = newsize+sizeof(blockmark_t);
    blockmark_t *m = NEXT_BLOCK(s);   // this is new n
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

#define THRESHOLD   (128-2*sizeof(blockmark_t))

static size_t roundSize(size_t size)
{
    if(!size)
        return size;
    size = (size+7)&~7LL;   // 8 bytes align in size

    if(size<THRESHOLD)
        size = THRESHOLD;

    return size;
}

void* customMalloc(size_t size)
{
    size = roundSize(size);
    // look for free space
    void* sub = NULL;
    size_t fullsize = size+2*sizeof(blockmark_t);
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, size, &rsize, p_blocks[i].first);
            if(sub) {
                if(rsize-size<THRESHOLD)
                    size = rsize;
                void* ret = allocBlock(p_blocks[i].block, sub, size, NULL);
                if(sub==p_blocks[i].first)
                    p_blocks[i].first = getNextFreeBlock(sub);
                if(rsize==p_blocks[i].maxfree)
                    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
                pthread_mutex_unlock(&mutex_blocks);
                return ret;
            }
        }
    }
    // add a new block
    int i = n_blocks++;
    if(n_blocks>c_blocks) {
        c_blocks += 4;
        p_blocks = (blocklist_t*)box_realloc(p_blocks, c_blocks*sizeof(blocklist_t));
    }
    size_t allocsize = (fullsize>MMAPSIZE)?fullsize:MMAPSIZE;
    #ifdef USE_MMAP
    void* p = mmap(NULL, allocsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    memset(p, 0, allocsize);
    #else
    void* p = box_calloc(1, allocsize);
    #endif
    p_blocks[i].block = p;
    p_blocks[i].first = p;
    p_blocks[i].size = allocsize;
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = allocsize-2*sizeof(blockmark_t);
    blockmark_t* n = NEXT_BLOCK(m);
    n->next.x32 = 0;
    n->prev.fill = 0;
    n->prev.size = m->next.size;
    // alloc 1st block
    void* ret  = allocBlock(p_blocks[i].block, p, size, NULL);
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, NULL);
    pthread_mutex_unlock(&mutex_blocks);
    return ret;
}
void* customCalloc(size_t n, size_t size)
{
    size_t newsize = roundSize(n*size);
    void* ret = customMalloc(newsize);
    memset(ret, 0, newsize);
    return ret;
}
void* customRealloc(void* p, size_t size)
{
    if(!p)
        return customMalloc(size);
    size = roundSize(size);
    uintptr_t addr = (uintptr_t)p;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            if(expandBlock(p_blocks[i].block, sub, size)) {
                if(sub<p_blocks[i].first && p+size<p_blocks[i].first)
                    p_blocks[i].first = getNextFreeBlock(sub);
                p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
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
            void* n = NEXT_BLOCK((blockmark_t*)sub);
            size_t newfree = freeBlock(p_blocks[i].block, sub, NULL);
            if(sub<=p_blocks[i].first)
                p_blocks[i].first = getPrevFreeBlock(n);
            if(p_blocks[i].maxfree < newfree) p_blocks[i].maxfree = newfree;
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
    void*               first;  // first free block, to speed up things
    int                 locked; // don't try to add stuff on locked block
} mmaplist_t;

uintptr_t FindFreeDynarecMap(dynablock_t* db, size_t size)
{
    // look for free space
    void* sub = NULL;
    for(size_t i=0; i<mmapsize; ++i) {
        if(mmaplist[i].maxfree>=size+sizeof(blockmark_t) && !mmaplist[i].locked) {
            mmaplist[i].locked = 1;
            size_t rsize = 0;
            sub = getFirstBlock(mmaplist[i].block, size, &rsize, mmaplist[i].first);
            if(sub) {
                uintptr_t ret = (uintptr_t)allocBlock(mmaplist[i].block, sub, size, &mmaplist[i].first);
                if(rsize==mmaplist[i].maxfree) {
                    mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size, mmaplist[i].first);
                }
                kh_dynablocks_t *blocks = mmaplist[i].dblist;
                if(!blocks) {
                    blocks = mmaplist[i].dblist = kh_init(dynablocks);
                    kh_resize(dynablocks, blocks, 64);
                }
                khint_t k;
                int r;
                k = kh_put(dynablocks, blocks, (uintptr_t)ret, &r);
                kh_value(blocks, k) = db;
                int size255=(size<256)?size:255;
                for(size_t j=0; j<size255; ++j)
                    mmaplist[i].helper[(uintptr_t)ret-(uintptr_t)mmaplist[i].block+j] = j;
                if(size!=size255)
                    memset(&mmaplist[i].helper[(uintptr_t)ret-(uintptr_t)mmaplist[i].block+256], -1, size-255);
                mmaplist[i].locked = 0;
                return ret;
            } else {
                printf_log(LOG_INFO, "BOX64: Warning, sub not found, corrupted mmaplist[%zu] info?\n", i);
                if(box64_log >= LOG_DEBUG)
                    printBlock(mmaplist[i].block, mmaplist[i].first);
            }
        }
    }
    return 0;
}

uintptr_t AddNewDynarecMap(dynablock_t* db, size_t size)
{
    size_t i = mmapsize++;
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%zu/%zu\n", mmapsize, mmapcap);
    if(mmapsize>mmapcap) {
        mmapcap += 32;
        mmaplist = (mmaplist_t*)box_realloc(mmaplist, mmapcap*sizeof(mmaplist_t));
    }
    #ifndef USE_MMAP
    void *p = NULL;
    if(!(p=box_memalign(box64_pagesize, MMAPSIZE))) {
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

    mmaplist[i].locked = 1;
    mmaplist[i].block = p;
    mmaplist[i].size = MMAPSIZE;
    mmaplist[i].helper = (uint8_t*)box_calloc(1, MMAPSIZE);
    mmaplist[i].first = p;
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = MMAPSIZE-2*sizeof(blockmark_t);
    blockmark_t* n = NEXT_BLOCK(m);
    n->next.x32 = 0;
    n->prev.fill = 0;
    n->prev.size = m->next.size;
    // alloc 1st block
    uintptr_t sub  = (uintptr_t)allocBlock(mmaplist[i].block, p, size, &mmaplist[i].first);
    mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size, mmaplist[i].first);
    kh_dynablocks_t *blocks = mmaplist[i].dblist = kh_init(dynablocks);
    kh_resize(dynablocks, blocks, 64);
    khint_t k;
    int ret;
    k = kh_put(dynablocks, blocks, (uintptr_t)sub, &ret);
    kh_value(blocks, k) = db;
    for(size_t j=0; j<size; ++j)
        mmaplist[i].helper[(uintptr_t)sub-(uintptr_t)mmaplist[i].block + j] = (j<256)?j:255;
    mmaplist[i].locked = 0;
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
            size_t newfree = freeBlock(mmaplist[i].block, sub, &mmaplist[i].first);
            if(mmaplist[i].maxfree < newfree) mmaplist[i].maxfree = newfree;
            kh_dynablocks_t *blocks = mmaplist[i].dblist;
            if(blocks) {
                khint_t k = kh_get(dynablocks, blocks, (uintptr_t)sub);
                if(k!=kh_end(blocks))
                    kh_del(dynablocks, blocks, k);
                memset(&mmaplist[i].helper[(uintptr_t)sub-(uintptr_t)mmaplist[i].block], 0, size);
            }
            if(mmaplist[i].locked) {
                printf_log(LOG_INFO, "BOX64: Warning, Free a chunk in a locked mmaplist[%zu]\n", i);
                ++mmaplist[i].locked;
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
        void *p = NULL;
        if(!(p=box_memalign(box64_pagesize, size))) {
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
        return (uintptr_t)p;
    }
    

    uintptr_t ret = FindFreeDynarecMap(db, size);
    if(!ret)
        ret = AddNewDynarecMap(db, size);

    return ret;
}

void FreeDynarecMap(dynablock_t* db, uintptr_t addr, size_t size)
{
    if(!addr || !size)
        return;
    if(size>MMAPSIZE-2*sizeof(blockmark_t)) {
        #ifndef USE_MMAP
        box_free((void*)addr);
        #else
        munmap((void*)addr, size);
        #endif
        kh_dynablocks_t *blocks = dblist_oversized;
        if(blocks) {
            khint_t k = kh_get(dynablocks, blocks, addr);
            if(k!=kh_end(blocks))
                kh_del(dynablocks, blocks, k);
        }
        return;
    }
    ActuallyFreeDynarecMap(db, addr, size);
}

uintptr_t getSizeJmpDefault(uintptr_t addr, size_t maxsize)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return (addr&~((1LL<<48)-1)|0xffffffffffffLL)-addr + 1;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return (addr&~((1LL<<32)-1)|0xffffffffLL)-addr + 1;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    uintptr_t* block = box64_jmptbl3[idx3][idx2][idx1];
    if(block == box64_jmptbldefault0)
        return (addr&~((1LL<<16)-1)|0xffffLL)-addr + 1;
    idx0 = addr&0xffff;
    if (maxsize>0x10000)
        maxsize = 0x10000;
    while(idx0<maxsize && block[idx0]==(uintptr_t)native_next)
        ++idx0;
    return idx0 - (addr&0xffff);
}

// each dynmap is 64k of size

void addDBFromAddressRange(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "addDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    // do nothing, dynablock are allowed based on memory protection flags
}

void cleanDBFromAddressRange(uintptr_t addr, size_t size, int destroy)
{
    uintptr_t start_addr = my_context?((addr<my_context->max_db_size)?0:(addr-my_context->max_db_size)):addr;
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p/%p -> %p %s\n", (void*)addr, (void*)start_addr, (void*)(addr+size-1), destroy?"destroy":"mark");
    for (uintptr_t i=start_addr; i<addr+size; ++i) {
        dynablock_t* db = getDB(i);
        if(db) {
            if(destroy)
                FreeRangeDynablock(db, addr, size);
            else
                MarkRangeDynablock(db, addr, size);
        } else {
            uintptr_t next = getSizeJmpDefault(i, size-i);
            if(next)
                i+=next-1;
        }
    }
}

int addJumpTableIfDefault64(void* addr, void* jmp)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(native_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)native_next;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            box_free(tbl);
    }

    return (native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1][idx0], jmp, native_next)==jmp)?1:0;
}
void setJumpTableDefault64(void* addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    box64_jmptbl3[idx3][idx2][idx1][idx0] = (uintptr_t)native_next;
}
void setJumpTableDefaultRef64(void* addr, void* jmp)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1][idx0], native_next, jmp);
}
int setJumpTableIfRef64(void* addr, void* jmp, void* ref)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(native_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)native_next;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            box_free(tbl);
    }
    return (native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1][idx0], jmp, ref)==jmp)?1:0;
}
int isJumpTableDefault64(void* addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = (((uintptr_t)addr)>>48)&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return 1;
    idx2 = (((uintptr_t)addr)>>32)&0xffff;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return 1;
    idx1 = (((uintptr_t)addr)>>16)&0xffff;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return 1;
    idx0 = (((uintptr_t)addr)    )&0xffff;
    return (box64_jmptbl3[idx3][idx2][idx1][idx0]==(uintptr_t)native_next)?1:0;
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
        uintptr_t*** tbl = (uintptr_t***)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(native_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)native_next;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            box_free(tbl);
    }

    return (uintptr_t)&box64_jmptbl3[idx3][idx2][idx1][idx0];
}

uintptr_t getJumpAddress64(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = ((addr)>>48)&0xffff;
    idx2 = ((addr)>>32)&0xffff;
    idx1 = ((addr)>>16)&0xffff;
    idx0 = ((addr)    )&0xffff;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(native_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            box_free(tbl);
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)native_next;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            box_free(tbl);
    }

    return (uintptr_t)box64_jmptbl3[idx3][idx2][idx1][idx0];
}

dynablock_t* getDB(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    idx3 = ((addr)>>48)&0xffff;
    idx2 = ((addr)>>32)&0xffff;
    idx1 = ((addr)>>16)&0xffff;
    idx0 = ((addr)    )&0xffff;
    /*if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        return NULL;
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        return NULL;
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        return NULL;
    }*/

    uintptr_t ret = (uintptr_t)box64_jmptbl3[idx3][idx2][idx1][idx0];
    if(ret==(uintptr_t)native_next)
        return NULL;

    return *(dynablock_t**)(ret - sizeof(void*));
}

// Remove the Write flag from an adress range, so DB can be executed safely
void protectDB(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "protectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1;
    if(end<idx) // memory addresses higher than 48bits are not tracked
        return;
    pthread_mutex_lock(&mutex_prot);
    int ret;
    for (uintptr_t i=(idx>>16); i<=(end>>16); ++i)
        if(memprot[i].prot==memprot_default) {
            uint8_t* newblock = box_calloc(1<<16, sizeof(uint8_t));
            /*if (native_lock_storeifref(&memprot[i], newblock, memprot_default) != newblock) {
                box_free(newblock);
            }*/
            memprot[i].prot = newblock;
        }
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i>>16].prot[i&0xffff];
        uint32_t dyn = prot&PROT_CUSTOM;
        prot&=~PROT_CUSTOM;
        if(!prot)
            prot = PROT_READ | PROT_WRITE | PROT_EXEC;      // comes from malloc & co, so should not be able to execute
        if((prot&PROT_WRITE)) {
            if(!dyn) mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
            memprot[i>>16].prot[i&0xffff] = prot|PROT_DYNAREC;   // need to use atomic exchange?
        } else 
            memprot[i>>16].prot[i&0xffff] = prot|PROT_DYNAREC_R;
    }
    pthread_mutex_unlock(&mutex_prot);
}

// Add the Write flag from an adress range, and mark all block as dirty
// no log, as it can be executed inside a signal handler
void unprotectDB(uintptr_t addr, size_t size, int mark)
{
    dynarec_log(LOG_DEBUG, "unprotectDB %p -> %p (mark=%d)\n", (void*)addr, (void*)(addr+size-1), mark);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1;
    if(end<idx) // memory addresses higher than 48bits are not tracked
        return;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=(idx>>16); i<=(end>>16); ++i)
        if(memprot[i].prot==memprot_default) {
            uint8_t* newblock = box_calloc(1<<16, sizeof(uint8_t));
            /*if (native_lock_storeifref(&memprot[i], newblock, memprot_default) != newblock) {
                box_free(newblock);
            }*/
            memprot[i].prot = newblock;
        }
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i>>16].prot[i&0xffff];
        if(prot&PROT_DYNAREC) {
            prot&=~PROT_CUSTOM;
            if(mark)
                cleanDBFromAddressRange((i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, 0);
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot);
            memprot[i>>16].prot[i&0xffff] = prot;  // need to use atomic exchange?
        } else if(prot&PROT_DYNAREC_R)
            memprot[i>>16].prot[i&0xffff] = prot&~PROT_CUSTOM;
    }
    pthread_mutex_unlock(&mutex_prot);
}

int isprotectedDB(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "isprotectedDB %p -> %p => ", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1LL;
    if(end<idx) { // memory addresses higher than 48bits are not tracked
        dynarec_log(LOG_DEBUG, "00\n");
        return 0;
    }
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i>>16].prot[i&0xffff];
        if(!(prot&PROT_DYNAREC || prot&PROT_DYNAREC_R)) {
            dynarec_log(LOG_DEBUG, "0\n");
            return 0;
        }
    }
    dynarec_log(LOG_DEBUG, "1\n");
    return 1;
}

#endif

void printMapMem()
{
    mapmem_t* m = mapmem;
    while(m) {
        printf_log(LOG_INFO, " %p-%p\n", (void*)m->begin, (void*)m->end);
        m = m->next;
    }
}

void addMapMem(uintptr_t begin, uintptr_t end)
{
    begin &=~0xfff;
    end = (end&~0xfff)+0xfff; // full page
    // sanitize values
    if(end<0x10000) return;
    if(!begin) begin = 0x10000;
    // find attach point (cannot be the 1st one by construction)
    mapmem_t* m = mapmem;
    while(m->next && begin>m->next->begin) {
        m = m->next;
    }
    // attach at the end of m
    mapmem_t* newm;
    if(m->end>=begin-1) {
        if(end<=m->end)
            return; // zone completly inside current block, nothing to do
        m->end = end;   // enlarge block
        newm = m;
    } else {
    // create a new block
        newm = (mapmem_t*)box_calloc(1, sizeof(mapmem_t));
        newm->next = m->next;
        newm->begin = begin;
        newm->end = end;
        m->next = newm;
    }
    while(newm && newm->next && (newm->next->begin-1)<=newm->end) {
        // fuse with next
        if(newm->next->end>newm->end)
            newm->end = newm->next->end;
        mapmem_t* tmp = newm->next;
        newm->next = tmp->next;
        box_free(tmp);
    }
    // all done!
}
void removeMapMem(uintptr_t begin, uintptr_t end)
{
    begin &=~0xfff;
    end = (end&~0xfff)+0xfff; // full page
    // sanitize values
    if(end<0x10000) return;
    if(!begin) begin = 0x10000;
    mapmem_t* m = mapmem, *prev = NULL;
    while(m) {
        // check if block is beyond the zone to free
        if(m->begin > end)
            return;
        // check if the block is completly inside the zone to free
        if(m->begin>=begin && m->end<=end) {
            // just free the block
            mapmem_t *tmp = m;
            if(prev) {
                prev->next = m->next;
                m = prev;
            } else {
                mapmem = m->next; // change attach, but should never happens
                m = mapmem;
                prev = NULL;
            }
            box_free(tmp);
        } else if(begin>m->begin && end<m->end) { // the zone is totaly inside the block => split it!
            mapmem_t* newm = (mapmem_t*)box_calloc(1, sizeof(mapmem_t));    // create a new "next"
            newm->end = m->end;
            m->end = begin - 1;
            newm->begin = end + 1;
            newm->next = m->next;
            m->next = newm;
            // nothing more to free
            return;
        } else if(begin>m->begin && begin<m->end) { // free the tail of the block
            m->end = begin - 1;
        } else if(end>m->begin && end<m->end) { // free the head of the block
            m->begin = end + 1;
        }
        prev = m;
        m = m->next;
    }
}

void updateProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "updateProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    addMapMem(addr, addr+size-1);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1;
    if(end<idx) // memory addresses higher than 48bits are not tracked
        return;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=(idx>>16); i<=(end>>16); ++i)
        if(memprot[i].prot==memprot_default) {
            uint8_t* newblock = box_calloc(1<<16, sizeof(uint8_t));
#if 0 //def ARM64   //disabled for now, not usefull with the mutex
            if (native_lock_storeifref(&memprot[i], newblock, memprot_default) != newblock) {
                box_free(newblock);
            }
#else
            memprot[i].prot = newblock;
#endif
        }
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t dyn=(memprot[i>>16].prot[i&0xffff]&(PROT_DYNAREC | PROT_DYNAREC_R));
        if(dyn && (prot&PROT_WRITE)) {   // need to remove the write protection from this block
            dyn = PROT_DYNAREC;
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
        } else if(dyn && !(prot&PROT_WRITE)) {
            dyn = PROT_DYNAREC_R;
        }
        memprot[i>>16].prot[i&0xffff] = prot|dyn;
    }
    pthread_mutex_unlock(&mutex_prot);
}

void setProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    addMapMem(addr, addr+size-1);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1;
    if(end<idx) // memory addresses higher than 48bits are not tracked
        return;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=(idx>>16); i<=(end>>16); ++i) {
        if(memprot[i].prot==memprot_default && prot) {
            uint8_t* newblock = box_calloc(MEMPROT_SIZE, sizeof(uint8_t));
#if 0 //def ARM64   //disabled for now, not usefull with the mutex
            if (native_lock_storeifref(&memprot[i], newblock, memprot_default) != newblock) {
                box_free(newblock);
            }
#else
            memprot[i].prot = newblock;
#endif
        }
        if(prot || memprot[i].prot!=memprot_default) {
            uintptr_t bstart = ((i<<16)<idx)?(idx&0xffff):0;
            uintptr_t bend = (((i<<16)+0xffff)>end)?(end&0xffff):0xffff;
            for (uintptr_t j=bstart; i<=bend; ++i)
                memprot[i].prot[j] = prot;
        }
    }
    pthread_mutex_unlock(&mutex_prot);
}

void refreshProtection(uintptr_t addr)
{
    pthread_mutex_lock(&mutex_prot);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    if(memprot[idx>>16].prot!=memprot_default) {
        int prot = memprot[idx>>16].prot[idx&0xffff];
        int ret = mprotect((void*)(idx<<MEMPROT_SHIFT), box64_pagesize, prot&~PROT_CUSTOM);
printf_log(LOG_INFO, "refreshProtection(%p): %p/0x%x (ret=%d/%s)\n", (void*)addr, (void*)(idx<<MEMPROT_SHIFT), prot, ret, ret?strerror(errno):"ok");
    }
    pthread_mutex_unlock(&mutex_prot);
}

void allocProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "allocProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    addMapMem(addr, addr+size-1);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1;
    if(end<idx) // memory addresses higher than 48bits are not tracked
        return;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=(idx>>16); i<=(end>>16); ++i)
        if(memprot[i].prot==memprot_default) {
            uint8_t* newblock = box_calloc(1<<16, sizeof(uint8_t));
#if 0 //def ARM64   //disabled for now, not usefull with the mutex
            if (native_lock_storeifref(&memprot[i], newblock, memprot_default) != newblock) {
                box_free(newblock);
            }
#else
            memprot[i].prot = newblock;
#endif
        }
    for (uintptr_t i=idx; i<=end; ++i) {
        const uintptr_t start = i&(MEMPROT_SIZE-1);
        const uintptr_t finish = (((i|(MEMPROT_SIZE-1))<end)?(MEMPROT_SIZE-1):end)&(MEMPROT_SIZE-1);
        uint8_t* block = memprot[i>>16].prot;
        for(uintptr_t ii = start; ii<=finish; ++ii) {
            if(!block[ii])
                block[ii] = prot;
        }
        i+=finish-start;    // +1 from the "for" loop
    }
    pthread_mutex_unlock(&mutex_prot);
}

#ifdef DYNAREC
#define HOTPAGE_STEP 16
int IsInHotPage(uintptr_t addr) {
    if(addr<=(1LL<<48))
        return 0;
    int idx = (addr>>MEMPROT_SHIFT)>>16;
    uint8_t *block = memprot[idx].hot;
    if(!block)
        return 0;
    int base = (addr>>MEMPROT_SHIFT)&0xffff;
    if(!block[base])
        return 0;
    // decrement hot
    native_lock_decifnot0b(&block[base]);
    return 1;
}

int AreaInHotPage(uintptr_t start, uintptr_t end_) {
    //dynarec_log(LOG_DEBUG, "AreaInHotPage %p -> %p => ", (void*)start, (void*)end_);
    uintptr_t idx = (start>>MEMPROT_SHIFT);
    uintptr_t end = (end_>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1LL;
    if(end<idx) { // memory addresses higher than 48bits are not tracked
        //dynarec_log(LOG_DEBUG, "00\n");
        return 0;
    }
    int ret = 0;
    for (uintptr_t i=idx; i<=end; ++i) {
        uint8_t *block = memprot[i>>16].hot;
        if(block) {
            uint32_t hot = block[i&0xffff];
            if(hot) {
                // decrement hot
                native_lock_decifnot0b(&block[i&0xffff]);
                //dynarec_log(LOG_DEBUG, "1\n");
                ret = 1;
            }
        } else {
            i+=0xffff;
        }
    }
    //dynarec_log(LOG_DEBUG, "0\n");
    return ret;

}

void AddHotPage(uintptr_t addr) {
    int idx = (addr>>MEMPROT_SHIFT)>>16;
    int base = (addr>>MEMPROT_SHIFT)&0xffff;
    if(!memprot[idx].hot) {
            uint8_t* newblock = box_calloc(1<<16, sizeof(uint8_t));
            if (native_lock_storeifnull(&memprot[idx].hot, newblock) != newblock) {
                box_free(newblock);
            }
    }
    native_lock_storeb(&memprot[idx].hot[base], HOTPAGE_STEP);
}
#endif

void loadProtectionFromMap()
{
    if(box64_mapclean)
        return;
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
    box64_mapclean = 1;
}

static int blockempty(uint8_t* mem)
{
    uint64_t *p8 = (uint64_t*)mem;
    for (int i=0; i<(MEMPROT_SIZE)/8; ++i, ++p8)
        if(*p8)
            return 0;
    return 1;
}

void freeProtection(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "freeProtection %p:%p\n", (void*)addr, (void*)(addr+size-1));
    removeMapMem(addr, addr+size-1);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    if(end>=(1LL<<(48-MEMPROT_SHIFT)))
        end = (1LL<<(48-MEMPROT_SHIFT))-1;
    if(end<idx) // memory addresses higher than 48bits are not tracked
        return;
    pthread_mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        const uint32_t key = (i>>16);
        const uintptr_t start = i&(MEMPROT_SIZE-1);
        const uintptr_t finish = (((i|(MEMPROT_SIZE-1))<end)?(MEMPROT_SIZE-1):end)&(MEMPROT_SIZE-1);
        if(memprot[key].prot!=memprot_default) {
            uint8_t *block = memprot[key].prot;
            if(start==0 && finish==MEMPROT_SIZE-1) {
                memprot[key].prot = memprot_default;
                box_free(block);
                if(memprot[key].hot) {
                    uint8_t *hot = memprot[key].hot;
                    memprot[key].hot = NULL;
                    box_free(hot);
                }
            } else {
                memset(block+start, 0, (finish-start+1)*sizeof(uint8_t));
                // blockempty is quite slow, so disable the free of blocks for now
#if 0 //def ARM64   //disabled for now, not useful with the mutex
                if (blockempty(block)) {
                    block = (void*)native_lock_xchg(&memprot[key], (uintptr_t)memprot_default);
                    if(!blockempty(block)) {
                        block = (void*)native_lock_xchg(&memprot[key], (uintptr_t)block);
                        for (int i = 0; i < 0x10000; ++i) {
                            memprot[key][i] |= block[i];
                        }
                    }
                    if (block != memprot_default) box_free(block);
                }
#else
                /*else if(blockempty(block)) {
                    memprot[key] = memprot_default;
                    box_free(block);
                }*/
#endif
            }
        }
        i+=finish-start;    // +1 from the "for" loop
    }
    pthread_mutex_unlock(&mutex_prot);
}

uint32_t getProtection(uintptr_t addr)
{
    if(addr>=(1LL<<48))
        return 0;
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uint32_t ret = memprot[idx>>16].prot[idx&0xffff];
    return ret;
}

#define LOWEST (void*)0x10000
#define MEDIUM (void*)0x20000000

void* find31bitBlockNearHint(void* hint, size_t size)
{
    mapmem_t* m = mapmem;
    uintptr_t h = (uintptr_t)hint;
    if(hint<LOWEST) hint = LOWEST;
    while(m && m->end<0x80000000LL) {
        // granularity 0x10000
        uintptr_t addr = (m->end+1+0xffff)&~0xffff;
        uintptr_t end = (m->next)?(m->next->begin-1):0xffffffffffffffffLL;
        // check hint and available size
        if(addr<=h && end>=h && end-h+1>=size)
            return hint;
        if(addr>=h && end-addr+1>=size)
            return (void*)addr;
        m = m->next;
    }
    return NULL;
}

void* find32bitBlock(size_t size)
{
    void* ret = find31bitBlockNearHint(MEDIUM, size);
    if(ret)
        return ret;
    ret = find31bitBlockNearHint(LOWEST, size);
    return ret?ret:find47bitBlock(size);
}
void* find47bitBlock(size_t size)
{
    void* ret = find47bitBlockNearHint((void*)0x100000000LL, size);
    if(!ret)
        ret = find32bitBlock(size);
    return ret;
}
void* find47bitBlockNearHint(void* hint, size_t size)
{
    mapmem_t* m = mapmem;
    uintptr_t h = (uintptr_t)hint;
    if(hint<LOWEST) hint = LOWEST;
    while(m && m->end<0x800000000000LL) {
        // granularity 0x10000
        uintptr_t addr = (m->end+1+0xffff)&~0xffff;
        uintptr_t end = (m->next)?(m->next->begin-1):0xffffffffffffffffLL;
        // check hint and available size
        if(addr<=h && end>=h && end-h+1>=size)
            return hint;
        if(addr>=h && end-addr+1>=size)
            return (void*)addr;
        m = m->next;
    }
    return NULL;
}

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
    #undef GO
    return ret;
}

void relockCustommemMutex(int locks)
{
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            pthread_mutex_trylock(&A);  \

    GO(mutex_blocks, 0)
    GO(mutex_prot, 1)
    #undef GO
}

static void init_mutexes(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex_blocks, &attr);
    pthread_mutex_init(&mutex_prot, &attr);

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
    memset(memprot_default, 0, sizeof(memprot_default));
    for(int i=0; i<(1<<MEMPROT_SIZE0); ++i)
        memprot[i].prot = memprot_default;
    init_mutexes();
#ifdef DYNAREC
    if(box64_dynarec)
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i) {
            box64_jmptbldefault0[i] = (uintptr_t)native_next;
            box64_jmptbldefault1[i] = box64_jmptbldefault0;
            box64_jmptbldefault2[i] = box64_jmptbldefault1;
            box64_jmptbl3[i] = box64_jmptbldefault2;
        }
    lockaddress = kh_init(lockaddress);
#endif
    pthread_atfork(NULL, NULL, atfork_child_custommem);
    // init mapmem list
    mapmem = (mapmem_t*)box_calloc(1, sizeof(mapmem_t));
    mapmem->begin = 0x0;
    mapmem->end = (uintptr_t)LOWEST - 1;
    loadProtectionFromMap();
    // check if PageSize is correctly defined
    if(box64_pagesize != (1<<MEMPROT_SHIFT)) {
        printf_log(LOG_NONE, "Error: PageSize configuation is wrong: configured with %d, but got %d\n", 1<<MEMPROT_SHIFT, box64_pagesize);
        exit(-1);   // abort or let it continue?
    }
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
                box_free(mmaplist[i].block);
                #endif
            if(mmaplist[i].dblist) {
                kh_destroy(dynablocks, mmaplist[i].dblist);
                mmaplist[i].dblist = NULL;
            }
            if(mmaplist[i].helper) {
                box_free(mmaplist[i].helper);
                mmaplist[i].helper = NULL;
            }
        }
        if(dblist_oversized) {
            kh_destroy(dynablocks, dblist_oversized);
            dblist_oversized = NULL;
        }
        mmapsize = 0;
        mmapcap = 0;

        box_free(mmaplist);
        for (int i3=0; i3<(1<<DYNAMAP_SHIFT); ++i3)
            if(box64_jmptbl3[i3]!=box64_jmptbldefault2) {
                for (int i2=0; i2<(1<<DYNAMAP_SHIFT); ++i2)
                    if(box64_jmptbl3[i3][i2]!=box64_jmptbldefault1) {
                        for (int i1=0; i1<(1<<DYNAMAP_SHIFT); ++i1)
                            if(box64_jmptbl3[i3][i2][i1]!=box64_jmptbldefault0) {
                                box_free(box64_jmptbl3[i3][i2][i1]);
                            }
                        box_free(box64_jmptbl3[i3][i2]);
                    }
                box_free(box64_jmptbl3[i3]);
            }
    }
    kh_destroy(lockaddress, lockaddress);
    lockaddress = NULL;
#endif
    uint8_t* m;
    for(int i=0; i<(1<<MEMPROT_SIZE0); ++i) {
        m = memprot[i].prot;
        if(m!=memprot_default)
            box_free(m);
        m = memprot[i].hot;
        if(m)
            box_free(m);
    }

    for(int i=0; i<n_blocks; ++i)
        #ifdef USE_MMAP
        munmap(p_blocks[i].block, p_blocks[i].size);
        #else
        box_free(p_blocks[i].block);
        #endif
    box_free(p_blocks);
    pthread_mutex_destroy(&mutex_prot);
    pthread_mutex_destroy(&mutex_blocks);
    while(mapmem) {
        mapmem_t *tmp = mapmem;
        mapmem = mapmem->next;
        box_free(tmp);
    }
}

#ifdef DYNAREC
// add an address to the list of "LOCK"able
void addLockAddress(uintptr_t addr)
{
    int ret;
    kh_put(lockaddress, lockaddress, addr, &ret);
}

// return 1 is the address is used as a LOCK, 0 else
int isLockAddress(uintptr_t addr)
{
    khint_t k = kh_get(lockaddress, lockaddress, addr);
    return (k==kh_end(lockaddress))?0:1;
}

#endif
