#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "os.h"
#include "backtrace.h"
#include "box64context.h"
#include "debug.h"
#include "x64trace.h"
#include "x64emu.h"
#include "librarian.h"
#include "bridge.h"
#include "library.h"
#include "callback.h"
#include "x64trace.h"
#include "custommem.h"
#include "khash.h"
#include "threads.h"
#include "rbtree.h"
#include "mysignal.h"
#include "mypthread.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynarec/dynablock_private.h"
#include "dynarec/native_lock.h"
#include "dynarec/dynarec_next.h"

// init inside dynablocks.c
static mmaplist_t          *mmaplist = NULL;
static rbtree_t            *rbt_dynmem = NULL;
static uint64_t jmptbl_allocated = 0, jmptbl_allocated1 = 0, jmptbl_allocated2 = 0, jmptbl_allocated3 = 0;
#if JMPTABL_SHIFTMAX != 16
#error Incorect value for jumptable shift max that should be 16
#endif
#ifdef JMPTABL_SHIFT4
#if JMPTABL_SHIFT3 != 16
#error Incorect value for jumptable shift3 that should be 16
#endif
static uint64_t jmptbl_allocated4 = 0;
static uintptr_t****       box64_jmptbl4[1<<JMPTABL_SHIFT4];
static uintptr_t***        box64_jmptbldefault3[1<<JMPTABL_SHIFT3];
static uintptr_t***        box64_jmptbl_48[1<<JMPTABL_SHIFT3];
#else
#if JMPTABL_SHIFT2 != 16
#error Incorect value for jumptable shift2 that should be 16
#endif
static uintptr_t***        box64_jmptbl3[1<<JMPTABL_SHIFT3];
static uintptr_t**         box64_jmptbl_48[1<<JMPTABL_SHIFT2];
#endif
static uintptr_t**         box64_jmptbldefault2[1<<JMPTABL_SHIFT2];
static uintptr_t*          box64_jmptbldefault1[1<<JMPTABL_SHIFT1];
static uintptr_t           box64_jmptbldefault0[1<<JMPTABL_SHIFT0];
// lock addresses
KHASH_SET_INIT_INT64(lockaddress)
static kh_lockaddress_t    *lockaddress = NULL;
#ifdef USE_CUSTOM_MUTEX
uint32_t            mutex_prot;
uint32_t            mutex_blocks;
#else
pthread_mutex_t     mutex_prot;
pthread_mutex_t     mutex_blocks;
#endif
#else
pthread_mutex_t     mutex_prot;
pthread_mutex_t     mutex_blocks;
#endif
//#define TRACE_MEMSTAT
rbtree_t* memprot = NULL;
int have48bits = 0;
static int inited = 0;
typedef enum {
    MEM_UNUSED = 0,
    MEM_ALLOCATED = 1,
    MEM_RESERVED = 2,
    MEM_MMAP = 3
} mem_flag_t;
rbtree_t*  mapallmem = NULL;
static rbtree_t*  blockstree = NULL;

#define BTYPE_MAP   1
#define BTYPE_LIST  0

typedef struct blocklist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
    void*               first;
    uint32_t            lowest;
    uint8_t             type;
} blocklist_t;

#define MMAPSIZE (512*1024)     // allocate 512kb sized blocks
#define MMAPSIZE128 (128*1024)  // allocate 128kb sized blocks for 128byte map
#define DYNMMAPSZ (2*1024*1024) // allocate 2Mb block for dynarec

static int                 n_blocks = 0;       // number of blocks for custom malloc
static int                 c_blocks = 0;       // capacity of blocks for custom malloc
static blocklist_t*        p_blocks = NULL;    // actual blocks for custom malloc
static int                 setting_prot = 0;

typedef union mark_s {
    struct {
        unsigned int    offs:31;
        unsigned int    fill:1;
    };
    uint32_t            x32;
} mark_t;
typedef struct blockmark_s {
    mark_t  prev;
    mark_t  next;
    uint8_t mark[];
} blockmark_t;

#define NEXT_BLOCK(b) (blockmark_t*)((uintptr_t)(b) + (b)->next.offs)
#define PREV_BLOCK(b) (blockmark_t*)(((uintptr_t)(b) - (b)->prev.offs))
#define LAST_BLOCK(b, s) (blockmark_t*)(((uintptr_t)(b)+(s))-sizeof(blockmark_t))
#define SIZE_BLOCK(b) (((ssize_t)b.offs)-sizeof(blockmark_t))

void printBlock(blockmark_t* b, void* start)
{
    if(!b) return;
    printf_log(LOG_NONE, "========== Block is:\n");
    do {
        printf_log(LOG_NONE, "%c%p, fill=%d, size=0x%x (prev=%d/0x%x)\n", b==start?'*':' ', b, b->next.fill, SIZE_BLOCK(b->next), b->prev.fill, SIZE_BLOCK(b->prev));
        b = NEXT_BLOCK(b);
    } while(b->next.x32);
    printf_log(LOG_NONE, "===================\n");
}

blockmark_t* checkPrevNextCoherent(blockmark_t* b)
{
    while(b->next.x32) {
        blockmark_t* next = NEXT_BLOCK(b);
        if(b->next.x32 != next->prev.x32)
            return next;
        b = next;
    }
    return NULL;
}

// get first subblock free in block. Return NULL if no block, else first subblock free (mark included), filling size
static blockmark_t* getFirstBlock(void* block, size_t maxsize, size_t* size, void* start)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)((start)?start:block);
    while(m->next.x32) {    // while there is a subblock
        if(!m->next.fill && SIZE_BLOCK(m->next)>=maxsize) {
            *size = SIZE_BLOCK(m->next);
            return m;
        }
        m = NEXT_BLOCK(m);
    }

    return NULL;
}

static blockmark_t* getNextFreeBlock(void* block)
{
    blockmark_t *m = (blockmark_t*)block;
    while (m->next.fill) {
         m = NEXT_BLOCK(m);
    };
    return m;
}
static blockmark_t* getPrevFreeBlock(void* block)
{
    blockmark_t *m = (blockmark_t*)block;
    do {
         m = PREV_BLOCK(m);
    } while (m->prev.x32 && m->next.fill);
    return m;
}

static size_t getMaxFreeBlock(void* block, size_t block_size, void* start)
{
    // get start of block
    if(start) {
        blockmark_t *m = (blockmark_t*)start;
        ssize_t maxsize = 0;
        while(m->next.x32) {    // while there is a subblock
            if(!m->next.fill && SIZE_BLOCK(m->next)>maxsize) {
                maxsize = SIZE_BLOCK(m->next);
            }
            m = NEXT_BLOCK(m);
        }
        return maxsize;
    } else {
        blockmark_t *m = LAST_BLOCK(block, block_size); // start with the end
        ssize_t maxsize = 0;
        while(m->prev.x32 && (((uintptr_t)block+maxsize)<(uintptr_t)m)) {    // while there is a subblock
            if(!m->prev.fill && SIZE_BLOCK(m->prev)>maxsize) {
                maxsize = SIZE_BLOCK(m->prev);
            }
            m = PREV_BLOCK(m);
        }
        return maxsize;
    }
}

#define THRESHOLD   (128-1*sizeof(blockmark_t))

