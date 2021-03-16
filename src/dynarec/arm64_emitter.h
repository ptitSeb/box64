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


// MOVZ
#define MOVZ_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b10<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVZx(Rd, imm16)                    EMIT(MOVZ_gen(1, 0, (imm16)&0xffff, Rd))
#define MOVZx_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(1, (shift)/16, (imm16)&0xffff, Rd))
#define MOVZw(Rd, imm16)                    EMIT(MOVZ_gen(0, 0, (imm16)&0xffff, Rd))
#define MOVZw_LSL(Rd, imm16, shift)         EMIT(MOVZ_gen(0, (shift)/16, (imm16)&0xffff, Rd))

// MOVN
#define MOVN_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b00<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVNx(Rd, imm16)                    EMIT(MOVN_gen(1, 0, (imm16)&0xffff, Rd))
#define MOVNx_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(1, (shift)/16, (imm16)&0xffff, Rd))
#define MOVNw(Rd, imm16)                    EMIT(MOVN_gen(0, 0, (imm16)&0xffff, Rd))
#define MOVNw_LSL(Rd, imm16, shift)         EMIT(MOVN_gen(0, (shift)/16, (imm16)&0xffff, Rd))

// MOVK
#define MOVK_gen(sf, hw, imm16, Rd)         ((sf)<<31 | 0b11<<29 | 0b100101<<23 | (hw)<<21 | (imm16)<<5 | (Rd))
#define MOVKx(Rd, imm16)                    EMIT(MOVK_gen(1, 0, (imm16)&0xffff, Rd))
#define MOVKx_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(1, (shift)/16, (imm16)&0xffff, Rd))
#define MOVKw(Rd, imm16)                    EMIT(MOVK_gen(0, 0, (imm16)&0xffff, Rd))
#define MOVKw_LSL(Rd, imm16, shift)         EMIT(MOVK_gen(0, (shift)/16, (imm16)&0xffff, Rd))

#define MOV32w(Rd, imm32) {MOVZw(Rd, (imm32)&0xffff); if((imm32)&0xffff0000) {MOVKw_LSL(Rd, ((imm32)>>16)&0xffff, 16);}}
#define MOV64x(Rd, imm64) { \
    MOVZx(Rd, ((uint64_t)(imm64))&0xffff); \
    if(((uint64_t)(imm64))&0xffff0000) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>16)&0xffff, 16);} \
    if(((uint64_t)(imm64))&0xffff00000000L) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>32)&0xffff, 32);} \
    if(((uint64_t)(imm64))&0xffff000000000000L) {MOVKx_LSL(Rd, (((uint64_t)(imm64))>>48)&0xffff, 48);}\
}

// ADD / SUB
#define ADDSUB_REG_gen(sf, op, S, shift, Rm, imm6, Rn, Rd) ((sf)<<31 | (op)<<30 | (S)<<29 | 0b01011<<24 | (shift)<<22 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ADDx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSx_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(1, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 0, 0, 0b00, Rm, lsl, Rn, Rd))
#define ADDw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 0, 0, 0b00, Rm, 0, Rn, Rd))
#define ADDSw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(0, 0, 1, 0b00, Rm, 0, Rn, Rd))
#define ADDxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 0, 0, 0b00, Rm, 0, Rn, Rd))

#define ADDSUB_IMM_gen(sf, op, S, shift, imm12, Rn, Rd)    ((sf)<<31 | (op)<<30 | (S)<<29 | 0b10001<<24 | (shift)<<22 | (imm12)<<10 | (Rn)<<5 | (Rd))
#define ADDx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSx_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(1, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDSw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(0, 0, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define ADDxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 0, 0, 0b00, (imm12)&0xfff, Rn, Rd))

#define SUBx_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSx_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(1, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBx_REG_LSL(Rd, Rn, Rm, lsl)       EMIT(ADDSUB_REG_gen(1, 1, 0, 0b00, Rm, lsl, Rn, Rd))
#define SUBw_REG(Rd, Rn, Rm)                EMIT(ADDSUB_REG_gen(0, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(0, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define SUBxw_REG(Rd, Rn, Rm)               EMIT(ADDSUB_REG_gen(rex.w, 1, 0, 0b00, Rm, 0, Rn, Rd))
#define SUBSxw_REG(Rd, Rn, Rm)              EMIT(ADDSUB_REG_gen(rex.w, 1, 1, 0b00, Rm, 0, Rn, Rd))
#define CMPSx_REG(Rn, Rm)                   SUBSx_REG(xZR, Rn, Rm)
#define CMPSw_REG(Rn, Rm)                   SUBSw_REG(wZR, Rn, Rm)
#define CMPSxw_REG(Rn, Rm)                  SUBSxw_REG(xZR, Rn, Rm)

