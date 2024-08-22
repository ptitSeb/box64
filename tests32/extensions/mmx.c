#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include<limits.h>
#include<immintrin.h>
#include<cpuid.h>



typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define I8_MAX  0x7F
#define I8_MIN -0x80
#define U8_MAX  0xFF
#define U8_MIN  0

#define I16_MAX  0x7FFF
#define I16_MIN -0x8000
#define U16_MAX  0xFFFF
#define U16_MIN  0

#define I32_MAX  0x7FFFFFFF
#define I32_MIN -0x80000000
#define U32_MAX  0xFFFFFFFF
#define U32_MIN  0

#define I64_MAX  0x7FFFFFFFFFFFFFFF
#define I64_MIN -0x8000000000000000
#define U64_MAX  0xFFFFFFFFFFFFFFFF
#define U64_MIN  0

#define MMX_TEST_STRUCT(sz) \
	typedef struct mmx_##sz##_test { \
		sz a; \
		sz b; \
		sz result; \
	} mmx_##sz##_test_t

MMX_TEST_STRUCT(u8);
MMX_TEST_STRUCT(i8);
MMX_TEST_STRUCT(u16);
MMX_TEST_STRUCT(i16);
MMX_TEST_STRUCT(u32);
MMX_TEST_STRUCT(i32);
MMX_TEST_STRUCT(u64);
MMX_TEST_STRUCT(i64);

// Binary compare two mm registers
bool mm_raw_compare(__m64 a, __m64 b) {
	__m64 a_upper_reg = _mm_srli_si64(a, 32);
	__m64 b_upper_reg = _mm_srli_si64(b, 32);

	int a_lower = _m_to_int(a);
	int a_upper = _m_to_int(a_upper_reg);

	int b_lower = _m_to_int(b);
	int b_upper = _m_to_int(b_upper_reg);

	return (a_lower == b_lower) && (a_upper == b_upper);
}

// Load a 64 bit value into a mm register
__m64 mm_load64(u64 val) {
	__m64 lower = _m_from_int(val & 0xFFFFFFFF);
	__m64 upper = _m_from_int((val >> 32) & 0xFFFFFFFF);

	__m64 shifted = _mm_slli_si64(upper, 32);
	__m64 final = _m_por(shifted, lower);

	return final;
}

#define MMX_ARITH_TEST(name, testcases, testcase_type, type, size, testfunc) \
bool name() { \
	printf("TEST: " #name "\n"); \
	int errors = 0; \
\
	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++ ) { \
		testcase_type test_data = testcases[i]; \
\
		__m64 a = _mm_set1_pi##size(test_data.a); \
		__m64 b = _mm_set1_pi##size(test_data.b); \
		__m64 expected = _mm_set1_pi##size(test_data.result); \
		__m64 result = testfunc(a, b); \
\
		bool success = mm_raw_compare(expected, result); \
		errors += (int) (!success); \
	} \
\
	_m_empty(); \
	printf("TEST: finished with: %d errors\n", errors); \
	return errors; \
}

#define MMX_SHIFT_TEST(name, testcases, testfunc) \
bool name() { \
	printf("TEST: " #name "\n"); \
	int errors = 0; \
\
	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++ ) { \
		mmx_u64_test_t test_data = testcases[i]; \
\
		__m64 a = mm_load64(test_data.a); \
		__m64 expected = mm_load64(test_data.result); \
		__m64 result = testfunc(a, test_data.b); \
\
		bool success = mm_raw_compare(expected, result); \
		if (!success) { \
			printf( \
				"Failed; Expected: 0x%08x_%08x\tGot: 0x%08x_%08x\n", \
				_m_to_int(_mm_srli_si64(expected, 32)), \
				_m_to_int(expected), \
				_m_to_int(_mm_srli_si64(result, 32)), \
				_m_to_int(result) \
			); \
		} \
		errors += (int) (!success); \
	} \
\
	_m_empty(); \
	printf("TEST: finished with: %d errors\n", errors); \
	return errors; \
}



