#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <setjmp.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "tools/bridge_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_private.h"
#include "elfloader.h"

#include "dynarec_native.h"
#include "native_lock.h"

#include "custommem.h"
#include "khash.h"

KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)

uint32_t X31_hash_code(void* addr, int len)
{
    if(!len) return 0;
    uint8_t* p = (uint8_t*)addr;
	int32_t h = *p;
	for (--len, ++p; len; --len, ++p) h = (h << 5) - h + (int32_t)*p;
	return (uint32_t)h;
}

dynablocklist_t* NewDynablockList(uintptr_t text, int textsz, int direct)
{
    if(!textsz) {
        printf_log(LOG_NONE, "Error, creating a NULL sized Dynablock\n");
        return NULL;
    }
    dynablocklist_t* ret = (dynablocklist_t*)calloc(1, sizeof(dynablocklist_t));
    ret->text = text;
    ret->textsz = textsz;
    ret->minstart = text;
    ret->maxend = text+textsz-1;
    if(direct && textsz) {
        ret->direct = (dynablock_t**)calloc(textsz, sizeof(dynablock_t*));
        if(!ret->direct) {printf_log(LOG_NONE, "Warning, fail to create direct block for dynablock @%p\n", (void*)text);}
    }
    dynarec_log(LOG_DEBUG, "New Dynablocklist %p, from %p->%p\n", ret, (void*)text, (void*)(text+textsz));
    return ret;
}

void FreeDynablock(dynablock_t* db, int need_lock)
{
    if(db) {
        if(db->gone)
            return; // already in the process of deletion!
        dynarec_log(LOG_DEBUG, "FreeDynablock(%p), db->block=%p x64=%p:%p parent=%p, father=%p, with %d son(s) already gone=%d\n", db, db->block, db->x64_addr, db->x64_addr+db->x64_size-1, db->parent, db->father, db->sons_size, db->gone);
        if(need_lock)
            pthread_mutex_lock(&my_context->mutex_dyndump);
        db->done = 0;
        db->gone = 1;
        // remove from direct if there
        uintptr_t startdb = db->parent->text;
        uintptr_t enddb = db->parent->text + db->parent->textsz;
        if(db->parent->direct) {
            uintptr_t addr = (uintptr_t)db->x64_addr;
            if(addr>=startdb && addr<enddb)
                native_lock_xchg(&db->parent->direct[addr-startdb], 0);   // secured write
        }
        // remove jumptable
        setJumpTableDefault64(db->x64_addr);
        // remove and free the sons
        for (int i=0; i<db->sons_size; ++i) {
            dynablock_t *son = (dynablock_t*)native_lock_xchg(&db->sons[i], 0);
            FreeDynablock(son, 0);
        }
        // only the father free the DynarecMap
        if(!db->father) {
            dynarec_log(LOG_DEBUG, " -- FreeDyrecMap(%p, %d)\n", db->block, db->size);
            FreeDynarecMap(db, (uintptr_t)db->block, db->size);
            free(db->sons);
            free(db->instsize);
        }
        free(db);
        if(need_lock)
            pthread_mutex_unlock(&my_context->mutex_dyndump);
    }
}

void FreeDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    dynarec_log(LOG_DEBUG, "Free Dynablocklist %p, with Direct Blocks %p\n", *dynablocks, (*dynablocks)->direct);
    if((*dynablocks)->direct) {
        for (int i=0; i<(*dynablocks)->textsz; ++i) {
            if((*dynablocks)->direct[i] && !(*dynablocks)->direct[i]->father) 
                FreeDynablock((*dynablocks)->direct[i], 1);
        }
        free((*dynablocks)->direct);
    }
    (*dynablocks)->direct = NULL;

    free(*dynablocks);
    *dynablocks = NULL;
}

