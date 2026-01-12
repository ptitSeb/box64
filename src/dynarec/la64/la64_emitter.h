#ifndef __LA64_EMITTER_H__
#define __LA64_EMITTER_H__
/*
    LA64 Emitter
*/

#include "la64_mapping.h"

#define RM_RNE 0b0000000000
#define RM_RZ  0b0100000000
#define RM_RP  0b1000000000
#define RM_RM  0b1100000000

// split a 32bits value in 20bits + 12bits, adjust the upper part is 12bits is negative
#define SPLIT20(A) (((A) + 0x800) >> 12)
#define SPLIT12(A) ((A) & 0xfff)

// Standard formats
#define type_4R(opc, ra, rk, rj, rd)     ((opc) << 20 | (ra) << 15 | (rk) << 10 | (rj) << 5 | (rd))
#define type_3R(opc, rk, rj, rd)         ((opc) << 15 | (rk) << 10 | (rj) << 5 | (rd))
#define type_3RI2(opc, imm2, rk, rj, rd) ((opc) << 17 | ((imm2) & 0x3) << 15 | (rk) << 10 | (rj) << 5 | (rd))
#define type_2R(opc, rj, rd)             ((opc) << 10 | (rj) << 5 | (rd))
#define type_2RI8(opc, imm8, rj, rd)     ((opc) << 18 | ((imm8) & 0xFF) << 10 | (rj) << 5 | (rd))
#define type_2RI12(opc, imm12, rj, rd)   ((opc) << 22 | ((imm12) & 0xFFF) << 10 | (rj) << 5 | (rd))
#define type_2RI14(opc, imm14, rj, rd)   ((opc) << 24 | ((imm14) & 0x3FFF) << 10 | (rj) << 5 | (rd))
#define type_2RI16(opc, imm16, rj, rd)   ((opc) << 26 | ((imm16) & 0xFFFF) << 10 | (rj) << 5 | (rd))
#define type_1RI20(opc, imm20, rd)       ((opc) << 25 | ((imm20) & 0xFFFFF) << 5 | (rd))
#define type_1RI21(opc, imm21, rj)       ((opc) << 26 | ((imm21) & 0xFFFF) << 10 | (rj) << 5 | ((imm21) & 0x1F0000) >> 16)
#define type_hint(opc, imm15)            ((opc) << 15 | ((imm15) & 0x7FFF))
#define type_I26(opc, imm26)             ((opc) << 26 | ((imm26) & 0xFFFF) << 10 | ((imm26 >> 16) & 0x3FF))

// Made-up formats not found in the spec.
#define type_1RI13(opc, imm13, rd)         ((opc) << 18 | ((imm13) & 0x1FFFF) << 5 | (rd))
#define type_2RI1(opc, imm1, rj, rd)       ((opc) << 11 | ((imm1) & 0x1) << 10 | (rj) << 5 | (rd))
#define type_2RI2(opc, imm2, rj, rd)       ((opc) << 12 | ((imm2) & 0x3) << 10 | (rj) << 5 | (rd))
#define type_2RI3(opc, imm3, rj, rd)       ((opc) << 13 | ((imm3) & 0x7) << 10 | (rj) << 5 | (rd))
#define type_2RI4(opc, imm4, rj, rd)       ((opc) << 14 | ((imm4) & 0xF) << 10 | (rj) << 5 | (rd))
#define type_2RI5(opc, imm5, rj, rd)       ((opc) << 15 | ((imm5) & 0x1F) << 10 | (rj) << 5 | (rd))
#define type_2RI6(opc, imm6, rj, rd)       ((opc) << 16 | ((imm6) & 0x3F) << 10 | (rj) << 5 | (rd))
#define type_2RI7(opc, imm7, rj, rd)       ((opc) << 17 | ((imm7) & 0x7F) << 10 | (rj) << 5 | (rd))
#define type_2RI9(opc, imm9, rj, rd)       ((opc) << 19 | ((imm9) & 0x1FF) << 10 | (rj) << 5 | (rd))
#define type_2RI10(opc, imm10, rj, rd)     ((opc) << 20 | ((imm10) & 0x3FF) << 10 | (rj) << 5 | (rd))
#define type_2RI11(opc, imm11, rj, rd)     ((opc) << 21 | ((imm11) & 0x7FF) << 10 | (rj) << 5 | (rd))
#define type_2RI13(opc, imm13, rj, rd)     ((opc) << 23 | ((imm13) & 0x1FFF) << 10 | (rj) << 5 | (rd))
#define type_1RI5I5(opc, imm5, imm5_2, rd) ((opc) << 15 | ((imm5) & 0x1F) << 10 | ((imm5_2) & 0x1F) << 5 | (rd))

// tmp = GR[rj][31:0] + GR[rk][31:0]
// Gr[rd] = SignExtend(tmp[31:0], GRLEN)
#define ADD_W(rd, rj, rk) EMIT(type_3R(0b00000000000100000, rk, rj, rd))
// tmp = GR[rj][31:0] - GR[rk][31:0]
// Gr[rd] = SignExtend(tmp[31:0], GRLEN)
#define SUB_W(rd, rj, rk) EMIT(type_3R(0b00000000000100010, rk, rj, rd))
// tmp = GR[rj][63:0] + GR[rk][63:0]
// Gr[rd] = tmp[63:0]
#define ADD_D(rd, rj, rk) EMIT(type_3R(0b00000000000100001, rk, rj, rd))
// tmp = GR[rj][63:0] - GR[rk][63:0]
// Gr[rd] = tmp[63:0]
#define SUB_D(rd, rj, rk) EMIT(type_3R(0b00000000000100011, rk, rj, rd))

// tmp = GR[rj][31:0] + SignExtend(imm12, 32)
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define ADDI_W(rd, rj, imm12) EMIT(type_2RI12(0b0000001010, imm12, rj, rd))
// tmp = GR[rj][63:0] + SignExtend(imm12, 64)
// GR[rd] = tmp[63:0]
#define ADDI_D(rd, rj, imm12) EMIT(type_2RI12(0b0000001011, imm12, rj, rd))
// tmp = GR[rj][63:0] + SignExtend({imm16, 16'b0}, 64)
// GR[rd] = tmp[63:0]
#define ADDU16I_D(rd, rj, imm16) EMIT(type_2RI16(0b000100, imm16, rj, rd))

// tmp = (GR[rj][31:0] << imm) + GR[rk][31:0]
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define ALSL_W(rd, rj, rk, imm)                                          \
    do {                                                                 \
        if (imm)                                                         \
            EMIT(type_3RI2(0b000000000000010, ((imm) - 1), rk, rj, rd)); \
        else                                                             \
            ADD_W(rd, rj, rk);                                           \
    } while (0)

// tmp = (GR[rj][31:0] << imm) + GR[rk][31:0]
// GR[rd] = ZeroExtend(tmp[31:0], GRLEN)
#define ALSL_WU(rd, rj, rk, imm)                                         \
    do {                                                                 \
        if (imm)                                                         \
            EMIT(type_3RI2(0b000000000000011, ((imm) - 1), rk, rj, rd)); \
        else {                                                           \
            ADD_W(rd, rj, rk);                                           \
            ZEROUP(rd);                                                  \
        }                                                                \
    } while (0)

// tmp = (GR[rj][63:0] << imm) + GR[rk][63:0]
// GR[rd] = tmp[63:0]
#define ALSL_D(rd, rj, rk, imm)                                          \
    do {                                                                 \
        if (imm)                                                         \
            EMIT(type_3RI2(0b000000000010110, ((imm) - 1), rk, rj, rd)); \
        else                                                             \
            ADD_D(rd, rj, rk);                                           \
    } while (0)

#define ALSLy(rd, rj, rk, imm)        \
    do {                              \
        if (rex.is32bits || rex.is67) \
            ALSL_WU(rd, rj, rk, imm); \
        else                          \
            ALSL_D(rd, rj, rk, imm);  \
    } while (0)

// GR[rd] = SignExtend({imm20, 12'b0}, GRLEN)
#define LU12I_W(rd, imm20) EMIT(type_1RI20(0b0001010, imm20, rd))
// GR[rd] = {SignExtend(imm20, 32), GR[rd][31:0]}
#define LU32I_D(rd, imm20) EMIT(type_1RI20(0b0001011, imm20, rd))
// GR[rd] = {imm12, GR[rj][51:0]}
#define LU52I_D(rd, rj, imm12) EMIT(type_2RI12(0b0000001100, imm12, rj, rd))

// GR[rd] = (signed(GR[rj]) < signed(GR[rk])) ? 1 : 0
#define SLT(rd, rj, rk) EMIT(type_3R(0b100100, rk, rj, rd))
// GR[rd] = (unsigned(GR[rj]) < unsigned(GR[rk])) ? 1 : 0
#define SLTU(rd, rj, rk) EMIT(type_3R(0b100101, rk, rj, rd))
// tmp = SignExtend(si12, GRLEN)
// GR[rd] = (signed(GR[rj]) < signed(tmp)) ? 1 : 0
#define SLTI(rd, rj, imm12) EMIT(type_2RI12(0b1000, imm12, rj, rd))
// tmp = SignExtend(si12, GRLEN)
// GR[rd] = (unsigned(GR[rj]) < unsigned(tmp)) ? 1 : 0
#define SLTUI(rd, rj, imm12) EMIT(type_2RI12(0b1001, imm12, rj, rd))

// rd = rj == 0
#define SEQZ(rd, rj) SLTUI(rd, rj, 1)
// rd = rj != 0
#define SNEZ(rd, rj) SLTU(rd, xZR, rj)

// GR[rd] = PC + SignExtend({imm20, 2'b0}, GRLEN)
#define PCADDI(rd, imm20) EMIT(type_1RI20(0b0001100, imm20, rd))
// GR[rd] = PC + SignExtend({imm20, 12'b0}, GRLEN)
#define PCADDU12I(rd, imm20) EMIT(type_1RI20(0b0001110, imm20, rd))
// GR[rd] = PC + SignExtend({imm20, 18'b0}, GRLEN)
#define PCADDU18I(rd, imm20) EMIT(type_1RI20(0b0001111, imm20, rd))
// tmp = PC + SignExtend({imm20, 12'b0}, GRLEN)
// GR[rd] = {tmp[GRLEN-1:12], 12'b0}
#define PCALAU12I(rd, imm20) EMIT(type_1RI20(0b0001101, imm20, rd))

#define LL_W(rd, rj, imm) EMIT(type_2RI14(0b00100000, imm >> 2, rj, rd))
#define SC_W(rd, rj, imm) EMIT(type_2RI14(0b00100001, imm >> 2, rj, rd))
#define LL_D(rd, rj, imm) EMIT(type_2RI14(0b00100010, imm >> 2, rj, rd))
#define SC_D(rd, rj, imm) EMIT(type_2RI14(0b00100011, imm >> 2, rj, rd))

#define SC_Q(rd, rk, rj) EMIT(type_3R(0b00111000010101110, rk, rj, rd))

#define LLxw(rd, rj, imm) EMIT(type_2RI14(0b00100000 | (rex.w ? 0b10 : 0b00), imm >> 2, rj, rd))
#define SCxw(rd, rj, imm) EMIT(type_2RI14(0b00100001 | (rex.w ? 0b10 : 0b00), imm >> 2, rj, rd))

// DBAR hint
#define DBAR(hint) EMIT(type_hint(0b00111000011100100, hint))

#define DBAR_RW_RW() DBAR(0b10000)
#define DBAR_R_RW()  DBAR(0b10100)
#define DBAR_W_RW()  DBAR(0b11000)

#define DMB_ISH()   DBAR_RW_RW()
#define DMB_ISHLD() DBAR_R_RW()
#define DMB_ISHST() DBAR_W_RW()

#define BRK(hint) EMIT(type_hint(0b00000000001010100, hint))

// GR[rd] = GR[rj] & GR[rk]
#define AND(rd, rj, rk) EMIT(type_3R(0b00000000000101001, rk, rj, rd))
// GR[rd] = GR[rj] | GR[rk]
#define OR(rd, rj, rk) EMIT(type_3R(0b00000000000101010, rk, rj, rd))
// GR[rd] = ~(GR[rj] | GR[rk])
#define NOR(rd, rj, rk) EMIT(type_3R(0b00000000000101000, rk, rj, rd))
// GR[rd] = GR[rj] ^ GR[rk]
#define XOR(rd, rj, rk) EMIT(type_3R(0b00000000000101011, rk, rj, rd))
// GR[rd] = GR[rj] & (~GR[rk])
#define ANDN(rd, rj, rk) EMIT(type_3R(0b00000000000101101, rk, rj, rd))
// GR[rd] = GR[rj] | (~GR[rk])
#define ORN(rd, rj, rk) EMIT(type_3R(0b00000000000101100, rk, rj, rd))

// GR[rd] = GR[rj] & ZeroExtend(imm12, GRLEN)
#define ANDI(rd, rj, imm12) EMIT(type_2RI12(0b0000001101, imm12, rj, rd))
// GR[rd] = GR[rj] | ZeroExtend(imm12, GRLEN)
#define ORI(rd, rj, imm12) EMIT(type_2RI12(0b0000001110, imm12, rj, rd))
// GR[rd] = GR[rj] ^ ZeroExtend(imm12, GRLEN)
#define XORI(rd, rj, imm12) EMIT(type_2RI12(0b0000001111, imm12, rj, rd))

#define NOP() ANDI(xZR, xZR, 0)

#define BREAK() EMIT(0b1010100)

// there is no UDF instruction, use BREAK instead is an acceptable offer
#define UDF() BREAK()

// tmp = SLL(GR[rj][31:0], GR[rk][4:0])
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define SLL_W(rd, rj, rk) EMIT(type_3R(0b00000000000101110, rk, rj, rd))
// tmp = SRL(GR[rj][31:0], GR[rk][4:0])
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define SRL_W(rd, rj, rk) EMIT(type_3R(0b00000000000101111, rk, rj, rd))
// tmp = SRA(GR[rj][31:0], GR[rk][4:0])
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define SRA_W(rd, rj, rk) EMIT(type_3R(0b00000000000110000, rk, rj, rd))
// tmp = ROTR(GR[rj][31:0], GR[rk][4:0])
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define ROTR_W(rd, rj, rk) EMIT(type_3R(0b00000000000110110, rk, rj, rd))

// GR[rd] = SLL(GR[rj][63:0], GR[rk][5:0])
#define SLL_D(rd, rj, rk) EMIT(type_3R(0b00000000000110001, rk, rj, rd))
// GR[rd] = SRL(GR[rj][63:0], GR[rk][5:0])
#define SRL_D(rd, rj, rk) EMIT(type_3R(0b00000000000110010, rk, rj, rd))
// GR[rd] = SRA(GR[rj][63:0], GR[rk][5:0])
#define SRA_D(rd, rj, rk) EMIT(type_3R(0b00000000000110011, rk, rj, rd))
// GR[rd] = ROTR(GR[rj][63:0], GR[rk][5:0])
#define ROTR_D(rd, rj, rk) EMIT(type_3R(0b00000000000110111, rk, rj, rd))

// GR[rd] = SLL(GR[rj][63:0], imm6) (Shift Left Logical)
#define SLLI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001000001, imm6, rj, rd))
// GR[rd] = SRL(GR[rj][63:0], imm6) (Shift Right Logical)
#define SRLI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001000101, imm6, rj, rd))
// GR[rd] = SRA(GR[rj][63:0], imm6) (Shift Right Arithmetic)
#define SRAI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001001001, imm6, rj, rd))
// GR[rd] = ROTR(GR[rj][63:0], imm6) (Rotate To Right)
#define ROTRI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001001101, imm6, rj, rd))

// GR[rd] = SLL(GR[rj][31:0], imm5) (Shift Left Logical)
#define SLLI_W(rd, rj, imm5) EMIT(type_2RI5(0b00000000010000001, imm5, rj, rd))
// GR[rd] = SRL(GR[rj][31:0], imm5) (Shift Right Logical)
#define SRLI_W(rd, rj, imm5) EMIT(type_2RI5(0b00000000010001001, imm5, rj, rd))
// GR[rd] = SRA(GR[rj][31:0], imm5) (Shift Right Arithmetic)
#define SRAI_W(rd, rj, imm5) EMIT(type_2RI5(0b00000000010010001, imm5, rj, rd))
// GR[rd] = ROTR(GR[rj][31:0], imm5) (Rotate To Right)
#define ROTRI_W(rd, rj, imm5) EMIT(type_2RI5(0b00000000010011001, imm5, rj, rd))

#define SRAxw(rd, rj, rk)      \
    do {                       \
        if (rex.w) {           \
            SRA_D(rd, rj, rk); \
        } else {               \
            SRA_W(rd, rj, rk); \
            ZEROUP(rd);        \
        }                      \
    } while (0)

#define SRLxw(rd, rj, rk)      \
    do {                       \
        if (rex.w) {           \
            SRL_D(rd, rj, rk); \
        } else {               \
            SRL_W(rd, rj, rk); \
            ZEROUP(rd);        \
        }                      \
    } while (0)

#define SLLxw(rd, rj, rk)      \
    do {                       \
        if (rex.w) {           \
            SLL_D(rd, rj, rk); \
        } else {               \
            SLL_W(rd, rj, rk); \
            ZEROUP(rd);        \
        }                      \
    } while (0)

// Shift Left Immediate
#define SLLIxw(rd, rj, imm)      \
    do {                         \
        if (rex.w) {             \
            SLLI_D(rd, rj, imm); \
        } else {                 \
            SLLI_W(rd, rj, imm); \
            ZEROUP(rd);          \
        }                        \
    } while (0)
#define SLLIy(rd, rj, imm)              \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            SLLI_W(rd, rj, imm);        \
            ZEROUP(rd);                 \
        } else                          \
            SLLI_D(rd, rj, imm);        \
    } while (0)

// Shift Right Logical Immediate
#define SRLIxw(rd, rj, imm)             \
    do {                                \
        if (rex.w) {                    \
            SRLI_D(rd, rj, imm);        \
        } else {                        \
            SRLI_W(rd, rj, imm);        \
            if ((imm) == 0) ZEROUP(rd); \
        }                               \
    } while (0)

// Shift Right Arithmetic Immediate
#define SRAIxw(rd, rj, imm)      \
    do {                         \
        if (rex.w) {             \
            SRAI_D(rd, rj, imm); \
        } else {                 \
            SRAI_W(rd, rj, imm); \
            ZEROUP(rd);          \
        }                        \
    } while (0)

#define ROTRIxw(rd, rj, imm)      \
    do {                          \
        if (rex.w) {              \
            ROTRI_D(rd, rj, imm); \
        } else {                  \
            ROTRI_W(rd, rj, imm); \
            ZEROUP(rd);           \
        }                         \
    } while (0)

#define SEXT_W(rd, rj) SLLI_W(rd, rj, 0)

// product = signed(GR[rj][31:0]) * signed(GR[rk][31:0])
// GR[rd] = SignExtend(product[31:0], GRLEN)
#define MUL_W(rd, rj, rk) EMIT(type_3R(0b00000000000111000, rk, rj, rd))

// product = signed(GR[rj][31:0]) * signed(GR[rk][31:0])
// GR[rd] = SignExtend(product[63:32], GRLEN)
#define MULH_W(rd, rj, rk) EMIT(type_3R(0b00000000000111001, rk, rj, rd))

// product = unsigned(GR[rj][31:0]) * unsigned(GR[rk][31:0])
// GR[rd] = SignExtend(product[63:32], GRLEN)
#define MULH_WU(rd, rj, rk) EMIT(type_3R(0b00000000000111010, rk, rj, rd))

// product = signed(GR[rj][63:0]) * signed(GR[rk][63:0])
// GR[rd] = product[63:0]
#define MUL_D(rd, rj, rk) EMIT(type_3R(0b00000000000111011, rk, rj, rd))

// product = signed(GR[rj][63:0]) * signed(GR[rk][63:0])
// GR[rd] = product[127:64]
#define MULH_D(rd, rj, rk) EMIT(type_3R(0b00000000000111100, rk, rj, rd))

// product = unsigned(GR[rj][63:0]) * unsigned(GR[rk][63:0])
// GR[rd] = product[127:64]
#define MULH_DU(rd, rj, rk) EMIT(type_3R(0b00000000000111101, rk, rj, rd))

// product = signed(GR[rj][31:0]) * signed(GR[rk][31:0])
// GR[rd] = product[63:0]
#define MULW_D_W(rd, rj, rk) EMIT(type_3R(0b00000000000111110, rk, rj, rd))

// product = unsigned(GR[rj][31:0]) * unsigned(GR[rk][31:0])
// GR[rd] = product[63:0]
#define MULW_D_WU(rd, rj, rk) EMIT(type_3R(0b00000000000111111, rk, rj, rd))

// quotient = signed(GR[rj][31:0]) / signed(GR[rk][31:0])
// GR[rd] = SignExtend(quotient[31:0], GRLEN)
// Warning: rj and rk must be sign-extended!
#define DIV_W(rd, rj, rk) EMIT(type_3R(0b00000000001000000, rk, rj, rd))

// quotient = unsigned(GR[rj][31:0]) / unsigned(GR[rk][31:0])
// GR[rd] = SignExtend(quotient[31:0], GRLEN)
// Warning: rj and rk must be sign-extended!
#define DIV_WU(rd, rj, rk) EMIT(type_3R(0b00000000001000010, rk, rj, rd))

// remainder = signed(GR[rj][31:0]) % signed(GR[rk][31:0])
// GR[rd] = SignExtend(remainder[31:0], GRLEN)
// Warning: rj and rk must be sign-extended!
#define MOD_W(rd, rj, rk) EMIT(type_3R(0b00000000001000001, rk, rj, rd))

// remainder = unsigned(GR[rj][31:0]) % unsigned(GR[rk][31:0])
// GR[rd] = SignExtend(remainder[31:0], GRLEN)
// Warning: rj and rk must be sign-extended!
#define MOD_WU(rd, rj, rk) EMIT(type_3R(0b00000000001000011, rk, rj, rd))

// GR[rd] = signed(GR[rj][63:0]) / signed(GR[rk][63:0])
#define DIV_D(rd, rj, rk) EMIT(type_3R(0b00000000001000100, rk, rj, rd))

// GR[rd] = unsigned(GR[rj][63:0]) / unsigned(GR[rk][63:0])
#define DIV_DU(rd, rj, rk) EMIT(type_3R(0b00000000001000110, rk, rj, rd))

// GR[rd] = signed(GR[rj] [63:0]) % signed(GR[rk] [63:0])
#define MOD_D(rd, rj, rk) EMIT(type_3R(0b00000000001000101, rk, rj, rd))

// GR[rd] = unsigned(GR[rj] [63:0]) % unsigned(GR[rk] [63:0])
#define MOD_DU(rd, rj, rk) EMIT(type_3R(0b00000000001000111, rk, rj, rd))

#define MULxw(rd, rj, rk)      \
    do {                       \
        if (rex.w) {           \
            MUL_D(rd, rj, rk); \
        } else {               \
            MUL_W(rd, rj, rk); \
        }                      \
    } while (0)


// bstr32[31:msbw+1] = GR[rd][31: msbw+1]
// bstr32[msbw:lsbw] = GR[rj][msbw-lsbw:0]
// bstr32[lsbw-1:0] = GR[rd][lsbw-1:0]
// GR[rd] = SignExtend(bstr32[31:0], GRLEN)
#define BSTRINS_W(rd, rj, msbw5, lsbw5) EMIT(type_2RI12(0b0000000001, 0b100000000000 | ((msbw5) & 0x1F) << 6 | ((lsbw5) & 0x1F), rj, rd))

// GR[rd][63:msbd+1] = GR[rd][63:msbd+1]
// GR[rd][msbd:lsbd] = GR[rj][msbd-lsbd:0]
// GR[rd][lsbd-1:0] = GR[rd][lsbd-1:0]
#define BSTRINS_D(rd, rj, msbd6, lsbd6) EMIT(type_2RI12(0b0000000010, ((msbd6) & 0x3F) << 6 | ((lsbd6) & 0x3F), rj, rd))

// bstr32[31:0] = ZeroExtend(GR[rj][msbw:lsbw], 32)
// GR[rd] = SignExtend(bstr32[31:0], GRLEN)
#define BSTRPICK_W(rd, rj, msbw5, lsbw5) EMIT(type_2RI12(0b0000000001, 0b100000100000 | ((msbw5) & 0x1F) << 6 | ((lsbw5) & 0x1F), rj, rd))

// GR[rd] = ZeroExtend(GR[rj][msbd:lsbd], 64)
#define BSTRPICK_D(rd, rj, msbd6, lsbd6) EMIT(type_2RI12(0b0000000011, ((msbd6) & 0x3F) << 6 | ((lsbd6) & 0x3F), rj, rd))

// ZERO the upper part
#define ZEROUP(rd)      BSTRPICK_D(rd, rd, 31, 0)
#define ZEROUP2(rd, rj) BSTRPICK_D(rd, rj, 31, 0)

#define BSTRINSz(rd, rj, msbd6, lsbd6)   \
    do {                                 \
        BSTRINS_D(rd, rj, msbd6, lsbd6); \
        if (rex.is32bits) ZEROUP(rd);    \
    } while (0)

#define CLO_W(rd, rj)     EMIT(type_2R(0b0000000000000000000100, rj, rd))
#define CLZ_W(rd, rj)     EMIT(type_2R(0b0000000000000000000101, rj, rd))
#define CTO_W(rd, rj)     EMIT(type_2R(0b0000000000000000000110, rj, rd))
#define CTZ_W(rd, rj)     EMIT(type_2R(0b0000000000000000000111, rj, rd))
#define CLO_D(rd, rj)     EMIT(type_2R(0b0000000000000000001000, rj, rd))
#define CLZ_D(rd, rj)     EMIT(type_2R(0b0000000000000000001001, rj, rd))
#define CTO_D(rd, rj)     EMIT(type_2R(0b0000000000000000001010, rj, rd))
#define CTZ_D(rd, rj)     EMIT(type_2R(0b0000000000000000001011, rj, rd))
#define REVB_2H(rd, rj)   EMIT(type_2R(0b0000000000000000001100, rj, rd))
#define REVB_4H(rd, rj)   EMIT(type_2R(0b0000000000000000001101, rj, rd))
#define REVB_2W(rd, rj)   EMIT(type_2R(0b0000000000000000001110, rj, rd))
#define REVB_D(rd, rj)    EMIT(type_2R(0b0000000000000000001111, rj, rd))
#define REVH_2W(rd, rj)   EMIT(type_2R(0b0000000000000000010000, rj, rd))
#define REVH_D(rd, rj)    EMIT(type_2R(0b0000000000000000010001, rj, rd))
#define BITREV_4B(rd, rj) EMIT(type_2R(0b0000000000000000010010, rj, rd))
#define BITREV_8B(rd, rj) EMIT(type_2R(0b0000000000000000010011, rj, rd))
#define BITREV_W(rd, rj)  EMIT(type_2R(0b0000000000000000010100, rj, rd))
#define BITREV_D(rd, rj)  EMIT(type_2R(0b0000000000000000010101, rj, rd))

#define REVBxw(rd, rj)       \
    do {                     \
        if (rex.w) {         \
            REVB_D(rd, rj);  \
        } else {             \
            REVB_2W(rd, rj); \
            ZEROUP(rd);      \
        }                    \
    } while (0)

#define CTZxw(rd, rj)      \
    do {                   \
        if (rex.w) {       \
            CTZ_D(rd, rj); \
        } else {           \
            CTZ_W(rd, rj); \
        }                  \
    } while (0)

#define CLZxw(rd, rj)      \
    do {                   \
        if (rex.w) {       \
            CLZ_D(rd, rj); \
        } else {           \
            CLZ_W(rd, rj); \
        }                  \
    } while (0)

// GR[rd] = SignExtend(GR[rj][7:0], GRLEN)
#define EXT_W_B(rd, rj) EMIT(type_2R(0b0000000000000000010111, rj, rd))

// GR[rd] = SignExtend(GR[rj][15:0], GRLEN)
#define EXT_W_H(rd, rj) EMIT(type_2R(0b0000000000000000010110, rj, rd))

// if GR[rj] == GR[rd]:
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BEQ(rj, rd, imm18) EMIT(type_2RI16(0b010110, ((imm18) >> 2), rj, rd))
// if GR[rj] != GR[rd]:
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BNE(rj, rd, imm18) EMIT(type_2RI16(0b010111, ((imm18) >> 2), rj, rd))
// if signed(GR[rj]) < signed(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BLT(rj, rd, imm18) EMIT(type_2RI16(0b011000, ((imm18) >> 2), rj, rd))
// if signed(GR[rj]) >= signed(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BGE(rj, rd, imm18) EMIT(type_2RI16(0b011001, ((imm18) >> 2), rj, rd))
// if unsigned(GR[rj]) == unsigned(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BLTU(rj, rd, imm18) EMIT(type_2RI16(0b011010, ((imm18) >> 2), rj, rd))
// if unsigned(GR[rj]) == unsigned(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BGEU(rj, rd, imm18) EMIT(type_2RI16(0b011011, ((imm18) >> 2), rj, rd))

// if GR[rj] == 0:
//     PC = PC + SignExtend({imm21, 2'b0}, GRLEN)
#define BEQZ(rj, imm23) EMIT(type_1RI21(0b010000, ((imm23) >> 2), rj))
// if GR[rj] != 0:
//     PC = PC + SignExtend({imm21, 2'b0}, GRLEN)
#define BNEZ(rj, imm23) EMIT(type_1RI21(0b010001, ((imm23) >> 2), rj))

#define BGT(rj, rd, imm13)  BLT(rd, rj, imm13)
#define BLE(rj, rd, imm13)  BGE(rd, rj, imm13)
#define BGTU(rj, rd, imm13) BLTU(rd, rj, imm13)
#define BLEU(rj, rd, imm13) BGEU(rd, rj, imm13)

#define SEQ(rd, rj, rk)         \
    do {                        \
        if (rj == xZR) {        \
            SEQZ(rd, rk);       \
        } else if (rk == xZR) { \
            SEQZ(rd, rj);       \
        } else {                \
            XOR(rd, rj, rk);    \
            SEQZ(rd, rd);       \
        }                       \
    } while (0)

#define SNE(rd, rj, rk)         \
    do {                        \
        if (rj == xZR) {        \
            SNEZ(rd, rk);       \
        } else if (rk == xZR) { \
            SNEZ(rd, rj);       \
        } else {                \
            XOR(rd, rj, rk);    \
            SNEZ(rd, rd);       \
        }                       \
    } while (0)

