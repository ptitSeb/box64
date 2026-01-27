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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* avahiclientName = "libavahi-client.so.3";
#define LIBNAME avahiclient

#define ADDED_FUNCTIONS()       \

#include "generated/wrappedavahiclienttypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// AvahiAddressResolverCallback
#define GO(A)   \
static uintptr_t my_AvahiAddressResolverCallback_fct_##A = 0;                                                       \
static void my_AvahiAddressResolverCallback_##A(void* a, int b, int c, int d, void* e, void* f, int g, void* h)     \
{                                                                                                                   \
    RunFunctionFmt(my_AvahiAddressResolverCallback_fct_##A, "piiippip", a, b, c, d, e, f, g, h);                    \
}
SUPER()
#undef GO
static void* findAvahiAddressResolverCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiAddressResolverCallback_fct_##A == (uintptr_t)fct) return my_AvahiAddressResolverCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiAddressResolverCallback_fct_##A == 0) {my_AvahiAddressResolverCallback_fct_##A = (uintptr_t)fct; return my_AvahiAddressResolverCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiAddressResolverCallback callback\n");
    return NULL;
}
// AvahiDomainBrowserCallback
#define GO(A)   \
static uintptr_t my_AvahiDomainBrowserCallback_fct_##A = 0;                                             \
static void my_AvahiDomainBrowserCallback_##A(void* a, int b, int c, int d, void* e, int f, void* g)    \
{                                                                                                       \
    RunFunctionFmt(my_AvahiDomainBrowserCallback_fct_##A, "piiipip", a, b, c, d, e, f, g);              \
}
SUPER()
#undef GO
static void* findAvahiDomainBrowserCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiDomainBrowserCallback_fct_##A == (uintptr_t)fct) return my_AvahiDomainBrowserCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiDomainBrowserCallback_fct_##A == 0) {my_AvahiDomainBrowserCallback_fct_##A = (uintptr_t)fct; return my_AvahiDomainBrowserCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiDomainBrowserCallback callback\n");
    return NULL;
}
// AvahiHostNameResolverCallback
#define GO(A)   \
static uintptr_t my_AvahiHostNameResolverCallback_fct_##A = 0;                                                      \
static void my_AvahiHostNameResolverCallback_##A(void* a, int b, int c, int d, void* e, void* f, int g, void* h)    \
{                                                                                                                   \
    RunFunctionFmt(my_AvahiHostNameResolverCallback_fct_##A, "piiippip", a, b, c, d, e, f, g, h);                   \
}
SUPER()
#undef GO
static void* findAvahiHostNameResolverCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiHostNameResolverCallback_fct_##A == (uintptr_t)fct) return my_AvahiHostNameResolverCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiHostNameResolverCallback_fct_##A == 0) {my_AvahiHostNameResolverCallback_fct_##A = (uintptr_t)fct; return my_AvahiHostNameResolverCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiHostNameResolverCallback callback\n");
    return NULL;
}
// AvahiRecordBrowserCallback
#define GO(A)   \
static uintptr_t my_AvahiRecordBrowserCallback_fct_##A = 0;                                                                                     \
static void my_AvahiRecordBrowserCallback_##A(void* a, int b, int c, int d, void* e, uint16_t f, uint16_t g, void* h, size_t i, int j, void* k) \
{                                                                                                                                               \
    RunFunctionFmt(my_AvahiRecordBrowserCallback_fct_##A, "piiipWWpLip", a, b, c, d, e, f, g, h, i, j, k);                                      \
}
SUPER()
#undef GO
static void* findAvahiRecordBrowserCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiRecordBrowserCallback_fct_##A == (uintptr_t)fct) return my_AvahiRecordBrowserCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiRecordBrowserCallback_fct_##A == 0) {my_AvahiRecordBrowserCallback_fct_##A = (uintptr_t)fct; return my_AvahiRecordBrowserCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiRecordBrowserCallback callback\n");
    return NULL;
}
// AvahiServiceBrowserCallback
#define GO(A)   \
static uintptr_t my_AvahiServiceBrowserCallback_fct_##A = 0;                                                                \
static void my_AvahiServiceBrowserCallback_##A(void* a, int b, int c, int d, void* e, void* f, void* g, int h, void* i)     \
{                                                                                                                           \
    RunFunctionFmt(my_AvahiServiceBrowserCallback_fct_##A, "piiipppip", a, b, c, d, e, f, g, h, i);                         \
}
SUPER()
#undef GO
static void* findAvahiServiceBrowserCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiServiceBrowserCallback_fct_##A == (uintptr_t)fct) return my_AvahiServiceBrowserCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiServiceBrowserCallback_fct_##A == 0) {my_AvahiServiceBrowserCallback_fct_##A = (uintptr_t)fct; return my_AvahiServiceBrowserCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiServiceBrowserCallback callback\n");
    return NULL;
}
// AvahiServiceTypeBrowserCallback
#define GO(A)   \
static uintptr_t my_AvahiServiceTypeBrowserCallback_fct_##A = 0;                                                    \
static void my_AvahiServiceTypeBrowserCallback_##A(void* a, int b, int c, int d, void* e, void* f, int g, void* h)  \
{                                                                                                                   \
    RunFunctionFmt(my_AvahiServiceTypeBrowserCallback_fct_##A, "piiippip", a, b, c, d, e, f, g, h);                 \
}
SUPER()
#undef GO
static void* findAvahiServiceTypeBrowserCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiServiceTypeBrowserCallback_fct_##A == (uintptr_t)fct) return my_AvahiServiceTypeBrowserCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiServiceTypeBrowserCallback_fct_##A == 0) {my_AvahiServiceTypeBrowserCallback_fct_##A = (uintptr_t)fct; return my_AvahiServiceTypeBrowserCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiServiceTypeBrowserCallback callback\n");
    return NULL;
}
// AvahiServiceResolverCallback
#define GO(A)   \
static uintptr_t my_AvahiServiceResolverCallback_fct_##A = 0;                                                                                                   \
static void my_AvahiServiceResolverCallback_##A(void* a, int b, int c, int d, void* e, void* f, void* g, void* h, void* i, uint16_t j, void* k, int l, void* m) \
{                                                                                                                                                               \
    RunFunctionFmt(my_AvahiServiceResolverCallback_fct_##A, "piiipppppWpip", a, b, c, d, e, f, g, h, i, j, k, l, m);                                            \
}
SUPER()
#undef GO
static void* findAvahiServiceResolverCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiServiceResolverCallback_fct_##A == (uintptr_t)fct) return my_AvahiServiceResolverCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiServiceResolverCallback_fct_##A == 0) {my_AvahiServiceResolverCallback_fct_##A = (uintptr_t)fct; return my_AvahiServiceResolverCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiServiceResolverCallback callback\n");
    return NULL;
}
// AvahiClientCallback
#define GO(A)   \
static uintptr_t my_AvahiClientCallback_fct_##A = 0;                \
static void my_AvahiClientCallback_##A(void* a, int b, void* c)     \
{                                                                   \
    RunFunctionFmt(my_AvahiClientCallback_fct_##A, "pip", a, b, c); \
}
SUPER()
#undef GO
static void* findAvahiClientCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiClientCallback_fct_##A == (uintptr_t)fct) return my_AvahiClientCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiClientCallback_fct_##A == 0) {my_AvahiClientCallback_fct_##A = (uintptr_t)fct; return my_AvahiClientCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiClientCallback callback\n");
    return NULL;
}
// AvahiEntryGroupCallback
#define GO(A)   \
static uintptr_t my_AvahiEntryGroupCallback_fct_##A = 0;                \
static void my_AvahiEntryGroupCallback_##A(void* a, int b, void* c)     \
{                                                                       \
    RunFunctionFmt(my_AvahiEntryGroupCallback_fct_##A, "pip", a, b, c); \
}
SUPER()
#undef GO
static void* findAvahiEntryGroupCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AvahiEntryGroupCallback_fct_##A == (uintptr_t)fct) return my_AvahiEntryGroupCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AvahiEntryGroupCallback_fct_##A == 0) {my_AvahiEntryGroupCallback_fct_##A = (uintptr_t)fct; return my_AvahiEntryGroupCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libavahi-client AvahiEntryGroupCallback callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_avahi_address_resolver_new(x64emu_t* emu, void* client, int interface, int protocol, void* a, int flags, void* cb, void* data)
{
    return my->avahi_address_resolver_new(client, interface, protocol, a, flags, findAvahiAddressResolverCallbackFct(cb), data);
}

EXPORT void* my_avahi_domain_browser_new(x64emu_t* emu, void* client, int interface, int protocol, void* domain, int btype, int flags, void* cb, void* data)
{
    return my->avahi_domain_browser_new(client, interface, protocol, domain, btype, flags, findAvahiDomainBrowserCallbackFct(cb), data);
}

EXPORT void* my_avahi_host_name_resolver_new(x64emu_t* emu, void* client, int interface, int protocol, void* name, int aprotocol, int flags, void* cb, void* data)
{
    return my->avahi_host_name_resolver_new(client, interface, protocol, name, aprotocol, flags, findAvahiHostNameResolverCallbackFct(cb), data);
}

EXPORT void* my_avahi_record_browser_new(x64emu_t* emu, void* client, int interface, int protocol, void* name, uint16_t class_, uint16_t type, int flags, void* cb, void* data)
{
    return my->avahi_record_browser_new(client, interface, protocol, name, class_, type, flags, findAvahiRecordBrowserCallbackFct(cb), data);
}

EXPORT void* my_avahi_service_browser_new(x64emu_t* emu, void* client, int interface, int protocol, void* type, void* domain, int flags, void* cb, void* data)
{
    return my->avahi_service_browser_new(client, interface, protocol, type, domain, flags, findAvahiServiceBrowserCallbackFct(cb), data);
}

EXPORT void* my_avahi_service_type_browser_new(x64emu_t* emu, void* client, int interface, int protocol, void* domain, int flags, void* cb, void* data)
{
    return my->avahi_service_type_browser_new(client, interface, protocol, domain, flags, findAvahiServiceTypeBrowserCallbackFct(cb), data);
}

EXPORT void* my_avahi_service_resolver_new(x64emu_t* emu, void* client, int interface, int protocol, void* name, void* type, void* domain, int aprotocol, int flags, void* cb, void* data)
{
    return my->avahi_service_resolver_new(client, interface, protocol, name, type, domain, aprotocol, flags, findAvahiServiceResolverCallbackFct(cb), data);
}

EXPORT void* my_avahi_client_new(x64emu_t* emu, void* poll_api, int flags, void* cb, void* data, void* err)
{
    return my->avahi_client_new(poll_api, flags, findAvahiClientCallbackFct(cb), data, err);
}

EXPORT void* my_avahi_entry_group_new(x64emu_t* emu, void* p, void* cb, void* data)
{
    return my->avahi_entry_group_new(p, findAvahiEntryGroupCallbackFct(cb), data);
}

#include "wrappedlib_init.h"
