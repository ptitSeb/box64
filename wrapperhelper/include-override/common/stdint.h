/* Copyright (C) 1997-2024 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.

This file has been adapted to work with the 'wrapperhelper' project on the 09/06/2024.
*/

/*
 *	ISO C99: 7.18 Integer types <stdint.h>
 */

#ifndef _STDINT_H
#define _STDINT_H	1

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>
#include <bits/types.h>
#include <bits/wchar.h>
#include <bits/wordsize.h>

#pragma wrappers allow_ints_ext

/* Exact integral types.  */

/* Signed.  */
#include <bits/stdint-intn.h>

/* Unsigned.  */
#include <bits/stdint-uintn.h>


/* Small types.  */
#include <bits/stdint-least.h>


/* Fast types.  */

/* Signed.  */
typedef int8_t int_fast8_t;
typedef long int_fast16_t;
typedef long int_fast32_t;
typedef int64_t int_fast64_t;

/* Unsigned.  */
typedef uint8_t uint_fast8_t;
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef uint64_t uint_fast64_t;


/* Types for `void *' pointers.  */
# ifndef __intptr_t_defined
typedef signed long intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned long uintptr_t;


/* Largest integral types.  */
typedef int64_t		intmax_t;
typedef uint64_t		uintmax_t;


# if __WORDSIZE == 64
#  define __INT64_C(c)	c ## L
#  define __UINT64_C(c)	c ## UL
# else
#  define __INT64_C(c)	c ## LL
#  define __UINT64_C(c)	c ## ULL
# endif

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
# define INT8_MIN		(-128)
# define INT16_MIN		(-32767-1)
# define INT32_MIN		(-2147483647-1)
# define INT64_MIN		(-__INT64_C(9223372036854775807)-1)
/* Maximum of signed integral types.  */
# define INT8_MAX		(127)
# define INT16_MAX		(32767)
# define INT32_MAX		(2147483647)
# define INT64_MAX		(__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
# define UINT8_MAX		(255)
# define UINT16_MAX		(65535)
# define UINT32_MAX		(4294967295U)
# define UINT64_MAX		(__UINT64_C(18446744073709551615))


/* Minimum of signed integral types having a minimum size.  */
# define INT_LEAST8_MIN  (-__INT_LEAST8_MAX__-1)
# define INT_LEAST16_MIN (-__INT_LEAST16_MAX__-1)
# define INT_LEAST32_MIN (-__INT_LEAST32_MAX__-1)
# define INT_LEAST64_MIN (-__INT_LEAST64_MAX__-1)
/* Maximum of signed integral types having a minimum size.  */
# define INT_LEAST8_MAX  __INT_LEAST8_MAX__
# define INT_LEAST16_MAX __INT_LEAST16_MAX__
# define INT_LEAST32_MAX __INT_LEAST32_MAX__
# define INT_LEAST64_MAX __INT_LEAST64_MAX__

/* Maximum of unsigned integral types having a minimum size.  */
# define UINT_LEAST8_MAX  __UINT_LEAST8_MAX__
# define UINT_LEAST16_MAX __UINT_LEAST16_MAX__
# define UINT_LEAST32_MAX __UINT_LEAST32_MAX__
# define UINT_LEAST64_MAX __UINT_LEAST64_MAX__


/* Minimum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MIN  (-__INT_FAST8_MAX__-1)
# define INT_FAST16_MIN (-__INT_FAST16_MAX__-1)
# define INT_FAST32_MIN (-__INT_FAST32_MAX__-1)
# define INT_FAST64_MIN (-__INT_FAST64_MAX__-1)
/* Maximum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MAX  __INT_FAST8_MAX__
# define INT_FAST16_MAX __INT_FAST16_MAX__
# define INT_FAST32_MAX __INT_FAST32_MAX__
# define INT_FAST64_MAX __INT_FAST64_MAX__

/* Maximum of fast unsigned integral types having a minimum size.  */
# define UINT_FAST8_MAX  __UINT_FAST8_MAX__
# define UINT_FAST16_MAX __UINT_FAST16_MAX__
# define UINT_FAST32_MAX __UINT_FAST32_MAX__
# define UINT_FAST64_MAX __UINT_FAST64_MAX__