static void* createAlignBlock(void* block, void *sub, size_t size)
{
    (void)block;

    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);

    s->next.fill = 0;
    size_t old_size = s->next.offs;
    s->next.offs = size;
    blockmark_t *m = NEXT_BLOCK(s);
    m->prev.x32 = s->next.x32;
    m->next.fill = 0;
    m->next.offs = old_size - size;
    n->prev.x32 = m->next.x32;
    n = m;
    return m;
}
static void* allocBlock(void* block, blockmark_t* sub, size_t size, void** pstart)
{
    (void)block;

    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);

    size+=sizeof(blockmark_t); // count current blockmark
    s->next.fill = 1;
    // check if a new mark is worth it
    if(SIZE_BLOCK(s->next)>size+2*sizeof(blockmark_t)+THRESHOLD) {
        // create a new mark
        size_t old_offs = s->next.offs;
        s->next.offs = size;
        blockmark_t *m = NEXT_BLOCK(s);
        m->prev.x32 = s->next.x32;
        m->next.fill = 0;
        m->next.offs = old_offs - size;
        n->prev.x32 = m->next.x32;
        n = m;
    } else {
        // just fill the blok
        n->prev.fill = 1;
    }

    if(pstart && sub==*pstart) {
        // get the next free block
        while(n->next.fill)
            n = NEXT_BLOCK(n);
        *pstart = (void*)n;
    }
    return sub->mark;
}
static size_t freeBlock(void *block, size_t bsize, blockmark_t* sub, void** pstart)
{
    blockmark_t *m = (blockmark_t*)block;
    blockmark_t *s = sub;
    blockmark_t *n = NEXT_BLOCK(s);
    s->next.fill = 0;
    n->prev.fill = 0;
    // check if merge with next
    while (n->next.x32 && !n->next.fill) {
        blockmark_t *n2 = NEXT_BLOCK(n);
        //remove n
        s->next.offs += n->next.offs;
        n2->prev.offs = s->next.offs;
        n = n2;
    }
    // check if merge with previous
    while (s->prev.x32 && !s->prev.fill) {
        m = PREV_BLOCK(s);
        // remove s...
        m->next.offs += s->next.offs;
        n->prev.offs = m->next.offs;
        s = m;
    }
    if(pstart && (uintptr_t)*pstart>(uintptr_t)s) {
        *pstart = (void*)s;
    }
    // return free size at current block (might be bigger)
    return SIZE_BLOCK(s->next);
}
// return 1 if block has been expanded to new size, 0 if not
static int expandBlock(void* block, blockmark_t* sub, size_t newsize, void** pstart)
{
    (void)block;

    blockmark_t *s = sub;
    blockmark_t *n = NEXT_BLOCK(s);
    int re_first = (pstart && (n==*pstart))?1:0;
    // big enough, nothing to do...
    if(SIZE_BLOCK(s->next)>=newsize)
        return 1;
    if(n->next.fill)
        return 0;   // next block is filled
    // check the total size of the new block (so both offset - blocklist)
    if(((size_t)s->next.offs + n->next.offs - sizeof(blockmark_t)) < newsize)
        return 0;   // free space too short
    // ok, expanding block!
    if((s->next.offs+n->next.offs)-newsize<THRESHOLD+2*sizeof(blockmark_t)) {
        // just remove n
        s->next.offs += n->next.offs;
        blockmark_t *m = NEXT_BLOCK(s);
        m->prev.x32 = s->next.x32;
        n = m;
    } else {
        // remove n and create a new mark (or, move n farther)
        blockmark_t *next = NEXT_BLOCK(n);// thenext of old n
        s->next.offs = newsize + sizeof(blockmark_t);
        blockmark_t *m = NEXT_BLOCK(s);   // this is new n
        m->prev.x32 = s->next.x32;
        // new mark
        m->next.fill = 0;
        m->next.offs = (uintptr_t)next - (uintptr_t)m;
        next->prev.x32 = m->next.x32;
        n = m;
    }
    if(re_first) {
        // get the next free block
        while(n->next.fill)
            n = NEXT_BLOCK(n);
        *pstart = (void*)n;
    }
    return 1;
}
// return size of block
static size_t sizeBlock(void* sub)
{
    blockmark_t *s = (blockmark_t*)sub;
    return SIZE_BLOCK(s->next);
}

static int isBlockChainCoherent(blockmark_t* m, blockmark_t* end)
{
    while(m) {
        if(m>end) return 0;
        if(m==end) return 1;
        m = NEXT_BLOCK(m);
    }
    return 0;
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
    if(!m) {printf_log(LOG_NONE, "Warning, block #%d is NULL\n", i); return 0;}
    // check coherency of the chained list first
    if(!isBlockChainCoherent(m, (blockmark_t*)(p_blocks[i].block+p_blocks[i].size-sizeof(blockmark_t)))) {printf_log(LOG_NONE, "Warning, block #%d chained list is not coherent\n", i); return 0;}
    // check if first is correct
    blockmark_t* first = getNextFreeBlock(m);
    if(p_blocks[i].first && p_blocks[i].first!=first) {printf_log(LOG_NONE, "First %p and stored first %p differs for block %d\n", first, p_blocks[i].first, i); ret = 0;}
    // check if maxfree is correct, with no hint
    size_t maxfree = getMaxFreeBlock(m, p_blocks[i].size, NULL);
    if(maxfree != p_blocks[i].maxfree) {printf_log(LOG_NONE, "Maxfree without hint %zd and stored maxfree %zd differs for block %d\n", maxfree, p_blocks[i].maxfree, i); ret = 0;}
    // check if maxfree from first is correct
    maxfree = getMaxFreeBlock(m, p_blocks[i].size, p_blocks[i].first);
    if(maxfree != p_blocks[i].maxfree) {printf_log(LOG_NONE, "Maxfree with hint %zd and stored maxfree %zd differs for block %d\n", maxfree, p_blocks[i].maxfree, i); ret = 0;}
    // check next/ prev coehrency
    blockmark_t *nope = checkPrevNextCoherent(m);
    if(nope) { printf_log(LOG_NONE, "Next/Prev incoherency for block %d, at %p\n", i, nope); ret = 0;}
    // check chain
    blockmark_t* last = LAST_BLOCK(p_blocks[i].block, p_blocks[i].size);
    while(m->next.x32) {
        blockmark_t* n = NEXT_BLOCK(m);
        if(!m->next.fill && !n->next.fill && n!=last) {
            printf_log(LOG_NONE, "Chain contains 2 subsequent free blocks %p (%d) and %p (%d) for block %d\n", m, SIZE_BLOCK(m->next), n, SIZE_BLOCK(n->next), i);
            ret = 0;
        }
        m = n;
    }
    if(m!=last) {
        printf_log(LOG_NONE, "Last block %p is not the expected last block %p for block %d\n", m, last, i);
        ret = 0;
    }

    return ret;
}

