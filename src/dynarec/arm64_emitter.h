#ifndef __ARM64_EMITTER_H__
#define __ARM64_EMITTER_H__
/*
    ARM64 Emitter

*/

// x86 Register mapping
#define xRAX    10
#define xRCX    11
#define xRDX    12
#define xRBX    13
#define xRSP    14
#define xRBP    15
#define xRSI    16
#define xRDI    17
#define xR8     18
#define xR9     19
#define xR10    20
#define xR11    21
#define xR12    22
#define xR13    23
#define xR14    24
#define xR15    25
#define xFlags  26
#define xRIP    27
// 32bits version
#define wEAX    xRAX
#define wECX    xRCX
#define wEDX    xRDX
#define wEBX    xRBX
#define wESP    xRSP
#define wEBP    xRBP
#define wESI    xRSI
#define wEDI    xRDI
#define wR8     xR8
#define wR9     xR9
#define wR10    xR10
#define wR11    xR11
#define wR12    xR12
#define wR13    xR13
#define wR14    xR14
#define wR15    xR15
#define wFlags  xFlags
// scratch registers
#define x1      1
#define x2      2
#define x3      3
#define x4      4
#define x5      5
#define x6      6
#define x7      7
// 32bits version of scratch
#define w1      x1
#define w2      x2
#define w3      x3
#define w4      x4
#define w5      x5
#define w6      x6
#define w7      x7
// emu is r0
#define xEmu    0
// ARM64 SP is r31 but is a special register
#define xSP     31      
// xZR regs is 31
#define xZR     31
#define wZR     xZR

// conditions
#define cEQ 0b0000
#define cNE 0b0001
#define cCS 0b0010
#define cHS cCS
#define cCC 0b0011
#define cLO cCC
#define cMI 0b0100
#define cPL 0b0101
#define cVS 0b0110
#define cVC 0b0111
#define cHI 0b1000
#define cLS 0b1001
#define cGE 0b1010
#define cLT 0b1011
#define cGT 0b1100
#define cLE 0b1101
#define c__ 0b1110

#define invCond(cond)   ((cond)^0b0001)

// MOVZ
#define MOVZ_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b10<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVZx(Rd, imm16)                    EMIT(MOVZ_gen(1, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVZx_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(1, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))
#define MOVZw(Rd, imm16)                    EMIT(MOVZ_gen(0, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVZw_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(0, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))

// MOVN
#define MOVN_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b00<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVNx(Rd, imm16)                    EMIT(MOVN_gen(1, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVNx_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(1, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))
#define MOVNw(Rd, imm16)                    EMIT(MOVN_gen(0, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVNw_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(0, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))

// MOVK
#define MOVK_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b11<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVKx(Rd, imm16)                    EMIT(MOVK_gen(1, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVKx_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(1, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))
#define MOVKw(Rd, imm16)                    EMIT(MOVK_gen(0, 0, ((uint16_t)imm16)&0xffff, Rd))
#define MOVKw_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(0, (shift)/16, ((uint16_t)imm16)&0xffff, Rd))

#define MOV32w(Rd, imm32) \
    if(~((uint32_t)(imm32))<0xffff) {                                       \
        MOVNw(Rd, (~(uint32_t)(imm32))&0xffff);                             \
    } else {                                                                \
        MOVZw(Rd, (imm32)&0xffff);                                          \
        if((imm32)&0xffff0000) {MOVKw_LSL(Rd, ((imm32)>>16)&0xffff, 16);}   \
    }
#define MOV64x(Rd, imm64) \
    if(~((uint64_t)(imm64))<0xffff) {                                                                       \
        MOVNx(Rd, (~(uint64_t)(imm64))&0xffff);                                                             \
    } else {                                                                                                \
        MOVZx(Rd, ((uint64_t)(imm64))&0xffff);                                                              \
        if(((uint64_t)(imm64))&0xffff0000) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>16)&0xffff, 16);}           \
        if(((uint64_t)(imm64))&0xffff00000000LL) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>32)&0xffff, 32);}      \
        if(((uint64_t)(imm64))&0xffff000000000000LL) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>48)&0xffff, 48);}  \
    }

#define MOV64xw(Rd, imm64)   if(rex.w) {MOV64x(Rd, imm64);} else {MOV32w(Rd, imm64);}


// ADD / SUB
#define ADDSUB_REG_gen(sf, op, S, shift, Rm, imm6, Rn, Rd) ((sf)<<31 | (op)<<30 | (S)<<29 | 0b01011<<24 | (shift)<<22 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ADDx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSx_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(1, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(0, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSxw_REG(Rd, Rn, Rm)              EMIT(ADDSUB_REG_gen(rex.w, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDxw_REG_LSR(Rd, Rn, Rm, lsr)      EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b01, Rm, lsr, Rn, Rd))

#define ADDSUB_IMM_gen(sf, op, S, shift, imm12, Rn, Rd)    ((sf)<<31 | (op)<<30 | (S)<<29 | 0b10001<<24 | (shift)<<22 | (imm12)<<10 | (Rn)<<5 | (Rd))
#define ADDx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSx_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(1, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(0, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSxw_U12(Rd, Rn, imm12)   EMIT(ADDSUB_IMM_gen(rex.w, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))