// Loads 2 64 bit immediates and compares with the third
// Test data must be of type mmx_u64_test_t
#define MMX_64_TEST(name, testcases, testfunc) \
bool name() { \
	printf("TEST: " #name "\n"); \
	int errors = 0; \
\
	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++ ) { \
		mmx_u64_test_t test_data = testcases[i]; \
\
		__m64 a = mm_load64(test_data.a); \
		__m64 b = mm_load64(test_data.b); \
		__m64 expected = mm_load64(test_data.result); \
		__m64 result = testfunc(a, b); \
\
		bool success = mm_raw_compare(expected, result); \
		if (!success) { \
			printf( \
				"Failed; Expected: 0x%08x_%08x\tGot: 0x%08x_%08x\n", \
				_m_to_int(_mm_srli_si64(expected, 32)), \
				_m_to_int(expected), \
				_m_to_int(_mm_srli_si64(result, 32)), \
				_m_to_int(result) \
			); \
		} \
		errors += (int) (!success); \
	} \
\
	_m_empty(); \
	printf("TEST: finished with: %d errors\n", errors); \
	return errors; \
}


mmx_i8_test_t mmx_i8_add_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = I8_MAX, .b = 1, .result = I8_MIN },
	{ .a = I8_MIN, .b = -1, .result = I8_MAX },
	{ .a = 0, .b = U8_MAX, .result = U8_MAX },
};
mmx_i8_test_t mmx_i8_add_sat_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = I8_MAX, .b = 1, .result = I8_MAX },
	{ .a = I8_MIN, .b = -1, .result = I8_MIN },
};
mmx_u8_test_t mmx_u8_add_sat_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = U8_MAX, .b = 1, .result = U8_MAX },
	{ .a = 0, .b = U8_MAX, .result = U8_MAX },
};

mmx_i16_test_t mmx_i16_add_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = I16_MAX, .b = 1, .result = I16_MIN },
	{ .a = I16_MIN, .b = -1, .result = I16_MAX },
};
mmx_i16_test_t mmx_i16_add_sat_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = I16_MAX, .b = 1, .result = I16_MAX },
	{ .a = I16_MIN, .b = -1, .result = I16_MIN },
};
mmx_u16_test_t mmx_u16_add_sat_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = U16_MAX, .b = 1, .result = U16_MAX },
	{ .a = 0, .b = U16_MAX, .result = U16_MAX },
};

mmx_i32_test_t mmx_i32_add_test_data[] = {
	{ .a = 1, .b = 2, .result = 3 },
	{ .a = 0, .b = 1, .result = 1 },
	{ .a = I32_MAX, .b = 1, .result = I32_MIN },
	{ .a = I32_MIN, .b = -1, .result = I32_MAX },
};

MMX_ARITH_TEST(test_mmx_paddb, mmx_i8_add_test_data, mmx_i8_test_t, i8, 8, _m_paddb);
MMX_ARITH_TEST(test_mmx_paddsb, mmx_i8_add_sat_test_data, mmx_i8_test_t, i8, 8, _m_paddsb);
MMX_ARITH_TEST(test_mmx_paddusb, mmx_u8_add_sat_test_data, mmx_u8_test_t, u8, 8, _m_paddusb);

MMX_ARITH_TEST(test_mmx_paddw, mmx_i16_add_test_data, mmx_i16_test_t, i16, 16, _m_paddw);
MMX_ARITH_TEST(test_mmx_paddsw, mmx_i16_add_sat_test_data, mmx_i16_test_t, i16, 16, _m_paddsw);
MMX_ARITH_TEST(test_mmx_paddusw, mmx_u16_add_sat_test_data, mmx_u16_test_t, u16, 16, _m_paddusw);

MMX_ARITH_TEST(test_mmx_paddd, mmx_i32_add_test_data, mmx_i32_test_t, i32, 32, _m_paddd);



