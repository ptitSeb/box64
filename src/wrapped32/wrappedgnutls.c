#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "converter32.h"

static const char* gnutlsName = "libgnutls.so.30";
#define ALTNAME "libgnutls.so"

#define LIBNAME gnutls

#include "generated/wrappedgnutlstypes32.h"

#include "wrappercallback32.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// gnutls_log
#define GO(A)   \
static uintptr_t my_gnutls_log_fct_##A = 0;                            \
static void my_gnutls_log_##A(int level, const char* p)                \
{                                                                      \
    RunFunctionFmt(my_gnutls_log_fct_##A, "ip", level, p);        \
}
SUPER()
#undef GO
static void* find_gnutls_log_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_gnutls_log_fct_##A == (uintptr_t)fct) return my_gnutls_log_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_gnutls_log_fct_##A == 0) {my_gnutls_log_fct_##A = (uintptr_t)fct; return my_gnutls_log_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgnutls.so.30 gnutls_log callback\n");
    return NULL;
}

// pullpush
#define GO(A)   \
static uintptr_t my_pullpush_fct_##A = 0;                                         \
static long my_pullpush_##A(void* p, void* d, size_t l)                           \
{                                                                                 \
    long_t ret = (long_t)RunFunctionFmt(my_pullpush_fct_##A, "ppL", p, d, l);     \
    return from_long(ret);\
}
SUPER()
#undef GO
static void* find_pullpush_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_pullpush_fct_##A == (uintptr_t)fct) return my_pullpush_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pullpush_fct_##A == 0) {my_pullpush_fct_##A = (uintptr_t)fct; return my_pullpush_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgnutls.so.30 pullpush callback\n");
    return NULL;
}

// timeout
#define GO(A)   \
static uintptr_t my_timeout_fct_##A = 0;                                    \
static int my_timeout_##A(void* p, uint32_t t)                              \
{                                                                           \
    return (int)RunFunctionFmt(my_timeout_fct_##A, "pu", p, t);       \
}
SUPER()
#undef GO
static void* find_timeout_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_timeout_fct_##A == (uintptr_t)fct) return my_timeout_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_timeout_fct_##A == 0) {my_timeout_fct_##A = (uintptr_t)fct; return my_timeout_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libgnutls.so.30 timeout callback\n");
    return NULL;
}

#undef SUPER

typedef struct datum_32_s {
    ptr_t       data;
    uint32_t    size;
} datum_32_t;

EXPORT void* my32_gnutls_certificate_get_peers(x64emu_t* emu, void* session, int* size)
{
    int num = 0;
    struct_pu_t* ret = my->gnutls_certificate_get_peers(session, &num);
    if(size) *size = num;
    if(!ret) return NULL;
    static datum_32_t res[128];
    if (num > 128) {
        printf_log(LOG_NONE, "Warning, return buffer for gnutls_certificate_get_peers too small: %d\n", num);
        num = 128;
    }
    for(int i=0; i<num; ++i) {
        res[i].data = to_ptrv(ret[i].p0);
        res[i].size = ret[i].u1;
    }
    return res;
}

EXPORT void my32_gnutls_global_set_log_function(x64emu_t* emu, void* f)
{
    (void)emu;
    my->gnutls_global_set_log_function(find_gnutls_log_Fct(f));
}

EXPORT void my32_gnutls_transport_set_pull_function(x64emu_t* emu, void* session, void* f)
{
    (void)emu;
    my->gnutls_transport_set_pull_function(session, find_pullpush_Fct(f));
}
EXPORT void my32_gnutls_transport_set_push_function(x64emu_t* emu, void* session, void* f)
{
    (void)emu;
    my->gnutls_transport_set_push_function(session, find_pullpush_Fct(f));
}

EXPORT void my32_gnutls_transport_set_pull_timeout_function(x64emu_t* emu, void* session, void* f)
{
    (void)emu;
    my->gnutls_transport_set_pull_timeout_function(session, find_timeout_Fct(f));
}

EXPORT int my32_gnutls_pkcs12_simple_parse(x64emu_t* emu, void* p12, void* pwd, ptr_t* key, ptr_t* chain, uint32_t* chain_len, ptr_t* extra_certs, uint32_t* extra_certs_len, ptr_t* crtl, uint32_t flags)
{
    void* key_l = key?from_ptrv(*key):NULL;
    void* chain_l = chain?from_ptrv(*chain):NULL;
    void* extra_certs_l = extra_certs?from_ptrv(*extra_certs):NULL;
    void* crtl_l = crtl?from_ptrv(*crtl):NULL;
    int ret = my->gnutls_pkcs12_simple_parse(p12, pwd, key?(&key_l):NULL, chain?(&chain_l):NULL, chain_len, extra_certs?(&extra_certs_l):NULL, extra_certs_len, crtl?(&crtl_l):NULL, flags);
    if(ret>=0) {
        if(key) *key = to_ptrv(key_l);
        if(chain) *chain = to_ptrv(chain_l);
        if(extra_certs) *extra_certs = to_ptrv(extra_certs_l);
        if(crtl) *crtl = to_ptrv(crtl_l);
    }
    return ret;
}

#include "wrappedlib_init32.h"
