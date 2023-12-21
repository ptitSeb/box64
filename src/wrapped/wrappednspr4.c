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

const char* nspr4Name = "libnspr4.so";
#define LIBNAME nspr4

#include "generated/wrappednspr4types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// PRCallOnceFN ...
#define GO(A)   \
static uintptr_t my_PRCallOnceFN_fct_##A = 0;           \
static int my_PRCallOnceFN_##A()                        \
{                                                       \
    return RunFunctionFmt(my_PRCallOnceFN_fct_##A, ""); \
}
SUPER()
#undef GO
static void* find_PRCallOnceFN_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PRCallOnceFN_fct_##A == (uintptr_t)fct) return my_PRCallOnceFN_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PRCallOnceFN_fct_##A == 0) {my_PRCallOnceFN_fct_##A = (uintptr_t)fct; return my_PRCallOnceFN_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nspr4 PRCallOnceFN callback\n");
    return NULL;
}
// PRCallOnceWithArgFN ...
#define GO(A)   \
static uintptr_t my_PRCallOnceWithArgFN_fct_##A = 0;                \
static int my_PRCallOnceWithArgFN_##A(void* a)                      \
{                                                                   \
    return RunFunctionFmt(my_PRCallOnceWithArgFN_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_PRCallOnceWithArgFN_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PRCallOnceWithArgFN_fct_##A == (uintptr_t)fct) return my_PRCallOnceWithArgFN_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PRCallOnceWithArgFN_fct_##A == 0) {my_PRCallOnceWithArgFN_fct_##A = (uintptr_t)fct; return my_PRCallOnceWithArgFN_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nspr4 PRCallOnceWithArgFN callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_PR_CallOnce(x64emu_t* emu, void* once, void* f)
{
    return my->PR_CallOnce(once, find_PRCallOnceFN_Fct(f));
}
EXPORT int my_PR_CallOnceWithArg(x64emu_t* emu, void* once, void* f, void* data)
{
    return my->PR_CallOnceWithArg(once, find_PRCallOnceWithArgFN_Fct(f), data);
}

#include "wrappedlib_init.h"

