#ifndef __RV64_EMITTER_H__
#define __RV64_EMITTER_H__

#include "rv64_mapping.h"

// RV64 Emitter

// replacement for F_OF internaly, using a reserved bit. Need to use F_OF2 internaly, never F_OF directly!
#define F_OF2 F_res3

// split a 32bits value in 20bits + 12bits, adjust the upper part is 12bits is negative
#define SPLIT20(A) (((A) + 0x800) >> 12)
#define SPLIT12(A) ((A) & 0xfff)

// MOV64x/MOV32w is quite complex, so use a function for this
#define MOV64x(A, B) rv64_move64(dyn, ninst, A, B)
#define MOV32w(A, B) rv64_move32(dyn, ninst, A, B, 1)
#define MOV64xw(A, B)     \
    do {                  \
        if (rex.w) {      \
            MOV64x(A, B); \
        } else {          \
            MOV32w(A, B); \
        }                 \
    } while (0)
#define MOV64y(A, B)                    \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            MOV32w(A, B);               \
        } else {                        \
            MOV64x(A, B);               \
        }                               \
    } while (0)
#define MOV64z(A, B)        \
    do {                    \
        if (rex.is32bits) { \
            MOV32w(A, B);   \
        } else {            \
            MOV64x(A, B);   \
        }                   \
    } while (0)

// ZERO the upper part, compatible to zba, xtheadbb, and rv64gc
#define ZEXTW2(rd, rs1)              \
    do {                             \
        if (cpuext.zba) {            \
            ZEXTW(rd, rs1);          \
        } else if (cpuext.xtheadbb) {\
            TH_EXTU(rd, rs1, 31, 0); \
        } else {                     \
            SLLI(rd, rs1, 32);       \
            SRLI(rd, rd, 32);        \
        }                            \
    } while (0)
#define ZEROUP(r) ZEXTW2(r, r)

#define R_type(funct7, rs2, rs1, funct3, rd, opcode) ((funct7) << 25 | (rs2) << 20 | (rs1) << 15 | (funct3) << 12 | (rd) << 7 | (opcode))
#define I_type(imm12, rs1, funct3, rd, opcode)       ((imm12) << 20 | (rs1) << 15 | (funct3) << 12 | (rd) << 7 | (opcode))
#define S_type(imm12, rs2, rs1, funct3, opcode)      (((imm12) >> 5) << 25 | (rs2) << 20 | (rs1) << 15 | (funct3) << 12 | ((imm12) & 31) << 7 | (opcode))
#define B_type(imm13, rs2, rs1, funct3, opcode)      ((((imm13) >> 12) & 1) << 31 | (((imm13) >> 5) & 63) << 25 | (rs2) << 20 | (rs1) << 15 | (funct3) << 12 | (((imm13) >> 1) & 15) << 8 | (((imm13) >> 11) & 1) << 7 | (opcode))
#define U_type(imm32, rd, opcode)                    (((imm32) >> 12) << 12 | (rd) << 7 | (opcode))
#define J_type(imm21, rd, opcode)                    ((((imm21) >> 20) & 1) << 31 | (((imm21) >> 1) & 0b1111111111) << 21 | (((imm21) >> 11) & 1) << 20 | (((imm21) >> 12) & 0b11111111) << 12 | (rd) << 7 | (opcode))

// RV32I
// put imm20 in the [31:12] bits of rd, zero [11:0] and sign extend bits31
#define LUI(rd, imm20) EMIT(U_type((imm20) << 12, rd, 0b0110111))

// put PC+imm20 in rd
#define AUIPC(rd, imm20) EMIT(U_type((imm20) << 12, rd, 0b0010111))

#define JAL_gen(rd, imm21) J_type(imm21, rd, 0b1101111)
// Unconditional branch, no return address set
#define B(imm21)               EMIT(JAL_gen(xZR, imm21))
#define B__(reg1, reg2, imm21) B(imm21)
// Unconditional branch, return set to xRA
#define JAL(imm21) EMIT(JAL_gen(xRA, imm21))
// Unconditional branch, without link
#define J(imm21) EMIT(JAL_gen(xZR, imm21))

#define JALR_gen(rd, rs1, imm12) I_type(imm12, rs1, 0b000, rd, 0b1100111)
// Unconditionnal branch to r, no return address set
#define BR(r) EMIT(JALR_gen(xZR, r, 0))
// Unconditionnal branch to r+i12, no return address set
#define BR_I12(r, imm12) EMIT(JALR_gen(xZR, r, (imm12) & 0b111111111111))
// Unconditionnal branch to r, return address set to xRA
#define JALR(rd, rs) EMIT(JALR_gen(rd, rs, 0))
// Unconditionnal branch to r+i12, return address set to xRA
#define JALR_I12(rd, rs, imm12) EMIT(JALR_gen(rd, rs, (imm12) & 0b111111111111))

// rd = rs1 + imm12
#define ADDI(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b000, rd, 0b0010011))
// rd = rs1 - imm12 (pseudo instruction)
#define SUBI(rd, rs1, imm12) EMIT(I_type((-(imm12)) & 0b111111111111, rs1, 0b000, rd, 0b0010011))
// rd = (rs1<imm12)?1:0
#define SLTI(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b010, rd, 0b0010011))
// rd = (rs1<imm12)?1:0 unsigned
#define SLTIU(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b011, rd, 0b0010011))
// rd = rs1 ^ imm12
#define XORI(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b100, rd, 0b0010011))
// rd = rs1 | imm12
#define ORI(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b110, rd, 0b0010011))
// rd = rs1 & imm12
#define ANDI(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b111, rd, 0b0010011))

// rd = imm12
#define MOV_U12(rd, imm12) ADDI(rd, xZR, imm12)
// nop
#define NOP() ADDI(xZR, xZR, 0)

#define UDF() EMIT(0xc0001073)

// rd = rs1 + rs2
#define ADD(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b000, rd, 0b0110011))
// rd = rs1 + rs2
#define ADDW(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b000, rd, 0b0111011))
// rd = rs1 + rs2
#define ADDxw(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b000, rd, rex.w ? 0b0110011 : 0b0111011))
// rd = rs1 + rs2
#define ADDz(rd, rs1, rs2)      \
    do {                        \
        if (!rex.is32bits) {    \
            ADD(rd, rs1, rs2);  \
        } else {                \
            ADDW(rd, rs1, rs2); \
            ZEROUP(rd);         \
        }                       \
    } while (0)
#define ADDy(rd, rs1, rs2)              \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADDW(rd, rs1, rs2);         \
            ZEROUP(rd);                 \
        } else {                        \
            ADD(rd, rs1, rs2);          \
        }                               \
    } while (0)
#define ADDxREGy(rd, rs1, rs2, s1)      \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADDIW(s1, rs2, 0);          \
            ADD(rd, rs1, s1);           \
        } else {                        \
            ADD(rd, rs1, rs2);          \
        }                               \
    } while (0)
// rd = rs1 - rs2
#define SUB(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b000, rd, 0b0110011))
// rd = rs1 - rs2
#define SUBW(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b000, rd, 0b0111011))
// rd = rs1 - rs2
#define SUBxw(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b000, rd, rex.w ? 0b0110011 : 0b0111011))
// rd = rs1 - rs2
#define SUBz(rd, rs1, rs2)     \
    do {                       \
        if (!rex.is32bits) {   \
            SUB(rd, rs1, rs2); \
        } else {               \
            SUB(rd, rs1, rs2); \
            ZEROUP(rd);        \
        }                      \
    } while (0)
// rd = rs1<<rs2
#define SLL(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b001, rd, 0b0110011))
// rd = (rs1<rs2)?1:0
#define SLT(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b010, rd, 0b0110011))
// rd = (rs1<rs2)?1:0 Unsigned
#define SLTU(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b011, rd, 0b0110011))
// rd = rs1 ^ rs2
#define XOR(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b100, rd, 0b0110011))
// rd = rs1 ^ rs2
#define XORxw(rd, rs1, rs2)     \
    do {                        \
        XOR(rd, rs1, rs2);      \
        if (!rex.w) ZEROUP(rd); \
    } while (0)
// rd = rs1>>rs2 logical
#define SRL(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b101, rd, 0b0110011))
// rd = rs1>>rs2 arithmetic
#define SRA(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b101, rd, 0b0110011))
// rd = rs1 | rs2
#define OR(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b110, rd, 0b0110011))
// rd = rs1 & rs2
#define AND(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b111, rd, 0b0110011))

// rd = rs1 (pseudo instruction)
#define MV(rd, rs1) ADDI(rd, rs1, 0)
// rd = rs1 (pseudo instruction)
#define MVxw(rd, rs1)            \
    do {                         \
        if (rex.w) {             \
            MV(rd, rs1);         \
        } else {                 \
            ZEXTW2(rd, rs1);     \
        }                        \
    } while (0)
// rd = rs1 (pseudo instruction)
#define MVz(rd, rs1)             \
    do {                         \
        if (rex.is32bits) {      \
            ZEXTW2(rd, rs1);     \
        } else {                 \
            MV(rd, rs1);         \
        }                        \
    } while (0)
// rd = !rs1
#define NOT(rd, rs1) XORI(rd, rs1, -1)
// rd = -rs1
#define NEG(rd, rs1) SUB(rd, xZR, rs1)
// rd = -rs1
#define NEGxw(rd, rs1) SUBxw(rd, xZR, rs1)
// rd = rs1 == 0
#define SEQZ(rd, rs1) SLTIU(rd, rs1, 1)
// rd = rs1 != 0
#define SNEZ(rd, rs1) SLTU(rd, xZR, rs1)


#define BEQ(rs1, rs2, imm13)  EMIT(B_type(imm13, rs2, rs1, 0b000, 0b1100011))
#define BNE(rs1, rs2, imm13)  EMIT(B_type(imm13, rs2, rs1, 0b001, 0b1100011))
#define BLT(rs1, rs2, imm13)  EMIT(B_type(imm13, rs2, rs1, 0b100, 0b1100011))
#define BGE(rs1, rs2, imm13)  EMIT(B_type(imm13, rs2, rs1, 0b101, 0b1100011))
#define BLTU(rs1, rs2, imm13) EMIT(B_type(imm13, rs2, rs1, 0b110, 0b1100011))
#define BGEU(rs1, rs2, imm13) EMIT(B_type(imm13, rs2, rs1, 0b111, 0b1100011))

#define BGT(rs1, rs2, imm13)  BLT(rs2, rs1, imm13)
#define BLE(rs1, rs2, imm13)  BGE(rs2, rs1, imm13)
#define BGTU(rs1, rs2, imm13) BLTU(rs2, rs1, imm13)
#define BLEU(rs1, rs2, imm13) BGEU(rs2, rs1, imm13)

#define SEQ(rd, rs1, rs2)        \
    do {                         \
        if (rs1 == xZR) {        \
            SEQZ(rd, rs2);       \
        } else if (rs2 == xZR) { \
            SEQZ(rd, rs1);       \
        } else {                 \
            XOR(rd, rs1, rs2);   \
            SEQZ(rd, rd);        \
        }                        \
    } while (0)

#define SNE(rd, rs1, rs2)        \
    do {                         \
        if (rs1 == xZR) {        \
            SNEZ(rd, rs2);       \
        } else if (rs2 == xZR) { \
            SNEZ(rd, rs1);       \
        } else {                 \
            XOR(rd, rs1, rs2);   \
            SNEZ(rd, rd);        \
        }                        \
    } while (0)

#define SGE(rd, rs1, rs2)      \
    do {                       \
        if (rs1 == xZR) {      \
            SLTI(rd, rs2, 1);  \
        } else {               \
            SLT(rd, rs1, rs2); \
            XORI(rd, rd, 1);   \
        }                      \
    } while (0)

#define SGEU(rd, rs1, rs2)       \
    do {                         \
        if (rs1 == xZR) {        \
            SEQZ(rd, rs2);       \
        } else if (rs2 == xZR) { \
            ADDI(rd, xZR, 1);    \
        } else {                 \
            SLTU(rd, rs1, rs2);  \
            XORI(rd, rd, 1);     \
        }                        \
    } while (0)

#define SGT(rd, rs1, rs2)  SLT(rd, rs2, rs1);
#define SLE(rd, rs1, rs2)  SGE(rd, rs2, rs1);
#define SGTU(rd, rs1, rs2) SLTU(rd, rs2, rs1);
#define SLEU(rd, rs1, rs2) SGEU(rd, rs2, rs1);

#define MVEQ(rd, rs1, rs2, rs3)                               \
    if (cpuext.xtheadcondmov && (rs2 == xZR || rs3 == xZR)) { \
        TH_MVEQZ(rd, rs1, ((rs2 == xZR) ? rs3 : rs2));        \
    } else {                                                  \
        BNE(rs2, rs3, 8);                                     \
        MV(rd, rs1);                                          \
    }
#define MVNE(rd, rs1, rs2, rs3)                               \
    if (cpuext.xtheadcondmov && (rs2 == xZR || rs3 == xZR)) { \
        TH_MVNEZ(rd, rs1, ((rs2 == xZR) ? rs3 : rs2));        \
    } else {                                                  \
        BEQ(rs2, rs3, 8);                                     \
        MV(rd, rs1);                                          \
    }
#define MVLT(rd, rs1, rs2, rs3) \
    BGE(rs2, rs3, 8);           \
    MV(rd, rs1);
#define MVGE(rd, rs1, rs2, rs3) \
    BLT(rs2, rs3, 8);           \
    MV(rd, rs1);
#define MVLTU(rd, rs1, rs2, rs3) \
    BGEU(rs2, rs3, 8);           \
    MV(rd, rs1);
#define MVGEU(rd, rs1, rs2, rs3) \
    BLTU(rs2, rs3, 8);           \
    MV(rd, rs1);
#define MVGT(rd, rs1, rs2, rs3) \
    BGE(rs3, rs2, 8);           \
    MV(rd, rs1);
#define MVLE(rd, rs1, rs2, rs3) \
    BLT(rs3, rs2, 8);           \
    MV(rd, rs1);
#define MVGTU(rd, rs1, rs2, rs3) \
    BGEU(rs3, rs2, 8);           \
    MV(rd, rs1);
#define MVLEU(rd, rs1, rs2, rs3) \
    BLTU(rs3, rs2, 8);           \
    MV(rd, rs1);

#define MVEQZ(rd, rs1, rs2) MVEQ(rd, rs1, rs2, xZR)
#define MVNEZ(rd, rs1, rs2) MVNE(rd, rs1, rs2, xZR)

#define BEQ_safe(rs1, rs2, imm)              \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BEQ(rs1, rs2, imm);                  \
        NOP();                               \
    } else {                                 \
        BNE(rs1, rs2, 8);                    \
        B(imm - 4);                          \
    }
#define BNE_safe(rs1, rs2, imm)              \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BNE(rs1, rs2, imm);                  \
        NOP();                               \
    } else {                                 \
        BEQ(rs1, rs2, 8);                    \
        B(imm - 4);                          \
    }
#define BLT_safe(rs1, rs2, imm)              \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BLT(rs1, rs2, imm);                  \
        NOP();                               \
    } else {                                 \
        BGE(rs1, rs2, 8);                    \
        B(imm - 4);                          \
    }
#define BGE_safe(rs1, rs2, imm)              \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BGE(rs1, rs2, imm);                  \
        NOP();                               \
    } else {                                 \
        BLT(rs1, rs2, 8);                    \
        B(imm - 4);                          \
    }
#define BLTU_safe(rs1, rs2, imm)             \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BLTU(rs1, rs2, imm);                 \
        NOP();                               \
    } else {                                 \
        BGEU(rs1, rs2, 8);                   \
        B(imm - 4);                          \
    }
