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
#include "bridge.h"

#include "dynarec_native.h"
#include "native_lock.h"

#include "custommem.h"
#include "khash.h"

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
            mutex_lock(&my_context->mutex_dyndump);
        // remove jumptable
        setJumpTableDefault64(db->x64_addr);
        dynarec_log(LOG_DEBUG, " -- FreeDyrecMap(%p, %d)\n", db->actual_block, db->size);
        db->done = 0;
        db->gone = 1;
        FreeDynarecMap((uintptr_t)db->actual_block);
        customFree(db);
        if(need_lock)
            mutex_unlock(&my_context->mutex_dyndump);
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
    // create and add new block
    dynarec_log(LOG_VERBOSE, "Ask for DynaRec Block creation @%p\n", (void*)addr);
    block = (dynablock_t*)customCalloc(1, sizeof(dynablock_t));
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
    if(hasAlternate((void*)addr))
        return NULL;
    dynablock_t* block = getDB(addr);
    if(block || !create)
        return block;

    if(need_lock) {
        if(box64_dynarec_wait) {
            mutex_lock(&my_context->mutex_dyndump);
        } else {
            if(mutex_trylock(&my_context->mutex_dyndump))   // FillBlock not available for now
                return NULL;
        }
    }
    
    block = getDB(addr);    // just in case
    if(block) {
        if(need_lock)
            mutex_unlock(&my_context->mutex_dyndump);
        return block;
    }
    
    block = AddNewDynablock(addr);

    // fill the block
    block->x64_addr = (void*)addr;
    if(sigsetjmp(&dynarec_jmpbuf, 1)) {
        printf_log(LOG_INFO, "FillBlock at %p triggered a segfault, cancelling\n", (void*)addr);
        if(need_lock)
            mutex_unlock(&my_context->mutex_dyndump);
        return NULL;
    }
    void* ret = FillBlock64(block, filladdr);
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
            FreeDynablock(block, 0);
            block = getDB(addr);
        } else {
            if(block->x64_size)
                block->done = 1;    // don't validate the block if the size is null, but keep the block
        }
    }
    if(need_lock)
        mutex_unlock(&my_context->mutex_dyndump);

    dynarec_log(LOG_DEBUG, "%04d| --- DynaRec Block created @%p:%p (%p, 0x%x bytes)\n", GetTID(), (void*)addr, (void*)(addr+((block)?block->x64_size:1)-1), (block)?block->block:0, (block)?block->size:0);

    return block;
}

dynablock_t* DBGetBlock(x64emu_t* emu, uintptr_t addr, int create)
{
    dynablock_t *db = internalDBGetBlock(emu, addr, addr, create, 1);
    if(db && db->done && db->block && db->need_test) {
        if(AreaInHotPage((uintptr_t)db->x64_addr, (uintptr_t)db->x64_addr + db->x64_size - 1)) {
            if(box64_dynarec_fastpage) {
                uint32_t hash = X31_hash_code(db->x64_addr, db->x64_size);
                if(hash==db->hash)  // seems ok, run it without reprotecting it
                    return db;
                db->done = 0;   // invalidating the block, it's already not good
                dynarec_log(LOG_DEBUG, "Invalidating block %p from %p:%p (hash:%X/%X) for %p\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, hash, db->hash, (void*)addr);
                // Free db, it's now invalid!
                FreeDynablock(db, 1);
                return NULL;    // not building a new one, it's still a hotpage
            } else {
                dynarec_log(LOG_INFO, "Not running block %p from %p:%p with for %p because it's in a hotpage\n", db, db->x64_addr, db->x64_addr+db->x64_size-1, (void*)addr);
                return NULL;
            }
        }
        uint32_t hash = X31_hash_code(db->x64_addr, db->x64_size);
        if(mutex_trylock(&my_context->mutex_dyndump)) {
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
        mutex_unlock(&my_context->mutex_dyndump);
    } 
    return db;
}

dynablock_t* DBAlternateBlock(x64emu_t* emu, uintptr_t addr, uintptr_t filladdr)
{
    dynarec_log(LOG_DEBUG, "Creating AlternateBlock at %p for %p\n", (void*)addr, (void*)filladdr);
    int create = 1;
    dynablock_t *db = internalDBGetBlock(emu, addr, filladdr, create, 1);
    if(db && db->done && db->block && db->need_test) {
        if(mutex_trylock(&my_context->mutex_dyndump))
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
        mutex_unlock(&my_context->mutex_dyndump);
    } 
    return db;
}
