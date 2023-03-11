#ifndef __RV64_LOCK__H__
#define __RV64_LOCK__H__
#include <stdint.h>

// Atomic read of ADDR
extern uint8_t rv64_lock_read_b(void* addr);
// Atomic store for ADDR, return 0 if ok, 1 if not
extern int rv64_lock_write_b(void* addr, uint8_t val);

// Atomic read of ADDR
extern uint16_t rv64_lock_read_h(void* addr);
// Atomic store for ADDR, return 0 if ok, 1 if not
extern int rv64_lock_write_h(void* addr, uint16_t val);

// Atoomic read of ADDR
extern uint32_t rv64_lock_read_d(void* addr);
// Atomic store for ADDR, return 0 if ok, 1 if not
extern int rv64_lock_write_d(void* addr, uint32_t val);

// Atomic read of ADDR
extern uint64_t rv64_lock_read_dd(void* addr);
// Atomic store for ADDR, return 0 if ok, 1 if not
extern int rv64_lock_write_dd(void* addr, uint64_t val);

// Atomic read of ADDR
extern void rv64_lock_read_dq(uint64_t * a, uint64_t* b, void* addr);
// Atomic store for ADDR, return 0 if ok, 1 if not
extern int rv64_lock_write_dq(uint64_t a, uint64_t b, void* addr);

// Atomicaly exchange value at [p] with val, return old p
extern uintptr_t rv64_lock_xchg(void* p, uintptr_t val);

// Atomicaly exchange value at [p] with val, return old p
extern uint32_t rv64_lock_xchg_d(void* p, uint32_t val);

// Atomicaly store value to [p] only if [p] is NULL. Return old [p] value
extern uint32_t rv64_lock_storeifnull_d(void*p, uint32_t val);

// Atomicaly store value to [p] only if [p] is NULL. Return old [p] value
extern void* rv64_lock_storeifnull(void*p, void* val);

// Atomicaly store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern void* rv64_lock_storeifref(void*p, void* val, void* ref);

// Atomicaly store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern uint32_t rv64_lock_storeifref_d(void*p, uint32_t val, uint32_t ref);

// Atomicaly store value to [p] only if [p] is ref. Return new [p] value (so val or old)
extern uint32_t rv64_lock_storeifref2_d(void*p, uint32_t val, uint32_t ref);

// decrement atomicaly the byte at [p] (but only if p not 0)
extern void rv64_lock_decifnot0b(void*p);

// atomic store (with memory barrier)
extern void rv64_lock_storeb(void*p, uint8_t b);

// increment atomicaly the int at [p] only if it was 0. Return the old value of [p]
extern int rv64_lock_incif0(void*p);

// decrement atomicaly the int at [p] (but only if p not 0)
extern int rv64_lock_decifnot0(void*p);

// atomic store (with memory barrier)
extern void rv64_lock_store(void*p, uint32_t v);

#endif  //__RV64_LOCK__H__