#define SGE(rd, rj, rk)      \
    do {                     \
        if (rj == xZR) {     \
            SLTI(rd, rk, 1); \
        } else {             \
            SLT(rd, rj, rk); \
            XORI(rd, rd, 1); \
        }                    \
    } while (0)

#define SGEU(rd, rj, rk)        \
    do {                        \
        if (rj == xZR) {        \
            SEQZ(rd, rk);       \
        } else if (rk == xZR) { \
            ADDI_D(rd, xZR, 1); \
        } else {                \
            SLTU(rd, rj, rk);   \
            XORI(rd, rd, 1);    \
        }                       \
    } while (0)

#define SGT(rd, rj, rk)  SLT(rd, rk, rj);
#define SLE(rd, rj, rk)  SGE(rd, rk, rj);
#define SGTU(rd, rj, rk) SLTU(rd, rk, rj);
#define SLEU(rd, rj, rk) SGEU(rd, rk, rj);

#define BCEQZ(cj, imm23) EMIT(type_1RI21(0b010010, ((imm23) >> 2), 0b00000 | cj))
#define BCNEZ(cj, imm23) EMIT(type_1RI21(0b010010, ((imm23) >> 2), 0b01000 | cj))

// GR[rd] = PC + 4
// PC = GR[rj] + SignExtend({imm16, 2'b0}, GRLEN)
#define JIRL(rd, rj, imm18) EMIT(type_2RI16(0b010011, ((imm18) >> 2), rj, rd))

// PC = GR[rj]
#define BR(rj) JIRL(xZR, rj, 0x0)

// PC = PC + SignExtend({imm26, 2'b0}, GRLEN)
#define B(imm28)               EMIT(type_I26(0b010100, ((imm28) >> 2)))
#define B__(reg1, reg2, imm28) B(imm28)

