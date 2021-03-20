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

// LDAXRD of ADDR
extern uint64_t arm64_lock_read_dd(void* addr);
// STLXR of ADDR, return 0 if ok, 1 if not
extern int arm64_lock_write_dd(void* addr, uint64_t val);

// Atomicaly exchange value at [p] with val, return old p
extern uintptr_t arm64_lock_xchg(void* p, uintptr_t val);

// Atomicaly store value to [p] only if [p] is NULL. Return new [p] value (so val or old)
extern void* arm64_lock_storeifnull(void*p, void* val);

#endif  //__ARM64_LOCK__H__