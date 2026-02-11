#ifndef __DYNAREC_HELPER__H_
#define __DYNAREC_HELPER__H_

#include "env.h"
#include "pe_tools.h"
#include "debug.h"

/* Box64 Strong Memory Model Emulation
 *
 * Definition of a SEQ:
 * A SEQ is a sequence of opcodes that writes to guest memory, terminated by JMP, RET, CALL, etc.
 *
 * Memory barriers are added in the following cases to emulate the strong memory model:
 * 1. End of a SEQ:
 *    - Scalar operations (a1)
 *    - SIMD operations (a2)
 * 2. Start of a SEQ:
 *    - Scalar operations (b1)
 *    - SIMD operations (b2)
 * 3. Right before the last guest memory store in a SEQ:
 *    - Scalar operations (c1)
 *    - SIMD operations (c2)
 * 4. After every third guest memory store in a SEQ (d)
 *
 * STRONGMEM levels (coarse-grained):
 * 1: Includes a1, b1, c1
 * 2: Includes LEVEL1, plus a2, b2, c2
 * 3: Includes LEVEL2, plus d
 *
 * WEAKBARRIER levels (fine-grained):
 * 1: Use dmb.ishld and dmb.ishst over dmb.ish for more performance
 * 2. All 1. Plus disabled the last write barriers (c1, c2)
 */

#define STRONGMEM_SIMD_WRITE 2 // The level of SIMD memory writes will be tracked
#define STRONGMEM_LAST_WRITE 1 // The level of a barrier before the last guest memory store will be put
#define STRONGMEM_SEQ_WRITE  3 // The level of a barrier at every third memory store will be  put
#define STRONGMEM_QEMU       4 // The level of a mimic to QEMU's strong memory model

#define STRONGMEM_LEVEL() ((box64_wine && VolatileRangesContains(ip)) ? 0 : BOX64DRENV(dynarec_strongmem))

#if STEP == 1

#define SMWRITE()                                                \
    do {                                                         \
        /* Mark that current sequence writes to guest memory. */ \
        /* This will be used in SMEND for last_write. */         \
        dyn->smwrite = 1;                                        \
        /* Mark that current opcode writes to guest memory. */   \
        dyn->insts[ninst].will_write = 1;                        \
    } while (0)

#define SMWRITELOCK(lock)                   \
    do {                                    \
        if(lock) dyn->insts[ninst].lock = 1;\
        SMWRITE();                          \
    } while (0)

#define SMWRITE2()                                       \
    do {                                                 \
        if (STRONGMEM_LEVEL() >= STRONGMEM_SIMD_WRITE) { \
            dyn->smwrite = 1;                            \
            dyn->insts[ninst].will_write = 2;            \
        }                                                \
    } while (0)

#define SMREAD()                                               \
    do {                                                       \
        /* Mark that current opcode read from guest memory. */ \
        dyn->insts[ninst].will_read = 1;                       \
    } while (0)

#define SMREADLOCK(lock)
#define WILLWRITE()
#define WILLWRITELOCK(lock)

#define SMSTART()                                                  \
    do {                                                           \
        /* Clear current state at the start of a potential SEQ. */ \
        dyn->smwrite = 0;                                          \
    } while (0)

#define SMEND()                                                                                \
    do {                                                                                       \
        /* If there is any guest memory write, which is a SEQ, then compute the last_write. */ \
        if (dyn->smwrite && (STRONGMEM_LEVEL() >= STRONGMEM_LAST_WRITE)) {                     \
            int i = ninst;                                                                     \
            while (i >= 0 && !dyn->insts[i].will_write)                                        \
                --i;                                                                           \
            if (i >= 0) { dyn->insts[i].last_write = 1; }                                      \
        }                                                                                      \
        dyn->smwrite = 0;                                                                      \
    } while (0)

#define SMDMB()

#else