mmx_i8_test_t mmx_i8_sub_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = I8_MIN, .b = 1, .result = I8_MAX },
	{ .a = I8_MAX, .b = -1, .result = I8_MIN },
	{ .a = U8_MAX, .b = U8_MAX, .result = 0 },
};
mmx_i8_test_t mmx_i8_sub_sat_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = I8_MIN, .b = 1, .result = I8_MIN },
	{ .a = I8_MAX, .b = -1, .result = I8_MAX },
};
mmx_u8_test_t mmx_u8_sub_sat_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = U8_MIN, .b = 1, .result = U8_MIN },
	{ .a = U8_MAX, .b = U8_MAX, .result = 0 },
};

mmx_i16_test_t mmx_i16_sub_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = I16_MIN, .b = 1, .result = I16_MAX },
	{ .a = I16_MAX, .b = -1, .result = I16_MIN },
};
mmx_i16_test_t mmx_i16_sub_sat_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = I16_MIN, .b = 1, .result = I16_MIN },
	{ .a = I16_MAX, .b = -1, .result = I16_MAX },
};
mmx_u16_test_t mmx_u16_sub_sat_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = U16_MIN, .b = 1, .result = U16_MIN },
	{ .a = U16_MIN, .b = U16_MIN, .result = 0 },
};

mmx_i32_test_t mmx_i32_sub_test_data[] = {
	{ .a = 3, .b = 2, .result = 1 },
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = I32_MIN, .b = 1, .result = I32_MAX },
	{ .a = I32_MAX, .b = -1, .result = I32_MIN },
};

MMX_ARITH_TEST(test_mmx_psubb, mmx_i8_sub_test_data, mmx_i8_test_t, i8, 8, _m_psubb);
MMX_ARITH_TEST(test_mmx_psubsb, mmx_i8_sub_sat_test_data, mmx_i8_test_t, i8, 8, _m_psubsb);
MMX_ARITH_TEST(test_mmx_psubusb, mmx_u8_sub_sat_test_data, mmx_u8_test_t, u8, 8, _m_psubusb);

MMX_ARITH_TEST(test_mmx_psubw, mmx_i16_sub_test_data, mmx_i16_test_t, i16, 16, _m_psubw);
MMX_ARITH_TEST(test_mmx_psubuw, mmx_i16_sub_sat_test_data, mmx_i16_test_t, i16, 16, _m_psubsw);
MMX_ARITH_TEST(test_mmx_psubusw, mmx_u16_sub_sat_test_data, mmx_u16_test_t, u16, 16, _m_psubusw);

MMX_ARITH_TEST(test_mmx_psubd, mmx_i32_sub_test_data, mmx_i32_test_t, i32, 32, _m_psubd);




mmx_u64_test_t mmx_por_test_data[] = {
	{ .a = 0xAAAAAAAAAAAAAAAA,
	  .b = 0x5555555555555555,
	  .result = 0xFFFFFFFFFFFFFFFF },
	{ .a = 0x0000000000000000,
	  .b = 0x1111111111111111,
	  .result = 0x1111111111111111 },
};

mmx_u64_test_t mmx_pand_test_data[] = {
	{ .a = 0xAAAAAAAAAAAAAAAA,
	  .b = 0x5555555555555555,
	  .result = 0x0000000000000000 },
	{ .a = 0xFFFFFFFFFFFFFFFF,
	  .b = 0xFFFFFFFFFFFFFFFF,
	  .result = 0xFFFFFFFFFFFFFFFF },
};

mmx_u64_test_t mmx_pandn_test_data[] = {
	{ .a = 0x0000000000000000,
	  .b = 0xFFFFFFFFFFFFFFFF,
	  .result = 0xFFFFFFFFFFFFFFFF },
	{ .a = 0xFFFFFFFFFFFFFFFF,
	  .b = 0x0000000000000000,
	  .result = 0x0000000000000000 },
};


mmx_u64_test_t mmx_pxor_test_data[] = {
	{ .a = 0xAAAAAAAAAAAAAAAA,
	  .b = 0x5555555555555555,
	  .result = 0xFFFFFFFFFFFFFFFF },
	{ .a = 0xFFFFFFFFFFFFFFFF,
	  .b = 0xFFFFFFFFFFFFFFFF,
	  .result = 0x0000000000000000 },
};


