#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <syscall.h>

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
#include "rbtree.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynarec/dynablock_private.h"
#include "dynarec/native_lock.h"
#include "dynarec/dynarec_next.h"

#define USE_MMAP

// init inside dynablocks.c
static mmaplist_t          *mmaplist = NULL;
static uint64_t jmptbl_allocated = 0, jmptbl_allocated1 = 0, jmptbl_allocated2 = 0, jmptbl_allocated3 = 0;
#ifdef JMPTABL_SHIFT4
static uint64_t jmptbl_allocated4 = 0;
static uintptr_t****       box64_jmptbl4[1<<JMPTABL_SHIFT4];
static uintptr_t***        box64_jmptbldefault3[1<<JMPTABL_SHIFT3];
#else
static uintptr_t***        box64_jmptbl3[1<<JMPTABL_SHIFT3];
#endif
static uintptr_t**         box64_jmptbldefault2[1<<JMPTABL_SHIFT2];
static uintptr_t*          box64_jmptbldefault1[1<<JMPTABL_SHIFT1];
static uintptr_t           box64_jmptbldefault0[1<<JMPTABL_SHIFT0];
// lock addresses
KHASH_SET_INIT_INT64(lockaddress)
static kh_lockaddress_t    *lockaddress = NULL;
#ifdef USE_CUSTOM_MUTEX
static uint32_t            mutex_prot;
static uint32_t            mutex_blocks;
#else
static pthread_mutex_t     mutex_prot;
static pthread_mutex_t     mutex_blocks;
#endif
#else
static pthread_mutex_t     mutex_prot;
static pthread_mutex_t     mutex_blocks;
#endif
//#define TRACE_MEMSTAT
rbtree* memprot = NULL;
int have48bits = 0;
static int inited = 0;

static rbtree*  mapallmem = NULL;
static rbtree*  mmapmem = NULL;

typedef struct blocklist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
    void*               first;
} blocklist_t;

#define MMAPSIZE (64*1024)      // allocate 64kb sized blocks
#define DYNMMAPSZ (2*1024*1024) // allocate 2Mb block for dynarec

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