void MarkDynablock(dynablock_t* db)
{
    if(db) {
        if(db->father)
            db = db->father;    // mark only father
        if(db->need_test)
            return; // already done
        dynarec_log(LOG_DEBUG, "MarkDynablock %p with %d son(s) %p-%p\n", db, db->sons_size, db->x64_addr, db->x64_addr+db->x64_size-1);
        db->need_test = 1;
        setJumpTableDefault64(db->x64_addr);
        for(int i=0; i<db->sons_size; ++i)
            setJumpTableDefault64(db->sons[i]->x64_addr);
    }
}

uintptr_t StartDynablockList(dynablocklist_t* db)
{
    if(db)
        return db->text;
    return 0;
}
uintptr_t EndDynablockList(dynablocklist_t* db)
{
    if(db)
        return db->text+db->textsz-1;
    return 0;
}

int IntervalIntersects(uintptr_t start1, uintptr_t end1, uintptr_t start2, uintptr_t end2)
{
    if(start1 > end2 || start2 > end1)
        return 0;
    return 1;
}

void MarkDirectDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size)
{
    // Mark will try to find *any* blocks that intersect the range to mark
    if(!dynablocks)
        return;
    if(!dynablocks->direct)
        return;
    uintptr_t startdb = dynablocks->text;
    uintptr_t sizedb = dynablocks->textsz;
    dynablock_t *db;
    dynarec_log(LOG_DEBUG, "MarkDirectDynablock %p-%p .. startdb=%p, sizedb=%p\n", (void*)addr, (void*)addr+size-1, (void*)startdb, (void*)sizedb);
    for(uintptr_t i = 0; i<sizedb; ++i)
        if((db=dynablocks->direct[i]))
            if(IntervalIntersects((uintptr_t)db->x64_addr, (uintptr_t)db->x64_addr+db->x64_size-1, addr, addr+size+1))
                MarkDynablock(db);
}

int FreeRangeDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size)
{
    if(!dynablocks)
        return 1;

    if(dynablocks->direct) {
        dynablock_t* db;
        int ret;
        khint_t k;
        kh_dynablocks_t *blocks = kh_init(dynablocks);
        // copy in a temporary list
        uintptr_t startdb = dynablocks->text;
        uintptr_t enddb = startdb + dynablocks->textsz;
        uintptr_t start = addr;
        uintptr_t end = addr+size;
        if(start<startdb)
            start = startdb;
        if(end>enddb)
            end = enddb;
        if(end>startdb && start<enddb)
            for(uintptr_t i = start; i<end; ++i) {
                db = (dynablock_t*)native_lock_xchg(&dynablocks->direct[i-startdb], 0);
                if(db) {
                    if(db->father)
                        db = db->father;
                    if(db->parent==dynablocks) {
                        k = kh_put(dynablocks, blocks, (uintptr_t)db, &ret);
                        kh_value(blocks, k) = db;
                    }
                }
            }
        // purge the list
        kh_foreach_value(blocks, db,
            FreeDynablock(db, 1);
        );
        kh_destroy(dynablocks, blocks);
        // check emptyness
        for(uintptr_t i=0; i<dynablocks->textsz; ++i)
            if(dynablocks->direct[i])
                return 0;
        return 1;
    }
    return 1;
}
void MarkRangeDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size)
{
    if(!dynablocks)
        return;
    dynarec_log(LOG_DEBUG, "MarkRangeDynablock %p-%p\n", (void*)addr, (void*)addr+size-1);
    if(dynablocks->direct) {
        uintptr_t new_addr = dynablocks->minstart;
        uintptr_t new_size = dynablocks->maxend - new_addr + 1;
        MarkDirectDynablock(dynablocks, addr, size);
        // the blocks check before
        for(unsigned idx=(new_addr)>>DYNAMAP_SHIFT; idx<(addr>>DYNAMAP_SHIFT); ++idx)
            MarkDirectDynablock(getDB(idx), addr, size);
    }
}

