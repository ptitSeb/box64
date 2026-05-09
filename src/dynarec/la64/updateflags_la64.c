#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "custommem.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynablock.h"
#include "../dynablock_private.h"

#include "dynarec_native.h"
#include "../dynarec_arch.h"

void updateflags_pass0(dynarec_la64_t* dyn, uint64_t jmp_df[]);
void updateflags_pass1(dynarec_la64_t* dyn, uint64_t jmp_df[]);
void updateflags_pass2(dynarec_la64_t* dyn, uint64_t jmp_df[]);
void updateflags_pass3(dynarec_la64_t* dyn, uint64_t jmp_df[]);

static dynablock_t* updateflags_la64 = NULL;

static uint8_t dummy_code[] = {0x90, 0xc3};

void* create_updateflags()
{
    if (updateflags_la64)
        return updateflags_la64->block;

    uint64_t jmp_df[d_unknown + 1] = {0};
    dynarec_la64_t helper = {0};
    instruction_la64_t insts[1] = {0};
    helper.insts = insts;
    helper.need_dump = BOX64ENV(dynarec_dump);
    helper.cap = 1;
    helper.f = status_none;
    helper.insts[0].x64.gen_flags = X_ALL;

    updateflags_pass0(&helper, jmp_df);
    int ok = 1;
    for (int i = d_none; i < d_unknown; ++i)
        if (!jmp_df[i]) {
            printf_log(LOG_NONE, "Error, UpdateFlags case %d is not handled, will crash later\n", i);
            ok = 0;
        }

    updateflags_pass1(&helper, jmp_df);
    helper.native_size = 0;
    updateflags_pass2(&helper, jmp_df);

    size_t native_size = (helper.native_size + 7) & ~7;
    size_t sz = sizeof(void*) + native_size + helper.table64size*sizeof(uint64_t) + 4*sizeof(void*) +  0  +  0  +  0  + sizeof(dynablock_t) + 0;
    //           dynablock_t*     block (arm insts)            table64               jmpnext code instsize arch callrets     dynablock      relocs
    void* actual_p = (void*)AllocDynarecMap((uintptr_t)&dummy_code, sz, 1);
    void* p = (void*)(((uintptr_t)actual_p) + sizeof(void*));
    void* tablestart = p + native_size;
    void* next = tablestart + helper.table64size * sizeof(uint64_t);
    void* instsize = next + 4 * sizeof(void*);
    void* callrets = instsize;
    if (actual_p == NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%zu) failed, canceling UpdateBlock\n", sz);
        return NULL;
    }

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
        dynarec_log(LOG_NONE, "%s%04d|Emitting %zu bytes for UpdateFlags (%p)", (helper.need_dump > 1) ? "\e[01;36m" : "", GetTID(), helper.native_size, helper.native_start);
        dynarec_log_prefix(0, LOG_NONE, "%s\n", (helper.need_dump > 1) ? "\e[m" : "");
    }

    helper.native_size = 0;
    updateflags_pass3(&helper, jmp_df);
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

    updateflags_la64 = block;
    return block->block;
}
