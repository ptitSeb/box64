#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/mman.h>

#ifdef DYNAREC
#include "dynarec/arm64_lock.h"
#endif
#include "box64context.h"
#include "debug.h"
#include "elfloader.h"
#include "custommem.h"
#include "threads.h"
#include "x64trace.h"
#include "bridge.h"
#include "librarian.h"
#include "library.h"
#include "wrapper.h"
#include "x64emu.h"
#include "signals.h"

EXPORTDYN
void initAllHelpers(box64context_t* context)
{
    static int inited = 0;
    if(inited)
        return;
    my_context = context;
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

void free_tlsdatasize(void* p)
{
    if(!p)
        return;
    tlsdatasize_t *data = (tlsdatasize_t*)p;
    free(data->tlsdata);
    free(p);
}

void x64Syscall(x64emu_t *emu);

int unlockMutex()
{
    int ret = unlockCustommemMutex();
    int i;
    #define GO(A, B)                    \
        i = checkUnlockMutex(&A);       \
        if(i) {                         \
            ret|=(1<<B);                \
        }

    GO(my_context->mutex_once, 5)
    GO(my_context->mutex_once2, 6)
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

    return ret;
}

void relockMutex(int locks)
{
    relockCustommemMutex(locks);
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            pthread_mutex_lock(&A);     \

    GO(my_context->mutex_once, 5)
    GO(my_context->mutex_once2, 6)
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

static void init_mutexes(box64context_t* context)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&context->mutex_once, &attr);
    pthread_mutex_init(&context->mutex_once2, &attr);
    pthread_mutex_init(&context->mutex_trace, &attr);
    #ifdef DYNAREC
    pthread_mutex_init(&context->mutex_dyndump, &attr);
    #else
    pthread_mutex_init(&context->mutex_lock, &attr);
    #endif
    pthread_mutex_init(&context->mutex_tls, &attr);
    pthread_mutex_init(&context->mutex_thread, &attr);
    pthread_mutex_init(&context->mutex_bridge, &attr);

    pthread_mutexattr_destroy(&attr);
}

void startMailbox(void);
void haltMailbox(void);
static void atfork_prepare_box64context(void)
{
    // semaphores will go in undetermined state
    haltMailbox();
}
static void atfork_parent_box64context(void)
{
    // reinit mailbox only
    init_mutexes(my_context);
    startMailbox();
}
static void atfork_child_box64context(void)
{
    // reinit mutexes and mailbox
    init_mutexes(my_context);
    startMailbox();
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
    box64context_t *context = my_context = (box64context_t*)calloc(1, sizeof(box64context_t));

    context->deferedInit = 1;
    context->sel_serial = 1;

    init_custommem_helper(context);

    context->maplib = NewLibrarian(context, 1);
    context->local_maplib = NewLibrarian(context, 1);
    context->versym = NewDictionnary();
    context->system = NewBridge();
    // create vsyscall
    context->vsyscall = AddBridge(context->system, vFEv, x64Syscall, 0, NULL);
    // create the vsyscalls
    context->vsyscalls[0] = AddVSyscall(context->system, 96);
    context->vsyscalls[1] = AddVSyscall(context->system, 201);
    context->vsyscalls[2] = AddVSyscall(context->system, 309);
    // create the alternate to map at address
    addAlternate((void*)0xffffffffff600000, (void*)context->vsyscalls[0]);
    addAlternate((void*)0xffffffffff600400, (void*)context->vsyscalls[1]);
    addAlternate((void*)0xffffffffff600800, (void*)context->vsyscalls[2]);
    // get handle to box64 itself
    context->box64lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
    context->dlprivate = NewDLPrivate();

    context->argc = argc;
    context->argv = (char**)calloc(context->argc+1, sizeof(char*));

    init_mutexes(context);
    pthread_atfork(atfork_prepare_box64context, atfork_parent_box64context, atfork_child_box64context);

    pthread_key_create(&context->tlskey, free_tlsdatasize);


    for (int i=0; i<8; ++i) context->canary[i] = 1 +  getrand(255);
    context->canary[getrand(4)] = 0;
    printf_log(LOG_DEBUG, "Setting up canary (for Stack protector) at FS:0x28, value:%08X\n", *(uint32_t*)context->canary);

    initAllHelpers(context);

    return context;
}

