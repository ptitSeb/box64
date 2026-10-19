// SPDX-License-Identifier: MIT
#ifndef _BOX64_LIBC_CTYPE_H_
#define _BOX64_LIBC_CTYPE_H_

#ifdef HAVE_CTYPE_B_LOC_EXT

#include <stdint.h>

const unsigned short **__ctype_b_loc(void);
const int32_t **__ctype_tolower_loc(void);
const int32_t **__ctype_toupper_loc(void);
#endif

#endif