#define BEQ_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BEQ(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BNE(rj, rd, 8);                        \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BNE_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BNE(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BEQ(rj, rd, 8);                        \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BLT_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BLT(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BGE(rj, rd, 8);                        \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BGE_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BGE(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BLT(rj, rd, 8);                        \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BLTU_safe(rj, rd, imm)                     \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BLTU(rj, rd, imm);                     \
            NOP();                                 \
        } else {                                   \
            BGEU(rj, rd, 8);                       \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BGEU_safe(rj, rd, imm)                     \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BGEU(rj, rd, imm);                     \
            NOP();                                 \
        } else {                                   \
            BLTU(rj, rd, 8);                       \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BGT_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BGT(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BLE(rj, rd, 8);                        \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BLE_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BLE(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BGT(rj, rd, 8);                        \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BGTU_safe(rj, rd, imm)                     \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BGTU(rj, rd, imm);                     \
            NOP();                                 \
        } else {                                   \
            BLEU(rj, rd, 8);                       \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BLEU_safe(rj, rd, imm)                     \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BLEU(rj, rd, imm);                     \
            NOP();                                 \
        } else {                                   \
            BGTU(rj, rd, 8);                       \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BEQZ_safe(rj, imm)                         \
    do {                                           \
        if ((imm) > -0x70000 && (imm) < 0x70000) { \
            BEQZ(rj, imm);                         \
            NOP();                                 \
        } else {                                   \
            BNEZ(rj, 8);                           \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

#define BNEZ_safe(rj, imm)                         \
    do {                                           \
        if ((imm) > -0x70000 && (imm) < 0x70000) { \
            BNEZ(rj, imm);                         \
            NOP();                                 \
        } else {                                   \
            BEQZ(rj, 8);                           \
            B((imm) - 4);                          \
        }                                          \
    } while (0)

// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// byte = MemoryLoad(paddr, BYTE)
// GR[rd] = SignExtend(byte, GRLEN)
#define LD_B(rd, rj, imm12) EMIT(type_2RI12(0b0010100000, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// haldword = MemoryLoad(paddr, HALFWORD)
// GR[rd] = SignExtend(halfword, GRLEN)
#define LD_H(rd, rj, imm12) EMIT(type_2RI12(0b0010100001, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// word = MemoryLoad(paddr, WORD)
// GR[rd] = SignExtend(word, GRLEN)
#define LD_W(rd, rj, imm12) EMIT(type_2RI12(0b0010100010, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// GR[rd] = MemoryLoad(paddr, DOUBLEWORD)
#define LD_D(rd, rj, imm12) EMIT(type_2RI12(0b0010100011, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// byte = MemoryLoad(paddr, BYTE)
// GR[rd] = ZeroExtend(byte, GRLEN)
#define LD_BU(rd, rj, imm12) EMIT(type_2RI12(0b0010101000, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// halfword = MemoryLoad(paddr, HALFWORD)
// GR[rd] = ZeroExtend(halfword, GRLEN)
#define LD_HU(rd, rj, imm12) EMIT(type_2RI12(0b0010101001, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// word = MemoryLoad(paddr, WORD)
// GR[rd] = ZeroExtend(word, GRLEN)
#define LD_WU(rd, rj, imm12) EMIT(type_2RI12(0b0010101010, imm12, rj, rd))

// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// MemoryStore(GR[rd][7:0], paddr, BYTE)
#define ST_B(rd, rj, imm12) EMIT(type_2RI12(0b0010100100, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// MemoryStore(GR[rd][15:0], paddr, HALFWORD)
#define ST_H(rd, rj, imm12) EMIT(type_2RI12(0b0010100101, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// MemoryStore(GR[rd][31:0], paddr, WORD)
#define ST_W(rd, rj, imm12) EMIT(type_2RI12(0b0010100110, imm12, rj, rd))
// vaddr = GR[rj] + SignExtend(imm12, GRLEN)
// AddressComplianceCheck(vaddr)
// paddr = AddressTranslation(vaddr)
// MemoryStore(GR[rd][63:0], paddr, DOUBLEWORD)
#define ST_D(rd, rj, imm12) EMIT(type_2RI12(0b0010100111, imm12, rj, rd))

#define PRELD(hint, rj, imm12) EMIT(type_2RI12(0b0010101011, imm12, rj, hint))

#define PRELDX(hint, rj, rk) EMIT(type_3R(0b00111000001011000, rk, rj, hint))

#define PRELDX_LOAD_L3_1CACHELINE(rj, s1)                      \
    do {                                                       \
        MOV64x(s1, (0b0000000000000000ULL << 44) /* stride */  \
                | (0b00000000ULL << 32)          /* 1 block */ \
                | (0b000000ULL << 20)            /* 16-byte */ \
                | (0b0ULL << 16)                 /* asc */     \
                | 0x0000ULL);                    /* offset */  \
        PRELDX(2 /* L3 load */, rj, s1);                       \
    } while (0)

#define LDX_B(rd, rj, rk)  EMIT(type_3R(0b00111000000000000, rk, rj, rd))
#define LDX_H(rd, rj, rk)  EMIT(type_3R(0b00111000000001000, rk, rj, rd))
#define LDX_W(rd, rj, rk)  EMIT(type_3R(0b00111000000010000, rk, rj, rd))
#define LDX_D(rd, rj, rk)  EMIT(type_3R(0b00111000000011000, rk, rj, rd))
#define STX_B(rd, rj, rk)  EMIT(type_3R(0b00111000000100000, rk, rj, rd))
#define STX_H(rd, rj, rk)  EMIT(type_3R(0b00111000000101000, rk, rj, rd))
#define STX_W(rd, rj, rk)  EMIT(type_3R(0b00111000000110000, rk, rj, rd))
#define STX_D(rd, rj, rk)  EMIT(type_3R(0b00111000000111000, rk, rj, rd))
#define LDX_BU(rd, rj, rk) EMIT(type_3R(0b00111000001000000, rk, rj, rd))
#define LDX_HU(rd, rj, rk) EMIT(type_3R(0b00111000001001000, rk, rj, rd))
#define LDX_WU(rd, rj, rk) EMIT(type_3R(0b00111000001010000, rk, rj, rd))

#define CRC_W_B_W(rd, rk, rj)  EMIT(type_3R(0b00000000001001000, rk, rj, rd))
#define CRC_W_H_W(rd, rk, rj)  EMIT(type_3R(0b00000000001001001, rk, rj, rd))
#define CRC_W_W_W(rd, rk, rj)  EMIT(type_3R(0b00000000001001010, rk, rj, rd))
#define CRC_W_D_W(rd, rk, rj)  EMIT(type_3R(0b00000000001001011, rk, rj, rd))
#define CRCC_W_B_W(rd, rk, rj) EMIT(type_3R(0b00000000001001100, rk, rj, rd))
#define CRCC_W_H_W(rd, rk, rj) EMIT(type_3R(0b00000000001001101, rk, rj, rd))
#define CRCC_W_W_W(rd, rk, rj) EMIT(type_3R(0b00000000001001110, rk, rj, rd))
#define CRCC_W_D_W(rd, rk, rj) EMIT(type_3R(0b00000000001001111, rk, rj, rd))

// Beware the position of rj and rk are swapped in atomic instructions.
#define AMCAS_B(rd, rk, rj)     EMIT(type_3R(0b00111000010110000, rk, rj, rd))
#define AMCAS_H(rd, rk, rj)     EMIT(type_3R(0b00111000010110001, rk, rj, rd))
#define AMCAS_W(rd, rk, rj)     EMIT(type_3R(0b00111000010110010, rk, rj, rd))
#define AMCAS_D(rd, rk, rj)     EMIT(type_3R(0b00111000010110011, rk, rj, rd))
#define AMCAS_DB_B(rd, rk, rj)  EMIT(type_3R(0b00111000010110100, rk, rj, rd))
#define AMCAS_DB_H(rd, rk, rj)  EMIT(type_3R(0b00111000010110101, rk, rj, rd))
#define AMCAS_DB_W(rd, rk, rj)  EMIT(type_3R(0b00111000010110110, rk, rj, rd))
#define AMCAS_DB_D(rd, rk, rj)  EMIT(type_3R(0b00111000010110111, rk, rj, rd))
#define AMSWAP_B(rd, rk, rj)    EMIT(type_3R(0b00111000010111000, rk, rj, rd))
#define AMSWAP_H(rd, rk, rj)    EMIT(type_3R(0b00111000010111001, rk, rj, rd))
#define AMADD_B(rd, rk, rj)     EMIT(type_3R(0b00111000010111010, rk, rj, rd))
#define AMADD_H(rd, rk, rj)     EMIT(type_3R(0b00111000010111011, rk, rj, rd))
#define AMSWAP_DB_B(rd, rk, rj) EMIT(type_3R(0b00111000010111100, rk, rj, rd))
#define AMSWAP_DB_H(rd, rk, rj) EMIT(type_3R(0b00111000010111101, rk, rj, rd))
#define AMADD_DB_B(rd, rk, rj)  EMIT(type_3R(0b00111000010111110, rk, rj, rd))
#define AMADD_DB_H(rd, rk, rj)  EMIT(type_3R(0b00111000010111111, rk, rj, rd))
#define AMSWAP_W(rd, rk, rj)    EMIT(type_3R(0b00111000011000000, rk, rj, rd))
#define AMSWAP_D(rd, rk, rj)    EMIT(type_3R(0b00111000011000001, rk, rj, rd))
#define AMADD_W(rd, rk, rj)     EMIT(type_3R(0b00111000011000010, rk, rj, rd))
#define AMADD_D(rd, rk, rj)     EMIT(type_3R(0b00111000011000011, rk, rj, rd))
#define AMAND_W(rd, rk, rj)     EMIT(type_3R(0b00111000011000100, rk, rj, rd))
#define AMAND_D(rd, rk, rj)     EMIT(type_3R(0b00111000011000101, rk, rj, rd))
#define AMOR_W(rd, rk, rj)      EMIT(type_3R(0b00111000011000110, rk, rj, rd))
#define AMOR_D(rd, rk, rj)      EMIT(type_3R(0b00111000011000111, rk, rj, rd))
#define AMXOR_W(rd, rk, rj)     EMIT(type_3R(0b00111000011001000, rk, rj, rd))
#define AMXOR_D(rd, rk, rj)     EMIT(type_3R(0b00111000011001001, rk, rj, rd))
#define AMMAX_W(rd, rk, rj)     EMIT(type_3R(0b00111000011001010, rk, rj, rd))
#define AMMAX_D(rd, rk, rj)     EMIT(type_3R(0b00111000011001011, rk, rj, rd))
#define AMMIN_W(rd, rk, rj)     EMIT(type_3R(0b00111000011001100, rk, rj, rd))
#define AMMIN_D(rd, rk, rj)     EMIT(type_3R(0b00111000011001101, rk, rj, rd))
#define AMMAX_WU(rd, rk, rj)    EMIT(type_3R(0b00111000011001110, rk, rj, rd))
#define AMMAX_DU(rd, rk, rj)    EMIT(type_3R(0b00111000011001111, rk, rj, rd))
#define AMMIN_WU(rd, rk, rj)    EMIT(type_3R(0b00111000011010000, rk, rj, rd))
#define AMMIN_DU(rd, rk, rj)    EMIT(type_3R(0b00111000011010001, rk, rj, rd))
#define AMSWAP_DB_W(rd, rk, rj) EMIT(type_3R(0b00111000011010010, rk, rj, rd))
#define AMSWAP_DB_D(rd, rk, rj) EMIT(type_3R(0b00111000011010011, rk, rj, rd))
#define AMADD_DB_W(rd, rk, rj)  EMIT(type_3R(0b00111000011010100, rk, rj, rd))
#define AMADD_DB_D(rd, rk, rj)  EMIT(type_3R(0b00111000011010101, rk, rj, rd))
#define AMAND_DB_W(rd, rk, rj)  EMIT(type_3R(0b00111000011010110, rk, rj, rd))
#define AMAND_DB_D(rd, rk, rj)  EMIT(type_3R(0b00111000011010111, rk, rj, rd))
#define AMOR_DB_W(rd, rk, rj)   EMIT(type_3R(0b00111000011011000, rk, rj, rd))
#define AMOR_DB_D(rd, rk, rj)   EMIT(type_3R(0b00111000011011001, rk, rj, rd))
#define AMXOR_DB_W(rd, rk, rj)  EMIT(type_3R(0b00111000011011010, rk, rj, rd))
#define AMXOR_DB_D(rd, rk, rj)  EMIT(type_3R(0b00111000011011011, rk, rj, rd))
#define AMMAX_DB_W(rd, rk, rj)  EMIT(type_3R(0b00111000011011100, rk, rj, rd))
#define AMMAX_DB_D(rd, rk, rj)  EMIT(type_3R(0b00111000011011101, rk, rj, rd))
#define AMMIN_DB_W(rd, rk, rj)  EMIT(type_3R(0b00111000011011110, rk, rj, rd))
#define AMMIN_DB_D(rd, rk, rj)  EMIT(type_3R(0b00111000011011111, rk, rj, rd))
#define AMMAX_DB_WU(rd, rk, rj) EMIT(type_3R(0b00111000011100000, rk, rj, rd))
#define AMMAX_DB_DU(rd, rk, rj) EMIT(type_3R(0b00111000011100001, rk, rj, rd))
#define AMMIN_DB_WU(rd, rk, rj) EMIT(type_3R(0b00111000011100010, rk, rj, rd))
#define AMMIN_DB_DU(rd, rk, rj) EMIT(type_3R(0b00111000011100011, rk, rj, rd))

#define AMAND_DBxw(rd, rk, rj) EMIT(type_3R(0b00111000011010110 | rex.w, rk, rj, rd))

#define FLD_D(fd, rj, imm12) EMIT(type_2RI12(0b0010101110, imm12, rj, fd))
#define FLD_S(fd, rj, imm12) EMIT(type_2RI12(0b0010101100, imm12, rj, fd))
#define FST_D(fd, rj, imm12) EMIT(type_2RI12(0b0010101111, imm12, rj, fd))
#define FST_S(fd, rj, imm12) EMIT(type_2RI12(0b0010101101, imm12, rj, fd))
#define FLDX_D(fd, rj, rk)   EMIT(type_3R(0b00111000001101000, rk, rj, fd))
#define FLDX_S(fd, rj, rk)   EMIT(type_3R(0b00111000001100000, rk, rj, fd))
#define FSTX_D(fd, rj, rk)   EMIT(type_3R(0b00111000001111000, rk, rj, fd))
#define FSTX_S(fd, rj, rk)   EMIT(type_3R(0b00111000001110000, rk, rj, fd))
#define FLDGT_D(fd, rj, rk)  EMIT(type_3R(0b00111000011101001, rk, rj, fd))
#define FLDGT_S(fd, rj, rk)  EMIT(type_3R(0b00111000011101000, rk, rj, fd))
#define FLDLE_D(fd, rj, rk)  EMIT(type_3R(0b00111000011101011, rk, rj, fd))
#define FLDLE_S(fd, rj, rk)  EMIT(type_3R(0b00111000011101010, rk, rj, fd))
#define FSTGT_D(fd, rj, rk)  EMIT(type_3R(0b00111000011101101, rk, rj, fd))
#define FSTGT_S(fd, rj, rk)  EMIT(type_3R(0b00111000011101100, rk, rj, fd))
#define FSTLE_D(fd, rj, rk)  EMIT(type_3R(0b00111000011101111, rk, rj, fd))
#define FSTLE_S(fd, rj, rk)  EMIT(type_3R(0b00111000011101110, rk, rj, fd))

#define FADD_S(fd, fj, fk)       EMIT(type_3R(0b00000001000000001, fk, fj, fd))
#define FADD_D(fd, fj, fk)       EMIT(type_3R(0b00000001000000010, fk, fj, fd))
#define FSUB_S(fd, fj, fk)       EMIT(type_3R(0b00000001000000101, fk, fj, fd))
#define FSUB_D(fd, fj, fk)       EMIT(type_3R(0b00000001000000110, fk, fj, fd))
#define FMUL_S(fd, fj, fk)       EMIT(type_3R(0b00000001000001001, fk, fj, fd))
#define FMUL_D(fd, fj, fk)       EMIT(type_3R(0b00000001000001010, fk, fj, fd))
#define FDIV_S(fd, fj, fk)       EMIT(type_3R(0b00000001000001101, fk, fj, fd))
#define FDIV_D(fd, fj, fk)       EMIT(type_3R(0b00000001000001110, fk, fj, fd))
#define FMAX_S(fd, fj, fk)       EMIT(type_3R(0b00000001000010001, fk, fj, fd))
#define FMAX_D(fd, fj, fk)       EMIT(type_3R(0b00000001000010010, fk, fj, fd))
#define FMIN_S(fd, fj, fk)       EMIT(type_3R(0b00000001000010101, fk, fj, fd))
#define FMIN_D(fd, fj, fk)       EMIT(type_3R(0b00000001000010110, fk, fj, fd))
#define FMAXA_S(fd, fj, fk)      EMIT(type_3R(0b00000001000011001, fk, fj, fd))
#define FMAXA_D(fd, fj, fk)      EMIT(type_3R(0b00000001000011010, fk, fj, fd))
#define FMINA_S(fd, fj, fk)      EMIT(type_3R(0b00000001000011101, fk, fj, fd))
#define FMINA_D(fd, fj, fk)      EMIT(type_3R(0b00000001000011110, fk, fj, fd))
#define FSCALEB_S(fd, fj, fk)    EMIT(type_3R(0b00000001000100001, fk, fj, fd))
#define FSCALEB_D(fd, fj, fk)    EMIT(type_3R(0b00000001000100010, fk, fj, fd))
#define FCOPYSIGN_S(fd, fj, fk)  EMIT(type_3R(0b00000001000100101, fk, fj, fd))
#define FCOPYSIGN_D(fd, fj, fk)  EMIT(type_3R(0b00000001000100110, fk, fj, fd))
#define FABS_S(fd, fj)           EMIT(type_2R(0b0000000100010100000001, fj, fd))
#define FABS_D(fd, fj)           EMIT(type_2R(0b0000000100010100000010, fj, fd))
#define FNEG_S(fd, fj)           EMIT(type_2R(0b0000000100010100000101, fj, fd))
#define FNEG_D(fd, fj)           EMIT(type_2R(0b0000000100010100000110, fj, fd))
#define FLOGB_S(fd, fj)          EMIT(type_2R(0b0000000100010100001001, fj, fd))
#define FLOGB_D(fd, fj)          EMIT(type_2R(0b0000000100010100001010, fj, fd))
#define FCLASS_S(fd, fj)         EMIT(type_2R(0b0000000100010100001101, fj, fd))
#define FCLASS_D(fd, fj)         EMIT(type_2R(0b0000000100010100001110, fj, fd))
#define FSQRT_S(fd, fj)          EMIT(type_2R(0b0000000100010100010001, fj, fd))
#define FSQRT_D(fd, fj)          EMIT(type_2R(0b0000000100010100010010, fj, fd))
#define FRECIP_S(fd, fj)         EMIT(type_2R(0b0000000100010100010101, fj, fd))
#define FRECIP_D(fd, fj)         EMIT(type_2R(0b0000000100010100010110, fj, fd))
#define FRSQRT_S(fd, fj)         EMIT(type_2R(0b0000000100010100011001, fj, fd))
#define FRSQRT_D(fd, fj)         EMIT(type_2R(0b0000000100010100011010, fj, fd))
#define FRECIPE_S(fd, fj)        EMIT(type_2R(0b0000000100010100011101, fj, fd))
#define FRECIPE_D(fd, fj)        EMIT(type_2R(0b0000000100010100011110, fj, fd))
#define FRSQRTE_S(fd, fj)        EMIT(type_2R(0b0000000100010100100001, fj, fd))
#define FRSQRTE_D(fd, fj)        EMIT(type_2R(0b0000000100010100100010, fj, fd))
#define FMOV_S(fd, fj)           EMIT(type_2R(0b0000000100010100100101, fj, fd))
#define FMOV_D(fd, fj)           EMIT(type_2R(0b0000000100010100100110, fj, fd))
#define MOVGR2FR_W(fd, rj)       EMIT(type_2R(0b0000000100010100101001, rj, fd))
#define MOVGR2FR_D(fd, rj)       EMIT(type_2R(0b0000000100010100101010, rj, fd))
#define MOVGR2FRH_W(fd, rj)      EMIT(type_2R(0b0000000100010100101011, rj, fd))
#define MOVFR2GR_S(rd, fj)       EMIT(type_2R(0b0000000100010100101101, fj, rd))
#define MOVFR2GR_D(rd, fj)       EMIT(type_2R(0b0000000100010100101110, fj, rd))
#define MOVFRH2GR_S(rd, fj)      EMIT(type_2R(0b0000000100010100101111, fj, rd))
#define MOVGR2FCSR(fcsr, rj)     EMIT(type_2R(0b0000000100010100110000, rj, fcsr))
#define MOVFCSR2GR(rd, fcsr)     EMIT(type_2R(0b0000000100010100110010, fcsr, rd))
#define MOVFR2CF(cd, fj)         EMIT(type_2R(0b0000000100010100110100, fj, cd & 0b111))
#define MOVCF2FR(fd, cj)         EMIT(type_2R(0b0000000100010100110101, cj & 0b111, fd))
#define MOVGR2CF(cd, rj)         EMIT(type_2R(0b0000000100010100110110, rj, cd & 0b111))
#define MOVCF2GR(rd, cj)         EMIT(type_2R(0b0000000100010100110111, cj & 0b111, rd))
#define FCVT_S_D(fd, fj)         EMIT(type_2R(0b0000000100011001000110, fj, fd))
#define FCVT_D_S(fd, fj)         EMIT(type_2R(0b0000000100011001001001, fj, fd))
#define FTINTRM_W_S(fd, fj)      EMIT(type_2R(0b0000000100011010000001, fj, fd))
#define FTINTRM_W_D(fd, fj)      EMIT(type_2R(0b0000000100011010000010, fj, fd))
#define FTINTRM_L_S(fd, fj)      EMIT(type_2R(0b0000000100011010001001, fj, fd))
#define FTINTRM_L_D(fd, fj)      EMIT(type_2R(0b0000000100011010001010, fj, fd))
#define FTINTRP_W_S(fd, fj)      EMIT(type_2R(0b0000000100011010010001, fj, fd))
#define FTINTRP_W_D(fd, fj)      EMIT(type_2R(0b0000000100011010010010, fj, fd))
#define FTINTRP_L_S(fd, fj)      EMIT(type_2R(0b0000000100011010011001, fj, fd))
#define FTINTRP_L_D(fd, fj)      EMIT(type_2R(0b0000000100011010011010, fj, fd))
#define FTINTRZ_W_S(fd, fj)      EMIT(type_2R(0b0000000100011010100001, fj, fd))
#define FTINTRZ_W_D(fd, fj)      EMIT(type_2R(0b0000000100011010100010, fj, fd))
#define FTINTRZ_L_S(fd, fj)      EMIT(type_2R(0b0000000100011010101001, fj, fd))
#define FTINTRZ_L_D(fd, fj)      EMIT(type_2R(0b0000000100011010101010, fj, fd))
#define FTINTRNE_W_S(fd, fj)     EMIT(type_2R(0b0000000100011010110001, fj, fd))
#define FTINTRNE_W_D(fd, fj)     EMIT(type_2R(0b0000000100011010110010, fj, fd))
#define FTINTRNE_L_S(fd, fj)     EMIT(type_2R(0b0000000100011010111001, fj, fd))
#define FTINTRNE_L_D(fd, fj)     EMIT(type_2R(0b0000000100011010111010, fj, fd))
#define FTINT_W_S(fd, fj)        EMIT(type_2R(0b0000000100011011000001, fj, fd))
#define FTINT_W_D(fd, fj)        EMIT(type_2R(0b0000000100011011000010, fj, fd))
#define FTINT_L_S(fd, fj)        EMIT(type_2R(0b0000000100011011001001, fj, fd))
#define FTINT_L_D(fd, fj)        EMIT(type_2R(0b0000000100011011001010, fj, fd))
#define FFINT_S_W(fd, fj)        EMIT(type_2R(0b0000000100011101000100, fj, fd))
#define FFINT_S_L(fd, fj)        EMIT(type_2R(0b0000000100011101000110, fj, fd))
#define FFINT_D_W(fd, fj)        EMIT(type_2R(0b0000000100011101001000, fj, fd))
#define FFINT_D_L(fd, fj)        EMIT(type_2R(0b0000000100011101001010, fj, fd))
#define FRINT_S(fd, fj)          EMIT(type_2R(0b0000000100011110010001, fj, fd))
#define FRINT_D(fd, fj)          EMIT(type_2R(0b0000000100011110010010, fj, fd))
#define FMADD_S(fd, fj, fk, fa)  EMIT(type_4R(0b000010000001, fa, fk, fj, fd))
#define FMADD_D(fd, fj, fk, fa)  EMIT(type_4R(0b000010000010, fa, fk, fj, fd))
#define FMSUB_S(fd, fj, fk, fa)  EMIT(type_4R(0b000010000101, fa, fk, fj, fd))
#define FMSUB_D(fd, fj, fk, fa)  EMIT(type_4R(0b000010000110, fa, fk, fj, fd))
#define FNMADD_S(fd, fj, fk, fa) EMIT(type_4R(0b000010001001, fa, fk, fj, fd))
#define FNMADD_D(fd, fj, fk, fa) EMIT(type_4R(0b000010001010, fa, fk, fj, fd))
#define FNMSUB_S(fd, fj, fk, fa) EMIT(type_4R(0b000010001101, fa, fk, fj, fd))
#define FNMSUB_D(fd, fj, fk, fa) EMIT(type_4R(0b000010001110, fa, fk, fj, fd))
#define FCMP_S(cd, fj, fk, cond) EMIT(type_4R(0b000011000001, cond, fk, fj, cd & 0b111))
#define FCMP_D(cd, fj, fk, cond) EMIT(type_4R(0b000011000010, cond, fk, fj, cd & 0b111))
#define FSEL(fd, fj, fk, ca)     EMIT(type_4R(0b000011010000, ca & 0b111, fk, fj, fd))

#define RDTIME_D(rd, rj) EMIT(type_2R(0b11010, rj, rd))

////////////////////////////////////////////////////////////////////////////////
// (undocumented) LSX/LASX extension instructions

/*

Register alias:

+-----------------------------------------------+----------------+
|                                               |   F1 (64-bit)  | FP
|------------------------------+----------------+----------------+
|                              |           VR1 (128-bit)         | LSX
|------------------------------+---------------------------------+
|                       XR1 (256-bit)                            | LASX
+----------------------------------------------------------------+

LSX instruction starts with V, LASX instruction starts with XV.

*/

#define VADD_B(vd, vj, vk)           EMIT(type_3R(0b01110000000010100, vk, vj, vd))
#define VADD_H(vd, vj, vk)           EMIT(type_3R(0b01110000000010101, vk, vj, vd))
#define VADD_W(vd, vj, vk)           EMIT(type_3R(0b01110000000010110, vk, vj, vd))
#define VADD_D(vd, vj, vk)           EMIT(type_3R(0b01110000000010111, vk, vj, vd))
#define VADD_Q(vd, vj, vk)           EMIT(type_3R(0b01110001001011010, vk, vj, vd))
#define VSUB_B(vd, vj, vk)           EMIT(type_3R(0b01110000000011000, vk, vj, vd))
#define VSUB_H(vd, vj, vk)           EMIT(type_3R(0b01110000000011001, vk, vj, vd))
#define VSUB_W(vd, vj, vk)           EMIT(type_3R(0b01110000000011010, vk, vj, vd))
#define VSUB_D(vd, vj, vk)           EMIT(type_3R(0b01110000000011011, vk, vj, vd))
#define VSUB_Q(vd, vj, vk)           EMIT(type_3R(0b01110001001011011, vk, vj, vd))
#define VADDI_BU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100010100, imm5, vj, vd))
#define VADDI_HU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100010101, imm5, vj, vd))
#define VADDI_WU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100010110, imm5, vj, vd))
#define VADDI_DU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100010111, imm5, vj, vd))
#define VSUBI_BU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100011000, imm5, vj, vd))
#define VSUBI_HU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100011001, imm5, vj, vd))
#define VSUBI_WU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100011010, imm5, vj, vd))
#define VSUBI_DU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100011011, imm5, vj, vd))
#define VSADD_B(vd, vj, vk)          EMIT(type_3R(0b01110000010001100, vk, vj, vd))
#define VSADD_H(vd, vj, vk)          EMIT(type_3R(0b01110000010001101, vk, vj, vd))
#define VSADD_W(vd, vj, vk)          EMIT(type_3R(0b01110000010001110, vk, vj, vd))
#define VSADD_D(vd, vj, vk)          EMIT(type_3R(0b01110000010001111, vk, vj, vd))
#define VSADD_BU(vd, vj, vk)         EMIT(type_3R(0b01110000010010100, vk, vj, vd))
#define VSADD_HU(vd, vj, vk)         EMIT(type_3R(0b01110000010010101, vk, vj, vd))
#define VSADD_WU(vd, vj, vk)         EMIT(type_3R(0b01110000010010110, vk, vj, vd))
#define VSADD_DU(vd, vj, vk)         EMIT(type_3R(0b01110000010010111, vk, vj, vd))
#define VSSUB_B(vd, vj, vk)          EMIT(type_3R(0b01110000010010000, vk, vj, vd))
#define VSSUB_H(vd, vj, vk)          EMIT(type_3R(0b01110000010010001, vk, vj, vd))
#define VSSUB_W(vd, vj, vk)          EMIT(type_3R(0b01110000010010010, vk, vj, vd))
#define VSSUB_D(vd, vj, vk)          EMIT(type_3R(0b01110000010010011, vk, vj, vd))
#define VSSUB_BU(vd, vj, vk)         EMIT(type_3R(0b01110000010011000, vk, vj, vd))
#define VSSUB_HU(vd, vj, vk)         EMIT(type_3R(0b01110000010011001, vk, vj, vd))
#define VSSUB_WU(vd, vj, vk)         EMIT(type_3R(0b01110000010011010, vk, vj, vd))
#define VSSUB_DU(vd, vj, vk)         EMIT(type_3R(0b01110000010011011, vk, vj, vd))
#define VHADDW_H_B(vd, vj, vk)       EMIT(type_3R(0b01110000010101000, vk, vj, vd))
#define VHADDW_W_H(vd, vj, vk)       EMIT(type_3R(0b01110000010101001, vk, vj, vd))
#define VHADDW_D_W(vd, vj, vk)       EMIT(type_3R(0b01110000010101010, vk, vj, vd))
#define VHADDW_Q_D(vd, vj, vk)       EMIT(type_3R(0b01110000010101011, vk, vj, vd))
#define VHADDW_HU_BU(vd, vj, vk)     EMIT(type_3R(0b01110000010110000, vk, vj, vd))
#define VHADDW_WU_HU(vd, vj, vk)     EMIT(type_3R(0b01110000010110001, vk, vj, vd))
#define VHADDW_DU_WU(vd, vj, vk)     EMIT(type_3R(0b01110000010110010, vk, vj, vd))
#define VHADDW_QU_DU(vd, vj, vk)     EMIT(type_3R(0b01110000010110011, vk, vj, vd))
#define VHSUBW_H_B(vd, vj, vk)       EMIT(type_3R(0b01110000010101100, vk, vj, vd))
#define VHSUBW_W_H(vd, vj, vk)       EMIT(type_3R(0b01110000010101101, vk, vj, vd))
#define VHSUBW_D_W(vd, vj, vk)       EMIT(type_3R(0b01110000010101110, vk, vj, vd))
#define VHSUBW_Q_D(vd, vj, vk)       EMIT(type_3R(0b01110000010101111, vk, vj, vd))
#define VHSUBW_HU_BU(vd, vj, vk)     EMIT(type_3R(0b01110000010110100, vk, vj, vd))
#define VHSUBW_WU_HU(vd, vj, vk)     EMIT(type_3R(0b01110000010110101, vk, vj, vd))
#define VHSUBW_DU_WU(vd, vj, vk)     EMIT(type_3R(0b01110000010110110, vk, vj, vd))
#define VHSUBW_QU_DU(vd, vj, vk)     EMIT(type_3R(0b01110000010110111, vk, vj, vd))
#define VADDWEV_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000000111100, vk, vj, vd))
#define VADDWEV_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000000111101, vk, vj, vd))
#define VADDWEV_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000000111110, vk, vj, vd))
#define VADDWEV_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000000111111, vk, vj, vd))
#define VADDWOD_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000001000100, vk, vj, vd))
#define VADDWOD_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000001000101, vk, vj, vd))
#define VADDWOD_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000001000110, vk, vj, vd))
#define VADDWOD_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000001000111, vk, vj, vd))
#define VSUBWEV_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000001000000, vk, vj, vd))
#define VSUBWEV_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000001000001, vk, vj, vd))
#define VSUBWEV_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000001000010, vk, vj, vd))
#define VSUBWEV_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000001000011, vk, vj, vd))
#define VSUBWOD_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000001001000, vk, vj, vd))
#define VSUBWOD_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000001001001, vk, vj, vd))
#define VSUBWOD_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000001001010, vk, vj, vd))
#define VSUBWOD_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000001001011, vk, vj, vd))
#define VADDWEV_H_BU(vd, vj, vk)     EMIT(type_3R(0b01110000001011100, vk, vj, vd))
#define VADDWEV_W_HU(vd, vj, vk)     EMIT(type_3R(0b01110000001011101, vk, vj, vd))
#define VADDWEV_D_WU(vd, vj, vk)     EMIT(type_3R(0b01110000001011110, vk, vj, vd))
#define VADDWEV_Q_DU(vd, vj, vk)     EMIT(type_3R(0b01110000001011111, vk, vj, vd))
#define VADDWOD_H_BU(vd, vj, vk)     EMIT(type_3R(0b01110000001100100, vk, vj, vd))
#define VADDWOD_W_HU(vd, vj, vk)     EMIT(type_3R(0b01110000001100101, vk, vj, vd))
#define VADDWOD_D_WU(vd, vj, vk)     EMIT(type_3R(0b01110000001100110, vk, vj, vd))
#define VADDWOD_Q_DU(vd, vj, vk)     EMIT(type_3R(0b01110000001100111, vk, vj, vd))
#define VSUBWEV_H_BU(vd, vj, vk)     EMIT(type_3R(0b01110000001100000, vk, vj, vd))
#define VSUBWEV_W_HU(vd, vj, vk)     EMIT(type_3R(0b01110000001100001, vk, vj, vd))
#define VSUBWEV_D_WU(vd, vj, vk)     EMIT(type_3R(0b01110000001100010, vk, vj, vd))
#define VSUBWEV_Q_DU(vd, vj, vk)     EMIT(type_3R(0b01110000001100011, vk, vj, vd))
#define VSUBWOD_H_BU(vd, vj, vk)     EMIT(type_3R(0b01110000001101000, vk, vj, vd))
#define VSUBWOD_W_HU(vd, vj, vk)     EMIT(type_3R(0b01110000001101001, vk, vj, vd))
#define VSUBWOD_D_WU(vd, vj, vk)     EMIT(type_3R(0b01110000001101010, vk, vj, vd))
#define VSUBWOD_Q_DU(vd, vj, vk)     EMIT(type_3R(0b01110000001101011, vk, vj, vd))
#define VADDWEV_H_BU_B(vd, vj, vk)   EMIT(type_3R(0b01110000001111100, vk, vj, vd))
#define VADDWEV_W_HU_H(vd, vj, vk)   EMIT(type_3R(0b01110000001111101, vk, vj, vd))
#define VADDWEV_D_WU_W(vd, vj, vk)   EMIT(type_3R(0b01110000001111110, vk, vj, vd))
#define VADDWEV_Q_DU_D(vd, vj, vk)   EMIT(type_3R(0b01110000001111111, vk, vj, vd))
#define VADDWOD_H_BU_B(vd, vj, vk)   EMIT(type_3R(0b01110000010000000, vk, vj, vd))
#define VADDWOD_W_HU_H(vd, vj, vk)   EMIT(type_3R(0b01110000010000001, vk, vj, vd))
#define VADDWOD_D_WU_W(vd, vj, vk)   EMIT(type_3R(0b01110000010000010, vk, vj, vd))
#define VADDWOD_Q_DU_D(vd, vj, vk)   EMIT(type_3R(0b01110000010000011, vk, vj, vd))
#define VAVG_B(vd, vj, vk)           EMIT(type_3R(0b01110000011001000, vk, vj, vd))
#define VAVG_H(vd, vj, vk)           EMIT(type_3R(0b01110000011001001, vk, vj, vd))
#define VAVG_W(vd, vj, vk)           EMIT(type_3R(0b01110000011001010, vk, vj, vd))
#define VAVG_D(vd, vj, vk)           EMIT(type_3R(0b01110000011001011, vk, vj, vd))
#define VAVG_BU(vd, vj, vk)          EMIT(type_3R(0b01110000011001100, vk, vj, vd))
#define VAVG_HU(vd, vj, vk)          EMIT(type_3R(0b01110000011001101, vk, vj, vd))
#define VAVG_WU(vd, vj, vk)          EMIT(type_3R(0b01110000011001110, vk, vj, vd))
#define VAVG_DU(vd, vj, vk)          EMIT(type_3R(0b01110000011001111, vk, vj, vd))
#define VAVGR_B(vd, vj, vk)          EMIT(type_3R(0b01110000011010000, vk, vj, vd))
#define VAVGR_H(vd, vj, vk)          EMIT(type_3R(0b01110000011010001, vk, vj, vd))
#define VAVGR_W(vd, vj, vk)          EMIT(type_3R(0b01110000011010010, vk, vj, vd))
#define VAVGR_D(vd, vj, vk)          EMIT(type_3R(0b01110000011010011, vk, vj, vd))
#define VAVGR_BU(vd, vj, vk)         EMIT(type_3R(0b01110000011010100, vk, vj, vd))
#define VAVGR_HU(vd, vj, vk)         EMIT(type_3R(0b01110000011010101, vk, vj, vd))
#define VAVGR_WU(vd, vj, vk)         EMIT(type_3R(0b01110000011010110, vk, vj, vd))
#define VAVGR_DU(vd, vj, vk)         EMIT(type_3R(0b01110000011010111, vk, vj, vd))
#define VABSD_B(vd, vj, vk)          EMIT(type_3R(0b01110000011000000, vk, vj, vd))
#define VABSD_H(vd, vj, vk)          EMIT(type_3R(0b01110000011000001, vk, vj, vd))
#define VABSD_W(vd, vj, vk)          EMIT(type_3R(0b01110000011000010, vk, vj, vd))
#define VABSD_D(vd, vj, vk)          EMIT(type_3R(0b01110000011000011, vk, vj, vd))
#define VABSD_BU(vd, vj, vk)         EMIT(type_3R(0b01110000011000100, vk, vj, vd))
#define VABSD_HU(vd, vj, vk)         EMIT(type_3R(0b01110000011000101, vk, vj, vd))
#define VABSD_WU(vd, vj, vk)         EMIT(type_3R(0b01110000011000110, vk, vj, vd))
#define VABSD_DU(vd, vj, vk)         EMIT(type_3R(0b01110000011000111, vk, vj, vd))
#define VADDA_B(vd, vj, vk)          EMIT(type_3R(0b01110000010111000, vk, vj, vd))
#define VADDA_H(vd, vj, vk)          EMIT(type_3R(0b01110000010111001, vk, vj, vd))
#define VADDA_W(vd, vj, vk)          EMIT(type_3R(0b01110000010111010, vk, vj, vd))
#define VADDA_D(vd, vj, vk)          EMIT(type_3R(0b01110000010111011, vk, vj, vd))
#define VMAXI_B(vd, vj, imm5)        EMIT(type_3R(0b01110010100100000, imm5, vj, vd))
#define VMAXI_H(vd, vj, imm5)        EMIT(type_3R(0b01110010100100001, imm5, vj, vd))
#define VMAXI_W(vd, vj, imm5)        EMIT(type_3R(0b01110010100100010, imm5, vj, vd))
#define VMAXI_D(vd, vj, imm5)        EMIT(type_3R(0b01110010100100011, imm5, vj, vd))
#define VMAXI_BU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101000, imm5, vj, vd))
#define VMAXI_HU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101001, imm5, vj, vd))
#define VMAXI_WU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101010, imm5, vj, vd))
#define VMAXI_DU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101011, imm5, vj, vd))
#define VMAX_B(vd, vj, vk)           EMIT(type_3R(0b01110000011100000, vk, vj, vd))
#define VMAX_H(vd, vj, vk)           EMIT(type_3R(0b01110000011100001, vk, vj, vd))
#define VMAX_W(vd, vj, vk)           EMIT(type_3R(0b01110000011100010, vk, vj, vd))
#define VMAX_D(vd, vj, vk)           EMIT(type_3R(0b01110000011100011, vk, vj, vd))
#define VMAX_BU(vd, vj, vk)          EMIT(type_3R(0b01110000011101000, vk, vj, vd))
#define VMAX_HU(vd, vj, vk)          EMIT(type_3R(0b01110000011101001, vk, vj, vd))
#define VMAX_WU(vd, vj, vk)          EMIT(type_3R(0b01110000011101010, vk, vj, vd))
#define VMAX_DU(vd, vj, vk)          EMIT(type_3R(0b01110000011101011, vk, vj, vd))
#define VMINI_B(vd, vj, imm5)        EMIT(type_3R(0b01110010100100100, imm5, vj, vd))
#define VMINI_H(vd, vj, imm5)        EMIT(type_3R(0b01110010100100101, imm5, vj, vd))
#define VMINI_W(vd, vj, imm5)        EMIT(type_3R(0b01110010100100110, imm5, vj, vd))
#define VMINI_D(vd, vj, imm5)        EMIT(type_3R(0b01110010100100111, imm5, vj, vd))
#define VMINI_BU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101100, imm5, vj, vd))
#define VMINI_HU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101101, imm5, vj, vd))
#define VMINI_WU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101110, imm5, vj, vd))
#define VMINI_DU(vd, vj, imm5)       EMIT(type_3R(0b01110010100101111, imm5, vj, vd))
#define VMIN_B(vd, vj, vk)           EMIT(type_3R(0b01110000011100100, vk, vj, vd))
#define VMIN_H(vd, vj, vk)           EMIT(type_3R(0b01110000011100101, vk, vj, vd))
#define VMIN_W(vd, vj, vk)           EMIT(type_3R(0b01110000011100110, vk, vj, vd))
#define VMIN_D(vd, vj, vk)           EMIT(type_3R(0b01110000011100111, vk, vj, vd))
#define VMIN_BU(vd, vj, vk)          EMIT(type_3R(0b01110000011101100, vk, vj, vd))
#define VMIN_HU(vd, vj, vk)          EMIT(type_3R(0b01110000011101101, vk, vj, vd))
#define VMIN_WU(vd, vj, vk)          EMIT(type_3R(0b01110000011101110, vk, vj, vd))
#define VMIN_DU(vd, vj, vk)          EMIT(type_3R(0b01110000011101111, vk, vj, vd))
#define VMUL_B(vd, vj, vk)           EMIT(type_3R(0b01110000100001000, vk, vj, vd))
#define VMUL_H(vd, vj, vk)           EMIT(type_3R(0b01110000100001001, vk, vj, vd))
#define VMUL_W(vd, vj, vk)           EMIT(type_3R(0b01110000100001010, vk, vj, vd))
#define VMUL_D(vd, vj, vk)           EMIT(type_3R(0b01110000100001011, vk, vj, vd))
#define VMUH_B(vd, vj, vk)           EMIT(type_3R(0b01110000100001100, vk, vj, vd))
#define VMUH_H(vd, vj, vk)           EMIT(type_3R(0b01110000100001101, vk, vj, vd))
#define VMUH_W(vd, vj, vk)           EMIT(type_3R(0b01110000100001110, vk, vj, vd))
#define VMUH_D(vd, vj, vk)           EMIT(type_3R(0b01110000100001111, vk, vj, vd))
#define VMUH_BU(vd, vj, vk)          EMIT(type_3R(0b01110000100010000, vk, vj, vd))
#define VMUH_HU(vd, vj, vk)          EMIT(type_3R(0b01110000100010001, vk, vj, vd))
#define VMUH_WU(vd, vj, vk)          EMIT(type_3R(0b01110000100010010, vk, vj, vd))
#define VMUH_DU(vd, vj, vk)          EMIT(type_3R(0b01110000100010011, vk, vj, vd))
#define VMULWEV_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000100100000, vk, vj, vd))
#define VMULWEV_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000100100001, vk, vj, vd))
#define VMULWEV_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000100100010, vk, vj, vd))
#define VMULWEV_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000100100011, vk, vj, vd))
#define VMULWOD_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000100100100, vk, vj, vd))
#define VMULWOD_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000100100101, vk, vj, vd))
#define VMULWOD_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000100100110, vk, vj, vd))
#define VMULWOD_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000100100111, vk, vj, vd))
#define VMULWEV_H_BU(vd, vj, vk)     EMIT(type_3R(0b01110000100110000, vk, vj, vd))
#define VMULWEV_W_HU(vd, vj, vk)     EMIT(type_3R(0b01110000100110001, vk, vj, vd))
#define VMULWEV_D_WU(vd, vj, vk)     EMIT(type_3R(0b01110000100110010, vk, vj, vd))
#define VMULWEV_Q_DU(vd, vj, vk)     EMIT(type_3R(0b01110000100110011, vk, vj, vd))
#define VMULWOD_H_BU(vd, vj, vk)     EMIT(type_3R(0b01110000100110100, vk, vj, vd))
#define VMULWOD_W_HU(vd, vj, vk)     EMIT(type_3R(0b01110000100110101, vk, vj, vd))
#define VMULWOD_D_WU(vd, vj, vk)     EMIT(type_3R(0b01110000100110110, vk, vj, vd))
#define VMULWOD_Q_DU(vd, vj, vk)     EMIT(type_3R(0b01110000100110111, vk, vj, vd))
#define VMULWEV_H_BU_B(vd, vj, vk)   EMIT(type_3R(0b01110000101000000, vk, vj, vd))
#define VMULWEV_W_HU_H(vd, vj, vk)   EMIT(type_3R(0b01110000101000001, vk, vj, vd))
#define VMULWEV_D_WU_W(vd, vj, vk)   EMIT(type_3R(0b01110000101000010, vk, vj, vd))
#define VMULWEV_Q_DU_D(vd, vj, vk)   EMIT(type_3R(0b01110000101000011, vk, vj, vd))
#define VMULWOD_H_BU_B(vd, vj, vk)   EMIT(type_3R(0b01110000101000100, vk, vj, vd))
#define VMULWOD_W_HU_H(vd, vj, vk)   EMIT(type_3R(0b01110000101000101, vk, vj, vd))
#define VMULWOD_D_WU_W(vd, vj, vk)   EMIT(type_3R(0b01110000101000110, vk, vj, vd))
#define VMULWOD_Q_DU_D(vd, vj, vk)   EMIT(type_3R(0b01110000101000111, vk, vj, vd))
#define VMADD_B(vd, vj, vk)          EMIT(type_3R(0b01110000101010000, vk, vj, vd))
#define VMADD_H(vd, vj, vk)          EMIT(type_3R(0b01110000101010001, vk, vj, vd))
#define VMADD_W(vd, vj, vk)          EMIT(type_3R(0b01110000101010010, vk, vj, vd))
#define VMADD_D(vd, vj, vk)          EMIT(type_3R(0b01110000101010011, vk, vj, vd))
#define VMSUB_B(vd, vj, vk)          EMIT(type_3R(0b01110000101010100, vk, vj, vd))
#define VMSUB_H(vd, vj, vk)          EMIT(type_3R(0b01110000101010101, vk, vj, vd))
#define VMSUB_W(vd, vj, vk)          EMIT(type_3R(0b01110000101010110, vk, vj, vd))
#define VMSUB_D(vd, vj, vk)          EMIT(type_3R(0b01110000101010111, vk, vj, vd))
#define VMADDWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000101011000, vk, vj, vd))
#define VMADDWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000101011001, vk, vj, vd))
#define VMADDWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000101011010, vk, vj, vd))
#define VMADDWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000101011011, vk, vj, vd))
#define VMADDWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000101011100, vk, vj, vd))
#define VMADDWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000101011101, vk, vj, vd))
#define VMADDWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000101011110, vk, vj, vd))
#define VMADDWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000101011111, vk, vj, vd))
#define VMADDWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000101101000, vk, vj, vd))
#define VMADDWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000101101001, vk, vj, vd))
#define VMADDWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000101101010, vk, vj, vd))
#define VMADDWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000101101011, vk, vj, vd))
#define VMADDWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000101101100, vk, vj, vd))
#define VMADDWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000101101101, vk, vj, vd))
#define VMADDWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000101101110, vk, vj, vd))
#define VMADDWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000101101111, vk, vj, vd))
#define VMADDWEV_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110000101111000, vk, vj, vd))
#define VMADDWEV_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110000101111001, vk, vj, vd))
#define VMADDWEV_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110000101111010, vk, vj, vd))
#define VMADDWEV_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110000101111011, vk, vj, vd))
#define VMADDWOD_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110000101111100, vk, vj, vd))
#define VMADDWOD_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110000101111101, vk, vj, vd))
#define VMADDWOD_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110000101111110, vk, vj, vd))
#define VMADDWOD_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110000101111111, vk, vj, vd))
#define VDIV_B(vd, vj, vk)           EMIT(type_3R(0b01110000111000000, vk, vj, vd))
#define VDIV_H(vd, vj, vk)           EMIT(type_3R(0b01110000111000001, vk, vj, vd))
#define VDIV_W(vd, vj, vk)           EMIT(type_3R(0b01110000111000010, vk, vj, vd))
#define VDIV_D(vd, vj, vk)           EMIT(type_3R(0b01110000111000011, vk, vj, vd))
#define VDIV_BU(vd, vj, vk)          EMIT(type_3R(0b01110000111001000, vk, vj, vd))
#define VDIV_HU(vd, vj, vk)          EMIT(type_3R(0b01110000111001001, vk, vj, vd))
#define VDIV_WU(vd, vj, vk)          EMIT(type_3R(0b01110000111001010, vk, vj, vd))
#define VDIV_DU(vd, vj, vk)          EMIT(type_3R(0b01110000111001011, vk, vj, vd))
#define VMOD_B(vd, vj, vk)           EMIT(type_3R(0b01110000111000100, vk, vj, vd))
#define VMOD_H(vd, vj, vk)           EMIT(type_3R(0b01110000111000101, vk, vj, vd))
#define VMOD_W(vd, vj, vk)           EMIT(type_3R(0b01110000111000110, vk, vj, vd))
#define VMOD_D(vd, vj, vk)           EMIT(type_3R(0b01110000111000111, vk, vj, vd))
#define VMOD_BU(vd, vj, vk)          EMIT(type_3R(0b01110000111001100, vk, vj, vd))
#define VMOD_HU(vd, vj, vk)          EMIT(type_3R(0b01110000111001101, vk, vj, vd))
#define VMOD_WU(vd, vj, vk)          EMIT(type_3R(0b01110000111001110, vk, vj, vd))
#define VMOD_DU(vd, vj, vk)          EMIT(type_3R(0b01110000111001111, vk, vj, vd))
#define VSIGNCOV_B(vd, vj, vk)       EMIT(type_3R(0b01110001001011100, vk, vj, vd))
#define VSIGNCOV_H(vd, vj, vk)       EMIT(type_3R(0b01110001001011101, vk, vj, vd))
#define VSIGNCOV_W(vd, vj, vk)       EMIT(type_3R(0b01110001001011110, vk, vj, vd))
#define VSIGNCOV_D(vd, vj, vk)       EMIT(type_3R(0b01110001001011111, vk, vj, vd))
#define VMSKLTZ_B(vd, vj)            EMIT(type_2R(0b0111001010011100010000, vj, vd))
#define VMSKLTZ_H(vd, vj)            EMIT(type_2R(0b0111001010011100010001, vj, vd))
#define VMSKLTZ_W(vd, vj)            EMIT(type_2R(0b0111001010011100010010, vj, vd))
#define VMSKLTZ_D(vd, vj)            EMIT(type_2R(0b0111001010011100010011, vj, vd))
#define VMSKGEZ_B(vd, vj)            EMIT(type_2R(0b0111001010011100010100, vj, vd))
#define VMSKNZ_B(vd, vj)             EMIT(type_2R(0b0111001010011100011000, vj, vd))
#define VAND_V(vd, vj, vk)           EMIT(type_3R(0b01110001001001100, vk, vj, vd))
#define VLDI(vd, imm13)              EMIT(type_1RI13(0b01110011111000, imm13, vd))
#define VOR_V(vd, vj, vk)            EMIT(type_3R(0b01110001001001101, vk, vj, vd))
#define VXOR_V(vd, vj, vk)           EMIT(type_3R(0b01110001001001110, vk, vj, vd))
#define VNOR_V(vd, vj, vk)           EMIT(type_3R(0b01110001001001111, vk, vj, vd))
#define VANDN_V(vd, vj, vk)          EMIT(type_3R(0b01110001001010000, vk, vj, vd))
#define VORN_V(vd, vj, vk)           EMIT(type_3R(0b01110001001010001, vk, vj, vd))
#define VSLL_B(vd, vj, vk)           EMIT(type_3R(0b01110000111010000, vk, vj, vd))
#define VSLL_H(vd, vj, vk)           EMIT(type_3R(0b01110000111010001, vk, vj, vd))
#define VSLL_W(vd, vj, vk)           EMIT(type_3R(0b01110000111010010, vk, vj, vd))
#define VSLL_D(vd, vj, vk)           EMIT(type_3R(0b01110000111010011, vk, vj, vd))
#define VSRL_B(vd, vj, vk)           EMIT(type_3R(0b01110000111010100, vk, vj, vd))
#define VSRL_H(vd, vj, vk)           EMIT(type_3R(0b01110000111010101, vk, vj, vd))
#define VSRL_W(vd, vj, vk)           EMIT(type_3R(0b01110000111010110, vk, vj, vd))
#define VSRL_D(vd, vj, vk)           EMIT(type_3R(0b01110000111010111, vk, vj, vd))
#define VSRA_B(vd, vj, vk)           EMIT(type_3R(0b01110000111011000, vk, vj, vd))
#define VSRA_H(vd, vj, vk)           EMIT(type_3R(0b01110000111011001, vk, vj, vd))
#define VSRA_W(vd, vj, vk)           EMIT(type_3R(0b01110000111011010, vk, vj, vd))
#define VSRA_D(vd, vj, vk)           EMIT(type_3R(0b01110000111011011, vk, vj, vd))
#define VSLLI_B(vd, vj, imm3)        EMIT(type_2RI3(0b0111001100101100001, imm3, vj, vd))
#define VSLLI_H(vd, vj, imm4)        EMIT(type_2RI4(0b011100110010110001, imm4, vj, vd))
#define VSLLI_W(vd, vj, imm5)        EMIT(type_2RI5(0b01110011001011001, imm5, vj, vd))
#define VSLLI_D(vd, vj, imm6)        EMIT(type_2RI6(0b0111001100101101, imm6, vj, vd))
#define VSRLI_B(vd, vj, imm3)        EMIT(type_2RI3(0b0111001100110000001, imm3, vj, vd))
#define VSRLI_H(vd, vj, imm4)        EMIT(type_2RI4(0b011100110011000001, imm4, vj, vd))
#define VSRLI_W(vd, vj, imm5)        EMIT(type_2RI5(0b01110011001100001, imm5, vj, vd))
#define VSRLI_D(vd, vj, imm6)        EMIT(type_2RI6(0b0111001100110001, imm6, vj, vd))
#define VSRAI_B(vd, vj, imm3)        EMIT(type_2RI3(0b0111001100110100001, imm3, vj, vd))
#define VSRAI_H(vd, vj, imm4)        EMIT(type_2RI4(0b011100110011010001, imm4, vj, vd))
#define VSRAI_W(vd, vj, imm5)        EMIT(type_2RI5(0b01110011001101001, imm5, vj, vd))
#define VSRAI_D(vd, vj, imm6)        EMIT(type_2RI6(0b0111001100110101, imm6, vj, vd))
#define VROTR_B(vd, vj, vk)          EMIT(type_3R(0b01110000111011100, vk, vj, vd))
#define VROTR_H(vd, vj, vk)          EMIT(type_3R(0b01110000111011101, vk, vj, vd))
#define VROTR_W(vd, vj, vk)          EMIT(type_3R(0b01110000111011110, vk, vj, vd))
#define VROTR_D(vd, vj, vk)          EMIT(type_3R(0b01110000111011111, vk, vj, vd))
#define VSRLR_B(vd, vj, vk)          EMIT(type_3R(0b01110000111100000, vk, vj, vd))
#define VSRLR_H(vd, vj, vk)          EMIT(type_3R(0b01110000111100001, vk, vj, vd))
#define VSRLR_W(vd, vj, vk)          EMIT(type_3R(0b01110000111100010, vk, vj, vd))
#define VSRLR_D(vd, vj, vk)          EMIT(type_3R(0b01110000111100011, vk, vj, vd))
#define VSRAR_B(vd, vj, vk)          EMIT(type_3R(0b01110000111100100, vk, vj, vd))
#define VSRAR_H(vd, vj, vk)          EMIT(type_3R(0b01110000111100101, vk, vj, vd))
#define VSRAR_W(vd, vj, vk)          EMIT(type_3R(0b01110000111100110, vk, vj, vd))
#define VSRAR_D(vd, vj, vk)          EMIT(type_3R(0b01110000111100111, vk, vj, vd))
#define VSRLN_B_H(vd, vj, vk)        EMIT(type_3R(0b01110000111101001, vk, vj, vd))
#define VSRLN_H_W(vd, vj, vk)        EMIT(type_3R(0b01110000111101010, vk, vj, vd))
#define VSRLN_W_D(vd, vj, vk)        EMIT(type_3R(0b01110000111101011, vk, vj, vd))
#define VSRAN_B_H(vd, vj, vk)        EMIT(type_3R(0b01110000111101101, vk, vj, vd))
#define VSRAN_H_W(vd, vj, vk)        EMIT(type_3R(0b01110000111101110, vk, vj, vd))
#define VSRAN_W_D(vd, vj, vk)        EMIT(type_3R(0b01110000111101111, vk, vj, vd))
#define VSRLNI_B_H(vd, vj, imm4)     EMIT(type_2RI4(0b011100110100000001, imm4, vj, vd))
#define VSRLNI_H_W(vd, vj, imm5)     EMIT(type_2RI5(0b01110011010000001, imm5, vj, vd))
#define VSRLNI_W_D(vd, vj, imm6)     EMIT(type_2RI6(0b0111001101000001, imm6, vj, vd))
#define VSRLNI_D_Q(vd, vj, imm7)     EMIT(type_2RI7(0b011100110100001, imm7, vj, vd))
#define VSRANI_B_H(vd, vj, imm4)     EMIT(type_2RI4(0b011100110101100001, imm4, vj, vd))
#define VSRANI_H_W(vd, vj, imm5)     EMIT(type_2RI5(0b01110011010110001, imm5, vj, vd))
#define VSRANI_W_D(vd, vj, imm6)     EMIT(type_2RI6(0b0111001101011001, imm6, vj, vd))
#define VSRANI_D_Q(vd, vj, imm7)     EMIT(type_2RI7(0b011100110101101, imm7, vj, vd))
#define VSRLRN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110000111110001, vk, vj, vd))
#define VSRLRN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110000111110010, vk, vj, vd))
#define VSRLRN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110000111110011, vk, vj, vd))
#define VSRARN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110000111110101, vk, vj, vd))
#define VSRARN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110000111110110, vk, vj, vd))
#define VSRARN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110000111110111, vk, vj, vd))
#define VSSRLN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110000111111001, vk, vj, vd))
#define VSSRLN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110000111111010, vk, vj, vd))
#define VSSRLN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110000111111011, vk, vj, vd))
#define VSSRAN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110000111111101, vk, vj, vd))
#define VSSRAN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110000111111110, vk, vj, vd))
#define VSSRAN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110000111111111, vk, vj, vd))
#define VSSRLN_BU_H(vd, vj, vk)      EMIT(type_3R(0b01110001000001001, vk, vj, vd))
#define VSSRLN_HU_W(vd, vj, vk)      EMIT(type_3R(0b01110001000001010, vk, vj, vd))
#define VSSRLN_WU_D(vd, vj, vk)      EMIT(type_3R(0b01110001000001011, vk, vj, vd))
#define VSSRAN_BU_H(vd, vj, vk)      EMIT(type_3R(0b01110001000001101, vk, vj, vd))
#define VSSRAN_HU_W(vd, vj, vk)      EMIT(type_3R(0b01110001000001110, vk, vj, vd))
#define VSSRAN_WU_D(vd, vj, vk)      EMIT(type_3R(0b01110001000001111, vk, vj, vd))
#define VSSRLNI_B_H(vd, vj, imm4)    EMIT(type_2RI4(0b011100110100100001, imm4, vj, vd))
#define VSSRLNI_H_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110011010010001, imm5, vj, vd))
#define VSSRLNI_W_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111001101001001, imm6, vj, vd))
#define VSSRLNI_D_Q(vd, vj, imm7)    EMIT(type_2RI7(0b011100110100101, imm7, vj, vd))
#define VSSRANI_B_H(vd, vj, imm4)    EMIT(type_2RI4(0b011100110110000001, imm4, vj, vd))
#define VSSRANI_H_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110011011000001, imm5, vj, vd))
#define VSSRANI_W_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111001101100001, imm6, vj, vd))
#define VSSRANI_D_Q(vd, vj, imm7)    EMIT(type_2RI7(0b011100110110001, imm7, vj, vd))
#define VSSRLNI_BU_H(vd, vj, imm4)   EMIT(type_2RI4(0b011100110100110001, imm4, vj, vd))
#define VSSRLNI_HU_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110011010011001, imm5, vj, vd))
#define VSSRLNI_WU_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111001101001101, imm6, vj, vd))
#define VSSRLNI_DU_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011100110100111, imm7, vj, vd))
#define VSSRANI_BU_H(vd, vj, imm4)   EMIT(type_2RI4(0b011100110110010001, imm4, vj, vd))
#define VSSRANI_HU_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110011011001001, imm5, vj, vd))
#define VSSRANI_WU_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111001101100101, imm6, vj, vd))
#define VSSRANI_DU_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011100110110011, imm7, vj, vd))
#define VSSRLRN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110001000000001, vk, vj, vd))
#define VSSRLRN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110001000000010, vk, vj, vd))
#define VSSRLRN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110001000000011, vk, vj, vd))
#define VSSRARN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110001000000101, vk, vj, vd))
#define VSSRARN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110001000000110, vk, vj, vd))
#define VSSRARN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110001000000111, vk, vj, vd))
#define VSSRLRN_BU_H(vd, vj, vk)     EMIT(type_3R(0b01110001000010001, vk, vj, vd))
#define VSSRLRN_HU_W(vd, vj, vk)     EMIT(type_3R(0b01110001000010010, vk, vj, vd))
#define VSSRLRN_WU_D(vd, vj, vk)     EMIT(type_3R(0b01110001000010011, vk, vj, vd))
#define VSSRARN_BU_H(vd, vj, vk)     EMIT(type_3R(0b01110001000010101, vk, vj, vd))
#define VSSRARN_HU_W(vd, vj, vk)     EMIT(type_3R(0b01110001000010110, vk, vj, vd))
#define VSSRARN_WU_D(vd, vj, vk)     EMIT(type_3R(0b01110001000010111, vk, vj, vd))
#define VSSRLRNI_B_H(vd, vj, imm4)   EMIT(type_2RI4(0b011100110101000001, imm4, vj, vd))
#define VSSRLRNI_H_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110011010100001, imm5, vj, vd))
#define VSSRLRNI_W_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111001101010001, imm6, vj, vd))
#define VSSRLRNI_D_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011100110101001, imm7, vj, vd))
#define VSSRARNI_B_H(vd, vj, imm4)   EMIT(type_2RI4(0b011100110110100001, imm4, vj, vd))
#define VSSRARNI_H_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110011011010001, imm5, vj, vd))
#define VSSRARNI_W_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111001101101001, imm6, vj, vd))
#define VSSRARNI_D_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011100110110101, imm7, vj, vd))
#define VSSRLRNI_BU_H(vd, vj, imm4)  EMIT(type_2RI4(0b011100110101010001, imm4, vj, vd))
#define VSSRLRNI_HU_W(vd, vj, imm5)  EMIT(type_2RI5(0b01110011010101001, imm5, vj, vd))
#define VSSRLRNI_WU_D(vd, vj, imm6)  EMIT(type_2RI6(0b0111001101010101, imm6, vj, vd))
#define VSSRLRNI_DU_Q(vd, vj, imm7)  EMIT(type_2RI7(0b011100110101011, imm7, vj, vd))
#define VSSRARNI_BU_H(vd, vj, imm4)  EMIT(type_2RI4(0b011100110110110001, imm4, vj, vd))
#define VSSRARNI_HU_W(vd, vj, imm5)  EMIT(type_2RI5(0b01110011011011001, imm5, vj, vd))
#define VSSRARNI_WU_D(vd, vj, imm6)  EMIT(type_2RI6(0b0111001101101101, imm6, vj, vd))
#define VSSRARNI_DU_Q(vd, vj, imm7)  EMIT(type_2RI7(0b011100110110111, imm7, vj, vd))
#define VBITCLR_B(vd, vj, vk)        EMIT(type_3R(0b01110001000011000, vk, vj, vd))
#define VBITCLR_H(vd, vj, vk)        EMIT(type_3R(0b01110001000011001, vk, vj, vd))
#define VBITCLR_W(vd, vj, vk)        EMIT(type_3R(0b01110001000011010, vk, vj, vd))
#define VBITCLR_D(vd, vj, vk)        EMIT(type_3R(0b01110001000011011, vk, vj, vd))
#define VBITCLRI_B(vd, vj, imm3)     EMIT(type_2RI3(0b0111001100010000001, imm3, vj, vd))
#define VBITCLRI_H(vd, vj, imm4)     EMIT(type_2RI4(0b011100110001000001, imm4, vj, vd))
#define VBITCLRI_W(vd, vj, imm5)     EMIT(type_2RI5(0b01110011000100001, imm5, vj, vd))
#define VBITCLRI_D(vd, vj, imm6)     EMIT(type_2RI6(0b0111001100010001, imm6, vj, vd))
#define VBITSET_B(vd, vj, vk)        EMIT(type_3R(0b01110001000011100, vk, vj, vd))
#define VBITSET_H(vd, vj, vk)        EMIT(type_3R(0b01110001000011101, vk, vj, vd))
#define VBITSET_W(vd, vj, vk)        EMIT(type_3R(0b01110001000011110, vk, vj, vd))
#define VBITSET_D(vd, vj, vk)        EMIT(type_3R(0b01110001000011111, vk, vj, vd))
#define VBITSEL_V(vd, vj, vk, va)    EMIT(type_4R(0b000011010001, va, vk, vj, vd))
#define VBITREV_B(vd, vj, vk)        EMIT(type_3R(0b01110001000100000, vk, vj, vd))
#define VBITREV_H(vd, vj, vk)        EMIT(type_3R(0b01110001000100001, vk, vj, vd))
#define VBITREV_W(vd, vj, vk)        EMIT(type_3R(0b01110001000100010, vk, vj, vd))
#define VBITREV_D(vd, vj, vk)        EMIT(type_3R(0b01110001000100011, vk, vj, vd))
#define VFRSTP_B(vd, vj, vk)         EMIT(type_3R(0b01110001001010110, vk, vj, vd))
#define VFRSTP_H(vd, vj, vk)         EMIT(type_3R(0b01110001001010111, vk, vj, vd))
#define VFRSTPI_B(vd, vj, imm5)      EMIT(type_2RI5(0b01110010100110100, imm5, vj, vd))
#define VFRSTPI_H(vd, vj, imm5)      EMIT(type_2RI5(0b01110010100110101, imm5, vj, vd))
#define VFADD_S(vd, vj, vk)          EMIT(type_3R(0b01110001001100001, vk, vj, vd))
#define VFADD_D(vd, vj, vk)          EMIT(type_3R(0b01110001001100010, vk, vj, vd))
#define VFSUB_S(vd, vj, vk)          EMIT(type_3R(0b01110001001100101, vk, vj, vd))
#define VFSUB_D(vd, vj, vk)          EMIT(type_3R(0b01110001001100110, vk, vj, vd))
#define VFMUL_S(vd, vj, vk)          EMIT(type_3R(0b01110001001110001, vk, vj, vd))
#define VFMUL_D(vd, vj, vk)          EMIT(type_3R(0b01110001001110010, vk, vj, vd))
#define VFDIV_S(vd, vj, vk)          EMIT(type_3R(0b01110001001110101, vk, vj, vd))
#define VFDIV_D(vd, vj, vk)          EMIT(type_3R(0b01110001001110110, vk, vj, vd))
#define VFMAX_S(vd, vj, vk)          EMIT(type_3R(0b01110001001111001, vk, vj, vd))
#define VFMAX_D(vd, vj, vk)          EMIT(type_3R(0b01110001001111010, vk, vj, vd))
#define VFMIN_S(vd, vj, vk)          EMIT(type_3R(0b01110001001111101, vk, vj, vd))
#define VFMIN_D(vd, vj, vk)          EMIT(type_3R(0b01110001001111110, vk, vj, vd))
#define VFMAXA_S(vd, vj, vk)         EMIT(type_3R(0b01110001010000001, vk, vj, vd))
#define VFMAXA_D(vd, vj, vk)         EMIT(type_3R(0b01110001010000010, vk, vj, vd))
#define VFMINA_S(vd, vj, vk)         EMIT(type_3R(0b01110001010000101, vk, vj, vd))
#define VFMINA_D(vd, vj, vk)         EMIT(type_3R(0b01110001010000110, vk, vj, vd))
#define VFSQRT_S(vd, vj)             EMIT(type_2R(0b0111001010011100111001, vj, vd))
#define VFSQRT_D(vd, vj)             EMIT(type_2R(0b0111001010011100111010, vj, vd))
#define VFRECIP_S(vd, vj)            EMIT(type_2R(0b0111001010011100111101, vj, vd))
#define VFRECIP_D(vd, vj)            EMIT(type_2R(0b0111001010011100111110, vj, vd))
#define VFRECIPE_S(vd, vj)           EMIT(type_2R(0b0111001010011101000101, vj, vd))
#define VFRECIPE_D(vd, vj)           EMIT(type_2R(0b0111001010011101000110, vj, vd))
#define VFRSQRT_S(vd, vj)            EMIT(type_2R(0b0111001010011101000001, vj, vd))
#define VFRSQRT_D(vd, vj)            EMIT(type_2R(0b0111001010011101000010, vj, vd))
#define VFRSQRTE_S(vd, vj)           EMIT(type_2R(0b0111001010011101001001, vj, vd))
#define VFRSQRTE_D(vd, vj)           EMIT(type_2R(0b0111001010011101001010, vj, vd))
#define VFCVTL_S_H(vd, vj)           EMIT(type_2R(0b0111001010011101111010, vj, vd))
#define VFCVTH_S_H(vd, vj)           EMIT(type_2R(0b0111001010011101111011, vj, vd))
#define VFCVTL_D_S(vd, vj)           EMIT(type_2R(0b0111001010011101111100, vj, vd))
#define VFCVTH_D_S(vd, vj)           EMIT(type_2R(0b0111001010011101111101, vj, vd))
#define VFCVT_H_S(vd, vj, vk)        EMIT(type_3R(0b01110001010001100, vk, vj, vd))
#define VFCVT_S_D(vd, vj, vk)        EMIT(type_3R(0b01110001010001101, vk, vj, vd))
#define VFTINT_W_S(vd, vj)           EMIT(type_2R(0b0111001010011110001100, vj, vd))
#define VFTINT_L_D(vd, vj)           EMIT(type_2R(0b0111001010011110001101, vj, vd))
#define VFTINTRM_W_S(vd, vj)         EMIT(type_2R(0b0111001010011110001110, vj, vd))
#define VFTINTRM_L_D(vd, vj)         EMIT(type_2R(0b0111001010011110001111, vj, vd))
#define VFTINTRP_W_S(vd, vj)         EMIT(type_2R(0b0111001010011110010000, vj, vd))
#define VFTINTRP_L_D(vd, vj)         EMIT(type_2R(0b0111001010011110010001, vj, vd))
#define VFTINTRZ_W_S(vd, vj)         EMIT(type_2R(0b0111001010011110010010, vj, vd))
#define VFTINTRZ_L_D(vd, vj)         EMIT(type_2R(0b0111001010011110010011, vj, vd))
#define VFTINTRNE_W_S(vd, vj)        EMIT(type_2R(0b0111001010011110010100, vj, vd))
#define VFTINTRNE_L_D(vd, vj)        EMIT(type_2R(0b0111001010011110010101, vj, vd))
#define VFTINT_WU_S(vd, vj)          EMIT(type_2R(0b0111001010011110010110, vj, vd))
#define VFTINT_LU_D(vd, vj)          EMIT(type_2R(0b0111001010011110010111, vj, vd))
#define VFTINTRZ_WU_S(vd, vj)        EMIT(type_2R(0b0111001010011110011100, vj, vd))
#define VFTINTRZ_LU_D(vd, vj)        EMIT(type_2R(0b0111001010011110011101, vj, vd))
#define VFTINT_W_D(vd, vj, vk)       EMIT(type_3R(0b01110001010010011, vk, vj, vd))
#define VFTINTRM_W_D(vd, vj, vk)     EMIT(type_3R(0b01110001010010100, vk, vj, vd))
#define VFTINTRP_W_D(vd, vj, vk)     EMIT(type_3R(0b01110001010010101, vk, vj, vd))
#define VFTINTRZ_W_D(vd, vj, vk)     EMIT(type_3R(0b01110001010010110, vk, vj, vd))
#define VFTINTRNE_W_D(vd, vj, vk)    EMIT(type_3R(0b01110001010010111, vk, vj, vd))
#define VFTINTL_L_S(vd, vj)          EMIT(type_2R(0b0111001010011110100000, vj, vd))
#define VFTINTH_L_S(vd, vj)          EMIT(type_2R(0b0111001010011110100001, vj, vd))
#define VFTINTRML_L_S(vd, vj)        EMIT(type_2R(0b0111001010011110100010, vj, vd))
#define VFTINTRMH_L_S(vd, vj)        EMIT(type_2R(0b0111001010011110100011, vj, vd))
#define VFTINTRPL_L_S(vd, vj)        EMIT(type_2R(0b0111001010011110100100, vj, vd))
#define VFTINTRPH_L_S(vd, vj)        EMIT(type_2R(0b0111001010011110100101, vj, vd))
#define VFTINTRZL_L_S(vd, vj)        EMIT(type_2R(0b0111001010011110100110, vj, vd))
#define VFTINTRZH_L_S(vd, vj)        EMIT(type_2R(0b0111001010011110100111, vj, vd))
#define VFTINTRNEL_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110101000, vj, vd))
#define VFTINTRNEH_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110101001, vj, vd))
#define VFFINT_S_W(vd, vj)           EMIT(type_2R(0b0111001010011110000000, vj, vd))
#define VFFINT_S_WU(vd, vj)          EMIT(type_2R(0b0111001010011110000001, vj, vd))
#define VFFINT_D_L(vd, vj)           EMIT(type_2R(0b0111001010011110000010, vj, vd))
#define VFFINT_D_LU(vd, vj)          EMIT(type_2R(0b0111001010011110000011, vj, vd))
#define VFFINTL_D_W(vd, vj)          EMIT(type_2R(0b0111001010011110000100, vj, vd))
#define VFFINTH_D_W(vd, vj)          EMIT(type_2R(0b0111001010011110000101, vj, vd))
#define VFFINT_S_L(vd, vj, vk)       EMIT(type_3R(0b01110001010010000, vk, vj, vd))
#define VSEQ_B(vd, vj, vk)           EMIT(type_3R(0b01110000000000000, vk, vj, vd))
#define VSEQ_H(vd, vj, vk)           EMIT(type_3R(0b01110000000000001, vk, vj, vd))
#define VSEQ_W(vd, vj, vk)           EMIT(type_3R(0b01110000000000010, vk, vj, vd))
#define VSEQ_D(vd, vj, vk)           EMIT(type_3R(0b01110000000000011, vk, vj, vd))
#define VSLE_B(vd, vj, vk)           EMIT(type_3R(0b01110000000000100, vk, vj, vd))
#define VSLE_H(vd, vj, vk)           EMIT(type_3R(0b01110000000000101, vk, vj, vd))
#define VSLE_W(vd, vj, vk)           EMIT(type_3R(0b01110000000000110, vk, vj, vd))
#define VSLE_D(vd, vj, vk)           EMIT(type_3R(0b01110000000000111, vk, vj, vd))
#define VSLE_BU(vd, vj, vk)          EMIT(type_3R(0b01110000000001000, vk, vj, vd))
#define VSLE_HU(vd, vj, vk)          EMIT(type_3R(0b01110000000001001, vk, vj, vd))
#define VSLE_WU(vd, vj, vk)          EMIT(type_3R(0b01110000000001010, vk, vj, vd))
#define VSLE_DU(vd, vj, vk)          EMIT(type_3R(0b01110000000001011, vk, vj, vd))
#define VSLEI_BU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100001000, imm5, vj, vd))
#define VSLEI_HU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100001001, imm5, vj, vd))
#define VSLEI_WU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100001010, imm5, vj, vd))
#define VSLEI_DU(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100001011, imm5, vj, vd))
#define VSLT_B(vd, vj, vk)           EMIT(type_3R(0b01110000000001100, vk, vj, vd))
#define VSLT_H(vd, vj, vk)           EMIT(type_3R(0b01110000000001101, vk, vj, vd))
#define VSLT_W(vd, vj, vk)           EMIT(type_3R(0b01110000000001110, vk, vj, vd))
#define VSLTI_B(vd, vj, imm5)        EMIT(type_2RI5(0b01110010100001100, imm5, vj, vd))
#define VSLTI_H(vd, vj, imm5)        EMIT(type_2RI5(0b01110010100001101, imm5, vj, vd))
#define VSLTI_W(vd, vj, imm5)        EMIT(type_2RI5(0b01110010100001110, imm5, vj, vd))
#define VSLTI_D(vd, vj, imm5)        EMIT(type_2RI5(0b01110010100001111, imm5, vj, vd))
#define VSLT_D(vd, vj, vk)           EMIT(type_3R(0b01110000000001111, vk, vj, vd))
#define VSLT_BU(vd, vj, vk)          EMIT(type_3R(0b01110000000010000, vk, vj, vd))
#define VSLT_HU(vd, vj, vk)          EMIT(type_3R(0b01110000000010001, vk, vj, vd))
#define VSLT_WU(vd, vj, vk)          EMIT(type_3R(0b01110000000010010, vk, vj, vd))
#define VSLT_DU(vd, vj, vk)          EMIT(type_3R(0b01110000000010011, vk, vj, vd))
#define VREPLVE_B(vd, vj, rk)        EMIT(type_3R(0b01110001001000100, rk, vj, vd))
#define VREPLVE_H(vd, vj, rk)        EMIT(type_3R(0b01110001001000101, rk, vj, vd))
#define VREPLVE_W(vd, vj, rk)        EMIT(type_3R(0b01110001001000110, rk, vj, vd))
#define VREPLVE_D(vd, vj, rk)        EMIT(type_3R(0b01110001001000111, rk, vj, vd))
#define VREPLVEI_B(vd, vk, imm4)     EMIT(type_2RI4(0b011100101111011110, imm4, vk, vd))
#define VREPLVEI_H(vd, vk, imm3)     EMIT(type_2RI3(0b0111001011110111110, imm3, vk, vd))
#define VREPLVEI_W(vd, vk, imm2)     EMIT(type_2RI2(0b01110010111101111110, imm2, vk, vd))
#define VREPLVEI_D(vd, vk, imm1)     EMIT(type_2RI1(0b011100101111011111110, imm1, vk, vd))
#define VBSLL_V(vd, vj, imm5)        EMIT(type_2RI5(0b01110010100011100, imm5, vj, vd))
#define VBSRL_V(vd, vj, imm5)        EMIT(type_2RI5(0b01110010100011101, imm5, vj, vd))
#define VPACKEV_B(vd, vj, vk)        EMIT(type_3R(0b01110001000101100, vk, vj, vd))
#define VPACKEV_H(vd, vj, vk)        EMIT(type_3R(0b01110001000101101, vk, vj, vd))
#define VPACKEV_W(vd, vj, vk)        EMIT(type_3R(0b01110001000101110, vk, vj, vd))
#define VPACKEV_D(vd, vj, vk)        EMIT(type_3R(0b01110001000101111, vk, vj, vd))
#define VPACKOD_B(vd, vj, vk)        EMIT(type_3R(0b01110001000110000, vk, vj, vd))
#define VPACKOD_H(vd, vj, vk)        EMIT(type_3R(0b01110001000110001, vk, vj, vd))
#define VPACKOD_W(vd, vj, vk)        EMIT(type_3R(0b01110001000110010, vk, vj, vd))
#define VPACKOD_D(vd, vj, vk)        EMIT(type_3R(0b01110001000110011, vk, vj, vd))
#define VPICKEV_B(vd, vj, vk)        EMIT(type_3R(0b01110001000111100, vk, vj, vd))
#define VPICKEV_H(vd, vj, vk)        EMIT(type_3R(0b01110001000111101, vk, vj, vd))
#define VPICKEV_W(vd, vj, vk)        EMIT(type_3R(0b01110001000111110, vk, vj, vd))
#define VPICKEV_D(vd, vj, vk)        EMIT(type_3R(0b01110001000111111, vk, vj, vd))
#define VPICKOD_B(vd, vj, vk)        EMIT(type_3R(0b01110001001000000, vk, vj, vd))
#define VPICKOD_H(vd, vj, vk)        EMIT(type_3R(0b01110001001000001, vk, vj, vd))
#define VPICKOD_W(vd, vj, vk)        EMIT(type_3R(0b01110001001000010, vk, vj, vd))
#define VPICKOD_D(vd, vj, vk)        EMIT(type_3R(0b01110001001000011, vk, vj, vd))
#define VILVL_B(vd, vj, vk)          EMIT(type_3R(0b01110001000110100, vk, vj, vd))
#define VILVL_H(vd, vj, vk)          EMIT(type_3R(0b01110001000110101, vk, vj, vd))
#define VILVL_W(vd, vj, vk)          EMIT(type_3R(0b01110001000110110, vk, vj, vd))
#define VILVL_D(vd, vj, vk)          EMIT(type_3R(0b01110001000110111, vk, vj, vd))
#define VILVH_B(vd, vj, vk)          EMIT(type_3R(0b01110001000111000, vk, vj, vd))
#define VILVH_H(vd, vj, vk)          EMIT(type_3R(0b01110001000111001, vk, vj, vd))
#define VILVH_W(vd, vj, vk)          EMIT(type_3R(0b01110001000111010, vk, vj, vd))
#define VILVH_D(vd, vj, vk)          EMIT(type_3R(0b01110001000111011, vk, vj, vd))
#define VSHUF_B(vd, vj, vk, va)      EMIT(type_4R(0b000011010101, va, vk, vj, vd))
#define VSHUF_H(vd, vj, vk)          EMIT(type_3R(0b01110001011110101, vk, vj, vd))
#define VSHUF_W(vd, vj, vk)          EMIT(type_3R(0b01110001011110110, vk, vj, vd))
#define VSHUF_D(vd, vj, vk)          EMIT(type_3R(0b01110001011110111, vk, vj, vd))
#define VSHUF4I_B(vd, vj, imm8)      EMIT(type_2RI8(0b01110011100100, imm8, vj, vd))
#define VSHUF4I_H(vd, vj, imm8)      EMIT(type_2RI8(0b01110011100101, imm8, vj, vd))
#define VSHUF4I_W(vd, vj, imm8)      EMIT(type_2RI8(0b01110011100110, imm8, vj, vd))
#define VSHUF4I_D(vd, vj, imm8)      EMIT(type_2RI8(0b01110011100111, imm8, vj, vd))
#define VEXTRINS_D(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100000, imm8, vj, vd))
#define VEXTRINS_W(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100001, imm8, vj, vd))
#define VEXTRINS_H(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100010, imm8, vj, vd))
#define VEXTRINS_B(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100011, imm8, vj, vd))
#define VLD(vd, rj, imm12)           EMIT(type_2RI12(0b0010110000, imm12, rj, vd))
#define VST(vd, rj, imm12)           EMIT(type_2RI12(0b0010110001, imm12, rj, vd))
#define VLDX(vd, rj, rk)             EMIT(type_3R(0b00111000010000000, rk, rj, vd))
#define VSTX(vd, rj, rk)             EMIT(type_3R(0b00111000010001000, rk, rj, vd))
#define VSTELM_D(vd, rj, imm8, imm1) EMIT(type_2RI9(0b0011000100010, (((imm1) << 8) | (imm8)), rj, vd))
#define VSTELM_W(vd, rj, imm8, imm2) EMIT(type_2RI10(0b001100010010, (((imm2) << 8) | (imm8)), rj, vd))
#define VSTELM_H(vd, rj, imm8, imm3) EMIT(type_2RI11(0b00110001010, (((imm3) << 8) | (imm8)), rj, vd))
#define VSTELM_B(vd, rj, imm8, imm4) EMIT(type_2RI12(0b0011000110, (((imm4) << 8) | (imm8)), rj, vd))
#define VLDREPL_D(vd, rj, imm9)      EMIT(type_2RI9(0b0011000000010, imm9, rj, vd))
#define VLDREPL_W(vd, rj, imm10)     EMIT(type_2RI10(0b001100000010, imm10, rj, vd))
#define VLDREPL_H(vd, rj, imm11)     EMIT(type_2RI11(0b00110000010, imm11, rj, vd))
#define VLDREPL_B(vd, rj, imm12)     EMIT(type_2RI12(0b0011000010, imm12, rj, vd))
#define VFCMP_S(vd, vj, vk, cond)    EMIT(type_4R(0b000011000101, cond, vk, vj, vd))
#define VFCMP_D(vd, vj, vk, cond)    EMIT(type_4R(0b000011000110, cond, vk, vj, vd))
#define XVFCMP_S(vd, vj, vk, cond)   EMIT(type_4R(0b000011001001, cond, vk, vj, vd))
#define XVFCMP_D(vd, vj, vk, cond)   EMIT(type_4R(0b000011001010, cond, vk, vj, vd))
#define VPERMI_W(vd, vj, imm8)       EMIT(type_2RI8(0b01110011111001, imm8, vj, vd))
#define VFMADD_S(vd, vj, vk, va)     EMIT(type_4R(0b000010010001, va, vk, vj, vd))
#define VFMSUB_S(vd, vj, vk, va)     EMIT(type_4R(0b000010010101, va, vk, vj, vd))
#define VFNMADD_S(vd, vj, vk, va)    EMIT(type_4R(0b000010011001, va, vk, vj, vd))
#define VFNMSUB_S(vd, vj, vk, va)    EMIT(type_4R(0b000010011101, va, vk, vj, vd))
#define VFMADD_D(vd, vj, vk, va)     EMIT(type_4R(0b000010010010, va, vk, vj, vd))
#define VFMSUB_D(vd, vj, vk, va)     EMIT(type_4R(0b000010010110, va, vk, vj, vd))
#define VFNMADD_D(vd, vj, vk, va)    EMIT(type_4R(0b000010011010, va, vk, vj, vd))
#define VFNMSUB_D(vd, vj, vk, va)    EMIT(type_4R(0b000010011110, va, vk, vj, vd))
#define VANDI_B(vd, vj, imm8)        EMIT(type_2RI8(0b01110011110100, imm8, vj, vd))
#define VORI_B(vd, vj, imm8)         EMIT(type_2RI8(0b01110011110101, imm8, vj, vd))
#define VXORI_B(vd, vj, imm8)        EMIT(type_2RI8(0b01110011110110, imm8, vj, vd))
#define VNORI_B(vd, vj, imm8)        EMIT(type_2RI8(0b01110011110111, imm8, vj, vd))

#define XVADD_B(vd, vj, vk)          EMIT(type_3R(0b01110100000010100, vk, vj, vd))
#define XVADD_H(vd, vj, vk)          EMIT(type_3R(0b01110100000010101, vk, vj, vd))
#define XVADD_W(vd, vj, vk)          EMIT(type_3R(0b01110100000010110, vk, vj, vd))
#define XVADD_D(vd, vj, vk)          EMIT(type_3R(0b01110100000010111, vk, vj, vd))
#define XVADD_Q(vd, vj, vk)          EMIT(type_3R(0b01110101001011010, vk, vj, vd))
#define XVSUB_B(vd, vj, vk)          EMIT(type_3R(0b01110100000011000, vk, vj, vd))
#define XVSUB_H(vd, vj, vk)          EMIT(type_3R(0b01110100000011001, vk, vj, vd))
#define XVSUB_W(vd, vj, vk)          EMIT(type_3R(0b01110100000011010, vk, vj, vd))
#define XVSUB_D(vd, vj, vk)          EMIT(type_3R(0b01110100000011011, vk, vj, vd))
#define XVSUB_Q(vd, vj, vk)          EMIT(type_3R(0b01110101001011011, vk, vj, vd))
#define XVSADD_B(vd, vj, vk)         EMIT(type_3R(0b01110100010001100, vk, vj, vd))
#define XVSADD_H(vd, vj, vk)         EMIT(type_3R(0b01110100010001101, vk, vj, vd))
#define XVSADD_W(vd, vj, vk)         EMIT(type_3R(0b01110100010001110, vk, vj, vd))
#define XVSADD_D(vd, vj, vk)         EMIT(type_3R(0b01110100010001111, vk, vj, vd))
#define XVSADD_BU(vd, vj, vk)        EMIT(type_3R(0b01110100010010100, vk, vj, vd))
#define XVSADD_HU(vd, vj, vk)        EMIT(type_3R(0b01110100010010101, vk, vj, vd))
#define XVSADD_WU(vd, vj, vk)        EMIT(type_3R(0b01110100010010110, vk, vj, vd))
#define XVSADD_DU(vd, vj, vk)        EMIT(type_3R(0b01110100010010111, vk, vj, vd))
#define XVSSUB_B(vd, vj, vk)         EMIT(type_3R(0b01110100010010000, vk, vj, vd))
#define XVSSUB_H(vd, vj, vk)         EMIT(type_3R(0b01110100010010001, vk, vj, vd))
#define XVSSUB_W(vd, vj, vk)         EMIT(type_3R(0b01110100010010010, vk, vj, vd))
#define XVSSUB_D(vd, vj, vk)         EMIT(type_3R(0b01110100010010011, vk, vj, vd))
#define XVSSUB_BU(vd, vj, vk)        EMIT(type_3R(0b01110100010011000, vk, vj, vd))
#define XVSSUB_HU(vd, vj, vk)        EMIT(type_3R(0b01110100010011001, vk, vj, vd))
#define XVSSUB_WU(vd, vj, vk)        EMIT(type_3R(0b01110100010011010, vk, vj, vd))
#define XVSSUB_DU(vd, vj, vk)        EMIT(type_3R(0b01110100010011011, vk, vj, vd))
#define XVHADDW_H_B(vd, vj, vk)      EMIT(type_3R(0b01110100010101000, vk, vj, vd))
#define XVHADDW_W_H(vd, vj, vk)      EMIT(type_3R(0b01110100010101001, vk, vj, vd))
#define XVHADDW_D_W(vd, vj, vk)      EMIT(type_3R(0b01110100010101010, vk, vj, vd))
#define XVHADDW_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110100010101011, vk, vj, vd))
#define XVHADDW_HU_BU(vd, vj, vk)    EMIT(type_3R(0b01110100010110000, vk, vj, vd))
#define XVHADDW_WU_HU(vd, vj, vk)    EMIT(type_3R(0b01110100010110001, vk, vj, vd))
#define XVHADDW_DU_WU(vd, vj, vk)    EMIT(type_3R(0b01110100010110010, vk, vj, vd))
#define XVHADDW_QU_DU(vd, vj, vk)    EMIT(type_3R(0b01110100010110011, vk, vj, vd))
#define XVHSUBW_H_B(vd, vj, vk)      EMIT(type_3R(0b01110100010101100, vk, vj, vd))
#define XVHSUBW_W_H(vd, vj, vk)      EMIT(type_3R(0b01110100010101101, vk, vj, vd))
#define XVHSUBW_D_W(vd, vj, vk)      EMIT(type_3R(0b01110100010101110, vk, vj, vd))
#define XVHSUBW_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110100010101111, vk, vj, vd))
#define XVHSUBW_HU_BU(vd, vj, vk)    EMIT(type_3R(0b01110100010110100, vk, vj, vd))
#define XVHSUBW_WU_HU(vd, vj, vk)    EMIT(type_3R(0b01110100010110101, vk, vj, vd))
#define XVHSUBW_DU_WU(vd, vj, vk)    EMIT(type_3R(0b01110100010110110, vk, vj, vd))
#define XVHSUBW_QU_DU(vd, vj, vk)    EMIT(type_3R(0b01110100010110111, vk, vj, vd))
#define XVADDWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110100000111100, vk, vj, vd))
#define XVADDWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110100000111101, vk, vj, vd))
#define XVADDWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110100000111110, vk, vj, vd))
#define XVADDWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110100000111111, vk, vj, vd))
#define XVADDWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110100001000100, vk, vj, vd))
#define XVADDWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110100001000101, vk, vj, vd))
#define XVADDWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110100001000110, vk, vj, vd))
#define XVADDWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110100001000111, vk, vj, vd))
#define XVSUBWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110100001000000, vk, vj, vd))
#define XVSUBWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110100001000001, vk, vj, vd))
#define XVSUBWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110100001000010, vk, vj, vd))
#define XVSUBWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110100001000011, vk, vj, vd))
#define XVSUBWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110100001001000, vk, vj, vd))
#define XVSUBWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110100001001001, vk, vj, vd))
#define XVSUBWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110100001001010, vk, vj, vd))
#define XVSUBWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110100001001011, vk, vj, vd))
#define XVADDWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110100001011100, vk, vj, vd))
#define XVADDWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110100001011101, vk, vj, vd))
#define XVADDWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110100001011110, vk, vj, vd))
#define XVADDWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110100001011111, vk, vj, vd))
#define XVADDWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110100001100100, vk, vj, vd))
#define XVADDWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110100001100101, vk, vj, vd))
#define XVADDWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110100001100110, vk, vj, vd))
#define XVADDWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110100001100111, vk, vj, vd))
#define XVSUBWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110100001100000, vk, vj, vd))
#define XVSUBWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110100001100001, vk, vj, vd))
#define XVSUBWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110100001100010, vk, vj, vd))
#define XVSUBWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110100001100011, vk, vj, vd))
#define XVSUBWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110100001101000, vk, vj, vd))
#define XVSUBWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110100001101001, vk, vj, vd))
#define XVSUBWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110100001101010, vk, vj, vd))
#define XVSUBWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110100001101011, vk, vj, vd))
#define XVADDWEV_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110100001111100, vk, vj, vd))
#define XVADDWEV_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110100001111101, vk, vj, vd))
#define XVADDWEV_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110100001111110, vk, vj, vd))
#define XVADDWEV_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110100001111111, vk, vj, vd))
#define XVADDWOD_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110100010000000, vk, vj, vd))
#define XVADDWOD_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110100010000001, vk, vj, vd))
#define XVADDWOD_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110100010000010, vk, vj, vd))
#define XVADDWOD_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110100010000011, vk, vj, vd))
#define XVAVG_B(vd, vj, vk)          EMIT(type_3R(0b01110100011001000, vk, vj, vd))
#define XVAVG_H(vd, vj, vk)          EMIT(type_3R(0b01110100011001001, vk, vj, vd))
#define XVAVG_W(vd, vj, vk)          EMIT(type_3R(0b01110100011001010, vk, vj, vd))
#define XVAVG_D(vd, vj, vk)          EMIT(type_3R(0b01110100011001011, vk, vj, vd))
#define XVAVG_BU(vd, vj, vk)         EMIT(type_3R(0b01110100011001100, vk, vj, vd))
#define XVAVG_HU(vd, vj, vk)         EMIT(type_3R(0b01110100011001101, vk, vj, vd))
#define XVAVG_WU(vd, vj, vk)         EMIT(type_3R(0b01110100011001110, vk, vj, vd))
#define XVAVG_DU(vd, vj, vk)         EMIT(type_3R(0b01110100011001111, vk, vj, vd))
#define XVAVGR_B(vd, vj, vk)         EMIT(type_3R(0b01110100011010000, vk, vj, vd))
#define XVAVGR_H(vd, vj, vk)         EMIT(type_3R(0b01110100011010001, vk, vj, vd))
#define XVAVGR_W(vd, vj, vk)         EMIT(type_3R(0b01110100011010010, vk, vj, vd))
#define XVAVGR_D(vd, vj, vk)         EMIT(type_3R(0b01110100011010011, vk, vj, vd))
#define XVAVGR_BU(vd, vj, vk)        EMIT(type_3R(0b01110100011010100, vk, vj, vd))
#define XVAVGR_HU(vd, vj, vk)        EMIT(type_3R(0b01110100011010101, vk, vj, vd))
#define XVAVGR_WU(vd, vj, vk)        EMIT(type_3R(0b01110100011010110, vk, vj, vd))
#define XVAVGR_DU(vd, vj, vk)        EMIT(type_3R(0b01110100011010111, vk, vj, vd))
#define XVABSD_B(vd, vj, vk)         EMIT(type_3R(0b01110100011000000, vk, vj, vd))
#define XVABSD_H(vd, vj, vk)         EMIT(type_3R(0b01110100011000001, vk, vj, vd))
#define XVABSD_W(vd, vj, vk)         EMIT(type_3R(0b01110100011000010, vk, vj, vd))
#define XVABSD_D(vd, vj, vk)         EMIT(type_3R(0b01110100011000011, vk, vj, vd))
#define XVABSD_BU(vd, vj, vk)        EMIT(type_3R(0b01110100011000100, vk, vj, vd))
#define XVABSD_HU(vd, vj, vk)        EMIT(type_3R(0b01110100011000101, vk, vj, vd))
#define XVABSD_WU(vd, vj, vk)        EMIT(type_3R(0b01110100011000110, vk, vj, vd))
#define XVABSD_DU(vd, vj, vk)        EMIT(type_3R(0b01110100011000111, vk, vj, vd))
#define XVADDA_B(vd, vj, vk)         EMIT(type_3R(0b01110100010111000, vk, vj, vd))
#define XVADDA_H(vd, vj, vk)         EMIT(type_3R(0b01110100010111001, vk, vj, vd))
#define XVADDA_W(vd, vj, vk)         EMIT(type_3R(0b01110100010111010, vk, vj, vd))
#define XVADDA_D(vd, vj, vk)         EMIT(type_3R(0b01110100010111011, vk, vj, vd))
#define XVMAX_B(vd, vj, vk)          EMIT(type_3R(0b01110100011100000, vk, vj, vd))
#define XVMAX_H(vd, vj, vk)          EMIT(type_3R(0b01110100011100001, vk, vj, vd))
#define XVMAX_W(vd, vj, vk)          EMIT(type_3R(0b01110100011100010, vk, vj, vd))
#define XVMAX_D(vd, vj, vk)          EMIT(type_3R(0b01110100011100011, vk, vj, vd))
#define XVMAX_BU(vd, vj, vk)         EMIT(type_3R(0b01110100011101000, vk, vj, vd))
#define XVMAX_HU(vd, vj, vk)         EMIT(type_3R(0b01110100011101001, vk, vj, vd))
#define XVMAX_WU(vd, vj, vk)         EMIT(type_3R(0b01110100011101010, vk, vj, vd))
#define XVMAX_DU(vd, vj, vk)         EMIT(type_3R(0b01110100011101011, vk, vj, vd))
#define XVMIN_B(vd, vj, vk)          EMIT(type_3R(0b01110100011100100, vk, vj, vd))
#define XVMIN_H(vd, vj, vk)          EMIT(type_3R(0b01110100011100101, vk, vj, vd))
#define XVMIN_W(vd, vj, vk)          EMIT(type_3R(0b01110100011100110, vk, vj, vd))
#define XVMIN_D(vd, vj, vk)          EMIT(type_3R(0b01110100011100111, vk, vj, vd))
#define XVMIN_BU(vd, vj, vk)         EMIT(type_3R(0b01110100011101100, vk, vj, vd))
#define XVMIN_HU(vd, vj, vk)         EMIT(type_3R(0b01110100011101101, vk, vj, vd))
#define XVMIN_WU(vd, vj, vk)         EMIT(type_3R(0b01110100011101110, vk, vj, vd))
#define XVMIN_DU(vd, vj, vk)         EMIT(type_3R(0b01110100011101111, vk, vj, vd))
#define XVMUL_B(vd, vj, vk)          EMIT(type_3R(0b01110100100001000, vk, vj, vd))
#define XVMUL_H(vd, vj, vk)          EMIT(type_3R(0b01110100100001001, vk, vj, vd))
#define XVMUL_W(vd, vj, vk)          EMIT(type_3R(0b01110100100001010, vk, vj, vd))
#define XVMUL_D(vd, vj, vk)          EMIT(type_3R(0b01110100100001011, vk, vj, vd))
#define XVMUH_B(vd, vj, vk)          EMIT(type_3R(0b01110100100001100, vk, vj, vd))
#define XVMUH_H(vd, vj, vk)          EMIT(type_3R(0b01110100100001101, vk, vj, vd))
#define XVMUH_W(vd, vj, vk)          EMIT(type_3R(0b01110100100001110, vk, vj, vd))
#define XVMUH_D(vd, vj, vk)          EMIT(type_3R(0b01110100100001111, vk, vj, vd))
#define XVMUH_BU(vd, vj, vk)         EMIT(type_3R(0b01110100100010000, vk, vj, vd))
#define XVMUH_HU(vd, vj, vk)         EMIT(type_3R(0b01110100100010001, vk, vj, vd))
#define XVMUH_WU(vd, vj, vk)         EMIT(type_3R(0b01110100100010010, vk, vj, vd))
#define XVMUH_DU(vd, vj, vk)         EMIT(type_3R(0b01110100100010011, vk, vj, vd))
#define XVMULWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110100100100000, vk, vj, vd))
#define XVMULWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110100100100001, vk, vj, vd))
#define XVMULWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110100100100010, vk, vj, vd))
#define XVMULWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110100100100011, vk, vj, vd))
#define XVMULWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110100100100100, vk, vj, vd))
#define XVMULWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110100100100101, vk, vj, vd))
#define XVMULWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110100100100110, vk, vj, vd))
#define XVMULWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110100100100111, vk, vj, vd))
#define XVMULWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110100100110000, vk, vj, vd))
#define XVMULWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110100100110001, vk, vj, vd))
#define XVMULWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110100100110010, vk, vj, vd))
#define XVMULWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110100100110011, vk, vj, vd))
#define XVMULWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110100100110100, vk, vj, vd))
#define XVMULWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110100100110101, vk, vj, vd))
#define XVMULWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110100100110110, vk, vj, vd))
#define XVMULWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110100100110111, vk, vj, vd))
#define XVMULWEV_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110100101000000, vk, vj, vd))
#define XVMULWEV_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110100101000001, vk, vj, vd))
#define XVMULWEV_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110100101000010, vk, vj, vd))
#define XVMULWEV_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110100101000011, vk, vj, vd))
#define XVMULWOD_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110100101000100, vk, vj, vd))
#define XVMULWOD_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110100101000101, vk, vj, vd))
#define XVMULWOD_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110100101000110, vk, vj, vd))
#define XVMULWOD_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110100101000111, vk, vj, vd))
#define XVMADD_B(vd, vj, vk)         EMIT(type_3R(0b01110100101010000, vk, vj, vd))
#define XVMADD_H(vd, vj, vk)         EMIT(type_3R(0b01110100101010001, vk, vj, vd))
#define XVMADD_W(vd, vj, vk)         EMIT(type_3R(0b01110100101010010, vk, vj, vd))
#define XVMADD_D(vd, vj, vk)         EMIT(type_3R(0b01110100101010011, vk, vj, vd))
#define XVMSUB_B(vd, vj, vk)         EMIT(type_3R(0b01110100101010100, vk, vj, vd))
#define XVMSUB_H(vd, vj, vk)         EMIT(type_3R(0b01110100101010101, vk, vj, vd))
#define XVMSUB_W(vd, vj, vk)         EMIT(type_3R(0b01110100101010110, vk, vj, vd))
#define XVMSUB_D(vd, vj, vk)         EMIT(type_3R(0b01110100101010111, vk, vj, vd))
#define XVMADDWEV_H_B(vd, vj, vk)    EMIT(type_3R(0b01110100101011000, vk, vj, vd))
#define XVMADDWEV_W_H(vd, vj, vk)    EMIT(type_3R(0b01110100101011001, vk, vj, vd))
#define XVMADDWEV_D_W(vd, vj, vk)    EMIT(type_3R(0b01110100101011010, vk, vj, vd))
#define XVMADDWEV_Q_D(vd, vj, vk)    EMIT(type_3R(0b01110100101011011, vk, vj, vd))
#define XVMADDWOD_H_B(vd, vj, vk)    EMIT(type_3R(0b01110100101011100, vk, vj, vd))
#define XVMADDWOD_W_H(vd, vj, vk)    EMIT(type_3R(0b01110100101011101, vk, vj, vd))
#define XVMADDWOD_D_W(vd, vj, vk)    EMIT(type_3R(0b01110100101011110, vk, vj, vd))
#define XVMADDWOD_Q_D(vd, vj, vk)    EMIT(type_3R(0b01110100101011111, vk, vj, vd))
#define XVMADDWEV_H_BU(vd, vj, vk)   EMIT(type_3R(0b01110100101101000, vk, vj, vd))
#define XVMADDWEV_W_HU(vd, vj, vk)   EMIT(type_3R(0b01110100101101001, vk, vj, vd))
#define XVMADDWEV_D_WU(vd, vj, vk)   EMIT(type_3R(0b01110100101101010, vk, vj, vd))
#define XVMADDWEV_Q_DU(vd, vj, vk)   EMIT(type_3R(0b01110100101101011, vk, vj, vd))
#define XVMADDWOD_H_BU(vd, vj, vk)   EMIT(type_3R(0b01110100101101100, vk, vj, vd))
#define XVMADDWOD_W_HU(vd, vj, vk)   EMIT(type_3R(0b01110100101101101, vk, vj, vd))
#define XVMADDWOD_D_WU(vd, vj, vk)   EMIT(type_3R(0b01110100101101110, vk, vj, vd))
#define XVMADDWOD_Q_DU(vd, vj, vk)   EMIT(type_3R(0b01110100101101111, vk, vj, vd))
#define XVMADDWEV_H_BU_B(vd, vj, vk) EMIT(type_3R(0b01110100101111000, vk, vj, vd))
#define XVMADDWEV_W_HU_H(vd, vj, vk) EMIT(type_3R(0b01110100101111001, vk, vj, vd))
#define XVMADDWEV_D_WU_W(vd, vj, vk) EMIT(type_3R(0b01110100101111010, vk, vj, vd))
#define XVMADDWEV_Q_DU_D(vd, vj, vk) EMIT(type_3R(0b01110100101111011, vk, vj, vd))
#define XVMADDWOD_H_BU_B(vd, vj, vk) EMIT(type_3R(0b01110100101111100, vk, vj, vd))
#define XVMADDWOD_W_HU_H(vd, vj, vk) EMIT(type_3R(0b01110100101111101, vk, vj, vd))
#define XVMADDWOD_D_WU_W(vd, vj, vk) EMIT(type_3R(0b01110100101111110, vk, vj, vd))
#define XVMADDWOD_Q_DU_D(vd, vj, vk) EMIT(type_3R(0b01110100101111111, vk, vj, vd))
#define XVDIV_B(vd, vj, vk)          EMIT(type_3R(0b01110100111000000, vk, vj, vd))
#define XVDIV_H(vd, vj, vk)          EMIT(type_3R(0b01110100111000001, vk, vj, vd))
#define XVDIV_W(vd, vj, vk)          EMIT(type_3R(0b01110100111000010, vk, vj, vd))
#define XVDIV_D(vd, vj, vk)          EMIT(type_3R(0b01110100111000011, vk, vj, vd))
#define XVMOD_B(vd, vj, vk)          EMIT(type_3R(0b01110100111000100, vk, vj, vd))
#define XVMOD_H(vd, vj, vk)          EMIT(type_3R(0b01110100111000101, vk, vj, vd))
#define XVMOD_W(vd, vj, vk)          EMIT(type_3R(0b01110100111000110, vk, vj, vd))
#define XVMOD_D(vd, vj, vk)          EMIT(type_3R(0b01110100111000111, vk, vj, vd))
#define XVDIV_BU(vd, vj, vk)         EMIT(type_3R(0b01110100111001000, vk, vj, vd))
#define XVDIV_HU(vd, vj, vk)         EMIT(type_3R(0b01110100111001001, vk, vj, vd))
#define XVDIV_WU(vd, vj, vk)         EMIT(type_3R(0b01110100111001010, vk, vj, vd))
#define XVDIV_DU(vd, vj, vk)         EMIT(type_3R(0b01110100111001011, vk, vj, vd))
#define XVMOD_BU(vd, vj, vk)         EMIT(type_3R(0b01110100111001100, vk, vj, vd))
#define XVMOD_HU(vd, vj, vk)         EMIT(type_3R(0b01110100111001101, vk, vj, vd))
#define XVMOD_WU(vd, vj, vk)         EMIT(type_3R(0b01110100111001110, vk, vj, vd))
#define XVMOD_DU(vd, vj, vk)         EMIT(type_3R(0b01110100111001111, vk, vj, vd))
#define VSAT_B(vd, vj, imm3)         EMIT(type_2RI3(0b0111001100100100001, imm3, vj, vd))
#define VSAT_H(vd, vj, imm4)         EMIT(type_2RI4(0b011100110010010001, imm4, vj, vd))
#define VSAT_W(vd, vj, imm5)         EMIT(type_2RI5(0b01110011001001001, imm5, vj, vd))
#define VSAT_D(vd, vj, imm6)         EMIT(type_2RI6(0b0111001100100101, imm6, vj, vd))
#define VSAT_BU(vd, vj, imm3)        EMIT(type_2RI3(0b0111001100101000001, imm3, vj, vd))
#define VSAT_HU(vd, vj, imm4)        EMIT(type_2RI4(0b011100110010100001, imm4, vj, vd))
#define VSAT_WU(vd, vj, imm5)        EMIT(type_2RI5(0b01110011001010001, imm5, vj, vd))
#define VSAT_DU(vd, vj, imm6)        EMIT(type_2RI6(0b0111001100101001, imm6, vj, vd))
#define VEXTH_H_B(vd, vj)            EMIT(type_2R(0b0111001010011110111000, vj, vd))
#define VEXTH_W_H(vd, vj)            EMIT(type_2R(0b0111001010011110111001, vj, vd))
#define VEXTH_D_W(vd, vj)            EMIT(type_2R(0b0111001010011110111010, vj, vd))
#define VEXTH_Q_D(vd, vj)            EMIT(type_2R(0b0111001010011110111011, vj, vd))
#define VEXTH_HU_BU(vd, vj)          EMIT(type_2R(0b0111001010011110111100, vj, vd))
#define VEXTH_WU_HU(vd, vj)          EMIT(type_2R(0b0111001010011110111101, vj, vd))
#define VEXTH_DU_WU(vd, vj)          EMIT(type_2R(0b0111001010011110111110, vj, vd))
#define VEXTH_QU_DU(vd, vj)          EMIT(type_2R(0b0111001010011110111111, vj, vd))
#define XVSIGNCOV_B(vd, vj, vk)      EMIT(type_3R(0b01110101001011100, vk, vj, vd))
#define XVSIGNCOV_H(vd, vj, vk)      EMIT(type_3R(0b01110101001011101, vk, vj, vd))
#define XVSIGNCOV_W(vd, vj, vk)      EMIT(type_3R(0b01110101001011110, vk, vj, vd))
#define XVSIGNCOV_D(vd, vj, vk)      EMIT(type_3R(0b01110101001011111, vk, vj, vd))
#define XVAND_V(vd, vj, vk)          EMIT(type_3R(0b01110101001001100, vk, vj, vd))
#define XVOR_V(vd, vj, vk)           EMIT(type_3R(0b01110101001001101, vk, vj, vd))
#define XVXOR_V(vd, vj, vk)          EMIT(type_3R(0b01110101001001110, vk, vj, vd))
#define XVNOR_V(vd, vj, vk)          EMIT(type_3R(0b01110101001001111, vk, vj, vd))
#define XVANDN_V(vd, vj, vk)         EMIT(type_3R(0b01110101001010000, vk, vj, vd))
#define XVORN_V(vd, vj, vk)          EMIT(type_3R(0b01110101001010001, vk, vj, vd))
#define XVSLL_B(vd, vj, vk)          EMIT(type_3R(0b01110100111010000, vk, vj, vd))
#define XVSLL_H(vd, vj, vk)          EMIT(type_3R(0b01110100111010001, vk, vj, vd))
#define XVSLL_W(vd, vj, vk)          EMIT(type_3R(0b01110100111010010, vk, vj, vd))
#define XVSLL_D(vd, vj, vk)          EMIT(type_3R(0b01110100111010011, vk, vj, vd))
#define XVSRL_B(vd, vj, vk)          EMIT(type_3R(0b01110100111010100, vk, vj, vd))
#define XVSRL_H(vd, vj, vk)          EMIT(type_3R(0b01110100111010101, vk, vj, vd))
#define XVSRL_W(vd, vj, vk)          EMIT(type_3R(0b01110100111010110, vk, vj, vd))
#define XVSRL_D(vd, vj, vk)          EMIT(type_3R(0b01110100111010111, vk, vj, vd))
#define XVSRA_B(vd, vj, vk)          EMIT(type_3R(0b01110100111011000, vk, vj, vd))
#define XVSRA_H(vd, vj, vk)          EMIT(type_3R(0b01110100111011001, vk, vj, vd))
#define XVSRA_W(vd, vj, vk)          EMIT(type_3R(0b01110100111011010, vk, vj, vd))
#define XVSRA_D(vd, vj, vk)          EMIT(type_3R(0b01110100111011011, vk, vj, vd))
#define XVROTR_B(vd, vj, vk)         EMIT(type_3R(0b01110100111011100, vk, vj, vd))
#define XVROTR_H(vd, vj, vk)         EMIT(type_3R(0b01110100111011101, vk, vj, vd))
#define XVROTR_W(vd, vj, vk)         EMIT(type_3R(0b01110100111011110, vk, vj, vd))
#define XVROTR_D(vd, vj, vk)         EMIT(type_3R(0b01110100111011111, vk, vj, vd))
#define XVSRLR_B(vd, vj, vk)         EMIT(type_3R(0b01110100111100000, vk, vj, vd))
#define XVSRLR_H(vd, vj, vk)         EMIT(type_3R(0b01110100111100001, vk, vj, vd))
#define XVSRLR_W(vd, vj, vk)         EMIT(type_3R(0b01110100111100010, vk, vj, vd))
#define XVSRLR_D(vd, vj, vk)         EMIT(type_3R(0b01110100111100011, vk, vj, vd))
#define XVSRAR_B(vd, vj, vk)         EMIT(type_3R(0b01110100111100100, vk, vj, vd))
#define XVSRAR_H(vd, vj, vk)         EMIT(type_3R(0b01110100111100101, vk, vj, vd))
#define XVSRAR_W(vd, vj, vk)         EMIT(type_3R(0b01110100111100110, vk, vj, vd))
#define XVSRAR_D(vd, vj, vk)         EMIT(type_3R(0b01110100111100111, vk, vj, vd))
#define XVSRLN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110100111101001, vk, vj, vd))
#define XVSRLN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110100111101010, vk, vj, vd))
#define XVSRLN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110100111101011, vk, vj, vd))
#define XVSRAN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110100111101101, vk, vj, vd))
#define XVSRAN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110100111101110, vk, vj, vd))
#define XVSRAN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110100111101111, vk, vj, vd))
#define XVSRLRN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110100111110001, vk, vj, vd))
#define XVSRLRN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110100111110010, vk, vj, vd))
#define XVSRLRN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110100111110011, vk, vj, vd))
#define XVSRARN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110100111110101, vk, vj, vd))
#define XVSRARN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110100111110110, vk, vj, vd))
#define XVSRARN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110100111110111, vk, vj, vd))
#define XVSSRLRN_B_H(vd, vj, vk)     EMIT(type_3R(0b01110101000000001, vk, vj, vd))
#define XVSSRLRN_H_W(vd, vj, vk)     EMIT(type_3R(0b01110101000000010, vk, vj, vd))
#define XVSSRLRN_W_D(vd, vj, vk)     EMIT(type_3R(0b01110101000000011, vk, vj, vd))
#define XVSSRARN_B_H(vd, vj, vk)     EMIT(type_3R(0b01110101000000101, vk, vj, vd))
#define XVSSRARN_H_W(vd, vj, vk)     EMIT(type_3R(0b01110101000000110, vk, vj, vd))
#define XVSSRARN_W_D(vd, vj, vk)     EMIT(type_3R(0b01110101000000111, vk, vj, vd))
#define XVSSRLRN_BU_H(vd, vj, vk)    EMIT(type_3R(0b01110101000010001, vk, vj, vd))
#define XVSSRLRN_HU_W(vd, vj, vk)    EMIT(type_3R(0b01110101000010010, vk, vj, vd))
#define XVSSRLRN_WU_D(vd, vj, vk)    EMIT(type_3R(0b01110101000010011, vk, vj, vd))
#define XVSSRARN_BU_H(vd, vj, vk)    EMIT(type_3R(0b01110101000010101, vk, vj, vd))
#define XVSSRARN_HU_W(vd, vj, vk)    EMIT(type_3R(0b01110101000010110, vk, vj, vd))
#define XVSSRARN_WU_D(vd, vj, vk)    EMIT(type_3R(0b01110101000010111, vk, vj, vd))
#define XVBITCLR_B(vd, vj, vk)       EMIT(type_3R(0b01110101000011000, vk, vj, vd))
#define XVBITCLR_H(vd, vj, vk)       EMIT(type_3R(0b01110101000011001, vk, vj, vd))
#define XVBITCLR_W(vd, vj, vk)       EMIT(type_3R(0b01110101000011010, vk, vj, vd))
#define XVBITCLR_D(vd, vj, vk)       EMIT(type_3R(0b01110101000011011, vk, vj, vd))
#define XVBITSET_B(vd, vj, vk)       EMIT(type_3R(0b01110101000011100, vk, vj, vd))
#define XVBITSET_H(vd, vj, vk)       EMIT(type_3R(0b01110101000011101, vk, vj, vd))
#define XVBITSET_W(vd, vj, vk)       EMIT(type_3R(0b01110101000011110, vk, vj, vd))
#define XVBITSET_D(vd, vj, vk)       EMIT(type_3R(0b01110101000011111, vk, vj, vd))
#define XVBITREV_B(vd, vj, vk)       EMIT(type_3R(0b01110101000100000, vk, vj, vd))
#define XVBITREV_H(vd, vj, vk)       EMIT(type_3R(0b01110101000100001, vk, vj, vd))
#define XVBITREV_W(vd, vj, vk)       EMIT(type_3R(0b01110101000100010, vk, vj, vd))
#define XVBITREV_D(vd, vj, vk)       EMIT(type_3R(0b01110101000100011, vk, vj, vd))
#define XVFRSTP_B(vd, vj, vk)        EMIT(type_3R(0b01110101001010110, vk, vj, vd))
#define XVFRSTP_H(vd, vj, vk)        EMIT(type_3R(0b01110101001010111, vk, vj, vd))
#define XVCLO_B(xd, xj)              EMIT(type_2R(0b0111011010011100000000, xj, xd))
#define XVCLO_H(xd, xj)              EMIT(type_2R(0b0111011010011100000001, xj, xd))
#define XVCLO_W(xd, xj)              EMIT(type_2R(0b0111011010011100000010, xj, xd))
#define XVCLO_D(xd, xj)              EMIT(type_2R(0b0111011010011100000011, xj, xd))
#define XVCLZ_B(xd, xj)              EMIT(type_2R(0b0111011010011100000100, xj, xd))
#define XVCLZ_H(xd, xj)              EMIT(type_2R(0b0111011010011100000101, xj, xd))
#define XVCLZ_W(xd, xj)              EMIT(type_2R(0b0111011010011100000110, xj, xd))
#define XVCLZ_D(xd, xj)              EMIT(type_2R(0b0111011010011100000111, xj, xd))
#define XVPCNT_B(xd, xj)             EMIT(type_2R(0b0111011010011100001000, xj, xd))
#define XVPCNT_H(xd, xj)             EMIT(type_2R(0b0111011010011100001001, xj, xd))
#define XVPCNT_W(xd, xj)             EMIT(type_2R(0b0111011010011100001010, xj, xd))
#define XVPCNT_D(xd, xj)             EMIT(type_2R(0b0111011010011100001011, xj, xd))
#define XVNEG_B(xd, xj)              EMIT(type_2R(0b0111011010011100001100, xj, xd))
#define XVNEG_H(xd, xj)              EMIT(type_2R(0b0111011010011100001101, xj, xd))
#define XVNEG_W(xd, xj)              EMIT(type_2R(0b0111011010011100001110, xj, xd))
#define XVNEG_D(xd, xj)              EMIT(type_2R(0b0111011010011100001111, xj, xd))
#define XVMSKLTZ_B(xd, xj)           EMIT(type_2R(0b0111011010011100010000, xj, xd))
#define XVMSKLTZ_H(xd, xj)           EMIT(type_2R(0b0111011010011100010001, xj, xd))
#define XVMSKLTZ_W(xd, xj)           EMIT(type_2R(0b0111011010011100010010, xj, xd))
#define XVMSKLTZ_D(xd, xj)           EMIT(type_2R(0b0111011010011100010011, xj, xd))
#define XVMSKGEZ_B(xd, xj)           EMIT(type_2R(0b0111011010011100010100, xj, xd))
#define XVMSKNZ_B(xd, xj)            EMIT(type_2R(0b0111011010011100011000, xj, xd))
#define XVSETEQZ_V(cd, xj)           EMIT(type_2R(0b0111011010011100100110, xj, cd & 0b111))
#define XVSETNEZ_V(cd, xj)           EMIT(type_2R(0b0111011010011100100111, xj, cd & 0b111))
#define XVSETANYEQZ_B(cd, xj)        EMIT(type_2R(0b0111011010011100101000, xj, cd & 0b111))
#define XVSETANYEQZ_H(cd, xj)        EMIT(type_2R(0b0111011010011100101001, xj, cd & 0b111))
#define XVSETANYEQZ_W(cd, xj)        EMIT(type_2R(0b0111011010011100101010, xj, cd & 0b111))
#define XVSETANYEQZ_D(cd, xj)        EMIT(type_2R(0b0111011010011100101011, xj, cd & 0b111))
#define XVSETALLNEZ_B(cd, xj)        EMIT(type_2R(0b0111011010011100101100, xj, cd & 0b111))
#define XVSETALLNEZ_H(cd, xj)        EMIT(type_2R(0b0111011010011100101101, xj, cd & 0b111))
#define XVSETALLNEZ_W(cd, xj)        EMIT(type_2R(0b0111011010011100101110, xj, cd & 0b111))
#define XVSETALLNEZ_D(cd, xj)        EMIT(type_2R(0b0111011010011100101111, xj, cd & 0b111))
#define XVFLOGB_S(xd, xj)            EMIT(type_2R(0b0111011010011100110001, xj, xd))
#define XVFLOGB_D(xd, xj)            EMIT(type_2R(0b0111011010011100110010, xj, xd))
#define XVFCLASS_S(xd, xj)           EMIT(type_2R(0b0111011010011100110101, xj, xd))
#define XVFCLASS_D(xd, xj)           EMIT(type_2R(0b0111011010011100110110, xj, xd))
#define XVFSQRT_S(xd, xj)            EMIT(type_2R(0b0111011010011100111001, xj, xd))
#define XVFSQRT_D(xd, xj)            EMIT(type_2R(0b0111011010011100111010, xj, xd))
#define XVFRECIP_S(xd, xj)           EMIT(type_2R(0b0111011010011100111101, xj, xd))
#define XVFRECIP_D(xd, xj)           EMIT(type_2R(0b0111011010011100111110, xj, xd))
#define XVFRSQRT_S(xd, xj)           EMIT(type_2R(0b0111011010011101000001, xj, xd))
#define XVFRSQRT_D(xd, xj)           EMIT(type_2R(0b0111011010011101000010, xj, xd))
#define XVFRECIPE_S(xd, xj)          EMIT(type_2R(0b0111011010011101000101, xj, xd))
#define XVFRECIPE_D(xd, xj)          EMIT(type_2R(0b0111011010011101000110, xj, xd))
#define XVFRSQRTE_S(xd, xj)          EMIT(type_2R(0b0111011010011101001001, xj, xd))
#define XVFRSQRTE_D(xd, xj)          EMIT(type_2R(0b0111011010011101001010, xj, xd))
#define XVFRINT_S(xd, xj)            EMIT(type_2R(0b0111011010011101001101, xj, xd))
#define XVFRINT_D(xd, xj)            EMIT(type_2R(0b0111011010011101001110, xj, xd))
#define XVFRINTRM_S(xd, xj)          EMIT(type_2R(0b0111011010011101010001, xj, xd))
#define XVFRINTRM_D(xd, xj)          EMIT(type_2R(0b0111011010011101010010, xj, xd))
#define XVFRINTRP_S(xd, xj)          EMIT(type_2R(0b0111011010011101010101, xj, xd))
#define XVFRINTRP_D(xd, xj)          EMIT(type_2R(0b0111011010011101010110, xj, xd))
#define XVFRINTRZ_S(xd, xj)          EMIT(type_2R(0b0111011010011101011001, xj, xd))
#define XVFRINTRZ_D(xd, xj)          EMIT(type_2R(0b0111011010011101011010, xj, xd))
#define XVFRINTRNE_S(xd, xj)         EMIT(type_2R(0b0111011010011101011101, xj, xd))
#define XVFRINTRNE_D(xd, xj)         EMIT(type_2R(0b0111011010011101011110, xj, xd))
#define XVFADD_S(vd, vj, vk)         EMIT(type_3R(0b01110101001100001, vk, vj, vd))
#define XVFADD_D(vd, vj, vk)         EMIT(type_3R(0b01110101001100010, vk, vj, vd))
#define XVFSUB_S(vd, vj, vk)         EMIT(type_3R(0b01110101001100101, vk, vj, vd))
#define XVFSUB_D(vd, vj, vk)         EMIT(type_3R(0b01110101001100110, vk, vj, vd))
#define XVFMUL_S(vd, vj, vk)         EMIT(type_3R(0b01110101001110001, vk, vj, vd))
#define XVFMUL_D(vd, vj, vk)         EMIT(type_3R(0b01110101001110010, vk, vj, vd))
#define XVFDIV_S(vd, vj, vk)         EMIT(type_3R(0b01110101001110101, vk, vj, vd))
#define XVFDIV_D(vd, vj, vk)         EMIT(type_3R(0b01110101001110110, vk, vj, vd))
#define XVFMAX_S(vd, vj, vk)         EMIT(type_3R(0b01110101001111001, vk, vj, vd))
#define XVFMAX_D(vd, vj, vk)         EMIT(type_3R(0b01110101001111010, vk, vj, vd))
#define XVFMIN_S(vd, vj, vk)         EMIT(type_3R(0b01110101001111101, vk, vj, vd))
#define XVFMIN_D(vd, vj, vk)         EMIT(type_3R(0b01110101001111110, vk, vj, vd))
#define XVFMAXA_S(vd, vj, vk)        EMIT(type_3R(0b01110101010000001, vk, vj, vd))
#define XVFMAXA_D(vd, vj, vk)        EMIT(type_3R(0b01110101010000010, vk, vj, vd))
#define XVFMINA_S(vd, vj, vk)        EMIT(type_3R(0b01110101010000101, vk, vj, vd))
#define XVFMINA_D(vd, vj, vk)        EMIT(type_3R(0b01110101010000110, vk, vj, vd))
#define XVFRINT_S(xd, xj)            EMIT(type_2R(0b0111011010011101001101, xj, xd))
#define XVFRINT_D(xd, xj)            EMIT(type_2R(0b0111011010011101001110, xj, xd))
#define XVFRINTRM_S(xd, xj)          EMIT(type_2R(0b0111011010011101010001, xj, xd))
#define XVFRINTRM_D(xd, xj)          EMIT(type_2R(0b0111011010011101010010, xj, xd))
#define XVFRINTRP_S(xd, xj)          EMIT(type_2R(0b0111011010011101010101, xj, xd))
#define XVFRINTRP_D(xd, xj)          EMIT(type_2R(0b0111011010011101010110, xj, xd))
#define XVFRINTRZ_S(xd, xj)          EMIT(type_2R(0b0111011010011101011001, xj, xd))
#define XVFRINTRZ_D(xd, xj)          EMIT(type_2R(0b0111011010011101011010, xj, xd))
#define XVFRINTRNE_S(xd, xj)         EMIT(type_2R(0b0111011010011101011101, xj, xd))
#define XVFRINTRNE_D(xd, xj)         EMIT(type_2R(0b0111011010011101011110, xj, xd))
#define XVFRINTRRD_S(vd, vj, rm)     EMIT(type_2RI4(0b011101101001110101, ((rm & 0b11) << 2) | 0b01, vj, vd))
#define XVFRINTRRD_D(vd, vj, rm)     EMIT(type_2RI4(0b011101101001110101, ((rm & 0b11) << 2) | 0b10, vj, vd))
#define XVFCVTL_S_H(xd, xj)          EMIT(type_2R(0b0111011010011101111010, xj, xd))
#define XVFCVTH_S_H(xd, xj)          EMIT(type_2R(0b0111011010011101111011, xj, xd))
#define XVFCVTL_D_S(xd, xj)          EMIT(type_2R(0b0111011010011101111100, xj, xd))
#define XVFCVTH_D_S(xd, xj)          EMIT(type_2R(0b0111011010011101111101, xj, xd))
#define XVFFINT_S_W(xd, xj)          EMIT(type_2R(0b0111011010011110000000, xj, xd))
#define XVFFINT_S_WU(xd, xj)         EMIT(type_2R(0b0111011010011110000001, xj, xd))
#define XVFFINT_D_L(xd, xj)          EMIT(type_2R(0b0111011010011110000010, xj, xd))
#define XVFFINT_D_LU(xd, xj)         EMIT(type_2R(0b0111011010011110000011, xj, xd))
#define XVFFINTL_D_W(xd, xj)         EMIT(type_2R(0b0111011010011110000100, xj, xd))
#define XVFFINTH_D_W(xd, xj)         EMIT(type_2R(0b0111011010011110000101, xj, xd))
#define XVFTINT_W_S(xd, xj)          EMIT(type_2R(0b0111011010011110001100, xj, xd))
#define XVFTINT_L_D(xd, xj)          EMIT(type_2R(0b0111011010011110001101, xj, xd))
#define XVFTINTRM_W_S(xd, xj)        EMIT(type_2R(0b0111011010011110001110, xj, xd))
#define XVFTINTRM_L_D(xd, xj)        EMIT(type_2R(0b0111011010011110001111, xj, xd))
#define XVFTINTRP_W_S(xd, xj)        EMIT(type_2R(0b0111011010011110010000, xj, xd))
#define XVFTINTRP_L_D(xd, xj)        EMIT(type_2R(0b0111011010011110010001, xj, xd))
#define XVFTINTRZ_W_S(xd, xj)        EMIT(type_2R(0b0111011010011110010010, xj, xd))
#define XVFTINTRZ_L_D(xd, xj)        EMIT(type_2R(0b0111011010011110010011, xj, xd))
#define XVFTINTRNE_W_S(xd, xj)       EMIT(type_2R(0b0111011010011110010100, xj, xd))
#define XVFTINTRNE_L_D(xd, xj)       EMIT(type_2R(0b0111011010011110010101, xj, xd))
#define XVFTINT_WU_S(xd, xj)         EMIT(type_2R(0b0111011010011110010110, xj, xd))
#define XVFTINT_LU_D(xd, xj)         EMIT(type_2R(0b0111011010011110010111, xj, xd))
#define XVFTINTRZ_WU_S(xd, xj)       EMIT(type_2R(0b0111011010011110011100, xj, xd))
#define XVFTINTRZ_LU_D(xd, xj)       EMIT(type_2R(0b0111011010011110011101, xj, xd))
#define XVFTINTL_L_S(xd, xj)         EMIT(type_2R(0b0111011010011110100000, xj, xd))
#define XVFTINTH_L_S(xd, xj)         EMIT(type_2R(0b0111011010011110100001, xj, xd))
#define XVFTINTRML_L_S(xd, xj)       EMIT(type_2R(0b0111011010011110100010, xj, xd))
#define XVFTINTRMH_L_S(xd, xj)       EMIT(type_2R(0b0111011010011110100011, xj, xd))
#define XVFTINTRPL_L_S(xd, xj)       EMIT(type_2R(0b0111011010011110100100, xj, xd))
#define XVFTINTRPH_L_S(xd, xj)       EMIT(type_2R(0b0111011010011110100101, xj, xd))
#define XVFTINTRZL_L_S(xd, xj)       EMIT(type_2R(0b0111011010011110100110, xj, xd))
#define XVFTINTRZH_L_S(xd, xj)       EMIT(type_2R(0b0111011010011110100111, xj, xd))
#define XVFTINTRNEL_L_S(xd, xj)      EMIT(type_2R(0b0111011010011110101000, xj, xd))
#define XVFTINTRNEH_L_S(xd, xj)      EMIT(type_2R(0b0111011010011110101001, xj, xd))
#define XVFCVT_H_S(xd, xj, xk)       EMIT(type_3R(0b01110101010001100, xk, xj, xd))
#define XVFCVT_S_D(xd, xj, xk)       EMIT(type_3R(0b01110101010001101, xk, xj, xd))
#define XVFFINT_S_L(xd, xj, xk)      EMIT(type_3R(0b01110101010010000, xk, xj, xd))
#define XVFTINT_W_D(xd, xj, xk)      EMIT(type_3R(0b01110101010010011, xk, xj, xd))
#define XVFTINTRM_W_D(xd, xj, xk)    EMIT(type_3R(0b01110101010010100, xk, xj, xd))
#define XVFTINTRP_W_D(xd, xj, xk)    EMIT(type_3R(0b01110101010010101, xk, xj, xd))
#define XVFTINTRZ_W_D(xd, xj, xk)    EMIT(type_3R(0b01110101010010110, xk, xj, xd))
#define XVFTINTRNE_W_D(xd, xj, xk)   EMIT(type_3R(0b01110101010010111, xk, xj, xd))
#define XVSEQ_B(vd, vj, vk)          EMIT(type_3R(0b01110100000000000, vk, vj, vd))
#define XVSEQ_H(vd, vj, vk)          EMIT(type_3R(0b01110100000000001, vk, vj, vd))
#define XVSEQ_W(vd, vj, vk)          EMIT(type_3R(0b01110100000000010, vk, vj, vd))
#define XVSEQ_D(vd, vj, vk)          EMIT(type_3R(0b01110100000000011, vk, vj, vd))
#define XVSLE_B(vd, vj, vk)          EMIT(type_3R(0b01110100000000100, vk, vj, vd))
#define XVSLE_H(vd, vj, vk)          EMIT(type_3R(0b01110100000000101, vk, vj, vd))
#define XVSLE_W(vd, vj, vk)          EMIT(type_3R(0b01110100000000110, vk, vj, vd))
#define XVSLE_D(vd, vj, vk)          EMIT(type_3R(0b01110100000000111, vk, vj, vd))
#define XVSLE_BU(vd, vj, vk)         EMIT(type_3R(0b01110100000001000, vk, vj, vd))
#define XVSLE_HU(vd, vj, vk)         EMIT(type_3R(0b01110100000001001, vk, vj, vd))
#define XVSLE_WU(vd, vj, vk)         EMIT(type_3R(0b01110100000001010, vk, vj, vd))
#define XVSLE_DU(vd, vj, vk)         EMIT(type_3R(0b01110100000001011, vk, vj, vd))
#define XVSLT_B(vd, vj, vk)          EMIT(type_3R(0b01110100000001100, vk, vj, vd))
#define XVSLT_H(vd, vj, vk)          EMIT(type_3R(0b01110100000001101, vk, vj, vd))
#define XVSLT_W(vd, vj, vk)          EMIT(type_3R(0b01110100000001110, vk, vj, vd))
#define XVSLT_D(vd, vj, vk)          EMIT(type_3R(0b01110100000001111, vk, vj, vd))
#define XVSLT_BU(vd, vj, vk)         EMIT(type_3R(0b01110100000010000, vk, vj, vd))
#define XVSLT_HU(vd, vj, vk)         EMIT(type_3R(0b01110100000010001, vk, vj, vd))
#define XVSLT_WU(vd, vj, vk)         EMIT(type_3R(0b01110100000010010, vk, vj, vd))
#define XVSLT_DU(vd, vj, vk)         EMIT(type_3R(0b01110100000010011, vk, vj, vd))
#define XVBSLL_V(vd, vj, imm5)       EMIT(type_2RI5(0b01110110100011100, imm5, vj, vd))
#define XVBSRL_V(vd, vj, imm5)       EMIT(type_2RI5(0b01110110100011101, imm5, vj, vd))
#define XVPACKEV_B(vd, vj, vk)       EMIT(type_3R(0b01110101000101100, vk, vj, vd))
#define XVPACKEV_H(vd, vj, vk)       EMIT(type_3R(0b01110101000101101, vk, vj, vd))
#define XVPACKEV_W(vd, vj, vk)       EMIT(type_3R(0b01110101000101110, vk, vj, vd))
#define XVPACKEV_D(vd, vj, vk)       EMIT(type_3R(0b01110101000101111, vk, vj, vd))
#define XVPACKOD_B(vd, vj, vk)       EMIT(type_3R(0b01110101000110000, vk, vj, vd))
#define XVPACKOD_H(vd, vj, vk)       EMIT(type_3R(0b01110101000110001, vk, vj, vd))
#define XVPACKOD_W(vd, vj, vk)       EMIT(type_3R(0b01110101000110010, vk, vj, vd))
#define XVPACKOD_D(vd, vj, vk)       EMIT(type_3R(0b01110101000110011, vk, vj, vd))
#define XVPICKEV_B(vd, vj, vk)       EMIT(type_3R(0b01110101000111100, vk, vj, vd))
#define XVPICKEV_H(vd, vj, vk)       EMIT(type_3R(0b01110101000111101, vk, vj, vd))
#define XVPICKEV_W(vd, vj, vk)       EMIT(type_3R(0b01110101000111110, vk, vj, vd))
#define XVPICKEV_D(vd, vj, vk)       EMIT(type_3R(0b01110101000111111, vk, vj, vd))
#define XVPICKOD_B(vd, vj, vk)       EMIT(type_3R(0b01110101001000000, vk, vj, vd))
#define XVPICKOD_H(vd, vj, vk)       EMIT(type_3R(0b01110101001000001, vk, vj, vd))
#define XVPICKOD_W(vd, vj, vk)       EMIT(type_3R(0b01110101001000010, vk, vj, vd))
#define XVPICKOD_D(vd, vj, vk)       EMIT(type_3R(0b01110101001000011, vk, vj, vd))
#define XVILVL_B(vd, vj, vk)         EMIT(type_3R(0b01110101000110100, vk, vj, vd))
#define XVILVL_H(vd, vj, vk)         EMIT(type_3R(0b01110101000110101, vk, vj, vd))
#define XVILVL_W(vd, vj, vk)         EMIT(type_3R(0b01110101000110110, vk, vj, vd))
#define XVILVL_D(vd, vj, vk)         EMIT(type_3R(0b01110101000110111, vk, vj, vd))
#define XVILVH_B(vd, vj, vk)         EMIT(type_3R(0b01110101000111000, vk, vj, vd))
#define XVILVH_H(vd, vj, vk)         EMIT(type_3R(0b01110101000111001, vk, vj, vd))
#define XVILVH_W(vd, vj, vk)         EMIT(type_3R(0b01110101000111010, vk, vj, vd))
#define XVILVH_D(vd, vj, vk)         EMIT(type_3R(0b01110101000111011, vk, vj, vd))
#define XVSHUF_B(xd, xj, xk, xa)     EMIT(type_4R(0b000011010110, xa, xk, xj, xd))
#define XVSHUF_H(vd, vj, vk)         EMIT(type_3R(0b01110101011110101, vk, vj, vd))
#define XVSHUF_W(vd, vj, vk)         EMIT(type_3R(0b01110101011110110, vk, vj, vd))
#define XVSHUF_D(vd, vj, vk)         EMIT(type_3R(0b01110101011110111, vk, vj, vd))
#define XVSHUF4I_B(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100100, imm8, vj, vd))
#define XVSHUF4I_H(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100101, imm8, vj, vd))
#define XVSHUF4I_W(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100110, imm8, vj, vd))
#define XVSHUF4I_D(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100111, imm8, vj, vd))
#define XVPERM_W(vd, vj, vk)         EMIT(type_3R(0b01110101011111010, vk, vj, vd))
#define XVPERMI_W(vd, vj, imm8)      EMIT(type_2RI8(0b01110111111001, imm8, vj, vd))
#define XVPERMI_D(vd, vj, imm8)      EMIT(type_2RI8(0b01110111111010, imm8, vj, vd))
#define XVPERMI_Q(vd, vj, imm8)      EMIT(type_2RI8(0b01110111111011, imm8, vj, vd))
#define VEXT2XV_H_B(vd, vj)          EMIT(type_2R(0b0111011010011111000100, vj, vd))
#define VEXT2XV_W_B(vd, vj)          EMIT(type_2R(0b0111011010011111000101, vj, vd))
#define VEXT2XV_D_B(vd, vj)          EMIT(type_2R(0b0111011010011111000110, vj, vd))
#define VEXT2XV_W_H(vd, vj)          EMIT(type_2R(0b0111011010011111000111, vj, vd))
#define VEXT2XV_D_H(vd, vj)          EMIT(type_2R(0b0111011010011111001000, vj, vd))
#define VEXT2XV_D_W(vd, vj)          EMIT(type_2R(0b0111011010011111001001, vj, vd))
#define VEXT2XV_HU_BU(vd, vj)        EMIT(type_2R(0b0111011010011111001010, vj, vd))
#define VEXT2XV_WU_BU(vd, vj)        EMIT(type_2R(0b0111011010011111001011, vj, vd))
#define VEXT2XV_DU_BU(vd, vj)        EMIT(type_2R(0b0111011010011111001100, vj, vd))
#define VEXT2XV_WU_HU(vd, vj)        EMIT(type_2R(0b0111011010011111001101, vj, vd))
#define VEXT2XV_DU_HU(vd, vj)        EMIT(type_2R(0b0111011010011111001110, vj, vd))
#define VEXT2XV_DU_WU(vd, vj)        EMIT(type_2R(0b0111011010011111001111, vj, vd))
#define XVADDI_WU(vd, vj, imm5)      EMIT(type_2RI5(0b01110110100010110, imm5, vj, vd))
#define XVSRLNI_H_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110111010000001, imm5, vj, vd))
#define XVSRLI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110111001100001, imm5, vj, vd))
#define VSETEQZ_V(cd, vj)            EMIT(type_2R(0b0111001010011100100110, vj, cd & 0b111))
#define VSETNEZ_V(cd, vj)            EMIT(type_2R(0b0111001010011100100111, vj, cd & 0b111))
#define VSETANYEQZ_B(cd, vj)         EMIT(type_2R(0b0111001010011100101000, vj, cd & 0b111))
#define VSETANYEQZ_H(cd, vj)         EMIT(type_2R(0b0111001010011100101001, vj, cd & 0b111))
#define VSETANYEQZ_W(cd, vj)         EMIT(type_2R(0b0111001010011100101010, vj, cd & 0b111))
#define VSETANYEQZ_D(cd, vj)         EMIT(type_2R(0b0111001010011100101011, vj, cd & 0b111))
#define VSETALLNEZ_B(cd, vj)         EMIT(type_2R(0b0111001010011100101100, vj, cd & 0b111))
#define VSETALLNEZ_H(cd, vj)         EMIT(type_2R(0b0111001010011100101101, vj, cd & 0b111))
#define VSETALLNEZ_W(cd, vj)         EMIT(type_2R(0b0111001010011100101110, vj, cd & 0b111))
#define VSETALLNEZ_D(cd, vj)         EMIT(type_2R(0b0111001010011100101111, vj, cd & 0b111))
#define VINSGR2VR_B(vd, rj, imm4)    EMIT(type_2RI4(0b011100101110101110, imm4, rj, vd))
#define VINSGR2VR_H(vd, rj, imm3)    EMIT(type_2RI3(0b0111001011101011110, imm3, rj, vd))
#define VINSGR2VR_W(vd, rj, imm2)    EMIT(type_2RI2(0b01110010111010111110, imm2, rj, vd))
#define VINSGR2VR_D(vd, rj, imm1)    EMIT(type_2RI1(0b011100101110101111110, imm1, rj, vd))
#define VPCNT_B(vd, vj)              EMIT(type_2R(0b0111001010011100001000, vj, vd))
#define VPCNT_H(vd, vj)              EMIT(type_2R(0b0111001010011100001001, vj, vd))
#define VPCNT_W(vd, vj)              EMIT(type_2R(0b0111001010011100001010, vj, vd))
#define VPCNT_D(vd, vj)              EMIT(type_2R(0b0111001010011100001011, vj, vd))
#define VPICKVE2GR_B(rd, vj, imm4)   EMIT(type_2RI4(0b011100101110111110, imm4, vj, rd))
#define VPICKVE2GR_H(rd, vj, imm3)   EMIT(type_2RI3(0b0111001011101111110, imm3, vj, rd))
#define VPICKVE2GR_W(rd, vj, imm2)   EMIT(type_2RI2(0b01110010111011111110, imm2, vj, rd))
#define VPICKVE2GR_D(rd, vj, imm1)   EMIT(type_2RI1(0b011100101110111111110, imm1, vj, rd))
#define VPICKVE2GR_BU(rd, vj, imm4)  EMIT(type_2RI4(0b011100101111001110, imm4, vj, rd))
#define VPICKVE2GR_HU(rd, vj, imm3)  EMIT(type_2RI3(0b0111001011110011110, imm3, vj, rd))
#define VPICKVE2GR_WU(rd, vj, imm2)  EMIT(type_2RI2(0b01110010111100111110, imm2, vj, rd))
#define VPICKVE2GR_DU(rd, vj, imm1)  EMIT(type_2RI1(0b011100101111001111110, imm1, vj, rd))
#define VFRINT_S(vd, vj)             EMIT(type_2R(0b0111001010011101001101, vj, vd))
#define VFRINT_D(vd, vj)             EMIT(type_2R(0b0111001010011101001110, vj, vd))
//  vfrintrm.d 0010 vfrintrm.s 0001, vfrintrne.d 1110 vfrintrne.s 1101...
#define VFRINTRRD_S(vd, vj, rm)     EMIT(type_2RI4(0b011100101001110101, ((rm & 0b11) << 2) | 0b01, vj, vd))
#define VFRINTRRD_D(vd, vj, rm)     EMIT(type_2RI4(0b011100101001110101, ((rm & 0b11) << 2) | 0b10, vj, vd))
#define VREPLGR2VR_B(vd, rj)        EMIT(type_2R(0b0111001010011111000000, rj, vd))
#define VREPLGR2VR_H(vd, rj)        EMIT(type_2R(0b0111001010011111000001, rj, vd))
#define VREPLGR2VR_W(vd, rj)        EMIT(type_2R(0b0111001010011111000010, rj, vd))
#define VREPLGR2VR_D(vd, rj)        EMIT(type_2R(0b0111001010011111000011, rj, vd))
#define VSLLWIL_H_B(vd, vj, imm3)   EMIT(type_2RI3(0b0111001100001000001, imm3, vj, vd))
#define VSLLWIL_W_H(vd, vj, imm4)   EMIT(type_2RI4(0b011100110000100001, imm4, vj, vd))
#define VSLLWIL_D_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110011000010001, imm5, vj, vd))
#define VSLLWIL_HU_BU(vd, vj, imm3) EMIT(type_2RI3(0b0111001100001100001, imm3, vj, vd))
#define VSLLWIL_WU_HU(vd, vj, imm4) EMIT(type_2RI4(0b011100110000110001, imm4, vj, vd))
#define VSLLWIL_DU_WU(vd, vj, imm5) EMIT(type_2RI5(0b01110011000011001, imm5, vj, vd))
#define VNEG_B(vd, vj)              EMIT(type_2R(0b0111011010011100001100, vj, vd))
#define VNEG_H(vd, vj)              EMIT(type_2R(0b0111011010011100001101, vj, vd))
#define VNEG_W(vd, vj)              EMIT(type_2R(0b0111011010011100001110, vj, vd))
#define VNEG_D(vd, vj)              EMIT(type_2R(0b0111011010011100001111, vj, vd))

#define XVLD(vd, rj, imm12)             EMIT(type_2RI12(0b0010110010, imm12, rj, vd))
#define XVST(vd, rj, imm12)             EMIT(type_2RI12(0b0010110011, imm12, rj, vd))
#define XVLDX(vd, vj, vk)               EMIT(type_3R(0b00111000010010000, vk, vj, vd))
#define XVSTX(vd, vj, vk)               EMIT(type_3R(0b00111000010011000, vk, vj, vd))
#define XVLDREPL_D(xd, rj, offset)      EMIT(type_2RI9(0b0011001000010, (offset >> 3), rj, xd))
#define XVLDREPL_W(xd, rj, offset)      EMIT(type_2RI10(0b001100100010, (offset >> 2), rj, xd))
#define XVLDREPL_H(xd, rj, offset)      EMIT(type_2RI11(0b00110010010, (offset >> 1), rj, xd))
#define XVLDREPL_B(xd, rj, offset)      EMIT(type_2RI12(0b0011001010, offset, rj, xd))
#define XVSTELM_D(xd, rj, offset, imm2) EMIT(type_2RI10(0b001100110001, ((imm2) << 8) | (offset), rj, xd))
#define XVSTELM_W(xd, rj, offset, imm3) EMIT(type_2RI11(0b00110011001, ((imm3) << 8) | (offset), rj, xd))
#define XVSTELM_H(xd, rj, offset, imm4) EMIT(type_2RI12(0b0011001101, ((imm4) << 8) | (offset), rj, xd))
#define XVSTELM_B(xd, rj, offset, imm5) EMIT(type_2RI13(0b001100111, ((imm5) << 8) | (offset), rj, xd))

#define XVHSELI_D(vd, vj, imm5)      EMIT(type_2RI5(0b01110110100111111, imm5, vj, vd))
#define XVROTRI_B(vd, vj, imm3)      EMIT(type_2RI3(0b0111011010100000001, imm3, vj, vd))
#define XVROTRI_H(vd, vj, imm4)      EMIT(type_2RI4(0b011101101010000001, imm4, vj, vd))
#define XVROTRI_W(vd, vj, imm5)      EMIT(type_2RI5(0b01110110101000001, imm5, vj, vd))
#define XVROTRI_D(vd, vj, imm6)      EMIT(type_2RI6(0b0111011010100001, imm6, vj, vd))
#define XVSRLRI_B(vd, vj, imm3)      EMIT(type_2RI3(0b0111011010100100001, imm3, vj, vd))
#define XVSRLRI_H(vd, vj, imm4)      EMIT(type_2RI4(0b011101101010010001, imm4, vj, vd))
#define XVSRLRI_W(vd, vj, imm5)      EMIT(type_2RI5(0b01110110101001001, imm5, vj, vd))
#define XVSRLRI_D(vd, vj, imm6)      EMIT(type_2RI6(0b0111011010100101, imm6, vj, vd))
#define XVSRARI_B(vd, vj, imm3)      EMIT(type_2RI3(0b0111011010101000001, imm3, vj, vd))
#define XVSRARI_H(vd, vj, imm4)      EMIT(type_2RI4(0b011101101010100001, imm4, vj, vd))
#define XVSRARI_W(vd, vj, imm5)      EMIT(type_2RI5(0b01110110101010001, imm5, vj, vd))
#define XVSRARI_D(vd, vj, imm6)      EMIT(type_2RI6(0b0111011010101001, imm6, vj, vd))
#define XVINSGR2VR_W(vd, vj, imm3)   EMIT(type_2RI3(0b0111011011101011110, imm3, vj, vd))
#define XVINSGR2VR_D(vd, vj, imm2)   EMIT(type_2RI2(0b01110110111010111110, imm2, vj, vd))
#define XVPICKVE2GR_W(vd, vj, imm3)  EMIT(type_2RI3(0b0111011011101111110, imm3, vj, vd))
#define XVPICKVE2GR_D(vd, vj, imm2)  EMIT(type_2RI2(0b01110110111011111110, imm2, vj, vd))
#define XVPICKVE2GR_WU(vd, vj, imm3) EMIT(type_2RI3(0b0111011011110011110, imm3, vj, vd))
#define XVPICKVE2GR_DU(vd, vj, imm2) EMIT(type_2RI2(0b01110110111100111110, imm2, vj, vd))
#define XVREPL128VEI_B(vd, vj, imm4) EMIT(type_2RI4(0b011101101111011110, imm4, vj, vd))
#define XVREPL128VEI_H(vd, vj, imm3) EMIT(type_2RI3(0b0111011011110111110, imm3, vj, vd))
#define XVREPL128VEI_W(vd, vj, imm2) EMIT(type_2RI2(0b01110110111101111110, imm2, vj, vd))
#define XVREPL128VEI_D(vd, vj, imm1) EMIT(type_2RI1(0b011101101111011111110, imm1, vj, vd))
#define XVINSVE0_W(vd, vj, imm3)     EMIT(type_2RI3(0b0111011011111111110, imm3, vj, vd))
#define XVINSVE0_D(vd, vj, imm2)     EMIT(type_2RI2(0b01110110111111111110, imm2, vj, vd))
#define XVPICKVE_W(vd, vj, imm3)     EMIT(type_2RI3(0b0111011100000011110, imm3, vj, vd))
#define XVPICKVE_D(vd, vj, imm2)     EMIT(type_2RI2(0b01110111000000111110, imm2, vj, vd))
#define XVREPLVE0_B(xd, xj)          EMIT(type_2R(0b0111011100000111000000, xj, xd))
#define XVREPLVE0_H(xd, xj)          EMIT(type_2R(0b0111011100000111100000, xj, xd))
#define XVREPLVE0_W(xd, xj)          EMIT(type_2R(0b0111011100000111110000, xj, xd))
#define XVREPLVE0_D(xd, xj)          EMIT(type_2R(0b0111011100000111111000, xj, xd))
#define XVREPLVE0_Q(xd, xj)          EMIT(type_2R(0b0111011100000111111100, xj, xd))
#define XVEXTL_Q_D(xd, xj)           EMIT(type_2R(0b0111011100001001000000, xj, xd))
#define XVEXTL_QU_DU(xd, xj)         EMIT(type_2R(0b0111011100001101000000, xj, xd))
#define XVSLLWIL_H_B(vd, vj, imm3)   EMIT(type_2RI3(0b0111011100001000001, imm3, vj, vd))
#define XVSLLWIL_W_H(vd, vj, imm4)   EMIT(type_2RI4(0b011101110000100001, imm4, vj, vd))
#define XVSLLWIL_D_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110111000010001, imm5, vj, vd))
#define XVSLLWIL_HU_BU(vd, vj, imm3) EMIT(type_2RI3(0b0111011100001100001, imm3, vj, vd))
#define XVSLLWIL_WU_HU(vd, vj, imm4) EMIT(type_2RI4(0b011101110000110001, imm4, vj, vd))
#define XVSLLWIL_DU_WU(vd, vj, imm5) EMIT(type_2RI5(0b01110111000011001, imm5, vj, vd))
#define XVBITCLRI_B(vd, vj, imm3)    EMIT(type_2RI3(0b0111011100010000001, imm3, vj, vd))
#define XVBITCLRI_H(vd, vj, imm4)    EMIT(type_2RI4(0b011101110001000001, imm4, vj, vd))
#define XVBITCLRI_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110111000100001, imm5, vj, vd))
#define XVBITCLRI_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111011100010001, imm6, vj, vd))
#define XVBITSETI_B(vd, vj, imm3)    EMIT(type_2RI3(0b0111011100010100001, imm3, vj, vd))
#define XVBITSETI_H(vd, vj, imm4)    EMIT(type_2RI4(0b011101110001010001, imm4, vj, vd))
#define XVBITSETI_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110111000101001, imm5, vj, vd))
#define XVBITSETI_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111011100010101, imm6, vj, vd))
#define XVBITREVI_B(vd, vj, imm3)    EMIT(type_2RI3(0b0111011100011000001, imm3, vj, vd))
#define XVBITREVI_H(vd, vj, imm4)    EMIT(type_2RI4(0b011101110001100001, imm4, vj, vd))
#define XVBITREVI_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110111000110001, imm5, vj, vd))
#define XVBITREVI_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111011100011001, imm6, vj, vd))
#define XVSAT_B(vd, vj, imm3)        EMIT(type_2RI3(0b0111011100100100001, imm3, vj, vd))
#define XVSAT_H(vd, vj, imm4)        EMIT(type_2RI4(0b011101110010010001, imm4, vj, vd))
#define XVSAT_W(vd, vj, imm5)        EMIT(type_2RI5(0b01110111001001001, imm5, vj, vd))
#define XVSAT_D(vd, vj, imm6)        EMIT(type_2RI6(0b0111011100100101, imm6, vj, vd))
#define XVSAT_BU(vd, vj, imm3)       EMIT(type_2RI3(0b0111011100101000001, imm3, vj, vd))
#define XVSAT_HU(vd, vj, imm4)       EMIT(type_2RI4(0b011101110010100001, imm4, vj, vd))
#define XVSAT_WU(vd, vj, imm5)       EMIT(type_2RI5(0b01110111001010001, imm5, vj, vd))
#define XVSAT_DU(vd, vj, imm6)       EMIT(type_2RI6(0b0111011100101001, imm6, vj, vd))
#define XVSLLI_B(vd, vj, imm3)       EMIT(type_2RI3(0b0111011100101100001, imm3, vj, vd))
#define XVSLLI_H(vd, vj, imm4)       EMIT(type_2RI4(0b011101110010110001, imm4, vj, vd))
#define XVSLLI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110111001011001, imm5, vj, vd))
#define XVSLLI_D(vd, vj, imm6)       EMIT(type_2RI6(0b0111011100101101, imm6, vj, vd))
#define XVSRLI_B(vd, vj, imm3)       EMIT(type_2RI3(0b0111011100110000001, imm3, vj, vd))
#define XVSRLI_H(vd, vj, imm4)       EMIT(type_2RI4(0b011101110011000001, imm4, vj, vd))
#define XVSRLI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110111001100001, imm5, vj, vd))
#define XVSRLI_D(vd, vj, imm6)       EMIT(type_2RI6(0b0111011100110001, imm6, vj, vd))
#define XVSRAI_B(vd, vj, imm3)       EMIT(type_2RI3(0b0111011100110100001, imm3, vj, vd))
#define XVSRAI_H(vd, vj, imm4)       EMIT(type_2RI4(0b011101110011010001, imm4, vj, vd))
#define XVSRAI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110111001101001, imm5, vj, vd))
#define XVSRAI_D(vd, vj, imm6)       EMIT(type_2RI6(0b0111011100110101, imm6, vj, vd))
#define XVSRLNI_B_H(vd, vj, imm4)    EMIT(type_2RI4(0b011101110100000001, imm4, vj, vd))
#define XVSRLNI_H_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110111010000001, imm5, vj, vd))
#define XVSRLNI_W_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111011101000001, imm6, vj, vd))
#define XVSRLNI_D_Q(vd, vj, imm7)    EMIT(type_2RI7(0b011101110100001, imm7, vj, vd))
#define XVSRLRNI_B_H(vd, vj, imm4)   EMIT(type_2RI4(0b011101110100010001, imm4, vj, vd))
#define XVSRLRNI_H_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110111010001001, imm5, vj, vd))
#define XVSRLRNI_W_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111011101000101, imm6, vj, vd))
#define XVSRLRNI_D_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011101110100011, imm7, vj, vd))
#define XVSSRLNI_B_H(vd, vj, imm4)   EMIT(type_2RI4(0b011101110100100001, imm4, vj, vd))
#define XVSSRLNI_H_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110111010010001, imm5, vj, vd))
#define XVSSRLNI_W_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111011101001001, imm6, vj, vd))
#define XVSSRLNI_D_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011101110100101, imm7, vj, vd))
#define XVSSRLNI_BU_H(vd, vj, imm4)  EMIT(type_2RI4(0b011101110100110001, imm4, vj, vd))
#define XVSSRLNI_HU_W(vd, vj, imm5)  EMIT(type_2RI5(0b01110111010011001, imm5, vj, vd))
#define XVSSRLNI_WU_D(vd, vj, imm6)  EMIT(type_2RI6(0b0111011101001101, imm6, vj, vd))
#define XVSSRLNI_DU_Q(vd, vj, imm7)  EMIT(type_2RI7(0b011101110100111, imm7, vj, vd))
#define XVSSRLRNI_B_H(vd, vj, imm4)  EMIT(type_2RI4(0b011101110101000001, imm4, vj, vd))
#define XVSSRLRNI_H_W(vd, vj, imm5)  EMIT(type_2RI5(0b01110111010100001, imm5, vj, vd))
#define XVSSRLRNI_W_D(vd, vj, imm6)  EMIT(type_2RI6(0b0111011101010001, imm6, vj, vd))
#define XVSSRLRNI_D_Q(vd, vj, imm7)  EMIT(type_2RI7(0b011101110101001, imm7, vj, vd))
#define XVSSRLRNI_BU_H(vd, vj, imm4) EMIT(type_2RI4(0b011101110101010001, imm4, vj, vd))
#define XVSSRLRNI_HU_W(vd, vj, imm5) EMIT(type_2RI5(0b01110111010101001, imm5, vj, vd))
#define XVSSRLRNI_WU_D(vd, vj, imm6) EMIT(type_2RI6(0b0111011101010101, imm6, vj, vd))
#define XVSSRLRNI_DU_Q(vd, vj, imm7) EMIT(type_2RI7(0b011101110101011, imm7, vj, vd))
#define XVSRANI_B_H(vd, vj, imm4)    EMIT(type_2RI4(0b011101110101100001, imm4, vj, vd))
#define XVSRANI_H_W(vd, vj, imm5)    EMIT(type_2RI5(0b01110111010110001, imm5, vj, vd))
#define XVSRANI_W_D(vd, vj, imm6)    EMIT(type_2RI6(0b0111011101011001, imm6, vj, vd))
#define XVSRANI_D_Q(vd, vj, imm7)    EMIT(type_2RI7(0b011101110101101, imm7, vj, vd))
#define XVSRARNI_B_H(vd, vj, imm4)   EMIT(type_2RI4(0b011101110101110001, imm4, vj, vd))
#define XVSRARNI_H_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110111010111001, imm5, vj, vd))
#define XVSRARNI_W_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111011101011101, imm6, vj, vd))
#define XVSRARNI_D_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011101110101111, imm7, vj, vd))
#define XVSSRANI_B_H(vd, vj, imm4)   EMIT(type_2RI4(0b011101110110000001, imm4, vj, vd))
#define XVSSRANI_H_W(vd, vj, imm5)   EMIT(type_2RI5(0b01110111011000001, imm5, vj, vd))
#define XVSSRANI_W_D(vd, vj, imm6)   EMIT(type_2RI6(0b0111011101100001, imm6, vj, vd))
#define XVSSRANI_D_Q(vd, vj, imm7)   EMIT(type_2RI7(0b011101110110001, imm7, vj, vd))
#define XVSSRANI_BU_H(vd, vj, imm4)  EMIT(type_2RI4(0b011101110110010001, imm4, vj, vd))
#define XVSSRANI_HU_W(vd, vj, imm5)  EMIT(type_2RI5(0b01110111011001001, imm5, vj, vd))
#define XVSSRANI_WU_D(vd, vj, imm6)  EMIT(type_2RI6(0b0111011101100101, imm6, vj, vd))
#define XVSSRANI_DU_Q(vd, vj, imm7)  EMIT(type_2RI7(0b011101110110011, imm7, vj, vd))
#define XVSSRARNI_B_H(vd, vj, imm4)  EMIT(type_2RI4(0b011101110110100001, imm4, vj, vd))
#define XVSSRARNI_H_W(vd, vj, imm5)  EMIT(type_2RI5(0b01110111011010001, imm5, vj, vd))
#define XVSSRARNI_W_D(vd, vj, imm6)  EMIT(type_2RI6(0b0111011101101001, imm6, vj, vd))
#define XVSSRARNI_D_Q(vd, vj, imm7)  EMIT(type_2RI7(0b011101110110101, imm7, vj, vd))
#define XVSSRARNI_BU_H(vd, vj, imm4) EMIT(type_2RI4(0b011101110110110001, imm4, vj, vd))
#define XVSSRARNI_HU_W(vd, vj, imm5) EMIT(type_2RI5(0b01110111011011001, imm5, vj, vd))
#define XVSSRARNI_WU_D(vd, vj, imm6) EMIT(type_2RI6(0b0111011101101101, imm6, vj, vd))
#define XVSSRARNI_DU_Q(vd, vj, imm7) EMIT(type_2RI7(0b011101110110111, imm7, vj, vd))
#define XVEXTRINS_D(vd, vj, imm8)    EMIT(type_2RI8(0b01110111100000, imm8, vj, vd))
#define XVEXTRINS_W(vd, vj, imm8)    EMIT(type_2RI8(0b01110111100001, imm8, vj, vd))
#define XVEXTRINS_H(vd, vj, imm8)    EMIT(type_2RI8(0b01110111100010, imm8, vj, vd))
#define XVEXTRINS_B(vd, vj, imm8)    EMIT(type_2RI8(0b01110111100011, imm8, vj, vd))
#define XVSHUF4I_B(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100100, imm8, vj, vd))
#define XVSHUF4I_H(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100101, imm8, vj, vd))
#define XVSHUF4I_W(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100110, imm8, vj, vd))
#define XVSHUF4I_D(vd, vj, imm8)     EMIT(type_2RI8(0b01110111100111, imm8, vj, vd))
#define XVBITSELI_B(vd, vj, imm8)    EMIT(type_2RI8(0b01110111110001, imm8, vj, vd))
#define XVANDI_B(vd, vj, imm8)       EMIT(type_2RI8(0b01110111110100, imm8, vj, vd))
#define XVORI_B(vd, vj, imm8)        EMIT(type_2RI8(0b01110111110101, imm8, vj, vd))
#define XVXORI_B(vd, vj, imm8)       EMIT(type_2RI8(0b01110111110110, imm8, vj, vd))
#define XVNORI_B(vd, vj, imm8)       EMIT(type_2RI8(0b01110111110111, imm8, vj, vd))
#define XVBITSEL_V(xd, xj, xk, xa)   EMIT(type_4R(0b000011010010, xa, xk, xj, xd))
#define XVSEQI_B(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000000, imm5, xj, xd))
#define XVSEQI_H(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000001, imm5, xj, xd))
#define XVSEQI_W(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000010, imm5, xj, xd))
#define XVSEQI_D(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000011, imm5, xj, xd))
#define XVSLEI_B(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000100, imm5, xj, xd))
#define XVSLEI_H(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000101, imm5, xj, xd))
#define XVSLEI_W(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000110, imm5, xj, xd))
#define XVSLEI_D(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100000111, imm5, xj, xd))
#define XVSLEI_BU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100001000, imm5, xj, xd))
#define XVSLEI_HU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100001001, imm5, xj, xd))
#define XVSLEI_WU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100001010, imm5, xj, xd))
#define XVSLEI_DU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100001011, imm5, xj, xd))
#define XVSLTI_B(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100001100, imm5, xj, xd))
#define XVSLTI_H(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100001101, imm5, xj, xd))
#define XVSLTI_W(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100001110, imm5, xj, xd))
#define XVSLTI_D(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100001111, imm5, xj, xd))
#define XVSLTI_BU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100010000, imm5, xj, xd))
#define XVSLTI_HU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100010001, imm5, xj, xd))
#define XVSLTI_WU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100010010, imm5, xj, xd))
#define XVSLTI_DU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100010011, imm5, xj, xd))
#define XVMAXI_B(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100000, imm5, xj, xd))
#define XVMAXI_H(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100001, imm5, xj, xd))
#define XVMAXI_W(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100010, imm5, xj, xd))
#define XVMAXI_D(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100011, imm5, xj, xd))
#define XVMINI_B(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100100, imm5, xj, xd))
#define XVMINI_H(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100101, imm5, xj, xd))
#define XVMINI_W(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100110, imm5, xj, xd))
#define XVMINI_D(xd, xj, imm5)       EMIT(type_2RI5(0b01110110100100111, imm5, xj, xd))
#define XVMAXI_BU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101000, imm5, xj, xd))
#define XVMAXI_HU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101001, imm5, xj, xd))
#define XVMAXI_WU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101010, imm5, xj, xd))
#define XVMAXI_DU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101011, imm5, xj, xd))
#define XVMINI_BU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101100, imm5, xj, xd))
#define XVMINI_HU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101101, imm5, xj, xd))
#define XVMINI_WU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101110, imm5, xj, xd))
#define XVMINI_DU(xd, xj, imm5)      EMIT(type_2RI5(0b01110110100101111, imm5, xj, xd))
#define XVFRSTPI_B(xd, xj, imm5)     EMIT(type_2RI5(0b01110110100110100, imm5, xj, xd))
#define XVFRSTPI_H(xd, xj, imm5)     EMIT(type_2RI5(0b01110110100110101, imm5, xj, xd))
#define XVLDI(xd, imm13)             EMIT(type_1RI13(0b01110111111000, imm13, xd))
#define XVSHUF_B(xd, xj, xk, xa)     EMIT(type_4R(0b000011010110, xa, xk, xj, xd))
#define XVREPLVE_B(xd, xj, rk)       EMIT(type_3R(0b01110101001000100, rk, xj, xd))
#define XVREPLVE_H(xd, xj, rk)       EMIT(type_3R(0b01110101001000101, rk, xj, xd))
#define XVREPLVE_W(xd, xj, rk)       EMIT(type_3R(0b01110101001000110, rk, xj, xd))
#define XVREPLVE_D(xd, xj, rk)       EMIT(type_3R(0b01110101001000111, rk, xj, xd))
#define XVREPLGR2VR_B(xd, rj)        EMIT(type_2R(0b0111011010011111000000, rj, xd))
#define XVREPLGR2VR_H(xd, rj)        EMIT(type_2R(0b0111011010011111000001, rj, xd))
#define XVREPLGR2VR_W(xd, rj)        EMIT(type_2R(0b0111011010011111000010, rj, xd))
#define XVREPLGR2VR_D(xd, rj)        EMIT(type_2R(0b0111011010011111000011, rj, xd))

#define XVFMADD_S(xd, xj, xk, xa)  EMIT(type_4R(0b000010100001, xa, xk, xj, xd))
#define XVFMSUB_S(xd, xj, xk, xa)  EMIT(type_4R(0b000010100101, xa, xk, xj, xd))
#define XVFNMADD_S(xd, xj, xk, xa) EMIT(type_4R(0b000010101001, xa, xk, xj, xd))
#define XVFNMSUB_S(xd, xj, xk, xa) EMIT(type_4R(0b000010101101, xa, xk, xj, xd))
#define XVFMADD_D(xd, xj, xk, xa)  EMIT(type_4R(0b000010100010, xa, xk, xj, xd))
#define XVFMSUB_D(xd, xj, xk, xa)  EMIT(type_4R(0b000010100110, xa, xk, xj, xd))
#define XVFNMADD_D(xd, xj, xk, xa) EMIT(type_4R(0b000010101010, xa, xk, xj, xd))
#define XVFNMSUB_D(xd, xj, xk, xa) EMIT(type_4R(0b000010101110, xa, xk, xj, xd))

#define VMEPATMSK_V(vd, mode, uimm5)  EMIT(type_1RI5I5(0b01110010100110111, uimm5, mode, vd))
#define XVMEPATMSK_V(xd, mode, uimm5) EMIT(type_1RI5I5(0b01110110100110111, uimm5, mode, xd))

////////////////////////////////////////////////////////////////////////////////
// (undocumented) LBT extension instructions

/* 5 new registers:
    LBT0 LBT1 LBT2 LBT3: scratch registers dedicated for the LBT extension.
    LBT4: high 32 bits is ftop, low 32 bits is eflags.

    Note: Instruction emitters below is not the exact name found in LoongArch Linux patches.
          We slightly changed some of the names to be more readable.

    If the floating point stack mode is set, any access to float registers will add LBT4.ftop to the register number.
    int fpr(num) {
        if (floating point stack mode)
            return fprs[num + ftop]
        else
            return fprs[num]
    }

    Beware, out of bound fpr access is unpredictable.
*/
// Set the floating point stack mode (FCSR0[6]) to 0.
#define X64_CLR_STACKMODE() EMIT(0x00008028)
// Set the floating point stack mode (FCSR0[6]) to 1.
#define X64_SET_STACKMODE() EMIT(0x00008008)

// INC/DEC LBT4.ftop.
#define X64_INC_TOP() EMIT(0x00008009)
#define X64_DEC_TOP() EMIT(0x00008029)

// GET/SET LBT4.ftop
#define X64_SET_TOP(imm3) EMIT(0x00007000 | ((imm3 & 0b111) << 5))
#define X64_GET_TOP(rd)   EMIT(type_2R(0x00007400, 0, rd))

#define X64_GET_EFLAGS(rd, mask8) EMIT(type_2RI8(0x17, mask8, 0, rd))
#define X64_SET_EFLAGS(rd, mask8) EMIT(type_2RI8(0x17, mask8, 1, rd))

// Reads OF/SF/ZF/CF/PF, set rd based on imm.
#define X64_SETJ(rd, imm) EMIT(type_2RI4(0b000000000011011010, imm, 0, rd))
// Here are the available enums:
#define X64_JMP_JNBE 0  /* CF=0 && ZF=0 */
#define X64_JMP_JNC  1  /* CF=0 */
#define X64_JMP_JC   2  /* CF=1 */
#define X64_JMP_JBE  3  /* CF=1 || ZF=1 */
#define X64_JMP_JZ   4  /* ZF=1 */
#define X64_JMP_JNZ  5  /* ZF=0 */
#define X64_JMP_JG   6  /* ZF=0 && SF == OF */
#define X64_JMP_JGE  7  /* SF == OF */
#define X64_JMP_JL   8  /* SF != OF */
#define X64_JMP_JLE  9  /* ZF=1 || SF != OF */
#define X64_JMP_JS   10 /* SF=1 */
#define X64_JMP_JNS  11 /* SF=0 */
#define X64_JMP_JO   12 /* OF=1 */
#define X64_JMP_JNO  13 /* OF=0 */
#define X64_JMP_JP   14 /* PF=1 */
#define X64_JMP_JNP  15 /* PF=0 */

// Note that these instructions only affect the LBT4.eflags.

#define X64_INC_B(rj)         EMIT(type_2R(0x20, rj, 0x0))
#define X64_INC_H(rj)         EMIT(type_2R(0x20, rj, 0x1))
#define X64_INC_W(rj)         EMIT(type_2R(0x20, rj, 0x2))
#define X64_INC_D(rj)         EMIT(type_2R(0x20, rj, 0x3))
#define X64_DEC_B(rj)         EMIT(type_2R(0x20, rj, 0x4))
#define X64_DEC_H(rj)         EMIT(type_2R(0x20, rj, 0x5))
#define X64_DEC_W(rj)         EMIT(type_2R(0x20, rj, 0x6))
#define X64_DEC_D(rj)         EMIT(type_2R(0x20, rj, 0x7))
#define X64_MUL_B(rj, rk)     EMIT(type_3R(0x7d, rk, rj, 0x0))
#define X64_MUL_H(rj, rk)     EMIT(type_3R(0x7d, rk, rj, 0x1))
#define X64_MUL_W(rj, rk)     EMIT(type_3R(0x7d, rk, rj, 0x2))
#define X64_MUL_D(rj, rk)     EMIT(type_3R(0x7d, rk, rj, 0x3))
#define X64_MUL_BU(rj, rk)    EMIT(type_3R(0x7d, rk, rj, 0x4))
#define X64_MUL_HU(rj, rk)    EMIT(type_3R(0x7d, rk, rj, 0x5))
#define X64_MUL_WU(rj, rk)    EMIT(type_3R(0x7d, rk, rj, 0x6))
#define X64_MUL_DU(rj, rk)    EMIT(type_3R(0x7d, rk, rj, 0x7))
#define X64_ADD_WU(rj, rk)    EMIT(type_3R(0x7e, rk, rj, 0x0))
#define X64_ADD_DU(rj, rk)    EMIT(type_3R(0x7e, rk, rj, 0x1))
#define X64_SUB_WU(rj, rk)    EMIT(type_3R(0x7e, rk, rj, 0x2))
#define X64_SUB_DU(rj, rk)    EMIT(type_3R(0x7e, rk, rj, 0x3))
#define X64_ADD_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x4))
#define X64_ADD_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x5))
#define X64_ADD_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x6))
#define X64_ADD_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x7))
#define X64_SUB_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x8))
#define X64_SUB_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x9))
#define X64_SUB_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0xa))
#define X64_SUB_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0xb))
#define X64_ADC_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0xc))
#define X64_ADC_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0xd))
#define X64_ADC_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0xe))
#define X64_ADC_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0xf))
#define X64_SBC_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x10))
#define X64_SBC_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x11))
#define X64_SBC_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x12))
#define X64_SBC_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x13))
#define X64_SLL_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x14))
#define X64_SLL_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x15))
#define X64_SLL_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x16))
#define X64_SLL_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x17))
#define X64_SRL_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x18))
#define X64_SRL_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x19))
#define X64_SRL_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x1a))
#define X64_SRL_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x1b))
#define X64_SRA_B(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x1c))
#define X64_SRA_H(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x1d))
#define X64_SRA_W(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x1e))
#define X64_SRA_D(rj, rk)     EMIT(type_3R(0x7e, rk, rj, 0x1f))
#define X64_ROTR_B(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x0))
#define X64_ROTR_H(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x1))
#define X64_ROTR_D(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x2))
#define X64_ROTR_W(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x3))
#define X64_ROTL_B(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x4))
#define X64_ROTL_H(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x5))
#define X64_ROTL_W(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x6))
#define X64_ROTL_D(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x7))
#define X64_RCR_B(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x8))
#define X64_RCR_H(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x9))
#define X64_RCR_W(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0xa))
#define X64_RCR_D(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0xb))
#define X64_RCL_B(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0xc))
#define X64_RCL_H(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0xd))
#define X64_RCL_W(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0xe))
#define X64_RCL_D(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0xf))
#define X64_AND_B(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x10))
#define X64_AND_H(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x11))
#define X64_AND_W(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x12))
#define X64_AND_D(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x13))
#define X64_OR_B(rj, rk)      EMIT(type_3R(0x7f, rk, rj, 0x14))
#define X64_OR_H(rj, rk)      EMIT(type_3R(0x7f, rk, rj, 0x15))
#define X64_OR_W(rj, rk)      EMIT(type_3R(0x7f, rk, rj, 0x16))
#define X64_OR_D(rj, rk)      EMIT(type_3R(0x7f, rk, rj, 0x17))
#define X64_XOR_B(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x18))
#define X64_XOR_H(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x19))
#define X64_XOR_W(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x1a))
#define X64_XOR_D(rj, rk)     EMIT(type_3R(0x7f, rk, rj, 0x1b))
#define X64_SLLI_B(rj, imm3)  EMIT(type_2RI3(0x2a1, imm3, rj, 0x0))
#define X64_SRLI_B(rj, imm3)  EMIT(type_2RI3(0x2a1, imm3, rj, 0x4))
#define X64_SRAI_B(rj, imm3)  EMIT(type_2RI3(0x2a1, imm3, rj, 0x8))
#define X64_ROTRI_B(rj, imm3) EMIT(type_2RI3(0x2a1, imm3, rj, 0xc))
#define X64_RCRI_B(rj, imm3)  EMIT(type_2RI3(0x2a1, imm3, rj, 0x10))
#define X64_ROTLI_B(rj, imm3) EMIT(type_2RI3(0x2a1, imm3, rj, 0x14))
#define X64_RCLI_B(rj, imm3)  EMIT(type_2RI3(0x2a1, imm3, rj, 0x18))
#define X64_SLLI_H(rj, imm4)  EMIT(type_2RI4(0x151, imm4, rj, 0x1))
#define X64_SRLI_H(rj, imm4)  EMIT(type_2RI4(0x151, imm4, rj, 0x5))
#define X64_SRAI_H(rj, imm4)  EMIT(type_2RI4(0x151, imm4, rj, 0x9))
#define X64_ROTRI_H(rj, imm4) EMIT(type_2RI4(0x151, imm4, rj, 0xd))
#define X64_RCRI_H(rj, imm4)  EMIT(type_2RI4(0x151, imm4, rj, 0x11))
#define X64_ROTLI_H(rj, imm4) EMIT(type_2RI4(0x151, imm4, rj, 0x15))
#define X64_RCLI_H(rj, imm4)  EMIT(type_2RI4(0x151, imm4, rj, 0x19))
#define X64_SLLI_W(rj, imm5)  EMIT(type_2RI5(0xa9, imm5, rj, 0x2))
#define X64_SRLI_W(rj, imm5)  EMIT(type_2RI5(0xa9, imm5, rj, 0x6))
#define X64_SRAI_W(rj, imm5)  EMIT(type_2RI5(0xa9, imm5, rj, 0xa))
#define X64_ROTRI_W(rj, imm5) EMIT(type_2RI5(0xa9, imm5, rj, 0xe))
#define X64_RCRI_W(rj, imm5)  EMIT(type_2RI5(0xa9, imm5, rj, 0x12))
#define X64_ROTLI_W(rj, imm5) EMIT(type_2RI5(0xa9, imm5, rj, 0x16))
#define X64_RCLI_W(rj, imm5)  EMIT(type_2RI5(0xa9, imm5, rj, 0x1a))
#define X64_SLLI_D(rj, imm6)  EMIT(type_2RI6(0x55, imm6, rj, 0x3))
#define X64_SRLI_D(rj, imm6)  EMIT(type_2RI6(0x55, imm6, rj, 0x7))
#define X64_SRAI_D(rj, imm6)  EMIT(type_2RI6(0x55, imm6, rj, 0xb))
#define X64_ROTRI_D(rj, imm6) EMIT(type_2RI6(0x55, imm6, rj, 0xf))
#define X64_RCRI_D(rj, imm6)  EMIT(type_2RI6(0x55, imm6, rj, 0x13))
#define X64_ROTLI_D(rj, imm6) EMIT(type_2RI6(0x55, imm6, rj, 0x17))
#define X64_RCLI_D(rj, imm6)  EMIT(type_2RI6(0x55, imm6, rj, 0x1b))

// Warning, these are LBT addons that uses LBT4.eflags internally
#define ADC_B(rd, rj, rk)    EMIT(type_3R(0x60, rk, rj, rd))
#define ADC_H(rd, rj, rk)    EMIT(type_3R(0x61, rk, rj, rd))
#define ADC_W(rd, rj, rk)    EMIT(type_3R(0x62, rk, rj, rd))
#define ADC_D(rd, rj, rk)    EMIT(type_3R(0x63, rk, rj, rd))
#define SBC_B(rd, rj, rk)    EMIT(type_3R(0x64, rk, rj, rd))
#define SBC_H(rd, rj, rk)    EMIT(type_3R(0x65, rk, rj, rd))
#define SBC_W(rd, rj, rk)    EMIT(type_3R(0x66, rk, rj, rd))
#define SBC_D(rd, rj, rk)    EMIT(type_3R(0x67, rk, rj, rd))
#define RCR_B(rd, rj, rk)    EMIT(type_3R(0x68, rk, rj, rd))
#define RCR_H(rd, rj, rk)    EMIT(type_3R(0x69, rk, rj, rd))
#define RCR_W(rd, rj, rk)    EMIT(type_3R(0x6a, rk, rj, rd))
#define RCR_D(rd, rj, rk)    EMIT(type_3R(0x6b, rk, rj, rd))
#define RCRI_B(rd, rj, imm3) EMIT(type_2RI3(0b0000000001010000001, imm3, rj, rd))
#define RCRI_H(rd, rj, imm4) EMIT(type_2RI4(0b000000000101000001, imm4, rj, rd))
#define RCRI_W(rd, rj, imm5) EMIT(type_2RI5(0b00000000010100001, imm5, rj, rd))
#define RCRI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001010001, imm6, rj, rd))

// Additional LBT inst
#define ROTR_B(rd, rj, rk)    EMIT(type_3R(0b00000000000110100, rk, rj, rd))
#define ROTR_H(rd, rj, rk)    EMIT(type_3R(0b00000000000110101, rk, rj, rd))
#define ROTRI_B(rd, rj, imm3) EMIT(type_2RI3(0b0000000001001100001, imm3, rj, rd))
#define ROTRI_H(rd, rj, imm4) EMIT(type_2RI4(0b000000000100110001, imm4, rj, rd))

////////////////////////////////////////////////////////////////////////////////

// MOV64x/MOV32w is quite complex, so use a function for this
#define MOV64x(A, B) la64_move64(dyn, ninst, A, B)
#define MOV32w(A, B) la64_move32(dyn, ninst, A, B, 1)
#define MOV64xw(A, B) \
    if (rex.w) {      \
        MOV64x(A, B); \
    } else {          \
        MOV32w(A, B); \
    }
#define MOV64z(A, B)    \
    if (rex.is32bits) { \
        MOV32w(A, B);   \
    } else {            \
        MOV64x(A, B);   \
    }
#define MOV64y(A, B)                    \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            MOV32w(A, B);               \
        } else {                        \
            MOV64x(A, B);               \
        }                               \
    } while (0)

// rd[63:0] = rj[63:0] (pseudo instruction)
#define MV(rd, rj) ADDI_D(rd, rj, 0)
// rd = rj (pseudo instruction)
#define MVxw(rd, rj)         \
    do {                     \
        if (rex.w) {         \
            MV(rd, rj);      \
        } else {             \
            ZEROUP2(rd, rj); \
        }                    \
    } while (0)

// rd = rj (pseudo instruction)
#define MVz(rd, rj)          \
    do {                     \
        if (rex.is32bits) {  \
            ZEROUP2(rd, rj); \
        } else {             \
            MV(rd, rj);      \
        }                    \
    } while (0)

#define ADDIxw(rd, rj, imm12)      \
    do {                           \
        if (rex.w)                 \
            ADDI_D(rd, rj, imm12); \
        else                       \
            ADDI_W(rd, rj, imm12); \
    } while (0)

#define ADDIz(rd, rj, imm12)       \
    do {                           \
        if (rex.is32bits) {        \
            ADDI_W(rd, rj, imm12); \
            ZEROUP(rd);            \
        } else                     \
            ADDI_D(rd, rj, imm12); \
    } while (0)

#define ADDIy(rd, rj, imm12)            \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADDI_W(rd, rj, imm12);      \
            ZEROUP(rd);                 \
        } else                          \
            ADDI_D(rd, rj, imm12);      \
    } while (0)

#define ADDxw(rd, rj, rk)      \
    do {                       \
        if (rex.w)             \
            ADD_D(rd, rj, rk); \
        else                   \
            ADD_W(rd, rj, rk); \
    } while (0)

#define ADDz(rd, rj, rk)       \
    do {                       \
        if (rex.is32bits) {    \
            ADD_W(rd, rj, rk); \
            ZEROUP(rd);        \
        } else                 \
            ADD_D(rd, rj, rk); \
    } while (0)

#define ADDy(rd, rj, rk)                \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADD_W(rd, rj, rk);          \
            ZEROUP(rd);                 \
        } else                          \
            ADD_D(rd, rj, rk);          \
    } while (0)

