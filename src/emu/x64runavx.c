#define _GNU_SOURCE
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
#include "x64shaext.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

static const char* avx_prefix_string(uint16_t p)
{
    switch(p) {
        case VEX_P_NONE: return "0";
        case VEX_P_66: return "66";
        case VEX_P_F2: return "F2";
        case VEX_P_F3: return "F3";
        default: return "??";
    }
}
static const char* avx_map_string(uint16_t m)
{
    switch(m) {
        case VEX_M_NONE: return "0";
        case VEX_M_0F: return "0F";
        case VEX_M_0F38: return "0F38";
        case VEX_M_0F3A: return "0F3A";
        default: return "??";
    }
}

#ifdef TEST_INTERPRETER
uintptr_t TestAVX(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
#ifdef TEST_INTERPRETER
    if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_NONE))
        addr = TestAVX_0F(test, vex, addr, step);
    else  if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_NONE))
        addr = TestAVX_0F38(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_66))
        addr = TestAVX_660F(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_F2))
        addr = TestAVX_F20F(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_F3))
        addr = TestAVX_F30F(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_66))
        addr = TestAVX_660F38(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_66))
        addr = TestAVX_660F3A(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F2))
        addr = TestAVX_F20F38(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F3))
        addr = TestAVX_F30F38(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_F2))
        addr = TestAVX_F20F3A(test, vex, addr, step);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_F3))
        addr = TestAVX_F30F3A(test, vex, addr, step);
    else addr = 0;
#else
    uint8_t opcode = PK(0);
    if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_NONE))
        addr = RunAVX_0F(emu, vex, addr, step);
    else  if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_NONE))
        addr = RunAVX_0F38(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_66))
        addr = RunAVX_660F(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_F2))
        addr = RunAVX_F20F(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_F3))
        addr = RunAVX_F30F(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_66))
        addr = RunAVX_660F38(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_66))
        addr = RunAVX_660F3A(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F2))
        addr = RunAVX_F20F38(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F3))
        addr = RunAVX_F30F38(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_F2))
        addr = RunAVX_F20F3A(emu, vex, addr, step);
    else if( (vex.m==VEX_M_0F3A) && (vex.p==VEX_P_F3))
        addr = RunAVX_F30F3A(emu, vex, addr, step);
    else addr = 0;

    if(!addr)
        printf_log(LOG_INFO, "Unimplemented AVX opcode size %d prefix %s map %s opcode %02X ", 128<<vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);
#endif

    return addr;
}
