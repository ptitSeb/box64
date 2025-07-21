#ifndef __LA64_MAPPING_H__
#define __LA64_MAPPING_H__


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
#define xRAX     12
#define xRCX     13
#define xRDX     14
#define xRBX     15
#define xRSP     16
#define xRBP     17
#define xRSI     18
#define xRDI     19
#define xR8      23
#define xR9      24
#define xR10     25
#define xR11     26
#define xR12     27
#define xR13     28
#define xR14     29
#define xR15     30
#define xFlags   31
#define xRIP     20
#define xSavedSP 22

// convert a x86 register to native according to the register mapping
#define TO_NAT(A) (xRAX + (A) + (((A) > 7) ? 3 : 0))

// scratch registers
#define x1 5
#define x2 6
#define x3 7
#define x4 8
#define x5 9
#define x6 10
#define x7 11

// emu is r0
#define xEmu 4
// LA64 RA
#define xRA 1
#define ra  xRA
// LA64 SP
#define xSP 3
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
#define xZR 0
#define wZR xZR
#define r0  xZR

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

#endif //__LA64_MAPPING_H__