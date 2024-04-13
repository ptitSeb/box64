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

#define ADDED_FUNCTIONS()           \

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
    return (int)RunFunctionFmt(my_pem_passwd_cb_fct_##A, "piip", buf, size, rwflag, password);  \
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
    return (void*)RunFunctionFmt(my_anonymous_fct_##A, "pppp", a, b, c, d);   \
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
    return (int)RunFunctionFmt(my_verify_fct_##A, "ip", a, b);    \
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
static void* reverse_verify_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_verify_##A == fct) return (void*)my_verify_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFip, fct, 0, NULL);
}

// ex_new
#define GO(A)   \
static uintptr_t my_ex_new_fct_##A = 0;                                                        \
static void my_ex_new_##A(void* parent, void* ptr, void* ad, int idx, long argl, void* argp)   \
{                                                                                           \
    RunFunctionFmt(my_ex_new_fct_##A, "pppilp", parent, ptr, ad, idx, argl, argp);           \
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
    RunFunctionFmt(my_ex_free_fct_##A, "pppilp", parent, ptr, ad, idx, argl, argp);           \
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
    return (int) RunFunctionFmt(my_ex_dup_fct_##A, "pppilp", to, from, from_d, idx, argl, argp);  \
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
    return RunFunctionFmt(my_client_cb_fct_##A, "pppupu", ssl, hint, identity, id_len, psk, psk_len);                 \
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


// server_cb
#define GO(A)   \
static uintptr_t my_server_cb_fct_##A = 0;                                                  \
static uint32_t my_server_cb_##A(void* ssl, void* identity, void* psk, uint32_t psk_len)    \
{                                                                                           \
    return RunFunctionFmt(my_server_cb_fct_##A, "pppu", ssl, identity, psk, psk_len);    \
}
SUPER()
#undef GO
static void* find_server_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_server_cb_fct_##A == (uintptr_t)fct) return my_server_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_server_cb_fct_##A == 0) {my_server_cb_fct_##A = (uintptr_t)fct; return my_server_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL server_cb callback\n");
    return NULL;
}


// use_session_cb
#define GO(A)   \
static uintptr_t my_use_session_cb_fct_##A = 0;                                                         \
static uint32_t my_use_session_cb_##A(void* ssl, void* md, void* id, void* id_len, void* sess)          \
{                                                                                                       \
    return RunFunctionFmt(my_use_session_cb_fct_##A, "ppppp", ssl, md, id, id_len, sess);   \
}
SUPER()
#undef GO
static void* find_use_session_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_use_session_cb_fct_##A == (uintptr_t)fct) return my_use_session_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_use_session_cb_fct_##A == 0) {my_use_session_cb_fct_##A = (uintptr_t)fct; return my_use_session_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL use_session_cb callback\n");
    return NULL;
}

// sess
#define GO(A)   \
static uintptr_t my_sess_fct_##A = 0;                                   \
static uint32_t my_sess_##A(void* ssl, void* sess)                      \
{                                                                       \
    return RunFunctionFmt(my_sess_fct_##A, "pp", ssl, sess);\
}
SUPER()
#undef GO
static void* find_sess_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_sess_fct_##A == (uintptr_t)fct) return my_sess_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_sess_fct_##A == 0) {my_sess_fct_##A = (uintptr_t)fct; return my_sess_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL sess callback\n");
    return NULL;
}

// proto_select
#define GO(A)   \
static uintptr_t my_proto_select_fct_##A = 0;                                                           \
static int my_proto_select_##A(void* s, void* out, void* outlen, void* in, uint32_t inlen, void* arg)   \
{                                                                                                       \
    return (int)RunFunctionFmt(my_proto_select_fct_##A, "ppppup", s, out, outlen, in, inlen, arg);    \
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

// client_cert
#define GO(A)   \
static uintptr_t my_client_cert_fct_##A = 0;                                    \
static int my_client_cert_##A(void* a, void* b, void* c)                        \
{                                                                               \
    return (int)RunFunctionFmt(my_client_cert_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* find_client_cert_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_client_cert_fct_##A == (uintptr_t)fct) return my_client_cert_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_client_cert_fct_##A == 0) {my_client_cert_fct_##A = (uintptr_t)fct; return my_client_cert_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL client_cert callback\n");
    return NULL;
}

// cookie_generate
#define GO(A)   \
static uintptr_t my_cookie_generate_fct_##A = 0;            \
static int my_cookie_generate_##A(void* a, void* b, void* c)\
{                                                           \
    return (int)RunFunctionFmt(my_cookie_generate_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* find_cookie_generate_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_cookie_generate_fct_##A == (uintptr_t)fct) return my_cookie_generate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cookie_generate_fct_##A == 0) {my_cookie_generate_fct_##A = (uintptr_t)fct; return my_cookie_generate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL cookie_generate callback\n");
    return NULL;
}


// cookie_verify
#define GO(A)   \
static uintptr_t my_cookie_verify_fct_##A = 0;                  \
static int my_cookie_verify_##A(void* a, void* b, uint32_t c)   \
{                                                               \
    return (int)RunFunctionFmt(my_cookie_verify_fct_##A, "ppu", a, b, c);   \
}
SUPER()
#undef GO
static void* find_cookie_verify_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_cookie_verify_fct_##A == (uintptr_t)fct) return my_cookie_verify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cookie_verify_fct_##A == 0) {my_cookie_verify_fct_##A = (uintptr_t)fct; return my_cookie_verify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL cookie_verify callback\n");
    return NULL;
}

// alpn_select
#define GO(A)   \
static uintptr_t my_alpn_select_fct_##A = 0;                                            \
static int my_alpn_select_##A(void* a, void* b, void* c, void* d, uint32_t e, void* f)  \
{                                                                                       \
    return (int)RunFunctionFmt(my_alpn_select_fct_##A, "ppppup", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_alpn_select_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_alpn_select_fct_##A == (uintptr_t)fct) return my_alpn_select_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alpn_select_fct_##A == 0) {my_alpn_select_fct_##A = (uintptr_t)fct; return my_alpn_select_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL alpn_select callback\n");
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

EXPORT void my_SSL_set_psk_server_callback(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_set_psk_server_callback(ctx, find_server_cb_Fct(cb));
}

EXPORT void my_SSL_set_psk_use_session_callback(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_set_psk_use_session_callback(ctx, find_use_session_cb_Fct(cb));
}

EXPORT void my_SSL_CTX_sess_set_new_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_sess_set_new_cb(ctx, find_sess_Fct(cb));
}

EXPORT void my_SSL_CTX_set_next_proto_select_cb(x64emu_t* emu, void* ctx, void* cb, void* arg)
{
    (void)emu;
    my->SSL_CTX_set_next_proto_select_cb(ctx, find_proto_select_Fct(cb), arg);
}

EXPORT void* my_SSL_get_verify_callback(x64emu_t* emu, void* ctx)
{
    (void)emu;
    return reverse_verify_Fct(my->SSL_get_verify_callback(ctx));
}

EXPORT void my_SSL_CTX_set_cert_verify_callback(x64emu_t* emu, void* ctx, void* cb, void* arg)
{
    (void)emu;
    my->SSL_CTX_set_cert_verify_callback(ctx, find_verify_Fct(cb), arg);
}

EXPORT void my_SSL_CTX_set_client_cert_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_set_client_cert_cb(ctx, find_client_cert_Fct(cb));
}

EXPORT void my_SSL_CTX_set_cookie_generate_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_set_cookie_generate_cb(ctx, find_cookie_generate_Fct(cb));
}

EXPORT void my_SSL_CTX_set_cookie_verify_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->SSL_CTX_set_cookie_verify_cb(ctx, find_cookie_verify_Fct(cb));
}

EXPORT void my_SSL_CTX_set_alpn_select_cb(x64emu_t* emu, void* ctx, void* f ,void* arg)
{
    (void)emu;
    my->SSL_CTX_set_alpn_select_cb(ctx, find_alpn_select_Fct(f), arg);
}

#define NEEDED_LIBS "libcrypto.so.1.1", "libpthread.so.0"

#include "wrappedlib_init.h"