#define ADDxREGy(rd, rj, rk, s1)        \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADDI_W(s1, rk, 0);          \
            ADD_D(rd, rj, s1);          \
        } else                          \
            ADD_D(rd, rj, rk);          \
    } while (0)

#define LDxw(rd, rj, imm12)       \
    do {                          \
        if (rex.w)                \
            LD_D(rd, rj, imm12);  \
        else                      \
            LD_WU(rd, rj, imm12); \
    } while (0)

#define LDXxw(rd, rj, rk)       \
    do {                        \
        if (rex.w)              \
            LDX_D(rd, rj, rk);  \
        else                    \
            LDX_WU(rd, rj, rk); \
    } while (0)

#define LDz(rd, rj, imm12)        \
    do {                          \
        if (rex.is32bits)         \
            LD_WU(rd, rj, imm12); \
        else                      \
            LD_D(rd, rj, imm12);  \
    } while (0)

#define FLDxw(rd, rj, imm12)      \
    do {                          \
        if (rex.w)                \
            FLD_D(rd, rj, imm12); \
        else                      \
            FLD_S(rd, rj, imm12); \
    } while (0)


#define SDxw(rd, rj, imm12)      \
    do {                         \
        if (rex.w)               \
            ST_D(rd, rj, imm12); \
        else                     \
            ST_W(rd, rj, imm12); \
    } while (0)