#define SUBx_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(1, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSx_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(1, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBw_U12(Rd, Rn, imm12)     EMIT(ADDSUB_IMM_gen(0, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(0, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBxw_U12(Rd, Rn, imm12)    EMIT(ADDSUB_IMM_gen(rex.w, 1, 0, 0b00, (imm12)&0xfff, Rn, Rd))
#define SUBSxw_U12(Rd, Rn, imm12)   EMIT(ADDSUB_IMM_gen(rex.w, 1, 1, 0b00, (imm12)&0xfff, Rn, Rd))
#define CMPSx_U12(Rn, imm12)        SUBSx_U12(xZR, Rn, imm12)
#define CMPSw_U12(Rn, imm12)        SUBSw_U12(wZR, Rn, imm12)
#define CMPSxw_U12(Rn, imm12)       SUBSxw_U12(xZR, Rn, imm12)

// LDR
#define LDR_gen(size, op1, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRx_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRx_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b11, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))
#define LDRw_S9_postindex(Rt, Rn, imm9)   EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b01, Rn, Rt))
#define LDRw_S9_preindex(Rt, Rn, imm9)    EMIT(LDR_gen(0b10, 0b00, (imm9)&0x1ff, 0b11, Rn, Rt))

#define LD_gen(size, op1, imm12, Rn, Rt)        ((size)<<30 | 0b111<<27 | (op1)<<24 | 0b01<<22 | (imm12)<<10 | (Rn)<<5 | (Rt))
#define LDRx_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b11, 0b01, ((uint32_t)(imm12>>3))&0xfff, Rn, Rt))
#define LDRw_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b10, 0b01, ((uint32_t)(imm12>>2))&0xfff, Rn, Rt))
#define LDRB_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b00, 0b01, ((uint32_t)(imm12))&0xfff, Rn, Rt))
#define LDRH_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b01, 0b01, ((uint32_t)(imm12>>1))&0xfff, Rn, Rt))
#define LDRxw_U12(Rt, Rn, imm12)          EMIT(LD_gen((rex.w)?0b11:0b10, 0b01, ((uint32_t)(imm12>>(2+rex.w)))&0xfff, Rn, Rt))

#define LDR_REG_gen(size, Rm, option, S, Rn, Rt)    ((size)<<30 | 0b111<<27 | 0b01<<22 | 1<<21 | (Rm)<<16 | (option)<<13 | (S)<<12 | (0b10)<<10 | (Rn)<<5 | (Rt))
#define LDRx_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b11, Rm, 0b011, 0, Rn, Rt))
#define LDRx_REG_LSL3(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b11, Rm, 0b011, 1, Rn, Rt))
#define LDRx_REG_UXTW(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b11, Rm, 0b010, 0, Rn, Rt))
#define LDRw_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b10, Rm, 0b011, 0, Rn, Rt))
#define LDRw_REG_LSL2(Rt, Rn, Rm)       EMIT(LDR_REG_gen(0b10, Rm, 0b011, 1, Rn, Rt))
#define LDRB_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b00, Rm, 0b011, 0, Rn, Rt))
#define LDRH_REG(Rt, Rn, Rm)            EMIT(LDR_REG_gen(0b01, Rm, 0b011, 0, Rn, Rt))

#define LDRSH_gen(size, op1, opc, imm9, op2, Rn, Rt)    ((size)<<30 | 0b111<<27 | (op1)<<24 | (opc)<<22 | (imm9)<<12 | (op2)<<10 | (Rn)<<5 | (Rt))
#define LDRSHx_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b01, 0b00, 0b10, ((uint32_t)(imm12>>2))&0xfff, Rn, Rt))
#define LDRSHw_U12(Rt, Rn, imm12)           EMIT(LD_gen(0b01, 0b00, 0b11, ((uint32_t)(imm12>2))&0xfff, Rn, Rt))

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