void printBlock(blockmark_t* b, void* start)
{
    printf_log(LOG_NONE, "========== Block is:\n");
    do {
        printf_log(LOG_NONE, "%c%p, fill=%d, size=0x%x (prev=%d/0x%x)\n", b==start?'*':' ', b, b->next.fill, b->next.size, b->prev.fill, b->prev.size);
        b = NEXT_BLOCK(b);
    } while(b->next.x32);
    printf_log(LOG_NONE, "===================\n");
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
        unsigned int maxsize = 0;
        while(m->next.x32) {    // while there is a subblock
            if(!m->next.fill && m->next.size>maxsize) {
                maxsize = m->next.size;
            }
            m = NEXT_BLOCK(m);
        }
        return (maxsize>=sizeof(blockmark_t))?maxsize:0;
    } else {
        blockmark_t *m = LAST_BLOCK(block, block_size); // start with the end
        unsigned int maxsize = 0;
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

#define THRESHOLD   (128-2*sizeof(blockmark_t))

static void* allocBlock(void* block, void *sub, size_t size, void** pstart)
{
    (void)block;

    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);

    s->next.fill = 1;
    // check if a new mark is worth it
    if(s->next.size>size+2*sizeof(blockmark_t)+THRESHOLD) {
        size_t old_size = s->next.size;
        s->next.size = size;
        blockmark_t *m = NEXT_BLOCK(s);
        m->prev.fill = 1;
        m->prev.size = s->next.size;
        m->next.fill = 0;
        m->next.size = old_size - (size + sizeof(blockmark_t));
        n->prev.fill = 0;
        n->prev.size = m->next.size;
        n = m;
    } else {
        n->prev.fill = 1;
    }

    if(pstart && sub==*pstart) {
        // get the next free block
        while(n->next.fill)
            n = NEXT_BLOCK(n);
        *pstart = (void*)n;
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
    if (m!=s && s->prev.x32 && !s->prev.fill) {
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
    if(pstart && (uintptr_t)*pstart>(uintptr_t)s) {
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
    if((s->next.size+n->next.size+sizeof(blockmark_t))-newsize<THRESHOLD+2*sizeof(blockmark_t))
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

// return 1 if block is coherent, 0 if not (and printf the issues)
int printBlockCoherent(int i)
{
    if(i<0 || i>=n_blocks) {
        printf_log(LOG_NONE, "Error, %d should be between 0 and %d\n", i, n_blocks);
        return 0;
    }
    int ret = 1;
    blockmark_t* m = (blockmark_t*)p_blocks[i].block;
    // check if first is correct
    blockmark_t* first = getNextFreeBlock(m);
    if(p_blocks[i].first && p_blocks[i].first!=first) {printf_log(LOG_NONE, "First %p and stored first %p differs for block %d\n", first, p_blocks[i].first, i); ret = 0;}
    // check if maxfree is correct, with no hint
    size_t maxfree = getMaxFreeBlock(m, p_blocks[i].size, NULL);
    if(maxfree != p_blocks[i].maxfree) {printf_log(LOG_NONE, "Maxfree without hint %zd and stored maxfree %zd differs for block %d\n", maxfree, p_blocks[i].maxfree, i); ret = 0;}
    // check if maxfree from first is correct
    maxfree = getMaxFreeBlock(m, p_blocks[i].size, p_blocks[i].first);
    if(maxfree != p_blocks[i].maxfree) {printf_log(LOG_NONE, "Maxfree with hint %zd and stored maxfree %zd differs for block %d\n", maxfree, p_blocks[i].maxfree, i); ret = 0;}
    // check chain
    blockmark_t* last = (blockmark_t*)(((uintptr_t)m)+p_blocks[i].size-sizeof(blockmark_t));
    while(m<last) {
        blockmark_t* n = NEXT_BLOCK(m);
        if(!m->next.fill && !n->next.fill && n!=last) {
            printf_log(LOG_NONE, "Chain contains 2 subsequent free blocks %p (%d) and %p (%d) for block %d\n", m, m->next.size, n, n->next.size, i);
            ret = 0;
        }
        m = n;
    }
    if(m!=last) {
        printf_log(LOG_NONE, "Last block %p is behond expexted block %p for block %d\n", m, last, i);
        ret = 0;
    }

    return ret;
}

void testAllBlocks()
{
    size_t total = 0;
    size_t fragmented_free = 0;
    size_t max_free = 0;
    for(int i=0; i<n_blocks; ++i) {
        printBlockCoherent(i);
        total += p_blocks[i].size;
        if(max_free<p_blocks[i].maxfree)
            max_free = p_blocks[i].maxfree;
        blockmark_t* m = (blockmark_t*)p_blocks[i].block;
        while(m->next.x32) {
            if(!m->next.fill)
                fragmented_free += m->next.size;
            m = NEXT_BLOCK(m);
        }
    }
    printf_log(LOG_NONE, "Total %d blocks, for %zd allocated memory, max_free %zd, toatal fragmented free %zd\n", n_blocks, total, max_free, fragmented_free);
}

static size_t roundSize(size_t size)
{
    if(!size)
        return size;
    size = (size+7)&~7LL;   // 8 bytes align in size

    if(size<THRESHOLD)
        size = THRESHOLD;

    return size;
}

#ifdef DYNAREC
#define GET_PROT_WAIT(A, B) \
        uint32_t A;         \
        do {                \
            A = native_lock_xchg_b(&block[B], PROT_WAIT);    \
        } while(A==PROT_WAIT)
#define GET_PROT(A, B)      \
        uint32_t A;         \
        do {                \
            A = native_lock_get_b(&block[B]);   \
        } while(A==PROT_WAIT)

#define SET_PROT(A, B)      native_lock_storeb(&block[A], B)
#define LOCK_NODYNAREC()
#define UNLOCK_DYNAREC()    UNLOCK_PROT()
#define UNLOCK_NODYNAREC()
#else
#define GET_PROT_WAIT(A, B) uint32_t A = block[B]
#define GET_PROT(A, B)      uint32_t A = block[B]
#define SET_PROT(A, B)      block[A] = B
#define LOCK_NODYNAREC()    LOCK_PROT()
#define UNLOCK_DYNAREC()
#define UNLOCK_NODYNAREC()  UNLOCK_PROT()
#endif
static uintptr_t    defered_prot_p = 0;
static size_t       defered_prot_sz = 0;
static uint32_t     defered_prot_prot = 0;
static sigset_t     critical_prot = {0};
#define LOCK_PROT()         sigset_t old_sig = {0}; pthread_sigmask(SIG_BLOCK, &critical_prot, &old_sig); mutex_lock(&mutex_prot)
#define LOCK_PROT_READ()    sigset_t old_sig = {0}; pthread_sigmask(SIG_BLOCK, &critical_prot, &old_sig); mutex_lock(&mutex_prot)
#define UNLOCK_PROT()       if(defered_prot_p) {                                \
                                uintptr_t p = defered_prot_p; size_t sz = defered_prot_sz; uint32_t prot = defered_prot_prot; \
                                defered_prot_p = 0;                             \
                                pthread_sigmask(SIG_SETMASK, &old_sig, NULL);   \
                                mutex_unlock(&mutex_prot);                      \
                                setProtection(p, sz, prot);                     \
                            } else {                                            \
                                pthread_sigmask(SIG_SETMASK, &old_sig, NULL);   \
                                mutex_unlock(&mutex_prot);                      \
                            }
#define UNLOCK_PROT_READ()  mutex_unlock(&mutex_prot); pthread_sigmask(SIG_SETMASK, &old_sig, NULL)


#ifdef TRACE_MEMSTAT
static uint64_t customMalloc_allocated = 0;
#endif
void* customMalloc(size_t size)
{
    size = roundSize(size);
    // look for free space
    void* sub = NULL;
    size_t fullsize = size+2*sizeof(blockmark_t);
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, size, &rsize, p_blocks[i].first);
            if(sub) {
                if(rsize-size<THRESHOLD)
                    size = rsize;
                void* ret = allocBlock(p_blocks[i].block, sub, size, &p_blocks[i].first);
                if(rsize==p_blocks[i].maxfree)
                    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
                mutex_unlock(&mutex_blocks);
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
    void* p = internal_mmap(NULL, allocsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    memset(p, 0, allocsize);
    #else
    void* p = box_calloc(1, allocsize);
    #endif
#ifdef TRACE_MEMSTAT
    customMalloc_allocated += allocsize;
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
    void* ret  = allocBlock(p_blocks[i].block, p, size, &p_blocks[i].first);
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
    mutex_unlock(&mutex_blocks);
    if(mapallmem) {
        // defer the setProtection...
        //setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE);
        defered_prot_p = (uintptr_t)p;
        defered_prot_sz = allocsize;
        defered_prot_prot = PROT_READ|PROT_WRITE;
    }
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
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            if(expandBlock(p_blocks[i].block, sub, size)) {
                if(sub<p_blocks[i].first && p+size>=p_blocks[i].first)
                    p_blocks[i].first = getNextFreeBlock(sub);
                p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
                mutex_unlock(&mutex_blocks);
                return p;
            }
            mutex_unlock(&mutex_blocks);
            void* newp = customMalloc(size);
            memcpy(newp, p, sizeBlock(sub));
            customFree(p);
            return newp;
        }
    }
    mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for realloc, malloc'ing again without free\n", (void*)addr);
    return customMalloc(size);
}
void customFree(void* p)
{
    if(!p)
        return;
    uintptr_t addr = (uintptr_t)p;
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(p_blocks[i].block, sub, &p_blocks[i].first);
            if(p_blocks[i].maxfree < newfree) p_blocks[i].maxfree = newfree;
            mutex_unlock(&mutex_blocks);
            return;
        }
    }
    mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for Free\n", (void*)addr);
}

#ifdef DYNAREC
#define NCHUNK          64
typedef struct mmaplist_s {
    blocklist_t         chunks[NCHUNK];
    mmaplist_t*         next;
} mmaplist_t;

dynablock_t* FindDynablockFromNativeAddress(void* p)
{
    if(!p)
        return NULL;
    
    uintptr_t addr = (uintptr_t)p;

    int i= 0;
    mmaplist_t* list = mmaplist;
    if(!list)
        return NULL;
    while(list) {
        if ((addr>(uintptr_t)list->chunks[i].block) 
         && (addr<((uintptr_t)list->chunks[i].block+list->chunks[i].size))) {
            blockmark_t* sub = (blockmark_t*)list->chunks[i].block;
            while((uintptr_t)sub<addr) {
                blockmark_t* n = NEXT_BLOCK(sub);
                if((uintptr_t)n>addr) {
                    // found it!
                    // self is the field of a block
                    return *(dynablock_t**)((uintptr_t)sub+sizeof(blockmark_t));
                }
                sub = n;
            }
            return NULL;
        }
        ++i;
        if(i==NCHUNK) {
            i = 0;
            list = list->next;
        }
    }
    return NULL;
}

