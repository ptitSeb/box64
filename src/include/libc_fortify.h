// SPDX-License-Identifier: MIT
#ifndef _BOX64_LIBC_FORTIFY_H_
#define _BOX64_LIBC_FORTIFY_H_

void fortify_fail(const char* msg) __attribute__((noreturn));
void __chk_fail(void) __attribute__((noreturn));

#endif //_BOX64_LIBC_FORTIFY_H_
