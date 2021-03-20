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

// LDR
#define LDR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRx_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRx_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRw_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRw_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRB_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b00, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRB_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b00, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))

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
#define LDRB_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define LDRH_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))

#define LDRSH_gen(size, op1, opc, imm12, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | (opc)<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRSHx_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b01, 0b01, 0b10, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHw_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b01, 0b01, 0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSHxw_U12(Rt, Rn, imm12)          EMIT(LDRSH_gen(0b01, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSBx_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b00, 0b01, 0b10, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSBw_U12(Rt, Rn, imm12)           EMIT(LDRSH_gen(0b00, 0b01, 0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRSBxw_U12(Rt, Rn, imm12)          EMIT(LDRSH_gen(0b00, 0b01, rex.w?0b10:0b11, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))

#define LDR_PC_gen(opc, imm19, Rt)      ((opc)<<30 | 0b011<<27 | (imm19)<<5 | (Rt))
#define LDRx_literal(Rt, imm19)         EMIT(LDR_PC_gen(0b01, ((imm19)>>2)&0x7FFFF, Rt))

// STR
#define STR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b00<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define STRx_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRx_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define STRw_S9_postindex(Rt, Rn, imm9)   EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define STRw_S9_preindex(Rt, Rn, imm9)    EMIT(STR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))

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

// BR and branchs
#define BR_gen(Z, op, A, M, Rn, Rm)       (0b1101011<<25 | (Z)<<24 | (op)<<21 | 0b11111<<16 | (A)<<11 | (M)<<10 | (Rn)<<5 | (Rm))
#define BR(Rn)                            EMIT(BR_gen(0, 0b00, 0, 0, Rn, 0))
#define BLR(Rn)                           EMIT(BR_gen(0, 0b01, 0, 0, Rn, 0))

#define CB_gen(sf, op, imm19, Rt)       ((sf)<<31 | 0b011010<<25 | (op)<<24 | (imm19)<<5 | (Rt))
#define CBNZx(Rt, imm19)                EMIT(CB_gen(1, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBNZw(Rt, imm19)                EMIT(CB_gen(0, 1, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZx(Rt, imm19)                 EMIT(CB_gen(1, 0, ((imm19)>>2)&0x7FFFF, Rt))
#define CBZw(Rt, imm19)                 EMIT(CB_gen(0, 0, ((imm19)>>2)&0x7FFFF, Rt))

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

#define CSEL_gen(sf, Rm, cond, Rn, Rd)      ((sf<<31) | 0b11010100<<21 | (Rm)<<16 | (cond)<<12 | (Rn)<<5 | Rd)
#define CSELx(Rd, Rn, Rm, cond)             EMIT(CSEL_gen(1, Rm, cond, Rn, Rd))
#define CSELw(Rd, Rn, Rm, cond)             EMIT(CSEL_gen(0, Rm, cond, Rn, Rd))
#define CSELxw(Rd, Rn, Rm, cond)            EMIT(CSEL_gen(rex.w, Rm, cond, Rn, Rd))

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
#define TSTx_mask(Rn, immr, imms)           ANDSx_mask(xZR, Rn, immr, imms)
#define TSTw_mask(Rn, immr, imms)           ANDSw_mask(wZR, Rn, immr, imms)
#define TSTxw_mask(Rn, immr, imms)          ANDSxw_mask(xZR, Rn, immr, imms)

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
#define SXTWx(Rd, Rn)                   SBFMx(Rd, Rn, 0, 31)
#define ASRx(Rd, Rn, shift)             SBFMx(Rd, Rn, shift, 63)
#define ASRw(Rd, Rn, shift)             SBFMw(Rd, Rn, shift, 31)
#define ASRxw(Rd, Rn, shift)            SBFMxw(Rd, Rn, shift, rex.w?63:31)

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
// imm15 must be 3-aligned
#define VLDR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b01, ((imm15)>>3)&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VLDR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b11, 0b11, ((imm16)>>4)&0xfff, Rn, Qt))
// imm15 must be 3-aligned
#define VSTR64_U12(Dt, Rn, imm15)           EMIT(VMEM_gen(0b11, 0b00, ((imm15)>>3)&0xfff, Rn, Dt))
// imm16 must be 4-aligned
#define VSTR128_U12(Qt, Rn, imm16)          EMIT(VMEM_gen(0b11, 0b10, ((imm16)>>4)&0xfff, Rn, Qt))

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

#define VLDR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 0, Rn, Dt))
#define VLDR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b01, Rm, 0b011, 1, Rn, Dt))
#define VLDR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 0, Rn, Dt))
#define VLDR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b11, Rm, 0b011, 1, Rn, Dt))

#define VSTR64_REG(Dt, Rn, Rm)              EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 0, Rn, Dt))
#define VSTR64_REG_LSL3(Dt, Rn, Rm)         EMIT(VMEM_REG_gen(0b11, 0b00, Rm, 0b011, 1, Rn, Dt))
#define VSTR128_REG(Qt, Rn, Rm)             EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 0, Rn, Dt))
#define VSTR128_REG_LSL4(Qt, Rn, Rm)        EMIT(VMEM_REG_gen(0b00, 0b10, Rm, 0b011, 1, Rn, Dt))


#endif  //__ARM64_EMITTER_H__
