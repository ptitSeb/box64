#ifndef __LA64_EMITTER_H__
#define __LA64_EMITTER_H__
/*
    LA64 Emitter
*/

// LA64 ABI
/*
Name     Alias     Meaning                         saver
---------------------------------------------------------
r0       zero      Zero register                   -
r1       ra        Return address                  Callee
r2       tp        Thread pointer                  -
r3       sp        Stack pointer                   Callee
r4-r5    a0-a1     Function arguments,Return val.  Caller
r6-r11   a2-a7     Function arguments              Caller
r12-r20  t0-t8     Temp registers                  Caller
r21      Reserved  Non-allocatable                 -
r22      fp/s9     Frame pointer/Static register   Callee
r23-31   s0-s8     Static registers                Callee
---------------------------------------------------------
f0-f1    fa0-fa1   Function arguments,Return val.  Caller
f2-f7    fa2-fa7   Function arguments              Caller
f8-f23   ft0-ft15  Temp registers                  Caller
f24-f31  fs0-fs7   Static registers                Callee
*/
/*
 LA64 GPR mapping
 There is no 15 registers free, so split the regs in 2 part
 AX..DI : r12-r19
 R8..R15: r23-r30
 flags in r31
 ip in r20
*/
// x86 Register mapping
#define xRAX    12
#define xRCX    13
#define xRDX    14
#define xRBX    15
#define xRSP    16
#define xRBP    17
#define xRSI    18
#define xRDI    19
#define xR8     23
#define xR9     24
#define xR10    25
#define xR11    26
#define xR12    27
#define xR13    28
#define xR14    29
#define xR15    30
#define xFlags  31
#define xRIP    20
#define xSavedSP 22
// function to move from x86 regs number to LA64 reg number
#define TO_LA64(A) (((A)>7)?((A)+15):((A)+12))
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
#define x1      5
#define x2      6
#define x3      7
#define x4      8
#define x5      9
#define x6      10
// used to clear the upper 32bits
#define xMASK   11
// 32bits version of scratch
#define w1      x1
#define w2      x2
#define w3      x3
#define w4      x4
#define w5      x5
#define w6      x6
// emu is r0
#define xEmu    4
// LA64 RA
#define xRA     1
#define ra      xRA
// LA64 SP
#define xSP     3
// RV64 args
#define A0 4
#define A1 5
#define A2 6
#define A3 7
#define A4 8
#define A5 9
#define A6 10
#define A7 11
// xZR regs
#define xZR     0
#define wZR     xZR
#define r0      xZR

#define fcc0 0
#define fcc1 1
#define fcc2 2
#define fcc3 3
#define fcc4 4
#define fcc5 5
#define fcc6 6
#define fcc7 7

#define cAF  0x0
#define cUN  0x8
#define cEQ  0x4
#define cUEQ 0xC
#define cLT  0x2
#define cULT 0xA
#define cLE  0x6
#define cULE 0xE
#define cNE  0x10
#define cOR  0x14
#define cUNE 0x18
#define sAF  0x1
#define sUN  0x9
#define sEQ  0x5
#define sUEQ 0xD
#define sLT  0x3
#define sULT 0xB
#define sLE  0x7
#define sULE 0xF
#define sNE  0x11
#define sOR  0x15
#define sUNE 0x19

#define FCSR0 0
#define FCSR1 1
#define FCSR2 2
#define FCSR3 3

#define FR_V 28
#define FR_Z 27
#define FR_O 26
#define FR_U 25
#define FR_I 24

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
#define type_1RI13(opc, imm13, rd)       ((opc) << 18 | ((imm13) & 0x1FFFF) << 5 | (rd))
#define type_2RI3(opc, imm3, rj, rd)     ((opc) << 13 | ((imm3)  & 0x7 )  << 10 | (rj) << 5 | (rd))
#define type_2RI4(opc, imm4, rj, rd)     ((opc) << 14 | ((imm4)  & 0xF )  << 10 | (rj) << 5 | (rd))
#define type_2RI5(opc, imm5, rj, rd)     ((opc) << 15 | ((imm5)  & 0x1F)  << 10 | (rj) << 5 | (rd))
#define type_2RI6(opc, imm6, rj, rd)     ((opc) << 16 | ((imm6)  & 0x3F)  << 10 | (rj) << 5 | (rd))

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
#define ALSL_W(rd, rj, rk, imm) EMIT(type_3RI2(0b000000000000010, (imm - 1), rk, rj, rd))
// tmp = (GR[rj][31:0] << imm) + GR[rk][31:0]
// GR[rd] = ZeroExtend(tmp[31:0], GRLEN)
#define ALSL_WU(rd, rj, rk, imm) EMIT(type_3RI2(0b000000000000011, (imm - 1), rk, rj, rd))
// tmp = (GR[rj][63:0] << imm) + GR[rk][63:0]
// GR[rd] = tmp[63:0]
#define ALSL_D(rd, rj, rk, imm) EMIT(type_3RI2(0b000000000010110, (imm - 1), rk, rj, rd))

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

// rd = rs1 == 0
#define SEQZ(rd, rs1) SLTUI(rd, rs1, 1)
// rd = rs1 != 0
#define SNEZ(rd, rs1) SLTU(rd, xZR, rs1)

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

#define LLxw(rd, rj, imm) EMIT(type_2RI14(0b00100000 | (rex.w ? 0b10 : 0b00), imm >> 2, rj, rd))
#define SCxw(rd, rj, imm) EMIT(type_2RI14(0b00100001 | (rex.w ? 0b10 : 0b00), imm >> 2, rj, rd))

// DBAR hint
#define DBAR(hint) EMIT(type_hint(0b00111000011100100, hint))

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
#define SLLIxw(rd, rs1, imm)      \
    do {                          \
        if (rex.w) {              \
            SLLI_D(rd, rs1, imm); \
        } else {                  \
            SLLI_W(rd, rs1, imm); \
            ZEROUP(rd);           \
        }                         \
    } while (0)
// Shift Right Logical Immediate
#define SRLIxw(rd, rs1, imm)          \
    do {                              \
        if (rex.w) {                  \
            SRLI_D(rd, rs1, imm);     \
        } else {                      \
            SRLI_W(rd, rs1, imm);     \
            if (imm == 0) ZEROUP(rd); \
        }                             \
    } while (0)

// Shift Right Arithmetic Immediate
#define SRAIxw(rd, rs1, imm)      \
    do {                          \
        if (rex.w) {              \
            SRAI_D(rd, rs1, imm); \
        } else {                  \
            SRAI_W(rd, rs1, imm); \
            ZEROUP(rd);           \
        }                         \
    } while (0)

#define ROTRIxw(rd, rs1, imm)      \
    do {                           \
        if (rex.w) {               \
            ROTRI_D(rd, rs1, imm); \
        } else {                   \
            ROTRI_W(rd, rs1, imm); \
            ZEROUP(rd);            \
        }                          \
    } while (0)

// rd = rj + (rk << imm6)
#define ADDSL(rd, rs1, rs2, imm6, scratch) \
    do {                                   \
        if (!(imm6)) {                     \
            ADD_D(rd, rs1, rs2);           \
        } else {                           \
            SLLI_D(scratch, rs2, imm6);    \
            ADD_D(rd, rs1, scratch);       \
        }                                  \
    } while (0)

#define SEXT_W(rd, rs1) SLLI_W(rd, rs1, 0)

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
#define DIV_W(rd, rj, rk) EMIT(type_3R(0b00000000001000000, rk, rj, rd))

// quotient = unsigned(GR[rj][31:0]) / unsigned(GR[rk][31:0])
// GR[rd] = SignExtend(quotient[31:0], GRLEN)
#define DIV_WU(rd, rj, rk) EMIT(type_3R(0b00000000001000010, rk, rj, rd))

// remainder = signed(GR[rj][31:0]) % signed(GR[rk][31:0])
// GR[rd] = SignExtend(remainder[31:0], GRLEN)
#define MOD_W(rd, rj, rk) EMIT(type_3R(0b00000000001000001, rk, rj, rd))

// remainder = unsigned(GR[rj][31:0]) % unsigned(GR[rk][31:0])
// GR[rd] = SignExtend(remainder[31:0], GRLEN)
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
#define BSTRINS_W(rd, rj, msbw5, lsbw5) EMIT(type_2RI12(0b0000000001, 0b100000000000 | (msbw5 & 0x1F) << 6 | (lsbw5 & 0x1F), rj, rd))

// GR[rd][63:msbd+1] = GR[rd][63:msbd+1]
// GR[rd][msbd:lsbd] = GR[rj][msbd-lsbd:0]
// GR[rd][lsbd-1:0] = GR[rd][lsbd-1:0]
#define BSTRINS_D(rd, rj, msbd6, lsbd6) EMIT(type_2RI12(0b0000000010, (msbd6 & 0x3F) << 6 | (lsbd6 & 0x3F), rj, rd))

// bstr32[31:0] = ZeroExtend(GR[rj][msbw:lsbw], 32)
// GR[rd] = SignExtend(bstr32[31:0], GRLEN)
#define BSTRPICK_W(rd, rj, msbw5, lsbw5) EMIT(type_2RI12(0b0000000001, 0b100000100000 | (msbw5 & 0x1F) << 6 | (lsbw5 & 0x1F), rj, rd))

// GR[rd] = ZeroExtend(GR[rj][msbd:lsbd], 64)
#define BSTRPICK_D(rd, rj, msbd6, lsbd6) EMIT(type_2RI12(0b0000000011, (msbd6 & 0x3F) << 6 | (lsbd6 & 0x3F), rj, rd))

