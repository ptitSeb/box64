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

const char* nss3Name = "libnss3.so";
#define LIBNAME nss3

#include "generated/wrappednss3types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// PK11PasswordFunc ...
#define GO(A)   \
static uintptr_t my_PK11PasswordFunc_fct_##A = 0;                                   \
static void* my_PK11PasswordFunc_##A(void* a, int b, void* c)                       \
{                                                                                   \
    return (void*)RunFunctionFmt(my_PK11PasswordFunc_fct_##A, "pip", a, b, c); \
}
SUPER()
#undef GO
static void* find_PK11PasswordFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PK11PasswordFunc_fct_##A == (uintptr_t)fct) return my_PK11PasswordFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PK11PasswordFunc_fct_##A == 0) {my_PK11PasswordFunc_fct_##A = (uintptr_t)fct; return my_PK11PasswordFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nss3 PK11PasswordFunc callback\n");
    return NULL;
}

// CERT_StringFromCertFcn ...
#define GO(A)   \
static uintptr_t my_CERT_StringFromCertFcn_fct_##A = 0;                             \
static void* my_CERT_StringFromCertFcn_##A(void* a)                                 \
{                                                                                   \
    return (void*)RunFunctionFmt(my_CERT_StringFromCertFcn_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_CERT_StringFromCertFcn_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CERT_StringFromCertFcn_fct_##A == (uintptr_t)fct) return my_CERT_StringFromCertFcn_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CERT_StringFromCertFcn_fct_##A == 0) {my_CERT_StringFromCertFcn_fct_##A = (uintptr_t)fct; return my_CERT_StringFromCertFcn_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nss3 CERT_StringFromCertFcn callback\n");
    return NULL;
}
static void* reverse_CERT_StringFromCertFcn_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_CERT_StringFromCertFcn_##A == fct) return (void*)my_CERT_StringFromCertFcn_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, pFp, fct, 0, NULL);
}
// CERTChainVerifyCallbackFunc ...
#define GO(A)   \
static uintptr_t my_CERTChainVerifyCallbackFunc_fct_##A = 0;                                    \
static int my_CERTChainVerifyCallbackFunc_##A(void* a, void* b, void* c)                        \
{                                                                                               \
    return (int)RunFunctionFmt(my_CERTChainVerifyCallbackFunc_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* find_CERTChainVerifyCallbackFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CERTChainVerifyCallbackFunc_fct_##A == (uintptr_t)fct) return my_CERTChainVerifyCallbackFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CERTChainVerifyCallbackFunc_fct_##A == 0) {my_CERTChainVerifyCallbackFunc_fct_##A = (uintptr_t)fct; return my_CERTChainVerifyCallbackFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nss3 CERTChainVerifyCallbackFunc callback\n");
    return NULL;
}
static void* reverse_CERTChainVerifyCallbackFunc_Fct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_CERTChainVerifyCallbackFunc_##A == fct) return (void*)my_CERTChainVerifyCallbackFunc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, iFppp, fct, 0, NULL);
}
// PORTCharConversionWSwapFunc ...
#define GO(A)   \
static uintptr_t my_PORTCharConversionWSwapFunc_fct_##A = 0;                                                    \
static int my_PORTCharConversionWSwapFunc_##A(int a, void* b, uint32_t c, void* d, uint32_t e, void* f, int g)  \
{                                                                                                               \
    return (int)RunFunctionFmt(my_PORTCharConversionWSwapFunc_fct_##A, "ipupupi", a, b, c, d, e, f, g);        \
}
SUPER()
#undef GO
static void* find_PORTCharConversionWSwapFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PORTCharConversionWSwapFunc_fct_##A == (uintptr_t)fct) return my_PORTCharConversionWSwapFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PORTCharConversionWSwapFunc_fct_##A == 0) {my_PORTCharConversionWSwapFunc_fct_##A = (uintptr_t)fct; return my_PORTCharConversionWSwapFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nss3 PORTCharConversionWSwapFunc callback\n");
    return NULL;
}
// NSS_ShutdownFunc ...
#define GO(A)   \
static uintptr_t my_NSS_ShutdownFunc_fct_##A = 0;                           \
static int my_NSS_ShutdownFunc_##A(void* a, void* b)                        \
{                                                                           \
    return (int)RunFunctionFmt(my_NSS_ShutdownFunc_fct_##A, "pp", a, b);    \
}
SUPER()
#undef GO
static void* find_NSS_ShutdownFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_NSS_ShutdownFunc_fct_##A == (uintptr_t)fct) return my_NSS_ShutdownFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_NSS_ShutdownFunc_fct_##A == 0) {my_NSS_ShutdownFunc_fct_##A = (uintptr_t)fct; return my_NSS_ShutdownFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nss3 NSS_ShutdownFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_PK11_SetPasswordFunc(x64emu_t* emu, void* f)
{
    my->PK11_SetPasswordFunc(find_PK11PasswordFunc_Fct(f));
}

EXPORT int my_CERT_RegisterAlternateOCSPAIAInfoCallBack(x64emu_t* emu, void* f, void** old)
{
    int ret = my->CERT_RegisterAlternateOCSPAIAInfoCallBack(find_CERT_StringFromCertFcn_Fct(f), old);
    if(old)
        *old = reverse_CERT_StringFromCertFcn_Fct(my_lib, *old);
    return ret;
}

typedef int (*my_CERTChainVerifyCallbackFunc)(void *isChainValidArg, void *currentChain, void* chainOK);
typedef struct my_CERTChainVerifyCallback_s {
    my_CERTChainVerifyCallbackFunc isChainValid;
    void *isChainValidArg;
} my_CERTChainVerifyCallback_t;

typedef struct my_CERTValParamInValueStr_t {
    union {
        uint64_t ul;
    } scalar;
    union {
        void*    p;
        my_CERTChainVerifyCallback_t *chainVerifyCallback;
    } pointer;
    union {
        uint64_t *pul;
    } array;
    int arraySize;
} my_CERTValParamInValue_t;

typedef struct my_CERTValInParam_s {
    int type;
    my_CERTValParamInValue_t value;
} my_CERTValInParam_t;

EXPORT int my_CERT_PKIXVerifyCert(x64emu_t* emu, void* cert, int64_t usages, my_CERTValInParam_t* pin, void* pout, void* wincx)
{
    //cert_pi_chainVerifyCallback = 13
    int i = 0;
    while(pin[i].type) {
        if(pin[i].type == 13)
            pin[i].value.pointer.chainVerifyCallback->isChainValid = find_CERTChainVerifyCallbackFunc_Fct(pin[i].value.pointer.chainVerifyCallback->isChainValid);
        ++i;
    }
    int ret = my->CERT_PKIXVerifyCert(cert, usages, pin, pout, wincx);
    /*while(pin[i].type) {
        if(pin[i].type == 13)
            pin[i].value.pointer.p = reverse_CERTChainVerifyCallbackFunc_Fct(my_lib, pin[i].value.pointer.p);
        ++i;
    }*/

    return ret;
}

EXPORT void my_PORT_SetUCS2_ASCIIConversionFunction(x64emu_t* emu, void* f)
{
    my->PORT_SetUCS2_ASCIIConversionFunction(find_PORTCharConversionWSwapFunc_Fct(f));
}

EXPORT int my_NSS_RegisterShutdown(x64emu_t* emu, void* f, void* data)
{
    return my->NSS_RegisterShutdown(find_NSS_ShutdownFunc_Fct(f), data);
}

#include "wrappedlib_init.h"
