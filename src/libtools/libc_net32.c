#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "box64stack.h"
#include "x64emu.h"
#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "myalign32.h"
#include "fileutils.h"
#include "globalsymbols.h"
#include "box32.h"
#include "converter32.h"

EXPORT ssize_t my32_recvmsg(x64emu_t* emu, int socket, void* msg, int flags)
{
    struct iovec iov;
    struct msghdr m;
    AlignMsgHdr_32(&m, &iov, msg);
    ssize_t ret = recvmsg(socket, &m, flags);
    // put back msg_flags in place
    ((struct i386_msghdr*)msg)->msg_flags = m.msg_flags;
    return ret;
}

EXPORT ssize_t my32_sendmsg(x64emu_t* emu, int socket, void* msg, int flags)
{
    struct iovec iov[256];
    struct msghdr m;
    AlignMsgHdr_32(&m, &iov, msg);
    ssize_t ret = sendmsg(socket, &m, flags);
    return ret;
}

EXPORT int my32_getaddrinfo(x64emu_t* emu, void* node, void* service, struct i386_addrinfo* hints, ptr_t* res)
{
    struct addrinfo* hints_ = (struct addrinfo*)hints;  // only first part is used, wich is identical
    struct addrinfo* p = {0};
    int ret = getaddrinfo(node, service, hints_, &p);
    if(!ret && p) {
        // counting the number of "next"
        struct addrinfo* p2 = p;
        int idx = 0;
        while(p2) {++idx; p2 = p2->ai_next;}
        // doing the malloc!
        void* r = box_malloc(idx*sizeof(struct i386_addrinfo)+sizeof(void*));
        ptr_t p3 = to_ptrv(r);
        *res = p3;
        p2 = p;
        for(int i=0; i<idx; ++i) {
            struct i386_addrinfo* dest = (struct i386_addrinfo*)from_ptrv(p3);
            p3+=sizeof(struct i386_addrinfo);
            if(!i) {
                *(void**)from_ptrv(p3) = p;
                p3+=sizeof(void*);
            }
            dest->ai_flags = p2->ai_flags;
            dest->ai_family = p2->ai_family;
            dest->ai_socktype = p2->ai_socktype;
            dest->ai_protocol = p2->ai_protocol;
            dest->ai_addrlen = p2->ai_addrlen;
            dest->ai_addr = to_ptrv(p2->ai_addr);
            dest->ai_canonname = to_cstring(p2->ai_canonname);
            p2 = p2->ai_next;
            dest->ai_next = p2?p3:0;
        }
    } else
        *res = 0;
    return ret;
}

EXPORT void my32_freeaddrinfo(x64emu_t* emu, void* a) {
    if(!a) return;
    void* orig = *(void**)(a+sizeof(struct i386_addrinfo));
    freeaddrinfo(orig);
    box_free(a);
}

EXPORT void* my32_gethostbyname(x64emu_t* emu, const char* a)
{
    static struct i386_hostent ret = {0};
    static ptr_t strings[128] = {0};
    struct hostent* h = gethostbyname(a);
    if(!h) return NULL;
    // convert...
    ret.h_name = to_cstring(h->h_name);
    ret.h_addrtype = h->h_addrtype;
    ret.h_length = h->h_length;
    ptr_t s = to_ptrv(&strings);
    int idx = 0;
    ret.h_aliases = h->h_aliases?s:0;
    if(h->h_aliases) {
        char** p = h->h_aliases;
        while(*p) {
            strings[idx++] = to_cstring(*p);
            ++p;
        }
        strings[idx++] = 0;
    }
    ret.h_addr_list = h->h_addr_list?to_ptrv(&strings[idx]):0;
    if(h->h_addr_list) {
        char** p = h->h_addr_list;
        while(*p) {
            strings[idx++] = to_ptrv(*p);
            ++p;
        }   
        strings[idx++] = 0;
    }
    // done
    return &ret;
}

struct i386_ifaddrs
{
  ptr_t     ifa_next;   // struct ifaddrs *
  ptr_t     ifa_name;   // char *
  uint32_t  ifa_flags;
  ptr_t     ifa_addr;   // struct sockaddr *
  ptr_t     ifa_netmask;// struct sockaddr *
  ptr_t     ifa_ifu;    // union of struct sockaddr
  ptr_t     ifa_data;   // void *
};

EXPORT int my32_getifaddrs(x64emu_t* emu, void** res)
{
    struct ifaddrs* addrs;
    int ret = getifaddrs(&addrs);
    if(!ret) {
        // convert the chained list of ifaddrs to i386 (narrowed) in place
        struct ifaddrs* p = addrs;
        while(p) {
            struct i386_ifaddrs *i386 = (struct i386_ifaddrs*)p;
            struct ifaddrs* next = p->ifa_next;
            i386->ifa_next = to_ptrv(p->ifa_next);
            i386->ifa_name = to_cstring(p->ifa_name);
            i386->ifa_flags = p->ifa_flags;
            i386->ifa_addr = to_ptrv(p->ifa_addr);
            i386->ifa_netmask = to_ptrv(p->ifa_netmask);
            i386->ifa_ifu = (i386->ifa_flags&IFF_BROADCAST)?to_ptrv(p->ifa_broadaddr):to_ptrv(p->ifa_dstaddr);
            i386->ifa_data = to_ptrv(p->ifa_data);
            p = next;
        }
    }
}

EXPORT void* my32___h_errno_location(x64emu_t* emu)
{
    // TODO: Find a better way to do this
    // cannot use __thread as it makes the address not 32bits
    emu->libc_herr = h_errno;
    return &emu->libc_herr;
}