#define BGEU_safe(rs1, rs2, imm)             \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BGEU(rs1, rs2, imm);                 \
        NOP();                               \
    } else {                                 \
        BLTU(rs1, rs2, 8);                   \
        B(imm - 4);                          \
    }
#define BGT_safe(rs1, rs2, imm)              \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BGT(rs1, rs2, imm);                  \
        NOP();                               \
    } else {                                 \
        BLE(rs1, rs2, 8);                    \
        B(imm - 4);                          \
    }
#define BLE_safe(rs1, rs2, imm)              \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BLE(rs1, rs2, imm);                  \
        NOP();                               \
    } else {                                 \
        BGT(rs1, rs2, 8);                    \
        B(imm - 4);                          \
    }
#define BGTU_safe(rs1, rs2, imm)             \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BGTU(rs1, rs2, imm);                 \
        NOP();                               \
    } else {                                 \
        BLEU(rs1, rs2, 8);                   \
        B(imm - 4);                          \
    }
#define BLEU_safe(rs1, rs2, imm)             \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BLEU(rs1, rs2, imm);                 \
        NOP();                               \
    } else {                                 \
        BGTU(rs1, rs2, 8);                   \
        B(imm - 4);                          \
    }

#define BEQZ(rs1, imm13) BEQ(rs1, 0, imm13)
#define BNEZ(rs1, imm13) BNE(rs1, 0, imm13)

#define BEQZ_safe(rs1, imm)                  \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BEQZ(rs1, imm);                      \
        NOP();                               \
    } else {                                 \
        BNEZ(rs1, 8);                        \
        B(imm - 4);                          \
    }
#define BNEZ_safe(rs1, imm)                  \
    if ((imm) > -0x1000 && (imm) < 0x1000) { \
        BNEZ(rs1, imm);                      \
        NOP();                               \
    } else {                                 \
        BEQZ(rs1, 8);                        \
        B(imm - 4);                          \
    }

// rd = 4-bytes[rs1+imm12] signed extended
#define LW(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b010, rd, 0b0000011))
// rd = 2-bytes[rs1+imm12] signed extended
#define LH(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b001, rd, 0b0000011))
// rd = byte[rs1+imm12] signed extended
#define LB(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b000, rd, 0b0000011))
// rd = 2-bytes[rs1+imm12] zero extended
#define LHU(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b101, rd, 0b0000011))
// rd = byte[rs1+imm12] zero extended
#define LBU(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b100, rd, 0b0000011))
// byte[rs1+imm12] = rs2
#define SB(rs2, rs1, imm12) EMIT(S_type(imm12, rs2, rs1, 0b000, 0b0100011))
// 2-bytes[rs1+imm12] = rs2
#define SH(rs2, rs1, imm12) EMIT(S_type(imm12, rs2, rs1, 0b001, 0b0100011))
// 4-bytes[rs1+imm12] = rs2
#define SW(rs2, rs1, imm12) EMIT(S_type(imm12, rs2, rs1, 0b010, 0b0100011))

#define PUSH1(reg)                                \
    do {                                          \
        if (cpuext.xtheadmemidx && reg != xRSP) { \
            TH_SDIB(reg, xRSP, -8, 0);            \
        } else {                                  \
            SD(reg, xRSP, 0xFF8);                 \
            SUBI(xRSP, xRSP, 8);                  \
        }                                         \
    } while (0)
#define POP1(reg)                                   \
    do {                                            \
        if (cpuext.xtheadmemidx && reg != xRSP) {   \
            TH_LDIA(reg, xRSP, 8, 0);               \
        } else {                                    \
            LD(reg, xRSP, 0);                       \
            if (reg != xRSP) ADDI(xRSP, xRSP, 8);   \
        }                                           \
    } while (0)
#define PUSH1_32(reg)                             \
    do {                                          \
        if (cpuext.xtheadmemidx && reg != xRSP) { \
            TH_SWIB(reg, xRSP, -4, 0);            \
        } else {                                  \
            SW(reg, xRSP, 0xFFC);                 \
            SUBI(xRSP, xRSP, 4);                  \
        }                                         \
    } while (0)
#define POP1_32(reg)                              \
    do {                                          \
        if (cpuext.xtheadmemidx && reg != xRSP) { \
            TH_LWUIA(reg, xRSP, 4, 0);            \
        } else {                                  \
            LWU(reg, xRSP, 0);                    \
            if (reg != xRSP) ADDI(xRSP, xRSP, 4); \
        }                                         \
    } while (0)

#define POP1z(reg)      \
    if (rex.is32bits) { \
        POP1_32(reg);   \
    } else {            \
        POP1(reg);      \
    }
#define PUSH1z(reg)     \
    if (rex.is32bits) { \
        PUSH1_32(reg);  \
    } else {            \
        PUSH1(reg);     \
    }

#define PUSH1_16(reg)                             \
    do {                                          \
        if (cpuext.xtheadmemidx && reg != xRSP) { \
            TH_SHIB(reg, xRSP, -2, 0);            \
        } else {                                  \
            SH(reg, xRSP, 0xFFE);                 \
            SUBI(xRSP, xRSP, 2);                  \
        }                                         \
    } while (0)

#define POP1_16(reg)                              \
    do {                                          \
        if (cpuext.xtheadmemidx && reg != xRSP) { \
            TH_LHUIA(reg, xRSP, 2, 0);            \
        } else {                                  \
            LHU(reg, xRSP, 0);                    \
            if (reg != xRSP) ADDI(xRSP, xRSP, 2); \
        }                                         \
    } while (0)

#define FENCE_gen(pred, succ) (((pred) << 24) | ((succ) << 20) | 0b0001111)
#define FENCE_R_RW()          EMIT(FENCE_gen(2, 3))
#define FENCE_W_W()           EMIT(FENCE_gen(1, 1))
#define FENCE_RW_RW()         EMIT(FENCE_gen(3, 3))

#define DMB_ISH()   FENCE_RW_RW()
#define DMB_ISHLD() FENCE_R_RW()
#define DMB_ISHST() FENCE_W_W()

#define FENCE_I_gen() ((0b001 << 12) | 0b0001111)
#define FENCE_I()     EMIT(FENCE_I_gen())

#define EBREAK() EMIT(I_type(1, 0, 0, 0, 0b1110011))

// RV64I
#define LWU(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b110, rd, 0b0000011))

// rd = [rs1 + imm12]
#define LD(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b011, rd, 0b0000011))
// rd = [rs1 + imm12]
#define LDxw(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b011 << (1 - rex.w), rd, 0b0000011))
// rd = [rs1 + imm12]
#define LDz(rd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b011 << rex.is32bits, rd, 0b0000011))
// [rs1 + imm12] = rs2
#define SD(rs2, rs1, imm12) EMIT(S_type(imm12, rs2, rs1, 0b011, 0b0100011))
// [rs1 + imm12] = rs2
#define SDxw(rs2, rs1, imm12) EMIT(S_type(imm12, rs2, rs1, 0b010 + rex.w, 0b0100011))
// [rs1 + imm12] = rs2
#define SDz(rs2, rs1, imm12) EMIT(S_type(imm12, rs2, rs1, 0b010 + (1 - rex.is32bits), 0b0100011))

// Shift Left Immediate
#define SLLI(rd, rs1, imm6) EMIT(I_type(((imm6) & 0x3f), rs1, 0b001, rd, 0b0010011))
// Shift Right Logical Immediate
#define SRLI(rd, rs1, imm6) EMIT(I_type(((imm6) & 0x3f), rs1, 0b101, rd, 0b0010011))
// Shift Right Arithmetic Immediate
#define SRAI(rd, rs1, imm6) EMIT(I_type(((imm6) & 0x3f) | (0b010000 << 6), rs1, 0b101, rd, 0b0010011))

// rd = rs1 + imm12
#define ADDIW(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b000, rd, 0b0011011))
// rd = rs1 - imm12
#define SUBIW(rd, rs1, imm12) EMIT(I_type((-imm12) & 0b111111111111, rs1, 0b000, rd, 0b0011011))
// rd = rs1 + imm12
#define ADDIxw(rd, rs1, imm12) EMIT(I_type((imm12) & 0b111111111111, rs1, 0b000, rd, rex.w ? 0b0010011 : 0b0011011))
// rd = rs1 + imm12
#define ADDIz(rd, rs1, imm12)      \
    do {                           \
        if (!rex.is32bits) {       \
            ADDI(rd, rs1, imm12);  \
        } else {                   \
            ADDIW(rd, rs1, imm12); \
            ZEROUP(rd);            \
        }                          \
    } while (0)
#define ADDIy(rd, rs1, imm12)           \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADDIW(rd, rs1, imm12);      \
            ZEROUP(rd);                 \
        } else {                        \
            ADDI(rd, rs1, imm12);       \
        }                               \
    } while (0)

// rd = rs1 + (rs2 << imm2)
#define ADDSL(rd, rs1, rs2, imm2, scratch) \
    if (!(imm2)) {                         \
        ADD(rd, rs1, rs2);                 \
    } else if (cpuext.zba) {               \
        SHxADD(rd, rs2, imm2, rs1);        \
    } else if (cpuext.xtheadba) {          \
        TH_ADDSL(rd, rs1, rs2, imm2);      \
    } else {                               \
        SLLI(scratch, rs2, imm2);          \
        ADD(rd, rs1, scratch);             \
    }
#define ADDSLy(rd, rs1, rs2, imm2, scratch) \
    if (rex.is32bits || rex.is67) {         \
        ADDSL(rd, rs1, rs2, imm2, scratch); \
        ZEROUP(rd);                         \
    } else {                                \
        ADDSL(rd, rs1, rs2, imm2, scratch); \
    }

#define SEXT_W(rd, rs1) ADDIW(rd, rs1, 0)

// rd = rs1<<rs2
#define SLLW(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b001, rd, 0b0111011))
// rd = rs1>>rs2 logical
#define SRLW(rd, rs1, rs2) EMIT(R_type(0b0000000, rs2, rs1, 0b101, rd, 0b0111011))
// rd = rs1>>rs2 arithmetic
#define SRAW(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b101, rd, 0b0111011))

#define SLLxw(rd, rs1, rs2)     \
    do {                        \
        if (rex.w) {            \
            SLL(rd, rs1, rs2);  \
        } else {                \
            SLLW(rd, rs1, rs2); \
            ZEROUP(rd);         \
        }                       \
    } while (0)

#define SRLxw(rd, rs1, rs2)     \
    do {                        \
        if (rex.w) {            \
            SRL(rd, rs1, rs2);  \
        } else {                \
            SRLW(rd, rs1, rs2); \
            ZEROUP(rd);         \
        }                       \
    } while (0)

#define SRAxw(rd, rs1, rs2)     \
    do {                        \
        if (rex.w) {            \
            SRA(rd, rs1, rs2);  \
        } else {                \
            SRAW(rd, rs1, rs2); \
            ZEROUP(rd);         \
        }                       \
    } while (0)

// Shift Left Immediate, 32-bit, sign-extended
#define SLLIW(rd, rs1, imm5) EMIT(I_type(imm5, rs1, 0b001, rd, 0b0011011))
// Shift Left Immediate
#define SLLIxw(rd, rs1, imm)     \
    do {                         \
        if (rex.w) {             \
            SLLI(rd, rs1, imm);  \
        } else {                 \
            SLLIW(rd, rs1, imm); \
            ZEROUP(rd);          \
        }                        \
    } while (0)
#define SLLIy(rd, rs1, imm)             \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            SLLIW(rd, rs1, imm);        \
            ZEROUP(rd);                 \
        } else {                        \
            SLLI(rd, rs1, imm);         \
        }                               \
    } while (0)
// Shift Right Logical Immediate, 32-bit, sign-extended
#define SRLIW(rd, rs1, imm5) EMIT(I_type(imm5, rs1, 0b101, rd, 0b0011011))
// Shift Right Logical Immediate
#define SRLIxw(rd, rs1, imm)            \
    do {                                \
        if (rex.w) {                    \
            SRLI(rd, rs1, imm);         \
        } else {                        \
            SRLIW(rd, rs1, imm);        \
            if ((imm) == 0) ZEROUP(rd); \
        }                               \
    } while (0)
// Shift Right Arithmetic Immediate, 32-bit, sign-extended
#define SRAIW(rd, rs1, imm5) EMIT(I_type((imm5) | (0b0100000 << 5), rs1, 0b101, rd, 0b0011011))
// Shift Right Arithmetic Immediate
#define SRAIxw(rd, rs1, imm)     \
    do {                         \
        if (rex.w) {             \
            SRAI(rd, rs1, imm);  \
        } else {                 \
            SRAIW(rd, rs1, imm); \
            ZEROUP(rd);          \
        }                        \
    } while (0)

#define CSRRW(rd, rs1, csr)  EMIT(I_type(csr, rs1, 0b001, rd, 0b1110011))
#define CSRRS(rd, rs1, csr)  EMIT(I_type(csr, rs1, 0b010, rd, 0b1110011))
#define CSRRC(rd, rs1, csr)  EMIT(I_type(csr, rs1, 0b011, rd, 0b1110011))
#define CSRRWI(rd, imm, csr) EMIT(I_type(csr, imm, 0b101, rd, 0b1110011))
#define CSRRSI(rd, imm, csr) EMIT(I_type(csr, imm, 0b110, rd, 0b1110011))
#define CSRRCI(rd, imm, csr) EMIT(I_type(csr, imm, 0b111, rd, 0b1110011))

// RV32M
// rd =(lower) rs1 * rs2 (both signed)
#define MUL(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b000, rd, 0b0110011))
// rd =(upper) rs1 * rs2 (both signed)
#define MULH(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b001, rd, 0b0110011))
// rd =(upper) (signed)rs1 * (unsigned)rs2
#define MULHSU(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b010, rd, 0b0110011))
// rd =(upper) rs1 * rs2 (both unsigned)
#define MULHU(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b011, rd, 0b0110011))
// rd =(upper) rs1 / rs2
#define DIV(rd, rs1, rs2)  EMIT(R_type(0b0000001, rs2, rs1, 0b100, rd, 0b0110011))
#define DIVU(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b101, rd, 0b0110011))
// rd = rs1 mod rs2
#define REM(rd, rs1, rs2)  EMIT(R_type(0b0000001, rs2, rs1, 0b110, rd, 0b0110011))
#define REMU(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b111, rd, 0b0110011))

// RV64M
// rd = rs1 * rs2
#define MULW(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b000, rd, 0b0111011))
// rd = rs1 * rs2
#define MULxw(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b000, rd, rex.w ? 0b0110011 : 0b0111011))
// rd = rs1 / rs2
#define DIVW(rd, rs1, rs2)   EMIT(R_type(0b0000001, rs2, rs1, 0b100, rd, 0b0111011))
#define DIVxw(rd, rs1, rs2)  EMIT(R_type(0b0000001, rs2, rs1, 0b100, rd, rex.w ? 0b0110011 : 0b0111011))
#define DIVUW(rd, rs1, rs2)  EMIT(R_type(0b0000001, rs2, rs1, 0b101, rd, 0b0111011))
#define DIVUxw(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b101, rd, rex.w ? 0b0110011 : 0b0111011))
// rd = rs1 mod rs2
#define REMW(rd, rs1, rs2)   EMIT(R_type(0b0000001, rs2, rs1, 0b110, rd, 0b0111011))
#define REMxw(rd, rs1, rs2)  EMIT(R_type(0b0000001, rs2, rs1, 0b110, rd, rex.w ? 0b0110011 : 0b0111011))
#define REMUW(rd, rs1, rs2)  EMIT(R_type(0b0000001, rs2, rs1, 0b111, rd, 0b0111011))
#define REMUxw(rd, rs1, rs2) EMIT(R_type(0b0000001, rs2, rs1, 0b111, rd, rex.w ? 0b0110011 : 0b0111011))

