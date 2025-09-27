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

typedef void*(*pFv_t)     ();
typedef int  (*iFppA_t)   (void*, void*, va_list);
typedef int  (*iFpip_t)   (void*, int, void*);
typedef void*(*pFppp_t)   (void*, void*, void*);

#define ADDED_FUNCTIONS()                   \
    GO(BIO_vprintf, iFppA_t);               \
    GO(i2t_ASN1_OBJECT, iFpip_t);           \
    GO(i2v_ASN1_BIT_STRING, pFppp_t);       \
    GO(i2v_GENERAL_NAME, pFppp_t);          \
    GO(i2v_GENERAL_NAMES, pFppp_t);         \
    GO(ASN1_BIT_STRING_it, pFv_t);          \
    GO(EXTENDED_KEY_USAGE_it, pFv_t);       \
    GO(ASN1_OCTET_STRING_it, pFv_t);        \
    GO(GENERAL_NAMES_it, pFv_t);            \
    GO(CERTIFICATEPOLICIES_it, pFv_t);      \
    GO(POLICYINFO_it, pFv_t);               \
    GO(CRL_DIST_POINTS_it, pFv_t);          \
    GO(ISSUING_DIST_POINT_it, pFv_t);       \

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

// copy_fnc
#define GO(A)   \
static uintptr_t my3_copy_fnc_fct_##A = 0;               \
static void my3_copy_fnc_##A(void* p)                    \
{                                                       \
    RunFunctionFmt(my3_copy_fnc_fct_##A, "p", p); \
}
SUPER()
#undef GO
static void* find_copy_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_copy_fnc_fct_##A == (uintptr_t)fct) return my3_copy_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_copy_fnc_fct_##A == 0) {my3_copy_fnc_fct_##A = (uintptr_t)fct; return my3_copy_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto copy_fnc callback\n");
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

// rsakeygen
#define GO(A)                                              \
static uintptr_t my3_rsakeygen_fct_##A = 0;                \
static void my3_rsakeygen_##A(int a, int b, void* c)       \
{                                                          \
    RunFunctionFmt(my3_rsakeygen_fct_##A, "iip", a, b, c); \
}
SUPER()
#undef GO
static void* find_rsakeygen_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my3_rsakeygen_fct_##A == (uintptr_t)fct) return my3_rsakeygen_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my3_rsakeygen_fct_##A == 0) {my3_rsakeygen_fct_##A = (uintptr_t)fct; return my3_rsakeygen_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto rsakeygen callback\n");
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

EXPORT void my3_PEM_read_bio_RSAPublicKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSAPublicKey(bp, x, find_passphrase_Fct(cb), u);
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

EXPORT void* my3_OPENSSL_sk_deep_copy(x64emu_t* emu, void* s, void* c, void* f)
{
    return my->OPENSSL_sk_deep_copy(s, find_copy_fnc_Fct(c), find_free_fnc_Fct(f));
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

EXPORT void my3_X509_STORE_set_verify_cb(x64emu_t* emu, void* ctx, void* cb)
{
    my->X509_STORE_set_verify_cb(ctx, find_verify_cb_Fct(cb));
}

EXPORT void* my3_PEM_read_bio_PrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    return my->PEM_read_bio_PrivateKey(bp,x, find_pem_password_cb_Fct(cb), u);
}

EXPORT int my3_PEM_write_bio_PrivateKey(x64emu_t* emu, void* bp, void* x, void* enc, void* kstr, int klen, void* cb, void* u)
{
    return my->PEM_write_bio_PrivateKey(bp, x, enc, kstr, klen, find_pem_password_cb_Fct(cb), u);
}

EXPORT int my3_PEM_write_bio_PrivateKey_traditional(x64emu_t* emu, void* bp, void* x, void* enc, void* kstr, int klen, void* cb, void* u)
{
    return my->PEM_write_bio_PrivateKey_traditional(bp, x, enc, kstr, klen, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    return my->PEM_read_bio_PUBKEY(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_PEM_read_bio_DHparams(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    return my->PEM_read_bio_DHparams(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my3_RSA_generate_key(x64emu_t* emu, int bits, unsigned long e, void* cb, void* cb_arg)
{
    return my->RSA_generate_key(bits, e, find_rsakeygen_Fct(cb), cb_arg);
}

typedef struct my_v3_ext_method_s {
    int ext_nid;
    int ext_flags;
    void* it;
    void* ext_new;
    void* ext_free;
    void* d2i;
    void* i2d;
    void* i2s;
    void* s2i;
    void* i2v;
    void* v2i;
    void* i2r;
    void* r2i;
} my_v3_ext_method_t;

EXPORT void* my3_X509V3_EXT_get(x64emu_t* emu, void* x)
{
    my_v3_ext_method_t* ret = my->X509V3_EXT_get(x);
    if(ret) {
        #define GO(A, W) if(ret->A) AddAutomaticBridge(my_lib->w.bridge, W, ret->A, 0, "v3_ext_method_" #A)
        GO(ext_new, pFv);
        GO(ext_free, vFp);
        GO(d2i, pFppl);
        GO(i2d, iFpp);
        GO(i2s, pFpp);
        GO(s2i, pFppp);
        GO(i2v, pFppp);
        GO(v2i, pFppp);
        GO(i2r, iFpppi);
        GO(r2i, pFppp);
        #undef GO
    }
    return ret;
}

#define ALTMY my3_

#define CUSTOM_INIT \
    AddAutomaticBridge(lib->w.bridge, iFpip, my->i2t_ASN1_OBJECT, 0, "i2t_ASN1_OBJECT");            \
    AddAutomaticBridge(lib->w.bridge, pFppp, my->i2v_ASN1_BIT_STRING, 0, "i2v_ASN1_BIT_STRING");    \
    AddAutomaticBridge(lib->w.bridge, pFppp, my->i2v_GENERAL_NAME, 0, "i2v_GENERAL_NAME");          \
    AddAutomaticBridge(lib->w.bridge, pFppp, my->i2v_GENERAL_NAMES, 0, "i2v_GENERAL_NAMES");        \
    AddAutomaticBridge(lib->w.bridge, pFv, my->ASN1_BIT_STRING_it, 0, "ASN1_BIT_STRING_it");        \
    AddAutomaticBridge(lib->w.bridge, pFv, my->EXTENDED_KEY_USAGE_it, 0, "EXTENDED_KEY_USAGE_it");  \
    AddAutomaticBridge(lib->w.bridge, pFv, my->ASN1_OCTET_STRING_it, 0, "ASN1_OCTET_STRING_it");    \
    AddAutomaticBridge(lib->w.bridge, pFv, my->GENERAL_NAMES_it, 0, "GENERAL_NAMES_it");            \
    AddAutomaticBridge(lib->w.bridge, pFv, my->CERTIFICATEPOLICIES_it, 0, "CERTIFICATEPOLICIES_it");\
    AddAutomaticBridge(lib->w.bridge, pFv, my->POLICYINFO_it, 0, "POLICYINFO_it");                  \
    AddAutomaticBridge(lib->w.bridge, pFv, my->CRL_DIST_POINTS_it, 0, "CRL_DIST_POINTS_it");        \
    AddAutomaticBridge(lib->w.bridge, pFv, my->ISSUING_DIST_POINT_it, 0, "ISSUING_DIST_POINT_it");  \

#include "wrappedlib_init.h"
