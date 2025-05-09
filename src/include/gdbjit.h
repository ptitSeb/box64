#ifndef __GDBJIT_H__
#define __GDBJIT_H__

#if defined(DYNAREC) && defined(GDBJIT)
#include <gdb/jit-reader.h>
#include <stdio.h>
#include <stdint.h>

typedef struct gdbjit_block_s {
    char filename[32];
    FILE* file;
    GDB_CORE_ADDR start;
    GDB_CORE_ADDR end;
    uintptr_t x64start;
    size_t alloced;
    size_t nlines;
    struct gdb_line_mapping lines[0];
} gdbjit_block_t;


void GdbJITNewBlock(gdbjit_block_t* block, GDB_CORE_ADDR start, GDB_CORE_ADDR end, uintptr_t x64start);
gdbjit_block_t* GdbJITBlockAddLine(gdbjit_block_t* block, GDB_CORE_ADDR addr, const char* line);
void GdbJITBlockReady(gdbjit_block_t* block);
void GdbJITBlockCleanup(gdbjit_block_t* block);

#else

#define GdbJITNewBlock(a, b, c, d)
#define GdbJITBlockAddLine(a, b, c) NULL
#define GdbJITBlockReady(a)
#define GdbJITBlockCleanup(a)

#endif

#endif // __GDBJIT_H__
