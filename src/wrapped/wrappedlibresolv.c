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

const char* libresolvName = "libresolv.so.2";
#define LIBNAME libresolv

#include "generated/wrappedlibresolvtypes.h"
#include "wrappercallback.h"

#define FIND_REAL_DNS_FUNC(name)                                                                  \
    void* my_func_##name = NULL;                                                                  \
    if (my->name)                                                                                 \
        my_func_##name = my->name;                                                                \
    else if (my->__##name)                                                                        \
        my_func_##name = my->__##name;                                                            \
    else {                                                                                        \
        my_func_##name = NULL;                                                                    \
        printf_log(LOG_NONE, "Error: " #name " or __" #name " symbol not found in libresolv!\n"); \
    }

EXPORT int my_res_search(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_search);
    return ((iFpiipi_t)my_func_res_search)(dname, class, type, answer, anslen);
}

EXPORT int my___res_search(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_search);
    return ((iFpiipi_t)my_func_res_search)(dname, class, type, answer, anslen);
}

EXPORT int my___dn_expand(x64emu_t* emu, void* msg, void* eomorig, void* comp_dn, void* exp_dn, int length)
{
    FIND_REAL_DNS_FUNC(dn_expand);
    return ((iFppppi_t)my_func_dn_expand)(msg, eomorig, comp_dn, exp_dn, length);
}

EXPORT int my_dn_expand(x64emu_t* emu, void* msg, void* eomorig, void* comp_dn, void* exp_dn, int length)
{
    FIND_REAL_DNS_FUNC(dn_expand);
    return ((iFppppi_t)my_func_dn_expand)(msg, eomorig, comp_dn, exp_dn, length);
}

EXPORT int my___res_nquery(x64emu_t* emu, void* statep, void* dname, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_nquery);
    return ((iFppiipi_t)my_func_res_nquery)(statep, dname, class, type, answer, anslen);
}

EXPORT int my_res_nquery(x64emu_t* emu, void* statep, void* dname, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_nquery);
    return ((iFppiipi_t)my_func_res_nquery)(statep, dname, class, type, answer, anslen);
}

EXPORT int my___res_nquerydomain(x64emu_t* emu, void* statep, void* dname, void* domain, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_nquerydomain);
    return ((iFpppiipi_t)my_func_res_nquerydomain)(statep, dname, domain, class, type, answer, anslen);
}

EXPORT int my_res_nquerydomain(x64emu_t* emu, void* statep, void* dname, void* domain, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_nquerydomain);
    return ((iFpppiipi_t)my_func_res_nquerydomain)(statep, dname, domain, class, type, answer, anslen);
}

EXPORT int my___res_query(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_query);
    return ((iFpiipi_t)my_func_res_query)(dname, class, type, answer, anslen);
}

EXPORT int my_res_query(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_query);
    return ((iFpiipi_t)my_func_res_query)(dname, class, type, answer, anslen);
}

EXPORT int my___res_querydomain(x64emu_t* emu, void* name, void* domain, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_querydomain);
    return ((iFppiipi_t)my_func_res_querydomain)(name, domain, class, type, answer, anslen);
}

EXPORT int my_res_querydomain(x64emu_t* emu, void* name, void* domain, int class, int type, void* answer, int anslen)
{
    FIND_REAL_DNS_FUNC(res_querydomain);
    return ((iFppiipi_t)my_func_res_querydomain)(name, domain, class, type, answer, anslen);
}

EXPORT int my___res_mkquery(x64emu_t* emu, int op, void* dname, int class, int type, void* data, int datalen, void* newrr, void* buf, int buflen)
{
    FIND_REAL_DNS_FUNC(res_mkquery);
    return ((iFipiipippi_t)my_func_res_mkquery)(op, dname, class, type, data, datalen, newrr, buf, buflen);
}

EXPORT int my_res_mkquery(x64emu_t* emu, int op, void* dname, int class, int type, void* data, int datalen, void* newrr, void* buf, int buflen)
{
    FIND_REAL_DNS_FUNC(res_mkquery);
    return ((iFipiipippi_t)my_func_res_mkquery)(op, dname, class, type, data, datalen, newrr, buf, buflen);
}

#ifdef STATICBUILD
#include <arpa/inet.h>
#include <resolv.h>

#undef __dn_comp
#undef __dn_expand
#undef __dn_skipname

int __dn_comp(void* a, void* b, int c, void* d, void* e ) {return dn_comp(a, b, c, d, e);}
int __dn_expand(void* a, void* b, void* c, void* d, int e) {return dn_expand(a, b, c, d, e);}
int __dn_skipname(void* a, void* b) {return dn_skipname(a, b);}
uint32_t __ns_get16(void* a);
size_t __ns_get32(void* a);
extern int __ns_name_ntop(void* a, void* b, size_t c) {return ns_name_ntop(a, b, c);}
extern int __ns_name_unpack(void* a, void* b, void* c, void* d, size_t e) {return ns_name_unpack(a, b, c, d, e);}
extern int __res_dnok(void* a) {return res_dnok(a);}
extern int __res_hnok(void* a) {return res_hnok(a);}
extern int __res_mailok(void* a) {return res_mailok(a);}
extern int __res_mkquery(int a, void* b, int c, int d, void* e, int f, void* g, void* h, int i) {return res_mkquery(a, b, c, d, e, f, g, h, i);}
extern int __res_nquery(void* a, void* b, int c, int d, void* e, int f) {return res_nquery(a, b, c, d, e, f);}
extern int __res_nsearch(void* a, void* b, int c, int d, void* e, int f) {return res_nsearch(a, b, c, d, e, f);}
extern int __res_ownok(void* a) {return res_ownok(a);}
extern int __res_query(void* a, int b, int c, void* d, int e) {return res_query(a, b, c, d, e);}
extern int __res_querydomain(void* a, void* b, int c, int d, void* e, int f) {return res_querydomain(a, b, c, d, e, f);}
extern int __res_search(void* a, int b, int c, void* d, int e) {return res_search(a, b, c, d, e);}
extern int __res_send(void* a, int b, void* c, int d) {return res_send(a, b, c, d);}
#endif

#include "wrappedlib_init.h"
