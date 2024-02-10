#define _GNU_SOURCE
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
#include "callback.h"
#include "myalign.h"

const char* crypto3Name = "libcrypto.so.3";
#define LIBNAME crypto3

typedef int(*iFppA_t)   (void*, void*, va_list);

#define ADDED_FUNCTIONS()                   \
    GO(BIO_vprintf, iFppA_t);               \

#include "generated/wrappedcrypto3types.h"

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
static uintptr_t my3_ENGINE_ctrl_cb_fct_##A = 0;            \
static void my3_ENGINE_ctrl_cb_##A()                        \
{                                                           \
    RunFunctionFmt(my3_ENGINE_ctrl_cb_fct_##A, "");   \
}
SUPER()
#undef GO
static void* find_ENGINE_ctrl_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_ENGINE_ctrl_cb_fct_##A == (uintptr_t)fct) return my3_ENGINE_ctrl_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_ENGINE_ctrl_cb_fct_##A == 0) {my3_ENGINE_ctrl_cb_fct_##A = (uintptr_t)fct; return my3_ENGINE_ctrl_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto ENGINE_ctrl_cb callback\n");
    return NULL;
}

// cmp_fnc
#define GO(A)   \
static uintptr_t my3_cmp_fnc_fct_##A = 0;                                \
static int my3_cmp_fnc_##A(void* a, void* b)                             \
{                                                                        \
    return (int)RunFunctionFmt(my3_cmp_fnc_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_cmp_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_cmp_fnc_fct_##A == (uintptr_t)fct) return my3_cmp_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_cmp_fnc_fct_##A == 0) {my3_cmp_fnc_fct_##A = (uintptr_t)fct; return my3_cmp_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto cmp_fnc callback\n");
    return NULL;
}

// free_fnc
#define GO(A)   \
static uintptr_t my3_free_fnc_fct_##A = 0;               \
static void my3_free_fnc_##A(void* p)                    \
{                                                       \
    RunFunctionFmt(my3_free_fnc_fct_##A, "p", p); \
}
SUPER()
#undef GO
static void* find_free_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_free_fnc_fct_##A == (uintptr_t)fct) return my3_free_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_free_fnc_fct_##A == 0) {my3_free_fnc_fct_##A = (uintptr_t)fct; return my3_free_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto free_fnc callback\n");
    return NULL;
}

// id_func
#define GO(A)   \
static uintptr_t my3_id_func_fct_##A = 0;                                    \
static unsigned long my3_id_func_##A()                                       \
{                                                                           \
    return (unsigned long)RunFunctionFmt(my3_id_func_fct_##A, "");   \
}
SUPER()
#undef GO
static void* find_id_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_id_func_fct_##A == (uintptr_t)fct) return my3_id_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_id_func_fct_##A == 0) {my3_id_func_fct_##A = (uintptr_t)fct; return my3_id_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto id_func callback\n");
    return NULL;
}

// lock_func
#define GO(A)   \
static uintptr_t my3_lock_func_fct_##A = 0;                              \
static void my3_lock_func_##A(int mode, int n, void* f, int l)           \
{                                                                       \
    RunFunctionFmt(my3_lock_func_fct_##A, "iipi", mode, n, f, l);    \
}
SUPER()
#undef GO
static void* find_lock_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_lock_func_fct_##A == (uintptr_t)fct) return my3_lock_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_lock_func_fct_##A == 0) {my3_lock_func_fct_##A = (uintptr_t)fct; return my3_lock_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto lock_func callback\n");
    return NULL;
}

// passphrase
#define GO(A)   \
static uintptr_t my3_passphrase_fct_##A = 0;                                             \
static int my3_passphrase_##A(void* buff, int size, int rw, void* u)                     \
{                                                                                       \
    return (int)RunFunctionFmt(my3_passphrase_fct_##A, "piip", buff, size, rw, u);   \
}
SUPER()
#undef GO
static void* find_passphrase_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_passphrase_fct_##A == (uintptr_t)fct) return my3_passphrase_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_passphrase_fct_##A == 0) {my3_passphrase_fct_##A = (uintptr_t)fct; return my3_passphrase_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto passphrase callback\n");
    return NULL;
}

// xnew
#define GO(A)   \
static uintptr_t my3_xnew_fct_##A = 0;                           \
static void* my3_xnew_##A()                                      \
{                                                               \
    return (void*)RunFunctionFmt(my3_xnew_fct_##A, "");  \
}
SUPER()
#undef GO
static void* find_xnew_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_xnew_fct_##A == (uintptr_t)fct) return my3_xnew_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_xnew_fct_##A == 0) {my3_xnew_fct_##A = (uintptr_t)fct; return my3_xnew_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto xnew callback\n");
    return NULL;
}

// d2i
#define GO(A)   \
static uintptr_t my3_d2i_fct_##A = 0;                            \
static void* my3_d2i_##A()                                       \
{                                                               \
    return (void*)RunFunctionFmt(my3_d2i_fct_##A, "");   \
}
SUPER()
#undef GO
static void* find_d2i_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_d2i_fct_##A == (uintptr_t)fct) return my3_d2i_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_d2i_fct_##A == 0) {my3_d2i_fct_##A = (uintptr_t)fct; return my3_d2i_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto d2i callback\n");
    return NULL;
}

// i2d
#define GO(A)   \
static uintptr_t my3_i2d_fct_##A = 0;                        \
static int my3_i2d_##A()                                     \
{                                                           \
    return (int)RunFunctionFmt(my3_i2d_fct_##A, ""); \
}
SUPER()
#undef GO
static void* find_i2d_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_i2d_fct_##A == (uintptr_t)fct) return my3_i2d_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_i2d_fct_##A == 0) {my3_i2d_fct_##A = (uintptr_t)fct; return my3_i2d_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto i2d callback\n");
    return NULL;
}

// pem_password_cb
#define GO(A)   \
static uintptr_t my3_pem_password_cb_fct_##A = 0;                                    \
static int my3_pem_password_cb_##A(void* a, int b, int c, void* d)                   \
{                                                                                   \
    return (int)RunFunctionFmt(my3_pem_password_cb_fct_##A, "piip", a, b, c, d); \
}
SUPER()
#undef GO
static void* find_pem_password_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_pem_password_cb_fct_##A == (uintptr_t)fct) return my3_pem_password_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_pem_password_cb_fct_##A == 0) {my3_pem_password_cb_fct_##A = (uintptr_t)fct; return my3_pem_password_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto pem_password_cb callback\n");
    return NULL;
}

// verify_cb
#define GO(A)   \
static uintptr_t my3_verify_cb_fct_##A = 0;                                     \
static int my3_verify_cb_##A(int a, void* b)                                    \
{                                                                               \
    return (int)RunFunctionFmt(my3_verify_cb_fct_##A, "ip", a, b);  \
}
SUPER()
#undef GO
static void* find_verify_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_verify_cb_fct_##A == (uintptr_t)fct) return my3_verify_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_verify_cb_fct_##A == 0) {my3_verify_cb_fct_##A = (uintptr_t)fct; return my3_verify_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto verify_cb callback\n");
    return NULL;
}

// do_all_provided_cb
#define GO(A)   \
static uintptr_t my3_do_all_provided_cb_fct_##A = 0;            \
static void my3_do_all_provided_cb_##A(void* a, void* b)        \
{                                                               \
    RunFunctionFmt(my3_do_all_provided_cb_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_do_all_provided_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_do_all_provided_cb_fct_##A == (uintptr_t)fct) return my3_do_all_provided_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_do_all_provided_cb_fct_##A == 0) {my3_do_all_provided_cb_fct_##A = (uintptr_t)fct; return my3_do_all_provided_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto do_all_provided_cb callback\n");
    return NULL;
}

#undef SUPER

EXPORT int32_t my3_ENGINE_ctrl(x64emu_t* emu, void* e, int32_t cmd, int32_t i, void* p, void* f)
{
    (void)emu;
    return my->ENGINE_ctrl(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f));
}

EXPORT int32_t my3_ENGINE_ctrl_cmd(x64emu_t* emu, void* e, void* cmd, long i, void* p, void* f, int optional)
{
    (void)emu;
    return my->ENGINE_ctrl_cmd(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f), optional);
}

