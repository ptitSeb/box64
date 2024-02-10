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

const char* libssl3Name = "libssl.so.3";
#define LIBNAME libssl3

#include "generated/wrappedlibssl3types.h"

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
static uintptr_t my3_pem_passwd_cb_fct_##A = 0;                                                      \
static int my3_pem_passwd_cb_##A(void* buf, int size, int rwflag, void* password)                    \
{                                                                                                   \
    return (int)RunFunctionFmt(my3_pem_passwd_cb_fct_##A, "piip", buf, size, rwflag, password);  \
}
SUPER()
#undef GO
static void* find_pem_passwd_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_pem_passwd_cb_fct_##A == (uintptr_t)fct) return my3_pem_passwd_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_pem_passwd_cb_fct_##A == 0) {my3_pem_passwd_cb_fct_##A = (uintptr_t)fct; return my3_pem_passwd_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL pem_passwd_cb callback\n");
    return NULL;
}
static void* reverse_pem_passwd_cb_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my3_pem_passwd_cb_##A == fct) return (void*)my3_pem_passwd_cb_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFp, fct, 0, NULL);
}

// anonymous
#define GO(A)   \
static uintptr_t my3_anonymous_fct_##A = 0;                                      \
static void* my3_anonymous_##A(void* a, void* b, void* c, void *d)               \
{                                                                               \
    return (void*)RunFunctionFmt(my3_anonymous_fct_##A, "pppp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_anonymous_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_anonymous_fct_##A == (uintptr_t)fct) return my3_anonymous_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_anonymous_fct_##A == 0) {my3_anonymous_fct_##A = (uintptr_t)fct; return my3_anonymous_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL anonymous callback\n");
    return NULL;
}


// verify
#define GO(A)   \
static uintptr_t my3_verify_fct_##A = 0;                                 \
static int my3_verify_##A(int a, void* b)                                \
{                                                                       \
    return (int)RunFunctionFmt(my3_verify_fct_##A, "ip", a, b);    \
}
SUPER()
#undef GO
static void* find_verify_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_verify_fct_##A == (uintptr_t)fct) return my3_verify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_verify_fct_##A == 0) {my3_verify_fct_##A = (uintptr_t)fct; return my3_verify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL verify callback\n");
    return NULL;
}
static void* reverse_verify_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my3_verify_##A == fct) return (void*)my3_verify_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFip, fct, 0, NULL);
}

// ex_new
#define GO(A)   \
static uintptr_t my3_ex_new_fct_##A = 0;                                                        \
static void my3_ex_new_##A(void* parent, void* ptr, void* ad, int idx, long argl, void* argp)   \
{                                                                                           \
    RunFunctionFmt(my3_ex_new_fct_##A, "pppilp", parent, ptr, ad, idx, argl, argp);           \
}
SUPER()
#undef GO
static void* find_ex_new_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_ex_new_fct_##A == (uintptr_t)fct) return my3_ex_new_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_ex_new_fct_##A == 0) {my3_ex_new_fct_##A = (uintptr_t)fct; return my3_ex_new_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL ex_new callback\n");
    return NULL;
}

// ex_free
#define GO(A)   \
static uintptr_t my3_ex_free_fct_##A = 0;                                                        \
static void my3_ex_free_##A(void* parent, void* ptr, void* ad, int idx, long argl, void* argp)   \
{                                                                                               \
    RunFunctionFmt(my3_ex_free_fct_##A, "pppilp", parent, ptr, ad, idx, argl, argp);           \
}
SUPER()
#undef GO
static void* find_ex_free_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_ex_free_fct_##A == (uintptr_t)fct) return my3_ex_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_ex_free_fct_##A == 0) {my3_ex_free_fct_##A = (uintptr_t)fct; return my3_ex_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL ex_free callback\n");
    return NULL;
}

// ex_dup
#define GO(A)   \
static uintptr_t my3_ex_dup_fct_##A = 0;                                                             \
static int my3_ex_dup_##A(void* to, void* from, void* from_d, int idx, long argl, void* argp)        \
{                                                                                                   \
    return (int) RunFunctionFmt(my3_ex_dup_fct_##A, "pppilp", to, from, from_d, idx, argl, argp);  \
}
SUPER()
#undef GO
static void* find_ex_dup_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_ex_dup_fct_##A == (uintptr_t)fct) return my3_ex_dup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_ex_dup_fct_##A == 0) {my3_ex_dup_fct_##A = (uintptr_t)fct; return my3_ex_dup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL ex_dup callback\n");
    return NULL;
}