#define SUBx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSx_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(1, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBw_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBSw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBSw_REG_LSL(Rd, Rn, Rm, lsl)      EMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, lsl, Rn, Rd))
#define SUBxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSxw_REG(Rd, Rn, Rm)              EMIT(ADDSUB_REG_gen(rex.w, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define CMPSx_REG(Rn, Rm)                   SUBSx_REG(xZR, Rn, Rm)
#define CMPSw_REG(Rn, Rm)                   SUBSw_REG(wZR, Rn, Rm)
#define CMPSxw_REG(Rn, Rm)                  SUBSxw_REG(xZR, Rn, Rm)
#define NEGx_REG(Rd, Rm)                    SUBx_REG(Rd, xZR, Rm);
#define NEGw_REG(Rd, Rm)                    SUBw_REG(Rd, wZR, Rm);
#define NEGxw_REG(Rd, Rm)                   SUBxw_REG(Rd, xZR, Rm);
#define NEGSx_REG(Rd, Rm)                   SUBSx_REG(Rd, xZR, Rm);
#define NEGSw_REG(Rd, Rm)                   SUBSw_REG(Rd, wZR, Rm);
#define NEGSxw_REG(Rd, Rm)                  SUBSxw_REG(Rd, xZR, Rm);

#define SUBx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSx_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(1, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(0, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSxw_U12(Rd, Rn, imm12)   EMIT(ADDSUB_IMM_gen(rex.w, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
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
#define ADCSx_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(1, 0, 1, Rm, Rn, Rd))
#define ADCSw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(0, 0, 1, Rm, Rn, Rd))
#define ADCSxw_REG(Rd, Rn, Rm)      EMIT(ADDSUBC_gen(rex.w, 0, 1, Rm, Rn, Rd))
#define SBCSx_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(1, 1, 1, Rm, Rn, Rd))
#define SBCSw_REG(Rd, Rn, Rm)       EMIT(ADDSUBC_gen(0, 1, 1, Rm, Rn, Rd))
#define SBCSxw_REG(Rd, Rn, Rm)      EMIT(ADDSUBC_gen(rex.w, 1, 1, Rm, Rn, Rd))

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
#define LDRx_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b11, 0b01, ((uint32_t)(imm12>>3))&0xfff, Rn, Rt))
#define LDRw_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b10, 0b01, ((uint32_t)(imm12>>2))&0xfff, Rn, Rt))
#define LDRB_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b00, 0b01, ((uint32_t)(imm12))&0xfff, Rn, Rt))
#define LDRH_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b01, 0b01, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRxw_U12(Rt, Rn, imm12)          EMIT(LD_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)(imm12>>(2+rex.w)))&0xfff, Rn, Rt))

#define LDS_gen(size, op1, imm12, Rn, Rt)       ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b10<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSW_U12(Rt, Rn, imm12)          EMIT(LDS_gen(0b10, 0b01, ((uint32_t)(imm12>>2))&0xfff, Rn, Rt))

#define LDR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b01<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define LDRx_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define LDRx_REG_LSL3(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define LDRx_REG_UXTW3(Rt, Rn, Rm)      EMIT(LDR_REG_gen(0b11, Rm, 0b010, 1, Rn, Rt))
#define LDRw_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define LDRw_REG_LSL2(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define LDRxw_REG(Rt, Rn, Rm)           EMIT(LDR_REG_gen(0b10+rex.w, Rm, 0b011, 0, Rn, Rt))
#define LDRB_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define LDRH_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))

#define LDRSH_gen(size, op1, opc, imm12, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSHx_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b01, 0b01, 0b10, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHw_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b01, 0b01, 0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHxw_U12(Rt, Rn, imm12)          EMIT(LDRSH_gen(0b01, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSBx_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b00, 0b01, 0b10, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))
#define LDRSBw_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b00, 0b01, 0b11, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))
#define LDRSBxw_U12(Rt, Rn, imm12)          EMIT(LDRSH_gen(0b00, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>0))&0xfff, Rn, Rt))

#define LDR_PC_gen(opc, imm19, Rt)      ((opc)<<30 | 0b011<<27 | (imm19)<<5 | (Rt))
#define LDRx_literal(Rt, imm19)         EMIT(LDR_PC_gen(0b01, ((imm19)>>2)&0x7FFFF, Rt))

// STR
#define STR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define STRx_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRx_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRw_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRw_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRxw_S9_postindex(Rt, Rn, imm9)  EMIT(STR_gen(rex.w?0b11:0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRB_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b00, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRH_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b01, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))