void testAllBlocks()
{
    size_t total = 0;
    size_t fragmented_free = 0;
    size_t max_free = 0;
    size_t total32 = 0;
    size_t fragmented_free32 = 0;
    size_t max_free32 = 0;
    int n_blocks32 = 0;
    for(int i=0; i<n_blocks; ++i) {
        // just silently skip blocks with 0 size, as they are not finished and so might be not coherent
        if(p_blocks[i].size) {
            int is32bits = (box64_is32bits && p_blocks[i].block<(void*)0x100000000LL);
            if(is32bits) ++n_blocks32;
            if((p_blocks[i].type==BTYPE_LIST) && !printBlockCoherent(i))
                printBlock(p_blocks[i].block, p_blocks[i].first);
            total += p_blocks[i].size;
            if(is32bits) total32 += p_blocks[i].size;
            if(p_blocks[i].type==BTYPE_LIST) {
                if(max_free<p_blocks[i].maxfree)
                    max_free = p_blocks[i].maxfree;
                if(is32bits && max_free32<p_blocks[i].maxfree)
                    max_free32 = p_blocks[i].maxfree;
                blockmark_t* m = (blockmark_t*)p_blocks[i].block;
                while(m->next.x32) {
                    if(!m->next.fill)
                        fragmented_free += SIZE_BLOCK(m->next);
                    if(is32bits && !m->next.fill)
                        fragmented_free32 += SIZE_BLOCK(m->next);
                    m = NEXT_BLOCK(m);
                }
            } else {
                if(p_blocks[i].maxfree) {
                    if(max_free<128) max_free=128;
                    if(is32bits && max_free32<128) max_free32=128;
                    fragmented_free += p_blocks[i].maxfree;
                    if(is32bits) fragmented_free32 += p_blocks[i].maxfree;
                }
            }
        }
    }
    printf_log(LOG_NONE, "CustomMem: Total %d blocks, for %zd (0x%zx) allocated memory, max_free %zd (0x%zx), total fragmented free %zd (0x%zx)\n", n_blocks, total, total, max_free, max_free, fragmented_free, fragmented_free);
    if(box64_is32bits)
        printf_log(LOG_NONE, "   32bits: Total %d blocks, for %zd (0x%zx) allocated memory, max_free %zd (0x%zx), total fragmented free %zd (0x%zx)\n", n_blocks32, total32, total32, max_free32, max_free32, fragmented_free32, fragmented_free32);
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

uintptr_t blockstree_start = 0;
uintptr_t blockstree_end = 0;
int blockstree_index = 0;

blocklist_t* findBlock(uintptr_t addr)
{
    if(blockstree) {
        uint32_t i;
        uintptr_t end;
        if(rb_get_end(blockstree, addr, &i, &end))
            return &p_blocks[i];
    } else {
        for(int i=0; i<n_blocks; ++i)
            if((addr>=(uintptr_t)p_blocks[i].block) && (addr<=(uintptr_t)p_blocks[i].block+p_blocks[i].size))
                return &p_blocks[i];
    }
    return NULL;
}
void add_blockstree(uintptr_t start, uintptr_t end, int idx)
{
    if(!blockstree)
        return;
    static int reent = 0;
    if(reent) {
        blockstree_start = start;
        blockstree_end = end;
        blockstree_index = idx;
        return;
    }
    reent = 1;
    blockstree_start = blockstree_end = 0;
    rb_set(blockstree, start, end, idx);
    while(blockstree_start || blockstree_end) {
        start = blockstree_start;
        end = blockstree_end;
        idx = blockstree_index;
        blockstree_start = blockstree_end = 0;
        rb_set(blockstree, start, end, idx);
    }
    reent = 0;
}

void* box32_dynarec_mmap(size_t size);
#ifdef BOX32
int isCustomAddr(void* p)
{
    return findBlock((uintptr_t)p)?1:0;
}
#endif
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
#define LOCK_PROT_FAST()    mutex_lock(&mutex_prot)
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
#define UNLOCK_PROT_FAST()  mutex_unlock(&mutex_prot)


#ifdef TRACE_MEMSTAT
static uint64_t customMalloc_allocated = 0;
#endif
// the BTYPE_MAP is a simple bitmap based allocator: it will allocate slices of 128bytes only, from a large 128k mapping
// the bitmap itself is also allocated in that mapping, as a slice of 128bytes, at the end of the mapping (and so marked as allocated)
void* map128_customMalloc(size_t size, int is32bits)
{
    size = 128;
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].block && (p_blocks[i].type==BTYPE_MAP) && p_blocks[i].maxfree && (!box64_is32bits || ((!is32bits && p_blocks[i].block>(void*)0xffffffffLL)) || (is32bits && p_blocks[i].block<(void*)0x100000000LL))) {
            // look for a free block
            uint8_t* map = p_blocks[i].first;
            for(uint32_t idx=p_blocks[i].lowest; idx<(p_blocks[i].size>>7); ++idx) {
                if(!(idx&7) && map[idx>>3]==0xff)
                    idx+=7;
                else if(!(map[idx>>3]&(1<<(idx&7)))) {
                    map[idx>>3] |= 1<<(idx&7);
                    p_blocks[i].maxfree -= 128;
                    p_blocks[i].lowest = idx+1;
                    mutex_unlock(&mutex_blocks);
                    return p_blocks[i].block+(idx<<7);
                }
            }
            #ifdef TRACE_MEMSTAT
            printf_log(LOG_INFO, "Warning, customme p_block[%d] MAP has maxfree=%d and lowest=%d but not free block found\n", i, p_blocks[i].maxfree, p_blocks[i].lowest);
            #endif
        }
    }
    // add a new block
    int i = n_blocks++;
    if(n_blocks>c_blocks) {
        c_blocks += box64_is32bits?256:8;
        p_blocks = (blocklist_t*)box_realloc(p_blocks, c_blocks*sizeof(blocklist_t));
    }
    size_t allocsize = MMAPSIZE128;
    p_blocks[i].block = NULL;   // incase there is a re-entrance
    p_blocks[i].first = NULL;
    p_blocks[i].size = 0;
    if(is32bits) mutex_unlock(&mutex_blocks);   // unlocking, because mmap might use it
    void* p = is32bits
        ? box_mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, -1, 0)
        : (box64_is32bits ? box32_dynarec_mmap(allocsize) : InternalMmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    if(is32bits) mutex_lock(&mutex_blocks);
    #ifdef TRACE_MEMSTAT
    customMalloc_allocated += allocsize;
    #endif
    size_t mapsize = (allocsize/128)/8;
    mapsize = (mapsize+127)&~127LL;
    p_blocks[i].type = BTYPE_MAP;
    p_blocks[i].block = p;
    p_blocks[i].first = p+allocsize-mapsize;
    p_blocks[i].size = allocsize;
    // setup marks
    uint8_t* map = p_blocks[i].first;
    for(int idx=(allocsize-mapsize)>>7;  idx<(allocsize>>7); ++idx)
        map[idx>>3] |= (1<<(idx&7));
    // 32bits check
    if(is32bits && p>(void*)0xffffffffLL) {
        printf_log(LOG_INFO, "Warning: failled to allocate 0x%x (0x%x) bytes in 32bits address space (block %d)\n", size, allocsize, i);
        // failled to allocate memory
        if(BOX64ENV(showbt) || BOX64ENV(showsegv)) {
            // mask size from this block
            p_blocks[i].size = 0;
            mutex_unlock(&mutex_blocks);
            ShowNativeBT(LOG_NONE);
            testAllBlocks();
            if(BOX64ENV(log)>=LOG_DEBUG) {
                printf_log(LOG_NONE, "Used 32bits address space map:\n");
                uintptr_t addr = rb_get_leftmost(mapallmem);
                while(addr<0x100000000LL) {
                    uintptr_t bend;
                    uint32_t val;
                    if(rb_get_end(mapallmem, addr, &val, &bend))
                        printf_log(LOG_NONE, "\t%p - %p\n", (void*)addr, (void*)bend);
                    addr = bend;
                }
            }
            p_blocks[i].size = allocsize;
        }
        #ifdef TRACE_MEMSTAT
        printf_log(LOG_INFO, "Custommem: Failled to alloc 32bits: allocation %p-%p for 128byte MAP Alloc p_blocks[%d]\n", p, p+allocsize, i);
        #endif
        p_blocks[i].maxfree = allocsize - mapsize;
        return NULL;
    }
    #ifdef TRACE_MEMSTAT
    printf_log(LOG_INFO, "Custommem: allocation %p-%p for %dbits 128byte MAP Alloc p_blocks[%d]\n", p, p+allocsize, is32bits?32:64, i);
    #endif
    // alloc 1st block
    void* ret = p_blocks[i].block;
    map[0] |= 1;
    p_blocks[i].lowest = 1;
    p_blocks[i].maxfree = allocsize - (mapsize+128);
    mutex_unlock(&mutex_blocks);
    add_blockstree((uintptr_t)p, (uintptr_t)p+allocsize, i);
    if(mapallmem) {
        if(setting_prot) {
            // defer the setProtection...
            defered_prot_p = (uintptr_t)p;
            defered_prot_sz = allocsize;
            defered_prot_prot = PROT_READ|PROT_WRITE;
        } else
            setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE);
    }
    return ret;
}

