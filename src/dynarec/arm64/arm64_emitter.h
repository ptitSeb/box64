#ifndef __ARM64_EMITTER_H__
#define __ARM64_EMITTER_H__
/*
    ARM64 Emitter

*/

#include "arm64_mapping.h"

int convert_bitmask(uint64_t bitmask);
#define convert_bitmask_w(A)    convert_bitmask(((uint64_t)(A) << 32) + (uint32_t)(A))
#define convert_bitmask_x(A)    convert_bitmask((uint64_t)A)
#define convert_bitmask_xw(A)   convert_bitmask(rex.w?((uint64_t)(A)):(((uint64_t)(A) << 32) + (uint32_t)(A)))

#define invCond(cond)   ((cond)^0b0001)

// MOVZ
#define MOVZ_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b10<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVZx(Rd, imm16)                    EMIT(MOVZ_gen(1, 0, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVZx_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(1, (shift)/16, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVZw(Rd, imm16)                    EMIT(MOVZ_gen(0, 0, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVZw_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(0, (shift)/16, ((uint16_t)(imm16))&0xffff, Rd))

// MOVN
#define MOVN_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b00<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVNx(Rd, imm16)                    EMIT(MOVN_gen(1, 0, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVNx_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(1, (shift)/16, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVNw(Rd, imm16)                    EMIT(MOVN_gen(0, 0, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVNw_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(0, (shift)/16, ((uint16_t)(imm16))&0xffff, Rd))

// MOVK
#define MOVK_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b11<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVKx(Rd, imm16)                    EMIT(MOVK_gen(1, 0, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVKx_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(1, (shift)/16, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVKw(Rd, imm16)                    EMIT(MOVK_gen(0, 0, ((uint16_t)(imm16))&0xffff, Rd))
#define MOVKw_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(0, (shift)/16, ((uint16_t)(imm16))&0xffff, Rd))

#define MOV32w(Rd, imm32) arm64_move32(dyn, ninst, Rd, imm32)
#define MOV64x(Rd, imm64) arm64_move64(dyn, ninst, Rd, imm64)

#define MOV64xw(Rd, imm64)   if(rex.w) {MOV64x(Rd, imm64);} else {MOV32w(Rd, imm64);}
#define MOV64z(Rd, imm64)    if(rex.is32bits) {MOV32w(Rd, imm64);} else {MOV64x(Rd, imm64);}
#define MOV64y(Rd, imm64)    if(rex.is32bits || rex.is67) {MOV32w(Rd, imm64);} else {MOV64x(Rd, imm64);}


// ADD / SUB
#define ADDSUB_REG_gen(sf, op, S, shift, Rm, imm6, Rn, Rd) ((sf)<<31 | (op)<<30 | (S)<<29 | 0b01011<<24 | (shift)<<22 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ADDx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSx_REG(Rd, Rn, Rm)              FEMIT(ADDSUB_REG_gen(1, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDx_REG_LSR(Rd, Rn, Rm, lsr)       EMIT(ADDSUB_REG_gen(1, 0, 0, 0b01, Rm, lsr, Rn, Rd))
#define ADDz_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen((rex.is32bits)?0:1, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDy_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen((rex.is32bits || rex.is67)?0:1, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSw_REG(Rd, Rn, Rm)              FEMIT(ADDSUB_REG_gen(0, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDw_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(0, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDw_REG_LSR(Rd, Rn, Rm, lsr)       EMIT(ADDSUB_REG_gen(0, 0, 0, 0b01, Rm, lsr, Rn, Rd))
#define ADDSw_REG_LSL(Rd, Rn, Rm, lsl)     FEMIT(ADDSUB_REG_gen(0, 0, 1, 0b00, Rm, lsl, Rn, Rd))
#define ADDxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDz_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen((rex.is32bits)?0:1, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDy_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen((rex.is32bits || rex.is67)?0:1, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSxw_REG(Rd, Rn, Rm)             FEMIT(ADDSUB_REG_gen(rex.w, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDxw_REG_LSR(Rd, Rn, Rm, lsr)      EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b01, Rm, lsr, Rn, Rd))

#define ADDSUB_IMM_gen(sf, op, S, shift, imm12, Rn, Rd)    ((sf)<<31 | (op)<<30 | (S)<<29 | 0b10001<<24 | (shift)<<22 | (imm12)<<10 | (Rn)<<5 | (Rd))
#define ADDx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSx_U12(Rd, Rn, imm12)   FEMIT(ADDSUB_IMM_gen(1, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSw_U12(Rd, Rn, imm12)   FEMIT(ADDSUB_IMM_gen(0, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSxw_U12(Rd, Rn, imm12)  FEMIT(ADDSUB_IMM_gen(rex.w, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDz_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen((rex.is32bits)?0:1, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDy_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen((rex.is32bits || rex.is67)?0:1, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))

#define SUBx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSx_REG(Rd, Rn, Rm)              FEMIT(ADDSUB_REG_gen(1, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBSx_REG_ASR(Rd, Rn, Rm, asr)     FEMIT(ADDSUB_REG_gen(1, 1, 1, 0b10, Rm, asr, Rn, Rd))
#define SUBx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBw_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBw_REG_ASR(Rd, Rn, Rm, asr)       EMIT(ADDSUB_REG_gen(0, 1, 0, 0b10, Rm, asr, Rn, Rd))
#define SUBSw_REG(Rd, Rn, Rm)              FEMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBSw_REG_LSL(Rd, Rn, Rm, lsl)     FEMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, lsl, Rn, Rd))
#define SUBSw_REG_LSR(Rd, Rn, Rm, lsr)     FEMIT(ADDSUB_REG_gen(0, 1, 1, 0b01, Rm, lsr, Rn, Rd))
#define SUBSw_REG_ASR(Rd, Rn, Rm, asr)     FEMIT(ADDSUB_REG_gen(0, 1, 1, 0b10, Rm, asr, Rn, Rd))
#define SUBxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBz_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(rex.is32bits?0:1, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBy_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen((rex.is32bits || rex.is67)?0:1, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSxw_REG(Rd, Rn, Rm)             FEMIT(ADDSUB_REG_gen(rex.w, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBSxw_REG_ASR(Rd, Rn, Rm, asr)    FEMIT(ADDSUB_REG_gen(rex.w, 1, 1, 0b10, Rm, asr, Rn, Rd))
#define CMPSx_REG(Rn, Rm)                   SUBSx_REG(xZR, Rn, Rm)
#define CMPSx_REG_ASR(Rn, Rm, asr)          SUBSx_REG_ASR(xZR, Rn, Rm, asr)
#define CMPSw_REG(Rn, Rm)                   SUBSw_REG(wZR, Rn, Rm)
#define CMPSw_REG_LSR(Rn, Rm, lsr)          SUBSw_REG_LSR(wZR, Rn, Rm, lsr)
#define CMPSw_REG_ASR(Rn, Rm, asr)          SUBSw_REG_ASR(wZR, Rn, Rm, asr)
#define CMPSxw_REG(Rn, Rm)                  SUBSxw_REG(xZR, Rn, Rm)
#define CMPSxw_REG_ASR(Rn, Rm, asr)         SUBSxw_REG_ASR(xZR, Rn, Rm, asr)
#define NEGx_REG(Rd, Rm)                    SUBx_REG(Rd, xZR, Rm);
#define NEGw_REG(Rd, Rm)                    SUBw_REG(Rd, wZR, Rm);
#define NEGxw_REG(Rd, Rm)                   SUBxw_REG(Rd, xZR, Rm);
#define NEGSx_REG(Rd, Rm)                   SUBSx_REG(Rd, xZR, Rm);
#define NEGSw_REG(Rd, Rm)                   SUBSw_REG(Rd, wZR, Rm);
#define NEGSxw_REG(Rd, Rm)                  SUBSxw_REG(Rd, xZR, Rm);

#define SUBx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSx_U12(Rd, Rn, imm12)   FEMIT(ADDSUB_IMM_gen(1, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSw_U12(Rd, Rn, imm12)   FEMIT(ADDSUB_IMM_gen(0, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBz_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(rex.is32bits?0:1, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBy_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen((rex.is32bits || rex.is67)?0:1, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSxw_U12(Rd, Rn, imm12)  FEMIT(ADDSUB_IMM_gen(rex.w, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define CMPSx_U12(Rn, imm12)        SUBSx_U12(xZR, Rn, imm12)
#define CMPSw_U12(Rn, imm12)        SUBSw_U12(wZR, Rn, imm12)
#define CMPSxw_U12(Rn, imm12)       SUBSxw_U12(xZR, Rn, imm12)

#define ADDSUBC_gen(sf, op, S, Rm, Rn, Rd)  ((sf)<<31 | (op)<<30 | (S)<<29 | 0b11010000<<21 | (Rm)<<16 | (Rn)<<5 | (Rd))
#define ADCx_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(1, 0, 0, Rm, Rn, Rd))
#define ADCw_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(0, 0, 0, Rm, Rn, Rd))
#define ADCxw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(rex.w, 0, 0, Rm, Rn, Rd))
#define SBCx_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(1, 1, 0, Rm, Rn, Rd))
#define SBCw_REG(Rd, Rn, Rm)        EMIT(ADDSUBC_gen(0, 1, 0, Rm, Rn, Rd))
#define SBCxw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(rex.w, 1, 0, Rm, Rn, Rd))
#define ADCSx_REG(Rd, Rn, Rm)      FEMIT(ADDSUBC_gen(1, 0, 1, Rm, Rn, Rd))
#define ADCSw_REG(Rd, Rn, Rm)      FEMIT(ADDSUBC_gen(0, 0, 1, Rm, Rn, Rd))
#define ADCSxw_REG(Rd, Rn, Rm)     FEMIT(ADDSUBC_gen(rex.w, 0, 1, Rm, Rn, Rd))
#define SBCSx_REG(Rd, Rn, Rm)      FEMIT(ADDSUBC_gen(1, 1, 1, Rm, Rn, Rd))
#define SBCSw_REG(Rd, Rn, Rm)      FEMIT(ADDSUBC_gen(0, 1, 1, Rm, Rn, Rd))
#define SBCSxw_REG(Rd, Rn, Rm)     FEMIT(ADDSUBC_gen(rex.w, 1, 1, Rm, Rn, Rd))

#define ADDSUB_ext(sf, op, S, Rm, option, imm3, Rn, Rd)    ((sf)<<31 | (op)<<30 | (S)<<29 | 0b01011<<24 | 1<<21 | (Rm)<<16 | (option)<<13 | (imm3)<<10 | (Rn)<<5 | (Rd))
#define SUBxw_UXTB(Rd, Rn, Rm)      EMIT(ADDSUB_ext(rex.w, 1, 0, Rm, 0b000, 0, Rn, Rd))
#define SUBw_UXTB(Rd, Rn, Rm)       EMIT(ADDSUB_ext(0, 1, 0, Rm, 0b000, 0, Rn, Rd))
#define ADDw_UXTH(Rd, Rn, Rm)       EMIT(ADDSUB_ext(0, 0, 0, Rm, 0b001, 0, Rn, Rd))
#define ADDx_UXTW(Rd, Rn, Rm)       EMIT(ADDSUB_ext(1, 0, 0, Rm, 0b010, 0, Rn, Rd))
#define ADDx_SXTW(Rd, Rn, Rm)       EMIT(ADDSUB_ext(1, 0, 0, Rm, 0b110, 0, Rn, Rd))
#define ADDx_REGy(Rd, Rn, Rm)       do {if(rex.is32bits || rex.is67) ADDx_SXTW(Rd, Rn, Rm); else ADDx_REG(Rd, Rn, Rm); } while(0)

// CCMP compare if cond is true, set nzcv if false
#define CCMP_reg(sf, Rm, cond, Rn, nzcv)    ((sf)<<31 | 1<<30 | 1<<29 | 0b11010010<<21 | (Rm)<<16 | (cond)<<12 | (Rn)<<5 | (nzcv))
#define CCMPw(Wn, Wm, nzcv, cond)  FEMIT(CCMP_reg(0, Wm, cond, Wn, nzcv))
#define CCMPx(Xn, Xm, nzcv, cond)  FEMIT(CCMP_reg(1, Xm, cond, Xn, nzcv))
#define CCMPxw(Xn, Xm, nzcv, cond) FEMIT(CCMP_reg(rex.w, Xm, cond, Xn, nzcv))

// ADR
#define ADR_gen(immlo, immhi, Rd)   ((immlo)<<29 | 0b10000<<24 | (immhi)<<5 | (Rd))
#define ADR_S20(Rd, imm)            EMIT(ADR_gen((imm)&3, ((imm)>>2)&0x7ffff, (Rd)))

// LDR
#define LDR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRx_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRx_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRw_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRw_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRB_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b00, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRB_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b00, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRH_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b01, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRH_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b01, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRxw_S9_postindex(Rt, Rn, imm9)  EMIT(LDR_gen(rex.w?0b11:0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))

#define LDRS_gen(size, op1, imm9, op2, Rn, Rt)   ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b10<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRSW_S9_postindex(Rt, Rn, imm9)  EMIT(LDRS_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRSW_S9_preindex(Rt, Rn, imm9)   EMIT(LDRS_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))

#define LD_gen(size, op1, imm12, Rn, Rt)        ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRx_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b11, 0b01, ((uint32_t)((imm12)>>3))&0xfff, Rn, Rt))
#define LDRw_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b10, 0b01, ((uint32_t)((imm12)>>2))&0xfff, Rn, Rt))
#define LDRB_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b00, 0b01, ((uint32_t)((imm12)))&0xfff, Rn, Rt))
#define LDRH_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b01, 0b01, ((uint32_t)((imm12)>>1))&0xfff, Rn, Rt))
#define LDRxw_U12(Rt, Rn, imm12)          EMIT(LD_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)((imm12)>>(2+rex.w)))&0xfff, Rn, Rt))
#define LDRz_U12(Rt, Rn, imm12)           EMIT(LD_gen((rex.is32bits)?0b10:0b11, 0b01, ((uint32_t)((imm12)>>(rex.is32bits?2:3)))&0xfff, Rn, Rt))

#define LDS_gen(size, op1, imm12, Rn, Rt)       ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b10<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSW_U12(Rt, Rn, imm12)          EMIT(LDS_gen(0b10, 0b01, ((uint32_t)((imm12)>>2))&0xfff, Rn, Rt))
#define LDRSH_U12(Rt, Rn, imm12)          EMIT(LDS_gen(0b01, 0b01, ((uint32_t)((imm12)>>1))&0xfff, Rn, Rt))
#define LDRSB_U12(Rt, Rn, imm12)          EMIT(LDS_gen(0b00, 0b01, ((uint32_t)(imm12))&0xfff, Rn, Rt))

#define LDR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b01<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define LDRx_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define LDRx_REG_LSL3(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define LDRx_REG_UXTW3(Rt, Rn, Rm)      EMIT(LDR_REG_gen(0b11, Rm, 0b010, 1, Rn, Rt))
#define LDRx_REG_SXTW(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b11, Rm, 0b110, 0, Rn, Rt))
#define LDRw_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define LDRw_REG_LSL2(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define LDRw_REG_SXTW(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b10, Rm, 0b110, 0, Rn, Rt))
#define LDRxw_REG(Rt, Rn, Rm)           EMIT(LDR_REG_gen(0b10+rex.w, Rm, 0b011, 0, Rn, Rt))
#define LDRxw_REG_SXTW(Rt, Rn, Rm)      EMIT(LDR_REG_gen(0b10+rex.w, Rm, 0b110, 0, Rn, Rt))
#define LDRB_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define LDRB_REG_UXTW(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b00, Rm, 0b010, 0, Rn, Rt))
#define LDRB_REG_SXTW(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b00, Rm, 0b110, 0, Rn, Rt))
#define LDRH_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))
#define LDRH_REG_SXTW(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b01, Rm, 0b110, 0, Rn, Rt))

#define LDRS_U12_gen(size, op1, opc, imm12, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSHx_U12(Rt, Rn, imm12)           EMIT(LDRS_U12_gen(0b01, 0b01, 0b10, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHw_U12(Rt, Rn, imm12)           EMIT(LDRS_U12_gen(0b01, 0b01, 0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHxw_U12(Rt, Rn, imm12)          EMIT(LDRS_U12_gen(0b01, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSBx_U12(Rt, Rn, imm12)           EMIT(LDRS_U12_gen(0b00, 0b01, 0b10, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))
#define LDRSBw_U12(Rt, Rn, imm12)           EMIT(LDRS_U12_gen(0b00, 0b01, 0b11, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))
#define LDRSBxw_U12(Rt, Rn, imm12)          EMIT(LDRS_U12_gen(0b00, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))

#define LDRS_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b10<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define LDRSW_REG(Rt, Rn, Rm)           EMIT(LDRS_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define LDRSW_REG_SXTW(Rt, Rn, Rm)      EMIT(LDRS_REG_gen(0b10, Rm, 0b110, 0, Rn, Rt))

#define LDR_PC_gen(opc, imm19, Rt)      ((opc)<<30 | 0b011<<27 | (imm19)<<5 | (Rt))
#define LDRx_literal(Rt, imm21)         EMIT(LDR_PC_gen(0b01, (((int64_t)(imm21))>>2)&0x7FFFF, Rt))

#define LDU_gen(size, opc, imm9, Rn, Rt)  ((size)<<30 | 0b111<<27 | (opc)<<22 | ((imm9)&0x1ff)<<12 | (Rn)<<5 | (Rt))
#define LDURx_I9(Rt, Rn, imm9)            EMIT(LDU_gen(0b11, 0b01, imm9, Rn, Rt))
#define LDURw_I9(Rt, Rn, imm9)            EMIT(LDU_gen(0b10, 0b01, imm9, Rn, Rt))
#define LDURxw_I9(Rt, Rn, imm9)           EMIT(LDU_gen((rex.w)?0b11:0b10, 0b01, imm9, Rn, Rt))
#define LDURz_I9(Rt, Rn, imm9)            EMIT(LDU_gen((rex.is32bits)?0b10:0b11, 0b01, imm9, Rn, Rt))
#define LDURH_I9(Rt, Rn, imm9)            EMIT(LDU_gen(0b01, 0b01, imm9, Rn, Rt))
#define LDURB_I9(Rt, Rn, imm9)            EMIT(LDU_gen(0b00, 0b01, imm9, Rn, Rt))
#define LDURSW_I9(Rt, Rn, imm9)           EMIT(LDU_gen(0b10, 0b10, imm9, Rn, Rt))
#define LDURSHw_I9(Rt, Rn, imm9)          EMIT(LDU_gen(0b01, 0b11, imm9, Rn, Rt))
#define LDURSHx_I9(Rt, Rn, imm9)          EMIT(LDU_gen(0b01, 0b10, imm9, Rn, Rt))
#define LDURSHxw_I9(Rt, Rn, imm9)         EMIT(LDU_gen(0b01, (rex.w)?0b10:0b11, imm9, Rn, Rt))
#define LDURSBw_I9(Rt, Rn, imm9)          EMIT(LDU_gen(0b00, 0b11, imm9, Rn, Rt))
#define LDURSBx_I9(Rt, Rn, imm9)          EMIT(LDU_gen(0b00, 0b10, imm9, Rn, Rt))
#define LDURSBxw_I9(Rt, Rn, imm9)         EMIT(LDU_gen(0b00, (rex.w)?0b10:0b11, imm9, Rn, Rt))

#define LDxw(A, B, C)   if(unscaled) {LDURxw_I9(A, B, C);} else {LDRxw_U12(A, B, C);}
#define LDz(A, B, C)    if(unscaled) {LDURz_I9(A, B, C);} else {LDRz_U12(A, B, C);}
#define LDx(A, B, C)    if(unscaled) {LDURx_I9(A, B, C);} else {LDRx_U12(A, B, C);}
#define LDW(A, B, C)    if(unscaled) {LDURw_I9(A, B, C);} else {LDRw_U12(A, B, C);}
#define LDH(A, B, C)    if(unscaled) {LDURH_I9(A, B, C);} else {LDRH_U12(A, B, C);}
#define LDB(A, B, C)    if(unscaled) {LDURB_I9(A, B, C);} else {LDRB_U12(A, B, C);}
#define LDSW(A, B, C)   if(unscaled) {LDURSW_I9(A, B, C);} else {LDRSW_U12(A, B, C);}
#define LDSHxw(A, B, C) if(unscaled) {LDURSHxw_I9(A, B, C);} else {LDRSHxw_U12(A, B, C);}
#define LDSHx(A, B, C)  if(unscaled) {LDURSHx_I9(A, B, C);} else {LDRSHx_U12(A, B, C);}
#define LDSHw(A, B, C)  if(unscaled) {LDURSHw_I9(A, B, C);} else {LDRSHw_U12(A, B, C);}
#define LDSBxw(A, B, C) if(unscaled) {LDURSBxw_I9(A, B, C);} else {LDRSBxw_U12(A, B, C);}
#define LDSBx(A, B, C)  if(unscaled) {LDURSBx_I9(A, B, C);} else {LDRSBx_U12(A, B, C);}
#define LDSBw(A, B, C)  if(unscaled) {LDURSBw_I9(A, B, C);} else {LDRSBw_U12(A, B, C);}
#define STxw(A, B, C)   if(unscaled) {STURxw_I9(A, B, C);} else {STRxw_U12(A, B, C);}
#define STz(A, B, C)    if(unscaled) {STURz_I9(A, B, C);} else {STRz_U12(A, B, C);}
#define STx(A, B, C)    if(unscaled) {STURx_I9(A, B, C);} else {STRx_U12(A, B, C);}
#define STW(A, B, C)    if(unscaled) {STURw_I9(A, B, C);} else {STRw_U12(A, B, C);}
#define STH(A, B, C)    if(unscaled) {STURH_I9(A, B, C);} else {STRH_U12(A, B, C);}
#define STB(A, B, C)    if(unscaled) {STURB_I9(A, B, C);} else {STRB_U12(A, B, C);}

// STR
#define STR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define STRx_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRx_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRw_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRw_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRxw_S9_postindex(Rt, Rn, imm9)  EMIT(STR_gen(rex.w?0b11:0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRB_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b00, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRH_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b01, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRH_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b01, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))

#define ST_gen(size, op1, imm12, Rn, Rt)        ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define STRx_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b11, 0b01, ((uint32_t)((imm12)>>3))&0xfff, Rn, Rt))
#define STRw_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b10, 0b01, ((uint32_t)((imm12)>>2))&0xfff, Rn, Rt))
#define STRB_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b00, 0b01, ((uint32_t)((imm12)))&0xfff, Rn, Rt))
#define STRH_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b01, 0b01, ((uint32_t)((imm12)>>1))&0xfff, Rn, Rt))
#define STRxw_U12(Rt, Rn, imm12)          EMIT(ST_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)((imm12)>>(2+rex.w)))&0xfff, Rn, Rt))
#define STRz_U12(Rt, Rn, imm12)           EMIT(ST_gen((rex.is32bits)?0b10:0b11, 0b01, ((uint32_t)((imm12)>>(rex.is32bits?2:3)))&0xfff, Rn, Rt))

#define STU_gen(size, opc, imm9, Rn, Rt)  ((size)<<30 | 0b111<<27 | (opc)<<22 | ((imm9)&0x1ff)<<12 | (Rn)<<5 | (Rt))
#define STURx_I9(Rt, Rn, imm9)            EMIT(STU_gen(0b11, 0b00, imm9, Rn, Rt))
#define STURw_I9(Rt, Rn, imm9)            EMIT(STU_gen(0b10, 0b00, imm9, Rn, Rt))
#define STURxw_I9(Rt, Rn, imm9)           EMIT(STU_gen((rex.w)?0b11:0b10, 0b00, imm9, Rn, Rt))
#define STURz_I9(Rt, Rn, imm9)            EMIT(STU_gen((rex.is32bits)?0b10:0b11, 0b00, imm9, Rn, Rt))
#define STURH_I9(Rt, Rn, imm9)            EMIT(STU_gen(0b01, 0b00, imm9, Rn, Rt))
#define STURB_I9(Rt, Rn, imm9)            EMIT(STU_gen(0b00, 0b00, imm9, Rn, Rt))

#define STR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b00<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define STRx_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define STRx_REG_LSL3(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define STRx_REG_UXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b010, 0, Rn, Rt))
#define STRx_REG_SXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b110, 0, Rn, Rt))
#define STRw_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define STRw_REG_LSL2(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define STRw_REG_SXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b10, Rm, 0b110, 0, Rn, Rt))
#define STRB_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define STRB_REG_SXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b00, Rm, 0b110, 0, Rn, Rt))
#define STRB_REGz(Rt, Rn, Rm)           EMIT(STR_REG_gen(0b00, Rm, rex.is32bits?0b110:0b011, 0, Rn, Rt))
#define STRH_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))
#define STRH_REG_SXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b01, Rm, 0b110, 0, Rn, Rt))
#define STRxw_REG(Rt, Rn, Rm)           EMIT(STR_REG_gen(rex.w?0b11:0b10, Rm, 0b011, 0, Rn, Rt))
#define STRxw_REG_SXTW(Rt, Rn, Rm)      EMIT(STR_REG_gen(rex.w?0b11:0b10, Rm, 0b110, 0, Rn, Rt))
#define STRxw_REGz(Rt, Rn, Rm)          EMIT(STR_REG_gen(rex.w?0b11:0b10, Rm, rex.is32bits?0b110:0b011, 0, Rn, Rt))
#define STRz_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(rex.is32bits?0b10:0b11, Rm, 0b011, 0, Rn, Rt))
#define STRz_REG_SXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(rex.is32bits?0b10:0b11, Rm, 0b110, 0, Rn, Rt))
#define STRz_REGz(Rt, Rn, Rm)           EMIT(STR_REG_gen(rex.is32bits?0b10:0b11, Rm, rex.is32bits?0b110:0b011, 0, Rn, Rt))

// LOAD/STORE PAIR
#define MEMPAIR_gen(size, L, op2, imm7, Rt2, Rn, Rt)    ((size)<<31 | 0b101<<27 | (op2)<<23 | (L)<<22 | (imm7)<<15 | (Rt2)<<10 | (Rn)<<5 | (Rt))

