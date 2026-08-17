#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "la64_mapping.h"
#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "la64_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "../dynablock_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "la64_printer.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "../dynarec_helper.h"
#include "dynarec_la64_native.h"

int isSimpleWrapper(wrapper_t fun);
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00_3(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed, tmp1, tmp2, tmp3;
    int8_t i8;
    int32_t i32, tmp;
    int64_t i64, j64;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint8_t wback, wb1, wb2, wb;
    int64_t fixedaddress;
    uint8_t v0, v1, v2;
    int unscaled;
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
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        if (MODREG && u8 > 1) {
                            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        } else {
                            SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        }
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_rol8c(dyn, ninst, ed, u8, x4, x5, x6);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        if (MODREG && u8 > 1) {
                            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        } else {
                            SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        }
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_ror8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_rcl8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEB(x1, 1);
                        u8 = F8 & 0x1f;
                        emit_rcr8c(dyn, ninst, x1, u8, x4, x5);
                        EBBACK();
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & 0x1f;
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_shl8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK();
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
                        if (MODREG && !dyn->insts[ninst].x64.gen_flags && !dyn->insts[ninst].nat_flags_fusion) {
                            u8 = (F8) & 0x1f;
                            if (rex.rex) {
                                wback = TO_NAT((nextop & 7) + (rex.b << 3));
                                wb2 = 0;
                            } else {
                                wb2 = ((nextop & 7) >> 2) * 8;
                                wback = TO_NAT((nextop & 7) & 3);
                            }
                            if (u8 < 8) {
                                BSTRPICK_D(x1, wback, wb2 + 7, wb2 + u8);
                                BSTRINS_D(wback, x1, wb2 + 7, wb2);
                            } else {
                                BSTRINS_D(wback, xZR, wb2 + 7, wb2);
                            }
                            break;
                        }
                        GETEB(x1, 1);
                        u8 = (F8) & 0x1f;
                        emit_shr8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK();
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
                        emit_sar8c(dyn, ninst, ed, u8, x4, x5);
                        EBBACK();
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
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        if (MODREG && u8 > 1) {
                            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        } else {
                            SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        }
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_rol32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        if (MODREG && u8 > 1) {
                            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        } else {
                            SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        }
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_ror32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_CF | X_OF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_rcl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        READFLAGS(X_CF);
                        SETFLAGS(X_CF | X_OF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_rcr32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_shl32c(dyn, ninst, rex, ed, u8, x3, x4, x5);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_shr32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        } else {
                            FAKEED;
                        }
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    u8 = geted_ib(dyn, addr, ninst, nextop) & (0x1f + (rex.w * 0x20));
                    if (u8) {
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                        GETEDsd(1);
                        u8 = (F8) & (rex.w ? 0x3f : 0x1f);
                        emit_sar32c(dyn, ninst, rex, ed, u8, x3, x4);
                        WBACK;
                    } else {
                        if (MODREG && !rex.w && !rex.is32bits) {
                            GETED(1);
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
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
            if (BOX64DRENV(dynarec_safeflags)) {
                READFLAGS(X_PEND); // lets play safe here too
            }
            BARRIER(BARRIER_FLOAT);
            u16 = F16;
            if (!rex.is32bits) UP32_READ(xRSP);
            POP1z(xRIP);
            if (u16 < 2048)
                ADDIz(xRSP, xRSP, u16);
            else {
                MOV32w(x1, u16);
                ADDz(xRSP, xRSP, x1);
            }
            ret_to_next(dyn, ip, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            if (BOX64DRENV(dynarec_safeflags)) {
                READFLAGS(X_PEND); // so instead, force the deferred flags, so it's not too slow, and flags are not lost
            }
            BARRIER(BARRIER_FLOAT);
            if (!rex.is32bits) UP32_READ(xRSP);
            POP1z(xRIP);
            ret_to_next(dyn, ip, ninst, rex);
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
                    eb1 = TO_NAT((ed & 3)); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;    // L or H
                } else {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                }
                MOV32w(x3, u8);
                BSTRINS_D(eb1, x3, eb2 * 8 + 7, eb2 * 8);
            } else { // mem <= u8
                SCRATCH_USAGE(0);
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, 1, 1);
                u8 = F8;
                if (u8) {
                    SCRATCH_USAGE(1);
                    ADDI_D(x3, xZR, u8);
                    ed = x3;
                } else
                    ed = xZR;
                ST_B(ed, wback, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop = F8;
            SCRATCH_USAGE(0);
            if (MODREG) { // reg <= i32
                i64 = F32S;
                GETEDsd(0);
                MOV64xw(ed, i64);
            } else { // mem <= i32
                IF_UNALIGNED(ip) {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, &lock, (1 << (2 + rex.w)) - 1, 4);
                    i64 = F32S;
                    if (i64) {
                        SCRATCH_USAGE(1);
                        MOV64x(x4, i64);
                        ed = x4;
                    } else
                        ed = xZR;
                    for (int i = 0; i < (1 << (2 + rex.w)); i++) {
                        if (i == 0 || ed == xZR) {
                            ST_B(ed, wback, fixedaddress + i);
                        } else {
                            SRLI_D(x3, ed, i * 8);
                            ST_B(x3, wback, fixedaddress + i);
                        }
                    }
                } else {
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
        case 0xC8:
            INST_NAME("ENTER Iw,Ib");
            u16 = F16;
            u8 = (F8) & 0x1f;
            if (!rex.is32bits) {
                UP32_READ(xRSP);
                UP32_READ(xRBP);
            }
            if (u8) {
                MV(x1, xRBP);
            }
            PUSH1z(xRBP);
            MV(xRBP, xRSP);
            if (u8) {
                for (u32 = 1; u32 < u8; u32++) {
                    LDz(x2, x1, rex.is32bits ? -4 : -8);
                    PUSH1z(x2);
                }
                PUSH1z(xRBP);
            }
            if (u16 < 2047) {
                ADDI_D(xRSP, xRSP, -u16);
            } else {
                MOV32w(x2, u16);
                SUB_D(xRSP, xRSP, x2);
            }
            break;
        case 0xC9:
            INST_NAME("LEAVE");
            MARKREGsz(xRBP);
            MVz(xRSP, xRBP);
            POP1z(xRBP);
            break;
        case 0xCA:
            INST_NAME("FAR RETN");
            u16 = F16;
            READFLAGS(X_PEND);
            BARRIER(BARRIER_FLOAT);
            if (!rex.is32bits) UP32_READ(xRSP);
            if (rex.w) {
                POP1(xRIP);
                POP1(x3);
            } else {
                POP1_32(xRIP);
                POP1_32(x3);
            }
            ST_H(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
            if (u16 < 2048)
                ADDIz(xRSP, xRSP, u16);
            else {
                MOV32w(x1, u16);
                ADDz(xRSP, xRSP, x1);
            }
            ret_to_next(dyn, ip, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xCB:
            INST_NAME("FAR RET");
            READFLAGS(X_PEND);
            BARRIER(BARRIER_FLOAT);
            if (!rex.is32bits) UP32_READ(xRSP);
            if (rex.w) {
                POP1(xRIP);
                POP1(x3);
            } else {
                POP1_32(xRIP);
                POP1_32(x3);
            }
            ST_H(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
            ret_to_next(dyn, ip, ninst, rex);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xCC:
            SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
            SKIPTEST(x1);
            if (IsBridgeSignature(PK(0), PK(1))) {
                addr += 2;
                BARRIER(BARRIER_FLOAT);
                INST_NAME("Special Box64 instruction");
                if (PK64(0) == 0) {
                    addr += 8;
                    MESSAGE(LOG_DEBUG, "Exit x64 Emu\n");
                    MOV64x(x1, 1);
                    ST_W(x1, xEmu, offsetof(x64emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip), 1));
                    la64_native_call_t inline_native_call = LA64_NATIVE_NONE;
                    if (BOX64ENV(log) < 2 && !BOX64ENV(rolling_log))
                        inline_native_call = la64_get_native_call(ip);
                    if (inline_native_call != LA64_NATIVE_NONE) {
                        if (la64_native_call_writes_memory(inline_native_call))
                            WILLWRITE();
                        la64_emit_native_call(dyn, ninst, inline_native_call);
                        if (la64_native_call_writes_memory(inline_native_call))
                            SMWRITE2();
                        addr += 8 + 8;
                    } else {
                        x87_stackcount(dyn, ninst, x1);
                        x87_forget(dyn, ninst, x3, x4, 0);
                        sse_purge07cache(dyn, ninst, x3);
                        // Partially support isSimpleWrapper
                        tmp = isSimpleWrapper(*(wrapper_t*)(addr));
                        if (isRetX87Wrapper(*(wrapper_t*)(addr)))
                            // return value will be on the stack, so the stack depth needs to be updated
                            x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                        if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && tmp) {
                            call_n(dyn, ninst, (void*)(addr + 8), tmp);
                            SMWRITE2();
                            addr += 8 + 8;
                        } else {
                            GETIP(ip + 1, x7); // read the 0xCC
                            STORE_XEMU_CALL();
                            ADDI_D(x3, xRIP, 8 + 8 + 2);                        // expected return address
                            ADDI_D(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                            CALL_(const_int3, -1, x3, x1, 0);
                            SMWRITE2();
                            LOAD_XEMU_CALL();
                            addr += 8 + 8;
                            BNE_MARK(xRIP, x3);
                            LD_W(x1, xEmu, offsetof(x64emu_t, quit));
                            CBZ_NEXT(x1);
                            MARK;
                            jump_to_epilog_fast(dyn, 0, xRIP, ninst);
                        }
                    }
                }
            } else {
                INST_NAME("INT 3");
                if (!BOX64ENV(ignoreint3)) {
                    // check if TRAP signal is handled
                    TABLE64C(x1, const_context);
                    MOV32w(x2, offsetof(box64context_t, signals[X64_SIGTRAP]));
                    LDX_D(x3, x1, x2);
                    BEQZ_MARK(x3);
                    GETIP(addr, x7);
                    STORE_XEMU_CALL();
                    CALL(const_native_int3, -1, 0, 0);
                    LOAD_XEMU_CALL();
                    MARK;
                    jump_to_epilog(dyn, addr, 0, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                }
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
                STORE_XEMU_CALL();
                MOV32w(x1, u8);
                CALL(const_native_int, -1, x1, 0);
                LOAD_XEMU_CALL();
            } else if (u8 == 0x80) {
                INST_NAME("32bits SYSCALL");
                NOTEST(x1);
                SMEND();
                GETIP(addr, x7);
                STORE_XEMU_CALL();
                CALL_S(const_x86syscall, -1, 0);
                LOAD_XEMU_CALL();
                TABLE64(x3, addr); // expected return address
                BNE_MARK(xRIP, x3);
                LD_W(x1, xEmu, offsetof(x64emu_t, quit));
                CBZ_NEXT(x1);
                MARK;
                LOAD_XEMU_REM();
                jump_to_epilog(dyn, 0, xRIP, ninst);
            } else if (u8 == 0x03) {
                INST_NAME("INT 3");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(addr, x7);
                STORE_XEMU_CALL();
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
                if(rex.is32bits && u8==0x04) {
                    GETIP(addr, x7);
                } else {
                    GETIP(ip, x7); // priviledged instruction, IP not updated
                }
                STORE_XEMU_CALL();
                MOV32w(x1, u8);
                CALL(const_native_int, -1, x1, 0);
                LOAD_XEMU_CALL();
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xCF:
            INST_NAME("IRET");
            SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION); // Not a hack, EFLAGS are restored
            BARRIER(BARRIER_FLOAT);
            iret_to_next(dyn, ip, ninst, rex.is32bits, rex.w);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xD0:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Eb, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rol8c(dyn, ninst, ed, 1, x4, x5, x6);
                    EBBACK();
                    break;
                case 1:
                    INST_NAME("ROR Eb, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_ror8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK();
                    break;
                case 2:
                    INST_NAME("RCL Eb, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rcl8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK();
                    break;
                case 3:
                    INST_NAME("RCR Eb, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    emit_rcr8c(dyn, ninst, ed, 1, x4, x5);
                    EBBACK();
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, 1");
                    GETEB(x1, 0);
                    MOV32w(x2, 1);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    emit_shl8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 5:
                    INST_NAME("SHR Eb, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    if ((MODREG) && !dyn->insts[ninst].x64.gen_flags && !dyn->insts[ninst].nat_flags_fusion) {
                        if (rex.rex) {
                            wback = TO_NAT((nextop & 7) + (rex.b << 3));
                            wb2 = 0;
                        } else {
                            wb2 = ((nextop & 7) >> 2) * 8;
                            wback = TO_NAT((nextop & 7) & 3);
                        }
                        BSTRPICK_D(x1, wback, wb2 + 7, wb2 + 1);
                        BSTRINS_D(wback, x1, wb2 + 7, wb2);
                        break;
                    }
                    GETEB(x1, 0);
                    MOV32w(x2, 1);
                    emit_shr8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 7:
                    INST_NAME("SAR Eb, 1");
                    GETSEB(x1, 0);
                    MOV32w(x2, 1);
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    emit_sar8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEDsd(0);
                    emit_rol32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEDsd(0);
                    emit_ror32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEDsd(0);
                    emit_rcl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_FUSION); // removed PENDING on purpose
                    GETEDsd(0);
                    emit_rcr32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEDsd(0);
                    emit_shl32c(dyn, ninst, rex, ed, 1, x3, x4, x5);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEDsd(0);
                    emit_shr32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION); // some flags are left undefined
                    GETEDsd(0);
                    emit_sar32c(dyn, ninst, rex, ed, 1, x3, x4);
                    WBACK;
                    break;
            }
            break;
        case 0xD2:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    UFLAG_IF {
                        ANDI(x2, xRCX, 0x1f);
                        CBZ_NEXT(x2);
                    }
                    ANDI(x2, xRCX, 7);
                    emit_rol8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                case 1:
                    INST_NAME("ROR Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION); // removed PENDING on purpose
                    GETEB(x1, 0);
                    UFLAG_IF {
                        ANDI(x2, xRCX, 0x1f);
                        CBZ_NEXT(x2);
                    }
                    ANDI(x2, xRCX, 7);
                    emit_ror8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                case 2:
                    INST_NAME("RCL Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    emit_rcl8(dyn, ninst, ed, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 3:
                    INST_NAME("RCR Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    emit_rcr8(dyn, ninst, ed, x2, x4, x5, x6);
                    EBBACK();
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ANDI(x2, xRCX, 0x1F);
                    CBZ_NEXT(x2);
                    emit_shl8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 5:
                    INST_NAME("SHR Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ANDI(x2, xRCX, 0x1F);
                    CBZ_NEXT(x2);
                    emit_shr8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
                case 7:
                    INST_NAME("SAR Eb, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    GETSEB(x1, 0);
                    ANDI(x2, xRCX, 0x1f);
                    CBZ_NEXT(x2);
                    emit_sar8(dyn, ninst, x1, x2, x5, x4, x6);
                    EBBACK();
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION); // removed PENDING on purpose
                    GETEDsd(0);
                    ANDI(x6, xRCX, rex.w ? 0x3f : 0x1f);
                    if (MODREG && NEED_ZEROUP(ed)) ZEROUP(ed);
                    CBZ_NEXT(x6);
                    emit_rol32(dyn, ninst, rex, ed, x6, x3, x4);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION); // removed PENDING on purpose
                    GETEDsd(0);
                    ANDI(x6, xRCX, rex.w ? 0x3f : 0x1f);
                    if (MODREG && NEED_ZEROUP(ed)) ZEROUP(ed);
                    CBZ_NEXT(x6);
                    emit_ror32(dyn, ninst, rex, ed, x6, x3, x4);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    ANDI(x3, xRCX, rex.w ? 63 : 31);
                    GETEDsd(0);
                    if (MODREG && NEED_ZEROUP(ed)) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_rcl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_OF | X_CF);
                    } else {
                        READFLAGS(X_CF);
                    }
                    SETFLAGS(X_OF | X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    if (rex.w) {
                        ANDI(x3, xRCX, 0x3f);
                    } else {
                        ANDI(x3, xRCX, 0x1f);
                    }
                    GETEDsd(0);
                    if (MODREG && NEED_ZEROUP(ed)) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_rcr32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    if (!dyn->insts[ninst].x64.gen_flags) {
                        GETEDsd(0);
                        if (rex.w)
                            SLL_D(ed, ed, xRCX);
                        else
                            SLL_W(ed, ed, xRCX);
                        if (dyn->insts[ninst].nat_flags_fusion) {
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                            NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else if (!rex.w && MODREG) {
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        }
                        WBACK;
                        break;
                    }
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETEDsd(0);
                    if (MODREG && NEED_ZEROUP(ed)) ZEROUP(ed);
                    CBZ_NEXT(x3);
                    emit_shl32(dyn, ninst, rex, ed, x3, x5, x4, x6);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    if (!dyn->insts[ninst].x64.gen_flags) {
                        GETEDsd(0);
                        if (rex.w)
                            SRL_D(ed, ed, xRCX);
                        else
                            SRL_W(ed, ed, xRCX);
                        if (dyn->insts[ninst].nat_flags_fusion) {
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                            NAT_FLAGS_OPS(ed, xZR, x5, xZR);
                        } else if (!rex.w && MODREG) {
                            if (NEED_ZEROUP(ed)) ZEROUP(ed);
                        }
                        WBACK;
                        break;
                    }
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETEDsd(0);
                    if (MODREG && NEED_ZEROUP(ed)) ZEROUP(ed);
                    CBZ_NEXT(x3);
                    emit_shr32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    if (BOX64DRENV(dynarec_safeflags) > 1) {
                        READFLAGS(X_ALL);
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    } else
                        SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_NOFUSION);
                    ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
                    GETEDsd(0);
                    if (MODREG && NEED_ZEROUP(ed)) { ZEROUP(ed); }
                    CBZ_NEXT(x3);
                    emit_sar32(dyn, ninst, rex, ed, x3, x5, x4);
                    WBACK;
                    break;
            }
            break;
        case 0xD4:
            if (rex.is32bits) {
                INST_NAME("AAM Ib");
                SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                BSTRPICK_D(x1, xRAX, 7, 0);
                u8 = F8;
                MOV32w(x2, u8);
                CALL(const_aam16, x1, x1, x2);
                BSTRINS_D(xRAX, x1, 15, 0);
            } else {
                INST_NAME("Illegal D4");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0xD5:
            if (rex.is32bits) {
                INST_NAME("AAD Ib");
                SETFLAGS(X_ALL, SF_SET_DF, NAT_FLAGS_NOFUSION);
                BSTRPICK_D(x1, xRAX, 15, 0);
                u8 = F8;
                MOV32w(x2, u8);
                CALL(const_aad16, x1, x1, x2);
                BSTRINS_D(xRAX, x1, 15, 0);
            } else {
                INST_NAME("Illegal D5");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
            break;
        case 0xD7:
            INST_NAME("XLAT");
            UP32_READ(xRBX);
            BSTRPICK_D(x1, xRAX, 7, 0);
            if (rex.seg) {
                grab_segdata(dyn, addr, ninst, x2, rex.seg);
                ADD_D(x1, x1, x2);
            }
            LDX_BU(x1, xRBX, x1);
            BSTRINS_D(xRAX, x1, 7, 0);
            break;
        case 0xD8:
            addr = dynarec64_D8(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xD9:
            addr = dynarec64_D9(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xDA:
            addr = dynarec64_DA(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xDB:
            addr = dynarec64_DB(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xDC:
            addr = dynarec64_DC(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xDD:
            addr = dynarec64_DD(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;

        case 0xDE:
            addr = dynarec64_DE(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;
        case 0xDF:
            addr = dynarec64_DF(dyn, addr, ip, ninst, rex, ok, need_epilog);
            break;

#define GO(Z, R)                                                                                   \
    JUMP(addr + i8, 1);                                                                            \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                                  \
        /* out of the block */                                                                     \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                       \
        if (Z) {                                                                                   \
            BNE(R, xZR, i32);                                                                      \
        } else {                                                                                   \
            BEQ(R, xZR, i32);                                                                      \
        }                                                                                          \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                               \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                                  \
                fpu_purgecache(dyn, ninst, 1, x1, x2, x3);                                         \
            jump_to_next(dyn, addr + i8, 0, ninst, rex.is32bits);                                  \
        } else {                                                                                   \
            CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);                                      \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].branch_address - (dyn->native_size); \
            B(i32);                                                                                \
        }                                                                                          \
    } else {                                                                                       \
        /* inside the block */                                                                     \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].branch_address - (dyn->native_size);     \
        if (Z) {                                                                                   \
            BEQ(R, xZR, i32);                                                                      \
        } else {                                                                                   \
            BNE(R, xZR, i32);                                                                      \
        };                                                                                         \
    }

        case 0xE0:
            INST_NAME("LOOPNZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            if (rex.is32bits && rex.is67) {
                BSTRPICK_D(x3, xRCX, 15, 0);
                ADDI_W(x3, x3, -1);
                BSTRINS_D(xRCX, x3, 15, 0);
                if (cpuext.lbt)
                    X64_GET_EFLAGS(x1, X_ZF);
                else
                    ANDI(x1, xFlags, 1 << F_ZF);
                CBNZ_NEXT(x1);
                GO(0, x3);
            } else {
                UP32_READ(xRCX);
                ADDIy(xRCX, xRCX, -1);
                if (cpuext.lbt)
                    X64_GET_EFLAGS(x1, X_ZF);
                else
                    ANDI(x1, xFlags, 1 << F_ZF);
                CBNZ_NEXT(x1);
                GO(0, xRCX);
            }
            break;
        case 0xE1:
            INST_NAME("LOOPZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            if (rex.is32bits && rex.is67) {
                BSTRPICK_D(x3, xRCX, 15, 0);
                ADDI_W(x3, x3, -1);
                BSTRINS_D(xRCX, x3, 15, 0);
                if (cpuext.lbt)
                    X64_GET_EFLAGS(x1, X_ZF);
                else
                    ANDI(x1, xFlags, 1 << F_ZF);
                CBZ_NEXT(x1);
                GO(0, x3);
            } else {
                UP32_READ(xRCX);
                ADDIy(xRCX, xRCX, -1);
                if (cpuext.lbt)
                    X64_GET_EFLAGS(x1, X_ZF);
                else
                    ANDI(x1, xFlags, 1 << F_ZF);
                CBZ_NEXT(x1);
                GO(0, xRCX);
            }
            break;
        case 0xE2:
            INST_NAME("LOOP");
            i8 = F8S;
            if (rex.is32bits && rex.is67) {
                BSTRPICK_D(x3, xRCX, 15, 0);
                ADDI_W(x3, x3, -1);
                BSTRINS_D(xRCX, x3, 15, 0);
                GO(0, x3);
            } else {
                UP32_READ(xRCX);
                ADDIy(xRCX, xRCX, -1);
                GO(0, xRCX);
            }
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            if (rex.is32bits && rex.is67) {
                BSTRPICK_D(x3, xRCX, 15, 0);
                GO(1, x3);
            } else if (rex.is32bits || rex.is67) {
                ADDIy(x3, xRCX, 0);
                GO(1, x3);
            } else {
                UP32_READ(xRCX);
                GO(1, xRCX);
            }
            break;
#undef GO
        case 0xE4: /* IN AL, Ib */
        case 0xE5: /* IN EAX, Ib */
        case 0xE6: /* OUT Ib, AL */
        case 0xE7: /* OUT Ib, EAX */
            INST_NAME(opcode == 0xE4 ? "IN AL, Ib" : (opcode == 0xE5 ? "IN EAX, Ib" : (opcode == 0xE6 ? "OUT Ib, AL" : "OUT Ib, EAX")));
            if (rex.is32bits && BOX64ENV(ignoreint3)) {
                F8;
            } else {
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                u8 = F8;
                GETIP(ip, x7);
                STORE_XEMU_CALL();
                CALL(const_native_priv, -1, 0, 0);
                LOAD_XEMU_CALL();
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xE8:
            INST_NAME("CALL Id");
            i32 = (rex.is32bits && rex.is66) ? F16S : F32S;
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
                    la64_native_call_t inline_native_call = LA64_NATIVE_NONE;
                    if (BOX64ENV(log) < 2 && !BOX64ENV(rolling_log) && dyn->insts[ninst].natcall)
                        inline_native_call = la64_get_native_call(dyn->insts[ninst].natcall - 1);
                    BARRIER(inline_native_call != LA64_NATIVE_NONE ? BARRIER_FLOAT : BARRIER_FULL);
                    if (dyn->last_ip && ((addr - dyn->last_ip < 0x800) || (dyn->last_ip - addr < 0x800))) {
                        ADDI_D(x2, xRIP, addr - dyn->last_ip);
                    } else {
                        if (dyn->need_reloc) {
                            TABLE64(x2, addr);
                        } else {
                            MOV64x(x2, addr);
                        }
                    }
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall - 1), 1), dyn->insts[ninst].retn);
                    if (inline_native_call != LA64_NATIVE_NONE) {
                        if (la64_native_call_writes_memory(inline_native_call))
                            WILLWRITE();
                        la64_emit_native_call(dyn, ninst, inline_native_call);
                        if (la64_native_call_writes_memory(inline_native_call))
                            SMWRITE2();
                        POP1(xRIP);
                        dyn->last_ip = addr;
                        break;
                    }
                    // calling a native function
                    sse_purge07cache(dyn, ninst, x3);
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && dyn->insts[ninst].natcall) {
                        // Partially support isSimpleWrapper
                        tmp = isSimpleWrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2));
                    } else
                        tmp = 0;
                    if (dyn->insts[ninst].natcall && isRetX87Wrapper(*(wrapper_t*)(dyn->insts[ninst].natcall + 2)))
                        // return value will be on the stack, so the stack depth needs to be updated
                        x87_purgecache(dyn, ninst, 0, x3, x1, x4);
                    if ((BOX64ENV(log) < 2 && !BOX64ENV(rolling_log)) && dyn->insts[ninst].natcall && tmp) {
                        call_n(dyn, ninst, (void*)(dyn->insts[ninst].natcall + 2 + 8), tmp);
                        SMWRITE2();
                        POP1(xRIP); // pop the return address
                        dyn->last_ip = addr;
                    } else {
                        GETIP_(dyn->insts[ninst].natcall, x7); // read the 0xCC already
                        STORE_XEMU_CALL();
                        ADDI_D(x1, xEmu, (uint32_t)offsetof(x64emu_t, ip)); // setup addr as &emu->ip
                        CALL_S(const_int3, -1, x1);
                        SMWRITE2();
                        LOAD_XEMU_CALL();
                        MOV64x(x3, dyn->insts[ninst].natcall);
                        ADDI_D(x3, x3, 2 + 8 + 8);
                        BNE_MARK(xRIP, x3); // Not the expected address, exit dynarec block
                        POP1(xRIP);         // pop the return address
                        if (dyn->insts[ninst].retn) {
                            if (dyn->insts[ninst].retn < 0x800) {
                                ADDI_D(xRSP, xRSP, dyn->insts[ninst].retn);
                            } else {
                                MOV64x(x3, dyn->insts[ninst].retn);
                                ADD_D(xRSP, xRSP, x3);
                            }
                        }
                        LD_W(x1, xEmu, offsetof(x64emu_t, quit));
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
                    BARRIER(BARRIER_FLOAT);
                    PUSH1z(x2);
                    int can_continue = (addr < (dyn->start + dyn->isize));
                    if (BOX64DRENV(dynarec_callret)) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if (can_continue && BOX64DRENV(dynarec_callret) > 1 && !dyn->always_test)
                            j64 = CALLRET_GETRET();
                        else
                            j64 = (dyn->insts) ? (GETMARK - (dyn->native_size)) : 0;
                        PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                        ADDI_D(x4, x4, j64 & 0xfff);
                        MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        ADDI_D(xSP, xSP, -16);
                        ST_D(x4, xSP, 0);
                        ST_D(x2, xSP, 8);
                    } else {
                        *ok = 0;
                        *need_epilog = 0;
                    }
                    if (rex.is32bits)
                        j64 = (uint32_t)(addr + i32);
                    else
                        j64 = addr + i32;
                    j64 = (uintptr_t)getAlternate((void*)j64);
                    jump_to_next(dyn, j64, 0, ninst, rex.is32bits);
                    CALLRET_RET(can_continue);
                    MARK;
                    if (BOX64DRENV(dynarec_callret) && !can_continue) {
                        // jumps out of current dynablock...
                        j64 = getJumpTableAddress64(addr);
                        if (dyn->need_reloc) {
                            AddRelocTable64JmpTbl(dyn, ninst, addr, STEP);
                            TABLE64_(x4, j64);
                        } else {
                            MOV64x(x4, j64);
                        }
                        LD_D(x4, x4, 0);
                        BR(x4);
                    }
                    CLEARIP();
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(BARRIER_MAYBE);
            if(opcode==0xEB && PK(0)==0xFF) {
                INST_NAME("JMP ib");
                MESSAGE(LOG_DEBUG, "Hack for EB FF opcode");
                NOP();
            } else {
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
                j64 = (uintptr_t)getAlternate((void*)j64);
                JUMP(j64, 0);
                if (dyn->insts[ninst].x64.jmp_insts == -1) {
                    // out of the block
                    BARRIER(BARRIER_FLOAT);
                    jump_to_next(dyn, (uintptr_t)getAlternate((void*)j64), 0, ninst, rex.is32bits);
                } else {
                    // inside the block
                    CacheTransform(dyn, ninst, CHECK_CACHE(), x1, x2, x3);
                    tmp = dyn->insts[dyn->insts[ninst].x64.jmp_insts].branch_address - (dyn->native_size);
                    MESSAGE(1, "Jump to %d / 0x%x\n", tmp, tmp);
                    if (tmp == 4) {
                        NOP();
                    } else {
                        B(tmp);
                    }
                }
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xEA:
            if (rex.is32bits) {
                DEFAULT;
            } else {
                INST_NAME("Illegal EA");
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7);
                BARRIER(BARRIER_FLOAT);
                UDF();
                *need_epilog = 1;
                *ok = 0;
            }
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
            if (rex.is32bits && BOX64ENV(ignoreint3)) {
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
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xF1:
            INST_NAME("INT1");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL();
            CALL(const_native_priv, -1, 0, 0); // is that a privileged opcodes or an int 1??
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xF4:
            INST_NAME("HLT");
            if (box64_unittest_mode) { // HLT in unittest mode is an exit
                READFLAGS(X_ALL);
                BARRIER(BARRIER_FLOAT);
                MOV32w(x1, 1);
                ST_W(x1, xEmu, offsetof(x64emu_t, quit));
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
            if (cpuext.lbt) {
                X64_GET_EFLAGS(x3, X_CF);
                XORI(x3, x3, 1 << F_CF);
                X64_SET_EFLAGS(x3, X_CF);
            } else {
                XORI(xFlags, xFlags, 1 << F_CF);
            }
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
                    emit_test8c(dyn, ninst, ed, u8, x3, x4, x5);
                    break;
                case 2:
                    INST_NAME("NOT Eb");
                    GETEB(x1, 0);
                    NOR(x1, x1, xZR);
                    EBBACK();
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEB(x1, 0);
                    emit_neg8(dyn, ninst, x1, x2, x4);
                    EBBACK();
                    break;
                case 4:
                    INST_NAME("MUL AL, Eb");
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    ANDI(x2, xRAX, 0xff);
                    MUL_W(x1, x2, x1);
                    BSTRINS_D(xRAX, x1, 15, 0);
                    SET_DFNONE();
                    CLEAR_FLAGS(x3);
                    IFX (X_CF | X_OF) {
                        SRLI_D(x3, x1, 8);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                    IFX (X_SF) {
                        SRLI_D(x3, xRAX, 7);
                        BSTRINS_D(xFlags, x3, F_SF, F_SF);
                    }
                    IFX (X_PF) emit_pf(dyn, ninst, xRAX, x3, x4);
                    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    GETSEB(x1, 0);
                    EXT_W_B(x2, xRAX);
                    MUL_W(x1, x2, x1);
                    BSTRINS_D(xRAX, x1, 15, 0);
                    SET_DFNONE();
                    CLEAR_FLAGS(x3);
                    IFX (X_CF | X_OF) {
                        EXT_W_H(x1, xRAX);
                        EXT_W_B(x2, x1);
                        XOR(x3, x1, x2);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                    IFX (X_SF) {
                        SRLI_D(x3, xRAX, 7);
                        BSTRINS_D(xFlags, x3, F_SF, F_SF);
                    }
                    IFX (X_PF) emit_pf(dyn, ninst, xRAX, x3, x4);
                    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    GETEB(x1, 0);
                    BSTRPICK_D(x2, xRAX, 15, 0);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(x1, xZR);
                        GETIP_(ip, x7);
                        STORE_XEMU_CALL();
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    DIV_WU(x3, x2, ed); // warning: x2 and ed must be signed extended!
                    MOD_WU(x4, x2, ed); // warning: x2 and ed must be signed extended!
                    BSTRINS_D(xRAX, x3, 7, 0);
                    BSTRINS_D(xRAX, x4, 15, 8);
                    FORCE_DFNONE();
                    RESTORE_EFLAGS(x3);
                    IFX (X_ZF | X_PF) ADDI_D(x6, xZR, 1);
                    IFX (X_OF) BSTRINS_D(xFlags, xZR, F_OF, F_OF);
                    IFX (X_CF) BSTRINS_D(xFlags, xZR, F_CF, F_CF);
                    IFX (X_AF) BSTRINS_D(xFlags, xZR, F_AF, F_AF);
                    IFX (X_ZF) BSTRINS_D(xFlags, x6, F_ZF, F_ZF);
                    IFX (X_SF) BSTRINS_D(xFlags, xZR, F_SF, F_SF);
                    IFX (X_PF) BSTRINS_D(xFlags, x6, F_PF, F_PF);
                    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SKIPTEST(x1);
                    if (!BOX64DRENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    }
                    GETSEB(x1, 0);
                    if (BOX64ENV(dynarec_div0)) {
                        BNE_MARK3(x1, xZR);
                        GETIP_(ip, x7);
                        STORE_XEMU_CALL();
                        CALL(const_native_div0, -1, 0, 0);
                        CLEARIP();
                        LOAD_XEMU_CALL();
                        jump_to_epilog(dyn, 0, xRIP, ninst);
                        MARK3;
                    }
                    EXT_W_H(x2, xRAX);
                    DIV_W(x3, x2, ed); // warning: x2 and ed must be signed extended!
                    MOD_W(x4, x2, ed); // warning: x2 and ed must be signed extended!
                    BSTRINS_D(xRAX, x3, 7, 0);
                    BSTRINS_D(xRAX, x4, 15, 8);
                    FORCE_DFNONE();
                    break;
                default:
                    DEFAULT;
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
                    GETEDsd(0);
                    NOR(ed, ed, xZR);
                    if (!rex.w && MODREG && NEED_ZEROUP(ed))
                        ZEROUP(ed);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                    GETEDsd(0);
                    emit_neg32(dyn, ninst, rex, ed, x3, x4);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    GETED(0);
                    if (rex.w) {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        MARKREGs(xRAX);
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULH_DU(gd, xRAX, ed);
                        MUL_D(xRAX, xRAX, ed);
                        if (gd != xRDX) { MV(xRDX, gd); }
                    } else {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        ZEROUP2(x3, xRAX);
                        if (MODREG) {
                            ZEROUP2(x4, ed);
                            ed = x4;
                        }
                        MUL_D(xRDX, x3, ed); // 64 <- 32x32
                        ZEROUP2(xRAX, xRDX);
                        SRLI_D(xRDX, xRDX, 32);
                    }
                    SET_DFNONE();
                    CLEAR_FLAGS(x3);
                    IFX (X_CF | X_OF) {
                        SNEZ(x3, xRDX);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                    }
                    IFX (X_SF) {
                        SRLI_D(x3, xRAX, rex.w ? 63 : 31);
                        BSTRINS_D(xFlags, x3, F_SF, F_SF);
                    }
                    IFX (X_PF) emit_pf(dyn, ninst, xRAX, x3, x5);
                    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    GETSED(0);
                    if (rex.w) {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        MARKREGs(xRAX);
                        if (ed == xRDX)
                            gd = x3;
                        else
                            gd = xRDX;
                        MULH_D(gd, xRAX, ed);
                        MUL_D(xRAX, xRAX, ed);
                        if (gd != xRDX) { MV(xRDX, gd); }
                    } else {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        ADDI_W(x3, xRAX, 0); // sign extend 32bits-> 64bits
                        MUL_D(xRDX, x3, ed); // 64 <- 32x32
                        ZEROUP2(xRAX, xRDX);
                        SRLI_D(xRDX, xRDX, 32);
                    }
                    SET_DFNONE();
                    CLEAR_FLAGS(x3);
                    IFX (X_CF | X_OF | X_SF) {
                        SRAIxw(x5, xRAX, rex.w ? 63 : 31);
                        MVxw(x6, xRDX);
                        XOR(x3, x6, x5);
                        SNEZ(x3, x3);
                        IFX (X_CF) BSTRINS_D(xFlags, x3, F_CF, F_CF);
                        IFX (X_OF) BSTRINS_D(xFlags, x3, F_OF, F_OF);
                        IFX (X_SF) BSTRINS_D(xFlags, x5, F_SF, F_SF);
                    }
                    IFX (X_PF) emit_pf(dyn, ninst, xRAX, x3, x5);
                    IFXA (X_ALL, cpuext.lbt) SPILL_EFLAGS();
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    FORCE_DFNONE();
                    if (rex.w) {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        MARKREGs(xRAX);
                        MARKREGs(xRDX);
                    }
                    if (!rex.w) {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        GETED(0);
                        if (MODREG) {
                            ZEROUP2(x4, ed);
                            ed = x4;
                        }
                        if (ninst && (nextop == 0xF0)
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0xB8
                            && *(uint32_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0) {
                            // hack for some protection that check a divide by zero actually trigger a divide by zero exception
                            MESSAGE(LOG_INFO, "Divide by 0 hack\n");
                            GETIP(ip, x7);
                            STORE_XEMU_CALL();
                            CALL(const_native_div0, -1, 0, 0);
                            LOAD_XEMU_CALL();
                        } else {
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL();
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            SLLI_D(x3, xRDX, 32);
                            ZEROUP2(x2, xRAX);
                            OR(x3, x3, x2);
                            DIV_DU(x2, x3, ed);
                            MOD_DU(xRDX, x3, ed);
                            ZEROUP2(xRAX, x2);
                            if (NEED_ZEROUP(xRDX)) ZEROUP(xRDX);
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
                                STORE_XEMU_CALL();
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARK3;
                            }
                            DIV_DU(x2, xRAX, ed);
                            MOD_DU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x4, x1, 0); // get edd changed addr, so cannot be called 2 times for same op...
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARK3(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL();
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
                            DIV_DU(x2, xRAX, ed);
                            MOD_DU(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        }
                    }
                    CLEAR_FLAGS(x7);
                    IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
                    IFX (X_PF) ORI(xFlags, xFlags, 1 << F_PF);
                    SPILL_EFLAGS();
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SKIPTEST(x1);
                    if (!BOX64DRENV(dynarec_safeflags)) {
                        SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION);
                    }
                    if (rex.w) {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        MARKREGs(xRAX);
                        MARKREGs(xRDX);
                    }
                    if (!rex.w) {
                        MARKREGd(xRAX);
                        MARKREGd(xRDX);
                        SET_DFNONE();
                        GETSED(0);
                        if (BOX64ENV(dynarec_div0)) {
                            BNE_MARK3(ed, xZR);
                            GETIP_(ip, x7);
                            STORE_XEMU_CALL();
                            CALL(const_native_div0, -1, 0, 0);
                            CLEARIP();
                            LOAD_XEMU_CALL();
                            jump_to_epilog(dyn, 0, xRIP, ninst);
                            MARK3;
                        }
                        SLLI_D(x3, xRDX, 32);
                        ZEROUP2(x2, xRAX);
                        OR(x3, x3, x2);
                        DIV_D(x2, x3, ed);
                        MOD_D(xRDX, x3, ed);
                        if (BOX64ENV(dynarec_div0)) {
                            SEXT_W(x5, x2);
                            BEQ_MARK2(x5, x2);
                            GETIP_(ip, x7);
                            STORE_XEMU_CALL();
                            CALL(const_native_div0, -1, 0, 0);
                            CLEARIP();
                            LOAD_XEMU_CALL();
                            jump_to_epilog(dyn, 0, xRIP, ninst);
                            MARK2;
                        }
                        ZEROUP2(xRAX, x2);
                        if (NEED_ZEROUP(xRDX)) ZEROUP(xRDX);
                    } else {
                        if (ninst && dyn->insts
                            && dyn->insts[ninst - 1].x64.addr
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr) == 0x48
                            && *(uint8_t*)(dyn->insts[ninst - 1].x64.addr + 1) == 0x99) {
                            FORCE_DFNONE();
                            GETED(0);
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARKF2(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL();
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARKF2;
                            }
                            DIV_D(x2, xRAX, ed);
                            MOD_D(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                        } else {
                            GETEDH(x4, x1, 0); // get edd changed addr, so cannot be called 2 times for same op...
                            if (BOX64ENV(dynarec_div0)) {
                                BNE_MARKF(ed, xZR);
                                GETIP_(ip, x7);
                                STORE_XEMU_CALL();
                                CALL(const_native_div0, -1, 0, 0);
                                CLEARIP();
                                LOAD_XEMU_CALL();
                                jump_to_epilog(dyn, 0, xRIP, ninst);
                                MARKF;
                            }
                            // need to see if RDX == 0 and RAX not signed
                            // or RDX == -1 and RAX signed
                            BNE_MARK2(xRDX, xZR);
                            BGE_MARK(xRAX, xZR);
                            MARK2;
                            NOR(x2, xZR, xRDX);
                            BNE_MARK3(x2, xZR);
                            BLT_MARK(xRAX, xZR);
                            MARK3;
                            CALL(const_idiv64, -1, ed, 0);
                            B_NEXT_nocond;
                            MARK;
                            DIV_D(x2, xRAX, ed);
                            MOD_D(xRDX, xRAX, ed);
                            MV(xRAX, x2);
                            FORCE_DFNONE();
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF8:
            INST_NAME("CLC");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            if (cpuext.lbt)
                X64_SET_EFLAGS(xZR, X_CF);
            else
                BSTRINS_D(xFlags, xZR, F_CF, F_CF);
            break;
        case 0xF9:
            INST_NAME("STC");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            if (cpuext.lbt) {
                ORI(x3, xZR, 1 << F_CF);
                X64_SET_EFLAGS(x3, X_CF);
            } else {
                ORI(xFlags, xFlags, 1 << F_CF);
            }
            break;
        case 0xFA:
        case 0xFB:
            INST_NAME(opcode == 0xFA ? "CLI" : "STI");
            if (rex.is32bits && BOX64ENV(ignoreint3)) {
            } else {
                if (BOX64DRENV(dynarec_safeflags) > 1) {
                    READFLAGS(X_PEND);
                } else {
                    SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
                }
                GETIP(ip, x7); // priviledged instruction, IP not updated
                STORE_XEMU_CALL();
                CALL(const_native_priv, -1, 0, 0);
                LOAD_XEMU_CALL();
                jump_to_epilog(dyn, 0, xRIP, ninst);
                *need_epilog = 0;
                *ok = 0;
            }
            break;
        case 0xFC:
            INST_NAME("CLD");
            BSTRINS_D(xFlags, xZR, F_DF, F_DF);
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
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEB(x1, 0);
                    emit_inc8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEB(x1, 0);
                    emit_dec8(dyn, ninst, ed, x2, x4, x5);
                    EBBACK();
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
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEDsd(0);
                    emit_inc32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 1: // DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL & ~X_CF, SF_SUBSET, NAT_FLAGS_FUSION);
                    GETEDsd(0);
                    emit_dec32(dyn, ninst, rex, ed, x3, x4, x5, x6);
                    WBACK;
                    break;
                case 2:
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
                        // Push actual return address
                        if (addr < (dyn->start + dyn->isize)) {
                            // there is a next
                            if(BOX64DRENV(dynarec_callret)>1 && !dyn->always_test)
                                j64 = CALLRET_GETRET();
                            else
                                j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0;
                            PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                            ADDI_D(x4, x4, j64 & 0xfff);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        } else {
                            j64 = (dyn->insts) ? (GETMARK - (dyn->native_size)) : 0;
                            PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                            ADDI_D(x4, x4, j64 & 0xfff);
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                        }
                        ADDI_D(xSP, xSP, -16);
                        ST_D(x4, xSP, 0);
                        ST_D(xRIP, xSP, 8);
                    }
                    PUSH1z(xRIP);
                    jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                    int can_continue = (addr < (dyn->start + dyn->isize));
                    CALLRET_RET(can_continue);
                    if (BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
                        // jumps out of current dynablock...
                        MARK;
                        j64 = getJumpTableAddress64(addr);
                        if (dyn->need_reloc) AddRelocTable64RetEndBlock(dyn, ninst, addr, STEP);
                        TABLE64_(x4, j64);
                        LD_D(x4, x4, 0);
                        BR(x4);
                    }
                    CLEARIP();
                    break;
                case 3: // CALL FAR Ed
                    if (MODREG) {
                        DEFAULT;
                    } else {
                        INST_NAME("CALL FAR Ed");
                        READFLAGS(X_PEND);
                        BARRIER(BARRIER_FLOAT);
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, 0);
                        LDxw(x1, wback, 0);
                        ed = x1;
                        LD_HU(x3, wback, rex.w ? 8 : 4);
                        LD_HU(x5, xEmu, offsetof(x64emu_t, segs[_CS]));
                        if (BOX64DRENV(dynarec_callret) && BOX64DRENV(dynarec_bigblock) > 1) {
                            BARRIER(BARRIER_FULL);
                        } else {
                            BARRIER(BARRIER_FLOAT);
                            *need_epilog = 0;
                            *ok = 0;
                        }
                        GETIP_(addr, x7);
                        int can_continue = (addr < (dyn->start + dyn->isize));
                        if (BOX64DRENV(dynarec_callret)) {
                            SET_HASCALLRET();
                            // Push actual return address. Note that CS will not be tested, but that should be ok?
                            if (can_continue) {
                                // there is a next...
                                if(BOX64DRENV(dynarec_callret)>1 && !dyn->always_test)
                                    j64 = CALLRET_GETRET();
                                else
                                    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0;
                                PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                                ADDI_D(x4, x4, j64 & 0xfff);
                                MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                            } else {
                                j64 = (dyn->insts) ? (GETMARK - (dyn->native_size)) : 0;
                                PCADDU12I(x4, ((j64 + 0x800) >> 12) & 0xfffff);
                                ADDI_D(x4, x4, j64 & 0xfff);
                                MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j64 >> 2);
                            }
                            ADDI_D(xSP, xSP, -16);
                            ST_D(x4, xSP, 0);
                            ST_D(xRIP, xSP, 8);
                        }
                        if (rex.w) {
                            PUSH1(x5);
                            PUSH1(xRIP);
                        } else {
                            PUSH1_32(x5);
                            PUSH1_32(xRIP);
                        }
                        ST_H(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
                        jump_to_next(dyn, 0, ed, ninst, rex.is32bits);
                        CALLRET_RET(can_continue);
                        if (BOX64DRENV(dynarec_callret) && addr >= (dyn->start + dyn->isize)) {
                            // jumps out of current dynablock...
                            MARK;
                            j64 = getJumpTableAddress64(addr);
                            if (dyn->need_reloc) AddRelocTable64RetEndBlock(dyn, ninst, addr, STEP);
                            TABLE64_(x4, j64);
                            LD_D(x4, x4, 0);
                            BR(x4);
                        }
                        CLEARIP();
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
                        LD_HU(x3, wback, rex.w ? 8 : 4);
                        ST_H(x3, xEmu, offsetof(x64emu_t, segs[_CS]));
                        jump_to_next(dyn, 0, ed, ninst, rex.w ? 0 : 1);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    if (!rex.is32bits) UP32_READ(xRSP);
                    GETEDz(0);
                    PUSH1z(ed);
                    SMWRITE();
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
