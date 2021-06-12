#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "arm64_printer.h"
#include "emu/x87emu_private.h"

#include "dynarec_arm64_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_DB(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)rep; (void)need_epilog;

    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback;
    uint8_t u8;
    int64_t fixedaddress;
    int v1, v2;
    int s0;
    int64_t j64;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j64);

    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FCMOVNB ST0, STx");
            READFLAGS(X_CF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTw_mask(xFlags, 0, 0);    //mask=1<<F_CF
            FCSELD(v1, v2, v1, cEQ);    // F_CF==0
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FCMOVNE ST0, STx");
            READFLAGS(X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTw_mask(xFlags, 0b011010, 0); //mask=1<<F_ZF
            FCSELD(v1, v2, v1, cEQ);        // F_ZF==0
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCMOVNBE ST0, STx");
            READFLAGS(X_CF|X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            MOV32w(x1, (1<<F_CF)|(1<<F_ZF));
            TSTw_REG(xFlags, x1);
            FCSELD(v1, v2, v1, cEQ);   // F_CF==0 & F_ZF==0
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FCMOVNU ST0, STx");
            READFLAGS(X_PF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTw_mask(xFlags, 0b011110, 0); //mask=1<<F_PF
            FCSELD(v1, v2, v1, cEQ);        // F_PF==0
            break;
        case 0xE1:
            INST_NAME("FDISI8087_NOP"); // so.. NOP?
            break;
        case 0xE2:
            INST_NAME("FNCLEX");
            LDRH_U12(x2, xEmu, offsetof(x64emu_t, sw));
            MOV32w(x1, 0);
            BFIw(x2, x1, 0, 8);  // IE .. PE, SF, ES
            BFIw(x2, x1, 15, 1); // B
            STRH_U12(x2, xEmu, offsetof(x64emu_t, sw));
            break;
        case 0xE3:
            INST_NAME("FNINIT");
            x87_purgecache(dyn, ninst, x1, x2, x3);
            CALL(reset_fpu, -1);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMI ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            FCMPD(v1, v2);
            FCOMI(x1, x2);
            break;
        case 0xF0:  
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FCOMI ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            FCMPD(v1, v2);
            FCOMI(x1, x2);
            break;

        case 0xE0:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ed");
                    v1 = x87_do_push(dyn, ninst);
                    s0 = fpu_get_scratch(dyn);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                    VLDR32_U12(s0, ed, fixedaddress);
                    SXTL_32(v1, s0);
                    SCVTFDD(v1, v1);
                    break;
                case 1:
                    INST_NAME("FISTTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    FRINT32ZD(s0, v1);
                    FCVTZSwD(ed, s0);
                    WBACK;
                    #else
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTZD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    VSTR32_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x80000000);
                    STRw_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_do_pop(dyn, ninst);
                    break;
                case 2:
                    INST_NAME("FIST Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x4); // x1 have the modified RPSCR reg
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    FRINT32XD(s0, v1);
                    FCVTZSwD(ed, s0);
                    WBACK;
                    #else
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    VSTR32_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x80000000);
                    STRw_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 3:
                    INST_NAME("FISTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x4); // x1 have the modified RPSCR reg
                    if(MODREG) {
                        ed = xRAX+(nextop&7)+(rex.b<<3);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, 0);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch(dyn);
                    #if 0
                    FRINT32XD(s0, v1);
                    FCVTZSwD(ed, s0);
                    WBACK;
                    #else
                    MRS_fpsr(x5);
                    BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                    MSR_fpsr(x5);
                    FRINTXD(s0, v1);
                    VFCVTZSd(s0, s0);
                    SQXTN_S_D(s0, s0);
                    VSTR32_U12(s0, wback, fixedaddress);
                    MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                    TBZ_MARK3(x5, FPSR_IOC);
                    MOV32w(x5, 0x80000000);
                    STRw_U12(x5, wback, fixedaddress);
                    MARK3;
                    #endif
                    x87_restoreround(dyn, ninst, u8);
                    x87_do_pop(dyn, ninst);
                    break;
                case 5:
                    INST_NAME("FLD tbyte");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                    if(PK(0)==0xDB && ((PK(1)>>3)&7)==7) {
                        // the FLD is immediatly followed by an FSTP
                        LDRx_U12(x5, ed, 0);
                        LDRH_U12(x6, ed, 8);
                        // no persistant scratch register, so unrool both instruction here...
                        MESSAGE(LOG_DUMP, "\tHack: FSTP tbyte\n");
                        nextop = F8;    //0xDB
                        nextop = F8;    //modrm
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                        STRx_U12(x5, ed, 0);
                        STRH_U12(x6, ed, 8);
                    } else {
                        if(ed!=x1) {
                            MOVx_REG(x1, ed);
                        }
                        x87_do_push_empty(dyn, ninst, x3);
                        CALL(arm_fld, -1);
                    }
                    break;
                case 7:
                    INST_NAME("FSTP tbyte");
                    x87_forget(dyn, ninst, x1, x3, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, rex, 0, 0);
                    if(ed!=x1) {
                        MOVx_REG(x1, ed);
                    }
                    CALL(arm_fstp, -1);
                    x87_do_pop(dyn, ninst);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}