#define SDXxw(rd, rj, rk)      \
    do {                       \
        if (rex.w)             \
            STX_D(rd, rj, rk); \
        else                   \
            STX_W(rd, rj, rk); \
    } while (0)

#define SDz(rd, rj, imm12)       \
    do {                         \
        if (rex.is32bits)        \
            ST_W(rd, rj, imm12); \
        else                     \
            ST_D(rd, rj, imm12); \
    } while (0)

#define NEG_D(rd, rj) SUB_D(rd, xZR, rj)

#define SUBxw(rd, rj, rk)      \
    do {                       \
        if (rex.w)             \
            SUB_D(rd, rj, rk); \
        else                   \
            SUB_W(rd, rj, rk); \
    } while (0)

#define NEGxw(rd, rj) SUBxw(rd, xZR, rj)

#define SUBz(rd, rj, rk)       \
    do {                       \
        if (rex.is32bits) {    \
            SUB_W(rd, rj, rk); \
            ZEROUP(rd);        \
        } else                 \
            SUB_D(rd, rj, rk); \
    } while (0)

// PUSH / POP reg[0:63]
#define PUSH1(reg)              \
    do {                        \
        ST_D(reg, xRSP, -8);    \
        ADDI_D(xRSP, xRSP, -8); \
    } while (0);
