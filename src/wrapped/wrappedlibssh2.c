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

const char* libssh2Name = "libssh2.so.1";
#define LIBNAME libssh2

#define NEEDED_LIBS "libgcrypt.so.20"

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibssh2types.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// alloc
#define GO(A)   \
static uintptr_t my_alloc_fct_##A = 0;                          \
static void* my_alloc_##A(size_t a, void* b)                    \
{                                                               \
    return (void*)RunFunctionFmt(my_alloc_fct_##A, "lp", a, b); \
}
SUPER()
#undef GO
static void* find_alloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_alloc_fct_##A == (uintptr_t)fct) return my_alloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_fct_##A == 0) {my_alloc_fct_##A = (uintptr_t)fct; return my_alloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libssh2 alloc callback\n");
    return NULL;
}
// realloc
#define GO(A)   \
static uintptr_t my_realloc_fct_##A = 0;                                \
static void* my_realloc_##A(void* a, size_t b, void* c)                 \
{                                                                       \
    return (void*)RunFunctionFmt(my_realloc_fct_##A, "plp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_realloc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_realloc_fct_##A == (uintptr_t)fct) return my_realloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_realloc_fct_##A == 0) {my_realloc_fct_##A = (uintptr_t)fct; return my_realloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libssh2 realloc callback\n");
    return NULL;
}
// free
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;               \
static void my_free_##A(void* a, void* b)           \
{                                                   \
    RunFunctionFmt(my_free_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_free_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libssh2 free callback\n");
    return NULL;
}
// paaswd_changereq
#define GO(A)   \
static uintptr_t my_paaswd_changereq_fct_##A = 0;                                               \
static void my_paaswd_changereq_##A(void* a, int b, void* c, int d, void* e, int f, void* g)    \
{                                                                                               \
    RunFunctionFmt(my_paaswd_changereq_fct_##A, "pipipip", a, b, c, d, e, f, g);                \
}
SUPER()
#undef GO
static void* find_paaswd_changereq_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_paaswd_changereq_fct_##A == (uintptr_t)fct) return my_paaswd_changereq_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_paaswd_changereq_fct_##A == 0) {my_paaswd_changereq_fct_##A = (uintptr_t)fct; return my_paaswd_changereq_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libssh2 paaswd_changereq callback\n");
    return NULL;
}
// userauth_kbdint_response
#define GO(A)   \
static uintptr_t my_userauth_kbdint_response_fct_##A = 0;                                                       \
static void my_userauth_kbdint_response_##A(void* a, int b, void* c, int d, int e, void* f, void* g, void* h)   \
{                                                                                                               \
    RunFunctionFmt(my_userauth_kbdint_response_fct_##A, "pipiippp", a, b, c, d, e, f, g, h);                    \
}
SUPER()
#undef GO
static void* find_userauth_kbdint_response_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_userauth_kbdint_response_fct_##A == (uintptr_t)fct) return my_userauth_kbdint_response_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_userauth_kbdint_response_fct_##A == 0) {my_userauth_kbdint_response_fct_##A = (uintptr_t)fct; return my_userauth_kbdint_response_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libssh2 userauth_kbdint_response callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_libssh2_userauth_password_ex(x64emu_t* emu, void* session, void* username, uint32_t name_len, void* passw, uint32_t pwd_len, void* f)
{
    return my->libssh2_userauth_password_ex(session, username, name_len, passw, pwd_len, find_paaswd_changereq_Fct(f));
}

EXPORT void* my_libssh2_session_init_ex(x64emu_t* emu, void* my_alloc, void* my_free, void* my_realloc, void* abstract)
{
    return my->libssh2_session_init_ex(find_alloc_Fct(my_alloc), find_free_Fct(my_free), find_realloc_Fct(my_realloc), abstract);
}

EXPORT int my_libssh2_userauth_keyboard_interactive_ex(x64emu_t* emu, void* session, void* username, uint32_t len, void* f)
{
    return my->libssh2_userauth_keyboard_interactive_ex(session, username, len, find_userauth_kbdint_response_Fct(f));
}

#include "wrappedlib_init.h"