#define AQ_RL(f5, aq, rl) ((f5 << 2) | ((aq & 1) << 1) | (rl & 1))

// RV32A
#define LR_W(rd, rs1, aq, rl)      EMIT(R_type(AQ_RL(0b00010, aq, rl), 0, rs1, 0b010, rd, 0b0101111))
#define SC_W(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b00011, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))

#define AMOSWAP_W(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b00001, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOADD_W(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00000, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOXOR_W(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00100, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOAND_W(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b01100, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOOR_W(rd, rs2, rs1, aq, rl)   EMIT(R_type(AQ_RL(0b01000, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOMIN_W(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b10000, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOMAX_W(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b10100, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOMINU_W(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b11000, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))
#define AMOMAXU_W(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b11100, aq, rl), rs2, rs1, 0b010, rd, 0b0101111))

// RV64A
#define LR_D(rd, rs1, aq, rl)      EMIT(R_type(AQ_RL(0b00010, aq, rl), 0, rs1, 0b011, rd, 0b0101111))
#define SC_D(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b00011, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))

#define LRxw(rd, rs1, aq, rl)      EMIT(R_type(AQ_RL(0b00010, aq, rl), 0, rs1, 0b010 | rex.w, rd, 0b0101111))
#define SCxw(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b00011, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))

#define AMOSWAP_D(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b00001, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOADD_D(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00000, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOXOR_D(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00100, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOAND_D(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b01100, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOOR_D(rd, rs2, rs1, aq, rl)   EMIT(R_type(AQ_RL(0b01000, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOMIN_D(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b10000, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOMAX_D(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b10100, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOMINU_D(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b11000, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))
#define AMOMAXU_D(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b11100, aq, rl), rs2, rs1, 0b011, rd, 0b0101111))

#define AMOSWAPxw(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b00001, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOADDxw(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00000, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOXORxw(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b00100, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOANDxw(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b01100, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOORxw(rd, rs2, rs1, aq, rl)   EMIT(R_type(AQ_RL(0b01000, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOMINxw(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b10000, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOMAXxw(rd, rs2, rs1, aq, rl)  EMIT(R_type(AQ_RL(0b10100, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOMINUxw(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b11000, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))
#define AMOMAXUxw(rd, rs2, rs1, aq, rl) EMIT(R_type(AQ_RL(0b11100, aq, rl), rs2, rs1, 0b010 | rex.w, rd, 0b0101111))

// RV32F
// Read round mode
#define FRRM(rd) CSRRS(rd, xZR, 0x002)
// Swap round mode
#define FSRM(rd, rs) CSRRW(rd, rs, 0x002)
// Write FP exception flags, immediate
#define FSFLAGSI(imm) CSRRWI(xZR, imm, 0x0001)
// Read  FP exception flags to rd
#define FRFLAGS(rd) CSRRS(rd, xZR, 0x0001)
// Inexact
#define FR_NX 0
// Underflow
#define FR_UF 1
// Overflow
#define FR_OF 2
// Divide by Zero
#define FR_DZ 3
// Invalid Operation
#define FR_NV 4

// Round to Nearest, ties to Even
#define RD_RNE 0b000
// Round towards Zero
#define RD_RTZ 0b001
// Round Down (towards −∞)
#define RD_RDN 0b010
// Round Up (towards +∞)
#define RD_RUP 0b011
// Round to Nearest, ties to Max Magnitude
#define RD_RMM 0b100
// In instruction’s rm field, selects dynamic rounding mode;
#define RD_RM  0b111
#define RD_DYN RD_RM

// load single precision from rs1+imm12 to frd
#define FLW(frd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b010, frd, 0b0000111))
// store single precision frs2 to rs1+imm12
#define FSW(frs2, rs1, imm12) EMIT(S_type(imm12, frs2, rs1, 0b010, 0b0100111))
// store rs1 with rs2 sign bit to rd
#define FSGNJS(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b000, rd, 0b1010011))
// move rs1 to rd
#define FMVS(rd, rs1) FSGNJS(rd, rs1, rs1)
// store rs1 with opposite rs2 sign bit to rd
#define FSGNJNS(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b001, rd, 0b1010011))
// -rs1 => rd
#define FNEGS(rd, rs1) FSGNJNS(rd, rs1, rs1)
// store rs1 with rs1^rs2 sign bit to rd
#define FSGNJXS(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b010, rd, 0b1010011))
// |rs1| => rd
#define FABSS(rd, rs1) FSGNJXS(rd, rs1, rs1)
// Move from Single
#define FMVXW(rd, frs1) EMIT(R_type(0b1110000, 0b00000, frs1, 0b000, rd, 0b1010011))
// Move to Single
#define FMVWX(frd, rs1) EMIT(R_type(0b1111000, 0b00000, rs1, 0b000, frd, 0b1010011))
// Convert from signed 32bits to Single
#define FCVTSW(frd, rs1, rm) EMIT(R_type(0b1101000, 0b00000, rs1, rm, frd, 0b1010011))
// Convert from Single to signed 32bits (truncated)
#define FCVTWS(rd, frs1, rm) EMIT(R_type(0b1100000, 0b00000, frs1, rm, rd, 0b1010011))

#define FADDS(frd, frs1, frs2) EMIT(R_type(0b0000000, frs2, frs1, 0b111, frd, 0b1010011))
#define FSUBS(frd, frs1, frs2) EMIT(R_type(0b0000100, frs2, frs1, 0b111, frd, 0b1010011))
#define FMULS(frd, frs1, frs2) EMIT(R_type(0b0001000, frs2, frs1, 0b111, frd, 0b1010011))
#define FDIVS(frd, frs1, frs2) EMIT(R_type(0b0001100, frs2, frs1, 0b111, frd, 0b1010011))
#define FSQRTS(frd, frs1)      EMIT(R_type(0b0101100, 0b00000, frs1, 0b111, frd, 0b1010011))
#define FMINS(frd, frs1, frs2) EMIT(R_type(0b0010100, frs2, frs1, 0b000, frd, 0b1010011))
#define FMAXS(frd, frs1, frs2) EMIT(R_type(0b0010100, frs2, frs1, 0b001, frd, 0b1010011))

// compare
#define FEQS(rd, frs1, frs2) EMIT(R_type(0b1010000, frs2, frs1, 0b010, rd, 0b1010011))
#define FLTS(rd, frs1, frs2) EMIT(R_type(0b1010000, frs2, frs1, 0b001, rd, 0b1010011))
#define FLES(rd, frs1, frs2) EMIT(R_type(0b1010000, frs2, frs1, 0b000, rd, 0b1010011))

// RV64F
// Convert from signed 64bits to Single
#define FCVTSL(frd, rs1, rm) EMIT(R_type(0b1101000, 0b00010, rs1, rm, frd, 0b1010011))
// Convert from unsigned 64bits to Single
#define FCVTSLU(frd, rs1, rm) EMIT(R_type(0b1101000, 0b00011, rs1, rm, frd, 0b1010011))
// Convert from Single to signed 64bits
#define FCVTLS(rd, frs1, rm) EMIT(R_type(0b1100000, 0b00010, frs1, rm, rd, 0b1010011))
// Convert from Single to unsigned 64bits
#define FCVTLUS(rd, frs1, rm) EMIT(R_type(0b1100000, 0b00011, frs1, rm, rd, 0b1010011))
// Convert from Single to signed 32/64bits (truncated)
#define FCVTSxw(rd, frs1, rm) EMIT(R_type(0b1100000, rex.w ? 0b00010 : 0b00000, frs1, rm, rd, 0b1010011))

// RV32D
// load double precision from rs1+imm12 to frd
#define FLD(frd, rs1, imm12) EMIT(I_type(imm12, rs1, 0b011, frd, 0b0000111))
// store double precision frs2 to rs1+imm12
#define FSD(frs2, rs1, imm12) EMIT(S_type(imm12, frs2, rs1, 0b011, 0b0100111))
// Convert Double frs1 to Single frd
#define FCVTSD(frd, frs1) EMIT(R_type(0b0100000, 0b00001, frs1, 0b111, frd, 0b1010011))
// Convert Single frs1 to Double frd
#define FCVTDS(frd, frs1) EMIT(R_type(0b0100001, 0b00000, frs1, 0b111, frd, 0b1010011))
// Convert from Double to signed 32bits
#define FCVTWD(rd, frs1, rm) EMIT(R_type(0b1100001, 0b00000, frs1, rm, rd, 0b1010011))
// Convert from Double to unsigned 32bits
#define FCVTWUD(rd, frs1, rm) EMIT(R_type(0b1100001, 0b00001, frs1, rm, rd, 0b1010011))
// store rs1 with rs2 sign bit to rd
#define FSGNJD(rd, rs1, rs2) EMIT(R_type(0b0010001, rs2, rs1, 0b000, rd, 0b1010011))
// move rs1 to rd
#define FMVD(rd, rs1) FSGNJD(rd, rs1, rs1)
// store rs1 with opposite rs2 sign bit to rd
#define FSGNJND(rd, rs1, rs2) EMIT(R_type(0b0010001, rs2, rs1, 0b001, rd, 0b1010011))
// -rs1 => rd
#define FNEGD(rd, rs1) FSGNJND(rd, rs1, rs1)
// store rs1 with rs1^rs2 sign bit to rd
#define FSGNJXD(rd, rs1, rs2) EMIT(R_type(0b0010001, rs2, rs1, 0b010, rd, 0b1010011))
// |rs1| => rd
#define FABSD(rd, rs1) FSGNJXD(rd, rs1, rs1)
// Convert from signed 32bits to Double
#define FCVTDW(frd, rs1, rm) EMIT(R_type(0b1101001, 0b00000, rs1, rm, frd, 0b1010011))

#define FEQD(rd, frs1, frs2) EMIT(R_type(0b1010001, frs2, frs1, 0b010, rd, 0b1010011))
#define FLTD(rd, frs1, frs2) EMIT(R_type(0b1010001, frs2, frs1, 0b001, rd, 0b1010011))
#define FLED(rd, frs1, frs2) EMIT(R_type(0b1010001, frs2, frs1, 0b000, rd, 0b1010011))

#define FADDD(frd, frs1, frs2) EMIT(R_type(0b0000001, frs2, frs1, 0b111, frd, 0b1010011))
#define FSUBD(frd, frs1, frs2) EMIT(R_type(0b0000101, frs2, frs1, 0b111, frd, 0b1010011))
#define FMULD(frd, frs1, frs2) EMIT(R_type(0b0001001, frs2, frs1, 0b111, frd, 0b1010011))
#define FDIVD(frd, frs1, frs2) EMIT(R_type(0b0001101, frs2, frs1, 0b111, frd, 0b1010011))
#define FSQRTD(frd, frs1)      EMIT(R_type(0b0101101, 0b00000, frs1, 0b111, frd, 0b1010011))
#define FMIND(frd, frs1, frs2) EMIT(R_type(0b0010101, frs2, frs1, 0b000, frd, 0b1010011))
#define FMAXD(frd, frs1, frs2) EMIT(R_type(0b0010101, frs2, frs1, 0b001, frd, 0b1010011))

// RV64D
//  Move from Double
#define FMVXD(rd, frs1) EMIT(R_type(0b1110001, 0b00000, frs1, 0b000, rd, 0b1010011))
// Move to Double
#define FMVDX(frd, rs1) EMIT(R_type(0b1111001, 0b00000, rs1, 0b000, frd, 0b1010011))
// Convert from signed 64bits to Double
#define FCVTDL(frd, rs1, rm) EMIT(R_type(0b1101001, 0b00010, rs1, rm, frd, 0b1010011))
// Convert from unsigned 64bits to Double
#define FCVTDLU(frd, rs1, rm) EMIT(R_type(0b1101001, 0b00011, rs1, rm, frd, 0b1010011))
// Convert from Double to signed 64bits
#define FCVTLD(rd, frs1, rm) EMIT(R_type(0b1100001, 0b00010, frs1, rm, rd, 0b1010011))
// Convert from Double to unsigned 64bits
#define FCVTLUD(rd, frs1, rm) EMIT(R_type(0b1100001, 0b00011, frs1, rm, rd, 0b1010011))

// Convert from Double to signed integer
#define FCVTLDxw(rd, frs1, rm) EMIT(R_type(0b1100001, 0b00000 + (rex.w ? 0b10 : 0b00), frs1, rm, rd, 0b1010011))
// Convert from Double to unsigned integer
#define FCVTLUDxw(rd, frs1, rm) EMIT(R_type(0b1100001, 0b00001 + (rex.w ? 0b10 : 0b00), frs1, rm, rd, 0b1010011))

#define FMV(frd, frs1, single) \
    do {                       \
        if (single)            \
            FMVS(frd, frs1);   \
        else                   \
            FMVD(frd, frs1);   \
    } while (0)

// Zba
//  Add unsigned word (Wz(rs1) + X(rs2))
#define ADDUW(rd, rs1, rs2) EMIT(R_type(0b0000100, rs2, rs1, 0b000, rd, 0b0111011))
// Zero-extend Word
#define ZEXTW(rd, rs1) ADDUW(rd, rs1, xZR)
// Shift left by 1 and add (rd = X(rs2) + X(rs1)<<1)
#define SH1ADD(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b010, rd, 0b0110011))
// Shift unsigned word left by 1 and add (rd = X(rs2) + Wz(rs1)<<1)
#define SH1ADDUW(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b010, rd, 0b0111011))
// Shift left by 2 and add (rd = X(rs2) + X(rs1)<<2)
#define SH2ADD(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b100, rd, 0b0110011))
// Shift unsigned word left by 2 and add (rd = X(rs2) + Wz(rs1)<<2)
#define SH2ADDUW(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b100, rd, 0b0111011))
// Shift left by 3 and add (rd = X(rs2) + X(rs1)<<3)
#define SH3ADD(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b110, rd, 0b0110011))
// Shift unsigned word left by 3 and add (rd = X(rs2) + Wz(rs1)<<3)
#define SH3ADDUW(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b110, rd, 0b0111011))
// Shift left unsigned word (immediate)
#define SLLIUW(rd, rs1, imm) EMIT(R_type(0b0000100, imm, rs1, 0b001, rd, 0b0011011))
// Shift left by 1,2 or 3 and add (rd = X(rs2) + X(rs1)<<x)
#define SHxADD(rd, rs1, x, rs2) EMIT(R_type(0b0010000, rs2, rs1, (x) << 1, rd, 0b0110011))
// Shift unsigned word left by 1,2 or 3 and add (rd = X(rs2) + Wz(rs1)<<x)
#define SHxADDUW(rd, rs1, x, rs2) EMIT(R_type(0b0010000, rs2, rs1, (x) << 1, rd, 0b0111011))

