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
#include "box64context.h"
#include "librarian.h"
#include "callback.h"

const char* xml2Name =
#ifdef ANDROID
    "libxml2.so"
#else
    "libxml2.so.2"
#endif
    ;
#define LIBNAME xml2
static library_t *my_lib = NULL;

typedef int     (*iFv_t)        ();
typedef void*   (*pFv_t)        ();
typedef void    (*vFp_t)        (void*);
typedef int     (*iFp_t)        (void*);
typedef void*   (*pFpp_t)       (void*, void*);
typedef void    (*vFpp_t)       (void*, void*);
typedef int     (*iFppp_t)      (void*, void*, void*);
typedef void    (*vFppp_t)      (void*, void*, void*);
typedef void*   (*pFppp_t)      (void*, void*, void*);
typedef void*   (*pFpppi_t)     (void*, void*, void*, int);
typedef int     (*iFpppp_t)     (void*, void*, void*, void*);
typedef void    (*vFpppp_t)     (void*, void*, void*, void*);
typedef void*   (*pFpppp_t)     (void*, void*, void*, void*);
typedef void*   (*pFppppi_t)    (void*, void*, void*, void*, int);
typedef int     (*iFppppp_t)    (void*, void*, void*, void*, void*);
typedef void    (*vFppppp_t)    (void*, void*, void*, void*, void*);
typedef int     (*iFpppppp_t)   (void*, void*, void*, void*, void*, void*);
typedef void    (*vFpppppp_t)   (void*, void*, void*, void*, void*, void*);
typedef void*   (*pFpppppi_t)   (void*, void*, void*, void*, void*, int);

#define SUPER()                                     \
    GO(xmlHashCopy, pFpp_t)                         \
    GO(xmlHashFree, vFpp_t)                         \
    GO(xmlHashRemoveEntry, iFppp_t)                 \
    GO(xmlHashRemoveEntry2, iFpppp_t)               \
    GO(xmlHashRemoveEntry3, iFppppp_t)              \
    GO(xmlHashScan, vFppp_t)                        \
    GO(xmlHashScan3, vFpppppp_t)                    \
    GO(xmlHashScanFull, vFppp_t)                    \
    GO(xmlHashScanFull3, vFpppppp_t)                \
    GO(xmlHashUpdateEntry, iFpppp_t)                \
    GO(xmlHashUpdateEntry2, iFppppp_t)              \
    GO(xmlHashUpdateEntry3, iFpppppp_t)             \
    GO(xmlGetExternalEntityLoader, pFv_t)           \
    GO(xmlNewCharEncodingHandler, pFppp_t)          \
    GO(xmlOutputBufferCreateIO, pFpppp_t)           \
    GO(xmlRegisterInputCallbacks, iFpppp_t)         \
    GO(xmlSaveToIO, pFppppi_t)                      \
    GO(xmlSchemaSetParserErrors, vFpppp_t)          \
    GO(xmlSchemaSetParserStructuredErrors, vFppp_t) \
    GO(xmlSchemaSetValidErrors, vFpppp_t)           \
    GO(xmlSchemaSetValidStructuredErrors, vFppp_t)  \
    GO(xmlSetExternalEntityLoader, vFp_t)           \
    GO(xmlXPathRegisterFunc, iFppp_t)               \
    GO(xmlParserInputBufferCreateIO, pFpppi_t)      \
    GO(xmlInitMemory, iFv_t)                        \
    GO(xmlParseDocument, iFp_t)                     \
    GO(xmlCreateIOParserCtxt, pFpppppi_t)           \

EXPORT uintptr_t my_xmlFree = 0;
EXPORT uintptr_t my_xmlMalloc = 0;
EXPORT uintptr_t my_xmlRealloc = 0;
EXPORT uintptr_t my_xmlMemStrdup = 0;

void my_wrap_xmlFree(void* p)
{
    if(my_xmlFree){
        RunFunction(my_context, my_xmlFree, 1, p);
        return;
    }
        free(p);
}
void* my_wrap_xmlMalloc(size_t s)
{
    if(my_xmlMalloc)
        return (void*)RunFunction(my_context, my_xmlMalloc, 1, s);
    else
        return malloc(s);
}
void* my_wrap_xmlRealloc(void* p, size_t s)
{
    if(my_xmlRealloc)
        return (void*)RunFunction(my_context, my_xmlRealloc, 2, p, s);
    else
        return realloc(p, s);
}
void* my_wrap_xmlMemStrdup(void* p)
{
    if(my_xmlMemStrdup)
        return (void*)RunFunction(my_context, my_xmlMemStrdup, 1, p);
    else
        return strdup(p);
}

typedef struct xml2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} xml2_my_t;

