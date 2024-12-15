#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <netdb.h>
#include <resolv.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "box32.h"
#include "myalign32.h"

static const char* libresolvName = "libresolv.so.2";
#define LIBNAME libresolv

#include "generated/wrappedlibresolvtypes32.h"

#include "wrappercallback32.h"

void* convert_res_state_to_32(void* d, void* s);
void* convert_res_state_to_64(void* d, void* s);

EXPORT int my32_res_query(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    convert_res_state_to_64(emu->res_state_64, emu->res_state_32);
    int ret = my->__res_query(dname, class, type, answer, anslen);
    emu->libc_herr = h_errno;
    return ret;
}

EXPORT int my32_res_search(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    convert_res_state_to_64(emu->res_state_64, emu->res_state_32);
    int ret = my->__res_search(dname, class, type, answer, anslen);
    emu->libc_herr = h_errno;
    return ret;
}

void convert_ns_msg_to_32(void* d, void* s)
{
    if(!d || !s) return;
    ns_msg* src = s;
    my_ns_msg_32_t* dst = d;
    dst->_msg = to_ptrv((void*)src->_msg);
    dst->_eom = to_ptrv((void*)src->_eom);
    dst->_id = src->_id;
    dst->_flags = src->_flags;
    for(int i=0; i<4; ++i)
        dst->_counts[i] = src->_counts[i];
    for(int i=0; i<4; ++i)
        dst->_sections[i] = to_ptrv((void*)src->_sections[i]);
    dst->_sect = src->_sect;
    dst->_rrnum = src->_rrnum;
    dst->_msg_ptr = to_ptrv((void*)src->_msg_ptr);
}
void convert_ns_msg_to_64(void* d, void* s)
{
    if(!d || !s) return;
    my_ns_msg_32_t* src = s;
    ns_msg* dst = d;
    dst->_msg_ptr = from_ptrv(src->_msg_ptr);
    dst->_rrnum = src->_rrnum;
    dst->_sect = src->_sect;
    for(int i=3; i>=0; --i)
        dst->_sections[i] = from_ptrv(src->_sections[i]);
    for(int i=3; i>=0; --i)
        dst->_counts[i] = src->_counts[i];
    dst->_flags = src->_flags;
    dst->_id = src->_id;
    dst->_eom = from_ptrv(src->_eom);
    dst->_msg = from_ptrv(src->_msg);
}


EXPORT int my32_ns_initparse(x64emu_t* emu, void* msg, int len, my_ns_msg_32_t* handle)
{
    ns_msg handle_l = {0};
    int ret = ns_initparse(msg, len, &handle_l);
    convert_ns_msg_to_32(handle, &handle_l);
    return ret;
}

void convert_ns_rr_to_32(void* d, void* s)
{
    if(!d || !s) return;
    ns_rr* src = s;
    my_ns_rr_32_t* dst = d;
    memcpy(dst->name, src->name, sizeof(dst->name));
    dst->type = src->type;
    dst->rr_class = src->rr_class;
    dst->ttl = src->ttl;
    dst->rdlength = src->rdlength;
    dst->rdata = to_ptrv((void*)src->rdata);
}
void convert_ns_rr_to_64(void* d, void* s)
{
    if(!d || !s) return;
    my_ns_rr_32_t* src = s;
    ns_rr* dst = d;
    dst->rdata = from_ptrv(src->rdata);
    dst->rdlength = src->rdlength;
    dst->ttl = src->ttl;
    dst->rr_class = src->rr_class;
    dst->type = src->type;
    memcpy(dst->name, src->name, sizeof(dst->name));
}

EXPORT int my32_ns_parserr(x64emu_t* emu, my_ns_msg_32_t* handle, uint32_t section, int rrnum, my_ns_rr_32_t* rr)
{
    ns_msg handle_l = {0};
    ns_rr rr_l = {0};
    convert_ns_msg_to_64(&handle_l, handle);
    convert_ns_rr_to_64(&rr_l, rr);
    int ret = ns_parserr(&handle_l, section, rrnum, &rr_l);
    convert_ns_rr_to_32(rr, &rr_l);
    convert_ns_msg_to_32(handle, &handle_l);
    return ret;
}

#include "wrappedlib_init32.h"