// Zbb
//  AND with reverted operand (rs1 & ~rs2)
#define ANDN(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b111, rd, 0b0110011))
// OR with reverted operand (rs1 | ~rs2)
#define ORN(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b110, rd, 0b0110011))
// Exclusive NOR (~(rs1 ^ rs2))
#define XNOR(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b100, rd, 0b0110011))
// Count leading zero bits
#define CLZ(rd, rs) EMIT(R_type(0b0110000, 0b00000, rs, 0b001, rd, 0b0010011))
// Count leading zero bits in word
#define CLZW(rd, rs) EMIT(R_type(0b0110000, 0b00000, rs, 0b001, rd, 0b0011011))
// Count leading zero bits
#define CLZxw(rd, rs, x, s1, s2, s3)  \
    if (cpuext.zbb) {                 \
        if (x)                        \
            CLZ(rd, rs);              \
        else                          \
            CLZW(rd, rs);             \
    } else if (cpuext.xtheadbb) {     \
        if (x) {                      \
            TH_FF1(rd, rs);           \
        } else {                      \
            ZEXTW2(rd, rs);           \
            TH_FF1(rd, rd);           \
            SUBI(rd, rd, 32);         \
        }                             \
    } else {                          \
        if (rs != rd)                 \
            u8 = rd;                  \
        else                          \
            u8 = s1;                  \
        ADDI(u8, xZR, x ? 63 : 31);   \
        if (x) {                      \
            MV(s2, rs);               \
            SRLI(s3, s2, 32);         \
            BEQZ(s3, 4 + 2 * 4);      \
            SUBI(u8, u8, 32);         \
            MV(s2, s3);               \
        } else {                      \
            ZEXTW2(s2, rs);           \
        }                             \
        SRLI(s3, s2, 16);             \
        BEQZ(s3, 4 + 2 * 4);          \
        SUBI(u8, u8, 16);             \
        MV(s2, s3);                   \
        SRLI(s3, s2, 8);              \
        BEQZ(s3, 4 + 2 * 4);          \
        SUBI(u8, u8, 8);              \
        MV(s2, s3);                   \
        SRLI(s3, s2, 4);              \
        BEQZ(s3, 4 + 2 * 4);          \
        SUBI(u8, u8, 4);              \
        MV(s2, s3);                   \
        ANDI(s2, s2, 0b1111);         \
        TABLE64C(s3, const_lead0tab); \
        ADD(s3, s3, s2);              \
        LBU(s2, s3, 0);               \
        SUB(rd, u8, s2);              \
    }

// Count trailing zero bits
#define CTZ(rd, rs) EMIT(R_type(0b0110000, 0b00001, rs, 0b001, rd, 0b0010011))
// Count trailing zero bits in word
#define CTZW(rd, rs) EMIT(R_type(0b0110000, 0b00001, rs, 0b001, rd, 0b0011011))
// Count trailing zero bits
// BEWARE: You should take care of the all zeros situation yourself,
//         and clear the high 32bit when x is 1.
#define CTZxw(rd, rs, x, s1, s2)             \
    if (cpuext.zbb) {                        \
        if (x)                               \
            CTZ(rd, rs);                     \
        else                                 \
            CTZW(rd, rs);                    \
    } else {                                 \
        NEG(s2, rs);                         \
        AND(s2, s2, rs);                     \
        TABLE64_(s1, 0x03f79d71b4ca8b09ULL); \
        MUL(s2, s2, s1);                     \
        SRLI(s2, s2, 64 - 6);                \
        TABLE64C(s1, const_deBruijn64tab);   \
        ADD(s1, s1, s2);                     \
        LBU(rd, s1, 0);                      \
    }

// Count set bits
#define CPOP(rd, rs) EMIT(R_type(0b0110000, 0b00010, rs, 0b001, rd, 0b0010011))
// Count set bits in word
#define CPOPW(rd, rs) EMIT(R_type(0b0110000, 0b00010, rs, 0b001, rd, 0b0011011))
// Count set bits
#define CPOPxw(rd, rs) EMIT(R_type(0b0110000, 0b00010, rs, 0b001, rd, rex.w ? 0b0010011 : 0b0011011))
// Maximum
#define MAX(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b110, rd, 0b0110011))
// Unisgned maximum
#define MAXU(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b111, rd, 0b0110011))
// Minimum
#define MIN(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b100, rd, 0b0110011))
// Unsigned minimum
#define MINU(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b101, rd, 0b0110011))
// Sign-extend byte
#define SEXTB(rd, rs) EMIT(R_type(0b0110000, 0b00100, rs, 0b001, rd, 0b0010011))
// Sign-extend half-word
#define SEXTH_(rd, rs) EMIT(R_type(0b0110000, 0b00101, rs, 0b001, rd, 0b0010011))
// Sign-extend half-word
#define SEXTH(rd, rs)          \
    if (cpuext.zbb)            \
        SEXTH_(rd, rs);        \
    else if (cpuext.xtheadbb)  \
        TH_EXT(rd, rs, 15, 0); \
    else {                     \
        SLLI(rd, rs, 48);      \
        SRAI(rd, rd, 48);      \
    }
// Zero-extend half-word
#define ZEXTH_(rd, rs) EMIT(R_type(0b0000100, 0b00000, rs, 0b100, rd, 0b0111011))
// Zero-extend half-word
#define ZEXTH(rd, rs)           \
    if (cpuext.zbb)             \
        ZEXTH_(rd, rs);         \
    else if (cpuext.xtheadbb)   \
        TH_EXTU(rd, rs, 15, 0); \
    else {                      \
        SLLI(rd, rs, 48);       \
        SRLI(rd, rd, 48);       \
    }

// Insert low 16bits in rs to low 16bits of rd
#define INSH(rd, rs, s1, s2, init_s1, zexth_rs) \
    if (init_s1) LUI(s1, 0xffff0);              \
    AND(rd, rd, s1);                            \
    if (zexth_rs) {                             \
        ZEXTH(s2, rs);                          \
        OR(rd, rd, s2);                         \
    } else {                                    \
        OR(rd, rd, rs);                         \
    }

// Insert low 16bits in rs to low 16bits of rd
#define INSHz(rd, rs, s1, s2, init_s1, zexth_rs) \
    INSH(rd, rs, s1, s2, init_s1, zexth_rs)      \
    if (rex.is32bits) ZEXTW2(rd, rd);

// Rotate left (register)
#define ROL(rd, rs1, rs2) EMIT(R_type(0b0110000, rs2, rs1, 0b001, rd, 0b0110011))
// Rotate left word (register)
#define ROLW(rd, rs1, rs2) EMIT(R_type(0b0110000, rs2, rs1, 0b001, rd, 0b0111011))
// Rotate left (register)
#define ROLxw(rd, rs1, rs2) EMIT(R_type(0b0110000, rs2, rs1, 0b001, rd, rex.w ? 0b0110011 : 0b0111011))
// Rotate right (register)
#define ROR(rd, rs1, rs2) EMIT(R_type(0b0110000, rs2, rs1, 0b101, rd, 0b0110011))
// Rotate right (immediate)
#define RORI(rd, rs1, shamt) EMIT(I_type((shamt) | (0b011000 << 6), rs1, 0b101, rd, 0b0010011))
// Rotate right word (immediate)
#define RORIW(rd, rs1, shamt) EMIT(R_type(0b0110000, shamt, rs1, 0b101, rd, 0b0011011))
// Rotate right (immediate)
#define RORIxw(rd, rs1, shamt) EMIT(R_type(0b0110000, shamt, rs1, 0b101, rd, rex.w ? 0b0010011 : 0b0011011))
// Rotate right word (register)
#define RORW(rd, rs1, rs2) EMIT(R_type(0b0110000, rs2, rs1, 0b101, rd, 0b0111011))
// Rotate right (register)
#define RORxw(rd, rs1, rs2) EMIT(R_type(0b0110000, rs2, rs1, 0b101, rd, rex.w ? 0b0110011 : 0b0111011))
// Bitwise OR Combine, byte granule (for all byte, if byte==0, res.byte=0, else res.byte=0xff)
#define ORCB(rd, rs) EMIT(I_type(0b001010000111, rs, 0b101, rd, 0b0010011))
// Byte-reverse register
#define REV8(rd, rs) EMIT(I_type(0b011010111000, rs, 0b101, rd, 0b0010011))

// Byte-reverse register, rd can be the same as rs or s1, but rs cannot be the same as s1.
#define REV8xw(rd, rs, s1, s2, s3, s4) \
    if (cpuext.zbb) {                  \
        REV8(rd, rs);                  \
        if (!rex.w) {                  \
            SRLI(rd, rd, 32);          \
        }                              \
    } else if (cpuext.xtheadbb) {      \
        if (rex.w) {                   \
            TH_REV(rd, rs);            \
        } else {                       \
            TH_REVW(rd, rs);           \
        }                              \
    } else {                           \
        MOV_U12(s2, 0xff);             \
        if (rex.w) {                   \
            SLLI(s1, rs, 56);          \
            SRLI(s3, rs, 56);          \
            SRLI(s4, rs, 40);          \
            SLLI(s2, s2, 8);           \
            AND(s4, s4, s2);           \
            OR(s1, s1, s3);            \
            OR(s1, s1, s4);            \
            SLLI(s3, rs, 40);          \
            SLLI(s4, s2, 40);          \
            AND(s3, s3, s4);           \
            OR(s1, s1, s3);            \
            SRLI(s3, rs, 24);          \
            SLLI(s4, s2, 8);           \
            AND(s3, s3, s4);           \
            OR(s1, s1, s3);            \
            SLLI(s3, rs, 24);          \
            SLLI(s4, s2, 32);          \
            AND(s3, s3, s4);           \
            OR(s1, s1, s3);            \
            SRLI(s3, rs, 8);           \
            SLLI(s4, s2, 16);          \
            AND(s3, s3, s4);           \
            OR(s1, s1, s3);            \
            SLLI(s3, rs, 8);           \
            SLLI(s4, s2, 24);          \
            AND(s3, s3, s4);           \
            OR(rd, s1, s3);            \
        } else {                       \
            SLLIW(s2, s2, 8);          \
            SLLIW(s1, rs, 24);         \
            SRLIW(s3, rs, 24);         \
            SRLIW(s4, rs, 8);          \
            AND(s4, s4, s2);           \
            OR(s1, s1, s3);            \
            OR(s1, s1, s4);            \
            SLLIW(s3, rs, 8);          \
            LUI(s2, 0xff0);            \
            AND(s3, s3, s2);           \
            OR(rd, s1, s3);            \
        }                              \
    }                                  \
    if (!rex.w)                        \
        ZEXTW2(rd, rd);


// Zbc
//  Carry-less multiply (low-part)
#define CLMUL(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b001, rd, 0b0110011))
// Carry-less multiply (high-part)
#define CLMULH(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b011, rd, 0b0110011))
// Carry-less multiply (reversed)
#define CLMULR(rd, rs1, rs2) EMIT(R_type(0b0000101, rs2, rs1, 0b010, rd, 0b0110011))

// Zbs
//  encoding of the "imm" on RV64 use a slight different mask, but it will work using R_type with high bit of imm overwriting low bit op func
//  Single-bit Clear (Register)
#define BCLR(rd, rs1, rs2) EMIT(R_type(0b0100100, rs2, rs1, 0b001, rd, 0b0110011))
// Single-bit Clear (Immediate)
#define BCLI(rd, rs1, imm) EMIT(R_type(0b0100100, imm, rs1, 0b001, rd, 0b0010011))
// Single-bit Extract (Register)
#define BEXT_(rd, rs1, rs2) EMIT(R_type(0b0100100, rs2, rs1, 0b101, rd, 0b0110011))
// Single-bit Extract (Immediate)
#define BEXTI_(rd, rs1, shamt) EMIT(I_type((shamt) | (0b010010 << 6), rs1, 0b101, rd, 0b0010011))
// Single-bit Invert (Register)
#define BINV(rd, rs1, rs2) EMIT(R_type(0b0110100, rs2, rs1, 0b001, rd, 0b0110011))
// Single-bit Invert (Immediate)
#define BINVI(rd, rs1, imm) EMIT(R_type(0b0110100, imm, rs1, 0b001, rd, 0b0010011))
// Single-bit Set (Register)
#define BSET(rd, rs1, rs2) EMIT(R_type(0b0010100, rs2, rs1, 0b001, rd, 0b0110011))
// Single-bit Set (Immediate)
#define BSETI(rd, rs1, imm) EMIT(R_type(0b0010100, imm, rs1, 0b001, rd, 0b0010011))

// Single-bit Extract (Register)
#define BEXT(rd, rs1, rs2, s0)              \
    if (cpuext.zbs) {                       \
        if (rex.w) {                        \
            BEXT_(rd, rs1, rs2);            \
        } else {                            \
            ANDI(s0, rs2, 0x1f);            \
            BEXT_(rd, rs1, s0);             \
        }                                   \
    } else {                                \
        ANDI(s0, rs2, rex.w ? 0x3f : 0x1f); \
        SRL(rd, rs1, s0);                   \
        ANDI(rd, rd, 1);                    \
    }

// Single-bit Extract (Immediate)
#define BEXTI(rd, rs1, imm)   \
    if (cpuext.zbs)           \
        BEXTI_(rd, rs1, imm); \
    else if (cpuext.xtheadbs) \
        TH_TST(rd, rs1, imm); \
    else {                    \
        SRLIxw(rd, rs1, imm); \
        ANDI(rd, rd, 1);      \
    }

/// THead vendor extension
/// https://github.com/T-head-Semi/thead-extension-spec/releases

// XTheadBa - Address calculation instructions

// Add a shifted operand to a second operand.
// reg[rd] := reg[rs1] + (reg[rs2] << imm2)
#define TH_ADDSL(rd, rs1, rs2, imm2) EMIT(R_type((imm2) & 0b11, rs2, rs1, 0b001, rd, 0b0001011))

// XTheadBb - Basic bit-manipulation

#define TH_SRRIxw(rd, rs1, imm) \
    if (rex.w) {                \
        TH_SRRI(rd, rs1, imm);  \
    } else {                    \
        TH_SRRIW(rd, rs1, imm); \
    }

// Perform a cyclic right shift.
// reg[rd] := (reg[rs1] >> imm6) | (reg[rs1] << (xlen - imm6))
#define TH_SRRI(rd, rs1, imm6) EMIT(I_type(0b000100000000 | ((imm6) & 0x3f), rs1, 0b001, rd, 0b0001011))

// Perform a cyclic right shift on word operand.
// data := zext.w(reg[rs1])
// reg[rd] := (data >> imm5) | (data << (32 - imm5))
#define TH_SRRIW(rd, rs1, imm5) EMIT(I_type(0b000101000000 | ((imm5) & 0x1f), rs1, 0b001, rd, 0b0001011))

// Extract and sign-extend bits.
// reg[rd] := sign_extend(reg[rs1][imm1:imm2])
#define TH_EXT(rd, rs1, imm1, imm2) EMIT(I_type((((imm1) & 0x3f) << 6) | ((imm2) & 0x3f), rs1, 0b010, rd, 0b0001011))

// Extract and zero-extend bits.
// reg[rd] := zero_extend(reg[rs1][imm1:imm2])
#define TH_EXTU(rd, rs1, imm1, imm2) EMIT(I_type((((imm1) & 0x3f) << 6) | ((imm2) & 0x3f), rs1, 0b011, rd, 0b0001011))

// Find first '0'-bit
// for i=xlen..0:
//   if reg[rs1][i] == 0:
//     break;
// reg[rd] = (xlen - 1) - i
#define TH_FF0(rd, rs1) EMIT(I_type(0b100001000000, rs1, 0b001, rd, 0b0001011))

// Find first '1'-bit
// for i=xlen..0:
//   if reg[rs1][i] == 1:
//     break;
// reg[rd] = (xlen - 1) - i
#define TH_FF1(rd, rs1) EMIT(I_type(0b100001100000, rs1, 0b001, rd, 0b0001011))

// Reverse the byte order.
// for i=0..(xlen/8-1):
//   j := xlen/8 - 1 - i
//   tmp[i] := reg[rs1][j]
// reg[rd] := tmp
#define TH_REV(rd, rs1) EMIT(I_type(0b100000100000, rs1, 0b001, rd, 0b0001011))

