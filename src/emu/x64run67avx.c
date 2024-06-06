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
uintptr_t Test67AVX(x64test_t *test, vex_t vex, uintptr_t addr)
#else
uintptr_t Run67AVX(x64emu_t *emu, vex_t vex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s;
    reg64_t *oped, *opgd, *opvd;
    sse_regs_t *opex, *opgx, *opvx, eax1;
    sse_regs_t *opey, *opgy, *opvy, eay1;

#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_F2))
        switch(opcode) {

            case 0xF6:  /* MULX Gd, Vd, Ed (,RDX) */
                nextop = F8;
                GETED32(0);
                GETGD;
                GETVD;
                if(rex.w) {
                    unsigned __int128 res = (unsigned __int128)R_RDX * ED->q[0];
                    VD->q[0] = res&0xFFFFFFFFFFFFFFFFLL;
                    GD->q[0] = res>>64;
                } else {
                    tmp64u = (uint64_t)R_EDX * ED->dword[0];
                    VD->q[0] = tmp64u&0xFFFFFFFF;
                    GD->q[0] = tmp64u>>32;
                }
                break;

            default: addr = 0;
        }
    else addr = 0;

    if(!addr)
        printf_log(LOG_INFO, "Unimplemented 67 AVX opcode size %d prefix %s map %s opcode %02X ", 128<<vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);

    return addr;
}