EXPORTDYN
void FreeBox64Context(box64context_t** context)
{
    if(!context)
        return;
    
    if(--(*context)->forked >= 0)
        return;

    box64context_t* ctx = *context;   // local copy to do the cleanning

    if(ctx->local_maplib)
        FreeLibrarian(&ctx->local_maplib, NULL);
    if(ctx->maplib)
        FreeLibrarian(&ctx->maplib, NULL);
    FreeDictionnary(&ctx->versym);

    for(int i=0; i<ctx->elfsize; ++i) {
        FreeElfHeader(&ctx->elfs[i]);
    }
    free(ctx->elfs);

    FreeCollection(&ctx->box64_path);
    FreeCollection(&ctx->box64_ld_lib);
    FreeCollection(&ctx->box64_emulated_libs);
    // stop trace now
    if(ctx->dec)
        DeleteX64TraceDecoder(&ctx->dec);
    if(ctx->zydis)
        DeleteX64Trace(ctx);

    if(ctx->deferedInitList)
        free(ctx->deferedInitList);

    free(ctx->argv);
    
    for (int i=0; i<ctx->envc; ++i)
        free(ctx->envv[i]);
    free(ctx->envv);

    if(ctx->atfork_sz) {
        free(ctx->atforks);
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

    free(ctx->fullpath);
    free(ctx->box64path);

    FreeBridge(&ctx->system);

    freeGLProcWrapper(ctx);
    freeALProcWrapper(ctx);


    void* ptr;
    if ((ptr = pthread_getspecific(ctx->tlskey)) != NULL) {
        free_tlsdatasize(ptr);
        pthread_setspecific(ctx->tlskey, NULL);
    }
    pthread_key_delete(ctx->tlskey);

    if(ctx->tlsdata)
        free(ctx->tlsdata);

    pthread_mutex_destroy(&ctx->mutex_once);
    pthread_mutex_destroy(&ctx->mutex_once2);
    pthread_mutex_destroy(&ctx->mutex_trace);
    #ifdef DYNAREC
    pthread_mutex_destroy(&ctx->mutex_dyndump);
    #else
    pthread_mutex_destroy(&ctx->mutex_lock);
    #endif
    pthread_mutex_destroy(&ctx->mutex_tls);
    pthread_mutex_destroy(&ctx->mutex_thread);
    pthread_mutex_destroy(&ctx->mutex_bridge);

    free_neededlib(&ctx->neededlibs);

    if(ctx->emu_sig)
        FreeX64Emu(&ctx->emu_sig);

    finiAllHelpers(ctx);

    free(ctx);
}

int AddElfHeader(box64context_t* ctx, elfheader_t* head) {
    int idx = ctx->elfsize;
    if(idx==ctx->elfcap) {
        // resize...
        ctx->elfcap += 16;
        ctx->elfs = (elfheader_t**)realloc(ctx->elfs, sizeof(elfheader_t*) * ctx->elfcap);
    }
    ctx->elfs[idx] = head;
    ctx->elfsize++;
    printf_log(LOG_DEBUG, "Adding \"%s\" as #%d in elf collection\n", ElfName(head), idx);
    return idx;
}

int AddTLSPartition(box64context_t* context, int tlssize) {
    int oldsize = context->tlssize;
    context->tlssize += tlssize;
    context->tlsdata = realloc(context->tlsdata, context->tlssize);
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

void add_neededlib(needed_libs_t* needed, library_t* lib)
{
    if(!needed)
        return;
    for(int i=0; i<needed->size; ++i)
        if(needed->libs[i] == lib)
            return;
    if(needed->size == needed->cap) {
        needed->cap += 8;
        needed->libs = (library_t**)realloc(needed->libs, needed->cap*sizeof(library_t*));
    }
    needed->libs[needed->size++] = lib;
}

void free_neededlib(needed_libs_t* needed)
{
    if(!needed)
        return;
    needed->cap = 0;
    needed->size = 0;
    if(needed->libs)
        free(needed->libs);
    needed->libs = NULL;
}

void add_dependedlib(needed_libs_t* depended, library_t* lib)
{
    if(!depended)
        return;
    for(int i=0; i<depended->size; ++i)
        if(depended->libs[i] == lib)
            return;
    if(depended->size == depended->cap) {
        depended->cap += 8;
        depended->libs = (library_t**)realloc(depended->libs, depended->cap*sizeof(library_t*));
    }
    depended->libs[depended->size++] = lib;
}

void free_dependedlib(needed_libs_t* depended)
{
    if(!depended)
        return;
    depended->cap = 0;
    depended->size = 0;
    if(depended->libs)
        free(depended->libs);
    depended->libs = NULL;
}
