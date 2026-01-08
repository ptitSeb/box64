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

static int real_res_search_wrapper(const char *dname, int class, int type, unsigned char *answer, int anslen)
{
    if(my->res_search) {
        return my->res_search((void*)dname, class, type, answer, anslen);
    }
    if(my->__res_search) {
        return my->__res_search((void*)dname, class, type, answer, anslen);
    }

    printf_log(LOG_NONE, "Error: res_search or __res_search symbol not found in libresolv!\n");
    return -1;
}

static int real_dn_expand_wrapper(const void *msg, const void *eomorig, const void *comp_dn, void *exp_dn, int length)
{
    if(my->__dn_expand) {
        return my->__dn_expand((void*)msg, (void*)eomorig, (void*)comp_dn, exp_dn, length);
    }
    if(my->dn_expand) {
        return my->dn_expand((void*)msg, (void*)eomorig, (void*)comp_dn, exp_dn, length);
    }

    printf_log(LOG_NONE, "Error: dn_expand or __dn_expand symbol not found in libresolv!\n");
    return -1;
}

static int real_res_mkquery_wrapper(int op, const char *dname, int class, int type, const unsigned char *data, int datalen, const unsigned char *newrr, unsigned char *buf, int buflen)
{
    if(my->__res_mkquery) {
        return my->__res_mkquery(op, (void*)dname, class, type, (void*)data, datalen, (void*)newrr, buf, buflen);
    }
    if(my->res_mkquery) {
        return my->res_mkquery(op, (void*)dname, class, type, (void*)data, datalen, (void*)newrr, buf, buflen);
    }

    printf_log(LOG_NONE, "Error: res_mkquery or __res_mkquery symbol not found in libresolv!\n");
    return -1;
}

static int real_res_query_wrapper(const char *dname, int class, int type, unsigned char *answer, int anslen)
{
    if(my->__res_query) {
        return my->__res_query((void*)dname, class, type, answer, anslen);
    }
    if(my->res_query) {
        return my->res_query((void*)dname, class, type, answer, anslen);
    }

    printf_log(LOG_NONE, "Error: res_query or __res_query symbol not found in libresolv!\n");
    return -1;
}

static int real_res_querydomain_wrapper(const char *name, const char *domain, int class, int type, unsigned char *answer, int anslen)
{
    if(my->__res_querydomain) {
        return my->__res_querydomain((void*)name, (void*)domain, class, type, answer, anslen);
    }
    if(my->res_querydomain) {
        return my->res_querydomain((void*)name, (void*)domain, class, type, answer, anslen);
    }

    printf_log(LOG_NONE, "Error: res_querydomain or __res_querydomain symbol not found in libresolv!\n");
    return -1;
}

EXPORT int my_res_search(x64emu_t* emu, const char* dname, int class, int type, unsigned char* answer, int anslen)
{
    return real_res_search_wrapper(dname, class, type, answer, anslen);
}

EXPORT int my___res_search(x64emu_t* emu, const char* dname, int class, int type, unsigned char* answer, int anslen)
{
    return real_res_search_wrapper(dname, class, type, answer, anslen);
}

EXPORT int my___dn_expand(x64emu_t* emu, const void* msg, const void* eomorig, const void* comp_dn, void* exp_dn, int length)
{
    return real_dn_expand_wrapper(msg, eomorig, comp_dn, exp_dn, length);
}

EXPORT int my_dn_expand(x64emu_t* emu, const void* msg, const void* eomorig, const void* comp_dn, void* exp_dn, int length)
{
    return real_dn_expand_wrapper(msg, eomorig, comp_dn, exp_dn, length);
}

EXPORT int my___res_query(x64emu_t* emu, const char* dname, int class, int type, unsigned char* answer, int anslen)
{
    return real_res_query_wrapper(dname, class, type, answer, anslen);
}

EXPORT int my_res_query(x64emu_t* emu, const char* dname, int class, int type, unsigned char* answer, int anslen)
{
    return real_res_query_wrapper(dname, class, type, answer, anslen);
}

EXPORT int my___res_querydomain(x64emu_t* emu, const char* name, const char* domain, int class, int type, unsigned char* answer, int anslen)
{
    return real_res_querydomain_wrapper(name, domain, class, type, answer, anslen);
}

EXPORT int my_res_querydomain(x64emu_t* emu, const char* name, const char* domain, int class, int type, unsigned char* answer, int anslen)
{
    return real_res_querydomain_wrapper(name, domain, class, type, answer, anslen);
}

EXPORT int my___res_mkquery(x64emu_t* emu, int op, const char* dname, int class, int type, const unsigned char* data, int datalen, const unsigned char* newrr, unsigned char* buf, int buflen)
{
    return real_res_mkquery_wrapper(op, dname, class, type, data, datalen, newrr, buf, buflen);
}

EXPORT int my_res_mkquery(x64emu_t* emu, int op, const char* dname, int class, int type, const unsigned char* data, int datalen, const unsigned char* newrr, unsigned char* buf, int buflen)
{
    return real_res_mkquery_wrapper(op, dname, class, type, data, datalen, newrr, buf, buflen);
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