#define ST_gen(size, op1, imm12, Rn, Rt)        ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define STRx_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b11, 0b01, ((uint32_t)(imm12>>3))&0xfff, Rn, Rt))
#define STRw_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b10, 0b01, ((uint32_t)(imm12>>2))&0xfff, Rn, Rt))
#define STRB_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b00, 0b01, ((uint32_t)(imm12))&0xfff, Rn, Rt))
#define STRH_U12(Rt, Rn, imm12)           EMIT(ST_gen(0b01, 0b01, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define STRxw_U12(Rt, Rn, imm12)          EMIT(ST_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)((imm12)>>(2+rex.w)))&0xfff, Rn, Rt))

#define STR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b00<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define STRx_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define STRx_REG_LSL3(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define STRx_REG_UXTW(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b11, Rm, 0b010, 0, Rn, Rt))
#define STRw_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define STRw_REG_LSL2(Rt, Rn, Rm)       EMIT(STR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define STRB_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define STRH_REG(Rt, Rn, Rm)            EMIT(STR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))

// LOAD/STORE PAIR
#define MEMPAIR_gen(size, L, op2, imm7, Rt2, Rn, Rt)    ((size)<<31 | 0b101<<27 | (op2)<<23 | (L)<<22 | (imm7)<<15 | (Rt2)<<10 | (Rn)<<5 | (Rt))

#define LDPx_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(1, 1, 0b01, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(0, 1, 0b01, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_postindex(Rt, Rt2, Rn, imm)            EMIT(MEMPAIR_gen(rex.w, 1, 0b01, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define LDPx_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(1, 1, 0b11, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(0, 1, 0b11, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_preindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(rex.w, 1, 0b11, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define LDPx_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(1, 1, 0b10, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define LDPw_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(0, 1, 0b10, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define LDPxw_S7_offset(Rt, Rt2, Rn, imm)               EMIT(MEMPAIR_gen(rex.w, 1, 0b10, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))

#define STPx_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(1, 0, 0b01, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_postindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(0, 0, 0b01, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_postindex(Rt, Rt2, Rn, imm)            EMIT(MEMPAIR_gen(rex.w, 0, 0b01, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define STPx_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(1, 0, 0b11, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_preindex(Rt, Rt2, Rn, imm)              EMIT(MEMPAIR_gen(0, 0, 0b11, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_preindex(Rt, Rt2, Rn, imm)             EMIT(MEMPAIR_gen(rex.w, 0, 0b11, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))
#define STPx_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(1, 0, 0b10, (((uint32_t)(imm))>>3)&0x7f, Rt2, Rn, Rt))
#define STPw_S7_offset(Rt, Rt2, Rn, imm)                EMIT(MEMPAIR_gen(0, 0, 0b10, (((uint32_t)(imm))>>2)&0x7f, Rt2, Rn, Rt))
#define STPxw_S7_offset(Rt, Rt2, Rn, imm)               EMIT(MEMPAIR_gen(rex.w, 0, 0b10, (((uint32_t)(imm))>>(2+rex.w)), Rt2, Rn, Rt))

// PUSH / POP helper
#define POP1(reg)       LDRx_S9_postindex(reg, xRSP, 8)
#define PUSH1(reg)      STRx_S9_preindex(reg, xRSP, -8)

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

// BR and Branches
#define BR_gen(Z, op, A, M, Rn, Rm)       (0b1101011<<25 | (Z)<<24 | (op)<<21 | 0b11111<<16 | (A)<<11 | (M)<<10 | (Rn)<<5 | (Rm))
#define BR(Rn)                            EMIT(BR_gen(0, 0b00, 0, 0, Rn, 0))
#define BLR(Rn)                           EMIT(BR_gen(0, 0b01, 0, 0, Rn, 0))

#define CB_gen(sf, op, imm19, Rt)       ((sf)<<31 | 0b011010<<25 | (op)<<24 | (imm19)<<5 | (Rt))
#define CBNZx(Rt, imm19)                EMIT(CB_gen(1, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZw(Rt, imm19)                EMIT(CB_gen(0, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZxw(Rt, imm19)               EMIT(CB_gen(rex.w, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZx(Rt, imm19)                 EMIT(CB_gen(1, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZw(Rt, imm19)                 EMIT(CB_gen(0, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZxw(Rt, imm19)                EMIT(CB_gen(rex.w, 0, ((imm19)>>2)&0x7FFFF, Rt))

#define TB_gen(b5, op, b40, imm14, Rt)  ((b5)<<31 | 0b011011<<25 | (op)<<24  | (b40)<<19 | (imm14)<<5 | (Rt))
#define TBZ(Rt, bit, imm16)             EMIT(TB_gen(((bit)>>5)&1, 0, (bit)&0x1f, ((imm16)>>2)&0x3FFF, Rt))
#define TBNZ(Rt, bit, imm16)            EMIT(TB_gen(((bit)>>5)&1, 1, (bit)&0x1f, ((imm16)>>2)&0x3FFF, Rt))

#define Bcond_gen(imm19, cond)          (0b0101010<<25 | (imm19)<<5 | (cond))
#define Bcond(cond, imm19)              EMIT(Bcond_gen(((imm19)>>2)&0x7FFFF, cond))