// ZERO the upper part
#define ZEROUP(rd) BSTRINS_D(rd, xZR, 63, 32);

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


// GR[rd] = SignExtend(GR[rj][7:0], GRLEN)
#define EXT_W_B(rd, rj) EMIT(type_2R(0b0000000000000000010111, rj, rd))

// GR[rd] = SignExtend(GR[rj][15:0], GRLEN)
#define EXT_W_H(rd, rj) EMIT(type_2R(0b0000000000000000010110, rj, rd))

// if GR[rj] == GR[rd]:
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BEQ(rj, rd, imm18) EMIT(type_2RI16(0b010110, ((imm18)>>2), rj, rd))
// if GR[rj] != GR[rd]:
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BNE(rj, rd, imm18) EMIT(type_2RI16(0b010111, ((imm18)>>2), rj, rd))
// if signed(GR[rj]) < signed(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BLT(rj, rd, imm18) EMIT(type_2RI16(0b011000, ((imm18)>>2), rj, rd))
// if signed(GR[rj]) >= signed(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BGE(rj, rd, imm18) EMIT(type_2RI16(0b011001, ((imm18)>>2), rj, rd))
// if unsigned(GR[rj]) == unsigned(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BLTU(rj, rd, imm18) EMIT(type_2RI16(0b011010, ((imm18)>>2), rj, rd))
// if unsigned(GR[rj]) == unsigned(GR[rd]):
//     PC = PC + SignExtend({imm16, 2'b0}, GRLEN)
#define BGEU(rj, rd, imm18) EMIT(type_2RI16(0b011011, ((imm18)>>2), rj, rd))

// if GR[rj] == 0:
//     PC = PC + SignExtend({imm21, 2'b0}, GRLEN)
#define BEQZ(rj, imm23) EMIT(type_1RI21(0b010000, ((imm23)>>2), rj))
// if GR[rj] != 0:
//     PC = PC + SignExtend({imm21, 2'b0}, GRLEN)
#define BNEZ(rj, imm23) EMIT(type_1RI21(0b010001, ((imm23) >> 2), rj))

#define BCEQZ(cj, imm23) EMIT(type_1RI21(0b010010, ((imm23)>>2), 0b00000 | cj))
#define BCNEZ(cj, imm23) EMIT(type_1RI21(0b010010, ((imm23)>>2), 0b01000 | cj))

// GR[rd] = PC + 4
// PC = GR[rj] + SignExtend({imm16, 2'b0}, GRLEN)
#define JIRL(rd, rj, imm18) EMIT(type_2RI16(0b010011, ((imm18)>>2), rj, rd))

// PC = GR[rj]
#define BR(rj) JIRL(xZR, rj, 0x0)

// PC = PC + SignExtend({imm26, 2'b0}, GRLEN)
#define B(imm28) EMIT(type_I26(0b010100, ((imm28)>>2)))
#define B__(reg1, reg2, imm28) B(imm28)

#define BEQ_safe(rj, rd, imm)                      \
    if {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BEQ(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BNE(rj, rd, 8);                        \
            B(imm - 4);                            \
        }                                          \
    }                                              \
    while (0)

#define BNE_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BNE(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BEQ(rj, rd, 8);                        \
            B(imm - 4);                            \
        }                                          \
    } while (0)

#define BLT_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BLT(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BGE(rj, rd, 8);                        \
            B(imm - 4);                            \
        }                                          \
    } while (0)

#define BGE_safe(rj, rd, imm)                      \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BGE(rj, rd, imm);                      \
            NOP();                                 \
        } else {                                   \
            BLT(rj, rd, 8);                        \
            B(imm - 4);                            \
        }                                          \
    } while (0)

#define BLTU_safe(rj, rd, imm)                     \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BLTU(rj, rd, imm);                     \
            NOP();                                 \
        } else {                                   \
            BGEU(rj, rd, 8);                       \
            B(imm - 4);                            \
        }                                          \
    } while (0)

#define BGEU_safe(rj, rd, imm)                     \
    do {                                           \
        if ((imm) > -0x20000 && (imm) < 0x20000) { \
            BGEU(rj, rd, imm);                     \
            NOP();                                 \
        } else {                                   \
            BLTU(rj, rd, 8);                       \
            B(imm - 4);                            \
        }                                          \
    } while (0)

#define BEQZ_safe(rj, imm)                         \
    do {                                           \
        if ((imm) > -0x70000 && (imm) < 0x70000) { \
            BEQZ(rj, imm);                         \
            NOP();                                 \
        } else {                                   \
            BNEZ(rj, 8);                           \
            B(imm - 4);                            \
        }                                          \
    } while (0)

