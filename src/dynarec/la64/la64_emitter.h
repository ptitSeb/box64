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
#define SLA_D(rd, rj, rk) EMIT(type_3R(0b00000000000110011, rk, rj, rd))
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

// Shift Left Immediate
#define SLLIxw(rd, rs1, imm)  \
    if (rex.w) {              \
        SLLI_D(rd, rs1, imm); \
    } else {                  \
        SLLI_W(rd, rs1, imm); \
    }
// Shift Right Logical Immediate
#define SRLIxw(rd, rs1, imm)      \
    do {                          \
        if (rex.w) {              \
            SRLI_D(rd, rs1, imm); \
        } else {                  \
            SRLI_W(rd, rs1, imm); \
        }                         \
    } while (0)

// Shift Right Arithmetic Immediate
#define SRAIxw(rd, rs1, imm)      \
    do {                          \
        if (rex.w) {              \
            SRAI_D(rd, rs1, imm); \
        } else {                  \
            SRAI_W(rd, rs1, imm); \
        }                         \
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
#define BNEZ(rj, imm23) EMIT(type_1RI21(0b010001, ((imm23)>>2), rj))

// GR[rd] = PC + 4
// PC = GR[rj] + SignExtend({imm16, 2'b0}, GRLEN)
#define JIRL(rd, rj, imm18) EMIT(type_2RI16(0b010011, ((imm18)>>2), rj, rd))

// PC = GR[rj]
#define BR(rj) JIRL(xZR, rj, 0x0)

// PC = PC + SignExtend({imm26, 2'b0}, GRLEN)
#define B(imm28) EMIT(type_I26(0b010100, ((imm28)>>2)))

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

// GR[rd] = SignExtend(GR[rj][15:0], GRLEN)
#define EXT_W_H(rd, rj) EMIT(type_2R(0b10110, rj, rd))
// GR[rd] = SignExtend(GR[rj][7:0], GRLEN)
#define EXT_W_B(rd, rj) EMIT(type_2R(0b10111, rj, rd))

////////////////////////////////////////////////////////////////////////////////
// LBT extension instructions

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
#define X64SLLI_B(rj, imm3)     EMIT(type_2RI3(0x2a1, imm3, rj, 0x0))
#define X64SRLI_B(rj, imm3)     EMIT(type_2RI3(0x2a1, imm3, rj, 0x4))
#define X64SRAI_B(rj, imm3)     EMIT(type_2RI3(0x2a1, imm3, rj, 0x8))
#define X64ROTRI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0xc))
#define X64RCRI_B(rj, imm3)     EMIT(type_2RI3(0x2a1, imm3, rj, 0x10))
#define X64ROTLI_B(rj, imm3)    EMIT(type_2RI3(0x2a1, imm3, rj, 0x14))
#define X64RCLI_B(rj, imm3)     EMIT(type_2RI3(0x2a1, imm3, rj, 0x18))
#define X64SLLI_H(rj, imm4)     EMIT(type_2RI4(0x151, imm4, rj, 0x1))
#define X64SRLI_H(rj, imm4)     EMIT(type_2RI4(0x151, imm4, rj, 0x5))
#define X64SRAI_H(rj, imm4)     EMIT(type_2RI4(0x151, imm4, rj, 0x9))
#define X64ROTRI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0xd))
#define X64RCRI_H(rj, imm4)     EMIT(type_2RI4(0x151, imm4, rj, 0x11))
#define X64ROTLI_H(rj, imm4)    EMIT(type_2RI4(0x151, imm4, rj, 0x15))
#define X64RCLI_H(rj, imm4)     EMIT(type_2RI4(0x151, imm4, rj, 0x19))
#define X64SLLI_W(rj, imm5)     EMIT(type_2RI5(0xa9, imm5, rj, 0x2))
#define X64SRLI_W(rj, imm5)     EMIT(type_2RI5(0xa9, imm5, rj, 0x6))
#define X64SRAI_W(rj, imm5)     EMIT(type_2RI5(0xa9, imm5, rj, 0xa))
#define X64ROTRI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0xe))
#define X64RCRI_W(rj, imm5)     EMIT(type_2RI5(0xa9, imm5, rj, 0x12))
#define X64ROTLI_W(rj, imm5)    EMIT(type_2RI5(0xa9, imm5, rj, 0x16))
#define X64RCLI_W(rj, imm5)     EMIT(type_2RI5(0xa9, imm5, rj, 0x1a))
#define X64SLLI_D(rj, imm6)     EMIT(type_2RI6(0x55, imm6, rj, 0x3))
#define X64SRLI_D(rj, imm6)     EMIT(type_2RI6(0x55, imm6, rj, 0x7))
#define X64SRAI_D(rj, imm6)     EMIT(type_2RI6(0x55, imm6, rj, 0xb))
#define X64ROTRI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0xf))
#define X64RCRI_D(rj, imm6)     EMIT(type_2RI6(0x55, imm6, rj, 0x13))
#define X64ROTLI_D(rj, imm6)    EMIT(type_2RI6(0x55, imm6, rj, 0x17))
#define X64RCLI_D(rj, imm6)     EMIT(type_2RI6(0x55, imm6, rj, 0x1b))

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

#define MOV32w(rd, imm32) MOV32w_(rd, imm32, 1)
// GR[rd] = imm64
#define MOV64x(rd, imm64)                               \
    do {                                                \
        MOV32w_(rd, imm64, 0);                          \
        if (((uint64_t)(imm64)) > 0xffffffffu) {        \
            LU32I_D(rd, ((uint64_t)(imm64)) >> 32);     \
            LU52I_D(rd, rd, ((uint64_t)(imm64)) >> 52); \
        }                                               \
    } while (0)

#define MOV64xw(A, B)     \
    do {                  \
        if (rex.w) {      \
            MOV64x(A, B); \
        } else {          \
            MOV32w(A, B); \
        }                 \
    } while (0)

#define MOV64z(A, B)        \
    do {                    \
        if (rex.is32bits) { \
            MOV32w(A, B);   \
        } else {            \
            MOV64x(A, B);   \
        }                   \
    } while (0)

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

#define LDz(rd, rj, imm12)        \
    do {                          \
        if (rex.is32bits)         \
            LD_WU(rd, rj, imm12); \
        else                      \
            LD_D(rd, rj, imm12);  \
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

#define SUBxw(rd, rj, rk)      \
    do {                       \
        if (rex.w)             \
            SUB_D(rd, rj, rk); \
        else                   \
            SUB_W(rd, rj, rk); \
    } while (0)

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

// DBAR hint
#define DBAR(hint) EMIT(type_hint(0b00111000011100100, hint))

#endif //__ARM64_EMITTER_H__
