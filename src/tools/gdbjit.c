// SPDX-License-Identifier: MIT
#ifdef GDBJIT
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <elf.h>
#include <errno.h>
#include <inttypes.h>
#include "gdbjit.h"
#include "debug.h"
#include "elfloader.h"


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

struct gdbjit_block_s {
    gdbjit_symfile_t* symfile;
    size_t alloced;
    uint32_t filelines;
    int registered;
    pid_t owner;
    FILE* file;
    gdbjit_code_entry_t* entry;
    struct gdbjit_block_s* next_alloc;
    struct gdbjit_block_s* prev_alloc;
};

#define GDBJIT_LINES_MAX_PREALLOC 64
#define GDBJIT_MAX_LINES          (1 << 24)

static pthread_mutex_t gdbjit_mutex = PTHREAD_MUTEX_INITIALIZER;
static __thread int gdbjit_in_lock = 0;
static gdbjit_code_entry_t* gdbjit_first_entry = NULL;
static gdbjit_code_entry_t* gdbjit_last_entry = NULL;
static gdbjit_block_t* gdbjit_blocks = NULL;
static int gdbjit_atexit_registered = 0;

static int GdbJITLock(void)
{
    if (gdbjit_in_lock)
        return 0;
    gdbjit_in_lock = 1;
    pthread_mutex_lock(&gdbjit_mutex);
    return 1;
}

static void GdbJITUnlock(void)
{
    pthread_mutex_unlock(&gdbjit_mutex);
    gdbjit_in_lock = 0;
}

static int GdbJITBlockInList(gdbjit_block_t* block)
{
    for (gdbjit_block_t* entry = gdbjit_blocks; entry; entry = entry->next_alloc)
        if (entry == block)
            return 1;
    return 0;
}

static void GdbJITUnregisterEntry(gdbjit_block_t* block)
{
    gdbjit_code_entry_t* entry = block->entry;
    if (!entry)
        return;

    if (entry->prev_entry)
        entry->prev_entry->next_entry = entry->next_entry;
    else
        gdbjit_first_entry = entry->next_entry;
    if (entry->next_entry)
        entry->next_entry->prev_entry = entry->prev_entry;
    else
        gdbjit_last_entry = entry->prev_entry;

    __jit_debug_descriptor.first_entry = gdbjit_first_entry;
    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = GDBJIT_UNREGISTER;
    __jit_debug_register_code();
    __jit_debug_descriptor.action_flag = GDBJIT_NOACTION;
    __jit_debug_descriptor.relevant_entry = gdbjit_last_entry;

    block->entry = NULL;
    block->registered = 0;
    box_free(entry);
}

static void GdbJITOpenFile(gdbjit_block_t* block)
{
    gdbjit_symfile_t* symfile = block->symfile;
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir || !tmpdir[0])
        tmpdir = "/tmp";

    int len = snprintf(symfile->filename, sizeof(symfile->filename), "%s/box64gdbjit-XXXXXX.S", tmpdir);
    if (len < 0 || len >= (int)sizeof(symfile->filename))
        snprintf(symfile->filename, sizeof(symfile->filename), "/tmp/box64gdbjit-XXXXXX.S");

    int fd = mkstemps(symfile->filename, 2);
    if (fd < 0) {
        symfile->filename[0] = 0;
        return;
    }
    block->file = fdopen(fd, "w");
    if (!block->file) {
        close(fd);
        unlink(symfile->filename);
        symfile->filename[0] = 0;
    }
}

static void GdbJITSetName(gdbjit_symfile_t* symfile, uintptr_t x64start)
{
    const char* function_name = getAddrFunctionName(x64start);
    if (function_name && function_name[0] && strcmp(function_name, "???")) {
        const char* slash = strrchr(function_name, '/');
        if (slash && slash[1])
            function_name = slash + 1;
        snprintf(symfile->name, sizeof(symfile->name), "dynablock_0x%" PRIxPTR "(%s)", x64start, function_name);
    } else {
        snprintf(symfile->name, sizeof(symfile->name), "dynablock_0x%" PRIxPTR, x64start);
    }
}

static int GdbJITMakeRoom(gdbjit_block_t* block)
{
    gdbjit_symfile_t* symfile = block->symfile;
    if (symfile->nlines < block->alloced)
        return 1;
    if (block->alloced >= GDBJIT_MAX_LINES)
        return 0;

    size_t new_size = block->alloced ? block->alloced * 2 : GDBJIT_LINES_MAX_PREALLOC;
    if (new_size > GDBJIT_MAX_LINES)
        new_size = GDBJIT_MAX_LINES;
    gdbjit_symfile_t* new_symfile = box_realloc(symfile, sizeof(gdbjit_symfile_t) + new_size * sizeof(struct gdb_line_mapping));
    if (!new_symfile)
        return 0;
    block->symfile = new_symfile;
    block->alloced = new_size;
    return 1;
}

void GdbJITCleanupFiles(void)
{
    if (!GdbJITLock())
        return;
    pid_t pid = getpid();
    for (gdbjit_block_t* block = gdbjit_blocks; block; block = block->next_alloc)
        if (block->symfile && block->symfile->filename[0]) {
            if (block->owner == pid)
                unlink(block->symfile->filename);
            block->symfile->filename[0] = 0;
        }
    GdbJITUnlock();
}