/* Values to test for integral types holding `void *' pointer.  */
# define INTPTR_MIN  (-__INTPTR_MAX__-1)
# define INTPTR_MAX  __INTPTR_MAX__
# define UINTPTR_MAX __UINTPTR_MAX__

/* Minimum for largest signed integral type.  */
# define INTMAX_MIN (-__INTMAX_MAX__-1)
/* Maximum for largest signed integral type.  */
# define INTMAX_MAX __INTMAX_MAX__

/* Maximum for largest unsigned integral type.  */
# define UINTMAX_MAX __UINTMAX_MAX__

/* Limits of other integer types.  */

/* Limits of `ptrdiff_t' type.  */
# define PTRDIFF_MIN (-__PTRDIFF_MAX__-1)
# define PTRDIFF_MAX __PTRDIFF_MAX__

/* Limits of `sig_atomic_t'.  */
# define SIG_ATOMIC_MIN (-__SIG_ATOMIC_MAX__-1)
# define SIG_ATOMIC_MAX __SIG_ATOMIC_MAX__

/* Limit of `size_t' type.  */
# define SIZE_MAX __SIZE_MAX__

/* Limits of `wchar_t'.  */
# ifndef WCHAR_MIN
/* These constants might also be defined in <wchar.h>.  */
#  define WCHAR_MIN __WCHAR_MIN__
#  define WCHAR_MAX __WCHAR_MAX__
# endif

/* Limits of `wint_t'.  */
# define WINT_MIN __WINT_MIN__
# define WINT_MAX __WINT_MAX__

/* Signed.  */
# define INT8_C(c)  __INT8_C(c)
# define INT16_C(c) __INT16_C(c)
# define INT32_C(c) __INT32_C(c)
# define INT64_C(c) __INT64_C(c)

/* Unsigned.  */
# define UINT8_C(c)  __UINT8_C(c)
# define UINT16_C(c) __UINT16_C(c)
# define UINT32_C(c) __UINT32_C(c)
# define UINT64_C(c) __UINT64_C(c)

/* Maximal type.  */
# define INTMAX_C(c)  __INTMAX_C(c)
# define UINTMAX_C(c) __UINTMAX_C(c)

#if __GLIBC_USE (IEC_60559_BFP_EXT_C23)

# define INT8_WIDTH 8
# define UINT8_WIDTH 8
# define INT16_WIDTH 16
# define UINT16_WIDTH 16
# define INT32_WIDTH 32
# define UINT32_WIDTH 32
# define INT64_WIDTH 64
# define UINT64_WIDTH 64

# define INT_LEAST8_WIDTH 8
# define UINT_LEAST8_WIDTH 8
# define INT_LEAST16_WIDTH 16
# define UINT_LEAST16_WIDTH 16
# define INT_LEAST32_WIDTH 32
# define UINT_LEAST32_WIDTH 32
# define INT_LEAST64_WIDTH 64
# define UINT_LEAST64_WIDTH 64

# define INT_FAST8_WIDTH 8
# define UINT_FAST8_WIDTH 8
# define INT_FAST16_WIDTH __WORDSIZE
# define UINT_FAST16_WIDTH __WORDSIZE
# define INT_FAST32_WIDTH __WORDSIZE
# define UINT_FAST32_WIDTH __WORDSIZE
# define INT_FAST64_WIDTH 64
# define UINT_FAST64_WIDTH 64

# define INTPTR_WIDTH __WORDSIZE
# define UINTPTR_WIDTH __WORDSIZE

# define INTMAX_WIDTH 64
# define UINTMAX_WIDTH 64

# define PTRDIFF_WIDTH __WORDSIZE
# define SIG_ATOMIC_WIDTH 32
# define SIZE_WIDTH __WORDSIZE
# define WCHAR_WIDTH 32
# define WINT_WIDTH 32

#endif

#endif /* stdint.h */