// Reverse the byte order of a word operand.
// for i=0..3:
//   j := 3 - i
//   tmp[i] := reg[rs1][j]
// reg[rd] := tmp
#define TH_REVW(rd, rs1) EMIT(I_type(0b100100000000, rs1, 0b001, rd, 0b0001011))

// Test for NUL bytes.
// for i=0..(xlen/8-1):
//   if reg[rs1][i] == 0:
//     reg[rd][i] := 0xff
//   else
//     reg[rd][i] := 0
#define TH_TSTNBZ(rd, rs1) EMIT(I_type(0b100000000000, rs1, 0b001, rd, 0b0001011))

// XTheadBs - Single-bit instructions

// Tests if a single bit is set.
// if (reg[rs1] & (1 << imm6))
//   rd := 1
// else
//   rd := 0
#define TH_TST(rd, rs1, imm6) EMIT(I_type(0b100010000000 | ((imm6) & 0x3f), rs1, 0b001, rd, 0b0001011))


// XTheadCondMov -  Conditional move

// Move if equal zero.
// if (reg[rs2] == 0x0)
//   reg[rd] := reg[rs1]
#define TH_MVEQZ(rd, rs1, rs2) EMIT(R_type(0b0100000, rs2, rs1, 0b001, rd, 0b0001011))

// Move if not equal zero.
// if (reg[rs2] != 0x0)
//   reg[rd] := reg[rs1]
#define TH_MVNEZ(rd, rs1, rs2) EMIT(R_type(0b0100001, rs2, rs1, 0b001, rd, 0b0001011))

// XTheadMemIdx -  Indexed memory operations

// Load indexed byte, increment address after loading.
// rd := sign_extend(mem[rs1])
// rs1 := rs1 + (sign_extend(imm5) << imm2)
#define TH_LBIA(rd, rs1, imm5, imm2) EMIT(I_type(0b000110000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b100, rd, 0b0001011))

// Load indexed half-word, increment address after loading.
// if (rs1 != rd) {
//     rd := sign_extend(mem[rs1+1:rs1])
//     rs1 := rs1 + (sign_extend(imm5) << imm2)
// }
#define TH_LHIA(rd, rs1, imm5, imm2) EMIT(I_type(0b001110000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b100, rd, 0b0001011))

// Load indexed unsigned half-word, increment address after loading.
// if (rs1 != rd) {
//     rd := zero_extend(mem[rs1+1:rs1])
//     rs1 := rs1 + (sign_extend(imm5) << imm2)
// }
#define TH_LHUIA(rd, rs1, imm5, imm2) EMIT(I_type(0b101110000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b100, rd, 0b0001011))

// Load indexed word, increment address after loading.
// if (rs1 != rd) {
//     rd := sign_extend(mem[rs1+3:rs1])
//     rs1 := rs1 + (sign_extend(imm5) << imm2)
// }
#define TH_LWIA(rd, rs1, imm5, imm2) EMIT(I_type(0b010110000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b100, rd, 0b0001011))

// Load indexed unsigned word, increment address after loading.
// if (rs1 != rd) {
//     rd := zero_extend(mem[rs1+3:rs1])
//     rs1 := rs1 + (sign_extend(imm5) << imm2)
// }
#define TH_LWUIA(rd, rs1, imm5, imm2) EMIT(I_type(0b110110000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b100, rd, 0b0001011))

// Load indexed double-word, increment address after loading.
// if (rs1 != rd) {
//     rd := sign_extend(mem[rs1+7:rs1])
//     rs1 := rs1 + (sign_extend(imm5) << imm2)
// }
#define TH_LDIA(rd, rs1, imm5, imm2) EMIT(I_type(0b011110000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b100, rd, 0b0001011))

// Store indexed half-word, increment address before storage.
// rs1 := rs1 + (sign_extend(imm5) << imm2)
// mem[rs1+1:rs1] := rd
#define TH_SHIB(rd, rs1, imm5, imm2) EMIT(I_type(0b001010000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b101, rd, 0b0001011))

// Store indexed word, increment address before storage.
// rs1 := rs1 + (sign_extend(imm5) << imm2)
// mem[rs1+3:rs1] := rd
#define TH_SWIB(rd, rs1, imm5, imm2) EMIT(I_type(0b010010000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b101, rd, 0b0001011))

// Store indexed double-word, increment address before storage.
// rs1 := rs1 + (sign_extend(imm5) << imm2)
// mem[rs1+7:rs1] := rd
#define TH_SDIB(rd, rs1, imm5, imm2) EMIT(I_type(0b011010000000 | (((imm2) & 0b11) << 5) | ((imm5) & 0x1f), rs1, 0b101, rd, 0b0001011))

// Load indexed word.
// addr := rs1 + (rs2 << imm2)
// rd := sign_extend(mem[addr+7:addr])
#define TH_LRD(rd, rs1, rs2, imm2) EMIT(R_type(0b0110000 | ((imm2) & 0b11), rs2, rs1, 0b100, rd, 0b0001011))

// TODO
// th.lbib rd, (rs1), imm5, imm2 Load indexed byte
// th.lbuia rd, (rs1), imm5, imm2 Load indexed unsigned byte
// th.lbuib rd, (rs1), imm5, imm2 Load indexed unsigned byte
// th.lhia rd, (rs1), imm5, imm2 Load indexed half-word
// th.lhib rd, (rs1), imm5, imm2 Load indexed half-word
// th.lhuib rd, (rs1), imm5, imm2 Load indexed unsigned half-word
// th.lwia rd, (rs1), imm5, imm2 Load indexed word
// th.lwib rd, (rs1), imm5, imm2 Load indexed word
// th.lwuib rd, (rs1), imm5, imm2 Load indexed unsigned word
// th.ldib rd, (rs1), imm5, imm2 Load indexed double-word
// th.sbia rd, (rs1), imm5, imm2 Store indexed byte
// th.sbib rd, (rs1), imm5, imm2 Store indexed byte
// th.shia rd, (rs1), imm5, imm2 Store indexed half-word
// th.shib rd, (rs1), imm5, imm2 Store indexed half-word
// th.swia rd, (rs1), imm5, imm2 Store indexed word
// th.swib rd, (rs1), imm5, imm2 Store indexed word
// th.sdia rd, (rs1), imm5, imm2 Store indexed double-word
// th.lrb rd, rs1, rs2, imm2 Load indexed byte
// th.lrbu rd, rs1, rs2, imm2 Load indexed unsigned byte
// th.lrh rd, rs1, rs2, imm2 Load indexed half-word
// th.lrhu rd, rs1, rs2, imm2 Load indexed unsigned half-word
// th.lrw rd, rs1, rs2, imm2 Load indexed word
// th.lrwu rd, rs1, rs2, imm2 Load indexed unsigned word
// th.srb rd, rs1, rs2, imm2 Store indexed byte
// th.srh rd, rs1, rs2, imm2 Store indexed half-word
// th.srw rd, rs1, rs2, imm2 Store indexed word
// th.srd rd, rs1, rs2, imm2 Store indexed double-word
// th.lurb rd, rs1, rs2, imm2 Load unsigned indexed byte
// th.lurbu rd, rs1, rs2, imm2 Load unsigned indexed unsigned byte
// th.lurh rd, rs1, rs2, imm2 Load unsigned indexed half-word
// th.lurhu rd, rs1, rs2, imm2 Load unsigned indexed unsigned half-word
// th.lurw rd, rs1, rs2, imm2 Load unsigned indexed word
// th.lurwu rd, rs1, rs2, imm2 Load unsigned indexed unsigned word
// th.lurd rd, rs1, rs2, imm2 Load unsigned indexed double-word
// th.surb rd, rs1, rs2, imm2 Store unsigned indexed byte
// th.surh rd, rs1, rs2, imm2 Store unsigned indexed half-word
// th.surw rd, rs1, rs2, imm2 Store unsigned indexed word
// th.surd rd, rs1, rs2, imm2 Store unsigned indexed double-word

// XTheadMemPair - Two-GPR memory operations


// Load two 64-bit values from memory into two GPRs.
// addr := rs1 + (zero_extend(imm2) << 4)
// rd1 := mem[addr+7:addr]
// rd2 := mem[addr+15:addr+8]
#define TH_LDD(rd1, rd2, rs1, imm2) EMIT(R_type(0b1111100 | ((imm2) & 0b11), rd2, rs1, 0b100, rd1, 0b0001011))

// Load two signed 32-bit values from memory into two GPRs.
// addr := rs1 + (zero_extend(imm2) << 3)
// reg[rd1] := sign_extend(mem[addr+3:addr])
// reg[rd2] := sign_extend(mem[addr+7:addr+4])
#define TH_LWD(rd1, rd2, rs1, imm2) EMIT(R_type(0b1110000 | ((imm2) & 0b11), rd2, rs1, 0b100, rd1, 0b0001011))

// Load two unsigned 32-bit values from memory into two GPRs.
// addr := rs1 + (zero_extend(imm2) << 3)
// reg[rd1] := zero_extend(mem[addr+3:addr])
// reg[rd2] := zero_extend(mem[addr+7:addr+4])
#define TH_LWUD(rd1, rd2, rs1, imm2) EMIT(R_type(0b1111000 | ((imm2) & 0b11), rd2, rs1, 0b100, rd1, 0b0001011))

// Store two 64-bit values to memory from two GPRs.
// addr := rs1 + (zero_extend(imm2) << 4)
// mem[addr+7:addr] := reg[rd1]
// mem[addr+15:addr+8] := reg[rd2]
#define TH_SDD(rd1, rd2, rs1, imm2) EMIT(R_type(0b1111100 | ((imm2) & 0b11), rd2, rs1, 0b101, rd1, 0b0001011))

// Store two 32-bit values to memory from two GPRs.
// addr := rs1 + (zero_extend(imm2) << 3)
// mem[addr+3:addr] := reg[rd1][31:0]
// mem[addr+7:addr+3] := reg[rd2][31:0]
#define TH_SWD(rd1, rd2, rs1, imm2) EMIT(R_type(0b1110000 | ((imm2) & 0b11), rd2, rs1, 0b101, rd1, 0b0001011))

// XTheadFMemIdx - Indexed memory operations for floating-point registers

// Load indexed double-precision floating point value.
// addr := rs1 + (rs2 << imm2)
// rd := fmem[addr+7:addr]
#define TH_FLRD(rd, rs1, rs2, imm2) EMIT(R_type(0b0110000 | ((imm2) & 0b11), rs2, rs1, 0b110, rd, 0b0001011))

// TODO
// th.flrw rd, rs1, rs2, imm2 Load indexed float
// th.flurd rd, rs1, rs2, imm2 Load unsigned indexed double
// th.flurw rd, rs1, rs2, imm2 Load unsigned indexed float
// th.fsrd rd, rs1, rs2, imm2 Store indexed double
// th.fsrw rd, rs1, rs2, imm2 Load indexed float
// th.fsurd rd, rs1, rs2, imm2 Store unsigned indexed double
// th.fsurw rd, rs1, rs2, imm2 Load unsigned indexed float

// XTheadMac - Multiply-accumulate instructions

// Compute multiply-add result of double-word operands.
// M := reg[rs1] * reg[rs2]
// reg[rd] := reg[rd] + M
#define TH_MULA(rd, rs1, rs2) EMIT(R_type(0b0010000, rs2, rs1, 0b001, rd, 0b0001011))

// TODO
// th.mulah rd, rs1, rs2 Multiply-add half-words
// th.mulaw rd, rs1, rs2 Multiply-add words
// th.muls rd, rs1, rs2 Multiply-subtract double-words
// th.mulsh rd, rs1, rs2 Multiply-subtract half-words
// th.mulsw rd, rs1, rs2 Multiply-subtract words

// XTheadFmv - Double-precision floating-point high-bit data transmission instructions

// Read double-precision floating-point high-bit data
// rd := fs1[63:32]
#define TH_FMV_X_HW(rd, fs1) EMIT(R_type(0b1100000, 0, fs1, 0b001, rd, 0b0001011))

// Write double-precision floating-point high-bit data
// fs1[63:32] := rd
#define TH_FMV_HW_X(rd, fs1) EMIT(R_type(0b1010000, 0, fs1, 0b001, rd, 0b0001011))

// Vector extension emitter

/* Warning: mind the differences between RVV 1.0 and XTheadVector!
 *
 * - Different encoding of vsetvl/th.vsetvl.
 * - No vsetivli instruction.
 * - Cannot configure vta and vma vsetvl instruction, the fixed value is TAMU.
 * - No whole register move instructions.
 * - No fractional lmul.
 * - Different load/store instructions.
 * - Different name of vector indexed instructions.
 * - Destination vector register cannot overlap source vector register group for vmadc/vmsbc/widen arithmetic/narrow arithmetic.
 * - No vlm/vsm instructions.
 * - Different vnsrl/vnsra/vfncvt suffix (vv/vx/vi vs wv/wx/wi).
 * - Different size of mask mode (1.0 is vl and xtheadvector is vlen).
 * - No vrgatherei16.vv instruction.
 * - Different encoding of vmv.s.x instruction.
 *
 * We ignore all the naming differences and use the RVV 1.0 naming convention.

 */

#define VECTOR_SEW8   0b000
#define VECTOR_SEW16  0b001
#define VECTOR_SEW32  0b010
#define VECTOR_SEW64  0b011
#define VECTOR_SEWNA  0b111  // N/A
#define VECTOR_SEWANY 0b1000 // any sew would be ok, but not N/A.

#define VECTOR_LMUL1 0b000
#define VECTOR_LMUL2 0b001
#define VECTOR_LMUL4 0b010
#define VECTOR_LMUL8 0b011

#define VECTOR_MASKED   0
#define VECTOR_UNMASKED 1

#define VECTOR_NFIELD1 0b000
#define VECTOR_NFIELD2 0b001
#define VECTOR_NFIELD3 0b010
#define VECTOR_NFIELD4 0b011
#define VECTOR_NFIELD5 0b100
#define VECTOR_NFIELD6 0b101
#define VECTOR_NFIELD7 0b110
#define VECTOR_NFIELD8 0b111

//  configuration setting
//  https://github.com/riscv/riscv-v-spec/blob/master/vcfg-format.adoc
#define VSETIVLI(rd, zimm, zimm10) EMIT(I_type(0b110000000000 | (zimm10), zimm, 0b111, rd, 0b1010111)) // 11...............111.....1010111
#define VSETVLI(rd, rs1, zimm11)   EMIT(I_type(zimm11, rs1, 0b111, rd, 0b1010111))                     // 0................111.....1010111
#define VSETVL(rd, rs1, rs2)       EMIT(R_type(0b1000000, rs2, rs1, 0b111, rd, 0b1010111))             // 1000000..........111.....1010111

//  Vector Loads and Store
//  https://github.com/riscv/riscv-v-spec/blob/master/vmem-format.adoc
//
//  Vector Unit-Stride Instructions (including segment part)
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#74-vector-unit-stride-instructions

#define VLM_V(vd, rs1)  EMIT(I_type(0b000000101011, rs1, 0b000, vd, 0b0000111))  // 000000101011.....000.....0000111
#define VSM_V(vs3, rs1) EMIT(I_type(0b000000101011, rs1, 0b000, vs3, 0b0100111)) // 000000101011.....000.....0100111

