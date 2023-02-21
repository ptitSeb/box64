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
#include "callback.h"

const char* cryptoName = "libcrypto.so.1.0.0";
#define LIBNAME crypto
#define ALTNAME "libcrypto.so.1.0.2"
#define ALTNAME2 "libcrypto.so.1.1"

#include "generated/wrappedcryptotypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// ENGINE_ctrl_cb
#define GO(A)   \
static uintptr_t my_ENGINE_ctrl_cb_fct_##A = 0;                                                      \
static void my_ENGINE_ctrl_cb_##A()                    \
{                                                                                                   \
    RunFunction(my_context, my_ENGINE_ctrl_cb_fct_##A, 0);  \
}
SUPER()
#undef GO
static void* find_ENGINE_ctrl_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ENGINE_ctrl_cb_fct_##A == (uintptr_t)fct) return my_ENGINE_ctrl_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ENGINE_ctrl_cb_fct_##A == 0) {my_ENGINE_ctrl_cb_fct_##A = (uintptr_t)fct; return my_ENGINE_ctrl_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto ENGINE_ctrl_cb callback\n");
    return NULL;
}

// cmp_fnc
#define GO(A)   \
static uintptr_t my_cmp_fnc_fct_##A = 0;                                \
static int my_cmp_fnc_##A(void* a, void* b)                             \
{                                                                       \
    return (int)RunFunction(my_context, my_cmp_fnc_fct_##A, 2, a, b);   \
}
SUPER()
#undef GO
static void* find_cmp_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_cmp_fnc_fct_##A == (uintptr_t)fct) return my_cmp_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cmp_fnc_fct_##A == 0) {my_cmp_fnc_fct_##A = (uintptr_t)fct; return my_cmp_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto cmp_fnc callback\n");
    return NULL;
}

// free_fnc
#define GO(A)   \
static uintptr_t my_free_fnc_fct_##A = 0;               \
static void my_free_fnc_##A(void* p)                    \
{                                                       \
    RunFunction(my_context, my_free_fnc_fct_##A, 1, p); \
}
SUPER()
#undef GO
static void* find_free_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_free_fnc_fct_##A == (uintptr_t)fct) return my_free_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fnc_fct_##A == 0) {my_free_fnc_fct_##A = (uintptr_t)fct; return my_free_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto free_fnc callback\n");
    return NULL;
}

// id_func
#define GO(A)   \
static uintptr_t my_id_func_fct_##A = 0;                                    \
static unsigned long my_id_func_##A()                                       \
{                                                                           \
    return (unsigned long)RunFunction(my_context, my_id_func_fct_##A, 0);   \
}
SUPER()
#undef GO
static void* find_id_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_id_func_fct_##A == (uintptr_t)fct) return my_id_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_id_func_fct_##A == 0) {my_id_func_fct_##A = (uintptr_t)fct; return my_id_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto id_func callback\n");
    return NULL;
}

// lock_func
#define GO(A)   \
static uintptr_t my_lock_func_fct_##A = 0;                              \
static void my_lock_func_##A(int mode, int n, void* f, int l)           \
{                                                                       \
    RunFunction(my_context, my_lock_func_fct_##A, 4, mode, n, f, l);    \
}
SUPER()
#undef GO
static void* find_lock_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_lock_func_fct_##A == (uintptr_t)fct) return my_lock_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lock_func_fct_##A == 0) {my_lock_func_fct_##A = (uintptr_t)fct; return my_lock_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto lock_func callback\n");
    return NULL;
}

// passphrase
#define GO(A)   \
static uintptr_t my_passphrase_fct_##A = 0;                                             \
static int my_passphrase_##A(void* buff, int size, int rw, void* u)                     \
{                                                                                       \
    return (int)RunFunction(my_context, my_passphrase_fct_##A, 4, buff, size, rw, u);   \
}
SUPER()
#undef GO
static void* find_passphrase_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_passphrase_fct_##A == (uintptr_t)fct) return my_passphrase_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_passphrase_fct_##A == 0) {my_passphrase_fct_##A = (uintptr_t)fct; return my_passphrase_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto passphrase callback\n");
    return NULL;
}

// xnew
#define GO(A)   \
static uintptr_t my_xnew_fct_##A = 0;                           \
static void* my_xnew_##A()                                      \
{                                                               \
    return (void*)RunFunction(my_context, my_xnew_fct_##A, 0);  \
}
SUPER()
#undef GO
static void* find_xnew_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_xnew_fct_##A == (uintptr_t)fct) return my_xnew_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xnew_fct_##A == 0) {my_xnew_fct_##A = (uintptr_t)fct; return my_xnew_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto xnew callback\n");
    return NULL;
}

// d2i
#define GO(A)   \
static uintptr_t my_d2i_fct_##A = 0;                            \
static void* my_d2i_##A()                                       \
{                                                               \
    return (void*)RunFunction(my_context, my_d2i_fct_##A, 0);   \
}
SUPER()
#undef GO
static void* find_d2i_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_d2i_fct_##A == (uintptr_t)fct) return my_d2i_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_d2i_fct_##A == 0) {my_d2i_fct_##A = (uintptr_t)fct; return my_d2i_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto d2i callback\n");
    return NULL;
}

