#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "x64_signals.h"
#include "os.h"
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
#include "emit_signals.h"
#ifdef DYNAREC
#include "custommem.h"
#include "../dynarec/native_lock.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestAVX_0F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step)
#else
uintptr_t RunAVX_0F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step)
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

        case 0xF2:  /* ANDN Gd, Vd, Ed */
            nextop = F8;
            if(vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            ResetFlags(emu);
            GETGD;
            GETED(0);
            GETVD;
            if(rex.w)
                GD->q[0] = ED->q[0] & ~VD->q[0];
            else
                GD->q[0] = ED->dword[0] & ~VD->dword[0];
            CONDITIONAL_SET_FLAG(GD->q[0]==0, F_ZF);
            CONDITIONAL_SET_FLAG(rex.w?(GD->q[0]>>63):(GD->dword[0]>>31), F_SF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_AF);   // Undef
            CLEAR_FLAG(F_PF);   // Undef
            break;
        case 0xF3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 1:     /* BLSR Vd, Ed */
                    if(vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                    ResetFlags(emu);
                    GETVD;
                    GETED(0);
                    CONDITIONAL_SET_FLAG(rex.w?(ED->q[0]==0):(ED->dword[0]==0), F_CF);
                    if(rex.w)
                        VD->q[0] = ED->q[0] & (ED->q[0]-1LL);
                    else
                        VD->q[0] = ED->dword[0] & (ED->dword[0]-1);
                    CONDITIONAL_SET_FLAG(VD->q[0]==0, F_ZF);
                    CONDITIONAL_SET_FLAG(rex.w?(VD->q[0]>>63):(VD->dword[0]>>31), F_SF);
                    CLEAR_FLAG(F_OF);
                    CLEAR_FLAG(F_AF);   // Undef
                    if(BOX64ENV(cputype)) {
                        CLEAR_FLAG(F_PF);
                    } else {
                        CONDITIONAL_SET_FLAG(PARITY(VD->byte[0] & 0xff), F_PF);
                    }
                    break;
                case 2:     /* BLSMSK Vd, Ed */
                    if(vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                    ResetFlags(emu);
                    GETVD;
                    GETED(0);
                    CONDITIONAL_SET_FLAG(rex.w?(ED->q[0]==0):(ED->dword[0]==0), F_CF);
                    if(rex.w)
                        VD->q[0] = ED->q[0] ^ (ED->q[0]-1LL);
                    else
                        VD->q[0] = ED->dword[0] ^ (ED->dword[0]-1);
                    CONDITIONAL_SET_FLAG(rex.w?(VD->q[0]>>63):(VD->dword[0]>>31), F_SF);
                    CLEAR_FLAG(F_ZF);
                    CLEAR_FLAG(F_OF);
                    // Undef flags
                    CLEAR_FLAG(F_AF);
                    if(BOX64ENV(cputype)) {
                        CLEAR_FLAG(F_PF);
                    } else {
                        CONDITIONAL_SET_FLAG(PARITY(VD->byte[0] & 0xff), F_PF);
                    }
                    break;
                case 3:     /* BLSI Vd, Ed */
                    if(vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
                    ResetFlags(emu);
                    GETVD;
                    GETED(0);
                    CONDITIONAL_SET_FLAG(rex.w?(ED->q[0]!=0):(ED->dword[0]!=0), F_CF);
                    if(rex.w)
                        VD->sq[0] = ED->sq[0] & (-ED->sq[0]);
                    else
                        VD->sdword[0] = ED->sdword[0] & (-ED->sdword[0]);
                    if(!rex.w)
                        VD->dword[1] = 0;
                    CONDITIONAL_SET_FLAG(rex.w?(VD->q[0]==0):(VD->dword[0]==0), F_ZF);
                    CONDITIONAL_SET_FLAG(rex.w?(VD->q[0]>>63):(VD->dword[0]>>31), F_SF);
                    CLEAR_FLAG(F_OF);
                    CLEAR_FLAG(F_AF);   // Undef
                    if(BOX64ENV(cputype)) {
                        CLEAR_FLAG(F_PF);
                    } else {
                        CONDITIONAL_SET_FLAG(PARITY(VD->byte[0] & 0xff), F_PF);
                    }

                    break;
                default:
                    return 0;
            }
            break;

        case 0xF5:  /* BZHI Gd, Ed, Vd */
            nextop = F8;
            if(vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            GETGD;
            GETED(0);
            GETVD;
            tmp32u = VD->byte[0];   // start
            if(rex.w) {
                GD->q[0] = ED->q[0];
                if(tmp32u<64) GD->q[0] &= ~((((uint64_t)-1)<<tmp32u));
                CONDITIONAL_SET_FLAG((tmp32u>63), F_CF);
            } else {
                GD->q[0] = ED->dword[0];
                if(tmp32u<32) GD->dword[0] &= ~((((uint64_t)-1)<<tmp32u));
                CONDITIONAL_SET_FLAG((tmp32u>31), F_CF);
            }
            CONDITIONAL_SET_FLAG(rex.w?(GD->q[0]==0):(GD->dword[0]==0), F_ZF);
            CONDITIONAL_SET_FLAG(rex.w?(GD->q[0]>>63):(GD->dword[0]>>31), F_SF);
            CLEAR_FLAG(F_OF);
            // UD flags
            CLEAR_FLAG(F_AF);
            if(BOX64ENV(cputype)) {
                CLEAR_FLAG(F_PF);
            } else {
                CONDITIONAL_SET_FLAG(PARITY(VD->byte[0] & 0xff), F_PF);
            }
            break;

        case 0xF7:  /* BEXTR Gd, Ed, Vd */
            nextop = F8;
            if(vex.l) EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
            ResetFlags(emu);
            GETGD;
            GETED(0);
            GETVD;
            tmp32u = VD->byte[0];   // start
            tmp32u2 = VD->byte[1];  // end
            if(rex.w) {
                if(tmp32u2<0x40)
                    tmp64u = ((1LL<<tmp32u2)-1LL);
                else
                    tmp64u = -1LL;
                if(tmp32u<0x40)
                    GD->q[0] = (ED->q[0]>>tmp32u)&tmp64u;
                else
                    GD->q[0] = 0;
            } else {
                if(tmp32u2<0x20)
                    tmp64u = ((1LL<<tmp32u2)-1LL);
                else
                    tmp64u = -1LL;
                if(tmp32u<0x20)
                    GD->dword[0] = (ED->dword[0]>>tmp32u)&tmp64u;
                else
                    GD->dword[0] = 0;
                if(MODREG)
                    GD->dword[1] = 0;
            }
            ResetFlags(emu);
            CONDITIONAL_SET_FLAG(rex.w?(GD->q[0]==0):(GD->dword[0]==0), F_ZF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_AF);   // Undef
            CLEAR_FLAG(F_SF);   // Undef
            CLEAR_FLAG(F_PF);   // Undef
            break;

        default:
            return 0;
    }
    return addr;
}
