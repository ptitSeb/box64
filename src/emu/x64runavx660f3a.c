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

static __int128_t pclmul_helper(uint64_t X, uint64_t Y)
{
    __int128 result = 0;
    __int128 op2 = Y;
    for (int i=0; i<64; ++i)
        if(X&(1LL<<i))
            result ^= (op2<<i);

    return result;
}

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_660F3A(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_660F3A(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
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
    opcode = F8;

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x44:    /* VPCLMULQDQ Gx, Vx, Ex, imm8 */
            nextop = F8;
            GETGX;
            GETEX(1);
            GETVX;
            GETGY;
            tmp8u = F8;
            GX->u128 = pclmul_helper(VX->q[tmp8u&1], EX->q[(tmp8u>>4)&1]);
            if(vex.l) {
                GETVY;
                GETEY;
                GY->u128 = pclmul_helper(VY->q[tmp8u&1], EY->q[(tmp8u>>4)&1]);
            } else
                GY->q[0] = GY->q[1] = 0;
            break;

        default:
            return 0;
    }
    return addr;
}