// i2d
#define GO(A)   \
static uintptr_t my_i2d_fct_##A = 0;                        \
static int my_i2d_##A()                                     \
{                                                           \
    return (int)RunFunction(my_context, my_i2d_fct_##A, 0); \
}
SUPER()
#undef GO
static void* find_i2d_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_i2d_fct_##A == (uintptr_t)fct) return my_i2d_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_i2d_fct_##A == 0) {my_i2d_fct_##A = (uintptr_t)fct; return my_i2d_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto i2d callback\n");
    return NULL;
}

// pem_password_cb
#define GO(A)   \
static uintptr_t my_pem_password_cb_fct_##A = 0;                                    \
static int my_pem_password_cb_##A(void* a, int b, int c, void* d)                   \
{                                                                                   \
    return (int)RunFunction(my_context, my_pem_password_cb_fct_##A, 4, a, b, c, d); \
}
SUPER()
#undef GO
static void* find_pem_password_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_pem_password_cb_fct_##A == (uintptr_t)fct) return my_pem_password_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pem_password_cb_fct_##A == 0) {my_pem_password_cb_fct_##A = (uintptr_t)fct; return my_pem_password_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto pem_password_cb callback\n");
    return NULL;
}

// verify_cb
#define GO(A)   \
static uintptr_t my_verify_cb_fct_##A = 0;                              \
static int my_verify_cb_##A(int a, void* b)                             \
{                                                                       \
    return (int)RunFunction(my_context, my_verify_cb_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* find_verify_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_verify_cb_fct_##A == (uintptr_t)fct) return my_verify_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_verify_cb_fct_##A == 0) {my_verify_cb_fct_##A = (uintptr_t)fct; return my_verify_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto verify_cb callback\n");
    return NULL;
}

#undef SUPER

EXPORT int32_t my_ENGINE_ctrl(x64emu_t* emu, void* e, int32_t cmd, int32_t i, void* p, void* f)
{
    (void)emu;
    return my->ENGINE_ctrl(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f));
}

EXPORT int32_t my_ENGINE_ctrl_cmd(x64emu_t* emu, void* e, void* cmd, long i, void* p, void* f, int optional)
{
    (void)emu;
    return my->ENGINE_ctrl_cmd(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f), optional);
}

EXPORT void my_CRYPTO_set_id_callback(x64emu_t* emu, void* cb)
{
    (void)emu;
    my->CRYPTO_set_id_callback(find_id_func_Fct(cb));
}

EXPORT void my_CRYPTO_set_locking_callback(x64emu_t* emu, void* cb)
{
    (void)emu;
    my->CRYPTO_set_locking_callback(find_lock_func_Fct(cb));
}

EXPORT void my_PEM_read_bio_DSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_DSAPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_DSA_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_DSA_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_RSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSAPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_RSA_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSA_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_ECPrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_ECPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_EC_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_EC_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT int my_PEM_write_bio_DSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_DSAPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT int my_PEM_write_bio_RSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_RSAPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT int my_PEM_write_bio_ECPrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_ECPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT void* my_sk_new(x64emu_t* emu, void* f)
{
    (void)emu;
    return my->sk_new(find_cmp_fnc_Fct(f));
}

EXPORT void my_sk_pop_free(x64emu_t* emu, void* st, void* f)
{
    (void)emu;
    my->sk_pop_free(st, find_free_fnc_Fct(f));
}

EXPORT void* my_ASN1_d2i_bio(x64emu_t* emu, void* xnew, void* d2i, void* bp, void* x)
{
    (void)emu;
    return my->ASN1_d2i_bio(find_xnew_Fct(xnew), find_d2i_Fct(d2i), bp, x);
}

EXPORT int my_ASN1_i2d_bio(x64emu_t* emu, void* i2d, void* bp, void* x)
{
    (void)emu;
    return my->ASN1_i2d_bio(find_i2d_Fct(i2d), bp, x);
}

EXPORT void* my_PEM_read_bio_PKCS7(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_PKCS7(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my_PEM_read_bio_X509(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my_PEM_read_bio_X509_AUX(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_AUX(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my_PEM_read_bio_X509_CRL(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_CRL(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my_PEM_read_bio_X509_REQ(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_REQ(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my_PEM_read_bio_X509_CERT_PAIR(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_CERT_PAIR(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void my_X509_STORE_CTX_set_verify_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->X509_STORE_CTX_set_verify_cb(ctx, find_verify_cb_Fct(cb));
}

EXPORT void my_OPENSSL_sk_pop_free(x64emu_t* emu, void* s, void* cb)
{
    (void)emu;
    my->OPENSSL_sk_pop_free(s, find_free_fnc_Fct(cb));
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