gdbjit_block_t* GdbJITNewBlock(GDB_CORE_ADDR start, GDB_CORE_ADDR end, uintptr_t x64start)
{
    gdbjit_block_t* block = box_calloc(1, sizeof(gdbjit_block_t));
    if (!block)
        return NULL;

    gdbjit_symfile_t* symfile = box_calloc(1, sizeof(gdbjit_symfile_t) + GDBJIT_LINES_MAX_PREALLOC * sizeof(struct gdb_line_mapping));
    if (!symfile) {
        box_free(block);
        return NULL;
    }

    block->symfile = symfile;
    block->alloced = GDBJIT_LINES_MAX_PREALLOC;
    block->owner = getpid();
    symfile->magic = GDBJIT_SYMFILE_MAGIC;
    symfile->version = GDBJIT_SYMFILE_VERSION;
    symfile->start = start;
    symfile->end = end;
    symfile->x64start = x64start;
    GdbJITSetName(symfile, (uintptr_t)x64start);
    GdbJITOpenFile(block);

    if (!GdbJITLock()) {
        if (block->file)
            fclose(block->file);
        if (symfile->filename[0])
            unlink(symfile->filename);
        box_free(symfile);
        box_free(block);
        return NULL;
    }
    if (!gdbjit_atexit_registered) {
        atexit(GdbJITCleanupFiles);
        gdbjit_atexit_registered = 1;
    }
    block->next_alloc = gdbjit_blocks;
    if (gdbjit_blocks)
        gdbjit_blocks->prev_alloc = block;
    gdbjit_blocks = block;
    GdbJITUnlock();

    return block;
}

void GdbJITBlockAddText(gdbjit_block_t* block, const char* line)
{
    if (!block || !block->file || !line)
        return;

    fprintf(block->file, "%s\n", line);
    if (block->filelines < INT32_MAX)
        ++block->filelines;
}

gdbjit_block_t* GdbJITBlockAddLine(gdbjit_block_t* block, GDB_CORE_ADDR addr, const char* line)
{
    if (!block || !block->symfile || !line)
        return block;
    if (!block->file)
        return block;

    fprintf(block->file, "%s\n", line);
    if (block->filelines < INT32_MAX)
        ++block->filelines;

    gdbjit_symfile_t* symfile = block->symfile;
    if (symfile->nlines) {
        struct gdb_line_mapping* last = &symfile->lines[symfile->nlines - 1];
        if (addr <= last->pc) {
            if (addr == last->pc)
                last->line = block->filelines;
            return block;
        }
    }

    if (!GdbJITMakeRoom(block))
        return block;
    symfile = block->symfile;
    symfile->lines[symfile->nlines].pc = addr;
    symfile->lines[symfile->nlines].line = block->filelines;
    ++symfile->nlines;
    return block;
}

void GdbJITBlockCleanup(gdbjit_block_t* block)
{
    if (block && block->file) {
        fflush(block->file);
        fclose(block->file);
        block->file = NULL;
    }
}

void GdbJITBlockReady(gdbjit_block_t* block)
{
    if (!block)
        return;

    if (!GdbJITLock())
        return;
    if (!GdbJITBlockInList(block)) {
        GdbJITUnlock();
        return;
    }

    GdbJITBlockCleanup(block);

    gdbjit_symfile_t* symfile = block->symfile;
    if (!symfile || !symfile->nlines || !symfile->filename[0] || symfile->end <= symfile->start) {
        GdbJITUnlock();
        return;
    }
    if (symfile->nlines > INT32_MAX) {
        GdbJITUnlock();
        return;
    }

    if (!block->registered) {
        gdbjit_code_entry_t* entry = box_calloc(1, sizeof(gdbjit_code_entry_t));
        if (entry) {
            entry->symfile_addr = (const char*)symfile;
            entry->symfile_size = sizeof(gdbjit_symfile_t) + symfile->nlines * sizeof(struct gdb_line_mapping);
            entry->prev_entry = gdbjit_last_entry;
            if (gdbjit_last_entry)
                gdbjit_last_entry->next_entry = entry;
            else
                gdbjit_first_entry = entry;
            gdbjit_last_entry = entry;

            __jit_debug_descriptor.first_entry = gdbjit_first_entry;
            __jit_debug_descriptor.relevant_entry = entry;
            __jit_debug_descriptor.action_flag = GDBJIT_REGISTER;
            __jit_debug_register_code();

            block->entry = entry;
            block->registered = 1;
        }
    }

    GdbJITUnlock();
}

void GdbJITBlockFree(gdbjit_block_t* block)
{
    if (!block)
        return;

    if (!GdbJITLock())
        return;
    if (!GdbJITBlockInList(block)) {
        GdbJITUnlock();
        return;
    }

    GdbJITBlockCleanup(block);
    GdbJITUnregisterEntry(block);

    if (block->prev_alloc)
        block->prev_alloc->next_alloc = block->next_alloc;
    else
        gdbjit_blocks = block->next_alloc;
    if (block->next_alloc)
        block->next_alloc->prev_alloc = block->prev_alloc;

    if (block->symfile && block->symfile->filename[0]) {
        if (block->owner == getpid())
            unlink(block->symfile->filename);
        block->symfile->filename[0] = 0;
    }

    GdbJITUnlock();

    box_free(block->symfile);
    box_free(block);
}
#endif