#ifdef TRACE_MEMSTAT
static uint64_t dynarec_allocated = 0;
#endif
uintptr_t AllocDynarecMap(size_t size)
{
    if(!size)
        return 0;

    size = roundSize(size);

    mmaplist_t* list = mmaplist;
    if(!list)
        list = mmaplist = (mmaplist_t*)box_calloc(1, sizeof(mmaplist_t));
    // check if there is space in current open ones
    int i = 0;
    uintptr_t sz = size + 2*sizeof(blockmark_t);
    while(1) {
        if(list->chunks[i].maxfree>=size) {
            // looks free, try to alloc!
            size_t rsize = 0;
            void* sub = getFirstBlock(list->chunks[i].block, size, &rsize, list->chunks[i].first);
            if(sub) {
                void* ret = allocBlock(list->chunks[i].block, sub, size, NULL);
                if(sub==list->chunks[i].first)
                    list->chunks[i].first = getNextFreeBlock(sub);
                if(rsize==list->chunks[i].maxfree)
                    list->chunks[i].maxfree = getMaxFreeBlock(list->chunks[i].block, list->chunks[i].size, list->chunks[i].first);
                return (uintptr_t)ret;
            }
        }
        // check if new
        if(!list->chunks[i].size) {
            // alloc a new block, aversized or not, we are at the end of the list
            size_t allocsize = (sz>DYNMMAPSZ)?sz:DYNMMAPSZ;
            // allign sz with pagesize
            allocsize = (allocsize+(box64_pagesize-1))&~(box64_pagesize-1);
            #ifndef USE_MMAP
            void *p = NULL;
            if(!(p=box_memalign(box64_pagesize, allocsize))) {
                dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes\n", allocsize), allocsize, strerror(errno);
                return 0;
            }
            mprotect(p, allocsize, PROT_READ | PROT_WRITE | PROT_EXEC);
            #else
            void* p=MAP_FAILED;
            // disabling for now. explicit hugepage needs to be enabled to be used on userspace 
            // with`/sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages` as the number of allowaed 2M huge page
            // At least with a 2M allocation, transparent huge page should kick-in
            #if 0//def MAP_HUGETLB
            if(allocsize==DYNMMAPSZ) {
                p = internal_mmap(NULL, allocsize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_HUGETLB, -1, 0);
                if(p!=MAP_FAILED) printf_log(LOG_INFO, "Allocated a dynarec memory block with HugeTLB\n");
                else printf_log(LOG_INFO, "Failled to allocated a dynarec memory block with HugeTLB (%s)\n", strerror(errno));
            }
            #endif
            if(p==MAP_FAILED)
                p = internal_mmap(NULL, allocsize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
            if(p==MAP_FAILED) {
                dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes (%s)\n", allocsize, strerror(errno));
                return 0;
            }
            #ifdef MADV_HUGEPAGE
            madvise(p, allocsize, MADV_HUGEPAGE);
            #endif
            #endif
#ifdef TRACE_MEMSTAT
            dynarec_allocated += allocsize;
#endif
            setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE | PROT_EXEC);
            list->chunks[i].block = p;
            list->chunks[i].first = p;
            list->chunks[i].size = allocsize;
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
            void* ret  = allocBlock(list->chunks[i].block, p, size, NULL);
            list->chunks[i].maxfree = getMaxFreeBlock(list->chunks[i].block, list->chunks[i].size, NULL);
            if(list->chunks[i].maxfree)
                list->chunks[i].first = getNextFreeBlock(m);
            return (uintptr_t)ret;
        }
        // next chunk...
        ++i;
        if(i==NCHUNK) {
            i = 0;
            if(!list->next)
                list->next = (mmaplist_t*)box_calloc(1, sizeof(mmaplist_t));
            list = list->next;
        }
    }
}

void FreeDynarecMap(uintptr_t addr)
{
    if(!addr)
        return;
    
    int i= 0;
    mmaplist_t* list = mmaplist;

    while(list) {
        if ((addr>(uintptr_t)list->chunks[i].block) 
         && (addr<((uintptr_t)list->chunks[i].block+list->chunks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(list->chunks[i].block, sub, &list->chunks[i].first);
            if(list->chunks[i].maxfree < newfree)
                list->chunks[i].maxfree = newfree;
            return;
        }
        ++i;
        if(i==NCHUNK) {
            i = 0;
            list = list->next;
        }
    }
}

static uintptr_t getDBSize(uintptr_t addr, size_t maxsize, dynablock_t** db)
{
    #ifdef JMPTABL_START4
    const uintptr_t idx4 = (addr>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    const uintptr_t idx3 = (addr>>JMPTABL_START3)&JMPTABLE_MASK3;
    const uintptr_t idx2 = (addr>>JMPTABL_START2)&JMPTABLE_MASK2;
    const uintptr_t idx1 = (addr>>JMPTABL_START1)&JMPTABLE_MASK1;
    uintptr_t idx0 = addr&JMPTABLE_MASK0;
    #ifdef JMPTABL_START4
    *db = *(dynablock_t**)(box64_jmptbl4[idx4][idx3][idx2][idx1][idx0]- sizeof(void*));
    #else
    *db = *(dynablock_t**)(box64_jmptbl3[idx3][idx2][idx1][idx0]- sizeof(void*));
    #endif
    if(*db)
        return addr+1;
    #ifdef JMPTABL_START4
    if(box64_jmptbl4[idx4] == box64_jmptbldefault3)
        return ((idx4+1)<<JMPTABL_START4);
    if(box64_jmptbl4[idx4][idx3] == box64_jmptbldefault2)
        return (((addr>>JMPTABL_START3)+1)<<JMPTABL_START3);
    if(box64_jmptbl4[idx4][idx3][idx2] == box64_jmptbldefault1)
        return (((addr>>JMPTABL_START2)+1)<<JMPTABL_START2);
    uintptr_t* block = box64_jmptbl4[idx4][idx3][idx2][idx1];
    #else
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return ((idx3+1)<<JMPTABL_START3);
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return (((addr>>JMPTABL_START2)+1)<<JMPTABL_START2);
    uintptr_t* block = box64_jmptbl3[idx3][idx2][idx1];
    #endif
    if(block == box64_jmptbldefault0)
        return (((addr>>JMPTABL_START1)+1)<<JMPTABL_START1);
    maxsize+=idx0;  // need to adjust maxsize to "end in current block"
    if (maxsize>JMPTABLE_MASK0)
        maxsize = JMPTABLE_MASK0;
    while(block[idx0]==(uintptr_t)native_next) {
        ++idx0;
        if(idx0>maxsize)
            return (addr&~JMPTABLE_MASK0)+idx0;
    }
    *db = *(dynablock_t**)(block[idx0]- sizeof(void*));
    return (addr&~JMPTABLE_MASK0)+idx0+1;
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
    dynablock_t* db = NULL;
    uintptr_t end = addr+size;
    while (start_addr<end) {
        start_addr = getDBSize(start_addr, end-start_addr, &db);
        if(db) {
            if(destroy)
                FreeRangeDynablock(db, addr, size);
            else
                MarkRangeDynablock(db, addr, size);
        }
    }
}

// Will return 1 if at least 1 db in the address range
int isDBFromAddressRange(uintptr_t addr, size_t size)
{
    uintptr_t start_addr = my_context?((addr<my_context->max_db_size)?0:(addr-my_context->max_db_size)):addr;
    dynarec_log(LOG_DEBUG, "isDBFromAddressRange %p/%p -> %p => ", (void*)addr, (void*)start_addr, (void*)(addr+size-1));
    dynablock_t* db = NULL;
    uintptr_t end = addr+size;
    while (start_addr<end) {
        start_addr = getDBSize(start_addr, end-start_addr, &db);
        if(db) {
            dynarec_log(LOG_DEBUG, "1\n");
            return 1;
        }
    }
    dynarec_log(LOG_DEBUG, "0\n");
    return 0;
}


#ifdef JMPTABL_SHIFT4
static uintptr_t *create_jmptbl(uintptr_t idx0, uintptr_t idx1, uintptr_t idx2, uintptr_t idx3, uintptr_t idx4)
{
    if(box64_jmptbl4[idx4] == box64_jmptbldefault3) {
        uintptr_t**** tbl = (uintptr_t****)customMalloc((1<<JMPTABL_SHIFT3)*sizeof(uintptr_t***));
        for(int i=0; i<(1<<JMPTABL_SHIFT3); ++i)
            tbl[i] = box64_jmptbldefault2;
        if(native_lock_storeifref(&box64_jmptbl4[idx4], tbl, box64_jmptbldefault3)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT3)*sizeof(uintptr_t***);
            ++jmptbl_allocated4;
        }
#endif
    }
    if(box64_jmptbl4[idx4][idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)customMalloc((1<<JMPTABL_SHIFT2)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT2); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(native_lock_storeifref(&box64_jmptbl4[idx4][idx3], tbl, box64_jmptbldefault2)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT2)*sizeof(uintptr_t**);
            ++jmptbl_allocated3;
        }
#endif
    }
    if(box64_jmptbl4[idx4][idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)customMalloc((1<<JMPTABL_SHIFT1)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT1); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(native_lock_storeifref(&box64_jmptbl4[idx4][idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT1)*sizeof(uintptr_t*);
            ++jmptbl_allocated2;
        }
#endif
    }
    if(box64_jmptbl4[idx4][idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)customMalloc((1<<JMPTABL_SHIFT0)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT0); ++i)
            tbl[i] = (uintptr_t)native_next;
        if(native_lock_storeifref(&box64_jmptbl4[idx4][idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT0)*sizeof(uintptr_t);
            ++jmptbl_allocated1;
        }
#endif
    }
    return &box64_jmptbl4[idx4][idx3][idx2][idx1][idx0];
}
#else
static uintptr_t *create_jmptbl(uintptr_t idx0, uintptr_t idx1, uintptr_t idx2, uintptr_t idx3)
{
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2) {
        uintptr_t*** tbl = (uintptr_t***)customMalloc((1<<JMPTABL_SHIFT2)*sizeof(uintptr_t**));
        for(int i=0; i<(1<<JMPTABL_SHIFT2); ++i)
            tbl[i] = box64_jmptbldefault1;
        if(native_lock_storeifref(&box64_jmptbl3[idx3], tbl, box64_jmptbldefault2)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT2)*sizeof(uintptr_t**);
            ++jmptbl_allocated3;
        }
#endif
    }
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) {
        uintptr_t** tbl = (uintptr_t**)customMalloc((1<<JMPTABL_SHIFT1)*sizeof(uintptr_t*));
        for(int i=0; i<(1<<JMPTABL_SHIFT1); ++i)
            tbl[i] = box64_jmptbldefault0;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2], tbl, box64_jmptbldefault1)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT1)*sizeof(uintptr_t*);
            ++jmptbl_allocated2;
        }
