#include <string.h>

#include "debug.h"
#include "bridge.h"
#include "box64context.h"
#include "box64cpu.h"
#include "custommem.h"
#include "dynablock.h"
#include "../dynablock_private.h"

#include "dynarec_la64_private.h"
#include "dynarec_la64_native.h"

la64_native_call_t la64_get_native_call(uintptr_t addr)
{
    if (box64_is32bits) return LA64_NATIVE_NONE;

    const char* name = getBridgeName((void*)addr);
    if (!name)
        return LA64_NATIVE_NONE;
    if (!strcmp(name, "memcmp"))
        return LA64_NATIVE_MEMCMP;
    if (!strcmp(name, "memcpy"))
        return LA64_NATIVE_MEMCPY;
    if (!strcmp(name, "memmove"))
        return LA64_NATIVE_MEMMOVE;
    if (!strcmp(name, "memset"))
        return LA64_NATIVE_MEMSET;
    if (!strcmp(name, "memchr"))
        return LA64_NATIVE_MEMCHR;
    if (!strcmp(name, "strcmp"))
        return LA64_NATIVE_STRCMP;
    if (!strcmp(name, "strlen"))
        return LA64_NATIVE_STRLEN;
    return LA64_NATIVE_NONE;
}

int la64_native_call_writes_memory(la64_native_call_t call)
{
    return call == LA64_NATIVE_MEMCPY || call == LA64_NATIVE_MEMMOVE || call == LA64_NATIVE_MEMSET;
}

int isInlinableNativeCall(uintptr_t addr)
{
    return la64_get_native_call(addr) != LA64_NATIVE_NONE;
}

void la64_native_call_pass2(dynarec_la64_t* dyn, la64_native_call_t call);
void la64_native_call_pass3(dynarec_la64_t* dyn, la64_native_call_t call);

static dynablock_t* native_call_blocks[LA64_NATIVE_LAST] = { NULL };

static uint8_t dummy_code[] = { 0x90, 0xc3 };

void* create_native_call(la64_native_call_t call)
{
    if (call <= LA64_NATIVE_NONE || call >= LA64_NATIVE_LAST)
        return NULL;
    if (native_call_blocks[call])
        return native_call_blocks[call]->block;

    dynarec_la64_t helper = { 0 };
    instruction_la64_t insts[1] = { 0 };
    helper.insts = insts;
    helper.need_dump = BOX64ENV(dynarec_dump) == 3 ? 0 : BOX64ENV(dynarec_dump);
    helper.cap = 1;

    la64_native_call_pass2(&helper, call);

    size_t native_size = (helper.native_size + 7) & ~7;
    size_t sz = sizeof(void*) + native_size + helper.table64size * sizeof(uint64_t) + 4 * sizeof(void*) + 0 + 0 + 0 + sizeof(dynablock_t) + 0;
    //           dynablock_t*     block (native insts)        table64               jmpnext code instsize arch callrets     dynablock      relocs
    void* actual_p = (void*)AllocDynarecMap((uintptr_t)&dummy_code, sz, 1);
    if (actual_p == NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%zu) failed, canceling NativeCall block\n", sz);
        return NULL;
    }
    void* p = (void*)(((uintptr_t)actual_p) + sizeof(void*));
    void* tablestart = p + native_size;
    void* next = tablestart + helper.table64size * sizeof(uint64_t);
    void* instsize = next + 4 * sizeof(void*);
    void* callrets = instsize;

    helper.block = p;
    dynablock_t* block = (dynablock_t*)(callrets + 0);
    memset(block, 0, sizeof(dynablock_t));
    void* relocs = helper.need_reloc ? (block + 1) : NULL;
    // fill the block
    block->x64_addr = &dummy_code;
    block->actual_block = actual_p;
    helper.relocs = relocs;
    block->relocs = relocs;
    block->table64size = helper.table64size;
    helper.native_start = (uintptr_t)p;
    helper.tablestart = (uintptr_t)tablestart;
    helper.jmp_next = (uintptr_t)next + sizeof(void*);
    helper.instsize = (instsize_t*)instsize;
    *(dynablock_t**)actual_p = block;
    helper.table64cap = helper.table64size;
    helper.table64 = (uint64_t*)helper.tablestart;
    helper.callrets = (callret_t*)callrets;
    block->table64 = helper.table64;

    if (helper.need_dump) {
        dynarec_log(LOG_NONE, "%s%04d|Emitting %zu bytes for NativeCall (%p)", (helper.need_dump > 1) ? "\e[01;36m" : "", GetTID(), helper.native_size, helper.native_start);
        dynarec_log_prefix(0, LOG_NONE, "%s\n", (helper.need_dump > 1) ? "\e[m" : "");
    }

    helper.native_size = 0;
    helper.lsx = helper.insts[0].lsx;
    la64_native_call_pass3(&helper, call);
    helper.jmp_sz = helper.jmp_cap = 0;
    helper.jmps = NULL;

    block->instsize = instsize;
    helper.table64 = NULL;
    helper.instsize = NULL;
    helper.predecessor = NULL;
    block->size = sz;
    block->isize = helper.size;
    block->block = p;
    block->jmpnext = next + sizeof(void*);
    block->always_test = helper.always_test;
    block->dirty = block->always_test;
    block->is32bits = 0;
    block->relocsize = helper.reloc_size * sizeof(uint32_t);
    block->arch = NULL;
    block->arch_size = 0;
    block->callret_size = helper.callret_size;
    block->callrets = helper.callrets;
    block->native_size = native_size;
    *(dynablock_t**)next = block;
    *(void**)(next + 3 * sizeof(void*)) = NULL;
    CreateJmpNext(block->jmpnext, next + 3 * sizeof(void*));
    ClearCache(block->jmpnext, 4 * sizeof(void*));
    block->x64_size = 0;
    ClearCache(actual_p + sizeof(void*), native_size);

    native_call_blocks[call] = block;
    return block->block;
}
