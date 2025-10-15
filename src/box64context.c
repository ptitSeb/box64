#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/mman.h>
#include <pthread.h>

#include "os.h"
#include "box64context.h"
#include "debug.h"
#include "elfloader.h"
#include "custommem.h"
#include "threads.h"
#include "x64trace.h"
#include "bridge.h"
#include "alternate.h"
#include "librarian.h"
#include "library.h"
#include "wrapper.h"
#include "x64emu.h"
#include "signals.h"
#include "gltools.h"
#include "rbtree.h"
#include "box64cpu.h"
#ifdef BOX32
#include "box32.h"
#endif

EXPORTDYN
void initAllHelpers(box64context_t* context)
{
    static int inited = 0;
    if(inited)
        return;
    my_context = context;
    #ifdef BOX32
    init_hash_helper();
    #endif
    init_pthread_helper();
    init_bridge_helper();
    init_signal_helper(context);
    inited = 1;
}

EXPORTDYN
void finiAllHelpers(box64context_t* context)
{
    static int finied = 0;
    if(finied)
        return;
    fini_pthread_helper(context);
    fini_signal_helper();
    fini_bridge_helper();
    #ifdef BOX32
    fini_hash_helper();
    #endif
    fini_custommem_helper(context);
    finied = 1;
}

/// maxval not inclusive
int getrand(int maxval)
{
    if(maxval<1024) {
        return ((random()&0x7fff)*maxval)/0x7fff;
    } 
        uint64_t r = random();
        r = (r*maxval) / RAND_MAX;
        return r;

}

void x64Syscall(x64emu_t *emu);
void x86Syscall(x64emu_t *emu);

void relockMutex(int locks)
{
    relockCustommemMutex(locks);
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            mutex_trylock(&A);          \

    GO(my_context->mutex_trace, 7)
    #ifdef DYNAREC
    GO(my_context->mutex_dyndump, 8)
    #else
    GO(my_context->mutex_lock, 8)
    #endif
    GO(my_context->mutex_tls, 9)
    GO(my_context->mutex_thread, 10)
    GO(my_context->mutex_bridge, 11)
    #undef GO
}

void init_mutexes(box64context_t* context)
{
#ifndef DYNAREC
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&context->mutex_lock, &attr);
    pthread_mutex_init(&context->mutex_trace, &attr);
    pthread_mutex_init(&context->mutex_tls, &attr);
    pthread_mutex_init(&context->mutex_thread, &attr);
    pthread_mutex_init(&context->mutex_bridge, &attr);

    pthread_mutexattr_destroy(&attr);
#else
    #ifdef USE_CUSTOM_MUTEX
    native_lock_store(&context->mutex_trace, 0);
    native_lock_store(&context->mutex_tls, 0);
    native_lock_store(&context->mutex_thread, 0);
    native_lock_store(&context->mutex_bridge, 0);
    native_lock_store(&context->mutex_dyndump, 0);
    #else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&context->mutex_trace, &attr);
    pthread_mutex_init(&context->mutex_tls, &attr);
    pthread_mutex_init(&context->mutex_thread, &attr);
    pthread_mutex_init(&context->mutex_bridge, &attr);
    pthread_mutex_init(&context->mutex_dyndump, &attr);
    pthread_mutexattr_destroy(&attr);
    #endif
    pthread_mutex_init(&context->mutex_lock, NULL);
#endif
}

static void atfork_child_box64context(void)
{
    // (re)init mutex if it was lock before the fork
    init_mutexes(my_context);
}

void freeCycleLog(box64context_t* ctx)
{
    if(BOX64ENV(rolling_log)) {
        box_free(ctx->log_call);
        box_free(ctx->log_ret);
        ctx->log_call = NULL;
        ctx->log_ret = NULL;
    }
}
void initCycleLog(box64context_t* context)
{
    if(context && BOX64ENV(rolling_log)) {
        context->log_call = (char*)box_calloc(BOX64ENV(rolling_log), 256*sizeof(char));
        context->log_ret = (char*)box_calloc(BOX64ENV(rolling_log), 128*sizeof(char));
    }
}

