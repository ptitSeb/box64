#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include "bridge.h"
#ifdef DYNAREC
#include "../dynarec/arm64_lock.h"
#endif

#include "modrm.h"

int Run670F(x64emu_t *emu, rex_t rex, int rep)
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int32_t tmp32s, tmp32s2;
    uint32_t tmp32u, tmp32u2;
    uint64_t tmp64u, tmp64u2;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1;
    mmx87_regs_t *opem, *opgm, eam1;

    opcode = F8;

    switch(opcode) {

        case 0x2E:                      /* UCOMISS Gx, Ex */
            // same for now
        case 0x2F:                      /* COMISS Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GETEX32(0);
            GETGX;
            if(isnan(GX->f[0]) || isnan(EX->f[0])) {
                SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
            } else if(isgreater(GX->f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            } else if(isless(GX->f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
            } else {
                SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            }
            CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
            break;

    default:
        return 1;
    }
    return 0;
}