#define LDPx_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(1, 1, 0b01, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(0, 1, 0b01, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_postindex(Rt, Rt2, Rn, imm)            EMIT(MEMPAIR_gen(rex.w, 1, 0b01, (((uint32_t)(imm))>>(2+rex.w))&0x7f, Rt2, Rn, Rt))
#define LDPx_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(1, 1, 0b11, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(0, 1, 0b11, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_preindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(rex.w, 1, 0b11, (((uint32_t)(imm))>>(2+rex.w))&0x7f, Rt2, Rn, Rt))
#define LDPx_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(1, 1, 0b10, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(0, 1, 0b10, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_offset(Rt, Rt2, Rn, imm)               EMIT(MEMPAIR_gen(rex.w, 1, 0b10, (((uint32_t)(imm))>>(2+rex.w))&0x7f, Rt2, Rn, Rt))

#define STPx_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(1, 0, 0b01, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(0, 0, 0b01, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_postindex(Rt, Rt2, Rn, imm)            EMIT(MEMPAIR_gen(rex.w, 0, 0b01, (((uint32_t)(imm))>>(2+rex.w))&0x7f, Rt2, Rn, Rt))
#define STPx_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(1, 0, 0b11, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(0, 0, 0b11, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_preindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(rex.w, 0, 0b11, (((uint32_t)(imm))>>(2+rex.w))&0x7f, Rt2, Rn, Rt))
#define STPx_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(1, 0, 0b10, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(0, 0, 0b10, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_offset(Rt, Rt2, Rn, imm)               EMIT(MEMPAIR_gen(rex.w, 0, 0b10, (((uint32_t)(imm))>>(2+rex.w))&0x7f, Rt2, Rn, Rt))

// PUSH / POP helper
#define POP1(reg)       LDRx_S9_postindex(reg, xRSP, 8)
#define PUSH1(reg)      STRx_S9_preindex(reg, xRSP, -8)
#define POP2(reg1, reg2)       LDPx_S7_postindex(reg1, reg2, xRSP, 16)
#define PUSH2(reg1, reg2)      STPx_S7_preindex(reg2, reg1, xRSP, -16)

#define POP1_32(reg)            LDRw_S9_postindex(reg, xRSP, 4)
#define PUSH1_32(reg)           STRw_S9_preindex(reg, xRSP, -4)
#define POP2_32(reg1, reg2)     LDPw_S7_postindex(reg1, reg2, xRSP, 8)
#define PUSH2_32(reg1, reg2)    STPw_S7_preindex(reg2, reg1, xRSP, -8)

#define POP1_16(reg)            LDRH_S9_postindex(reg, xRSP, 2)
#define PUSH1_16(reg)           STRH_S9_preindex(reg, xRSP, -2)

#define POP1z(reg)              if(rex.is32bits) {POP1_32(reg);} else {POP1(reg);}
#define PUSH1z(reg)             if(rex.is32bits) {PUSH1_32(reg);} else {PUSH1(reg);}
#define POP2z(reg1, reg2)       if(rex.is32bits) {POP2_32(reg1, reg2);} else {POP2(reg1, reg2);}
#define PUSH2z(reg1, reg2)      if(rex.is32bits) {PUSH2_32(reg1, reg2);} else {PUSH2(reg1, reg2);}

// LOAD/STORE Acquire Exclusive
#define MEMAX_gen(size, L, Rs, Rn, Rt)      ((size)<<30 | 0b001000<<24 | (L)<<22 | (Rs)<<16 | 1<<15 | 0b11111<<10 | (Rn)<<5 | (Rt))
#define LDAXRB(Rt, Rn)                  EMIT(MEMAX_gen(0b00, 1, 31, Rn, Rt))
#define STLXRB(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b00, 0, Rs, Rn, Rt))
#define LDAXRH(Rt, Rn)                  EMIT(MEMAX_gen(0b01, 1, 31, Rn, Rt))
#define STLXRH(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b01, 0, Rs, Rn, Rt))
#define LDAXRw(Rt, Rn)                  EMIT(MEMAX_gen(0b10, 1, 31, Rn, Rt))
#define STLXRw(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b10, 0, Rs, Rn, Rt))
#define LDAXRx(Rt, Rn)                  EMIT(MEMAX_gen(0b11, 1, 31, Rn, Rt))
#define STLXRx(Rs, Rt, Rn)              EMIT(MEMAX_gen(0b11, 0, Rs, Rn, Rt))
#define LDAXRxw(Rt, Rn)                 EMIT(MEMAX_gen(2+rex.w, 1, 31, Rn, Rt))
#define STLXRxw(Rs, Rt, Rn)             EMIT(MEMAX_gen(2+rex.w, 0, Rs, Rn, Rt))

#define MEMAX_pair(size, L, Rs, Rt2, Rn, Rt)    (1<<31 | (size)<<30 | 0b001000<<24 | (L)<<22 | 1<<21 | (Rs)<<16 | 1<<15 | (Rt2)<<10 | (Rn)<<5 | (Rt))
#define LDAXPx(Rt, Rt2, Rn)             EMIT(MEMAX_pair(1, 1, 31, Rt2, Rn, Rt))
#define LDAXPw(Rt, Rt2, Rn)             EMIT(MEMAX_pair(0, 1, 31, Rt2, Rn, Rt))
#define LDAXPxw(Rt, Rt2, Rn)            EMIT(MEMAX_pair(rex.w, 1, 31, Rt2, Rn, Rt))
#define STLXPx(Rs, Rt, Rt2, Rn)         EMIT(MEMAX_pair(1, 0, Rs, Rt2, Rn, Rt))
#define STLXPw(Rs, Rt, Rt2, Rn)         EMIT(MEMAX_pair(0, 0, Rs, Rt2, Rn, Rt))
#define STLXPxw(Rs, Rt, Rt2, Rn)        EMIT(MEMAX_pair(rex.w, 0, Rs, Rt2, Rn, Rt))

// LOAD/STORE Exclusive
#define MEMX_gen(size, L, Rs, Rn, Rt)       ((size)<<30 | 0b001000<<24 | (L)<<22 | (Rs)<<16 | 0<<15 | 0b11111<<10 | (Rn)<<5 | (Rt))
#define LDXRB(Rt, Rn)                   EMIT(MEMX_gen(0b00, 1, 31, Rn, Rt))
#define STXRB(Rs, Rt, Rn)               EMIT(MEMX_gen(0b00, 0, Rs, Rn, Rt))
#define LDXRH(Rt, Rn)                   EMIT(MEMX_gen(0b01, 1, 31, Rn, Rt))
#define STXRH(Rs, Rt, Rn)               EMIT(MEMX_gen(0b01, 0, Rs, Rn, Rt))
#define LDXRw(Rt, Rn)                   EMIT(MEMX_gen(0b10, 1, 31, Rn, Rt))
#define STXRw(Rs, Rt, Rn)               EMIT(MEMX_gen(0b10, 0, Rs, Rn, Rt))
#define LDXRx(Rt, Rn)                   EMIT(MEMX_gen(0b11, 1, 31, Rn, Rt))
#define STXRx(Rs, Rt, Rn)               EMIT(MEMX_gen(0b11, 0, Rs, Rn, Rt))
#define LDXRxw(Rt, Rn)                  EMIT(MEMX_gen(2+rex.w, 1, 31, Rn, Rt))
#define STXRxw(Rs, Rt, Rn)              EMIT(MEMX_gen(2+rex.w, 0, Rs, Rn, Rt))

// Prefetch
#define PRFM_register(Rm, option, S, Rn, Rt)    (0b11<<30 | 0b111<<27 | 0b10<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | 0b10<<10 | (Rn)<<5 | (Rt))
#define PLD_L1_KEEP(Rn, Rm)             EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00000))
#define PLD_L2_KEEP(Rn, Rm)             EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00010))
#define PLD_L3_KEEP(Rn, Rm)             EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00100))
#define PLD_L1_STREAM(Rn, Rm)           EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00001))
#define PLD_L2_STREAM(Rn, Rm)           EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00011))
#define PLD_L3_STREAM(Rn, Rm)           EMIT(PRFM_register(Rm, 0b011, 0, Rn, 0b00101))

#define PRFM_imm(imm12, Rn, Rt)         (0b11<<30 | 0b111<<27 | 0b01<<24 | 0b10<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define PLD_L1_KEEP_U12(Rn, imm12)      EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00000))
#define PLD_L2_KEEP_U12(Rn, imm12)      EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00010))
#define PLD_L3_KEEP_U12(Rn, imm12)      EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00100))
#define PLD_L1_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00001))
#define PLD_L2_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00011))
#define PLD_L3_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b00101))

#define PST_L1_STREAM_U12(Rn, imm12)    EMIT(PRFM_imm(((imm12)>>3)&0xfff, Rn, 0b01001))

// Data Memory Barrier
#define DMB_gen(CRm)                    (0b1101010100<<22 | 0b011<<16 | 0b0011<<12 | (CRm)<<8 | 1<<7 | 0b01<<5 | 0b11111)
#define DMB_ISH()                       EMIT(DMB_gen(0b1011))
#define DMB_ISHLD()                     EMIT(DMB_gen(0b1001))
#define DMB_ISHST()                     EMIT(DMB_gen(0b1010))
#define DMB_LD()                        EMIT(DMB_gen(0b1101))
#define DMB_ST()                        EMIT(DMB_gen(0b1110))
#define DMB_SY()                        EMIT(DMB_gen(0b1111))

// Data Synchronization Barrier
#define DSB_gen(CRm)                    (0b1101010100<<22 | 0b011<<16 | 0b0011<<12 | (CRm)<<8 | 1<<7 | 0b00<<5 | 0b11111)
#define DSB_ISH()                       EMIT(DSB_gen(0b1011))
#define DSB_ISHLD()                     EMIT(DSB_gen(0b1001))
#define DSB_ISHST()                     EMIT(DSB_gen(0b1010))
#define DSB_LD()                        EMIT(DSB_gen(0b1101))
#define DSB_ST()                        EMIT(DSB_gen(0b1110))
#define DSB_SY()                        EMIT(DSB_gen(0b1111))

// Break
#define BRK_gen(imm16)                  (0b11010100<<24 | 0b001<<21 | (((imm16)&0xffff)<<5))
#define BRK(imm16)                      EMIT(BRK_gen(imm16))

// BR and Branches
#define BR_gen(Z, op, A, M, Rn, Rm)       (0b1101011<<25 | (Z)<<24 | (op)<<21 | 0b11111<<16 | (A)<<11 | (M)<<10 | (Rn)<<5 | (Rm))
#define BR(Rn)                            EMIT(BR_gen(0, 0b00, 0, 0, Rn, 0))
#define BLR(Rn)                           EMIT(BR_gen(0, 0b01, 0, 0, Rn, 0))
#define RET(Rn)                           EMIT(BR_gen(0, 0b10, 0, 0, Rn, 0))

#define CB_gen(sf, op, imm19, Rt)       ((sf)<<31 | 0b011010<<25 | (op)<<24 | (imm19)<<5 | (Rt))
#define CBNZx(Rt, imm19)                EMIT(CB_gen(1, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZw(Rt, imm19)                EMIT(CB_gen(0, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZxw(Rt, imm19)               EMIT(CB_gen(rex.w, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZy(Rt, imm19)                EMIT(CB_gen((rex.is32bits || rex.is67)?0:1, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZx(Rt, imm19)                 EMIT(CB_gen(1, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZw(Rt, imm19)                 EMIT(CB_gen(0, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZxw(Rt, imm19)                EMIT(CB_gen(rex.w, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZy(Rt, imm19)                 EMIT(CB_gen((rex.is32bits || rex.is67)?0:1, 0, ((imm19)>>2)&0x7FFFF, Rt))

#define TB_gen(b5, op, b40, imm14, Rt)  ((b5)<<31 | 0b011011<<25 | (op)<<24  | (b40)<<19 | (imm14)<<5 | (Rt))
#define TBZ(Rt, bit, imm16)             EMIT(TB_gen(((bit)>>5)&1, 0, (bit)&0x1f, ((imm16)>>2)&0x3FFF, Rt))
#define TBNZ(Rt, bit, imm16)            EMIT(TB_gen(((bit)>>5)&1, 1, (bit)&0x1f, ((imm16)>>2)&0x3FFF, Rt))

#define Bcond_gen(imm19, cond)          (0b0101010<<25 | (imm19)<<5 | (cond))
#define Bcond(cond, imm19)              EMIT(Bcond_gen(((imm19)>>2)&0x7FFFF, cond))

#define B_gen(imm26)                    (0b000101<<26 | (imm26))
#define B(imm26)                        EMIT(B_gen(((imm26)>>2)&0x3ffffff))

#define BL_gen(imm26)                   (0b100101<<26 | (imm26))
#define BL(imm26)                       EMIT(BL_gen(((imm26)>>2)&0x3ffffff))

#define NOP                             EMIT(0b11010101000000110010000000011111)
#define WFE                             EMIT(0b11010101000000110010000001011111)
#define WFI                             EMIT(0b11010101000000110010000001111111)
#define YIELD                           EMIT(0b11010101000000110010000000111111)
#define SEVL                            EMIT(0b11010101000000110010000010111111)

#define CSINC_gen(sf, Rm, cond, Rn, Rd)     ((sf)<<31 | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define CSINCx(Rd, Rn, Rm, cond)            EMIT(CSINC_gen(1, Rm, cond, Rn, Rd))
#define CSINCw(Rd, Rn, Rm, cond)            EMIT(CSINC_gen(0, Rm, cond, Rn, Rd))
#define CSINCxw(Rd, Rn, Rm, cond)           EMIT(CSINC_gen(rex.w, Rm, cond, Rn, Rd))
#define CSETx(Rd, cond)                     CSINCx(Rd, xZR, xZR, invCond(cond))
#define CSETw(Rd, cond)                     CSINCw(Rd, xZR, xZR, invCond(cond))
#define CSETxw(Rd, cond)                    CSINCxw(Rd, xZR, xZR, invCond(cond))

#define CSINV_gen(sf, Rm, cond, Rn, Rd)     ((sf)<<31 | 1<<30 | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | (Rn)<<5 | (Rd))
#define CSINVx(Rd, Rn, Rm, cond)            EMIT(CSINV_gen(1, Rm, cond, Rn, Rd))
#define CSINVw(Rd, Rn, Rm, cond)            EMIT(CSINV_gen(0, Rm, cond, Rn, Rd))
#define CSINVxw(Rd, Rn, Rm, cond)           EMIT(CSINV_gen(rex.w, Rm, cond, Rn, Rd))
#define CINVx(Rd, Rn, cond)                 CSINVx(Rd, Rn, Rn, invertCond(cond))
#define CINVw(Rd, Rn, cond)                 CSINVw(Rd, Rn, Rn, invertCond(cond))
#define CINVxw(Rd, Rn, cond)                CSINVxw(Rd, Rn, Rn, invertCond(cond))
#define CSETMx(Rd, cond)                    CSINVx(Rd, xZR, xZR, invCond(cond))
#define CSETMw(Rd, cond)                    CSINVw(Rd, xZR, xZR, invCond(cond))
#define CSETMxw(Rd, cond)                   CSINVxw(Rd, xZR, xZR, invCond(cond))

#define CSEL_gen(sf, Rm, cond, Rn, Rd)      ((sf<<31) | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | (Rn)<<5 | Rd)
#define CSELx(Rd, Rn, Rm, cond)             EMIT(CSEL_gen(1, Rm, cond, Rn, Rd))
#define CSELw(Rd, Rn, Rm, cond)             EMIT(CSEL_gen(0, Rm, cond, Rn, Rd))
#define CSELxw(Rd, Rn, Rm, cond)            EMIT(CSEL_gen(rex.w, Rm, cond, Rn, Rd))

#define CSNEG_gen(sf, Rm, cond, Rn, Rd)     ((sf)<<31 | 1<<30 | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define CSNEGx(Rd, Rn, Rm, cond)            EMIT(CSNEG_gen(1, Rm, cond, Rn, Rd))
#define CSNEGw(Rd, Rn, Rm, cond)            EMIT(CSNEG_gen(0, Rm, cond, Rn, Rd))
#define CSNEGxw(Rd, Rn, Rm, cond)           EMIT(CSNEG_gen(rex.w, Rm, cond, Rn, Rd))
#define CNEGx(Rd, Rn, cond)                 CSNEGx(Rn, Rn, Rd, invCond(cond))
#define CNEGw(Rd, Rn, cond)                 CSNEGw(Rn, Rn, Rd, invCond(cond))
#define CNEGxw(Rd, Rn, cond)                CSNEGxw(Rn, Rn, Rd, invCond(cond))

// AND / ORR
#define LOGIC_gen(sf, opc, N, immr, imms, Rn, Rd)  ((sf)<<31 | (opc)<<29 | 0b100100<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | Rd)
// logic to get the mask is ... convoluted... list of possible value there: https://gist.github.com/dinfuehr/51a01ac58c0b23e4de9aac313ed6a06a
#define ANDx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b00, N, immr, imms, Rn, Rd))
#define ANDw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b00, 0, immr, imms, Rn, Rd))
#define ANDxw_mask(Rd, Rn, N, immr, imms)   EMIT(LOGIC_gen(rex.w, 0b00, rex.w?(N):0, immr, imms, Rn, Rd))
#define ANDSx_mask(Rd, Rn, N, immr, imms)  FEMIT(LOGIC_gen(1, 0b11, N, immr, imms, Rn, Rd))
#define ANDSw_mask(Rd, Rn, immr, imms)     FEMIT(LOGIC_gen(0, 0b11, 0, immr, imms, Rn, Rd))
#define ANDSxw_mask(Rd, Rn, N, immr, imms) FEMIT(LOGIC_gen(rex.w, 0b11, rex.w?(N):0, immr, imms, Rn, Rd))
#define ORRx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b01, N, immr, imms, Rn, Rd))
#define ORRw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b01, 0, immr, imms, Rn, Rd))
#define ORRxw_mask(Rd, Rn, N, immr, imms)   EMIT(LOGIC_gen(rex.w, 0b01, rex.w?(N):0, immr, imms, Rn, Rd))
#define EORx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b10, N, immr, imms, Rn, Rd))
#define EORw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b10, 0, immr, imms, Rn, Rd))
#define EORxw_mask(Rd, Rn, N, immr, imms)   EMIT(LOGIC_gen(rex.w, 0b10, rex.w?(N):0, immr, imms, Rn, Rd))
#define TSTx_mask(Rn, N, immr, imms)        ANDSx_mask(xZR, Rn, N, immr, imms)
#define TSTw_mask(Rn, immr, imms)           ANDSw_mask(wZR, Rn, immr, imms)
#define TSTxw_mask(Rn, N, immr, imms)       ANDSxw_mask(xZR, Rn, N, immr, imms)

#define LOGIC_REG_gen(sf, opc, shift, N, Rm, imm6, Rn, Rd)    ((sf)<<31 | (opc)<<29 | 0b01010<<24 | (shift)<<22 | (N)<<21 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ANDx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b00, 0b01, 0, Rm, lsr, Rn, Rd))
#define ANDxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSx_REG(Rd, Rn, Rm)          FEMIT(LOGIC_REG_gen(1, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSw_REG(Rd, Rn, Rm)          FEMIT(LOGIC_REG_gen(0, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSxw_REG(Rd, Rn, Rm)         FEMIT(LOGIC_REG_gen(rex.w, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSw_REG_LSL(Rd, Rn, Rm, lsl) FEMIT(LOGIC_REG_gen(0, 0b11, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRx_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(1, 0b01, 0b01, 0, Rm, lsr, Rn, Rd))
#define ORRw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b01, 0b01, 0, Rm, lsr, Rn, Rd))
#define ORRxw_REG_LSR(Rd, Rn, Rm, lsr)  EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b01, 0, Rm, lsr, Rn, Rd))
#define ORRxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORNx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 1, Rm, lsl, Rn, Rd))
#define EORx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORx_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(1, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EORw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EORxw_REG_LSR(Rd, Rn, Rm, lsr)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EONx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 1, Rm, 0, Rn, Rd))
#define EONw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 1, Rm, 0, Rn, Rd))
#define EONxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 1, Rm, 0, Rn, Rd))
#define EONx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 1, Rm, lsl, Rn, Rd))
#define EONw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 1, Rm, lsl, Rn, Rd))
#define EONxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 1, Rm, lsl, Rn, Rd))
#define EONx_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(1, 0b10, 0b01, 1, Rm, lsr, Rn, Rd))
#define EONw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b10, 0b01, 1, Rm, lsr, Rn, Rd))
#define EONxw_REG_LSR(Rd, Rn, Rm, lsr)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b01, 1, Rm, lsr, Rn, Rd))
#define MOVx_REG(Rd, Rm)                ORRx_REG(Rd, xZR, Rm)
#define MOVw_REG(Rd, Rm)                ORRw_REG(Rd, xZR, Rm)
#define MOVxw_REG(Rd, Rm)               ORRxw_REG(Rd, xZR, Rm)
#define MOVz_REG(Rd, Rm)                if(rex.is32bits) {MOVw_REG(Rd, Rm);} else {MOVx_REG(Rd, Rm);}
#define LSLw_IMM(Rd, Rm, lsl)           ORRw_REG_LSL(Rd, xZR, Rm, lsl)
#define LSLx_IMM(Rd, Rm, lsl)           ORRx_REG_LSL(Rd, xZR, Rm, lsl)
#define LSLxw_IMM(Rd, Rm, lsl)          ORRxw_REG_LSL(Rd, xZR, Rm, lsl)
#define LSRw_IMM(Rd, Rm, lsr)           ORRw_REG_LSR(Rd, xZR, Rm, lsr)
#define LSRx_IMM(Rd, Rm, lsr)           ORRx_REG_LSR(Rd, xZR, Rm, lsr)
#define LSRxw_IMM(Rd, Rm, lsr)          ORRxw_REG_LSR(Rd, xZR, Rm, lsr)
#define MVNx_REG(Rd, Rm)                ORNx_REG(Rd, xZR, Rm)
#define MVNx_REG_LSL(Rd, Rm, lsl)       ORNx_REG_LSL(Rd, xZR, Rm, lsl)
#define MVNw_REG(Rd, Rm)                ORNw_REG(Rd, xZR, Rm)
#define MVNxw_REG(Rd, Rm)               ORNxw_REG(Rd, xZR, Rm)
#define MOV_frmSP(Rd)                   ADDx_U12(Rd, xSP, 0)
#define MOV_toSP(Rm)                    ADDx_U12(xSP, Rm, 0)
#define BICx(Rd, Rn, Rm)                EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICw(Rd, Rn, Rm)                EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICw_LSL(Rd, Rn, Rm, lsl)       EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, lsl, Rn, Rd))
#define BICSx(Rd, Rn, Rm)              FEMIT(LOGIC_REG_gen(1, 0b11, 0b00, 1, Rm, 0, Rn, Rd))
#define BICSw(Rd, Rn, Rm)              FEMIT(LOGIC_REG_gen(0, 0b11, 0b00, 1, Rm, 0, Rn, Rd))
#define BICxw(Rd, Rn, Rm)              FEMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICSxw(Rd, Rn, Rm)             FEMIT(LOGIC_REG_gen(rex.w, 0b11, 0b00, 1, Rm, 0, Rn, Rd))
#define BICx_REG    BICx
#define BICw_REG    BICw
#define BICxw_REG   BICxw
#define BICx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 1, Rm, lsl, Rn, Rd))
#define BICw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, lsl, Rn, Rd))
#define BICxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 1, Rm, lsl, Rn, Rd))
#define BICx_REG_LSR(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b00, 0b01, 1, Rm, lsl, Rn, Rd))
#define BICw_REG_LSR(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b00, 0b01, 1, Rm, lsl, Rn, Rd))
#define BICxw_REG_LSR(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b01, 1, Rm, lsl, Rn, Rd))
#define TSTx_REG(Rn, Rm)                ANDSx_REG(xZR, Rn, Rm)
#define TSTw_REG(Rn, Rm)                ANDSw_REG(wZR, Rn, Rm)
#define TSTxw_REG(Rn, Rm)               ANDSxw_REG(xZR, Rn, Rm)

// ASRV
#define ASRV_gen(sf, Rm, Rn, Rd)        ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define ASRx_REG(Rd, Rn, Rm)            EMIT(ASRV_gen(1, Rm, Rn, Rd))
#define ASRw_REG(Rd, Rn, Rm)            EMIT(ASRV_gen(0, Rm, Rn, Rd))
#define ASRxw_REG(Rd, Rn, Rm)           EMIT(ASRV_gen(rex.w, Rm, Rn, Rd))

// BFI
#define BFM_gen(sf, opc, N, immr, imms, Rn, Rd) ((sf)<<31 | (opc)<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define BFMx(Rd, Rn, immr, imms)        EMIT(BFM_gen(1, 0b01, 1, immr, imms, Rn, Rd))
#define BFMw(Rd, Rn, immr, imms)        EMIT(BFM_gen(0, 0b01, 0, immr, imms, Rn, Rd))
#define BFMxw(Rd, Rn, immr, imms)       EMIT(BFM_gen(rex.w, 0b01, rex.w, immr, imms, Rn, Rd))
#define BFIx(Rd, Rn, lsb, width)        BFMx(Rd, Rn, ((-(lsb))%64)&0x3f, (width)-1)
#define BFIw(Rd, Rn, lsb, width)        BFMw(Rd, Rn, ((-(lsb))%32)&0x1f, (width)-1)
#define BFIxw(Rd, Rn, lsb, width)       if(rex.w) {BFIx(Rd, Rn, lsb, width);} else {BFIw(Rd, Rn, lsb, width);}
#define BFIz(Rd, Rn, lsb, width)        if(rex.is32bits) {BFIw(Rd, Rn, lsb, width);} else {BFIx(Rd, Rn, lsb, width);}
#define BFCx(Rd, lsb, width)            BFMx(Rd, xZR, ((-(lsb))%64)&0x3f, (width)-1)
#define BFCw(Rd, lsb, width)            BFMw(Rd, xZR, ((-(lsb))%32)&0x1f, (width)-1)
#define BFCxw(Rd, lsb, width)           BFMxw(Rd, xZR, rex.w?(((-(lsb))%64)&0x3f):(((-(lsb))%32)&0x1f), (width)-1)
#define BFCz(Rd, lsb, width)            if(rex.is32bits) {BFCw(Rd, lsb, width);} else {BFCx(Rd, lsb, width);}
// Insert lsb:width part of Rn into low part of Rd (leaving rest of Rd untouched)
#define BFXILx(Rd, Rn, lsb, width)      EMIT(BFM_gen(1, 0b01, 1, (lsb), (lsb)+(width)-1, Rn, Rd))
// Insert lsb:width part of Rn into low part of Rd (leaving rest of Rd untouched)
#define BFXILw(Rd, Rn, lsb, width)      EMIT(BFM_gen(0, 0b01, 0, (lsb), (lsb)+(width)-1, Rn, Rd))
// Insert lsb:width part of Rn into low part of Rd (leaving rest of Rd untouched)
#define BFXILxw(Rd, Rn, lsb, width)     EMIT(BFM_gen(rex.w, 0b01, rex.w, (lsb), (lsb)+(width)-1, Rn, Rd))

