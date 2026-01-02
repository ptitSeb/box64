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
#include <resolv.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

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
#include "custommem.h"

EXPORT ssize_t my32_recvmsg(x64emu_t* emu, int socket, struct i386_msghdr* msg, int flags)
{
    struct iovec iov[msg->msg_iovlen];
    struct msghdr m;
    uint8_t buff[msg->msg_controllen+256];
    AlignMsgHdr_32(&m, iov, buff, msg, 0);
    ssize_t ret = recvmsg(socket, &m, flags);
    if(ret>0)
        UnalignMsgHdr_32(msg, &m);
    return ret;
}

EXPORT ssize_t my32_sendmsg(x64emu_t* emu, int socket, struct i386_msghdr* msg, int flags)
{
    struct iovec iov[msg->msg_iovlen];
    struct msghdr m;
    uint8_t buff[msg->msg_controllen+256];
    AlignMsgHdr_32(&m, iov, buff, msg, 1);
    ssize_t ret = sendmsg(socket, &m, flags);
    UnalignMsgHdr_32(msg, &m);
    return ret;
}

EXPORT int my32_recvmmsg(x64emu_t* emu, int socket, struct i386_mmsghdr* msgs, uint32_t vlen, uint32_t flags, void* timeout)
{
    struct mmsghdr m[vlen];
    uint32_t iovlen = 0;
    size_t ctrlen = 0;
    for(uint32_t i=0; i<vlen; ++i) {
        if(msgs[i].msg_hdr.msg_iovlen>iovlen) iovlen = msgs[i].msg_hdr.msg_iovlen;
        if(msgs[i].msg_hdr.msg_controllen>ctrlen) ctrlen = msgs[i].msg_hdr.msg_controllen;
        m[i].msg_len = msgs[i].msg_len;
    }
    struct iovec iov[vlen][iovlen];
    uint8_t buff[vlen][ctrlen+256];
    for(uint32_t i=0; i<vlen; ++i)
        AlignMsgHdr_32(&m[i].msg_hdr, iov[i], buff[i], &msgs[i].msg_hdr, 1);
    int ret = recvmmsg(socket, m, vlen, flags, timeout);
    for(uint32_t i=0; i<vlen; ++i) {
        UnalignMsgHdr_32(&msgs[i].msg_hdr, &m[i].msg_hdr);
        msgs[i].msg_len = m[i].msg_len;
    }
    return ret;
}

EXPORT int my32_sendmmsg(x64emu_t* emu, int socket, struct i386_mmsghdr* msgs, uint32_t vlen, uint32_t flags)
{
    struct mmsghdr m[vlen];
    uint32_t iovlen = 0;
    size_t ctrlen = 0;
    for(uint32_t i=0; i<vlen; ++i) {
        if(msgs[i].msg_hdr.msg_iovlen>iovlen) iovlen = msgs[i].msg_hdr.msg_iovlen;
        if(msgs[i].msg_hdr.msg_controllen>ctrlen) ctrlen = msgs[i].msg_hdr.msg_controllen;
        m[i].msg_len = msgs[i].msg_len;
    }
    struct iovec iov[vlen][iovlen];
    uint8_t buff[vlen][ctrlen+256];
    for(uint32_t i=0; i<vlen; ++i)
        AlignMsgHdr_32(&m[i].msg_hdr, iov[i], buff[i], &msgs[i].msg_hdr, 1);
    int ret = sendmmsg(socket, m, vlen, flags);
    for(uint32_t i=0; i<vlen; ++i) {
        UnalignMsgHdr_32(&msgs[i].msg_hdr, &m[i].msg_hdr);
        msgs[i].msg_len = m[i].msg_len;
    }
    return ret;
}

EXPORT void* my32___cmsg_nxthdr(struct i386_msghdr* mhdr, struct i386_cmsghdr* cmsg)
{
    // simpler to redo, also, will be used internaly
    if(cmsg->cmsg_len < sizeof(struct i386_cmsghdr))
        return NULL;
    // compute next
    cmsg = (struct i386_cmsghdr*)(((uintptr_t)cmsg) + ((cmsg->cmsg_len+3)&~3));
    // check it still inside limits
    if((uintptr_t)(cmsg+1) > mhdr->msg_control+mhdr->msg_controllen)
        return NULL;
    if((uintptr_t)(cmsg)+cmsg->cmsg_len > mhdr->msg_control+mhdr->msg_controllen)
        return NULL;
    return cmsg;
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
        void* r = actual_malloc(idx*sizeof(struct i386_addrinfo)+sizeof(void*));
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
    actual_free(a);
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
    emu->libc_herr = h_errno;
    return &ret;
}

