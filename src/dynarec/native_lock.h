#ifndef __NATIVE_LOCK__H__
#define __NATIVE_LOCK__H__

#ifdef ARM64
#include "arm64/arm64_lock.h"

#define native_lock_read_b(A)               arm64_lock_read_b(A)
#define native_lock_write_b(A, B)           arm64_lock_write_b(A, B)
#define native_lock_read_h(A)               arm64_lock_read_h(A)
#define native_lock_write_h(A, B)           arm64_lock_write_h(A, B)
#define native_lock_read_d(A)               arm64_lock_read_d(A)
#define native_lock_write_d(A, B)           arm64_lock_write_d(A, B)
#define native_lock_read_dd(A)              arm64_lock_read_dd(A)
#define native_lock_write_dd(A, B)          arm64_lock_write_dd(A, B)
#define native_lock_read_dq(A, B, C)        arm64_lock_read_dq(A, B, C)
#define native_lock_write_dq(A, B, C)       arm64_lock_write_dq(A, B, C)
#define native_lock_xchg_dd(A, B)           arm64_lock_xchg_dd(A, B)
#define native_lock_xchg_d(A, B)            arm64_lock_xchg_d(A, B)
#define native_lock_xchg_h(A, B)            arm64_lock_xchg_h(A, B)
#define native_lock_xchg_b(A, B)            arm64_lock_xchg_b(A, B)
#define native_lock_storeifref(A, B, C)     arm64_lock_storeifref(A, B, C)
#define native_lock_storeifref_d(A, B, C)   arm64_lock_storeifref_d(A, B, C)
#define native_lock_storeifref2_d(A, B, C)  arm64_lock_storeifref2_d(A, B, C)
#define native_lock_storeifnull(A, B)       arm64_lock_storeifnull(A, B)
#define native_lock_storeifnull_d(A, B)     arm64_lock_storeifnull_d(A, B)
// #define native_lock_decifnot0b(A)           arm64_lock_decifnot0b(A)
#define native_lock_storeb(A, B)            arm64_lock_storeb(A, B)
#define native_lock_incif0(A)               arm64_lock_incif0(A)
#define native_lock_decifnot0(A)            arm64_lock_decifnot0(A)
#define native_lock_store(A, B)             arm64_lock_store(A, B)
#define native_lock_store_dd(A, B)          arm64_lock_store_dd(A, B)
#define native_lock_get_b(A)                arm64_lock_get_b(A)
#define native_lock_get_d(A)                arm64_lock_get_d(A)
#define native_lock_get_dd(A)               arm64_lock_get_dd(A)

#elif defined(RV64)
#include "rv64/rv64_lock.h"

#define USE_CAS
// RV64 is quite strict (or at least strongly recommand) on what you can do between an LR and an SC
// That basicaly forbid to call a function, so there cannot be READ / WRITE separated
// And so need to use a Compare and Swap mecanism instead

// no byte or 2-bytes atomic access on RISC-V
#define native_lock_xchg_dd(A, B)           rv64_lock_xchg_dd(A, B)
#define native_lock_xchg_d(A, B)            rv64_lock_xchg_d(A, B)
#define native_lock_storeifref(A, B, C)     rv64_lock_storeifref(A, B, C)
#define native_lock_storeifref_d(A, B, C)   rv64_lock_storeifref_d(A, B, C)
#define native_lock_storeifref2_d(A, B, C)  rv64_lock_storeifref2_d(A, B, C)
#define native_lock_storeifnull(A, B)       rv64_lock_storeifnull(A, B)
#define native_lock_storeifnull_d(A, B)     rv64_lock_storeifnull_d(A, B)
// #define native_lock_decifnot0b(A)           rv64_lock_decifnot0b(A)
#define native_lock_storeb(A, B)            rv64_lock_storeb(A, B)
#define native_lock_incif0(A)               rv64_lock_incif0(A)
#define native_lock_decifnot0(A)            rv64_lock_decifnot0(A)
#define native_lock_store(A, B)             rv64_lock_store(A, B)
#define native_lock_store_dd(A, B)          rv64_lock_store_dd(A, B)
#define native_lock_cas_d(A, B, C)          rv64_lock_cas_d(A, B, C)
#define native_lock_cas_dd(A, B, C)         rv64_lock_cas_dd(A, B, C)
#define native_lock_xchg_b(A, B)            rv64_lock_xchg_b(A, B)
#define native_lock_read_b(A)               tmpcas=*(uint8_t*)(A)
#define native_lock_write_b(A, B)           rv64_lock_cas_b(A, tmpcas, B)
#define native_lock_read_h(A)               tmpcas=*(uint16_t*)(A)
#define native_lock_write_h(A, B)           rv64_lock_cas_h(A, tmpcas, B)
#define native_lock_read_d(A)               tmpcas=*(uint32_t*)(A)
#define native_lock_write_d(A, B)           rv64_lock_cas_d(A, tmpcas, B)
#define native_lock_read_dd(A)              tmpcas=*(uint64_t*)(A)
#define native_lock_write_dd(A, B)          rv64_lock_cas_dd(A, tmpcas, B)
// there is no atomic move on 16bytes, so faking it
#define native_lock_read_dq(A, B, C)       \
    do {                                   \
        *A = tmpcas = ((uint64_t*)(C))[0]; \
        *B = ((uint64_t*)(C))[1];          \
    } while (0)
