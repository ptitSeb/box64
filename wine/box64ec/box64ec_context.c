// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

#include "box64ec_private.h"
#include "env.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"

NTSYSAPI PVOID WINAPI RtlLocateExtendedFeature(PVOID context_ex, ULONG feature,
                                               PULONG length);

_Static_assert(offsetof(ARM64EC_NT_CONTEXT, AMD64_EFlags) == 0x44, "CTX_EFLAGS");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X8) == 0x78, "CTX_X8/Rax");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X0) == 0x80, "CTX_X0/Rcx");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X1) == 0x88, "CTX_X1/Rdx");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X27) == 0x90, "CTX_X27/Rbx");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, Sp) == 0x98, "CTX_SP");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, Fp) == 0xa0, "CTX_FP");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X25) == 0xa8, "CTX_X25/Rsi");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X26) == 0xb0, "CTX_X26/Rdi");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X2) == 0xb8, "CTX_X2/R8");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X3) == 0xc0, "CTX_X3/R9");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X4) == 0xc8, "CTX_X4/R10");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X5) == 0xd0, "CTX_X5/R11");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X19) == 0xd8, "CTX_X19/R12");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X20) == 0xe0, "CTX_X20/R13");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X21) == 0xe8, "CTX_X21/R14");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, X22) == 0xf0, "CTX_X22/R15");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, Pc) == 0xf8, "CTX_PC");
_Static_assert(offsetof(ARM64EC_NT_CONTEXT, V) == 0x1a0, "CTX_V/XMM");
_Static_assert(sizeof(((x64emu_t*)0)->ymm) ==
                   16 * sizeof(ARM64_NT_NEON128),
               "Box64 YMM upper halves must match ARM V16-V31");

uint32_t Box64EC_GetEflags(x64emu_t* emu)
{
    if (!emu)
        return 0;
    if (emu->df)
        UpdateFlags(emu);
    return emu->eflags.x64;
}

static void* context_ymm(const ARM64EC_NT_CONTEXT* context)
{
    if ((context->ContextFlags & CONTEXT_XSTATE) != CONTEXT_XSTATE)
        return NULL;
    return RtlLocateExtendedFeature((void*)(context + 1), XSTATE_AVX, NULL);
}

static uint8_t x87_abridged_tag(const x64emu_t* emu)
{
    uint8_t tag = 0;

    for (int i = 0; i < 8; ++i)
        if (((emu->fpu_tags >> (i * 2)) & 3) != 3)
            tag |= 1u << i;
    return tag;
}

static uint16_t x87_status_word(const x64emu_t* emu)
{
    return (emu->sw.x16 & ~(7u << 11)) | ((emu->top & 7u) << 11);
}

static void x87_slots_from_emu(x64emu_t* emu, uint8_t slots[8][16])
{
    uint8_t tag = x87_abridged_tag(emu);

    memset(slots, 0, 8 * 16);
    for (int i = 0; i < 8; ++i) {
        if (tag & (1u << i)) {
            if (fpu_ld80_raw_valid(emu, i))
                memcpy(slots[i], &STld(i).ld, 10);
            else
                D2LD(&ST(i).d, slots[i]);
        } else
            memcpy(slots[i], &emu->mmx[i].q, sizeof(emu->mmx[i].q));
    }
}

static void x87_context_to_emu(const ARM64EC_NT_CONTEXT* context,
                               x64emu_t* emu)
{
    const uint8_t* slots = (const uint8_t*)&context->Lr;
    uint8_t tag = context->AMD64_TagWord;

    emu->cw.x16 = context->AMD64_ControlWord;
    emu->sw.x16 = context->AMD64_StatusWord;
    emu->top = emu->sw.f.F87_TOP;
    /* Wine tags are physical; Box64 tags are relative to TOP. */
    tag = (tag >> emu->top) | (tag << ((8 - emu->top) & 7));
    emu->fpu_stack = 0;
    emu->fpu_tags = 0;
    for (int i = 0; i < 8; ++i) {
        if (tag & (1u << i)) {
            memcpy(&STld(i).ld, slots + i * 16, 10);
            LD2D(&STld(i).ld, &ST(i).d);
            STld(i).uref = ST(i).q;
            ++emu->fpu_stack;
        } else {
            fpu_ld80_clear(emu, i);
            emu->fpu_tags |= 3ull << (i * 2);
            memcpy(&emu->mmx[i].q, slots + i * 16, sizeof(emu->mmx[i].q));
        }
    }
}

