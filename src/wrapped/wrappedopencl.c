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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* openclName = "libOpenCL.so.1";
#define LIBNAME opencl
#define ALTNAME "libOpenCL.so"

#include "generated/wrappedopencltypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// notify_program ...
#define GO(A)   \
static uintptr_t my_notify_program_fct_##A = 0;             \
static void my_notify_program_##A(void* a, void* b)         \
{                                                   \
    RunFunctionFmt(my_notify_program_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_notify_program_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_notify_program_fct_##A == (uintptr_t)fct) return my_notify_program_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_notify_program_fct_##A == 0) {my_notify_program_fct_##A = (uintptr_t)fct; return my_notify_program_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for opencl notify_program callback\n");
    return NULL;
}
// notity_context ...
#define GO(A)   \
static uintptr_t my_notity_context_fct_##A = 0;                         \
static void my_notity_context_##A(void* a, void* b, size_t c, void* d)  \
{                                                                       \
    RunFunctionFmt(my_notity_context_fct_##A, "ppLp", a, b, c, d);      \
}
SUPER()
#undef GO
static void* find_notity_context_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_notity_context_fct_##A == (uintptr_t)fct) return my_notity_context_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_notity_context_fct_##A == 0) {my_notity_context_fct_##A = (uintptr_t)fct; return my_notity_context_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for opencl notity_context callback\n");
    return NULL;
}
// notity_event ...
#define GO(A)   \
static uintptr_t my_notity_event_fct_##A = 0;                   \
static void my_notity_event_##A(void* a, int b, void* c)        \
{                                                               \
    RunFunctionFmt(my_notity_event_fct_##A, "pip", a, b, c);    \
}
SUPER()
#undef GO
static void* find_notity_event_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_notity_event_fct_##A == (uintptr_t)fct) return my_notity_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_notity_event_fct_##A == 0) {my_notity_event_fct_##A = (uintptr_t)fct; return my_notity_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for opencl notity_event callback\n");
    return NULL;
}
// user_function ...
#define GO(A)   \
static uintptr_t my_user_function_fct_##A = 0;          \
static void my_user_function_##A(void* a)               \
{                                                       \
    RunFunctionFmt(my_user_function_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_user_function_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_function_fct_##A == (uintptr_t)fct) return my_user_function_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_function_fct_##A == 0) {my_user_function_fct_##A = (uintptr_t)fct; return my_user_function_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for opencl user_function callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_clBuildProgram(x64emu_t* emu, void* program, uint32_t num, void* devices, void* options, void* f, void* data)
{
    return my->clBuildProgram(program, num, devices, options, find_notify_program_Fct(f), data);
}

EXPORT void* my_clCreateContext(x64emu_t* emu, void* prop, uint32_t num, void* devices, void* f, void* data, void* ret)
{
    return my->clCreateContext(prop, num, devices, find_notity_context_Fct(f), data, ret);
}

EXPORT void* my_clCreateContextFromType(x64emu_t* emu, void* prop, uint32_t type, void* f, void* data, void* ret)
{
    return my->clCreateContextFromType(prop, type, find_notity_context_Fct(f), data, ret);
}

EXPORT int my_clEnqueueNativeKernel(x64emu_t* emu, void* f, void* args, size_t nb_args, uint32_t nb_mem, void* mem, void* mem_loc, uint32_t nb_events, void* events, void* event)
{
    return my->clEnqueueNativeKernel(find_user_function_Fct(f), args, nb_args, nb_mem, mem, mem_loc, nb_events, events, event);
}

EXPORT int my_clSetMemObjectDestructorCallback(x64emu_t* emu, void* mem, void* f, void* data)
{
    return my->clSetMemObjectDestructorCallback(mem, find_notify_program_Fct(f), data);
}

EXPORT int my_clSetEventCallback(x64emu_t* emu, void* event, void*f, void* data)
{
    return my->clSetEventCallback(event, find_notity_event_Fct(f), data);
}

EXPORT int my_clCompileProgram(x64emu_t* emu, void* program, uint32_t nb_devices, void* devices, void* options, uint32_t nb_headers, void* headers, void* headers_inc, void* f, void* data)
{
    return my->clCompileProgram(program, nb_devices, devices, options, nb_headers, headers, headers_inc, find_notify_program_Fct(f), data);
}

EXPORT void* my_clLinkProgram(x64emu_t* emu, void* context, uint32_t nb_devices, void* devices, void* options, uint32_t nb_progs, void* progs, void* f, void* data, void* ret)
{
    return my->clLinkProgram(context, nb_devices, devices, options, nb_progs, progs, find_notify_program_Fct(f), data, ret);
}

#include "wrappedlib_init.h"
