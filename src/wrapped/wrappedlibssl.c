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

const char* libsslName = "libssl.so.1.0.0";
#define LIBNAME libssl
#define ALTNAME "libssl.so.1.0.2"
#define ALTNAME2 "libssl.so.1.1"

#include "generated/wrappedlibssltypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// pem_passwd_cb
#define GO(A)   \
static uintptr_t my_pem_passwd_cb_fct_##A = 0;                                                      \
static int my_pem_passwd_cb_##A(void* buf, int size, int rwflag, void* password)                    \
{                                                                                                   \
    return (int)RunFunction(my_context, my_pem_passwd_cb_fct_##A, 4, buf, size, rwflag, password);  \
}
SUPER()
#undef GO
static void* find_pem_passwd_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_pem_passwd_cb_fct_##A == (uintptr_t)fct) return my_pem_passwd_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pem_passwd_cb_fct_##A == 0) {my_pem_passwd_cb_fct_##A = (uintptr_t)fct; return my_pem_passwd_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL pem_passwd_cb callback\n");
    return NULL;
}

// anonymous
#define GO(A)   \
static uintptr_t my_anonymous_fct_##A = 0;                                      \
static void* my_anonymous_##A(void* a, void* b, void* c, void *d)               \
{                                                                               \
    return (void*)RunFunction(my_context, my_anonymous_fct_##A, 4, a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_anonymous_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_anonymous_fct_##A == (uintptr_t)fct) return my_anonymous_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_anonymous_fct_##A == 0) {my_anonymous_fct_##A = (uintptr_t)fct; return my_anonymous_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL anonymous callback\n");
    return NULL;
}


// verify
#define GO(A)   \
static uintptr_t my_verify_fct_##A = 0;                                 \
static int my_verify_##A(int a, void* b)                                \
{                                                                       \
    return (int)RunFunction(my_context, my_verify_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_verify_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_verify_fct_##A == (uintptr_t)fct) return my_verify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_verify_fct_##A == 0) {my_verify_fct_##A = (uintptr_t)fct; return my_verify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL verify callback\n");
    return NULL;
}

// ex_new
#define GO(A)   \
static uintptr_t my_ex_new_fct_##A = 0;                                                        \
static void my_ex_new_##A(void* parent, void* ptr, void* ad, int idx, long argl, void* argp)   \
{                                                                                           \
    RunFunction(my_context, my_ex_new_fct_##A, 6, parent, ptr, ad, idx, argl, argp);           \
}
SUPER()
#undef GO
static void* find_ex_new_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ex_new_fct_##A == (uintptr_t)fct) return my_ex_new_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ex_new_fct_##A == 0) {my_ex_new_fct_##A = (uintptr_t)fct; return my_ex_new_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL ex_new callback\n");
    return NULL;
}

// ex_free
#define GO(A)   \
static uintptr_t my_ex_free_fct_##A = 0;                                                        \
static void my_ex_free_##A(void* parent, void* ptr, void* ad, int idx, long argl, void* argp)   \
{                                                                                               \
    RunFunction(my_context, my_ex_free_fct_##A, 6, parent, ptr, ad, idx, argl, argp);           \
}
SUPER()
#undef GO
static void* find_ex_free_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ex_free_fct_##A == (uintptr_t)fct) return my_ex_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ex_free_fct_##A == 0) {my_ex_free_fct_##A = (uintptr_t)fct; return my_ex_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL ex_free callback\n");
    return NULL;
}

// ex_dup
#define GO(A)   \
static uintptr_t my_ex_dup_fct_##A = 0;                                                             \
static int my_ex_dup_##A(void* to, void* from, void* from_d, int idx, long argl, void* argp)        \
{                                                                                                   \
    return (int) RunFunction(my_context, my_ex_dup_fct_##A, 6, to, from, from_d, idx, argl, argp);  \
}
SUPER()
#undef GO
static void* find_ex_dup_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ex_dup_fct_##A == (uintptr_t)fct) return my_ex_dup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ex_dup_fct_##A == 0) {my_ex_dup_fct_##A = (uintptr_t)fct; return my_ex_dup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL ex_dup callback\n");
    return NULL;
}

// client_cb
#define GO(A)   \
static uintptr_t my_client_cb_fct_##A = 0;                                                                              \
static uint32_t my_client_cb_##A(void* ssl, void* hint, void* identity, uint32_t id_len, void* psk, uint32_t psk_len)   \
{                                                                                                                       \
    return RunFunction(my_context, my_client_cb_fct_##A, 6, ssl, hint, identity, id_len, psk, psk_len);                 \
}
SUPER()
#undef GO
static void* find_client_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_client_cb_fct_##A == (uintptr_t)fct) return my_client_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_client_cb_fct_##A == 0) {my_client_cb_fct_##A = (uintptr_t)fct; return my_client_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL client_cb callback\n");
    return NULL;
}

// proto_select
#define GO(A)   \
static uintptr_t my_proto_select_fct_##A = 0;                                                           \
static int my_proto_select_##A(void* s, void* out, void* outlen, void* in, uint32_t inlen, void* arg)   \
{                                                                                                       \
    return (int)RunFunction(my_context, my_proto_select_fct_##A, 6, s, out, outlen, in, inlen, arg);    \
}
SUPER()
#undef GO
static void* find_proto_select_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_proto_select_fct_##A == (uintptr_t)fct) return my_proto_select_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_proto_select_fct_##A == 0) {my_proto_select_fct_##A = (uintptr_t)fct; return my_proto_select_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL proto_select callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_SSL_CTX_set_default_passwd_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_set_default_passwd_cb(ctx, find_pem_passwd_cb_Fct(cb));
}

EXPORT long my_SSL_CTX_callback_ctrl(x64emu_t* emu, void* ctx, int cmd, void* f)
{
    (void)emu;
    return my->SSL_CTX_callback_ctrl(ctx, cmd, find_anonymous_Fct(f));
}

EXPORT long my_SSL_callback_ctrl(x64emu_t* emu, void* ctx, int cmd, void* f)
{
    (void)emu;
    return my->SSL_callback_ctrl(ctx, cmd, find_anonymous_Fct(f));
}

EXPORT void my_SSL_CTX_set_verify(x64emu_t* emu, void* ctx, int mode, void* f)
{
    (void)emu;
    my->SSL_CTX_set_verify(ctx, mode, find_verify_Fct(f));
}

EXPORT void my_SSL_set_verify(x64emu_t* emu, void* ctx, int mode, void* f)
{
    (void)emu;
    my->SSL_set_verify(ctx, mode, find_verify_Fct(f));
}

EXPORT void my_SSL_get_ex_new_index(x64emu_t* emu, long argl, void* argp, void* new_func, void* dup_func, void* free_func)
{
    (void)emu;
    my->SSL_get_ex_new_index(argl, argp, find_ex_new_Fct(new_func), find_ex_dup_Fct(dup_func), find_ex_free_Fct(free_func));
}

EXPORT void my_SSL_set_psk_client_callback(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_set_psk_client_callback(ctx, find_client_cb_Fct(cb));
}

EXPORT void my_SSL_CTX_set_next_proto_select_cb(x64emu_t* emu, void* ctx, void* cb, void* arg)
{
    (void)emu;
    my->SSL_CTX_set_next_proto_select_cb(ctx, find_proto_select_Fct(cb), arg);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