// An opcode writes guest memory, this need to be put after the STORE instruction manually. It will also end the SEQ automaticaly on last_write immediatly
#define SMWRITE()                                                        \
    do {                                                                 \
        if (dyn->insts[ninst].last_write) {                              \
            dyn->smwrite = 1;                                            \
            SMEND();                                                     \
        } else {                                                         \
            /* Put a barrier at every third memory write. */             \
            if (STRONGMEM_LEVEL() >= STRONGMEM_SEQ_WRITE) {              \
                if (++dyn->smwrite >= 3 /* Every third memory write */   \
                    || STRONGMEM_LEVEL() >= STRONGMEM_QEMU) {            \
                    DMB_ISH();                                           \
                    dyn->smwrite = 1;                                    \
                }                                                        \
            } else {                                                     \
                /* Mark that current sequence writes to guest memory. */ \
                dyn->smwrite = 1;                                        \
            }                                                            \
        }                                                                \
    } while (0)

// Similar to SMWRITE, but checks lock.
#define SMWRITELOCK(lock) \
    do {                  \
        if (lock) {       \
            DMB_ISH();    \
        } else {          \
            SMWRITE();    \
        }                 \
    } while (0)

// Similar to SMWRITE, but for SIMD instructions.
#define SMWRITE2()                                     \
    do {                                               \
        if (STRONGMEM_LEVEL() >= STRONGMEM_SIMD_WRITE) \
            SMWRITE();                                 \
    } while (0)

// An opcode reads guest memory, this need to be put before the LOAD instruction manually.
#define SMREAD()                                   \
    do {                                           \
        if (STRONGMEM_LEVEL() >= STRONGMEM_QEMU) { \
            DMB_ISHLD();                           \
        }                                          \
    } while (0)

// Similar to SMREAD, but checks lock.
#define SMREADLOCK(lock) \
    do {                 \
        if (lock) {      \
            DMB_ISH();   \
        } else {         \
            SMREAD();    \
        }                \
    } while (0)

// An opcode will write memory, this will be put before the STORE instruction automatically.
#define WILLWRITE()                                                                                \
    do {                                                                                           \
        if (STRONGMEM_LEVEL() >= dyn->insts[ninst].will_write && dyn->smwrite == 0) {              \
            /* Will write but never written, this is the start of a SEQ, put a barrier. */         \
            if (BOX64ENV(dynarec_weakbarrier))                                                     \
                DMB_ISHLD();                                                                       \
            else                                                                                   \
                DMB_ISH();                                                                         \
        } else if (STRONGMEM_LEVEL() >= STRONGMEM_LAST_WRITE && BOX64ENV(dynarec_weakbarrier) != 2 \
            && dyn->insts[ninst].last_write) {                                                     \
            /* Last write, put a barrier */                                                        \
            if (BOX64ENV(dynarec_weakbarrier))                                                     \
                DMB_ISHST();                                                                       \
            else                                                                                   \
                DMB_ISH();                                                                         \
        }                                                                                          \
    } while (0)

// Similar to WILLWRITE, but checks lock.
#define WILLWRITELOCK(lock) \
    do {                    \
        if (lock) {         \
            DMB_ISH();      \
        } else {            \
            WILLWRITE();    \
        }                   \
    } while (0)

// Used to clear the state at the start of a SEQ
#define SMSTART()         \
    do {                  \
        dyn->smwrite = 0; \
    } while (0)

// Will be put at the end of the SEQ
#define SMEND()                                                         \
    do {                                                                \
        if (STRONGMEM_LEVEL()) {                                        \
            /* It's a SEQ, put a barrier here. */                       \
            if (dyn->smwrite) {                                         \
                /* Check if the next instruction has a end loop mark */ \
                if (BOX64ENV(dynarec_weakbarrier))                      \
                    DMB_ISHST();                                        \
                else                                                    \
                    DMB_ISH();                                          \
            }                                                           \
        }                                                               \
        dyn->smwrite = 0;                                               \
    } while (0)

// The barrier.
#define SMDMB() DMB_ISH()
#endif

int is_addr_unaligned(uintptr_t addr);
int is_addr_autosmc(uintptr_t addr);

#ifdef ARM64
#include "arm64/dynarec_arm64_helper.h"
#elif defined(LA64)
#include "la64/dynarec_la64_helper.h"
#elif defined(RV64)
#include "rv64/dynarec_rv64_helper.h"
#else
#error Unsupported architecture
#endif

#endif //__DYNAREC_HELPER__H_
