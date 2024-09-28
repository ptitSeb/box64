#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <ucontext.h>
#include <sys/mman.h>

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
#include "myalign32.h"

static const char* expatName = "libexpat.so.1";
#define LIBNAME expat

#include "generated/wrappedexpattypes32.h"

#include "wrappercallback32.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// StartElementHandler ...
#define GO(A)   \
static uintptr_t my_StartElementHandler_fct_##A = 0;                                                \
static void* my_StartElementHandler_##A(void* data, void* name, void** attr)                        \
{                                                                                                   \
    int cnt = 0;                                                                                    \
    while(attr[cnt]) cnt+=2;                                                                        \
    ++cnt;                                                                                          \
    ptr_t attr_s[cnt];                                                                              \
    for(int i=0; i<cnt; ++i) attr_s[i] = to_ptrv(attr[i]);                                          \
    return (void*)RunFunctionFmt(my_StartElementHandler_fct_##A, "ppp", data, name, attr_s);        \
}
SUPER()
#undef GO
static void* find_StartElementHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_StartElementHandler_fct_##A == (uintptr_t)fct) return my_StartElementHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_StartElementHandler_fct_##A == 0) {my_StartElementHandler_fct_##A = (uintptr_t)fct; return my_StartElementHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat StartElementHandler callback\n");
    return NULL;
}
// XML_StartDoctypeDecl ...
#define GO(A)   \
static uintptr_t my_XML_StartDoctypeDecl_fct_##A = 0;                                                               \
static void* my_XML_StartDoctypeDecl_##A(void* data, void* name, void* sysid, void* pubid, int has_internal)        \
{                                                                                                                   \
    return (void*)RunFunctionFmt(my_XML_StartDoctypeDecl_fct_##A, "ppppi", data, name, sysid, pubid, has_internal); \
}
SUPER()
#undef GO
static void* find_XML_StartDoctypeDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XML_StartDoctypeDecl_fct_##A == (uintptr_t)fct) return my_XML_StartDoctypeDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XML_StartDoctypeDecl_fct_##A == 0) {my_XML_StartDoctypeDecl_fct_##A = (uintptr_t)fct; return my_XML_StartDoctypeDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat XML_StartDoctypeDecl callback\n");
    return NULL;
}
// Start ...
#define GO(A)   \
static uintptr_t my_Start_fct_##A = 0;                                                      \
static void* my_Start_##A(void* data, void* name, void* attr)                               \
{                                                                                           \
    return (void*)RunFunctionFmt(my_Start_fct_##A, "ppp", data, name, attr);          \
}
SUPER()
#undef GO
static void* find_Start_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Start_fct_##A == (uintptr_t)fct) return my_Start_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Start_fct_##A == 0) {my_Start_fct_##A = (uintptr_t)fct; return my_Start_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat Start callback\n");
    return NULL;
}
// XML_EndDoctypeDecl ...
#define GO(A)   \
static uintptr_t my_XML_EndDoctypeDecl_fct_##A = 0;             \
static void my_XML_EndDoctypeDecl_##A(void* data)               \
{                                                               \
    RunFunctionFmt(my_XML_EndDoctypeDecl_fct_##A, "p", data);   \
}
SUPER()
#undef GO
static void* find_XML_EndDoctypeDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XML_EndDoctypeDecl_fct_##A == (uintptr_t)fct) return my_XML_EndDoctypeDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XML_EndDoctypeDecl_fct_##A == 0) {my_XML_EndDoctypeDecl_fct_##A = (uintptr_t)fct; return my_XML_EndDoctypeDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat XML_EndDoctypeDecl callback\n");
    return NULL;
}
// End ...
#define GO(A)   \
static uintptr_t my_End_fct_##A = 0;                                \
static void my_End_##A(void* data, void* name)                      \
{                                                                   \
    RunFunctionFmt(my_End_fct_##A, "pp", data, name);         \
}
SUPER()
#undef GO
static void* find_End_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_End_fct_##A == (uintptr_t)fct) return my_End_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_End_fct_##A == 0) {my_End_fct_##A = (uintptr_t)fct; return my_End_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat End callback\n");
    return NULL;
}
// CharData ...
#define GO(A)   \
static uintptr_t my_CharData_fct_##A = 0;                               \
static void my_CharData_##A(void* data, void* s, int l)                 \
{                                                                       \
    RunFunctionFmt(my_CharData_fct_##A, "ppi", data, s, l);       \
}
SUPER()
#undef GO
static void* find_CharData_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CharData_fct_##A == (uintptr_t)fct) return my_CharData_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CharData_fct_##A == 0) {my_CharData_fct_##A = (uintptr_t)fct; return my_CharData_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat CharData callback\n");
    return NULL;
}
// StartNamespaceDecl ...
#define GO(A)   \
static uintptr_t my_StartNamespaceDecl_fct_##A = 0;                                     \
static void my_StartNamespaceDecl_##A(void* data, void* name, void* attr)               \
{                                                                                       \
    RunFunctionFmt(my_StartNamespaceDecl_fct_##A, "ppp", data, name, attr);       \
}
SUPER()
#undef GO
static void* find_StartNamespaceDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_StartNamespaceDecl_fct_##A == (uintptr_t)fct) return my_StartNamespaceDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_StartNamespaceDecl_fct_##A == 0) {my_StartNamespaceDecl_fct_##A = (uintptr_t)fct; return my_StartNamespaceDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat StartNamespaceDecl callback\n");
    return NULL;
}
// EndNamespaceDecl ...
#define GO(A)   \
static uintptr_t my_EndNamespaceDecl_fct_##A = 0;                               \
static void my_EndNamespaceDecl_##A(void* data, void* name)                     \
{                                                                               \
    RunFunctionFmt(my_EndNamespaceDecl_fct_##A, "pp", data, name);        \
}
SUPER()
#undef GO
static void* find_EndNamespaceDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EndNamespaceDecl_fct_##A == (uintptr_t)fct) return my_EndNamespaceDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EndNamespaceDecl_fct_##A == 0) {my_EndNamespaceDecl_fct_##A = (uintptr_t)fct; return my_EndNamespaceDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat EndNamespaceDecl callback\n");
    return NULL;
}
// StartElement ...
#define GO(A)   \
static uintptr_t my_StartElement_fct_##A = 0;                                       \
static void my_StartElement_##A(void* data, void* name, void* attr)                 \
{                                                                                   \
    RunFunctionFmt(my_StartElement_fct_##A, "ppp", data, name, attr);         \
}
SUPER()
#undef GO
static void* find_StartElement_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_StartElement_fct_##A == (uintptr_t)fct) return my_StartElement_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_StartElement_fct_##A == 0) {my_StartElement_fct_##A = (uintptr_t)fct; return my_StartElement_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat StartElement callback\n");
    return NULL;
}
// EndElement ...
#define GO(A)   \
static uintptr_t my_EndElement_fct_##A = 0;                                 \
static void my_EndElement_##A(void* data, void* name)                       \
{                                                                           \
    RunFunctionFmt(my_EndElement_fct_##A, "pp", data, name);          \
}
SUPER()
#undef GO
static void* find_EndElement_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EndElement_fct_##A == (uintptr_t)fct) return my_EndElement_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EndElement_fct_##A == 0) {my_EndElement_fct_##A = (uintptr_t)fct; return my_EndElement_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat EndElement callback\n");
    return NULL;
}
// ProcessingInstruction ...
#define GO(A)   \
static uintptr_t my_ProcessingInstruction_fct_##A = 0;                            \
static void my_ProcessingInstruction_##A(void* a, void* b, void* c)               \
{                                                                                 \
    RunFunctionFmt(my_ProcessingInstruction_fct_##A, "ppp", a, b, c);         \
}
SUPER()
#undef GO
static void* find_ProcessingInstruction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ProcessingInstruction_fct_##A == (uintptr_t)fct) return my_ProcessingInstruction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ProcessingInstruction_fct_##A == 0) {my_ProcessingInstruction_fct_##A = (uintptr_t)fct; return my_ProcessingInstruction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat ProcessingInstruction callback\n");
    return NULL;
}
// Comment ...
#define GO(A)   \
static uintptr_t my_Comment_fct_##A = 0;                        \
static void my_Comment_##A(void* a, void* b)                    \
{                                                               \
    RunFunctionFmt(my_Comment_fct_##A, "pp", a, b);       \
}
SUPER()
#undef GO
static void* find_Comment_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Comment_fct_##A == (uintptr_t)fct) return my_Comment_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Comment_fct_##A == 0) {my_Comment_fct_##A = (uintptr_t)fct; return my_Comment_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat Comment callback\n");
    return NULL;
}
// StartCdataSection ...
#define GO(A)   \
static uintptr_t my_StartCdataSection_fct_##A = 0;                       \
static void my_StartCdataSection_##A(void* data)                         \
{                                                                        \
    RunFunctionFmt(my_StartCdataSection_fct_##A, "p", data);      \
}
SUPER()
#undef GO
static void* find_StartCdataSection_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_StartCdataSection_fct_##A == (uintptr_t)fct) return my_StartCdataSection_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_StartCdataSection_fct_##A == 0) {my_StartCdataSection_fct_##A = (uintptr_t)fct; return my_StartCdataSection_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat StartCdataSection callback\n");
    return NULL;
}
// EndCdataSection ...
#define GO(A)   \
static uintptr_t my_EndCdataSection_fct_##A = 0;                       \
static void my_EndCdataSection_##A(void* data)                         \
{                                                                      \
    RunFunctionFmt(my_EndCdataSection_fct_##A, "p", data);        \
}
SUPER()
#undef GO
static void* find_EndCdataSection_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EndCdataSection_fct_##A == (uintptr_t)fct) return my_EndCdataSection_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EndCdataSection_fct_##A == 0) {my_EndCdataSection_fct_##A = (uintptr_t)fct; return my_EndCdataSection_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat EndCdataSection callback\n");
    return NULL;
}
// Default ...
#define GO(A)   \
static uintptr_t my_Default_fct_##A = 0;                                    \
static void my_Default_##A(void* data, void* name, int len)                 \
{                                                                           \
    RunFunctionFmt(my_Default_fct_##A, "ppi", data, name, len);       \
}
SUPER()
#undef GO
static void* find_Default_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Default_fct_##A == (uintptr_t)fct) return my_Default_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Default_fct_##A == 0) {my_Default_fct_##A = (uintptr_t)fct; return my_Default_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat Default callback\n");
    return NULL;
}
// StartDoctypeDecl ...
#define GO(A)   \
static uintptr_t my_StartDoctypeDecl_fct_##A = 0;                                    \
static void my_StartDoctypeDecl_##A(void* a, void* b, void* c, void* d, int e)       \
{                                                                                    \
    RunFunctionFmt(my_StartDoctypeDecl_fct_##A, "ppppi", a, b, c, d, e);      \
}
SUPER()
#undef GO
static void* find_StartDoctypeDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_StartDoctypeDecl_fct_##A == (uintptr_t)fct) return my_StartDoctypeDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_StartDoctypeDecl_fct_##A == 0) {my_StartDoctypeDecl_fct_##A = (uintptr_t)fct; return my_StartDoctypeDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat StartDoctypeDecl callback\n");
    return NULL;
}
// EndDoctypeDecl ...
#define GO(A)   \
static uintptr_t my_EndDoctypeDecl_fct_##A = 0;                       \
static void my_EndDoctypeDecl_##A(void* data)                         \
{                                                                     \
    RunFunctionFmt(my_EndDoctypeDecl_fct_##A, "p", data);         \
}
SUPER()
#undef GO
static void* find_EndDoctypeDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EndDoctypeDecl_fct_##A == (uintptr_t)fct) return my_EndDoctypeDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EndDoctypeDecl_fct_##A == 0) {my_EndDoctypeDecl_fct_##A = (uintptr_t)fct; return my_EndDoctypeDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat EndDoctypeDecl callback\n");
    return NULL;
}
// EntityDecl ...
#define GO(A)   \
static uintptr_t my_EntityDecl_fct_##A = 0;                                                                \
static void my_EntityDecl_##A(void* a, void* b, int c, void* d, int e, void* f, void* g, void* h, void* i) \
{                                                                                                          \
    RunFunctionFmt(my_EntityDecl_fct_##A, "ppipipppp", a, b, c, d, e, f, g, h, i);                    \
}
SUPER()
#undef GO
static void* find_EntityDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EntityDecl_fct_##A == (uintptr_t)fct) return my_EntityDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EntityDecl_fct_##A == 0) {my_EntityDecl_fct_##A = (uintptr_t)fct; return my_EntityDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat EntityDecl callback\n");
    return NULL;
}
// ElementDecl ...
#define GO(A)   \
static uintptr_t my_ElementDecl_fct_##A = 0;                            \
static void my_ElementDecl_##A(void* a, void* b, void* c)               \
{                                                                       \
    RunFunctionFmt(my_ElementDecl_fct_##A, "ppp", a, b, c);       \
}
SUPER()
#undef GO
static void* find_ElementDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ElementDecl_fct_##A == (uintptr_t)fct) return my_ElementDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ElementDecl_fct_##A == 0) {my_ElementDecl_fct_##A = (uintptr_t)fct; return my_ElementDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat ElementDecl callback\n");
    return NULL;
}
// UnknownEncoding ...
#define GO(A)   \
static uintptr_t my_UnknownEncoding_fct_##A = 0;                                        \
static int my_UnknownEncoding_##A(void* a, void* b, void* c)                            \
{                                                                                       \
    return (int)RunFunctionFmt(my_UnknownEncoding_fct_##A, "ppp", a, b, c);       \
}
SUPER()
#undef GO
static void* find_UnknownEncoding_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_UnknownEncoding_fct_##A == (uintptr_t)fct) return my_UnknownEncoding_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_UnknownEncoding_fct_##A == 0) {my_UnknownEncoding_fct_##A = (uintptr_t)fct; return my_UnknownEncoding_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat UnknownEncoding callback\n");
    return NULL;
}
// UnparsedEntityDecl ...
#define GO(A)   \
static uintptr_t my_UnparsedEntityDecl_fct_##A = 0;                                         \
static void my_UnparsedEntityDecl_##A(void* a, void* b, void* c, void* d, void* e, void* f) \
{                                                                                           \
    RunFunctionFmt(my_UnparsedEntityDecl_fct_##A, "pppppp", a, b, c, d, e, f);        \
}
SUPER()
#undef GO
static void* find_UnparsedEntityDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_UnparsedEntityDecl_fct_##A == (uintptr_t)fct) return my_UnparsedEntityDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_UnparsedEntityDecl_fct_##A == 0) {my_UnparsedEntityDecl_fct_##A = (uintptr_t)fct; return my_UnparsedEntityDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat UnparsedEntityDecl callback\n");
    return NULL;
}
// NotationDecl ...
#define GO(A)   \
static uintptr_t my_NotationDecl_fct_##A = 0;                                    \
static void my_NotationDecl_##A(void* a, void* b, void* c, void* d, void* e)     \
{                                                                                \
    RunFunctionFmt(my_NotationDecl_fct_##A, "ppppp", a, b, c, d, e);      \
}
SUPER()
#undef GO
static void* find_NotationDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_NotationDecl_fct_##A == (uintptr_t)fct) return my_NotationDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_NotationDecl_fct_##A == 0) {my_NotationDecl_fct_##A = (uintptr_t)fct; return my_NotationDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat NotationDecl callback\n");
    return NULL;
}
// NotStandalone ...
#define GO(A)   \
static uintptr_t my_NotStandalone_fct_##A = 0;                                   \
static int my_NotStandalone_##A(void* data)                                      \
{                                                                                \
    return (int)RunFunctionFmt(my_NotStandalone_fct_##A, "p", data);      \
}
SUPER()
#undef GO
static void* find_NotStandalone_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_NotStandalone_fct_##A == (uintptr_t)fct) return my_NotStandalone_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_NotStandalone_fct_##A == 0) {my_NotStandalone_fct_##A = (uintptr_t)fct; return my_NotStandalone_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat NotStandalone callback\n");
    return NULL;
}
// ExternalEntityRef ...
#define GO(A)   \
static uintptr_t my_ExternalEntityRef_fct_##A = 0;                                                \
static int my_ExternalEntityRef_##A(void* a, void* b, void* c, void* d, void* e)                  \
{                                                                                                 \
    return (int)RunFunctionFmt(my_ExternalEntityRef_fct_##A, "ppppp", a, b, c, d, e);         \
}
SUPER()
#undef GO
static void* find_ExternalEntityRef_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ExternalEntityRef_fct_##A == (uintptr_t)fct) return my_ExternalEntityRef_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ExternalEntityRef_fct_##A == 0) {my_ExternalEntityRef_fct_##A = (uintptr_t)fct; return my_ExternalEntityRef_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat ExternalEntityRef callback\n");
    return NULL;
}
// XmlDecl ...
#define GO(A)   \
static uintptr_t my_XmlDecl_fct_##A = 0;                                \
static void my_XmlDecl_##A(void* a, void* b, void* c, int d)            \
{                                                                       \
    RunFunctionFmt(my_XmlDecl_fct_##A, "pppi", a, b, c, d);       \
}
SUPER()
#undef GO
static void* find_XmlDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XmlDecl_fct_##A == (uintptr_t)fct) return my_XmlDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XmlDecl_fct_##A == 0) {my_XmlDecl_fct_##A = (uintptr_t)fct; return my_XmlDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat XmlDecl callback\n");
    return NULL;
}
// AttlistDecl ...
#define GO(A)   \
static uintptr_t my_AttlistDecl_fct_##A = 0;                                        \
static void my_AttlistDecl_##A(void* a, void* b, void* c, void* d, void* e, int f)  \
{                                                                                   \
    RunFunctionFmt(my_AttlistDecl_fct_##A, "pppppi", a, b, c, d, e, f);       \
}
SUPER()
#undef GO
static void* find_AttlistDecl_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AttlistDecl_fct_##A == (uintptr_t)fct) return my_AttlistDecl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AttlistDecl_fct_##A == 0) {my_AttlistDecl_fct_##A = (uintptr_t)fct; return my_AttlistDecl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat AttlistDecl callback\n");
    return NULL;
}
// SkippedEntity ...
#define GO(A)   \
static uintptr_t my_SkippedEntity_fct_##A = 0;                            \
static void my_SkippedEntity_##A(void* a, void* b, int c)                 \
{                                                                         \
    RunFunctionFmt(my_SkippedEntity_fct_##A, "ppi", a, b, c);         \
}
SUPER()
#undef GO
static void* find_SkippedEntity_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_SkippedEntity_fct_##A == (uintptr_t)fct) return my_SkippedEntity_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_SkippedEntity_fct_##A == 0) {my_SkippedEntity_fct_##A = (uintptr_t)fct; return my_SkippedEntity_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bits expat SkippedEntity callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my32_XML_SetElementHandler(x64emu_t* emu, void* p, void* start, void* end)
{
    my->XML_SetElementHandler(p, find_StartElementHandler_Fct(start), find_End_Fct(end));
}

EXPORT void my32_XML_SetCharacterDataHandler(x64emu_t* emu, void* p, void* h)
{
    my->XML_SetCharacterDataHandler(p, find_CharData_Fct(h));
}

//EXPORT void my32_XML_SetNamespaceDeclHandler(x64emu_t* emu, void* p, void* start, void* end)
//{
//    my->XML_SetNamespaceDeclHandler(p, find_StartNamespaceDecl_Fct(start), find_EndNamespaceDecl_Fct(end));
//}

//EXPORT void my32_XML_SetEntityDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetEntityDeclHandler(p, find_EntityDecl_Fct(f));
//}

//EXPORT void my32_XML_SetStartDoctypeDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetStartDoctypeDeclHandler(p, find_StartDoctypeDecl_Fct(f));
//}

//EXPORT void my32_XML_SetEndDoctypeDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetEndDoctypeDeclHandler(p, find_EndDoctypeDecl_Fct(f));
//}

//EXPORT void my32_XML_SetElementDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetElementDeclHandler(p, find_ElementDecl_Fct(f));
//}

//EXPORT void my32_XML_SetDefaultHandlerExpand(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetDefaultHandlerExpand(p, find_Default_Fct(f));
//}

//EXPORT void my32_XML_SetCommentHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetCommentHandler(p, find_Comment_Fct(f));
//}

//EXPORT void my32_XML_SetProcessingInstructionHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetProcessingInstructionHandler(p, find_ProcessingInstruction_Fct(f));
//}

//EXPORT void my32_XML_SetStartElementHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetStartElementHandler(p, find_StartElement_Fct(f));
//}

//EXPORT void my32_XML_SetEndElementHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetEndElementHandler(p, find_EndElement_Fct(f));
//}

//EXPORT void my32_XML_SetUnknownEncodingHandler(x64emu_t* emu, void* p, void* f, void* d)
//{
//    my->XML_SetUnknownEncodingHandler(p, find_UnknownEncoding_Fct(f), d);
//}

//EXPORT void my32_XML_SetUnparsedEntityDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetUnparsedEntityDeclHandler(p, find_UnparsedEntityDecl_Fct(f));
//}

//EXPORT void my32_XML_SetNotationDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetNotationDeclHandler(p, find_NotationDecl_Fct(f));
//}

//EXPORT void my32_XML_SetStartNamespaceDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetStartNamespaceDeclHandler(p, find_StartNamespaceDecl_Fct(f));
//}

//EXPORT void my32_XML_SetEndNamespaceDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetEndNamespaceDeclHandler(p, find_EndNamespaceDecl_Fct(f));
//}

//EXPORT void my32_XML_SetStartCdataSectionHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetStartCdataSectionHandler(p, find_StartCdataSection_Fct(f));
//}

//EXPORT void my32_XML_SetEndCdataSectionHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetEndCdataSectionHandler(p, find_EndCdataSection_Fct(f));
//}

//EXPORT void my32_XML_SetDefaultHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetDefaultHandler(p, find_Default_Fct(f));
//}

//EXPORT void my32_XML_SetNotStandaloneHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetNotStandaloneHandler(p, find_NotStandalone_Fct(f));
//}

//EXPORT void my32_XML_SetExternalEntityRefHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetExternalEntityRefHandler(p, find_ExternalEntityRef_Fct(f));
//}

//EXPORT void my32_XML_SetXmlDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetXmlDeclHandler(p, find_XmlDecl_Fct(f));
//}

//EXPORT void my32_XML_SetAttlistDeclHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetAttlistDeclHandler(p, find_AttlistDecl_Fct(f));
//}

//EXPORT void my32_XML_SetSkippedEntityHandler(x64emu_t* emu, void* p, void* f)
//{
//    my->XML_SetSkippedEntityHandler(p, find_SkippedEntity_Fct(f));
//}

EXPORT void my32_XML_SetDoctypeDeclHandler(x64emu_t* emu, void* p, void* start, void* end)
{
    (void)emu;
    my->XML_SetDoctypeDeclHandler(p, find_StartDoctypeDecl_Fct(start), find_EndDoctypeDecl_Fct(end));
}

// this function will call the callbacks with many structures created on the stack, wich might be a 64bits walue on the initial thred
// so using a swapcontext trick to use a 32bits stack temporarily
// TODO: generalise this on main?
static void func1(uint32_t* ret, void* p, int a, int b)
{
    *ret = my->XML_ParseBuffer(p, a, b);
}
ucontext_t uctx_main, uctx_func1;

#ifndef ANDROID
EXPORT uint32_t my32_XML_ParseBuffer(x64emu_t* emu, void* p, int a, int b)
{
    getcontext(&uctx_func1);
    size_t stack_size = 1024*1024;
    void* func1_stack = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_32BIT|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
    uctx_func1.uc_stack.ss_sp = func1_stack;
    uctx_func1.uc_stack.ss_size = stack_size;
    uctx_func1.uc_link = &uctx_main;
    uint32_t ret = 0;
    makecontext(&uctx_func1, (void*)func1, 4, &ret, p, a, b);
    swapcontext(&uctx_main, &uctx_func1);
    munmap(func1_stack, stack_size);
    return ret;
}
#endif

#include "wrappedlib_init32.h"
