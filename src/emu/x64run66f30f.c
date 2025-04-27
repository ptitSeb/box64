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
#include "bridge.h"

#include "modrm.h"
#include "x64compstrings.h"

#ifdef TEST_INTERPRETER
uintptr_t Test66F30F(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t Run66F30F(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int16_t tmp16s;
    uint16_t tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u;
    uint64_t tmp64u;
    int64_t tmp64s, i64[4];
    float tmpf;
    double tmpd;
    #ifndef NOALIGN
    int is_nan;
    #endif
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1, *opex2;
    mmx87_regs_t *opem, *opgm;

    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    switch(opcode) {

    case 0xBC:  /* TZCNT Ed,Gd */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w) {
            tmp64u = ED->q[0];
            if(tmp64u) {
                tmp8u = 0;
                while(!(tmp64u&(1LL<<tmp8u))) ++tmp8u;
                GD->q[0] = tmp8u;
                CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
                CLEAR_FLAG(F_CF);
            } else {
                CLEAR_FLAG(F_ZF);
                SET_FLAG(F_CF);
                GD->q[0] = 64;
            }
        } else {
            tmp32u = ED->word[0];
            if(tmp32u) {
                tmp8u = 0;
                while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                GD->word[0] = tmp8u;
                CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
                CLEAR_FLAG(F_CF);
            } else {
                CLEAR_FLAG(F_ZF);
                SET_FLAG(F_CF);
                GD->word[0] = 16;
            }
        }
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_SF);
        CLEAR_FLAG(F_PF);
        if(!BOX64ENV(cputype))
            CLEAR_FLAG(F_OF);
        break;
    case 0xBD:  /* LZCNT Ed,Gd */
        CHECK_FLAGS(emu);
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w) {
            tmp64u = ED->q[0];
            tmp8u = (tmp64u)?__builtin_clzl(tmp64u):64;
            CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u==64, F_CF);
            GD->q[0] = tmp8u;
        } else {
            tmp32u = EW->word[0];
            tmp8u = (tmp32u)?__builtin_clz(tmp32u<<16):16;
            CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u==16, F_CF);
            GD->word[0] = tmp8u;
        }
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_SF);
        CLEAR_FLAG(F_PF);
        if(!BOX64ENV(cputype))
            CLEAR_FLAG(F_OF);
        break;

    case 0xB8:  /* POPCNT Gd,Ed */
        nextop = F8;
        GETEW(0);
        GETGW;
        if(rex.w)
            GD->q[0] = __builtin_popcountl(ED->q[0]);
        else
            GD->q[0] = __builtin_popcount(ED->word[0]);
        RESET_FLAGS(emu);
        CLEAR_FLAG(F_OF);
        CLEAR_FLAG(F_SF);
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CONDITIONAL_SET_FLAG(GD->q[0]==0, F_ZF);
        break;

    default:
        return 0;
    }
    return addr;
}
