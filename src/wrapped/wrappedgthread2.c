#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"
#include "gtkclass.h"

const char* gthread2Name = "libgthread-2.0.so.0";
#define LIBNAME gthread2

#define ADDED_FUNCTIONS() \

#include "wrappedgthread2types.h"

#include "wrappercallback.h"

EXPORT int g_threads_got_initialized;

typedef struct my_GThreadFunctions_s
{
  void*  (*mutex_new)               (void);
  void     (*mutex_lock)            (void *mutex);
  int (*mutex_trylock)              (void *mutex);
  void     (*mutex_unlock)          (void *mutex);
  void     (*mutex_free)            (void *mutex);
  void*   (*cond_new)               (void);
  void     (*cond_signal)           (void *cond);
  void     (*cond_broadcast)        (void *cond);
  void     (*cond_wait)             (void *cond, void *mutex);
  int (*cond_timed_wait)            (void *cond, void *mutex, void *end_time);
  void      (*cond_free)            (void *cond);
  void* (*private_new)              (void* destructor);            //GDestroyNotify
  void*  (*private_get)             (void *private_key);
  void      (*private_set)          (void *private_key, void* data);
  void      (*thread_create)        (void* func, void* data, unsigned long stack_size, int joinable, int bound, int priority, void* thread, void **error);  //GThreadFunc
  void      (*thread_yield)         (void);
  void      (*thread_join)          (void* thread);
  void      (*thread_exit)          (void);
  void      (*thread_set_priority)  (void* thread, int priority);
  void      (*thread_self)          (void* thread);
  int  (*thread_equal)              (void* thread1, void* thread2);
} my_GThreadFunctions_t;

EXPORT void my_g_thread_init(x64emu_t* emu, my_GThreadFunctions_t* vtable)
{
    if(g_threads_got_initialized) {
        // no need to do it twice
        my_setGlobalGThreadsInit();
        return;
    }

    if(!vtable)
        return my->g_thread_init(NULL);

    printf_log(LOG_INFO, "Warning, vtable not NULL in g_thread_init not supported yet!\n");
    
    my->g_thread_init(vtable);

    if(g_threads_got_initialized)
        my_setGlobalGThreadsInit();
}

EXPORT void my_g_thread_init_with_errorcheck_mutexes(x64emu_t* emu, my_GThreadFunctions_t* vtable)
{
    if(g_threads_got_initialized) {
        // no need to do it twice
        my_setGlobalGThreadsInit();
        return;
    }

    if(vtable)
        printf_log(LOG_NONE, "Warning, vtable is not NULL in g_thread_init_with_errorcheck_mutexes call!\n");
    
    my->g_thread_init_with_errorcheck_mutexes(vtable);  // will certainly crash here...

    if(g_threads_got_initialized)
        my_setGlobalGThreadsInit();
}

void** my_GetGthreadsGotInitialized()
{
    return (void**)&g_threads_got_initialized;
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#include "wrappedlib_init.h"
