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

const char* atkName = "libatk-1.0.so.0";
#define LIBNAME atk

typedef size_t      (*LFv_t)  ();

#define ADDED_FUNCTIONS()                   \
GO(atk_object_get_type, LFv_t)              \
GO(atk_util_get_type, LFv_t)                \

#include "generated/wrappedatktypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// AtkEventListenerInit ...
#define GO(A)   \
static uintptr_t my_AtkEventListenerInit_fct_##A = 0;               \
static void my_AtkEventListenerInit_##A()                           \
{                                                                   \
    RunFunctionFmt(my_AtkEventListenerInit_fct_##A, "");            \
}
SUPER()
#undef GO
static void* find_AtkEventListenerInit_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AtkEventListenerInit_fct_##A == (uintptr_t)fct) return my_AtkEventListenerInit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AtkEventListenerInit_fct_##A == 0) {my_AtkEventListenerInit_fct_##A = (uintptr_t)fct; return my_AtkEventListenerInit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for atk AtkEventListenerInit callback\n");
    return NULL;
}
// AtkEventListener ...
#define GO(A)   \
static uintptr_t my_AtkEventListener_fct_##A = 0;               \
static void my_AtkEventListener_##A(void* a)                    \
{                                                               \
    RunFunctionFmt(my_AtkEventListener_fct_##A, "p", a);            \
}
SUPER()
#undef GO
static void* find_AtkEventListener_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AtkEventListener_fct_##A == (uintptr_t)fct) return my_AtkEventListener_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AtkEventListener_fct_##A == 0) {my_AtkEventListener_fct_##A = (uintptr_t)fct; return my_AtkEventListener_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for atk AtkEventListener callback\n");
    return NULL;
}
// AtkKeySnoopFunc ...
#define GO(A)   \
static uintptr_t my_AtkKeySnoopFunc_fct_##A = 0;                                \
static int my_AtkKeySnoopFunc_##A(void* a, void* b)                             \
{                                                                               \
    return (int)RunFunctionFmt(my_AtkKeySnoopFunc_fct_##A, "pp", a, b);             \
}
SUPER()
#undef GO
static void* find_AtkKeySnoopFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AtkKeySnoopFunc_fct_##A == (uintptr_t)fct) return my_AtkKeySnoopFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AtkKeySnoopFunc_fct_##A == 0) {my_AtkKeySnoopFunc_fct_##A = (uintptr_t)fct; return my_AtkKeySnoopFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for atk AtkKeySnoopFunc callback\n");
    return NULL;
}
// GSignalEmissionHook ...
#define GO(A)   \
static uintptr_t my_GSignalEmissionHook_fct_##A = 0;                                    \
static int my_GSignalEmissionHook_##A(void* a, uint32_t b, void* c, void* d)            \
{                                                                                       \
    return (int)RunFunctionFmt(my_GSignalEmissionHook_fct_##A, "pupp", a, b, c, d);             \
}
SUPER()
#undef GO
static void* find_GSignalEmissionHook_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GSignalEmissionHook_fct_##A == (uintptr_t)fct) return my_GSignalEmissionHook_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GSignalEmissionHook_fct_##A == 0) {my_GSignalEmissionHook_fct_##A = (uintptr_t)fct; return my_GSignalEmissionHook_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for atk GSignalEmissionHook callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_atk_focus_tracker_init(x64emu_t* emu, void* f)
{
    my->atk_focus_tracker_init(find_AtkEventListenerInit_Fct(f));
}

EXPORT uint32_t my_atk_add_focus_tracker(x64emu_t* emu, void* f)
{
    return my->atk_add_focus_tracker(find_AtkEventListener_Fct(f));
}

EXPORT uint32_t my_atk_add_key_event_listener(x64emu_t* emu, void* f, void* p)
{
    return my->atk_add_key_event_listener(find_AtkEventListener_Fct(f), p);
}

EXPORT uint32_t my_atk_add_global_event_listener(x64emu_t* emu, void* f, void* p)
{
    return my->atk_add_global_event_listener(find_GSignalEmissionHook_Fct(f), p);
}

#define PRE_INIT \
    if (BOX64ENV(nogtk)) return -2;

#define CUSTOM_INIT \
    SetAtkObjectID(my->atk_object_get_type());  \
    SetAtkUtilID(my->atk_util_get_type());      \

#include "wrappedlib_init.h"