#define B_gen(imm26)                    (0b000101<<26 | (imm26))
#define B(imm26)                        EMIT(B_gen(((imm26)>>2)&0x3ffffff))

#define NOP                             EMIT(0b11010101000000110010000000011111)

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
#define CSINVxw(Rd, Rn, Rm, cond)           EMIT(CSINV_gen(rex.w?, Rm, cond, Rn, Rd))
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
#define CNEGx(Rd, Rn, cond)                 CSNEGx(Rn, Rn, Rn, invCond(cond))
#define CNEGw(Rd, Rn, cond)                 CSNEGw(Rn, Rn, Rn, invCond(cond))
#define CNEGxw(Rd, Rn, cond)                CSNEGxw(Rn, Rn, Rn, invCond(cond))

// AND / ORR
#define LOGIC_gen(sf, opc, N, immr, imms, Rn, Rd)  ((sf)<<31 | (opc)<<29 | 0b100100<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | Rd)
// logic to get the mask is ... convoluted... list of possible value there: https://gist.github.com/dinfuehr/51a01ac58c0b23e4de9aac313ed6a06a
#define ANDx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b00, N, immr, imms, Rn, Rd))
#define ANDw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b00, 0, immr, imms, Rn, Rd))
#define ANDSx_mask(Rd, Rn, N, immr, imms)   EMIT(LOGIC_gen(1, 0b11, N, immr, imms, Rn, Rd))
#define ANDSw_mask(Rd, Rn, immr, imms)      EMIT(LOGIC_gen(0, 0b11, 0, immr, imms, Rn, Rd))
#define ORRx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b01, N, immr, imms, Rn, Rd))
#define ORRw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b01, 0, immr, imms, Rn, Rd))
#define EORx_mask(Rd, Rn, N, immr, imms)    EMIT(LOGIC_gen(1, 0b10, N, immr, imms, Rn, Rd))
#define EORw_mask(Rd, Rn, immr, imms)       EMIT(LOGIC_gen(0, 0b10, 0, immr, imms, Rn, Rd))
#define TSTx_mask(Rn, N, immr, imms)        ANDSx_mask(xZR, Rn, N, immr, imms)
#define TSTw_mask(Rn, immr, imms)           ANDSw_mask(wZR, Rn, immr, imms)

#define LOGIC_REG_gen(sf, opc, shift, N, Rm, imm6, Rn, Rd)    ((sf)<<31 | (opc)<<29 | 0b01010<<24 | (shift)<<22 | (N)<<21 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ANDx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSx_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(1, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(0, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSxw_REG(Rd, Rn, Rm)          EMIT(LOGIC_REG_gen(rex.w, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, lsl, Rn, Rd))
#define ORRxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORNx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define EORx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 0, Rm, 0, Rn, Rd))
#define EORx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORw_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(0, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORxw_REG_LSL(Rd, Rn, Rm, lsl)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b00, 0, Rm, lsl, Rn, Rd))
#define EORx_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(1, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EORw_REG_LSR(Rd, Rn, Rm, lsr)   EMIT(LOGIC_REG_gen(0, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define EORxw_REG_LSR(Rd, Rn, Rm, lsr)  EMIT(LOGIC_REG_gen(rex.w, 0b10, 0b01, 0, Rm, lsr, Rn, Rd))
#define MOVx_REG(Rd, Rm)                ORRx_REG(Rd, xZR, Rm)
#define MOVw_REG(Rd, Rm)                ORRw_REG(Rd, xZR, Rm)
#define MOVxw_REG(Rd, Rm)               ORRxw_REG(Rd, xZR, Rm)
#define MVNx_REG(Rd, Rm)                ORNx_REG(Rd, xZR, Rm)
#define MVNx_REG_LSL(Rd, Rm, lsl)       ORNx_REG_LSL(Rd, xZR, Rm, lsl)
#define MVNw_REG(Rd, Rm)                ORNw_REG(Rd, xZR, Rm)
#define MVNxw_REG(Rd, Rm)               ORNxw_REG(Rd, xZR, Rm)
#define MOV_frmSP(Rd)                   ADDx_U12(Rd, xSP, 0)
#define MOV_toSP(Rm)                    ADDx_U12(xSP, Rm, 0)
#define BICx(Rd, Rn, Rm)                EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICw(Rd, Rn, Rm)                EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICw_LSL(Rd, Rn, Rm, lsl)       EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, lsl, Rn, Rd))
#define BICSx(Rd, Rn, Rm)               EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICSw(Rd, Rn, Rm)               EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICxw(Rd, Rn, Rm)               EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICSxw(Rd, Rn, Rm)              EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 1, Rm, 0, Rn, Rd))
#define BICx_REG    BICx
#define BICw_REG    BICw
#define BICxw_REG   BICxw
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
#define BFIx(Rd, Rn, lsb, width)        BFMx(Rd, Rn, ((-lsb)%64)&0x3f, (width)-1)
#define BFIw(Rd, Rn, lsb, width)        BFMw(Rd, Rn, ((-lsb)%32)&0x1f, (width)-1)
#define BFIxw(Rd, Rn, lsb, width)       if(rex.w) {BFIx(Rd, Rn, lsb, width);} else {BFIw(Rd, Rn, lsb, width);}
#define BFCx(Rd, lsb, width)            BFMx(Rd, xZR, ((-lsb)%64)&0x3f, (width)-1)
#define BFCw(Rd, lsb, width)            BFMw(Rd, xZR, ((-lsb)%32)&0x1f, (width)-1)
#define BFCxw(Rd, lsb, width)           BFMxw(Rd, xZR, rex.w?(((-lsb)%64)&0x3f):(((-lsb)%32)&0x1f), (width)-1)
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
#define SBFXx(Rd, Rn, lsb, width)       SBFMx(Rd, Rn, lsb, lsb+width-1)
#define SBFXw(Rd, Rn, lsb, width)       SBFMw(Rd, Rn, lsb, lsb+width-1)
#define SBFXxw(Rd, Rn, lsb, width)      SBFMxw(Rd, Rn, lsb, lsb+width-1)
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

