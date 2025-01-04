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
#include "myalign.h"

const char* libnettle8Name = "libnettle.so.8";
#define ALTNAME "libnettle.so"
#define LIBNAME libnettle8

#define ADDED_FUNCTIONS()

#include "generated/wrappedlibnettle8types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \
GO(10)  \

// nettle_cipher_func
#define GO(A)   \
static uintptr_t my_nettle_cipher_func_fct_##A = 0;                         \
static void my_nettle_cipher_func_##A(void* a, size_t b, void* c, void* d)  \
{                                                                           \
    RunFunctionFmt(my_nettle_cipher_func_fct_##A, "pLpp", a, b, c, d);      \
}
SUPER()
#undef GO
static void* findnettle_cipher_funcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_cipher_func_fct_##A == (uintptr_t)fct) return my_nettle_cipher_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_cipher_func_fct_##A == 0) {my_nettle_cipher_func_fct_##A = (uintptr_t)fct; return my_nettle_cipher_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libnettle.so.8 nettle_cipher_func callback\n");
    return NULL;
}

// nettle_hash_update_func
#define GO(A)   \
static uintptr_t my_nettle_hash_update_func_fct_##A = 0;                \
static void my_nettle_hash_update_func_##A(void* a, size_t b, void* c)  \
{                                                                       \
    RunFunctionFmt(my_nettle_hash_update_func_fct_##A, "pLp", a, b, c); \
}
SUPER()
#undef GO
static void* findnettle_hash_update_funcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_hash_update_func_fct_##A == (uintptr_t)fct) return my_nettle_hash_update_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_hash_update_func_fct_##A == 0) {my_nettle_hash_update_func_fct_##A = (uintptr_t)fct; return my_nettle_hash_update_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libnettle.so.8 nettle_hash_update_func callback\n");
    return NULL;
}

