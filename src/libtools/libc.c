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
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>


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


#ifndef HAVE_SCANDIRAT

// Copyright (c) 2015-2016 Nuxi, https://nuxi.nl/
// SPDX-License-Identifier: BSD-2-Clause
// https://github.com/WebAssembly/wasi-libc/blob/main/libc-bottom-half/cloudlibc/src/libc/dirent/scandirat.c
int scandirat(int dirfd, const char *dir, struct dirent ***res,
              int (*sel)(const struct dirent *),
              int (*cmp)(const struct dirent **, const struct dirent **))
{
    dirfd = openat(dirfd, dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        return -1;
    }

    DIR *d = fdopendir(dirfd);
    struct dirent *de, **names=0, **tmp;
    size_t cnt=0, len=0;
    int old_errno = errno;

    if (!d) {
        close(dirfd);
        return -1;
    }

    while ((errno=0), (de = readdir(d))) {
        if (sel && !sel(de)) continue;
        if (cnt >= len) {
            len = 2*len+1;
            if (len > SIZE_MAX/sizeof *names) break;
            tmp = realloc(names, len * sizeof *names);
            if (!tmp) break;
            names = tmp;
        }
        size_t size = de->d_reclen;
        names[cnt] = malloc(size);
        if (!names[cnt]) break;
        memcpy(names[cnt++], de, size);
    }

    closedir(d);

    if (errno) {
        if (names) while (cnt-->0) free(names[cnt]);
        free(names);
        return -1;
    }
    errno = old_errno;

    if (cmp) qsort(names, cnt, sizeof *names, (int (*)(const void *, const void *))cmp);
    *res = names;
    return cnt;
}
#endif