// AND / ORR
#define LOGIC_gen(sf, opc, N, immr, imms, Rn, Rd)  ((sf)<<31 | (opc)<<29 | 0b100100<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn) | Rd)
#define ANDx_U13(Rd, Rn, imm13)         EMIT(LOGIC_gen(1, 0b00, ((imm13)>>12)&1, (imm13)&0b111111, ((imm13)>>6)&0b111111, Rn, Rd))
#define ANDw_U12(Rd, Rn, imm12)         EMIT(LOGIC_gen(0, 0b00, 0, (imm12)&0b111111, ((imm12)>>6)&0b111111, Rn, Rd))
#define ANDxw_U12(Rd, Rn, imm12)        EMIT(LOGIC_gen(rex.w, 0b00, 0, (imm12)&0b111111, ((imm12)>>6)&0b111111, Rn, Rd))
#define ANDSx_U13(Rd, Rn, imm13)        EMIT(LOGIC_gen(1, 0b11, ((imm13)>>12)&1, (imm13)&0b111111, ((imm13)>>6)&0b111111, Rn, Rd))
#define ANDSw_U12(Rd, Rn, imm12)        EMIT(LOGIC_gen(0, 0b11, 0, (imm12)&0b111111, ((imm12)>>6)&0b111111, Rn, Rd))
#define ORRx_U13(Rd, Rn, imm13)         EMIT(LOGIC_gen(1, 0b01, ((imm13)>>12)&1, (imm13)&0b111111, ((imm13)>>6)&0b111111, Rn, Rd))
#define ORRw_U12(Rd, Rn, imm12)         EMIT(LOGIC_gen(0, 0b01, 0, (imm12)&0b111111, ((imm12)>>6)&0b111111, Rn, Rd))
#define ORRxw_U12(Rd, Rn, imm12)        EMIT(LOGIC_gen(rex.w, 0b01, 0, (imm12)&0b111111, ((imm12)>>6)&0b111111, Rn, Rd))

#define LOGIC_REG_gen(sf, opc, shift, N, Rm, imm6, Rn, Rd)    ((sf)<<31 | (opc)<<29 | 0b01010<<24 | (shift)<<22 | (N)<<21 | (Rm)<<16 | (imm6)<<10 | (Rn)<<5 | (Rd))
#define ANDx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b00, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSx_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(1, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ANDSw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(0, 0b11, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRx_REG_LSL(Rd, Rn, Rm, lsl)   EMIT(LOGIC_REG_gen(1, 0b01, lsl, 0, Rm, 0, Rn, Rd))
#define ORRxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORRw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 0, Rm, 0, Rn, Rd))
#define ORNx_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(1, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNw_REG(Rd, Rn, Rm)            EMIT(LOGIC_REG_gen(0, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define ORNxw_REG(Rd, Rn, Rm)           EMIT(LOGIC_REG_gen(rex.w, 0b01, 0b00, 1, Rm, 0, Rn, Rd))
#define MOVx(Rd, Rm)                    ORRx_REG(Rd, xZR, Rm)
#define MOVw(Rd, Rm)                    ORRw_REG(Rd, xZR, Rm)
#define MOVxw(Rd, Rm)                   ORRxw_REG(Rd, xZR, Rm)
#define MVNx(Rd, Rm)                    ORNx_REG(Rd, xZR, Rm)
#define MVNx_LSL(Rd, Rm, lsl)           ORNx_REG_LSL(Rd, xZR, Rm, lsl)
#define MVNw(Rd, Rm)                    ORNw_REG(Rd, xZR, Rm)
#define MVNxw(Rd, Rm)                   ORNxw_REG(Rd, xZR, Rm)
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