#define POP1(reg)                               \
    do {                                        \
        LD_D(reg, xRSP, 0);                     \
        if (reg != xRSP) ADDI_D(xRSP, xRSP, 8); \
    } while (0);

// PUSH / POP reg[0:31]
#define PUSH1_32(reg)           \
    do {                        \
        ST_W(reg, xRSP, -4);    \
        ADDI_D(xRSP, xRSP, -4); \
    } while (0);
#define POP1_32(reg)                            \
    do {                                        \
        LD_WU(reg, xRSP, 0);                    \
        if (reg != xRSP) ADDI_D(xRSP, xRSP, 4); \
    } while (0);

#define PUSH1_16(reg)           \
    do {                        \
        ST_H(reg, xRSP, -2);    \
        ADDI_D(xRSP, xRSP, -2); \
    } while (0)

#define POP1_16(reg)                            \
    do {                                        \
        LD_HU(reg, xRSP, 0);                    \
        if (reg != xRSP) ADDI_D(xRSP, xRSP, 2); \
    } while (0)

// POP reg
#define POP1z(reg)          \
    do {                    \
        if (rex.is32bits) { \
            POP1_32(reg);   \
        } else {            \
            POP1(reg);      \
        }                   \
    } while (0)

// PUSH reg
#define PUSH1z(reg)         \
    do {                    \
        if (rex.is32bits) { \
            PUSH1_32(reg);  \
        } else {            \
            PUSH1(reg);     \
        }                   \
    } while (0)

