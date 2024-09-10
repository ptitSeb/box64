// C standard
#define __STDC__         1
#define __STDC_HOSTED__  1
#define __STDC_UTF_16__  1
#define __STDC_UTF_32__  1
#define __STDC_VERSION__ 201710L
// Generic x86_64 infos
#define __ELF__                 1
#define __NO_INLINE__           1
#define __ORDER_BIG_ENDIAN__    4321
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_PDP_ENDIAN__    3412
#define __PIC__                 2
#define __pic__                 2
#define __PIE__                 2
#define __pie__                 2
#define __SSP_STRONG__          3
#define __USER_LABEL_PREFIX__
#define __gnu_linux__           1
#define __linux__               1
#define __linux                 1
#define linux                   1
#define __unix__                1
#define __unix                  1
#define unix                    1
// GCC
//#define __GCC_ASM_FLAG_OUTPUTS__ 1
//#define __GCC_ATOMIC_BOOL_LOCK_FREE 2
//#define __GCC_ATOMIC_CHAR_LOCK_FREE 2
//#define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 2
//#define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 2
//#define __GCC_ATOMIC_INT_LOCK_FREE 2
//#define __GCC_ATOMIC_LLONG_LOCK_FREE 2
//#define __GCC_ATOMIC_LONG_LOCK_FREE 2
//#define __GCC_ATOMIC_POINTER_LOCK_FREE 2
//#define __GCC_ATOMIC_SHORT_LOCK_FREE 2
//#define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
//#define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 2
//#define __GCC_CONSTRUCTIVE_SIZE 64
//#define __GCC_DESTRUCTIVE_SIZE 64
//#define __GCC_HAVE_DWARF2_CFI_ASM 1
//#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1
//#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1
//#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1
//#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1
//#define __GCC_IEC_559 2
//#define __GCC_IEC_559_COMPLEX 2
//#define __GNUC__ 14
//#define __GNUC_EXECUTION_CHARSET_NAME "UTF-8"
//#define __GNUC_MINOR__ 2
//#define __GNUC_PATCHLEVEL__ 1
//#define __GNUC_STDC_INLINE__ 1
//#define __GNUC_WIDE_EXECUTION_CHARSET_NAME "UTF-32LE"
//#define __GXX_ABI_VERSION 1019
//#define __PRAGMA_REDEFINE_EXTNAME 1
//#define __VERSION__ "14.2.1 20240805"
// Specific x86_64 architecture
#define __FXSR__                      1
#define __FINITE_MATH_ONLY__          0
#define __HAVE_SPECULATION_SAFE_VALUE 1
#define __LP64__                      1
#define _LP64                         1
#define __MMX__                       1
#define __MMX_WITH_SSE__              1
#define __REGISTER_PREFIX__
#define __SEG_FS                      1
#define __SEG_GS                      1
#define __SSE__                       1
#define __SSE_MATH__                  1
#define __SSE2__                      1
#define __SSE2_MATH__                 1
#define __amd64__                     1
#define __amd64                       1
#define __code_model_small__          1
#define __k8__                        1
#define __k8                          1
#define __x86_64__                    1
#define __x86_64                      1
// Atomic
#define __ATOMIC_RELAXED     0
#define __ATOMIC_CONSUME     1
#define __ATOMIC_ACQUIRE     2
#define __ATOMIC_RELEASE     3
#define __ATOMIC_ACQ_REL     4
#define __ATOMIC_SEQ_CST     5
#define __ATOMIC_HLE_ACQUIRE 65536
#define __ATOMIC_HLE_RELEASE 131072
// Metainfo on types
#define __BIGGEST_ALIGNMENT__  16
#define __BYTE_ORDER__         __ORDER_LITTLE_ENDIAN__
#define __FLOAT_WORD_ORDER__   __ORDER_LITTLE_ENDIAN__
#define __CHAR_BIT__           8
#define __SIZEOF_SHORT__       2
#define __SIZEOF_WCHAR_T__     4
#define __SIZEOF_INT__         4
#define __SIZEOF_WINT_T__      4
#define __SIZEOF_LONG__        8
#define __SIZEOF_LONG_LONG__   8
#define __SIZEOF_POINTER__     8
#define __SIZEOF_PTRDIFF_T__   8
#define __SIZEOF_SIZE_T__      8
#define __SIZEOF_INT128__      16
#define __SIZEOF_FLOAT__       4
#define __SIZEOF_DOUBLE__      8
#define __SIZEOF_LONG_DOUBLE__ 16
#define __SIZEOF_FLOAT80__     16
#define __SIZEOF_FLOAT128__    16
// Integers
//#define __BITINT_MAXWIDTH__ 65535
//#define __CHAR16_TYPE__ short unsigned int
//#define __CHAR32_TYPE__ unsigned int
#define __INT8_C(c) c
#define __INT8_MAX__ 0x7f
//#define __INT8_TYPE__ signed char
#define __INT16_C(c) c
#define __INT16_MAX__ 0x7fff
//#define __INT16_TYPE__ short int
#define __INT32_C(c) c
#define __INT32_MAX__ 0x7fffffff
//#define __INT32_TYPE__ int
#define __INT64_C(c) c ## L
#define __INT64_MAX__ 0x7fffffffffffffffL
//#define __INT64_TYPE__ long int
#define __INT_FAST8_MAX__ 0x7f
//#define __INT_FAST8_TYPE__ signed char
#define __INT_FAST8_WIDTH__ 8
#define __INT_FAST16_MAX__ 0x7fffffffffffffffL
//#define __INT_FAST16_TYPE__ long int
#define __INT_FAST16_WIDTH__ 64
#define __INT_FAST32_MAX__ 0x7fffffffffffffffL
//#define __INT_FAST32_TYPE__ long int
#define __INT_FAST32_WIDTH__ 64
#define __INT_FAST64_MAX__ 0x7fffffffffffffffL
//#define __INT_FAST64_TYPE__ long int
#define __INT_FAST64_WIDTH__ 64
#define __INT_LEAST8_MAX__ 0x7f
//#define __INT_LEAST8_TYPE__ signed char
#define __INT_LEAST8_WIDTH__ 8
#define __INT_LEAST16_MAX__ 0x7fff
//#define __INT_LEAST16_TYPE__ short int
#define __INT_LEAST16_WIDTH__ 16
#define __INT_LEAST32_MAX__ 0x7fffffff
//#define __INT_LEAST32_TYPE__ int
#define __INT_LEAST32_WIDTH__ 32
#define __INT_LEAST64_MAX__ 0x7fffffffffffffffL
//#define __INT_LEAST64_TYPE__ long int
#define __INT_LEAST64_WIDTH__ 64
#define __INT_MAX__ 0x7fffffff
#define __INT_WIDTH__ 32
#define __INTMAX_C(c) c ## L
#define __INTMAX_MAX__ 0x7fffffffffffffffL
//#define __INTMAX_TYPE__ long int
#define __INTMAX_WIDTH__ 64
#define __INTPTR_MAX__ 0x7fffffffffffffffL
//#define __INTPTR_TYPE__ long int
#define __INTPTR_WIDTH__ 64
#define __LONG_LONG_MAX__ 0x7fffffffffffffffLL
#define __LONG_LONG_WIDTH__ 64
#define __LONG_MAX__ 0x7fffffffffffffffL
#define __LONG_WIDTH__ 64
#define __PTRDIFF_MAX__ 0x7fffffffffffffffL
//#define __PTRDIFF_TYPE__ long int
#define __PTRDIFF_WIDTH__ 64
#define __SCHAR_MAX__ 0x7f
#define __SCHAR_WIDTH__ 8
#define __SHRT_MAX__ 0x7fff
#define __SHRT_WIDTH__ 16
#define __SIG_ATOMIC_MAX__ 0x7fffffff
#define __SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)
//#define __SIG_ATOMIC_TYPE__ int
#define __SIG_ATOMIC_WIDTH__ 32
#define __SIZE_MAX__ 0xffffffffffffffffUL
//#define __SIZE_TYPE__ long unsigned int
#define __SIZE_WIDTH__ 64
#define __UINT8_C(c) c
#define __UINT8_MAX__ 0xff
//#define __UINT8_TYPE__ unsigned char
#define __UINT16_C(c) c
#define __UINT16_MAX__ 0xffff
//#define __UINT16_TYPE__ short unsigned int
#define __UINT32_C(c) c ## U
#define __UINT32_MAX__ 0xffffffffU
//#define __UINT32_TYPE__ unsigned int
#define __UINT64_C(c) c ## UL
#define __UINT64_MAX__ 0xffffffffffffffffUL
//#define __UINT64_TYPE__ long unsigned int
#define __UINT_FAST8_MAX__ 0xff
//#define __UINT_FAST8_TYPE__ unsigned char
#define __UINT_FAST16_MAX__ 0xffffffffffffffffUL
//#define __UINT_FAST16_TYPE__ long unsigned int
#define __UINT_FAST32_MAX__ 0xffffffffffffffffUL
//#define __UINT_FAST32_TYPE__ long unsigned int
#define __UINT_FAST64_MAX__ 0xffffffffffffffffUL
//#define __UINT_FAST64_TYPE__ long unsigned int
#define __UINT_LEAST8_MAX__ 0xff
//#define __UINT_LEAST8_TYPE__ unsigned char
#define __UINT_LEAST16_MAX__ 0xffff
//#define __UINT_LEAST16_TYPE__ short unsigned int
#define __UINT_LEAST32_MAX__ 0xffffffffU
//#define __UINT_LEAST32_TYPE__ unsigned int
#define __UINT_LEAST64_MAX__ 0xffffffffffffffffUL
//#define __UINT_LEAST64_TYPE__ long unsigned int
#define __UINTMAX_C(c) c ## UL
#define __UINTMAX_MAX__ 0xffffffffffffffffUL
//#define __UINTMAX_TYPE__ long unsigned int
#define __UINTPTR_MAX__ 0xffffffffffffffffUL
//#define __UINTPTR_TYPE__ long unsigned int
#define __WCHAR_MAX__ 0x7fffffff
#define __WCHAR_MIN__ (-__WCHAR_MAX__ - 1)
//#define __WCHAR_TYPE__ int
#define __WCHAR_WIDTH__ 32
#define __WINT_MAX__ 0xffffffffU
#define __WINT_MIN__ 0U
//#define __WINT_TYPE__ unsigned int
#define __WINT_WIDTH__ 32
// Floats
//#define __BFLT16_DECIMAL_DIG__ 4
//#define __BFLT16_DENORM_MIN__ 9.18354961579912115600575419704879436e-41BF16
//#define __BFLT16_DIG__ 2
//#define __BFLT16_EPSILON__ 7.81250000000000000000000000000000000e-3BF16
//#define __BFLT16_HAS_DENORM__ 1
//#define __BFLT16_HAS_INFINITY__ 1
//#define __BFLT16_HAS_QUIET_NAN__ 1
//#define __BFLT16_IS_IEC_60559__ 0
//#define __BFLT16_MANT_DIG__ 8
//#define __BFLT16_MAX_10_EXP__ 38
//#define __BFLT16_MAX__ 3.38953138925153547590470800371487867e+38BF16
//#define __BFLT16_MAX_EXP__ 128
//#define __BFLT16_MIN_10_EXP__ (-37)
//#define __BFLT16_MIN__ 1.17549435082228750796873653722224568e-38BF16
//#define __BFLT16_MIN_EXP__ (-125)
//#define __BFLT16_NORM_MAX__ 3.38953138925153547590470800371487867e+38BF16
#define __DBL_DECIMAL_DIG__ 17
#define __DBL_DENORM_MIN__ ((double)4.94065645841246544176568792868221372e-324L)
#define __DBL_DIG__ 15
#define __DBL_EPSILON__ ((double)2.22044604925031308084726333618164062e-16L)
#define __DBL_HAS_DENORM__ 1
#define __DBL_HAS_INFINITY__ 1
#define __DBL_HAS_QUIET_NAN__ 1
#define __DBL_IS_IEC_60559__ 1
#define __DBL_MANT_DIG__ 53
#define __DBL_MAX_10_EXP__ 308
#define __DBL_MAX__ ((double)1.79769313486231570814527423731704357e+308L)
#define __DBL_MAX_EXP__ 1024
#define __DBL_MIN_10_EXP__ (-307)
#define __DBL_MIN__ ((double)2.22507385850720138309023271733240406e-308L)
#define __DBL_MIN_EXP__ (-1021)
#define __DBL_NORM_MAX__ ((double)1.79769313486231570814527423731704357e+308L)
//#define __DEC32_EPSILON__ 1E-6DF
//#define __DEC32_MANT_DIG__ 7
//#define __DEC32_MAX__ 9.999999E96DF
//#define __DEC32_MAX_EXP__ 97
//#define __DEC32_MIN__ 1E-95DF
//#define __DEC32_MIN_EXP__ (-94)
//#define __DEC32_SUBNORMAL_MIN__ 0.000001E-95DF
//#define __DEC64_EPSILON__ 1E-15DD
//#define __DEC64_MANT_DIG__ 16
//#define __DEC64_MAX__ 9.999999999999999E384DD
//#define __DEC64_MAX_EXP__ 385
//#define __DEC64_MIN__ 1E-383DD
//#define __DEC64_MIN_EXP__ (-382)
//#define __DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD
//#define __DEC128_EPSILON__ 1E-33DL
//#define __DEC128_MANT_DIG__ 34
//#define __DEC128_MAX__ 9.999999999999999999999999999999999E6144DL
//#define __DEC128_MAX_EXP__ 6145
//#define __DEC128_MIN__ 1E-6143DL
//#define __DEC128_MIN_EXP__ (-6142)
//#define __DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL
//#define __DEC_EVAL_METHOD__ 2
//#define __DECIMAL_BID_FORMAT__ 1
//#define __DECIMAL_DIG__ 21
//#define __FLT16_DECIMAL_DIG__ 5
//#define __FLT16_DENORM_MIN__ 5.96046447753906250000000000000000000e-8F16
//#define __FLT16_DIG__ 3
//#define __FLT16_EPSILON__ 9.76562500000000000000000000000000000e-4F16
//#define __FLT16_HAS_DENORM__ 1
//#define __FLT16_HAS_INFINITY__ 1
//#define __FLT16_HAS_QUIET_NAN__ 1
//#define __FLT16_IS_IEC_60559__ 1
//#define __FLT16_MANT_DIG__ 11
//#define __FLT16_MAX_10_EXP__ 4
//#define __FLT16_MAX__ 6.55040000000000000000000000000000000e+4F16
//#define __FLT16_MAX_EXP__ 16
//#define __FLT16_MIN_10_EXP__ (-4)
//#define __FLT16_MIN__ 6.10351562500000000000000000000000000e-5F16
//#define __FLT16_MIN_EXP__ (-13)
//#define __FLT16_NORM_MAX__ 6.55040000000000000000000000000000000e+4F16
//#define __FLT32_DECIMAL_DIG__ 9
//#define __FLT32_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F32
//#define __FLT32_DIG__ 6
//#define __FLT32_EPSILON__ 1.19209289550781250000000000000000000e-7F32
//#define __FLT32_HAS_DENORM__ 1
//#define __FLT32_HAS_QUIET_NAN__ 1
//#define __FLT32_HAS_INFINITY__ 1
//#define __FLT32_IS_IEC_60559__ 1
//#define __FLT32_MAX_10_EXP__ 38
//#define __FLT32_MAX__ 3.40282346638528859811704183484516925e+38F32
//#define __FLT32_MAX_EXP__ 128
//#define __FLT32_MIN_10_EXP__ (-37)
//#define __FLT32_MIN__ 1.17549435082228750796873653722224568e-38F32
//#define __FLT32_MIN_EXP__ (-125)
//#define __FLT32_MANT_DIG__ 24
//#define __FLT32_NORM_MAX__ 3.40282346638528859811704183484516925e+38F32
//#define __FLT32X_DECIMAL_DIG__ 17
//#define __FLT32X_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F32x
//#define __FLT32X_DIG__ 15
//#define __FLT32X_EPSILON__ 2.22044604925031308084726333618164062e-16F32x
//#define __FLT32X_HAS_DENORM__ 1
//#define __FLT32X_HAS_INFINITY__ 1
//#define __FLT32X_HAS_QUIET_NAN__ 1
//#define __FLT32X_IS_IEC_60559__ 1
//#define __FLT32X_MANT_DIG__ 53
//#define __FLT32X_MAX_10_EXP__ 308
//#define __FLT32X_MAX__ 1.79769313486231570814527423731704357e+308F32x
//#define __FLT32X_MAX_EXP__ 1024
//#define __FLT32X_MIN_10_EXP__ (-307)
//#define __FLT32X_MIN__ 2.22507385850720138309023271733240406e-308F32x
//#define __FLT32X_MIN_EXP__ (-1021)
//#define __FLT32X_NORM_MAX__ 1.79769313486231570814527423731704357e+308F32x
//#define __FLT64_DECIMAL_DIG__ 17
//#define __FLT64_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F64
//#define __FLT64_DIG__ 15
//#define __FLT64_EPSILON__ 2.22044604925031308084726333618164062e-16F64
//#define __FLT64_HAS_DENORM__ 1
//#define __FLT64_HAS_INFINITY__ 1
//#define __FLT64_HAS_QUIET_NAN__ 1
//#define __FLT64_IS_IEC_60559__ 1
//#define __FLT64_MANT_DIG__ 53
//#define __FLT64_MAX_10_EXP__ 308
//#define __FLT64_MAX__ 1.79769313486231570814527423731704357e+308F64
//#define __FLT64_MAX_EXP__ 1024
//#define __FLT64_MIN_10_EXP__ (-307)
//#define __FLT64_MIN__ 2.22507385850720138309023271733240406e-308F64
//#define __FLT64_MIN_EXP__ (-1021)
//#define __FLT64_NORM_MAX__ 1.79769313486231570814527423731704357e+308F64
//#define __FLT64X_DECIMAL_DIG__ 21
//#define __FLT64X_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951F64x
//#define __FLT64X_DIG__ 18
//#define __FLT64X_EPSILON__ 1.08420217248550443400745280086994171e-19F64x
//#define __FLT64X_HAS_DENORM__ 1
//#define __FLT64X_HAS_INFINITY__ 1
//#define __FLT64X_HAS_QUIET_NAN__ 1
//#define __FLT64X_IS_IEC_60559__ 1
//#define __FLT64X_MANT_DIG__ 64
//#define __FLT64X_MAX_10_EXP__ 4932
//#define __FLT64X_MAX__ 1.18973149535723176502126385303097021e+4932F64x
//#define __FLT64X_MAX_EXP__ 16384
//#define __FLT64X_MIN_10_EXP__ (-4931)
//#define __FLT64X_MIN__ 3.36210314311209350626267781732175260e-4932F64x
//#define __FLT64X_MIN_EXP__ (-16381)
//#define __FLT64X_NORM_MAX__ 1.18973149535723176502126385303097021e+4932F64x
//#define __FLT128_DECIMAL_DIG__ 36
//#define __FLT128_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966F128
//#define __FLT128_DIG__ 33
//#define __FLT128_EPSILON__ 1.92592994438723585305597794258492732e-34F128
//#define __FLT128_HAS_DENORM__ 1
//#define __FLT128_HAS_INFINITY__ 1
//#define __FLT128_HAS_QUIET_NAN__ 1
//#define __FLT128_IS_IEC_60559__ 1
//#define __FLT128_MANT_DIG__ 113
//#define __FLT128_MAX__ 1.18973149535723176508575932662800702e+4932F128
//#define __FLT128_MAX_10_EXP__ 4932
//#define __FLT128_MAX_EXP__ 16384
//#define __FLT128_MIN_10_EXP__ (-4931)
//#define __FLT128_MIN__ 3.36210314311209350626267781732175260e-4932F128
//#define __FLT128_MIN_EXP__ (-16381)
//#define __FLT128_NORM_MAX__ 1.18973149535723176508575932662800702e+4932F128
#define __FLT_DECIMAL_DIG__ 9
#define __FLT_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F
#define __FLT_DIG__ 6
#define __FLT_EPSILON__ 1.19209289550781250000000000000000000e-7F
#define __FLT_EVAL_METHOD__ 0
#define __FLT_EVAL_METHOD_TS_18661_3__ 0
#define __FLT_HAS_DENORM__ 1
#define __FLT_HAS_INFINITY__ 1
#define __FLT_HAS_QUIET_NAN__ 1
#define __FLT_IS_IEC_60559__ 1
#define __FLT_MANT_DIG__ 24
#define __FLT_MAX_10_EXP__ 38
#define __FLT_MAX__ 3.40282346638528859811704183484516925e+38F
#define __FLT_MAX_EXP__ 128
#define __FLT_MIN_10_EXP__ (-37)
#define __FLT_MIN__ 1.17549435082228750796873653722224568e-38F
#define __FLT_MIN_EXP__ (-125)
#define __FLT_NORM_MAX__ 3.40282346638528859811704183484516925e+38F
#define __FLT_RADIX__ 2
#define __LDBL_DECIMAL_DIG__ 21
#define __LDBL_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951L
#define __LDBL_DIG__ 18
#define __LDBL_EPSILON__ 1.08420217248550443400745280086994171e-19L
#define __LDBL_HAS_DENORM__ 1
#define __LDBL_HAS_INFINITY__ 1
#define __LDBL_HAS_QUIET_NAN__ 1
#define __LDBL_IS_IEC_60559__ 1
#define __LDBL_MANT_DIG__ 64
#define __LDBL_MAX_10_EXP__ 4932
#define __LDBL_MAX__ 1.18973149535723176502126385303097021e+4932L
#define __LDBL_MAX_EXP__ 16384
#define __LDBL_MIN_10_EXP__ (-4931)
#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
#define __LDBL_MIN_EXP__ (-16381)
#define __LDBL_NORM_MAX__ 1.18973149535723176502126385303097021e+4932L

#include_next "stdc-predef.h"
