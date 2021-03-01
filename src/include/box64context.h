#ifndef __BOX64CONTEXT_H_
#define __BOX64CONTEXT_H_
#include <stdint.h>
#include <pthread.h>
#include "pathcoll.h"

typedef struct elfheader_s elfheader_t;

typedef void* (*procaddess_t)(const char* name);
typedef void* (*vkprocaddess_t)(void* instance, const char* name);

#define MAX_SIGNAL 64

typedef struct tlsdatasize_s {
    int32_t     tlssize;
    void*       tlsdata;
} tlsdatasize_t;

void free_tlsdatasize(void* p);

typedef struct box64context_s {
    path_collection_t   box64_path;     // PATH env. variable
    path_collection_t   box64_ld_lib;   // LD_LIBRARY_PATH env. variable

    path_collection_t   box64_emulated_libs;    // Collection of libs that should not be wrapped

    int                 x86trace;
    int                 trace_tid;

    uint32_t            sel_serial;     // will be increment each time selectors changes

    //zydis_t             *zydis;         // dlopen the zydis dissasembler
    void*               box64lib;       // dlopen on box86 itself

    int                 argc;
    char**              argv;

    int                 envc;
    char**              envv;

    char*               fullpath;
    char*               box64path;      // path of current box86 executable

    uint32_t            stacksz;
    int                 stackalign;
    void*               stack;          // alocated stack

    elfheader_t         **elfs;         // elf headers and memory
    int                 elfcap;
    int                 elfsize;        // number of elf loaded

    int                 deferedInit;

    pthread_key_t       tlskey;     // then tls key to have actual tlsdata
    void*               tlsdata;    // the initial global tlsdata
    int32_t             tlssize;    // wanted size of tlsdata

    //zydis_dec_t         *dec;           // trace

    uint8_t             canary[4];

    uintptr_t           signals[MAX_SIGNAL];
    uintptr_t           restorer[MAX_SIGNAL];
    int                 onstack[MAX_SIGNAL];
    int                 is_sigaction[MAX_SIGNAL];

    int                 no_sigsegv;
    int                 no_sigill;

} box64context_t;

extern box64context_t *my_context; // global context

box64context_t *NewBox64Context(int argc);
void FreeBox64Context(box64context_t** context);

// return the index of the added header
int AddElfHeader(box64context_t* ctx, elfheader_t* head);

// return the tlsbase (negative) for the new TLS partition created (no partition index is stored in the context)
int AddTLSPartition(box64context_t* context, int tlssize);

// defined in fact in threads.c
//void thread_set_emu(x64emu_t* emu);
//x64emu_t* thread_get_emu();

#endif //__BOX64CONTEXT_H_