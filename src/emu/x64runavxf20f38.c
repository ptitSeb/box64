#define *GNU*SOURCE
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
uintptr_t TestAVX_F20F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_F20F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t u8;
    uint32_t tmp32u;
    uint64_t tmp64u;
    reg64_t *oped, *opgd, *opvd;
    sse_regs_t *opex, *opgx, *opvx;
#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    rex_t rex = vex.rex;

    switch(opcode) {
        case 0xF5:  /* PDEP Gd, Ed, Vd */
            nextop = F8;
            GETED(0);
            GETGD;
            GETVD;
            if(rex.w) {
                tmp64u = 0;
                u8 = 0;
                for(int i=0; i<64; ++i) {
                    if(ED->q[0] & (1ULL << i)) {
                        tmp64u |= ((VD->q[0] >> u8) & 1ULL) << i;
                        ++u8;
                    }
                }
                GD->q[0] = tmp64u;
            } else {
                tmp32u = 0;
                u8 = 0;
                for(int i=0; i<32; ++i) {
                    if(ED->dword[0] & (1U << i)) {
                        tmp32u |= ((VD->dword[0] >> u8) & 1U) << i;
                        ++u8;
                    }
                }
                GD->q[0] = tmp32u;
            }
            break;
        case 0xF6:  /* MULX Gd, Vd, Ed (,RDX) */
            nextop = F8;
            GETED(0);
            GETGD;
            GETVD;
            if(rex.w) {
                __uint128_t res = (__uint128_t)R_RDX * ED->q[0];
                VD->q[0] = (uint64_t)res;
                GD->q[0] = (uint64_t)(res >> 64);
            } else {
                tmp64u = (uint64_t)R_EDX * ED->dword[0];
                VD->q[0] = (uint32_t)tmp64u;
                GD->q[0] = (uint32_t)(tmp64u >> 32);
            }
            break;
        case 0xF7:  /* SHRX Gd, Ed, Vd */
            nextop = F8;
            GETED(0);
            GETGD;
            GETVD;
            if(rex.w) {
                u8 = VD->q[0] & 0x3f;
                GD->q[0] = ED->q[0] >> u8;
            } else {
                u8 = VD->dword[0] & 0x1f;
                GD->q[0] = ED->dword[0] >> u8;
            }
            break;
        default:
            return 0;
    }
    return addr;
}