MMX_64_TEST(test_mmx_por, mmx_por_test_data, _m_por);
MMX_64_TEST(test_mmx_pand, mmx_pand_test_data, _m_pand);
MMX_64_TEST(test_mmx_pandn, mmx_pandn_test_data, _m_pandn);
MMX_64_TEST(test_mmx_pxor, mmx_pxor_test_data, _m_pxor);





mmx_i16_test_t mmx_pmullw_test_data[] = {
	{ .a = 10, .b = 10, .result = 100 },
	{ .a = 32000, .b = 10, .result = 0xE200 },
	{ .a = 20000, .b = 20000, .result = 0x8400 },
};
mmx_i16_test_t mmx_pmulhw_test_data[] = {
	{ .a = 10, .b = 10, .result = 0 },
	{ .a = 32000, .b = 10, .result = 4 },
	{ .a = 20000, .b = 20000, .result = 0x17D7 },
};
mmx_u64_test_t mmx_pmaddwd_test_data[] = {
	{ .a = 0x0000000100000001,
	  .b = 0x0000000100000001,
	  .result = 0x0000000100000001 },
	{ .a = 0x0000000200000004,
	  .b = 0x0000000200000004,
	  .result = 0x0000000400000010 },

	{ .a = 0x000000007FFFFFFF,
	  .b = 0x000000007FFFFFFF,
	  .result = 0x000000003FFF0002 },

	// -1 * -1 = 2
	{ .a = 0x00000000FFFFFFFF,
	  .b = 0x00000000FFFFFFFF,
	  .result = 0x0000000000000002 },
};


MMX_ARITH_TEST(test_mmx_pmullw, mmx_pmullw_test_data, mmx_i16_test_t, i16, 16, _m_pmullw);
MMX_ARITH_TEST(test_mmx_pmulhw, mmx_pmulhw_test_data, mmx_i16_test_t, i16, 16, _m_pmulhw);
MMX_64_TEST(test_mmx_pmaddwd, mmx_pmaddwd_test_data, _m_pmaddwd);





mmx_u64_test_t mmx_packssdw_test_data[] = {
	{ .a = 0x0000000200000001,
	  .b = 0x0000000400000003,
	  .result = 0x0004000300020001 },
	{ .a = 0x7FFFFFFF7FFFFFFF,
	  .b = 0x7FFFFFFF7FFFFFFF,
	  .result = 0x7FFF7FFF7FFF7FFF },
	{ .a = 0x8000000080000000,
	  .b = 0x8000000080000000,
	  .result = 0x8000800080008000 },
};
mmx_u64_test_t mmx_packsswb_test_data[] = {
	{ .a = 0x0004000300020001,
	  .b = 0x0008000700060005,
	  .result = 0x0807060504030201 },
	{ .a = 0x7FFF7FFF7FFF7FFF,
	  .b = 0x7FFF7FFF7FFF7FFF,
	  .result = 0x7F7F7F7F7F7F7F7F },
	{ .a = 0x8000800080008000,
	  .b = 0x8000800080008000,
	  .result = 0x8080808080808080 },
};
mmx_u64_test_t mmx_packuswb_test_data[] = {
	{ .a = 0x0004000300020001,
	  .b = 0x0008000700060005,
	  .result = 0x0807060504030201 },
	{ .a = 0x7FFF7FFF7FFF7FFF,
	  .b = 0x7FFF7FFF7FFF7FFF,
	  .result = 0xFFFFFFFFFFFFFFFF },
	{ .a = 0x8000800080008000,
	  .b = 0x8000800080008000,
	  .result = 0x0000000000000000 },
};


MMX_64_TEST(test_mmx_packssdw, mmx_packssdw_test_data, _m_packssdw);
MMX_64_TEST(test_mmx_packsswb, mmx_packsswb_test_data, _m_packsswb);
MMX_64_TEST(test_mmx_packuswb, mmx_packuswb_test_data, _m_packuswb);



