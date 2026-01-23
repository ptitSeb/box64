#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "library.h"
#include "box64context.h"
#include "myalign.h"
#include "globalsymbols.h"

const char* libtinfo6Name = "libtinfo.so.6";
#define LIBNAME libtinfo6

#include "generated/wrappedlibtinfo6types.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// putc
#define GO(A)   \
static uintptr_t my_putc_fct_##A = 0;                           \
static int my_putc_##A(char c)                                  \
{                                                               \
    return (int)RunFunctionFmt(my_putc_fct_##A, "c", c);  \
}
SUPER()
#undef GO
static void* find_putc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_putc_fct_##A == (uintptr_t)fct) return my_putc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_putc_fct_##A == 0) {my_putc_fct_##A = (uintptr_t)fct; return my_putc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL putc callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my6_tputs(x64emu_t* emu, void* str, int affcnt, void* f)
{
    return my->tputs(str, affcnt, find_putc_Fct(f));
}

EXPORT void* my6__nc_tiparm(x64emu_t* emu, int expected, void* fmt, void* b){
    CREATE_VALIST_FROM_VAARG(b, emu->scratch, 2);
    long args[9] = {0};
    int n = expected;
    if(n > 9) {
        printf_log(LOG_NONE, "Warning: my6__nc_tiparm with %d parameters not handled\n", n);
        return NULL;
    }
    for(int i = 0; i < n; ++i) args[i] = va_arg(VARARGS, long);
    switch(n) {
        case 0: return my->_nc_tiparm(expected, fmt);
        case 1: return my->_nc_tiparm(expected, fmt, args[0]);
        case 2: return my->_nc_tiparm(expected, fmt, args[0], args[1]);
        case 3: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2]);
        case 4: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2], args[3]);
        case 5: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2], args[3], args[4]);
        case 6: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2], args[3], args[4], args[5]);
        case 7: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
        case 8: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
        case 9: return my->_nc_tiparm(expected, fmt, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
    }
    return NULL;
}

// ncurses exports cur_term as a global; we forward it through globalsymbols.
extern void* cur_term;

// Refresh cur_term to the current native TERMINAL* and sync global mirrors.
static void update_shadow_from_native(void){
    void* native = NULL;
    if(!native && my && my->set_curterm) {
        // use set_curterm(NULL) to retrieve current native TERMINAL*
        void* old = my->set_curterm(NULL);
        if(old) {
            my->set_curterm(old);
            native = old;
            printf_log(LOG_DEBUG, "tinfo cur_term: fetched via set_curterm native=%p\n", native);
        }
    }
    if(!native) {
        cur_term = NULL;
        return;
    }
    cur_term = native;
    printf_log(LOG_DEBUG, "tinfo cur_term: native=%p\n", native);
    my_checkGlobalTInfo();
}

EXPORT int my6_setupterm(x64emu_t* emu, void* term, int fd, void* err)
{
    my_updateGlobalTInfo();
    printf_log(LOG_DEBUG, "my6_setupterm tinfo cur_term: after updateGlobal cur_term=%p\n", cur_term);
    int ret = my->setupterm(term, fd, err);
    // setupterm may replace cur_term, so refresh after the call.
    update_shadow_from_native();
    return ret;
}

EXPORT void* my6_set_curterm(x64emu_t* emu, void* term)
{
    my_updateGlobalTInfo();
    printf_log(LOG_DEBUG, "my6_set_curterm tinfo cur_term: after updateGlobal cur_term=%p\n", cur_term);
    // set_curterm returns the previous native TERMINAL*.
    void* old_native = my->set_curterm(term);
    update_shadow_from_native();
    return old_native;
}

EXPORT int my6_del_curterm(x64emu_t* emu, void* term)
{
    // del_curterm may clear cur_term; resync afterwards.
    int ret = my->del_curterm(term);
    update_shadow_from_native();
    return ret;
}

#define ALTMY my6_

#include "wrappedlib_init.h"
