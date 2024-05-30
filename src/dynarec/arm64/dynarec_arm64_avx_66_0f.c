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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

uintptr_t dynarec64_AVX_66_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2, gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    uint64_t tmp64u;
    int64_t j64;
    int64_t fixedaddress;
    int unscaled;
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(j64);
    MAYUSE(cacheupd);

    rex_t rex = vex.rex;

    switch(opcode) {

        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if(box64_dynarec_fastround) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                VFRINTISQ(v0, v1);
                if(!vex.l) x87_restoreround(dyn, ninst, u8);
                VFCVTZSQS(v0, v0);
            } else {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                MOV32w(x4, 0x80000000);
                d0 = fpu_get_scratch(dyn, ninst);
                for(int i=0; i<4; ++i) {
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    VMOVeS(d0, 0, v1, i);
                    FRINTIS(d0, d0);
                    VFCVTZSs(d0, d0);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ(x5, FPSR_IOC, 4+4);
                    VMOVQSfrom(d0, 0, x4);
                    VMOVeS(v0, i, d0, 0);
                }
                if(!vex.l) x87_restoreround(dyn, ninst, u8);
            }
            if(vex.l) {
                GETGY_empty_EY(v0, v1);
                if(box64_dynarec_fastround) {
                    VFRINTISQ(v0, v1);
                    x87_restoreround(dyn, ninst, u8);
                    VFCVTZSQS(v0, v0);
                } else {
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    MOV32w(x4, 0x80000000);
                    d0 = fpu_get_scratch(dyn, ninst);
                    for(int i=0; i<4; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        VMOVeS(d0, 0, v1, i);
                        FRINTIS(d0, d0);
                        VFCVTZSs(d0, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        VMOVQSfrom(d0, 0, x4);
                        VMOVeS(v0, i, d0, 0);
                    }
                    x87_restoreround(dyn, ninst, u8);
                }
            } else YMM0(gd);
            break;

        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                GETGX_empty(v0);
                VMOVQ(v0, v1);
                if(vex.l) {
                    GETGY_empty_EY(v0, v1);
                    VMOVQ(v0, v1);
                }
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
                if(vex.l) {
                    GETGY_empty(v0, -1, -1, -1);
                    VLD128(v0, ed, fixedaddress+16);
                }
            }
            if(!vex.l) YMM0(gd);
            break;

        default:
            DEFAULT;
    }
    return addr;
}
