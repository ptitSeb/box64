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

// BIO_meth_set_write
#define GO(A)   \
static uintptr_t my_BIO_meth_set_write_fct_##A = 0;             \
static int my_BIO_meth_set_write_##A(void* a, void* b, int c)   \
{                                                               \
    return (int)RunFunctionFmt(my_BIO_meth_set_write_fct_##A, "ppi", a, b, c);  \
}
SUPER()
#undef GO
static void* find_BIO_meth_set_write_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_write_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_write_fct_##A == 0) {my_BIO_meth_set_write_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_write callback\n");
    return NULL;
}

// BIO_meth_set_read
#define GO(A)   \
static uintptr_t my_BIO_meth_set_read_fct_##A = 0;          \
static int my_BIO_meth_set_read_##A(void* a, void* b, int c)\
{                                                           \
    return (int)RunFunctionFmt(my_BIO_meth_set_read_fct_##A, "ppi", a, b, c);   \
}
SUPER()
#undef GO
static void* find_BIO_meth_set_read_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_read_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_read_fct_##A == 0) {my_BIO_meth_set_read_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_read callback\n");
    return NULL;
}

// BIO_meth_set_puts
#define GO(A)   \
static uintptr_t my_BIO_meth_set_puts_fct_##A = 0;      \
static int my_BIO_meth_set_puts_##A(void* a, void* b)   \
{                                                       \
    return (int)RunFunctionFmt(my_BIO_meth_set_puts_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_BIO_meth_set_puts_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_puts_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_puts_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_puts_fct_##A == 0) {my_BIO_meth_set_puts_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_puts_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_puts callback\n");
    return NULL;
}

// BIO_meth_set_gets
#define GO(A)   \
static uintptr_t my_BIO_meth_set_gets_fct_##A = 0;          \
static int my_BIO_meth_set_gets_##A(void* a, void* b, int c)\
{                                                           \
    return (int)RunFunctionFmt(my_BIO_meth_set_gets_fct_##A, "ppi", a, b, c);   \
}
SUPER()
#undef GO
static void* find_BIO_meth_set_gets_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_gets_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_gets_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_gets_fct_##A == 0) {my_BIO_meth_set_gets_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_gets_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_gets callback\n");
    return NULL;
}

// BIO_meth_set_ctrl
#define GO(A)   \
static uintptr_t my_BIO_meth_set_ctrl_fct_##A = 0;                      \
static long my_BIO_meth_set_ctrl_##A(void* a, int b, long c, void* d)   \
{                                                                       \
    return (long)RunFunctionFmt(my_BIO_meth_set_ctrl_fct_##A, "pilp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_BIO_meth_set_ctrl_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_ctrl_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_ctrl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_ctrl_fct_##A == 0) {my_BIO_meth_set_ctrl_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_ctrl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_ctrl callback\n");
    return NULL;
}

// BIO_meth_set_create
#define GO(A)   \
static uintptr_t my_BIO_meth_set_create_fct_##A = 0;\
static int my_BIO_meth_set_create_##A(void* a)      \
{                                                   \
    return (int)RunFunctionFmt(my_BIO_meth_set_create_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_BIO_meth_set_create_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_create_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_create_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_create_fct_##A == 0) {my_BIO_meth_set_create_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_create_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_create callback\n");
    return NULL;
}

// BIO_meth_set_destroy
#define GO(A)   \
static uintptr_t my_BIO_meth_set_destroy_fct_##A = 0;   \
static int my_BIO_meth_set_destroy_##A(void* a)         \
{                                                       \
    return (int)RunFunctionFmt(my_BIO_meth_set_destroy_fct_##A, "p", a);\
}
SUPER()
#undef GO
static void* find_BIO_meth_set_destroy_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_BIO_meth_set_destroy_fct_##A == (uintptr_t)fct) return my_BIO_meth_set_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_BIO_meth_set_destroy_fct_##A == 0) {my_BIO_meth_set_destroy_fct_##A = (uintptr_t)fct; return my_BIO_meth_set_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto BIO_meth_set_destroy callback\n");
    return NULL;
}

// ENGINE_ctrl_cb
#define GO(A)   \
static uintptr_t my_ENGINE_ctrl_cb_fct_##A = 0; \
static void my_ENGINE_ctrl_cb_##A()             \
{                                               \
    RunFunctionFmt(my_ENGINE_ctrl_cb_fct_##A, "");  \
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
    return (int)RunFunctionFmt(my_cmp_fnc_fct_##A, "pp", a, b);   \
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
    RunFunctionFmt(my_free_fnc_fct_##A, "p", p); \
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
    return (unsigned long)RunFunctionFmt(my_id_func_fct_##A, "");   \
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
    RunFunctionFmt(my_lock_func_fct_##A, "iipi", mode, n, f, l);    \
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
    return (int)RunFunctionFmt(my_passphrase_fct_##A, "piip", buff, size, rw, u);   \
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
    return (void*)RunFunctionFmt(my_xnew_fct_##A, "");  \
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
    return (void*)RunFunctionFmt(my_d2i_fct_##A, "");   \
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
    return (int)RunFunctionFmt(my_i2d_fct_##A, ""); \
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
    return (int)RunFunctionFmt(my_pem_password_cb_fct_##A, "piip", a, b, c, d); \
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

// ctx_verify_cb
#define GO(A)   \
static uintptr_t my_ctx_verify_cb_fct_##A = 0;                          \
static int my_ctx_verify_cb_##A(int a, void* b)                         \
{                                                                       \
    return (int)RunFunctionFmt(my_ctx_verify_cb_fct_##A, "ip", a, b);   \
}
SUPER()
#undef GO
static void* find_ctx_verify_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ctx_verify_cb_fct_##A == (uintptr_t)fct) return my_ctx_verify_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ctx_verify_cb_fct_##A == 0) {my_ctx_verify_cb_fct_##A = (uintptr_t)fct; return my_ctx_verify_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto ctx_verify_cb callback\n");
    return NULL;
}


// verify_cb
#define GO(A)   \
static uintptr_t my_verify_cb_fct_##A = 0;                              \
static int my_verify_cb_##A(int a, void* b)                             \
{                                                                       \
    return (int)RunFunctionFmt(my_verify_cb_fct_##A, "ip", a, b); \
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

EXPORT int32_t my_BIO_meth_set_write(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_write(biom, find_BIO_meth_set_write_Fct(cb));
}
EXPORT int32_t my_BIO_meth_set_read(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_read(biom, find_BIO_meth_set_read_Fct(cb));
}
EXPORT int32_t my_BIO_meth_set_puts(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_puts(biom, find_BIO_meth_set_puts_Fct(cb));
}
EXPORT int32_t my_BIO_meth_set_gets(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_gets(biom, find_BIO_meth_set_gets_Fct(cb));
}
EXPORT int32_t my_BIO_meth_set_ctrl(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_ctrl(biom, find_BIO_meth_set_ctrl_Fct(cb));
}
EXPORT int32_t my_BIO_meth_set_create(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_create(biom, find_BIO_meth_set_create_Fct(cb));
}
EXPORT int32_t my_BIO_meth_set_destroy(x64emu_t* emu, void* biom, void* cb)
{
    (void)emu;
    return my->BIO_meth_set_destroy(biom, find_BIO_meth_set_destroy_Fct(cb));
}

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


EXPORT int my_PEM_write_bio_PrivateKey(x64emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_PrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
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

EXPORT void* my_PEM_read_bio_PrivateKey(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_PrivateKey(bp, x, find_pem_password_cb_Fct(cb), u);
}


EXPORT void* my_PEM_read_bio_PUBKEY(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_PUBKEY(bp, x, find_pem_password_cb_Fct(cb), u);
}

EXPORT void* my_PEM_read_bio_DHparams(x64emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    return my->PEM_read_bio_DHparams(bp, x, find_pem_password_cb_Fct(cb), u);
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
    my->X509_STORE_CTX_set_verify_cb(ctx, find_ctx_verify_cb_Fct(cb));
}


EXPORT void my_X509_STORE_set_verify_cb(x64emu_t* emu, void* ctx, void* cb)
{
    (void)emu;
    my->X509_STORE_set_verify_cb(ctx, find_verify_cb_Fct(cb));
}

EXPORT void my_OPENSSL_sk_pop_free(x64emu_t* emu, void* s, void* cb)
{
    (void)emu;
    my->OPENSSL_sk_pop_free(s, find_free_fnc_Fct(cb));
}

#include "wrappedlib_init.h"