// UBFX
#define UBFM_gen(sf, N, immr, imms, Rn, Rd)    ((sf)<<31 | 0b10<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define UBFMx(Rd, Rn, immr, imms)       EMIT(UBFM_gen(1, 1, immr, imms, Rn, Rd))
#define UBFMw(Rd, Rn, immr, imms)       EMIT(UBFM_gen(0, 0, immr, imms, Rn, Rd))
#define UBFMxw(Rd, Rn, immr, imms)      EMIT(UBFM_gen(rex.w, rex.w, immr, imms, Rn, Rd))
#define UBFXx(Rd, Rn, lsb, width)       EMIT(UBFM_gen(1, 1, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UBFXw(Rd, Rn, lsb, width)       EMIT(UBFM_gen(0, 0, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UBFXxw(Rd, Rn, lsb, width)      EMIT(UBFM_gen(rex.w, rex.w, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UXTBx(Rd, Rn)                   EMIT(UBFM_gen(1, 1, 0, 7, Rn, Rd))
#define UXTBw(Rd, Rn)                   EMIT(UBFM_gen(0, 0, 0, 7, Rn, Rd))
#define UXTBxw(Rd, Rn)                  EMIT(UBFM_gen(rex.w, rex.w, 0, 7, Rn, Rd))
#define UXTHx(Rd, Rn)                   EMIT(UBFM_gen(1, 1, 0, 15, Rn, Rd))
#define UXTHw(Rd, Rn)                   EMIT(UBFM_gen(0, 0, 0, 15, Rn, Rd))
#define LSRx(Rd, Rn, shift)             EMIT(UBFM_gen(1, 1, shift, 63, Rn, Rd))
#define LSRw(Rd, Rn, shift)             EMIT(UBFM_gen(0, 0, shift, 31, Rn, Rd))
#define LSRxw(Rd, Rn, shift)            EMIT(UBFM_gen(rex.w, rex.w, shift, (rex.w)?63:31, Rn, Rd))
#define LSLx(Rd, Rn, lsl)               UBFMx(Rd, Rn, ((-(lsl))%64)&63, 63-(lsl))
#define LSLw(Rd, Rn, lsl)               UBFMw(Rd, Rn, ((-(lsl))%32)&31, 31-(lsl))
#define LSLxw(Rd, Rn, lsl)              UBFMxw(Rd, Rn, rex.w?(((-(lsl))%64)&63):(((-(lsl))%32)&31), (rex.w?63:31)-(lsl))
#define LSLy(Rd, Rn, lsl)               do {if(rex.is32bits || rex.is67) LSLw(Rd, Rn, lsl); else LSLx(Rd, Rn, lsl);} while(0)
// Take width first bits from Rn, LSL lsb and create Rd
#define UBFIZx(Rd, Rn, lsb, width)      UBFMx(Rd, Rn, ((-(lsb))%64)&63, width-1)
// Take width first bits from Rn, LSL lsb and create Rd
#define UBFIZw(Rd, Rn, lsb, width)      UBFMw(Rd, Rn, ((-(lsb))%32)&31, width-1)
// Take width first bits from Rn, LSL lsb and create Rd
#define UBFIZxw(Rd, Rn, lsb, width)     UBFMxw(Rd, Rn, rex.w?(((-(lsb))%64)&63):(((-(lsb))%32)&31), width-1)

// SBFM
#define SBFM_gen(sf, N, immr, imms, Rn, Rd)    ((sf)<<31 | 0b00<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define SBFMx(Rd, Rn, immr, imms)       EMIT(SBFM_gen(1, 1, immr, imms, Rn, Rd))
#define SBFMw(Rd, Rn, immr, imms)       EMIT(SBFM_gen(0, 0, immr, imms, Rn, Rd))
#define SBFMxw(Rd, Rn, immr, imms)      EMIT(SBFM_gen(rex.w, rex.w, immr, imms, Rn, Rd))
#define SBFXx(Rd, Rn, lsb, width)       SBFMx(Rd, Rn, lsb, (lsb)+(width)-1)
#define SBFXw(Rd, Rn, lsb, width)       SBFMw(Rd, Rn, lsb, (lsb)+(width)-1)
#define SBFXxw(Rd, Rn, lsb, width)      SBFMxw(Rd, Rn, lsb, (lsb)+(width)-1)
#define SXTBx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 7)
#define SXTBw(Rd, Rn)                   SBFMw(Rd, Rn, 0, 7)
#define SXTHx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 15)
#define SXTHw(Rd, Rn)                   SBFMw(Rd, Rn, 0, 15)
#define SXTHxw(Rd, Rn)                  SBFMxw(Rd, Rn, 0, 15)
#define SXTWx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 31)
#define ASRx(Rd, Rn, shift)             SBFMx(Rd, Rn, shift, 63)
#define ASRw(Rd, Rn, shift)             SBFMw(Rd, Rn, shift, 31)
#define ASRxw(Rd, Rn, shift)            SBFMxw(Rd, Rn, shift, rex.w?63:31)
#define SBFIZx(Rd, Rn, lsb, width)      SFBFMx(Rd, Rn, ((-(lsb))%64), (width)-1)
#define SBFIZw(Rd, Rn, lsb, width)      SFBFMw(Rd, Rn, ((-(lsb))%32), (width)-1)
#define SBFIZxw(Rd, Rn, lsb, width)     SFBFMxw(Rd, Rn, ((-(lsb))%(rex.w?64:32)), (width)-1)

// EXTR
#define EXTR_gen(sf, N, Rm, imms, Rn, Rd)   ((sf)<<31 | 0b00<<29 | 0b100111<<23 | (N)<<22 | (Rm)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define EXTRx(Rd, Rn, Rm, lsb)          EMIT(EXTR_gen(1, 1, Rm, lsb, Rn, Rd))
#define EXTRw(Rd, Rn, Rm, lsb)          EMIT(EXTR_gen(0, 0, Rm, lsb, Rn, Rd))
#define EXTRxw(Rd, Rn, Rm, lsb)         EMIT(EXTR_gen(rex.w, rex.w, Rm, lsb, Rn, Rd))
#define RORx(Rd, Rn, lsb)               EMIT(EXTR_gen(1, 1, Rn, lsb, Rn, Rd))
#define RORw(Rd, Rn, lsb)               EMIT(EXTR_gen(0, 0, Rn, lsb, Rn, Rd))
#define RORxw(Rd, Rn, lsb)              EMIT(EXTR_gen(rex.w, rex.w, Rn, lsb, Rn, Rd))

// RORV
#define RORV_gen(sf, Rm, Rn, Rd)        ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | 0b11<<10 | (Rn)<<5 | (Rd))
#define RORx_REG(Rd, Rn, Rm)            EMIT(RORV_gen(1, Rm, Rn, Rd))
#define RORw_REG(Rd, Rn, Rm)            EMIT(RORV_gen(0, Rm, Rn, Rd))
#define RORxw_REG(Rd, Rn, Rm)           EMIT(RORV_gen(rex.w, Rm, Rn, Rd))


// LSRV / LSLV
#define LS_V_gen(sf, Rm, op2, Rn, Rd)   ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | (op2)<<10 | (Rn)<<5 | (Rd))
#define LSRx_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(1, Rm, 0b01, Rn, Rd))
#define LSRw_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(0, Rm, 0b01, Rn, Rd))
#define LSRxw_REG(Rd, Rn, Rm)           EMIT(LS_V_gen(rex.w, Rm, 0b01, Rn, Rd))

#define LSLx_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(1, Rm, 0b00, Rn, Rd))
#define LSLw_REG(Rd, Rn, Rm)            EMIT(LS_V_gen(0, Rm, 0b00, Rn, Rd))
#define LSLxw_REG(Rd, Rn, Rm)           EMIT(LS_V_gen(rex.w, Rm, 0b00, Rn, Rd))

// UMULL / SMULL
#define MADDL_gen(U, Rm, o0, Ra, Rn, Rd)    (1<<31 | 0b11011<<24 | (U)<<23 | 0b01<<21 | (Rm)<<16 | (o0)<<15 | (Ra)<<10 | (Rn)<<5 | (Rd))
#define UMADDL(Xd, Wn, Wm, Xa)          EMIT(MADDL_gen(1, Wm, 0, Xa, Wn, Xd))
#define UMULL(Xd, Wn, Wm)               UMADDL(Xd, Wn, Wm, xZR)
#define SMADDL(Xd, Wn, Wm, Xa)          EMIT(MADDL_gen(0, Wm, 0, Xa, Wn, Xd))
#define SMULL(Xd, Wn, Wm)               SMADDL(Xd, Wn, Wm, xZR)

#define MULH_gen(U, Rm, Rn, Rd)         (1<<31 | 0b11011<<24 | (U)<<23 | 0b10<<21 | (Rm)<<16 | 0b11111<<10 | (Rn)<<5 | (Rd))
#define UMULH(Xd, Xn, Xm)               EMIT(MULH_gen(1, Xm, Xn, Xd))
#define SMULH(Xd, Xn, Xm)               EMIT(MULH_gen(0, Xm, Xn, Xd))

#define MADD_gen(sf, Rm, o0, Ra, Rn, Rd)    ((sf)<<31 | 0b11011<<24 | (Rm)<<16 | (o0)<<15 | (Ra)<<10 | (Rn)<<5 | (Rd))
#define MADDx(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(1, Rm, 0, Ra, Rn, Rd))
#define MADDw(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(0, Rm, 0, Ra, Rn, Rd))
#define MADDxw(Rd, Rn, Rm, Ra)          EMIT(MADD_gen(rex.w, Rm, 0, Ra, Rn, Rd))
#define MULx(Rd, Rn, Rm)                MADDx(Rd, Rn, Rm, xZR)
#define MULw(Rd, Rn, Rm)                MADDw(Rd, Rn, Rm, xZR)
#define MULxw(Rd, Rn, Rm)               MADDxw(Rd, Rn, Rm, xZR)
#define MSUBx(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(1, Rm, 1, Ra, Rn, Rd))
#define MSUBw(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(0, Rm, 1, Ra, Rn, Rd))
#define MSUBxw(Rd, Rn, Rm, Ra)          EMIT(MADD_gen(rex.w, Rm, 1, Ra, Rn, Rd))
#define MNEGx(Rd, Rn, Rm)               EMIT(MADD_gen(1, Rm, 1, xZR, Rn, Rd))
#define MNEGw(Rd, Rn, Rm)               EMIT(MADD_gen(0, Rm, 1, xZR, Rn, Rd))
#define MNEGxw(Rd, Rn, Rm)              EMIT(MADD_gen(rex.w, Rm, 1, xZR, Rn, Rd))


// DIV
#define DIV_gen(sf, Rm, o1, Rn, Rd)     ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b00001<<11 | (o1)<<10 | (Rn)<<5 | (Rd))
#define UDIVw(Wd, Wn, Wm)               EMIT(DIV_gen(0, Wm, 0, Wn, Wd))
#define UDIVx(Xd, Xn, Xm)               EMIT(DIV_gen(1, Xm, 0, Xn, Xd))
#define SDIVw(Wd, Wn, Wm)               EMIT(DIV_gen(0, Wm, 1, Wn, Wd))
#define SDIVx(Xd, Xn, Xm)               EMIT(DIV_gen(1, Xm, 1, Xn, Xd))

// CLZ
#define CL_gen(sf, op, Rn, Rd)          ((sf)<<31 | 1<<30 | 0b11010110<<21 | 0b00010<<11 | (op)<<10 | (Rn)<<5 | (Rd))
#define CLZx(Rd, Rn)                    EMIT(CL_gen(1, 0, Rn, Rd))
#define CLZw(Rd, Rn)                    EMIT(CL_gen(0, 0, Rn, Rd))
#define CLZxw(Rd, Rn)                   EMIT(CL_gen(rex.w, 0, Rn, Rd))
#define CLSx(Rd, Rn)                    EMIT(CL_gen(1, 1, Rn, Rd))
#define CLSw(Rd, Rn)                    EMIT(CL_gen(0, 1, Rn, Rd))
#define CLSxw(Rd, Rn)                   EMIT(CL_gen(rex.w, 1, Rn, Rd))

// RBIT
#define RBIT_gen(sf, Rn, Rd)            ((sf)<<31 | 1<<30 | 0b11010110<<21 | (Rn)<<5 | (Rd))
#define RBITx(Rd, Rn)                   EMIT(RBIT_gen(1, Rn, Rd))
#define RBITw(Rd, Rn)                   EMIT(RBIT_gen(0, Rn, Rd))
#define RBITxw(Rd, Rn)                  EMIT(RBIT_gen(rex.w, Rn, Rd))

// REV
#define REV_gen(sf, opc, Rn, Rd)        ((sf)<<31 | 1<<30 | 0b11010110<<21 | (opc)<<10 | (Rn)<<5 | (Rd))
#define REV64x(Rd, Rn)                  EMIT(REV_gen(1, 0b11, Rn, Rd))
#define REV32w(Rd, Rn)                  EMIT(REV_gen(0, 0b10, Rn, Rd))
#define REVxw(Rd, Rn)                   EMIT(REV_gen(rex.w, 0b10|rex.w, Rn, Rd))
#define REV16w(Rd, Rn)                  EMIT(REV_gen(0, 0b01, Rn, Rd))
#define REV16x(Rd, Rn)                  EMIT(REV_gen(1, 0b01, Rn, Rd))

// UDF
#define UDF_gen(imm16)                  ((imm16)&0xffff)
#define UDF(imm16)                      EMIT(UDF_gen(imm16))

// MRS
#define MRS_gen(L, o0, op1, CRn, CRm, op2, Rt)  (0b1101010100<<22 | (L)<<21 | 1<<20 | (o0)<<19 | (op1)<<16 | (CRn)<<12 | (CRm)<<8 | (op2)<<5 | (Rt))
// mrs    x0, nzcv : 1101010100 1 1 1 011 0100 0010 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0010(2) op2=0
// MRS : from System register
#define MRS_nzcv(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 2, 0, Rt))
// MSR : to System register
#define MSR_nzcv(Rt)                   FEMIT(MRS_gen(0, 1, 3, 4, 2, 0, Rt))
// mrs    x0, fpcr : 1101010100 1 1 1 011 0100 0100 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=0
#define MRS_fpcr(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 4, 0, Rt))
#define MSR_fpcr(Rt)                    EMIT(MRS_gen(0, 1, 3, 4, 4, 0, Rt))
// mrs    x0, fpsr : 1101010100 1 1 1 011 0100 0100 001 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=1
#define MRS_fpsr(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 4, 1, Rt))
#define MSR_fpsr(Rt)                    EMIT(MRS_gen(0, 1, 3, 4, 4, 1, Rt))
// mrs   x0, cntvct_el0     op0=0b11 op1=0b011 CRn=0b1110 CRm=0b0000 op2=0b010
#define MRS_cntvct_el0(Rt)              EMIT(MRS_gen(1, 1, 0b011, 0b1110, 0b0000, 0b010, Rt))
// mrs   x0, cntpctss_el0   op0=0b11 op1=0b011 CRn=0b1110 CRm=0b0000 op2=0b101
#define MRS_cntpctss_el0(Rt)            EMIT(MRS_gen(1, 1, 0b011, 0b1110, 0b0000, 0b101, Rt))
// mrs   rt, rndr           op0=0b11 op1=0b011 CRn=0b0010 CRm=0b0100 op2=0b000
#define MRS_rndr(Rt)                    EMIT(MRS_gen(1, 1, 0b011, 0b0010, 0b0100, 0b000, Rt))
// NEON Saturation Bit
#define FPSR_QC 27
// NEON Input Denormal Cumulative
#define FPSR_IDC    7
// NEON IneXact Cumulative
#define FPSR_IXC    4
// NEON Underflow Cumulative
#define FPSR_UFC    3
// NEON Overflow Cumulative
#define FPSR_OFC    2
// NEON Divide by 0 Cumulative
#define FPSR_DZC    1
// NEON Invalid Operation Cumulative
#define FPSR_IOC    0
// NZCV N
#define NZCV_N      31
// NZCV Z
#define NZCV_Z      30
// NZCV C
#define NZCV_C      29
// NZCV V
#define NZCV_V      28
                     
// FCSEL
#define FCSEL_scalar(type, Rm, cond, Rn, Rd)    (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | (cond)<<12 | 0b11<<10 | (Rn)<<5 | (Rd))
#define FCSELS(Sd, Sn, Sm, cond)        EMIT(FCSEL_scalar(0b00, Sm, cond, Sn, Sd))
#define FCSELD(Dd, Dn, Dm, cond)        EMIT(FCSEL_scalar(0b01, Dm, cond, Dn, Dd))
 
// VLDR/VSTR
#define VMEM_gen(size, opc, imm12, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | 0b01<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
// imm13 must be 1-aligned
#define VLDR16_U12(Ht, Rn, imm13)           EMIT(VMEM_gen(0b01, 0b01, ((uint32_t)((imm13)>>1))&0xfff, Rn, Ht))
// imm14 must be 2-aligned
#define VLDR32_U12(Dt, Rn, imm14)           EMIT(VMEM_gen(0b10, 0b01, ((uint32_t)((imm14)>>2))&0xfff, Rn, Dt))
// imm15 must be 3-aligned
#define VLDR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b01, ((uint32_t)((imm15)>>3))&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VLDR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b00, 0b11, ((uint32_t)((imm16)>>4))&0xfff, Rn, Qt))
// (imm14) must be 3-aligned
#define VSTR32_U12(Dt, Rn, imm14)           EMIT(VMEM_gen(0b10, 0b00, ((uint32_t)((imm14)>>2))&0xfff, Rn, Dt))
// (imm15) must be 3-aligned
#define VSTR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b00, ((uint32_t)((imm15)>>3))&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VSTR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b00, 0b10, ((uint32_t)((imm16)>>4))&0xfff, Rn, Qt))
// (imm13) must be 1-aligned
#define VSTR16_U12(Ht, Rn, imm13)           EMIT(VMEM_gen(0b01, 0b00, ((uint32_t)((imm13)>>1))&0xfff, Rn, Ht))

//VLDP/VSTP
#define VMEMP_vector(opc, L, imm7, Rt2, Rn, Rt) ((opc)<<30 | 0b101<<27 | 1<<26 | 0b010<<23 | (L)<<22 | (imm7)<<15 | (Rt2)<<10 | (Rn)<<5 | (Rt))
#define VLDP32_I7(Rt1, Rt2, Rn, imm9)       EMIT(VMEMP_vector(0b00, 1, (((int64_t)(imm9))>>2)&0x7f, Rt2, Rn, Rt1))
#define VLDP64_I7(Rt1, Rt2, Rn, imm10)      EMIT(VMEMP_vector(0b01, 1, (((int64_t)(imm10))>>3)&0x7f, Rt2, Rn, Rt1))
#define VLDP128_I7(Rt1, Rt2, Rn, imm11)     EMIT(VMEMP_vector(0b10, 1, (((int64_t)(imm11))>>4)&0x7f, Rt2, Rn, Rt1))
#define VSTP32_I7(Rt1, Rt2, Rn, imm9)       EMIT(VMEMP_vector(0b00, 0, (((int64_t)(imm9))>>2)&0x7f, Rt2, Rn, Rt1))
#define VSTP64_I7(Rt1, Rt2, Rn, imm10)      EMIT(VMEMP_vector(0b01, 0, (((int64_t)(imm10))>>3)&0x7f, Rt2, Rn, Rt1))
#define VSTP128_I7(Rt1, Rt2, Rn, imm11)     EMIT(VMEMP_vector(0b10, 0, (((int64_t)(imm11))>>4)&0x7f, Rt2, Rn, Rt1))

#define VMEMUR_vector(size, opc, imm9, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | (opc)<<22 | (imm9)<<12 | (Rn)<<5 | (Rt))
// signed offset, no alignement!
#define VLDR8_I9(Vt, Rn, imm9)              EMIT(VMEMUR_vector(0b00, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR16_I9(Vt, Rn, imm9)             EMIT(VMEMUR_vector(0b01, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR32_I9(Vt, Rn, imm9)             EMIT(VMEMUR_vector(0b10, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR64_I9(Vt, Rn, imm9)             EMIT(VMEMUR_vector(0b11, 0b01, (imm9)&0b111111111, Rn, Vt))
#define VLDR128_I9(Vt, Rn, imm9)            EMIT(VMEMUR_vector(0b00, 0b11, (imm9)&0b111111111, Rn, Vt))
// signed offset, no alignement!
#define VSTR8_I9(Vt, Rn, imm9)              EMIT(VMEMUR_vector(0b00, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR16_I9(Vt, Rn, imm9)             EMIT(VMEMUR_vector(0b01, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR32_I9(Vt, Rn, imm9)             EMIT(VMEMUR_vector(0b10, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR64_I9(Vt, Rn, imm9)             EMIT(VMEMUR_vector(0b11, 0b00, (imm9)&0b111111111, Rn, Vt))
#define VSTR128_I9(Vt, Rn, imm9)            EMIT(VMEMUR_vector(0b00, 0b10, (imm9)&0b111111111, Rn, Vt))

#define VLD128(A, B, C)     if(unscaled) {VLDR128_I9(A, B, C);} else {VLDR128_U12(A, B, C);}
#define VLD64(A, B, C)      if(unscaled) {VLDR64_I9(A, B, C);} else {VLDR64_U12(A, B, C);}
#define VLD32(A, B, C)      if(unscaled) {VLDR32_I9(A, B, C);} else {VLDR32_U12(A, B, C);}
#define VLD16(A, B, C)      if(unscaled) {VLDR16_I9(A, B, C);} else {VLDR16_U12(A, B, C);}
#define VLD8(A, B, C)       if(unscaled) {VLDR8_I9(A, B, C);} else {VLDR8_U12(A, B, C);}
#define VST128(A, B, C)     if(unscaled) {VSTR128_I9(A, B, C);} else {VSTR128_U12(A, B, C);}
#define VST64(A, B, C)      if(unscaled) {VSTR64_I9(A, B, C);} else {VSTR64_U12(A, B, C);}
#define VST32(A, B, C)      if(unscaled) {VSTR32_I9(A, B, C);} else {VSTR32_U12(A, B, C);}
#define VST16(A, B, C)      if(unscaled) {VSTR16_I9(A, B, C);} else {VSTR16_U12(A, B, C);}
#define VST8(A, B, C)       if(unscaled) {VSTR8_I9(A, B, C);} else {VSTR8_U12(A, B, C);}

#define VMEMW_gen(size, opc, imm9, op2, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | (opc)<<22 | (imm9)<<12 | (op2)<<10 | 0b01<<10 | (Rn)<<5 | (Rt))
#define VLDR64_S9_postindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b01, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VLDR64_S9_preindex(Rt, Rn, imm9)    EMIT(VMEMW_gen(0b11, 0b01, (imm9)&0x1ff, 0b11, Rn, Rt))
#define VLDR128_S9_postindex(Rt, Rn, imm9)  EMIT(VMEMW_gen(0b11, 0b11, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VLDR128_S9_preindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b11, (imm9)&0x1ff, 0b11, Rn, Rt))
#define VSTR64_S9_postindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VSTR64_S9_preindex(Rt, Rn, imm9)    EMIT(VMEMW_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define VSTR128_S9_postindex(Rt, Rn, imm9)  EMIT(VMEMW_gen(0b11, 0b10, (imm9)&0x1ff, 0b01, Rn, Rt))
#define VSTR128_S9_preindex(Rt, Rn, imm9)   EMIT(VMEMW_gen(0b11, 0b10, (imm9)&0x1ff, 0b11, Rn, Rt))

#define VMEM_REG_gen(size, opc, Rm, option, S, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | (opc)<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | 0b10<<10 | (Rn)<<5 | (Rt))

#define VLDR32_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b10, 0b01, Rm, 0b011, 0, Rn, Dt))
#define VLDR32_REG_SXTW(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b01, Rm, 0b110, 0, Rn, Dt))
#define VLDR32_REG_LSL2(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 0, Rn, Dt))
#define VLDR64_REG_SXTW(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b110, 0, Rn, Dt))
#define VLDR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 0, Rn, Qt))
#define VLDR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 1, Rn, Qt))
#define VLDR128_REG_SXTW(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b110, 0, Rn, Qt))

#define VSTR32_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR32_REG_LSL2(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR32_REG_SXTW(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b110, 0, Rn, Dt))
#define VSTR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR64_REG_SXTW(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b110, 0, Rn, Dt))
#define VSTR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 0, Rn, Qt))
#define VSTR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 1, Rn, Qt))
#define VSTR128_REG_SXTW(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b110, 0, Rn, Qt))

#define VLDR_PC_gen(opc, imm19, Rt)         ((opc)<<30 | 0b011<<27 | 1<<26 | (imm19)<<5 | (Rt))
#define VLDR32_literal(Vt, imm19)           EMIT(VLDR_PC_gen(0b00, ((imm19)>>2)&0x7FFFF, Vt))
#define VLDR64_literal(Vt, imm19)           EMIT(VLDR_PC_gen(0b01, ((imm19)>>2)&0x7FFFF, Vt))
#define VLDR128_literal(Vt, imm19)          EMIT(VLDR_PC_gen(0b10, ((imm19)>>2)&0x7FFFF, Vt))


#define LD1R_gen(Q, size, Rn, Rt)           ((Q)<<30 | 0b0011010<<23 | 1<<22 | 0<<21 | 0b110<<13 | (size)<<10 | (Rn)<<5 | (Rt))
#define VLDQ1R_8(Vt, Rn)                    EMIT(LD1R_gen(1, 0b00, Rn, Vt))
#define VLDQ1R_16(Vt, Rn)                   EMIT(LD1R_gen(1, 0b01, Rn, Vt))
#define VLDQ1R_32(Vt, Rn)                   EMIT(LD1R_gen(1, 0b10, Rn, Vt))
#define VLDQ1R_64(Vt, Rn)                   EMIT(LD1R_gen(1, 0b11, Rn, Vt))
#define VLD1R_8(Vt, Rn)                     EMIT(LD1R_gen(0, 0b00, Rn, Vt))
#define VLD1R_16(Vt, Rn)                    EMIT(LD1R_gen(0, 0b01, Rn, Vt))
#define VLD1R_32(Vt, Rn)                    EMIT(LD1R_gen(0, 0b10, Rn, Vt))

