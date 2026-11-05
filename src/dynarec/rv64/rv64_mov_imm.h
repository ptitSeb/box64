// SPDX-License-Identifier: MIT
#ifndef __RV64_MOV_IMM_H__
#define __RV64_MOV_IMM_H__

#include <stdint.h>

#define RV64_MOV_MAXOPS 16
#ifndef RV64_MOV_MAXDEPTH
#define RV64_MOV_MAXDEPTH 2
#endif
#ifndef RV64_MOV_BUDGET
#define RV64_MOV_BUDGET 4096
#endif

enum rv64_movop_e {
    RV64_MOV_ADDI,
    RV64_MOV_ADDIW,
    RV64_MOV_LUI,
    RV64_MOV_SLLI,
    RV64_MOV_SRLI,
    RV64_MOV_SLLIUW,
    RV64_MOV_BSETI,
    RV64_MOV_BCLRI,
    RV64_MOV_XORI,
    RV64_MOV_ZEXTW,
    RV64_MOV_RORI,
    RV64_MOV_ADD,
    RV64_MOV_SLLI2,
    RV64_MOV_SH1ADD,
    RV64_MOV_SH2ADD,
    RV64_MOV_SH3ADD,
    RV64_MOV_ADDI2,
    RV64_MOV_XORI2,
};

#define RV64_MOV_DST 0
#define RV64_MOV_S1  1
#define RV64_MOV_S2  2

typedef struct rv64_movop_s {
    uint8_t op;
    uint8_t rd;
    uint8_t rs;
    uint8_t rs2;
    int32_t imm;
} rv64_movop_t;

typedef struct rv64_mov_seq_s {
    uint8_t n;
    uint8_t depth;
    uint8_t overflow;
    rv64_movop_t op[RV64_MOV_MAXOPS];
} rv64_mov_seq_t;

typedef struct rv64_movcaps_s {
    uint8_t zba;
    uint8_t zbs;
    uint8_t zbb;
} rv64_movcaps_t;

typedef struct rv64_movctx_s {
    rv64_movcaps_t caps;
    int budget;
} rv64_movctx_t;

typedef void (*rv64_mov_emit_fn)(void* ctx, uint32_t insn);

int rv64_mov_seq64(rv64_movctx_t* ctx, uint64_t v, int depth, int nscratch, rv64_mov_seq_t* out);
int rv64_mov_seq32(rv64_movctx_t* ctx, uint32_t v, int depth, rv64_mov_seq_t* out);
void rv64_mov_emit_seq(int reg, int s1, int s2, const rv64_mov_seq_t* seq, rv64_mov_emit_fn emit_fn, void* emit_ctx);
int rv64_mov_cache_get(uint64_t v, int nscratch, int is32, rv64_mov_seq_t* out);
void rv64_mov_cache_put(uint64_t v, int nscratch, int is32, const rv64_mov_seq_t* seq);

#endif
