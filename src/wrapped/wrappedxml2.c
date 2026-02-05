#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

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
#include "myalign.h"

const char* xml2Name = "libxml2.so.2";
#define ALTNAME "libxml2.so"
#define ALTNAME2 "libxml2.so.16"

#define LIBNAME xml2


EXPORT uintptr_t my_xmlFree = 0;
EXPORT uintptr_t my_xmlMalloc = 0;
EXPORT uintptr_t my_xmlRealloc = 0;
EXPORT uintptr_t my_xmlMemStrdup = 0;

void my_wrap_xmlFree(void* p)
{
    if(my_xmlFree){
        RunFunctionFmt(my_xmlFree, "p", p);
        return;
    }
        free(p);
}
void* my_wrap_xmlMalloc(size_t s)
{
    if(my_xmlMalloc)
        return (void*)RunFunctionFmt(my_xmlMalloc, "L", s);
    else
        return malloc(s);
}
void* my_wrap_xmlRealloc(void* p, size_t s)
{
    if(my_xmlRealloc)
        return (void*)RunFunctionFmt(my_xmlRealloc, "pL", p, s);
    else
        return realloc(p, s);
}
void* my_wrap_xmlMemStrdup(void* p)
{
    if(my_xmlMemStrdup)
        return (void*)RunFunctionFmt(my_xmlMemStrdup, "p", p);
    else
        return strdup(p);
}

#define ADDED_FUNCTIONS() \

#include "generated/wrappedxml2types.h"

#define ADDED_INIT() \
    void** p;                                                                                               \
    p=dlsym(lib->w.lib, "xmlFree");                                                                         \
        my_xmlFree = (p && *p)?AddCheckBridge(lib->w.bridge, vFp, *p, 0, "my_wrap_xmlFree"):0;              \
        if(p) *p = my_wrap_xmlFree;                                                                         \
    p=dlsym(lib->w.lib, "xmlMalloc");                                                                       \
        my_xmlMalloc = (p && *p)?AddCheckBridge(lib->w.bridge, pFL, *p, 0, "my_wrap_xmlMalloc"):0;          \
        if(p) *p = my_wrap_xmlMalloc;                                                                       \
    p=dlsym(lib->w.lib, "xmlRealloc");                                                                      \
        my_xmlRealloc = (p && *p)?AddCheckBridge(lib->w.bridge, pFpL, *p, 0, "my_wrap_xmlRealloc"):0;       \
        if(p) *p = my_wrap_xmlRealloc;                                                                      \
    p=dlsym(lib->w.lib, "xmlMemStrdup");                                                                    \
        my_xmlMemStrdup = (p && *p)?AddCheckBridge(lib->w.bridge, pFp, *p, 0, "my_wrap_xmlMemStrdup"):0;    \
        if(p) *p = my_wrap_xmlMemStrdup;

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
GO(9)