void* internal_customMalloc(size_t size, int is32bits)
{
    if(size<=128)
        return map128_customMalloc(size, is32bits);
    size_t init_size = size;
    size = roundSize(size);
    // look for free space
    blockmark_t* sub = NULL;
    size_t fullsize = size+2*sizeof(blockmark_t);
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].block && (p_blocks[i].type==BTYPE_LIST) && p_blocks[i].maxfree>=init_size && (!box64_is32bits || ((!is32bits && p_blocks[i].block>(void*)0xffffffffLL)) || (is32bits && p_blocks[i].block<(void*)0x100000000LL))) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, init_size, &rsize, p_blocks[i].first);
            if(sub) {
                if(size>rsize)
                    size = init_size;
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
        c_blocks += box64_is32bits?256:8;
        p_blocks = (blocklist_t*)box_realloc(p_blocks, c_blocks*sizeof(blocklist_t));
    }
    size_t allocsize = (fullsize>MMAPSIZE)?fullsize:MMAPSIZE;
    allocsize = (allocsize+box64_pagesize-1)&~(box64_pagesize-1);
    if(is32bits) allocsize = (allocsize+0xffffLL)&~(0xffffLL);
    p_blocks[i].block = NULL;   // incase there is a re-entrance
    p_blocks[i].first = NULL;
    p_blocks[i].size = 0;
    p_blocks[i].type = BTYPE_LIST;
    if(is32bits)    // unlocking, because mmap might use it
        mutex_unlock(&mutex_blocks);
    void* p = is32bits
        ? box_mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, -1, 0)
        : (box64_is32bits ? box32_dynarec_mmap(allocsize) : InternalMmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    if(is32bits)
        mutex_lock(&mutex_blocks);
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
    m->next.offs = allocsize-sizeof(blockmark_t);
    blockmark_t* n = NEXT_BLOCK(m);
    n->next.x32 = 0;
    n->prev.x32 = m->next.x32;
    if(is32bits && p>(void*)0xffffffffLL) {
        printf_log(LOG_INFO, "Warning: failled to allocate 0x%x (0x%x) bytes in 32bits address space (block %d)\n", size, allocsize, i);
        // failled to allocate memory
        if(BOX64ENV(showbt) || BOX64ENV(showsegv)) {
            // mask size from this block
            p_blocks[i].size = 0;
            mutex_unlock(&mutex_blocks);
            ShowNativeBT(LOG_NONE);
            testAllBlocks();
            if(BOX64ENV(log)>=LOG_DEBUG) {
                printf_log(LOG_NONE, "Used 32bits address space map:\n");
                uintptr_t addr = rb_get_leftmost(mapallmem);
                while(addr<0x100000000LL) {
                    uintptr_t bend;
                    uint32_t val;
                    if(rb_get_end(mapallmem, addr, &val, &bend))
                        printf_log(LOG_NONE, "\t%p - %p\n", (void*)addr, (void*)bend);
                    addr = bend;
                }
            }
            p_blocks[i].size = allocsize;
        }
        #ifdef TRACE_MEMSTAT
        printf_log(LOG_INFO, "Custommem: Failed to alloc 32bits: allocation %p-%p for LIST Alloc p_blocks[%d]\n", p, p+allocsize, i);
        #endif
        p_blocks[i].maxfree = allocsize - sizeof(blockmark_t)*2;
        return NULL;
    }
    #ifdef TRACE_MEMSTAT
    printf_log(LOG_INFO, "Custommem: allocation %p-%p for %dbits LIST Alloc p_blocks[%d]\n", p, p+allocsize, is32bits?32:64, i);
    #endif
    // alloc 1st block
    void* ret  = allocBlock(p_blocks[i].block, p, size, &p_blocks[i].first);
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
    mutex_unlock(&mutex_blocks);
    add_blockstree((uintptr_t)p, (uintptr_t)p+allocsize, i);
    if(mapallmem) {
        if(setting_prot) {
            // defer the setProtection...
            defered_prot_p = (uintptr_t)p;
            defered_prot_sz = allocsize;
            defered_prot_prot = PROT_READ|PROT_WRITE;
        } else
            setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE);
    }
    return ret;
}
void* customMalloc(size_t size)
{
    return internal_customMalloc(size, 0);
}
void* customMalloc32(size_t size)
{
    return internal_customMalloc(size, 1);
}

void* customCalloc(size_t n, size_t size)
{
    size_t newsize = roundSize(n*size);
    void* ret = internal_customMalloc(newsize, 0);
    if(ret) memset(ret, 0, newsize);
    return ret;
}
void* customCalloc32(size_t n, size_t size)
{
    size_t newsize = roundSize(n*size);
    void* ret = internal_customMalloc(newsize, 1);
    if(ret) memset(ret, 0, newsize);
    return ret;
}

void internal_customFree(void*, int);
void* internal_customRealloc(void* p, size_t size, int is32bits)
{
    if(!p)
        return internal_customMalloc(size, is32bits);
    //size = roundSize(size);
    uintptr_t addr = (uintptr_t)p;
    mutex_lock(&mutex_blocks);
    blocklist_t* l = findBlock(addr);
    if(l) {
        size_t subsize;
        if(l->type==BTYPE_LIST) {
            blockmark_t* sub = (blockmark_t*)(addr-sizeof(blockmark_t));
            if(expandBlock(l->block, sub, size, &l->first)) {
                l->maxfree = getMaxFreeBlock(l->block, l->size, l->first);
                mutex_unlock(&mutex_blocks);
                return p;
            }
            subsize = sizeBlock(sub);
        } else {
            //BTYPE_MAP
            if(size<=128) {
                mutex_unlock(&mutex_blocks);
                return p;
            }
            subsize = 128;
        }
        mutex_unlock(&mutex_blocks);
        void* newp = internal_customMalloc(size, is32bits);
        memcpy(newp, p, subsize);
        internal_customFree(p, is32bits);
        return newp;
    }
    mutex_unlock(&mutex_blocks);
    if(n_blocks) {
        if(is32bits) {
            return box_realloc(p, size);
        } else {
            printf_log(LOG_INFO, "Warning, block %p not found in p_blocks for realloc, malloc'ing again without free\n", (void*)addr);
        }
    }
    return internal_customMalloc(size, is32bits);
}
void* customRealloc(void* p, size_t size)
{
    return internal_customRealloc(p, size, 0);
}
void* customRealloc32(void* p, size_t size)
{
    return internal_customRealloc(p, size, 1);
}

void internal_customFree(void* p, int is32bits)
{
    if(!p || !inited) {
        return;
    }
    uintptr_t addr = (uintptr_t)p;
    mutex_lock(&mutex_blocks);
    blocklist_t* l = findBlock(addr);
    if(l) {
        if(l->type==BTYPE_LIST) {
            blockmark_t* sub = (blockmark_t*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(l->block, l->size, sub, &l->first);
            if(l->maxfree < newfree) l->maxfree = newfree;
            mutex_unlock(&mutex_blocks);
            return;
        } else {
            //BTYPE_MAP
            size_t idx = (addr-(uintptr_t)l->block)>>7;
            uint8_t* map = l->first;
            if(map[idx>>3]&(1<<(idx&7))) {
                map[idx>>3] ^= (1<<(idx&7));
                l->maxfree += 128;
            }   // warn if double free?
            #ifdef TRACE_MEMSTAT
            else printf_log(LOG_INFO, "Warning, customme free(%p) from MAP block %p, but not found as allocated\n", p, l);
            #endif
            if(l->lowest>idx)
                l->lowest = idx;
            mutex_unlock(&mutex_blocks);
            return;
        }
    }
    mutex_unlock(&mutex_blocks);
    if(n_blocks) {
        if(is32bits) {
            box_free(p);
        } else {
            printf_log(LOG_INFO, "Warning, block %p not found in p_blocks for Free\n", (void*)addr);
        }
    }
}
void customFree(void* p)
{
    internal_customFree(p, 0);
}
void customFree32(void* p)
{
    internal_customFree(p, 1);
}

void internal_print_block(int i)
{
    if(p_blocks[i].type==BTYPE_LIST) {
        blockmark_t* m = p_blocks[i].block;
        size_t sz = p_blocks[i].size;
        while(m) {
            blockmark_t *next = NEXT_BLOCK(m);
            printf_log(LOG_INFO, " block %p(%p)->%p : %d\n", m, (void*)m+sizeof(blockmark_t), next, m->next.fill);
            if(next!=m)
                m = next;
        }
    }
}

void* internal_customMemAligned(size_t align, size_t size, int is32bits)
{
    size_t align_mask = align-1;
    size_t init_size = (size+align_mask)&~align_mask;
    size = roundSize(size);
    if(align<8) align = 8;
    if(size<=128 && align<=128)
        return map128_customMalloc(size, is32bits);
    // look for free space
    blockmark_t* sub = NULL;
    size_t fullsize = size+2*sizeof(blockmark_t);
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].block && (p_blocks[i].type==BTYPE_LIST) && p_blocks[i].maxfree>=size && ((!is32bits) || ((uintptr_t)p_blocks[i].block<0x100000000LL))) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, init_size, &rsize, p_blocks[i].first);
            uintptr_t p = (uintptr_t)sub+sizeof(blockmark_t);
            uintptr_t aligned_p = (p+align_mask)&~align_mask;
            uintptr_t empty_size = 0;
            if(aligned_p!=p)
                empty_size = aligned_p-p;
            if(empty_size<=sizeof(blockmark_t)) {
                empty_size += align;
                aligned_p += align;
            }
            if(sub && (empty_size+init_size<=rsize)) {
                if(size<empty_size+init_size)
                    size = empty_size+init_size;
                if(rsize<size)
                    size = rsize;
                if(rsize-size<THRESHOLD)
                    size = rsize;
                blockmark_t* new_sub = sub;
                if(empty_size)
                    new_sub = createAlignBlock(p_blocks[i].block, sub, empty_size); // this block is a marker, between 2 free blocks
                void* ret = allocBlock(p_blocks[i].block, new_sub, size-empty_size, &p_blocks[i].first);
                if((uintptr_t)p_blocks[i].first>(uintptr_t)sub && (sub!=new_sub))
                    p_blocks[i].first = sub;
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
    p_blocks[i].block = NULL;   // incase there is a re-entrance
    p_blocks[i].first = NULL;
    p_blocks[i].size = 0;
    p_blocks[i].type = BTYPE_LIST;
    fullsize += 2*align+sizeof(blockmark_t);
    size_t allocsize = (fullsize>MMAPSIZE)?fullsize:MMAPSIZE;
    allocsize = (allocsize+box64_pagesize-1)&~(box64_pagesize-1);
    if(is32bits)
        mutex_unlock(&mutex_blocks);
    void* p = is32bits
        ? mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, -1, 0)
        : InternalMmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(is32bits)
        mutex_lock(&mutex_blocks);
