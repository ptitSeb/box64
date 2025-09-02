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

const char* libdrmName = "libdrm.so.2";
#define ALTNAME "libdrm.so"
#define LIBNAME libdrm

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibdrmtypes.h"

#include "wrappercallback.h"


#define SUPER() \
    GO(0)       \
    GO(1)       \
    GO(2)       \
    GO(3)       \
    GO(4)       \
    GO(5)       \
    GO(6)       \
    GO(7)       \
    GO(8)       \
    GO(9)       \
    GO(10)      \
    GO(11)      \
    GO(12)      \
    GO(13)      \
    GO(14)      \
    GO(15)

// vblank_handler
#define GO(A)                                                                                                                    \
    static uintptr_t my_vblank_handler_fct_##A = 0;                                                                              \
    static void my_vblank_handler_##A(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void* user_data) \
    {                                                                                                                            \
        RunFunctionFmt(my_vblank_handler_fct_##A, "iuuup", fd, sequence, tv_sec, tv_usec, user_data);                            \
    }
SUPER()
#undef GO
static void* find_vblank_handlerFct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_vblank_handler_fct_##A == (uintptr_t)fct) return my_vblank_handler_##A;
    SUPER()
#undef GO
#define GO(A)                                       \
    if (my_vblank_handler_fct_##A == 0) {           \
        my_vblank_handler_fct_##A = (uintptr_t)fct; \
        return my_vblank_handler_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject vblank_handler callback\n");
    return NULL;
}

// page_flip_handler
#define GO(A)                                                                                                                       \
    static uintptr_t my_page_flip_handler_fct_##A = 0;                                                                              \
    static void my_page_flip_handler_##A(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void* user_data) \
    {                                                                                                                               \
        RunFunctionFmt(my_page_flip_handler_fct_##A, "iuuup", fd, sequence, tv_sec, tv_usec, user_data);                            \
    }
SUPER()
#undef GO
static void* find_page_flip_handlerFct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_page_flip_handler_fct_##A == (uintptr_t)fct) return my_page_flip_handler_##A;
    SUPER()
#undef GO
#define GO(A)                                          \
    if (my_page_flip_handler_fct_##A == 0) {           \
        my_page_flip_handler_fct_##A = (uintptr_t)fct; \
        return my_page_flip_handler_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject page_flip_handler callback\n");
    return NULL;
}

EXPORT void my_drmMsg(x64emu_t* emu, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    char* buf = NULL;
    int dummy = vasprintf(&buf, (const char*)fmt, VARARGS);
    (void)dummy;
    my->drmMsg(buf);
    free(buf);
}

typedef struct drmEventContext_s {
    int version;
    void (*vblank_handler)(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void* user_data);
    void (*page_flip_handler)(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void* user_data);
} drmEventContext_t, *drmEventContextPtr_t;


EXPORT void my_drmHandleEvent(x64emu_t* emu, int fd, drmEventContextPtr_t evctx)
{
    drmEventContext_t ctx = { 0 };
    ctx.version = evctx->version;
    ctx.vblank_handler = find_vblank_handlerFct(evctx->vblank_handler);
    ctx.page_flip_handler = find_page_flip_handlerFct(evctx->page_flip_handler);
    my->drmHandleEvent(fd, &ctx);
}


#include "wrappedlib_init.h"
