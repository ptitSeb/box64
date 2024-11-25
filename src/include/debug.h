#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdint.h>

typedef struct box64context_s box64context_t;
extern int box64_log;    // log level
extern int box64_dump;   // dump elf or not
extern int box64_dynarec_log;
extern int box64_dynarec;
extern uintptr_t box64_pagesize;
extern uintptr_t box64_load_addr;
extern int box64_dynarec_test;
extern int box64_maxcpu;
extern int box64_mmap32;
extern int box64_ignoreint3;
extern int box64_rdtsc;
extern int box64_rdtsc_1ghz;
extern uint8_t box64_rdtsc_shift;
extern int box64_is32bits;
extern int box64_x11sync;
#ifdef DYNAREC
extern int box64_dynarec_dump;
extern int box64_dynarec_trace;
extern int box64_dynarec_forced;
extern uintptr_t box64_nodynarec_start, box64_nodynarec_end;
extern uintptr_t box64_dynarec_test_start, box64_dynarec_test_end;
extern int box64_dynarec_bigblock;
extern int box64_dynarec_forward;
extern int box64_dynarec_strongmem;
extern int box64_dynarec_weakbarrier;
extern int box64_dynarec_pause;
extern int box64_dynarec_fastnan;
extern int box64_dynarec_fastround;
extern int box64_dynarec_x87double;
extern int box64_dynarec_div0;
extern int box64_dynarec_safeflags;
extern int box64_dynarec_callret;
extern int box64_dynarec_bleeding_edge;
extern int box64_dynarec_tbb;
extern int box64_dynarec_wait;
extern int box64_dynarec_missing;
extern int box64_dynarec_aligned_atomics;
extern int box64_dynarec_nativeflags;
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
extern int box64_libcef;
extern int box64_jvm;
extern int box64_unityplayer;
extern int box64_sdl2_jguid;
extern int dlsym_error;    // log dlsym error
extern int cycle_log;      // if using rolling logs
#ifdef HAVE_TRACE
extern int trace_xmm;      // include XMM reg in trace?
extern int trace_emm;      // include EMM reg in trace?
extern int trace_regsdiff; // colorize standard registers on changes
extern uintptr_t trace_start, trace_end;
extern char* trace_func;
extern char* trace_init;
extern char* box64_trace;
extern uint64_t start_cnt;
#endif
extern int box64_nosandbox;
extern int box64_inprocessgpu;
extern int box64_cefdisablegpu;
extern int box64_cefdisablegpucompositor;
extern int box64_maxcpu_immutable;
extern int box64_malloc_hack;
extern int box64_dummy_crashhandler;
extern int box64_sse_flushto0;
extern int box64_x87_no80bits;
extern int box64_sync_rounding;
extern int box64_shaext;
extern int box64_sse42;
extern int box64_avx;
extern int box64_avx2;
extern int allow_missing_libs;
extern int box64_mapclean;
extern int box64_prefer_wrapped;
extern int box64_prefer_emulated;
extern int box64_wrap_egl;
extern int box64_steam;
extern int box64_steamcmd;
extern int box64_wine;
extern int box64_musl;
extern int box64_nopulse;   // disabling the use of wrapped pulseaudio
extern int box64_nogtk; // disabling the use of wrapped gtk
extern int box64_novulkan;  // disabling the use of wrapped vulkan
extern int box64_showsegv;  // show sigv, even if a signal handler is present
extern int box64_showbt;    // show a backtrace if a signal is caught
extern int box64_isglibc234; // is the program linked with glibc 2.34+
extern int box64_futex_waitv;
extern int box64_x11threads;
extern int box64_x11glx;
extern char* box64_libGL;
extern uintptr_t fmod_smc_start, fmod_smc_end; // to handle libfmod (from Unreal) SMC (self modifying code)
extern uint32_t default_gs, default_fs;
extern int jit_gdb; // launch gdb when a segfault is trapped
extern int box64_tcmalloc_minimal;  // when using tcmalloc_minimal
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_NEVER 3
#define LOG_VERBOSE 3

void printf_ftrace(const char* fmt, ...);

#define printf_log(L, ...) do {if((L)<=box64_log) {printf_ftrace(__VA_ARGS__);}} while(0)

#define printf_dump(L, ...) do {if(box64_dump || ((L)<=box64_log)) {printf_ftrace(__VA_ARGS__);}} while(0)

#define printf_dlsym(L, ...) do {if(dlsym_error || ((L)<=box64_log)) {printf_ftrace(__VA_ARGS__);}} while(0)

#define dynarec_log(L, ...) do {if((L)<=box64_dynarec_log) {printf_ftrace(__VA_ARGS__);}} while(0)

#define EXPORT __attribute__((visibility("default")))
#ifdef BUILD_DYNAMIC
#define EXPORTDYN __attribute__((visibility("default")))
#else
#define EXPORTDYN
#endif

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
