#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "../dynarec_helper.h"

int isSimpleWrapper(wrapper_t fun);
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00_3(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    int lock;
    int cacheupd = 0;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j64);
    MAYUSE(wb);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    switch (opcode) {
        case 0xC0:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(const_rol8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(const_ror8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(const_rcl8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 1);
                    u8 = F8;
                    MOV32w(x2, u8);
                    CALL_(const_rcr8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_shl8c(dyn, ninst, ed, u8, x4, x5, x6);
                        EBBACK(x5, 0);
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_shr8c(dyn, ninst, ed, u8, x4, x5, x6);
                        EBBACK(x5, 0);
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETSEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_sar8c(dyn, ninst, ed, u8, x4, x5, x6);
                        EBBACK(x5, 0);
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                        GETED(1);
                        F8;
                        emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                        GETED(1);
                        F8;
                        emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEDW(x4, x1, 0);
                    u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                    MOV32w(x2, u8);
                    CALL_(rex.w ? (const_rcl64) : (const_rcl32), ed, x4, x1, x2);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEDW(x4, x1, 0);
                    u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                    MOV32w(x2, u8);
                    CALL_(rex.w ? (const_rcr64) : (const_rcr32), ed, x4, x1, x2);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                        GETED(1);
                        F8;
                        emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                        GETED(1);
                        F8;
                        emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 0x3f : 0x1f);
                    // flags are not affected if count is 0, we make it a nop if possible.
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                        GETED(1);
                        F8;
                        emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w) {
                            GETED(1);
                            ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xC2:
            INST_NAME("RETN");
            // SETFLAGS(X_ALL, SF_SET_NODF);    // Hack, set all flags (to an unknown state...)
            if (BOX64DRENV(dynarec_safeflags)) {
                READFLAGS(X_PEND); // lets play safe here too
            }
            fpu_purgecache(dyn, ninst, 1, x1, x2, x3); // using next, even if there no next
            i32 = F16;
            retn_to_epilog(dyn, ip, ninst, rex, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET_NODF);    // Hack, set all flags (to an unknown state...)
            if (BOX64DRENV(dynarec_safeflags)) {
                READFLAGS(X_PEND); // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            fpu_purgecache(dyn, ninst, 1, x1, x2, x3); // using next, even if there no next
            ret_to_epilog(dyn, ip, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC4:
            nextop = F8;
            if (rex.is32bits && !(MODREG)) {
                DEFAULT;
            } else {
                vex_t vex = { 0 };
                vex.rex = rex;
                u8 = nextop;
                vex.m = u8 & 0b00011111;
                vex.rex.b = (u8 & 0b00100000) ? 0 : 1;
                vex.rex.x = (u8 & 0b01000000) ? 0 : 1;
                vex.rex.r = (u8 & 0b10000000) ? 0 : 1;
                u8 = F8;
                vex.p = u8 & 0b00000011;
                vex.l = (u8 >> 2) & 1;
                vex.v = ((~u8) >> 3) & 0b1111;
                vex.rex.w = (u8 >> 7) & 1;
                addr = dynarec64_AVX(dyn, addr, ip, ninst, vex, ok, need_epilog);
            }
            break;
        case 0xC5:
            nextop = F8;
            if (rex.is32bits && !(MODREG)) {
                DEFAULT;
            } else {
                vex_t vex = { 0 };
                vex.rex = rex;
                u8 = nextop;
                vex.p = u8 & 0b00000011;
                vex.l = (u8 >> 2) & 1;
                vex.v = ((~u8) >> 3) & 0b1111;
                vex.rex.r = (u8 & 0b10000000) ? 0 : 1;
                vex.rex.b = 0;
                vex.rex.x = 0;
                vex.rex.w = 0;
                vex.m = VEX_M_0F;
                addr = dynarec64_AVX(dyn, addr, ip, ninst, vex, ok, need_epilog);
            }
            break;
        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop = F8;
            if (MODREG) { // reg <= u8
                u8 = F8;
                if (!rex.rex) {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                } else {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                }

                if (eb2) {
                    // load a mask to x3 (ffffffffffff00ff)
                    LUI(x3, 0xffff0);
                    ORI(x3, x3, 0xff);
                    // apply mask
                    AND(eb1, eb1, x3);
                    if (u8) {
                        if ((u8 << 8) < 2048) {
                            ADDI(x4, xZR, u8 << 8);
                        } else {
                            ADDI(x4, xZR, u8);
                            SLLI(x4, x4, 8);
                        }
                        OR(eb1, eb1, x4);
                    }
                } else {
                    ANDI(eb1, eb1, 0xf00); // mask ffffffffffffff00
                    ORI(eb1, eb1, u8);
                }
            } else { // mem <= u8
                SCRATCH_USAGE(0);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 1, 1);
                u8 = F8;
                if (u8) {
                    SCRATCH_USAGE(1);
                    ADDI(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                SB(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop = F8;
            if (MODREG) { // reg <= i32
                SCRATCH_USAGE(0);
                i64 = F32S;
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                MOV64xw(ed, i64);
            } else { // mem <= i32
                IF_UNALIGNED(ip) {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, (1 << (2 + rex.w)) - 1, 4);
                    i64 = F32S;
                    if (i64) {
                        MOV64x(x4, i64);
                        ed = x4;
                    } else
                        ed = xZR;
                    for (int i = 0; i < (1 << (2 + rex.w)); i++) {
                        if (i == 0 || ed == xZR) {
                            SB(ed, wback, fixedaddress + i);
                        } else {
                            SRLI(x3, ed, i * 8);
                            SB(x3, wback, fixedaddress + i);
                        }
                    }
                } else {
                    SCRATCH_USAGE(0);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 1, 4);
                    i64 = F32S;
                    if (i64) {
                        SCRATCH_USAGE(1);
                        MOV64x(x3, i64);
                        ed = x3;
                    } else
                        ed = xZR;
                    SDxw(ed, wback, fixedaddress);
                }
                SMWRITELOCK(lock);
            }
            break;

        case 0xC9:
            INST_NAME("LEAVE");
            MVz(xRSP, xRBP);
            POP1z(xRBP);
            break;

        case 0xCC:
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack, set all flags (to an unknown state...)
            SKIPTEST(x1);
            if (IsBridgeSignature(PK(0), PK(1))) {
                addr += 2;
                BARRIER(BARRIER_FLOAT);
                INST_NAME("Special Box64 instruction");
                if ((PK64(0) == 0)) {
                    addr += 8;
                    MESSAGE(LOG_DEBUG, "Exit x64 Emu\n");
                    MOV64x(x1, 1);
                    SW(x1, xEmu, offsetof(x64emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip)));
                    x87_stackcount(dyn, ninst, x1);
                    x87_forget(dyn, ninst, x3, x4, 0);
                    sse_purge07cache(dyn, ninst, x3);
                    // Partially support isSimpleWrapper
                    tmp = isSimpleWrapper(*(wrapper_t*)(addr));
                    if (isRetX87Wrapper(*(wrapper_t*)(addr)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if (tmp < 0 || (tmp & 15) > 1)
                        tmp = 0; // TODO: removed when FP is in place
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && tmp) {
                        call_n(dyn, ninst, (void*)(addr + 8), tmp);
                        addr += 8 + 8;
                    } else {
                        GETIP(ip + 1, x7); // read the 0xCC
                        STORE_XEMU_CALL(x3);
                        ADDI(x3, xRIP, 8 + 8 + 2);                        // expected return address
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_(const_int3, -1, x3, x1, 0);
                        LOAD_XEMU_CALL();
                        addr += 8 + 8;
                        BNE_MARK(xRIP, x3);
                        LW(x1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(x1);
                        MARK;
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                    }
                }
            } else {
                INST_NAME("INT 3");
                if (!BOX64ENV(ignoreint3)) {
                    // check if TRAP signal is handled
                    TABLE64C(x1, const_context);
                    MOV32w(x2, offsetof(box64context_t, signals[X64_SIGTRAP]));
                    ADD(x2, x2, x1);
                    LD(x3, x2, 0);
                    BEQZ_MARK(x3);
                    GETIP(addr, x7);
                    STORE_XEMU_CALL(x3);
                    CALL(const_native_int3, -1, 0, 0);
                    LOAD_XEMU_CALL();
                    MARK;
                    jump_to_epilog(dyn, addr, 0, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                }
                break;
            }
            break;
        case 0xCD:
            u8 = F8;
            NOTEST(x1);
            if (box64_wine && (u8 == 0x2D || u8 == 0x2C || u8 == 0x29)) {
                INST_NAME("INT 29/2c/2d");
                // lets do nothing
                MESSAGE(LOG_INFO, "INT 29/2c/2d Windows interruption\n");
                GETIP(ip, x7); // priviledged instruction, IP not updated
                STORE_XEMU_CALL(x3);
                MOV32w(x1, u8);
                CALL(const_native_int, -1, x1, 0);
                LOAD_XEMU_CALL();
            } else if (u8 == 0x80) {
                INST_NAME("32bits SYSCALL");
                NOTEST(x1);
                SMEND();
                GETIP(addr, x7);
                STORE_XEMU_CALL(x3);
                CALL_S(const_x86syscall, -1, 0);
                LOAD_XEMU_CALL();
                TABLE64(x3, addr); // expected return address
                BNE_MARK(xRIP, x3);
                LW(x1, xEmu, offsetof(x64emu_t, quit));
                BEQ_NEXT(x1, xZR);
                MARK;
                LOAD_XEMU_REM(x3);
                jump_to_epilog(dyn, 0, xRIP, ninst);
            } else if (u8 == 0x03) {
                INST_NAME("INT 3");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(addr, x7);
                STORE_XEMU_CALL(x3);
                CALL(const_native_int3, -1, 0, 0);
                LOAD_XEMU_CALL();
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            } else {
                INST_NAME("INT n");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7); // priviledged instruction, IP not updated
                STORE_XEMU_CALL(x3);
                CALL(const_native_priv, -1, 0, 0);
                LOAD_XEMU_CALL();
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xCF:
            INST_NAME("IRET");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Not a hack, EFLAGS are restored
            BARRIER(BARRIER_FLOAT);
            iret_to_epilog(dyn, ip, ninst, rex.w);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xD0:
        case 0xD2: // TODO: Jump if CL is 0
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    if (opcode == 0xD0) {
                        INST_NAME("ROL Eb, 1");
                        GETEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROL Eb, CL");
                        GETEB(x1, 0);
                        ANDI(x2, xRCX, 0x1f);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    CALL_(const_rol8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 1:
                    if (opcode == 0xD0) {
                        INST_NAME("ROR Eb, 1");
                        GETEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("ROR Eb, CL");
                        GETEB(x1, 0);
                        ANDI(x2, xRCX, 0x1f);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    CALL_(const_ror8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 2:
                    if (opcode == 0xD0) {
                        INST_NAME("RCL Eb, 1");
                        GETEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("RCL Eb, CL");
                        GETEB(x1, 0);
                        ANDI(x2, xRCX, 0x1f);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    CALL_(const_rcl8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 3:
                    if (opcode == 0xD0) {
                        INST_NAME("RCR Eb, 1");
                        GETEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("RCR Eb, CL");
                        GETEB(x1, 0);
                        ANDI(x2, xRCX, 0x1f);
                    }
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    CALL_(const_rcr8, ed, x3, x1, x2);
                    EBBACK(x5, 0);
                    break;
                case 4:
                case 6:
                    if (opcode == 0xD0) {
                        INST_NAME("SHL Eb, 1");
                        GETEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHL Eb, CL");
                        GETEB(x1, 0);
                        ANDI(x2, xRCX, 0x1f);
                        BEQ_NEXT(x2, xZR);
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    emit_shl8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK(x5, 0);
                    break;
                case 5:
                    if (opcode == 0xD0) {
                        INST_NAME("SHR Eb, 1");
                        GETEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SHR Eb, CL");
                        GETEB(x1, 0);
                        ANDI(x2, xRCX, 0x1F);
                        BEQ_NEXT(x2, xZR);
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    emit_shr8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK(x5, 0);
                    break;
                case 7:
                    if (opcode == 0xD0) {
                        INST_NAME("SAR Eb, 1");
                        GETSEB(x1, 0);
                        MOV32w(x2, 1);
                    } else {
                        INST_NAME("SAR Eb, CL");
                        GETSEB(x1, 0);
                        ANDI(x2, xRCX, 0x1f);
                        BEQ_NEXT(x2, xZR);
                    }
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    emit_sar8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK(x5, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETED(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETED(0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    MOV32w(x2, 1);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w ? (const_rcl64) : (const_rcl32), ed, x4, x1, x2);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    MOV32w(x2, 1);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w ? (const_rcr64) : (const_rcr32), ed, x4, x1, x2);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETED(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETED(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETED(0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETED(0);
                    ANDI(x6, xRCX, rex.w ? 0x3f : 0x1f);
                    emit_rol32(dyn, ninst, rex, ed, x6, x3, x4);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETED(0);
                    ANDI(x6, xRCX, rex.w ? 0x3f : 0x1f);
                    emit_ror32(dyn, ninst, rex, ed, x6, x3, x4);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    MESSAGE("LOG_DUMP", "Need optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    ANDI(x2, xRCX, rex.w ? 0x3f : 0x1f);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w ? (const_rcl64) : (const_rcl32), ed, x4, x1, x2);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    ANDI(x2, xRCX, rex.w ? 0x3f : 0x1f);
                    GETEDW(x4, x1, 0);
                    CALL_(rex.w ? (const_rcr64) : (const_rcr32), ed, x4, x1, x2);
                    WBACK;
                    if (!wback && !rex.w) ZEROUP(ed);
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (!dyn->insts[ninst].x64.gen_flags) {
                        GETED(0);
                        if (rex.w)
                            SLL(ed, ed, xRCX);
                        else
                            SLLW(ed, ed, xRCX);
                        if (dyn->insts[ninst].nat_flags_fusion) {
                            if (!rex.w) ZEROUP(ed);
                            NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else if (!rex.w && MODREG) {
                            ZEROUP(ed);
                        }
                        WBACK;
                        break;
                    }
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && MODREG) ZEROUP(ed);
                    CBZ_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if (!dyn->insts[ninst].x64.gen_flags) {
                        GETED(0);
                        if (rex.w)
                            SRL(ed, ed, xRCX);
                        else
                            SRLW(ed, ed, xRCX);
                        if (dyn->insts[ninst].nat_flags_fusion) {
                            if (!rex.w) ZEROUP(ed);
                            NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else if (!rex.w && MODREG) {
                            ZEROUP(ed);
                        }
                        WBACK;
                        break;
                    }
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && MODREG) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETED(0);
                    if (!rex.w && MODREG) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    UFLAG_OP12(ed, x3);
                    SRAxw(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, rex.w ? d_sar64 : d_sar32);
                    break;
            }
            break;

        case 0xD7:
            INST_NAME("XLAT");
            ANDI(x1, xRAX, 0xff);
            ADD(x1, xRBX, x1);
            LBU(x1, x1, 0);
            ANDI(xRAX, xRAX, ~0xff);
            OR(xRAX, xRAX, x1);
            break;
        case 0xD8:
            addr = dynarec64_D8(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xD9:
            addr = dynarec64_D9(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDA:
            addr = dynarec64_DA(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDB:
            addr = dynarec64_DB(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDC:
            addr = dynarec64_DC(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDD:
            addr = dynarec64_DD(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;

        case 0xDE:
            addr = dynarec64_DE(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xDF:
            addr = dynarec64_DF(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
#define GO(Z)                                                                               \
    BARRIER(BARRIER_MAYBE);                                                                 \
    JUMP(addr + i8, 1);                                                                     \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                           \
        /* out of the block */                                                              \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                \
        if (Z) {                                                                            \
            BNE(xRCX, xZR, i32);                                                            \
        } else {                                                                            \
            BEQ(xRCX, xZR, i32);                                                            \
        };                                                                                  \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                        \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                           \
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                  \
            jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                           \
        } else {                                                                            \
            CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                               \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size); \
            B(i32);                                                                         \
        }                                                                                   \
    } else {                                                                                \
        /* inside the block */                                                              \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);     \
        if (Z) {                                                                            \
            BEQ(xRCX, xZR, i32);                                                            \
        } else {                                                                            \
            BNE(xRCX, xZR, i32);                                                            \
        };                                                                                  \
    }
        case 0xE0:
            INST_NAME("LOOPNZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ANDI(x1, xFlags, 1 << F_ZF);
            CBNZ_NEXT(x1);
            GO(0);
            break;
        case 0xE1:
            INST_NAME("LOOPZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            ANDI(x1, xFlags, 1 << F_ZF);
            CBZ_NEXT(x1);
            GO(0);
            break;
        case 0xE2:
            INST_NAME("LOOP");
            i8 = F8S;
            SUBI(xRCX, xRCX, 1);
            GO(0);
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            GO(1);
            break;
#undef GO

        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if (addr + i32 == 0) {
#if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr - 1));
#endif
            }
#if STEP < 2
            if (!rex.is32bits && !dyn->need_reloc && IsNativeCall(addr + i32, rex.is32bits, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else
                tmp = dyn->insts[ninst].pass2choice = i32 ? 0 : 1;
#else
            tmp = dyn->insts[ninst].pass2choice;
#endif
            switch (tmp) {
                case 3:
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags to "dont'care" state
                    SKIPTEST(x1);
                    BARRIER(BARRIER_FULL);
                    if (dyn->last_ip && (addr - dyn->last_ip < 0x800)) {
                        ADDI(x2, xRIP, addr - dyn->last_ip);
                    } else if (dyn->last_ip && (dyn->last_ip - addr < 0x800)) {
                        SUBI(x2, xRIP, dyn->last_ip - addr);
                    } else {
                        if (dyn->need_reloc) {
                            TABLE64(x2, addr);
                        } else {
                            MOV64x(x2, addr);
                        }
                    }
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall - 1)), dyn->insts[ninst].retn);
                    // calling a native function
                    sse_purge07cache(dyn, ninst, x3);
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && dyn->insts[ninst].natcall) {
                        // Partially support isSimpleWrapper
                        tmp = isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2));
                    } else
                        tmp = 0;
                    if (tmp < 0 || (tmp & 15) > 1)
                        tmp = 0; // TODO: removed when FP is in place
                    if (dyn->insts[ninst].natcall && isRetX87Wrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && dyn->insts[ninst].natcall && tmp) {
                        call_n(dyn, ninst, (void*)(dyn->insts[ninst].natcall + 2 + 8), tmp);
                        POP1(xRIP); // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall, x7); // read the 0xCC already
                        STORE_XEMU_CALL(x3);
                        ADDI(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(const_int3, -1, x1);
                        LOAD_XEMU_CALL();
                        MOV64x(x3, dyn->insts[ninst].natcall);
                        ADDI(x3, x3, 2 + 8 + 8);
                        BNE_MARK(xRIP, x3); // Not the expected address, exit dynarec block
                        POP1(xRIP);         // pop the return address
                        if (dyn->insts[ninst].retn) {
                            if (dyn->insts[ninst].retn < 0x800) {
                                ADDI(xRSP, xRSP, dyn->insts[ninst].retn);
                            } else {
                                MOV64x(x3, dyn->insts[ninst].retn);
                                ADD(xRSP, xRSP, x3);
                            }
                        }
                        LW(x1, xEmu, offsetof(x64emu_t, quit));
                        CBZ_NEXT(x1);
                        MARK;
                        jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                        dyn->last_ip = addr;
                    }
                    break;
                case 1:
                    // this is call to next step, so just push the return address to the stack
                    if (dyn->need_reloc) {
                        TABLE64(x2, addr);
                    } else {
                        MOV64x(x2, addr);
                    }
                    PUSH1z(x2);
                    break;
                default:
                    if ((BOX64DRENV(dynarec_safeflags) > 1) || (ninst && dyn->insts[ninst - 1].x64.set_flags)) {
                        READFLAGS(X_PEND); // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    if (dyn->need_reloc) {
                        TABLE64(x2, addr);
                    } else {
                        MOV64x(x2, addr);
                    }
                    fpu_purgecache(dyn, ninst, 1, x1, x3, x4);
                    PUSH1z(x2);
                    if (BOX64DRENV(dynarec_callret)) {
                        SET_HASCALLRET();
                        // Push actual return address
                        j64 = (dyn->insts) ? (GETMARK - (dyn->native_size)) : 0;
                        AUIPC(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                        ADDI(x4, x4, j64 & 0xfff);
                        MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        ADDI(xSP, xSP, -16);
                        SD(x4, xSP, 0);
                        SD(x2, xSP, 8);
                    } else {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    if (rex.is32bits)
                        j64 = (uint32_t)(addr + i32);
                    else
                        j64 = addr + i32;
                    jump_to_next(dyn, j64, 0, ninst, rex.is32bits);
                    MARK;
                    if (BOX64DRENV(dynarec_callret) && dyn->vector_sew != VECTOR_SEWNA)
                        vector_vsetvli(dyn, ninst, x3, dyn->vector_sew, VECTOR_LMUL1, 1);
                    if (BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
                        // jumps out of current dynablock...
                        j64 = getJumpTableAddress64(addr);
                        if (dyn->need_reloc) {
                            AddRelocTable64JmpTbl(dyn, ninst, addr, STEP);
                            TABLE64_(x4, j64);
                        } else {
                            MOV64x(x4, j64);
                        }
                        LD(x4, x4, 0);
                        BR(x4);
                    }
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if (opcode == 0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            if (rex.is32bits)
                j64 = (uint32_t)(addr + i32);
            else
                j64 = addr + i32;
            JUMP((uintptr_t)getAlternate((void*)j64), 0);
            if (dyn->insts[ninst].x64.jmp_insts == -1) {
                // out of the block
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);
                jump_to_next(dyn, (uintptr_t)getAlternate((void*)j64), 0, ninst, rex.is32bits);
            } else {
                // inside the block
                CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);
                MESSAGE(1, "Jump to %d / 0x%x\n", tmp, tmp);
                if (tmp == 4) {
                    NOP();
                } else {
                    B(tmp);
                }
            }
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xEC ... 0xEF:
            if (opcode == 0xEC)
                INST_NAME("IN AL, DX");
            else if (opcode == 0xED)
                INST_NAME("IN EAX, DX");
            else if (opcode == 0xEE)
                INST_NAME("OUT DX, AL");
            else
                INST_NAME("OUT DX, EAX");
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            GETIP(ip, x7);
            STORE_XEMU_CALL(xRIP);
            CALL(const_native_priv, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xF0:
            addr = dynarec64_F0(dyn, addr, ip, ninst, rex, rep, ok, need_epilog);
            break;
        case 0xF4:
            INST_NAME("HLT");
            if (box64_unittest_mode) { // HLT in unittest mode is an exit
                READFLAGS(X_ALL);
                BARRIER(BARRIER_FLOAT);
                MOV32w(x1, 1);
                SW(x1, xEmu, offsetof(x64emu_t, quit));
            } else {
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                STORE_XEMU_CALL();
                CALL(const_native_priv, -1, 0, 0);
                LOAD_XEMU_CALL();
            }
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xF5:
            INST_NAME("CMC");
            READFLAGS(X_CF);
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            XORI(xFlags, xFlags, 1 << F_CF);
            break;
        case 0xF6:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    INST_NAME("TEST Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    if (MODREG && (rex.rex || (((nextop & 7) >> 2) == 0))) {
                        // quick path for low 8bit registers
                        if (rex.rex)
                            ed = TO_NAT((nextop & 7) + (rex.b << 3));
                        else
                            ed = TO_NAT(nextop & 3);
                    } else {
                        GETEB(x1, 1);
                        ed = x1;
                    }
                    u8 = F8;
                    emit_test8c(dyn, ninst, ed, u8, x6, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Eb");
                    GETEB(x1, 0);
                    NOT(x1, x1);
                    EBBACK(x5, 1);
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ANDI(x2, xRAX, 0xff);
                    MULW(x1, x2, x1);
                    UFLAG_RES(x1);
                    LUI(x2, 0xffff0);
                    AND(xRAX, xRAX, x2);
                    ZEXTH(x1, x1);
                    OR(xRAX, xRAX, x1);
                    UFLAG_DF(x1, d_mul8);
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_PENDING, NAT_FLAGS_NOFUSION);
                    GETSEB(x1, 0);
                    SLLI(x2, xRAX, 56);
                    SRAI(x2, x2, 56);
                    MULW(x1, x2, x1);
                    UFLAG_RES(x1);
                    LUI(x2, 0xffff0);
                    AND(xRAX, xRAX, x2);
                    ZEXTH(x1, x1);
                    OR(xRAX, xRAX, x1);
                    UFLAG_DF(x1, d_imul8);
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ZEXTH(x2, xRAX);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(ed, xZR);
                        GETIP_(ip, x7);
                        STORE_XEMU_CALL(x3);
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    DIVUW(x3, x2, ed);
                    REMUW(x4, x2, ed);
                    LUI(x5, 0xffff0);
                    AND(xRAX, xRAX, x5);
                    ANDI(x3, x3, 0xff);
                    OR(xRAX, xRAX, x3);
                    ANDI(x4, x4, 0xff);
                    SLLI(x4, x4, 8);
                    OR(xRAX, xRAX, x4);
                    SET_DFNONE();
                    CLEAR_FLAGS();
                    IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
                    IFX (X_PF) ORI(xFlags, xFlags, 1 << F_PF);
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SKIPTEST(x1);
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    CALL(const_idiv8, -1, x1, 0);
                    break;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED(4);
                    i64 = F32S;
                    emit_test32c(dyn, ninst, rex, ed, i64, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED(0);
                    XORI(ed, ed, -1);
                    if (!rex.w && MODREG)
                        ZEROUP(ed);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETED(0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    CLEAR_FLAGS();
                    SET_DFNONE();
                    GETED(0);
                    if (rex.w) {
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULHU(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if (gd != xRDX) { MV(xRDX, gd); }
                    } else {
                        ZEXTW2(x3, xRAX);
                        if (MODREG) {
                            ZEXTW2(x4, ed);
                            ed = x4;
                        }
                        MUL(xRDX, x3, ed); // 64 <- 32x32
                        ZEXTW2(xRAX, xRDX);
                        SRLI(xRDX, xRDX, 32);
                    }
                    IFX (X_CF | X_OF) {
                        // CF = OF = RDX != 0
                        SNEZ(x6, xRDX);
                        IFX (X_CF) {
                            OR(xFlags, xFlags, x6); // F_CF == 0
                        }
                        IFX (X_OF) {
                            SLLI(x6, x6, F_OF2);
                            OR(xFlags, xFlags, x6);
                        }
                    }
                    IFX (X_SF) {
                        SRLI(x6, xRAX, rex.w ? 63 : 31);
                        SLLI(x6, x6, F_SF);
                        OR(xFlags, xFlags, x6);
                    }
                    if (X_PF) emit_pf(dyn, ninst, xRAX, x6, x4);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    CLEAR_FLAGS();
                    SET_DFNONE();
                    GETSED(0);
                    if (rex.w) {
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULH(gd, xRAX, ed);
                        MUL(xRAX, xRAX, ed);
                        if (gd != xRDX) { MV(xRDX, gd); }
                    } else {
                        ADDIW(x3, xRAX, 0); // sign extend 32bits-> 64bits
                        MUL(xRDX, x3, ed);  // 64 <- 32x32
                        ZEXTW2(xRAX, xRDX);
                        SRLI(xRDX, xRDX, 32);
                    }
                    IFX (X_CF | X_OF) {
                        // CF = OF = SignExtend(RAX) != RDX:RAX
                        SRAIxw(x6, xRAX, rex.w ? 63 : 31);
                        SUBxw(x6, xRDX, x6);
                        SNEZ(x6, x6);
                        IFX (X_CF) {
                            OR(xFlags, xFlags, x6); // F_CF == 0
                        }
                        IFX (X_OF) {
                            SLLI(x6, x6, F_OF2);
                            OR(xFlags, xFlags, x6);
                        }
                    }
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    if (!rex.w) {
                        GETED(0);
                        if (ninst && (nextop == 0xF0)
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0xB8
                            && *(uint32_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0) {
                            // hack for some protection that check a divide by zero actually trigger a divide by zero exception
                            MESSAGE(LOG_INFO, "Divide by 0 hack\n");
                            GETIP(ip, x7);
                            STORE_XEMU_CALL(x3);
                            CALL(const_native_div0, -1, 0, 0);
                            LOAD_XEMU_CALL();
                        } else {
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL(x3);
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            SLLI(x3, xRDX, 32);
                            ZEXTW2(x2, xRAX);
                            OR(x3, x3, x2);
                            if (MODREG) {
                                ZEXTW2(x4, ed);
                                ed = x4;
                            }
                            DIVU(x2, x3, ed);
                            REMU(xRDX, x3, ed);
                            ZEXTW2(xRAX, x2);
                            ZEROUP(xRDX);
                        }
                    } else {
                        if (ninst
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0x31
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0xD2) {
                            GETED(0);
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL(x3);
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            DIVU(x2, xRAX, ed);
                            REMU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x4, x1, 0); // get edd changed addr, so cannot be called 2 times for same op...
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL(x3);
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            BEQ_MARK(xRDX, xZR);
                            CALL(const_div64, -1, ed, 0);
                            B_NEXT_nocond;
                            MARK;
                            DIVU(x2, xRAX, ed);
                            REMU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        }
                    }
                    SET_DFNONE();
                    CLEAR_FLAGS();
                    IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
                    IFX (X_PF) ORI(xFlags, xFlags, 1 << F_PF);
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SKIPTEST(x1);
                    SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    if (!rex.w) {
                        GETSED(0);
                        if (BOX64ENV(dynarec_div0)) {
                            BNE_MARK3(ed, xZR);
                            GETIP_(ip, x7);
                            STORE_XEMU_CALL(x3);
                            CALL(const_native_div0, -1, 0, 0);
                            CLEARIP();
                            LOAD_XEMU_CALL();
                            jump_to_epilog(dyn, 0, xRIP, ninst);
                            MARK3;
                        }
                        SLLI(x3, xRDX, 32);
                        ZEXTW2(x2, xRAX);
                        OR(x3, x3, x2);
                        DIV(x2, x3, ed);
                        REM(xRDX, x3, ed);
                        ZEXTW2(xRAX, x2);
                        ZEROUP(xRDX);
                    } else {
                        if (ninst && dyn->insts
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0x48
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0x99) {
                            GETED(0);
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL(x3);
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            DIV(x2, xRAX, ed);
                            REM(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x4, x1, 0); // get edd changed addr, so cannot be called 2 times for same op...
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL(x3);
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            // Need to see if RDX==0 and RAX not signed
                            //  or RDX==-1 and RAX signed
                            BNE_MARK2(xRDX, xZR);
                            BGE_MARK(xRAX, xZR);
                            MARK2;
                            NOT(x2, xRDX);
                            BNE_MARK3(x2, xZR);
                            BLT_MARK(xRAX, xZR);
                            MARK3;
                            CALL(const_idiv64, -1, ed, 0);
                            B_NEXT_nocond;
                            MARK;
                            DIV(x2, xRAX, ed);
                            REM(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            };
            break;

        case 0xF8:
            INST_NAME("CLC");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            ANDI(xFlags, xFlags, ~(1 << F_CF));
            break;
        case 0xF9:
            INST_NAME("STC");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            ORI(xFlags, xFlags, 1 << F_CF);
            break;
        case 0xFC:
            INST_NAME("CLD");
            ANDI(xFlags, xFlags, ~(1 << F_DF));
            break;
        case 0xFD:
            INST_NAME("STD");
            ORI(xFlags, xFlags, 1 << F_DF);
            break;
        case 0xFE:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 0);
                    emit_inc8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 0);
                    emit_dec8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK(x5, 0);
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETED(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET_PENDING, NAT_FLAGS_FUSION);
                    GETED(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF ((BOX64DRENV(dynarec_safeflags) > 1) || ((ninst && dyn->insts[ninst - 1].x64.set_flags) || ((ninst > 1) && dyn->insts[ninst - 2].x64.set_flags)), 1) {
                        READFLAGS(X_PEND); // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to put flag in "don't care" state
                    }
                    GETEDz(0);
                    if (BOX64DRENV(dynarec_callret) && BOX64DRENV(dynarec_bigblock) > 1) {
                        BARRIER(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    GETIP_(addr, x7);
                    if (BOX64DRENV(dynarec_callret)) {
                        SET_HASCALLRET();
                        j64 = (dyn->insts) ? (GETMARK - (dyn->native_size)) : 0;
                        AUIPC(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                        ADDI(x4, x4, j64 & 0xfff);
                        MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        ADDI(xSP, xSP, -16);
                        SD(x4, xSP, 0);
                        SD(xRIP, xSP, 8);
                    }
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    MARK;
                    if (BOX64DRENV(dynarec_callret) && dyn->vector_sew != VECTOR_SEWNA)
                        vector_vsetvli(dyn, ninst, x3, dyn->vector_sew, VECTOR_LMUL1, 1);
                    if (BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
                        // jumps out of current dynablock...
                        j64 = getJumpTableAddress64(addr);
                        if (dyn->need_reloc) AddRelocTable64RetEndBlock(dyn, ninst, addr, STEP);
                        TABLE64_(x4, j64);
                        LD(x4, x4, 0);
                        BR(x4);
                    }
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    READFLAGS(X_PEND);
                    BARRIER(BARRIER_FLOAT);
                    GETEDz(0);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 5: // JMP FAR Ed
                    if (MODREG) {
                        DEFAULT;
                    } else {
                        INST_NAME("JMP FAR Ed");
                        READFLAGS(X_PEND);
                        BARRIER(BARRIER_FLOAT);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                        LDxw(x1, wback, 0);
                        ed = x1;
                        LHU(x3, wback, rex.w ? 8 : 4);
                        SH(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
                        jump_to_epilog(dyn, 0, ed, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETEDz(0);
                    PUSH1z(ed);
                    break;

                default:
                    DEFAULT;
            }
            break;
        default:
            DEFAULT;
    }

    return addr;
}