#ifdef TRACE_MEMSTAT
    customMalloc_allocated += allocsize;
#endif
    p_blocks[i].block = p;
    p_blocks[i].first = p;
    p_blocks[i].size = allocsize;
    #ifdef TRACE_MEMSTAT
    printf_log(LOG_INFO, "Custommem: allocation %p-%p for LIST Alloc p_blocks[%d], aligned\n", p, p+allocsize, i);
    #endif
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.offs = allocsize-2*sizeof(blockmark_t);
    blockmark_t* n = NEXT_BLOCK(m);
    n->next.x32 = 0;
    n->prev.x32 = m->next.x32;
    uintptr_t aligned_p = ((uintptr_t)p+sizeof(blockmark_t)+align-1)&~(align-1);
    size_t empty_size = 0;
    if(aligned_p!=(uintptr_t)p+sizeof(blockmark_t))
        empty_size = aligned_p-sizeof(blockmark_t)-(uintptr_t)p;
    if(empty_size<=sizeof(blockmark_t)) {
        empty_size += align;
        aligned_p += align;
    }
    void* new_sub = NULL;
    sub = p;
    if(empty_size)
        new_sub = createAlignBlock(p_blocks[i].block, sub, empty_size);
    // alloc 1st block
    void* ret  = allocBlock(p_blocks[i].block, new_sub, size, &p_blocks[i].first);
    if(sub!=new_sub)
        p_blocks[i].first = sub;
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
    mutex_unlock(&mutex_blocks);
    if(mapallmem)
        setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE);
    add_blockstree((uintptr_t)p, (uintptr_t)p+allocsize, i);
    return ret;
}
void* customMemAligned(size_t align, size_t size)
{
    return internal_customMemAligned(align, size, 0);
}
void* customMemAligned32(size_t align, size_t size)
{
    void* ret = internal_customMemAligned(align, size, 1);
    if(((uintptr_t)ret)>=0x100000000LL) {
        printf_log(LOG_NONE, "Error, customAligned32(0x%lx, 0x%lx) return 64bits point %p\n", align, size, ret);
    }
    return ret;
}

size_t customGetUsableSize(void* p)
{
    if(!p)
        return 0;
    uintptr_t addr = (uintptr_t)p;
    mutex_lock(&mutex_blocks);
    blocklist_t* l = findBlock(addr);
    if(l) {
        if(l->type==BTYPE_MAP) {
            mutex_unlock(&mutex_blocks);
            return 128;
        }
        blockmark_t* sub = (void*)(addr-sizeof(blockmark_t));

        size_t size = SIZE_BLOCK(sub->next);
        mutex_unlock(&mutex_blocks);
        return size;
    }
    mutex_unlock(&mutex_blocks);
    return 0;
}

