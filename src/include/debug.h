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
#ifdef DYNAREC
extern int box64_dynarec_dump;
extern int box64_dynarec_trace;
extern int box64_dynarec_forced;
extern uintptr_t box64_nodynarec_start, box64_nodynarec_end;
extern int box64_dynarec_bigblock;
extern int box64_dynarec_strongmem;
extern int box64_dynarec_fastnan;
extern int box64_dynarec_x87double;
extern int box64_dynarec_safeflags;
extern int box64_dynarec_callret;
extern int box64_dynarec_bleeding_edge;
extern int box64_dynarec_hotpage;
#ifdef ARM64
extern int arm64_asimd;
extern int arm64_aes;
extern int arm64_pmull;
extern int arm64_crc32;
extern int arm64_atomics;
#endif
#endif
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
extern int box64_dummy_crashhandler;
extern int box64_sse_flushto0;
extern int allow_missing_libs;
extern int box64_mapclean;
extern int box64_prefer_wrapped;
extern int box64_prefer_emulated;
extern int box64_steam;
extern int box64_wine;
extern int box64_musl;
extern int box64_nopulse;   // disabling the use of wrapped pulseaudio
extern int box64_nogtk; // disabling the use of wrapped gtk
extern int box64_novulkan;  // disabling the use of wrapped vulkan
extern int box64_showsegv;  // show sigv, even if a signal handler is present
extern int box64_showbt;    // show a backtrace if a signal is caught
extern int box64_isglibc234; // is the program linked with glibc 2.34+
extern int box64_x11threads;
extern int box64_x11glx;
extern char* box64_libGL;
extern uintptr_t fmod_smc_start, fmod_smc_end; // to handle libfmod (from Unreal) SMC (self modifying code)
extern uint32_t default_gs;
extern int jit_gdb; // launch gdb when a segfault is trapped
extern int box64_tcmalloc_minimal;  // when using tcmalloc_minimal
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_NEVER 3
#define LOG_VERBOSE 3

extern FILE* ftrace;

#define printf_log(L, ...) do {if((L)<=box64_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define printf_dump(L, ...) do {if(box64_dump || ((L)<=box64_log)) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define printf_dlsym(L, ...) do {if(dlsym_error || ((L)<=box64_log)) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define dynarec_log(L, ...) do {if((L)<=box64_dynarec_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define EXPORT __attribute__((visibility("default")))
#ifdef BUILD_DYNAMIC
#define EXPORTDYN __attribute__((visibility("default")))
#else
#define EXPORTDYN 
#endif

void init_malloc_hook();
extern size_t(*box_malloc_usable_size)(void*);
#ifdef ANDROID
extern void*(*__libc_malloc)(size_t);
extern void*(*__libc_realloc)(void*, size_t);
extern void*(*__libc_calloc)(size_t, size_t);
extern void (*__libc_free)(void*);
extern void*(*__libc_memalign)(size_t, size_t);
#else
extern void* __libc_malloc(size_t);
extern void* __libc_realloc(void*, size_t);
extern void* __libc_calloc(size_t, size_t);
extern void  __libc_free(void*);
extern void* __libc_memalign(size_t, size_t);
#endif
#define box_malloc      __libc_malloc
#define box_realloc     __libc_realloc
#define box_calloc      __libc_calloc
#define box_free        __libc_free
#define box_memalign    __libc_memalign 
extern char* box_strdup(const char* s);
extern char* box_realpath(const char* path, char* ret);

#endif //__DEBUG_H_