#define VAND_Vxy(vd, vj, vk)     \
    do {                         \
        if (vex.l) {             \
            XVAND_V(vd, vj, vk); \
        } else {                 \
            VAND_V(vd, vj, vk);  \
        }                        \
    } while (0)

#define VANDN_Vxy(vd, vj, vk)     \
    do {                          \
        if (vex.l) {              \
            XVANDN_V(vd, vj, vk); \
        } else {                  \
            VANDN_V(vd, vj, vk);  \
        }                         \
    } while (0)

#define VOR_Vxy(vd, vj, vk)     \
    do {                        \
        if (vex.l) {            \
            XVOR_V(vd, vj, vk); \
        } else {                \
            VOR_V(vd, vj, vk);  \
        }                       \
    } while (0)

#define VXOR_Vxy(vd, vj, vk)     \
    do {                         \
        if (vex.l) {             \
            XVXOR_V(vd, vj, vk); \
        } else {                 \
            VXOR_V(vd, vj, vk);  \
        }                        \
    } while (0)

#define VBSLL_Vxy(vd, vj, imm)     \
    do {                           \
        if (vex.l) {               \
            XVBSLL_V(vd, vj, imm); \
        } else {                   \
            VBSLL_V(vd, vj, imm);  \
        }                          \
    } while (0)