// client_cb
#define GO(A)   \
static uintptr_t my3_client_cb_fct_##A = 0;                                                                              \
static uint32_t my3_client_cb_##A(void* ssl, void* hint, void* identity, uint32_t id_len, void* psk, uint32_t psk_len)   \
{                                                                                                                       \
    return RunFunctionFmt(my3_client_cb_fct_##A, "pppupu", ssl, hint, identity, id_len, psk, psk_len);                 \
}
SUPER()
#undef GO
static void* find_client_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_client_cb_fct_##A == (uintptr_t)fct) return my3_client_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_client_cb_fct_##A == 0) {my3_client_cb_fct_##A = (uintptr_t)fct; return my3_client_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL client_cb callback\n");
    return NULL;
}

// proto_select
#define GO(A)   \
static uintptr_t my3_proto_select_fct_##A = 0;                                                           \
static int my3_proto_select_##A(void* s, void* out, void* outlen, void* in, uint32_t inlen, void* arg)   \
{                                                                                                       \
    return (int)RunFunctionFmt(my3_proto_select_fct_##A, "ppppup", s, out, outlen, in, inlen, arg);    \
}
SUPER()
#undef GO
static void* find_proto_select_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_proto_select_fct_##A == (uintptr_t)fct) return my3_proto_select_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_proto_select_fct_##A == 0) {my3_proto_select_fct_##A = (uintptr_t)fct; return my3_proto_select_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL proto_select callback\n");
    return NULL;
}