dynablock_t* FindDynablockDynablocklist(void* addr, kh_dynablocks_t* dynablocks)
{
    if(!dynablocks)
        return NULL;
    dynablock_t* db;
    kh_foreach_value(dynablocks, db, 
        const uintptr_t s = (uintptr_t)db->block;
        const uintptr_t e = (uintptr_t)db->block+db->size;
        if((uintptr_t)addr>=s && (uintptr_t)addr<e)
            return db->father?db->father:db;
    )
    return NULL;
}

static dynablocklist_t* getDBFromAddress(uintptr_t addr)
{
    const uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    return getDB(idx);
}

dynablock_t *AddNewDynablock(dynablocklist_t* dynablocks, uintptr_t addr, int* created)
{
    if(!dynablocks) {
        dynarec_log(LOG_INFO, "Warning: Ask to create a dynablock with a NULL dynablocklist (addr=%p)\n", (void*)addr);
        *created = 0;
        return NULL;
    }
    if((addr<dynablocks->text) || (addr>=(dynablocks->text+dynablocks->textsz))) {
        return AddNewDynablock(getDBFromAddress(addr), addr, created);
    }
    dynablock_t* block = NULL;
    // first, check if it exist in direct access mode
    if(dynablocks->direct) {
        block = dynablocks->direct[addr-dynablocks->text];
        if(block) {
            dynarec_log(LOG_VERBOSE, "Block already exist in Direct Map\n");
            *created = 0;
            return block;
        }
    }
    
    if (!*created)
        return block;
    
    pthread_mutex_lock(&my_context->mutex_dyndump);
    if(!dynablocks->direct) {
        dynablock_t** p = (dynablock_t**)calloc(dynablocks->textsz, sizeof(dynablock_t*));
        if(native_lock_storeifnull(&dynablocks->direct, p)!=p)
            free(p);    // someone already create the direct array, too late...
    }

    // create and add new block
    dynarec_log(LOG_VERBOSE, "Ask for DynaRec Block creation @%p\n", (void*)addr);

    block = (dynablock_t*)calloc(1, sizeof(dynablock_t));
    block->parent = dynablocks; 
    dynablock_t* tmp = (dynablock_t*)native_lock_storeifnull(&dynablocks->direct[addr-dynablocks->text], block);
    if(tmp !=  block) {
        // a block appeard!
        pthread_mutex_unlock(&my_context->mutex_dyndump);
        free(block);
        *created = 0;
        return tmp;
    }

    *created = 1;
    pthread_mutex_lock(&my_context->mutex_dyndump);
    return block;
}

//TODO: move this to dynrec_arm.c and track allocated structure to avoid memory leak
static __thread struct __jmp_buf_tag dynarec_jmpbuf;

void cancelFillBlock()
{
    longjmp(&dynarec_jmpbuf, 1);
}

