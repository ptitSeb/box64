// SPDX-License-Identifier: MIT
#ifndef __GDBJIT_H__
#define __GDBJIT_H__

#if defined(DYNAREC) && defined(GDBJIT)
#include <gdb/jit-reader.h>
#include <stdint.h>

#define GDBJIT_SYMFILE_MAGIC   0x4744424au  /* "GDBJ" */
#define GDBJIT_SYMFILE_VERSION 1

#define GDBJIT_FILENAME_MAX 128
#define GDBJIT_NAME_MAX     96

typedef struct gdbjit_symfile_s {
    uint32_t magic;
    uint32_t version;
    GDB_CORE_ADDR start;
    GDB_CORE_ADDR end;
    uint64_t x64start;
    uint64_t nlines;
    char filename[GDBJIT_FILENAME_MAX];
    char name[GDBJIT_NAME_MAX];
    struct gdb_line_mapping lines[0];
} gdbjit_symfile_t;

typedef struct gdbjit_block_s gdbjit_block_t;

gdbjit_block_t* GdbJITNewBlock(GDB_CORE_ADDR start, GDB_CORE_ADDR end, uintptr_t x64start);
gdbjit_block_t* GdbJITBlockAddLine(gdbjit_block_t* block, GDB_CORE_ADDR addr, const char* line);
void GdbJITBlockAddText(gdbjit_block_t* block, const char* line);
void GdbJITBlockReady(gdbjit_block_t* block);
void GdbJITBlockCleanup(gdbjit_block_t* block);
void GdbJITBlockFree(gdbjit_block_t* block);
void GdbJITCleanupFiles(void);

#else

#define GdbJITNewBlock(a, b, c) NULL
#define GdbJITBlockAddLine(a, b, c) NULL
#define GdbJITBlockAddText(a, b)
#define GdbJITBlockReady(a)
#define GdbJITBlockCleanup(a)
#define GdbJITBlockFree(a)
#define GdbJITCleanupFiles()

#endif

#endif // __GDBJIT_H__
