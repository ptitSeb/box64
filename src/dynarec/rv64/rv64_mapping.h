#ifndef __RV64_MAPPING_H__
#define __RV64_MAPPING_H__

// RV64 Register Mapping Scheme
/*****************************************************************************************
reg     name   mapping      native description              Box64 description       saver
******************************************************************************************
x0      zero   native zero  Hard-wired zero                 N/A                     —
x1      ra     native ra    Return address                  N/A                     Caller
x2      sp     native sp    Stack pointer                   N/A                     Callee
x3      gp     native gp    Global pointer                  N/A                     —
x4      tp     native tp    Thread pointer                  N/A                     —
x5      t0     -            Temporary                       X87 Precision Control   Caller
x6      t1     x1           Temporary                       Scratch                 Caller
x7      t2     x2           Temporary                       Scratch                 Caller
x8      s0/fp  RBP          Saved register/frame pointer    -                       Callee
x9      s1     RSP          Saved register                  -                       Callee
x10     a0     RDI          Function argument/return val.   -                       Caller
x11     a1     RSI          Function argument/return val.   -                       Caller
x12     a2     RDX          Function argument               -                       Caller
x13     a3     RCX          Function argument               -                       Caller
x14     a4     R8           Function argument               -                       Caller
x15     a5     R9           Function argument               -                       Caller
x16     a6     RAX          Function argument               -                       Caller
x17     a7     x7           Function argument               Scratch                 Caller
x18     s2     R12          Saved register                  -                       Callee
x19     s3     R13          Saved register                  -                       Callee
x20     s4     R14          Saved register                  -                       Callee
x21     s5     R15          Saved register                  -                       Callee
x22     s6     RIP          Saved register                  -                       Callee
x23     s7     FLAGS        Saved register                  -                       Callee
x24     s8     RBX          Saved register                  -                       Callee
x25     s9     xEmu         Saved register                  The Emu struct          Callee
x26     s10    R10          Saved register                  -                       Callee
x27     s11    R11          Saved register                  -                       Callee
x28     t3     x3           Temporary                       Scratch                 Caller
x29     t4     x4           Temporary                       Scratch                 Caller
x30     t5     x5           Temporary                       Scratch                 Caller
x31     t6     x6           Temporary                       Scratch                 Caller
******************************************************************************************/

// replacement for F_OF internaly, using a reserved bit. Need to use F_OF2 internaly, never F_OF directly!
#define F_OF2 F_res3

#ifndef ASM_MAPPING

#include <stdint.h>

// x86 Register mapping
#define xRAX   16
#define xRCX   13
#define xRDX   12
#define xRBX   24
#define xRSP   9
#define xRBP   8
#define xRSI   11
#define xRDI   10
#define xR8    14
#define xR9    15
#define xR10   26
#define xR11   27
#define xR12   18
#define xR13   19
#define xR14   20
#define xR15   21
#define xRIP   22
#define xFlags 23

// convert a x86 register to native according to the register mapping
#define TO_NAT(A) (((uint8_t[]) { 16, 13, 12, 24, 9, 8, 11, 10, 14, 15, 26, 27, 18, 19, 20, 21 })[(A)])
#define IS_GPR(A) (((uint8_t[]) { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0 })[(A)])

#define x1   6
#define x2   7
#define x3   28
#define x4   29
#define x5   30
#define x6   31
#define xEmu 25

#define x7    17
#define x87pc 5

#define xRA 1
#define xSP 2
#define A0  10
#define A1  11
#define A2  12
#define A3  13
#define A4  14
#define A5  15
#define A6  16
#define A7  17

#define xZR 0

#else

// x86 Register mapping
#define RAX   x16
#define RCX   x13
#define RDX   x12
#define RBX   x24
#define RSP   x9
#define RBP   x8
#define RSI   x11
#define RDI   x10
#define R8    x14
#define R9    x15
#define R10   x26
#define R11   x27
#define R12   x18
#define R13   x19
#define R14   x20
#define R15   x21
#define RIP   x22
#define Flags x23
#define Emu   x25

#endif

#endif // __RV64_MAPPING_H__
