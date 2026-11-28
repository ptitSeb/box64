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
#include <wchar.h>
#include <poll.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <limits.h>

#include "debug.h"
#include "libc_fortify.h"


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


// glibc _FORTIFY_SOURCE support
// Bionic has its own fortify functions
#ifndef ANDROID

// failure handlers

// same message format as glibc's internal __fortify_fail
void fortify_fail(const char* msg)
{
    struct iovec iov[3] = {
        { (void*)"*** ", 4 },
        { (void*)msg, strlen(msg) },
        { (void*)" ***: terminated\n", 17 },
    };
    (void)!writev(STDERR_FILENO, iov, 3);
    abort();
}

#ifndef HAVE___CHK_FAIL
EXPORT void __chk_fail(void)
{
    fortify_fail("buffer overflow detected");
}
#endif

// runtime checks (__*_chk):

// memory and strings

#ifndef HAVE___MEMCPY_CHK
EXPORT void* __memcpy_chk(void* dest, const void* src, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return memcpy(dest, src, len);
}
#endif

#ifndef HAVE___MEMMOVE_CHK
EXPORT void* __memmove_chk(void* dest, const void* src, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return memmove(dest, src, len);
}
#endif

#ifndef HAVE___MEMPCPY_CHK
EXPORT void* __mempcpy_chk(void* dest, const void* src, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return mempcpy(dest, src, len);
}
#endif

#ifndef HAVE___MEMSET_CHK
EXPORT void* __memset_chk(void* dest, int c, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return memset(dest, c, len);
}
#endif

#ifndef HAVE___EXPLICIT_BZERO_CHK
EXPORT void __explicit_bzero_chk(void* dest, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    explicit_bzero(dest, len);
}
#endif

#ifndef HAVE___STRCPY_CHK
EXPORT char* __strcpy_chk(char* dest, const char* src, size_t destlen)
{
    size_t len = strlen(src);
    if (len >= destlen) __chk_fail();
    return memcpy(dest, src, len + 1);
}
#endif

#ifndef HAVE___STPCPY_CHK
EXPORT char* __stpcpy_chk(char* dest, const char* src, size_t destlen)
{
    size_t len = strlen(src);
    if (len >= destlen) __chk_fail();
    return (char*)memcpy(dest, src, len + 1) + len;
}
#endif

#ifndef HAVE___STRNCPY_CHK
EXPORT char* __strncpy_chk(char* dest, const char* src, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    return strncpy(dest, src, n);
}
#endif

#ifndef HAVE___STPNCPY_CHK
EXPORT char* __stpncpy_chk(char* dest, const char* src, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    return stpncpy(dest, src, n);
}
#endif

#ifndef HAVE___STRCAT_CHK
EXPORT char* __strcat_chk(char* dest, const char* src, size_t destlen)
{
    size_t dlen = strnlen(dest, destlen);
    size_t slen = strlen(src);
    if (dlen + slen >= destlen) __chk_fail();
    memcpy(dest + dlen, src, slen + 1);
    return dest;
}
#endif

#ifndef HAVE___STRNCAT_CHK
EXPORT char* __strncat_chk(char* dest, const char* src, size_t n, size_t destlen)
{
    size_t dlen = strnlen(dest, destlen);
    size_t slen = strnlen(src, n);
    if (dlen + slen >= destlen) __chk_fail();
    memcpy(dest + dlen, src, slen);
    dest[dlen + slen] = '\0';
    return dest;
}
#endif

#ifndef HAVE___STRLCPY_CHK
EXPORT size_t __strlcpy_chk(char* dest, const char* src, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    // no strlcpy before glibc 2.38
    size_t len = strlen(src);
    if (n) {
        size_t c = (len < n - 1) ? len : n - 1;
        memcpy(dest, src, c);
        dest[c] = '\0';
    }
    return len;
}
#endif

// wide characters, lengths in wchar_t

#ifndef HAVE___WMEMCPY_CHK
EXPORT wchar_t* __wmemcpy_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    return wmemcpy(dest, src, n);
}
#endif

#ifndef HAVE___WMEMMOVE_CHK
EXPORT wchar_t* __wmemmove_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    return wmemmove(dest, src, n);
}
#endif

#ifndef HAVE___WMEMSET_CHK
EXPORT wchar_t* __wmemset_chk(wchar_t* dest, wchar_t c, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    return wmemset(dest, c, n);
}
#endif

