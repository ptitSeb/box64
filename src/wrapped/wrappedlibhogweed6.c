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

const char* libhogweed6Name = "libhogweed.so.6";
#define LIBNAME libhogweed6


#define ADDED_FUNCTIONS()

#include "generated/wrappedlibhogweed6types.h"

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

// nettle_random_func
#define GO(A)   \
static uintptr_t my_nettle_random_func_fct_##A = 0;                 \
static void my_nettle_random_func_##A(void* a, size_t b, void* c)   \
{                                                                   \
    RunFunctionFmt(my_nettle_random_func_fct_##A, "pLp", a, b, c);  \
}
SUPER()
#undef GO
static void* findnettle_random_funcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_random_func_fct_##A == (uintptr_t)fct) return my_nettle_random_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_random_func_fct_##A == 0) {my_nettle_random_func_fct_##A = (uintptr_t)fct; return my_nettle_random_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libhogweed.so.6 nettle_random_func callback\n");
    return NULL;
}

// nettle_progress_func
#define GO(A)   \
static uintptr_t my_nettle_progress_func_fct_##A = 0;               \
static void my_nettle_progress_func_##A(void* a, int b)             \
{                                                                   \
    RunFunctionFmt(my_nettle_progress_func_fct_##A, "pi", a, b);    \
}
SUPER()
#undef GO
static void* findnettle_progress_funcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_nettle_progress_func_fct_##A == (uintptr_t)fct) return my_nettle_progress_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nettle_progress_func_fct_##A == 0) {my_nettle_progress_func_fct_##A = (uintptr_t)fct; return my_nettle_progress_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libhogweed.so.6 nettle_progress_func callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_nettle_dsa_generate_params(x64emu_t* emu, void* params, void* rnd_ctx, void* rnd, void* progress_ctx, void* progress, uint32_t p_bits, uint32_t q_bits)
{
    return my->nettle_dsa_generate_params(params, rnd_ctx, findnettle_random_funcFct(rnd), progress_ctx, findnettle_progress_funcFct(progress), p_bits, q_bits);
}

EXPORT int my_nettle_dsa_sign(x64emu_t* emu, void* params, void* x, void* rnd_ctx, void* rnd, size_t len, void* digest, void* sign)
{
    return my->nettle_dsa_sign(params, x, rnd_ctx, findnettle_random_funcFct(rnd), len, digest, sign);
}

EXPORT void my_nettle_ecdsa_generate_keypair(x64emu_t* emu, void* pub, void* key, void* ctx, void* f)
{
    my->nettle_ecdsa_generate_keypair(pub, key, ctx, findnettle_random_funcFct(f));
}

EXPORT void my_nettle_ecdsa_sign(x64emu_t* emu, void* key, void* ctx, void* f, size_t len, void* digest, void* sign)
{
    my->nettle_ecdsa_sign(key, ctx, findnettle_random_funcFct(f), len, digest, sign);
}

EXPORT void my_nettle_gostdsa_sign(x64emu_t* emu, void* key, void* ctx, void* f, size_t len, void* digest, void* sign)
{
    my->nettle_gostdsa_sign(key, ctx, findnettle_random_funcFct(f), len, digest, sign);
}

EXPORT void my_nettle_mpz_random(x64emu_t* emu, void* x, void* ctx, void* f, void* n)
{
    my->nettle_mpz_random(x, ctx, findnettle_random_funcFct(f), n);
}

EXPORT void my_nettle_mpz_random_size(x64emu_t* emu, void* x, void* ctx, void* f, uint32_t n)
{
    my->nettle_mpz_random_size(x, ctx, findnettle_random_funcFct(f), n);
}

EXPORT int my_nettle_rsa_decrypt_tr(x64emu_t* emu, void* pub, void* key, void* ctx, void* f, void* len, void* msg, void* gib)
{
    return my->nettle_rsa_decrypt_tr(pub, key, ctx, findnettle_random_funcFct(f), len, msg, gib);
}

EXPORT int my_nettle_rsa_encrypt(x64emu_t* emu, void* key, void* ctx, void* f, size_t len, void* txt, void* cipher)
{
    return my->nettle_rsa_encrypt(key, ctx, findnettle_random_funcFct(f), len, txt, cipher);
}

EXPORT int my_nettle_rsa_generate_keypair(x64emu_t* emu, void* pub, void* key, void* ctx_rnd, void* rnd, void* ctx_progress, void* progress, uint32_t n_size, uint32_t e_size)
{
    return my->nettle_rsa_generate_keypair(pub, key, ctx_rnd, findnettle_random_funcFct(rnd), ctx_progress, findnettle_progress_funcFct(progress), n_size, e_size);
}

EXPORT int my_nettle_rsa_pkcs1_sign_tr(x64emu_t* emu, void* pub, void* key, void* ctx, void* f, size_t len, void* info, void* s)
{
    return my->nettle_rsa_pkcs1_sign_tr(pub, key, ctx, findnettle_random_funcFct(f), len, info, s);
}

EXPORT int my_nettle_rsa_pss_sha256_sign_digest_tr(x64emu_t* emu, void* pub, void* key, void* ctx, void* f, size_t len, void* salt, void* dig, void* s)
{
    return my->nettle_rsa_pss_sha256_sign_digest_tr(pub, key, ctx, findnettle_random_funcFct(f), len, salt, dig, s);
}

EXPORT int my_nettle_rsa_pss_sha384_sign_digest_tr(x64emu_t* emu, void* pub, void* key, void* ctx, void* f, size_t len, void* salt, void* dig, void* s)
{
    return my->nettle_rsa_pss_sha384_sign_digest_tr(pub, key, ctx, findnettle_random_funcFct(f), len, salt, dig, s);
}

EXPORT int my_nettle_rsa_pss_sha512_sign_digest_tr(x64emu_t* emu, void* pub, void* key, void* ctx, void* f, size_t len, void* salt, void* dig, void* s)
{
    return my->nettle_rsa_pss_sha512_sign_digest_tr(pub, key, ctx, findnettle_random_funcFct(f), len, salt, dig, s);
}

EXPORT int my_nettle_rsa_sec_decrypt(x64emu_t* emu, void* pub, void* key, void* ctx, void* f, size_t len, void* msg, void* s)
{
    return my->nettle_rsa_sec_decrypt(pub, key, ctx, findnettle_random_funcFct(f), len, msg, s);
}

#define NEEDED_LIBS "libnettle.so.8"

#include "wrappedlib_init.h"
