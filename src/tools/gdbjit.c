#ifdef GDBJIT
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <errno.h>
#include "gdbjit.h"
#include "dynablock.h"
#include "debug.h"


/* GDB JIT Compilation Interface -----------------------------------------------
 * https://sourceware.org/gdb/current/onlinedocs/gdb.html/JIT-Interface.html
 */

enum {
    GDBJIT_NOACTION = 0,
    GDBJIT_REGISTER,
    GDBJIT_UNREGISTER
};

typedef struct gdbjit_code_entry_s {
    struct gdbjit_code_entry_s* next_entry;
    struct gdbjit_code_entry_s* prev_entry;
    const char* symfile_addr;
    uint64_t symfile_size;
} gdbjit_code_entry_t;

typedef struct gdbjit_descriptor_s {
    uint32_t version;
    uint32_t action_flag;
    gdbjit_code_entry_t* relevant_entry;
    gdbjit_code_entry_t* first_entry;
} gdbjit_descriptor_t;

/* GDB puts a breakpoint in this function. This can't be optimized out. */
EXPORT void __attribute__((noinline)) __jit_debug_register_code()
{
    asm volatile("" ::: "memory");
};

/* Make sure to specify the version statically, because the debugger may check
 * the version before we can set it.
 */
EXPORT gdbjit_descriptor_t __jit_debug_descriptor = { 1, GDBJIT_NOACTION, NULL, NULL };

/* --------------------------------------------------------------------------- */

void GdbJITNewBlock(gdbjit_block_t* block, GDB_CORE_ADDR start, GDB_CORE_ADDR end, uintptr_t x64start)
{
    if (!block) return;

    size_t alloced = block->alloced;
    memset(block, 0, sizeof(gdbjit_block_t));

    strcpy(block->filename, "/tmp/box64gdbjit-XXXXXX.S");
    int fd = mkstemps(block->filename, 2);
    block->file = fdopen(fd, "w");

    block->start = start;
    block->end = end;
    block->x64start = x64start;
    block->alloced = alloced;
    block->nlines = 0;
}

static size_t GdbJITLinesAvailable(gdbjit_block_t* block)
{
    if (!block) return 0;

    return block->alloced > block->nlines;
}

#define GDBJIT_LINES_MAX_PREALLOC 64

static gdbjit_block_t* GdbJITMakeRoom(gdbjit_block_t* block)
{
    if (!block) return NULL;

    if (!GdbJITLinesAvailable(block)) {
        size_t new_size = block->alloced + GDBJIT_LINES_MAX_PREALLOC;
        block = box_realloc(block, sizeof(gdbjit_block_t) + new_size * sizeof(struct gdb_line_mapping));
        if (!block) return NULL;
        block->alloced = new_size;
    }
    return block;
}

gdbjit_block_t* GdbJITBlockAddLine(gdbjit_block_t* block, GDB_CORE_ADDR addr, const char* line)
{
    if (!block || !block->file) return NULL;

    block->nlines++;
    block = GdbJITMakeRoom(block);
    block->lines[block->nlines-1].pc = addr;
    block->lines[block->nlines-1].line = block->nlines;
    fprintf(block->file, "%s\n", line);
    return block;
}

void GdbJITBlockCleanup(gdbjit_block_t* block) {
    if (block && block->file) {
        fclose(block->file);
        block->file = NULL;
    }
}

void GdbJITBlockReady(gdbjit_block_t* block)
{
    if (!block) return;

    if (block->nlines == 0) return;

    gdbjit_code_entry_t* entry = (gdbjit_code_entry_t*)box_malloc(sizeof(gdbjit_code_entry_t));
    if (!entry) return;

    entry->symfile_addr = (const char*)block;
    entry->symfile_size = sizeof(gdbjit_block_t) + block->nlines * sizeof(struct gdb_line_mapping);

    if (__jit_debug_descriptor.first_entry) {
        __jit_debug_descriptor.relevant_entry->next_entry = entry;
        entry->prev_entry = __jit_debug_descriptor.relevant_entry;
    } else {
        __jit_debug_descriptor.first_entry = entry;
    }

    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = GDBJIT_REGISTER;
    __jit_debug_register_code();
}
#endif