#endif
    }
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) {
        uintptr_t* tbl = (uintptr_t*)customMalloc((1<<JMPTABL_SHIFT0)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT0); ++i)
            tbl[i] = (uintptr_t)native_next;
        if(native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1], tbl, box64_jmptbldefault0)!=tbl)
            customFree(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT0)*sizeof(uintptr_t);
            ++jmptbl_allocated1;
        }
#endif
    }
    return &box64_jmptbl3[idx3][idx2][idx1][idx0];
}
#endif

int addJumpTableIfDefault64(void* addr, void* jmp)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4;
    idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    idx3 = (((uintptr_t)addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    idx2 = (((uintptr_t)addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    idx1 = (((uintptr_t)addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    idx0 = (((uintptr_t)addr)                )&JMPTABLE_MASK0;

    #ifdef JMPTABL_SHIFT4
    return (native_lock_storeifref(create_jmptbl(idx0, idx1, idx2, idx3, idx4), jmp, native_next)==jmp)?1:0;
    #else
    return (native_lock_storeifref(create_jmptbl(idx0, idx1, idx2, idx3), jmp, native_next)==jmp)?1:0;
    #endif
}
void setJumpTableDefault64(void* addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4;
    idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    if(box64_jmptbl4[idx4] == box64_jmptbldefault3)
        return;
    uintptr_t ****box64_jmptbl3 = box64_jmptbl4[idx4];
    #endif
    idx3 = (((uintptr_t)addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return;
    idx2 = (((uintptr_t)addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return;
    idx1 = (((uintptr_t)addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return;
    idx0 = (((uintptr_t)addr)    )&JMPTABLE_MASK0;
    native_lock_store_dd(&box64_jmptbl3[idx3][idx2][idx1][idx0], (uintptr_t)native_next);
}
void setJumpTableDefaultRef64(void* addr, void* jmp)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4;
    idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    if(box64_jmptbl4[idx4] == box64_jmptbldefault3)
        return;
    uintptr_t ****box64_jmptbl3 = box64_jmptbl4[idx4];
    #endif
    idx3 = (((uintptr_t)addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return;
    idx2 = (((uintptr_t)addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return;
    idx1 = (((uintptr_t)addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return;
    idx0 = (((uintptr_t)addr)    )&JMPTABLE_MASK0;
    native_lock_storeifref(&box64_jmptbl3[idx3][idx2][idx1][idx0], native_next, jmp);
}
int setJumpTableIfRef64(void* addr, void* jmp, void* ref)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    idx3 = (((uintptr_t)addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    idx2 = (((uintptr_t)addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    idx1 = (((uintptr_t)addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    idx0 = (((uintptr_t)addr)    )&JMPTABLE_MASK0;
    #ifdef JMPTABL_SHIFT4
    return (native_lock_storeifref(create_jmptbl(idx0, idx1, idx2, idx3, idx4), jmp, ref)==jmp)?1:0;
    #else
    return (native_lock_storeifref(create_jmptbl(idx0, idx1, idx2, idx3), jmp, ref)==jmp)?1:0;
    #endif
}
int isJumpTableDefault64(void* addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4;
    idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    if(box64_jmptbl4[idx4] == box64_jmptbldefault3)
        return 1;
    uintptr_t ****box64_jmptbl3 = box64_jmptbl4[idx4];
    #endif
    idx3 = (((uintptr_t)addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    if(box64_jmptbl3[idx3] == box64_jmptbldefault2)
        return 1;
    idx2 = (((uintptr_t)addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    if(box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1)
        return 1;
    idx1 = (((uintptr_t)addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    if(box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0)
        return 1;
    idx0 = (((uintptr_t)addr)    )&JMPTABLE_MASK0;
    return (box64_jmptbl3[idx3][idx2][idx1][idx0]==(uintptr_t)native_next)?1:0;
}
uintptr_t getJumpTable64()
{
    #ifdef JMPTABL_SHIFT4
    return (uintptr_t)box64_jmptbl4;
    #else
    return (uintptr_t)box64_jmptbl3;
    #endif
}

uintptr_t getJumpTable32()
{
    #ifdef JMPTABL_SHIFT4
    return (uintptr_t)box64_jmptbl4[0][0];
    #else
    return (uintptr_t)box64_jmptbl3[0];
    #endif
}

uintptr_t getJumpTableAddress64(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    idx3 = ((addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    idx2 = ((addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    idx1 = ((addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    idx0 = ((addr)                )&JMPTABLE_MASK0;
    #ifdef JMPTABL_SHIFT4
    return (uintptr_t)create_jmptbl(idx0, idx1, idx2, idx3, idx4);
    #else
    return (uintptr_t)create_jmptbl(idx0, idx1, idx2, idx3);
    #endif
}

dynablock_t* getDB(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    idx3 = ((addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    idx2 = ((addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    idx1 = ((addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    idx0 = ((addr)                )&JMPTABLE_MASK0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t ret = (uintptr_t)box64_jmptbl4[idx4][idx3][idx2][idx1][idx0];
    #else
    uintptr_t ret = (uintptr_t)box64_jmptbl3[idx3][idx2][idx1][idx0];
    #endif

    return *(dynablock_t**)(ret - sizeof(void*));
}

int getNeedTest(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    idx3 = ((addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    idx2 = ((addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    idx1 = ((addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    idx0 = ((addr)                )&JMPTABLE_MASK0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t ret = (uintptr_t)box64_jmptbl4[idx4][idx3][idx2][idx1][idx0];
    #else
    uintptr_t ret = (uintptr_t)box64_jmptbl3[idx3][idx2][idx1][idx0];
    #endif
    dynablock_t* db = *(dynablock_t**)(ret - sizeof(void*));
    return db?((ret!=(uintptr_t)db->block)?1:0):0;
}

uintptr_t getJumpAddress64(uintptr_t addr)
{
    uintptr_t idx3, idx2, idx1, idx0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t idx4 = (((uintptr_t)addr)>>JMPTABL_START4)&JMPTABLE_MASK4;
    #endif
    idx3 = ((addr)>>JMPTABL_START3)&JMPTABLE_MASK3;
    idx2 = ((addr)>>JMPTABL_START2)&JMPTABLE_MASK2;
    idx1 = ((addr)>>JMPTABL_START1)&JMPTABLE_MASK1;
    idx0 = ((addr)                )&JMPTABLE_MASK0;
    #ifdef JMPTABL_SHIFT4
    return (uintptr_t)box64_jmptbl4[idx4][idx3][idx2][idx1][idx0];
    #else
    return (uintptr_t)box64_jmptbl3[idx3][idx2][idx1][idx0];
    #endif
}

// Remove the Write flag from an adress range, so DB can be executed safely
void protectDBJumpTable(uintptr_t addr, size_t size, void* jump, void* ref)
{
    dynarec_log(LOG_DEBUG, "protectDBJumpTable %p -> %p\n", (void*)addr, (void*)(addr+size-1));

    uintptr_t cur = addr&~(box64_pagesize-1);
    uintptr_t end = ALIGN(addr+size);

    LOCK_PROT();
    while(cur!=end) {
        uint32_t prot = 0, oprot;
        uintptr_t bend = 0;
        rb_get_end(memprot, cur, &prot, &bend);
        if(bend>end)
            bend = end;
        oprot = prot;
        uint32_t dyn = prot&PROT_DYN;
        if(!prot)
            prot = PROT_READ | PROT_WRITE | PROT_EXEC;
        if(!(dyn&PROT_NEVERPROT)) {
            prot&=~PROT_CUSTOM;
            if(prot&PROT_WRITE) {
                if(!dyn) 
                    mprotect((void*)cur, bend-cur, prot&~PROT_WRITE);
                prot |= PROT_DYNAREC;
            } else 
                prot |= PROT_DYNAREC_R;
        }
        if (prot != oprot) // If the node doesn't exist, then prot != 0
            rb_set(memprot, cur, bend, prot);
        cur = bend;
    }
    if(jump)
        setJumpTableIfRef64((void*)addr, jump, ref);
    UNLOCK_PROT();
}

// Remove the Write flag from an adress range, so DB can be executed safely
void protectDB(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "protectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));

    uintptr_t cur = addr&~(box64_pagesize-1);
    uintptr_t end = ALIGN(addr+size);

    LOCK_PROT();
    while(cur!=end) {
        uint32_t prot = 0, oprot;
        uintptr_t bend = 0;
        rb_get_end(memprot, cur, &prot, &bend);
        if(bend>end)
            bend = end;
        oprot = prot;
        uint32_t dyn = prot&PROT_DYN;
        if(!prot)
            prot = PROT_READ | PROT_WRITE | PROT_EXEC;
        if(!(dyn&PROT_NEVERPROT)) {
            prot&=~PROT_CUSTOM;
            if(prot&PROT_WRITE) {
                if(!dyn) 
                    mprotect((void*)cur, bend-cur, prot&~PROT_WRITE);
                prot |= PROT_DYNAREC;
            } else 
                prot |= PROT_DYNAREC_R;
        }
        if (prot != oprot) // If the node doesn't exist, then prot != 0
            rb_set(memprot, cur, bend, prot);
        cur = bend;
    }
    UNLOCK_PROT();
}

// Add the Write flag from an adress range, and mark all block as dirty
void unprotectDB(uintptr_t addr, size_t size, int mark)
{
    dynarec_log(LOG_DEBUG, "unprotectDB %p -> %p (mark=%d)\n", (void*)addr, (void*)(addr+size-1), mark);

    uintptr_t cur = addr&~(box64_pagesize-1);
    uintptr_t end = ALIGN(addr+size);

    LOCK_PROT();
    while(cur!=end) {
        uint32_t prot = 0, oprot;
        uintptr_t bend = 0;
        if (!rb_get_end(memprot, cur, &prot, &bend)) {
            if(bend>=end) break;
            else {
                cur = bend;
                continue;
            }
        }
        oprot = prot;
        if(bend>end)
            bend = end;
        if(!(prot&PROT_NEVERPROT)) {
            if(prot&PROT_DYNAREC) {
                prot&=~PROT_DYN;
                if(mark)
                    cleanDBFromAddressRange(cur, bend-cur, 0);
                mprotect((void*)cur, bend-cur, prot);
            } else if(prot&PROT_DYNAREC_R)
                prot &= ~PROT_CUSTOM;
        }
        if (prot != oprot)
            rb_set(memprot, cur, bend, prot);
        cur = bend;
    }
    UNLOCK_PROT();
}

int isprotectedDB(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "isprotectedDB %p -> %p => ", (void*)addr, (void*)(addr+size-1));
    addr &=~(box64_pagesize-1);
    uintptr_t end = ALIGN(addr+size);
    LOCK_PROT_READ();
    while (addr < end) {
        uint32_t prot;
        uintptr_t bend;
        if (!rb_get_end(memprot, addr, &prot, &bend) || !(prot&PROT_DYN)) {
            dynarec_log(LOG_DEBUG, "0\n");
            UNLOCK_PROT_READ();
            return 0;
        } else {
            addr = bend;
        }
    }
    UNLOCK_PROT_READ();
    dynarec_log(LOG_DEBUG, "1\n");
    return 1;
}

uintptr_t hotpage = 0;
int hotpage_cnt = 0;
#define HOTPAGE_MARK 64
void SetHotPage(uintptr_t addr)
{
    hotpage = addr&~(box64_pagesize-1);
    hotpage_cnt = HOTPAGE_MARK;
}
int isInHotPage(uintptr_t addr)
{
    if(!hotpage_cnt)
        return 0;
    --hotpage_cnt;
    return (addr>=hotpage) && (addr<hotpage+box64_pagesize);
}
int checkInHotPage(uintptr_t addr)
{
    return hotpage_cnt && (addr>=hotpage) && (addr<hotpage+box64_pagesize);
}


#endif

void updateProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "updateProtection %p:%p 0x%hhx\n", (void*)addr, (void*)(addr+size-1), prot);
    LOCK_PROT();
    uintptr_t cur = addr & ~(box64_pagesize-1);
    uintptr_t end = ALIGN(cur+size);
    rb_set(mapallmem, cur, cur+size, 1);
    while (cur < end) {
        uintptr_t bend;
        uint32_t oprot;
        rb_get_end(memprot, cur, &oprot, &bend);
        uint32_t dyn=(oprot&PROT_DYN);
        if(!(dyn&PROT_NEVERPROT)) {
            if(dyn && (prot&PROT_WRITE)) {   // need to remove the write protection from this block
                dyn = PROT_DYNAREC;
                mprotect((void*)cur, bend-cur, prot&~PROT_WRITE);
            } else if(dyn && !(prot&PROT_WRITE)) {
                dyn = PROT_DYNAREC_R;
            }
        }
        if ((prot|dyn) != oprot)
            rb_set(memprot, cur, bend, prot|dyn);
        cur = bend;
    }
    UNLOCK_PROT();
}

void setProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    size = ALIGN(size);
    LOCK_PROT();
    uintptr_t cur = addr & ~(box64_pagesize-1);
    uintptr_t end = ALIGN(cur+size);
    rb_set(mapallmem, cur, end, 1);
    rb_set(memprot, cur, end, prot);
    UNLOCK_PROT();
}

void setProtection_mmap(uintptr_t addr, size_t size, uint32_t prot)
{
    if(!size)
        return;
    addr &= ~(box64_pagesize-1);
    size = ALIGN(size);
    LOCK_PROT();
    rb_set(mmapmem, addr, addr+size, 1);
    if(!prot)
        rb_set(mapallmem, addr, addr+size, 1);
    UNLOCK_PROT();
    if(prot)
        setProtection(addr, size, prot);
}

void setProtection_elf(uintptr_t addr, size_t size, uint32_t prot)
{
    size = ALIGN(size);
    addr &= ~(box64_pagesize-1);
    if(prot)
        setProtection(addr, size, prot);
    else {
        LOCK_PROT();
        rb_set(mapallmem, addr, addr+size, 1);
        UNLOCK_PROT();
    }
}

void refreshProtection(uintptr_t addr)
{
    LOCK_PROT();
    uint32_t prot;
    uintptr_t bend;
    if (rb_get_end(memprot, addr, &prot, &bend)) {
        int ret = mprotect((void*)(addr&~(box64_pagesize-1)), box64_pagesize, prot&~PROT_CUSTOM);
        dynarec_log(LOG_DEBUG, "refreshProtection(%p): %p/0x%x (ret=%d/%s)\n", (void*)addr, (void*)(addr&~(box64_pagesize-1)), prot, ret, ret?strerror(errno):"ok");
    }
    UNLOCK_PROT();
}

void allocProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "allocProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    size = ALIGN(size);
    addr &= ~(box64_pagesize-1);
    LOCK_PROT();
    rb_set(mapallmem, addr, addr+size, 1);
    UNLOCK_PROT();
    // don't need to add precise tracking probably
}

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
            if(s>0x7fff00000000LL)
                have48bits = 1;
        }
    }
    static int shown48bits = 0;
    if(!shown48bits) {
        shown48bits = 1;
        if(have48bits)
            printf_log(LOG_INFO, "BOX64: Detected 48bits at least of address space\n");
        else
            printf_log(LOG_INFO, "BOX64: Didn't detect 48bits of address space, considering it's 39bits\n");
    }
    fclose(f);
    box64_mapclean = 1;
}

void freeProtection(uintptr_t addr, size_t size)
{
    size = ALIGN(size);
    addr &= ~(box64_pagesize-1);
    dynarec_log(LOG_DEBUG, "freeProtection %p:%p\n", (void*)addr, (void*)(addr+size-1));
    LOCK_PROT();
    rb_unset(mapallmem, addr, addr+size);
    rb_unset(mmapmem, addr, addr+size);
    rb_unset(memprot, addr, addr+size);
    UNLOCK_PROT();
}

uint32_t getProtection(uintptr_t addr)
{
    LOCK_PROT_READ();
    uint32_t ret = rb_get(memprot, addr);
    UNLOCK_PROT_READ();
    return ret;
}

int getMmapped(uintptr_t addr)
{
    return rb_get(mmapmem, addr);
}

#define LOWEST (void*)0x10000
#define MEDIUM (void*)0x40000000
#define HIGH   (void*)0x60000000

void* find31bitBlockNearHint(void* hint, size_t size, uintptr_t mask)
{
    uint32_t prot;
    if(hint<LOWEST) hint = LOWEST;
    uintptr_t bend = 0;
    uintptr_t cur = (uintptr_t)hint;
    if(!mask) mask = 0xffff;
    while(bend<0xc0000000LL) {
        if(!rb_get_end(mapallmem, cur, &prot, &bend)) {
            if(bend-cur>=size)
                return (void*)cur;
        }
        // granularity 0x10000
        cur = (bend+mask)&~mask;
    }
    return NULL;
}

void* find32bitBlock(size_t size)
{
    void* ret = find31bitBlockNearHint(MEDIUM, size, 0);
    if(ret)
        return ret;
    ret = find31bitBlockNearHint(LOWEST, size, 0);
    return ret?ret:find47bitBlock(size);
}
void* find47bitBlock(size_t size)
{
    void* ret = find47bitBlockNearHint(HIGH, size, 0);
    if(!ret)
        ret = find32bitBlock(size);
    return ret;
}
void* find47bitBlockNearHint(void* hint, size_t size, uintptr_t mask)
{
    uint32_t prot;
    if(hint<LOWEST) hint = LOWEST;
    uintptr_t bend = 0;
    uintptr_t cur = (uintptr_t)hint;
    if(!mask) mask = 0xffff;
    while(bend<0x800000000000LL) {
        if(!rb_get_end(mapallmem, cur, &prot, &bend)) {
            if(bend-cur>=size)
                return (void*)cur;
        }
        // granularity 0x10000
        cur = (bend+mask)&~mask;
    }
    return NULL;
}
void* find47bitBlockElf(size_t size, int mainbin, uintptr_t mask)
{
    static void* startingpoint = NULL;
    if(!startingpoint) {
        startingpoint = (void*)(have48bits?0x7fff00000000LL:0x3f00000000LL);
    }
    void* mainaddr = (void*)0x100000000LL;
    void* ret = find47bitBlockNearHint(mainbin?mainaddr:startingpoint, size, mask);
    if(!ret)
        ret = find31bitBlockNearHint(MEDIUM, size, mask);
    if(!ret)
        ret = find31bitBlockNearHint(LOWEST, size, mask);
    if(!mainbin)
        startingpoint = (void*)(((uintptr_t)startingpoint+size+0x1000000LL)&~0xffffffLL);
    return ret;
}

int isBlockFree(void* hint, size_t size)
{
    uint32_t prot;
    uintptr_t bend = 0;
    uintptr_t cur = (uintptr_t)hint;
    if(!rb_get_end(mapallmem, cur, &prot, &bend)) {
        if(bend-cur>=size)
            return 1;
    }
    return 0;
}

int unlockCustommemMutex()
{
    int ret = 0;
    int i = 0;
    #ifdef USE_CUSTOM_MUTEX
    uint32_t tid = (uint32_t)GetTID();
    #define GO(A, B)                    \
        i = (native_lock_storeifref2_d(&A, 0, tid)==tid); \
        if(i) {                         \
            ret|=(1<<B);                \
        }
    #else
    #define GO(A, B)                    \
        i = checkUnlockMutex(&A);       \
        if(i) {                         \
            ret|=(1<<B);                \
        }
    #endif
    GO(mutex_blocks, 0)
    GO(mutex_prot, 1) // See also signals.c
    #undef GO
    return ret;
}

void relockCustommemMutex(int locks)
{
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            mutex_trylock(&A);          \

    GO(mutex_blocks, 0)
    GO(mutex_prot, 1) // See also signals.c
    #undef GO
}

static void init_mutexes(void)
{
    #ifdef USE_CUSTOM_MUTEX
    native_lock_store(&mutex_blocks, 0);
    native_lock_store(&mutex_prot, 0);
    #else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex_blocks, &attr);
    pthread_mutex_init(&mutex_prot, &attr);

    pthread_mutexattr_destroy(&attr);
    #endif
}

static void atfork_child_custommem(void)
{
    // (re)init mutex if it was lock before the fork
    init_mutexes();
}

void reserveHighMem()
{
    char* p = getenv("BOX64_RESERVE_HIGH");
    #if 0//def ADLINK
    if(p && p[0]=='0')
    #else
    if(!p || p[0]=='0')
    #endif
        return; // don't reserve by default
    uintptr_t cur = 1ULL<<47;
    uintptr_t bend = 0;
    uint32_t prot;
    while (bend!=0xffffffffffffffffLL) {
        if(!rb_get_end(mapallmem, cur, &prot, &bend)) {
            void* ret = internal_mmap((void*)cur, bend-cur, 0, MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
            printf_log(LOG_DEBUG, "Reserve %p-%p => %p (%s)\n", (void*)cur, bend, ret, strerror(errno));
            printf_log(LOG_DEBUG, "mmap %p-%p\n", cur, bend);
            if(ret!=(void*)-1) {
                rb_set(mapallmem, cur, bend, 1);
            }
        }
        cur = bend;
    }
}

void init_custommem_helper(box64context_t* ctx)
{
    (void)ctx;
    if(inited) // already initialized
        return;
    inited = 1;
    memprot = init_rbtree();
    sigfillset(&critical_prot);
    init_mutexes();
#ifdef DYNAREC
    if(box64_dynarec) {
        #ifdef JMPTABL_SHIFT4
        for(int i=0; i<(1<<JMPTABL_SHIFT4); ++i)
            box64_jmptbl4[i] = box64_jmptbldefault3;
        for(int i=0; i<(1<<JMPTABL_SHIFT3); ++i)
            box64_jmptbldefault3[i] = box64_jmptbldefault2;
        #else
        for(int i=0; i<(1<<JMPTABL_SHIFT3); ++i)
            box64_jmptbl3[i] = box64_jmptbldefault2;
        #endif
        for(int i=0; i<(1<<JMPTABL_SHIFT2); ++i)
            box64_jmptbldefault2[i] = box64_jmptbldefault1;
        for(int i=0; i<(1<<JMPTABL_SHIFT1); ++i)
            box64_jmptbldefault1[i] = box64_jmptbldefault0;
        for(int i=0; i<(1<<JMPTABL_SHIFT0); ++i)
            box64_jmptbldefault0[i] = (uintptr_t)native_next;
    }
    lockaddress = kh_init(lockaddress);
#endif
    pthread_atfork(NULL, NULL, atfork_child_custommem);
    // init mapallmem list
    mapallmem = init_rbtree();
    // init mmapmem list
    mmapmem = init_rbtree();
    // Load current MMap
    loadProtectionFromMap();
    reserveHighMem();
}

void fini_custommem_helper(box64context_t *ctx)
{
    (void)ctx;
#ifdef TRACE_MEMSTAT
    uintptr_t njmps = 0, njmps_in_lv1_max = 0;
    #ifdef JMPTABL_SHIFT4
    uintptr_t**** box64_jmptbl3;
    for(uintptr_t idx4 = 0; idx4 < (1<< JMPTABL_SHIFT4); ++idx4) {    
        if (box64_jmptbl4[idx4] == box64_jmptbldefault3) continue;
        box64_jmptbl3 = box64_jmptbl4[idx4];
    #endif
    for (uintptr_t idx3 = 0; idx3 < (1 << JMPTABL_SHIFT3); ++idx3) {
        if (box64_jmptbl3[idx3] == box64_jmptbldefault2) continue;
        for (uintptr_t idx2 = 0; idx2 < (1 << JMPTABL_SHIFT2); ++idx2) {
            if (box64_jmptbl3[idx3][idx2] == box64_jmptbldefault1) continue;
            for (uintptr_t idx1 = 0; idx1 < (1 << JMPTABL_SHIFT1); ++idx1) {
                if (box64_jmptbl3[idx3][idx2][idx1] == box64_jmptbldefault0) continue;
                uintptr_t njmps_in_cur_lv1 = 0;
                for (uintptr_t idx0 = 0; idx0 < (1 << JMPTABL_SHIFT0); ++idx0) {
                    if (box64_jmptbl3[idx3][idx2][idx1][idx0] == (uintptr_t)native_next) continue;
                    ++njmps;
                    ++njmps_in_cur_lv1;
                }
                if (njmps_in_cur_lv1 > njmps_in_lv1_max) njmps_in_lv1_max = njmps_in_cur_lv1;
            }
        }
    }
    #ifdef JMPTABL_SHIFT4
    }
    if(box64_log) printf("Allocation:\n- dynarec: %lld kio\n- customMalloc: %lld kio\n- memprot: %lld kio (peak at %lld kio)\n- jump table: %lld kio (%lld level 4, %lld level 3, %lld level 2, %lld level 1 table allocated, for %lld jumps, with at most %lld per level 1)\n", dynarec_allocated / 1024, customMalloc_allocated / 1024, memprot_allocated / 1024, memprot_max_allocated / 1024, jmptbl_allocated / 1024, jmptbl_allocated4, jmptbl_allocated3, jmptbl_allocated2, jmptbl_allocated1, njmps, njmps_in_lv1_max);
    #else
    if(box64_log) printf("Allocation:\n- dynarec: %lld kio\n- customMalloc: %lld kio\n- memprot: %lld kio (peak at %lld kio)\n- jump table: %lld kio (%lld level 3, %lld level 2, %lld level 1 table allocated, for %lld jumps, with at most %lld per level 1)\n", dynarec_allocated / 1024, customMalloc_allocated / 1024, memprot_allocated / 1024, memprot_max_allocated / 1024, jmptbl_allocated / 1024, jmptbl_allocated3, jmptbl_allocated2, jmptbl_allocated1, njmps, njmps_in_lv1_max);
    #endif
#endif
    if(!inited)
        return;
    inited = 0;
#ifdef DYNAREC
    if(box64_dynarec) {
        dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
        mmaplist_t* head = mmaplist;
        mmaplist = NULL;
        while(head) {
            for (int i=0; i<NCHUNK; ++i) {
                if(head->chunks[i].block)
                    #ifdef USE_MMAP
                    internal_munmap(head->chunks[i].block, head->chunks[i].size);
                    #else
                    box_free(head->chunks[i].block);
                    #endif
            }
            mmaplist_t *old = head;
            head = head->next;
            free(old);
        }

        box_free(mmaplist);
        #ifdef JMPTABL_SHIFT4
        uintptr_t**** box64_jmptbl3;
        for(int i4 = 0; i4 < (1<< JMPTABL_SHIFT4); ++i4)
            if (box64_jmptbl4[i4] != box64_jmptbldefault3) {
            box64_jmptbl3 = box64_jmptbl4[i4];
        #endif
        for (int i3=0; i3<(1<<JMPTABL_SHIFT3); ++i3)
            if(box64_jmptbl3[i3]!=box64_jmptbldefault2) {
                for (int i2=0; i2<(1<<JMPTABL_SHIFT2); ++i2)
                    if(box64_jmptbl3[i3][i2]!=box64_jmptbldefault1) {
                        for (int i1=0; i1<(1<<JMPTABL_SHIFT1); ++i1)
                            if(box64_jmptbl3[i3][i2][i1]!=box64_jmptbldefault0) {
                                customFree(box64_jmptbl3[i3][i2][i1]);
                            }
                        customFree(box64_jmptbl3[i3][i2]);
                    }
                customFree(box64_jmptbl3[i3]);
            }
        #ifdef JMPTABL_SHIFT4
                customFree(box64_jmptbl4[i4]);
            }
        #endif
    }
    kh_destroy(lockaddress, lockaddress);
    lockaddress = NULL;
#endif
    delete_rbtree(memprot);
    memprot = NULL;
    delete_rbtree(mmapmem);
    mmapmem = NULL;
    delete_rbtree(mapallmem);
    mapallmem = NULL;

    for(int i=0; i<n_blocks; ++i)
        #ifdef USE_MMAP
        internal_munmap(p_blocks[i].block, p_blocks[i].size);
        #else
        box_free(p_blocks[i].block);
        #endif
    box_free(p_blocks);
    #ifndef USE_CUSTOM_MUTEX
    pthread_mutex_destroy(&mutex_prot);
    pthread_mutex_destroy(&mutex_blocks);
    #endif
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

void* internal_mmap(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset)
{
    #if 1//def STATICBUILD
    void* ret = (void*)syscall(__NR_mmap, addr, length, prot, flags, fd, offset);
    #else
    static int grab = 1;
    typedef void*(*pFpLiiiL_t)(void*, unsigned long, int, int, int, size_t);
    static pFpLiiiL_t libc_mmap64 = NULL;
    if(grab) {
        libc_mmap64 = dlsym(RTLD_NEXT, "mmap64");
    }
    void* ret = libc_mmap64(addr, length, prot, flags, fd, offset);
    #endif
    return ret;
}
int internal_munmap(void* addr, unsigned long length)
{
    #if 1//def STATICBUILD
    int ret = syscall(__NR_munmap, addr, length);
    #else
    static int grab = 1;
    typedef int(*iFpL_t)(void*, unsigned long);
    static iFpL_t libc_munmap = NULL;
    if(grab) {
        libc_munmap = dlsym(RTLD_NEXT, "munmap");
    }
    int ret = libc_munmap(addr, length);
    #endif
    return ret;
}

void* my_mmap64(x64emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset);

extern int running32bits;
EXPORT void* mmap64(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset)
{
    void* ret;
    if(!addr && ((running32bits && box64_mmap32) || (flags&0x40)))
        ret = my_mmap64(NULL, addr, length, prot, flags | 0x40, fd, offset);
    else
        ret = internal_mmap(addr, length, prot, flags, fd, offset);
    if(ret!=MAP_FAILED && mapallmem)
        setProtection((uintptr_t)ret, length, prot);
    return ret;
}
EXPORT void* mmap(void *addr, unsigned long length, int prot, int flags, int fd, ssize_t offset) __attribute__((alias("mmap64")));

EXPORT int munmap(void* addr, unsigned long length)
{
    int ret = internal_munmap(addr, length);
    if(!ret && mapallmem) {
        freeProtection((uintptr_t)addr, length);
    }
    return ret;
}
