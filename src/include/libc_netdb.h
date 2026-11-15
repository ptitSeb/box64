// SPDX-License-Identifier: MIT
#ifndef _BOX64_LIBC_NETDB_H_
#define _BOX64_LIBC_NETDB_H_

#ifndef HAVE_GETPROTOBYNAME_R
int getprotobyname_r(const char *name, struct protoent *result_buf, char *buf, size_t buflen, struct protoent **result);
#endif

#ifndef HAVE_GETPROTOBYNUMBER_R
int getprotobynumber_r(int proto, struct protoent *result_buf, char *buf, size_t buflen, struct protoent **result);
#endif

#endif