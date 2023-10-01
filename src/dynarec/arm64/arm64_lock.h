#ifndef __ARM64_LOCK__H__
#define __ARM64_LOCK__H__
#include <stdint.h>

// LDAXRB of ADDR
extern uint8_t arm64_lock_read_b(void* addr);
// STLXRB of ADDR, return 0 if ok, 1 if not
extern int arm64_lock_write_b(void* addr, uint8_t val);

// LDAXRH of ADDR
extern uint16_t arm64_lock_read_h(void* addr);
// STLXRH of ADDR, return 0 if ok, 1 if not
extern int arm64_lock_write_h(void* addr, uint16_t val);

// LDAXR of ADDR
extern uint32_t arm64_lock_read_d(void* addr);
// STLXR of ADDR, return 0 if ok, 1 if not
extern int arm64_lock_write_d(void* addr, uint32_t val);

// LDAXR of ADDR
extern uint64_t arm64_lock_read_dd(void* addr);
// STLXR of ADDR, return 0 if ok, 1 if not
extern int arm64_lock_write_dd(void* addr, uint64_t val);

// LDAXRD of ADDR
extern void arm64_lock_read_dq(uint64_t * a, uint64_t* b, void* addr);
// STLXRD of ADDR, return 0 if ok, 1 if not
extern int arm64_lock_write_dq(uint64_t a, uint64_t b, void* addr);

// Atomically exchange value at [p] with val, return old p
extern uintptr_t arm64_lock_xchg_dd(void* p, uintptr_t val);

// Atomically exchange value at [p] with val, return old p
extern uint32_t arm64_lock_xchg_d(void* p, uint32_t val);

// Atomically exchange value at [p] with val, return old p
extern uint32_t arm64_lock_xchg_h(void* p, uint32_t val);

// Atomically exchange value at [p] with val, return old p
extern uint32_t arm64_lock_xchg_b(void* p, uint32_t val);

// Atomically store value to [p] only if [p] is NULL. Return old [p] value
extern uint32_t arm64_lock_storeifnull_d(void*p, uint32_t val);

// Atomically store value to [p] only if [p] is NULL. Return old [p] value
extern void* arm64_lock_storeifnull(void*p, void* val);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern void* arm64_lock_storeifref(void*p, void* val, void* ref);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern uint32_t arm64_lock_storeifref_d(void*p, uint32_t val, uint32_t ref);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern uint32_t arm64_lock_storeifref2_d(void*p, uint32_t val, uint32_t ref);

// decrement atomically the byte at [p] (but only if p not 0)
extern void arm64_lock_decifnot0b(void*p);

// atomic store (with memory barrier)
extern void arm64_lock_storeb(void*p, uint8_t b);

// increment atomically the int at [p] only if it was 0. Return the old value of [p]
extern int arm64_lock_incif0(void*p);

// decrement atomically the int at [p] (but only if p not 0)
extern int arm64_lock_decifnot0(void*p);

// atomic store (with memory barrier)
extern void arm64_lock_store(void*p, uint32_t v);

// atomic store (with memory barrier)
extern void arm64_lock_store_dd(void*p, uint64_t v);

// atomic get (with memory barrier)
extern uint8_t arm64_lock_get_b(void*p);

// atomic get (with memory barrier)
extern uint32_t arm64_lock_get_d(void*p);

// atomic get (with memory barrier)
extern void* arm64_lock_get_dd(void*p);

#endif  //__ARM64_LOCK__H__
