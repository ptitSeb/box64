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

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t u8;
    uint64_t u64, j64;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int64_t fixedaddress;
    int unscaled;

    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(v0);
    MAYUSE(v1);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 0);
                VMOVeD(v0, 0, d0, 0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VLD64(v0, ed, fixedaddress); // upper part reseted
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = (nextop&7)+ (rex.b<<3);
                d0 = sse_get_reg(dyn, ninst, x1, ed, 1);
                VMOVeD(d0, 0, v0, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, 0);
                VST64(v0, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VDUPQ_64(v0, d0, 0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, NULL, 0, 0, rex, NULL, 0, 0);
                VLDQ1R_64(v0, ed);
            }
            break;

        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            GETGX(v0, 1);
            GETED(0);
            d1 = fpu_get_scratch(dyn, ninst);
            if(BOX64ENV(dynarec_fastround)<2) {
                u8 = sse_setround(dyn, ninst, x3, x4, x5);
            }
            if(rex.w) {
                SCVTFDx(d1, ed);
            } else {
                SCVTFDw(d1, ed);
            }
            if(BOX64ENV(dynarec_fastround)<2) {
                x87_restoreround(dyn, ninst, u8);
            }
            VMOVeD(v0, 0, d1, 0);
            break;

        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(q0, 0, 0);
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            if(!BOX64ENV(dynarec_fastround) && cpuext.frintts) {
                v0 = fpu_get_scratch(dyn, ninst);
                if(rex.w) {
                    FRINT64ZD(v0, q0);
                } else {
                    FRINT32ZD(v0, q0);
                }
                FCVTZSxwD(gd, v0);
            } else {
                FCVTZSxwD(gd, q0);
            }
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSD(q0, 0, 0);
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);
                BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                MSR_fpsr(x5);
            }
            u8 = sse_setround(dyn, ninst, x1, x2, x3);
            d1 = fpu_get_scratch(dyn, ninst);
            if(!BOX64ENV(dynarec_fastround) && cpuext.frintts) {
                if(rex.w) {
                    FRINT64XD(d1, q0);
                } else {
                    FRINT32XD(d1, q0);
                }
            } else {
                FRINTID(d1, q0);
            }
            x87_restoreround(dyn, ninst, u8);
            FCVTZSxwD(gd, d1);
            if(!BOX64ENV(dynarec_fastround) && !cpuext.frintts) {
                MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                TBZ_NEXT(x5, FPSR_IOC);
                if(rex.w) {
                    ORRx_mask(gd, xZR, 1, 1, 0);    //0x8000000000000000
                } else {
                    MOV32w(gd, 0x80000000);
                }
            }
            break;

        case 0x38:  // these are some more SSSE4.2+ opcodes
            opcode = F8;
            switch(opcode) {

                case 0xF0:
                    INST_NAME("CRC32 Gd, Eb");
                    nextop = F8;
                    GETEB(x1, 0);
                    GETGD;
                    if(cpuext.crc32) {
                        CRC32CB(gd, gd, ed);
                    } else {
                        EORw_REG(gd, gd, ed);
                        MOV32w(x2, 0x82f63b78);
                        for(int i=0; i<8; ++i) {
                            LSRw_IMM((i&1)?gd:x4, (i&1)?x4:gd, 1);
                            TBZ((i&1)?x4:gd, 0, 4+4);
                            EORw_REG((i&1)?gd:x4, (i&1)?gd:x4, x2);
                        }
                    }
                    break;
                case 0xF1:
                    INST_NAME("CRC32 Gd, Ed");
                    nextop = F8;
                    GETED(0);
                    GETGD;
                    if(cpuext.crc32) {
                        CRC32Cxw(gd, gd, ed);
                    } else {
                        MOV32w(x2, 0x82f63b78);
                        for(int j=0; j<4*(1+rex.w); ++j) {
                            UBFXxw(x3, ed, 8*j, 8);
                            EORw_REG(gd, gd, x3);
                            for(int i=0; i<8; ++i) {
                                LSRw_IMM((i&1)?gd:x4, (i&1)?x4:gd, 1);
                                TBZ((i&1)?x4:gd, 0, 4+4);
                                EORw_REG((i&1)?gd:x4, (i&1)?gd:x4, x2);
                            }
                        }
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;


        case 0x51:
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(d0, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FCMEQD(v0, d0, d0);    // 0 if NAN, 1 if not NAN
                FSQRTD(d1, d0);
                FCMEQD(v1, d1, d1);    // 0 => out is NAN
                VBIC(v1, v0, v1);      // forget it in any input was a NAN already
                SHL_64(v1, v1, 63);   // only keep the sign bit
                VORR(d1, d1, v1);      // NAN -> -NAN
            } else {
                FSQRTD(d1, d0);
            }
            VMOVeD(q0, 0, d1, 0);
            break;

        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            GETGX(d1, 1);
            v1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(d0, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(v0, d0, d1);    // propagate NAN
                FCMEQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
                FADDD(v1, d1, d0);  // the high part of the vector is erased...
                FCMEQD(q0, v1, v1);    // 0 => out is NAN
                VBIC(q0, v0, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(v1, v1, q0);      // NAN -> -NAN
            } else {
                FADDD(v1, d1, d0);  // the high part of the vector is erased...
            }
            VMOVeD(d1, 0, v1, 0);
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            GETGX(d1, 1);
            v1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(d0, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(v0, d0, d1);    // propagate NAN
                FCMEQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
                FMULD(v1, d1, d0);
                FCMEQD(q0, v1, v1);    // 0 => out is NAN
                VBIC(q0, v0, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(v1, v1, q0);      // NAN -> -NAN
            } else {
                FMULD(v1, d1, d0);
            }
            VMOVeD(d1, 0, v1, 0);
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(d0, 0, 0);
            d1 = fpu_get_scratch(dyn, ninst);
            if(BOX64ENV(dynarec_fastround)>1) {
                FCVT_S_D(d1, d0);
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                FCVT_S_D(d1, d0);
                x87_restoreround(dyn, ninst, u8);
            }
            VMOVeS(v0, 0, d1, 0);
            break;

        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            GETGX(d1, 1);
            v1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(d0, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(v0, d0, d1);    // propagate NAN
                FCMEQD(v0, v0, v0);    // 0 if NAN, 1 if not NAN
                FSUBD(v1, d1, d0);
                FCMEQD(q0, v1, v1);    // 0 => out is NAN
                VBIC(q0, v0, q0);      // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(v1, v1, q0);      // NAN -> -NAN
            } else {
                FSUBD(v1, d1, d0);
            }
            VMOVeD(d1, 0, v1, 0);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            // MINSD: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            FCMPD(v0, v1);
            B_NEXT(cCC);    //CC invert of CS: NAN or == or Gx > Ex
            VMOVeD(v0, 0, v1, 0);   // to not erase uper part
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            d1 = fpu_get_scratch(dyn, ninst);
            GETEXSD(v1, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn, ninst);
                q0 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                FMAXD(d0, v0, v1);      // propagate NAN
                FCMEQD(d0, d0, d0);     // 0 if NAN, 1 if not NAN
                FDIVD(d1, v0, v1);
                FCMEQD(q0, d1, d1);     // 0 => out is NAN
                VBIC(q0, d0, q0);       // forget it in any input was a NAN already
                SHL_64(q0, q0, 63);     // only keep the sign bit
                VORR(d1, d1, q0);       // NAN -> -NAN
            } else {
                FDIVD(d1, v0, v1);
            }
            VMOVeD(v0, 0, d1, 0);
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 0);
            // MAXSD: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            FCMPD(v1, v0);
            B_NEXT(cCC);    //CC invert of CS: NAN or == or Ex > Gx
            VMOVeD(v0, 0, v1, 0);   // to not erase uper part
            break;

        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX(v0, 1);

            u8 = F8;
            if(u8==0b00000000 || u8==0b01010101 || u8==0b10101010 || u8==0b11111111) {
                if(v0==v1) {
                    d0 = fpu_get_scratch(dyn, ninst);
                    VMOVQ(d0, v1);
                }
                VDUP_16(v0, v1, u8&3);
                if(v0==v1)
                    v1 = d0;
            } else {
                // only low part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
                u64 = 0;
                for (int i=0; i<4; ++i) {
                    u64 |= ((uint64_t)((u8>>(i*2))&3)*2+0)<<(i*16+0);
                    u64 |= ((uint64_t)((u8>>(i*2))&3)*2+1)<<(i*16+8);
                }
                MOV64x(x2, u64);
                d0 = fpu_get_scratch(dyn, ninst);
                VMOVQDfrom(d0, 0, x2);
                VTBL1_8(d0, v1, d0);
                VMOVeD(v0, 0, d0, 0);
                }
            if(v0!=v1) {
                VMOVeD(v0, 1, v1, 1);
            }
            break;

        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
            nextop = F8;
            GETGX(q1, 1);
            GETEX(q0, 0, 0);
            if(!BOX64ENV(dynarec_fastnan)) {
                v0 = fpu_get_scratch(dyn, ninst);
                v1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXPQS(v0, q1, q0);    // propagate NAN
                VFCMEQQS(v0, v0, v0);    // 0 if NAN, 1 if not NAN
            }
            VFADDPQS(q1, q1, q0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(v1, q1, q1);    // 0 => out is NAN
                VBICQ(v1, v0, v1);      // forget it in any input was a NAN already
                VSHLQ_32(v1, v1, 31);   // only keep the sign bit
                VORRQ(q1, q1, v1);      // NAN -> -NAN
            }
            break;
        case 0x7D:
            INST_NAME("HSUBPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            d0 = fpu_get_scratch(dyn, ninst);
            VUZP1Q_32(d0, v0, v1);
            VUZP2Q_32(v0, v0, v1);
            if(!BOX64ENV(dynarec_fastnan)) {
                d1 = fpu_get_scratch(dyn, ninst);
                // check if any input value was NAN
                VFMAXQS(d1, d0, v0);    // propagate NAN
                VFCMEQQS(d1, d1, d1);    // 0 if NAN, 1 if not NAN
            }
            VFSUBQS(v0, d0, v0);
            if(!BOX64ENV(dynarec_fastnan)) {
                VFCMEQQS(d0, v0, v0);    // 0 => out is NAN
                VBICQ(d1, d1, d0);      // forget it in any input was a NAN already
                VSHLQ_32(d1, d1, 31);   // only keep the sign bit
                VORRQ(v0, v0, d1);      // NAN -> -NAN
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)   \
            READFLAGS(F);                                               \
            i32_ = F32S;                                                \
            if(rex.is32bits)                                            \
                j64 = (uint32_t)(addr+i32_);                            \
            else                                                        \
                j64 = addr+i32_;                                        \
            JUMP(j64, 1);                                               \
            GETFLAGS;                                                   \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3, 0);   \
                    jump_to_next(dyn, j64, 0, ninst, rex.is32bits);     \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd);               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                    SKIP_SEVL(i32);                                     \
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                SKIP_SEVL(i32);                                         \
                Bcond(YES, i32);                                        \
            }

        GOCOND(0x80, "J", "Id");
        #undef GO

        case 0xAE:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 6:
                    INST_NAME("(unsupported) UWAIT Ed");
                    FAKEED;
                    UDF(0);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xC2:
            INST_NAME("CMPSD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSD(v1, 0, 1);
            u8 = F8;
            FCMPD(v0, v1);
            switch(u8&7) {
                case 0: CSETMx(x2, cEQ); break;   // Equal
                case 1: CSETMx(x2, cCC); break;   // Less than
                case 2: CSETMx(x2, cLS); break;   // Less or equal
                case 3: CSETMx(x2, cVS); break;   // NaN
                case 4: CSETMx(x2, cNE); break;   // Not Equal or unordered
                case 5: CSETMx(x2, cCS); break;   // Greater or equal or unordered
                case 6: CSETMx(x2, cHI); break;   // Greater or unordered, test inverted, N!=V so unordered or less than (inverted)
                case 7: CSETMx(x2, cVC); break;   // not NaN
            }
            VMOVQDfrom(v0, 0, x2);
            break;

        case 0xD0:
            INST_NAME("ADDSUBPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            q0 = fpu_get_scratch(dyn, ninst);
            TABLE64C(x2, const_4f_m1_1_m1_1);
            VLDR128_U12(q0, x2, 0);
            VFMLAQS(v0, v1, q0);
            break;

        case 0xD6:
            INST_NAME("MOVDQ2Q Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEXSD(v1, 0, 0);
            VMOV(v0, v1);
            break;

        case 0xE6:
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            if(BOX64ENV(dynarec_fastround)) {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                VFRINTIDQ(v0, v1);
                x87_restoreround(dyn, ninst, u8);
                VFCVTNSQD(v0, v0);  // convert double -> int64
                SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
            } else {
                u8 = sse_setround(dyn, ninst, x1, x2, x3);
                if(cpuext.frintts) {
                    VFRINT32XDQ(v0, v1);    // round, handling of overflow and Nan to 0x80000000
                    VFCVTNSQD(v0, v0);  // convert double -> int64
                    SQXTN_32(v0, v0);   // convert int64 -> int32 with saturation in lower part, RaZ high part
                } else {
                    MRS_fpsr(x5);
                    ORRw_mask(x4, xZR, 1, 0);    //0x80000000
                    d0 = fpu_get_scratch(dyn, ninst);
                    for(int i=0; i<2; ++i) {
                        BFCw(x5, FPSR_IOC, 1);   // reset IOC bit
                        MSR_fpsr(x5);
                        if(i) {
                            VMOVeD(d0, 0, v1, i);
                            FRINTID(d0, d0);
                        } else {
                            FRINTID(d0, v1);
                        }
                        FCVTZSwD(x1, d0);
                        MRS_fpsr(x5);   // get back FPSR to check the IOC bit
                        TBZ(x5, FPSR_IOC, 4+4);
                        MOVw_REG(x1, x4);
                        VMOVQSfrom(v0, i, x1);
                    }
                    VMOVQDfrom(v0, 1, xZR);
                }
                x87_restoreround(dyn, ninst, u8);
            }
            break;

        case 0xF0:
            INST_NAME("LDDQU Gx,Ex");
            nextop = F8;
            GETG;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, rex, NULL, 0, 0);
                VLD128(v0, ed, fixedaddress);
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
