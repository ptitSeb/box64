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

typedef struct my_sqlite3_io_methods_s {
  int iVersion;
  int (*xClose)(void*);
  int (*xRead)(void*, void*, int iAmt, int64_t iOfst);
  int (*xWrite)(void*, const void*, int iAmt, int64_t iOfst);
  int (*xTruncate)(void*, int64_t size);
  int (*xSync)(void*, int flags);
  int (*xFileSize)(void*, int64_t *pSize);
  int (*xLock)(void*, int);
  int (*xUnlock)(void*, int);
  int (*xCheckReservedLock)(void*, int *pResOut);
  int (*xFileControl)(void*, int op, void *pArg);
  int (*xSectorSize)(void*);
  int (*xDeviceCharacteristics)(void*);
  int (*xShmMap)(void*, int iPg, int pgsz, int, void volatile**);
  int (*xShmLock)(void*, int offset, int n, int flags);
  void (*xShmBarrier)(void*);
  int (*xShmUnmap)(void*, int deleteFlag);
  int (*xFetch)(void*, int64_t iOfst, int iAmt, void **pp);
  int (*xUnfetch)(void*, int64_t iOfst, void *p);
} my_sqlite3_io_methods_t;

typedef struct my_sqlite3_file_s {
  const my_sqlite3_io_methods_t *pMethods;
} my_sqlite3_file_t;

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
EXPORT int my_sqlite3_exec(x64emu_t* emu, void *db, char *sql, void* callback, void* data, char **errmsg)
{
    return my->sqlite3_exec(db, sql, find_sqlite3_exec_fct(callback), data, errmsg);
}

// sqlite3_destroy ...
#define GO(A) \
static uintptr_t my_sqlite3_destroy_fct_##A = 0;    \
static void my_sqlite3_destroy_##A(void* p) {      \
    RunFunctionFmt(my_sqlite3_destroy_fct_##A,  "p", p); \
}
SUPER()
#undef GO
static void *find_sqlite3_destroy_fct(void *fct) {
    if (!fct) return NULL;
    if(fct == (void*)-1) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_sqlite3_destroy_fct_##A == (uintptr_t)fct) return my_sqlite3_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_sqlite3_destroy_fct_##A == 0) {my_sqlite3_destroy_fct_##A = (uintptr_t)fct; return my_sqlite3_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sqlite3_destroy callback\n");
    return NULL;
}

EXPORT int my_sqlite3_bind_blob(x64emu_t* emu, void* stmt, int idx, void* data, int n, void* dtor) {
    return my->sqlite3_bind_blob(stmt, idx, data, n, find_sqlite3_destroy_fct(dtor));
}

EXPORT int my_sqlite3_bind_text(x64emu_t* emu, void* stmt, int idx, void* text, int n, void* dtor) {
    return my->sqlite3_bind_text(stmt, idx, text, n, find_sqlite3_destroy_fct(dtor));
}

// sqlite3_func ...
#define GO(A) \
static uintptr_t my_sqlite3_func_fct_##A = 0; \
static void my_sqlite3_func_##A(void* ctx, int argc, void* argv) { \
    RunFunctionFmt(my_sqlite3_func_fct_##A, "pip", ctx, argc, argv); \
}
SUPER()
#undef GO

static void* find_sqlite3_func_fct(void* fct) {
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_sqlite3_func_fct_##A == (uintptr_t)fct) return my_sqlite3_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_sqlite3_func_fct_##A == 0) { my_sqlite3_func_fct_##A = (uintptr_t)fct; return my_sqlite3_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sqlite3 func callback\n");
    return NULL;
}

// salite_final
#define GO(A) \
static uintptr_t my_sqlite3_final_fct_##A = 0; \
static void my_sqlite3_final_##A(void* ctx) { \
    RunFunctionFmt(my_sqlite3_final_fct_##A, "p", ctx); \
}
SUPER()
#undef GO

static void* find_sqlite3_final_fct(void* fct) {
    if (!fct) return NULL;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if (my_sqlite3_final_fct_##A == (uintptr_t)fct) return my_sqlite3_final_##A;
    SUPER()
    #undef GO
    #define GO(A) if (my_sqlite3_final_fct_##A == 0) { my_sqlite3_final_fct_##A = (uintptr_t)fct; return my_sqlite3_final_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for sqlite3 final callback\n");
    return NULL;
}

EXPORT int my_sqlite3_create_function(x64emu_t* emu,
    void* db, void* name, int nArg, int eTextRep, void* pApp,
    void* xFunc, void* xStep, void* xFinal)
{
    return my->sqlite3_create_function(
        db, name, nArg, eTextRep, pApp,
        find_sqlite3_func_fct(xFunc),
        find_sqlite3_func_fct(xStep),
        find_sqlite3_final_fct(xFinal)
    );
}

EXPORT void* my_sqlite3_vmprintf(x64emu_t *emu, void* fmt, x64_va_list_t b) {
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return my->sqlite3_vmprintf(fmt, VARARGS);
}
EXPORT void* my_sqlite3_mprintf(x64emu_t *emu, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->sqlite3_vmprintf(fmt, VARARGS);
}

EXPORT void* my_sqlite3_database_file_object(x64emu_t* emu, void* a)
{
    my_sqlite3_file_t* ret = my->sqlite3_database_file_object(a);
    // add autobridge on all the functions
    #define GO(A, W)    if(ret->pMethods->A) AddAutomaticBridge(my_lib->w.bridge, W, ret->pMethods->A, 0, #A)
    GO(xClose, iFp);
    GO(xRead, iFppiI);
    GO(xWrite, iFppiI);
    GO(xTruncate, iFpI);
    GO(xSync, iFpi);
    GO(xFileSize, iFpp);
    GO(xLock, iFpi);
    GO(xUnlock, iFpi);
    GO(xCheckReservedLock, iFpp);
    GO(xFileControl, iFpip);
    GO(xSectorSize, iFp);
    GO(xDeviceCharacteristics, iFp);
    GO(xShmMap, iFpiiip);
    GO(xShmLock, iFpiii);
    GO(xShmBarrier, iFp);
    GO(xShmUnmap, iFpi);
    GO(xFetch, iFpIip);
    GO(xUnfetch, iFpIp);
    #undef GO
    return ret;
}


#include "wrappedlib_init.h"
