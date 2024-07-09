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
uintptr_t TestAVX_F30F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_F30F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
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

        case 0xF5:  /* PEXT Gd, Ed, Vd */
            nextop = F8;
            GETED(0);
            GETGD;
            GETVD;
            if(rex.w) {
                tmp64u = 0;
                u8 = 0;
                for(int i=0; i<64; ++i)
                    if(ED->q[0]&(1LL<<i))
                        tmp64u |= ((VD->q[0]>>i)&1LL)<<(u8++);
                GD->q[0] = tmp64u;
            } else {
                tmp32u = 0;
                u8 = 0;
                for(int i=0; i<32; ++i)
                    if(ED->dword[0]&(1<<i))
                        tmp32u |= ((VD->dword[0]>>i)&1)<<(u8++);
                GD->q[0] = tmp32u;
            }
            break;
        case 0xF7:  /* SARX Gd, Ed, Vd */
            nextop = F8;
            GETED(0);
            GETGD;
            GETVD;
            if(rex.w) {
                u8 = VD->q[0] & 0x3f;
                GD->sq[0] = ED->sq[0] >> u8;
            } else {
                u8 = VD->dword[0] & 0x1f;
                GD->sdword[0] = ED->sdword[0] >> u8;
                GD->dword[1] = 0;
            }
            break;

        default:
            return 0;
    }
    return addr;
}
