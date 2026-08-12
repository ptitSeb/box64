#include <stddef.h>

#include "la64_mapping.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "emu/x64run_private.h"
#include "dynarec_native.h"

#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_consts.h"
#include "../dynarec_helper.h"
#include "dynarec_la64_native.h"

static void emit_forward_tail(dynarec_la64_t* dyn, int ninst)
{
    ANDI(x4, x3, 8);
    BEQZ(x4, 5 * 4);
    LD_D(x5, x2, 0);
    ST_D(x5, x1, 0);
    ADDI_D(x1, x1, 8);
    ADDI_D(x2, x2, 8);

    ANDI(x4, x3, 4);
    BEQZ(x4, 5 * 4);
    LD_WU(x5, x2, 0);
    ST_W(x5, x1, 0);
    ADDI_D(x1, x1, 4);
    ADDI_D(x2, x2, 4);

    ANDI(x4, x3, 2);
    BEQZ(x4, 5 * 4);
    LD_HU(x5, x2, 0);
    ST_H(x5, x1, 0);
    ADDI_D(x1, x1, 2);
    ADDI_D(x2, x2, 2);

    ANDI(x4, x3, 1);
    BEQZ(x4, 3 * 4);
    LD_BU(x5, x2, 0);
    ST_B(x5, x1, 0);
}

static void emit_backward_tail(dynarec_la64_t* dyn, int ninst)
{
    ANDI(x4, x3, 8);
    BEQZ(x4, 5 * 4);
    ADDI_D(x1, x1, -8);
    ADDI_D(x2, x2, -8);
    LD_D(x5, x2, 0);
    ST_D(x5, x1, 0);

    ANDI(x4, x3, 4);
    BEQZ(x4, 5 * 4);
    ADDI_D(x1, x1, -4);
    ADDI_D(x2, x2, -4);
    LD_WU(x5, x2, 0);
    ST_W(x5, x1, 0);

    ANDI(x4, x3, 2);
    BEQZ(x4, 5 * 4);
    ADDI_D(x1, x1, -2);
    ADDI_D(x2, x2, -2);
    LD_HU(x5, x2, 0);
    ST_H(x5, x1, 0);

    ANDI(x4, x3, 1);
    BEQZ(x4, 5 * 4);
    ADDI_D(x1, x1, -1);
    ADDI_D(x2, x2, -1);
    LD_BU(x5, x2, 0);
    ST_B(x5, x1, 0);
}

static void emit_memcpy(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);
    int s1 = fpu_get_scratch(dyn);

    MV(xRAX, xRDI);
    MV(x1, xRDI);
    MV(x2, xRSI);
    MV(x3, xRDX);
    MOV32w(x4, 32);
    BLTU_MARK2(x3, x4);
    MARK;
    if (cpuext.lasx) {
        XVLD(s0, x2, 0);
        XVST(s0, x1, 0);
    } else {
        VLD(s0, x2, 0);
        VLD(s1, x2, 16);
        VST(s0, x1, 0);
        VST(s1, x1, 16);
    }
    ADDI_D(x1, x1, 32);
    ADDI_D(x2, x2, 32);
    ADDI_D(x3, x3, -32);
    BGEU_MARK(x3, x4);
    MARK2;
    ANDI(x4, x3, 16);
    BEQZ(x4, 5 * 4);
    VLD(s0, x2, 0);
    VST(s0, x1, 0);
    ADDI_D(x1, x1, 16);
    ADDI_D(x2, x2, 16);
    emit_forward_tail(dyn, ninst);
}