#ifndef HAVE___WCSCPY_CHK
EXPORT wchar_t* __wcscpy_chk(wchar_t* dest, const wchar_t* src, size_t destlen)
{
    size_t len = wcslen(src);
    if (len >= destlen) __chk_fail();
    return wmemcpy(dest, src, len + 1);
}
#endif

#ifndef HAVE___WCSNCPY_CHK
EXPORT wchar_t* __wcsncpy_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)
{
    if (destlen < n) __chk_fail();
    return wcsncpy(dest, src, n);
}
#endif

#ifndef HAVE___WCSCAT_CHK
EXPORT wchar_t* __wcscat_chk(wchar_t* dest, const wchar_t* src, size_t destlen)
{
    size_t dlen = wcsnlen(dest, destlen);
    size_t slen = wcslen(src);
    if (dlen + slen >= destlen) __chk_fail();
    wmemcpy(dest + dlen, src, slen + 1);
    return dest;
}
#endif

#ifndef HAVE___WCSNCAT_CHK
EXPORT wchar_t* __wcsncat_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)
{
    size_t dlen = wcsnlen(dest, destlen);
    size_t slen = wcsnlen(src, n);
    if (dlen + slen >= destlen) __chk_fail();
    wmemcpy(dest + dlen, src, slen);
    dest[dlen + slen] = L'\0';
    return dest;
}
#endif

// multibyte conversions

#ifndef HAVE___WCRTOMB_CHK
// glibc only fails if the converted character does not fit, not on buflen < MB_CUR_MAX
EXPORT size_t __wcrtomb_chk(char* s, wchar_t wc, mbstate_t* ps, size_t buflen)
{
    char buf[MB_LEN_MAX];
    size_t len = wcrtomb(s ? buf : NULL, wc, ps);
    if (!s || len == (size_t)-1) return len;
    if (len > buflen) __chk_fail();
    memcpy(s, buf, len);
    return len;
}
#endif

#ifndef HAVE___WCTOMB_CHK
EXPORT int __wctomb_chk(char* s, wchar_t wc, size_t buflen)
{
    if (buflen < MB_CUR_MAX) __chk_fail();
    return wctomb(s, wc);
}
#endif

#ifndef HAVE___MBSTOWCS_CHK
EXPORT size_t __mbstowcs_chk(wchar_t* dest, const char* src, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return mbstowcs(dest, src, len);
}
#endif

#ifndef HAVE___MBSRTOWCS_CHK
EXPORT size_t __mbsrtowcs_chk(wchar_t* dest, const char** src, size_t len, mbstate_t* ps, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return mbsrtowcs(dest, src, len, ps);
}
#endif

#ifndef HAVE___MBSNRTOWCS_CHK
EXPORT size_t __mbsnrtowcs_chk(wchar_t* dest, const char** src, size_t nmc, size_t len, mbstate_t* ps, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return mbsnrtowcs(dest, src, nmc, len, ps);
}
#endif

#ifndef HAVE___WCSTOMBS_CHK
EXPORT size_t __wcstombs_chk(char* dest, const wchar_t* src, size_t len, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return wcstombs(dest, src, len);
}
#endif

#ifndef HAVE___WCSRTOMBS_CHK
EXPORT size_t __wcsrtombs_chk(char* dest, const wchar_t** src, size_t len, mbstate_t* ps, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return wcsrtombs(dest, src, len, ps);
}
#endif

#ifndef HAVE___WCSNRTOMBS_CHK
EXPORT size_t __wcsnrtombs_chk(char* dest, const wchar_t** src, size_t nwc, size_t len, mbstate_t* ps, size_t destlen)
{
    if (destlen < len) __chk_fail();
    return wcsnrtombs(dest, src, nwc, len, ps);
}
#endif

// stdio

#if !defined(HAVE___FGETS_CHK) || !defined(HAVE___FGETS_UNLOCKED_CHK)
// glibc reads at most min(n - 1, size) chars and only fails if that fills the whole buffer
static char* fgets_chk(char* buf, size_t size, int n, FILE* fp, char* (*fgets_fn)(char*, int, FILE*), int locked)
{
    if (n <= 0) return NULL;
    if ((size_t)n <= size) return fgets_fn(buf, n, fp);
    if (locked) flockfile(fp);
    size_t count = 0;
    int c;
    while (count < size && (c = getc_unlocked(fp)) != EOF) {
        buf[count++] = (char)c;
        if (c == '\n') break;
    }
    int error = ferror(fp);
    if (locked) funlockfile(fp);
    if (!count || error) return NULL;
    if (count >= size) __chk_fail();
    buf[count] = '\0';
    return buf;
}
#endif