// client_cert
#define GO(A)   \
static uintptr_t my3_client_cert_fct_##A = 0;                                    \
static int my3_client_cert_##A(void* a, void* b, void* c)                        \
{                                                                               \
    return (int)RunFunctionFmt(my3_client_cert_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* find_client_cert_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_client_cert_fct_##A == (uintptr_t)fct) return my3_client_cert_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_client_cert_fct_##A == 0) {my3_client_cert_fct_##A = (uintptr_t)fct; return my3_client_cert_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL client_cert callback\n");
    return NULL;
}

// alpn_select_cb
#define GO(A)   \
static uintptr_t my3_alpn_select_cb_fct_##A = 0;                                                    \
static int my3_alpn_select_cb_##A(void* a, void* b, void* c, void* d, uint32_t e, void* f)          \
{                                                                                                   \
    return (int)RunFunctionFmt(my3_alpn_select_cb_fct_##A, "ppppup", a, b, c, d, e, f); \
}
SUPER()
#undef GO
static void* find_alpn_select_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_alpn_select_cb_fct_##A == (uintptr_t)fct) return my3_alpn_select_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_alpn_select_cb_fct_##A == 0) {my3_alpn_select_cb_fct_##A = (uintptr_t)fct; return my3_alpn_select_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL alpn_select_cb callback\n");
    return NULL;
}
// keylog_cb
#define GO(A)   \
static uintptr_t my3_keylog_cb_fct_##A = 0;                 \
static void my3_keylog_cb_##A(void* ssl, void* line)        \
{                                                           \
    RunFunctionFmt(my3_keylog_cb_fct_##A, "pp", ssl, line); \
}
SUPER()
#undef GO
static void* find_keylog_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_keylog_cb_fct_##A == (uintptr_t)fct) return my3_keylog_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_keylog_cb_fct_##A == 0) {my3_keylog_cb_fct_##A = (uintptr_t)fct; return my3_keylog_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL keylog_cb callback\n");
    return NULL;
}
// msg_cb
#define GO(A)   \
static uintptr_t my3_msg_cb_fct_##A = 0;                                                \
static void my3_msg_cb_##A(int a, int b, int c, void* d, size_t e, void* f, void* g)    \
{                                                                                       \
    RunFunctionFmt(my3_msg_cb_fct_##A, "iiipLpp", a, b, c, d, e, f, g);                 \
}
SUPER()
#undef GO
static void* find_msg_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_msg_cb_fct_##A == (uintptr_t)fct) return my3_msg_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_msg_cb_fct_##A == 0) {my3_msg_cb_fct_##A = (uintptr_t)fct; return my3_msg_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL msg_cb callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my3_SSL_CTX_set_default_passwd_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_set_default_passwd_cb(ctx, find_pem_passwd_cb_Fct(cb));
}

EXPORT long my3_SSL_CTX_callback_ctrl(x64emu_t* emu, void* ctx, int cmd, void* f)
{
    (void)emu;
    return my->SSL_CTX_callback_ctrl(ctx, cmd, find_anonymous_Fct(f));
}

EXPORT long my3_SSL_callback_ctrl(x64emu_t* emu, void* ctx, int cmd, void* f)
{
    (void)emu;
    return my->SSL_callback_ctrl(ctx, cmd, find_anonymous_Fct(f));
}

EXPORT void my3_SSL_CTX_set_verify(x64emu_t* emu, void* ctx, int mode, void* f)
{
    (void)emu;
    my->SSL_CTX_set_verify(ctx, mode, find_verify_Fct(f));
}

EXPORT void my3_SSL_set_verify(x64emu_t* emu, void* ctx, int mode, void* f)
{
    (void)emu;
    my->SSL_set_verify(ctx, mode, find_verify_Fct(f));
}

EXPORT void my3_SSL_get_ex_new_index(x64emu_t* emu, long argl, void* argp, void* new_func, void* dup_func, void* free_func)
{
    (void)emu;
    my->SSL_get_ex_new_index(argl, argp, find_ex_new_Fct(new_func), find_ex_dup_Fct(dup_func), find_ex_free_Fct(free_func));
}

EXPORT void my3_SSL_set_psk_client_callback(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_set_psk_client_callback(ctx, find_client_cb_Fct(cb));
}

EXPORT void my3_SSL_CTX_set_next_proto_select_cb(x64emu_t* emu, void* ctx, void* cb, void* arg)
{
    (void)emu;
    my->SSL_CTX_set_next_proto_select_cb(ctx, find_proto_select_Fct(cb), arg);
}

EXPORT void* my3_SSL_get_verify_callback(x64emu_t* emu, void* ctx)
{
    (void)emu;
    return reverse_verify_Fct(my->SSL_get_verify_callback(ctx));
}

EXPORT void my3_SSL_CTX_set_cert_verify_callback(x64emu_t* emu, void* ctx, void* cb, void* arg)
{
    (void)emu;
    my->SSL_CTX_set_cert_verify_callback(ctx, find_verify_Fct(cb), arg);
}

EXPORT void my3_SSL_CTX_set_client_cert_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_set_client_cert_cb(ctx, find_client_cert_Fct(cb));
}

EXPORT void my3_SSL_CTX_set_alpn_select_cb(x64emu_t* emu, void* ctx, void* f, void* arg)
{
    my->SSL_CTX_set_alpn_select_cb(ctx, find_alpn_select_cb_Fct(f), arg);
}

EXPORT void* my3_SSL_CTX_get_default_passwd_cb(x64emu_t* emu, void* ctx)
{
    return reverse_pem_passwd_cb_Fct(my->SSL_CTX_get_default_passwd_cb(ctx));
}

EXPORT void* my3_SSL_CTX_get_verify_callback(x64emu_t* emu, void* ctx)
{
    return reverse_verify_Fct(my->SSL_CTX_get_verify_callback(ctx));
}

EXPORT void my3_SSL_CTX_set_keylog_callback(x64emu_t* emu, void* ctx, void* cb)
{
    my->SSL_CTX_set_keylog_callback(ctx, find_keylog_cb_Fct(cb));
}

EXPORT void my3_SSL_CTX_set_msg_callback(x64emu_t* emu, void* ctx, void* cb)
{
    my->SSL_CTX_set_msg_callback(ctx, find_msg_cb_Fct(cb));
}

EXPORT void my3_SSL_set_msg_callback(x64emu_t* emu, void* ctx, void* cb)
{
    my->SSL_set_msg_callback(ctx, find_msg_cb_Fct(cb));
}

#define ALTMY my3_

#define NEEDED_LIBS "libcrypto.so.3", "libpthread.so.0"

#include "wrappedlib_init.h"