EXPORT int my32_gethostbyname_r(x64emu_t* emu, void* name, struct i386_hostent* ret, void* buff, size_t buflen, ptr_t* result, int* h_err)
{
    struct hostent ret_l = {0};
    struct hostent *result_l = NULL;
    int r = gethostbyname_r(name, &ret_l, buff, buflen, &result_l, h_err);
    if(!result_l)
        *result = 0;
    else
        *result = to_ptrv(ret);
    // convert result, all memory allocated should be in program space
    if(result_l) {
        ret->h_name = to_cstring(result_l->h_name);
        ret->h_addrtype = result_l->h_addrtype;
        ret->h_length = result_l->h_length;
        int idx = 0;
        ret->h_aliases = to_ptrv(result_l->h_aliases);
        if(result_l->h_aliases) {
            char** p = result_l->h_aliases;
            ptr_t* strings = from_ptrv(ret->h_aliases);
            while(*p) {
                strings[idx++] = to_cstring(*p);
                ++p;
            }
            strings[idx++] = 0;
        }
        idx = 0;
        ret->h_addr_list = to_ptrv(result_l->h_addr_list);
        if(result_l->h_addr_list) {
            char** p = result_l->h_addr_list;
            ptr_t* strings = from_ptrv(ret->h_addr_list);
            while(*p) {
                strings[idx++] = to_ptrv(*p);
                ++p;
            }   
            strings[idx++] = 0;
        }
    }
    return r;
}

EXPORT int my32_gethostbyname2_r(x64emu_t* emu, void* name, int af, struct i386_hostent* ret, void* buff, size_t buflen, ptr_t* result, int* h_err)
{
    struct hostent ret_l = {0};
    struct hostent *result_l = NULL;
    int r = gethostbyname2_r(name, af, &ret_l, buff, buflen, &result_l, h_err);
    if(!result_l)
        *result = 0;
    else
        *result = to_ptrv(ret);
    // convert result, all memory allocated should be in program space
    if(result_l) {
        ret->h_name = to_cstring(result_l->h_name);
        ret->h_addrtype = result_l->h_addrtype;
        ret->h_length = result_l->h_length;
        int idx = 0;
        ret->h_aliases = to_ptrv(result_l->h_aliases);
        if(result_l->h_aliases) {
            char** p = result_l->h_aliases;
            ptr_t* strings = from_ptrv(ret->h_aliases);
            while(*p) {
                strings[idx++] = to_cstring(*p);
                ++p;
            }
            strings[idx++] = 0;
        }
        idx = 0;
        ret->h_addr_list = to_ptrv(result_l->h_addr_list);
        if(result_l->h_addr_list) {
            char** p = result_l->h_addr_list;
            ptr_t* strings = from_ptrv(ret->h_addr_list);
            while(*p) {
                strings[idx++] = to_ptrv(*p);
                ++p;
            }   
            strings[idx++] = 0;
        }
    }
    return r;
}

EXPORT void* my32_gethostbyaddr(x64emu_t* emu, const char* a, uint32_t len, int type)
{
    static struct i386_hostent ret = {0};
    static ptr_t strings[128] = {0};
    struct hostent* h = gethostbyaddr(a, len, type);
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
    emu->libc_herr = h_errno;
    return &ret;
}

