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
static uintptr_t my3_pem_passwd_cb_fct_##A = 0;                                                     \
static int my3_pem_passwd_cb_##A(void* buf, int size, int rwflag, void* password)                   \
{                                                                                                   \
    return (int)RunFunctionFmt(my3_pem_passwd_cb_fct_##A, "piip", buf, size, rwflag, password);     \
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
static uintptr_t my3_anonymous_fct_##A = 0;                                     \
static void* my3_anonymous_##A(void* a, void* b, void* c, void *d)              \
{                                                                               \
    return (void*)RunFunctionFmt(my3_anonymous_fct_##A, "pppp", a, b, c, d);    \
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
static uintptr_t my3_verify_fct_##A = 0;                            \
static int my3_verify_##A(int a, void* b)                           \
{                                                                   \
    return (int)RunFunctionFmt(my3_verify_fct_##A, "ip", a, b);     \
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
{                                                                                               \
    RunFunctionFmt(my3_ex_new_fct_##A, "pppilp", parent, ptr, ad, idx, argl, argp);             \
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
static uintptr_t my3_ex_free_fct_##A = 0;                                                       \
static void my3_ex_free_##A(void* parent, void* ptr, void* ad, int idx, long argl, void* argp)  \
{                                                                                               \
    RunFunctionFmt(my3_ex_free_fct_##A, "pppilp", parent, ptr, ad, idx, argl, argp);            \
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
static uintptr_t my3_ex_dup_fct_##A = 0;                                                            \
static int my3_ex_dup_##A(void* to, void* from, void* from_d, int idx, long argl, void* argp)       \
{                                                                                                   \
    return (int) RunFunctionFmt(my3_ex_dup_fct_##A, "pppilp", to, from, from_d, idx, argl, argp);   \
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
static uintptr_t my3_client_cb_fct_##A = 0;                                                                             \
static uint32_t my3_client_cb_##A(void* ssl, void* hint, void* identity, uint32_t id_len, void* psk, uint32_t psk_len)  \
{                                                                                                                       \
    return RunFunctionFmt(my3_client_cb_fct_##A, "pppupu", ssl, hint, identity, id_len, psk, psk_len);                  \
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
static uintptr_t my3_proto_select_fct_##A = 0;                                                          \
static int my3_proto_select_##A(void* s, void* out, void* outlen, void* in, uint32_t inlen, void* arg)  \
{                                                                                                       \
    return (int)RunFunctionFmt(my3_proto_select_fct_##A, "ppppup", s, out, outlen, in, inlen, arg);     \
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
static uintptr_t my3_client_cert_fct_##A = 0;                               \
static int my3_client_cert_##A(void* a, void* b, void* c)                   \
{                                                                           \
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
    return (int)RunFunctionFmt(my3_alpn_select_cb_fct_##A, "ppppup", a, b, c, d, e, f);             \
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
// info_cb
#define GO(A)   \
static uintptr_t my3_info_cb_fct_##A = 0;                   \
static void my3_info_cb_##A(void* a, int b, int c)          \
{                                                           \
    RunFunctionFmt(my3_info_cb_fct_##A, "pii", a, b, c);    \
}
SUPER()
#undef GO
static void* find_info_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_info_cb_fct_##A == (uintptr_t)fct) return my3_info_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_info_cb_fct_##A == 0) {my3_info_cb_fct_##A = (uintptr_t)fct; return my3_info_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSSL info_cb callback\n");
    return NULL;
}
// new_seesion_cb
#define GO(A)   \
static uintptr_t my_new_seesion_cb_fct_##A = 0;                         \
static int my_new_seesion_cb_##A(void* a, void* b)                      \
{                                                                       \
    return (int)RunFunctionFmt(my_new_seesion_cb_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* find_new_seesion_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_new_seesion_cb_fct_##A == (uintptr_t)fct) return my_new_seesion_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_new_seesion_cb_fct_##A == 0) {my_new_seesion_cb_fct_##A = (uintptr_t)fct; return my_new_seesion_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 new_seesion_cb callback\n");
    return NULL;
}
// psk_use_session_cb
#define GO(A)   \
static uintptr_t my_psk_use_session_cb_fct_##A = 0;                                     \
static int my_psk_use_session_cb_##A(void* a, void* b, void* c, void* d, void* e)       \
{                                                                                       \
    return (int)RunFunctionFmt(my_psk_use_session_cb_fct_##A, "ppppp", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* find_psk_use_session_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_psk_use_session_cb_fct_##A == (uintptr_t)fct) return my_psk_use_session_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_psk_use_session_cb_fct_##A == 0) {my_psk_use_session_cb_fct_##A = (uintptr_t)fct; return my_psk_use_session_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 psk_use_session_cb callback\n");
    return NULL;
}
// app_verify_cookie_cb
#define GO(A)   \
static uintptr_t my_app_verify_cookie_cb_fct_##A = 0;                               \
static int my_app_verify_cookie_cb_##A(void* a, void* b, uint32_t c)                \
{                                                                                   \
    return (int)RunFunctionFmt(my_app_verify_cookie_cb_fct_##A, "ppu", a, b, c);    \
}
SUPER()
#undef GO
static void* find_app_verify_cookie_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_app_verify_cookie_cb_fct_##A == (uintptr_t)fct) return my_app_verify_cookie_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_app_verify_cookie_cb_fct_##A == 0) {my_app_verify_cookie_cb_fct_##A = (uintptr_t)fct; return my_app_verify_cookie_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 app_verify_cookie_cb callback\n");
    return NULL;
}
// set_cookie_generate_cb
#define GO(A)   \
static uintptr_t my_set_cookie_generate_cb_fct_##A = 0;                             \
static int my_set_cookie_generate_cb_##A(void* a, void* b, void* c)                 \
{                                                                                   \
    return (int)RunFunctionFmt(my_set_cookie_generate_cb_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* find_set_cookie_generate_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_set_cookie_generate_cb_fct_##A == (uintptr_t)fct) return my_set_cookie_generate_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_set_cookie_generate_cb_fct_##A == 0) {my_set_cookie_generate_cb_fct_##A = (uintptr_t)fct; return my_set_cookie_generate_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 set_cookie_generate_cb callback\n");
    return NULL;
}
// psk_server_cb
#define GO(A)   \
static uintptr_t my_psk_server_cb_fct_##A = 0;                                      \
static uint32_t my_psk_server_cb_##A(void* a, void* b, void* c, uint32_t d)         \
{                                                                                   \
    return (uint32_t)RunFunctionFmt(my_psk_server_cb_fct_##A, "pppu", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_psk_server_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_psk_server_cb_fct_##A == (uintptr_t)fct) return my_psk_server_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_psk_server_cb_fct_##A == 0) {my_psk_server_cb_fct_##A = (uintptr_t)fct; return my_psk_server_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 psk_server_cb callback\n");
    return NULL;
}
// psk_server_callback
#define GO(A)   \
static uintptr_t my_psk_server_callback_fct_##A = 0;                                        \
static uint32_t my_psk_server_callback_##A(void* a, void* b, void* c, int d)                \
{                                                                                           \
    return (uint32_t)RunFunctionFmt(my_psk_server_callback_fct_##A, "pppi", a, b, c, d);    \
}
SUPER()
#undef GO
static void* find_psk_server_callback_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_psk_server_callback_fct_##A == (uintptr_t)fct) return my_psk_server_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_psk_server_callback_fct_##A == 0) {my_psk_server_callback_fct_##A = (uintptr_t)fct; return my_psk_server_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 psk_server_callback callback\n");
    return NULL;
}
// read_write
#define GO(A)   \
static uintptr_t my_read_write_fct_##A = 0;                             \
static int my_read_write_##A(void* a, void* b, int c)                   \
{                                                                       \
    return (int)RunFunctionFmt(my_read_write_fct_##A, "ppi", a, b, c);  \
}
SUPER()
#undef GO
static void* find_read_write_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_read_write_fct_##A == (uintptr_t)fct) return my_read_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_write_fct_##A == 0) {my_read_write_fct_##A = (uintptr_t)fct; return my_read_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 read_write callback\n");
    return NULL;
}
// puts
#define GO(A)   \
static uintptr_t my_puts_fct_##A = 0;                           \
static int my_puts_##A(void* a, void* b)                        \
{                                                               \
    return (int)RunFunctionFmt(my_puts_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_puts_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_puts_fct_##A == (uintptr_t)fct) return my_puts_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_puts_fct_##A == 0) {my_puts_fct_##A = (uintptr_t)fct; return my_puts_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 puts callback\n");
    return NULL;
}
// ctrl
#define GO(A)   \
static uintptr_t my_ctrl_fct_##A = 0;                                   \
static long my_ctrl_##A(void* a, int b, long c, void* d)                \
{                                                                       \
    return (long)RunFunctionFmt(my_ctrl_fct_##A, "pilp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_ctrl_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ctrl_fct_##A == (uintptr_t)fct) return my_ctrl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ctrl_fct_##A == 0) {my_ctrl_fct_##A = (uintptr_t)fct; return my_ctrl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 ctrl callback\n");
    return NULL;
}
// create_destroy
#define GO(A)   \
static uintptr_t my_create_destroy_fct_##A = 0;                     \
static int my_create_destroy_##A(void* a)                           \
{                                                                   \
    return (int)RunFunctionFmt(my_create_destroy_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_create_destroy_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_create_destroy_fct_##A == (uintptr_t)fct) return my_create_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_create_destroy_fct_##A == 0) {my_create_destroy_fct_##A = (uintptr_t)fct; return my_create_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 create_destroy callback\n");
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

EXPORT void my3_SSL_CTX_sess_set_new_cb(x64emu_t* emu, void *ctx, void* f)
{
    my->SSL_CTX_sess_set_new_cb(ctx, find_new_seesion_cb_Fct(f));
}

EXPORT void my3_SSL_set_info_callback(x64emu_t* emmu, void* ctx, void* f)
{
    my->SSL_set_info_callback(ctx, find_info_cb_Fct(f));
}

EXPORT void my3_SSL_CTX_set_info_callback(x64emu_t* emu, void* ctx, void* f)
{
    (void)emu;
    my->SSL_CTX_set_info_callback(ctx, find_info_cb_Fct(f));
}

EXPORT void my3_SSL_set_psk_use_session_callback(x64emu_t* emu, void* ctx, void* f)
{
    my->SSL_set_psk_use_session_callback(ctx, find_psk_use_session_cb_Fct(f));
}

EXPORT void my3_SSL_CTX_set_cookie_verify_cb(x64emu_t* emu, void* ctx, void* f)
{
    my->SSL_CTX_set_cookie_verify_cb(ctx, find_app_verify_cookie_cb_Fct(f));
}

EXPORT void my3_SSL_CTX_set_cookie_generate_cb(x64emu_t* emu, void* ctx, void* f)
{
    my->SSL_CTX_set_cookie_generate_cb(ctx, find_set_cookie_generate_cb_Fct(f));
}

EXPORT void my3_SSL_set_psk_server_callback(x64emu_t* emu, void* ctx, void* f)
{
    my->SSL_set_psk_server_callback(ctx, find_psk_server_cb_Fct(f));
}

EXPORT int my3_BIO_meth_set_write(x64emu_t* emu, void* biom, void* f)
{
    return my->BIO_meth_set_write(biom, find_read_write_Fct(f));
}

EXPORT int my3_BIO_meth_set_read(x64emu_t* emu, void* biom, void* f)
{
    return my->BIO_meth_set_read(biom, find_read_write_Fct(f));
}

EXPORT int my3_BIO_meth_set_puts(x64emu_t* emu, void* biom, void* f)
{
    return my->BIO_meth_set_puts(biom, find_puts_Fct(f));
}

EXPORT int my3_BIO_meth_set_ctrl(x64emu_t* emu, void* biom, void* f)
{
    return my->BIO_meth_set_ctrl(biom, find_ctrl_Fct(f));
}

EXPORT int my3_BIO_meth_set_create(x64emu_t* emu, void* biom, void* f)
{
    return my->BIO_meth_set_create(biom, find_create_destroy_Fct(f));
}

EXPORT int my3_BIO_meth_set_destroy(x64emu_t* emu, void* biom, void* f)
{
    return my->BIO_meth_set_destroy(biom, find_create_destroy_Fct(f));
}

EXPORT void my3_SSL_CTX_set_psk_server_callback(x64emu_t* emu, void* ssl, void* cb)
{
    my->SSL_CTX_set_psk_server_callback(ssl, find_psk_server_callback_Fct(cb));
}

EXPORT void my3_SSL_CTX_set_psk_client_callback(x64emu_t* emu, void* ssl, void* cb)
{
    my->SSL_CTX_set_psk_client_callback(ssl, find_client_cb_Fct(cb));
}

#define ALTMY my3_

#define NEEDED_LIBS "libcrypto.so.3", "libpthread.so.0"

#include "wrappedlib_init.h"