#define VLE8_V(vd, rs1, vm, nf)   EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b000, vd, 0b0000111))  // ...000.00000.....000.....0000111
#define VLE16_V(vd, rs1, vm, nf)  EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b101, vd, 0b0000111))  // ...000.00000.....101.....0000111
#define VLE32_V(vd, rs1, vm, nf)  EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b110, vd, 0b0000111))  // ...000.00000.....110.....0000111
#define VLE64_V(vd, rs1, vm, nf)  EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b111, vd, 0b0000111))  // ...000.00000.....111.....0000111
#define VSE8_V(vs3, rs1, vm, nf)  EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b000, vs3, 0b0100111)) // ...000.00000.....000.....0100111
#define VSE16_V(vs3, rs1, vm, nf) EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b101, vs3, 0b0100111)) // ...000.00000.....101.....0100111
#define VSE32_V(vs3, rs1, vm, nf) EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b110, vs3, 0b0100111)) // ...000.00000.....110.....0100111
#define VSE64_V(vs3, rs1, vm, nf) EMIT(I_type(((nf) << 9) | (vm << 5), rs1, 0b111, vs3, 0b0100111)) // ...000.00000.....111.....0100111

#define VLE_V(vd, rs1, sew, vm, nf)  EMIT(I_type(((nf) << 9) | (vm << 5), rs1, (sew == 0b000 ? 0b000 : (0b100 | sew)), vd, 0b0000111))
#define VSE_V(vs3, rs1, sew, vm, nf) EMIT(I_type(((nf) << 9) | (vm << 5), rs1, (sew == 0b000 ? 0b000 : (0b100 | sew)), vs3, 0b0100111))

//  Vector Indexed-Unordered Instructions (including segment part)
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#76-vector-indexed-instructions
//  Note: Make sure SEW in vtype is always the same as EEW, for xtheadvector compatibility!

#define VLUXEI8_V(vd, vs2, rs1, vm, nf)   EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b0110 : 0b0010), vs2, rs1, 0b000, vd, 0b0000111))  // ...001...........000.....0000111
#define VLUXEI16_V(vd, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b0110 : 0b0010), vs2, rs1, 0b101, vd, 0b0000111))  // ...001...........101.....0000111
#define VLUXEI32_V(vd, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b0110 : 0b0010), vs2, rs1, 0b110, vd, 0b0000111))  // ...001...........110.....0000111
#define VLUXEI64_V(vd, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b0110 : 0b0010), vs2, rs1, 0b111, vd, 0b0000111))  // ...001...........111.....0000111
#define VSUXEI8_V(vs3, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b1110 : 0b0010), vs2, rs1, 0b000, vs3, 0b0100111)) // ...001...........000.....0100111
#define VSUXEI16_V(vs3, vs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b1110 : 0b0010), vs2, rs1, 0b101, vs3, 0b0100111)) // ...001...........101.....0100111
#define VSUXEI32_V(vs3, vs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b1110 : 0b0010), vs2, rs1, 0b110, vs3, 0b0100111)) // ...001...........110.....0100111
#define VSUXEI64_V(vs3, vs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | (cpuext.xtheadvector ? 0b1110 : 0b0010), vs2, rs1, 0b111, vs3, 0b0100111)) // ...001...........111.....0100111

//  Vector Strided Instructions (including segment part)
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#75-vector-strided-instructions

#define VLSE8_V(vd, rs2, rs1, vm, nf)   EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b000, vd, 0b0000111))  // ...010...........000.....0000111
#define VLSE16_V(vd, rs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b101, vd, 0b0000111))  // ...010...........101.....0000111
#define VLSE32_V(vd, rs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b110, vd, 0b0000111))  // ...010...........110.....0000111
#define VLSE64_V(vd, rs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b111, vd, 0b0000111))  // ...010...........111.....0000111
#define VSSE8_V(vs3, rs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b000, vs3, 0b0100111)) // ...010...........000.....0100111
#define VSSE16_V(vs3, rs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b101, vs3, 0b0100111)) // ...010...........101.....0100111
#define VSSE32_V(vs3, rs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b110, vs3, 0b0100111)) // ...010...........110.....0100111
#define VSSE64_V(vs3, rs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | 0b0100, rs2, rs1, 0b111, vs3, 0b0100111)) // ...010...........111.....0100111

//  Vector Indexed-Ordered Instructions (including segment part)
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#76-vector-indexed-instructions

#define VLOXEI8_V(vd, vs2, rs1, vm, nf)   EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b000, vd, 0b0000111))  // ...011...........000.....0000111
#define VLOXEI16_V(vd, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b101, vd, 0b0000111))  // ...011...........101.....0000111
#define VLOXEI32_V(vd, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b110, vd, 0b0000111))  // ...011...........110.....0000111
#define VLOXEI64_V(vd, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b111, vd, 0b0000111))  // ...011...........111.....0000111
#define VSOXEI8_V(vs3, vs2, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b000, vs3, 0b0100111)) // ...011...........000.....0100111
#define VSOXEI16_V(vs3, vs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b101, vs3, 0b0100111)) // ...011...........101.....0100111
#define VSOXEI32_V(vs3, vs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b110, vs3, 0b0100111)) // ...011...........110.....0100111
#define VSOXEI64_V(vs3, vs2, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm) | 0b0110, vs2, rs1, 0b111, vs3, 0b0100111)) // ...011...........111.....0100111

//  Unit-stride F31..29=0ault-Only-First Loads
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#77-unit-stride-fault-only-first-loads

#define VLE8FF_V(vd, rs1, vm, nf)  EMIT(R_type(((nf) << 4) | (vm), 0b10000, rs1, 0b000, vd, 0b0000111)) // ...000.10000.....000.....0000111
#define VLE16FF_V(vd, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm), 0b10000, rs1, 0b101, vd, 0b0000111)) // ...000.10000.....101.....0000111
#define VLE32FF_V(vd, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm), 0b10000, rs1, 0b110, vd, 0b0000111)) // ...000.10000.....110.....0000111
#define VLE64FF_V(vd, rs1, vm, nf) EMIT(R_type(((nf) << 4) | (vm), 0b10000, rs1, 0b111, vd, 0b0000111)) // ...000.10000.....111.....0000111

//  Vector Load/Store Whole Registers
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#79-vector-loadstore-whole-register-instructions

#define VL1RE8_V(vd, rs1)  EMIT(I_type(0b000000101000, rs1, 0b000, vd, 0b0000111))  // 000000101000.....000.....0000111
#define VL1RE16_V(vd, rs1) EMIT(I_type(0b000000101000, rs1, 0b101, vd, 0b0000111))  // 000000101000.....101.....0000111
#define VL1RE32_V(vd, rs1) EMIT(I_type(0b000000101000, rs1, 0b110, vd, 0b0000111))  // 000000101000.....110.....0000111
#define VL1RE64_V(vd, rs1) EMIT(I_type(0b000000101000, rs1, 0b111, vd, 0b0000111))  // 000000101000.....111.....0000111
#define VL2RE8_V(vd, rs1)  EMIT(I_type(0b001000101000, rs1, 0b000, vd, 0b0000111))  // 001000101000.....000.....0000111
#define VL2RE16_V(vd, rs1) EMIT(I_type(0b001000101000, rs1, 0b101, vd, 0b0000111))  // 001000101000.....101.....0000111
#define VL2RE32_V(vd, rs1) EMIT(I_type(0b001000101000, rs1, 0b110, vd, 0b0000111))  // 001000101000.....110.....0000111
#define VL2RE64_V(vd, rs1) EMIT(I_type(0b001000101000, rs1, 0b111, vd, 0b0000111))  // 001000101000.....111.....0000111
#define VL4RE8_V(vd, rs1)  EMIT(I_type(0b011000101000, rs1, 0b000, vd, 0b0000111))  // 011000101000.....000.....0000111
#define VL4RE16_V(vd, rs1) EMIT(I_type(0b011000101000, rs1, 0b101, vd, 0b0000111))  // 011000101000.....101.....0000111
#define VL4RE32_V(vd, rs1) EMIT(I_type(0b011000101000, rs1, 0b110, vd, 0b0000111))  // 011000101000.....110.....0000111
#define VL4RE64_V(vd, rs1) EMIT(I_type(0b011000101000, rs1, 0b111, vd, 0b0000111))  // 011000101000.....111.....0000111
#define VL8RE8_V(vd, rs1)  EMIT(I_type(0b111000101000, rs1, 0b000, vd, 0b0000111))  // 111000101000.....000.....0000111
#define VL8RE16_V(vd, rs1) EMIT(I_type(0b111000101000, rs1, 0b101, vd, 0b0000111))  // 111000101000.....101.....0000111
#define VL8RE32_V(vd, rs1) EMIT(I_type(0b111000101000, rs1, 0b110, vd, 0b0000111))  // 111000101000.....110.....0000111
#define VL8RE64_V(vd, rs1) EMIT(I_type(0b111000101000, rs1, 0b111, vd, 0b0000111))  // 111000101000.....111.....0000111
#define VS1R_V(vs3, rs1)   EMIT(I_type(0b000000101000, rs1, 0b000, vs3, 0b0100111)) // 000000101000.....000.....0100111
#define VS2R_V(vs3, rs1)   EMIT(I_type(0b001000101000, rs1, 0b000, vs3, 0b0100111)) // 001000101000.....000.....0100111
#define VS4R_V(vs3, rs1)   EMIT(I_type(0b011000101000, rs1, 0b000, vs3, 0b0100111)) // 011000101000.....000.....0100111
#define VS8R_V(vs3, rs1)   EMIT(I_type(0b111000101000, rs1, 0b000, vs3, 0b0100111)) // 111000101000.....000.....0100111

//  Vector Floating-Point Instructions
//  https://github.com/riscv/riscv-v-spec/blob/master/v-spec.adoc#14-vector-floating-point-instructions

//  OPFVF
#define VFADD_VF(vd, vs2, rs1, vm)        EMIT(R_type(0b0000000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 000000...........101.....1010111
#define VFSUB_VF(vd, vs2, rs1, vm)        EMIT(R_type(0b0000100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 000010...........101.....1010111
#define VFMIN_VF(vd, vs2, rs1, vm)        EMIT(R_type(0b0001000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 000100...........101.....1010111
#define VFMAX_VF(vd, vs2, rs1, vm)        EMIT(R_type(0b0001100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 000110...........101.....1010111
#define VFSGNJ_VF(vd, vs2, rs1, vm)       EMIT(R_type(0b0010000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 001000...........101.....1010111
#define VFSGNJN_VF(vd, vs2, rs1, vm)      EMIT(R_type(0b0010010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 001001...........101.....1010111
#define VFSGNJX_VF(vd, vs2, rs1, vm)      EMIT(R_type(0b0010100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 001010...........101.....1010111
#define VFSLIDE1UP_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b0011100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 001110...........101.....1010111
#define VFSLIDE1DOWN_VF(vd, vs2, rs1, vm) EMIT(R_type(0b0011110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 001111...........101.....1010111

#define VFMV_S_F(vd, rs1) EMIT(I_type((cpuext.xtheadvector ? 0b001101100000 : 0b010000100000), rs1, 0b101, vd, 0b1010111)) // 010000100000.....101.....1010111
#define VFMV_V_F(vd, rs1) EMIT(I_type(0b010111100000, rs1, 0b101, vd, 0b1010111))                                        // 010111100000.....101.....1010111

#define VFMERGE_VFM(vd, vs2, rs1) EMIT(R_type(0b0101110, vs2, rs1, 0b101, vd, 0b1010111)) // 0101110..........101.....1010111

#define VMFEQ_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b0110000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 011000...........101.....1010111
#define VMFLE_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b0110010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 011001...........101.....1010111
#define VMFLT_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b0110110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 011011...........101.....1010111
#define VMFNE_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b0111000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 011100...........101.....1010111
#define VMFGT_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b0111010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 011101...........101.....1010111
#define VMFGE_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b0111110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 011111...........101.....1010111
#define VFDIV_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b1000000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 100000...........101.....1010111
#define VFRDIV_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1000010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 100001...........101.....1010111
#define VFMUL_VF(vd, vs2, rs1, vm)    EMIT(R_type(0b1001000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 100100...........101.....1010111
#define VFRSUB_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1001110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 100111...........101.....1010111
#define VFMADD_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1010000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101000...........101.....1010111
#define VFNMADD_VF(vd, vs2, rs1, vm)  EMIT(R_type(0b1010010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101001...........101.....1010111
#define VFMSUB_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1010100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101010...........101.....1010111
#define VFNMSUB_VF(vd, vs2, rs1, vm)  EMIT(R_type(0b1010110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101011...........101.....1010111
#define VFMACC_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1011000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101100...........101.....1010111
#define VFNMACC_VF(vd, vs2, rs1, vm)  EMIT(R_type(0b1011010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101101...........101.....1010111
#define VFMSAC_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1011100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101110...........101.....1010111
#define VFNMSAC_VF(vd, vs2, rs1, vm)  EMIT(R_type(0b1011110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 101111...........101.....1010111
#define VFWADD_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1100000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 110000...........101.....1010111
#define VFWSUB_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1100100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 110010...........101.....1010111
#define VFWADD_WF(vd, vs2, rs1, vm)   EMIT(R_type(0b1101000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 110100...........101.....1010111
#define VFWSUB_WF(vd, vs2, rs1, vm)   EMIT(R_type(0b1101100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 110110...........101.....1010111
#define VFWMUL_VF(vd, vs2, rs1, vm)   EMIT(R_type(0b1110000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 111000...........101.....1010111
#define VFWMACC_VF(vd, vs2, rs1, vm)  EMIT(R_type(0b1111000 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 111100...........101.....1010111
#define VFWNMACC_VF(vd, vs2, rs1, vm) EMIT(R_type(0b1111010 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 111101...........101.....1010111
#define VFWMSAC_VF(vd, vs2, rs1, vm)  EMIT(R_type(0b1111100 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 111110...........101.....1010111
#define VFWNMSAC_VF(vd, vs2, rs1, vm) EMIT(R_type(0b1111110 | (vm), vs2, rs1, 0b101, vd, 0b1010111)) // 111111...........101.....1010111

//  OPFVV
#define VFADD_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0000000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000000...........001.....1010111
#define VFREDUSUM_VS(vd, vs2, vs1, vm) EMIT(R_type(0b0000010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000001...........001.....1010111
#define VFSUB_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0000100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000010...........001.....1010111
#define VFREDOSUM_VS(vd, vs2, vs1, vm) EMIT(R_type(0b0000110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000011...........001.....1010111
#define VFMIN_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0001000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000100...........001.....1010111
#define VFREDMIN_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0001010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000101...........001.....1010111
#define VFMAX_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0001100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000110...........001.....1010111
#define VFREDMAX_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0001110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 000111...........001.....1010111
#define VFSGNJ_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b0010000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 001000...........001.....1010111
#define VFSGNJN_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b0010010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 001001...........001.....1010111
#define VFSGNJX_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b0010100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 001010...........001.....1010111

#define VFMV_F_S(rd, vs2) EMIT(R_type((cpuext.xtheadvector ? 0b0011001 : 0b0100001), vs2, 0b00000, 0b001, rd, 0b1010111)) // 0100001.....00000001.....1010111

#define VMFEQ_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b0110000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 011000...........001.....1010111
#define VMFLE_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b0110010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 011001...........001.....1010111
#define VMFLT_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b0110110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 011011...........001.....1010111
#define VMFNE_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b0111000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 011100...........001.....1010111
#define VFDIV_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1000000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 100000...........001.....1010111
#define VFMUL_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1001000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 100100...........001.....1010111
#define VFMADD_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1010000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101000...........001.....1010111
#define VFNMADD_VV(vd, vs2, vs1, vm) EMIT(R_type(0b1010010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101001...........001.....1010111
#define VFMSUB_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1010100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101010...........001.....1010111
#define VFNMSUB_VV(vd, vs2, vs1, vm) EMIT(R_type(0b1010110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101011...........001.....1010111
#define VFMACC_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1011000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101100...........001.....1010111
#define VFNMACC_VV(vd, vs2, vs1, vm) EMIT(R_type(0b1011010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101101...........001.....1010111
#define VFMSAC_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1011100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101110...........001.....1010111
#define VFNMSAC_VV(vd, vs2, vs1, vm) EMIT(R_type(0b1011110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 101111...........001.....1010111