#define LD1_single(Q, opcode, S, size, Rn, Rt)  ((Q)<<30 | 0b0011010<<23 | 1<<22 | 0<<21 | (opcode)<<13 | (S)<<12 | (size)<<10 | (Rn)<<5 | (Rt))
#define VLD1_8(Vt, index, Rn)               EMIT(LD1_single(((index)>>3)&1, 0b000, ((index)>>2)&1, (index)&3, Rn, Vt))
#define VLD1_16(Vt, index, Rn)              EMIT(LD1_single(((index)>>2)&1, 0b010, ((index)>>1)&1, ((index)&1)<<1, Rn, Vt))
#define VLD1_32(Vt, index, Rn)              EMIT(LD1_single(((index)>>1)&1, 0b100, ((index))&1, 0b00, Rn, Vt))
#define VLD1_64(Vt, index, Rn)              EMIT(LD1_single(((index))&1, 0b100, 0, 0b01, Rn, Vt))

#define ST1_single(Q, opcode, S, size, Rn, Rt)  ((Q)<<30 | 0b0011010<<23 | 0<<22 | 0<<21 | (opcode)<<13 | (S)<<12 | (size)<<10 | (Rn)<<5 | (Rt))
#define VST1_8(Vt, index, Rn)               EMIT(ST1_single(((index)>>3)&1, 0b000, ((index)>>2)&1, (index)&3, Rn, Vt))
#define VST1_16(Vt, index, Rn)              EMIT(ST1_single(((index)>>2)&1, 0b010, ((index)>>1)&1, ((index)&1)<<1, Rn, Vt))
#define VST1_32(Vt, index, Rn)              EMIT(ST1_single(((index)>>1)&1, 0b100, ((index))&1, 0b00, Rn, Vt))
#define VST1_64(Vt, index, Rn)              EMIT(ST1_single(((index))&1, 0b100, 0, 0b01, Rn, Vt))

// LOGIC
#define VLOGIC_gen(Q, opc2, Rm, Rn, Rd)     ((Q)<<30 | 1<<29 | 0b01110<<24 | (opc2)<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VEORQ(Vd, Vn, Vm)                   EMIT(VLOGIC_gen(1, 0b00, Vm, Vn, Vd))
#define VEOR(Vd, Vn, Vm)                    EMIT(VLOGIC_gen(0, 0b00, Vm, Vn, Vd))

#define VLOGIC_immediate(Q, op, abc, cmade, defgh, Rd)  ((Q)<<30 | (op)<<29 | 0b0111100000<<19 | (abc)<<16 | (cmode)<<12 | 1<<10 | (defgh)<<5 | (Rd))
//#define V

#define SHL_vector(Q, immh, immb, Rn, Rd)   ((Q)<<30 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b01010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VSHLQ_8(Vd, Vn, shift)              EMIT(SHL_vector(1, 0b0001, (shift)&7, Vn, Vd))
#define VSHLQ_16(Vd, Vn, shift)             EMIT(SHL_vector(1, 0b0010 | (((shift)>>3)&1), (shift)&7, Vn, Vd))
#define VSHLQ_32(Vd, Vn, shift)             EMIT(SHL_vector(1, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))
#define VSHLQ_64(Vd, Vn, shift)             EMIT(SHL_vector(1, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))
#define VSHL_8(Vd, Vn, shift)               EMIT(SHL_vector(0, 0b0001, (shift)&7, Vn, Vd))
#define VSHL_16(Vd, Vn, shift)              EMIT(SHL_vector(0, 0b0010 | (((shift)>>3)&1), (shift)&7, Vn, Vd))
#define VSHL_32(Vd, Vn, shift)              EMIT(SHL_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

#define SHL_scalar(U, size, Rm, R, S, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b010<<13 | (R)<<12 | (S)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SSHL_R_64(Vd, Vn, Vm)               EMIT(SHL_scalar(0, 0b11, Vm, 0, 0, Vn, Vd))
#define USHL_R_64(Vd, Vn, Vm)               EMIT(SHL_scalar(1, 0b11, Vm, 0, 0, Vn, Vd))

#define SHL_scalar_imm(U, immh, immb, Rn, Rd)   (0b01<<30 | 0b111110<<23 | (immh)<<19 | (immb)<<16 | 0b01010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SHL_64(Vd, Vn, shift)               EMIT(SHL_scalar_imm(0, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))

#define SHL_vector_vector(Q, U, size, Rm, R, S, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b010<<13 | (R)<<12 | (S)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SSHL_8(Vd, Vn, Vm)                  EMIT(SHL_vector_vector(0, 0, 0b00, Vm, 0, 0, Vn, Vd))
#define SSHL_16(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 0, 0b01, Vm, 0, 0, Vn, Vd))
#define SSHL_32(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 0, 0b10, Vm, 0, 0, Vn, Vd))
#define SSHLQ_8(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(1, 0, 0b00, Vm, 0, 0, Vn, Vd))
#define SSHLQ_16(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 0, 0b01, Vm, 0, 0, Vn, Vd))
#define SSHLQ_32(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 0, 0b10, Vm, 0, 0, Vn, Vd))
#define SSHLQ_64(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 0, 0b11, Vm, 0, 0, Vn, Vd))
#define USHL_8(Vd, Vn, Vm)                  EMIT(SHL_vector_vector(0, 1, 0b00, Vm, 0, 0, Vn, Vd))
#define USHL_16(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 1, 0b01, Vm, 0, 0, Vn, Vd))
#define USHL_32(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(0, 1, 0b10, Vm, 0, 0, Vn, Vd))
#define USHLQ_8(Vd, Vn, Vm)                 EMIT(SHL_vector_vector(1, 1, 0b00, Vm, 0, 0, Vn, Vd))
#define USHLQ_16(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 1, 0b01, Vm, 0, 0, Vn, Vd))
#define USHLQ_32(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 1, 0b10, Vm, 0, 0, Vn, Vd))
#define USHLQ_64(Vd, Vn, Vm)                EMIT(SHL_vector_vector(1, 1, 0b11, Vm, 0, 0, Vn, Vd))

#define SHR_vector(Q, U, immh, immb, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b00000<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VSHRQ_8(Vd, Vn, shift)              EMIT(SHR_vector(1, 1, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSHRQ_16(Vd, Vn, shift)             EMIT(SHR_vector(1, 1, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSHRQ_32(Vd, Vn, shift)             EMIT(SHR_vector(1, 1, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))
#define VSHRQ_64(Vd, Vn, shift)             EMIT(SHR_vector(1, 1, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, Vn, Vd))
#define VSHR_8(Vd, Vn, shift)               EMIT(SHR_vector(0, 1, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSHR_16(Vd, Vn, shift)              EMIT(SHR_vector(0, 1, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSHR_32(Vd, Vn, shift)              EMIT(SHR_vector(0, 1, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))
#define VSSHRQ_8(Vd, Vn, shift)             EMIT(SHR_vector(1, 0, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSSHRQ_16(Vd, Vn, shift)            EMIT(SHR_vector(1, 0, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSSHRQ_32(Vd, Vn, shift)            EMIT(SHR_vector(1, 0, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))
#define VSSHRQ_64(Vd, Vn, shift)            EMIT(SHR_vector(1, 0, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, Vn, Vd))
#define VSSHR_8(Vd, Vn, shift)              EMIT(SHR_vector(0, 0, 0b0001, (8-(shift))&7, Vn, Vd))
#define VSSHR_16(Vd, Vn, shift)             EMIT(SHR_vector(0, 0, 0b0010 | (((16-(shift))>>3)&1), (16-(shift))&7, Vn, Vd))
#define VSSHR_32(Vd, Vn, shift)             EMIT(SHR_vector(0, 0, 0b0100 | (((32-(shift))>>3)&3), (32-(shift))&7, Vn, Vd))

#define SHR_scalar_imm(U, immh, immb, o1, o0, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b111110<<23 | (immh)<<19 | (immb)<<16 | (o1)<<13 | (o0)<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define SSHR_64(Vd, Vn, shift)              EMIT(SHR_scalar_imm(0, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, 0, 0, Vn, Vd))
#define USHR_64(Vd, Vn, shift)              EMIT(SHR_scalar_imm(1, 0b1000 | (((64-(shift))>>3)&7), (64-(shift))&7, 0, 0, Vn, Vd))

#define EXT_vector(Q, Rm, imm4, Rn, Rd)     ((Q)<<30 | 0b101110<<24 | (Rm)<<16 | (imm4)<<11 | (Rn)<<5 | (Rd))
#define VEXT_8(Rd, Rn, Rm, index)           EMIT(EXT_vector(0, Rm, index, Rn, Rd))
#define VEXTQ_8(Rd, Rn, Rm, index)          EMIT(EXT_vector(1, Rm, index, Rn, Rd))

// Shift Left and Insert (not touching lower part of dest)
#define SLI_vector(Q, immh, immb, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b01010<<1 | 1<<10 | (Rn)<<5 | (Rd))
#define VSLIQ_8(Vd, Vn, shift)              EMIT(VSLI_vector(1, 0b0001, (shift)&7, Vn, Vd))
#define VSLIQ_16(Vd, Vn, shift)             EMIT(VSLI_vector(1, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSLIQ_32(Vd, Vn, shift)             EMIT(VSLI_vector(1, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))
#define VSLIQ_64(Vd, Vn, shift)             EMIT(VSLI_vector(1, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))
#define VSLI_8(Vd, Vn, shift)               EMIT(VSLI_vector(0, 0b0001, (shift)&7, Vn, Vd))
#define VSLI_16(Vd, Vn, shift)              EMIT(VSLI_vector(0, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSLI_32(Vd, Vn, shift)              EMIT(VSLI_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

// Shift Right and Insert (not touching higher part of dest)
#define SRI_vector(Q, immh, immb, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b01000<<1 | 1<<10 | (Rn)<<5 | (Rd))
#define VSRIQ_8(Vd, Vn, shift)              EMIT(VSRI_vector(1, 0b0001, (shift)&7, Vn, Vd))
#define VSRIQ_16(Vd, Vn, shift)             EMIT(VSRI_vector(1, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSRIQ_32(Vd, Vn, shift)             EMIT(VSRI_vector(1, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))
#define VSRIQ_64(Vd, Vn, shift)             EMIT(VSRI_vector(1, 0b1000 | (((shift)>>3)&7), (shift)&7, Vn, Vd))
#define VSRI_8(Vd, Vn, shift)               EMIT(VSRI_vector(0, 0b0001, (shift)&7, Vn, Vd))
#define VSRI_16(Vd, Vn, shift)              EMIT(VSRI_vector(0, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSRI_32(Vd, Vn, shift)              EMIT(VSRI_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

// Reverse elements in 64-bit doublewords (vector). This instruction reverses the order of 8-bit, 16-bit, or 32-bit elements in each doubleword
#define VREVx_vector(Q, U, size, o0, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | (o0)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VREV64_32(Vd, Vn)                   EMIT(VREVx_vector(0, 0, 0b10, 0, Vn, Vd))
#define VREV64_16(Vd, Vn)                   EMIT(VREVx_vector(0, 0, 0b01, 0, Vn, Vd))
#define VREV64_8(Vd, Vn)                    EMIT(VREVx_vector(0, 0, 0b00, 0, Vn, Vd))
#define VREV64Q_32(Vd, Vn)                  EMIT(VREVx_vector(1, 0, 0b10, 0, Vn, Vd))
#define VREV64Q_16(Vd, Vn)                  EMIT(VREVx_vector(1, 0, 0b01, 0, Vn, Vd))
#define VREV64Q_8(Vd, Vn)                   EMIT(VREVx_vector(1, 0, 0b00, 0, Vn, Vd))

// Integer MATH
#define ADDSUB_vector(Q, U, size, Rm, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10000<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VADDQ_8(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(1, 0, 0b00, Vm, Vn, Vd))
#define VADDQ_16(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 0, 0b01, Vm, Vn, Vd))
#define VADDQ_32(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 0, 0b10, Vm, Vn, Vd))
#define VADDQ_64(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 0, 0b11, Vm, Vn, Vd))
#define VADD_8(Vd, Vn, Vm)                  EMIT(ADDSUB_vector(0, 0, 0b00, Vm, Vn, Vd))
#define VADD_16(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 0, 0b01, Vm, Vn, Vd))
#define VADD_32(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 0, 0b10, Vm, Vn, Vd))
#define VSUBQ_8(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(1, 1, 0b00, Vm, Vn, Vd))
#define VSUBQ_16(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 1, 0b01, Vm, Vn, Vd))
#define VSUBQ_32(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 1, 0b10, Vm, Vn, Vd))
#define VSUBQ_64(Vd, Vn, Vm)                EMIT(ADDSUB_vector(1, 1, 0b11, Vm, Vn, Vd))
#define VSUB_8(Vd, Vn, Vm)                  EMIT(ADDSUB_vector(0, 1, 0b00, Vm, Vn, Vd))
#define VSUB_16(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 1, 0b01, Vm, Vn, Vd))
#define VSUB_32(Vd, Vn, Vm)                 EMIT(ADDSUB_vector(0, 1, 0b10, Vm, Vn, Vd))

#define ADDSUB_scalar(U, size, Rm, Rn, Rd)  (01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10000<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define ADD_64(Vd, Vn, Vm)                  EMIT(ADDSUB_scalar(0, 0b11, Vm, Vn, Vd))
#define SUB_64(Vd, Vn, Vm)                  EMIT(ADDSUB_scalar(1, 0b11, Vm, Vn, Vd))

#define NEGABS_vector(Q, U, size, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b01011<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define NEG_8(Vd, Vn)                       EMIT(NEGABS_vector(0, 1, 0b00, Vn, Vd))
#define NEG_16(Vd, Vn)                      EMIT(NEGABS_vector(0, 1, 0b01, Vn, Vd))
#define NEG_32(Vd, Vn)                      EMIT(NEGABS_vector(0, 1, 0b10, Vn, Vd))
#define NEGQ_8(Vd, Vn)                      EMIT(NEGABS_vector(1, 1, 0b00, Vn, Vd))
#define NEGQ_16(Vd, Vn)                     EMIT(NEGABS_vector(1, 1, 0b01, Vn, Vd))
#define NEGQ_32(Vd, Vn)                     EMIT(NEGABS_vector(1, 1, 0b10, Vn, Vd))
#define NEGQ_64(Vd, Vn)                     EMIT(NEGABS_vector(1, 1, 0b11, Vn, Vd))
#define ABS_8(Vd, Vn)                       EMIT(NEGABS_vector(0, 0, 0b00, Vn, Vd))
#define ABS_16(Vd, Vn)                      EMIT(NEGABS_vector(0, 0, 0b01, Vn, Vd))
#define ABS_32(Vd, Vn)                      EMIT(NEGABS_vector(0, 0, 0b10, Vn, Vd))
#define ABSQ_8(Vd, Vn)                      EMIT(NEGABS_vector(1, 0, 0b00, Vn, Vd))
#define ABSQ_16(Vd, Vn)                     EMIT(NEGABS_vector(1, 0, 0b01, Vn, Vd))
#define ABSQ_32(Vd, Vn)                     EMIT(NEGABS_vector(1, 0, 0b10, Vn, Vd))
#define ABSQ_64(Vd, Vn)                     EMIT(NEGABS_vector(1, 0, 0b11, Vn, Vd))

#define NEGABS_vector_scalar(U, size, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 0b10000<<17 | 0b01011<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define NEG_64(Vd, Vn)                     EMIT(NEGABS_vector_scalar(1, 0b11, Vn, Vd))
#define ABS_64(Vd, Vn)                     EMIT(NEGABS_vector_scalar(0, 0b11, Vn, Vd))

// FMOV
#define FMOV_general(sf, type, mode, opcode, Rn, Rd)    ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (mode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
// 32-bit to single-precision
#define FMOVSw(Sd, Wn)                      EMIT(FMOV_general(0, 0b00, 0b00, 0b111, Wn, Sd))
// Single-precision to 32-bit
#define FMOVwS(Wd, Sn)                      EMIT(FMOV_general(0, 0b00, 0b00, 0b110, Sn, Wd))
// 64-bit to double-precision
#define FMOVDx(Dd, Xn)                      EMIT(FMOV_general(1, 0b01, 0b00, 0b111, Xn, Dd))
// 64-bit to top half of 128-bit
#define FMOVD1x(Vd, Xn)                     EMIT(FMOV_general(1, 0b10, 0b01, 0b111, Xn, Vd))
// Double-precision to 64-bit
#define FMOVxD(Xd, Dn)                      EMIT(FMOV_general(1, 0b01, 0b00, 0b110, Dn, Xd))
// Top half of 128-bit to 64-bit
#define FMOVxD1(Xd, Vn)                     EMIT(FMOV_general(1, 0b10, 0b01, ob110, Vn, Xd))

#define FMOV_register(type, Rn, Rd)         (0b11110<<24 | (type)<<22 | 1<<21 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FMOVS(Sd, Sn)                       EMIT(FMOV_register(0b00, Sn, Sd))
#define FMOVD(Dd, Dn)                       EMIT(FMOV_register(0b01, Dn, Dd))

#define FMOV_vector_imm(Q, op, abc, defgh, Rd)  ((Q)<<30 | (op)<<29 | 0b0111100000<<19 | (abc)<<16 | 0b1111<<12 | 1<<10 | (defgh)<<5 | (Rd))
#define VFMOVS_8(Vd, u8)                    EMIT(FMOV_vector_imm(0, 0, ((u8)>>5)&0b111, (u8)&0b11111, Vd))
#define VFMOVSQ_8(Vd, u8)                   EMIT(FMOV_vector_imm(1, 0, ((u8)>>5)&0b111, (u8)&0b11111, Vd))
#define VFMOVDQ_8(Vd, u8)                   EMIT(FMOV_vector_imm(1, 1, ((u8)>>5)&0b111, (u8)&0b11111, Vd))

#define FMOV_scalar_imm(type, imm8, Rd)     (0b11110<<24 | (type)<<22 | 1<<21 | (imm8)<<13 | 0b100<<10 | (Rd))
// FMOV to Sd, imm=7 :~6:6:6:6:6:6:5:4 :3:2:1:0....
#define FMOVS_8(Sd, u8)                     EMIT(FMOV_scalar_imm(0b00, u8, Sd))
// FMOV to Dd, imm=7 :~6:6:6:6:6:6:6:6:6:5:4 :3:2:1:0....
#define FMOVD_8(Dd, u8)                     EMIT(FMOV_scalar_imm(0b01, u8, Dd))

// VMOV
#define VMOV_element(imm5, imm4, Rn, Rd)    (1<<30 | 1<<29 | 0b01110000<<21 | (imm5)<<16 | (imm4)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVeB(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<1) | 1, (i2), Vn, Vd))
#define VMOVeH(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<2) | 2, (i2)<<1, Vn, Vd))
#define VMOVeS(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<3) | 4, (i2)<<2, Vn, Vd))
#define VMOVeD(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<4) | 8, (i2)<<3, Vn, Vd))

#define VMOV_from(imm5, Rn, Rd)     (1<<30 | 0<<29 | 0b01110000<<21 | (imm5)<<16 | 0b0011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVQBfrom(Vd, index, Wn)    EMIT(VMOV_from(((index)<<1) | 1, Wn, Vd))
#define VMOVQHfrom(Vd, index, Wn)    EMIT(VMOV_from(((index)<<2) | 2, Wn, Vd))
#define VMOVQSfrom(Vd, index, Wn)    EMIT(VMOV_from(((index)<<3) | 4, Wn, Vd))
#define VMOVQDfrom(Vd, index, Xn)    EMIT(VMOV_from(((index)<<4) | 8, Xn, Vd))

#define UMOV_gen(Q, imm5, Rn, Rd)   ((Q)<<30 | 0b01110000<<21 | (imm5)<<16 | 0b01<<13 | 1<<12 | 1<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVQDto(Xd, Vn, index)     EMIT(UMOV_gen(1, ((index)<<4) | 8, Vn, Xd))
#define VMOVBto(Wd, Vn, index)      EMIT(UMOV_gen(0, ((index)<<1) | 1, Vn, Wd))
#define VMOVHto(Wd, Vn, index)      EMIT(UMOV_gen(0, ((index)<<2) | 2, Vn, Wd))
#define VMOVSto(Wd, Vn, index)      EMIT(UMOV_gen(0, ((index)<<3) | 4, Vn, Wd))

#define SMOV_gen(Q, imm5, Rn, Rd)   ((Q)<<30 | 0b01110000<<21 | (imm5)<<16 | 0b01<<13 | 0<<12 | 1<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SMOVQDto(Xd, Vn, index)     EMIT(SMOV_gen(1, ((index)<<4) | 8, Vn, Xd))
#define SMOVQBto(Xd, Vn, index)     EMIT(SMOV_gen(1, ((index)<<1) | 1, Vn, Xd))
#define SMOVQHto(Xd, Vn, index)     EMIT(SMOV_gen(1, ((index)<<2) | 2, Vn, Xd))
#define SMOVQSto(Xd, Vn, index)     EMIT(SMOV_gen(1, ((index)<<3) | 4, Vn, Xd))

#define MVN_vector(Q, Rn, Rd)       ((Q)<<30 | 1<<29 | 0b01110<<24 | 0b10000<<17 | 0b00101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VMVNQ(Rd, Rn)               EMIT(MVN_vector(1, Rn, Rd))

// VORR
#define ORR_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b10<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VORRQ(Vd, Vn, Vm)           EMIT(ORR_vector(1, Vm, Vn, Vd))
#define VORR(Dd, Dn, Dm)            EMIT(ORR_vector(0, Dm, Dn, Dd))
#define VMOVQ(Vd, Vn)               EMIT(ORR_vector(1, Vn, Vn, Vd))
#define VMOV(Dd, Dn)                EMIT(ORR_vector(0, Dn, Dn, Dd))

// VAND
#define AND_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b00<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VANDQ(Vd, Vn, Vm)           EMIT(AND_vector(1, Vm, Vn, Vd))
#define VAND(Dd, Dn, Dm)            EMIT(AND_vector(0, Dm, Dn, Dd))

// VBIC
#define BIC_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b01<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VBICQ(Vd, Vn, Vm)           EMIT(BIC_vector(1, Vm, Vn, Vd))
#define VBIC(Dd, Dn, Dm)            EMIT(BIC_vector(0, Dm, Dn, Dd))

// VORN
#define ORN_vector(Q, Rm, Rn, Rd)   ((Q)<<30 | 0b01110<<24 | 0b11<<22 | 1<<21 | (Rm)<<16 | 0b00011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VORNQ(Vd, Vn, Vm)           EMIT(ORN_vector(1, Vm, Vn, Vd))
#define VORN(Dd, Dn, Dm)            EMIT(ORN_vector(0, Dm, Dn, Dd))

// ADD / SUB
#define FADDSUB_vector(Q, U, op, sz, Rm, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (op)<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFADDQS(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 0, 0, Vm, Vn, Vd))
#define VFADDQD(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 0, 1, Vm, Vn, Vd))
#define VFADDS(Dd, Dn, Dm)          EMIT(FADDSUB_vector(0, 0, 0, 0, Dm, Dn, Dd))

#define VFSUBQS(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 1, 0, Vm, Vn, Vd))
#define VFSUBQD(Vd, Vn, Vm)         EMIT(FADDSUB_vector(1, 0, 1, 1, Vm, Vn, Vd))
#define VFSUBS(Dd, Dn, Dm)          EMIT(FADDSUB_vector(0, 0, 1, 0, Dm, Dn, Dd))

#define FADDSUB_scalar(type, Rm, op, Rn, Rd)    (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b001<<13 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FADDS(Sd, Sn, Sm)           EMIT(FADDSUB_scalar(0b00, Sm, 0, Sn, Sd))
#define FADDD(Dd, Dn, Dm)           EMIT(FADDSUB_scalar(0b01, Dm, 0, Dn, Dd))

#define FSUBS(Sd, Sn, Sm)           EMIT(FADDSUB_scalar(0b00, Sm, 1, Sn, Sd))
#define FSUBD(Dd, Dn, Dm)           EMIT(FADDSUB_scalar(0b01, Dm, 1, Dn, Dd))

// ADD Pair
#define ADDP_vector(Q, size, Rm, Rn, Rd)    ((Q)<<30 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VADDPQ_8(Vd, Vn, Vm)        EMIT(ADDP_vector(1, 0b00, Vm, Vn, Vd))
#define VADDPQ_16(Vd, Vn, Vm)       EMIT(ADDP_vector(1, 0b01, Vm, Vn, Vd))
#define VADDPQ_32(Vd, Vn, Vm)       EMIT(ADDP_vector(1, 0b10, Vm, Vn, Vd))
#define VADDPQ_64(Vd, Vn, Vm)       EMIT(ADDP_vector(1, 0b11, Vm, Vn, Vd))
#define VADDP_8(Vd, Vn, Vm)         EMIT(ADDP_vector(0, 0b00, Vm, Vn, Vd))
#define VADDP_16(Vd, Vn, Vm)        EMIT(ADDP_vector(0, 0b01, Vm, Vn, Vd))
#define VADDP_32(Vd, Vn, Vm)        EMIT(ADDP_vector(0, 0b10, Vm, Vn, Vd))

#define FADDP_vector(Q, sz, Rm, Rn, Rd) ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFADDPQS(Vd, Vn, Vm)        EMIT(FADDP_vector(1, 0, Vm, Vn, Vd))
#define VFADDPQD(Vd, Vn, Vm)        EMIT(FADDP_vector(1, 1, Vm, Vn, Vd))
#define VFADDPS(Vd, Vn, Vm)         EMIT(FADDP_vector(0, 0, Vm, Vn, Vd))
#define VFADDPD(Vd, Vn, Vm)         EMIT(FADDP_vector(0, 1, Vm, Vn, Vd))

#define FADDP_scalar(sz, Rn, Rd)        (0b01<<30 | 1<<29 | 0b11110<<24 | (sz)<<22 | 0b11000<<17 | 0b01101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FADDPS(Sd, Sn)              EMIT(FADDP_scalar(0, Sn, Sd))
#define FADDPD(Dd, Dn)              EMIT(FADDP_scalar(1, Dn, Dd))

