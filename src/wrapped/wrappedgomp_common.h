// SPDX-License-Identifier: MIT
#ifndef __WRAPPEDGOMP_COMMON_H_
#define __WRAPPEDGOMP_COMMON_H_

#include <stdint.h>
#include <string.h>

typedef struct gomp_parallel_context_s {
    uintptr_t   callback;
    void*       data;
} gomp_parallel_context_t;

static void gomp_parallel_callback(void* opaque)
{
    gomp_parallel_context_t* context = opaque;
    RunFunctionFmt(context->callback, "p", context->data);
}

// GOMP_parallel only returns once the whole region completed, so the context can live on the stack.
static void gomp_run_parallel(void* f, void* data, unsigned num_threads, uint32_t flags)
{
    void* native = GetNativeFnc((uintptr_t)f);
    if(native) {
        my->GOMP_parallel(native, data, num_threads, flags);
        return;
    }
    gomp_parallel_context_t context = {(uintptr_t)f, data};
    my->GOMP_parallel(gomp_parallel_callback, &context, num_threads, flags);
}

typedef struct gomp_task_context_s {
    uintptr_t   callback;
    void*       data;
    uintptr_t   copyfn;
    long        arg_size;
    long        arg_off;
} gomp_task_context_t;

typedef struct gomp_task_header_s {
    uintptr_t   callback;
    uintptr_t   arg_off;
} gomp_task_header_t;

static void gomp_task_copy(void* dst, void* src)
{
    gomp_task_context_t* context = src;
    gomp_task_header_t header = {context->callback, (uintptr_t)context->arg_off};
    memcpy(dst, &header, sizeof(header));
    void* args = (char*)dst + context->arg_off;
    if(context->copyfn)
        RunFunctionFmt(context->copyfn, "pp", args, context->data);
    else if(context->arg_size)
        memcpy(args, context->data, context->arg_size);
}

static void gomp_task_callback(void* opaque)
{
    gomp_task_header_t header;
    memcpy(&header, opaque, sizeof(header));
    RunFunctionFmt(header.callback, "p", (char*)opaque + header.arg_off);
}

// A task can start after this function has already returned, so we cannot keep our data on the stack.
// Instead we put it at the front of the task arguments.
// The native runtime always copies those arguments before running the task, then passes that copy to gomp_task_callback, so our data stays valid until the task finishes.
static void gomp_run_task(void* fn, void* data, void* cpyfn, long arg_size, long arg_align, int if_clause, unsigned flags, void** depend, int priority)
{
    long align = arg_align > (long)sizeof(void*) ? arg_align : (long)sizeof(void*);
    long arg_off = (long)(((sizeof(gomp_task_header_t) + (size_t)align - 1) / (size_t)align) * (size_t)align);
    gomp_task_context_t context = {(uintptr_t)fn, data, (uintptr_t)cpyfn, arg_size, arg_off};
    my->GOMP_task(gomp_task_callback, &context, gomp_task_copy, arg_size + arg_off, align, if_clause, flags, depend, priority);
}

#endif //__WRAPPEDGOMP_COMMON_H_