// nettle_hash_digest_func
#define GO(A)   \
static uintptr_t my_nettle_hash_digest_func_fct_##A = 0;                \
static void my_nettle_hash_digest_func_##A(void* a, size_t b, void* c)  \
{                                                                       \
    RunFunctionFmt(my_nettle_hash_digest_func_fct_##A, "pLp", a, b, c); \
}
SUPER()
#undef GO
static void* findnettle_hash_digest_funcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_hash_digest_func_fct_##A == (uintptr_t)fct) return my_nettle_hash_digest_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_hash_digest_func_fct_##A == 0) {my_nettle_hash_digest_func_fct_##A = (uintptr_t)fct; return my_nettle_hash_digest_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libnettle.so.8 nettle_hash_digest_func callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_nettle_ccm_encrypt_message(x64emu_t* emu, void* cipher, void* f, size_t nlen, void* nonce, size_t alen, void* adata, size_t tlen, size_t clen, void* dst, void* src)
{
    my->nettle_ccm_encrypt_message(cipher, findnettle_cipher_funcFct(f), nlen, nonce, alen, adata, tlen, clen, dst, src);
}

EXPORT void my_nettle_cbc_decrypt(x64emu_t* emu, void* ctx, void* f, size_t bsize, void* iv, size_t len, void* dst, void* src)
{
    my->nettle_cbc_decrypt(ctx, findnettle_cipher_funcFct(f), bsize, iv, len, dst, src);
}

EXPORT void my_nettle_cbc_encrypt(x64emu_t* emu, void* ctx, void* f, size_t bsize, void* iv, size_t len, void* dst, void* src)
{
    my->nettle_cbc_encrypt(ctx, findnettle_cipher_funcFct(f), bsize, iv, len, dst, src);
}

EXPORT int my_nettle_ccm_decrypt_message(x64emu_t* emu, void* cipher, void* f, size_t nlen, void* nonce, size_t alen, void* adata, size_t tlen, size_t mlen, void* dst, void* src)
{
    return my->nettle_ccm_decrypt_message(cipher, findnettle_cipher_funcFct(f), nlen, nonce, alen, adata, tlen, mlen, dst, src);
}

EXPORT void my_nettle_cfb_decrypt(x64emu_t* emu, void* cipher, void* f, size_t blen, void* iv, size_t len, void* dst, void* src)
{
    my->nettle_cfb_decrypt(cipher, findnettle_cipher_funcFct(f), blen, iv, len, dst, src);
}

EXPORT void my_nettle_cfb_encrypt(x64emu_t* emu, void* cipher, void* f, size_t blen, void* iv, size_t len, void* dst, void* src)
{
    my->nettle_cfb_encrypt(cipher, findnettle_cipher_funcFct(f), blen, iv, len, dst, src);
}

EXPORT void my_nettle_cfb8_decrypt(x64emu_t* emu, void* cipher, void* f, size_t blen, void* iv, size_t len, void* dst, void* src)
{
    my->nettle_cfb8_decrypt(cipher, findnettle_cipher_funcFct(f), blen, iv, len, dst, src);
}

EXPORT void my_nettle_cfb8_encrypt(x64emu_t* emu, void* cipher, void* f, size_t blen, void* iv, size_t len, void* dst, void* src)
{
    my->nettle_cfb8_encrypt(cipher, findnettle_cipher_funcFct(f), blen, iv, len, dst, src);
}

EXPORT void my_nettle_cmac128_digest(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, uint32_t len, void* digest)
{
    my->nettle_cmac128_digest(ctx, key, cipher, findnettle_cipher_funcFct(f), len, digest);
}

EXPORT void my_nettle_cmac128_set_key(x64emu_t* emu, void* key, void* cipher, void* f)
{
    my->nettle_cmac128_set_key(key, cipher, findnettle_cipher_funcFct(f));
}

EXPORT void my_nettle_cmac128_update(x64emu_t* emu, void* ctx, void* cipher, void* f, size_t len, void* msg)
{
    my->nettle_cmac128_update(ctx, cipher, findnettle_cipher_funcFct(f), len, msg);
}

EXPORT void my_nettle_cmac64_digest(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, uint32_t len, void* digest)
{
    my->nettle_cmac64_digest(ctx, key, cipher, findnettle_cipher_funcFct(f), len, digest);
}

EXPORT void my_nettle_cmac64_set_key(x64emu_t* emu, void* key, void* cipher, void* f)
{
    my->nettle_cmac64_set_key(key, cipher, findnettle_cipher_funcFct(f));
}

EXPORT void my_nettle_cmac64_update(x64emu_t* emu, void* ctx, void* cipher, void* f, size_t len, void* msg)
{
    my->nettle_cmac64_update(ctx, cipher, findnettle_cipher_funcFct(f), len, msg);
}

EXPORT void my_nettle_ctr_crypt(x64emu_t* emu, void* ctx, void* f, size_t bsize, void* ctr, size_t len, void* dst, void* src)
{
    my->nettle_ctr_crypt(ctx, findnettle_cipher_funcFct(f), bsize, ctr, len, dst, src);
}

EXPORT void my_nettle_gcm_decrypt(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, size_t len, void* dst, void* src)
{
    my->nettle_gcm_decrypt(ctx, key, cipher, findnettle_cipher_funcFct(f), len, dst, src);
}

EXPORT void my_nettle_gcm_digest(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, size_t len, void* digest)
{
    my->nettle_gcm_digest(ctx, key, cipher, findnettle_cipher_funcFct(f), len, digest);
}

EXPORT void my_nettle_gcm_encrypt(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, size_t len, void* dst, void* src)
{
    my->nettle_gcm_encrypt(ctx, key, cipher, findnettle_cipher_funcFct(f), len, dst, src);
}

EXPORT void my_nettle_gcm_set_key(x64emu_t* emu, void* key, void* cipher, void* f)
{
    my->nettle_gcm_set_key(key, cipher, findnettle_cipher_funcFct(f));
}

EXPORT void my_nettle_hkdf_expand(x64emu_t* emu, void* ctx, void* update, void* digest, size_t dsize, size_t isize, void* info, size_t len, void* dst)
{
    my->nettle_hkdf_expand(ctx, findnettle_hash_update_funcFct(update), findnettle_hash_digest_funcFct(digest), dsize, isize, info, len, dst);
}

EXPORT void my_nettle_hkdf_extract(x64emu_t* emu, void* ctx, void* update, void* digest, size_t dsize, size_t ssz, void* secret, void* dst)
{
    my->nettle_hkdf_extract(ctx, findnettle_hash_update_funcFct(update), findnettle_hash_digest_funcFct(digest), dsize, ssz, secret, dst);
}

EXPORT void my_nettle_pbkdf2(x64emu_t* emu, void* ctx, void* update, void* digest, size_t dsize, uint32_t iter, size_t ssalt, void* salt, size_t len, void* dst)
{
    my->nettle_pbkdf2(ctx, findnettle_hash_update_funcFct(update), findnettle_hash_digest_funcFct(digest), dsize, iter, ssalt, salt, len, dst);
}

#include "wrappedlib_init.h"