EXPORTDYN
box64context_t *NewBox64Context(int argc)
{
#ifdef BUILD_DYNAMIC
    if(my_context) {
        ++my_context->count;
        return my_context;
    }
#endif
    // init and put default values
    box64context_t *context = my_context = (box64context_t*)box_calloc(1, sizeof(box64context_t));

    initCycleLog(context);

    context->deferredInit = 1;
    context->sel_serial = 1;

    init_custommem_helper(context);

    context->maplib = NewLibrarian(context);
    context->local_maplib = NewLibrarian(context);
    context->versym = NewDictionnary();
    context->system = NewBridge();
    // Cannot use Bridge name as the map is not initialized yet
    // create vsyscall
    context->vsyscall = AddBridge(context->system, vFEv, box64_is32bits?x86Syscall:x64Syscall, 0, NULL);
    // create the vsyscalls
    if(box64_is32bits) {
        #ifdef BOX32
        addAlternate((void*)0xffffe400, from_ptrv(context->vsyscall));
        #endif
    } else {
        context->vsyscalls[0] = AddVSyscall(context->system, 96);
        context->vsyscalls[1] = AddVSyscall(context->system, 201);
        context->vsyscalls[2] = AddVSyscall(context->system, 309);
        // create the alternate to map at address
        addAlternate((void*)0xffffffffff600000, (void*)context->vsyscalls[0]);
        addAlternate((void*)0xffffffffff600400, (void*)context->vsyscalls[1]);
        addAlternate((void*)0xffffffffff600800, (void*)context->vsyscalls[2]);
    }
    // create exit bridge
    context->exit_bridge = AddBridge(context->system, NULL, NULL, 0, NULL);
    // get handle to box64 itself
    #ifndef STATICBUILD
    context->box64lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
    #endif
    context->dlprivate = NewDLPrivate();

    context->argc = argc;
    context->argv = (char**)box_calloc(context->argc+1, sizeof(char*));

    init_mutexes(context);
    pthread_atfork(NULL, NULL, atfork_child_box64context);

    for (int i=0; i<8; ++i) context->canary[i] = 1 +  getrand(255);
    context->canary[getrand(4)] = 0;
    printf_log(LOG_DEBUG, "Setting up canary (for Stack protector) at FS:0x28, value:%08X\n", *(uint32_t*)context->canary);

    // init segments
    for(int i=0; i<16; i++) {
        context->seggdt[i].limit = (uintptr_t)-1LL;
    }
    // 0x53 selector
    context->seggdt[10].present = 1;
    // 0x43 selector
    context->seggdt[8].present = 1;
    // 0x33 selector
    context->seggdt[6].present = 1;
    // 0x2b selector
    context->seggdt[5].present = 1;
    // 0x23 selector
    context->seggdt[4].present = 1;
    context->seggdt[4].is32bits = 1;

    context->globdata = NewMapSymbols();
    context->uniques = NewMapSymbols();

    initAllHelpers(context);
    
    #ifdef DYNAREC
    context->db_sizes = rbtree_init("db_sizes");
    #endif

    return context;
}