// BFI
#define BFM_gen(sf, opc, N, immr, imms, Rn, Rd) ((sf)<<31 | (opc)<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define BFMx(Rd, Rn, immr, imms)        EMIT(BFM_gen(1, 0b01, 1, immr, imms, Rn, Rd))
#define BFMw(Rd, Rn, immr, imms)        EMIT(BFM_gen(0, 0b01, 0, immr, imms, Rn, Rd))
#define BFMxw(Rd, Rn, immr, imms)       EMIT(BFM_gen(rex.w, 0b01, rex.w, immr, imms, Rn, Rd))
#define BFIx(Rd, Rn, lsb, width)        BFMx(Rd, Rn, (-lsb)%64, (width)-1)
#define BFIw(Rd, Rn, lsb, width)        BFMw(Rd, Rn, (-lsb)%32, (width)-1)
#define BFIxw(Rd, Rn, lsb, width)       BFMxw(Rd, Rn, (-lsb)%(rex.w?64:32), (width)-1)
#define BFCx(Rd, Rn, lsb, width)        BFMx(Rd, xZR, (-lsb)%64, (width)-1)
#define BFCw(Rd, Rn, lsb, width)        BFMw(Rd, xZR, (-lsb)%32, (width)-1)
#define BFCxw(Rd, Rn, lsb, width)       BFMxw(Rd, xZR, (-lsb)%(rex.w?64:32), (width)-1)

// UBFX
#define UBFM_gen(sf, N, immr, imms, Rn, Rd)    ((sf)<<31 | 0b10<<29 | 0b100110<<23 | (N)<<22 | (immr)<<16 | (imms)<<10 | (Rn)<<5 | (Rd))
#define UBFMx(Rd, Rn, immr, imms)       EMIT(UBFM_gen(1, 1, immr, imms, Rn, Rd))
#define UBFMw(Rd, Rn, immr, imms)       EMIT(UBFM_gen(0, 0, immr, imms, Rn, Rd))
#define UBFMxw(Rd, Rn, immr, imms)      EMIT(UBFM_gen(rex.w, rex.w, immr, imms, Rn, Rd))
#define UBFXx(Rd, Rn, lsb, width)       EMIT(UBFM_gen(1, 1, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UBFXw(Rd, Rn, lsb, width)       EMIT(UBFM_gen(0, 1, (lsb), (lsb)+(width)-1, Rn, Rd))
#define UXTBx(Rd, Rn)                   EMIT(UBFM_gen(1, 1, 0, 7, Rn, Rd))
#define UXTBw(Rd, Rn)                   EMIT(UBFM_gen(0, 1, 0, 7, Rn, Rd))
#define UXTHx(Rd, Rn)                   EMIT(UBFM_gen(1, 1, 0, 15, Rn, Rd))
#define UXTHw(Rd, Rn)                   EMIT(UBFM_gen(0, 1, 0, 15, Rn, Rd))
#define LSRx(Rd, Rn, shift)             EMIT(UBFM_gen(1, 1, shift, 63, Rn, Rd))
#define LSRw(Rd, Rn, shift)             EMIT(UBFM_gen(0, 0, shift, 31, Rn, Rd))
#define LSRxw(Rd, Rn, shift)            EMIT(UBFM_gen(rex.w, rex.w, shift, (rex.w)?63:31, Rn, Rd))
#define LSLx(Rd, Rn, lsl)               UBFMx(Rd, Rn, ((-(lsl))%64)&63, 63-(lsl))
#define LSLw(Rd, Rn, lsl)               UBFMw(Rd, Rn, ((-(lsl))%32)&31, 31-(lsl))

// LSRV
#define LSRV_gen(sf, Rm, op2, Rn, Rd)   ((sf)<<31 | 0b11010110<<21 | (Rm)<<16 | 0b0010<<12 | (op2)<<10 | (Rn)<<5 | (Rd))
#define LSRx_REG(Rd, Rn, Rm)            EMIT(LSRV_gen(1, Rm, 0b01, Rn, Rd))
#define LSRw_REG(Rd, Rn, Rm)            EMIT(LSRV_gen(0, Rm, 0b01, Rn, Rd))
#define LSRxw_REG(Rd, Rn, Rm)           EMIT(LSRV_gen(rex.w, Rm, 0b01, Rn, Rd))

// MRS
#define MRS_gen(L, o0, op1, CRn, CRm, op2, Rt)  (0b1101010100<<22 | (L)<<21 | 1<<20 | (o0)<<19 | (op1)<<16 | (CRn)<<12 | (CRm)<<8 | (op2)<<5 | (Rt))
// mrs    x0, nzcv : 1101010100 1 1 1 011 0100 0010 000 00000    o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0010(2) op2=0
#define VMRS_nzvc(Rt)                   EMIT(MRS_gen(1, 1, 3, 4, 2, 0, Rt))
#define VMSR_nzvc(Rt)                   EMIT(MRS_gen(0, 1, 3, 4, 2, 0, Rt))
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
