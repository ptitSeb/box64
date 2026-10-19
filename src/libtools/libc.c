// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>


#if !defined(HAVE_OBSTACK_VPRINTF) && !defined(ANDROID)
#include <obstack.h>

int obstack_vprintf(struct obstack *obstack,  const char *fmt, va_list args)
{
    char buffer[200];
    char *buf = buffer;
    va_list copy;
    
    va_copy(copy, args);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, copy);
    va_end(copy);

    if (len < 0) return -1;

    if ((size_t)len >= sizeof(buffer)) 
    {
        buf = malloc((size_t)len + 1);
        if (!buf) return -1;

        va_copy(copy, args);
        int n = vsnprintf(buf, len + 1, fmt, copy);
        va_end(copy);

        if (n != len) 
        {
            free(buf);
            return -1;
        }
        len = n;
    }

    if(len > 0) obstack_grow(obstack, buf, len);
    if(buf != buffer) free(buf);

	return len;
}
#endif

#if ! defined(HAVE_GETPROTOBYNAME_R) || ! defined(HAVE_GETPROTOBYNUMBER_R)

static pthread_mutex_t mutex_getproto = PTHREAD_MUTEX_INITIALIZER;

static int copyprotoent(const struct protoent *src, struct protoent *result_buf, char *buf, size_t buflen, struct protoent **result)
{
    size_t size_buf = 0;
    char *ptr = buf;
    *result = NULL;

    if(!src) return 0;

    // official protocol name
    size_t size_name = (strlen(src->p_name) + 1) * sizeof(char);
    size_buf += size_name;
    if(size_buf > buflen) return ERANGE;
    memcpy(ptr, src->p_name, size_name);
    result_buf->p_name = ptr;
    ptr += size_name;

    // padding
    size_t align   = _Alignof(char *);
    size_t pad_len = (align - (uintptr_t)ptr % align) % align;
    size_buf += pad_len;
    if(size_buf > buflen) return ERANGE;
    ptr += pad_len;

    // alias list
    size_t num_aliases = 0;
    while(src->p_aliases[num_aliases]) num_aliases++;
    size_t size_aliases = (num_aliases + 1) * sizeof(char*);
    size_buf += size_aliases;
    if(size_buf > buflen) return ERANGE;
    char **aliases = (char **) ptr;
    aliases[num_aliases] = NULL;
    result_buf->p_aliases = aliases;
    ptr += size_aliases;

    // aliases
    for(size_t i = 0; i < num_aliases; i++)
    {
        size_t size_alias = (strlen(src->p_aliases[i]) + 1) * sizeof(char);
        size_buf += size_alias;
        if(size_buf > buflen) return ERANGE;
        memcpy(ptr, src->p_aliases[i], size_alias);
        aliases[i] = ptr;
        ptr += size_alias;
    }

    // protocol number
    result_buf->p_proto = src->p_proto;

    // done
    *result = result_buf;
    return 0;
}

#ifndef HAVE_GETPROTOBYNAME_R
int getprotobyname_r(const char *name, struct protoent *result_buf, char *buf, size_t buflen, struct protoent **result)
{
    pthread_mutex_lock(&mutex_getproto);
    int res = copyprotoent(getprotobyname(name), result_buf, buf, buflen, result);
    pthread_mutex_unlock(&mutex_getproto);
    return res;
}
#endif

#ifndef HAVE_GETPROTOBYNUMBER_R
int getprotobynumber_r(int proto, struct protoent *result_buf, char *buf, size_t buflen, struct protoent **result)
{
    pthread_mutex_lock(&mutex_getproto);
    int res = copyprotoent(getprotobynumber(proto), result_buf, buf, buflen, result);
    pthread_mutex_unlock(&mutex_getproto);
    return res;
}
#endif
#endif