#ifndef HAVE___FGETS_CHK
EXPORT char* __fgets_chk(char* buf, size_t size, int n, FILE* fp)
{
    return fgets_chk(buf, size, n, fp, fgets, 1);
}
#endif

#ifndef HAVE___FGETS_UNLOCKED_CHK
EXPORT char* __fgets_unlocked_chk(char* buf, size_t size, int n, FILE* fp)
{
    return fgets_chk(buf, size, n, fp, fgets_unlocked, 0);
}
#endif

#ifndef HAVE___FREAD_CHK
EXPORT size_t __fread_chk(void* ptr, size_t ptrlen, size_t size, size_t n, FILE* fp)
{
    size_t bytes;
    if (__builtin_mul_overflow(size, n, &bytes) || bytes > ptrlen) __chk_fail();
    if (!bytes) return 0;
    return fread(ptr, size, n, fp);
}
#endif

#ifndef HAVE___FREAD_UNLOCKED_CHK
EXPORT size_t __fread_unlocked_chk(void* ptr, size_t ptrlen, size_t size, size_t n, FILE* fp)
{
    size_t bytes;
    if (__builtin_mul_overflow(size, n, &bytes) || bytes > ptrlen) __chk_fail();
    if (!bytes) return 0;
    return fread_unlocked(ptr, size, n, fp);
}
#endif

// system interfaces

#ifndef HAVE___CONFSTR_CHK
EXPORT size_t __confstr_chk(int name, char* buf, size_t len, size_t buflen)
{
    if (buflen < len) __chk_fail();
    return confstr(name, buf, len);
}
#endif

#ifndef HAVE___FDELT_CHK
EXPORT long int __fdelt_chk(long int d)
{
    if (d < 0 || d >= FD_SETSIZE) fortify_fail("bit out of range 0 - FD_SETSIZE on fd_set");
    return d / (8 * (long int)sizeof(long int));
}
#endif

#ifndef HAVE___GETGROUPS_CHK
EXPORT int __getgroups_chk(int size, gid_t list[], size_t listlen)
{
    if (size < 0) {
        errno = EINVAL;
        return -1;
    }
    if ((size_t)size * sizeof(gid_t) > listlen) __chk_fail();
    return getgroups(size, list);
}
#endif

#ifndef HAVE___GETHOSTNAME_CHK
EXPORT int __gethostname_chk(char* buf, size_t buflen, size_t nreal)
{
    if (buflen > nreal) __chk_fail();
    return gethostname(buf, buflen);
}
#endif

#ifndef HAVE___GETWD_CHK
EXPORT char* __getwd_chk(char* buf, size_t buflen)
{
    char* res = getcwd(buf, buflen);
    if (!res && errno == ERANGE) __chk_fail();
    return res;
}
#endif

#ifndef HAVE___POLL_CHK
EXPORT int __poll_chk(struct pollfd* fds, nfds_t nfds, int timeout, size_t fdslen)
{
    if (fdslen / sizeof(*fds) < nfds) __chk_fail();
    return poll(fds, nfds, timeout);
}
#endif

#ifndef HAVE___PPOLL_CHK
EXPORT int __ppoll_chk(struct pollfd* fds, nfds_t nfds, const struct timespec* timeout, const sigset_t* ss, size_t fdslen)
{
    if (fdslen / sizeof(*fds) < nfds) __chk_fail();
    return ppoll(fds, nfds, timeout, ss);
}
#endif

#ifndef HAVE___READ_CHK
EXPORT ssize_t __read_chk(int fd, void* buf, size_t nbytes, size_t buflen)
{
    if (nbytes > buflen) __chk_fail();
    return read(fd, buf, nbytes);
}
#endif

#ifndef HAVE___PREAD_CHK
EXPORT ssize_t __pread_chk(int fd, void* buf, size_t nbytes, off_t offset, size_t buflen)
{
    if (nbytes > buflen) __chk_fail();
    return pread(fd, buf, nbytes, offset);
}
#endif

#ifndef HAVE___PREAD64_CHK
// box64 hosts are 64-bit, so off_t is already 64-bit
EXPORT ssize_t __pread64_chk(int fd, void* buf, size_t nbytes, off_t offset, size_t buflen)
{
    if (nbytes > buflen) __chk_fail();
    return pread(fd, buf, nbytes, offset);
}
#endif

#ifndef HAVE___RECV_CHK
EXPORT ssize_t __recv_chk(int fd, void* buf, size_t len, size_t buflen, int flags)
{
    if (len > buflen) __chk_fail();
    return recv(fd, buf, len, flags);
}
#endif

#endif // ANDROID
