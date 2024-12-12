#define ASM_MAPPING 1
#include "rv64_mapping.h"
#undef ASM_MAPPING

.text
.align 4

.extern LinkNext

.global rv64_next

    // NULL pointer before rv64_next, for getDB
    .8byte  0

// rv64(void)
rv64_next:

    // 16 bytes aligned
    addi    sp,  sp,  -(8 * 10)

    // push regs we care that might be destoryed
    sd      RDI, (0*8)(sp)
    sd      RSI, (1*8)(sp)
    sd      RDX, (2*8)(sp)
    sd      RCX, (3*8)(sp)
    sd      R8,  (4*8)(sp)
    sd      R9,  (5*8)(sp)
    sd      RAX, (6*8)(sp)
    sd      RIP, (8*8)(sp)

    mv      a0, Emu
    mv      a1, RIP
#ifdef HAVE_TRACE
    mv      a2, ra          // "from" is in ra, so put in a2
#endif
    addi    a3, sp, 8*8     // a3 is address to change rip

    // call the function
1:
    auipc   t4, %pcrel_hi(LinkNext)
    jalr    t4, %pcrel_lo(1b)

    // reserve return value
    mv      t3, a0

    // pop regs
    ld      RDI, (0*8)(sp)
    ld      RSI, (1*8)(sp)
    ld      RDX, (2*8)(sp)
    ld      RCX, (3*8)(sp)
    ld      R8,  (4*8)(sp)
    ld      R9,  (5*8)(sp)
    ld      RAX, (6*8)(sp)
    ld      RIP, (8*8)(sp)

    addi    sp,  sp,  (8 * 10)

    // return offset is jump address
    jr      t3

