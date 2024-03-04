#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"
#include "bitutils.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "dynarec_la64_helper.h"

uintptr_t dynarec64_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wb1, wback, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr);
            STORE_XEMU_CALL();
            CALL_S(x64Syscall, -1);
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LD_W(w1, xEmu, offsetof(x64emu_t, quit));
            CBZ_NEXT(w1);
            MARK;
            LOAD_XEMU_REM();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        #define GO(GETFLAGS, NO, YES, F, I)                                                            \
            READFLAGS(F);                                                                           \
            i32_ = F32S;                                                                            \
            BARRIER(BARRIER_MAYBE);                                                                 \
            JUMP(addr + i32_, 1);                                                                   \
            if (la64_lbt && (opcode - 0x70) >= 0xC) {                                               \
                X64_SET_EFLAGS(xFlags, F);                                                          \
                X64_SETJ(x1, I);                                                                    \
            } else {                                                                                \
                GETFLAGS;                                                                           \
            }                                                                                       \
            if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
                /* out of the block */                                                              \
                i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
                if (la64_lbt && (opcode - 0x70) >= 0xC)                                             \
                    BEQZ_safe(x1, i32);                                                             \
                else                                                                                \
                    B##NO##_safe(x1, i32);                                                          \
                if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
                    if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
                    jump_to_next(dyn, addr + i32_, 0, ninst, rex.is32bits);                         \
                } else {                                                                            \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
                    B(i32);                                                                         \
                }                                                                                   \
            } else {                                                                                \
                /* inside the block */                                                              \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
                if (la64_lbt && (opcode - 0x70) >= 0xC)                                             \
                    BNEZ_safe(x1, i32);                                                             \
                else                                                                                \
                    B##YES##_safe(x1, i32);                                                         \
            }

            GOCOND(0x80, "J", "Id");

        #undef GO

        default:
            DEFAULT;
    }
    return addr;
}