#define native_lock_write_dq(A, B, C)       rv64_lock_cas_dq(C, A, tmpcas, B);
#define native_lock_get_b(A)                rv64_lock_get_b(A)
#define native_lock_get_d(A)                rv64_lock_get_d(A)
#define native_lock_get_dd(A)               rv64_lock_get_dd(A)

#elif defined(LA64)
#include "la64/la64_lock.h"

#define USE_CAS
// LA64 is quite strict (or at least strongly recommand) on what you can do between an LL and an SC
// That basicaly forbid to call a function, so there cannot be READ / WRITE separated
// And so need to use a Compare and Swap mecanism instead

#define native_lock_xchg_dd(A, B)           la64_lock_xchg_dd(A, B)
#define native_lock_xchg_d(A, B)            la64_lock_xchg_d(A, B)
#define native_lock_storeifref(A, B, C)     la64_lock_storeifref(A, B, C)
#define native_lock_storeifref_d(A, B, C)   la64_lock_storeifref_d(A, B, C)
#define native_lock_storeifref2_d(A, B, C)  la64_lock_storeifref2_d(A, B, C)
#define native_lock_storeifnull(A, B)       la64_lock_storeifnull(A, B)
#define native_lock_storeifnull_d(A, B)     la64_lock_storeifnull_d(A, B)
// #define native_lock_decifnot0b(A)           la64_lock_decifnot0b(A)
#define native_lock_storeb(A, B)            la64_lock_storeb(A, B)
#define native_lock_incif0(A)               la64_lock_incif0(A)
#define native_lock_decifnot0(A)            la64_lock_decifnot0(A)
#define native_lock_store(A, B)             la64_lock_store(A, B)
#define native_lock_store_dd(A, B)          la64_lock_store_dd(A, B)
#define native_lock_cas_d(A, B, C)          la64_lock_cas_d(A, B, C)
#define native_lock_cas_dd(A, B, C)         la64_lock_cas_dd(A, B, C)
#define native_lock_xchg_b(A, B) \
    la64_lam_bh ? la64_lock_xchg_b(A, B) : la64_lock_xchg_b_slow(A, B)
#define native_lock_read_b(A)               tmpcas=*(uint8_t*)(A)
#define native_lock_write_b(A, B) \
    la64_lamcas ? la64_lock_cas_b(A, tmpcas, B) : la64_lock_cas_b_slow(A, tmpcas, B)
#define native_lock_read_h(A)               tmpcas=*(uint16_t*)(A)
#define native_lock_write_h(A, B) \
    la64_lamcas ? la64_lock_cas_h(A, tmpcas, B) : la64_lock_cas_h_slow(A, tmpcas, B)
#define native_lock_read_d(A)               tmpcas=*(uint32_t*)(A)
#define native_lock_write_d(A, B)           la64_lock_cas_d(A, tmpcas, B)
#define native_lock_read_dd(A)              tmpcas=*(uint64_t*)(A)
#define native_lock_write_dd(A, B)          la64_lock_cas_dd(A, tmpcas, B)
#define native_lock_read_dq(A, B, C)       \
    do {                                   \
        *A = tmpcas = ((uint64_t*)(C))[0]; \
        *B = ((uint64_t*)(C))[1];          \
    } while (0)
#define native_lock_write_dq(A, B, C) la64_lock_cas_dq(C, A, tmpcas, B);
#define native_lock_get_b(A)          la64_lock_get_b(A)
#define native_lock_get_d(A)          la64_lock_get_d(A)
#define native_lock_get_dd(A)         la64_lock_get_dd(A)

#else
#error Unsupported architecture
#endif

#endif //#define __NATIVE_LOCK__H__