#define VFCVT_XU_F_V(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b00000, 0b001, vd, 0b1010111)) // 010010......00000001.....1010111
#define VFCVT_X_F_V(vd, vs2, vm)       EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b00001, 0b001, vd, 0b1010111)) // 010010......00001001.....1010111
#define VFCVT_F_XU_V(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b00010, 0b001, vd, 0b1010111)) // 010010......00010001.....1010111
#define VFCVT_F_X_V(vd, vs2, vm)       EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b00011, 0b001, vd, 0b1010111)) // 010010......00011001.....1010111
#define VFCVT_RTZ_XU_F_V(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b00110, 0b001, vd, 0b1010111)) // 010010......00110001.....1010111
#define VFCVT_RTZ_X_F_V(vd, vs2, vm)   EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b00111, 0b001, vd, 0b1010111)) // 010010......00111001.....1010111
#define VFWCVT_XU_F_V(vd, vs2, vm)     EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01000, 0b001, vd, 0b1010111)) // 010010......01000001.....1010111
#define VFWCVT_X_F_V(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01001, 0b001, vd, 0b1010111)) // 010010......01001001.....1010111
#define VFWCVT_F_XU_V(vd, vs2, vm)     EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01010, 0b001, vd, 0b1010111)) // 010010......01010001.....1010111
#define VFWCVT_F_X_V(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01011, 0b001, vd, 0b1010111)) // 010010......01011001.....1010111
#define VFWCVT_F_F_V(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01100, 0b001, vd, 0b1010111)) // 010010......01100001.....1010111
#define VFWCVT_RTZ_XU_F_V(vd, vs2, vm) EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01110, 0b001, vd, 0b1010111)) // 010010......01110001.....1010111
#define VFWCVT_RTZ_X_F_V(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b01111, 0b001, vd, 0b1010111)) // 010010......01111001.....1010111
#define VFNCVT_XU_F_W(vd, vs2, vm)     EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10000, 0b001, vd, 0b1010111)) // 010010......10000001.....1010111
#define VFNCVT_X_F_W(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10001, 0b001, vd, 0b1010111)) // 010010......10001001.....1010111
#define VFNCVT_F_XU_W(vd, vs2, vm)     EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10010, 0b001, vd, 0b1010111)) // 010010......10010001.....1010111
#define VFNCVT_F_X_W(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10011, 0b001, vd, 0b1010111)) // 010010......10011001.....1010111
#define VFNCVT_F_F_W(vd, vs2, vm)      EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10100, 0b001, vd, 0b1010111)) // 010010......10100001.....1010111
#define VFNCVT_ROD_F_F_W(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10101, 0b001, vd, 0b1010111)) // 010010......10101001.....1010111
#define VFNCVT_RTZ_XU_F_W(vd, vs2, vm) EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10110, 0b001, vd, 0b1010111)) // 010010......10110001.....1010111
#define VFNCVT_RTZ_X_F_W(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b1000100 : 0b0100100) | (vm), vs2, 0b10111, 0b001, vd, 0b1010111)) // 010010......10111001.....1010111
#define VFSQRT_V(vd, vs2, vm)          EMIT(R_type((cpuext.xtheadvector ? 0b1000110 : 0b0100110) | (vm), vs2, 0b00000, 0b001, vd, 0b1010111)) // 010011......00000001.....1010111
#define VFCLASS_V(vd, vs2, vm)         EMIT(R_type((cpuext.xtheadvector ? 0b1000110 : 0b0100110) | (vm), vs2, 0b10000, 0b001, vd, 0b1010111)) // 010011......10000001.....1010111

#define VFRSQRT7_V(vd, vs2, vm) EMIT(R_type(0b0100110 | (vm), vs2, 0b00100, 0b001, vd, 0b1010111)) // 010011......00100001.....1010111
#define VFREC7_V(vd, vs2, vm)   EMIT(R_type(0b0100110 | (vm), vs2, 0b00101, 0b001, vd, 0b1010111)) // 010011......00101001.....1010111

#define VFWADD_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1100000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 110000...........001.....1010111
#define VFWREDUSUM_VS(vd, vs2, vs1, vm) EMIT(R_type(0b1100010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 110001...........001.....1010111
#define VFWSUB_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1100100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 110010...........001.....1010111
#define VFWREDOSUM_VS(vd, vs2, vs1, vm) EMIT(R_type(0b1100110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 110011...........001.....1010111
#define VFWADD_WV(vd, vs2, vs1, vm)     EMIT(R_type(0b1101000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 110100...........001.....1010111
#define VFWSUB_WV(vd, vs2, vs1, vm)     EMIT(R_type(0b1101100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 110110...........001.....1010111
#define VFWMUL_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1110000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 111000...........001.....1010111
#define VFWMACC_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1111000 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 111100...........001.....1010111
#define VFWNMACC_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1111010 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 111101...........001.....1010111
#define VFWMSAC_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1111100 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 111110...........001.....1010111
#define VFWNMSAC_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1111110 | (vm), vs2, vs1, 0b001, vd, 0b1010111)) // 111111...........001.....1010111

//  OPIVX
#define VADD_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0000000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000000...........100.....1010111
#define VSUB_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0000100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000010...........100.....1010111
#define VRSUB_VX(vd, vs2, rs1, vm)      EMIT(R_type(0b0000110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000011...........100.....1010111
#define VMINU_VX(vd, vs2, rs1, vm)      EMIT(R_type(0b0001000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000100...........100.....1010111
#define VMIN_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0001010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000101...........100.....1010111
#define VMAXU_VX(vd, vs2, rs1, vm)      EMIT(R_type(0b0001100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000110...........100.....1010111
#define VMAX_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0001110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 000111...........100.....1010111
#define VAND_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0010010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 001001...........100.....1010111
#define VOR_VX(vd, vs2, rs1, vm)        EMIT(R_type(0b0010100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 001010...........100.....1010111
#define VXOR_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0010110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 001011...........100.....1010111
#define VRGATHER_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0011000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 001100...........100.....1010111
#define VSLIDEUP_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0011100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 001110...........100.....1010111
#define VSLIDEDOWN_VX(vd, vs2, rs1, vm) EMIT(R_type(0b0011110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 001111...........100.....1010111

#define VADC_VXM(vd, vs2, rs1)   EMIT(R_type((0b0100000 | cpuext.xtheadvector), vs2, rs1, 0b100, vd, 0b1010111)) // 0100000..........100.....1010111
#define VMADC_VXM(vd, vs2, rs1)  EMIT(R_type(0b0100010, vs2, rs1, 0b100, vd, 0b1010111))                         // 0100010..........100.....1010111
#define VMADC_VX(vd, vs2, rs1)   EMIT(R_type(0b0100011, vs2, rs1, 0b100, vd, 0b1010111))                         // 0100011..........100.....1010111
#define VSBC_VXM(vd, vs2, rs1)   EMIT(R_type((0b0100100 | cpuext.xtheadvector), vs2, rs1, 0b100, vd, 0b1010111)) // 0100100..........100.....1010111
#define VMSBC_VXM(vd, vs2, rs1)  EMIT(R_type(0b0100110, vs2, rs1, 0b100, vd, 0b1010111))                         // 0100110..........100.....1010111
#define VMSBC_VX(vd, vs2, rs1)   EMIT(R_type(0b0100111, vs2, rs1, 0b100, vd, 0b1010111))                         // 0100111..........100.....1010111
#define VMERGE_VXM(vd, vs2, rs1) EMIT(R_type(0b0101110, vs2, rs1, 0b100, vd, 0b1010111))                         // 0101110..........100.....1010111

#define VMV_V_X(vd, rs1) EMIT(I_type(0b010111100000, rs1, 0b100, vd, 0b1010111)) // 010111100000.....100.....1010111

#define VMSEQ_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0110000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011000...........100.....1010111
#define VMSNE_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0110010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011001...........100.....1010111
#define VMSLTU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b0110100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011010...........100.....1010111
#define VMSLT_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0110110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011011...........100.....1010111
#define VMSLEU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b0111000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011100...........100.....1010111
#define VMSLE_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0111010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011101...........100.....1010111
#define VMSGTU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b0111100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011110...........100.....1010111
#define VMSGT_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0111110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 011111...........100.....1010111
#define VSADDU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b1000000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 100000...........100.....1010111
#define VSADD_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1000010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 100001...........100.....1010111
#define VSSUBU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b1000100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 100010...........100.....1010111
#define VSSUB_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1000110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 100011...........100.....1010111
#define VSLL_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1001010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 100101...........100.....1010111
#define VSMUL_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1001110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 100111...........100.....1010111
#define VSRL_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1010000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101000...........100.....1010111
#define VSRA_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1010010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101001...........100.....1010111
#define VSSRL_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1010100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101010...........100.....1010111
#define VSSRA_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1010110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101011...........100.....1010111
#define VNSRL_WX(vd, vs2, rs1, vm)   EMIT(R_type(0b1011000 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101100...........100.....1010111
#define VNSRA_WX(vd, vs2, rs1, vm)   EMIT(R_type(0b1011010 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101101...........100.....1010111
#define VNCLIPU_WX(vd, vs2, rs1, vm) EMIT(R_type(0b1011100 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101110...........100.....1010111
#define VNCLIP_WX(vd, vs2, rs1, vm)  EMIT(R_type(0b1011110 | (vm), vs2, rs1, 0b100, vd, 0b1010111)) // 101111...........100.....1010111

//  OPIVV
#define VADD_VV(vd, vs2, vs1, vm)         EMIT(R_type(0b0000000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 000000...........000.....1010111
#define VSUB_VV(vd, vs2, vs1, vm)         EMIT(R_type(0b0000100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 000010...........000.....1010111
#define VMINU_VV(vd, vs2, vs1, vm)        EMIT(R_type(0b0001000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 000100...........000.....1010111
#define VMIN_VV(vd, vs2, vs1, vm)         EMIT(R_type(0b0001010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 000101...........000.....1010111
#define VMAXU_VV(vd, vs2, vs1, vm)        EMIT(R_type(0b0001100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 000110...........000.....1010111
#define VMAX_VV(vd, vs2, vs1, vm)         EMIT(R_type(0b0001110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 000111...........000.....1010111
#define VAND_VV(vd, vs2, vs1, vm)         EMIT(R_type(0b0010010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 001001...........000.....1010111
#define VOR_VV(vd, vs2, vs1, vm)          EMIT(R_type(0b0010100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 001010...........000.....1010111
#define VXOR_VV(vd, vs2, vs1, vm)         EMIT(R_type(0b0010110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 001011...........000.....1010111
#define VRGATHER_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0011000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 001100...........000.....1010111
#define VRGATHEREI16_VV(vd, vs2, vs1, vm) EMIT(R_type(0b0011100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 001110...........000.....1010111

#define VADC_VVM(vd, vs2, vs1)   EMIT(R_type((0b0100000 | cpuext.xtheadvector), vs2, vs1, 0b000, vd, 0b1010111)) // 0100000..........000.....1010111
#define VMADC_VVM(vd, vs2, vs1)  EMIT(R_type(0b0100010, vs2, vs1, 0b000, vd, 0b1010111))                         // 0100010..........000.....1010111
#define VMADC_VV(vd, vs2, vs1)   EMIT(R_type(0b0100011, vs2, vs1, 0b000, vd, 0b1010111))                         // 0100011..........000.....1010111
#define VSBC_VVM(vd, vs2, vs1)   EMIT(R_type((0b0100100 | cpuext.xtheadvector), vs2, vs1, 0b000, vd, 0b1010111)) // 0100100..........000.....1010111
#define VMSBC_VVM(vd, vs2, vs1)  EMIT(R_type(0b0100110, vs2, vs1, 0b000, vd, 0b1010111))                         // 0100110..........000.....1010111
#define VMSBC_VV(vd, vs2, vs1)   EMIT(R_type(0b0100111, vs2, vs1, 0b000, vd, 0b1010111))                         // 0100111..........000.....1010111
#define VMERGE_VVM(vd, vs2, vs1) EMIT(R_type(0b0101110, vs2, vs1, 0b000, vd, 0b1010111))                         // 0101110..........000.....1010111

#define VMV_V_V(vd, vs1) EMIT(I_type(0b010111100000, vs1, 0b000, vd, 0b1010111)) // 010111100000.....000.....1010111

#define VMSEQ_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0110000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 011000...........000.....1010111
#define VMSNE_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0110010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 011001...........000.....1010111
#define VMSLTU_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b0110100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 011010...........000.....1010111
#define VMSLT_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0110110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 011011...........000.....1010111
#define VMSLEU_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b0111000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 011100...........000.....1010111
#define VMSLE_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b0111010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 011101...........000.....1010111
#define VSADDU_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1000000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 100000...........000.....1010111
#define VSADD_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1000010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 100001...........000.....1010111
#define VSSUBU_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1000100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 100010...........000.....1010111
#define VSSUB_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1000110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 100011...........000.....1010111
#define VSLL_VV(vd, vs2, vs1, vm)      EMIT(R_type(0b1001010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 100101...........000.....1010111
#define VSMUL_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1001110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 100111...........000.....1010111
#define VSRL_VV(vd, vs2, vs1, vm)      EMIT(R_type(0b1010000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101000...........000.....1010111
#define VSRA_VV(vd, vs2, vs1, vm)      EMIT(R_type(0b1010010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101001...........000.....1010111
#define VSSRL_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1010100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101010...........000.....1010111
#define VSSRA_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1010110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101011...........000.....1010111
#define VNSRL_WV(vd, vs2, vs1, vm)     EMIT(R_type(0b1011000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101100...........000.....1010111
#define VNSRA_WV(vd, vs2, vs1, vm)     EMIT(R_type(0b1011010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101101...........000.....1010111
#define VNCLIPU_WV(vd, vs2, vs1, vm)   EMIT(R_type(0b1011100 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101110...........000.....1010111
#define VNCLIP_WV(vd, vs2, vs1, vm)    EMIT(R_type(0b1011110 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 101111...........000.....1010111
#define VWREDSUMU_VS(vd, vs2, vs1, vm) EMIT(R_type(0b1100000 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 110000...........000.....1010111
#define VWREDSUM_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b1100010 | (vm), vs2, vs1, 0b000, vd, 0b1010111)) // 110001...........000.....1010111

//  OPIVI
#define VADD_VI(vd, vs2, simm5, vm)       EMIT(R_type(0b0000000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 000000...........011.....1010111
#define VRSUB_VI(vd, vs2, simm5, vm)      EMIT(R_type(0b0000110 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 000011...........011.....1010111
#define VAND_VI(vd, vs2, simm5, vm)       EMIT(R_type(0b0010010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 001001...........011.....1010111
#define VOR_VI(vd, vs2, simm5, vm)        EMIT(R_type(0b0010100 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 001010...........011.....1010111
#define VXOR_VI(vd, vs2, simm5, vm)       EMIT(R_type(0b0010110 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 001011...........011.....1010111
#define VRGATHER_VI(vd, vs2, simm5, vm)   EMIT(R_type(0b0011000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 001100...........011.....1010111
#define VSLIDEUP_VI(vd, vs2, simm5, vm)   EMIT(R_type(0b0011100 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 001110...........011.....1010111
#define VSLIDEDOWN_VI(vd, vs2, simm5, vm) EMIT(R_type(0b0011110 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 001111...........011.....1010111