#define BNEZ_safe(rj, imm)                         \
    do {                                           \
        if ((imm) > -0x70000 && (imm) < 0x70000) { \
            BNEZ(rj, imm);                         \
            NOP();                                 \
        } else {                                   \
            BEQZ(rj, 8);                           \
            B(imm - 4);                            \
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

#define FLD_D(fd, rj, imm12) EMIT(type_2RI12(0b0010101110, imm12, rj, fd))
#define FLD_S(fd, rj, imm12) EMIT(type_2RI12(0b0010101100, imm12, rj, fd))
#define FST_D(fd, rj, imm12) EMIT(type_2RI12(0b0010101111, imm12, rj, fd))
#define FST_S(fd, rj, imm12) EMIT(type_2RI12(0b0010101101, imm12, rj, fd))

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

#define VADD_B(vd, vj, vk)          EMIT(type_3R(0b01110000000010100, vk, vj, vd))
#define VADD_H(vd, vj, vk)          EMIT(type_3R(0b01110000000010101, vk, vj, vd))
#define VADD_W(vd, vj, vk)          EMIT(type_3R(0b01110000000010110, vk, vj, vd))
#define VADD_D(vd, vj, vk)          EMIT(type_3R(0b01110000000010111, vk, vj, vd))
#define VADD_Q(vd, vj, vk)          EMIT(type_3R(0b01110001001011010, vk, vj, vd))
#define VSUB_B(vd, vj, vk)          EMIT(type_3R(0b01110000000011000, vk, vj, vd))
#define VSUB_H(vd, vj, vk)          EMIT(type_3R(0b01110000000011001, vk, vj, vd))
#define VSUB_W(vd, vj, vk)          EMIT(type_3R(0b01110000000011010, vk, vj, vd))
#define VSUB_D(vd, vj, vk)          EMIT(type_3R(0b01110000000011011, vk, vj, vd))
#define VSUB_Q(vd, vj, vk)          EMIT(type_3R(0b01110001001011011, vk, vj, vd))
#define VSADD_B(vd, vj, vk)         EMIT(type_3R(0b01110000010001100, vk, vj, vd))
#define VSADD_H(vd, vj, vk)         EMIT(type_3R(0b01110000010001101, vk, vj, vd))
#define VSADD_W(vd, vj, vk)         EMIT(type_3R(0b01110000010001110, vk, vj, vd))
#define VSADD_D(vd, vj, vk)         EMIT(type_3R(0b01110000010001111, vk, vj, vd))
#define VSADD_BU(vd, vj, vk)        EMIT(type_3R(0b01110000010010100, vk, vj, vd))
#define VSADD_HU(vd, vj, vk)        EMIT(type_3R(0b01110000010010101, vk, vj, vd))
#define VSADD_WU(vd, vj, vk)        EMIT(type_3R(0b01110000010010110, vk, vj, vd))
#define VSADD_DU(vd, vj, vk)        EMIT(type_3R(0b01110000010010111, vk, vj, vd))
#define VSSUB_B(vd, vj, vk)         EMIT(type_3R(0b01110000010010000, vk, vj, vd))
#define VSSUB_H(vd, vj, vk)         EMIT(type_3R(0b01110000010010001, vk, vj, vd))
#define VSSUB_W(vd, vj, vk)         EMIT(type_3R(0b01110000010010010, vk, vj, vd))
#define VSSUB_D(vd, vj, vk)         EMIT(type_3R(0b01110000010010011, vk, vj, vd))
#define VSSUB_BU(vd, vj, vk)        EMIT(type_3R(0b01110000010011000, vk, vj, vd))
#define VSSUB_HU(vd, vj, vk)        EMIT(type_3R(0b01110000010011001, vk, vj, vd))
#define VSSUB_WU(vd, vj, vk)        EMIT(type_3R(0b01110000010011010, vk, vj, vd))
#define VSSUB_DU(vd, vj, vk)        EMIT(type_3R(0b01110000010011011, vk, vj, vd))
#define VHADDW_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000010101000, vk, vj, vd))
#define VHADDW_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000010101001, vk, vj, vd))
#define VHADDW_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000010101010, vk, vj, vd))
#define VHADDW_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000010101011, vk, vj, vd))
#define VHADDW_HU_BU(vd, vj, vk)    EMIT(type_3R(0b01110000010110000, vk, vj, vd))
#define VHADDW_WU_HU(vd, vj, vk)    EMIT(type_3R(0b01110000010110001, vk, vj, vd))
#define VHADDW_DU_WU(vd, vj, vk)    EMIT(type_3R(0b01110000010110010, vk, vj, vd))
#define VHADDW_QU_DU(vd, vj, vk)    EMIT(type_3R(0b01110000010110011, vk, vj, vd))
#define VHSUBW_H_B(vd, vj, vk)      EMIT(type_3R(0b01110000010101100, vk, vj, vd))
#define VHSUBW_W_H(vd, vj, vk)      EMIT(type_3R(0b01110000010101101, vk, vj, vd))
#define VHSUBW_D_W(vd, vj, vk)      EMIT(type_3R(0b01110000010101110, vk, vj, vd))
#define VHSUBW_Q_D(vd, vj, vk)      EMIT(type_3R(0b01110000010101111, vk, vj, vd))
#define VHSUBW_HU_BU(vd, vj, vk)    EMIT(type_3R(0b01110000010110100, vk, vj, vd))
#define VHSUBW_WU_HU(vd, vj, vk)    EMIT(type_3R(0b01110000010110101, vk, vj, vd))
#define VHSUBW_DU_WU(vd, vj, vk)    EMIT(type_3R(0b01110000010110110, vk, vj, vd))
#define VHSUBW_QU_DU(vd, vj, vk)    EMIT(type_3R(0b01110000010110111, vk, vj, vd))
#define VADDWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000000111100, vk, vj, vd))
#define VADDWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000000111101, vk, vj, vd))
#define VADDWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000000111110, vk, vj, vd))
#define VADDWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000000111111, vk, vj, vd))
#define VADDWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000001000100, vk, vj, vd))
#define VADDWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000001000101, vk, vj, vd))
#define VADDWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000001000110, vk, vj, vd))
#define VADDWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000001000111, vk, vj, vd))
#define VSUBWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000001000000, vk, vj, vd))
#define VSUBWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000001000001, vk, vj, vd))
#define VSUBWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000001000010, vk, vj, vd))
#define VSUBWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000001000011, vk, vj, vd))
#define VSUBWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000001001000, vk, vj, vd))
#define VSUBWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000001001001, vk, vj, vd))
#define VSUBWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000001001010, vk, vj, vd))
#define VSUBWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000001001011, vk, vj, vd))
#define VADDWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000001011100, vk, vj, vd))
#define VADDWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000001011101, vk, vj, vd))
#define VADDWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000001011110, vk, vj, vd))
#define VADDWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000001011111, vk, vj, vd))
#define VADDWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000001100100, vk, vj, vd))
#define VADDWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000001100101, vk, vj, vd))
#define VADDWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000001100110, vk, vj, vd))
#define VADDWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000001100111, vk, vj, vd))
#define VSUBWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000001100000, vk, vj, vd))
#define VSUBWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000001100001, vk, vj, vd))
#define VSUBWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000001100010, vk, vj, vd))
#define VSUBWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000001100011, vk, vj, vd))
#define VSUBWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000001101000, vk, vj, vd))
#define VSUBWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000001101001, vk, vj, vd))
#define VSUBWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000001101010, vk, vj, vd))
#define VSUBWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000001101011, vk, vj, vd))
#define VADDWEV_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110000001111100, vk, vj, vd))
#define VADDWEV_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110000001111101, vk, vj, vd))
#define VADDWEV_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110000001111110, vk, vj, vd))
#define VADDWEV_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110000001111111, vk, vj, vd))
#define VADDWOD_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110000010000000, vk, vj, vd))
#define VADDWOD_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110000010000001, vk, vj, vd))
#define VADDWOD_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110000010000010, vk, vj, vd))
#define VADDWOD_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110000010000011, vk, vj, vd))
#define VAVG_B(vd, vj, vk)          EMIT(type_3R(0b01110000011001000, vk, vj, vd))
#define VAVG_H(vd, vj, vk)          EMIT(type_3R(0b01110000011001001, vk, vj, vd))
#define VAVG_W(vd, vj, vk)          EMIT(type_3R(0b01110000011001010, vk, vj, vd))
#define VAVG_D(vd, vj, vk)          EMIT(type_3R(0b01110000011001011, vk, vj, vd))
#define VAVG_BU(vd, vj, vk)         EMIT(type_3R(0b01110000011001100, vk, vj, vd))
#define VAVG_HU(vd, vj, vk)         EMIT(type_3R(0b01110000011001101, vk, vj, vd))
#define VAVG_WU(vd, vj, vk)         EMIT(type_3R(0b01110000011001110, vk, vj, vd))
#define VAVG_DU(vd, vj, vk)         EMIT(type_3R(0b01110000011001111, vk, vj, vd))
#define VAVGR_B(vd, vj, vk)         EMIT(type_3R(0b01110000011010000, vk, vj, vd))
#define VAVGR_H(vd, vj, vk)         EMIT(type_3R(0b01110000011010001, vk, vj, vd))
#define VAVGR_W(vd, vj, vk)         EMIT(type_3R(0b01110000011010010, vk, vj, vd))
#define VAVGR_D(vd, vj, vk)         EMIT(type_3R(0b01110000011010011, vk, vj, vd))
#define VAVGR_BU(vd, vj, vk)        EMIT(type_3R(0b01110000011010100, vk, vj, vd))
#define VAVGR_HU(vd, vj, vk)        EMIT(type_3R(0b01110000011010101, vk, vj, vd))
#define VAVGR_WU(vd, vj, vk)        EMIT(type_3R(0b01110000011010110, vk, vj, vd))
#define VAVGR_DU(vd, vj, vk)        EMIT(type_3R(0b01110000011010111, vk, vj, vd))
#define VABSD_B(vd, vj, vk)         EMIT(type_3R(0b01110000011000000, vk, vj, vd))
#define VABSD_H(vd, vj, vk)         EMIT(type_3R(0b01110000011000001, vk, vj, vd))
#define VABSD_W(vd, vj, vk)         EMIT(type_3R(0b01110000011000010, vk, vj, vd))
#define VABSD_D(vd, vj, vk)         EMIT(type_3R(0b01110000011000011, vk, vj, vd))
#define VABSD_BU(vd, vj, vk)        EMIT(type_3R(0b01110000011000100, vk, vj, vd))
#define VABSD_HU(vd, vj, vk)        EMIT(type_3R(0b01110000011000101, vk, vj, vd))
#define VABSD_WU(vd, vj, vk)        EMIT(type_3R(0b01110000011000110, vk, vj, vd))
#define VABSD_DU(vd, vj, vk)        EMIT(type_3R(0b01110000011000111, vk, vj, vd))
#define VADDA_B(vd, vj, vk)         EMIT(type_3R(0b01110000010111000, vk, vj, vd))
#define VADDA_H(vd, vj, vk)         EMIT(type_3R(0b01110000010111001, vk, vj, vd))
#define VADDA_W(vd, vj, vk)         EMIT(type_3R(0b01110000010111010, vk, vj, vd))
#define VADDA_D(vd, vj, vk)         EMIT(type_3R(0b01110000010111011, vk, vj, vd))
#define VMAXI_B(vd, vj, imm5)       EMIT(type_3R(0b01110010100100000, imm5, vj, vd))
#define VMAXI_H(vd, vj, imm5)       EMIT(type_3R(0b01110010100100001, imm5, vj, vd))
#define VMAXI_W(vd, vj, imm5)       EMIT(type_3R(0b01110010100100010, imm5, vj, vd))
#define VMAXI_D(vd, vj, imm5)       EMIT(type_3R(0b01110010100100011, imm5, vj, vd))
#define VMAXI_BU(vd, vj, imm5)      EMIT(type_3R(0b01110010100101000, imm5, vj, vd))
#define VMAXI_HU(vd, vj, imm5)      EMIT(type_3R(0b01110010100101001, imm5, vj, vd))
#define VMAXI_WU(vd, vj, imm5)      EMIT(type_3R(0b01110010100101010, imm5, vj, vd))
#define VMAXI_DU(vd, vj, imm5)      EMIT(type_3R(0b01110010100101011, imm5, vj, vd))
#define VMAX_B(vd, vj, vk)          EMIT(type_3R(0b01110000011100000, vk, vj, vd))
#define VMAX_H(vd, vj, vk)          EMIT(type_3R(0b01110000011100001, vk, vj, vd))
#define VMAX_W(vd, vj, vk)          EMIT(type_3R(0b01110000011100010, vk, vj, vd))
#define VMAX_D(vd, vj, vk)          EMIT(type_3R(0b01110000011100011, vk, vj, vd))
#define VMAX_BU(vd, vj, vk)         EMIT(type_3R(0b01110000011101000, vk, vj, vd))
#define VMAX_HU(vd, vj, vk)         EMIT(type_3R(0b01110000011101001, vk, vj, vd))
#define VMAX_WU(vd, vj, vk)         EMIT(type_3R(0b01110000011101010, vk, vj, vd))
#define VMAX_DU(vd, vj, vk)         EMIT(type_3R(0b01110000011101011, vk, vj, vd))
#define VMINI_B(vd, vj, imm5)          EMIT(type_3R(0b01110010100100100, imm5, vj, vd))
#define VMINI_H(vd, vj, imm5)          EMIT(type_3R(0b01110010100100101, imm5, vj, vd))
#define VMINI_W(vd, vj, imm5)          EMIT(type_3R(0b01110010100100110, imm5, vj, vd))
#define VMINI_D(vd, vj, imm5)          EMIT(type_3R(0b01110010100100111, imm5, vj, vd))
#define VMINI_BU(vd, vj, imm5)         EMIT(type_3R(0b01110010100101100, imm5, vj, vd))
#define VMINI_HU(vd, vj, imm5)         EMIT(type_3R(0b01110010100101101, imm5, vj, vd))
#define VMINI_WU(vd, vj, imm5)         EMIT(type_3R(0b01110010100101110, imm5, vj, vd))
#define VMINI_DU(vd, vj, imm5)         EMIT(type_3R(0b01110010100101111, imm5, vj, vd))
#define VMIN_B(vd, vj, vk)          EMIT(type_3R(0b01110000011100100, vk, vj, vd))
#define VMIN_H(vd, vj, vk)          EMIT(type_3R(0b01110000011100101, vk, vj, vd))
#define VMIN_W(vd, vj, vk)          EMIT(type_3R(0b01110000011100110, vk, vj, vd))
#define VMIN_D(vd, vj, vk)          EMIT(type_3R(0b01110000011100111, vk, vj, vd))
#define VMIN_BU(vd, vj, vk)         EMIT(type_3R(0b01110000011101100, vk, vj, vd))
#define VMIN_HU(vd, vj, vk)         EMIT(type_3R(0b01110000011101101, vk, vj, vd))
#define VMIN_WU(vd, vj, vk)         EMIT(type_3R(0b01110000011101110, vk, vj, vd))
#define VMIN_DU(vd, vj, vk)         EMIT(type_3R(0b01110000011101111, vk, vj, vd))
#define VMUL_B(vd, vj, vk)          EMIT(type_3R(0b01110000100001000, vk, vj, vd))
#define VMUL_H(vd, vj, vk)          EMIT(type_3R(0b01110000100001001, vk, vj, vd))
#define VMUL_W(vd, vj, vk)          EMIT(type_3R(0b01110000100001010, vk, vj, vd))
#define VMUL_D(vd, vj, vk)          EMIT(type_3R(0b01110000100001011, vk, vj, vd))
#define VMUH_B(vd, vj, vk)          EMIT(type_3R(0b01110000100001100, vk, vj, vd))
#define VMUH_H(vd, vj, vk)          EMIT(type_3R(0b01110000100001101, vk, vj, vd))
#define VMUH_W(vd, vj, vk)          EMIT(type_3R(0b01110000100001110, vk, vj, vd))
#define VMUH_D(vd, vj, vk)          EMIT(type_3R(0b01110000100001111, vk, vj, vd))
#define VMUH_BU(vd, vj, vk)         EMIT(type_3R(0b01110000100010000, vk, vj, vd))
#define VMUH_HU(vd, vj, vk)         EMIT(type_3R(0b01110000100010001, vk, vj, vd))
#define VMUH_WU(vd, vj, vk)         EMIT(type_3R(0b01110000100010010, vk, vj, vd))
#define VMUH_DU(vd, vj, vk)         EMIT(type_3R(0b01110000100010011, vk, vj, vd))
#define VMULWEV_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000100100000, vk, vj, vd))
#define VMULWEV_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000100100001, vk, vj, vd))
#define VMULWEV_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000100100010, vk, vj, vd))
#define VMULWEV_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000100100011, vk, vj, vd))
#define VMULWOD_H_B(vd, vj, vk)     EMIT(type_3R(0b01110000100100100, vk, vj, vd))
#define VMULWOD_W_H(vd, vj, vk)     EMIT(type_3R(0b01110000100100101, vk, vj, vd))
#define VMULWOD_D_W(vd, vj, vk)     EMIT(type_3R(0b01110000100100110, vk, vj, vd))
#define VMULWOD_Q_D(vd, vj, vk)     EMIT(type_3R(0b01110000100100111, vk, vj, vd))
#define VMULWEV_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000100110000, vk, vj, vd))
#define VMULWEV_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000100110001, vk, vj, vd))
#define VMULWEV_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000100110010, vk, vj, vd))
#define VMULWEV_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000100110011, vk, vj, vd))
#define VMULWOD_H_BU(vd, vj, vk)    EMIT(type_3R(0b01110000100110100, vk, vj, vd))
#define VMULWOD_W_HU(vd, vj, vk)    EMIT(type_3R(0b01110000100110101, vk, vj, vd))
#define VMULWOD_D_WU(vd, vj, vk)    EMIT(type_3R(0b01110000100110110, vk, vj, vd))
#define VMULWOD_Q_DU(vd, vj, vk)    EMIT(type_3R(0b01110000100110111, vk, vj, vd))
#define VMULWEV_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110000101000000, vk, vj, vd))
#define VMULWEV_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110000101000001, vk, vj, vd))
#define VMULWEV_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110000101000010, vk, vj, vd))
#define VMULWEV_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110000101000011, vk, vj, vd))
#define VMULWOD_H_BU_B(vd, vj, vk)  EMIT(type_3R(0b01110000101000100, vk, vj, vd))
#define VMULWOD_W_HU_H(vd, vj, vk)  EMIT(type_3R(0b01110000101000101, vk, vj, vd))
#define VMULWOD_D_WU_W(vd, vj, vk)  EMIT(type_3R(0b01110000101000110, vk, vj, vd))
#define VMULWOD_Q_DU_D(vd, vj, vk)  EMIT(type_3R(0b01110000101000111, vk, vj, vd))
#define VMADD_B(vd, vj, vk)         EMIT(type_3R(0b01110000101010000, vk, vj, vd))
#define VMADD_H(vd, vj, vk)         EMIT(type_3R(0b01110000101010001, vk, vj, vd))
#define VMADD_W(vd, vj, vk)         EMIT(type_3R(0b01110000101010010, vk, vj, vd))
#define VMADD_D(vd, vj, vk)         EMIT(type_3R(0b01110000101010011, vk, vj, vd))
#define VMSUB_B(vd, vj, vk)         EMIT(type_3R(0b01110000101010100, vk, vj, vd))
#define VMSUB_H(vd, vj, vk)         EMIT(type_3R(0b01110000101010101, vk, vj, vd))
#define VMSUB_W(vd, vj, vk)         EMIT(type_3R(0b01110000101010110, vk, vj, vd))
#define VMSUB_D(vd, vj, vk)         EMIT(type_3R(0b01110000101010111, vk, vj, vd))
#define VMADDWEV_H_B(vd, vj, vk)    EMIT(type_3R(0b01110000101011000, vk, vj, vd))
#define VMADDWEV_W_H(vd, vj, vk)    EMIT(type_3R(0b01110000101011001, vk, vj, vd))
#define VMADDWEV_D_W(vd, vj, vk)    EMIT(type_3R(0b01110000101011010, vk, vj, vd))
#define VMADDWEV_Q_D(vd, vj, vk)    EMIT(type_3R(0b01110000101011011, vk, vj, vd))
#define VMADDWOD_H_B(vd, vj, vk)    EMIT(type_3R(0b01110000101011100, vk, vj, vd))
#define VMADDWOD_W_H(vd, vj, vk)    EMIT(type_3R(0b01110000101011101, vk, vj, vd))
#define VMADDWOD_D_W(vd, vj, vk)    EMIT(type_3R(0b01110000101011110, vk, vj, vd))
#define VMADDWOD_Q_D(vd, vj, vk)    EMIT(type_3R(0b01110000101011111, vk, vj, vd))
#define VMADDWEV_H_BU(vd, vj, vk)   EMIT(type_3R(0b01110000101101000, vk, vj, vd))
#define VMADDWEV_W_HU(vd, vj, vk)   EMIT(type_3R(0b01110000101101001, vk, vj, vd))
#define VMADDWEV_D_WU(vd, vj, vk)   EMIT(type_3R(0b01110000101101010, vk, vj, vd))
#define VMADDWEV_Q_DU(vd, vj, vk)   EMIT(type_3R(0b01110000101101011, vk, vj, vd))
#define VMADDWOD_H_BU(vd, vj, vk)   EMIT(type_3R(0b01110000101101100, vk, vj, vd))
#define VMADDWOD_W_HU(vd, vj, vk)   EMIT(type_3R(0b01110000101101101, vk, vj, vd))
#define VMADDWOD_D_WU(vd, vj, vk)   EMIT(type_3R(0b01110000101101110, vk, vj, vd))
#define VMADDWOD_Q_DU(vd, vj, vk)   EMIT(type_3R(0b01110000101101111, vk, vj, vd))
#define VMADDWEV_H_BU_B(vd, vj, vk) EMIT(type_3R(0b01110000101111000, vk, vj, vd))
#define VMADDWEV_W_HU_H(vd, vj, vk) EMIT(type_3R(0b01110000101111001, vk, vj, vd))
#define VMADDWEV_D_WU_W(vd, vj, vk) EMIT(type_3R(0b01110000101111010, vk, vj, vd))
#define VMADDWEV_Q_DU_D(vd, vj, vk) EMIT(type_3R(0b01110000101111011, vk, vj, vd))
#define VMADDWOD_H_BU_B(vd, vj, vk) EMIT(type_3R(0b01110000101111100, vk, vj, vd))
#define VMADDWOD_W_HU_H(vd, vj, vk) EMIT(type_3R(0b01110000101111101, vk, vj, vd))
#define VMADDWOD_D_WU_W(vd, vj, vk) EMIT(type_3R(0b01110000101111110, vk, vj, vd))
#define VMADDWOD_Q_DU_D(vd, vj, vk) EMIT(type_3R(0b01110000101111111, vk, vj, vd))
#define VDIV_B(vd, vj, vk)          EMIT(type_3R(0b01110000111000000, vk, vj, vd))
#define VDIV_H(vd, vj, vk)          EMIT(type_3R(0b01110000111000001, vk, vj, vd))
#define VDIV_W(vd, vj, vk)          EMIT(type_3R(0b01110000111000010, vk, vj, vd))
#define VDIV_D(vd, vj, vk)          EMIT(type_3R(0b01110000111000011, vk, vj, vd))
#define VDIV_BU(vd, vj, vk)         EMIT(type_3R(0b01110000111001000, vk, vj, vd))
#define VDIV_HU(vd, vj, vk)         EMIT(type_3R(0b01110000111001001, vk, vj, vd))
#define VDIV_WU(vd, vj, vk)         EMIT(type_3R(0b01110000111001010, vk, vj, vd))
#define VDIV_DU(vd, vj, vk)         EMIT(type_3R(0b01110000111001011, vk, vj, vd))
#define VMOD_B(vd, vj, vk)          EMIT(type_3R(0b01110000111000100, vk, vj, vd))
#define VMOD_H(vd, vj, vk)          EMIT(type_3R(0b01110000111000101, vk, vj, vd))
#define VMOD_W(vd, vj, vk)          EMIT(type_3R(0b01110000111000110, vk, vj, vd))
#define VMOD_D(vd, vj, vk)          EMIT(type_3R(0b01110000111000111, vk, vj, vd))
#define VMOD_BU(vd, vj, vk)         EMIT(type_3R(0b01110000111001100, vk, vj, vd))
#define VMOD_HU(vd, vj, vk)         EMIT(type_3R(0b01110000111001101, vk, vj, vd))
#define VMOD_WU(vd, vj, vk)         EMIT(type_3R(0b01110000111001110, vk, vj, vd))
#define VMOD_DU(vd, vj, vk)         EMIT(type_3R(0b01110000111001111, vk, vj, vd))
#define VSIGNCOV_B(vd, vj, vk)      EMIT(type_3R(0b01110001001011100, vk, vj, vd))
#define VSIGNCOV_H(vd, vj, vk)      EMIT(type_3R(0b01110001001011101, vk, vj, vd))
#define VSIGNCOV_W(vd, vj, vk)      EMIT(type_3R(0b01110001001011110, vk, vj, vd))
#define VSIGNCOV_D(vd, vj, vk)      EMIT(type_3R(0b01110001001011111, vk, vj, vd))
#define VAND_V(vd, vj, vk)          EMIT(type_3R(0b01110001001001100, vk, vj, vd))
#define VLDI(vd, imm13)             EMIT(type_1RI13(0b01110011111000, imm13, vd))
#define VOR_V(vd, vj, vk)           EMIT(type_3R(0b01110001001001101, vk, vj, vd))
#define VXOR_V(vd, vj, vk)          EMIT(type_3R(0b01110001001001110, vk, vj, vd))
#define VNOR_V(vd, vj, vk)          EMIT(type_3R(0b01110001001001111, vk, vj, vd))
#define VANDN_V(vd, vj, vk)         EMIT(type_3R(0b01110001001010000, vk, vj, vd))
#define VORN_V(vd, vj, vk)          EMIT(type_3R(0b01110001001010001, vk, vj, vd))
#define VSLL_B(vd, vj, vk)          EMIT(type_3R(0b01110000111010000, vk, vj, vd))
#define VSLL_H(vd, vj, vk)          EMIT(type_3R(0b01110000111010001, vk, vj, vd))
#define VSLL_W(vd, vj, vk)          EMIT(type_3R(0b01110000111010010, vk, vj, vd))
#define VSLL_D(vd, vj, vk)          EMIT(type_3R(0b01110000111010011, vk, vj, vd))
#define VSRL_B(vd, vj, vk)          EMIT(type_3R(0b01110000111010100, vk, vj, vd))
#define VSRL_H(vd, vj, vk)          EMIT(type_3R(0b01110000111010101, vk, vj, vd))
#define VSRL_W(vd, vj, vk)          EMIT(type_3R(0b01110000111010110, vk, vj, vd))
#define VSRL_D(vd, vj, vk)          EMIT(type_3R(0b01110000111010111, vk, vj, vd))
#define VSRA_B(vd, vj, vk)          EMIT(type_3R(0b01110000111011000, vk, vj, vd))
#define VSRA_H(vd, vj, vk)          EMIT(type_3R(0b01110000111011001, vk, vj, vd))
#define VSRA_W(vd, vj, vk)          EMIT(type_3R(0b01110000111011010, vk, vj, vd))
#define VSRA_D(vd, vj, vk)          EMIT(type_3R(0b01110000111011011, vk, vj, vd))
#define VSLLI_B(vd, vj, imm3)       EMIT(type_2RI3(0b0111001100101100001, imm3, vj, vd))
#define VSLLI_H(vd, vj, imm4)       EMIT(type_2RI4(0b011100110010110001, imm4, vj, vd))
#define VSLLI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110011001011001, imm5, vj, vd))
#define VSLLI_D(vd, vj, imm6)       EMIT(type_2RI6(0b0111001100101101, imm6, vj, vd))
#define VSRLI_B(vd, vj, imm3)       EMIT(type_2RI3(0b0111001100110000001, imm3, vj, vd))
#define VSRLI_H(vd, vj, imm4)       EMIT(type_2RI4(0b011100110011000001, imm4, vj, vd))
#define VSRLI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110011001100001, imm5, vj, vd))
#define VSRLI_D(vd, vj, imm6)       EMIT(type_2RI6(0b0111001100110001, imm6, vj, vd))
#define VSRAI_B(vd, vj, imm3)       EMIT(type_2RI3(0b0111001100110100001, imm3, vj, vd))
#define VSRAI_H(vd, vj, imm4)       EMIT(type_2RI4(0b011100110011010001, imm4, vj, vd))
#define VSRAI_W(vd, vj, imm5)       EMIT(type_2RI5(0b01110011001101001, imm5, vj, vd))
#define VSRAI_D(vd, vj, imm6)       EMIT(type_2RI6(0b0111001100110101, imm6, vj, vd))
#define VROTR_B(vd, vj, vk)         EMIT(type_3R(0b01110000111011100, vk, vj, vd))
#define VROTR_H(vd, vj, vk)         EMIT(type_3R(0b01110000111011101, vk, vj, vd))
#define VROTR_W(vd, vj, vk)         EMIT(type_3R(0b01110000111011110, vk, vj, vd))
#define VROTR_D(vd, vj, vk)         EMIT(type_3R(0b01110000111011111, vk, vj, vd))
#define VSRLR_B(vd, vj, vk)         EMIT(type_3R(0b01110000111100000, vk, vj, vd))
#define VSRLR_H(vd, vj, vk)         EMIT(type_3R(0b01110000111100001, vk, vj, vd))
#define VSRLR_W(vd, vj, vk)         EMIT(type_3R(0b01110000111100010, vk, vj, vd))
#define VSRLR_D(vd, vj, vk)         EMIT(type_3R(0b01110000111100011, vk, vj, vd))
#define VSRAR_B(vd, vj, vk)         EMIT(type_3R(0b01110000111100100, vk, vj, vd))
#define VSRAR_H(vd, vj, vk)         EMIT(type_3R(0b01110000111100101, vk, vj, vd))
#define VSRAR_W(vd, vj, vk)         EMIT(type_3R(0b01110000111100110, vk, vj, vd))
#define VSRAR_D(vd, vj, vk)         EMIT(type_3R(0b01110000111100111, vk, vj, vd))
#define VSRLN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110000111101001, vk, vj, vd))
#define VSRLN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110000111101010, vk, vj, vd))
#define VSRLN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110000111101011, vk, vj, vd))
#define VSRAN_B_H(vd, vj, vk)       EMIT(type_3R(0b01110000111101101, vk, vj, vd))
#define VSRAN_H_W(vd, vj, vk)       EMIT(type_3R(0b01110000111101110, vk, vj, vd))
#define VSRAN_W_D(vd, vj, vk)       EMIT(type_3R(0b01110000111101111, vk, vj, vd))
#define VSRLRN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110000111110001, vk, vj, vd))
#define VSRLRN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110000111110010, vk, vj, vd))
#define VSRLRN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110000111110011, vk, vj, vd))
#define VSRARN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110000111110101, vk, vj, vd))
#define VSRARN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110000111110110, vk, vj, vd))
#define VSRARN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110000111110111, vk, vj, vd))
#define VSSRLN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110000111111001, vk, vj, vd))
#define VSSRLN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110000111111010, vk, vj, vd))
#define VSSRLN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110000111111011, vk, vj, vd))
#define VSSRAN_B_H(vd, vj, vk)      EMIT(type_3R(0b01110000111111101, vk, vj, vd))
#define VSSRAN_H_W(vd, vj, vk)      EMIT(type_3R(0b01110000111111110, vk, vj, vd))
#define VSSRAN_W_D(vd, vj, vk)      EMIT(type_3R(0b01110000111111111, vk, vj, vd))
#define VSSRLN_BU_H(vd, vj, vk)     EMIT(type_3R(0b01110001000001001, vk, vj, vd))
#define VSSRLN_HU_W(vd, vj, vk)     EMIT(type_3R(0b01110001000001010, vk, vj, vd))
#define VSSRLN_WU_D(vd, vj, vk)     EMIT(type_3R(0b01110001000001011, vk, vj, vd))
#define VSSRAN_BU_H(vd, vj, vk)     EMIT(type_3R(0b01110001000001101, vk, vj, vd))
#define VSSRAN_HU_W(vd, vj, vk)     EMIT(type_3R(0b01110001000001110, vk, vj, vd))
#define VSSRAN_WU_D(vd, vj, vk)     EMIT(type_3R(0b01110001000001111, vk, vj, vd))
#define VSSRLRN_B_H(vd, vj, vk)     EMIT(type_3R(0b01110001000000001, vk, vj, vd))
#define VSSRLRN_H_W(vd, vj, vk)     EMIT(type_3R(0b01110001000000010, vk, vj, vd))
#define VSSRLRN_W_D(vd, vj, vk)     EMIT(type_3R(0b01110001000000011, vk, vj, vd))
#define VSSRARN_B_H(vd, vj, vk)     EMIT(type_3R(0b01110001000000101, vk, vj, vd))
#define VSSRARN_H_W(vd, vj, vk)     EMIT(type_3R(0b01110001000000110, vk, vj, vd))
#define VSSRARN_W_D(vd, vj, vk)     EMIT(type_3R(0b01110001000000111, vk, vj, vd))
#define VSSRLRN_BU_H(vd, vj, vk)    EMIT(type_3R(0b01110001000010001, vk, vj, vd))
#define VSSRLRN_HU_W(vd, vj, vk)    EMIT(type_3R(0b01110001000010010, vk, vj, vd))
#define VSSRLRN_WU_D(vd, vj, vk)    EMIT(type_3R(0b01110001000010011, vk, vj, vd))
#define VSSRARN_BU_H(vd, vj, vk)    EMIT(type_3R(0b01110001000010101, vk, vj, vd))
#define VSSRARN_HU_W(vd, vj, vk)    EMIT(type_3R(0b01110001000010110, vk, vj, vd))
#define VSSRARN_WU_D(vd, vj, vk)    EMIT(type_3R(0b01110001000010111, vk, vj, vd))
#define VBITCLR_B(vd, vj, vk)       EMIT(type_3R(0b01110001000011000, vk, vj, vd))
#define VBITCLR_H(vd, vj, vk)       EMIT(type_3R(0b01110001000011001, vk, vj, vd))
#define VBITCLR_W(vd, vj, vk)       EMIT(type_3R(0b01110001000011010, vk, vj, vd))
#define VBITCLR_D(vd, vj, vk)       EMIT(type_3R(0b01110001000011011, vk, vj, vd))
#define VBITSET_B(vd, vj, vk)       EMIT(type_3R(0b01110001000011100, vk, vj, vd))
#define VBITSET_H(vd, vj, vk)       EMIT(type_3R(0b01110001000011101, vk, vj, vd))
#define VBITSET_W(vd, vj, vk)       EMIT(type_3R(0b01110001000011110, vk, vj, vd))
#define VBITSET_D(vd, vj, vk)       EMIT(type_3R(0b01110001000011111, vk, vj, vd))
#define VBITREV_B(vd, vj, vk)       EMIT(type_3R(0b01110001000100000, vk, vj, vd))
#define VBITREV_H(vd, vj, vk)       EMIT(type_3R(0b01110001000100001, vk, vj, vd))
#define VBITREV_W(vd, vj, vk)       EMIT(type_3R(0b01110001000100010, vk, vj, vd))
#define VBITREV_D(vd, vj, vk)       EMIT(type_3R(0b01110001000100011, vk, vj, vd))
#define VFRSTP_B(vd, vj, vk)        EMIT(type_3R(0b01110001001010110, vk, vj, vd))
#define VFRSTP_H(vd, vj, vk)        EMIT(type_3R(0b01110001001010111, vk, vj, vd))
#define VFADD_S(vd, vj, vk)         EMIT(type_3R(0b01110001001100001, vk, vj, vd))
#define VFADD_D(vd, vj, vk)         EMIT(type_3R(0b01110001001100010, vk, vj, vd))
#define VFSUB_S(vd, vj, vk)         EMIT(type_3R(0b01110001001100101, vk, vj, vd))
#define VFSUB_D(vd, vj, vk)         EMIT(type_3R(0b01110001001100110, vk, vj, vd))
#define VFMUL_S(vd, vj, vk)         EMIT(type_3R(0b01110001001110001, vk, vj, vd))
#define VFMUL_D(vd, vj, vk)         EMIT(type_3R(0b01110001001110010, vk, vj, vd))
#define VFDIV_S(vd, vj, vk)         EMIT(type_3R(0b01110001001110101, vk, vj, vd))
#define VFDIV_D(vd, vj, vk)         EMIT(type_3R(0b01110001001110110, vk, vj, vd))
#define VFMAX_S(vd, vj, vk)         EMIT(type_3R(0b01110001001111001, vk, vj, vd))
#define VFMAX_D(vd, vj, vk)         EMIT(type_3R(0b01110001001111010, vk, vj, vd))
#define VFMIN_S(vd, vj, vk)         EMIT(type_3R(0b01110001001111101, vk, vj, vd))
#define VFMIN_D(vd, vj, vk)         EMIT(type_3R(0b01110001001111110, vk, vj, vd))
#define VFMAXA_S(vd, vj, vk)        EMIT(type_3R(0b01110001010000001, vk, vj, vd))
#define VFMAXA_D(vd, vj, vk)        EMIT(type_3R(0b01110001010000010, vk, vj, vd))
#define VFMINA_S(vd, vj, vk)        EMIT(type_3R(0b01110001010000101, vk, vj, vd))
#define VFMINA_D(vd, vj, vk)        EMIT(type_3R(0b01110001010000110, vk, vj, vd))
#define VFSQRT_S(vd, vj)            EMIT(type_2R(0b0111001010011100111001, vj, vd))
#define VFSQRT_D(vd, vj)            EMIT(type_2R(0b0111001010011100111010, vj, vd))
#define VFRECIP_S(vd, vj)           EMIT(type_2R(0b0111001010011100111101, vj, vd))
#define VFRECIP_D(vd, vj)           EMIT(type_2R(0b0111001010011100111110, vj, vd))
#define VFRSQRT_S(vd, vj)           EMIT(type_2R(0b0111001010011101000001, vj, vd))
#define VFRSQRT_D(vd, vj)           EMIT(type_2R(0b0111001010011101000010, vj, vd))
#define VFCVTL_S_H(vd, vj)          EMIT(type_2R(0b0111001010011101111010, vj, vd))
#define VFCVTH_S_H(vd, vj)          EMIT(type_2R(0b0111001010011101111011, vj, vd))
#define VFCVTL_D_S(vd, vj)          EMIT(type_2R(0b0111001010011101111100, vj, vd))
#define VFCVTH_D_S(vd, vj)          EMIT(type_2R(0b0111001010011101111101, vj, vd))
#define VFCVT_H_S(vd, vj, vk)       EMIT(type_3R(0b01110001010001100, vk, vj, vd))
#define VFCVT_S_D(vd, vj, vk)       EMIT(type_3R(0b01110001010001101, vk, vj, vd))
#define VFTINT_W_S(vd, vj)          EMIT(type_2R(0b0111001010011110001100, vj, vd))
#define VFTINT_L_D(vd, vj)          EMIT(type_2R(0b0111001010011110001101, vj, vd))
#define VFTINTRM_W_S(vd, vj)        EMIT(type_2R(0b0111001010011110001110, vj, vd))
#define VFTINTRM_L_D(vd, vj)        EMIT(type_2R(0b0111001010011110001111, vj, vd))
#define VFTINTRP_W_S(vd, vj)        EMIT(type_2R(0b0111001010011110010000, vj, vd))
#define VFTINTRP_L_D(vd, vj)        EMIT(type_2R(0b0111001010011110010001, vj, vd))
#define VFTINTRZ_W_S(vd, vj)        EMIT(type_2R(0b0111001010011110010010, vj, vd))
#define VFTINTRZ_L_D(vd, vj)        EMIT(type_2R(0b0111001010011110010011, vj, vd))
#define VFTINTRNE_W_S(vd, vj)       EMIT(type_2R(0b0111001010011110010100, vj, vd))
#define VFTINTRNE_L_D(vd, vj)       EMIT(type_2R(0b0111001010011110010101, vj, vd))
#define VFTINT_WU_S(vd, vj)         EMIT(type_2R(0b0111001010011110010110, vj, vd))
#define VFTINT_LU_D(vd, vj)         EMIT(type_2R(0b0111001010011110010111, vj, vd))
#define VFTINTRZ_WU_S(vd, vj)       EMIT(type_2R(0b0111001010011110011100, vj, vd))
#define VFTINTRZ_LU_D(vd, vj)       EMIT(type_2R(0b0111001010011110011101, vj, vd))
#define VFTINT_W_D(vd, vj, vk)      EMIT(type_3R(0b01110001010010011, vk, vj, vd))
#define VFTINTRM_W_D(vd, vj, vk)    EMIT(type_3R(0b01110001010010100, vk, vj, vd))
#define VFTINTRP_W_D(vd, vj, vk)    EMIT(type_3R(0b01110001010010101, vk, vj, vd))
#define VFTINTRZ_W_D(vd, vj, vk)    EMIT(type_3R(0b01110001010010110, vk, vj, vd))
#define VFTINTRNE_W_D(vd, vj, vk)   EMIT(type_3R(0b01110001010010111, vk, vj, vd))
#define VFTINTL_L_S(vd, vj)         EMIT(type_2R(0b0111001010011110100000, vj, vd))
#define VFTINTH_L_S(vd, vj)         EMIT(type_2R(0b0111001010011110100001, vj, vd))
#define VFTINTRML_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110100010, vj, vd))
#define VFTINTRMH_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110100011, vj, vd))
#define VFTINTRPL_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110100100, vj, vd))
#define VFTINTRPH_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110100101, vj, vd))
#define VFTINTRZL_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110100110, vj, vd))
#define VFTINTRZH_L_S(vd, vj)       EMIT(type_2R(0b0111001010011110100111, vj, vd))
#define VFTINTRNEL_L_S(vd, vj)      EMIT(type_2R(0b0111001010011110101000, vj, vd))
#define VFTINTRNEH_L_S(vd, vj)      EMIT(type_2R(0b0111001010011110101001, vj, vd))
#define VFFINT_S_W(vd, vj)          EMIT(type_2R(0b0111001010011110000000, vj, vd))
#define VFFINT_S_WU(vd, vj)         EMIT(type_2R(0b0111001010011110000001, vj, vd))
#define VFFINT_D_L(vd, vj)          EMIT(type_2R(0b0111001010011110000010, vj, vd))
#define VFFINT_D_LU(vd, vj)         EMIT(type_2R(0b0111001010011110000011, vj, vd))
#define VFFINTL_D_W(vd, vj)         EMIT(type_2R(0b0111001010011110000100, vj, vd))
#define VFFINTH_D_W(vd, vj)         EMIT(type_2R(0b0111001010011110000101, vj, vd))
#define VFFINT_S_L(vd, vj, vk)      EMIT(type_3R(0b01110001010010000, vk, vj, vd))
#define VSEQ_B(vd, vj, vk)          EMIT(type_3R(0b01110000000000000, vk, vj, vd))
#define VSEQ_H(vd, vj, vk)          EMIT(type_3R(0b01110000000000001, vk, vj, vd))
#define VSEQ_W(vd, vj, vk)          EMIT(type_3R(0b01110000000000010, vk, vj, vd))
#define VSEQ_D(vd, vj, vk)          EMIT(type_3R(0b01110000000000011, vk, vj, vd))
#define VSLE_B(vd, vj, vk)          EMIT(type_3R(0b01110000000000100, vk, vj, vd))
#define VSLE_H(vd, vj, vk)          EMIT(type_3R(0b01110000000000101, vk, vj, vd))
#define VSLE_W(vd, vj, vk)          EMIT(type_3R(0b01110000000000110, vk, vj, vd))
#define VSLE_D(vd, vj, vk)          EMIT(type_3R(0b01110000000000111, vk, vj, vd))
#define VSLE_BU(vd, vj, vk)         EMIT(type_3R(0b01110000000001000, vk, vj, vd))
#define VSLE_HU(vd, vj, vk)         EMIT(type_3R(0b01110000000001001, vk, vj, vd))
#define VSLE_WU(vd, vj, vk)         EMIT(type_3R(0b01110000000001010, vk, vj, vd))
#define VSLE_DU(vd, vj, vk)         EMIT(type_3R(0b01110000000001011, vk, vj, vd))
#define VSLT_B(vd, vj, vk)          EMIT(type_3R(0b01110000000001100, vk, vj, vd))
#define VSLT_H(vd, vj, vk)          EMIT(type_3R(0b01110000000001101, vk, vj, vd))
#define VSLT_W(vd, vj, vk)          EMIT(type_3R(0b01110000000001110, vk, vj, vd))
#define VSLT_D(vd, vj, vk)          EMIT(type_3R(0b01110000000001111, vk, vj, vd))
#define VSLT_BU(vd, vj, vk)         EMIT(type_3R(0b01110000000010000, vk, vj, vd))
#define VSLT_HU(vd, vj, vk)         EMIT(type_3R(0b01110000000010001, vk, vj, vd))
#define VSLT_WU(vd, vj, vk)         EMIT(type_3R(0b01110000000010010, vk, vj, vd))
#define VSLT_DU(vd, vj, vk)         EMIT(type_3R(0b01110000000010011, vk, vj, vd))
#define VBSLL_V(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100011100, imm5, vj, vd))
#define VBSRL_V(vd, vj, imm5)       EMIT(type_2RI5(0b01110010100011101, imm5, vj, vd))
#define VPACKEV_B(vd, vj, vk)       EMIT(type_3R(0b01110001000101100, vk, vj, vd))
#define VPACKEV_H(vd, vj, vk)       EMIT(type_3R(0b01110001000101101, vk, vj, vd))
#define VPACKEV_W(vd, vj, vk)       EMIT(type_3R(0b01110001000101110, vk, vj, vd))
#define VPACKEV_D(vd, vj, vk)       EMIT(type_3R(0b01110001000101111, vk, vj, vd))
#define VPACKOD_B(vd, vj, vk)       EMIT(type_3R(0b01110001000110000, vk, vj, vd))
#define VPACKOD_H(vd, vj, vk)       EMIT(type_3R(0b01110001000110001, vk, vj, vd))
#define VPACKOD_W(vd, vj, vk)       EMIT(type_3R(0b01110001000110010, vk, vj, vd))
#define VPACKOD_D(vd, vj, vk)       EMIT(type_3R(0b01110001000110011, vk, vj, vd))
#define VPICKEV_B(vd, vj, vk)       EMIT(type_3R(0b01110001000111100, vk, vj, vd))
#define VPICKEV_H(vd, vj, vk)       EMIT(type_3R(0b01110001000111101, vk, vj, vd))
#define VPICKEV_W(vd, vj, vk)       EMIT(type_3R(0b01110001000111110, vk, vj, vd))
#define VPICKEV_D(vd, vj, vk)       EMIT(type_3R(0b01110001000111111, vk, vj, vd))
#define VPICKOD_B(vd, vj, vk)       EMIT(type_3R(0b01110001001000000, vk, vj, vd))
#define VPICKOD_H(vd, vj, vk)       EMIT(type_3R(0b01110001001000001, vk, vj, vd))
#define VPICKOD_W(vd, vj, vk)       EMIT(type_3R(0b01110001001000010, vk, vj, vd))
#define VPICKOD_D(vd, vj, vk)       EMIT(type_3R(0b01110001001000011, vk, vj, vd))
#define VILVL_B(vd, vj, vk)         EMIT(type_3R(0b01110001000110100, vk, vj, vd))
#define VILVL_H(vd, vj, vk)         EMIT(type_3R(0b01110001000110101, vk, vj, vd))
#define VILVL_W(vd, vj, vk)         EMIT(type_3R(0b01110001000110110, vk, vj, vd))
#define VILVL_D(vd, vj, vk)         EMIT(type_3R(0b01110001000110111, vk, vj, vd))
#define VILVH_B(vd, vj, vk)         EMIT(type_3R(0b01110001000111000, vk, vj, vd))
#define VILVH_H(vd, vj, vk)         EMIT(type_3R(0b01110001000111001, vk, vj, vd))
#define VILVH_W(vd, vj, vk)         EMIT(type_3R(0b01110001000111010, vk, vj, vd))
#define VILVH_D(vd, vj, vk)         EMIT(type_3R(0b01110001000111011, vk, vj, vd))
#define VSHUF_B(vd, vj, vk, va)     EMIT(type_4R(0b000011010101, va, vk, vj, vd))
#define VSHUF_H(vd, vj, vk)         EMIT(type_3R(0b01110001011110101, vk, vj, vd))
#define VSHUF_W(vd, vj, vk)         EMIT(type_3R(0b01110001011110110, vk, vj, vd))
#define VSHUF_D(vd, vj, vk)         EMIT(type_3R(0b01110001011110111, vk, vj, vd))
#define VSHUF4I_B(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100100, imm8, vj, vd))
#define VSHUF4I_H(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100101, imm8, vj, vd))
#define VSHUF4I_W(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100110, imm8, vj, vd))
#define VSHUF4I_D(vd, vj, imm8)     EMIT(type_2RI8(0b01110011100111, imm8, vj, vd))
#define VEXTRINS_D(vd, vj, imm8)    EMIT(type_2RI8(0b01110011100000, imm8, vj, vd))
#define VEXTRINS_W(vd, vj, imm8)    EMIT(type_2RI8(0b01110011100001, imm8, vj, vd))
#define VEXTRINS_H(vd, vj, imm8)    EMIT(type_2RI8(0b01110011100010, imm8, vj, vd))
#define VEXTRINS_B(vd, vj, imm8)    EMIT(type_2RI8(0b01110011100011, imm8, vj, vd))
#define VLD(vd, rj, imm12)          EMIT(type_2RI12(0b0010110000, imm12, rj, vd))
#define VST(vd, rj, imm12)          EMIT(type_2RI12(0b0010110001, imm12, rj, vd))

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
#define XVFCVT_H_S(vd, vj, vk)       EMIT(type_3R(0b01110101010001100, vk, vj, vd))
#define XVFCVT_S_D(vd, vj, vk)       EMIT(type_3R(0b01110101010001101, vk, vj, vd))
#define XVFTINTRNE_W_D(vd, vj, vk)   EMIT(type_3R(0b01110101010010111, vk, vj, vd))
#define XVFTINTRZ_W_D(vd, vj, vk)    EMIT(type_3R(0b01110101010010110, vk, vj, vd))
#define XVFTINTRP_W_D(vd, vj, vk)    EMIT(type_3R(0b01110101010010101, vk, vj, vd))
#define XVFTINTRM_W_D(vd, vj, vk)    EMIT(type_3R(0b01110101010010100, vk, vj, vd))
#define XVFTINT_W_D(vd, vj, vk)      EMIT(type_3R(0b01110101010010011, vk, vj, vd))
#define XVFFINT_S_L(vd, vj, vk)      EMIT(type_3R(0b01110101010010000, vk, vj, vd))
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
#define XVSHUF_H(vd, vj, vk)         EMIT(type_3R(0b01110101011110101, vk, vj, vd))
#define XVSHUF_W(vd, vj, vk)         EMIT(type_3R(0b01110101011110110, vk, vj, vd))
#define XVSHUF_D(vd, vj, vk)         EMIT(type_3R(0b01110101011110111, vk, vj, vd))
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
#define X64_GET_TOP(rd) EMIT(type_2R(0x00007400, 0, rd))

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

#define X64_INC_B(rj) EMIT(type_2R(0x20, rj, 0x0))
#define X64_INC_H(rj) EMIT(type_2R(0x20, rj, 0x1))
#define X64_INC_W(rj) EMIT(type_2R(0x20, rj, 0x2))
#define X64_INC_D(rj) EMIT(type_2R(0x20, rj, 0x3))
#define X64_DEC_B(rj) EMIT(type_2R(0x20, rj, 0x4))
#define X64_DEC_H(rj) EMIT(type_2R(0x20, rj, 0x5))
#define X64_DEC_W(rj) EMIT(type_2R(0x20, rj, 0x6))
#define X64_DEC_D(rj) EMIT(type_2R(0x20, rj, 0x7))
#define X64_MUL_B(rj, rk)   EMIT(type_3R(0x7d, rk, rj, 0x0))
#define X64_MUL_H(rj, rk)   EMIT(type_3R(0x7d, rk, rj, 0x1))
#define X64_MUL_W(rj, rk)   EMIT(type_3R(0x7d, rk, rj, 0x2))
#define X64_MUL_D(rj, rk)   EMIT(type_3R(0x7d, rk, rj, 0x3))
#define X64_MUL_BU(rj, rk)  EMIT(type_3R(0x7d, rk, rj, 0x4))
#define X64_MUL_HU(rj, rk)  EMIT(type_3R(0x7d, rk, rj, 0x5))
#define X64_MUL_WU(rj, rk)  EMIT(type_3R(0x7d, rk, rj, 0x6))
#define X64_MUL_DU(rj, rk)  EMIT(type_3R(0x7d, rk, rj, 0x7))
#define X64_ADD_WU(rj, rk)  EMIT(type_3R(0x7e, rk, rj, 0x0))
#define X64_ADD_DU(rj, rk)  EMIT(type_3R(0x7e, rk, rj, 0x1))
#define X64_SUB_WU(rj, rk)  EMIT(type_3R(0x7e, rk, rj, 0x2))
#define X64_SUB_DU(rj, rk)  EMIT(type_3R(0x7e, rk, rj, 0x3))
#define X64_ADD_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x4))
#define X64_ADD_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x5))
#define X64_ADD_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x6))
#define X64_ADD_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x7))
#define X64_SUB_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x8))
#define X64_SUB_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x9))
#define X64_SUB_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0xa))
#define X64_SUB_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0xb))
#define X64_ADC_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0xc))
#define X64_ADC_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0xd))
#define X64_ADC_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0xe))
#define X64_ADC_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0xf))
#define X64_SBC_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x10))
#define X64_SBC_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x11))
#define X64_SBC_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x12))
#define X64_SBC_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x13))
#define X64_SLL_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x14))
#define X64_SLL_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x15))
#define X64_SLL_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x16))
#define X64_SLL_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x17))
#define X64_SRL_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x18))
#define X64_SRL_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x19))
#define X64_SRL_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x1a))
#define X64_SRL_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x1b))
#define X64_SRA_B(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x1c))
#define X64_SRA_H(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x1d))
#define X64_SRA_W(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x1e))
#define X64_SRA_D(rj, rk)   EMIT(type_3R(0x7e, rk, rj, 0x1f))
#define X64_ROTR_B(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x0))
#define X64_ROTR_H(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x1))
#define X64_ROTR_D(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x2))
#define X64_ROTR_W(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x3))
#define X64_ROTL_B(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x4))
#define X64_ROTL_H(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x5))
#define X64_ROTL_W(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x6))
#define X64_ROTL_D(rj, rk)  EMIT(type_3R(0x7f, rk, rj, 0x7))
#define X64_RCR_B(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x8))
#define X64_RCR_H(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x9))
#define X64_RCR_W(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0xa))
#define X64_RCR_D(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0xb))
#define X64_RCL_B(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0xc))
#define X64_RCL_H(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0xd))
#define X64_RCL_W(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0xe))
#define X64_RCL_D(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0xf))
#define X64_AND_B(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x10))
#define X64_AND_H(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x11))
#define X64_AND_W(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x12))
#define X64_AND_D(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x13))
#define X64_OR_B(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x14))
#define X64_OR_H(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x15))
#define X64_OR_W(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x16))
#define X64_OR_D(rj, rk)    EMIT(type_3R(0x7f, rk, rj, 0x17))
#define X64_XOR_B(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x18))
#define X64_XOR_H(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x19))
#define X64_XOR_W(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x1a))
#define X64_XOR_D(rj, rk)   EMIT(type_3R(0x7f, rk, rj, 0x1b))
#define X64_SLLI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0x0))
#define X64_SRLI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0x4))
#define X64_SRAI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0x8))
#define X64_ROTRI_B(rj, imm3)   EMIT(type_2RI3(0x2a1, imm3, rj, 0xc))
#define X64_RCRI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0x10))
#define X64_ROTLI_B(rj, imm3)   EMIT(type_2RI3(0x2a1, imm3, rj, 0x14))
#define X64_RCLI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0x18))
#define X64_SLLI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0x1))
#define X64_SRLI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0x5))
#define X64_SRAI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0x9))
#define X64_ROTRI_H(rj, imm4)   EMIT(type_2RI4(0x151, imm4, rj, 0xd))
#define X64_RCRI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0x11))
#define X64_ROTLI_H(rj, imm4)   EMIT(type_2RI4(0x151, imm4, rj, 0x15))
#define X64_RCLI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0x19))
#define X64_SLLI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0x2))
#define X64_SRLI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0x6))
#define X64_SRAI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0xa))
#define X64_ROTRI_W(rj, imm5)   EMIT(type_2RI5(0xa9, imm5, rj, 0xe))
#define X64_RCRI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0x12))
#define X64_ROTLI_W(rj, imm5)   EMIT(type_2RI5(0xa9, imm5, rj, 0x16))
#define X64_RCLI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0x1a))
#define X64_SLLI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0x3))
#define X64_SRLI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0x7))
#define X64_SRAI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0xb))
#define X64_ROTRI_D(rj, imm6)   EMIT(type_2RI6(0x55, imm6, rj, 0xf))
#define X64_RCRI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0x13))
#define X64_ROTLI_D(rj, imm6)   EMIT(type_2RI6(0x55, imm6, rj, 0x17))
#define X64_RCLI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0x1b))

// Warning, these are LBT addons that uses LBT4.eflags internally
#define ADC_B(rd, rj, rk) EMIT(type_3R(0x60, rk, rj, rd))
#define ADC_H(rd, rj, rk) EMIT(type_3R(0x61, rk, rj, rd))
#define ADC_W(rd, rj, rk) EMIT(type_3R(0x62, rk, rj, rd))
#define ADC_D(rd, rj, rk) EMIT(type_3R(0x63, rk, rj, rd))
#define SBC_B(rd, rj, rk) EMIT(type_3R(0x64, rk, rj, rd))
#define SBC_H(rd, rj, rk) EMIT(type_3R(0x65, rk, rj, rd))
#define SBC_W(rd, rj, rk) EMIT(type_3R(0x66, rk, rj, rd))
#define SBC_D(rd, rj, rk) EMIT(type_3R(0x67, rk, rj, rd))
#define RCR_B(rd, rj, rk) EMIT(type_3R(0x68, rk, rj, rd))
#define RCR_H(rd, rj, rk) EMIT(type_3R(0x69, rk, rj, rd))
#define RCR_W(rd, rj, rk) EMIT(type_3R(0x6a, rk, rj, rd))
#define RCR_D(rd, rj, rk) EMIT(type_3R(0x6b, rk, rj, rd))

////////////////////////////////////////////////////////////////////////////////


// GR[rd] = imm32
#define MOV32w_(rd, imm32, zeroup)            \
    do {                                      \
        if (((uint32_t)(imm32)) > 0xfffu) {   \
            LU12I_W(rd, (imm32) >> 12);       \
            ORI(rd, rd, imm32);               \
            if (zeroup && (int32_t)imm32 < 0) \
                ZEROUP(rd);                   \
        } else {                              \
            ORI(rd, xZR, imm32);              \
        }                                     \
    } while (0)

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

// rd[63:0] = rj[63:0] (pseudo instruction)
#define MV(rd, rj) ADDI_D(rd, rj, 0)
// rd = rj (pseudo instruction)
#define MVxw(rd, rj)            \
    do {                        \
        if (rex.w) {            \
            MV(rd, rj);         \
        } else {                \
            AND(rd, rj, xMASK); \
        }                       \
    } while (0)

// rd = rj (pseudo instruction)
#define MVz(rd, rj)             \
    do {                        \
        if (rex.is32bits) {     \
            AND(rd, rj, xMASK); \
        } else {                \
            MV(rd, rj);         \
        }                       \
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
        if (rex.is32bits)          \
            ADDI_W(rd, rj, imm12); \
        else                       \
            ADDI_D(rd, rj, imm12); \
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
        if (rex.is32bits)      \
            ADD_W(rd, rj, rk); \
        else                   \
            ADD_D(rd, rj, rk); \
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

#define SDz(rd, rj, imm12)       \
    do {                         \
        if (rex.is32bits)        \
            ST_W(rd, rj, imm12); \
        else                     \
            ST_D(rd, rj, imm12); \
    } while (0)

#define NEG_D(rd, rs1) SUB_D(rd, xZR, rs1)

#define SUBxw(rd, rj, rk)      \
    do {                       \
        if (rex.w)             \
            SUB_D(rd, rj, rk); \
        else                   \
            SUB_W(rd, rj, rk); \
    } while (0)

#define NEGxw(rd, rs1) SUBxw(rd, xZR, rs1)

#define SUBz(rd, rj, rk)       \
    do {                       \
        if (rex.is32bits)      \
            SUB_W(rd, rj, rk); \
        else                   \
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
        ADDI_W(xRSP, xRSP, -4); \
    } while (0);
#define POP1_32(reg)                            \
    do {                                        \
        LD_WU(reg, xRSP, 0);                    \
        if (reg != xRSP) ADDI_W(xRSP, xRSP, 4); \
    } while (0);

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

#endif //__ARM64_EMITTER_H__