// NEG / ABS
#define FNEGABS_scalar(type, opc, Rn, Rd)  (0b11110<<24 | (type)<<22 | 1<<21 | (opc)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FNEGS(Sd, Sn)               EMIT(FNEGABS_scalar(0b00, 0b10, Sn, Sd))
#define FNEGD(Dd, Dn)               EMIT(FNEGABS_scalar(0b01, 0b10, Dn, Dd))

#define FABSS(Sd, Sn)               EMIT(FNEGABS_scalar(0b00, 0b01, Sn, Sd))
#define FABSD(Dd, Dn)               EMIT(FNEGABS_scalar(0b01, 0b01, Dn, Dd))

#define FNEGABS_vector(Q, U, sz, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b01111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFNEGS(Vd, Vn)              EMIT(FNEGABS_vector(0, 1, 0, Vn, Vd))
#define VFNEGQS(Vd, Vn)             EMIT(FNEGABS_vector(1, 1, 0, Vn, Vd))
#define VFNEGQD(Vd, Vn)             EMIT(FNEGABS_vector(1, 1, 1, Vn, Vd))
#define VFABSS(Vd, Vn)              EMIT(FNEGABS_vector(0, 0, 0, Vn, Vd))
#define VFABSQS(Vd, Vn)             EMIT(FNEGABS_vector(1, 0, 0, Vn, Vd))
#define VFABSQD(Vd, Vn)             EMIT(FNEGABS_vector(1, 0, 1, Vn, Vd))

// MUL
#define FMUL_vector(Q, sz, Rm, Rn, Rd)  ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFMULS(Sd, Sn, Sm)          EMIT(FMUL_vector(0, 0, Sm, Sn, Sd))
#define VFMULQS(Sd, Sn, Sm)         EMIT(FMUL_vector(1, 0, Sm, Sn, Sd))
#define VFMULQD(Sd, Sn, Sm)         EMIT(FMUL_vector(1, 1, Sm, Sn, Sd))

#define FMUL_scalar(type, Rm, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b10<<10 | (Rn)<<5 | Rd)
#define FMULS(Sd, Sn, Sm)           EMIT(FMUL_scalar(0b00, Sm, Sn, Sd))
#define FMULD(Dd, Dn, Dm)           EMIT(FMUL_scalar(0b01, Dm, Dn, Dd))

// DIV
#define FDIV_vector(Q, sz, Rm, Rn, Rd)  ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFDIVS(Sd, Sn, Sm)          EMIT(FDIV_vector(0, 0, Sm, Sn, Sd))
#define VFDIVQS(Sd, Sn, Sm)         EMIT(FDIV_vector(1, 0, Sm, Sn, Sd))
#define VFDIVQD(Sd, Sn, Sm)         EMIT(FDIV_vector(1, 1, Sm, Sn, Sd))

#define FDIV_scalar(type, Rm, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b0001<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FDIVS(Sd, Sn, Sm)           EMIT(FDIV_scalar(0b00, Sm, Sn, Sd))
#define FDIVD(Dd, Dn, Dm)           EMIT(FDIV_scalar(0b01, Dm, Dn, Dd))

#define FRECPE_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRECPES(Vd, Vn)           EMIT(FRECPE_vector(0, 0, Vn, Vd))
#define VFRECPEQS(Vd, Vn)          EMIT(FRECPE_vector(1, 0, Vn, Vd))
#define VFRECPEQD(Vd, Vn)          EMIT(FRECPE_vector(1, 0, Vn, Vd))

#define FRECPS_vector(Q, sz, Rm, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | 0<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFRECPSS(Vd, Vn, Vm)       EMIT(FRECPS_vector(0, 0, Vm, Vn, Vd))
#define VFRECPSQS(Vd, Vn, Vm)      EMIT(FRECPS_vector(1, 0, Vm, Vn, Vd))
#define VFRECPSQD(Vd, Vn, Vm)      EMIT(FRECPS_vector(1, 0, Vm, Vn, Vd))

// SQRT
#define FSQRT_vector(Q, sz, Rn, Rd)     ((Q)<<30 | 1<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFSQRTS(Sd, Sn)             EMIT(FSQRT_vector(0, 0, Sn, Sd))
#define VFSQRTQS(Sd, Sn)            EMIT(FSQRT_vector(1, 0, Sn, Sd))
#define VFSQRTQD(Sd, Sn)            EMIT(FSQRT_vector(1, 1, Sn, Sd))

#define FSQRT_scalar(type, Rn, Rd)      (0b11110<<24 | (type)<<22 | 1<<21 | 0b11<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FSQRTS(Sd, Sn)              EMIT(FSQRT_scalar(0b00, Sn, Sd))
#define FSQRTD(Dd, Dn)              EMIT(FSQRT_scalar(0b01, Dn, Dd))

#define FRSQRTE_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRSQRTES(Vd, Vn)           EMIT(FRSQRTE_vector(0, 0, Vn, Vd))
#define VFRSQRTEQS(Vd, Vn)          EMIT(FRSQRTE_vector(1, 0, Vn, Vd))
#define VFRSQRTEQD(Vd, Vn)          EMIT(FRSQRTE_vector(1, 0, Vn, Vd))

#define FRSQRTS_vector(Q, sz, Rm, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11111<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFRSQRTSS(Vd, Vn, Vm)       EMIT(FRSQRTS_vector(0, 0, Vm, Vn, Vd))
#define VFRSQRTSQS(Vd, Vn, Vm)      EMIT(FRSQRTS_vector(1, 0, Vm, Vn, Vd))
#define VFRSQRTSQD(Vd, Vn, Vm)      EMIT(FRSQRTS_vector(1, 0, Vm, Vn, Vd))

// CMP . NZCV: unordere=0011, eq=0110, inf=1000, sup=0010
#define FCMP_scalar(type, Rn, Rm, opc)  (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b1000<<10 | (Rn)<<5 | (opc)<<3)
#define FCMPS(Sn, Sm)              FEMIT(FCMP_scalar(0b00, Sn, Sm, 0b00))
#define FCMPD(Dn, Dm)              FEMIT(FCMP_scalar(0b01, Dn, Dm, 0b00))
#define FCMPS_0(Sn)                FEMIT(FCMP_scalar(0b00, Sn, 0, 0b01))
#define FCMPD_0(Dn)                FEMIT(FCMP_scalar(0b01, Dn, 0, 0b01))

// CVT
#define FCVT_scalar(sf, type, rmode, opcode, Rn, Rd)    ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (rmode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
// Floating-point Convert to Signed integer, rounding to nearest with ties to Away
#define FCVTASwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b100, Sn, Wd))
#define FCVTASxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b100, Sn, Xd))
#define FCVTASwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b100, Dn, Wd))
#define FCVTASxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b100, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding to nearest with ties to Away
#define FCVTAUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b101, Sn, Wd))
#define FCVTAUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b101, Sn, Xd))
#define FCVTAUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b101, Dn, Wd))
#define FCVTAUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b101, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Minus infinity
#define FCVTMSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b10, 0b100, Sn, Wd))
#define FCVTMSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b10, 0b100, Sn, Xd))
#define FCVTMSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b10, 0b100, Sn, Xd))
#define FCVTMSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b10, 0b100, Dn, Wd))
#define FCVTMSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b10, 0b100, Dn, Xd))
#define FCVTMSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b10, 0b100, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Minus infinity
#define FCVTMUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b10, 0b101, Sn, Wd))
#define FCVTMUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b10, 0b101, Sn, Xd))
#define FCVTMUxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b10, 0b101, Sn, Xd))
#define FCVTMUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b10, 0b101, Dn, Wd))
#define FCVTMUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b10, 0b101, Dn, Xd))
#define FCVTMUxwD(Xd, Dn)           EMIT(FCVT_scalar(rfex.w, 0b01, 0b10, 0b101, Dn, Xd))
// Floating-point Convert to Signed integer, rounding to nearest with ties to even
#define FCVTNSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b000, Sn, Wd))
#define FCVTNSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b000, Sn, Xd))
#define FCVTNSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b00, 0b000, Sn, Xd))
#define FCVTNSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b000, Dn, Wd))
#define FCVTNSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b000, Dn, Xd))
#define FCVTNSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b00, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding to nearest with ties to even
#define FCVTNUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b001, Sn, Wd))
#define FCVTNUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b001, Sn, Xd))
#define FCVTNUxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b00, 0b001, Sn, Xd))
#define FCVTNUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b001, Dn, Wd))
#define FCVTNUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b001, Dn, Xd))
#define FCVTNUxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b00, 0b001, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Plus infinity
#define FCVTPSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b01, 0b000, Sn, Wd))
#define FCVTPSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b01, 0b000, Sn, Xd))
#define FCVTPSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b01, 0b000, Sn, Xd))
#define FCVTPSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b01, 0b000, Dn, Wd))
#define FCVTPSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b01, 0b000, Dn, Xd))
#define FCVTPSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b01, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Plus infinity
#define FCVTPUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b01, 0b001, Sn, Wd))
#define FCVTPUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b01, 0b001, Sn, Xd))
#define FCVTPUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b01, 0b001, Dn, Wd))
#define FCVTPUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b01, 0b001, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Zero
#define FCVTZSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b11, 0b000, Sn, Wd))
#define FCVTZSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b11, 0b000, Sn, Xd))
#define FCVTZSxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b11, 0b000, Sn, Xd))
#define FCVTZSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b11, 0b000, Dn, Wd))
#define FCVTZSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b11, 0b000, Dn, Xd))
#define FCVTZSxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b11, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Zero
#define FCVTZUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b11, 0b001, Sn, Wd))
#define FCVTZUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b11, 0b001, Sn, Xd))
#define FCVTZUxwS(Xd, Sn)           EMIT(FCVT_scalar(rex.w, 0b00, 0b11, 0b001, Sn, Xd))
#define FCVTZUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b11, 0b001, Dn, Wd))
#define FCVTZUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b11, 0b001, Dn, Xd))
#define FCVTZUxwD(Xd, Dn)           EMIT(FCVT_scalar(rex.w, 0b01, 0b11, 0b001, Dn, Xd))

#define FCVT_vector_scalar(U, o2, sz, o1, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1110<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to Away
#define VFCVTASs(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 0, 0, Vn, Vd))
#define VFCVTASd(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 1, 0, Vn, Vd))
#define VFCVTAUs(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 0, 0, Vn, Vd))
#define VFCVTAUd(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Minus infinity
#define VFCVTMSs(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 0, 1, Vn, Vd))
#define VFCVTMSd(Vd, Vn)            EMIT(FCVT_vector_scalar(0, 0, 1, 1, Vn, Vd))
#define VFCVTMUs(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 0, 1, Vn, Vd))
#define VFCVTMUd(Vd, Vn)            EMIT(FCVT_vector_scalar(1, 0, 1, 1, Vn, Vd))

#define FCVT2_vector_scalar(U, o2, sz, o1, Rn, Rd)   (0b01<<30 | (U)<<29 | 0b11110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1101<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to even
#define VFCVTNSs(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 0, 0, 0, Vn, Vd))
#define VFCVTNSd(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 0, 1, 0, Vn, Vd))
#define VFCVTNUs(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 0, 0, 0, Vn, Vd))
#define VFCVTNUd(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Plus infinity
#define VFCVTPSs(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 0, 0, Vn, Vd))
#define VFCVTPSd(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 1, 0, Vn, Vd))
#define VFCVTPUs(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 0, 0, Vn, Vd))
#define VFCVTPUd(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Zero
#define VFCVTZSs(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 0, 1, Vn, Vd))
#define VFCVTZSd(Vd, Vn)            EMIT(FCVT2_vector_scalar(0, 1, 1, 1, Vn, Vd))
#define VFCVTZUs(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 0, 1, Vn, Vd))
#define VFCVTZUd(Vd, Vn)            EMIT(FCVT2_vector_scalar(1, 1, 1, 1, Vn, Vd))

#define FCVT_vector(Q, U, o2, sz, o1, Rn, Rd)       ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1110<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to Away
#define VFCVTASS(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 0, 0, Vn, Vd))
#define VFCVTASD(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 1, 0, Vn, Vd))
#define VFCVTASQS(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 0, 0, Vn, Vd))
#define VFCVTASQD(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 1, 0, Vn, Vd))
#define VFCVTAUS(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 0, 0, Vn, Vd))
#define VFCVTAUD(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 1, 0, Vn, Vd))
#define VFCVTAUQS(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 0, 0, Vn, Vd))
#define VFCVTAUQD(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Minus infinity
#define VFCVTMSS(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 0, 1, Vn, Vd))
#define VFCVTMSD(Vd, Vn)            EMIT(FCVT_vector(0, 0, 0, 1, 1, Vn, Vd))
#define VFCVTMSQS(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 0, 1, Vn, Vd))
#define VFCVTMSQD(Vd, Vn)           EMIT(FCVT_vector(1, 0, 0, 1, 1, Vn, Vd))
#define VFCVTMUS(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 0, 1, Vn, Vd))
#define VFCVTMUD(Vd, Vn)            EMIT(FCVT_vector(0, 1, 0, 1, 1, Vn, Vd))
#define VFCVTMUQS(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 0, 1, Vn, Vd))
#define VFCVTMUQD(Vd, Vn)           EMIT(FCVT_vector(1, 1, 0, 1, 1, Vn, Vd))

#define FCVT2_vector(Q, U, o2, sz, o1, Rn, Rd)       ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1101<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Floating-point Convert to (Un)signed integer, rounding to nearest with ties to even
#define VFCVTNSS(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 0, 0, 0, Vn, Vd))
#define VFCVTNSD(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 0, 1, 0, Vn, Vd))
#define VFCVTNSQS(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 0, 0, 0, Vn, Vd))
#define VFCVTNSQD(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 0, 1, 0, Vn, Vd))
#define VFCVTNUS(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 0, 0, 0, Vn, Vd))
#define VFCVTNUD(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 0, 1, 0, Vn, Vd))
#define VFCVTNUQS(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 0, 0, 0, Vn, Vd))
#define VFCVTNUQD(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 0, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Plus infinity
#define VFCVTPSS(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 0, 0, Vn, Vd))
#define VFCVTPSD(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 1, 0, Vn, Vd))
#define VFCVTPSQS(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 0, 0, Vn, Vd))
#define VFCVTPSQD(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 1, 0, Vn, Vd))
#define VFCVTPUS(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 0, 0, Vn, Vd))
#define VFCVTPUD(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 1, 0, Vn, Vd))
#define VFCVTPUQS(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 0, 0, Vn, Vd))
#define VFCVTPUQD(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 1, 0, Vn, Vd))
// Floating-point Convert to (Un)signed integer, rounding toward Zero
#define VFCVTZSS(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 0, 1, Vn, Vd))
#define VFCVTZSD(Vd, Vn)            EMIT(FCVT2_vector(0, 0, 1, 1, 1, Vn, Vd))
#define VFCVTZSQS(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 0, 1, Vn, Vd))
#define VFCVTZSQD(Vd, Vn)           EMIT(FCVT2_vector(1, 0, 1, 1, 1, Vn, Vd))
#define VFCVTZUS(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 0, 1, Vn, Vd))
#define VFCVTZUD(Vd, Vn)            EMIT(FCVT2_vector(0, 1, 1, 1, 1, Vn, Vd))
#define VFCVTZUQS(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 0, 1, Vn, Vd))
#define VFCVTZUQD(Vd, Vn)           EMIT(FCVT2_vector(1, 1, 1, 1, 1, Vn, Vd))

#define FCVT_precision(type, opc, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | 0b0001<<17 | (opc)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FCVT_D_S(Dd, Sn)            EMIT(FCVT_precision(0b00, 0b01, Sn, Dd))
#define FCVT_S_D(Sd, Dn)            EMIT(FCVT_precision(0b01, 0b00, Dn, Sd))

#define FCVTN_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 0<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10110<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert Vn from 2*Double to lower Vd as 2*float and clears the upper half, use FPCR rounding
#define FCVTN(Vd, Vn)               EMIT(FCVTN_vector(0, 1, Vn, Vd))
// Convert Vn from 2*Double to higher Vd as 2*float, use FPCR rounding
#define FCVTN2(Vd, Vn)              EMIT(FCVTN_vector(1, 1, Vn, Vd))
// Convert Vn from 2*Float to lower Vd as 2*float16 and clears the upper half, use FPCR rounding
#define FCVTN16(Vd, Vn)             EMIT(FCVTN_vector(0, 0, Vn, Vd))
// Convert Vn from 2*Float to higher Vd as 2*float16, use FPCR rounding
#define FCVTN162(Vd, Vn)            EMIT(FCVTN_vector(1, 0, Vn, Vd))

#define FCVTXN_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10110<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert Vn from 2*Double to lower Vd as 2*float and clears the upper half
#define FCVTXN(Vd, Vn)              EMIT(FCVTXN_vector(0, 1, Vn, Vd))
// Convert Vn from 2*Double to higher Vd as 2*float
#define FCVTXN2(Vd, Vn)             EMIT(FCVTXN_vector(1, 1, Vn, Vd))
// Convert Vn from 2*Float to lower Vd as 2*float16 and clears the upper half
#define FCVTXN16(Vd, Vn)            EMIT(FCVTXN_vector(0, 0, Vn, Vd))
// Convert Vn from 2*Float to higher Vd as 2*float16
#define FCVTXN162(Vd, Vn)           EMIT(FCVTXN_vector(1, 0, Vn, Vd))

#define FCVTL_vector(Q, sz, Rn, Rd)     ((Q)<<30 | 0<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert lower Vn from 2*float to Vd as 2*double
#define FCVTL(Vd, Vn)               EMIT(FCVTL_vector(0, 1, Vn, Vd))
// Convert higher Vn from 2*float to Vd as 2*double
#define FCVTL2(Vd, Vn)              EMIT(FCVTL_vector(1, 1, Vn, Vd))
// Convert lower Vn from 2*float16 to Vd as 2*float
#define FCVTL16(Vd, Vn)             EMIT(FCVTL_vector(0, 0, Vn, Vd))
// Convert higher Vn from 2*float16 to Vd as 2*float
#define FCVTL162(Vd, Vn)            EMIT(FCVTL_vector(1, 0, Vn, Vd))

#define SCVTF_scalar(sf, type, rmode, opcode, Rn, Rd)   ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (rmode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
#define SCVTFSw(Sd, Wn)             EMIT(SCVTF_scalar(0, 0b00, 0b00, 0b010, Wn, Sd))
#define SCVTFDw(Dd, Wn)             EMIT(SCVTF_scalar(0, 0b01, 0b00, 0b010, Wn, Dd))
#define SCVTFSx(Sd, Xn)             EMIT(SCVTF_scalar(1, 0b00, 0b00, 0b010, Xn, Sd))
#define SCVTFDx(Dd, Xn)             EMIT(SCVTF_scalar(1, 0b01, 0b00, 0b010, Xn, Dd))

#define SCVTF_vector_scalar(U, sz, Rn, Rd)    (1<<30 | (U)<<29 | 0b11110<<24 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert Vn from i32 to Vd float
#define SCVTFSS(Vd, Vn)             EMIT(SCVTF_vector_scalar(0, 0, Vn, Vd))
// Convert Vn from i64 to Vd double
#define SCVTFDD(Vd, Vn)             EMIT(SCVTF_vector_scalar(0, 1, Vn, Vd))

#define SCVTF_vector(Q, U, sz, Rn, Rd)      ((Q)<<30 | (U)<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SCVTFS(Vd, Vn)              EMIT(SCVTF_vector(0, 0, 0, Vn, Vd))
#define SCVTFD(Vd, Vn)              EMIT(SCVTF_vector(0, 0, 1, Vn, Vd))
#define SCVTQFS(Vd, Vn)             EMIT(SCVTF_vector(1, 0, 0, Vn, Vd))
#define SCVTQFD(Vd, Vn)             EMIT(SCVTF_vector(1, 0, 1, Vn, Vd))

// FRINTI Floating-point Round to Integral, using current rounding mode from FPCR (vector).
#define FRINT_vector(Q, U, o2, sz, o1, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22 | 0b10000<<17 | 0b1100<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRINTIS(Vd,Vn)             EMIT(FRINT_vector(0, 1, 1, 0, 1, Vn, Vd))
#define VFRINTISQ(Vd,Vn)            EMIT(FRINT_vector(1, 1, 1, 0, 1, Vn, Vd))
#define VFRINTIDQ(Vd,Vn)            EMIT(FRINT_vector(1, 1, 1, 1, 1, Vn, Vd))
// round with mode, mode is 0 = TieEven, 1=+inf, 2=-inf, 3=zero
#define VFRINTRDQ(Vd,Vn, mode)      EMIT(FRINT_vector(1, 0, (mode)&1, 1, ((mode)>>1)&1, Vn, Vd))
// round with mode, mode is 0 = TieEven, 1=+inf, 2=-inf, 3=zero
#define VFRINTRSQ(Vd,Vn, mode)      EMIT(FRINT_vector(1, 0, (mode)&1, 0, ((mode)>>1)&1, Vn, Vd))
#define VFRINTRS(Vd, Vn, mode)      EMIT(FRINT_vector(0, 0, (mode)&1, 0, ((mode)>>1)&1, Vn, Vd))

#define FRINTI_scalar(type, Rn, Rd)  (0b11110<<24 | (type)<<22 | 1<<21 | 0b001<<18 | 0b111<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FRINTIS(Sd, Sn)             EMIT(FRINTI_scalar(0b00, Sn, Sd))
#define FRINTID(Dd, Dn)             EMIT(FRINTI_scalar(0b01, Dn, Dd))

#define FRINT_scalar(type, rmode, Rn, Rd)   (0b11110<<24 | (type)<<22 | 1<<21 | 0b001<<18 | (rmode)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
// round toward 0 (truncate)
#define FRINTZS(Sd, Sn)             EMIT(FRINT_scalar(0b00, 0b011, Sn, Sd))
// round toward 0 (truncate)
#define FRINTZD(Sd, Sn)             EMIT(FRINT_scalar(0b01, 0b011, Sn, Sd))
// round with current FPCR mode
#define FRINTXS(Sd, Sn)             EMIT(FRINT_scalar(0b00, 0b110, Sn, Sd))
// round with current FPCR mode
#define FRINTXD(Sd, Sn)             EMIT(FRINT_scalar(0b01, 0b110, Sn, Sd))
// round with mode, mode is 0 = TieEven, 1=+inf, 2=-inf, 3=zero
#define FRINTRRS(Sd, Sn, mode)      EMIT(FRINT_scalar(0b00, ((mode)&3), Sn, Sd))
// round with mode, mode is 0 = TieEven, 1=+inf, 2=-inf, 3=zero
#define FRINTRRD(Dd, Dn, mode)      EMIT(FRINT_scalar(0b01, ((mode)&3), Dn, Dd))

// FMAX / FMIN
#define FMINMAX_vector(Q, U, o1, sz, Rm, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o1)<<23 | (sz)<<22 | 0b1<<21 | (Rm)<<16 | 0b11110<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFMINS(Vd, Vn, Vm)          EMIT(FMINMAX_vector(0, 0, 1, 0, Vm, Vn, Vd))
#define VFMAXS(Vd, Vn, Vm)          EMIT(FMINMAX_vector(0, 0, 0, 0, Vm, Vn, Vd))
#define VFMINQS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 1, 0, Vm, Vn, Vd))
#define VFMAXQS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 0, 0, Vm, Vn, Vd))
#define VFMINQD(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 1, 1, Vm, Vn, Vd))
#define VFMAXQD(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 0, 1, Vm, Vn, Vd))
#define VFMINPS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(0, 1, 1, 0, Vm, Vn, Vd))
#define VFMAXPS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(0, 1, 0, 0, Vm, Vn, Vd))
#define VFMINPQS(Vd, Vn, Vm)        EMIT(FMINMAX_vector(1, 1, 1, 0, Vm, Vn, Vd))
#define VFMAXPQS(Vd, Vn, Vm)        EMIT(FMINMAX_vector(1, 1, 0, 0, Vm, Vn, Vd))
#define VFMINPQD(Vd, Vn, Vm)        EMIT(FMINMAX_vector(1, 1, 1, 1, Vm, Vn, Vd))
#define VFMAXPQD(Vd, Vn, Vm)        EMIT(FMINMAX_vector(1, 1, 0, 1, Vm, Vn, Vd))

#define FMINMAX_scalar(type, Rm, op, Rn, Rd)        (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b01<<14 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define FMINS(Sd, Sn, Sm)           EMIT(FMINMAX_scalar(0b00, Sm, 0b01, Sn, Sd))
#define FMIND(Dd, Dn, Dm)           EMIT(FMINMAX_scalar(0b01, Dm, 0b01, Dn, Dd))
#define FMAXS(Sd, Sn, Sm)           EMIT(FMINMAX_scalar(0b00, Sm, 0b00, Sn, Sd))
#define FMAXD(Dd, Dn, Dm)           EMIT(FMINMAX_scalar(0b01, Dm, 0b00, Dn, Dd))
// FMINNM NaN vs Number: number is picked
#define FMINNMS(Sd, Sn, Sm)         EMIT(FMINMAX_scalar(0b00, Sm, 0b11, Sn, Sd))
// FMINNM NaN vs Number: number is picked
#define FMINNMD(Dd, Dn, Dm)         EMIT(FMINMAX_scalar(0b01, Dm, 0b11, Dn, Dd))
// FMAXNM NaN vs Number: number is picked
#define FMAXNMS(Sd, Sn, Sm)         EMIT(FMINMAX_scalar(0b00, Sm, 0b10, Sn, Sd))
// FMAXNM NaN vs Number: number is picked
#define FMAXNMD(Dd, Dn, Dm)         EMIT(FMINMAX_scalar(0b01, Dm, 0b10, Dn, Dd))