#define MADD_gen(sf, Rm, Ra, Rn, Rd)    ((sf)<<31 | 0b11011<<24 | (Rm)<<16 | (Ra)<<10 | (Rn)<<5 | (Rd))
#define MADDx(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(1, Rm, Ra, Rn, Rd))
#define MADDw(Rd, Rn, Rm, Ra)           EMIT(MADD_gen(0, Rm, Ra, Rn, Rd))
#define MADDxw(Rd, Rn, Rm, Ra)          EMIT(MADD_gen(rex.w, Rm, Ra, Rn, Rd))
#define MULx(Rd, Rn, Rm)                MADDx(Rd, Rn, Rm, xZR)
#define MULw(Rd, Rn, Rm)                MADDw(Rd, Rn, Rm, xZR)
#define MULxw(Rd, Rn, Rm)               MADDxw(Rd, Rn, Rm, xZR)

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
#define REVx(Rd, Rn)                    EMIT(REV_gen(1, 0b11, Rn, Rd))
#define REVw(Rd, Rn)                    EMIT(REV_gen(0, 0b10, Rn, Rd))
#define REVxw(Rd, Rn)                   EMIT(REV_gen(rex.w, 0b10|rex.w, Rn, Rd))

// MRS
#define MRS_gen(L, o0, op1, CRn, CRm, op2, Rt)  (0b1101010100<<22 | (L)<<21 | 1<<20 | (o0)<<19 | (op1)<<16 | (CRn)<<12 | (CRm)<<8 | (op2)<<5 | (Rt))
// mrs    x0, nzcv : 1101010100 1 1 1 011 0100 0010 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0010(2) op2=0
// MRS : from System register
#define MRS_nzvc(Rt)                    EMIT(MRS_gen(1, 1, 3, 4, 2, 0, Rt))
// MSR : to System register
#define MSR_nzvc(Rt)                    EMIT(MRS_gen(0, 1, 3, 4, 2, 0, Rt))
// mrs    x0, fpcr : 1101010100 1 1 1 011 0100 0100 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=2
#define VMRS(Rt)                        EMIT(MRS_gen(1, 1, 3, 4, 4, 0, Rt))
#define VMSR(Rt)                        EMIT(MRS_gen(0, 1, 3, 4, 4, 0, Rt))
 
// VLDR
#define VMEM_gen(size, opc, imm12, Rn, Rt)  ((size)<<30 | 0b111<<27 | 1<<26 | 0b01<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
// imm14 must be 3-aligned
#define VLDR32_U12(Dt, Rn, imm14)           EMIT(VMEM_gen(0b10, 0b01, ((uint32_t)((imm14)>>2))&0xfff, Rn, Dt))
// imm15 must be 3-aligned
#define VLDR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b01, ((uint32_t)((imm15)>>3))&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VLDR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b00, 0b11, ((uint32_t)((imm16)>>4))&0xfff, Rn, Qt))
// (imm14) must be 3-aligned
#define VSTR32_U12(Dt, Rn, imm14)           EMIT(VMEM_gen(0b10, 0b00, ((uint32_t)(imm14>>2))&0xfff, Rn, Dt))
// (imm15) must be 3-aligned
#define VSTR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b00, ((uint32_t)(imm15>>3))&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VSTR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b00, 0b10, ((uint32_t)((imm16)>>4))&0xfff, Rn, Qt))

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
#define VLDR32_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 0, Rn, Dt))
#define VLDR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 0, Rn, Dt))
#define VLDR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 1, Rn, Dt))

#define VSTR32_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR32_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b10, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 0, Rn, Dt))
#define VSTR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 1, Rn, Dt))

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
#define VSHL_16(Vd, Vn, shift)              EMIT(SHL_vector(0, 0b0010 | ((shift)>>3)&1, (shift)&7, Vn, Vd))
#define VSHL_32(Vd, Vn, shift)              EMIT(SHL_vector(0, 0b0100 | (((shift)>>3)&3), (shift)&7, Vn, Vd))

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

