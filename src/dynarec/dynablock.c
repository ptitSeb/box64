#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/mman.h>

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

void FreeDynablock(dynablock_t* db, int need_lock)
{
    if(db) {
        if(db->gone)
            return; // already in the process of deletion!
        dynarec_log(LOG_DEBUG, "FreeDynablock(%p), db->block=%p x64=%p:%p already gone=%d\n", db, db->block, db->x64_addr, db->x64_addr+db->x64_size-1, db->gone);
        if(need_lock)
            pthread_mutex_lock(&my_context->mutex_dyndump);
        // remove jumptable
        setJumpTableDefault64(db->x64_addr);
        dynarec_log(LOG_DEBUG, " -- FreeDyrecMap(%p, %d)\n", db->actual_block, db->size);
        db->done = 0;
        db->gone = 1;
        FreeDynarecMap(db, (uintptr_t)db->actual_block, db->size);
        customFree(db->instsize);
        customFree(db);
        if(need_lock)
            pthread_mutex_unlock(&my_context->mutex_dyndump);
    }
}

void MarkDynablock(dynablock_t* db)
{
    if(db) {
        if(db->need_test)
            return; // already done
        dynarec_log(LOG_DEBUG, "MarkDynablock %p %p-%p\n", db, db->x64_addr, db->x64_addr+db->x64_size-1);
        db->need_test = 1;
        setJumpTableIfRef64(db->x64_addr, db->jmpnext, db->block);
    }
}

int IntervalIntersects(uintptr_t start1, uintptr_t end1, uintptr_t start2, uintptr_t end2)
{
    if(start1 > end2 || start2 > end1)
        return 0;
    return 1;
}

static int MarkedDynablock(dynablock_t* db)
{
    if(db) {
        if(db->need_test)
            return 1; // already done
    }
    return 0;
}

void MarkRangeDynablock(dynablock_t* db, uintptr_t addr, uintptr_t size)
{
    // Mark will try to find *any* blocks that intersect the range to mark
    if(!db)
        return;
    dynarec_log(LOG_DEBUG, "MarkRangeDynablock %p-%p .. startdb=%p, sizedb=%p\n", (void*)addr, (void*)addr+size-1, (void*)db->x64_addr, (void*)db->x64_size);
    if(!MarkedDynablock(db))
        if(IntervalIntersects((uintptr_t)db->x64_addr, (uintptr_t)db->x64_addr+db->x64_size-1, addr, addr+size+1))
            MarkDynablock(db);
}

int FreeRangeDynablock(dynablock_t* db, uintptr_t addr, uintptr_t size)
{
    if(!db)
        return 1;

    int need_lock = my_context?1:0;
    if(IntervalIntersects((uintptr_t)db->x64_addr, (uintptr_t)db->x64_addr+db->x64_size-1, addr, addr+size+1)) {
        FreeDynablock(db, need_lock);
        return 0;
    }
    return 1;
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
            return db;
    )
    return NULL;
}

dynablock_t *AddNewDynablock(uintptr_t addr)
{
    dynablock_t* block;
    #if 0
    // check if memory as the correct flags
    int prot = getProtection(addr);
    if(!(prot&(PROT_EXEC|PROT_DYNAREC|PROT_DYNAREC_R))) {
        dynarec_log(LOG_VERBOSE, "Block asked on a memory with no execution flags 0x%02X\n", prot);
        return NULL;
    }
    
    #endif
    pthread_mutex_lock(&my_context->mutex_dyndump);

    // create and add new block
    dynarec_log(LOG_VERBOSE, "Ask for DynaRec Block creation @%p\n", (void*)addr);

    block = (dynablock_t*)customCalloc(1, sizeof(dynablock_t));

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
static dynablock_t* internalDBGetBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr, int create, int need_lock)
{
    dynablock_t* block = getDB(addr);
    if(block || !create)
        return block;

    block = AddNewDynablock(addr);

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
        customFree(block);
        block = NULL;
    }
    // check size
    if(block && (block->x64_size || (!block->x64_size && !block->done))) {
        int blocksz = block->x64_size;
        if(blocksz>my_context->max_db_size)
            my_context->max_db_size = blocksz;
        // fill-in jumptable
        if(!addJumpTableIfDefault64(block->x64_addr, block->block)) {
            FreeDynablock(block, 1);
            block = getDB(addr);
        } else {
            if(block->x64_size)
                block->done = 1;    // don't validate the block if the size is null, but keep the block
        }
    }

    dynarec_log(LOG_DEBUG, "%04d| --- DynaRec Block created @%p:%p (%p, 0x%x bytes)\n", GetTID(), (void*)addr, (void*)(addr+((block)?block->x64_size:1)-1), (block)?block->block:0, (block)?block->size:0);

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