static void emit_memmove(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);
    int s1 = fpu_get_scratch(dyn);

    MV(xRAX, xRDI);
    MV(x1, xRDI);
    MV(x2, xRSI);
    MV(x3, xRDX);
    BEQZ_MARKLOCK(x3);
    BEQ_MARKLOCK(x1, x2);
    BLTU_MARK(x1, x2);

    ADD_D(x1, x1, x3);
    ADD_D(x2, x2, x3);
    MOV32w(x4, 32);
    BLTU_MARKF2(x3, x4);
    MARK3;
    ADDI_D(x1, x1, -32);
    ADDI_D(x2, x2, -32);
    if (cpuext.lasx) {
        XVLD(s0, x2, 0);
        XVST(s0, x1, 0);
    } else {
        VLD(s0, x2, 0);
        VLD(s1, x2, 16);
        VST(s0, x1, 0);
        VST(s1, x1, 16);
    }
    ADDI_D(x3, x3, -32);
    BGEU_MARK3(x3, x4);
    MARKF2;
    ANDI(x4, x3, 16);
    BEQZ(x4, 5 * 4);
    ADDI_D(x1, x1, -16);
    ADDI_D(x2, x2, -16);
    VLD(s0, x2, 0);
    VST(s0, x1, 0);
    emit_backward_tail(dyn, ninst);
    B_MARKLOCK_nocond;

    MARK;
    MOV32w(x4, 32);
    BLTU_MARKF(x3, x4);
    MARK2;
    if (cpuext.lasx) {
        XVLD(s0, x2, 0);
        XVST(s0, x1, 0);
    } else {
        VLD(s0, x2, 0);
        VLD(s1, x2, 16);
        VST(s0, x1, 0);
        VST(s1, x1, 16);
    }
    ADDI_D(x1, x1, 32);
    ADDI_D(x2, x2, 32);
    ADDI_D(x3, x3, -32);
    BGEU_MARK2(x3, x4);
    MARKF;
    ANDI(x4, x3, 16);
    BEQZ(x4, 5 * 4);
    VLD(s0, x2, 0);
    VST(s0, x1, 0);
    ADDI_D(x1, x1, 16);
    ADDI_D(x2, x2, 16);
    emit_forward_tail(dyn, ninst);
    MARKLOCK;
}

static void emit_memset(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);

    MV(xRAX, xRDI);
    MV(x1, xRDI);
    MV(x3, xRDX);
    if (cpuext.lasx)
        XVREPLGR2VR_B(s0, xRSI);
    else
        VREPLGR2VR_B(s0, xRSI);
    VPICKVE2GR_DU(x5, s0, 0);
    MOV32w(x4, 32);
    BLTU_MARK2(x3, x4);
    MARK;
    if (cpuext.lasx)
        XVST(s0, x1, 0);
    else {
        VST(s0, x1, 0);
        VST(s0, x1, 16);
    }
    ADDI_D(x1, x1, 32);
    ADDI_D(x3, x3, -32);
    BGEU_MARK(x3, x4);
    MARK2;
    ANDI(x4, x3, 16);
    BEQZ(x4, 3 * 4);
    VST(s0, x1, 0);
    ADDI_D(x1, x1, 16);

    ANDI(x4, x3, 8);
    BEQZ(x4, 3 * 4);
    ST_D(x5, x1, 0);
    ADDI_D(x1, x1, 8);
    ANDI(x4, x3, 4);
    BEQZ(x4, 3 * 4);
    ST_W(x5, x1, 0);
    ADDI_D(x1, x1, 4);
    ANDI(x4, x3, 2);
    BEQZ(x4, 3 * 4);
    ST_H(x5, x1, 0);
    ADDI_D(x1, x1, 2);
    ANDI(x4, x3, 1);
    BEQZ(x4, 2 * 4);
    ST_B(x5, x1, 0);
}

static void emit_memchr(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);
    int s1 = fpu_get_scratch(dyn);

    MV(x1, xRDI);
    MV(x3, xRDX);
    MV(xRAX, xZR);
    if (cpuext.lasx)
        XVREPLGR2VR_B(s0, xRSI);
    else
        VREPLGR2VR_B(s0, xRSI);
    VPICKVE2GR_BU(x5, s0, 0);
    BEQZ_MARKLOCK(x3);
    MARK;
    ANDI(x2, x1, cpuext.lasx ? 31 : 15);
    BEQZ_MARK2(x2);
    LD_BU(x4, x1, 0);
    XOR(x2, x4, x5);
    BEQZ_MARKF(x2);
    ADDI_D(x1, x1, 1);
    ADDI_D(x3, x3, -1);
    BNEZ_MARK(x3);
    B_MARKLOCK_nocond;
    MARK2;
    MOV32w(x4, cpuext.lasx ? 32 : 16);
    BLTU_MARK3(x3, x4);
    if (cpuext.lasx) {
        XVLD(s1, x1, 0);
        XVSEQ_B(s1, s1, s0);
        XVMSKLTZ_B(s1, s1);
        XVPICKVE2GR_DU(x2, s1, 0);
        XVPICKVE2GR_DU(x6, s1, 2);
        SLLI_D(x6, x6, 16);
        OR(x2, x2, x6);
        BNEZ_MARKF2(x2);
        ADDI_D(x1, x1, 32);
        ADDI_D(x3, x3, -32);
    } else {
        VLD(s1, x1, 0);
        VSEQ_B(s1, s1, s0);
        VMSKLTZ_B(s1, s1);
        VPICKVE2GR_DU(x2, s1, 0);
        BNEZ_MARKF2(x2);
        ADDI_D(x1, x1, 16);
        ADDI_D(x3, x3, -16);
    }
    BGEU_MARK2(x3, x4);
    MARK3;
    BEQZ_MARKLOCK(x3);
    LD_BU(x4, x1, 0);
    XOR(x2, x4, x5);
    BEQZ_MARKF(x2);
    ADDI_D(x1, x1, 1);
    ADDI_D(x3, x3, -1);
    BNEZ_MARK3(x3);
    B_MARKLOCK_nocond;
    MARKF;
    MV(xRAX, x1);
    B_MARKLOCK_nocond;
    MARKF2;
    CTZ_D(x2, x2);
    ADD_D(x1, x1, x2);
    MV(xRAX, x1);
    MARKLOCK;
}

