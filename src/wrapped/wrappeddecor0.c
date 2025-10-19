#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char* decor0Name = "libdecor-0.so.0";
#define LIBNAME decor0

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappeddecor0types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \

// libdecor_frame_interface ...
typedef struct my_libdecor_frame_interface_s {
    uintptr_t   configure; //vFppp
    uintptr_t   close;  //vFpp
    uintptr_t   commit;  //vFpp
    uintptr_t   dismiss_popup;  //vFppp
    uintptr_t   reserved0;
    uintptr_t   reserved1;
    uintptr_t   reserved2;
    uintptr_t   reserved3;
    uintptr_t   reserved4;
    uintptr_t   reserved5;
    uintptr_t   reserved6;
    uintptr_t   reserved7;
    uintptr_t   reserved8;
    uintptr_t   reserved9;
} my_libdecor_frame_interface_t;
#define GO(A)   \
static my_libdecor_frame_interface_t* ref_libdecor_frame_interface_##A = NULL;          \
static void my_libdecor_frame_interface_configure_##A(void* a, void* b, void* c)        \
{                                                                                       \
    RunFunctionFmt(ref_libdecor_frame_interface_##A->configure, "ppp", a, b, c);        \
}                                                                                       \
static void my_libdecor_frame_interface_close_##A(void* a, void* b)                     \
{                                                                                       \
    RunFunctionFmt(ref_libdecor_frame_interface_##A->close, "pp", a, b);                \
}                                                                                       \
static void my_libdecor_frame_interface_commit_##A(void* a, void* b)                    \
{                                                                                       \
    RunFunctionFmt(ref_libdecor_frame_interface_##A->commit, "pp", a, b);               \
}                                                                                       \
static void my_libdecor_frame_interface_dismiss_popup_##A(void* a, void* b, void* c)    \
{                                                                                       \
    RunFunctionFmt(ref_libdecor_frame_interface_##A->dismiss_popup, "ppp", a, b, c);    \
}                                                                                       \
static my_libdecor_frame_interface_t my_libdecor_frame_interface_fct_##A = {            \
    (uintptr_t)my_libdecor_frame_interface_configure_##A,                               \
    (uintptr_t)my_libdecor_frame_interface_close_##A,                                   \
    (uintptr_t)my_libdecor_frame_interface_commit_##A,                                  \
    (uintptr_t)my_libdecor_frame_interface_dismiss_popup_##A,                           \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                        \
};
SUPER()
#undef GO
static void* find_libdecor_frame_interface_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(ref_libdecor_frame_interface_##A == fct) return &my_libdecor_frame_interface_fct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_libdecor_frame_interface_##A == 0) {ref_libdecor_frame_interface_##A = fct; return &my_libdecor_frame_interface_fct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libdecor_frame_interface callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_libdecor_decorate(x64emu_t* emu, void* context, void* wl_surface, void* iface, void* data)
{
    iface = find_libdecor_frame_interface_Fct(iface);
    return my->libdecor_decorate(context, wl_surface, iface, data);
}

#include "wrappedlib_init.h"