void* box32_dynarec_mmap(size_t size)
{
#ifdef BOX32
    // find a block that was prereserve before and big enough
    size = (size+box64_pagesize-1)&~(box64_pagesize-1);
    uint32_t flag;
    static uintptr_t cur = 0x100000000LL;
    uintptr_t bend = 0;
    while(bend<0x800000000000LL) {
        if(rb_get_end(mapallmem, cur, &flag, &bend)) {
            if(flag == MEM_RESERVED && bend-cur>=size) {
                void* ret = InternalMmap((void*)cur, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                if(ret!=MAP_FAILED)
                    rb_set(mapallmem, cur, cur+size, MEM_ALLOCATED);    // mark as allocated
                else
                    printf_log(LOG_INFO, "BOX32: Error allocating Dynarec memory: %s\n", strerror(errno));
                cur = cur+size;
                return ret;
            }
        }
        cur = bend;
    }
#endif
    //printf_log(LOG_INFO, "BOX32: Error allocating Dynarec memory: %s\n", "fallback to internal mmap");
    return InternalMmap((void*)0x100000000LL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    ;
}

#ifdef DYNAREC
#define NCHUNK          64
typedef struct mapchunk_s {
    blocklist_t         chunk;
} mapchunk_t;
typedef struct mmaplist_s {
    mapchunk_t          chunks[NCHUNK];
    mmaplist_t*         next;
} mmaplist_t;

dynablock_t* FindDynablockFromNativeAddress(void* p)
{
    if(!p)
        return NULL;
    
    uintptr_t addr = (uintptr_t)p;

    mapchunk_t* bl = (mapchunk_t*)rb_get_64(rbt_dynmem, addr);
    if(bl) {
        // browse the map allocation as a fallback
        blockmark_t* sub = (blockmark_t*)bl->chunk.block;
        while((uintptr_t)sub<addr) {
            blockmark_t* n = NEXT_BLOCK(sub);
            if((uintptr_t)n>addr) {
                // found it!
                if(!sub->next.fill) return NULL; // empty space?
                // self is the field of a block
                dynablock_t** ret = (dynablock_t**)((uintptr_t)sub+sizeof(blockmark_t));
                return *ret;
            }
            sub = n;
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
    int idx = 0;
    uintptr_t sz = size + 2*sizeof(blockmark_t);
    while(1) {
        int i = idx%NCHUNK;
        if(list->chunks[i].chunk.maxfree>=size) {
            // looks free, try to alloc!
            size_t rsize = 0;
            void* sub = getFirstBlock(list->chunks[i].chunk.block, size, &rsize, list->chunks[i].chunk.first);
            if(sub) {
                void* ret = allocBlock(list->chunks[i].chunk.block, sub, size, &list->chunks[i].chunk.first);
                if(rsize==list->chunks[i].chunk.maxfree)
                    list->chunks[i].chunk.maxfree = getMaxFreeBlock(list->chunks[i].chunk.block, list->chunks[i].chunk.size, list->chunks[i].chunk.first);
                //rb_set_64(list->chunks[i].tree, (uintptr_t)ret, (uintptr_t)ret+size, (uintptr_t)ret);
                return (uintptr_t)ret;
            }
        }
        // check if new
        if(!list->chunks[i].chunk.size) {
            // alloc a new block, aversized or not, we are at the end of the list
            size_t allocsize = (sz>DYNMMAPSZ)?sz:DYNMMAPSZ;
            // allign sz with pagesize
            allocsize = (allocsize+(box64_pagesize-1))&~(box64_pagesize-1);
            void* p=MAP_FAILED;
            #ifdef BOX32
            if(box64_is32bits)
                p = box32_dynarec_mmap(allocsize);
            #endif
            // disabling for now. explicit hugepage needs to be enabled to be used on userspace 
            // with`/sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages` as the number of allowaed 2M huge page
            // At least with a 2M allocation, transparent huge page should kick-in
            #if 0//def MAP_HUGETLB
            if(p==MAP_FAILED && allocsize==DYNMMAPSZ) {
                p = InternalMmap(NULL, allocsize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_HUGETLB, -1, 0);
                if(p!=MAP_FAILED) printf_log(LOG_INFO, "Allocated a dynarec memory block with HugeTLB\n");
                else printf_log(LOG_INFO, "Failled to allocated a dynarec memory block with HugeTLB (%s)\n", strerror(errno));
            }
            #endif
            if(p==MAP_FAILED)
                p = InternalMmap(NULL, allocsize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            if(p==MAP_FAILED) {
                dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes (%s)\n", allocsize, strerror(errno));
                return 0;
            }
            #ifdef MADV_HUGEPAGE
            madvise(p, allocsize, MADV_HUGEPAGE);
            #endif
#ifdef TRACE_MEMSTAT
            dynarec_allocated += allocsize;
            printf_log(LOG_INFO, "Custommem: allocation %p-%p for Dynarec block %d\n", p, p+allocsize, idx);
#endif
            setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE | PROT_EXEC);

            list->chunks[i].chunk.block = p;
            list->chunks[i].chunk.first = p;
            list->chunks[i].chunk.size = allocsize;
            rb_set_64(rbt_dynmem, (uintptr_t)p, (uintptr_t)p+allocsize, (uintptr_t)&list->chunks[i]);
            // setup marks
            blockmark_t* m = (blockmark_t*)p;
            m->prev.x32 = 0;
            m->next.fill = 0;
            m->next.offs = allocsize-sizeof(blockmark_t);
            blockmark_t* n = NEXT_BLOCK(m);
            n->next.x32 = 0;
            n->prev.x32 = m->next.x32;
            // alloc 1st block
            void* ret  = allocBlock(list->chunks[i].chunk.block, p, size, &list->chunks[i].chunk.first);
            list->chunks[i].chunk.maxfree = getMaxFreeBlock(list->chunks[i].chunk.block, list->chunks[i].chunk.size, list->chunks[i].chunk.first);
            if(list->chunks[i].chunk.maxfree)
                list->chunks[i].chunk.first = getNextFreeBlock(m);
            //rb_set_64(list->chunks[i].tree, (uintptr_t)ret, (uintptr_t)ret+size, (uintptr_t)ret);
            return (uintptr_t)ret;
        }
        // next chunk...
        ++idx; ++i;
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
    

    mapchunk_t* bl = (mapchunk_t*)rb_get_64(rbt_dynmem, addr);

    if(bl) {
        void* sub = (void*)(addr-sizeof(blockmark_t));
        size_t newfree = freeBlock(bl->chunk.block, bl->chunk.size, sub, &bl->chunk.first);
        if(bl->chunk.maxfree < newfree)
            bl->chunk.maxfree = newfree;
        return;
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
    // mark existing blocks as unclean, to be sure...
    uintptr_t start_addr = my_context?((addr<my_context->max_db_size)?0:(addr-my_context->max_db_size)):addr;
    dynablock_t* db = NULL;
    uintptr_t end = addr+size;
    while (start_addr<end) {
        start_addr = getDBSize(start_addr, end-start_addr, &db);
        if(db)
            MarkRangeDynablock(db, addr, size);
    }
}

// Will return 1 if at least 1 db in the address range
int cleanDBFromAddressRange(uintptr_t addr, size_t size, int destroy)
{
    uintptr_t start_addr = my_context?((addr<my_context->max_db_size)?0:(addr-my_context->max_db_size)):addr;
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p/%p -> %p %s\n", (void*)addr, (void*)start_addr, (void*)(addr+size-1), destroy?"destroy":"mark");
    dynablock_t* db = NULL;
    uintptr_t end = addr+size;
    int ret = 0;
    while (start_addr<end) {
        start_addr = getDBSize(start_addr, end-start_addr, &db);
        if(db) {
            ret = 1;
            if(destroy)
                FreeRangeDynablock(db, addr, size);
            else
                MarkRangeDynablock(db, addr, size);
        }
    }
    return ret;
}

#ifdef JMPTABL_SHIFT4
static uintptr_t *create_jmptbl(int for32bits, uintptr_t idx0, uintptr_t idx1, uintptr_t idx2, uintptr_t idx3, uintptr_t idx4)
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
    if(for32bits) return NULL;
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
static uintptr_t *create_jmptbl(int for32bits, uintptr_t idx0, uintptr_t idx1, uintptr_t idx2, uintptr_t idx3)
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
    if(for32bits) return NULL;
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
    return (native_lock_storeifref(create_jmptbl(0, idx0, idx1, idx2, idx3, idx4), jmp, native_next)==jmp)?1:0;
    #else
    return (native_lock_storeifref(create_jmptbl(0, idx0, idx1, idx2, idx3), jmp, native_next)==jmp)?1:0;
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
    return (native_lock_storeifref(create_jmptbl(0, idx0, idx1, idx2, idx3, idx4), jmp, ref)==jmp)?1:0;
    #else
    return (native_lock_storeifref(create_jmptbl(0, idx0, idx1, idx2, idx3), jmp, ref)==jmp)?1:0;
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
uintptr_t getJumpTable48()
{
    return (uintptr_t)box64_jmptbl_48;
}

uintptr_t getJumpTable32()
{
    #ifdef JMPTABL_SHIFT4
    create_jmptbl(1, 0, 0, 0, 0, 0);
    return (uintptr_t)box64_jmptbl4[0][0];
    #else
    create_jmptbl(1, 0, 0, 0, 0);
    return (uintptr_t)box64_jmptbl3[0][0];
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
    return (uintptr_t)create_jmptbl(0, idx0, idx1, idx2, idx3, idx4);
    #else
    return (uintptr_t)create_jmptbl(0, idx0, idx1, idx2, idx3);
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
            } else if(prot&PROT_DYNAREC_R) {
                if(mark)
                    cleanDBFromAddressRange(cur, bend-cur, 0);
                prot &= ~PROT_CUSTOM;
            }
        }
        if (prot != oprot)
            rb_set(memprot, cur, bend, prot);
        cur = bend;
    }
    UNLOCK_PROT();
}
// Add the NEVERCLEAN flag for an adress range, mark all block as dirty, and lift write protection if needed
void neverprotectDB(uintptr_t addr, size_t size, int mark)
{
    dynarec_log(LOG_DEBUG, "neverprotectDB %p -> %p (mark=%d)\n", (void*)addr, (void*)(addr+size-1), mark);

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
            } else if(prot&PROT_DYNAREC_R) {
                if(mark)
                    cleanDBFromAddressRange(cur, bend-cur, 0);
                prot &= ~PROT_DYN;
            }
            prot |= PROT_NEVERCLEAN;
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
            dynarec_log_prefix(0, LOG_DEBUG, "0\n");
            UNLOCK_PROT_READ();
            return 0;
        } else {
            addr = bend;
        }
    }
    UNLOCK_PROT_READ();
    dynarec_log_prefix(0, LOG_DEBUG, "1\n");
    return 1;
}

static uintptr_t hotpage = 0;
static int hotpage_cnt = 0;
static int repeated_count = 0;
static uintptr_t repeated_page = 0;
#define HOTPAGE_MARK 64
#define HOTPAGE_DIRTY 4
void SetHotPage(uintptr_t addr)
{
    hotpage = addr&~(box64_pagesize-1);
    hotpage_cnt = BOX64ENV(dynarec_dirty)?HOTPAGE_DIRTY:HOTPAGE_MARK;
}
void CheckHotPage(uintptr_t addr)
{
    uintptr_t page = (uintptr_t)addr&~(box64_pagesize-1);
    if(repeated_count==1 && repeated_page==page) {
        if(BOX64ENV(dynarec_dirty)>1) {
            dynarec_log(LOG_INFO, "Detecting a Hotpage at %p (%d), marking page as NEVERCLEAN\n", (void*)repeated_page, repeated_count);
            neverprotectDB(repeated_page, box64_pagesize, 1);
        } else {
            dynarec_log(LOG_INFO, "Detecting a Hotpage at %p (%d)\n", (void*)repeated_page, repeated_count);
            SetHotPage(repeated_page);
        }
        repeated_count = 0;
        repeated_page = 0;
    } else {
        repeated_count = 1;
        repeated_page = page;
    }
}
int isInHotPage(uintptr_t addr)
{
    if(!hotpage_cnt)
        return 0;
    int ret = (addr>=hotpage) && (addr<hotpage+box64_pagesize);
    if(ret)
        --hotpage_cnt;
    return ret;
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
    rb_set(mapallmem, cur, cur+size, MEM_ALLOCATED);
    while (cur < end) {
        uintptr_t bend;
        uint32_t oprot;
        rb_get_end(memprot, cur, &oprot, &bend);
        if(bend>end) bend = end;
        uint32_t dyn=(oprot&PROT_DYN);
        uint32_t never = dyn&PROT_NEVERPROT;
        if(!(never)) {
            if(dyn && (prot&PROT_WRITE)) {   // need to remove the write protection from this block
                dyn = PROT_DYNAREC;
                int ret = mprotect((void*)cur, bend-cur, prot&~PROT_WRITE);
                dynarec_log(LOG_DEBUG, " mprotect %p:%p 0x%hhx => %d\n", (void*)cur, (void*)(bend-1), prot&~PROT_WRITE, ret);
            } else if(dyn && !(prot&PROT_WRITE)) {
                dyn = PROT_DYNAREC_R;
            }
        }
        uint32_t new_prot = prot?(prot|dyn):(prot|never);
        if (new_prot != oprot)
            rb_set(memprot, cur, bend, new_prot);
        cur = bend;
    }
    UNLOCK_PROT();
}

void setProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    size = ALIGN(size);
    LOCK_PROT();
    ++setting_prot;
    uintptr_t cur = addr & ~(box64_pagesize-1);
    uintptr_t end = ALIGN(cur+size);
    rb_set(mapallmem, cur, end, MEM_ALLOCATED);
    rb_set(memprot, cur, end, prot);
    --setting_prot;
    UNLOCK_PROT();
}