EXPORT int my32_gethostbyaddr_r(x64emu_t* emu, void* addr, uint32_t len, int type, struct i386_hostent* ret, void* buff, size_t buflen, ptr_t* result, int* h_err)
{
    struct hostent ret_l = {0};
    struct hostent *result_l = NULL;
    int r = gethostbyaddr_r(addr, len, type, &ret_l, buff, buflen, &result_l, h_err);
    if(!result_l)
        *result = 0;
    else
        *result = to_ptrv(ret);
    // convert result, all memory allocated should be in program space
    if(result_l) {
        ret->h_name = to_cstring(result_l->h_name);
        ret->h_addrtype = result_l->h_addrtype;
        ret->h_length = result_l->h_length;
        int idx = 0;
        ret->h_aliases = to_ptrv(result_l->h_aliases);
        if(result_l->h_aliases) {
            char** p = result_l->h_aliases;
            ptr_t* strings = from_ptrv(ret->h_aliases);
            while(*p) {
                strings[idx++] = to_cstring(*p);
                ++p;
            }
            strings[idx++] = 0;
        }
        ret->h_addr_list = to_ptrv(result_l->h_addr_list);
        idx = 0;
        if(result_l->h_addr_list) {
            char** p = result_l->h_addr_list;
            ptr_t* strings = from_ptrv(ret->h_addr_list);
            while(*p) {
                strings[idx++] = to_ptrv(*p);
                ++p;
            }   
            strings[idx++] = 0;
        }
    }
    return r;
}