#define VBSRL_Vxy(vd, vj, imm)     \
    do {                           \
        if (vex.l) {               \
            XVBSRL_V(vd, vj, imm); \
        } else {                   \
            VBSRL_V(vd, vj, imm);  \
        }                          \
    } while (0)

#define VSLLIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVSLLI_##width(vd, vj, imm); \
        } else {                         \
            VSLLI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VSRLIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVSRLI_##width(vd, vj, imm); \
        } else {                         \
            VSRLI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VSRAIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVSRAI_##width(vd, vj, imm); \
        } else {                         \
            VSRAI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VSLLxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSLL_##width(vd, vj, vk); \
        } else {                       \
            VSLL_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSRLxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSRL_##width(vd, vj, vk); \
        } else {                       \
            VSRL_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSRAxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSRA_##width(vd, vj, vk); \
        } else {                       \
            VSRA_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSLEIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVSLEI_##width(vd, vj, imm); \
        } else {                         \
            VSLEI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VSLExy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSLE_##width(vd, vj, vk); \
        } else {                       \
            VSLE_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VLDIxy(vd, imm)     \
    do {                    \
        if (vex.l) {        \
            XVLDI(vd, imm); \
        } else {            \
            VLDI(vd, imm);  \
        }                   \
    } while (0)

#define VREPLVE0xy(width, vd, vj)        \
    do {                                 \
        if (vex.l) {                     \
            XVREPLVE0_##width(vd, vj);   \
        } else {                         \
            VREPLVEI_##width(vd, vj, 0); \
        }                                \
    } while (0)

#define VMAXIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVMAXI_##width(vd, vj, imm); \
        } else {                         \
            VMAXI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VMINIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVMINI_##width(vd, vj, imm); \
        } else {                         \
            VMINI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VADDxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVADD_##width(vd, vj, vk); \
        } else {                       \
            VADD_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSUBxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSUB_##width(vd, vj, vk); \
        } else {                       \
            VSUB_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSADDxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVSADD_##width(vd, vj, vk); \
        } else {                        \
            VSADD_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VSSUBxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVSSUB_##width(vd, vj, vk); \
        } else {                        \
            VSSUB_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VMULxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVMUL_##width(vd, vj, vk); \
        } else {                       \
            VMUL_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VMUHxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVMUH_##width(vd, vj, vk); \
        } else {                       \
            VMUH_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VMULWEVxy(width, vd, vj, vk)      \
    do {                                  \
        if (vex.l) {                      \
            XVMULWEV_##width(vd, vj, vk); \
        } else {                          \
            VMULWEV_##width(vd, vj, vk);  \
        }                                 \
    } while (0)

#define VMULWODxy(width, vd, vj, vk)      \
    do {                                  \
        if (vex.l) {                      \
            XVMULWOD_##width(vd, vj, vk); \
        } else {                          \
            VMULWOD_##width(vd, vj, vk);  \
        }                                 \
    } while (0)

#define VMAXxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVMAX_##width(vd, vj, vk); \
        } else {                       \
            VMAX_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VMINxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVMIN_##width(vd, vj, vk); \
        } else {                       \
            VMIN_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSIGNCOVxy(width, vd, vj, vk)      \
    do {                                   \
        if (vex.l) {                       \
            XVSIGNCOV_##width(vd, vj, vk); \
        } else {                           \
            VSIGNCOV_##width(vd, vj, vk);  \
        }                                  \
    } while (0)

#define VAVGxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVAVG_##width(vd, vj, vk); \
        } else {                       \
            VAVG_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VAVGRxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVAVGR_##width(vd, vj, vk); \
        } else {                        \
            VAVGR_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VABSDxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVABSD_##width(vd, vj, vk); \
        } else {                        \
            VABSD_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VHADDWxy(width, vd, vj, vk)      \
    do {                                 \
        if (vex.l) {                     \
            XVHADDW_##width(vd, vj, vk); \
        } else {                         \
            VHADDW_##width(vd, vj, vk);  \
        }                                \
    } while (0)

#define VMADDxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVMADD_##width(vd, vj, vk); \
        } else {                        \
            VMADD_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VPICKEVxy(width, vd, vj, vk)      \
    do {                                  \
        if (vex.l) {                      \
            XVPICKEV_##width(vd, vj, vk); \
        } else {                          \
            VPICKEV_##width(vd, vj, vk);  \
        }                                 \
    } while (0)

#define VPICKODxy(width, vd, vj, vk)      \
    do {                                  \
        if (vex.l) {                      \
            XVPICKOD_##width(vd, vj, vk); \
        } else {                          \
            VPICKOD_##width(vd, vj, vk);  \
        }                                 \
    } while (0)

#define VPACKEVxy(width, vd, vj, vk)      \
    do {                                  \
        if (vex.l) {                      \
            XVPACKEV_##width(vd, vj, vk); \
        } else {                          \
            VPACKEV_##width(vd, vj, vk);  \
        }                                 \
    } while (0)

#define VPACKODxy(width, vd, vj, vk)      \
    do {                                  \
        if (vex.l) {                      \
            XVPACKOD_##width(vd, vj, vk); \
        } else {                          \
            VPACKOD_##width(vd, vj, vk);  \
        }                                 \
    } while (0)

#define VILVLxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVILVL_##width(vd, vj, vk); \
        } else {                        \
            VILVL_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VILVHxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVILVH_##width(vd, vj, vk); \
        } else {                        \
            VILVH_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VSATxy(width, vd, vj, imm)      \
    do {                                \
        if (vex.l) {                    \
            XVSAT_##width(vd, vj, imm); \
        } else {                        \
            VSAT_##width(vd, vj, imm);  \
        }                               \
    } while (0)

#define VSLTIxy(width, vd, vj, imm)      \
    do {                                 \
        if (vex.l) {                     \
            XVSLTI_##width(vd, vj, imm); \
        } else {                         \
            VSLTI_##width(vd, vj, imm);  \
        }                                \
    } while (0)

#define VBITSEL_Vxy(vd, vj, vk, va)     \
    do {                                \
        if (vex.l) {                    \
            XVBITSEL_V(vd, vj, vk, va); \
        } else {                        \
            VBITSEL_V(vd, vj, vk, va);  \
        }                               \
    } while (0)

#define VSHUF_Bxy(vd, vj, vk, va)     \
    do {                              \
        if (vex.l) {                  \
            XVSHUF_B(vd, vj, vk, va); \
        } else {                      \
            VSHUF_B(vd, vj, vk, va);  \
        }                             \
    } while (0)

#define VSHUFxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVSHUF_##width(vd, vj, vk); \
        } else {                        \
            VSHUF_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VSHUF4Ixy(width, vd, vj, imm)      \
    do {                                   \
        if (vex.l) {                       \
            XVSHUF4I_##width(vd, vj, imm); \
        } else {                           \
            VSHUF4I_##width(vd, vj, imm);  \
        }                                  \
    } while (0)

#define VEXTRINSxy(width, vd, vj, imm)      \
    do {                                    \
        if (vex.l) {                        \
            XVEXTRINS_##width(vd, vj, imm); \
        } else {                            \
            VEXTRINS_##width(vd, vj, imm);  \
        }                                   \
    } while (0)

#define VANDIxy(vd, vj, imm)       \
    do {                           \
        if (vex.l) {               \
            XVANDI_B(vd, vj, imm); \
        } else {                   \
            VANDI_B(vd, vj, imm);  \
        }                          \
    } while (0)

#define VLDREPLxy(width, vd, rj, imm)      \
    do {                                   \
        if (vex.l) {                       \
            XVLDREPL_##width(vd, rj, imm); \
        } else {                           \
            VLDREPL_##width(vd, rj, imm);  \
        }                                  \
    } while (0)

#define XVPICKVE2GRxw(rd, xj, imm)       \
    do {                                 \
        if (rex.w)                       \
            XVPICKVE2GR_D(rd, xj, imm);  \
        else                             \
            XVPICKVE2GR_WU(rd, xj, imm); \
    } while (0)

#define XVINSGR2VRxw(xd, rj, imm)      \
    do {                               \
        if (rex.w)                     \
            XVINSGR2VR_D(xd, rj, imm); \
        else                           \
            XVINSGR2VR_W(xd, rj, imm); \
    } while (0)

#define VPICKVE2GRxw(rd, xj, imm)       \
    do {                                \
        if (rex.w)                      \
            VPICKVE2GR_D(rd, xj, imm);  \
        else                            \
            VPICKVE2GR_WU(rd, xj, imm); \
    } while (0)

#define VINSGR2VRxw(xd, rj, imm)      \
    do {                              \
        if (rex.w)                    \
            VINSGR2VR_D(xd, rj, imm); \
        else                          \
            VINSGR2VR_W(xd, rj, imm); \
    } while (0)

#define XVINSVE0xw(xd, xj, imm)      \
    do {                             \
        if (rex.w)                   \
            XVINSVE0_D(xd, xj, imm); \
        else                         \
            XVINSVE0_W(xd, xj, imm); \
    } while (0)

#define VEXTRINSxw(xd, xj, imm)      \
    do {                             \
        if (rex.w)                   \
            VEXTRINS_D(xd, xj, imm); \
        else                         \
            VEXTRINS_W(xd, xj, imm); \
    } while (0)

#define VFMADDxy(width, vd, vj, vk, va)      \
    do {                                     \
        if (vex.l) {                         \
            XVFMADD_##width(vd, vj, vk, va); \
        } else {                             \
            VFMADD_##width(vd, vj, vk, va);  \
        }                                    \
    } while (0)

#define VFMSUBxy(width, vd, vj, vk, va)      \
    do {                                     \
        if (vex.l) {                         \
            XVFMSUB_##width(vd, vj, vk, va); \
        } else {                             \
            VFMSUB_##width(vd, vj, vk, va);  \
        }                                    \
    } while (0)

#define VFNMADDxy(width, vd, vj, vk, va)      \
    do {                                      \
        if (vex.l) {                          \
            XVFNMADD_##width(vd, vj, vk, va); \
        } else {                              \
            VFNMADD_##width(vd, vj, vk, va);  \
        }                                     \
    } while (0)

#define VFNMSUBxy(width, vd, vj, vk, va)      \
    do {                                      \
        if (vex.l) {                          \
            XVFNMSUB_##width(vd, vj, vk, va); \
        } else {                              \
            VFNMSUB_##width(vd, vj, vk, va);  \
        }                                     \
    } while (0)

#define VFMADDxyxw(vd, vj, vk, va)       \
    do {                                 \
        if (rex.w) {                     \
            VFMADDxy(D, vd, vj, vk, va); \
        } else {                         \
            VFMADDxy(S, vd, vj, vk, va); \
        }                                \
    } while (0)

#define VFMSUBxyxw(vd, vj, vk, va)       \
    do {                                 \
        if (rex.w) {                     \
            VFMSUBxy(D, vd, vj, vk, va); \
        } else {                         \
            VFMSUBxy(S, vd, vj, vk, va); \
        }                                \
    } while (0)

#define VFNMADDxyxw(vd, vj, vk, va)       \
    do {                                  \
        if (rex.w) {                      \
            VFNMADDxy(D, vd, vj, vk, va); \
        } else {                          \
            VFNMADDxy(S, vd, vj, vk, va); \
        }                                 \
    } while (0)

#define VFNMSUBxyxw(vd, vj, vk, va)       \
    do {                                  \
        if (rex.w) {                      \
            VFNMSUBxy(D, vd, vj, vk, va); \
        } else {                          \
            VFNMSUBxy(S, vd, vj, vk, va); \
        }                                 \
    } while (0)

#define VPICKEVxyxw(vd, vj, vk)       \
    do {                              \
        if (rex.w) {                  \
            VPICKEVxy(D, vd, vj, vk); \
        } else {                      \
            VPICKEVxy(W, vd, vj, vk); \
        }                             \
    } while (0)

#define VPICKODxyxw(vd, vj, vk)       \
    do {                              \
        if (rex.w) {                  \
            VPICKODxy(D, vd, vj, vk); \
        } else {                      \
            VPICKODxy(W, vd, vj, vk); \
        }                             \
    } while (0)

#define VILVLxyxw(vd, vj, vk)       \
    do {                            \
        if (rex.w) {                \
            VILVLxy(D, vd, vj, vk); \
        } else {                    \
            VILVLxy(W, vd, vj, vk); \
        }                           \
    } while (0)

#define VILVHxyxw(vd, vj, vk)       \
    do {                            \
        if (rex.w) {                \
            VILVHxy(D, vd, vj, vk); \
        } else {                    \
            VILVHxy(W, vd, vj, vk); \
        }                           \
    } while (0)

#define FMADDxw(fd, fj, fk, fa)      \
    do {                             \
        if (rex.w) {                 \
            FMADD_D(fd, fj, fk, fa); \
        } else {                     \
            FMADD_S(fd, fj, fk, fa); \
        }                            \
    } while (0)

#define FMSUBxw(fd, fj, fk, fa)      \
    do {                             \
        if (rex.w) {                 \
            FMSUB_D(fd, fj, fk, fa); \
        } else {                     \
            FMSUB_S(fd, fj, fk, fa); \
        }                            \
    } while (0)

#define FNMADDxw(fd, fj, fk, fa)      \
    do {                              \
        if (rex.w) {                  \
            FNMADD_D(fd, fj, fk, fa); \
        } else {                      \
            FNMADD_S(fd, fj, fk, fa); \
        }                             \
    } while (0)

#define FNMSUBxw(fd, fj, fk, fa)      \
    do {                              \
        if (rex.w) {                  \
            FNMSUB_D(fd, fj, fk, fa); \
        } else {                      \
            FNMSUB_S(fd, fj, fk, fa); \
        }                             \
    } while (0)


#define VFCMPxy(width, vd, vj, vk, cond)      \
    do {                                      \
        if (vex.l) {                          \
            XVFCMP_##width(vd, vj, vk, cond); \
        } else {                              \
            VFCMP_##width(vd, vj, vk, cond);  \
        }                                     \
    } while (0)

#define VFADDxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVFADD_##width(vd, vj, vk); \
        } else {                        \
            VFADD_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VFSUBxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVFSUB_##width(vd, vj, vk); \
        } else {                        \
            VFSUB_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VFMULxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVFMUL_##width(vd, vj, vk); \
        } else {                        \
            VFMUL_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VFDIVxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVFDIV_##width(vd, vj, vk); \
        } else {                        \
            VFDIV_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VFRECIPxy(width, vd, vj)      \
    do {                              \
        if (vex.l) {                  \
            XVFRECIP_##width(vd, vj); \
        } else {                      \
            VFRECIP_##width(vd, vj);  \
        }                             \
    } while (0)

#define VFRECIPExy(width, vd, vj)      \
    do {                               \
        if (vex.l) {                   \
            XVFRECIPE_##width(vd, vj); \
        } else {                       \
            VFRECIPE_##width(vd, vj);  \
        }                              \
    } while (0)

#define VFRSQRTxy(width, vd, vj)      \
    do {                              \
        if (vex.l) {                  \
            XVFRSQRT_##width(vd, vj); \
        } else {                      \
            VFRSQRT_##width(vd, vj);  \
        }                             \
    } while (0)

#define VFRSQRTExy(width, vd, vj)      \
    do {                               \
        if (vex.l) {                   \
            XVFRSQRTE_##width(vd, vj); \
        } else {                       \
            VFRSQRTE_##width(vd, vj);  \
        }                              \
    } while (0)

#define VFSQRTxy(width, vd, vj)      \
    do {                             \
        if (vex.l) {                 \
            XVFSQRT_##width(vd, vj); \
        } else {                     \
            VFSQRT_##width(vd, vj);  \
        }                            \
    } while (0)

#define VFMAXxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVFMAX_##width(vd, vj, vk); \
        } else {                        \
            VFMAX_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VFMINxy(width, vd, vj, vk)      \
    do {                                \
        if (vex.l) {                    \
            XVFMIN_##width(vd, vj, vk); \
        } else {                        \
            VFMIN_##width(vd, vj, vk);  \
        }                               \
    } while (0)

#define VREPLVEIxy(width, vd, vj, imm)         \
    do {                                       \
        if (vex.l) {                           \
            if (imm > 0) {                     \
                ADDI_D(x5, xZR, imm);          \
                XVREPLVE_##width(vd, vj, x5);  \
            } else {                           \
                XVREPLVE_##width(vd, vj, xZR); \
            }                                  \
        } else {                               \
            VREPLVEI_##width(vd, vj, imm);     \
        }                                      \
    } while (0)

#define VSEQxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSEQ_##width(vd, vj, vk); \
        } else {                       \
            VSEQ_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSLTxy(width, vd, vj, vk)      \
    do {                               \
        if (vex.l) {                   \
            XVSLT_##width(vd, vj, vk); \
        } else {                       \
            VSLT_##width(vd, vj, vk);  \
        }                              \
    } while (0)

#define VSETEQZ_Vxy(fcc, vd)     \
    do {                         \
        if (vex.l) {             \
            XVSETEQZ_V(fcc, vd); \
        } else {                 \
            VSETEQZ_V(fcc, vd);  \
        }                        \
    } while (0)

#define VSETNEZ_Vxy(fcc, vd)     \
    do {                         \
        if (vex.l) {             \
            XVSETNEZ_V(fcc, vd); \
        } else {                 \
            VSETNEZ_V(fcc, vd);  \
        }                        \
    } while (0)

#define VBITCLRIxy(width, vd, vj, imm)      \
    do {                                    \
        if (vex.l) {                        \
            XVBITCLRI_##width(vd, vj, imm); \
        } else {                            \
            VBITCLRI_##width(vd, vj, imm);  \
        }                                   \
    } while (0)

#define VMSKLTZxy(width, vd, vj)      \
    do {                              \
        if (vex.l) {                  \
            XVMSKLTZ_##width(vd, vj); \
        } else {                      \
            VMSKLTZ_##width(vd, vj);  \
        }                             \
    } while (0)

#endif //__LA64_EMITTER_H__