void setProtection_mmap(uintptr_t addr, size_t size, uint32_t prot)
{
    if(!size)
        return;
    addr &= ~(box64_pagesize-1);
    size = ALIGN(size);
    if(!prot) {
        LOCK_PROT();
        rb_set(mapallmem, addr, addr+size, MEM_MMAP);
        rb_unset(memprot, addr, addr+size);
        UNLOCK_PROT();
    }
    else{//SetProtection
        LOCK_PROT();
        ++setting_prot;
        uintptr_t cur = addr & ~(box64_pagesize-1);
        uintptr_t end = ALIGN(cur+size);
        rb_set(mapallmem, cur, end, MEM_MMAP);
        rb_set(memprot, cur, end, prot);
        --setting_prot;
        UNLOCK_PROT();
    }
}

void setProtection_elf(uintptr_t addr, size_t size, uint32_t prot)
{
    size = ALIGN(size);
    addr &= ~(box64_pagesize-1);
    if(prot)
        setProtection(addr, size, prot);
    else {
        LOCK_PROT();
        rb_set(mapallmem, addr, addr+size, MEM_ALLOCATED);
        rb_unset(memprot, addr, addr+size);
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
    uint32_t val;
    uintptr_t endb; 
    int there = rb_get_end(mapallmem, addr, &val, &endb);
    // block is here or absent, no half-block handled..
    if(!there)
        rb_set(mapallmem, addr, addr+size, MEM_ALLOCATED);
    UNLOCK_PROT();
    // don't need to add precise tracking probably
}

uintptr_t pbrk = 0;
uintptr_t old_brk = 0;
uintptr_t* cur_brk = NULL;
void loadProtectionFromMap()
{
#ifndef _WIN32 // TODO: Should this be implemented on Win32?
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
            if(!pbrk && strstr(buf, "[heap]"))
                pbrk = s;
            if(s>0x7fff00000000LL)
                have48bits = 1;
        }
    }
    static int shown48bits = 0;
    if(!shown48bits) {
        shown48bits = 1;
        if(have48bits)
            printf_log(LOG_INFO, "Detected 48bits at least of address space\n");
        else
            printf_log(LOG_INFO, "Didn't detect 48bits of address space, considering it's 39bits\n");
    }
    if(!pbrk) {
        printf_log(LOG_INFO, "Warning, program break not found\n");
        if(cur_brk) pbrk = *cur_brk;    // approximate is better than nothing
    }
    fclose(f);
    box64_mapclean = 1;
#endif
}

