#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdint.h>

typedef struct box64context_s box64context_t;
extern int box64_log;    // log level
extern int box64_dump;   // dump elf or not
extern int box64_dynarec_log;
extern int box64_dynarec;
extern int box64_pagesize;
#ifdef DYNAREC
extern int box64_dynarec_dump;
extern int box64_dynarec_trace;
extern int box64_dynarec_forced;
extern uintptr_t box64_nodynarec_start, box64_nodynarec_end;
extern int box64_dynarec_bigblock;
extern int box64_dynarec_strongmem;
#ifdef ARM64
extern int arm64_asimd;
extern int arm64_aes;
extern int arm64_pmull;
extern int arm64_crc32;
extern int arm64_atomics;
#endif
#endif
extern int dlsym_error;    // log dlsym error
#ifdef HAVE_TRACE
extern int trace_xmm;      // include XMM reg in trace?
extern int trace_emm;      // include EMM reg in trace?
extern int trace_regsdiff; // colorize standard registers on changes
extern uintptr_t trace_start, trace_end;
extern char* trace_func;
#endif
extern int allow_missing_libs;
extern int box64_mapclean;
extern int box64_prefer_wrapped;
extern int box64_prefer_emulated;
extern int box64_steam;
extern int box64_wine;
extern int box64_nopulse;   // disabling the use of wrapped pulseaudio
extern int box64_nogtk; // disabling the use of wrapped gtk
extern int box64_novulkan;  // disabling the use of wrapped vulkan
extern int box64_showsegv;  // show sigv, even if a signal handler is present
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

#define printf_log(L, ...) do {if(L<=box64_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define printf_dump(L, ...) do {if(box64_dump || (L<=box64_log)) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define dynarec_log(L, ...) do {if(L<=box64_dynarec_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}} while(0)

#define EXPORT __attribute__((visibility("default")))
#ifdef BUILD_DYNAMIC
#define EXPORTDYN __attribute__((visibility("default")))
#else
#define EXPORTDYN 
#endif

#endif //__DEBUG_H_