#define EXT_vector(Q, Rm, imm4, Rn, Rd)     ((Q)<<30 | 0b101110<<24 | (Rm)<<16 | (imm4)<<11 | (Rn)<<5 | (Rd))
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

// VMOV
#define VMOV_element(imm5, imm4, Rn, Rd)    (1<<30 | 1<<29 | 0b01110000<<21 | (imm5)<<16 | (imm4)<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VMOVeB(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<1) | 1, i2, Vn, Vd))
#define VMOVeH(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<2) | 2, i2<<1, Vn, Vd))
#define VMOVeS(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<3) | 4, i2<<2, Vn, Vd))
#define VMOVeD(Vd, i1, Vn, i2)              EMIT(VMOV_element(((i1)<<4) | 8, i2<<3, Vn, Vd))

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
#define VADDPQ_8(Vd, Vn, Vm)        EMIT(ADDP(1, 0b00, Vm, Vn, Vd))
#define VADDPQ_16(Vd, Vn, Vm)       EMIT(ADDP(1, 0b01, Vm, Vn, Vd))
#define VADDPQ_32(Vd, Vn, Vm)       EMIT(ADDP(1, 0b10, Vm, Vn, Vd))
#define VADDPQ_64(Vd, Vn, Vm)       EMIT(ADDP(1, 0b11, Vm, Vn, Vd))
#define VADDP_8(Vd, Vn, Vm)         EMIT(ADDP(0, 0b00, Vm, Vn, Vd))
#define VADDP_16(Vd, Vn, Vm)        EMIT(ADDP(0, 0b01, Vm, Vn, Vd))
#define VADDP_32(Vd, Vn, Vm)        EMIT(ADDP(0, 0b10, Vm, Vn, Vd))

#define FADDP_vector(Q, sz, Rm, Rn, Rd) ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 1<<21 | (Rm)<<16 | 0b11010<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFADDPQS(Vd, Vn, Vm)        EMIT(FADDP_vector(1, 0, Vm, Vn, Vd))
#define VFADDPQD(Vd, Vn, Vm)        EMIT(FADDP_vector(1, 1, Vm, Vn, Vd))

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

// SQRT
#define FSQRT_vector(Q, sz, Rn, Rd)     ((Q)<<30 | 1<<29 | 0b01110<<24 | 1<<23 | (sz)<<22 | 0b10000<<17 | 0b11111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VFSQRTS(Sd, Sn)             EMIT(FSQRT_vector(0, 0, Sn, Sd))
#define VFSQRTQS(Sd, Sn)            EMIT(FSQRT_vector(1, 0, Sn, Sd))
#define VFSQRTQD(Sd, Sn)            EMIT(FSQRT_vector(1, 1, Sn, Sd))

#define FSQRT_scalar(type, Rn, Rd)      (0b11110<<24 | (type)<<22 | 1<<21 | 0b11<<15 | 0b10000<<10 | (Rn)<<5 | (Rd))
#define FSQRTS(Sd, Sn)              EMIT(FSQRT_scalar(0b00, Sn, Sd))
#define FSQRTD(Dd, Dn)              EMIT(FSQRT_scalar(0b01, Dn, Dd))

// CMP
#define FCMP_scalar(type, Rn, Rm, opc)  (0b11110<<24 | (type)<<22 | 1<<21 | (Rm)<<16 | 0b1000<<10 | (Rn)<<5 | (opc)<<3)
#define FCMPS(Sn, Sm)               EMIT(FCMP_scalar(0b00, Sn, Sm, 0b00))
#define FCMPD(Dn, Dm)               EMIT(FCMP_scalar(0b01, Dn, Dm, 0b00))
#define FCMPS_0(Sn)                 EMIT(FCMP_scalar(0b00, 0, Sn, 0b01))
#define FCMPD_0(Dn)                 EMIT(FCMP_scalar(0b01, 0, Dn, 0b01))

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
#define FCVTMSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b10, 0b100, Dn, Wd))
#define FCVTMSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b10, 0b100, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding toward Minus infinity
#define FCVTMUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b10, 0b101, Sn, Wd))
#define FCVTMUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b10, 0b101, Sn, Xd))
#define FCVTMUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b10, 0b101, Dn, Wd))
#define FCVTMUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b10, 0b101, Dn, Xd))
// Floating-point Convert to Signed integer, rounding to nearest with ties to even
#define FCVTNSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b000, Sn, Wd))
#define FCVTNSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b000, Sn, Xd))
#define FCVTNSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b000, Dn, Wd))
#define FCVTNSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b000, Dn, Xd))
// Floating-point Convert to Unsigned integer, rounding to nearest with ties to even
#define FCVTNUwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b00, 0b001, Sn, Wd))
#define FCVTNUxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b00, 0b001, Sn, Xd))
#define FCVTNUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b00, 0b001, Dn, Wd))
#define FCVTNUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b00, 0b001, Dn, Xd))
// Floating-point Convert to Signed integer, rounding toward Plus infinity
#define FCVTPSwS(Wd, Sn)            EMIT(FCVT_scalar(0, 0b00, 0b01, 0b000, Sn, Wd))
#define FCVTPSxS(Xd, Sn)            EMIT(FCVT_scalar(1, 0b00, 0b01, 0b000, Sn, Xd))
#define FCVTPSwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b01, 0b000, Dn, Wd))
#define FCVTPSxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b01, 0b000, Dn, Xd))
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
#define FCVTZUwD(Wd, Dn)            EMIT(FCVT_scalar(0, 0b01, 0b11, 0b001, Dn, Wd))
#define FCVTZUxD(Xd, Dn)            EMIT(FCVT_scalar(1, 0b01, 0b11, 0b001, Dn, Xd))

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