EXPORT void* my32_getservbyname(x64emu_t* emu, void* name, void* proto)
{
    static struct i386_servent ret = {0};
    static ptr_t strings[128] = {0};
    struct servent* s = getservbyname(name, proto);
    if(!s) return NULL;
    // convert...
    ret.s_name = to_cstring(s->s_name);
    ret.s_port = s->s_port;
    ret.s_proto = to_cstring(s->s_proto);
    ptr_t strs = to_ptrv(&strings);
    int idx = 0;
    ret.s_aliases = s->s_aliases?strs:0;
    if(s->s_aliases) {
        char** p = s->s_aliases;
        while(*p) {
            strings[idx++] = to_cstring(*p);
            ++p;
        }
        strings[idx++] = 0;
    }
    // done
    emu->libc_herr = h_errno;
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
    int ret = getifaddrs((void*)res);
    if(!ret) {
        // convert the chained list of ifaddrs to i386 (narrowed) in place
        struct ifaddrs* p = *res;
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
    return ret;
}

EXPORT void* my32___h_errno_location(x64emu_t* emu)
{
    // TODO: Find a better way to do this
    // cannot use __thread as it makes the address not 32bits
    emu->libc_herr = h_errno;
    return &emu->libc_herr;
}

struct protoent_32
{
  ptr_t p_name; //char*
  ptr_t p_aliases;// char**
  int p_proto;
};

EXPORT void* my32_getprotobyname(x64emu_t* emu, void* name)
{
    static struct protoent_32 my_protoent = {0};
    static ptr_t strings[256];
    struct protoent *ret = getprotobyname(name);
    if(!ret)
        return NULL;
    my_protoent.p_name = to_cstring(ret->p_name);
    my_protoent.p_proto = ret->p_proto;
    if(ret->p_aliases) {
        my_protoent.p_aliases = to_ptrv(&strings);
        int i = 0;
        while(ret->p_aliases[i]) {
            strings[i] = to_cstring(ret->p_aliases[i]);
            ++i;
        }
        strings[i] = 0;
    } else 
        my_protoent.p_aliases = 0;

    return &my_protoent;
}

EXPORT int my32_getprotobyname_r(x64emu_t* emu, void* name,struct protoent_32* ret, void* buff, size_t buflen, ptr_t* result)
{
    struct protoent ret_l = {0};
    struct protoent *result_l = NULL;
    int r = getprotobyname_r(name, &ret_l, buff, buflen, &result_l);
    if(!result_l)
        *result = 0;
    else
        *result = to_ptrv(ret);
    // convert result, all memory allocated should be in program space
    if(result_l) {
        ret->p_name = to_cstring(result_l->p_name);
        int idx = 0;
        if(result_l->p_aliases) {
            char** p = result_l->p_aliases;
            ptr_t* strings = from_ptrv(ret->p_aliases);
            while(*p) {
                strings[idx++] = to_cstring(*p);
                ++p;
            }
            strings[idx++] = 0;
        }
        ret->p_aliases = to_ptrv(result_l->p_aliases);
        ret->p_proto = result_l->p_proto;
    }
    return r;
}

EXPORT int my32_getprotobynumber_r(x64emu_t* emu, int proto,struct protoent_32* ret, void* buff, size_t buflen, ptr_t* result)
{
    struct protoent ret_l = {0};
    struct protoent *result_l = NULL;
    int r = getprotobynumber_r(proto, &ret_l, buff, buflen, &result_l);
    if(!result_l)
        *result = 0;
    else
        *result = to_ptrv(ret);
    // convert result, all memory allocated should be in program space
    if(result_l) {
        ret->p_name = to_cstring(result_l->p_name);
        int idx = 0;
        if(result_l->p_aliases) {
            char** p = result_l->p_aliases;
            ptr_t* strings = from_ptrv(ret->p_aliases);
            while(*p) {
                strings[idx++] = to_cstring(*p);
                ++p;
            }
            strings[idx++] = 0;
        }
        ret->p_aliases = to_ptrv(result_l->p_aliases);
        ret->p_proto = result_l->p_proto;
    }
    return r;
}

typedef struct my_res_state_32_s {
	int	retrans;
	int	retry;
	ulong_t options;
	int	nscount;
	struct sockaddr_in
		nsaddr_list[3];
	unsigned short id;
	/* 2 byte hole here.  */
	ptr_t   dnsrch[7];  //char*
	char	defdname[256];
	ulong_t pfcode;
	unsigned ndots:4;
	unsigned nsort:4;
	unsigned ipv6_unavail:1;
	unsigned unused:23;
	struct {
		struct in_addr	addr;
		uint32_t	mask;
	} sort_list[10];
	/* 4 byte hole here on 64-bit architectures.  */
	ptr_t __glibc_unused_qhook; //void*
	ptr_t __glibc_unused_rhook; //void*
	int	res_h_errno;
	int	_vcsock;
	unsigned int _flags;
	/* 4 byte hole here on 64-bit architectures.  */
	union {
		char	pad[52];	/* On an i386 this means 512b total. */
		struct {
			uint16_t		nscount;
			uint16_t		nsmap[3];
			int			    nssocks[3];
			uint16_t		nscount6;
			uint16_t		nsinit;
			ptr_t           nsaddrs[3]; //struct sockaddr_in6*
			unsigned int		__glibc_reserved[2];
		} _ext;
	} _u;
} my_res_state_32_t;

void* convert_res_state_to_32(void* d, void* s)
{
    if(!d || !s) return NULL;
    struct __res_state* src = s;
    my_res_state_32_t* dst = d;

    dst->retrans = src->retrans;
    dst->retry = src->retry;
    dst->options = to_ulong(src->options);
    memmove(dst->nsaddr_list, src->nsaddr_list, sizeof(dst->nsaddr_list));
    dst->id = src->id;
    for(int i=0; i<7; ++i)
        dst->dnsrch[i] = to_ptrv(src->dnsrch[i]);
    memmove(dst->defdname, src->defdname, sizeof(dst->defdname));
    dst->pfcode = to_ulong(src->pfcode);
    dst->ndots = src->ndots;
    dst->nsort = src->nsort;
    dst->ipv6_unavail = src->ipv6_unavail;
    dst->unused = src->unused;
    memmove(dst->sort_list, src->sort_list, sizeof(dst->sort_list));
    dst->__glibc_unused_qhook = to_ptrv(src->__glibc_unused_qhook);
    dst->__glibc_unused_rhook = to_ptrv(src->__glibc_unused_rhook);
    dst->res_h_errno = src->res_h_errno;
    dst->_vcsock = src->_vcsock;
    dst->_flags = src->_flags;
    memmove(dst->_u.pad, src->_u.pad, sizeof(dst->_u.pad));

    return dst;
}

void* convert_res_state_to_64(void* d, void* s)
{
    if(!d || !s) return NULL;
    my_res_state_32_t* src = s;
    struct __res_state* dst = d;

    memmove(dst->_u.pad, src->_u.pad, sizeof(dst->_u.pad));
    dst->_flags = src->_flags;
    dst->_vcsock = src->_vcsock;
    dst->res_h_errno = src->res_h_errno;
    dst->__glibc_unused_rhook = from_ptrv(src->__glibc_unused_rhook);
    dst->__glibc_unused_qhook = from_ptrv(src->__glibc_unused_qhook);
    memmove(dst->sort_list, src->sort_list, sizeof(dst->sort_list));
    dst->unused = src->unused;
    dst->ipv6_unavail = src->ipv6_unavail;
    dst->nsort = src->nsort;
    dst->ndots = src->ndots;
    dst->pfcode = from_ulong(src->pfcode);
    memmove(dst->defdname, src->defdname, sizeof(dst->defdname));
    for(int i=6; i>=0; --i)
        dst->dnsrch[i] = from_ptrv(src->dnsrch[i]);
    dst->id = src->id;
    memmove(dst->nsaddr_list, src->nsaddr_list, sizeof(dst->nsaddr_list));
    dst->options = from_ulong(src->options);
    dst->retry = src->retry;
    dst->retrans = src->retrans;

    return dst;
}

EXPORT void* my32___res_state(x64emu_t* emu)
{
    if(emu->res_state_64)   // update res?
        convert_res_state_to_64(emu->res_state_64, emu->res_state_32);
    void* ret = __res_state();
    if(!ret)
        return ret;
    if(emu->res_state_64!=ret) {
        if(!emu->res_state_32)
            emu->res_state_32 = actual_calloc(1, sizeof(my_res_state_32_t));
        emu->res_state_64 = ret;
    }
    convert_res_state_to_32(emu->res_state_32, emu->res_state_64);
    return emu->res_state_32;
}

/*EXPORT void my32___res_iclose(x64emu_t* emu, void* s, int f)
{
    if(emu->res_state_64==s) {
        emu->res_state_64 = NULL;
        actual_free(emu->res_state_32);
        emu->res_state_32 = NULL;
    }
    __res_iclose(s, f);
}*/

EXPORT int my32_res_query(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    convert_res_state_to_64(emu->res_state_64, emu->res_state_32);
    int ret = res_query(dname, class, type, answer, anslen);
    emu->libc_herr = h_errno;
    return ret;
}

EXPORT int my32_res_search(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    convert_res_state_to_64(emu->res_state_64, emu->res_state_32);
    int ret = res_search(dname, class, type, answer, anslen);
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

EXPORT struct in6_addr my32_in6addr_any;
EXPORT struct in6_addr my32_in6addr_loopback;

void libc32_net_init()
{
    my32_in6addr_any = in6addr_any;
    my32_in6addr_loopback = in6addr_loopback;
}

int ioctl_cgifconf(x64emu_t* emu, int fd, void* arg)
{
    i386_ifconf_t *i386_conf = (i386_ifconf_t*)arg;
    if(!i386_conf->i386_ifc_buf)
    {
        return ioctl(fd, SIOCGIFCONF, i386_conf);
    }
    else
    {
        struct ifconf conf;
        conf.ifc_len = i386_conf->ifc_len;
        conf.ifc_buf = from_ptrv(i386_conf->i386_ifc_buf);
        int ret = ioctl(fd, SIOCGIFCONF, &conf);
        if(ret<0) return ret;
        i386_ifreq_t *i386_reqs = (i386_ifreq_t*)conf.ifc_buf;
        struct ifreq *reqs = conf.ifc_req;
        for(int i=0; i*sizeof(struct ifreq)<conf.ifc_len; ++i)
        {
            memmove(i386_reqs + i, reqs + i, offsetof(struct ifreq, ifr_map));
            i386_reqs[i].i386_ifr_map.mem_start = to_ulong_silent(reqs[i].ifr_map.mem_start);
            i386_reqs[i].i386_ifr_map.mem_end = to_ulong_silent(reqs[i].ifr_map.mem_end);
            i386_reqs[i].i386_ifr_map.base_addr = reqs[i].ifr_map.base_addr;
            i386_reqs[i].i386_ifr_map.irq = reqs[i].ifr_map.irq;
            i386_reqs[i].i386_ifr_map.dma = reqs[i].ifr_map.dma;
            i386_reqs[i].i386_ifr_map.port = reqs[i].ifr_map.port;
            memmove(&i386_reqs[i].i386_ifr_slave, &reqs[i].ifr_slave, sizeof(struct ifreq) - offsetof(struct ifreq, ifr_slave));
        }
        i386_conf->ifc_len = conf.ifc_len * sizeof(i386_ifreq_t) / sizeof(struct ifreq);
        return ret;
    }
}
