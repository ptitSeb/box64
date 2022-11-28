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
#define native_lock_xchg(A, B)              arm64_lock_xchg(A, B)
#define native_lock_xchg_d(A, B)            arm64_lock_xchg_d(A, B)
#define native_lock_storeifref(A, B, C)     arm64_lock_storeifref(A, B, C)
#define native_lock_storeifnull(A, B)       arm64_lock_storeifnull(A, B)
#define native_lock_decifnot0b(A)           arm64_lock_decifnot0b(A)
#define native_lock_storeb(A, B)            arm64_lock_storeb(A, B)

#else
#error Unsupported architecture
#endif

#endif //#define __NATIVE_LOCK__H__