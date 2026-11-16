// SPDX-License-Identifier: MIT
#include "gdbjit.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

__attribute__((visibility("default"))) int plugin_is_GPL_compatible(void) { return 0; }

#if defined(__aarch64__)
#define GDBJIT_ARCH_ARM64 1
#elif defined(__riscv) && (__riscv_xlen == 64)
#define GDBJIT_ARCH_RV64 1
#elif defined(__loongarch__) || defined(__loongarch64) || defined(LA64)
#define GDBJIT_ARCH_LA64 1
#endif

#if defined(GDBJIT_ARCH_ARM64)
struct reader_range {
    GDB_CORE_ADDR start;
    GDB_CORE_ADDR end;
};

struct reader_state {
    struct reader_range* ranges;
    size_t nranges;
    size_t cap;
};

static int add_range(struct reader_state* state, GDB_CORE_ADDR start, GDB_CORE_ADDR end)
{
    if (state->nranges == state->cap) {
        size_t cap = state->cap ? state->cap * 2 : 256;
        struct reader_range* ranges = realloc(state->ranges, cap * sizeof(*ranges));
        if (!ranges)
            return 0;
        state->ranges = ranges;
        state->cap = cap;
    }
    state->ranges[state->nranges].start = start;
    state->ranges[state->nranges].end = end;
    ++state->nranges;
    return 1;
}

static const struct reader_range* find_range(struct reader_state* state, GDB_CORE_ADDR pc)
{
    for (size_t i = 0; i < state->nranges; ++i)
        if (pc >= state->ranges[i].start && pc < state->ranges[i].end)
            return &state->ranges[i];
    return NULL;
}
#else
struct reader_state {
    int unused;
};
#endif

static enum gdb_status read_debug_info(struct gdb_reader_funcs* self, struct gdb_symbol_callbacks* cbs, void* memory, long memory_sz)
{
    struct reader_state* state = (struct reader_state*)self->priv_data;

    if (!memory || memory_sz < (long)sizeof(gdbjit_symfile_t))
        return GDB_FAIL;

    const gdbjit_symfile_t* symfile = (const gdbjit_symfile_t*)memory;
    if (symfile->magic != GDBJIT_SYMFILE_MAGIC || symfile->version != GDBJIT_SYMFILE_VERSION)
        return GDB_FAIL;
    if (symfile->end <= symfile->start || !symfile->nlines || symfile->nlines > INT_MAX)
        return GDB_FAIL;
    if (symfile->nlines > ((size_t)memory_sz - sizeof(gdbjit_symfile_t)) / sizeof(struct gdb_line_mapping))
        return GDB_FAIL;

    char filename[GDBJIT_FILENAME_MAX + 1];
    memcpy(filename, symfile->filename, GDBJIT_FILENAME_MAX);
    filename[GDBJIT_FILENAME_MAX] = '\0';
    if (!filename[0])
        return GDB_FAIL;

    char name[GDBJIT_NAME_MAX + 1];
    memcpy(name, symfile->name, GDBJIT_NAME_MAX);
    name[GDBJIT_NAME_MAX] = '\0';
    if (!name[0])
        snprintf(name, sizeof(name), "dynablock_0x%" PRIx64, symfile->x64start);

#if defined(GDBJIT_ARCH_ARM64)
    if (state)
        add_range(state, symfile->start, symfile->end);
#else
    (void)state;
#endif

    struct gdb_object* object = cbs->object_open(cbs);
    struct gdb_symtab* symtab = cbs->symtab_open(cbs, object, filename);
    cbs->block_open(cbs, symtab, NULL, symfile->start, symfile->end, name);
    cbs->line_mapping_add(cbs, symtab, (int)symfile->nlines, (struct gdb_line_mapping*)symfile->lines);
    cbs->symtab_close(cbs, symtab);
    cbs->object_close(cbs, object);

    return GDB_SUCCESS;
}

static void free_reg_value(struct gdb_reg_value* value)
{
    free(value);
}

static int read_reg(struct gdb_unwind_callbacks* cbs, int regnum, uint64_t* value)
{
    struct gdb_reg_value* reg = cbs->reg_get(cbs, regnum);
    int ok = 0;
    if (reg) {
        if (reg->defined && reg->size == (int)sizeof(*value)) {
            memcpy(value, reg->value, sizeof(*value));
            ok = 1;
        }
        if (reg->free)
            reg->free(reg);
    }
    return ok;
}

