#ifndef __BOX32_64__H_
#define __BOX32_64__H_

#include <stdint.h>
#include <stdlib.h>

#ifndef BOX32_DEF
#define BOX32_DEF
typedef uint32_t ptr_t;
typedef int32_t long_t;
typedef uint32_t ulong_t;
#endif

#define TEST32
#define TEST_ABORT

static inline uintptr_t from_ptr(ptr_t p) {
    return (uintptr_t)p;
}
static inline void* from_ptrv(ptr_t p) {
    return (void*)(uintptr_t)p;
}
static inline long long from_long(long_t l) {
    return (long long)l;
}
static inline unsigned long long from_ulong(ulong_t l) {
    return (unsigned long long)l;
}
uintptr_t from_hash(ulong_t l);
uintptr_t from_hash_d(ulong_t l);
#ifdef TEST32
#include "debug.h"
void box64_abort();

static inline ptr_t to_ptr(uintptr_t p) {
    if(p!=0xffffffffffffffffLL && (p>>32)) {
        printf_log(LOG_NONE, "Warning, uintptr_t %p is not a 32bits value\n", (void*)p);
        #ifdef TEST_ABORT
        box64_abort();
        #endif
    }
    return (ptr_t)p;
}
static inline ptr_t to_ptrv(void* p2) {
    uintptr_t p = (uintptr_t)p2;
    if(p!=0xffffffffffffffffLL && (p>>32)) {
        printf_log(LOG_NONE, "Warning, pointer %p is not a 32bits value\n", p2);
        #ifdef TEST_ABORT
        box64_abort();
        #endif
    }
    return (ptr_t)p;
}
static inline long_t to_long(long long l) {
    long_t ret = (long_t)l;
    if(l!=ret)
        printf_log(LOG_NONE, "Warning, long %ld is not a 32bits value\n", l);
    return ret;
}
static inline ulong_t to_ulong(unsigned long long l) {
    if(l!=0xffffffffffffffffLL && (l>>32))
        printf_log(LOG_NONE, "Warning, ulong %p is not a 32bits value\n", (void*)l);
    return (ulong_t)l;
}
#else //TEST32
static inline ptr_t to_ptr(uintptr_t p) {
    return (ptr_t)p;
}
static inline ptr_t to_ptrv(void* p) {
    return (ptr_t)(uintptr_t)p;
}
static inline long_t to_long(long long l) {
    return (long_t)l;
}
static inline ulong_t to_ulong(unsigned long long l) {
    return (ulong_t)l;
}
#endif //TEST32

static inline ptr_t to_ptr_silent(uintptr_t p) {
    return (ptr_t)p;
}
static inline ptr_t to_ptrv_silent(void* p) {
    return (ptr_t)(uintptr_t)p;
}
// indirect l -> T
#define from_ptri(T, l) *(T*)from_ptr(l)
// indirect l -> void*
static inline void* from_ptriv(ptr_t l) {
    return from_ptrv(from_ptri(ptr_t, l));
}
static inline ptr_t to_ulong_silent(unsigned long l) {
    return (ulong_t)l;
}

ulong_t to_hash(uintptr_t p);
ulong_t to_hash_d(uintptr_t p);
static inline ulong_t to_hashv(void* p) {return to_hash((uintptr_t)p);}
static inline ulong_t to_hashv_d(uintptr_t p) {return to_hash_d((uintptr_t)p);}

void* from_locale(ptr_t l);
void* from_locale_d(ptr_t l);
ptr_t to_locale(void* p);
ptr_t to_locale_d(void* p);

char* from_cstring(ptr_t p);
ptr_t to_cstring(char* p);
ptr_t to_cstring_d(char* p);

void init_hash_helper();
void fini_hash_helper();

typedef struct x86emu_s x86emu_t;

#endif //__BOX32_64__H_
