// SPDX-License-Identifier: MIT
#ifndef _BOX64_LIBC_OBSTACK_H_
#define _BOX64_LIBC_OBSTACK_H_

#ifndef HAVE_OBSTACK_VPRINTF
int obstack_vprintf(struct obstack *obstack,  const char *fmt, va_list args);
#endif

#endif