static int write_reg(struct gdb_unwind_callbacks* cbs, int regnum, uint64_t value)
{
    struct gdb_reg_value* reg = malloc(sizeof(*reg) + sizeof(value) - 1);
    if (!reg)
        return 0;
    reg->size = sizeof(value);
    reg->defined = 1;
    reg->free = free_reg_value;
    memcpy(reg->value, &value, sizeof(value));
    cbs->reg_set(cbs, regnum, reg);
    return 1;
}

#if defined(GDBJIT_ARCH_ARM64)

#define ARM64_DWARF_X28 28
#define ARM64_DWARF_X29 29
#define ARM64_DWARF_SP  31
#define ARM64_DWARF_PC  32
#define ARM64_FRAME_LR  144
#define ARM64_FRAME_FP  152
#define ARM64_FRAME_SIZE 160
#define FRAME_LR_ALIGN  3

#elif defined(GDBJIT_ARCH_RV64)

#define RV64_DWARF_RA   1
#define RV64_DWARF_SP   2
#define RV64_DWARF_FP   8
#define RV64_DWARF_EMU  25
#define RV64_FRAME_LR   0
#define RV64_FRAME_FP   8
#define RV64_FRAME_SIZE 192
#define RV64_EMU_XSPSAVE 808
#define FRAME_LR_ALIGN  1

#elif defined(GDBJIT_ARCH_LA64)

#define LA64_DWARF_RA   1
#define LA64_DWARF_SP   3
#define LA64_DWARF_FP   22
#define LA64_FRAME_LR   0
#define LA64_FRAME_FP   8
#define LA64_FRAME_SIZE 160
#define FRAME_LR_ALIGN  3
#endif

#if defined(GDBJIT_ARCH_RV64) || defined(GDBJIT_ARCH_LA64)
#if defined(GDBJIT_ARCH_RV64)
#define FRAME_RA_REG  RV64_DWARF_RA
#define FRAME_SP_REG  RV64_DWARF_SP
#define FRAME_FP_REG  RV64_DWARF_FP
#define FRAME_LR_OFF  RV64_FRAME_LR
#define FRAME_FP_OFF  RV64_FRAME_FP
#define FRAME_SIZE    RV64_FRAME_SIZE
#else
#define FRAME_RA_REG  LA64_DWARF_RA
#define FRAME_SP_REG  LA64_DWARF_SP
#define FRAME_FP_REG  LA64_DWARF_FP
#define FRAME_LR_OFF  LA64_FRAME_LR
#define FRAME_FP_OFF  LA64_FRAME_FP
#define FRAME_SIZE    LA64_FRAME_SIZE
#endif

static int get_dynarec_frame(struct gdb_unwind_callbacks* cbs, uint64_t* base, uint64_t* sp, uint64_t* lr, uint64_t* fp)
{
#if defined(GDBJIT_ARCH_RV64)
    uint64_t emu;
    if (!read_reg(cbs, RV64_DWARF_EMU, &emu) || emu < 0x10000 || (emu & 7))
        return 0;
    if (cbs->target_read((GDB_CORE_ADDR)(emu + RV64_EMU_XSPSAVE), base, sizeof(*base)) != GDB_SUCCESS)
        return 0;
#else
    if (!read_reg(cbs, LA64_DWARF_FP, base))
        return 0;
#endif
    if (!*base || (*base & 15))
        return 0;
    if (!read_reg(cbs, FRAME_SP_REG, sp))
        return 0;
    if (*sp != *base - 16)
        return 0;

    uint64_t sentinel;
    if (cbs->target_read((GDB_CORE_ADDR)(*base - 8), &sentinel, sizeof(sentinel)) != GDB_SUCCESS || sentinel)
        return 0;
    if (cbs->target_read((GDB_CORE_ADDR)(*base + FRAME_LR_OFF), lr, sizeof(*lr)) != GDB_SUCCESS)
        return 0;
    if (cbs->target_read((GDB_CORE_ADDR)(*base + FRAME_FP_OFF), fp, sizeof(*fp)) != GDB_SUCCESS)
        return 0;
    if (!*lr || (*lr & FRAME_LR_ALIGN))
        return 0;

    return 1;
}
#endif

