#define _GNU_SOURCE /* See feature_test_macros(7) */
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
#include "box64context.h"
#include "librarian.h"
#include "myalign.h"

const char* libsqlite3Name = "libsqlite3.so.0";
#define LIBNAME libsqlite3


#define ADDED_FUNCTIONS()

#include "generated/wrappedlibsqlite3types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// sqlite3_exec ...
#define GO(A) \
static uintptr_t my_sqlite3_exec_fct_##A = 0; \
static int my_sqlite3_exec_##A(void* a, int b, char** c, char** d) { \
    return RunFunctionFmt(my_sqlite3_exec_fct_##A,  "pipp", a, b, c, d); \
}
SUPER()
#undef GO
static void *find_sqlite3_exec_fct(void *fct) {
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_sqlite3_exec_fct_##A == (uintptr_t)fct) return my_sqlite3_exec_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_sqlite3_exec_fct_##A == 0) {my_sqlite3_exec_fct_##A = (uintptr_t)fct; return my_sqlite3_exec_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sqlite3_exec callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_sqlite3_exec(x64emu_t* emu, void *db, char *sql, void* callback, void* data, char **errmsg)
{
    return my->sqlite3_exec(db, sql, find_sqlite3_exec_fct(callback), data, errmsg);
}

#include "wrappedlib_init.h"