#define FCVT_vector(Q, U, o2, sz, o1, Rn, Rd)       ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o2)<<23 | (sz)<<22) | 0b10000<<17 | 0b1110<<13 | (o1)<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
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

#define FCVTXN_vector(Q, sz, Rn, Rd)   ((Q)<<30 | 1<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10110<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert Vn from 2*Double to lower Vd as 2*float and clears the upper half
#define FCVTXN(Vd, Vn)              EMIT(FCVTXN_vector(0, 1, Vn, Vd))
// Convert Vn from 2*Double to higher Vd as 2*float
#define FCVTXN2(Vd, Vn)             EMIT(FCVTXN_vector(1, 1, Vn, Vd))

#define FCVTL_vector(Q, sz, Rn, Rd)     ((Q)<<30 | 0<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b10111<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
// Convert lower Vn from 2*float to Vd as 2*double
#define FCVTL(Vd, Vn)               EMIT(FCVTL_vector(0, 1, Vn, Vd))
// Convert higher Vn from 2*float to Vd as 2*double
#define FCVTL2(Vd, Vn)              EMIT(FCVTL_vector(1, 1, Vn, Vd))

#define SCVTF_scalar(sf, type, rmode, opcode, Rn, Rd)   ((sf)<<31 | 0b11110<<24 | (type)<<22 | 1<<21 | (rmode)<<19 | (opcode)<<16 | (Rn)<<5 | (Rd))
#define SCVTFSw(Sd, Wn)             EMIT(SCVTF_scalar(0, 0b00, 0b00, 0b010, Wn, Sd))
#define SCVTFDw(Dd, Wn)             EMIT(SCVTF_scalar(0, 0b01, 0b00, 0b010, Wn, Dd))
#define SCVTFSx(Sd, Xn)             EMIT(SCVTF_scalar(1, 0b00, 0b00, 0b010, Xn, Sd))
#define SCVTFDx(Dd, Xn)             EMIT(SCVTF_scalar(1, 0b01, 0b00, 0b010, Xn, Dd))

#define SCVTF_vector_scalar(U, sz, Rn, Rd)    (1<<30 | (U)<<29 | 0b11110<<24 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SCVTFSS(Vd, Vn)             EMIT(SCVT_vector_scalar(0, 0, Vn, Vd))
#define SCVTFDD(Vd, Vn)             EMIT(SCVT_vector_scalar(0, 1, Vn, Vd))

#define SCVTF_vector(Q, U, sz, Rn, Rd)      ((Q)<<30 | (U)<<29 | 0b01110<<24 | (sz)<<22 | 0b10000<<17 | 0b11101<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define SCVTFS(Vd, Vn)              EMIT(SCVTF_vector(0, 0, 0, Vn, Vd))
#define SCVTFD(Vd, Vn)              EMIT(SCVTF_vector(0, 0, 1, Vn, Vd))
#define SCVTQFS(Vd, Vn)             EMIT(SCVTF_vector(1, 0, 0, Vn, Vd))
#define SCVTQFD(Vd, Vn)             EMIT(SCVTF_vector(1, 0, 1, Vn, Vd))

// FMAX / FMIN
#define FMINMAX_vector(Q, U, o1, sz, Rm, Rn, Rd)    ((Q)<<30 | (U)<<29 | 0b01110<<24 | (o1)<<23 | (sz)<<22 | 0b1<<21 | (Rm)<<16 | 0b11110<<11 | 1<<10 | (Rn)<<5 | (Rd))
#define VFMINS(Vd, Vn, Vm)          EMIT(FMINMAX_vector(0, 0, 1, 0, Vm, Vn, Vd))
#define VFMAXS(Vd, Vn, Vm)          EMIT(FMINMAX_vector(0, 0, 0, 0, Vm, Vn, Vd))
#define VFMINQS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 1, 0, Vm, Vn, Vd))
#define VFMAXQS(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 0, 0, Vm, Vn, Vd))
#define VFMINQD(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 1, 1, Vm, Vn, Vd))
#define VFMAXQD(Vd, Vn, Vm)         EMIT(FMINMAX_vector(1, 0, 0, 1, Vm, Vn, Vd))

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

