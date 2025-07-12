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
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "my_cpuid.h"
#include "bridge.h"
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
uintptr_t Test64AVX(x64test_t *test, vex_t vex, uintptr_t addr, uintptr_t tlsdata)
#else
uintptr_t Run64AVX(x64emu_t *emu, vex_t vex, uintptr_t addr, uintptr_t tlsdata)
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
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, *opvx, eax1;
    sse_regs_t *opey, *opgy, *opvy, eay1;

#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif

    opcode = PK(0);
    if( (vex.m==VEX_M_0F) && (vex.p==VEX_P_NONE)) {
        opcode = F8;
        rex_t rex = vex.rex;
        switch(opcode) {

            case 0x10:  /* VMOVUPS Gx, Ex */
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                GETGY;
                GX->q[0] = EX->q[0];
                GX->q[1] = EX->q[1];
                if(vex.l) {
                    GETEY;
                    GY->q[0] = EY->q[0];
                    GY->q[1] = EY->q[1];
                } else {
                    GY->u128 = 0;
                }
                break;
            case 0x11:  /* VMOVUPS Ex, Gx */
                nextop = F8;
                GETEX_OFFS(0, tlsdata);
                GETGX;
                EX->q[0] = GX->q[0];
                EX->q[1] = GX->q[1];
                if(vex.l) {
                    GETEY;
                    GETGY;
                    EY->q[0] = GY->q[0];
                    EY->q[1] = GY->q[1];
                } else if(MODREG) {
                    GETEY;
                    EY->u128 = 0;
                }
                break;
            default:
                addr = 0;
            }
    }
    /*else  if( (vex.m==VEX_M_0F38) && (vex.p==VEX_P_NONE))
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
        addr = RunAVX_F30F3A(emu, vex, addr, step);*/
    else addr = 0;

    if(!addr)
        printf_log(LOG_INFO, "Unimplemented 64/65 AVX opcode size %d prefix %s map %s opcode %02X ", 128<<vex.l, avx_prefix_string(vex.p), avx_map_string(vex.m), opcode);

    return addr;
}