static void x87_emu_to_context(x64emu_t* emu,
                               ARM64EC_NT_CONTEXT* context)
{
    uint8_t slots[8][16];

    x87_slots_from_emu(emu, slots);
    context->AMD64_ControlWord = emu->cw.x16;
    context->AMD64_StatusWord = x87_status_word(emu);
    uint8_t tag = x87_abridged_tag(emu);
    unsigned top = emu->top & 7;
    context->AMD64_TagWord = (tag << top) | (tag >> ((8 - top) & 7));
    memcpy(&context->Lr, slots, sizeof(slots));
}

void context_to_emu(ARM64EC_NT_CONTEXT* context, x64emu_t* emu)
{
    const void* ymm;

    if ((context->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {
        R_RAX = context->X8;
        R_RCX = context->X0;
        R_RDX = context->X1;
        R_RBX = context->X27;
        R_RSI = context->X25;
        R_RDI = context->X26;
        R_R8  = context->X2;
        R_R9  = context->X3;
        R_R10 = context->X4;
        R_R11 = context->X5;
        R_R12 = context->X19;
        R_R13 = context->X20;
        R_R14 = context->X21;
        R_R15 = context->X22;
    }
    if ((context->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {
        R_RSP = context->Sp;
        R_RBP = context->Fp;
        R_RIP = context->Pc;
        emu->eflags.x64 = context->AMD64_EFlags;
        emu->df = d_none;
    }
    if ((context->ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {
        R_CS = context->AMD64_SegCs;
        R_DS = context->AMD64_SegDs;
        R_ES = context->AMD64_SegEs;
        R_FS = context->AMD64_SegFs;
        R_GS = context->AMD64_SegGs;
        R_SS = context->AMD64_SegSs;
    }
    if ((context->ContextFlags & CONTEXT_FLOATING_POINT) ==
        CONTEXT_FLOATING_POINT) {
        emu->mxcsr.x32 = context->AMD64_MxCsr;
        x87_context_to_emu(context, emu);
        memcpy(emu->xmm, context->V,
               sizeof(emu->xmm) > sizeof(context->V)
                   ? sizeof(context->V) : sizeof(emu->xmm));
        if ((ymm = context_ymm(context)))
            memcpy(emu->ymm, ymm, sizeof(emu->ymm));
    }
    emu->win64_teb = (uint64_t)NtCurrentTeb();
    emu->segs_offs[_GS] = (uintptr_t)emu->win64_teb;
}

void emu_to_context(x64emu_t* emu, ARM64EC_NT_CONTEXT* context)
{
    void* ymm;
    uint32_t flags = Box64EC_GetEflags(emu);

    context->X8  = R_RAX;
    context->X0  = R_RCX;
    context->X1  = R_RDX;
    context->X27 = R_RBX;
    context->Sp  = R_RSP;
    context->Fp  = R_RBP;
    context->X25 = R_RSI;
    context->X26 = R_RDI;
    context->X2  = R_R8;
    context->X3  = R_R9;
    context->X4  = R_R10;
    context->X5  = R_R11;
    context->X19 = R_R12;
    context->X20 = R_R13;
    context->X21 = R_R14;
    context->X22 = R_R15;
    context->Pc  = R_RIP;
    context->AMD64_EFlags = flags;
    context->AMD64_SegCs = R_CS;
    context->AMD64_SegDs = R_DS;
    context->AMD64_SegEs = R_ES;
    context->AMD64_SegFs = R_FS;
    context->AMD64_SegGs = R_GS;
    context->AMD64_SegSs = R_SS;
    context->AMD64_MxCsr = emu->mxcsr.x32;
    if ((context->ContextFlags & CONTEXT_FLOATING_POINT) ==
        CONTEXT_FLOATING_POINT)
        x87_emu_to_context(emu, context);
    memcpy(context->V, emu->xmm,
           sizeof(context->V) > sizeof(emu->xmm)
               ? sizeof(emu->xmm) : sizeof(context->V));
    if ((ymm = context_ymm(context)))
        memcpy(ymm, emu->ymm, sizeof(emu->ymm));
}

extern NTSTATUS (WINAPI* __os_arm64x_get_x64_information)(ULONG, void*, void*);
extern NTSTATUS (WINAPI* __os_arm64x_set_x64_information)(ULONG, ULONG_PTR, void*);

static const uint32_t EC_VALID_EFLAGS =
    (1u << 0) | (1u << 6) | (1u << 7) | (1u << 8) | (1u << 11);

#define CONTEXT_ARM64_BOX64_YMMSTATE (CONTEXT_ARM64 | 0x00000040u)

static int read_live_mxcsr(uint32_t* mxcsr)
{
    return __os_arm64x_get_x64_information &&
           !__os_arm64x_get_x64_information(0, mxcsr, NULL);
}

static void write_live_mxcsr(uint32_t mxcsr)
{
    if (__os_arm64x_set_x64_information)
        __os_arm64x_set_x64_information(0, mxcsr, NULL);
}

x64emu_t* Box64EC_ApplyLiveMxcsr(x64emu_t* emu)
{
    if (emu)
        write_live_mxcsr(emu->mxcsr.x32);
    return emu;
}

x64emu_t* Box64EC_CaptureLiveMxcsr(x64emu_t* emu)
{
    uint32_t mxcsr;

    if (emu && read_live_mxcsr(&mxcsr))
        emu->mxcsr.x32 = mxcsr;
    return emu;
}

void Box64EC_CaptureNeonToContext(ARM64EC_NT_CONTEXT* context)
{
    uint32_t mxcsr;

    if (!context)
        return;
    __asm__ __volatile__(
        "stp q0,  q1,  [%0, #0x1a0]\n\t"
        "stp q2,  q3,  [%0, #0x1c0]\n\t"
        "stp q4,  q5,  [%0, #0x1e0]\n\t"
        :
        : "r"(context)
        : "memory");
    if (read_live_mxcsr(&mxcsr))
        context->AMD64_MxCsr = context->AMD64_MxCsr_copy = mxcsr;
}

void Box64EC_ApplyContextMxcsr(ARM64EC_NT_CONTEXT* context)
{
    if (context)
        write_live_mxcsr(context->AMD64_MxCsr);
}

void merge_eflags_from_emu(x64emu_t* emu, ARM64EC_NT_CONTEXT* context)
{
    uint32_t current = context->AMD64_EFlags;
    uint32_t flags = Box64EC_GetEflags(emu);

    context->AMD64_EFlags =
        (current & EC_VALID_EFLAGS) | (flags & ~EC_VALID_EFLAGS);
}

void merge_unmapped_context_from_emu(x64emu_t* emu,
                                     ARM64EC_NT_CONTEXT* context)
{
    merge_eflags_from_emu(emu, context);
    context->AMD64_ControlWord = emu->cw.x16;
    context->AMD64_StatusWord = x87_status_word(emu);
    uint8_t tag = x87_abridged_tag(emu);
    unsigned top = emu->top & 7;
    context->AMD64_TagWord = (tag << top) | (tag >> ((8 - top) & 7));
}

void emu_to_arm64_ec_packed(x64emu_t* emu, ARM64_NT_CONTEXT* context)
{
    uint8_t fp[8][16];
    uint64_t flags;

    memset(context, 0, sizeof(*context));
    context->ContextFlags = CONTEXT_ARM64_FULL;
    if (BOX64ENV(avx))
        context->ContextFlags |= CONTEXT_ARM64_BOX64_YMMSTATE;
    context->X8  = R_RAX;
    context->X0  = R_RCX;
    context->X1  = R_RDX;
    context->X27 = R_RBX;
    context->Sp  = R_RSP;
    context->Fp  = R_RBP;
    context->X25 = R_RSI;
    context->X26 = R_RDI;
    context->X2  = R_R8;
    context->X3  = R_R9;
    context->X4  = R_R10;
    context->X5  = R_R11;
    context->X19 = R_R12;
    context->X20 = R_R13;
    context->X21 = R_R14;
    context->X22 = R_R15;
    context->Pc  = R_RIP;
    memcpy(&context->V[0], &emu->xmm[0], sizeof(emu->xmm));
    if (BOX64ENV(avx))
        memcpy(&context->V[16], &emu->ymm[0], sizeof(emu->ymm));

    x87_slots_from_emu(emu, fp);
    memcpy(&context->Lr,  fp[0], 8);
    memcpy(&context->X6,  fp[1], 8);
    memcpy(&context->X7,  fp[2], 8);
    memcpy(&context->X9,  fp[3], 8);
    memcpy(&context->X10, fp[4], 8);
    memcpy(&context->X11, fp[5], 8);
    memcpy(&context->X12, fp[6], 8);
    memcpy(&context->X15, fp[7], 8);
    for (int i = 0; i < 4; ++i) {
        uint16_t hi;
        memcpy(&hi, fp[i] + 8, sizeof(hi));
        context->X16 |= (uint64_t)hi << (i * 16);
        memcpy(&hi, fp[i + 4] + 8, sizeof(hi));
        context->X17 |= (uint64_t)hi << (i * 16);
    }
    context->X13 = context->X14 = context->X18 = context->X23 =
        context->X24 = context->X28 = 0;
    flags = Box64EC_GetEflags(emu);
    context->Cpsr = 0;
    if (flags & (1u << 8))  context->Cpsr |= (1u << 21);
    if (flags & (1u << 0))  context->Cpsr |= (1u << 29);
    if (flags & (1u << 6))  context->Cpsr |= (1u << 30);
    if (flags & (1u << 7))  context->Cpsr |= (1u << 31);
    if (flags & (1u << 11)) context->Cpsr |= (1u << 28);
}