// xmlHashCopier ...
#define GO(A)   \
static uintptr_t my_xmlHashCopier_fct_##A = 0;                                      \
static void* my_xmlHashCopier_##A(void* a, void* b)                                 \
{                                                                                   \
    return (void*)RunFunctionFmt(my_xmlHashCopier_fct_##A, "pp", a, b); \
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
static uintptr_t my_xmlHashDeallocator_fct_##A = 0;                         \
static void my_xmlHashDeallocator_##A(void* a, void* b)                     \
{                                                                           \
    RunFunctionFmt(my_xmlHashDeallocator_fct_##A, "pp", a, b);  \
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
static uintptr_t my_xmlHashScanner_fct_##A = 0;                             \
static void my_xmlHashScanner_##A(void* a, void* b, void* c)                \
{                                                                           \
    RunFunctionFmt(my_xmlHashScanner_fct_##A, "ppp", a, b, c);  \
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
static uintptr_t my_xmlHashScannerFull_fct_##A = 0;                                     \
static void my_xmlHashScannerFull_##A(void* a, void* b, void* c, void* c2, void* c3)    \
{                                                                                       \
    RunFunctionFmt(my_xmlHashScannerFull_fct_##A, "ppppp", a, b, c, c2, c3);\
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
static uintptr_t my_xmlCharEncodingInputFunc_fct_##A = 0;                                       \
static int my_xmlCharEncodingInputFunc_##A(void* a, void* b, void* c, void* d)                  \
{                                                                                               \
    return RunFunctionFmt(my_xmlCharEncodingInputFunc_fct_##A, "pppp", a, b, c, d); \
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
static uintptr_t my_xmlCharEncodingOutputFunc_fct_##A = 0;                                      \
static int my_xmlCharEncodingOutputFunc_##A(void* a, void* b, void* c, void* d)                 \
{                                                                                               \
    return RunFunctionFmt(my_xmlCharEncodingOutputFunc_fct_##A, "pppp", a, b, c, d);\
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
static uintptr_t my_xmlOutputWriteCallback_fct_##A = 0;                                     \
static int my_xmlOutputWriteCallback_##A(void* a, void* b, int c)                           \
{                                                                                           \
    return RunFunctionFmt(my_xmlOutputWriteCallback_fct_##A, "ppi", a, b, c);   \
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
static uintptr_t my_xmlOutputCloseCallback_fct_##A = 0;                             \
static int my_xmlOutputCloseCallback_##A(void* a)                                   \
{                                                                                   \
    return RunFunctionFmt(my_xmlOutputCloseCallback_fct_##A, "p", a);   \
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
static uintptr_t my_xmlInputMatchCallback_fct_##A = 0;                          \
static int my_xmlInputMatchCallback_##A(void* a)                                \
{                                                                               \
    return RunFunctionFmt(my_xmlInputMatchCallback_fct_##A, "p", a);\
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
static uintptr_t my_xmlInputOpenCallback_fct_##A = 0;                                   \
static void* my_xmlInputOpenCallback_##A(void* a)                                       \
{                                                                                       \
    return (void*)RunFunctionFmt(my_xmlInputOpenCallback_fct_##A, "p", a);  \
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
static uintptr_t my_xmlInputReadCallback_fct_##A = 0;                                   \
static int my_xmlInputReadCallback_##A(void* a, void* b, int c)                         \
{                                                                                       \
    return RunFunctionFmt(my_xmlInputReadCallback_fct_##A, "ppi", a, b, c); \
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
static uintptr_t my_xmlInputCloseCallback_fct_##A = 0;                              \
static int my_xmlInputCloseCallback_##A(void* a)                                    \
{                                                                                   \
    return RunFunctionFmt(my_xmlInputCloseCallback_fct_##A, "p", a);    \
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
    RunFunctionFmt(my_xmlSchemaValidityErrorFunc_fct_##A, "ppppppppp", a, b, c, d, e, f, g, h, i);                  \
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
    RunFunctionFmt(my_xmlSchemaValidityWarningFunc_fct_##A, "ppppppppp", a, b, c, d, e, f, g, h, i);                    \
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
// xmlXPathFunction ...
#define GO(A)   \
static uintptr_t my_xmlXPathFunction_fct_##A = 0;                       \
static void my_xmlXPathFunction_##A(void* a, int b)                     \
{                                                                       \
    RunFunctionFmt(my_xmlXPathFunction_fct_##A, "pi", a, b);\
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
static uintptr_t my_internalSubsetSAXFunc_fct_##A = 0;                                  \
static void my_internalSubsetSAXFunc_##A(void* a, void* b, void* c, void* d)            \
{                                                                                       \
    RunFunctionFmt(my_internalSubsetSAXFunc_fct_##A, "pppp", a, b, c, d);   \
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
static uintptr_t my_isStandaloneSAXFunc_fct_##A = 0;                            \
static int my_isStandaloneSAXFunc_##A(void* a)                                  \
{                                                                               \
    return RunFunctionFmt(my_isStandaloneSAXFunc_fct_##A, "p", a);  \
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
static uintptr_t my_hasInternalSubsetSAXFunc_fct_##A = 0;                           \
static int my_hasInternalSubsetSAXFunc_##A(void* a)                                 \
{                                                                                   \
    return RunFunctionFmt(my_hasInternalSubsetSAXFunc_fct_##A, "p", a); \
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
static uintptr_t my_hasExternalSubsetSAXFunc_fct_##A = 0;                           \
static int my_hasExternalSubsetSAXFunc_##A(void* a)                                 \
{                                                                                   \
    return RunFunctionFmt(my_hasExternalSubsetSAXFunc_fct_##A, "p", a); \
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
static uintptr_t my_resolveEntitySAXFunc_fct_##A = 0;                                           \
static void* my_resolveEntitySAXFunc_##A(void* a, void* b, void* c)                             \
{                                                                                               \
    return (void*)RunFunctionFmt(my_resolveEntitySAXFunc_fct_##A, "ppp", a, b, c);  \
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
static uintptr_t my_getEntitySAXFunc_fct_##A = 0;                                       \
static void* my_getEntitySAXFunc_##A(void* a, void* b)                                  \
{                                                                                       \
    return (void*)RunFunctionFmt(my_getEntitySAXFunc_fct_##A, "pp", a, b);  \
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
static uintptr_t my_entityDeclSAXFunc_fct_##A = 0;                                          \
static void my_entityDeclSAXFunc_##A(void* a, void* b, int c, void* d, void* e, void* f)    \
{                                                                                           \
    RunFunctionFmt(my_entityDeclSAXFunc_fct_##A, "ppippp", a, b, c, d, e, f);   \
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
static uintptr_t my_notationDeclSAXFunc_fct_##A = 0;                                \
static void my_notationDeclSAXFunc_##A(void* a, void* b, void* c, void* d)          \
{                                                                                   \
    RunFunctionFmt(my_notationDeclSAXFunc_fct_##A, "pppp", a, b, c, d); \
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
    RunFunctionFmt(my_attributeDeclSAXFunc_fct_##A, "pppiipp", a, b, c, d, e, f, g);    \
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
static uintptr_t my_elementDeclSAXFunc_fct_##A = 0;                                 \
static void my_elementDeclSAXFunc_##A(void* a, void* b, int c, void* d)             \
{                                                                                   \
    RunFunctionFmt(my_elementDeclSAXFunc_fct_##A, "ppip", a, b, c, d);  \
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
static uintptr_t my_unparsedEntityDeclSAXFunc_fct_##A = 0;                                      \
static void my_unparsedEntityDeclSAXFunc_##A(void* a, void* b, void* c, void* d, void* e)       \
{                                                                                               \
    RunFunctionFmt(my_unparsedEntityDeclSAXFunc_fct_##A, "ppppp", a, b, c, d, e);   \
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
static uintptr_t my_setDocumentLocatorSAXFunc_fct_##A = 0;                      \
static void my_setDocumentLocatorSAXFunc_##A(void* a, void* b)                  \
{                                                                               \
    RunFunctionFmt(my_setDocumentLocatorSAXFunc_fct_##A, "pp", a, b);\
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
static uintptr_t my_startDocumentSAXFunc_fct_##A = 0;                   \
static void my_startDocumentSAXFunc_##A(void* a)                        \
{                                                                       \
    RunFunctionFmt(my_startDocumentSAXFunc_fct_##A, "p", a);\
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
static uintptr_t my_endDocumentSAXFunc_fct_##A = 0;                     \
static void my_endDocumentSAXFunc_##A(void* a)                          \
{                                                                       \
    RunFunctionFmt(my_endDocumentSAXFunc_fct_##A, "p", a);  \
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
static uintptr_t my_startElementSAXFunc_fct_##A = 0;                            \
static void my_startElementSAXFunc_##A(void* a, void* b, void* c)               \
{                                                                               \
    RunFunctionFmt(my_startElementSAXFunc_fct_##A, "ppp", a, b, c); \
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
static uintptr_t my_endElementSAXFunc_fct_##A = 0;                          \
static void my_endElementSAXFunc_##A(void* a, void* b)                      \
{                                                                           \
    RunFunctionFmt(my_endElementSAXFunc_fct_##A, "pp", a, b);   \
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
static uintptr_t my_referenceSAXFunc_fct_##A = 0;                       \
static void my_referenceSAXFunc_##A(void* a, void* b)                   \
{                                                                       \
    RunFunctionFmt(my_referenceSAXFunc_fct_##A, "pp", a, b);\
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
static uintptr_t my_charactersSAXFunc_fct_##A = 0;                              \
static void my_charactersSAXFunc_##A(void* a, void* b, int c)                   \
{                                                                               \
    RunFunctionFmt(my_charactersSAXFunc_fct_##A, "ppi", a, b, c);   \
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
static uintptr_t my_ignorableWhitespaceSAXFunc_fct_##A = 0;                             \
static void my_ignorableWhitespaceSAXFunc_##A(void* a, void* b, int c)                  \
{                                                                                       \
    RunFunctionFmt(my_ignorableWhitespaceSAXFunc_fct_##A, "ppi", a, b, c);  \
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
static uintptr_t my_processingInstructionSAXFunc_fct_##A = 0;                               \
static void my_processingInstructionSAXFunc_##A(void* a, void* b, void* c)                  \
{                                                                                           \
    RunFunctionFmt(my_processingInstructionSAXFunc_fct_##A, "ppp", a, b, c);    \
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
static uintptr_t my_commentSAXFunc_fct_##A = 0;                         \
static void my_commentSAXFunc_##A(void* a, void* b)                     \
{                                                                       \
    RunFunctionFmt(my_commentSAXFunc_fct_##A, "pp", a, b);  \
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
static uintptr_t my_warningSAXFunc_fct_##A = 0; \
static void my_warningSAXFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j) \
{   \
    RunFunctionFmt(my_warningSAXFunc_fct_##A, "pppppppppp", a, b, c, d, e, f, g, h, i, j);  \
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
static uintptr_t my_errorSAXFunc_fct_##A = 0;   \
static void my_errorSAXFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j)   \
{   \
    RunFunctionFmt(my_errorSAXFunc_fct_##A, "pppppppppp", a, b, c, d, e, f, g, h, i, j);    \
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
static uintptr_t my_fatalErrorSAXFunc_fct_##A = 0;  \
static void my_fatalErrorSAXFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j)  \
{   \
    RunFunctionFmt(my_fatalErrorSAXFunc_fct_##A, "pppppppppp", a, b, c, d, e, f, g, h, i, j);   \
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
static uintptr_t my_getParameterEntitySAXFunc_fct_##A = 0;                                      \
static void* my_getParameterEntitySAXFunc_##A(void* a, void* b)                                 \
{                                                                                               \
    return (void*)RunFunctionFmt(my_getParameterEntitySAXFunc_fct_##A, "pp", a, b); \
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
static uintptr_t my_cdataBlockSAXFunc_fct_##A = 0;                              \
static void my_cdataBlockSAXFunc_##A(void* a, void* b, int c)                   \
{                                                                               \
    RunFunctionFmt(my_cdataBlockSAXFunc_fct_##A, "ppi", a, b, c);   \
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
static uintptr_t my_externalSubsetSAXFunc_fct_##A = 0;                                  \
static void my_externalSubsetSAXFunc_##A(void* a, void* b, void* c, void* d)            \
{                                                                                       \
    RunFunctionFmt(my_externalSubsetSAXFunc_fct_##A, "pppp", a, b, c, d);   \
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
    RunFunctionFmt(my_xmlSAX2StartElementNs_fct_##A, "ppppipiip", a, b, c, d, e, f, g, h, i);           \
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
static uintptr_t my_xmlSAX2EndElementNs_fct_##A = 0;                                \
static void my_xmlSAX2EndElementNs_##A(void* a, void* b, void* c, void* d)          \
{                                                                                   \
    RunFunctionFmt(my_xmlSAX2EndElementNs_fct_##A, "pppp", a, b, c, d); \
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
static uintptr_t my_xmlExternalEntityLoader_fct_##A = 0;                                            \
static void* my_xmlExternalEntityLoader_##A(void* a, void* b, void* c)                              \
{                                                                                                   \
    return (void*)RunFunctionFmt(my_xmlExternalEntityLoader_fct_##A, "ppp", a, b, c);   \
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
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_xmlExternalEntityLoader_##A == fct) return (void*)my_xmlExternalEntityLoader_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFppp, fct, 0, "xmlExternalEntityLoader_callback");
}


// xmlGenericErrorFunc
#define GO(A)   \
static uintptr_t my_xmlGenericErrorFunc_fct_##A = 0;                            \
static void my_xmlGenericErrorFunc_##A(void* a, const char* fmt, ...)           \
{                                                                               \
    char buf[4096];                                                             \
    va_list args;                                                               \
    va_start(args, fmt);                                                        \
    vsnprintf(buf, 4096, fmt, args);                                            \
    va_end(args);                                                               \
    RunFunctionFmt(my_xmlGenericErrorFunc_fct_##A, "pp", a, buf);   \
}
SUPER()
#undef GO
static void* find_xmlGenericErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlGenericErrorFunc_fct_##A == (uintptr_t)fct) return my_xmlGenericErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlGenericErrorFunc_fct_##A == 0) {my_xmlGenericErrorFunc_fct_##A = (uintptr_t)fct; return my_xmlGenericErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlGenericErrorFunc callback\n");
    return NULL;
}
static void* reverse_xmlGenericErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_xmlGenericErrorFunc_##A == fct) return (void*)my_xmlGenericErrorFunc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpp, fct, 0, "xmlGenericErrorFunc_callback");
}

// xmlStructuredErrorFunc
#define GO(A)   \
static uintptr_t my_xmlStructuredErrorFunc_fct_##A = 0;                         \
static void my_xmlStructuredErrorFunc_##A(void* a, const char* b)               \
{                                                                               \
    RunFunctionFmt(my_xmlStructuredErrorFunc_fct_##A, "pp", a, b);  \
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
static void* reverse_xmlStructuredErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_xmlStructuredErrorFunc_##A == fct) return (void*)my_xmlStructuredErrorFunc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpp, fct, 0, "xmlStructuredErrorFunc_callback");
}

// xmlOutputMatchCallback ...
#define GO(A)   \
static uintptr_t my_xmlOutputMatchCallback_fct_##A = 0;                                 \
static int my_xmlOutputMatchCallback_##A(void* a)                                       \
{                                                                                       \
    return (int)RunFunctionFmt(my_xmlOutputMatchCallback_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_xmlOutputMatchCallback_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlOutputMatchCallback_fct_##A == (uintptr_t)fct) return my_xmlOutputMatchCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlOutputMatchCallback_fct_##A == 0) {my_xmlOutputMatchCallback_fct_##A = (uintptr_t)fct; return my_xmlOutputMatchCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlOutputMatchCallback callback\n");
    return NULL;
}

// xmlOutputOpenCallback ...
#define GO(A)   \
static uintptr_t my_xmlOutputOpenCallback_fct_##A = 0;                                  \
static void* my_xmlOutputOpenCallback_##A(void* a)                                      \
{                                                                                       \
    return (void*)RunFunctionFmt(my_xmlOutputOpenCallback_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_xmlOutputOpenCallback_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlOutputOpenCallback_fct_##A == (uintptr_t)fct) return my_xmlOutputOpenCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlOutputOpenCallback_fct_##A == 0) {my_xmlOutputOpenCallback_fct_##A = (uintptr_t)fct; return my_xmlOutputOpenCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlOutputOpenCallback callback\n");
    return NULL;
}

// xmlTextReaderErrorFunc ...
#define GO(A)   \
static uintptr_t my_xmlTextReaderErrorFunc_fct_##A = 0;                                 \
static void my_xmlTextReaderErrorFunc_##A(void* a, void* b, int c, void* d)             \
{                                                                                       \
    RunFunctionFmt(my_xmlTextReaderErrorFunc_fct_##A, "ppip", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_xmlTextReaderErrorFunc_Fct(void* fct) // this one have a VAArg
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlTextReaderErrorFunc_fct_##A == (uintptr_t)fct) return my_xmlTextReaderErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlTextReaderErrorFunc_fct_##A == 0) {my_xmlTextReaderErrorFunc_fct_##A = (uintptr_t)fct; return my_xmlTextReaderErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlTextReaderErrorFunc callback\n");
    return NULL;
}

// xmlFreeFunc ...
#define GO(A)   \
static uintptr_t my_xmlFreeFunc_fct_##A = 0;                      \
static void my_xmlFreeFunc_##A(void* a)                          \
{                                                                 \
    return (void)RunFunctionFmt(my_xmlFreeFunc_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_xmlFreeFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlFreeFunc_fct_##A == (uintptr_t)fct) return my_xmlFreeFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlFreeFunc_fct_##A == 0) {my_xmlFreeFunc_fct_##A = (uintptr_t)fct; return my_xmlFreeFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlFreeFunc callback\n");
    return NULL;
}

// xmlMallocFunc ...
#define GO(A)   \
static uintptr_t my_xmlMallocFunc_fct_##A = 0;                      \
static void* my_xmlMallocFunc_##A(size_t a)                         \
{                                                                   \
    return (void*)RunFunctionFmt(my_xmlMallocFunc_fct_##A, "L", a); \
}
SUPER()
#undef GO
static void* find_xmlMallocFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlMallocFunc_fct_##A == (uintptr_t)fct) return my_xmlMallocFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlMallocFunc_fct_##A == 0) {my_xmlMallocFunc_fct_##A = (uintptr_t)fct; return my_xmlMallocFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlMallocFunc callback\n");
    return NULL;
}

// xmlReallocFunc ...
#define GO(A)   \
static uintptr_t my_xmlReallocFunc_fct_##A = 0;                          \
static void* my_xmlReallocFunc_##A(void* a, size_t b)                    \
{                                                                        \
    return (void*)RunFunctionFmt(my_xmlReallocFunc_fct_##A, "pL", a, b); \
}
SUPER()
#undef GO
static void* find_xmlReallocFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlReallocFunc_fct_##A == (uintptr_t)fct) return my_xmlReallocFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlReallocFunc_fct_##A == 0) {my_xmlReallocFunc_fct_##A = (uintptr_t)fct; return my_xmlReallocFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlReallocFunc callback\n");
    return NULL;
}

// xmlStrdupFunc ...
#define GO(A)   \
static uintptr_t my_xmlStrdupFunc_fct_##A = 0;                      \
static char* my_xmlStrdupFunc_##A(char* a)                          \
{                                                                   \
    return (char*)RunFunctionFmt(my_xmlStrdupFunc_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_xmlStrdupFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlStrdupFunc_fct_##A == (uintptr_t)fct) return my_xmlStrdupFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlStrdupFunc_fct_##A == 0) {my_xmlStrdupFunc_fct_##A = (uintptr_t)fct; return my_xmlStrdupFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlStrdupFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_xmlHashCopy(x64emu_t* emu, void* table, void* f)
{
    return my->xmlHashCopy(table, find_xmlHashCopier_Fct(f));
}

EXPORT void my_xmlHashFree(x64emu_t* emu, void* table, void* f)
{
    my->xmlHashFree(table, find_xmlHashDeallocator_Fct(f));
}

EXPORT int my_xmlHashRemoveEntry(x64emu_t* emu, void* table, void* name, void* f)
{
    return my->xmlHashRemoveEntry(table, name, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashRemoveEntry2(x64emu_t* emu, void* table, void* name, void* name2, void* f)
{
    return my->xmlHashRemoveEntry2(table, name, name2, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashRemoveEntry3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* f)
{
    return my->xmlHashRemoveEntry3(table, name, name2, name3, find_xmlHashDeallocator_Fct(f));
}

EXPORT void my_xmlHashScan(x64emu_t* emu, void* table, void* f, void* data)
{
    my->xmlHashScan(table, find_xmlHashScanner_Fct(f), data);
}
EXPORT void my_xmlHashScan3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* f, void* data)
{
    my->xmlHashScan3(table, name, name2, name3, find_xmlHashScanner_Fct(f), data);
}
EXPORT void my_xmlHashScanFull(x64emu_t* emu, void* table, void* f, void* data)
{
    my->xmlHashScanFull(table, find_xmlHashScannerFull_Fct(f), data);
}
EXPORT void my_xmlHashScanFull3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* f, void* data)
{
    my->xmlHashScanFull3(table, name, name2, name3, find_xmlHashScannerFull_Fct(f), data);
}

EXPORT int my_xmlHashUpdateEntry(x64emu_t* emu, void* table, void* name, void* data, void* f)
{
    return my->xmlHashUpdateEntry(table, name, data, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashUpdateEntry2(x64emu_t* emu, void* table, void* name, void* name2, void* data, void* f)
{
    return my->xmlHashUpdateEntry2(table, name, name2, data, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashUpdateEntry3(x64emu_t* emu, void* table, void* name, void* name2, void* name3, void* data, void* f)
{
    return my->xmlHashUpdateEntry3(table, name, name2, name3, data, find_xmlHashDeallocator_Fct(f));
}

EXPORT void* my_xmlGetExternalEntityLoader(x64emu_t* emu)
{
    return reverse_xmlExternalEntityLoaderFct(my->xmlGetExternalEntityLoader());
}

EXPORT void* my_xmlNewCharEncodingHandler(x64emu_t* emu, void* name, void* fin, void* fout)
{
    return my->xmlNewCharEncodingHandler(name, find_xmlCharEncodingInputFunc_Fct(fin), find_xmlCharEncodingOutputFunc_Fct(fout));
}

EXPORT void* my_xmlOutputBufferCreateIO(x64emu_t* emu, void* fwrite, void* fclose, void* ioctx, void* encoder)
{
    return my->xmlOutputBufferCreateIO(find_xmlOutputWriteCallback_Fct(fwrite), find_xmlOutputCloseCallback_Fct(fclose), ioctx, encoder);
}

EXPORT int my_xmlRegisterInputCallbacks(x64emu_t* emu, void* fmatch, void* fop, void* frd, void* fcl)
{
    return my->xmlRegisterInputCallbacks(find_xmlInputMatchCallback_Fct(fmatch), find_xmlInputOpenCallback_Fct(fop), find_xmlInputReadCallback_Fct(frd), find_xmlInputCloseCallback_Fct(fcl));
}

EXPORT void* my_xmlSaveToIO(x64emu_t* emu, void* fwrt, void* fcl, void* ioctx, void* encoding, int options)
{
    return my->xmlSaveToIO(find_xmlOutputWriteCallback_Fct(fwrt), find_xmlOutputCloseCallback_Fct(fcl), ioctx, encoding, options);
}

EXPORT void my_xmlSchemaSetParserErrors(x64emu_t* emu, void* ctxt, void* ferr, void* fwarn, void* ctx)
{
    my->xmlSchemaSetParserErrors(ctxt, find_xmlSchemaValidityErrorFunc_Fct(ferr), find_xmlSchemaValidityWarningFunc_Fct(fwarn), ctx);
}

EXPORT void my_xmlSchemaSetParserStructuredErrors(x64emu_t* emu, void* ctxt, void* ferr, void* ctx)
{
    my->xmlSchemaSetParserStructuredErrors(ctxt, find_xmlStructuredErrorFunc_Fct(ferr), ctx);
}

EXPORT void my_xmlSchemaSetValidErrors(x64emu_t* emu, void* ctxt, void* ferr, void* fwarn, void* ctx)
{
    my->xmlSchemaSetValidErrors(ctxt, find_xmlSchemaValidityErrorFunc_Fct(ferr), find_xmlSchemaValidityWarningFunc_Fct(fwarn), ctx);
}

EXPORT void my_xmlSchemaSetValidStructuredErrors(x64emu_t* emu, void* ctxt, void* ferr, void* ctx)
{
    my->xmlSchemaSetValidStructuredErrors(ctxt, find_xmlStructuredErrorFunc_Fct(ferr), ctx);
}

EXPORT void my_xmlSetExternalEntityLoader(x64emu_t* emu, void* f)
{
    my->xmlSetExternalEntityLoader(find_xmlExternalEntityLoaderFct(f));
}

EXPORT int my_xmlXPathRegisterFunc(x64emu_t* emu, void* ctxt, void* name, void* f)
{
    return my->xmlXPathRegisterFunc(ctxt, name, find_xmlXPathFunction_Fct(f));
}

EXPORT void* my_xmlParserInputBufferCreateIO(x64emu_t* emu, void* ioread, void* ioclose, void* ioctx, int enc)
{
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

EXPORT void* my_xmlCreatePushParserCtxt(x64emu_t* emu, my_xmlSAXHandler_t* p, void* user_data, void* chunk,
					            int size, void* filename)
{
    // handling of wine that change the default sax handler of...
    my_xmlSAXHandler_t* old_saxhandler = p;
    my_xmlSAXHandler_t sax_handler = {0};
    wrapSaxHandler(&sax_handler, old_saxhandler);
    void* ret = my->xmlCreatePushParserCtxt(p, user_data, chunk, size, filename);
    restoreSaxHandler(&sax_handler, old_saxhandler);
    return ret;
}

EXPORT int my_xmlParseDocument(x64emu_t* emu, my_xmlSAXHandler_t** p)
{
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
    // handling of wine that change the default sax handler of...
    my_xmlSAXHandler_t* old_saxhandler = p?(*p):NULL;
    my_xmlSAXHandler_t sax_handler = {0};
    wrapSaxHandler(&sax_handler, old_saxhandler);
    void* ret = my->xmlCreateIOParserCtxt(p, user_data, find_xmlInputReadCallback_Fct(ioread), find_xmlInputCloseCallback_Fct(ioclose), ioctx, enc);
    restoreSaxHandler(&sax_handler, old_saxhandler);
    return ret;
}

EXPORT void*  my___xmlGenericError(x64emu_t* emu)
{
    return reverse_xmlGenericErrorFunc_Fct(my->__xmlGenericError());
}

EXPORT void* my___xmlStructuredError(x64emu_t* emu)
{
    return reverse_xmlStructuredErrorFunc_Fct(my->__xmlStructuredError());
}

EXPORT void my_xmlSetGenericErrorFunc(x64emu_t* emu, void* ctx, void* f)
{
    my->xmlSetGenericErrorFunc(ctx, find_xmlGenericErrorFunc_Fct(f));
}

EXPORT void my_xmlSetStructuredErrorFunc(x64emu_t* emu, void* ctx, void* f)
{
    my->xmlSetStructuredErrorFunc(ctx, find_xmlStructuredErrorFunc_Fct(f));
}

EXPORT int my_xmlRegisterOutputCallbacks(x64emu_t* emu, void* match, void* open, void* write, void* close)
{
    return my->xmlRegisterOutputCallbacks(find_xmlOutputMatchCallback_Fct(match), find_xmlOutputOpenCallback_Fct(open), find_xmlOutputWriteCallback_Fct(write), find_xmlOutputCloseCallback_Fct(close));
}

EXPORT int my_xmlTextWriterWriteVFormatAttribute(x64emu_t* emu, void* writer, void* name, void* fmt, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return my->xmlTextWriterWriteVFormatAttribute(writer, name, fmt, VARARGS);
}

EXPORT int my_xmlTextWriterWriteFormatAttribute(x64emu_t* emu, void* writer, void* name, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->xmlTextWriterWriteVFormatAttribute(writer, name, fmt, VARARGS);
}

EXPORT void* my_xmlReaderForIO(void *emu, void * ioread, void * ioclose, void * ioctx, char * URL, char * encoding, int  options)
{
    return my->xmlReaderForIO(find_xmlInputReadCallback_Fct(ioread), find_xmlInputCloseCallback_Fct(ioclose), ioctx, URL, encoding, options);
}

EXPORT void my_xmlTextReaderSetErrorHandler(x64emu_t* emu, void* reader, void* f, void* arg)
{
    my->xmlTextReaderSetErrorHandler(reader, find_xmlTextReaderErrorFunc_Fct(f), arg);
}

EXPORT void* my_xmlCtxtReadIO(x64emu_t* emu, void* ctxt, void* ioread, void* ioclose, void* ioctx, void* url, void* encoding, int options)
{
    return my->xmlCtxtReadIO(ctxt, find_xmlInputReadCallback_Fct(ioread), find_xmlInputCloseCallback_Fct(ioclose), ioctx, url, encoding, options);
}

EXPORT int my_xmlMemSetup(x64emu_t* emu, void* v1, void* v2, void* v3, void* v4)
{
    int ret = my->xmlMemSetup(find_xmlFreeFunc_Fct(v1), find_xmlMallocFunc_Fct(v2), find_xmlReallocFunc_Fct(v3), find_xmlStrdupFunc_Fct(v4));
    return ret;
}

#include "wrappedlib_init.h"
