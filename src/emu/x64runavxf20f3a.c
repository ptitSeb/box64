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
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_F20F3A(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_F20F3A(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u, u8;
    int8_t tmp8s;
    int16_t tmp16s;
    uint16_t tmp16u;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    int64_t tmp64s;
    reg64_t *oped, *opgd, *opvd;
    sse_regs_t *opex, *opgx, *opvx, eax1, eax2;
    sse_regs_t *opey, *opgy, *opvy, eay1, eay2;


#ifdef TEST_INTERPRETER
    x64emu_t *emu = test->emu;
#endif
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0xF0:  /* RORX Gd, Ed, Ib */
            nextop = F8;
            GETED(1);
            GETGD;
            u8 = F8;
            if(rex.w) {
                u8 &= 0x3f;
                GD->q[0] = (ED->q[0] << (64 - u8)) + ((ED->q[0] >> (u8)) & ((1L << (64 - u8)) - 1L));
            } else {
                u8 &= 0x1f;
                ED->dword[0] = (ED->dword[0] << (32 - u8)) + ((ED->dword[0] >> (u8)) & ((1 << (32 - u8)) - 1));
                if(MODREG)
                    ED->dword[1] = 0;
            }
            break;

        default:
            return 0;
    }
    return addr;
}
