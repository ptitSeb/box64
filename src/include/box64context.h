#ifndef __BOX64CONTEXT_H_
#define __BOX64CONTEXT_H_
#include <stdint.h>

#include "mypthread.h"
#include "pathcoll.h"
#include "dictionnary.h"
#ifdef DYNAREC
#include "dynarec/native_lock.h"
#endif
#ifndef BOX32_DEF
#define BOX32_DEF
typedef uint32_t ptr_t;
typedef int32_t long_t;
typedef uint32_t ulong_t;
#endif

#ifdef DYNAREC
// disabling for now, seems to have a negative impact on performances
//#define USE_CUSTOM_MUTEX
#endif


typedef struct elfheader_s elfheader_t;
typedef struct cleanup_s cleanup_t;
typedef struct x64emu_s x64emu_t;
typedef struct zydis_s zydis_t;
typedef struct zydis_dec_s zydis_dec_t;
typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;
typedef struct dlprivate_s dlprivate_t;
typedef struct kh_symbolmap_s kh_symbolmap_t;
typedef struct kh_defaultversion_s kh_defaultversion_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct library_s library_t;
typedef struct linkmap_s linkmap_t;
typedef struct linkmap32_s linkmap32_t;
typedef struct kh_threadstack_s kh_threadstack_t;
typedef struct rbtree rbtree_t;
typedef struct atfork_fnc_s {
    uintptr_t prepare;
    uintptr_t parent;
    uintptr_t child;
    void*     handle;
} atfork_fnc_t;
#ifdef DYNAREC
typedef struct dynablock_s      dynablock_t;
typedef struct mmaplist_s       mmaplist_t;
typedef struct kh_dynablocks_s  kh_dynablocks_t;
#endif
#define DYNAMAP_SHIFT 16

typedef void* (*procaddress_t)(const char* name);
typedef void* (*vkprocaddess_t)(void* instance, const char* name);

#ifdef LA64_ABI_1
#define MAX_SIGNAL 128
#else
#define MAX_SIGNAL 64
#endif

typedef struct tlsdatasize_s {
    int         tlssize;
    int         n_elfs;
    void*       data;
    void*       ptr;
} tlsdatasize_t;

void free_tlsdatasize(void* p);

typedef struct needed_libs_s {
    int         cap;
    int         size;
    int         init_size;
    char**      names;
    library_t** libs;
    int         nb_done;
} needed_libs_t;

void free_neededlib(needed_libs_t* needed);
needed_libs_t* new_neededlib(int n);
needed_libs_t* copy_neededlib(needed_libs_t* needed);
void add1_neededlib(needed_libs_t* needed);
void add1lib_neededlib(needed_libs_t* needed, library_t* lib, const char* name);
void add1lib_neededlib_name(needed_libs_t* needed, library_t* lib, const char* name);
void add1libref_neededlib(needed_libs_t* needed, library_t* lib);

typedef struct base_segment_s {
    uintptr_t       base;
    uint64_t        limit;
    uint8_t         present;
    uint8_t         is32bits;
} base_segment_t;