/* 
    return NULL if block is not found / cannot be created. 
    Don't create if create==0
*/
static dynablock_t* internalDBGetBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr, int create, dynablock_t* current, int need_lock)
{
    // try the quickest way first: get parent of current and check if ok!
    dynablocklist_t *dynablocks = NULL;
    dynablock_t* block = NULL;
    if(current && current->done && !current->gone) {
        dynablocks = current->parent;
        if(dynablocks && !(addr>=dynablocks->text && addr<(dynablocks->text+dynablocks->textsz)))
            dynablocks = NULL;
    }
    // nope, lets do the long way
    if(!dynablocks) {
        dynablocks = getDBFromAddress(addr);
        if(!dynablocks) {
            dynablocks = GetDynablocksFromAddress(emu->context, addr);
            if(!dynablocks)
                return NULL;
        }
    }
    // check direct first, without lock
    if(dynablocks->direct/* && (addr>=dynablocks->text) && (addr<(dynablocks->text+dynablocks->textsz))*/)
        if((block = dynablocks->direct[addr-dynablocks->text]))
            return block;

    int created = create;
    block = AddNewDynablock(dynablocks, addr, &created);
    if(!created)
        return block;   // existing block...

    // fill the block
    block->x64_addr = (void*)addr;
    if(need_lock)
        pthread_mutex_lock(&my_context->mutex_dyndump);
    if(sigsetjmp(&dynarec_jmpbuf, 1)) {
        printf_log(LOG_INFO, "FillBlock at %p triggered a segfault, cancelling\n", (void*)addr);
        if(need_lock)
            pthread_mutex_unlock(&my_context->mutex_dyndump);
        return NULL;
    }
    void* ret = FillBlock64(block, filladdr);
    if(need_lock)
        pthread_mutex_unlock(&my_context->mutex_dyndump);
    if(!ret) {
        dynarec_log(LOG_DEBUG, "Fillblock of block %p for %p returned an error\n", block, (void*)addr);
        void* old = (void*)native_lock_storeifref(&dynablocks->direct[addr-dynablocks->text], 0, block);
        if(old!=block && old) {// put it back in place, strange things are happening here!
            dynarec_log(LOG_INFO, "Warning, a wild block appeared at %p: %p\n", (void*)addr, old);
            // doing nothing else, the block has not be writen
        }
        free(block);
        block = NULL;
    }
    // check size
    if(block && block->x64_size) {
        if(dynablocks->minstart>addr)
            dynablocks->minstart = addr;
        int blocksz = block->x64_size;
        if(dynablocks->maxend<addr+blocksz) {
            dynablocks->maxend = addr+blocksz;
            for(unsigned idx=(addr>>DYNAMAP_SHIFT)+1; idx<=((addr+blocksz-1)>>DYNAMAP_SHIFT); ++idx) {
                dynablocklist_t* dblist;
                if((dblist = getDB(idx)))
                    if(dblist->minstart>addr)
                        dblist->minstart = addr;
            }
        }
        // fill-in jumptable
        addJumpTableIfDefault64(block->x64_addr, block->block);
        for(int i=0; i<block->sons_size; ++i) {
            addJumpTableIfDefault64(block->sons[i]->x64_addr, block->sons[i]->block);
            block->sons[i]->done = 1;
        }
        block->done = 1;
    }

    dynarec_log(LOG_DEBUG, "%04d| --- DynaRec Block %s @%p:%p (%p, 0x%x bytes, with %d son(s))\n", GetTID(), created?"created":"recycled", (void*)addr, (void*)(addr+((block)?block->x64_size:1)-1), (block)?block->block:0, (block)?block->size:0, (block)?block->sons_size:0);

    return block;
}

#define MAX_HOTPAGE 64
#define HOTPAGE_STEP 64
static int volatile hotpage_count[MAX_HOTPAGE] = {0};
static uintptr_t volatile hotpage[MAX_HOTPAGE] = {0};
static uintptr_t volatile hotpage_size[MAX_HOTPAGE] = {0};
static volatile int hotpages = 0;

int IsInHotPage(uintptr_t addr) {
    if(!hotpages)
        return 0;
    for(int i=0; i<MAX_HOTPAGE; ++i) {
        if((hotpage_count[i]>0) && (addr>=hotpage[i]) && (addr<hotpage[i]+0x1000*(hotpage_size[i]+1))) {
            --hotpage_count[i];
            if(!hotpage_count[i]) {
                --hotpages;
                hotpage_size[i] = 0;
                dynarec_log(LOG_DEBUG, "End of Hotpage %p\n", (void*)hotpage[i]);
            }
            __sync_synchronize();
            return 1;
        }
    }
    return 0;
}

int AreaInHotPage(uintptr_t start, uintptr_t end) {
    if(!hotpages)
        return 0;
    for(int i=0; i<MAX_HOTPAGE; ++i) {
        if(hotpage_count[i]>0)
            if(IntervalIntersects(start, end, hotpage[i], hotpage[i]+0x1000*(hotpage_size[i]+1)-1)) {
                --hotpage_count[i];
                if(!hotpage_count[i]) {
                    --hotpages;
                    hotpage_size[i] = 0;
                    dynarec_log(LOG_DEBUG, "End of Hotpage %p\n", (void*)hotpage[i]);
                }
                return 1;
        }
    }
    return 0;
}