static void emit_memcmp(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);
    int s1 = fpu_get_scratch(dyn);
    int s2 = fpu_get_scratch(dyn);

    MV(x1, xRDI);
    MV(x2, xRSI);
    MV(x3, xRDX);
    MV(xRAX, xZR);
    MOV32w(x4, 16);
    MOV32w(x6, 0xffff);
    BLTU_MARK2(x3, x4);
    MARK;
    VLD(s0, x1, 0);
    VLD(s1, x2, 0);
    VSEQ_B(s2, s0, s1);
    VMSKLTZ_B(s2, s2);
    VPICKVE2GR_DU(x5, s2, 0);
    XOR(x5, x5, x6);
    BNEZ_MARKF(x5);
    ADDI_D(x1, x1, 16);
    ADDI_D(x2, x2, 16);
    ADDI_D(x3, x3, -16);
    BGEU_MARK(x3, x4);
    MARK2;
    BEQZ_MARKLOCK(x3);
    MARK3;
    LD_BU(x4, x1, 0);
    LD_BU(x5, x2, 0);
    SUB_W(xRAX, x4, x5);
    BNEZ_MARKLOCK(xRAX);
    ADDI_D(x1, x1, 1);
    ADDI_D(x2, x2, 1);
    ADDI_D(x3, x3, -1);
    BNEZ_MARK3(x3);
    B_MARKLOCK_nocond;
    MARKF;
    CTZ_D(x5, x5);
    ADD_D(x1, x1, x5);
    ADD_D(x2, x2, x5);
    LD_BU(x4, x1, 0);
    LD_BU(x5, x2, 0);
    SUB_W(xRAX, x4, x5);
    MARKLOCK;
    ZEROUP(xRAX);
}

static void emit_strcmp(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);
    int s1 = fpu_get_scratch(dyn);
    int s2 = fpu_get_scratch(dyn);
    int s3 = fpu_get_scratch(dyn);

    MV(x3, xRDI);
    MV(x4, xRSI);
    MV(xRAX, xZR);
    XOR(x1, x3, x4);
    ANDI(x1, x1, 15);
    BNEZ_MARK3(x1);
    VXOR_V(s3, s3, s3);
    MOV32w(x6, 0xffff);
    MARKF;
    ANDI(x1, x3, 15);
    BEQZ_MARK(x1);
    LD_BU(x1, x3, 0);
    LD_BU(x2, x4, 0);
    SUB_W(xRAX, x1, x2);
    BNEZ_MARKLOCK(xRAX);
    BEQZ_MARKLOCK(x1);
    ADDI_D(x3, x3, 1);
    ADDI_D(x4, x4, 1);
    B_MARKF_nocond;
    MARK;
    VLD(s0, x3, 0);
    VLD(s1, x4, 0);
    VSEQ_B(s2, s0, s1);
    VMSKLTZ_B(s2, s2);
    VPICKVE2GR_DU(x5, s2, 0);
    XOR(x5, x5, x6);
    VSEQ_B(s2, s0, s3);
    VMSKLTZ_B(s2, s2);
    VPICKVE2GR_DU(x7, s2, 0);
    OR(x5, x5, x7);
    BNEZ_MARK2(x5);
    ADDI_D(x3, x3, 16);
    ADDI_D(x4, x4, 16);
    B_MARK_nocond;
    MARK2;
    CTZ_D(x5, x5);
    ADD_D(x3, x3, x5);
    ADD_D(x4, x4, x5);
    LD_BU(x1, x3, 0);
    LD_BU(x2, x4, 0);
    SUB_W(xRAX, x1, x2);
    B_MARKLOCK_nocond;
    MARK3;
    LD_BU(x1, x3, 0);
    LD_BU(x2, x4, 0);
    SUB_W(xRAX, x1, x2);
    BNEZ_MARKLOCK(xRAX);
    BEQZ_MARKLOCK(x1);
    ADDI_D(x3, x3, 1);
    ADDI_D(x4, x4, 1);
    B_MARK3_nocond;
    MARKLOCK;
    ZEROUP(xRAX);
}