#define VADC_VIM(vd, vs2, simm5)   EMIT(R_type((0b0100000 | cpuext.xtheadvector), vs2, simm5, 0b011, vd, 0b1010111)) // 0100000..........011.....1010111
#define VMADC_VIM(vd, vs2, simm5)  EMIT(R_type(0b0100010, vs2, simm5, 0b011, vd, 0b1010111))                         // 0100010..........011.....1010111
#define VMADC_VI(vd, vs2, simm5)   EMIT(R_type(0b0100011, vs2, simm5, 0b011, vd, 0b1010111))                         // 0100011..........011.....1010111
#define VMERGE_VIM(vd, vs2, simm5) EMIT(R_type(0b0101110, vs2, simm5, 0b011, vd, 0b1010111))                         // 0101110..........011.....1010111

#define VMV_V_I(vd, simm5) EMIT(I_type(0b010111100000, simm5, 0b011, vd, 0b1010111)) // 010111100000.....011.....1010111

#define VMSEQ_VI(vd, vs2, simm5, vm)  EMIT(R_type(0b0110000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 011000...........011.....1010111
#define VMSNE_VI(vd, vs2, simm5, vm)  EMIT(R_type(0b0110010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 011001...........011.....1010111
#define VMSLEU_VI(vd, vs2, simm5, vm) EMIT(R_type(0b0111000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 011100...........011.....1010111
#define VMSLE_VI(vd, vs2, simm5, vm)  EMIT(R_type(0b0111010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 011101...........011.....1010111
#define VMSGTU_VI(vd, vs2, simm5, vm) EMIT(R_type(0b0111100 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 011110...........011.....1010111
#define VMSGT_VI(vd, vs2, simm5, vm)  EMIT(R_type(0b0111110 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 011111...........011.....1010111

#define VSADDU_VI(vd, vs2, simm5, vm)  EMIT(R_type(0b1000000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 100000...........011.....1010111
#define VSADD_VI(vd, vs2, simm5, vm)   EMIT(R_type(0b1000010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 100001...........011.....1010111
#define VSLL_VI(vd, vs2, simm5, vm)    EMIT(R_type(0b1001010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 100101...........011.....1010111
#define VSRL_VI(vd, vs2, simm5, vm)    EMIT(R_type(0b1010000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101000...........011.....1010111
#define VSRA_VI(vd, vs2, simm5, vm)    EMIT(R_type(0b1010010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101001...........011.....1010111
#define VSSRL_VI(vd, vs2, simm5, vm)   EMIT(R_type(0b1010100 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101010...........011.....1010111
#define VSSRA_VI(vd, vs2, simm5, vm)   EMIT(R_type(0b1010110 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101011...........011.....1010111
#define VNSRL_WI(vd, vs2, simm5, vm)   EMIT(R_type(0b1011000 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101100...........011.....1010111
#define VNSRA_WI(vd, vs2, simm5, vm)   EMIT(R_type(0b1011010 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101101...........011.....1010111
#define VNCLIPU_WI(vd, vs2, simm5, vm) EMIT(R_type(0b1011100 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101110...........011.....1010111
#define VNCLIP_WI(vd, vs2, simm5, vm)  EMIT(R_type(0b1011110 | (vm), vs2, simm5, 0b011, vd, 0b1010111)) // 101111...........011.....1010111

#define VMV1R_V(vd, vs2) EMIT(R_type(0b1001111, vs2, 0b00000, 0b011, vd, 0b1010111)) // 1001111.....00000011.....1010111
#define VMV2R_V(vd, vs2) EMIT(R_type(0b1001111, vs2, 0b00001, 0b011, vd, 0b1010111)) // 1001111.....00001011.....1010111
#define VMV4R_V(vd, vs2) EMIT(R_type(0b1001111, vs2, 0b00011, 0b011, vd, 0b1010111)) // 1001111.....00011011.....1010111
#define VMV8R_V(vd, vs2) EMIT(R_type(0b1001111, vs2, 0b00111, 0b011, vd, 0b1010111)) // 1001111.....00111011.....1010111

//  OPMVV
#define VREDSUM_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0000000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000000...........010.....1010111
#define VREDAND_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0000010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000001...........010.....1010111
#define VREDOR_VS(vd, vs2, vs1, vm)   EMIT(R_type(0b0000100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000010...........010.....1010111
#define VREDXOR_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0000110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000011...........010.....1010111
#define VREDMINU_VS(vd, vs2, vs1, vm) EMIT(R_type(0b0001000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000100...........010.....1010111
#define VREDMIN_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0001010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000101...........010.....1010111
#define VREDMAXU_VS(vd, vs2, vs1, vm) EMIT(R_type(0b0001100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000110...........010.....1010111
#define VREDMAX_VS(vd, vs2, vs1, vm)  EMIT(R_type(0b0001110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 000111...........010.....1010111
#define VAADD_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b0010010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 001001...........010.....1010111
#define VASUB_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b0010110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 001011...........010.....1010111
// Warning, no unsigned edition in Xtheadvector
#define VAADDU_VV(vd, vs2, vs1, vm) EMIT(R_type(0b0010000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 001000...........010.....1010111
#define VASUBU_VV(vd, vs2, vs1, vm) EMIT(R_type(0b0010100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 001010...........010.....1010111

// Warning: zero-extended on xtheadvector!
#define VMV_X_S(rd, vs2) EMIT(R_type((cpuext.xtheadvector ? 0b0011001 : 0b0100001), vs2, 0b00000, 0b010, rd, 0b1010111)) // 0100001.....00000010.....1010111

// Warning: xtheadvector only
#define VEXT_X_V(rd, vs2, rs1) EMIT(R_type((cpuext.xtheadvector ? 0b0011001 : 0b0100001), vs2, rs1, 0b010, rd, 0b1010111))

//  Vector Integer Extension Instructions
//  https://github.com/riscv/riscv-v-spec/blob/e49574c92b072fd4d71e6cb20f7e8154de5b83fe/v-spec.adoc#123-vector-integer-extension

#define VZEXT_VF8(vd, vs2, vm) EMIT(R_type(0b0100100 | (vm), vs2, 0b00010, 0b010, vd, 0b1010111)) // 010010......00010010.....1010111
#define VSEXT_VF8(vd, vs2, vm) EMIT(R_type(0b0100100 | (vm), vs2, 0b00011, 0b010, vd, 0b1010111)) // 010010......00011010.....1010111
#define VZEXT_VF4(vd, vs2, vm) EMIT(R_type(0b0100100 | (vm), vs2, 0b00100, 0b010, vd, 0b1010111)) // 010010......00100010.....1010111
#define VSEXT_VF4(vd, vs2, vm) EMIT(R_type(0b0100100 | (vm), vs2, 0b00101, 0b010, vd, 0b1010111)) // 010010......00101010.....1010111
#define VZEXT_VF2(vd, vs2, vm) EMIT(R_type(0b0100100 | (vm), vs2, 0b00110, 0b010, vd, 0b1010111)) // 010010......00110010.....1010111
#define VSEXT_VF2(vd, vs2, vm) EMIT(R_type(0b0100100 | (vm), vs2, 0b00111, 0b010, vd, 0b1010111)) // 010010......00111010.....1010111

#define VCOMPRESS_VM(vd, vs2, vs1) EMIT(R_type(0b0101111, vs2, vs1, 0b010, vd, 0b1010111)) // 0101111..........010.....1010111
#define VMANDN_MM(vd, vs2, vs1)    EMIT(R_type(0b0110001, vs2, vs1, 0b010, vd, 0b1010111)) // 0110001..........010.....1010111
#define VMAND_MM(vd, vs2, vs1)     EMIT(R_type(0b0110011, vs2, vs1, 0b010, vd, 0b1010111)) // 0110011..........010.....1010111
#define VMOR_MM(vd, vs2, vs1)      EMIT(R_type(0b0110101, vs2, vs1, 0b010, vd, 0b1010111)) // 0110101..........010.....1010111
#define VMXOR_MM(vd, vs2, vs1)     EMIT(R_type(0b0110111, vs2, vs1, 0b010, vd, 0b1010111)) // 0110111..........010.....1010111
#define VMORN_MM(vd, vs2, vs1)     EMIT(R_type(0b0111001, vs2, vs1, 0b010, vd, 0b1010111)) // 0111001..........010.....1010111
#define VMNAND_MM(vd, vs2, vs1)    EMIT(R_type(0b0111011, vs2, vs1, 0b010, vd, 0b1010111)) // 0111011..........010.....1010111
#define VMNOR_MM(vd, vs2, vs1)     EMIT(R_type(0b0111101, vs2, vs1, 0b010, vd, 0b1010111)) // 0111101..........010.....1010111
#define VMXNOR_MM(vd, vs2, vs1)    EMIT(R_type(0b0111111, vs2, vs1, 0b010, vd, 0b1010111)) // 0111111..........010.....1010111

#define VMSBF_M(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0101000) | (vm), vs2, 0b00001, 0b010, vd, 0b1010111)) // 010100......00001010.....1010111
#define VMSOF_M(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0101000) | (vm), vs2, 0b00010, 0b010, vd, 0b1010111)) // 010100......00010010.....1010111
#define VMSIF_M(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0101000) | (vm), vs2, 0b00011, 0b010, vd, 0b1010111)) // 010100......00011010.....1010111
#define VIOTA_M(vd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0101000) | (vm), vs2, 0b10000, 0b010, vd, 0b1010111)) // 010100......10000010.....1010111
#define VCPOP_M(rd, vs2, vm)  EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0100000) | (vm), vs2, 0b10000, 0b010, rd, 0b1010111)) // 010000......10000010.....1010111
#define VFIRST_M(rd, vs2, vm) EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0100000) | (vm), vs2, 0b10001, 0b010, rd, 0b1010111)) // 010000......10001010.....1010111

#define VID_V(vd, vm) EMIT(R_type((cpuext.xtheadvector ? 0b0101100 : 0b0101000) | (vm), 0b00000, 0b10001, 0b010, vd, 0b1010111)) // 010100.0000010001010.....1010111

#define VDIVU_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1000000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100000...........010.....1010111
#define VDIV_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1000010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100001...........010.....1010111
#define VREMU_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1000100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100010...........010.....1010111
#define VREM_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1000110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100011...........010.....1010111
#define VMULHU_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1001000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100100...........010.....1010111
#define VMUL_VV(vd, vs2, vs1, vm)     EMIT(R_type(0b1001010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100101...........010.....1010111
#define VMULHSU_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1001100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100110...........010.....1010111
#define VMULH_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1001110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 100111...........010.....1010111
#define VMADD_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1010010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 101001...........010.....1010111
#define VNMSUB_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1010110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 101011...........010.....1010111
#define VMACC_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1011010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 101101...........010.....1010111
#define VNMSAC_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1011110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 101111...........010.....1010111
#define VWADDU_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1100000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110000...........010.....1010111
#define VWADD_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1100010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110001...........010.....1010111
#define VWSUBU_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1100100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110010...........010.....1010111
#define VWSUB_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1100110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110011...........010.....1010111
#define VWADDU_WV(vd, vs2, vs1, vm)   EMIT(R_type(0b1101000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110100...........010.....1010111
#define VWADD_WV(vd, vs2, vs1, vm)    EMIT(R_type(0b1101010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110101...........010.....1010111
#define VWSUBU_WV(vd, vs2, vs1, vm)   EMIT(R_type(0b1101100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110110...........010.....1010111
#define VWSUB_WV(vd, vs2, vs1, vm)    EMIT(R_type(0b1101110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 110111...........010.....1010111
#define VWMULU_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1110000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 111000...........010.....1010111
#define VWMULSU_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1110100 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 111010...........010.....1010111
#define VWMUL_VV(vd, vs2, vs1, vm)    EMIT(R_type(0b1110110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 111011...........010.....1010111
#define VWMACCU_VV(vd, vs2, vs1, vm)  EMIT(R_type(0b1111000 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 111100...........010.....1010111
#define VWMACC_VV(vd, vs2, vs1, vm)   EMIT(R_type(0b1111010 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 111101...........010.....1010111
#define VWMACCSU_VV(vd, vs2, vs1, vm) EMIT(R_type(0b1111110 | (vm), vs2, vs1, 0b010, vd, 0b1010111)) // 111111...........010.....1010111

//  OPMVX
#define VAADDU_VX(vd, vs2, rs1, vm)      EMIT(R_type(0b0010000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 001000...........110.....1010111
#define VAADD_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0010010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 001001...........110.....1010111
#define VASUBU_VX(vd, vs2, rs1, vm)      EMIT(R_type(0b0010100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 001010...........110.....1010111
#define VASUB_VX(vd, vs2, rs1, vm)       EMIT(R_type(0b0010110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 001011...........110.....1010111
#define VSLIDE1UP_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b0011100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 001110...........110.....1010111
#define VSLIDE1DOWN_VX(vd, vs2, rs1, vm) EMIT(R_type(0b0011110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 001111...........110.....1010111

// Warning, upper elements will be cleared in xtheadvector!
#define VMV_S_X(vd, rs1) EMIT(I_type((cpuext.xtheadvector ? 0b001101100000 : 0b010000100000), rs1, 0b110, vd, 0b1010111))

#define VDIVU_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1000000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100000...........110.....1010111
#define VDIV_VX(vd, vs2, rs1, vm)     EMIT(R_type(0b1000010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100001...........110.....1010111
#define VREMU_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1000100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100010...........110.....1010111
#define VREM_VX(vd, vs2, rs1, vm)     EMIT(R_type(0b1000110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100011...........110.....1010111
#define VMULHU_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1001000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100100...........110.....1010111
#define VMUL_VX(vd, vs2, rs1, vm)     EMIT(R_type(0b1001010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100101...........110.....1010111
#define VMULHSU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b1001100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100110...........110.....1010111
#define VMULH_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1001110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 100111...........110.....1010111
#define VMADD_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1010010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 101001...........110.....1010111
#define VNMSUB_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1010110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 101011...........110.....1010111
#define VMACC_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1011010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 101101...........110.....1010111
#define VNMSAC_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1011110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 101111...........110.....1010111
#define VWADDU_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1100000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110000...........110.....1010111
#define VWADD_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1100010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110001...........110.....1010111
#define VWSUBU_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1100100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110010...........110.....1010111
#define VWSUB_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1100110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110011...........110.....1010111
#define VWADDU_WX(vd, vs2, rs1, vm)   EMIT(R_type(0b1101000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110100...........110.....1010111
#define VWADD_WX(vd, vs2, rs1, vm)    EMIT(R_type(0b1101010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110101...........110.....1010111
#define VWSUBU_WX(vd, vs2, rs1, vm)   EMIT(R_type(0b1101100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110110...........110.....1010111
#define VWSUB_WX(vd, vs2, rs1, vm)    EMIT(R_type(0b1101110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 110111...........110.....1010111
#define VWMULU_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1110000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111000...........110.....1010111
#define VWMULSU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b1110100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111010...........110.....1010111
#define VWMUL_VX(vd, vs2, rs1, vm)    EMIT(R_type(0b1110110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111011...........110.....1010111
#define VWMACCU_VX(vd, vs2, rs1, vm)  EMIT(R_type(0b1111000 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111100...........110.....1010111
#define VWMACC_VX(vd, vs2, rs1, vm)   EMIT(R_type(0b1111010 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111101...........110.....1010111
#define VWMACCUS_VX(vd, vs2, rs1, vm) EMIT(R_type(0b1111100 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111110...........110.....1010111
#define VWMACCSU_VX(vd, vs2, rs1, vm) EMIT(R_type(0b1111110 | (vm), vs2, rs1, 0b110, vd, 0b1010111)) // 111111...........110.....1010111

#endif //__RV64_EMITTER_H__