void* getXml2My(library_t* lib)
{
    xml2_my_t* my = (xml2_my_t*)calloc(1, sizeof(xml2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    void** p;
    // wrap memory pointer
    #define GO(A, W)                                                                    \
        p=dlsym(lib->priv.w.lib, #A);                                                   \
        printf_log(LOG_DEBUG, "Wrapping %s=>%p(%p) from lixml2\n", #A, p, p?*p:NULL);   \
        my_##A = (p && *p)?AddBridge(lib->priv.w.bridge, W, *p, 0, #A):0;               \
        if(p) *p = my_wrap_##A
    GO(xmlFree, vFp);
    GO(xmlMalloc, pFL);
    GO(xmlRealloc, pFpL);
    GO(xmlMemStrdup, pFp);
    #undef GO
    return my;
}
#undef SUPER

void freeXml2My(void* lib)
{
    //xml2_my_t *my = (xml2_my_t *)lib;
}

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
GO(9)

// xmlHashCopier ...
#define GO(A)   \
static uintptr_t my_xmlHashCopier_fct_##A = 0;                                  \
static void* my_xmlHashCopier_##A(void* a, void* b)                             \
{                                                                               \
    return (void*)RunFunction(my_context, my_xmlHashCopier_fct_##A, 2, a, b);   \
}
SUPER()
#undef GO
static void* find_xmlHashCopier_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashCopier_fct_##A == (uintptr_t)fct) return my_xmlHashCopier_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashCopier_fct_##A == 0) {my_xmlHashCopier_fct_##A = (uintptr_t)fct; return my_xmlHashCopier_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashCopier callback\n");
    return NULL;
}
// xmlHashDeallocator ...
#define GO(A)   \
static uintptr_t my_xmlHashDeallocator_fct_##A = 0;                     \
static void my_xmlHashDeallocator_##A(void* a, void* b)                 \
{                                                                       \
    RunFunction(my_context, my_xmlHashDeallocator_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_xmlHashDeallocator_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashDeallocator_fct_##A == (uintptr_t)fct) return my_xmlHashDeallocator_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashDeallocator_fct_##A == 0) {my_xmlHashDeallocator_fct_##A = (uintptr_t)fct; return my_xmlHashDeallocator_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashDeallocator callback\n");
    return NULL;
}
// xmlHashScanner ...
#define GO(A)   \
static uintptr_t my_xmlHashScanner_fct_##A = 0;                     \
static void my_xmlHashScanner_##A(void* a, void* b, void* c)        \
{                                                                   \
    RunFunction(my_context, my_xmlHashScanner_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_xmlHashScanner_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashScanner_fct_##A == (uintptr_t)fct) return my_xmlHashScanner_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashScanner_fct_##A == 0) {my_xmlHashScanner_fct_##A = (uintptr_t)fct; return my_xmlHashScanner_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashScanner callback\n");
    return NULL;
}
// xmlHashScannerFull ...
#define GO(A)   \
static uintptr_t my_xmlHashScannerFull_fct_##A = 0;                                 \
static void my_xmlHashScannerFull_##A(void* a, void* b, void* c, void* c2, void* c3)\
{                                                                                   \
    RunFunction(my_context, my_xmlHashScannerFull_fct_##A, 5, a, b, c, c2, c3);     \
}
SUPER()
#undef GO
static void* find_xmlHashScannerFull_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashScannerFull_fct_##A == (uintptr_t)fct) return my_xmlHashScannerFull_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashScannerFull_fct_##A == 0) {my_xmlHashScannerFull_fct_##A = (uintptr_t)fct; return my_xmlHashScannerFull_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashScannerFull callback\n");
    return NULL;
}
// xmlCharEncodingInputFunc ...
#define GO(A)   \
static uintptr_t my_xmlCharEncodingInputFunc_fct_##A = 0;                               \
static int my_xmlCharEncodingInputFunc_##A(void* a, void* b, void* c, void* d)          \
{                                                                                       \
    return RunFunction(my_context, my_xmlCharEncodingInputFunc_fct_##A, 4, a, b, c, d); \
}
SUPER()
#undef GO
static void* find_xmlCharEncodingInputFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlCharEncodingInputFunc_fct_##A == (uintptr_t)fct) return my_xmlCharEncodingInputFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlCharEncodingInputFunc_fct_##A == 0) {my_xmlCharEncodingInputFunc_fct_##A = (uintptr_t)fct; return my_xmlCharEncodingInputFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlCharEncodingInputFunc callback\n");
    return NULL;
}
// xmlCharEncodingOutputFunc ...
#define GO(A)   \
static uintptr_t my_xmlCharEncodingOutputFunc_fct_##A = 0;                              \
static int my_xmlCharEncodingOutputFunc_##A(void* a, void* b, void* c, void* d)         \
{                                                                                       \
    return RunFunction(my_context, my_xmlCharEncodingOutputFunc_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* find_xmlCharEncodingOutputFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlCharEncodingOutputFunc_fct_##A == (uintptr_t)fct) return my_xmlCharEncodingOutputFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlCharEncodingOutputFunc_fct_##A == 0) {my_xmlCharEncodingOutputFunc_fct_##A = (uintptr_t)fct; return my_xmlCharEncodingOutputFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlCharEncodingOutputFunc callback\n");
    return NULL;
}
// xmlOutputWriteCallback ...
#define GO(A)   \
static uintptr_t my_xmlOutputWriteCallback_fct_##A = 0;                             \
static int my_xmlOutputWriteCallback_##A(void* a, void* b, int c)                   \
{                                                                                   \
    return RunFunction(my_context, my_xmlOutputWriteCallback_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_xmlOutputWriteCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlOutputWriteCallback_fct_##A == (uintptr_t)fct) return my_xmlOutputWriteCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlOutputWriteCallback_fct_##A == 0) {my_xmlOutputWriteCallback_fct_##A = (uintptr_t)fct; return my_xmlOutputWriteCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlOutputWriteCallback callback\n");
    return NULL;
}
// xmlOutputCloseCallback ...
#define GO(A)   \
static uintptr_t my_xmlOutputCloseCallback_fct_##A = 0;                         \
static int my_xmlOutputCloseCallback_##A(void* a)                               \
{                                                                               \
    return RunFunction(my_context, my_xmlOutputCloseCallback_fct_##A, 1, a);    \
}
SUPER()
#undef GO
static void* find_xmlOutputCloseCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlOutputCloseCallback_fct_##A == (uintptr_t)fct) return my_xmlOutputCloseCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlOutputCloseCallback_fct_##A == 0) {my_xmlOutputCloseCallback_fct_##A = (uintptr_t)fct; return my_xmlOutputCloseCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlOutputCloseCallback callback\n");
    return NULL;
}
// xmlInputMatchCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputMatchCallback_fct_##A = 0;                      \
static int my_xmlInputMatchCallback_##A(void* a)                            \
{                                                                           \
    return RunFunction(my_context, my_xmlInputMatchCallback_fct_##A, 1, a); \
}
SUPER()
#undef GO
static void* find_xmlInputMatchCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputMatchCallback_fct_##A == (uintptr_t)fct) return my_xmlInputMatchCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputMatchCallback_fct_##A == 0) {my_xmlInputMatchCallback_fct_##A = (uintptr_t)fct; return my_xmlInputMatchCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputMatchCallback callback\n");
    return NULL;
}
// xmlInputOpenCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputOpenCallback_fct_##A = 0;                               \
static void* my_xmlInputOpenCallback_##A(void* a)                                   \
{                                                                                   \
    return (void*)RunFunction(my_context, my_xmlInputOpenCallback_fct_##A, 1, a);   \
}
SUPER()
#undef GO
static void* find_xmlInputOpenCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputOpenCallback_fct_##A == (uintptr_t)fct) return my_xmlInputOpenCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputOpenCallback_fct_##A == 0) {my_xmlInputOpenCallback_fct_##A = (uintptr_t)fct; return my_xmlInputOpenCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputOpenCallback callback\n");
    return NULL;
}
// xmlInputReadCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputReadCallback_fct_##A = 0;                               \
static int my_xmlInputReadCallback_##A(void* a, void* b, int c)                     \
{                                                                                   \
    return RunFunction(my_context, my_xmlInputReadCallback_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_xmlInputReadCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputReadCallback_fct_##A == (uintptr_t)fct) return my_xmlInputReadCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputReadCallback_fct_##A == 0) {my_xmlInputReadCallback_fct_##A = (uintptr_t)fct; return my_xmlInputReadCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputReadCallback callback\n");
    return NULL;
}
// xmlInputCloseCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputCloseCallback_fct_##A = 0;                         \
static int my_xmlInputCloseCallback_##A(void* a)                               \
{                                                                               \
    return RunFunction(my_context, my_xmlInputCloseCallback_fct_##A, 1, a);    \
}
SUPER()
#undef GO
static void* find_xmlInputCloseCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputCloseCallback_fct_##A == (uintptr_t)fct) return my_xmlInputCloseCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputCloseCallback_fct_##A == 0) {my_xmlInputCloseCallback_fct_##A = (uintptr_t)fct; return my_xmlInputCloseCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputCloseCallback callback\n");
    return NULL;
}
// xmlSchemaValidityErrorFunc (use a VA)...
#define GO(A)   \
static uintptr_t my_xmlSchemaValidityErrorFunc_fct_##A = 0;                                                                     \
static void my_xmlSchemaValidityErrorFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i)  \
{                                                                                                                               \
    RunFunction(my_context, my_xmlSchemaValidityErrorFunc_fct_##A, 9, a, b, c, d, e, f, g, h, i);                               \
}
SUPER()
#undef GO
static void* find_xmlSchemaValidityErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlSchemaValidityErrorFunc_fct_##A == (uintptr_t)fct) return my_xmlSchemaValidityErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlSchemaValidityErrorFunc_fct_##A == 0) {my_xmlSchemaValidityErrorFunc_fct_##A = (uintptr_t)fct; return my_xmlSchemaValidityErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlSchemaValidityErrorFunc callback\n");
    return NULL;
}
// xmlSchemaValidityWarningFunc (use a VA)...
#define GO(A)   \
static uintptr_t my_xmlSchemaValidityWarningFunc_fct_##A = 0;                                                                       \
static void my_xmlSchemaValidityWarningFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i)    \
{                                                                                                                                   \
    RunFunction(my_context, my_xmlSchemaValidityWarningFunc_fct_##A, 9, a, b, c, d, e, f, g, h, i);                                 \
}
SUPER()
#undef GO
static void* find_xmlSchemaValidityWarningFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlSchemaValidityWarningFunc_fct_##A == (uintptr_t)fct) return my_xmlSchemaValidityWarningFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlSchemaValidityWarningFunc_fct_##A == 0) {my_xmlSchemaValidityWarningFunc_fct_##A = (uintptr_t)fct; return my_xmlSchemaValidityWarningFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlSchemaValidityWarningFunc callback\n");
    return NULL;
}
// xmlStructuredErrorFunc ...
#define GO(A)   \
static uintptr_t my_xmlStructuredErrorFunc_fct_##A = 0;                     \
static void my_xmlStructuredErrorFunc_##A(void* a, void* b)                 \
{                                                                           \
    RunFunction(my_context, my_xmlStructuredErrorFunc_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_xmlStructuredErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlStructuredErrorFunc_fct_##A == (uintptr_t)fct) return my_xmlStructuredErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlStructuredErrorFunc_fct_##A == 0) {my_xmlStructuredErrorFunc_fct_##A = (uintptr_t)fct; return my_xmlStructuredErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlStructuredErrorFunc callback\n");
    return NULL;
}
// xmlXPathFunction ...
#define GO(A)   \
static uintptr_t my_xmlXPathFunction_fct_##A = 0;                   \
static void my_xmlXPathFunction_##A(void* a, int b)                 \
{                                                                   \
    RunFunction(my_context, my_xmlXPathFunction_fct_##A, 2, a, b);  \
}
SUPER()
#undef GO
static void* find_xmlXPathFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlXPathFunction_fct_##A == (uintptr_t)fct) return my_xmlXPathFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlXPathFunction_fct_##A == 0) {my_xmlXPathFunction_fct_##A = (uintptr_t)fct; return my_xmlXPathFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlXPathFunction callback\n");
    return NULL;
}
// internalSubsetSAXFunc ...
#define GO(A)   \
static uintptr_t my_internalSubsetSAXFunc_fct_##A = 0;                          \
static void my_internalSubsetSAXFunc_##A(void* a, void* b, void* c, void* d)    \
{                                                                               \
    RunFunction(my_context, my_internalSubsetSAXFunc_fct_##A, 4, a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_internalSubsetSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_internalSubsetSAXFunc_fct_##A == (uintptr_t)fct) return my_internalSubsetSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_internalSubsetSAXFunc_fct_##A == 0) {my_internalSubsetSAXFunc_fct_##A = (uintptr_t)fct; return my_internalSubsetSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 internalSubsetSAXFunc callback\n");
    return NULL;
}
// isStandaloneSAXFunc ...
#define GO(A)   \
static uintptr_t my_isStandaloneSAXFunc_fct_##A = 0;                        \
static int my_isStandaloneSAXFunc_##A(void* a)                              \
{                                                                           \
    return RunFunction(my_context, my_isStandaloneSAXFunc_fct_##A, 1, a);   \
}
SUPER()
#undef GO
static void* find_isStandaloneSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_isStandaloneSAXFunc_fct_##A == (uintptr_t)fct) return my_isStandaloneSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_isStandaloneSAXFunc_fct_##A == 0) {my_isStandaloneSAXFunc_fct_##A = (uintptr_t)fct; return my_isStandaloneSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 isStandaloneSAXFunc callback\n");
    return NULL;
}
// hasInternalSubsetSAXFunc ...
#define GO(A)   \
static uintptr_t my_hasInternalSubsetSAXFunc_fct_##A = 0;                       \
static int my_hasInternalSubsetSAXFunc_##A(void* a)                             \
{                                                                               \
    return RunFunction(my_context, my_hasInternalSubsetSAXFunc_fct_##A, 1, a);  \
}
SUPER()
#undef GO
static void* find_hasInternalSubsetSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_hasInternalSubsetSAXFunc_fct_##A == (uintptr_t)fct) return my_hasInternalSubsetSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_hasInternalSubsetSAXFunc_fct_##A == 0) {my_hasInternalSubsetSAXFunc_fct_##A = (uintptr_t)fct; return my_hasInternalSubsetSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 hasInternalSubsetSAXFunc callback\n");
    return NULL;
}
// hasExternalSubsetSAXFunc ...
#define GO(A)   \
static uintptr_t my_hasExternalSubsetSAXFunc_fct_##A = 0;                       \
static int my_hasExternalSubsetSAXFunc_##A(void* a)                             \
{                                                                               \
    return RunFunction(my_context, my_hasExternalSubsetSAXFunc_fct_##A, 1, a);  \
}
SUPER()
#undef GO
static void* find_hasExternalSubsetSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_hasExternalSubsetSAXFunc_fct_##A == (uintptr_t)fct) return my_hasExternalSubsetSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_hasExternalSubsetSAXFunc_fct_##A == 0) {my_hasExternalSubsetSAXFunc_fct_##A = (uintptr_t)fct; return my_hasExternalSubsetSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 hasExternalSubsetSAXFunc callback\n");
    return NULL;
}
// resolveEntitySAXFunc ...
#define GO(A)   \
static uintptr_t my_resolveEntitySAXFunc_fct_##A = 0;                                   \
static void* my_resolveEntitySAXFunc_##A(void* a, void* b, void* c)                     \
{                                                                                       \
    return (void*)RunFunction(my_context, my_resolveEntitySAXFunc_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_resolveEntitySAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_resolveEntitySAXFunc_fct_##A == (uintptr_t)fct) return my_resolveEntitySAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_resolveEntitySAXFunc_fct_##A == 0) {my_resolveEntitySAXFunc_fct_##A = (uintptr_t)fct; return my_resolveEntitySAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 resolveEntitySAXFunc callback\n");
    return NULL;
}
// getEntitySAXFunc ...
#define GO(A)   \
static uintptr_t my_getEntitySAXFunc_fct_##A = 0;                                   \
static void* my_getEntitySAXFunc_##A(void* a, void* b)                              \
{                                                                                   \
    return (void*)RunFunction(my_context, my_getEntitySAXFunc_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_getEntitySAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getEntitySAXFunc_fct_##A == (uintptr_t)fct) return my_getEntitySAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getEntitySAXFunc_fct_##A == 0) {my_getEntitySAXFunc_fct_##A = (uintptr_t)fct; return my_getEntitySAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 getEntitySAXFunc callback\n");
    return NULL;
}
// entityDeclSAXFunc ...
#define GO(A)   \
static uintptr_t my_entityDeclSAXFunc_fct_##A = 0;                                      \
static void my_entityDeclSAXFunc_##A(void* a, void* b, int c, void* d, void* e, void* f)\
{                                                                                       \
    RunFunction(my_context, my_entityDeclSAXFunc_fct_##A, 6, a, b, c, d, e, f);         \
}
SUPER()
#undef GO
static void* find_entityDeclSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_entityDeclSAXFunc_fct_##A == (uintptr_t)fct) return my_entityDeclSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_entityDeclSAXFunc_fct_##A == 0) {my_entityDeclSAXFunc_fct_##A = (uintptr_t)fct; return my_entityDeclSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 entityDeclSAXFunc callback\n");
    return NULL;
}
// notationDeclSAXFunc ...
#define GO(A)   \
static uintptr_t my_notationDeclSAXFunc_fct_##A = 0;                        \
static void my_notationDeclSAXFunc_##A(void* a, void* b, void* c, void* d)  \
{                                                                           \
    RunFunction(my_context, my_notationDeclSAXFunc_fct_##A, 4, a, b, c, d); \
}
SUPER()
#undef GO
static void* find_notationDeclSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_notationDeclSAXFunc_fct_##A == (uintptr_t)fct) return my_notationDeclSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_notationDeclSAXFunc_fct_##A == 0) {my_notationDeclSAXFunc_fct_##A = (uintptr_t)fct; return my_notationDeclSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 notationDeclSAXFunc callback\n");
    return NULL;
}
// attributeDeclSAXFunc ...
#define GO(A)   \
static uintptr_t my_attributeDeclSAXFunc_fct_##A = 0;                                               \
static void my_attributeDeclSAXFunc_##A(void* a, void* b, void* c, int d, int e, void* f, void* g)  \
{                                                                                                   \
    RunFunction(my_context, my_attributeDeclSAXFunc_fct_##A, 7, a, b, c, d, e, f, g);               \
}
SUPER()
#undef GO
static void* find_attributeDeclSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_attributeDeclSAXFunc_fct_##A == (uintptr_t)fct) return my_attributeDeclSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_attributeDeclSAXFunc_fct_##A == 0) {my_attributeDeclSAXFunc_fct_##A = (uintptr_t)fct; return my_attributeDeclSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 attributeDeclSAXFunc callback\n");
    return NULL;
}
// elementDeclSAXFunc ...
#define GO(A)   \
static uintptr_t my_elementDeclSAXFunc_fct_##A = 0;                         \
static void my_elementDeclSAXFunc_##A(void* a, void* b, int c, void* d)     \
{                                                                           \
    RunFunction(my_context, my_elementDeclSAXFunc_fct_##A, 4, a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_elementDeclSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_elementDeclSAXFunc_fct_##A == (uintptr_t)fct) return my_elementDeclSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_elementDeclSAXFunc_fct_##A == 0) {my_elementDeclSAXFunc_fct_##A = (uintptr_t)fct; return my_elementDeclSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 elementDeclSAXFunc callback\n");
    return NULL;
}
// unparsedEntityDeclSAXFunc ...
#define GO(A)   \
static uintptr_t my_unparsedEntityDeclSAXFunc_fct_##A = 0;                                  \
static void my_unparsedEntityDeclSAXFunc_##A(void* a, void* b, void* c, void* d, void* e)   \
{                                                                                           \
    RunFunction(my_context, my_unparsedEntityDeclSAXFunc_fct_##A, 5, a, b, c, d, e);        \
}
SUPER()
#undef GO
static void* find_unparsedEntityDeclSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_unparsedEntityDeclSAXFunc_fct_##A == (uintptr_t)fct) return my_unparsedEntityDeclSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_unparsedEntityDeclSAXFunc_fct_##A == 0) {my_unparsedEntityDeclSAXFunc_fct_##A = (uintptr_t)fct; return my_unparsedEntityDeclSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 unparsedEntityDeclSAXFunc callback\n");
    return NULL;
}
// setDocumentLocatorSAXFunc ...
#define GO(A)   \
static uintptr_t my_setDocumentLocatorSAXFunc_fct_##A = 0;                  \
static void my_setDocumentLocatorSAXFunc_##A(void* a, void* b)              \
{                                                                           \
    RunFunction(my_context, my_setDocumentLocatorSAXFunc_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* find_setDocumentLocatorSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_setDocumentLocatorSAXFunc_fct_##A == (uintptr_t)fct) return my_setDocumentLocatorSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_setDocumentLocatorSAXFunc_fct_##A == 0) {my_setDocumentLocatorSAXFunc_fct_##A = (uintptr_t)fct; return my_setDocumentLocatorSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 setDocumentLocatorSAXFunc callback\n");
    return NULL;
}
// startDocumentSAXFunc ...
#define GO(A)   \
static uintptr_t my_startDocumentSAXFunc_fct_##A = 0;               \
static void my_startDocumentSAXFunc_##A(void* a)                    \
{                                                                   \
    RunFunction(my_context, my_startDocumentSAXFunc_fct_##A, 1, a); \
}
SUPER()
#undef GO
static void* find_startDocumentSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_startDocumentSAXFunc_fct_##A == (uintptr_t)fct) return my_startDocumentSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_startDocumentSAXFunc_fct_##A == 0) {my_startDocumentSAXFunc_fct_##A = (uintptr_t)fct; return my_startDocumentSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 startDocumentSAXFunc callback\n");
    return NULL;
}
// endDocumentSAXFunc ...
#define GO(A)   \
static uintptr_t my_endDocumentSAXFunc_fct_##A = 0;               \
static void my_endDocumentSAXFunc_##A(void* a)                    \
{                                                                   \
    RunFunction(my_context, my_endDocumentSAXFunc_fct_##A, 1, a); \
}
SUPER()
#undef GO
static void* find_endDocumentSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_endDocumentSAXFunc_fct_##A == (uintptr_t)fct) return my_endDocumentSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_endDocumentSAXFunc_fct_##A == 0) {my_endDocumentSAXFunc_fct_##A = (uintptr_t)fct; return my_endDocumentSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 endDocumentSAXFunc callback\n");
    return NULL;
}
// startElementSAXFunc ...
#define GO(A)   \
static uintptr_t my_startElementSAXFunc_fct_##A = 0;                        \
static void my_startElementSAXFunc_##A(void* a, void* b, void* c)           \
{                                                                           \
    RunFunction(my_context, my_startElementSAXFunc_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_startElementSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_startElementSAXFunc_fct_##A == (uintptr_t)fct) return my_startElementSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_startElementSAXFunc_fct_##A == 0) {my_startElementSAXFunc_fct_##A = (uintptr_t)fct; return my_startElementSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 startElementSAXFunc callback\n");
    return NULL;
}
// endElementSAXFunc ...
#define GO(A)   \
static uintptr_t my_endElementSAXFunc_fct_##A = 0;                  \
static void my_endElementSAXFunc_##A(void* a, void* b)              \
{                                                                   \
    RunFunction(my_context, my_endElementSAXFunc_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* find_endElementSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_endElementSAXFunc_fct_##A == (uintptr_t)fct) return my_endElementSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_endElementSAXFunc_fct_##A == 0) {my_endElementSAXFunc_fct_##A = (uintptr_t)fct; return my_endElementSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 endElementSAXFunc callback\n");
    return NULL;
}
// referenceSAXFunc ...
#define GO(A)   \
static uintptr_t my_referenceSAXFunc_fct_##A = 0;                  \
static void my_referenceSAXFunc_##A(void* a, void* b)              \
{                                                                   \
    RunFunction(my_context, my_referenceSAXFunc_fct_##A, 2, a, b); \
}
SUPER()
#undef GO
static void* find_referenceSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_referenceSAXFunc_fct_##A == (uintptr_t)fct) return my_referenceSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_referenceSAXFunc_fct_##A == 0) {my_referenceSAXFunc_fct_##A = (uintptr_t)fct; return my_referenceSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 referenceSAXFunc callback\n");
    return NULL;
}
// charactersSAXFunc ...
#define GO(A)   \
static uintptr_t my_charactersSAXFunc_fct_##A = 0;                      \
static void my_charactersSAXFunc_##A(void* a, void* b, int c)           \
{                                                                       \
    RunFunction(my_context, my_charactersSAXFunc_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_charactersSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_charactersSAXFunc_fct_##A == (uintptr_t)fct) return my_charactersSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_charactersSAXFunc_fct_##A == 0) {my_charactersSAXFunc_fct_##A = (uintptr_t)fct; return my_charactersSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 charactersSAXFunc callback\n");
    return NULL;
}
// ignorableWhitespaceSAXFunc ...
#define GO(A)   \
static uintptr_t my_ignorableWhitespaceSAXFunc_fct_##A = 0;                      \
static void my_ignorableWhitespaceSAXFunc_##A(void* a, void* b, int c)           \
{                                                                       \
    RunFunction(my_context, my_ignorableWhitespaceSAXFunc_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_ignorableWhitespaceSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ignorableWhitespaceSAXFunc_fct_##A == (uintptr_t)fct) return my_ignorableWhitespaceSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ignorableWhitespaceSAXFunc_fct_##A == 0) {my_ignorableWhitespaceSAXFunc_fct_##A = (uintptr_t)fct; return my_ignorableWhitespaceSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 ignorableWhitespaceSAXFunc callback\n");
    return NULL;
}
// processingInstructionSAXFunc ...
#define GO(A)   \
static uintptr_t my_processingInstructionSAXFunc_fct_##A = 0;                       \
static void my_processingInstructionSAXFunc_##A(void* a, void* b, void* c)          \
{                                                                                   \
    RunFunction(my_context, my_processingInstructionSAXFunc_fct_##A, 3, a, b, c);   \
}
SUPER()
#undef GO
static void* find_processingInstructionSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_processingInstructionSAXFunc_fct_##A == (uintptr_t)fct) return my_processingInstructionSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_processingInstructionSAXFunc_fct_##A == 0) {my_processingInstructionSAXFunc_fct_##A = (uintptr_t)fct; return my_processingInstructionSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 processingInstructionSAXFunc callback\n");
    return NULL;
}
// commentSAXFunc ...
#define GO(A)   \
static uintptr_t my_commentSAXFunc_fct_##A = 0;                     \
static void my_commentSAXFunc_##A(void* a, void* b)                 \
{                                                                   \
    RunFunction(my_context, my_commentSAXFunc_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_commentSAXFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_commentSAXFunc_fct_##A == (uintptr_t)fct) return my_commentSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_commentSAXFunc_fct_##A == 0) {my_commentSAXFunc_fct_##A = (uintptr_t)fct; return my_commentSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 commentSAXFunc callback\n");
    return NULL;
}
// warningSAXFunc ...
#define GO(A)   \
static uintptr_t my_warningSAXFunc_fct_##A = 0;                     \
static void my_warningSAXFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j)\
{                                                                   \
    RunFunction(my_context, my_warningSAXFunc_fct_##A, 10, a, b, c, d, e, f, g, h, i, j);    \
}
SUPER()
#undef GO
static void* find_warningSAXFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_warningSAXFunc_fct_##A == (uintptr_t)fct) return my_warningSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_warningSAXFunc_fct_##A == 0) {my_warningSAXFunc_fct_##A = (uintptr_t)fct; return my_warningSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 warningSAXFunc callback\n");
    return NULL;
}
// errorSAXFunc ...
#define GO(A)   \
static uintptr_t my_errorSAXFunc_fct_##A = 0;                     \
static void my_errorSAXFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j)\
{                                                                   \
    RunFunction(my_context, my_errorSAXFunc_fct_##A, 10, a, b, c, d, e, f, g, h, i, j);    \
}
SUPER()
#undef GO
static void* find_errorSAXFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_errorSAXFunc_fct_##A == (uintptr_t)fct) return my_errorSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_errorSAXFunc_fct_##A == 0) {my_errorSAXFunc_fct_##A = (uintptr_t)fct; return my_errorSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 errorSAXFunc callback\n");
    return NULL;
}
// fatalErrorSAXFunc ...
#define GO(A)   \
static uintptr_t my_fatalErrorSAXFunc_fct_##A = 0;                     \
static void my_fatalErrorSAXFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j)\
{                                                                   \
    RunFunction(my_context, my_fatalErrorSAXFunc_fct_##A, 10, a, b, c, d, e, f, g, h, i, j);    \
}
SUPER()
#undef GO
static void* find_fatalErrorSAXFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fatalErrorSAXFunc_fct_##A == (uintptr_t)fct) return my_fatalErrorSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fatalErrorSAXFunc_fct_##A == 0) {my_fatalErrorSAXFunc_fct_##A = (uintptr_t)fct; return my_fatalErrorSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 fatalErrorSAXFunc callback\n");
    return NULL;
}
// getParameterEntitySAXFunc ...
#define GO(A)   \
static uintptr_t my_getParameterEntitySAXFunc_fct_##A = 0;                                  \
static void* my_getParameterEntitySAXFunc_##A(void* a, void* b)                             \
{                                                                                           \
    return (void*)RunFunction(my_context, my_getParameterEntitySAXFunc_fct_##A, 2, a, b);   \
}
SUPER()
#undef GO
static void* find_getParameterEntitySAXFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_getParameterEntitySAXFunc_fct_##A == (uintptr_t)fct) return my_getParameterEntitySAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_getParameterEntitySAXFunc_fct_##A == 0) {my_getParameterEntitySAXFunc_fct_##A = (uintptr_t)fct; return my_getParameterEntitySAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 getParameterEntitySAXFunc callback\n");
    return NULL;
}
// cdataBlockSAXFunc ...
#define GO(A)   \
static uintptr_t my_cdataBlockSAXFunc_fct_##A = 0;                      \
static void my_cdataBlockSAXFunc_##A(void* a, void* b, int c)           \
{                                                                       \
    RunFunction(my_context, my_cdataBlockSAXFunc_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_cdataBlockSAXFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cdataBlockSAXFunc_fct_##A == (uintptr_t)fct) return my_cdataBlockSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cdataBlockSAXFunc_fct_##A == 0) {my_cdataBlockSAXFunc_fct_##A = (uintptr_t)fct; return my_cdataBlockSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 cdataBlockSAXFunc callback\n");
    return NULL;
}
// externalSubsetSAXFunc ...
#define GO(A)   \
static uintptr_t my_externalSubsetSAXFunc_fct_##A = 0;                          \
static void my_externalSubsetSAXFunc_##A(void* a, void* b, void* c, void* d)    \
{                                                                               \
    RunFunction(my_context, my_externalSubsetSAXFunc_fct_##A, 4, a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_externalSubsetSAXFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_externalSubsetSAXFunc_fct_##A == (uintptr_t)fct) return my_externalSubsetSAXFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_externalSubsetSAXFunc_fct_##A == 0) {my_externalSubsetSAXFunc_fct_##A = (uintptr_t)fct; return my_externalSubsetSAXFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 externalSubsetSAXFunc callback\n");
    return NULL;
}
// xmlSAX2StartElementNs ...
#define GO(A)   \
static uintptr_t my_xmlSAX2StartElementNs_fct_##A = 0;                                                              \
static void my_xmlSAX2StartElementNs_##A(void* a, void* b, void* c, void* d, int e, void* f, int g, int h, void* i) \
{                                                                                                                   \
    RunFunction(my_context, my_xmlSAX2StartElementNs_fct_##A, 9, a, b, c, d, e, f, g, h, i);                        \
}
SUPER()
#undef GO
static void* find_xmlSAX2StartElementNs_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlSAX2StartElementNs_fct_##A == (uintptr_t)fct) return my_xmlSAX2StartElementNs_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlSAX2StartElementNs_fct_##A == 0) {my_xmlSAX2StartElementNs_fct_##A = (uintptr_t)fct; return my_xmlSAX2StartElementNs_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlSAX2StartElementNs callback\n");
    return NULL;
}
// xmlSAX2EndElementNs ...
#define GO(A)   \
static uintptr_t my_xmlSAX2EndElementNs_fct_##A = 0;                        \
static void my_xmlSAX2EndElementNs_##A(void* a, void* b, void* c, void* d)  \
{                                                                           \
    RunFunction(my_context, my_xmlSAX2EndElementNs_fct_##A, 4, a, b, c, d); \
}
SUPER()
#undef GO
static void* find_xmlSAX2EndElementNs_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlSAX2EndElementNs_fct_##A == (uintptr_t)fct) return my_xmlSAX2EndElementNs_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlSAX2EndElementNs_fct_##A == 0) {my_xmlSAX2EndElementNs_fct_##A = (uintptr_t)fct; return my_xmlSAX2EndElementNs_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlSAX2EndElementNs callback\n");
    return NULL;
}

// xmlExternalEntityLoader
#define GO(A)   \
static uintptr_t my_xmlExternalEntityLoader_fct_##A = 0;                                    \
static void* my_xmlExternalEntityLoader_##A(void* a, void* b, void* c)                      \
{                                                                                           \
    return (void*)RunFunction(my_context, my_xmlExternalEntityLoader_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_xmlExternalEntityLoaderFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlExternalEntityLoader_fct_##A == (uintptr_t)fct) return my_xmlExternalEntityLoader_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlExternalEntityLoader_fct_##A == 0) {my_xmlExternalEntityLoader_fct_##A = (uintptr_t)fct; return my_xmlExternalEntityLoader_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlExternalEntityLoader callback\n");
    return NULL;
}
static void* reverse_xmlExternalEntityLoaderFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_lib->priv.w.bridge, fct);
    #define GO(A) if(my_xmlExternalEntityLoader_##A == fct) return (void*)my_xmlExternalEntityLoader_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->priv.w.bridge, pFppp, fct, 0, "xmlExternalEntityLoader_callback");
}


#undef SUPER

EXPORT void* my_xmlHashCopy(x64emu_t* emu, void* table, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashCopy(table, find_xmlHashCopier_Fct(f));
}

EXPORT void my_xmlHashFree(x64emu_t* emu, void* table, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashFree(table, find_xmlHashDeallocator_Fct(f));
}

EXPORT int my_xmlHashRemoveEntry(x64emu_t* emu, void* table, void* name, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashRemoveEntry(table, name, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashRemoveEntry2(x64emu_t* emu, void* table, void* name, void* name2, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashRemoveEntry2(table, name, name2, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashRemoveEntry3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashRemoveEntry3(table, name, name2, name3, find_xmlHashDeallocator_Fct(f));
}

EXPORT void my_xmlHashScan(x64emu_t* emu, void* table, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScan(table, find_xmlHashScanner_Fct(f), data);
}
EXPORT void my_xmlHashScan3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScan3(table, name, name2, name3, find_xmlHashScanner_Fct(f), data);
}
EXPORT void my_xmlHashScanFull(x64emu_t* emu, void* table, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScanFull(table, find_xmlHashScannerFull_Fct(f), data);
}
EXPORT void my_xmlHashScanFull3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScanFull3(table, name, name2, name3, find_xmlHashScannerFull_Fct(f), data);
}

EXPORT int my_xmlHashUpdateEntry(x64emu_t* emu, void* table, void* name, void* data, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashUpdateEntry(table, name, data, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashUpdateEntry2(x64emu_t* emu, void* table, void* name, void* name2, void* data, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashUpdateEntry2(table, name, name2, data, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashUpdateEntry3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* data, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashUpdateEntry3(table, name, name2, name3, data, find_xmlHashDeallocator_Fct(f));
}

EXPORT void* my_xmlGetExternalEntityLoader(x64emu_t* emu)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return reverse_xmlExternalEntityLoaderFct(my->xmlGetExternalEntityLoader());
}

EXPORT void* my_xmlNewCharEncodingHandler(x64emu_t* emu, void* name, void* fin, void* fout)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlNewCharEncodingHandler(name, find_xmlCharEncodingInputFunc_Fct(fin), find_xmlCharEncodingOutputFunc_Fct(fout));
}

EXPORT void* my_xmlOutputBufferCreateIO(x64emu_t* emu, void* fwrite, void* fclose, void* ioctx, void* encoder)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlOutputBufferCreateIO(find_xmlOutputWriteCallback_Fct(fwrite), find_xmlOutputCloseCallback_Fct(fclose), ioctx, encoder);
}

EXPORT int my_xmlRegisterInputCallbacks(x64emu_t* emu, void* fmatch, void* fop, void* frd, void* fcl)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlRegisterInputCallbacks(find_xmlInputMatchCallback_Fct(fmatch), find_xmlInputOpenCallback_Fct(fop), find_xmlInputReadCallback_Fct(frd), find_xmlInputCloseCallback_Fct(fcl));
}

EXPORT void* my_xmlSaveToIO(x64emu_t* emu, void* fwrt, void* fcl, void* ioctx, void* encoding, int options)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlSaveToIO(find_xmlOutputWriteCallback_Fct(fwrt), find_xmlOutputCloseCallback_Fct(fcl), ioctx, encoding, options);
}

EXPORT void my_xmlSchemaSetParserErrors(x64emu_t* emu, void* ctxt, void* ferr, void* fwarn, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetParserErrors(ctxt, find_xmlSchemaValidityErrorFunc_Fct(ferr), find_xmlSchemaValidityWarningFunc_Fct(fwarn), ctx);
}

EXPORT void my_xmlSchemaSetParserStructuredErrors(x64emu_t* emu, void* ctxt, void* ferr, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetParserStructuredErrors(ctxt, find_xmlStructuredErrorFunc_Fct(ferr), ctx);
}

EXPORT void my_xmlSchemaSetValidErrors(x64emu_t* emu, void* ctxt, void* ferr, void* fwarn, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetValidErrors(ctxt, find_xmlSchemaValidityErrorFunc_Fct(ferr), find_xmlSchemaValidityWarningFunc_Fct(fwarn), ctx);
}

EXPORT void my_xmlSchemaSetValidStructuredErrors(x64emu_t* emu, void* ctxt, void* ferr, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetValidStructuredErrors(ctxt, find_xmlStructuredErrorFunc_Fct(ferr), ctx);
}

EXPORT void my_xmlSetExternalEntityLoader(x64emu_t* emu, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSetExternalEntityLoader(find_xmlExternalEntityLoaderFct(f));
}

EXPORT int my_xmlXPathRegisterFunc(x64emu_t* emu, void* ctxt, void* name, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlXPathRegisterFunc(ctxt, name, find_xmlXPathFunction_Fct(f));
}

EXPORT void* my_xmlParserInputBufferCreateIO(x64emu_t* emu, void* ioread, void* ioclose, void* ioctx, int enc)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlParserInputBufferCreateIO(find_xmlInputReadCallback_Fct(ioread), find_xmlInputCloseCallback_Fct(ioclose), ioctx, enc);
}

#define SUPER()\
    GO(internalSubsetSAXFunc, internalSubset)               \
    GO(isStandaloneSAXFunc, isStandalone)                   \
    GO(hasInternalSubsetSAXFunc, hasInternalSubset)         \
    GO(hasExternalSubsetSAXFunc, hasExternalSubset)         \
    GO(resolveEntitySAXFunc, resolveEntity)                 \
    GO(getEntitySAXFunc, getEntity)                         \
    GO(entityDeclSAXFunc, entityDecl)                       \
    GO(notationDeclSAXFunc, notationDecl)                   \
    GO(attributeDeclSAXFunc, attributeDecl)                 \
    GO(elementDeclSAXFunc, elementDecl)                     \
    GO(unparsedEntityDeclSAXFunc, unparsedEntityDecl)       \
    GO(setDocumentLocatorSAXFunc, setDocumentLocator)       \
    GO(startDocumentSAXFunc, startDocument)                 \
    GO(endDocumentSAXFunc, endDocument)                     \
    GO(startElementSAXFunc, startElement)                   \
    GO(endElementSAXFunc, endElement)                       \
    GO(referenceSAXFunc, reference)                         \
    GO(charactersSAXFunc, characters)                       \
    GO(ignorableWhitespaceSAXFunc, ignorableWhitespace)     \
    GO(processingInstructionSAXFunc, processingInstruction) \
    GO(commentSAXFunc, comment)                             \
    GO(warningSAXFunc, warning)                             \
    GO(errorSAXFunc, error)                                 \
    GO(fatalErrorSAXFunc, fatalError)                       \
    GO(getParameterEntitySAXFunc, getParameterEntity)       \
    GO(cdataBlockSAXFunc, cdataBlock)                       \
    GO(externalSubsetSAXFunc, externalSubset)               \
    GA(int, initialized)                                    \
    GA(void*, _private)                                     \
    GO(xmlSAX2StartElementNs, startElementNs)               \
    GO(xmlSAX2EndElementNs,  endElementNs)                  \
    GO(xmlStructuredErrorFunc, serror)

#define GO(T, A) void* A;
#define GA(T, A) T A;
typedef struct my_xmlSAXHandler_s {
    SUPER()
} my_xmlSAXHandler_t;
#undef GO
#undef GA
#define GA(T, A)

static void wrapSaxHandler(my_xmlSAXHandler_t* sav, my_xmlSAXHandler_t* v)
{
    if(!v) return;
    #define GO(T, A) sav->A=v->A; v->A=find_##T##_Fct(v->A);
    SUPER()
    #undef GO
}
static void restoreSaxHandler(my_xmlSAXHandler_t* sav, my_xmlSAXHandler_t* v)
{
    if(!v) return;
    #define GO(T, A) v->A = sav->A;
    SUPER()
    #undef GO
}
#undef GA

#undef SUPER

EXPORT int my_xmlParseDocument(x64emu_t* emu, my_xmlSAXHandler_t** p)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;
    // handling of wine that change the default sax handler of...
    my_xmlSAXHandler_t* old_saxhandler = p?(*p):NULL;
    my_xmlSAXHandler_t sax_handler = {0};
    wrapSaxHandler(&sax_handler, old_saxhandler);
    int ret = my->xmlParseDocument(p);
    restoreSaxHandler(&sax_handler, old_saxhandler);
    return ret;
}

EXPORT void* my_xmlCreateIOParserCtxt(x64emu_t* emu, my_xmlSAXHandler_t** p, void* user_data, void* ioread, void* ioclose, void* ioctx, int enc)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;
    // handling of wine that change the default sax handler of...
    my_xmlSAXHandler_t* old_saxhandler = p?(*p):NULL;
    my_xmlSAXHandler_t sax_handler = {0};
    wrapSaxHandler(&sax_handler, old_saxhandler);
    void* ret = my->xmlCreateIOParserCtxt(p, user_data, find_xmlInputReadCallback_Fct(ioread), find_xmlInputCloseCallback_Fct(ioclose), ioctx, enc);
    restoreSaxHandler(&sax_handler, old_saxhandler);
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXml2My(lib);    \
    my_lib = lib;

#define CUSTOM_FINI \
    freeXml2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"
