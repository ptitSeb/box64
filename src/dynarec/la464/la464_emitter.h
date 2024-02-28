#ifndef __LA464_EMITTER_H__
#define __LA464_EMITTER_H__
/*
    LA464 Emitter
*/

// LA464 ABI
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
// x86 Register mapping
#define xRAX     12
#define xRCX     13
#define xRDX     14
#define xRBX     15
#define xRSP     16
#define xRBP     17
#define xRSI     18
#define xRDI     19
#define xR8      20
#define xR9      22
#define xR10     23
#define xR11     24
#define xR12     25
#define xR13     26
#define xR14     27
#define xR15     28
#define xFlags   29
#define xRIP     30
#define xSavedSP 31
// 32bits version
#define wEAX   xRAX
#define wECX   xRCX
#define wEDX   xRDX
#define wEBX   xRBX
#define wESP   xRSP
#define wEBP   xRBP
#define wESI   xRSI
#define wEDI   xRDI
#define wR8    xR8
#define wR9    xR9
#define wR10   xR10
#define wR11   xR11
#define wR12   xR12
#define wR13   xR13
#define wR14   xR14
#define wR15   xR15
#define wFlags xFlags
// scratch registers
#define x1 5
#define x2 6
#define x3 7
#define x4 8
#define x5 9
#define x6 10
#define x7 11
// 32bits version of scratch
#define w1 x1
#define w2 x2
#define w3 x3
#define w4 x4
#define w5 x5
#define w6 x6
#define w7 x7
// emu is r4
#define xEmu 4
// LOONGARCH64 RA
#define xRA 1
#define xSP 3
// LOONGARCH64 args
#define A0 4
#define A1 5
#define A2 6
#define A3 7
#define A4 8
#define A5 9
#define A6 10
#define A7 11
// xZR reg is 0
#define xZR 0
#define wZR xZR

// split a 32bits value in 20bits + 12bits, adjust the upper part is 12bits is negative
#define SPLIT20(A) (((A) + 0x800) >> 12)
#define SPLIT12(A) ((A) & 0xfff)

// ZERO the upper part
#define ZEROUP(r)               \
    do {                        \
        MOV32w(x2, 0xffffffff); \
        AND(r, r, x2);          \
    } while (0);

#define type_4R(opc, ra, rk, rj, rd)     ((opc) << 20 | (ra) << 15 | (rk) << 10 | (rj) << 5 | (rd))
#define type_3R(opc, rk, rj, rd)         ((opc) << 15 | (rk) << 10 | (rj) << 5 | (rd))
#define type_3RI2(opc, imm2, rk, rj, rd) ((opc) << 17 | ((imm2) & 0x3) << 15 | (rk) << 10 | (rj) << 5 | (rd))
#define type_2R(opc, rj, rd)             ((opc) << 10 | (rj) << 5 | (rd))
#define type_2RI5(opc, imm5, rj, rd)     ((opc) << 15 | ((imm5) & 0x1F) << 10 | (rj) << 5 | (rd))
#define type_2RI6(opc, imm6, rj, rd)     ((opc) << 16 | ((imm6) & 0x3F) << 10 | (rj) << 5 | (rd))
#define type_2RI8(opc, imm8, rj, rd)     ((opc) << 18 | ((imm8) & 0xFF) << 10 | (rj) << 5 | (rd))
#define type_2RI12(opc, imm12, rj, rd)   ((opc) << 22 | ((imm12) & 0xFFF) << 10 | (rj) << 5 | (rd))
#define type_2RI14(opc, imm14, rj, rd)   ((opc) << 24 | ((imm14) & 0x3FFF) << 10 | (rj) << 5 | (rd))
#define type_2RI16(opc, imm16, rj, rd)   ((opc) << 26 | ((imm16) & 0xFFFF) << 10 | (rj) << 5 | (rd))
#define type_1RI20(opc, imm20, rd)       ((opc) << 25 | ((imm20) & 0xFFFFF) << 5 | (rd))
#define type_1RI21(opc, imm21, rj)       ((opc) << 26 | ((imm21) & 0xFFFF) << 10 | (rj) << 5 | ((imm21) & 0x1F0000) >> 16)
#define type_hint(opc, imm15)            ((opc) << 15 | ((imm15) & 0x7FFF))
#define type_I26(opc, imm26)             ((opc) << 26 | ((imm26) & 0xFFFF) << 10 | ((imm26) & 0x3FF0000))

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

// tmp = (GR[rj][31:0] << (imm2 + 1)) + GR[rk][31:0]
// GR[rd] = SignExtend(tmp[31:0], GRLEN)
#define ALSL_W(rd, rj, rk, imm2) EMIT(type_3RI2(0b000000000000010, imm2, rk, rj, rd))
// tmp = (GR[rj][31:0] << (imm2 + 1)) + GR[rk][31:0]
// GR[rd] = ZeroExtend(tmp[31:0], GRLEN)
#define ALSL_WU(rd, rj, rk, imm2) EMIT(type_3RI2(0b000000000000011, imm2, rk, rj, rd))
// tmp = (GR[rj][63:0] << (imm2 + 1)) + GR[rk][63:0]
// GR[rd] = tmp[63:0]
#define ALSL_D(rd, rj, rk, imm2) EMIT(type_3RI2(0b000000000010110, imm2, rk, rj, rd))

