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

typedef void(*vFp_t)(void*);
typedef void(*vFpp_t)(void*, void*);
typedef void(*vFpLp_t)(void*, size_t, void*);
typedef void(*vFpLpp_t)(void*, size_t, void*, void*);

typedef struct my_nettle_hash_s
{
  const char*   name;
  unsigned      context_size;
  unsigned      digest_size;
  unsigned      block_size;
  vFp_t         init;
  vFpLp_t       update;
  vFpLp_t       digest;
} my_nettle_hash_t;

typedef struct my_nettle_cipher_s
{
  const char*   name;
  unsigned      context_size;
  unsigned      block_size;
  unsigned      key_size;
  vFpp_t        set_encrypt_key;
  vFpp_t        set_decrypt_key;
  vFpLpp_t      encrypt;
  vFpLpp_t      decrypt;
} my_nettle_cipher_t;


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

// nettle_cipher_set
#define GO(A)   \
static uintptr_t my_nettle_cipher_set_fct_##A = 0;              \
static void my_nettle_cipher_set_##A(void* a, void* b)          \
{                                                               \
    RunFunctionFmt(my_nettle_cipher_set_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* findnettle_cipher_setFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_cipher_set_fct_##A == (uintptr_t)fct) return my_nettle_cipher_set_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_cipher_set_fct_##A == 0) {my_nettle_cipher_set_fct_##A = (uintptr_t)fct; return my_nettle_cipher_set_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libnettle.so.8 nettle_cipher_set callback\n");
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
// nettle_hash_init
#define GO(A)   \
static uintptr_t my_nettle_hash_init_fct_##A = 0;                   \
static void my_nettle_hash_init_##A(void* a)                        \
{                                                                   \
    RunFunctionFmt(my_nettle_hash_init_fct_##A, "p", a);            \
}
SUPER()
#undef GO
static void* findnettle_hash_initFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_hash_init_fct_##A == (uintptr_t)fct) return my_nettle_hash_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_hash_init_fct_##A == 0) {my_nettle_hash_init_fct_##A = (uintptr_t)fct; return my_nettle_hash_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libnettle.so.8 nettle_hash_init callback\n");
    return NULL;
}


#undef SUPER

my_nettle_hash_t* Wrap_nettle_hash(my_nettle_hash_t* d, my_nettle_hash_t* s)
{
    memcpy(d, s, sizeof(my_nettle_hash_t));
    d->init = findnettle_hash_initFct(s->init);
    d->update = findnettle_hash_update_funcFct(s->update);
    d->digest = findnettle_hash_digest_funcFct(s->digest);
    return d;
}
my_nettle_cipher_t* Wrap_nettle_cipher(my_nettle_cipher_t* d, my_nettle_cipher_t* s)
{
    memcpy(d, s, sizeof(my_nettle_cipher_t));
    d->set_decrypt_key = findnettle_cipher_setFct(s->set_decrypt_key);
    d->set_encrypt_key = findnettle_cipher_setFct(s->set_encrypt_key);
    d->encrypt = findnettle_cipher_funcFct(s->encrypt);
    d->decrypt = findnettle_cipher_funcFct(s->decrypt);
    return d;
}

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

EXPORT void my_nettle_hmac_set_key(x64emu_t* emu, void* outer, void* inner, void* state, my_nettle_hash_t* hash, size_t l, void* key)
{
    my_nettle_hash_t save = {0};
    my->nettle_hmac_set_key(outer, inner, state, Wrap_nettle_hash(&save, hash), l, key);
}

EXPORT void my_nettle_hmac_update(x64emu_t* emu, void* state, my_nettle_hash_t* hash, size_t l, void* key)
{
    my_nettle_hash_t save = {0};
    my->nettle_hmac_update(state, Wrap_nettle_hash(&save, hash), l, key);
}

EXPORT void my_nettle_hmac_digest(x64emu_t* emu, void* outer, void* inner, void* state, my_nettle_hash_t* hash, size_t l, void* key)
{
    my_nettle_hash_t save = {0};
    my->nettle_hmac_digest(outer, inner, state, Wrap_nettle_hash(&save, hash), l, key);
}

EXPORT void my_nettle_eax_decrypt(x64emu_t* emu, void* eax, void* key, void* cipher, void* f, unsigned long len, void* dst, void* src)
{
    my->nettle_eax_decrypt(eax, key, cipher, findnettle_cipher_funcFct(f), len, dst, src);
}

EXPORT void my_nettle_eax_digest(x64emu_t* emu, void* eax, void* key, void* cipher, void* f, unsigned long len, void* digest)
{
    my->nettle_eax_digest(eax, key, cipher, findnettle_cipher_funcFct(f), len, digest);
}

EXPORT void my_nettle_eax_encrypt(x64emu_t* emu, void* eax, void* key, void* cipher, void* f, unsigned long len, void* dst, void* src)
{
    my->nettle_eax_encrypt(eax, key, cipher, findnettle_cipher_funcFct(f), len, dst, src);
}


EXPORT void my_nettle_eax_set_key(x64emu_t* emu, void* key, void* cipher, void* f)
{
    my->nettle_eax_set_key(key, cipher, findnettle_cipher_funcFct(f));
}

EXPORT void my_nettle_eax_set_nonce(x64emu_t* emu, void* eax, void* key, void* cipher)
{

}

EXPORT void my_nettle_eax_update(x64emu_t* emu, void* eax, void* key, void* cipher, void* f, unsigned long len, void* data)
{
    my->nettle_eax_update(eax, key, cipher, findnettle_cipher_funcFct(f), len, data);
}

EXPORT void my_nettle_ocb_decrypt(x64emu_t* emu, void* ctx, void* key, void* encrypt_ctx, void* encrypt_f, void* decrypt_ctx, void* decrypt_f, unsigned long len, void* dst, void* src)
{
    my->nettle_ocb_decrypt(ctx, key, encrypt_ctx, findnettle_cipher_funcFct(encrypt_f), decrypt_ctx, findnettle_cipher_funcFct(decrypt_f), len, dst, src);
}

EXPORT void my_nettle_ocb_digest(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, unsigned long len, void* digest)
{
    my->nettle_ocb_digest(ctx, key, cipher, findnettle_cipher_funcFct(f), len, digest);
}

EXPORT void my_nettle_ocb_encrypt(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, unsigned long len, void* dst, void* src)
{
    my->nettle_ocb_encrypt(ctx, key, cipher, findnettle_cipher_funcFct(f), len, dst, src);
}

EXPORT void my_nettle_ocb_set_key(x64emu_t* emu, void* key, void* cipher, void* f)
{
    my->nettle_ocb_set_key(key, cipher, findnettle_cipher_funcFct(f));
}

EXPORT void my_nettle_ocb_set_nonce(x64emu_t* emu, void* ctx, void* cipher, void* f, unsigned long len1, unsigned long len2, void* nonce)
{
    my->nettle_ocb_set_nonce(ctx, cipher, findnettle_cipher_funcFct(f), len1, len2, nonce);
}

EXPORT void my_nettle_ocb_update(x64emu_t* emu, void* ctx, void* key, void* cipher, void* f, unsigned long len, void* data)
{
    my->nettle_ocb_update(ctx, key, cipher, findnettle_cipher_funcFct(f), len, data);
}

#include "wrappedlib_init.h"
