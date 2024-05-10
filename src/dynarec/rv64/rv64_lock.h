#ifndef __RV64_LOCK__H__
#define __RV64_LOCK__H__
#include <stdint.h>

// Atomically store val at [p] if old [p] is ref. Return 0 if OK, 1 is not. p needs to be aligned
extern int rv64_lock_cas_d(void* p, int32_t ref, int32_t val);

// Atomically store val at [p] if old [p] is ref. Return 0 if OK, 1 is not. p needs to be aligned
extern int rv64_lock_cas_dd(void* p, int64_t ref, int64_t val);

// Atomically exchange value at [p] with val, return old p
extern uintptr_t rv64_lock_xchg_dd(void* p, uintptr_t val);

// Atomically exchange value at [p] with val, return old p
extern uint32_t rv64_lock_xchg_d(void* p, uint32_t val);

// Atomically store value to [p] only if [p] is NULL. Return old [p] value
extern uint32_t rv64_lock_storeifnull_d(void*p, uint32_t val);

// Atomically store value to [p] only if [p] is NULL. Return old [p] value
extern void* rv64_lock_storeifnull(void*p, void* val);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern void* rv64_lock_storeifref(void*p, void* val, void* ref);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern uint32_t rv64_lock_storeifref_d(void*p, uint32_t val, uint32_t ref);

// Atomically store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern uint32_t rv64_lock_storeifref2_d(void*p, uint32_t val, uint32_t ref);

// decrement atomically the byte at [p] (but only if p not 0)
extern void rv64_lock_decifnot0b(void*p);

// atomic store (with memory barrier)
extern void rv64_lock_storeb(void*p, uint8_t b);

// increment atomically the int at [p] only if it was 0. Return the old value of [p]
extern int rv64_lock_incif0(void*p);

// decrement atomically the int at [p] (but only if p not 0)
extern int rv64_lock_decifnot0(void*p);

// atomic store (with memory barrier)
extern void rv64_lock_store(void*p, uint32_t v);

// atomic store (with memory barrier)
extern void rv64_lock_store_dd(void*p, uint64_t v);

// (mostly) Atomically store val1 and val2 at [p] if old [p] is ref. Return 0 if OK, 1 is not. p needs to be aligned
extern int rv64_lock_cas_dq(void* p, uint64_t ref, uint64_t val1, uint64_t val2);

// atomic get (with memory barrier)
extern uint32_t rv64_lock_get_b(void* p);

// atomic get (with memory barrier)
extern uint32_t rv64_lock_get_d(void* p);

// atomic get (with memory barrier)
extern void* rv64_lock_get_dd(void* p);

// Not defined in assembler but in dynarec_rv64_functions
uint8_t rv64_lock_xchg_b(void* addr, uint8_t v);
extern int rv64_lock_cas_b(void* p, uint8_t ref, uint8_t val);
extern int rv64_lock_cas_h(void* p, uint16_t ref, uint16_t val);

#endif  //__RV64_LOCK__H__