static enum gdb_status unwind_frame(struct gdb_reader_funcs* self, struct gdb_unwind_callbacks* cbs)
{
    struct reader_state* state = (struct reader_state*)self->priv_data;
#if defined(GDBJIT_ARCH_ARM64)
    uint64_t pc;
    if (!state || !read_reg(cbs, ARM64_DWARF_PC, &pc) || !find_range(state, (GDB_CORE_ADDR)pc))
        return GDB_FAIL;

    uint64_t frame_base;
    if (!read_reg(cbs, ARM64_DWARF_X28, &frame_base) || !frame_base || (frame_base & 15))
        return GDB_FAIL;

    uint64_t lr, fp;
    if (cbs->target_read((GDB_CORE_ADDR)(frame_base + ARM64_FRAME_LR), &lr, sizeof(lr)) != GDB_SUCCESS)
        return GDB_FAIL;
    if (cbs->target_read((GDB_CORE_ADDR)(frame_base + ARM64_FRAME_FP), &fp, sizeof(fp)) != GDB_SUCCESS)
        return GDB_FAIL;
    if (!lr || (lr & FRAME_LR_ALIGN))
        return GDB_FAIL;

    if (!write_reg(cbs, ARM64_DWARF_PC, lr)
        || !write_reg(cbs, ARM64_DWARF_X29, fp)
        || !write_reg(cbs, ARM64_DWARF_SP, frame_base + ARM64_FRAME_SIZE))
        return GDB_FAIL;

    return GDB_SUCCESS;
#elif defined(GDBJIT_ARCH_RV64) || defined(GDBJIT_ARCH_LA64)
    uint64_t base, sp, lr, fp;
    (void)state;
    if (!get_dynarec_frame(cbs, &base, &sp, &lr, &fp))
        return GDB_FAIL;

    if (!write_reg(cbs, FRAME_RA_REG, lr)
        || !write_reg(cbs, FRAME_FP_REG, fp)
        || !write_reg(cbs, FRAME_SP_REG, base + FRAME_SIZE))
        return GDB_FAIL;

    return GDB_SUCCESS;
#else
    (void)state;
    (void)cbs;
    return GDB_FAIL;
#endif
}

static struct gdb_frame_id get_frame_id(struct gdb_reader_funcs* self, struct gdb_unwind_callbacks* cbs)
{
    struct gdb_frame_id frame = {0, 0};
    struct reader_state* state = (struct reader_state*)self->priv_data;
#if defined(GDBJIT_ARCH_ARM64)
    uint64_t pc, sp;
    const struct reader_range* range;

    if (!state || !read_reg(cbs, ARM64_DWARF_PC, &pc))
        return frame;
    range = find_range(state, (GDB_CORE_ADDR)pc);
    if (!range)
        return frame;

    frame.code_address = range->start;
    if (read_reg(cbs, ARM64_DWARF_SP, &sp))
        frame.stack_address = (GDB_CORE_ADDR)sp;
#elif defined(GDBJIT_ARCH_RV64) || defined(GDBJIT_ARCH_LA64)
    uint64_t base, sp, lr, fp;
    (void)state;
    if (!get_dynarec_frame(cbs, &base, &sp, &lr, &fp))
        return frame;

    frame.code_address = (GDB_CORE_ADDR)lr;
    frame.stack_address = (GDB_CORE_ADDR)sp;
#else
    (void)state;
    (void)cbs;
#endif
    return frame;
}

static void destroy_reader(struct gdb_reader_funcs* self)
{
    if (self) {
        struct reader_state* state = (struct reader_state*)self->priv_data;
        if (state) {
#if defined(GDBJIT_ARCH_ARM64)
            free(state->ranges);
#endif
            free(state);
        }
        free(self);
    }
}

__attribute__((visibility("default"))) struct gdb_reader_funcs* gdb_init_reader(void)
{
    struct reader_state* state = calloc(1, sizeof(*state));
    struct gdb_reader_funcs* funcs = calloc(1, sizeof(*funcs));
    if (!state || !funcs) {
        free(state);
        free(funcs);
        return NULL;
    }

    funcs->reader_version = GDB_READER_INTERFACE_VERSION;
    funcs->priv_data = state;
    funcs->read = read_debug_info;
    funcs->unwind = unwind_frame;
    funcs->get_frame_id = get_frame_id;
    funcs->destroy = destroy_reader;
    return funcs;
}