typedef struct box64context_s {
    uint32_t            tick;           // for dynarec age
    path_collection_t   box64_path;     // PATH env. variable
    path_collection_t   box64_ld_lib;   // LD_LIBRARY_PATH env. variable

    path_collection_t   box64_emulated_libs;    // Collection of libs that should not be wrapped

    int                 x64trace;
    int                 trace_tid;

    zydis_t             *zydis;         // dlopen the zydis dissasembler
    void*               box64lib;       // dlopen on box64 itself

    int                 argc;
    char**              argv;
    ptr_t               argv32;

    int                 envc;
    char**              envv;
    ptr_t               envv32;

    int                 orig_argc;
    char**              orig_argv;

    char*               fullpath;
    char*               box64path;      // path of current box64 executable
    char*               box86path;      // path of box86 executable (if present)
    char*               bashpath;       // path of x86_64 bash (defined with BOX64_BASH or by running bash directly)
    char*               pythonpath;     // path of x86_64 python3 (defined with BOX64_PYTHON3)

    uint64_t            stacksz;
    size_t              stackalign;
    void*               stack;          // alocated stack

    elfheader_t         **elfs;         // elf headers and memory
    int                 elfcap;
    int                 elfsize;        // number of elf loaded


    needed_libs_t       *neededlibs;    // needed libs for main elf
    needed_libs_t       *preload;

    uintptr_t           ep;             // entry point

    lib_t               *maplib;        // lib and symbols handling
    lib_t               *local_maplib;  // libs and symbols opened has local (only collection of libs, no symbols)
    dic_t               *versym;        // dictionnary of versioned symbols
    kh_mapsymbols_t     *globdata;      // GLOBAL_DAT relocation for COPY mapping in main elf
    kh_mapsymbols_t     *uniques;       // symbols with STB_GNU_UNIQUE bindings

    kh_threadstack_t    *stacksizes;    // stack sizes attributes for thread (temporary)
    bridge_t            *system;        // other bridges
    uintptr_t           exit_bridge;    // exit bridge value
    uintptr_t           vsyscall;       // vsyscall bridge value
    uintptr_t           vsyscalls[3];   // the 3 x86 VSyscall pseudo bridges (mapped at 0xffffffffff600000+)
    dlprivate_t         *dlprivate;     // dlopen library map
    kh_symbolmap_t      *alwrappers;    // the map of wrapper for alGetProcAddress
    kh_symbolmap_t      *almymap;       // link to the mysymbolmap if libOpenAL
    kh_symbolmap_t      *vkwrappers;    // the map of wrapper for VulkanProcs (TODO: check SDL2)
    kh_symbolmap_t      *vkmymap;       // link to the mysymbolmap of libGL
    kh_symbolmap_t      *cudawrappers;  // the map of wrapper for cuda
    kh_symbolmap_t      *mycuda;        // link to the mysymbolmap of cuda
    vkprocaddess_t      vkprocaddress;

    #ifndef DYNAREC
    pthread_mutex_t     mutex_lock;     // dynarec build will use their own mecanism
    pthread_mutex_t     mutex_trace;
    pthread_mutex_t     mutex_tls;
    pthread_mutex_t     mutex_thread;
    pthread_mutex_t     mutex_bridge;
    #else
    #ifdef USE_CUSTOM_MUTEX
    uint32_t            mutex_dyndump;
    uint32_t            mutex_trace;
    uint32_t            mutex_tls;
    uint32_t            mutex_thread;
    uint32_t            mutex_bridge;
    #else
    pthread_mutex_t     mutex_dyndump;
    pthread_mutex_t     mutex_trace;
    pthread_mutex_t     mutex_tls;
    pthread_mutex_t     mutex_thread;
    pthread_mutex_t     mutex_bridge;
    #endif
    uintptr_t           max_db_size;    // the biggest (in x86_64 instructions bytes) built dynablock
    rbtree_t*             db_sizes;
    int                 trace_dynarec;
    pthread_mutex_t     mutex_lock;     // this is for the Test interpreter
    #if defined(__riscv) || defined(__loongarch64)
    uint32_t            mutex_16b;
    #endif
    #endif

    library_t           *libclib;       // shortcut to libc library (if loaded, so probably yes)
    library_t           *sdl1mixerlib;
    library_t           *sdl2lib;
    library_t           *sdl2mixerlib;
    library_t           *libx11;
    linkmap_t           *linkmap;
    linkmap32_t         *linkmap32;
    void*               sdl1allocrw;    // SDL1 AllocRW/FreeRW function
    void*               sdl1freerw;
    void*               sdl2allocrw;    // SDL2 AllocRW/FreeRW function
    void*               sdl2freerw;

    int                 deferredInit;
    elfheader_t         **deferredInitList;
    int                 deferredInitSz;
    int                 deferredInitCap;

    void*               tlsdata;    // the initial global tlsdata
    int64_t             tlssize;    // wanted size of tlsdata
    base_segment_t      seggdt[16];

    uintptr_t           *auxval_start;

    cleanup_t           *cleanups;          // atexit functions
    int                 clean_sz;
    int                 clean_cap;

    void*               video_mem;

    zydis_dec_t         *dec;           // trace
    zydis_dec_t         *dec32;         // trace

    int                 forked;         //  how many forks... cleanup only when < 0

    atfork_fnc_t        *atforks;       // fnc for atfork...
    int                 atfork_sz;
    int                 atfork_cap;

    uint8_t             canary[8];

    uintptr_t           signals[MAX_SIGNAL+1];  // signal should be 1..MAX_SIGNAL (no 0), but that's too much change...
    uintptr_t           restorer[MAX_SIGNAL+1];
    int                 onstack[MAX_SIGNAL+1];
    int                 is_sigaction[MAX_SIGNAL+1];
    x64emu_t            *emu_sig;       // the emu with stack used for signal handling (must be separated from main ones)
    int                 no_sigsegv;
    int                 no_sigill;
    void*               stack_clone;
    int                 stack_clone_used;

    // rolling logs
    char*               log_call;   // is a large string composed of slices of 256 chars
    char*               log_ret;    // is a large string composed of sloces of 128 chars
    int                 current_line;

} box64context_t;

#ifndef USE_CUSTOM_MUTEX
#define mutex_lock(A)    pthread_mutex_lock(A)
#define mutex_trylock(A) pthread_mutex_trylock(A)
#define mutex_unlock(A)  pthread_mutex_unlock(A)
#else
#define mutex_lock(A)                             \
    do {                                          \
        uint32_t tid = (uint32_t)GetTID();        \
        while (native_lock_storeifnull_d(A, tid)) \
            sched_yield();                        \
    } while (0)
#define mutex_trylock(A) native_lock_storeifnull_d(A, (uint32_t)GetTID())
#define mutex_unlock(A)  native_lock_storeifref_d(A, 0, (uint32_t)GetTID())
#endif

extern box64context_t *my_context; // global context

box64context_t *NewBox64Context(int argc);
void FreeBox64Context(box64context_t** context);

// Cycle log handling
void freeCycleLog(box64context_t* ctx);
void initCycleLog(box64context_t* context);
void print_rolling_log(int loglevel);

// return the index of the added header
int AddElfHeader(box64context_t* ctx, elfheader_t* head);
// remove an elf from list (but list is never reduced, so there can be holes)
void RemoveElfHeader(box64context_t* ctx, elfheader_t* head);

// return the tlsbase (negative) for the new TLS partition created (no partition index is stored in the context)
int AddTLSPartition(box64context_t* context, int tlssize);

// defined in fact in threads.c
void thread_set_emu(x64emu_t* emu);
void thread_forget_emu();
x64emu_t* thread_get_emu(void);

// relock the muxtex that were unlocked
void relockMutex(int locks);

#endif //__BOX64CONTEXT_H_
