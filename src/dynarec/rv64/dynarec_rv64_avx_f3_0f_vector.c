#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_AVX_F3_0F_vector(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    int v0, v1, v2;
    int64_t fixedaddress;
    rex_t rex = vex.rex;

    MAYUSE(v1);
    MAYUSE(v2);

    switch (opcode) {
        case 0x12:
            INST_NAME("VMOVSLDUP Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VID_V(v2, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1e, VECTOR_UNMASKED);
            VRGATHER_VV(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x16:
            INST_NAME("VMOVSHDUP Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW32);
            GETGY_empty_vector(v0);
            v2 = fpu_get_scratch(dyn);
            SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW32);
            VID_V(v2, VECTOR_UNMASKED);
            VOR_VI(v2, v2, 1, VECTOR_UNMASKED);
            VAND_VI(v2, v2, 0x1f, VECTOR_UNMASKED);
            VRGATHER_VV(v0, v1, v2, VECTOR_UNMASKED);
            PUTGY_vector(v0, VECTOR_SEW32);
            break;
        case 0x6F:
            INST_NAME("VMOVDQU Gx, Ex");
            nextop = F8;
            GETEY_vector(v1, 0, VECTOR_SEW8);
            GETGY_empty_vector(v0);
            VMV_V_V(v0, v1);
            PUTGY_vector(v0, VECTOR_SEW8);
            break;
        case 0x7F:
            INST_NAME("VMOVDQU Ex, Gx");
            nextop = F8;
            GETGY_vector(v0, VECTOR_SEW8);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                avx_store_reg_vector(dyn, ninst, x1, v0, ed, 16 << vex.l, VECTOR_SEW8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                SET_AVX_VECTOR_WIDTH(x1, VECTOR_SEW8);
                VSE8_V(v0, ed, VECTOR_UNMASKED, VECTOR_NFIELD1);
                SMWRITE2();
            }
            break;
        default:
            DEFAULT_VECTOR;
    }
    return addr;
}