EXPORT void my3_CRYPTO_set_id_callback(x64emu_t* emu, void* cb)
{
    (void)emu;
    my->CRYPTO_set_id_callback(find_id_func_Fct(cb));
}

EXPORT void my3_CRYPTO_set_locking_callback(x64emu_t* emu, void* cb)
{
    (void)emu;
    my->CRYPTO_set_locking_callback(find_lock_func_Fct(cb));
}

EXPORT void my3_PEM_read_bio_DSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_DSAPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my3_PEM_read_bio_DSA_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_DSA_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my3_PEM_read_bio_RSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSAPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my3_PEM_read_bio_RSA_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSA_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my3_PEM_read_bio_ECPrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_ECPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my3_PEM_read_bio_EC_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_EC_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT int my3_PEM_write_bio_DSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_DSAPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT int my3_PEM_write_bio_RSAPrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_RSAPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT int my3_PEM_write_bio_ECPrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_ECPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT void* my3_ASN1_d2i_bio(x64emu_t* emu, void* xnew, void* d2i, void* bp, void* x)
{
    (void)emu;
    return my->ASN1_d2i_bio(find_xnew_Fct(xnew), find_d2i_Fct(d2i), bp, x);
}

EXPORT int my3_ASN1_i2d_bio(x64emu_t* emu, void* i2d, void* bp, void* x)
{
    (void)emu;
    return my->ASN1_i2d_bio(find_i2d_Fct(i2d), bp, x);
}

