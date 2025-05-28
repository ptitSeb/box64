#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdint.h>
#include <env.h>

#include "os.h"

typedef struct box64context_s box64context_t;
extern box64env_t box64env;
extern box64env_t* cur_box64env;

extern uintptr_t box64_pagesize;
extern int box64_rdtsc;
extern uint8_t box64_rdtsc_shift;
extern int box64_is32bits;
#ifdef DYNAREC
#ifdef ARM64
extern int arm64_asimd;
extern int arm64_aes;
extern int arm64_pmull;
extern int arm64_crc32;
extern int arm64_atomics;
extern int arm64_sha1;
extern int arm64_sha2;
extern int arm64_uscat;
extern int arm64_flagm;
extern int arm64_flagm2;
extern int arm64_frintts;
extern int arm64_rndr;
#elif defined(RV64)
extern int rv64_zba;
extern int rv64_zbb;
extern int rv64_zbc;
extern int rv64_zbs;
extern int rv64_vector;
extern int rv64_xtheadvector; // rvv 1.0 or xtheadvector
extern int rv64_vlen;
extern int rv64_xtheadba;
extern int rv64_xtheadbb;
extern int rv64_xtheadbs;
extern int rv64_xtheadcondmov;
extern int rv64_xtheadmemidx;
extern int rv64_xtheadmempair;
extern int rv64_xtheadfmemidx;
extern int rv64_xtheadmac;
extern int rv64_xtheadfmv;
#elif defined(LA64)
extern int la64_lbt;
extern int la64_lam_bh;
extern int la64_lamcas;
extern int la64_scq;
#endif
#endif
#ifdef HAVE_TRACE
extern uintptr_t trace_start, trace_end;
extern char* trace_func;
#endif
extern int box64_mapclean;
extern int box64_steam;
extern int box64_steamcmd;
extern int box64_wine;
extern int box64_musl;
extern int box64_isglibc234; // is the program linked with glibc 2.34+
extern uintptr_t fmod_smc_start, fmod_smc_end; // to handle libfmod (from Unreal) SMC (self modifying code)
extern uint32_t default_gs, default_fs;
extern int box64_tcmalloc_minimal;  // when using tcmalloc_minimal
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_NEVER 3
#define LOG_VERBOSE 3


#define printf_log_prefix(prefix, L, ...)                                                              \
    do {                                                                                               \
        if ((L) <= BOX64ENV(log)) { PrintfFtrace(prefix + (prefix && (L) == LOG_NONE), __VA_ARGS__); } \
    } while (0)

#define printf_log(L, ...) printf_log_prefix(1, L, __VA_ARGS__)

#define printf_dump_prefix(prefix, L, ...)                                                   \
    do {                                                                                     \
        if (BOX64ENV(dump) || ((L) <= BOX64ENV(log))) { PrintfFtrace(prefix, __VA_ARGS__); } \
    } while (0)

#define printf_dump(L, ...) printf_dump_prefix(1, L, __VA_ARGS__)

#define printf_dlsym_prefix(prefix, L, ...)                                                                           \
    do {                                                                                                              \
        if (BOX64ENV(dlsym_error) || BOX64ENV(dump) || ((L) <= BOX64ENV(log))) { PrintfFtrace(prefix, __VA_ARGS__); } \
    } while (0)

#define printf_dlsym_dump(L, ...) printf_dlsym_dump_prefix(1, L, __VA_ARGS__)

#define printf_dlsym_dump_prefix(prefix, L, ...)                                                    \
    do {                                                                                            \
        if (BOX64ENV(dlsym_error) || ((L) <= BOX64ENV(log))) { PrintfFtrace(prefix, __VA_ARGS__); } \
    } while (0)

#define printf_dlsym(L, ...) printf_dlsym_prefix(1, L, __VA_ARGS__)

#define dynarec_log_prefix(prefix, L, ...)                                       \
    do {                                                                         \
        if ((L) <= BOX64ENV(dynarec_log)) { PrintfFtrace(prefix, __VA_ARGS__); } \
    } while (0)

#define dynarec_log(L, ...) dynarec_log_prefix(1, L, __VA_ARGS__)


#define EXPORT __attribute__((visibility("default")))
#ifdef BUILD_DYNAMIC
#define EXPORTDYN __attribute__((visibility("default")))
#else
#define EXPORTDYN
#endif

#ifndef _WIN32 // TODO: better wow64 support?
#ifndef STATICBUILD
void init_malloc_hook(void);
#endif
#if defined(ANDROID) || defined(STATICBUILD)
#define box_malloc      malloc
#define box_realloc     realloc
#define box_calloc      calloc
#define box_free        free
#define box_memalign    memalign
#define box_strdup      strdup
#define box_realpath    realpath
#else
extern size_t(*box_malloc_usable_size)(void*);
extern void* __libc_malloc(size_t);
extern void* __libc_realloc(void*, size_t);
extern void* __libc_calloc(size_t, size_t);
extern void  __libc_free(void*);
extern void* __libc_memalign(size_t, size_t);
#define box_malloc      __libc_malloc
#define box_realloc     __libc_realloc
#define box_calloc      __libc_calloc
#define box_free        __libc_free
#define box_memalign    __libc_memalign
extern char* box_strdup(const char* s);
extern char* box_realpath(const char* path, char* ret);
#endif
#else
#define box_malloc  WinMalloc
#define box_realloc WinRealloc
#define box_calloc  WinCalloc
#define box_free    WinFree
#endif

//use actual_XXXX for internal memory that should be in 32bits space when box32 is active
//use box_XXX for internal memory that doesn't need anything special
#ifdef BOX32
int isCustomAddr(void* p);
void* box32_calloc(size_t n, size_t s);
void* box32_malloc(size_t s);
void* box32_realloc(void* p, size_t s);
void box32_free(void* p);
void* box32_memalign(size_t align, size_t s);
size_t box32_malloc_usable_size(void* p);
char* box32_strdup(const char* s);

#define actual_calloc(A, B)             (box64_is32bits?box32_calloc(A, B):box_calloc(A, B))
#define actual_malloc(A)                (box64_is32bits?box32_malloc(A):box_malloc(A))
#define actual_realloc(A, B)            (box64_is32bits?box32_realloc(A, B):box_realloc(A, B))
#define actual_free(A)                  (box64_is32bits?box32_free(A):box_free(A))
#define actual_memalign(A, B)           (box64_is32bits?box32_memalign(A, B):box_memalign(A, B))
#define actual_malloc_usable_size(A)    (box64_is32bits?box32_malloc_usable_size(A):box_malloc_usable_size(A))
#else
#define actual_calloc(A, B)             box_calloc(A, B)
#define actual_malloc(A)                box_malloc(A)
#define actual_realloc(A, B)            box_realloc(A, B)
#define actual_free(A)                  box_free(A)
#define actual_memalign(A, B)           box_memalign(A, B)
#define actual_malloc_usable_size(A)    box_malloc_usable_size(A)
#endif

#endif //__DEBUG_H_