static void emit_strlen(dynarec_la64_t* dyn, int ninst)
{
    int64_t j64;
    MAYUSE(j64);

    int s0 = fpu_get_scratch(dyn);
    int s1 = fpu_get_scratch(dyn);

    MV(x1, xRDI);
    MV(x2, xRDI);
    if (cpuext.lasx)
        XVXOR_V(s1, s1, s1);
    else
        VXOR_V(s1, s1, s1);
    MARK;
    ANDI(x3, x1, cpuext.lasx ? 31 : 15);
    BEQZ_MARK2(x3);
    LD_BU(x4, x1, 0);
    BEQZ_MARK3(x4);
    ADDI_D(x1, x1, 1);
    B_MARK_nocond;
    MARK2;
    if (cpuext.lasx) {
        XVLD(s0, x1, 0);
        XVSEQ_B(s0, s0, s1);
        XVMSKLTZ_B(s0, s0);
        XVPICKVE2GR_DU(x3, s0, 0);
        XVPICKVE2GR_DU(x6, s0, 2);
        SLLI_D(x6, x6, 16);
        OR(x3, x3, x6);
        BNEZ_MARKF(x3);
        ADDI_D(x1, x1, 32);
    } else {
        VLD(s0, x1, 0);
        VSEQ_B(s0, s0, s1);
        VMSKLTZ_B(s0, s0);
        VPICKVE2GR_DU(x3, s0, 0);
        BNEZ_MARKF(x3);
        ADDI_D(x1, x1, 16);
    }
    B_MARK2_nocond;
    MARK3;
    SUB_D(xRAX, x1, x2);
    B_MARKLOCK_nocond;
    MARKF;
    CTZ_D(x3, x3);
    ADD_D(x1, x1, x3);
    SUB_D(xRAX, x1, x2);
    MARKLOCK;
}

static const la64_consts_t native_call_consts[LA64_NATIVE_LAST] = {
    [LA64_NATIVE_NONE] = const_none,
    [LA64_NATIVE_MEMCMP] = const_native_memcmp,
    [LA64_NATIVE_MEMCPY] = const_native_memcpy,
    [LA64_NATIVE_MEMMOVE] = const_native_memmove,
    [LA64_NATIVE_MEMSET] = const_native_memset,
    [LA64_NATIVE_MEMCHR] = const_native_memchr,
    [LA64_NATIVE_STRCMP] = const_native_strcmp,
    [LA64_NATIVE_STRLEN] = const_native_strlen,
};

void la64_emit_native_call(dynarec_la64_t* dyn, int ninst, la64_native_call_t call)
{
    switch (call) {
        case LA64_NATIVE_MEMCMP:
        case LA64_NATIVE_STRCMP:
            UP32_READ(xRDI);
            UP32_READ(xRSI);
            UP32_WRITE32(xRAX);
            break;
        case LA64_NATIVE_MEMCPY:
        case LA64_NATIVE_MEMMOVE:
        case LA64_NATIVE_MEMSET:
        case LA64_NATIVE_MEMCHR:
            UP32_READ(xRDI);
            UP32_READ(xRSI);
            UP32_READ(xRDX);
            UP32_WRITE64(xRAX);
            break;
        case LA64_NATIVE_STRLEN:
            UP32_READ(xRDI);
            UP32_WRITE64(xRAX);
            break;
        default:
            break;
    }
    TABLE64C(x6, native_call_consts[call]);
    JIRL(xRA, x6, 0);
}

void la64_native_call_pass(dynarec_la64_t* dyn, la64_native_call_t call)
{
    int ninst = 0;
    MAYUSE(ninst);
    switch (call) {
        case LA64_NATIVE_MEMCMP:
            emit_memcmp(dyn, ninst);
            break;
        case LA64_NATIVE_MEMCPY:
            emit_memcpy(dyn, ninst);
            break;
        case LA64_NATIVE_MEMMOVE:
            emit_memmove(dyn, ninst);
            break;
        case LA64_NATIVE_MEMSET:
            emit_memset(dyn, ninst);
            break;
        case LA64_NATIVE_MEMCHR:
            emit_memchr(dyn, ninst);
            break;
        case LA64_NATIVE_STRCMP:
            emit_strcmp(dyn, ninst);
            break;
        case LA64_NATIVE_STRLEN:
            emit_strlen(dyn, ninst);
            break;
        default:
            break;
    }
    BR(xRA);
}