EXPORT void* my3_PEM_read_bio_PKCS7(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_PKCS7(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_X509(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_X509_AUX(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_AUX(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_X509_CRL(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_CRL(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_X509_REQ(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_REQ(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_X509_CERT_PAIR(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_X509_CERT_PAIR(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void my3_X509_STORE_CTX_set_verify_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->X509_STORE_CTX_set_verify_cb(ctx, find_verify_cb_Fct(cb));
}

EXPORT void my3_OPENSSL_sk_pop_free(x64emu_t* emu, void* s, void* cb)
{
    (void)emu;
    my->OPENSSL_sk_pop_free(s, find_free_fnc_Fct(cb));
}

EXPORT void* my3_OPENSSL_sk_new(x64emu_t* emu, void* f)
{
    return my->OPENSSL_sk_new(find_cmp_fnc_Fct(f));
}

EXPORT void my3_ERR_set_error(x64emu_t* emu, int lib, int reason, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    my->ERR_vset_error(lib, reason, fmt, VARARGS);
}
EXPORT void my3_ERR_vset_error(x64emu_t* emu, int lib, int reason, void* fmt, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    my->ERR_vset_error(lib, reason, fmt, VARARGS);
}

EXPORT int my3_BIO_printf(x64emu_t* emu, void* bio, void* fmt, uintptr_t* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->BIO_vprintf(bio, fmt, VARARGS);
}

EXPORT void* my3_PEM_read_DHparams(x64emu_t* emu, void* fp, void* x, void* cb, void* u)
{
    return my->PEM_read_DHparams(fp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void my3_EVP_MD_do_all_provided(x64emu_t* emu, void* ctx, void* cb, void* arg)
{
    my->EVP_MD_do_all_provided(ctx, find_do_all_provided_cb_Fct(cb), arg);
}

#define ALTMY my3_

#include "wrappedlib_init.h"