mmx_u64_test_t mmx_punpckhbw_test_data[] = {
	{ .a = 0x4433221100000000,
	  .b = 0x8877665500000000,
	  .result = 0x8844773366225511 },
};
mmx_u64_test_t mmx_punpckhdq_test_data[] = {
	{ .a = 0xAAAAAAAA00000000,
	  .b = 0xBBBBBBBB00000000,
	  .result = 0xBBBBBBBBAAAAAAAA },
};
mmx_u64_test_t mmx_punpckhwd_test_data[] = {
	{ .a = 0xBBBBAAAA00000000,
	  .b = 0xDDDDCCCC00000000,
	  .result = 0xDDDDBBBBCCCCAAAA },
};
mmx_u64_test_t mmx_punpcklbw_test_data[] = {
	{ .a = 0x0000000044332211,
	  .b = 0x0000000088776655,
	  .result = 0x8844773366225511 },
};
mmx_u64_test_t mmx_punpckldq_test_data[] = {
	{ .a = 0x00000000AAAAAAAA,
	  .b = 0x00000000BBBBBBBB,
	  .result = 0xBBBBBBBBAAAAAAAA },
};
mmx_u64_test_t mmx_punpcklwd_test_data[] = {
	{ .a = 0x00000000BBBBAAAA,
	  .b = 0x00000000DDDDCCCC,
	  .result = 0xDDDDBBBBCCCCAAAA },
};


MMX_64_TEST(test_mmx_punpckhbw, mmx_punpckhbw_test_data, _m_punpckhbw);
MMX_64_TEST(test_mmx_punpckhdq, mmx_punpckhdq_test_data, _m_punpckhdq);
MMX_64_TEST(test_mmx_punpckhwd, mmx_punpckhwd_test_data, _m_punpckhwd);
MMX_64_TEST(test_mmx_punpcklbw, mmx_punpcklbw_test_data, _m_punpcklbw);
MMX_64_TEST(test_mmx_punpckldq, mmx_punpckldq_test_data, _m_punpckldq);
MMX_64_TEST(test_mmx_punpcklwd, mmx_punpcklwd_test_data, _m_punpcklwd);






mmx_u64_test_t mmx_pcmpeqb_test_data[] = {
	{ .a = 0x8877665544332211,
	  .b = 0x0077005500330011,
	  .result = 0x00FF00FF00FF00FF },
};
mmx_u64_test_t mmx_pcmpeqw_test_data[] = {
	{ .a = 0x4444333322221111,
	  .b = 0x0000333300001111,
	  .result = 0x0000FFFF0000FFFF },
};
mmx_u64_test_t mmx_pcmpeqd_test_data[] = {
	{ .a = 0x2222222211111111,
	  .b = 0x2222222200000000,
	  .result = 0xFFFFFFFF00000000 },
};

mmx_u64_test_t mmx_pcmpgtb_test_data[] = {
	{ .a = 0x0000000000002201,
	  .b = 0x0000000000002300,
	  .result = 0x00000000000000FF },
};
mmx_u64_test_t mmx_pcmpgtw_test_data[] = {
	{ .a = 0x4444333322221111,
	  .b = 0x0000333300001112,
	  .result = 0xFFFF0000FFFF0000 },
};
mmx_u64_test_t mmx_pcmpgtd_test_data[] = {
	{ .a = 0x2222222111111111,
	  .b = 0x2222222200000000,
	  .result = 0x00000000FFFFFFFF },
};


MMX_64_TEST(test_mmx_pcmpeqb, mmx_pcmpeqb_test_data, _m_pcmpeqb);
MMX_64_TEST(test_mmx_pcmpeqw, mmx_pcmpeqw_test_data, _m_pcmpeqw);
MMX_64_TEST(test_mmx_pcmpeqd, mmx_pcmpeqd_test_data, _m_pcmpeqd);
MMX_64_TEST(test_mmx_pcmpgtb, mmx_pcmpgtb_test_data, _m_pcmpgtb);
MMX_64_TEST(test_mmx_pcmpgtw, mmx_pcmpgtw_test_data, _m_pcmpgtw);
MMX_64_TEST(test_mmx_pcmpgtd, mmx_pcmpgtd_test_data, _m_pcmpgtd);




