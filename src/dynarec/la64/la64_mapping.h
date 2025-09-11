#ifndef __LA64_MAPPING_H__
#define __LA64_MAPPING_H__

// LA64 Register Mapping Scheme
/*****************************************************************************************
name    alias  mapping      native description              Box64 description       saver
******************************************************************************************
r0      zero   native zero  Hard-wired zero                 N/A                     -
r1      ra     native ra    Return address                  N/A                     Caller
r2      tp     -            Thread pointer                  N/A                     -
r3      sp     native sp    Stack pointer                   N/A                     Callee
r4      a0     RDI          Function argument/return val.   -                       Caller
r5      a1     RSI          Function argument/return val.   -                       Caller
r6      a2     RDX          Function argument               -                       Caller
r7      a3     RCX          Function argument               -                       Caller
r8      a4     R8           Function argument               -                       Caller
r9      a5     R9           Function argument               -                       Caller
r10     a6     RBX          Function argument               -                       Caller
r11     a7     RSP          Function argument               -                       Caller
r12     t0     RAX          Temporary                       -                       Caller
r13     t1     x1           Temporary                       Scratch                 Caller
r14     t2     x2           Temporary                       Scratch                 Caller
r15     t3     x3           Temporary                       Scratch                 Caller
r16     t4     x4           Temporary                       Scratch                 Caller
r17     t5     x5           Temporary                       Scratch                 Caller
r18     t6     x6           Temporary                       Scratch                 Caller
r19     t7     -            Temporary                       -                       Caller
r20     t8     x7           Temporary                       Scratch                 Caller
r21     rx     -            Reserved                        N/A                     -
r22     fp     RBP          Saved register/frame pointer    -                       Callee
r23     s0     R10          Saved register                  -                       Callee
r24     s1     R11          Saved register                  -                       Callee
r25     s2     R12          Saved register                  -                       Callee
r26     s3     R13          Saved register                  -                       Callee
r27     s4     R14          Saved register                  -                       Callee
r28     s5     R15          Saved register                  -                       Callee
r29     s6     RIP          Saved register                  -                       Callee
r30     s7     FLAGS        Saved register                  -                       Callee
r31     s8     xEmu         Saved register                  The Emu struct          Callee
******************************************************************************************/

#ifndef ASM_MAPPING

// x86 Register mapping
#define xRAX     12
#define xRCX     7
#define xRDX     6
#define xRBX     10
#define xRSP     11
#define xRBP     22
#define xRSI     5
#define xRDI     4
#define xR8      8
#define xR9      9
#define xR10     23
#define xR11     24
#define xR12     25
#define xR13     26
#define xR14     27
#define xR15     28
#define xFlags   30
#define xRIP     29

// convert a x86 register to native according to the register mapping
#define TO_NAT(A) (((uint8_t[]) { 12, 7, 6, 10, 11, 22, 5, 4, 8, 9, 23, 24, 25, 26, 27, 28 })[(A)])

// scratch registers
#define x1 13
#define x2 14
#define x3 15
#define x4 16
#define x5 17
#define x6 18
#define x7 20

// emu is $r31
#define xEmu 31
// LA64 RA
#define xRA 1
#define ra  xRA
// LA64 SP
#define xSP 3
// LA64 args
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

#else

// x86 Register mapping
#define RAX     $r12
#define RCX     $r7
#define RDX     $r6
#define RBX     $r10
#define RSP     $r11
#define RBP     $r22
#define RSI     $r5
#define RDI     $r4
#define R8      $r8
#define R9      $r9
#define R10     $r23
#define R11     $r24
#define R12     $r25
#define R13     $r26
#define R14     $r27
#define R15     $r28
#define Flags   $r30
#define RIP     $r29
#define Emu     $r31

#ifdef LA64_ABI_1

.macro ret
    jr  $ra
.endm

#endif // LA64_ABI_1

#endif // ASM_MAPPING

#endif //__LA64_MAPPING_H__