// Fused Add Multiply
#define FMADD_gen(type, o1, Rm, o0, Ra, Rn, Rd) (0b11111<<24 | (type)<<22 | (o1)<<21 | (Rm)<<16 | (o0)<<15 | (Ra)<<10 | (Rn)<<5 | (Rd))
// scalar Rd = Ra + Rn*Rm
#define FMADD_32(Sd, Sa, Sn, Sm)    EMIT(FMADD_gen(0b00, 0, Sm, 0, Sa, Sn, Sd))
// scalar Rd = Ra + Rn*Rm
#define FMADD_64(Dd, Da, Dn, Dm)    EMIT(FMADD_gen(0b01, 0, Dm, 0, Da, Dn, Dd))
// scalar Rd = -Ra - Rn*Rm
#define FNMADD_32(Sd, Sa, Sn, Sm)   EMIT(FMADD_gen(0b00, 1, Sm, 0, Sa, Sn, Sd))
// scalar Rd = -Ra - Rn*Rm
#define FNMADD_64(Dd, Da, Dn, Dm)   EMIT(FMADD_gen(0b01, 1, Dm, 0, Da, Dn, Dd))
// scalar Rd = Ra - Rn*Rm
#define FMSUB_32(Sd, Sa, Sn, Sm)    EMIT(FMADD_gen(0b00, 0, Sm, 1, Sa, Sn, Sd))
// scalar Rd = Ra - Rn*Rm
#define FMSUB_64(Dd, Da, Dn, Dm)    EMIT(FMADD_gen(0b01, 0, Dm, 1, Da, Dn, Dd))
// scalar Rd = -Ra + Rn*Rm
#define FNMSUB_32(Sd, Sa, Sn, Sm)   EMIT(FMADD_gen(0b00, 1, Sm, 1, Sa, Sn, Sd))
// scalar Rd = -Ra + Rn*Rm
#define FNMSUB_64(Dd, Da, Dn, Dm)   EMIT(FMADD_gen(0b01, 1, Dm, 1, Da, Dn, Dd))

#define FMLA_vector(Q, op, sz, Rm, Rn, Rd)  ((Q)<<30 | 0b01110<<24 | (op)<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11001<<11 | 1<<10 | (Rn)<<5 | (Rd))
// Vd += Vn*Vm
#define VFMLAS(Vd, Vn, Vm)          EMIT(FMLA_vector(0, 0, 0, Vm, Vn, Vd))
// Vd += Vn*Vm
#define VFMLAQS(Vd, Vn, Vm)         EMIT(FMLA_vector(1, 0, 0, Vm, Vn, Vd))
// Vd += Vn*Vm
#define VFMLAQD(Vd, Vn, Vm)         EMIT(FMLA_vector(1, 0, 1, Vm, Vn, Vd))
// Vd -= Vn*Vm
#define VFMLSQS(Vd, Vn, Vm)         EMIT(FMLA_vector(1, 1, 0, Vm, Vn, Vd))
// Vd -= Vn*Vm
#define VFMLSQD(Vd, Vn, Vm)         EMIT(FMLA_vector(1, 1, 1, Vm, Vn, Vd))

// ZIP / UZP
#define ZIP_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b11<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VZIP1Q_8(Rt, Rn, Rm)        EMIT(ZIP_gen(1, 0b00, Rm, 0, Rn, Rt))
#define VZIP2Q_8(Rt, Rn, Rm)        EMIT(ZIP_gen(1, 0b00, Rm, 1, Rn, Rt))
#define VZIP1_8(Rt, Rn, Rm)         EMIT(ZIP_gen(0, 0b00, Rm, 0, Rn, Rt))
#define VZIP2_8(Rt, Rn, Rm)         EMIT(ZIP_gen(0, 0b00, Rm, 1, Rn, Rt))
#define VZIP1Q_16(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b01, Rm, 0, Rn, Rt))
#define VZIP2Q_16(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b01, Rm, 1, Rn, Rt))
#define VZIP1_16(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b01, Rm, 0, Rn, Rt))
#define VZIP2_16(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b01, Rm, 1, Rn, Rt))
#define VZIP1Q_32(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b10, Rm, 0, Rn, Rt))
#define VZIP2Q_32(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b10, Rm, 1, Rn, Rt))
#define VZIP1_32(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b10, Rm, 0, Rn, Rt))
#define VZIP2_32(Rt, Rn, Rm)        EMIT(ZIP_gen(0, 0b10, Rm, 1, Rn, Rt))
#define VZIP1Q_64(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b11, Rm, 0, Rn, Rt))
#define VZIP2Q_64(Rt, Rn, Rm)       EMIT(ZIP_gen(1, 0b11, Rm, 1, Rn, Rt))

#define UZP_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b01<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VUZP1Q_8(Rt, Rn, Rm)        EMIT(UZP_gen(1, 0b00, Rm, 0, Rn, Rt))
#define VUZP2Q_8(Rt, Rn, Rm)        EMIT(UZP_gen(1, 0b00, Rm, 1, Rn, Rt))
#define VUZP1_8(Rt, Rn, Rm)         EMIT(UZP_gen(0, 0b00, Rm, 0, Rn, Rt))
#define VUZP2_8(Rt, Rn, Rm)         EMIT(UZP_gen(0, 0b00, Rm, 1, Rn, Rt))
#define VUZP1Q_16(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b01, Rm, 0, Rn, Rt))
#define VUZP2Q_16(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b01, Rm, 1, Rn, Rt))
#define VUZP1_16(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b01, Rm, 0, Rn, Rt))
#define VUZP2_16(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b01, Rm, 1, Rn, Rt))
#define VUZP1Q_32(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b10, Rm, 0, Rn, Rt))
#define VUZP2Q_32(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b10, Rm, 1, Rn, Rt))
#define VUZP1_32(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b10, Rm, 0, Rn, Rt))
#define VUZP2_32(Rt, Rn, Rm)        EMIT(UZP_gen(0, 0b10, Rm, 1, Rn, Rt))
#define VUZP1Q_64(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b11, Rm, 0, Rn, Rt))
#define VUZP2Q_64(Rt, Rn, Rm)       EMIT(UZP_gen(1, 0b11, Rm, 1, Rn, Rt))

#define BITBIF_gen(Q, opc2, Rm, Rn, Rd) ((Q)<<30 | 0b101110<<24 | (opc2)<<22 | 1<<21 | (Rm)<<16 | 0b000111<<10 | (Rn)<<5 | (Rd))
// Bitwise insert Vn in Vd if Vm is "0"
#define VBIF(Vd, Vn,Vm)             EMIT(BITBIF_gen(0, 0b11, Vm, Vn, Vd))
// Bitwise insert Vn in Vd if Vm is "0"
#define VBIFQ(Vd, Vn,Vm)            EMIT(BITBIF_gen(1, 0b11, Vm, Vn, Vd))
// Bitwise insert Vn in Vd if Vm is "1"
#define VBIT(Vd, Vn,Vm)             EMIT(BITBIF_gen(0, 0b10, Vm, Vn, Vd))
// Bitwise insert Vn in Vd if Vm is "1"
#define VBITQ(Vd, Vn,Vm)            EMIT(BITBIF_gen(1, 0b10, Vm, Vn, Vd))

#define DUP_element(Q, imm5, Rn, Rd)    ((Q)<<30 | 0b01110000<<21 | (imm5)<<16 | 1<<10 | (Rn)<<5 | (Rd))
#define VDUP_8(Vd, Vn, idx)         EMIT(DUP_element(0, ((idx)<<1|1), Vn, Vd))
#define VDUPQ_8(Vd, Vn, idx)        EMIT(DUP_element(1, ((idx)<<1|1), Vn, Vd))
#define VDUP_16(Vd, Vn, idx)        EMIT(DUP_element(0, ((idx)<<2|0b10), Vn, Vd))
#define VDUPQ_16(Vd, Vn, idx)       EMIT(DUP_element(1, ((idx)<<2|0b10), Vn, Vd))
#define VDUP_32(Vd, Vn, idx)        EMIT(DUP_element(0, ((idx)<<3|0b100), Vn, Vd))
#define VDUPQ_32(Vd, Vn, idx)       EMIT(DUP_element(1, ((idx)<<3|0b100), Vn, Vd))
#define VDUPQ_64(Vd, Vn, idx)       EMIT(DUP_element(1, ((idx)<<4|0b1000), Vn, Vd))

#define DUP_general(Q, imm5, Rn, Rd)    ((Q)<<30 | 0b01110000<<21 | (imm5)<<16 | 0b11<<10 | (Rn)<<5 | (Rd))
#define VDUPB(Vd, Wn)         EMIT(DUP_general(0, 0b1, Wn, Vd))
#define VDUPQB(Vd, Wn)        EMIT(DUP_general(1, 0b1, Wn, Vd))
#define VDUPH(Vd, Wn)        EMIT(DUP_general(0, 0b10, Wn, Vd))
#define VDUPQH(Vd, Wn)       EMIT(DUP_general(1, 0b10, Wn, Vd))
#define VDUPS(Vd, Wn)        EMIT(DUP_general(0, 0b100, Wn, Vd))
#define VDUPQS(Vd, Wn)       EMIT(DUP_general(1, 0b100, Wn, Vd))
#define VDUPQD(Vd, Xn)       EMIT(DUP_general(1, 0b1000, Xn, Vd))

// TBL
#define TBL_gen(Q, Rm, len, op, Rn, Rd) ((Q)<<30 | 0b001110<<24 | (Rm)<<16 | (len)<<13 | (op)<<12 | (Rn)<<5 | (Rd))
//Use Rm[] to pick from Rn element and store in Rd. Out-of-range element gets 0
#define VTBLQ1_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b00, 0, Rn, Rd))
#define VTBL1_8(Rd, Rn, Rm)         EMIT(TBL_gen(0, Rm, 0b00, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1 element and store in Rd. Out-of-range element gets 0
#define VTBLQ2_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b01, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2 element and store in Rd. Out-of-range element gets 0
#define VTBLQ3_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b10, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2, Rn+3 element and store in Rd. Out-of-range element gets 0
#define VTBLQ4_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b11, 0, Rn, Rd))
//Use Rm[] to pick from Rn element and store in Rd. Out-of-range element stay untouched
#define VTBXQ1_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b00, 1, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ2_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b01, 1, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ3_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b10, 1, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2, Rn+3 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ4_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b11, 1, Rn, Rd))

// TRN
#define TRN_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b10<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VTRNQ1_64(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b11, Vm, 0, Vn, Vd))
#define VTRNQ1_32(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b10, Vm, 0, Vn, Vd))
#define VTRNQ1_16(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b01, Vm, 0, Vn, Vd))
#define VTRNQ1_8(Vd, Vn, Vm)        EMIT(TRN_gen(1, 0b00, Vm, 0, Vn, Vd))
#define VTRNQ2_64(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b11, Vm, 1, Vn, Vd))
#define VTRNQ2_32(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b10, Vm, 1, Vn, Vd))
#define VTRNQ2_16(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b01, Vm, 1, Vn, Vd))
#define VTRNQ2_8(Vd, Vn, Vm)        EMIT(TRN_gen(1, 0b00, Vm, 1, Vn, Vd))

// QXTN / QXTN2
#define QXTN_scalar(U, size, Rn, Rd)        (0b01<<30 | (U)<<29 | 0b11110<<24 | (size)<<22 | 0b10000<<17 | 0b10100<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Signed saturating extract Narrow, from D to S
#define SQXTN_S_D(Sd, Dn)           EMIT(QXTN_scalar(0, 0b10, Dn, Sd))
// Signed saturating extract Narrow, from S to H
#define SQXTN_H_S(Hd, Sn)           EMIT(QXTN_scalar(0, 0b01, Sn, Hd))
// Signed saturating extract Narrow, from H to B
#define SQXTN_B_H(Bd, Hn)           EMIT(QXTN_scalar(0, 0b00, Hn, Bd))

#define QXTN_vector(Q, U, size, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b10100<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 64->32 with Signed saturation and fit lower part of Rd
#define SQXTN_32(Rd, Rn)            EMIT(QXTN_vector(0, 0, 0b10, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 64->32 with Signed saturation and fit higher part of Rd
#define SQXTN2_32(Rd, Rn)           EMIT(QXTN_vector(1, 0, 0b10, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 32->16 with Signed saturation and fit lower part of Rd
#define SQXTN_16(Rd, Rn)            EMIT(QXTN_vector(0, 0, 0b01, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 32->16 with Signed saturation and fit higher part of Rd
#define SQXTN2_16(Rd, Rn)           EMIT(QXTN_vector(1, 0, 0b01, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 16->8 with Signed saturation and fit lower part of Rd
#define SQXTN_8(Rd, Rn)             EMIT(QXTN_vector(0, 0, 0b00, Rn, Rd))
// Signed saturating extract Narrow, takes Rn element and reduce 16->8 with Signed saturation and fit higher part of Rd
#define SQXTN2_8(Rd, Rn)            EMIT(QXTN_vector(1, 0, 0b00, Rn, Rd))
// Unsigned saturating Extract Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit lower part of Rd
#define UQXTN_32(Rd, Rn)            EMIT(QXTN_vector(0, 1, 0b10, Rn, Rd))
// Unsigned saturating Extract Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit higher part of Rd
#define UQXTN2_32(Rd, Rn)           EMIT(QXTN_vector(1, 1, 0b10, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit lower part of Rd
#define UQXTN_16(Rd, Rn)            EMIT(QXTN_vector(0, 1, 0b01, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit higher part of Rd
#define UQXTN2_16(Rd, Rn)           EMIT(QXTN_vector(1, 1, 0b01, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit lower part of Rd
#define UQXTN_8(Rd, Rn)             EMIT(QXTN_vector(0, 1, 0b00, Rn, Rd))
// Unsigned saturating extract Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit higher part of Rd
#define UQXTN2_8(Rd, Rn)            EMIT(QXTN_vector(1, 1, 0b00, Rn, Rd))

#define QXTUN_vector(Q, U, size, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b10010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_32(Rd, Rn)           EMIT(QXTUN_vector(0, 1, 0b10, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_32(Rd, Rn)          EMIT(QXTUN_vector(1, 1, 0b10, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_16(Rd, Rn)           EMIT(QXTUN_vector(0, 1, 0b01, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_16(Rd, Rn)          EMIT(QXTUN_vector(1, 1, 0b01, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_8(Rd, Rn)            EMIT(QXTUN_vector(0, 1, 0b00, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_8(Rd, Rn)           EMIT(QXTUN_vector(1, 1, 0b00, Rn, Rd))

#define XTN_vector(Q, size, Rn, Rd) ((Q)<<30 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b10010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Xtract narrow to X bits
#define XTN_8(Vd, Vn)               EMIT(XTN_vector(0, 0b00, Vn, Vd))
#define XTN_16(Vd, Vn)              EMIT(XTN_vector(0, 0b01, Vn, Vd))
#define XTN_32(Vd, Vn)              EMIT(XTN_vector(0, 0b10, Vn, Vd))
#define XTN2_8(Vd, Vn)              EMIT(XTN_vector(1, 0b00, Vn, Vd))
#define XTN2_16(Vd, Vn)             EMIT(XTN_vector(1, 0b01, Vn, Vd))
#define XTN2_32(Vd, Vn)             EMIT(XTN_vector(1, 0b10, Vn, Vd))

// Integer CMP
// EQual
#define CMEQ_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10001<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VCMEQ_8(Rd, Rn, Rm)         EMIT(CMEQ_vector(0, 1, 0b00, Rm, Rn, Rd))
#define VCMEQ_16(Rd, Rn, Rm)        EMIT(CMEQ_vector(0, 1, 0b01, Rm, Rn, Rd))
#define VCMEQ_32(Rd, Rn, Rm)        EMIT(CMEQ_vector(0, 1, 0b10, Rm, Rn, Rd))
#define VCMEQQ_8(Rd, Rn, Rm)        EMIT(CMEQ_vector(1, 1, 0b00, Rm, Rn, Rd))
#define VCMEQQ_16(Rd, Rn, Rm)       EMIT(CMEQ_vector(1, 1, 0b01, Rm, Rn, Rd))
#define VCMEQQ_32(Rd, Rn, Rm)       EMIT(CMEQ_vector(1, 1, 0b10, Rm, Rn, Rd))
#define VCMEQQ_64(Rd, Rn, Rm)       EMIT(CMEQ_vector(1, 1, 0b11, Rm, Rn, Rd))
// Greater test
#define CMG_vector(Q, U, size, eq, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b0011<<12 | (eq)<<11 | 1<<10 | (Rn)<<5 | (Rd))
// Signed Greater or Equal
#define VCMGEQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 0, 0b00, 1, Rm, Rn, Rd))
#define VCMGEQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b01, 1, Rm, Rn, Rd))
#define VCMGEQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b10, 1, Rm, Rn, Rd))
#define VCMGEQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b11, 1, Rm, Rn, Rd))
// Unsigned Higher or Same
#define VCMHSQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 1, 0b00, 1, Rm, Rn, Rd))
#define VCMHSQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b01, 1, Rm, Rn, Rd))
#define VCMHSQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b10, 1, Rm, Rn, Rd))
#define VCMHSQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b11, 1, Rm, Rn, Rd))
// Signed Greater Than
#define VCMGTQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 0, 0b00, 0, Rm, Rn, Rd))
#define VCMGTQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b01, 0, Rm, Rn, Rd))
#define VCMGTQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b10, 0, Rm, Rn, Rd))
#define VCMGTQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 0, 0b11, 0, Rm, Rn, Rd))
#define VCMGT_8(Rd, Rn, Rm)         EMIT(CMG_vector(0, 0, 0b00, 0, Rm, Rn, Rd))
#define VCMGT_16(Rd, Rn, Rm)        EMIT(CMG_vector(0, 0, 0b01, 0, Rm, Rn, Rd))
#define VCMGT_32(Rd, Rn, Rm)        EMIT(CMG_vector(0, 0, 0b10, 0, Rm, Rn, Rd))
// Unsigned Higher
#define VCHIQQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 1, 0b00, 0, Rm, Rn, Rd))
#define VCHIQQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b01, 0, Rm, Rn, Rd))
#define VCHIQQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b10, 0, Rm, Rn, Rd))
#define VCHIQQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b11, 0, Rm, Rn, Rd))

// Less Than 0
#define CMLT_0_vector(Q, size, Rn, Rd)      ((Q)<<30 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b01010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define CMLT_0_8(Rd, Rn)            EMIT(CMLT_0_vector(0, 0b00, Rn, Rd))
#define CMLT_0_16(Rd, Rn)           EMIT(CMLT_0_vector(0, 0b01, Rn, Rd))
#define CMLT_0_32(Rd, Rn)           EMIT(CMLT_0_vector(0, 0b10, Rn, Rd))
#define CMLTQ_0_8(Rd, Rn)           EMIT(CMLT_0_vector(1, 0b00, Rn, Rd))
#define CMLTQ_0_16(Rd, Rn)          EMIT(CMLT_0_vector(1, 0b01, Rn, Rd))
#define CMLTQ_0_32(Rd, Rn)          EMIT(CMLT_0_vector(1, 0b10, Rn, Rd))
#define CMLTQ_0_64(Rd, Rn)          EMIT(CMLT_0_vector(1, 0b11, Rn, Rd))
// Equal 0
#define CMEQ_0_vector(Q, size, Rn, Rd)          ((Q)<<30 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b0100<<13 | 1<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define CMEQ_0_8(Rd, Rn)            EMIT(CMEQ_0_vector(0, 0b00, Rn, Rd))
#define CMEQ_0_16(Rd, Rn)           EMIT(CMEQ_0_vector(0, 0b01, Rn, Rd))
#define CMEQ_0_32(Rd, Rn)           EMIT(CMEQ_0_vector(0, 0b10, Rn, Rd))
#define CMEQQ_0_8(Rd, Rn)           EMIT(CMEQ_0_vector(1, 0b00, Rn, Rd))
#define CMEQQ_0_16(Rd, Rn)          EMIT(CMEQ_0_vector(1, 0b01, Rn, Rd))
#define CMEQQ_0_32(Rd, Rn)          EMIT(CMEQ_0_vector(1, 0b10, Rn, Rd))
#define CMEQQ_0_64(Rd, Rn)          EMIT(CMEQ_0_vector(1, 0b11, Rn, Rd))
// Greater Than 0
#define CMCond_0_vector(Q, U, size, op, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b0100<<13 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define CMGT_0_8(Rd, Rn)            EMIT(CMCond_0_vector(0, 0, 0b00, 0, Rn, Rd))
#define CMGT_0_16(Rd, Rn)           EMIT(CMCond_0_vector(0, 0, 0b01, 0, Rn, Rd))
#define CMGT_0_32(Rd, Rn)           EMIT(CMCond_0_vector(0, 0, 0b10, 0, Rn, Rd))
#define CMGTQ_0_8(Rd, Rn)           EMIT(CMCond_0_vector(1, 0, 0b00, 0, Rn, Rd))
#define CMGTQ_0_16(Rd, Rn)          EMIT(CMCond_0_vector(1, 0, 0b01, 0, Rn, Rd))
#define CMGTQ_0_32(Rd, Rn)          EMIT(CMCond_0_vector(1, 0, 0b10, 0, Rn, Rd))
#define CMGTQ_0_64(Rd, Rn)          EMIT(CMCond_0_vector(1, 0, 0b11, 0, Rn, Rd))

// Vector Float CMP
// EQual
#define FCMP_vector(Q, U, E, sz, Rm, ac, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (E)<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b1110<<12 | (ac)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFCMEQQD(Rd, Rn, Rm)         EMIT(FCMP_vector(1, 0, 0, 1, Rm, 0, Rn, Rd))
#define VFCMEQQS(Rd, Rn, Rm)         EMIT(FCMP_vector(1, 0, 0, 0, Rm, 0, Rn, Rd))
// Greater or Equal
#define VFCMGEQD(Rd, Rn, Rm)         EMIT(FCMP_vector(1, 1, 0, 1, Rm, 0, Rn, Rd))
#define VFCMGEQS(Rd, Rn, Rm)         EMIT(FCMP_vector(1, 1, 0, 0, Rm, 0, Rn, Rd))
#define VFCMGEQD_ABS(Rd, Rn, Rm)     EMIT(FCMP_vector(1, 1, 0, 1, Rm, 1, Rn, Rd))
#define VFCMGEQS_ABS(Rd, Rn, Rm)     EMIT(FCMP_vector(1, 1, 0, 0, Rm, 1, Rn, Rd))
// Greater Than
#define VFCMGTQD(Rd, Rn, Rm)         EMIT(FCMP_vector(1, 1, 1, 1, Rm, 0, Rn, Rd))
#define VFCMGTQS(Rd, Rn, Rm)         EMIT(FCMP_vector(1, 1, 1, 0, Rm, 0, Rn, Rd))
#define VFCMGTQD_ABS(Rd, Rn, Rm)     EMIT(FCMP_vector(1, 1, 1, 1, Rm, 1, Rn, Rd))
#define VFCMGTQS_ABS(Rd, Rn, Rm)     EMIT(FCMP_vector(1, 1, 1, 0, Rm, 1, Rn, Rd))

// Scalar Float CMP to 0
#define FCMP_0_scalar(U, sz, op, Rn, Rd) (0b01<<30 | (U)<<29| 0b11110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b011<<14 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Less or equal to 0
#define FCMLES_0(Rd, Rn)             EMIT(FCMP_0_scalar(1, 0, 0b01, (Rn), (Rd)))
#define FCMLED_0(Rd, Rn)             EMIT(FCMP_0_scalar(1, 1, 0b01, (Rn), (Rd)))
// Greater than 0
#define FCMGTS_0(Rd, Rn)             EMIT(FCMP_0_scalar(0, 0, 0b00, (Rn), (Rd)))
#define FCMGTD_0(Rd, Rn)             EMIT(FCMP_0_scalar(0, 1, 0b00, (Rn), (Rd)))
// Less than 0
#define FCMLTS_0(Rd, Rn)             EMIT(FCMP_0_scalar(0, 0, 0b10, (Rn), (Rd)))
#define FCMLTD_0(Rd, Rn)             EMIT(FCMP_0_scalar(0, 1, 0b10, (Rn), (Rd)))

// Scalar Float CMEQ
#define FCMP_op_scalar(U, E, sz, Rm, ac, Rn, Rd)    (0b01<<30 | (U)<<29 | 0b11110<<24 | (E)<<23 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b1110<<12 | (ac)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define FCMEQS(Rd, Rn, Rm)          EMIT(FCMP_op_scalar(0, 0, 0, (Rm), 0, (Rn), (Rd)))
#define FCMEQD(Rd, Rn, Rm)          EMIT(FCMP_op_scalar(0, 0, 1, (Rm), 0, (Rn), (Rd)))

// UMULL / SMULL
#define MULL_vector(Q, U, size, Rm, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b1100<<12 |(Rn)<<5 |(Rd))
#define VUMULL_8(Rd, Rn, Rm)        EMIT(MULL_vector(0, 1, 0b00, Rm, Rn, Rd))
#define VUMULL_16(Rd, Rn, Rm)       EMIT(MULL_vector(0, 1, 0b01, Rm, Rn, Rd))
#define VUMULL_32(Rd, Rn, Rm)       EMIT(MULL_vector(0, 1, 0b10, Rm, Rn, Rd))
#define VUMULL2_8(Rd, Rn, Rm)       EMIT(MULL_vector(1, 1, 0b00, Rm, Rn, Rd))
#define VUMULL2_16(Rd, Rn, Rm)      EMIT(MULL_vector(1, 1, 0b01, Rm, Rn, Rd))
#define VUMULL2_32(Rd, Rn, Rm)      EMIT(MULL_vector(1, 1, 0b10, Rm, Rn, Rd))
#define VSMULL_8(Rd, Rn, Rm)        EMIT(MULL_vector(0, 0, 0b00, Rm, Rn, Rd))
#define VSMULL_16(Rd, Rn, Rm)       EMIT(MULL_vector(0, 0, 0b01, Rm, Rn, Rd))
#define VSMULL_32(Rd, Rn, Rm)       EMIT(MULL_vector(0, 0, 0b10, Rm, Rn, Rd))
#define VSMULL2_8(Rd, Rn, Rm)       EMIT(MULL_vector(1, 0, 0b00, Rm, Rn, Rd))
#define VSMULL2_16(Rd, Rn, Rm)      EMIT(MULL_vector(1, 0, 0b01, Rm, Rn, Rd))
#define VSMULL2_32(Rd, Rn, Rm)      EMIT(MULL_vector(1, 0, 0b10, Rm, Rn, Rd))

// MUL
#define MUL_vector(Q, size, Rm, Rn, Rd)     ((Q)<<30 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10011<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMUL_8(Vd, Vn, Vm)          EMIT(MUL_vector(0, 0b00, Vm, Vn, Vd))
#define VMUL_16(Vd, Vn, Vm)         EMIT(MUL_vector(0, 0b01, Vm, Vn, Vd))
#define VMUL_32(Vd, Vn, Vm)         EMIT(MUL_vector(0, 0b10, Vm, Vn, Vd))
#define VMULQ_8(Vd, Vn, Vm)         EMIT(MUL_vector(1, 0b00, Vm, Vn, Vd))
#define VMULQ_16(Vd, Vn, Vm)        EMIT(MUL_vector(1, 0b01, Vm, Vn, Vd))
#define VMULQ_32(Vd, Vn, Vm)        EMIT(MUL_vector(1, 0b10, Vm, Vn, Vd))