mmx_u64_test_t mmx_pslld_test_data[] = {
	{ .a = 1, .b = 1, .result = 2 },
	{ .a = 16, .b = 1, .result = 32 },
	{ .a = 16, .b = 32, .result = 0 },
	{ .a = 16, .b = 0, .result = 16 },
};
mmx_u64_test_t mmx_psllq_test_data[] = {
	{ .a = 1, .b = 1, .result = 2 },
	{ .a = 16, .b = 1, .result = 32 },
	{ .a = 16, .b = 64, .result = 0 },
	{ .a = 16, .b = 0, .result = 16 },
};
mmx_u64_test_t mmx_psllw_test_data[] = {
	{ .a = 1, .b = 1, .result = 2 },
	{ .a = 16, .b = 1, .result = 32 },
	{ .a = 16, .b = 16, .result = 0 },
	{ .a = 16, .b = 0, .result = 16 },
};
mmx_u64_test_t mmx_psrad_test_data[] = {
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = 16, .b = 1, .result = 8 },
	{ .a = 16, .b = 0, .result = 16 },
	{ .a = 0x7FFFFFFF, .b = 1, .result = 0x3FFFFFFF },

	{ .a = I32_MAX, .b = 32, .result = 0 },
	{ .a = I32_MIN, .b = 32, .result = U32_MAX },
};
mmx_u64_test_t mmx_psraw_test_data[] = {
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = 16, .b = 1, .result = 8 },
	{ .a = 16, .b = 0, .result = 16 },
	{ .a = 0x7FFF, .b = 1, .result = 0x3FFF },

	{ .a = I16_MAX, .b = 16, .result = 0 },
	{ .a = U16_MAX, .b = 16, .result = U16_MAX },
};
mmx_u64_test_t mmx_psrld_test_data[] = {
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = 16, .b = 1, .result = 8 },
	{ .a = 16, .b = 0, .result = 16 },
	{ .a = 0x7FFFFFFF, .b = 1, .result = 0x3FFFFFFF },

	{ .a = I32_MAX, .b = 32, .result = 0 },
	{ .a = I32_MIN, .b = 32, .result = 0 },
};
mmx_u64_test_t mmx_psrlq_test_data[] = {
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = 16, .b = 1, .result = 8 },
	{ .a = 16, .b = 0, .result = 16 },

	{ .a = I64_MAX, .b = 64, .result = 0 },
	{ .a = I64_MIN, .b = 64, .result = 0 },
};
mmx_u64_test_t mmx_psrlw_test_data[] = {
	{ .a = 1, .b = 1, .result = 0 },
	{ .a = 16, .b = 1, .result = 8 },
	{ .a = 16, .b = 0, .result = 16 },

	{ .a = I16_MAX, .b = 16, .result = 0 },

	// TODO: Works on my machine
	// { .a = I16_MIN, .b = 16, .result = 0 },
};



MMX_64_TEST(test_mmx_pslld, mmx_pslld_test_data, _m_pslld);
MMX_64_TEST(test_mmx_psllq, mmx_psllq_test_data, _m_psllq);
MMX_64_TEST(test_mmx_psllw, mmx_psllw_test_data, _m_psllw);
MMX_64_TEST(test_mmx_psrad, mmx_psrad_test_data, _m_psrad);
MMX_64_TEST(test_mmx_psraw, mmx_psraw_test_data, _m_psraw);
MMX_64_TEST(test_mmx_psrld, mmx_psrld_test_data, _m_psrld);
MMX_64_TEST(test_mmx_psrlq, mmx_psrlq_test_data, _m_psrlq);
MMX_64_TEST(test_mmx_psrlw, mmx_psrlw_test_data, _m_psrlw);

