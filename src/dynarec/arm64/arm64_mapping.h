#ifndef __ARM64_MAPPING_H__
#define __ARM64_MAPPING_H__


/* 
    ARM64 Linux Call Convention

SP          The Stack Pointer.
r30 LR      The Link Register.
r29 FP      The Frame Pointer
r19…r28     Callee-saved registers
r18         The Platform Register, if needed; otherwise a temporary register. See notes.
r17 IP1     The second intra-procedure-call temporary register (can be used by call veneers and PLT code); at other times may be used as a temporary register.
r16 IP0     The first intra-procedure-call scratch register (can be used by call veneers and PLT code); at other times may be used as a temporary register.
r9…r15      Temporary registers
r8          Indirect result location register
r0…r7       Parameter/result registers

For SIMD:
The first eight registers, v0-v7, are used to pass argument values into a subroutine and to return result values from a function. 
    They may also be used to hold intermediate values within a routine (but, in general, only between subroutine calls).

Registers v8-v15 must be preserved by a callee across subroutine calls; 
    the remaining registers (v0-v7, v16-v31) do not need to be preserved (or should be preserved by the caller).
    Additionally, only the bottom 64 bits of each value stored in v8-v15 need to be preserved [8];
    it is the responsibility of the caller to preserve larger values.

For SVE:
z0-z7 are used to pass scalable vector arguments to a subroutine, and to return scalable vector results from a function.
    If a subroutine takes at least one argument in scalable vector registers or scalable predicate registers,
    or returns results in such regisers, the subroutine must ensure that the entire contents of z8-z23 are preserved across the call.
    In other cases it need only preserve the low 64 bits of z8-z15, as described in SIMD and Floating-Point registers.
p0-p3 are used to pass scalable predicate arguments to a subroutine and to return scalable predicate results from a function.
    If a subroutine takes at least one argument in scalable vector registers or scalable predicate registers,
    or returns results in such registers, the subroutine must ensure that p4-p15 are preserved across the call.
    In other cases it need not preserve any scalable predicate register contents.

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
#define xSavedSP 28

// convert a x86 register to native according to the register mapping
#define TO_NAT(A) (xRAX + (A))
#define IS_GPR(A) ((A)>=xRAX && (A)<=xRIP)

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
#define x87pc   7
// x87 can be a scratch, but check if it's used as x87 PC and restore if needed in that case
// 32bits version of scratch
#define w1      x1
#define w2      x2
#define w3      x3
#define w4      x4
#define w5      x5
#define w6      x6
#define w87pc   x87pc
// emu is r0
#define xEmu    0
// ARM64 LR
#define xLR     30
// ARM64 SP is r31 but is a special register
#define xSP     31      
// xZR regs is 31
#define xZR     31
#define wZR     xZR

// conditions
// Z == 1
#define cEQ 0b0000
// Z != 1
#define cNE 0b0001
// C == 1
#define cCS 0b0010
// C == 1
#define cHS cCS
// C != 1
#define cCC 0b0011
// C != 1
#define cLO cCC
// N == 1
#define cMI 0b0100
// N != 1
#define cPL 0b0101
// V == 1
#define cVS 0b0110
// V != 1
#define cVC 0b0111
// C == 1 && Z == 0
#define cHI 0b1000
// C !=1 || Z == 1
#define cLS 0b1001
// N == V
#define cGE 0b1010
// N != V
#define cLT 0b1011
// N == V && Z == 0
#define cGT 0b1100
// N != V || Z == 1
#define cLE 0b1101
// always
#define c__ 0b1110

//FCMP type of opcode produce:
// if any NAN: CV / v1 == v2: ZC / v1 < v2: N / v1 > v2: C

#endif //__ARM64_MAPPING_H__