// TBL
#define TBL_gen(Q, Rm, len, op, Rn, Rd) ((Q)<<30 | 0b001110<<24 | (Rm)<<16 | (len)<<13 | (op)<<12 | (Rn)<<5 | (Rd))
//Use Rm[] to pick from Rn element and store in Rd. Out-of-range element gets 0
#define VTBLQ1_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b00, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1 element and store in Rd. Out-of-range element gets 0
#define VTBLQ2_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b01, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2 element and store in Rd. Out-of-range element gets 0
#define VTBLQ3_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b10, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2, Rn+3 element and store in Rd. Out-of-range element gets 0
#define VTBLQ4_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b11, 0, Rn, Rd))
//Use Rm[] to pick from Rn element and store in Rd. Out-of-range element stay untouched
#define VTBXQ1_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b00, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ2_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b01, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ3_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b10, 0, Rn, Rd))
//Use Rm[] to pick from Rn, Rn+1, Rn+2, Rn+3 element and store in Rd. Out-of-range element stay untouched
#define VTBXQ4_8(Rd, Rn, Rm)        EMIT(TBL_gen(1, Rm, 0b11, 0, Rn, Rd))

// TRN
#define TRN_gen(Q, size, Rm, op, Rn, Rd)    ((Q)<<30 | 0b001110<<24 | (size)<<22 | (Rm)<<16 | (op)<<14 | 0b10<<12 | 0b10<<10 | (Rn)<<5 | (Rd))
#define VTRNQ1_64(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b11, Vm, 0, Vn, Vd))
#define VTRNQ1_32(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b10, Vm, 0, Vn, Vd))
#define VTRNQ1_16(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b01, Vm, 0, Vn, Vd))
#define VTRNQ1_8(Vd, Vn, Vm)        EMIT(TRN_gen(1, 0b00, Vm, 0, Vn, Vd))
#define VSWP(Vd, Vn)                VTRNQ1_64(Vd, Vn, Vn)
#define VTRNQ2_64(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b11, Vm, 1, Vn, Vd))
#define VTRNQ2_32(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b10, Vm, 1, Vn, Vd))
#define VTRNQ2_16(Vd, Vn, Vm)       EMIT(TRN_gen(1, 0b01, Vm, 1, Vn, Vd))
#define VTRNQ2_8(Vd, Vn, Vm)        EMIT(TRN_gen(1, 0b00, Vm, 1, Vn, Vd))

// QXTN / QXTN2
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
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_32(Rd, Rn)           EMIT(QXTN_vector(0, 1, 0b10, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 64->32 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_32(Rd, Rn)          EMIT(QXTN_vector(1, 1, 0b10, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_16(Rd, Rn)           EMIT(QXTN_vector(0, 1, 0b01, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 32->16 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_16(Rd, Rn)          EMIT(QXTN_vector(1, 1, 0b01, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit lower part of Rd
#define SQXTUN_8(Rd, Rn)            EMIT(QXTN_vector(0, 1, 0b00, Rn, Rd))
// Signed saturating extract Unsigned Narrow, takes Rn element and reduce 16->8 with Unsigned saturation and fit higher part of Rd
#define SQXTUN2_8(Rd, Rn)           EMIT(QXTN_vector(1, 1, 0b00, Rn, Rd))

// Integer CMP
// EQual
#define CMEQ_vector(Q, U, size, Rm, Rn, Rd)     ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b10001<<11 | 1<<10 | (Rn)<<5 | (Rd))
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
// Unsigned Higher
#define VCHIQQ_8(Rd, Rn, Rm)        EMIT(CMG_vector(1, 1, 0b00, 0, Rm, Rn, Rd))
#define VCHIQQ_16(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b01, 0, Rm, Rn, Rd))
#define VCHIQQ_32(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b10, 0, Rm, Rn, Rd))
#define VCHIQQ_64(Rd, Rn, Rm)       EMIT(CMG_vector(1, 1, 0b11, 0, Rm, Rn, Rd))

// UMULL / SMULL
#define MULL_vector(Q, U, size, Rm, Rn, Rd) ((Q)<<30 | (U)<<29 | 0b01110<<24 | (size)<<22 | 1<<21 | (Rm)<<16 | 0b1100<<12 |(Rn)<<5 |(Rd))
#define VUMULL_8(Rd, Rn, Rm)        EMIT(MULL_vector(0, 1, 0b00, Rm, Rn, Rd))
#define VUMULL_16(Rd, Rn, Rm)       EMIT(MULL_vector(0, 1, 0b01, Rm, Rn, Rd))
#define VUMULL_32(Rd, Rn, Rm)       EMIT(MULL_vector(0, 1, 0b10, Rm, Rn, Rd))
#define VUMULL2_8(Rd, Rn, Rm)       EMIT(MULL_vector(1, 1, 0b00, Rm, Rn, Rd))
#define VUMULL2_16(Rd, Rn, Rm)      EMIT(MULL_vector(1, 1, 0b01, Rm, Rn, Rd))
#define VUMULL2_32(Rd, Rn, Rm)      EMIT(MULL_vector(1, 1, 0b10, Rm, Rn, Rd))

#endif  //__ARM64_EMITTER_H__
