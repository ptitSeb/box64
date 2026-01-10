#ifndef __HOSTEXT_H__
#define __HOSTEXT_H__

#include <stdint.h>

typedef union cpu_ext_s {
    struct {
#ifdef ARM64
        uint64_t atomics : 1; // it's important this is the 1st bit
        uint64_t asimd : 1;
        uint64_t aes : 1;
        uint64_t pmull : 1;
        uint64_t crc32 : 1;
        uint64_t sha1 : 1;
        uint64_t sha2 : 1;
        uint64_t uscat : 1;
        uint64_t flagm : 1;
        uint64_t flagm2 : 1;
        uint64_t frintts : 1;
        uint64_t afp : 1;
        uint64_t rndr : 1;
#elif defined(RV64)
        uint64_t vlen : 8; // Not *8, 8bits should be enugh? that's 2048 vector
        uint64_t zba : 1;
        uint64_t zbb : 1;
        uint64_t zbc : 1;
        uint64_t zbs : 1;
        uint64_t vector : 1; // rvv 1.0 or xtheadvector
        uint64_t xtheadvector : 1;
        uint64_t xtheadba : 1;
        uint64_t xtheadbb : 1;
        uint64_t xtheadbs : 1;
        uint64_t xtheadcondmov : 1;
        uint64_t xtheadmemidx : 1;
        uint64_t xtheadmempair : 1;
        uint64_t xtheadfmemidx : 1;
        uint64_t xtheadmac : 1;
        uint64_t xtheadfmv : 1;
#elif defined(LA64)
        uint64_t lbt : 1; // it's important it's stay the 1st bit
        uint64_t lam_bh : 1;
        uint64_t lamcas : 1;
        uint64_t scq : 1;
        uint64_t frecipe : 1;
        uint64_t lasx : 1;
#endif
    };
    uint64_t x;
} cpu_ext_t;

int DetectHostCpuFeatures(void);
void PrintHostCpuFeatures(void);

#endif //__HOSTEXT_H__