// (S/Q)ADD
#define QADD_vector(Q, U, size, Rm, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b00001<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SQADDQ_8(Vd, Vn, Vm)        EMIT(QADD_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SQADDQ_16(Vd, Vn, Vm)       EMIT(QADD_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SQADDQ_32(Vd, Vn, Vm)       EMIT(QADD_vector(1, 0, 0b10, Vm, Vn, Vd))
#define SQADDQ_64(Vd, Vn, Vm)       EMIT(QADD_vector(1, 0, 0b11, Vm, Vn, Vd))
#define UQADDQ_8(Vd, Vn, Vm)        EMIT(QADD_vector(1, 1, 0b00, Vm, Vn, Vd))
#define UQADDQ_16(Vd, Vn, Vm)       EMIT(QADD_vector(1, 1, 0b01, Vm, Vn, Vd))
#define UQADDQ_32(Vd, Vn, Vm)       EMIT(QADD_vector(1, 1, 0b10, Vm, Vn, Vd))
#define UQADDQ_64(Vd, Vn, Vm)       EMIT(QADD_vector(1, 1, 0b11, Vm, Vn, Vd))
#define SQADD_8(Vd, Vn, Vm)         EMIT(QADD_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SQADD_16(Vd, Vn, Vm)        EMIT(QADD_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SQADD_32(Vd, Vn, Vm)        EMIT(QADD_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SQADD_64(Vd, Vn, Vm)        EMIT(QADD_vector(0, 0, 0b11, Vm, Vn, Vd))
#define UQADD_8(Vd, Vn, Vm)         EMIT(QADD_vector(0, 1, 0b00, Vm, Vn, Vd))
#define UQADD_16(Vd, Vn, Vm)        EMIT(QADD_vector(0, 1, 0b01, Vm, Vn, Vd))
#define UQADD_32(Vd, Vn, Vm)        EMIT(QADD_vector(0, 1, 0b10, Vm, Vn, Vd))
#define UQADD_64(Vd, Vn, Vm)        EMIT(QADD_vector(0, 1, 0b11, Vm, Vn, Vd))

// Absolute Difference
#define AD_vector(Q, U, size, Rm, ac, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b0111<<12 | (ac)<<11 | 1<<10 | (Rn)<<5 | (Rd))
// Signed Absolute Difference and accumulate
#define SABAQ_8(Rd, Rn, Rm)         EMIT(AD_vector(1, 0, 0b00, Rm, 1, Rn, Rd))
#define SABAQ_16(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b01, Rm, 1, Rn, Rd))
#define SABAQ_32(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b10, Rm, 1, Rn, Rd))
#define SABA_8(Rd, Rn, Rm)          EMIT(AD_vector(0, 0, 0b00, Rm, 1, Rn, Rd))
#define SABA_16(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b01, Rm, 1, Rn, Rd))
#define SABA_32(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b10, Rm, 1, Rn, Rd))
// Signed Absolute Difference
#define SABDQ_8(Rd, Rn, Rm)         EMIT(AD_vector(1, 0, 0b00, Rm, 0, Rn, Rd))
#define SABDQ_16(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b01, Rm, 0, Rn, Rd))
#define SABDQ_32(Rd, Rn, Rm)        EMIT(AD_vector(1, 0, 0b10, Rm, 0, Rn, Rd))
#define SABD_8(Rd, Rn, Rm)          EMIT(AD_vector(0, 0, 0b00, Rm, 0, Rn, Rd))
#define SABD_16(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b01, Rm, 0, Rn, Rd))
#define SABD_32(Rd, Rn, Rm)         EMIT(AD_vector(0, 0, 0b10, Rm, 0, Rn, Rd))

#define ADL_vector(Q, U, size, Rm, op, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b01<<14 | (op)<<13 | 1<<12 | (Rn)<<5 | (Rd))
#define SABAL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 0, 0b00, Rm, 0, Rn, Rd))
#define SABAL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 0, 0b00, Rm, 0, Rn, Rd))
#define SABAL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b01, Rm, 0, Rn, Rd))
#define SABAL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b01, Rm, 0, Rn, Rd))
#define SABAL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b10, Rm, 0, Rn, Rd))
#define SABAL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b10, Rm, 0, Rn, Rd))
#define UABAL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 1, 0b00, Rm, 0, Rn, Rd))
#define UABAL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 1, 0b00, Rm, 0, Rn, Rd))
#define UABAL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b01, Rm, 0, Rn, Rd))
#define UABAL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b01, Rm, 0, Rn, Rd))
#define UABAL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b10, Rm, 0, Rn, Rd))
#define UABAL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b10, Rm, 0, Rn, Rd))
#define SABDL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 0, 0b00, Rm, 1, Rn, Rd))
#define SABDL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 0, 0b00, Rm, 1, Rn, Rd))
#define SABDL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b01, Rm, 1, Rn, Rd))
#define SABDL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b01, Rm, 1, Rn, Rd))
#define SABDL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 0, 0b10, Rm, 1, Rn, Rd))
#define SABDL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 0, 0b10, Rm, 1, Rn, Rd))
#define UABDL_8(Rd, Rn, Rm)         EMIT(ADL_vector(0, 1, 0b00, Rm, 1, Rn, Rd))
#define UABDL2_8(Rd, Rn, Rm)        EMIT(ADL_vector(1, 1, 0b00, Rm, 1, Rn, Rd))
#define UABDL_16(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b01, Rm, 1, Rn, Rd))
#define UABDL2_16(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b01, Rm, 1, Rn, Rd))
#define UABDL_32(Rd, Rn, Rm)        EMIT(ADL_vector(0, 1, 0b10, Rm, 1, Rn, Rd))
#define UABDL2_32(Rd, Rn, Rm)       EMIT(ADL_vector(1, 1, 0b10, Rm, 1, Rn, Rd))

// Add Pairwise
#define ADDLP_vector(Q, U, size, op, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (op)<<14 | 0b10<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SADDLPQ_8(Rd, Rn)           EMIT(ADDLP_vector(1, 0, 0b00, 0, Rn, Rd))
#define SADDLPQ_16(Rd, Rn)          EMIT(ADDLP_vector(1, 0, 0b01, 0, Rn, Rd))
#define SADDLPQ_32(Rd, Rn)          EMIT(ADDLP_vector(1, 0, 0b10, 0, Rn, Rd))
#define SADDLP_8(Rd, Rn)            EMIT(ADDLP_vector(0, 0, 0b00, 0, Rn, Rd))
#define SADDLP_16(Rd, Rn)           EMIT(ADDLP_vector(0, 0, 0b01, 0, Rn, Rd))
#define SADDLP_32(Rd, Rn)           EMIT(ADDLP_vector(0, 0, 0b10, 0, Rn, Rd))
#define UADDLPQ_8(Rd, Rn)           EMIT(ADDLP_vector(1, 1, 0b00, 0, Rn, Rd))
#define UADDLPQ_16(Rd, Rn)          EMIT(ADDLP_vector(1, 1, 0b01, 0, Rn, Rd))
#define UADDLPQ_32(Rd, Rn)          EMIT(ADDLP_vector(1, 1, 0b10, 0, Rn, Rd))
#define UADDLP_8(Rd, Rn)            EMIT(ADDLP_vector(0, 1, 0b00, 0, Rn, Rd))
#define UADDLP_16(Rd, Rn)           EMIT(ADDLP_vector(0, 1, 0b01, 0, Rn, Rd))
#define UADDLP_32(Rd, Rn)           EMIT(ADDLP_vector(0, 1, 0b10, 0, Rn, Rd))

// Add accros vector
#define ADDLV_vector(Q, U, size, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b11000<<17 | 0b00011<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SADDLVQ_8(Rd, Rn)           EMIT(ADDLV_vector(1, 0, 0b00, Rn, Rd))
#define SADDLVQ_16(Rd, Rn)          EMIT(ADDLV_vector(1, 0, 0b01, Rn, Rd))
#define SADDLVQ_32(Rd, Rn)          EMIT(ADDLV_vector(1, 0, 0b10, Rn, Rd))
#define SADDLV_8(Rd, Rn)            EMIT(ADDLV_vector(0, 0, 0b00, Rn, Rd))
#define SADDLV_16(Rd, Rn)           EMIT(ADDLV_vector(0, 0, 0b01, Rn, Rd))
#define SADDLV_32(Rd, Rn)           EMIT(ADDLV_vector(0, 0, 0b10, Rn, Rd))
#define UADDLVQ_8(Rd, Rn)           EMIT(ADDLV_vector(1, 1, 0b00, Rn, Rd))
#define UADDLVQ_16(Rd, Rn)          EMIT(ADDLV_vector(1, 1, 0b01, Rn, Rd))
#define UADDLVQ_32(Rd, Rn)          EMIT(ADDLV_vector(1, 1, 0b10, Rn, Rd))
#define UADDLV_8(Rd, Rn)            EMIT(ADDLV_vector(0, 1, 0b00, Rn, Rd))
#define UADDLV_16(Rd, Rn)           EMIT(ADDLV_vector(0, 1, 0b01, Rn, Rd))
#define UADDLV_32(Rd, Rn)           EMIT(ADDLV_vector(0, 1, 0b10, Rn, Rd))

// Population Count per byte
#define CNT_vector(Q, size, Rn, Rd)     ((Q)<<30 | 0b01110<<24 | (size)<<22 | 0b10000<<17 | 0b00101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define CNT_8(Rd, Rn)               EMIT(CNT_vector(0, 0b00, Rn, Rd))
#define CNTQ_8(Rd, Rn)              EMIT(CNT_vector(1, 0b00, Rn, Rd))

// MOV Immediate
#define MOVI_vector(Q, op, abc, cmode, defgh, Rd)   ((Q)<<30 | (op)<<29 | 0b0111100000<<19 | (abc)<<16 | (cmode)<<12 | 1<<10 | (defgh)<<5 | (Rd))
#define MOVIQ_8(Rd, imm8)           EMIT(MOVI_vector(1, 0, (((imm8)>>5)&0b111), 0b1110, ((imm8)&0b11111), Rd))
#define MOVIQ_16(Rd, imm8, lsl8)    EMIT(MOVI_vector(1, 0, (((imm8)>>5)&0b111), 0b1000|((lsl8)?0b10:0), ((imm8)&0b11111), Rd))
#define MOVIQ_32(Rd, imm8)          EMIT(MOVI_vector(1, 0, (((imm8)>>5)&0b111), 0b0000, ((imm8)&0b11111), Rd))
#define MOVIQ_32_lsl(Rd, imm8, lsl8) EMIT(MOVI_vector(1, 0, (((imm8)>>5)&0b111), (lsl8<<1), ((imm8)&0b11111), Rd))
#define MOVIQ_64(Rd, imm8)          EMIT(MOVI_vector(1, 1, (((imm8)>>5)&0b111), 0b1110, ((imm8)&0b11111), Rd))
#define MOVI_8(Rd, imm8)            EMIT(MOVI_vector(0, 0, (((imm8)>>5)&0b111), 0b1110, ((imm8)&0b11111), Rd))
#define MOVI_16(Rd, imm8, lsl8)     EMIT(MOVI_vector(0, 0, (((imm8)>>5)&0b111), 0b1000|((lsl8)?0b10:0), ((imm8)&0b11111), Rd))
#define MOVI_32_lsl(Rd, imm8, lsl8) EMIT(MOVI_vector(0, 0, (((imm8)>>5)&0b111), (lsl8<<1), ((imm8)&0b11111), Rd))
#define MOVI_32(Rd, imm8)           EMIT(MOVI_vector(0, 0, (((imm8)>>5)&0b111), 0b0000, ((imm8)&0b11111), Rd))
#define MOVI_64(Rd, imm8)           EMIT(MOVI_vector(0, 1, (((imm8)>>5)&0b111), 0b1110, ((imm8)&0b11111), Rd))

// SHLL and eXtend Long
#define SHLL_vector(Q, U, immh, immb, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b10100<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define USHLL2_8(Vd, Vn, imm)       EMIT(SHLL_vector(1, 1, 0b0001, (imm)&0x7, Vn, Vd))
#define USHLL_8(Vd, Vn, imm)        EMIT(SHLL_vector(0, 1, 0b0001, (imm)&0x7, Vn, Vd))
#define SSHLL2_8(Vd, Vn, imm)       EMIT(SHLL_vector(1, 0, 0b0001, (imm)&0x7, Vn, Vd))
#define SSHLL_8(Vd, Vn, imm)        EMIT(SHLL_vector(0, 0, 0b0001, (imm)&0x7, Vn, Vd))
#define USHLL2_16(Vd, Vn, imm)      EMIT(SHLL_vector(1, 1, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define USHLL_16(Vd, Vn, imm)       EMIT(SHLL_vector(0, 1, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define SSHLL2_16(Vd, Vn, imm)      EMIT(SHLL_vector(1, 0, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define SSHLL_16(Vd, Vn, imm)       EMIT(SHLL_vector(0, 0, 0b0010|(((imm)>>3)&1), (imm)&0x7, Vn, Vd))
#define USHLL2_32(Vd, Vn, imm)      EMIT(SHLL_vector(1, 1, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))
#define USHLL_32(Vd, Vn, imm)       EMIT(SHLL_vector(0, 1, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))
#define SSHLL2_32(Vd, Vn, imm)      EMIT(SHLL_vector(1, 0, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))
#define SSHLL_32(Vd, Vn, imm)       EMIT(SHLL_vector(0, 0, 0b0100|(((imm)>>3)&3), (imm)&0x7, Vn, Vd))

#define UXTL_8(Vd, Vn)              USHLL_8(Vd, Vn, 0)
#define UXTL2_8(Vd, Vn)             USHLL2_8(Vd, Vn, 0)
#define UXTL_16(Vd, Vn)             USHLL_16(Vd, Vn, 0)
#define UXTL2_16(Vd, Vn)            USHLL2_16(Vd, Vn, 0)
#define UXTL_32(Vd, Vn)             USHLL_32(Vd, Vn, 0)
#define UXTL2_32(Vd, Vn)            USHLL2_32(Vd, Vn, 0)

#define SXTL_8(Vd, Vn)              SSHLL_8(Vd, Vn, 0)
#define SXTL2_8(Vd, Vn)             SSHLL2_8(Vd, Vn, 0)
#define SXTL_16(Vd, Vn)             SSHLL_16(Vd, Vn, 0)
#define SXTL2_16(Vd, Vn)            SSHLL2_16(Vd, Vn, 0)
#define SXTL_32(Vd, Vn)             SSHLL_32(Vd, Vn, 0)
#define SXTL2_32(Vd, Vn)            SSHLL2_32(Vd, Vn, 0)

// SHRN
#define QSHRN_vector(Q, U, immh, immb, op, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 0b1001<<12 | (op)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define UQSHRN_8(Vd, Vn, imm)       EMIT(QSHRN_vector(0, 1, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN2_8(Vd, Vn, imm)      EMIT(QSHRN_vector(1, 1, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN_8(Vd, Vn, imm)       EMIT(QSHRN_vector(0, 0, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN2_8(Vd, Vn, imm)      EMIT(QSHRN_vector(1, 0, 0b0001, (8-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN_16(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 1, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN2_16(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 1, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN_16(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 0, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN2_16(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 0, 0b0010|(((16-(imm))>>3)&1), (16-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN_32(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 1, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))
#define UQSHRN2_32(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 1, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN_32(Vd, Vn, imm)      EMIT(QSHRN_vector(0, 0, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))
#define SQSHRN2_32(Vd, Vn, imm)     EMIT(QSHRN_vector(1, 0, 0b0100|(((32-(imm))>>3)&3), (32-(imm))&0x7, 0, Vn, Vd))

// UQSUB
#define QSUB_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b00101<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define UQSUB_8(Vd, Vn, Vm)         EMIT(QSUB_vector(0, 1, 0b00, Vm, Vn, Vd))
#define UQSUB_16(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 1, 0b01, Vm, Vn, Vd))
#define UQSUB_32(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 1, 0b10, Vm, Vn, Vd))
#define UQSUB_64(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 1, 0b11, Vm, Vn, Vd))
#define SQSUB_8(Vd, Vn, Vm)         EMIT(QSUB_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SQSUB_16(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SQSUB_32(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SQSUB_64(Vd, Vn, Vm)        EMIT(QSUB_vector(0, 0, 0b11, Vm, Vn, Vd))
#define UQSUBQ_8(Vd, Vn, Vm)        EMIT(QSUB_vector(1, 1, 0b00, Vm, Vn, Vd))
#define UQSUBQ_16(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 1, 0b01, Vm, Vn, Vd))
#define UQSUBQ_32(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 1, 0b10, Vm, Vn, Vd))
#define UQSUBQ_64(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 1, 0b11, Vm, Vn, Vd))
#define SQSUBQ_8(Vd, Vn, Vm)        EMIT(QSUB_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SQSUBQ_16(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SQSUBQ_32(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 0, 0b10, Vm, Vn, Vd))
#define SQSUBQ_64(Vd, Vn, Vm)       EMIT(QSUB_vector(1, 0, 0b11, Vm, Vn, Vd))

// MAX/MIN vector
#define MINMAX_vector(Q, U, size, Rm, op, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b0110<<12 | (op)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SMAX_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 0, 0b00, Vm, 0, Vn, Vd))
#define SMAX_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b01, Vm, 0, Vn, Vd))
#define SMAX_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b10, Vm, 0, Vn, Vd))
//#define SMAX_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b11, Vm, 0, Vn, Vd))
#define UMAX_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 1, 0b00, Vm, 0, Vn, Vd))
#define UMAX_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b01, Vm, 0, Vn, Vd))
#define UMAX_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b10, Vm, 0, Vn, Vd))
//#define UMAX_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b11, Vm, 0, Vn, Vd))
#define SMIN_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 0, 0b00, Vm, 1, Vn, Vd))
#define SMIN_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b01, Vm, 1, Vn, Vd))
#define SMIN_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b10, Vm, 1, Vn, Vd))
//#define SMIN_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 0, 0b11, Vm, 1, Vn, Vd))
#define UMIN_8(Vd, Vn, Vm)          EMIT(MINMAX_vector(0, 1, 0b00, Vm, 1, Vn, Vd))
#define UMIN_16(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b01, Vm, 1, Vn, Vd))
#define UMIN_32(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b10, Vm, 1, Vn, Vd))
//#define UMIN_64(Vd, Vn, Vm)         EMIT(MINMAX_vector(0, 1, 0b11, Vm, 1, Vn, Vd))
#define SMAXQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 0, 0b00, Vm, 0, Vn, Vd))
#define SMAXQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b01, Vm, 0, Vn, Vd))
#define SMAXQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b10, Vm, 0, Vn, Vd))
//#define SMAXQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b11, Vm, 0, Vn, Vd))
#define UMAXQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 1, 0b00, Vm, 0, Vn, Vd))
#define UMAXQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b01, Vm, 0, Vn, Vd))
#define UMAXQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b10, Vm, 0, Vn, Vd))
//#define UMAXQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b11, Vm, 0, Vn, Vd))
#define SMINQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 0, 0b00, Vm, 1, Vn, Vd))
#define SMINQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b01, Vm, 1, Vn, Vd))
#define SMINQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b10, Vm, 1, Vn, Vd))
//#define SMINQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 0, 0b11, Vm, 1, Vn, Vd))
#define UMINQ_8(Vd, Vn, Vm)         EMIT(MINMAX_vector(1, 1, 0b00, Vm, 1, Vn, Vd))
#define UMINQ_16(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b01, Vm, 1, Vn, Vd))
#define UMINQ_32(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b10, Vm, 1, Vn, Vd))
//#define UMINQ_64(Vd, Vn, Vm)        EMIT(MINMAX_vector(1, 1, 0b11, Vm, 1, Vn, Vd))

// MIN or MAX accros vector
#define MAXMINV_vector(Q, U, size, op, Rn, Rd)  ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 0b11000<<17 | (op)<<16 | 0b1010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SMAXV_8(Vd, Vn)             EMIT(MAXMINV_vector(0, 0, 0b00, 0, Vn, Vd))
#define SMINV_8(Vd, Vn)             EMIT(MAXMINV_vector(0, 0, 0b00, 1, Vn, Vd))
#define UMAXV_8(Vd, Vn)             EMIT(MAXMINV_vector(0, 1, 0b00, 0, Vn, Vd))
#define UMINV_8(Vd, Vn)             EMIT(MAXMINV_vector(0, 1, 0b00, 1, Vn, Vd))
#define SMAXV_16(Vd, Vn)            EMIT(MAXMINV_vector(0, 0, 0b01, 0, Vn, Vd))
#define SMINV_16(Vd, Vn)            EMIT(MAXMINV_vector(0, 0, 0b01, 1, Vn, Vd))
#define UMAXV_16(Vd, Vn)            EMIT(MAXMINV_vector(0, 1, 0b01, 0, Vn, Vd))
#define UMINV_16(Vd, Vn)            EMIT(MAXMINV_vector(0, 1, 0b01, 1, Vn, Vd))
#define SMAXV_32(Vd, Vn)            EMIT(MAXMINV_vector(0, 0, 0b10, 0, Vn, Vd))
#define SMINV_32(Vd, Vn)            EMIT(MAXMINV_vector(0, 0, 0b10, 1, Vn, Vd))
#define UMAXV_32(Vd, Vn)            EMIT(MAXMINV_vector(0, 1, 0b10, 0, Vn, Vd))
#define UMINV_32(Vd, Vn)            EMIT(MAXMINV_vector(0, 1, 0b10, 1, Vn, Vd))
#define SMAXVQ_8(Vd, Vn)            EMIT(MAXMINV_vector(1, 0, 0b00, 0, Vn, Vd))
#define SMINVQ_8(Vd, Vn)            EMIT(MAXMINV_vector(1, 0, 0b00, 1, Vn, Vd))
#define UMAXVQ_8(Vd, Vn)            EMIT(MAXMINV_vector(1, 1, 0b00, 0, Vn, Vd))
#define UMINVQ_8(Vd, Vn)            EMIT(MAXMINV_vector(1, 1, 0b00, 1, Vn, Vd))
#define SMAXVQ_16(Vd, Vn)           EMIT(MAXMINV_vector(1, 0, 0b01, 0, Vn, Vd))
#define SMINVQ_16(Vd, Vn)           EMIT(MAXMINV_vector(1, 0, 0b01, 1, Vn, Vd))
#define UMAXVQ_16(Vd, Vn)           EMIT(MAXMINV_vector(1, 1, 0b01, 0, Vn, Vd))
#define UMINVQ_16(Vd, Vn)           EMIT(MAXMINV_vector(1, 1, 0b01, 1, Vn, Vd))
#define SMAXVQ_32(Vd, Vn)           EMIT(MAXMINV_vector(1, 0, 0b10, 0, Vn, Vd))
#define SMINVQ_32(Vd, Vn)           EMIT(MAXMINV_vector(1, 0, 0b10, 1, Vn, Vd))
#define UMAXVQ_32(Vd, Vn)           EMIT(MAXMINV_vector(1, 1, 0b10, 0, Vn, Vd))
#define UMINVQ_32(Vd, Vn)           EMIT(MAXMINV_vector(1, 1, 0b10, 1, Vn, Vd))

// HADD vector
#define HADD_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 1<<10 | (Rn)<<5 | (Rd))
#define SHADD_8(Vd, Vn, Vm)         EMIT(HADD_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SHADD_16(Vd, Vn, Vm)        EMIT(HADD_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SHADD_32(Vd, Vn, Vm)        EMIT(HADD_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SHADDQ_8(Vd, Vn, Vm)        EMIT(HADD_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SHADDQ_16(Vd, Vn, Vm)       EMIT(HADD_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SHADDQ_32(Vd, Vn, Vm)       EMIT(HADD_vector(1, 0, 0b10, Vm, Vn, Vd))
#define UHADD_8(Vd, Vn, Vm)         EMIT(HADD_vector(0, 1, 0b00, Vm, Vn, Vd))
#define UHADD_16(Vd, Vn, Vm)        EMIT(HADD_vector(0, 1, 0b01, Vm, Vn, Vd))
#define UHADD_32(Vd, Vn, Vm)        EMIT(HADD_vector(0, 1, 0b10, Vm, Vn, Vd))
#define UHADDQ_8(Vd, Vn, Vm)        EMIT(HADD_vector(1, 1, 0b00, Vm, Vn, Vd))
#define UHADDQ_16(Vd, Vn, Vm)       EMIT(HADD_vector(1, 1, 0b01, Vm, Vn, Vd))
#define UHADDQ_32(Vd, Vn, Vm)       EMIT(HADD_vector(1, 1, 0b10, Vm, Vn, Vd))

#define RHADD_vector(Q, U, size, Rm, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b00010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SRHADD_8(Vd, Vn, Vm)        EMIT(RHADD_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SRHADD_16(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SRHADD_32(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SRHADDQ_8(Vd, Vn, Vm)       EMIT(RHADD_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SRHADDQ_16(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SRHADDQ_32(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 0, 0b10, Vm, Vn, Vd))
#define URHADD_8(Vd, Vn, Vm)        EMIT(RHADD_vector(0, 1, 0b00, Vm, Vn, Vd))
#define URHADD_16(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 1, 0b01, Vm, Vn, Vd))
#define URHADD_32(Vd, Vn, Vm)       EMIT(RHADD_vector(0, 1, 0b10, Vm, Vn, Vd))
#define URHADDQ_8(Vd, Vn, Vm)       EMIT(RHADD_vector(1, 1, 0b00, Vm, Vn, Vd))
#define URHADDQ_16(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 1, 0b01, Vm, Vn, Vd))
#define URHADDQ_32(Vd, Vn, Vm)      EMIT(RHADD_vector(1, 1, 0b10, Vm, Vn, Vd))

//SRSHR/URSHR
#define RSHR(Q, U, immh, immb, Rn, Rd)      ((Q)<<30 | (U)<<29 | 0b011110<<23 | (immh)<<19 | (immb)<<16 | 1<<13 | 0<<12 | 1<<10 | (Rn)<<5 | (Rd))
#define SRSHRQ_32(Vd, Vn, shift)    EMIT(RSHR(1, 0, 0b0100 | (((32-(shift))>>3)&0b11), (32-(shift))&0b111, Vn, Vd))