void FuseHotPage(int idx) {
    uintptr_t start = hotpage[idx];
    uintptr_t end = start+0x1000*(hotpage_size[idx]+1);
    for(int i=0; i<MAX_HOTPAGE; ++i)
        if(i!=idx && hotpage_count[i]>0) {
            if(IntervalIntersects(start, end, hotpage[i], hotpage[i]+0x1000*(hotpage_size[i]+1)-1)) {
                if(hotpage_count[i]>hotpage_count[idx])
                    hotpage_count[idx] = hotpage_count[i];
                if(hotpage[i]>hotpage[idx])
                    hotpage[idx]=hotpage[i];
                if(hotpage[i]+0x1000*(hotpage_size[i]+1)>end)
                    hotpage_size[idx] = ((hotpage[i]+0x1000*(hotpage_size[i]+1))-hotpage[idx])/0x1000 - 1;
                hotpage_count[i] = 0;
                return;
            }
        }
}

void AddHotPage(uintptr_t addr) {
    addr&=~0xfff;
    // look for same address
    for(int i=0; i<MAX_HOTPAGE; ++i) {
        if(addr>=hotpage[i] && addr<hotpage[i]+0x1000*(hotpage_size[i]+1)) {
            if(!hotpage_count[i])
                ++hotpages;
            hotpage_count[i] = HOTPAGE_STEP;
            __sync_synchronize();
            return;
        }
        if(addr==hotpage[i]+0x1000*(hotpage_size[i]+1)) {
            ++hotpage_size[i];
            hotpage_count[i] = HOTPAGE_STEP;
            FuseHotPage(i);
            __sync_synchronize();
            return;
        }
        if(addr+0x1000==hotpage[i]) {
            ++hotpage_size[i];
            hotpage[i] = addr;
            hotpage_count[i] = HOTPAGE_STEP;
            __sync_synchronize();
            return;
        }
        if(addr==hotpage[i]+0x1000*(hotpage_size[i]+2)) {
            hotpage_size[i]+=2;
            hotpage_count[i] = HOTPAGE_STEP;
            FuseHotPage(i);
            __sync_synchronize();
            return;
        }
        if(addr+0x2000==hotpage[i]) {
            hotpage_size[i]+=2;
            hotpage[i] = addr;
            hotpage_count[i] = HOTPAGE_STEP;
            FuseHotPage(i);
            __sync_synchronize();
            return;
        }
    }
    // look for empty spot / minium
    int mincnt = hotpage_count[0]*(hotpage_size[0]+1);
    int minidx = 0;
    for(int i=1; i<MAX_HOTPAGE; ++i)
        if((hotpage_count[i]*(hotpage_size[i]+1))<mincnt) {
            mincnt = (hotpage_count[i]*(hotpage_size[i]+1));
            minidx = i;
        }
    if(hotpage_count[minidx]) {
        static int cnt = 0;
        if(cnt<50) {
            dynarec_log(LOG_NONE, "Warning, not enough Hotpage, replacing %p(%p/%d) with %p\n", (void*)hotpage[minidx], (void*)(0x1000*(hotpage_size[minidx]+1)), hotpage_count[minidx], (void*)addr);
            ++cnt;
            if(cnt==50)   // stop spamming console with message...
                dynarec_log(LOG_NONE, "    will stop warning about not enough Hotpage now\n");
        }
        hotpage_size[minidx] = 0;
    } else
        ++hotpages;
    hotpage[minidx] = addr;
    hotpage_count[minidx] = HOTPAGE_STEP;
    __sync_synchronize();
}

