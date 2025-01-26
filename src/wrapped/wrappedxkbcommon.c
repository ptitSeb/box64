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

const char* xkbcommonName = "libxkbcommon.so.0";
#define ALTNAME "libxkbcommon.so"

#define LIBNAME xkbcommon

#define ADDED_FUNCTIONS() \

#include "generated/wrappedxkbcommontypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \

//xkb_keymap_key_iter_t
#define GO(A)                                                           \
static uintptr_t my_xkb_keymap_key_iter_t_fct_##A = 0;                  \
static void my_xkb_keymap_key_iter_t_##A(void* a, uint32_t b, void* c)  \
{                                                                       \
    RunFunctionFmt(my_xkb_keymap_key_iter_t_fct_##A, "pup", a, b, c);   \
}
SUPER()
#undef GO
static void* find_xkb_keymap_key_iter_t_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xkb_keymap_key_iter_t_fct_##A == (uintptr_t)fct) return my_xkb_keymap_key_iter_t_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xkb_keymap_key_iter_t_fct_##A == 0) {my_xkb_keymap_key_iter_t_fct_##A = (uintptr_t)fct; return my_xkb_keymap_key_iter_t_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for xkbcommon xkb_keymap_key_iter_t callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_xkb_keymap_key_for_each(x64emu_t* emu, void* keymap, void* f, void* data)
{
    my->xkb_keymap_key_for_each(keymap, find_xkb_keymap_key_iter_t_Fct(f), data);
}

#include "wrappedlib_init.h"