void freeProtection(uintptr_t addr, size_t size)
{
    size = ALIGN(size);
    addr &= ~(box64_pagesize-1);
    dynarec_log(LOG_DEBUG, "freeProtection %p:%p\n", (void*)addr, (void*)(addr+size-1));
    LOCK_PROT();
    rb_unset(mapallmem, addr, addr+size);
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

uint32_t getProtection_fast(uintptr_t addr)
{
    LOCK_PROT_FAST();
    uint32_t ret = rb_get(memprot, addr);
    UNLOCK_PROT_FAST();
    return ret;
}

int getMmapped(uintptr_t addr)
{
    return (rb_get(mapallmem, addr)&MEM_ALLOCATED); // will be ok for both MEM_ALLOCATED & MEM_MMAP
}

int memExist(uintptr_t addr)
{
    return rb_get(mapallmem, addr);
}

#define LOWEST (void*)0x10000
#define WINE_LOWEST (void*)0x30000000
#define MEDIUM (void*)0x40000000
#define HIGH   (void*)0x60000000

void* find31bitBlockNearHint(void* hint_, size_t size, uintptr_t mask)
{
    // first, check if program break as changed
    if(pbrk && cur_brk && *cur_brk!=old_brk) {
        old_brk = *cur_brk;
        setProtection(pbrk, old_brk-pbrk, PROT_READ|PROT_WRITE);
    }
    uint32_t prot;
    uintptr_t hint = (uintptr_t)hint_;
    if(hint_<LOWEST) hint = (uintptr_t)WINE_LOWEST;
    uintptr_t bend = 0;
    uintptr_t cur = (uintptr_t)hint;
    uintptr_t upper = 0xc0000000LL;
    if(cur>upper) upper = 0x100000000LL;
    if(!mask) mask = 0xffff;
    while(cur<upper) {
        if(!rb_get_end(mapallmem, cur, &prot, &bend)) {
            if(bend-cur>=size)
                return (void*)cur;
        }
        // granularity 0x10000
        cur = (bend+mask)&~mask;
    }
    if(hint_)
        return NULL;
    cur = (uintptr_t)LOWEST;
    while(cur<(uintptr_t)hint) {
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
    return ret;
}
void* find47bitBlock(size_t size)
{
    void* ret = find47bitBlockNearHint(HIGH, size, 0);
    if(!ret)
        ret = find31bitBlockNearHint(MEDIUM, size, 0);
    if(!ret)
        ret = find31bitBlockNearHint(LOWEST, size, 0);
    return ret;
}
void* find47bitBlockNearHint(void* hint, size_t size, uintptr_t mask)
{
    // first, check if program break as changed
    if(pbrk && cur_brk && *cur_brk!=old_brk) {
        old_brk = *cur_brk;
        setProtection(pbrk, old_brk-pbrk, PROT_READ|PROT_WRITE);
    }
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

void* find31bitBlockElf(size_t size, int mainbin, uintptr_t mask)
{
    static void* startingpoint = NULL;
    if(!startingpoint) {
        startingpoint = (void*)WINE_LOWEST;
    }
    void* mainaddr = (void*)0x1000000;
    void* ret = find31bitBlockNearHint(MEDIUM, size, mask);
    if(!ret)
        ret = find31bitBlockNearHint(LOWEST, size, mask);
    if(!mainbin)
        startingpoint = (void*)(((uintptr_t)startingpoint+size+0x1000000)&~0xffffff);
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
#ifdef BOX32
void reverveHigMem32(void)
{
    loadProtectionFromMap();
    uintptr_t cur_size = 1024LL*1024*1024*1024; // start with 1TB check
    void* cur;
    while(cur_size>=65536) {
        cur = InternalMmap(NULL, cur_size, 0, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
        if((cur==MAP_FAILED) || (cur<(void*)0x100000000LL)) {
            if(cur!=MAP_FAILED) {
                //printf_log(LOG_INFO, " Failed to reserve high %p (%zx)\n", cur, cur_size);
                InternalMunmap(cur, cur_size);
            } //else 
                //printf_log(LOG_INFO, " Failed to reserve %zx sized block\n", cur_size);
            cur_size>>=1;
        } else {
            rb_set(mapallmem, (uintptr_t)cur, (uintptr_t)cur+cur_size, MEM_RESERVED);
            //printf_log(LOG_INFO, "Reserved high %p (%zx)\n", cur, cur_size);
        }
    }
    printf_log(LOG_INFO, "Memory higher than 32bits reserved\n");
    if (BOX64ENV(log)>=LOG_DEBUG) {
        uintptr_t start=0x100000000LL;
        int prot;
        uintptr_t bend = start;
        while (bend!=0xffffffffffffffffLL) {
            if(rb_get_end(mapallmem, start, &prot, &bend)) {
                printf_log(LOG_NONE, " Reserved: %p - %p (%d)\n", (void*)start, (void*)bend, prot);
            }
            start = bend;
        }
    }
    PersonalityAddrLimit32Bit();
}
#endif
void my_reserveHighMem()
{
    static int reserved = 0;
    if(reserved || (!have48bits && !box64_is32bits))
        return;
    reserved = 1;
    #ifdef BOX32
    if(box64_is32bits) {
        reverveHigMem32();
        return;
    }
    #endif
    uintptr_t cur = box64_is32bits?(1ULL<<32):(1ULL<<47);
    uintptr_t bend = 0;
    uint32_t prot;
    while (bend!=0xffffffffffffffffLL) {
        if(!rb_get_end(mapallmem, cur, &prot, &bend)) {
            // create a border at 39bits...
            if(cur<(1ULL<<39) && bend>(1ULL<<39))
                bend = 1ULL<<39;
            // create a border at 47bits
            if(cur<(1ULL<<47) && bend>(1ULL<<47))
                bend = 1ULL<<47;
            // create a border at 48bits
            if(cur<(1ULL<<48) && bend>(1ULL<<48))
                bend = 1ULL<<48;
            void* ret = InternalMmap((void*)cur, bend - cur, 0, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
            printf_log(LOG_DEBUG, "Reserve %p-%p => %p (%s)\n", (void*)cur, bend, ret, (ret==MAP_FAILED)?strerror(errno):"ok");
            if(ret!=(void*)-1) {
                rb_set(mapallmem, cur, bend, MEM_ALLOCATED);
            }
        }
        cur = bend;
    }

}

void reserveHighMem()
{
    if(!box64_is32bits && !BOX64ENV(reserve_high))
        return; // don't reserve by default on 64bits
    my_reserveHighMem();
}

void init_custommem_helper(box64context_t* ctx)
{
    (void)ctx;
    if(inited) // already initialized
        return;
    inited = 1;

    cur_brk = dlsym(RTLD_NEXT, "__curbrk");
    init_mutexes();
    blockstree = rbtree_init("blockstree");
    // if there is some blocks already
    if(n_blocks)
        for(int i=0; i<n_blocks; ++i)
            rb_set(blockstree, (uintptr_t)p_blocks[i].block, (uintptr_t)p_blocks[i].block+p_blocks[i].size, i);
    memprot = rbtree_init("memprot");
    sigfillset(&critical_prot);
#ifdef DYNAREC
    if(BOX64ENV(dynarec)) {
        #ifdef JMPTABL_SHIFT4
        for(int i=0; i<(1<<JMPTABL_SHIFT4); ++i)
            box64_jmptbl4[i] = box64_jmptbldefault3;
        for(int i=0; i<(1<<JMPTABL_SHIFT3); ++i) {
            box64_jmptbldefault3[i] = box64_jmptbldefault2;
            box64_jmptbl_48[i] = box64_jmptbldefault2;
        }
        box64_jmptbl4[0] = box64_jmptbl_48;
        #else
        for(int i=0; i<(1<<JMPTABL_SHIFT3); ++i) {
                box64_jmptbl3[i] = box64_jmptbldefault2;
                box64_jmptbl_48[i] = box64_jmptbldefault1;
            }
        box64_jmptbl3[0] = box64_jmptbl_48;
        #endif
        for(int i=0; i<(1<<JMPTABL_SHIFT2); ++i)
            box64_jmptbldefault2[i] = box64_jmptbldefault1;
        for(int i=0; i<(1<<JMPTABL_SHIFT1); ++i)
            box64_jmptbldefault1[i] = box64_jmptbldefault0;
        for(int i=0; i<(1<<JMPTABL_SHIFT0); ++i)
            box64_jmptbldefault0[i] = (uintptr_t)native_next;
    }
    lockaddress = kh_init(lockaddress);
    rbt_dynmem = rbtree_init("rbt_dynmem");
#endif
    pthread_atfork(NULL, NULL, atfork_child_custommem);
    // init mapallmem list
    mapallmem = rbtree_init("mapallmem");
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
    if(BOX64ENV(log)) printf("Allocation:\n- dynarec: %lld kio\n- customMalloc: %lld kio\n- jump table: %lld kio (%lld level 4, %lld level 3, %lld level 2, %lld level 1 table allocated, for %lld jumps, with at most %lld per level 1)\n", dynarec_allocated / 1024, customMalloc_allocated / 1024, jmptbl_allocated / 1024, jmptbl_allocated4, jmptbl_allocated3, jmptbl_allocated2, jmptbl_allocated1, njmps, njmps_in_lv1_max);
#else
    if(BOX64ENV(log)) printf("Allocation:\n- dynarec: %lld kio\n- customMalloc: %lld kio\n- jump table: %lld kio (%lld level 3, %lld level 2, %lld level 1 table allocated, for %lld jumps, with at most %lld per level 1)\n", dynarec_allocated / 1024, customMalloc_allocated / 1024, jmptbl_allocated / 1024, jmptbl_allocated3, jmptbl_allocated2, jmptbl_allocated1, njmps, njmps_in_lv1_max);
#endif
    if(BOX64ENV(log)) testAllBlocks();
#endif
    if(!inited)
        return;
    inited = 0;
#ifdef DYNAREC
    if(BOX64ENV(dynarec)) {
        dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
        mmaplist_t* head = mmaplist;
        mmaplist = NULL;
        while(head) {
            for (int i=0; i<NCHUNK; ++i) {
                if(head->chunks[i].chunk.block)
                    InternalMunmap(head->chunks[i].chunk.block, head->chunks[i].chunk.size);
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
                #ifndef JMPTABL_SHIFT4
                if(i3)
                #endif
                    customFree(box64_jmptbl3[i3]);
            }
        #ifdef JMPTABL_SHIFT4
                if(i4)
                    customFree(box64_jmptbl4[i4]);
            }
        #endif
    }
    kh_destroy(lockaddress, lockaddress);
    lockaddress = NULL;
    rbtree_delete(rbt_dynmem);
    rbt_dynmem = NULL;
#endif
    rbtree_delete(memprot);
    memprot = NULL;
    rbtree_delete(mapallmem);
    mapallmem = NULL;
    rbtree_delete(blockstree);
    blockstree = NULL;

    for(int i=0; i<n_blocks; ++i)
        InternalMunmap(p_blocks[i].block, p_blocks[i].size);
    box_free(p_blocks);
#if !defined(USE_CUSTOM_MUTEX)
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

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x200000
#endif
#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif
EXPORT void* box_mmap(void *addr, size_t length, int prot, int flags, int fd, ssize_t offset)
{
    if(prot&PROT_WRITE)
        prot|=PROT_READ;    // PROT_READ is implicit with PROT_WRITE on i386
    int new_flags = flags;
    void* old_addr = addr;
    #ifndef NOALIGN
    new_flags&=~MAP_32BIT;   // remove MAP_32BIT
    if((flags&MAP_32BIT) && !(flags&MAP_FIXED)) {
        // MAP_32BIT only exist on x86_64!
        addr = find31bitBlockNearHint(old_addr, length, 0);
    } else if (box64_wine || 1) {   // other mmap should be restricted to 47bits
        if (!(flags&MAP_FIXED) && !addr)
            addr = find47bitBlock(length);
    }
    #endif
    void* ret = InternalMmap(addr, length, prot, new_flags, fd, offset);
#if !defined(NOALIGN)
    if((ret!=MAP_FAILED) && (flags&MAP_32BIT) &&
      (((uintptr_t)ret>0xffffffffLL) || ((box64_wine) && ((uintptr_t)ret&0xffff) && (ret!=addr)))) {
        int olderr = errno;
        InternalMunmap(ret, length);
        loadProtectionFromMap();    // reload map, because something went wrong previously
        addr = find31bitBlockNearHint(old_addr, length, 0); // is this the best way?
        new_flags = (addr && isBlockFree(addr, length) )? (new_flags|MAP_FIXED) : new_flags;
        if((new_flags&(MAP_FIXED|MAP_FIXED_NOREPLACE))==(MAP_FIXED|MAP_FIXED_NOREPLACE)) new_flags&=~MAP_FIXED_NOREPLACE;
        ret = InternalMmap(addr, length, prot, new_flags, fd, offset);
        if(old_addr && ret!=old_addr && ret!=MAP_FAILED)
            errno = olderr;
    } else if((ret!=MAP_FAILED) && !(flags&MAP_FIXED) && ((box64_wine)) && (addr && (addr!=ret)) &&
             (((uintptr_t)ret>0x7fffffffffffLL) || ((uintptr_t)ret&~0xffff))) {
        int olderr = errno;
        InternalMunmap(ret, length);
        loadProtectionFromMap();    // reload map, because something went wrong previously
        addr = find47bitBlockNearHint(old_addr, length, 0); // is this the best way?
        new_flags = (addr && isBlockFree(addr, length)) ? (new_flags|MAP_FIXED) : new_flags;
        if((new_flags&(MAP_FIXED|MAP_FIXED_NOREPLACE))==(MAP_FIXED|MAP_FIXED_NOREPLACE)) new_flags&=~MAP_FIXED_NOREPLACE;
        ret = InternalMmap(addr, length, prot, new_flags, fd, offset);
        if(old_addr && ret!=old_addr && ret!=MAP_FAILED) {
            errno = olderr;
            if(old_addr>(void*)0x7fffffffff && !have48bits)
                errno = EEXIST;
        }
    }
    #endif
    return ret;
}

EXPORT int box_munmap(void* addr, size_t length)
{
    int ret = InternalMunmap(addr, length);
    return ret;
}