dynablock_t* DBGetBlock(x64emu_t* emu, uintptr_t addr, int create, dynablock_t** current)
{
    dynablock_t *db = internalDBGetBlock(emu, addr, addr, create, *current, 1);
    dynablock_t *father = (db && db->father)?db->father:db;
    if(father && father->done && db->block && father->need_test) {
        if(pthread_mutex_trylock(&my_context->mutex_dyndump)) {
            dynarec_log(LOG_DEBUG, "mutex_dyndump not available when trying to validate block %p from %p:%p (hash:%X) with %d son(s) for %p\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, db->hash, db->sons_size, (void*)addr);
            return NULL;
        }
        if(AreaInHotPage((uintptr_t)father->x64_addr, (uintptr_t)father->x64_addr + father->x64_size - 1)) {
            dynarec_log(LOG_DEBUG, "Not running block %p from %p:%p with %d son(s) for %p because it's in a hotpage\n", father, father->x64_addr, father->x64_addr+father->x64_size-1, father->sons_size, (void*)addr);
            pthread_mutex_unlock(&my_context->mutex_dyndump);
            return NULL;
        }
        uint32_t hash = X31_hash_code(father->x64_addr, father->x64_size);
        if(hash!=father->hash) {
            father->done = 0;   // invalidating the block
            dynarec_log(LOG_DEBUG, "Invalidating block %p from %p:%p (hash:%X/%X) with %d son(s) for %p\n", father, father->x64_addr, father->x64_addr+father->x64_size-1, hash, father->hash, father->sons_size, (void*)addr);
            // no more current if it gets invalidated too
            if(*current && IntervalIntersects(
             (uintptr_t)father->x64_addr, 
             (uintptr_t)father->x64_addr+father->x64_size-1, 
             (uintptr_t)(*current)->x64_addr, 
             (uintptr_t)(*current)->x64_addr+(*current)->x64_size-1))
                *current = NULL;
            // Free father, it's now invalid!
            FreeDynablock(father, 0);
            // start again... (will create a new block)
            db = internalDBGetBlock(emu, addr, addr, create, *current, 0);
        } else {
            father->need_test = 0;
            dynarec_log(LOG_DEBUG, "Validating block %p from %p:%p (hash:%X) with %d son(s) for %p\n", father, father->x64_addr, father->x64_addr+father->x64_size-1, father->hash, father->sons_size, (void*)addr);
            protectDB((uintptr_t)father->x64_addr, father->x64_size);
            // fill back jumptable
            addJumpTableIfDefault64(father->x64_addr, father->block);
            for(int i=0; i<father->sons_size; ++i)
                addJumpTableIfDefault64(father->sons[i]->x64_addr, father->sons[i]->block);
        }
        pthread_mutex_unlock(&my_context->mutex_dyndump);
    } 
    return db;
}

dynablock_t* DBAlternateBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr)
{
    dynarec_log(LOG_DEBUG, "Creating AlternateBlock at %p for %p\n", (void*)addr, (void*)filladdr);
    int create = 1;
    dynablock_t *db = internalDBGetBlock(emu, addr, filladdr, create, NULL, 1);
    dynablock_t *father = (db && db->father)?db->father:db;
    if(father && father->done && db->block && father->need_test) {
        if(pthread_mutex_trylock(&my_context->mutex_dyndump))
            return NULL;
        uint32_t hash = X31_hash_code(father->x64_addr, father->x64_size);
        if(hash!=father->hash) {
            father->done = 0;   // invalidating the block
            dynarec_log(LOG_DEBUG, "Invalidating alt block %p from %p:%p (hash:%X/%X) with %d son(s) for %p\n", father, father->x64_addr, father->x64_addr+father->x64_size, hash, father->hash, father->sons_size, (void*)addr);
            // Free father, it's now invalid!
            FreeDynablock(father, 0);
            // start again... (will create a new block)
            db = internalDBGetBlock(emu, addr, filladdr, create, NULL, 0);
        } else {
            father->need_test = 0;
            protectDB((uintptr_t)father->x64_addr, father->x64_size);
            // fill back jumptable
            addJumpTableIfDefault64(father->x64_addr, father->block);
            for(int i=0; i<father->sons_size; ++i)
                addJumpTableIfDefault64(father->sons[i]->x64_addr, father->sons[i]->block);
        }
        pthread_mutex_unlock(&my_context->mutex_dyndump);
    } 
    return db;
}