dynablock_t* DBGetBlock(x64emu_t* emu, uintptr_t addr, int create)
{
    dynablock_t *db = internalDBGetBlock(emu, addr, addr, create, 1);
    if(db && db->done && db->block && db->need_test) {
        if(AreaInHotPage((uintptr_t)db->x64_addr, (uintptr_t)db->x64_addr + db->x64_size - 1)) {
            dynarec_log(LOG_DEBUG, "Not running block %p from %p:%p with for %p because it's in a hotpage\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, (void*)addr);
            return NULL;
        }
        uint32_t hash = X31_hash_code(db->x64_addr, db->x64_size);
        if(pthread_mutex_trylock(&my_context->mutex_dyndump)) {
            dynarec_log(LOG_DEBUG, "mutex_dyndump not available when trying to validate block %p from %p:%p (hash:%X) for %p\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, db->hash, (void*)addr);
            return NULL;
        }
        if(hash!=db->hash) {
            db->done = 0;   // invalidating the block
            dynarec_log(LOG_DEBUG, "Invalidating block %p from %p:%p (hash:%X/%X) for %p\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, hash, db->hash, (void*)addr);
            // Free db, it's now invalid!
            FreeDynablock(db, 0);
            // start again... (will create a new block)
            db = internalDBGetBlock(emu, addr, addr, create, 0);
        } else {
            db->need_test = 0;
            dynarec_log(LOG_DEBUG, "Validating block %p from %p:%p (hash:%X) for %p\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, db->hash, (void*)addr);
            protectDB((uintptr_t)db->x64_addr, db->x64_size);
            // fill back jumptable
            setJumpTableIfRef64(db->x64_addr, db->block, db->jmpnext);
        }
        pthread_mutex_unlock(&my_context->mutex_dyndump);
    } 
    return db;
}

dynablock_t* DBAlternateBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr)
{
    dynarec_log(LOG_DEBUG, "Creating AlternateBlock at %p for %p\n", (void*)addr, (void*)filladdr);
    int create = 1;
    dynablock_t *db = internalDBGetBlock(emu, addr, filladdr, create, 1);
    if(db && db->done && db->block && db->need_test) {
        if(pthread_mutex_trylock(&my_context->mutex_dyndump))
            return NULL;
        uint32_t hash = X31_hash_code(db->x64_addr, db->x64_size);
        if(hash!=db->hash) {
            db->done = 0;   // invalidating the block
            dynarec_log(LOG_DEBUG, "Invalidating alt block %p from %p:%p (hash:%X/%X) for %p\n", db, db->x64_addr, db->x64_addr+db->x64_size, hash, db->hash, (void*)addr);
            // Free db, it's now invalid!
            FreeDynablock(db, 0);
            // start again... (will create a new block)
            db = internalDBGetBlock(emu, addr, filladdr, create, 0);
        } else {
            db->need_test = 0;
            protectDB((uintptr_t)db->x64_addr, db->x64_size);
            // fill back jumptable
            addJumpTableIfDefault64(db->x64_addr, db->block);
        }
        pthread_mutex_unlock(&my_context->mutex_dyndump);
    } 
    return db;
}
