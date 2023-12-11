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
#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"
#include "x64compstrings.h"

#ifdef TEST_INTERPRETER
uintptr_t Test670F(x64test_t *test, rex_t rex, int rep, uintptr_t addr)
#else
uintptr_t Run670F(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr)
#endif
{
    (void)rep;
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;                      (void)tmp8u;
    int8_t tmp8s;                       (void)tmp8s;
    int32_t tmp32s, tmp32s2;            (void)tmp32s; (void)tmp32s2;
    uint32_t tmp32u, tmp32u2;           (void)tmp32u; (void)tmp32u2;
    uint64_t tmp64u, tmp64u2;           (void)tmp64u; (void)tmp64u2;
    reg64_t *oped, *opgd;               (void)oped;   (void)opgd;
    sse_regs_t *opex, *opgx, eax1;      (void)eax1;
    mmx87_regs_t *opem, *opgm, eam1;    (void)opem;   (void)opgm;    (void)eam1;

    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    switch(opcode) {

        case 0x11:
            switch(rep) {
                case 0:                      /* MOVUPS Ex,Gx */
                    nextop = F8;
                    GETEX32(0);
                    GETGX;
                    EX->u128 = GX->u128;
                    break;
                default:
                    return 0;
            }
            break;

        case 0x29:                      /* MOVAPS Ex,Gx */
            nextop = F8;
            GETEX32(0);
            GETGX;
            EX->q[0] = GX->q[0];
            EX->q[1] = GX->q[1];
            break;

        case 0x2E:
            // same for now
        case 0x2F:                      
            switch(rep) {
                case 0: /* (U)COMISS Gx, Ex */
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
                    return 0;
            }
            break;

        case 0x6F:
            switch(rep) {
                case 0: /* MOVQ Gm, Em */
                    nextop = F8;
                    GETEM32(0);
                    GETGM;
                    GM->q = EM->q;
                    break;
                case 2:  /* MOVDQU Gx, Ex */
                    nextop = F8;
                    GETEX32(0);
                    GETGX;
                    memcpy(GX, EX, 16);    // unaligned...
                    break;
                default:
                    return 0;
            }
            break;

        case 0x7F:
            switch(rep) {
                case 0: /* MOVQ Em, Gm */ 
                    nextop = F8;
                    GETEM32(0);
                    GETGM;
                    EM->q = GM->q;
                    break;
                default:
                    return 0;
            }
            break;

        case 0xB6:                      /* MOVZX Gd,Eb */
            nextop = F8;
            GETEB32(0);
            GETGD;
            GD->q[0] = EB->byte[0];
            break;

        case 0xB9:
            switch(rep) {
                case 0: /* UD1 Ed */
                    nextop = F8;
                    FAKEED32(0);
                    #ifndef TEST_INTERPRETER
                    emit_signal(emu, SIGILL, (void*)R_RIP, 0);
                    #endif
                    break;
                default:
                    return 0;
            }
            break;

        case 0xB7:                      /* MOVZX Gd,Ew */
            nextop = F8;
            GETEW32(0);
            GETGD;
            GD->q[0] = EW->word[0];
            break;

    default:
        return 0;
    }
    return addr;
}
