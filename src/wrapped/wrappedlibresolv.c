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
extern int __res_nquery(int a, void* b, int c, int d, void* e, int f) {return res_nquery(a, b, c, d, e, f);}
extern int __res_nsearch(void* a, void* b, int c, int d, void* e, int f) {return res_nsearch(a, b, c, d, e, f);}
extern int __res_ownok(void* a) {return res_ownok(a);}
extern int __res_query(void* a, int b, int c, void* d, int e) {return res_query(a, b, c, d, e);}
extern int __res_querydomain(void* a, void* b, int c, int d, void* e, int f) {return res_querydomain(a, b, c, d, e, f);}
extern int __res_search(void* a, int b, int c, void* d, int e) {return res_search(a, b, c, d, e);}
extern int __res_send(void* a, int b, void* c, int d) {return res_send(a, b, c, d);}
#endif

#include "wrappedlib_init.h"