MMX_SHIFT_TEST(test_mmx_pslldi, mmx_pslld_test_data, _m_pslldi);
MMX_SHIFT_TEST(test_mmx_psllqi, mmx_psllq_test_data, _m_psllqi);
MMX_SHIFT_TEST(test_mmx_psllwi, mmx_psllw_test_data, _m_psllwi);
MMX_SHIFT_TEST(test_mmx_psradi, mmx_psrad_test_data, _m_psradi);
MMX_SHIFT_TEST(test_mmx_psrawi, mmx_psraw_test_data, _m_psrawi);
MMX_SHIFT_TEST(test_mmx_psrldi, mmx_psrld_test_data, _m_psrldi);
MMX_SHIFT_TEST(test_mmx_psrlqi, mmx_psrlq_test_data, _m_psrlqi);
MMX_SHIFT_TEST(test_mmx_psrlwi, mmx_psrlw_test_data, _m_psrlwi);




bool test_mmx_cpuid() {
	printf("TEST: test_mmx_cpuid\n");

	unsigned int eax, ebx, ecx, edx;
	asm volatile(
		"cpuid"
		: "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
		: "a" (1), "c" (0)
	);

	int has_mmx = !!(edx & (1 << 23));
	if (has_mmx) {
		return 0;
	} 
		return 1;
	
}

int main() {
	int errors = 0;

	errors += (int) test_mmx_cpuid();


	errors += (int) test_mmx_paddb();
	errors += (int) test_mmx_paddsb();
	errors += (int) test_mmx_paddusb();
	errors += (int) test_mmx_paddw();
	errors += (int) test_mmx_paddsw();
	errors += (int) test_mmx_paddusw();
	errors += (int) test_mmx_paddd();

	errors += (int) test_mmx_psubb();
	errors += (int) test_mmx_psubsb();
	errors += (int) test_mmx_psubusb();
	errors += (int) test_mmx_psubw();
	errors += (int) test_mmx_psubuw();
	errors += (int) test_mmx_psubusw();
	errors += (int) test_mmx_psubd();

	errors += (int) test_mmx_por();
	errors += (int) test_mmx_pand();
	errors += (int) test_mmx_pandn();
	errors += (int) test_mmx_pxor();

	errors += (int) test_mmx_pmullw();
	errors += (int) test_mmx_pmulhw();
	errors += (int) test_mmx_pmaddwd();

	errors += (int) test_mmx_packssdw();
	errors += (int) test_mmx_packsswb();
	errors += (int) test_mmx_packuswb();

	errors += (int) test_mmx_punpckhbw();
	errors += (int) test_mmx_punpckhdq();
	errors += (int) test_mmx_punpckhwd();
	errors += (int) test_mmx_punpcklbw();
	errors += (int) test_mmx_punpckldq();
	errors += (int) test_mmx_punpcklwd();

	errors += (int) test_mmx_pcmpeqb();
	errors += (int) test_mmx_pcmpeqw();
	errors += (int) test_mmx_pcmpeqd();
	errors += (int) test_mmx_pcmpgtb();
	errors += (int) test_mmx_pcmpgtw();
	errors += (int) test_mmx_pcmpgtd();

	errors += (int) test_mmx_psllw();
	errors += (int) test_mmx_psllwi();
	errors += (int) test_mmx_pslld();
	errors += (int) test_mmx_pslldi();
	errors += (int) test_mmx_psllq();
	errors += (int) test_mmx_psllqi();
	errors += (int) test_mmx_psraw();
	errors += (int) test_mmx_psrawi();
	errors += (int) test_mmx_psrad();
	errors += (int) test_mmx_psradi();
	errors += (int) test_mmx_psrld();
	errors += (int) test_mmx_psrldi();
	errors += (int) test_mmx_psrlq();
	errors += (int) test_mmx_psrlqi();
	errors += (int) test_mmx_psrlw();
	errors += (int) test_mmx_psrlwi();


	printf("Errors: %d\n", errors);
	return errors;
}