void freeALProcWrapper(box64context_t* context);
EXPORTDYN
void FreeBox64Context(box64context_t** context)
{
    if(!context)
        return;
    
    if(--(*context)->forked >= 0)
        return;

    box64context_t* ctx = *context;   // local copy to do the cleaning

    //clean_current_emuthread();    // cleaning main thread seems a bad idea
    if(ctx->local_maplib)
        FreeLibrarian(&ctx->local_maplib, NULL);
    if(ctx->maplib)
        FreeLibrarian(&ctx->maplib, NULL);
    FreeDictionnary(&ctx->versym);

    for(int i=0; i<ctx->elfsize; ++i) {
        FreeElfHeader(&ctx->elfs[i]);
    }
    box_free(ctx->elfs);

    FreeCollection(&ctx->box64_path);
    FreeCollection(&ctx->box64_ld_lib);
    FreeCollection(&ctx->box64_emulated_libs);
    // stop trace now
    if(ctx->dec)
        DeleteX64TraceDecoder(&ctx->dec);
    if(ctx->dec32)
        DeleteX86TraceDecoder(&ctx->dec32);
    if(ctx->zydis)
        DeleteX64Trace(ctx);

    if(ctx->deferredInitList)
        box_free(ctx->deferredInitList);

    /*box_free(ctx->argv);*/
    
    /*for (int i=0; i<ctx->envc; ++i)
        box_free(ctx->envv[i]);
    box_free(ctx->envv);*/

    if(ctx->atfork_sz) {
        box_free(ctx->atforks);
        ctx->atforks = NULL;
        ctx->atfork_sz = ctx->atfork_cap = 0;
    }

    for(int i=0; i<MAX_SIGNAL; ++i)
        if(ctx->signals[i]!=0 && ctx->signals[i]!=1) {
            signal(i, SIG_DFL);
        }

    *context = NULL;                // bye bye my_context

    CleanStackSize(ctx);

    FreeDLPrivate(&ctx->dlprivate);

    box_free(ctx->fullpath);
    box_free(ctx->box64path);
    box_free(ctx->bashpath);

    FreeBridge(&ctx->system);

    #ifndef STATICBUILD
    freeGLProcWrapper(ctx);
    freeALProcWrapper(ctx);
    #ifdef BOX32
    #endif
    #endif

    if(ctx->stack_clone)
        box_free(ctx->stack_clone);


    if(ctx->tlsdata)
        box_free(ctx->tlsdata);

    free_neededlib(ctx->neededlibs);
    ctx->neededlibs = NULL;

    if(ctx->emu_sig)
        FreeX64Emu(&ctx->emu_sig);

    FreeMapSymbols(&ctx->globdata);
    FreeMapSymbols(&ctx->uniques);

#ifdef DYNAREC
    //dynarec_log(LOG_INFO, "BOX64 Dynarec at exit: Max DB=%d, rightmost=%d\n", ctx->max_db_size, rb_get_rightmost(ctx->db_sizes));
    rbtree_delete(ctx->db_sizes);
#endif

    finiAllHelpers(ctx);

#ifdef DYNAREC
    pthread_mutex_destroy(&ctx->mutex_lock);
#else
    pthread_mutex_destroy(&ctx->mutex_trace);
    pthread_mutex_destroy(&ctx->mutex_lock);
    pthread_mutex_destroy(&ctx->mutex_tls);
    pthread_mutex_destroy(&ctx->mutex_thread);
    pthread_mutex_destroy(&ctx->mutex_bridge);
#endif

    freeCycleLog(ctx);

    box_free(ctx);
}

int AddElfHeader(box64context_t* ctx, elfheader_t* head) {
    int idx = 0;
    while(idx<ctx->elfsize && ctx->elfs[idx]) idx++;
    if(idx == ctx->elfsize) {
        if(idx==ctx->elfcap) {
            // resize...
            ctx->elfcap += 16;
            ctx->elfs = (elfheader_t**)box_realloc(ctx->elfs, sizeof(elfheader_t*) * ctx->elfcap);
        }
        ctx->elfs[idx] = head;
        ctx->elfsize++;
    } else {
        ctx->elfs[idx] = head;
    }
    printf_log(LOG_DEBUG, "Adding \"%s\" as #%d in elf collection\n", ElfName(head), idx);
    return idx;
}

void RemoveElfHeader(box64context_t* ctx, elfheader_t* head) {
    if(GetTLSBase(head)) {
        // should remove the tls info
        int tlsbase = GetTLSBase(head);
        /*if(tlsbase == -ctx->tlssize) {
            // not really correct, but will do for now
            ctx->tlssize -= GetTLSSize(head);
            if(!(++ctx->sel_serial))
                ++ctx->sel_serial;
        }*/
    }
    for(int i=0; i<ctx->elfsize; ++i)
        if(ctx->elfs[i] == head) {
            ctx->elfs[i] = NULL;
            return;
        }
}

int AddTLSPartition(box64context_t* context, int tlssize) {
    int oldsize = context->tlssize;
    // should in fact first try to map a hole, but rewinding all elfs and checking filled space, like with the mapmem utilities
    context->tlssize += tlssize;
    context->tlsdata = box_realloc(context->tlsdata, context->tlssize);
    memmove(context->tlsdata+tlssize, context->tlsdata, oldsize);   // move to the top, using memmove as regions will probably overlap
    memset(context->tlsdata, 0, tlssize);           // fill new space with 0 (not mandatory)
    // clean GS segment for current emu
    if(my_context) {
        //ResetSegmentsCache(thread_get_emu());
        if(!(++context->sel_serial))
            ++context->sel_serial;
    }

    return -context->tlssize;   // negative offset
}