// GR[rd] = SignExtend({imm20, 12'b0}, GRLEN)
#define LU12I_W(rd, imm20) EMIT(type_1RI20(0b0001010, imm20, rd))
// GR[rd] = {SignExtend(imm20, 32), GR[rd][31:0]}
#define LU32I_D(rd, imm20) EMIT(type_1RI20(0b0001011, imm20, rd))
// GR[rd] = {imm12, GR[rj][51:0]}
#define LU52I_D(rd, rj, imm12) EMIT(type_2RI12(0b0000001100, imm12, rj, rd))

// GR[rd] = PC + SignExtend({imm20, 2'b0}, GRLEN)
#define PCADDI(rd, imm20) EMIT(type_1RI20(0b0001100, imm20, rd))
// GR[rd] = PC + SignExtend({imm20, 12'b0}, GRLEN)
#define PCADDU12I(rd, imm20) EMIT(type_1RI20(0b0001110, imm20, rd))
// GR[rd] = PC + SignExtend({imm20, 18'b0}, GRLEN)
#define PCADDU18I(rd, imm20) EMIT(type_1RI20(0b0001111, imm20, rd))
// tmp = PC + SignExtend({imm20, 12'b0}, GRLEN)
// GR[rd] = {tmp[GRLEN-1:12], 12'b0}
#define PCALAU12I(rd, imm20) EMIT(type_1RI20(0b0001101, imm20, rd))

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

// GR[rd] = SLL(GR[rj][63:0], imm6) (Shift Left Logical)
#define SLLI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001000001, imm6, rj, rd))
// GR[rd] = SRL(GR[rj][63:0], imm6) (Shift Right Logical)
#define SRLI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001000101, imm6, rj, rd))
// GR[rd] = SRA(GR[rj][63:0], imm6) (Shift Right Arithmetic)
#define SRAI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001001001, imm6, rj, rd))
// GR[rd] = ROTR(GR[rj][63:0], imm6) (Rotate To Right)
#define ROTRI_D(rd, rj, imm6) EMIT(type_2RI6(0b0000000001001101, imm6, rj, rd))

// rd = rj + (rk << imm6)
#define ADDSL(rd, rs1, rs2, imm6, scratch) \
    if (!(imm6)) {                         \
        ADD_D(rd, rs1, rs2);               \
    } else {                               \
        SLLI_D(scratch, rs2, imm6);        \
        ADD_D(rd, rs1, scratch);           \
    }

// if GR[rj] == 0:
//     PC = PC + SignExtend({imm21, 2'b0}, GRLEN)
#define BEQZ(rj, imm21) EMIT(type_1RI21(0b010000, (imm21) >> 2, rj))
// if GR[rj] != 0:
//     PC = PC + SignExtend({imm21, 2'b0}, GRLEN)
#define BNEZ(rj, imm21) EMIT(type_1RI21(0b010001, (imm21) >> 2, rj))

// GR[rd] = PC + 4
// PC = GR[rj] + SignExtend({imm16, 2'b0}, GRLEN)
#define JIRL(rd, rj, imm16) EMIT(type_2RI16(0b010011, imm16, rj, rd))
// PC = GR[rj]
#define BR(rj) JIRL(xZR, rj, 0x0)

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

// GR[rd] = imm32
#define MOV32w(rd, imm32)               \
    if (((uint32_t)(imm32)) > 0xfffu) { \
        LU12I_W(rd, (imm32) >> 12);     \
        ORI(rd, rd, imm32);             \
    } else {                            \
        ORI(rd, xZR, imm32);            \
    }
// GR[rd] = imm64
#define MOV64x(rd, imm64)                           \
    MOV32w(rd, imm64);                              \
    if (((uint64_t)(imm64)) > 0xffffffffu) {        \
        LU32I_D(rd, ((uint64_t)(imm64)) >> 32);     \
        LU52I_D(rd, rd, ((uint64_t)(imm64)) >> 52); \
    }

// rd[63:0] = rj[63:0] (pseudo instruction)
#define MV(rd, rj) ADDI_D(rd, rj, 0)
// rd = rj (pseudo instruction)
#define MVxw(rd, rj)            \
    if (rex.w) {                \
        MV(rd, rj);             \
    } else {                    \
        MOV32w(x2, 0xffffffff); \
        AND(rd, rj, x2);        \
    }
// rd = rj (pseudo instruction)
#define MVz(rd, rj)             \
    if (rex.is32bits) {         \
        MOV32w(x2, 0xffffffff); \
        AND(rd, rj, x2);        \
    } else {                    \
        MV(rd, rj);             \
    }

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
#define POP1z(reg)      \
    if (rex.is32bits) { \
        POP1_32(reg);   \
    } else {            \
        POP1(reg);      \
    }
// PUSH reg
#define PUSH1z(reg)     \
    if (rex.is32bits) { \
        PUSH1_32(reg);  \
    } else {            \
        PUSH1(reg);     \
    }

// DBAR hint
#define DBAR(hint) EMIT(type_hint(0b00111000011100100, hint))

#endif //__ARM64_EMITTER_H__