// QRDMULH Signed saturating (Rounding) Doubling Multiply returning High half
#define QDMULH_vector(Q, U, size, Rm, Rn, Rd)   ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10110<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define SQRDMULH_8(Vd, Vn, Vm)      EMIT(QDMULH_vector(0, 1, 0b00, Vm, Vn, Vd))
#define SQRDMULH_16(Vd, Vn, Vm)     EMIT(QDMULH_vector(0, 1, 0b01, Vm, Vn, Vd))
#define SQRDMULH_32(Vd, Vn, Vm)     EMIT(QDMULH_vector(0, 1, 0b10, Vm, Vn, Vd))
#define SQRDMULHQ_8(Vd, Vn, Vm)     EMIT(QDMULH_vector(1, 1, 0b00, Vm, Vn, Vd))
#define SQRDMULHQ_16(Vd, Vn, Vm)    EMIT(QDMULH_vector(1, 1, 0b01, Vm, Vn, Vd))
#define SQRDMULHQ_32(Vd, Vn, Vm)    EMIT(QDMULH_vector(1, 1, 0b10, Vm, Vn, Vd))
#define SQDMULH_8(Vd, Vn, Vm)       EMIT(QDMULH_vector(0, 0, 0b00, Vm, Vn, Vd))
#define SQDMULH_16(Vd, Vn, Vm)      EMIT(QDMULH_vector(0, 0, 0b01, Vm, Vn, Vd))
#define SQDMULH_32(Vd, Vn, Vm)      EMIT(QDMULH_vector(0, 0, 0b10, Vm, Vn, Vd))
#define SQDMULHQ_8(Vd, Vn, Vm)      EMIT(QDMULH_vector(1, 0, 0b00, Vm, Vn, Vd))
#define SQDMULHQ_16(Vd, Vn, Vm)     EMIT(QDMULH_vector(1, 0, 0b01, Vm, Vn, Vd))
#define SQDMULHQ_32(Vd, Vn, Vm)     EMIT(QDMULH_vector(1, 0, 0b10, Vm, Vn, Vd))

// AES extension
#define AES_gen(D, Rn, Rd)      (0b01001110<<24 | 0b00<<22 | 0b10100<<17 | 0b0010<<13 | (D)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define AESD(Vd, Vn)    EMIT(AES_gen(1, Vn, Vd))
#define AESE(Vd, Vn)    EMIT(AES_gen(0, Vn, Vd))

#define AESMC_gen(D, Rn, Rd)    (0b01001110<<24 | 0b00<<22 | 0b10100<<17 | 0b0011<<13 | (D)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define AESIMC(Vd, Vn)  EMIT(AESMC_gen(1, Vn, Vd))
#define AESMC(Vd, Vn)   EMIT(AESMC_gen(0, Vn, Vd))

// PMULL extension is PMULL_128
#define PMULL_gen(Q, size, Rm, Rn, Rd)  (0<<31 | (Q)<<30 | 0b001110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b1110<<12 | (Rn)<<5 | (Rd))
#define PMULL(Rd, Rn, Rm)   EMIT(PMULL_gen(0, 0b00, Rm, Rn, Rd))
#define PMULL2(Rd, Rn, Rm)  EMIT(PMULL_gen(1, 0b00, Rm, Rn, Rd))
#define PMULL_128(Rd, Rn, Rm)   EMIT(PMULL_gen(0, 0b11, Rm, Rn, Rd))
#define PMULL2_128(Rd, Rn, Rm)  EMIT(PMULL_gen(1, 0b11, Rm, Rn, Rd))

// ATOMIC extension
#define ATOMIC_gen(size, A, R, Rs, opc, Rn, Rt) ((size)<<30 | 0b111<<27 | (A)<<23 | (R)<<22 | 1<<21 | (Rs)<<16 | (opc)<<12 | (Rn)<<5 | (Rt))
// Atomic ADD
#define LDADDxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b000, Rn, Rt))
#define LDADDAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b000, Rn, Rt))
#define LDADDALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b000, Rn, Rt))
#define LDADDLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b000, Rn, Rt))
#define STADDLw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10, 0, 1, Rs, 0b000, Rn, 0b11111))
#define STADDxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b000, Rn, 0b11111))
#define STADDLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b000, Rn, 0b11111))
#define LDADDB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b000, Rn, Rt))
#define LDADDAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b000, Rn, Rt))
#define LDADDALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b000, Rn, Rt))
#define LDADDLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b000, Rn, Rt))
#define STADDB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b000, Rn, 0b11111))
#define STADDLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b000, Rn, 0b11111))
#define LDADDH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b000, Rn, Rt))
#define LDADDAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b000, Rn, Rt))
#define LDADDALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b000, Rn, Rt))
#define LDADDLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b000, Rn, Rt))
#define STADDH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b000, Rn, 0b11111))
#define STADDLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b000, Rn, 0b11111))
// Atomic AND with complement (i.e. BIC)
#define LDCLRxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b001, Rn, Rt))
#define LDCLRAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b001, Rn, Rt))
#define LDCLRALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b001, Rn, Rt))
#define LDCLRLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b001, Rn, Rt))
#define STCLRxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b001, Rn, 0b11111))
#define STCLRLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b001, Rn, 0b11111))
#define LDCLRB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b001, Rn, Rt))
#define LDCLRAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b001, Rn, Rt))
#define LDCLRALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b001, Rn, Rt))
#define LDCLRLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b001, Rn, Rt))
#define STCLRB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b001, Rn, 0b11111))
#define STCLRLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b001, Rn, 0b11111))
#define LDCLRH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b001, Rn, Rt))
#define LDCLRAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b001, Rn, Rt))
#define LDCLRALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b001, Rn, Rt))
#define LDCLRLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b001, Rn, Rt))
#define STCLRH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b001, Rn, 0b11111))
#define STCLRLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b001, Rn, 0b11111))
// Atomic EOR
#define LDEORxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b010, Rn, Rt))
#define LDEORAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b010, Rn, Rt))
#define LDEORALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b010, Rn, Rt))
#define LDEORLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b010, Rn, Rt))
#define STEORxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b010, Rn, 0b11111))
#define STEORLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b010, Rn, 0b11111))
#define LDEORB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b010, Rn, Rt))
#define LDEORAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b010, Rn, Rt))
#define LDEORALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b010, Rn, Rt))
#define LDEORLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b010, Rn, Rt))
#define STEORB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b010, Rn, 0b11111))
#define STEORLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b010, Rn, 0b11111))
#define LDEORH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b010, Rn, Rt))
#define LDEORAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b010, Rn, Rt))
#define LDEORALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b010, Rn, Rt))
#define LDEORLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b010, Rn, Rt))
#define STEORH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b010, Rn, 0b11111))
#define STEORLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b010, Rn, 0b11111))
// Atomic OR
#define LDSETxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b011, Rn, Rt))
#define LDSETAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b011, Rn, Rt))
#define LDSETALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b011, Rn, Rt))
#define LDSETLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b011, Rn, Rt))
#define STSETxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b011, Rn, 0b11111))
#define STSETLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b011, Rn, 0b11111))
#define LDSETB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b011, Rn, Rt))
#define LDSETAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b011, Rn, Rt))
#define LDSETALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b011, Rn, Rt))
#define LDSETLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b011, Rn, Rt))
#define STSETB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b011, Rn, 0b11111))
#define STSETLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b011, Rn, 0b11111))
#define LDSETH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b011, Rn, Rt))
#define LDSETAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b011, Rn, Rt))
#define LDSETALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b011, Rn, Rt))
#define LDSETLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b011, Rn, Rt))
#define STSETH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b011, Rn, 0b11111))
#define STSETLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b011, Rn, 0b11111))
// Atomic Signed Max
#define LDSMAXxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b100, Rn, Rt))
#define LDSMAXAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b100, Rn, Rt))
#define LDSMAXALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b100, Rn, Rt))
#define LDSMAXLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b100, Rn, Rt))
#define STSMAXxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b100, Rn, 0b11111))
#define STSMAXLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b100, Rn, 0b11111))
#define LDSMAXB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b100, Rn, Rt))
#define LDSMAXAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b100, Rn, Rt))
#define LDSMAXALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b100, Rn, Rt))
#define LDSMAXLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b100, Rn, Rt))
#define STSMAXB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b100, Rn, 0b11111))
#define STSMAXLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b100, Rn, 0b11111))
#define LDSMAXH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b100, Rn, Rt))
#define LDSMAXAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b100, Rn, Rt))
#define LDSMAXALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b100, Rn, Rt))
#define LDSMAXLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b100, Rn, Rt))
#define STSMAXH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b100, Rn, 0b11111))
#define STSMAXLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b100, Rn, 0b11111))
// Atomic Signed Min
#define LDSMINxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b101, Rn, Rt))
#define LDSMINAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b101, Rn, Rt))
#define LDSMINALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b101, Rn, Rt))
#define LDSMINLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b101, Rn, Rt))
#define STSMINxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b101, Rn, 0b11111))
#define STSMINLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b101, Rn, 0b11111))
#define LDSMINB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b101, Rn, Rt))
#define LDSMINAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b101, Rn, Rt))
#define LDSMINALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b101, Rn, Rt))
#define LDSMINLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b101, Rn, Rt))
#define STSMINB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b101, Rn, 0b11111))
#define STSMINLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b101, Rn, 0b11111))
#define LDSMINH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b101, Rn, Rt))
#define LDSMINAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b101, Rn, Rt))
#define LDSMINALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b101, Rn, Rt))
#define LDSMINLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b101, Rn, Rt))
#define STSMINH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b101, Rn, 0b11111))
#define STSMINLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b101, Rn, 0b11111))
// Atomic Unsigned Max
#define LDUMAXxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b110, Rn, Rt))
#define LDUMAXAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b110, Rn, Rt))
#define LDUMAXALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b110, Rn, Rt))
#define LDUMAXLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b110, Rn, Rt))
#define STUMAXxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b110, Rn, 0b11111))
#define STUMAXLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b110, Rn, 0b11111))
#define LDUMAXB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b110, Rn, Rt))
#define LDUMAXAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b110, Rn, Rt))
#define LDUMAXALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b110, Rn, Rt))
#define LDUMAXLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b110, Rn, Rt))
#define STUMAXB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b110, Rn, 0b11111))
#define STUMAXLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b110, Rn, 0b11111))
#define LDUMAXH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b110, Rn, Rt))
#define LDUMAXAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b110, Rn, Rt))
#define LDUMAXALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b110, Rn, Rt))
#define LDUMAXLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b110, Rn, Rt))
#define STUMAXH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b110, Rn, 0b11111))
#define STUMAXLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b110, Rn, 0b11111))
// Atomic Unsigned Min
#define LDUMINxw(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b111, Rn, Rt))
#define LDUMINAxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 1, 0, Rs, 0b111, Rn, Rt))
#define LDUMINALxw(Rs, Rt, Rn)           EMIT(ATOMIC_gen(0b10+rex.w, 1, 1, Rs, 0b111, Rn, Rt))
#define LDUMINLxw(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b111, Rn, Rt))
#define STUMINxw(Rs, Rn)                 EMIT(ATOMIC_gen(0b10+rex.w, 0, 0, Rs, 0b111, Rn, 0b11111))
#define STUMINLxw(Rs, Rn)                EMIT(ATOMIC_gen(0b10+rex.w, 0, 1, Rs, 0b111, Rn, 0b11111))
#define LDUMINB(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b111, Rn, Rt))
#define LDUMINAB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 1, 0, Rs, 0b111, Rn, Rt))
#define LDUMINALB(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b00, 1, 1, Rs, 0b111, Rn, Rt))
#define LDUMINLB(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b111, Rn, Rt))
#define STUMINB(Rs, Rn)                  EMIT(ATOMIC_gen(0b00, 0, 0, Rs, 0b111, Rn, 0b11111))
#define STUMINLB(Rs, Rn)                 EMIT(ATOMIC_gen(0b00, 0, 1, Rs, 0b111, Rn, 0b11111))
#define LDUMINH(Rs, Rt, Rn)              EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b111, Rn, Rt))
#define LDUMINAH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 1, 0, Rs, 0b111, Rn, Rt))
#define LDUMINALH(Rs, Rt, Rn)            EMIT(ATOMIC_gen(0b01, 1, 1, Rs, 0b111, Rn, Rt))
#define LDUMINLH(Rs, Rt, Rn)             EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b111, Rn, Rt))
#define STUMINH(Rs, Rn)                  EMIT(ATOMIC_gen(0b01, 0, 0, Rs, 0b111, Rn, 0b11111))
#define STUMINLH(Rs, Rn)                 EMIT(ATOMIC_gen(0b01, 0, 1, Rs, 0b111, Rn, 0b11111))

// SWAPxx(Xs, Xt, Xn) : [Xn] => Xt, Xs => [Xn]
#define SWAP_gen(size, A, R, Rs, Rn, Rt)    ((size)<<30 | 0b111<<27 | (A)<<23 | (R)<<22 | 1<<21 | (Rs)<<16 | 1<<15 | (Rn)<<5 | (Rt))
#define SWPxw(Rs, Rt, Rn)               EMIT(SWAP_gen(0b10+rex.w, 0, 0, Rs, Rn, Rt))
#define SWPAxw(Rs, Rt, Rn)              EMIT(SWAP_gen(0b10+rex.w, 1, 0, Rs, Rn, Rt))
#define SWPALxw(Rs, Rt, Rn)             EMIT(SWAP_gen(0b10+rex.w, 1, 1, Rs, Rn, Rt))
#define SWPLxw(Rs, Rt, Rn)              EMIT(SWAP_gen(0b10+rex.w, 0, 1, Rs, Rn, Rt))
#define SWPB(Rs, Rt, Rn)                EMIT(SWAP_gen(0b00, 0, 0, Rs, Rn, Rt))
#define SWPAB(Rs, Rt, Rn)               EMIT(SWAP_gen(0b00, 1, 0, Rs, Rn, Rt))
#define SWPALB(Rs, Rt, Rn)              EMIT(SWAP_gen(0b00, 1, 1, Rs, Rn, Rt))
#define SWPLB(Rs, Rt, Rn)               EMIT(SWAP_gen(0b00, 0, 1, Rs, Rn, Rt))
#define SWPH(Rs, Rt, Rn)                EMIT(SWAP_gen(0b01, 0, 0, Rs, Rn, Rt))
#define SWPAH(Rs, Rt, Rn)               EMIT(SWAP_gen(0b01, 1, 0, Rs, Rn, Rt))
#define SWPALH(Rs, Rt, Rn)              EMIT(SWAP_gen(0b01, 1, 1, Rs, Rn, Rt))
#define SWPLH(Rs, Rt, Rn)               EMIT(SWAP_gen(0b01, 0, 1, Rs, Rn, Rt))

#define CAS_gen(size, L, Rs, O0, Rn, Rt)    ((size)<<30 | 0b001000<<24 | 1<<23 | (L)<<22 | 1<<21 | (Rs)<<16 | (O0)<<15 | 0b11111<<10 | (Rn)<<5 | (Rt))
// Compare and Swap compare Rs with [Rn], write Rt is same, return old [Rn] in Rs
#define CASxw(Rs, Rt, Rn)               EMIT(CAS_gen(0b10+rex.w, 0, Rs, 0, Rn, Rt))
#define CASAxw(Rs, Rt, Rn)              EMIT(CAS_gen(0b10+rex.w, 1, Rs, 0, Rn, Rt))
#define CASALxw(Rs, Rt, Rn)             EMIT(CAS_gen(0b10+rex.w, 1, Rs, 1, Rn, Rt))
#define CASLxw(Rs, Rt, Rn)              EMIT(CAS_gen(0b10+rex.w, 0, Rs, 1, Rn, Rt))
#define CASB(Rs, Rt, Rn)                EMIT(CAS_gen(0b00, 0, Rs, 0, Rn, Rt))
#define CASAB(Rs, Rt, Rn)               EMIT(CAS_gen(0b00, 1, Rs, 0, Rn, Rt))
#define CASALB(Rs, Rt, Rn)              EMIT(CAS_gen(0b00, 1, Rs, 1, Rn, Rt))
#define CASLB(Rs, Rt, Rn)               EMIT(CAS_gen(0b00, 0, Rs, 1, Rn, Rt))
#define CASH(Rs, Rt, Rn)                EMIT(CAS_gen(0b01, 0, Rs, 0, Rn, Rt))
#define CASAH(Rs, Rt, Rn)               EMIT(CAS_gen(0b01, 1, Rs, 0, Rn, Rt))
#define CASALH(Rs, Rt, Rn)              EMIT(CAS_gen(0b01, 1, Rs, 1, Rn, Rt))
#define CASLH(Rs, Rt, Rn)               EMIT(CAS_gen(0b01, 0, Rs, 1, Rn, Rt))

#define CASP_gen(size, L, Rs, O0, Rn, Rt)   ((size)<<30 | 0b001000<<24 | 0<<23 | (L)<<22 | 1<<21 | (Rs)<<16 | (O0)<<15 | 0b11111<<10 | (Rn)<<5 | (Rt))
// Compare and Swap with pair, Rs, Rs+1 and Rt,Rt+1 with [Rt]
#define CASPxw(Rs, Rt, Rn)              EMIT(CASP_gen(0b00+rex.w, 0, Rs, 0, Rn, Rt))
#define CASPAxw(Rs, Rt, Rn)             EMIT(CASP_gen(0b00+rex.w, 1, Rs, 0, Rn, Rt))
#define CASPALxw(Rs, Rt, Rn)            EMIT(CASP_gen(0b00+rex.w, 1, Rs, 1, Rn, Rt))
#define CASPLxw(Rs, Rt, Rn)             EMIT(CASP_gen(0b00+rex.w, 0, Rs, 1, Rn, Rt))

// FLAGM extension
// Invert Carry Flag
#define CFINV()                        FEMIT(0b1101010100<<22 | 0b0100<<12 | 0b000<<5 | 0b11111)

#define RMIF_gen(imm6, Rn, mask)        (0b10111010000<<21 | (imm6)<<15 | 0b00001<<10 | (Rn)<<5 | (mask))
// Rotate right reg and use as NZCV
#define RMIF(Xn, shift, mask)          FEMIT(RMIF_gen(shift, Xn, mask))

#define SETF_gen(sz, Rn)                (0b00111010000<<21 | (sz)<<14 | 0b0010<<10 | (Rn)<<5 | 0b1101)
// Set NZVc with 8bit value of reg: N=bit7, Z=[0..7]==0, V=bit8 eor bit7, C unchanged
#define SETF8(Wn)                      FEMIT(SETF_gen(0, Wn))
// Set NZVc with 16bit value of reg: N=bit15, Z=[0..15]==0, V=bit16 eor bit15, C unchanged
#define SETF16(Wn)                     FEMIT(SETF_gen(1, Wn))

// FLAGM2 extension
// NZCV -> N=0 Z=C|V C=C&!V V=0
#define AXFLAG()           FEMIT(0b1101010100<<22 | 0b0100<<12 | 0b010<<5 | 0b11111)
// NZCV -> N=!C&!Z Z=Z&C C=C|Z V=!C&Z
#define XAFLAG()           FEMIT(0b1101010100<<22 | 0b0100<<12 | 0b001<<5 | 0b11111)

// FRINTTS extension
#define FRINTxx_scalar(type, op, Rn, Rd)  (0b11110<<24 | (type)<<22 | 1<<21 | 0b0100<<17 | (op)<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FRINT32ZS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b00, Sn, Sd))
#define FRINT32ZD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b00, Dn, Dd))
#define FRINT32XS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b01, Sn, Sd))
#define FRINT32XD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b01, Dn, Dd))
#define FRINT64ZS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b10, Sn, Sd))
#define FRINT64ZD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b10, Dn, Dd))
#define FRINT64XS(Sd, Sn)           EMIT(FRINTxx_scalar(0b00, 0b11, Sn, Sd))
#define FRINT64XD(Dd, Dn)           EMIT(FRINTxx_scalar(0b01, 0b11, Dn, Dd))

#define FRINTxx_vector(Q, U, sz, op, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b1111<<13 | (op)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFRINT32ZS(Vd, Vn)          EMIT(FRINTxx_vector(0, 0, 0, 0, Vn, Vd))
#define VFRINT32ZSQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 0, 0, 0, Vn, Vd))
#define VFRINT32XS(Vd, Vn)          EMIT(FRINTxx_vector(0, 1, 0, 0, Vn, Vd))
#define VFRINT32XSQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 1, 0, 0, Vn, Vd))
#define VFRINT32ZD(Vd, Vn)          EMIT(FRINTxx_vector(0, 0, 1, 0, Vn, Vd))
#define VFRINT32ZDQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 0, 1, 0, Vn, Vd))
#define VFRINT32XD(Vd, Vn)          EMIT(FRINTxx_vector(0, 1, 1, 0, Vn, Vd))
#define VFRINT32XDQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 1, 1, 0, Vn, Vd))
#define VFRINT64ZS(Vd, Vn)          EMIT(FRINTxx_vector(0, 0, 0, 1, Vn, Vd))
#define VFRINT64ZSQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 0, 0, 1, Vn, Vd))
#define VFRINT64XS(Vd, Vn)          EMIT(FRINTxx_vector(0, 1, 0, 1, Vn, Vd))
#define VFRINT64XSQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 1, 0, 1, Vn, Vd))
#define VFRINT64ZD(Vd, Vn)          EMIT(FRINTxx_vector(0, 0, 1, 1, Vn, Vd))
#define VFRINT64ZDQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 0, 1, 1, Vn, Vd))
#define VFRINT64XD(Vd, Vn)          EMIT(FRINTxx_vector(0, 1, 1, 1, Vn, Vd))
#define VFRINT64XDQ(Vd, Vn)         EMIT(FRINTxx_vector(1, 1, 1, 1, Vn, Vd))

// CRC32 extension
#define CRC32C_gen(sf, Rm, sz, Rn, Rd)  ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b010<<13 | 1<<12 | (sz)<<10 | (Rn)<<5 | (Rd))
#define CRC32CB(Wd, Wn, Wm)         EMIT(CRC32C_gen(0, Wm, 0b00, Wn, Wd))
#define CRC32CH(Wd, Wn, Wm)         EMIT(CRC32C_gen(0, Wm, 0b01, Wn, Wd))
#define CRC32CW(Wd, Wn, Wm)         EMIT(CRC32C_gen(0, Wm, 0b10, Wn, Wd))
#define CRC32CX(Wd, Wn, Xm)         EMIT(CRC32C_gen(1, Xm, 0b11, Wn, Wd))
#define CRC32Cxw(Wd, Wn, Rm)        EMIT(CRC32C_gen(rex.w, Rm, 0b10|rex.w, Wn, Wd))

// SHA1 extension
#define SHA1H_gen(Rn, Rd)       (0b01011110<<24 | 0b10100<<17 | 0b10<<10 | (Rn)<<5 | (Rd))
// SHA1 fixed rotate (ROL 30 of 32bits value)
#define SHA1H(Sd, Sn)           EMIT(SHA1H_gen(Sn, Sd))

#define SHA1SU1_gen(Rn, Rd)     (0b01011110<<24 | 0b10100<<17 | 0b00001<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// SHA1 schedule update 1
#define SHA1SU1(Vd, Vn)         EMIT(SHA1SU1_gen(Vn, Vd))

#define SHA1C_gen(Rm, Rn, Rd)   (0b01011110<<24 | (Rm)<<16 | (Rn)<<5 | (Rd))
// SHA1 hash update (choose)
#define SHA1C(Qd, Sn, Vm)       EMIT(SHA1C_gen(Vm, Sn, Qd))

#define SHA1M_gen(Rm, Rn, Rd)   (0b01011110<<24 | (Rm)<<16 | 0b010<<12 | (Rn)<<5 | (Rd))
// SHA1 hash update (majority)
#define SHA1M(Qd, Sn, Vm)       EMIT(SHA1M_gen(Vm, Sn, Qd))

#define SHA1P_gen(Rm, Rn, Rd)   (0b01011110<<24 | (Rm)<<16 | 0b001<<12 | (Rn)<<5 | (Rd))
// SHA1 hash update (parity)
#define SHA1P(Qd, Sn, Vm)       EMIT(SHA1P_gen(Vm, Sn, Qd))

#define SHA256SU0_gen(Rn,Rd)    (0b01011110<<24 | 0b10100<<17 | 0b00010<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
//SHA256 schedule update 0
#define SHA256SU0(Vd, Vn)       EMIT(SHA256SU0_gen(Vn, Vd))

#define SHA256SU1_gen(Rm, Rn, Rd)   (0b01011110<<24 | (Rm)<<16 | 0b110<<12 | (Rn)<<5 | (Rd))
//SHA256 schedule update 1
#define SHA256SU1(Vd, Vn, Vm)       EMIT(SHA256SU1_gen(Vm, Vn, Vd))

#define SHA256H_gen(Rm, Rn, Rd) (0b01011110<<24 | (Rm)<<16 | 0b100<<12 | (Rn)<<5 | (Rd))
//SHA256 hash update (part 1)
#define SHA256H(Vd, Vn, Vm)     EMIT(SHA256H_gen(Vm, Vn, Vd))

#define SHA256H2_gen(Rm, Rn, Rd)    (0b01011110<<24 | (Rm)<<16 | 0b101<<12 | (Rn)<<5 | (Rd))
//SHA256 hash update (part 2)
#define SHA256H2(Vd, Vn, Vm)        EMIT(SHA256H2_gen(Vm, Vn, Vd))

// LRCPC extension

#define LDAPR_gen(sz, Rn, Rt)   ((sz)<<30 | 0b111<<27 | 1<<23 | 1<<21 | 0b11111<<16 | 1<<15 | 0b100<<12 | (Rn)<<5 | (Rt))
#define LDAPRx(Rt, Rn)      EMIT(LDAPR_gen(0b11, Rn, Rt))
#define LDAPRw(Rt, Rn)      EMIT(LDAPR_gen(0b10, Rn, Rt))
#define LDAPRxw(Rt, Rn)     EMIT(LDAPR_gen(0b10+rex.w, Rn, Rt))
#define LDAPRz(Rt, Rn)      EMIT(LDAPR_gen(rex.is32bits?0b10:0b11, Rn, Rt))
#define LDAPRH(Rt, Rn)      EMIT(LDAPR_gen(0b01, Rn, Rt))
#define LDAPRB(Rt, Rn)      EMIT(LDAPR_gen(0b00, Rn, Rt))

#endif  //__ARM64_EMITTER_H__
