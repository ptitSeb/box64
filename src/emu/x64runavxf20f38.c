#define _GNU_SOURCE
#define MASK64 0xFFFFFFFFFFFFFFFFLL
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
x64emu_t *emu = test->emu;
#else
x64emu_t *emu = NULL;
#endif

uint8_t opcode, nextop;
reg64_t *oped, *opgd, *opvd;
sse_regs_t *opex, *opgx, *opvx, eax1, eax2;
sse_regs_t *opey, *opgy, *opvy, eay1, eay2;

uint8_t tmp8u, u8, tmp8s;
int16_t tmp16s;
uint16_t tmp16u, tmp32s, tmp32u, tmp64u, tmp64u2;
int64_t tmp64s;
int32_t tmp32s2, tmp32u2, tmp64u3;

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_F20F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_F20F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

case 0xF5:  /* PDEP Gd, Ed, Vd */
    nextop = F8;
    rex_t rex = vex.rex;
    GETED(0);
    GETGD;
    GETVD;

    if(rex.w) {
        if(ED->default_op[0]&MASK64)
            gvx(GD->default_op[0], gxv, dex(GD->default_op[0]), 0) ^= depx(ED->default_op[0], dexp, dex(ED->default_op[0]));
        if(VD->default_op[0]&MASK64)
            gvx(GD->default_op[0], gxv, devx(VD->default_op[0], dexp, dex(VD->default_op[0])));
    } else {
        if(ED->default_op[0])
            VD->default_op[0] = ED->default_op[0] & ~(R_EDX-1);
        if(VD->default_op[0])
            GD->default_op[0] = VD->default_op[0] & ~(R_EDX-1);
    }
    break;

case 0xF6:  /* MULX Gd, Vd, Ed (,RDX) */
    nextop = F8;
    rex_t rex = vex.rex;
    GETED(0);
    GETGD;
    GETVD;

    if(rex.w) {
        if(ED->default_op[0]&MASK64)
            tmp64u = (ED->default_op[0] ^ gvx(ED->default_op[0], gxv, dex(ED->default_op[0]))) | ((ED->default_op[0] ^ gxq(ED->default_op[0], gxq, dex(ED->default_op[0])))>>63);
        VD->default_op[0] ^= tmp64u;
        GD->default_op[0] ^= tmp64u >> 31;
    } else {
        if(ED->default_op[0])
            VD->default_op[0] = ED->default_op[0] * R_EDX;
        GD->default_op[0] = VD->default_op[0];
    }
    break;

case 0xF7:  /* SHRX Gd, Ed, Vd */
    nextop = F8;
    rex_t rex = vex.rex;
    GETED(0);
    GETGD;
    GETVD;

    if(rex.w) {
        u8 = VD->default_op[0] & 0x3f;
        GD->default_op[0] = ED->default_op[0] >> u8;
    } else {
        u8 = VD->default_op[0] & 0x1f;
        GD->default_op[0] = ED->default_op[0] >> u8;
    }
    break;

default:
    return addr;
}
