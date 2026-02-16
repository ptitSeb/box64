// Manually created for now
#ifndef __CONVERTER_H_
#define __CONVERTER_H_

#include "box32.h"

typedef struct struct_p_s {
	void *p0;
} struct_p_t;
void from_struct_p(struct_p_t *dest, ptr_t src);
void to_struct_p(ptr_t dest, const struct_p_t *src);
typedef struct struct_l_s {
	long L0;
} struct_l_t;
void from_struct_l(struct_l_t *dest, ptr_t src);
void to_struct_l(ptr_t dest, const struct_l_t *src);
typedef struct struct_L_s {
	unsigned long L0;
} struct_L_t;
void from_struct_L(struct_L_t *dest, ptr_t src);
void to_struct_L(ptr_t dest, const struct_L_t *src);
typedef struct struct_ll_s {
	long L0;
	long L1;
} struct_ll_t;
void from_struct_ll(struct_ll_t *dest, ptr_t src);
void to_struct_ll(ptr_t dest, const struct_ll_t *src);
typedef struct struct_LL_s {
	unsigned long L0;
	unsigned long L1;
} struct_LL_t;
void from_struct_LL(struct_LL_t *dest, ptr_t src);
void to_struct_LL(ptr_t dest, const struct_LL_t *src);
typedef struct struct_LC_s {
	unsigned long L0;
	uint8_t C1;
} struct_LC_t;
void from_struct_LC(struct_LC_t *dest, ptr_t src);
void to_struct_LC(ptr_t dest, const struct_LC_t *src);
typedef struct struct_liu_s {
	long l0;
	int i1;
	uint32_t u2;
} struct_liu_t;
void from_struct_liu(struct_liu_t *dest, ptr_t src);
void to_struct_liu(ptr_t dest, const struct_liu_t *src);
typedef struct struct_llll_s {
	long l0;
	long l1;
	long l2;
	long l3;
} struct_llll_t;
void from_struct_llll(struct_llll_t *dest, ptr_t src);
void to_struct_llll(ptr_t dest, const struct_llll_t *src);
typedef struct struct_LLLL_s {
	unsigned long L0;
	unsigned long L1;
	unsigned long L2;
	unsigned long L3;
} struct_LLLL_t;
void from_struct_LLLL(struct_LLLL_t *dest, ptr_t src);
void to_struct_LLLL(ptr_t dest, const struct_LLLL_t *src);
typedef struct struct_LLLLLLLLLL_s {
	unsigned long L0;
	unsigned long L1;
	unsigned long L2;
	unsigned long L3;
	unsigned long L4;
	unsigned long L5;
	unsigned long L6;
	unsigned long L7;
	unsigned long L8;
	unsigned long L9;
} struct_LLLLLLLLLL_t;
void from_struct_LLLLLLLLLL(struct_LLLLLLLLLL_t *dest, ptr_t src);
void to_struct_LLLLLLLLLL(ptr_t dest, const struct_LLLLLLLLLL_t *src);
typedef struct struct_LLLLLLLLLLLLLLLLLL_s {
	unsigned long L0;
	unsigned long L1;
	unsigned long L2;
	unsigned long L3;
	unsigned long L4;
	unsigned long L5;
	unsigned long L6;
	unsigned long L7;
	unsigned long L8;
	unsigned long L9;
	unsigned long L10;
	unsigned long L11;
	unsigned long L12;
	unsigned long L13;
	unsigned long L14;
	unsigned long L15;
	unsigned long L16;
	unsigned long L17;
} struct_LLLLLLLLLLLLLLLLLL_t;
void from_struct_LLLLLLLLLLLLLLLLLL(struct_LLLLLLLLLLLLLLLLLL_t *dest, ptr_t src);
void to_struct_LLLLLLLLLLLLLLLLLL(ptr_t dest, const struct_LLLLLLLLLLLLLLLLLL_t *src);
typedef struct struct_LpCC_s {
	unsigned long L0;
	void* p1;
	uint8_t C2;
	uint8_t C3;
} struct_LpCC_t;
void from_struct_LpCC(struct_LpCC_t *dest, ptr_t src);
void to_struct_LpCC(ptr_t dest, const struct_LpCC_t *src);
typedef struct struct_h_s {
	uintptr_t h0;
} struct_h_t;
void from_struct_h(struct_h_t *dest, ptr_t src);
void to_struct_h(ptr_t dest, const struct_h_t *src);
typedef struct struct_H_s {
	uintptr_t H0;
} struct_H_t;
void from_struct_H(struct_H_t *dest, ptr_t src);
void to_struct_H(ptr_t dest, const struct_H_t *src);
typedef struct struct_pL_s {
	void* p0;
	unsigned long L1;
} struct_pL_t;
void from_struct_pL(struct_pL_t *dest, ptr_t src);
void to_struct_pL(ptr_t dest, const struct_pL_t *src);
typedef struct struct_ppppii_s {
	void* p0;
	void* p1;
	void* p2;
	void* p3;
	int i4;
	int i5;
} struct_ppppii_t;
void from_struct_ppppii(struct_ppppii_t *dest, ptr_t src);
void to_struct_ppppii(ptr_t dest, const struct_ppppii_t *src);
typedef struct struct_ppppp_s {
	void* p0;
	void* p1;
	void* p2;
	void* p3;
	void* p4;
} struct_ppppp_t;
void from_struct_ppppp(struct_ppppp_t *dest, ptr_t src);
void to_struct_ppppp(ptr_t dest, const struct_ppppp_t *src);
typedef struct struct_iiiiiiiiilt_s {
	int i0;
	int i1;
	int i2;
	int i3;
	int i4;
	int i5;
	int i6;
	int i7;
	int i8;
	long l9;
	void* p10;
} struct_iiiiiiiiilt_t;
void from_struct_iiiiiiiiilt(struct_iiiiiiiiilt_t* dest, ptr_t src);
void to_struct_iiiiiiiiilt(ptr_t dest, const struct_iiiiiiiiilt_t* src);
typedef struct struct_up_s {
	uint32_t u0;
	void* p1;
} struct_up_t;
void from_struct_up(struct_up_t *dest, ptr_t src);
void to_struct_up(ptr_t dest, const struct_up_t *src);
typedef struct struct_upi_s {
	uint32_t u0;
	void* p1;
	int i2;
} struct_upi_t;
void from_struct_upi(struct_upi_t *dest, ptr_t src);
void to_struct_upi(ptr_t dest, const struct_upi_t *src);
typedef struct struct_Ldd_s {
	unsigned long L0;
	double d1;
	double d2;
} struct_Ldd_t;
void from_struct_Ldd(struct_Ldd_t *dest, ptr_t src);
void to_struct_Ldd(ptr_t dest, const struct_Ldd_t *src);
typedef struct struct_upu_s {
	uint32_t u0;
	void* p1;
	uint32_t u2;
} struct_upu_t;
void from_struct_upu(struct_upu_t *dest, ptr_t src);
void to_struct_upu(ptr_t dest, const struct_upu_t *src);
typedef struct struct_LWWWcc_s {
	unsigned long L0;
	unsigned short W1;
	unsigned short W2;
	unsigned short W3;
	char c4;
	char c5;
} struct_LWWWcc_t;
void from_struct_LWWWcc(struct_LWWWcc_t *dest, ptr_t src);
void to_struct_LWWWcc(ptr_t dest, const struct_LWWWcc_t *src);
typedef struct struct_pLiL_s {
	void* p0;
	unsigned long L1;
	int i2;
	unsigned long L3;
} struct_pLiL_t;
void from_struct_pLiL(struct_pLiL_t *dest, ptr_t src);
void to_struct_pLiL(ptr_t dest, const struct_pLiL_t *src);
typedef struct struct_Lip_s {
	unsigned long L0;
	int i1;
	void* p2;
} struct_Lip_t;
void from_struct_Lip(struct_Lip_t *dest, ptr_t src);
void to_struct_Lip(ptr_t dest, const struct_Lip_t *src);
typedef struct struct_Lipi_s {
	unsigned long L0;
	int i1;
	void* p2;
	int i3;
} struct_Lipi_t;
void from_struct_Lipi(struct_Lipi_t *dest, ptr_t src);
void to_struct_Lipi(ptr_t dest, const struct_Lipi_t *src);
typedef struct struct_LLii_s {
	unsigned long L0;
	unsigned long L1;
	int i2;
	int i3;
} struct_LLii_t;
void from_struct_LLii(struct_LLii_t *dest, ptr_t src);
void to_struct_LLii(ptr_t dest, const struct_LLii_t *src);
typedef struct struct_uuipWCCp_s {
	uint32_t u0;
	uint32_t u1;
	int i2;
	void* p3;
	uint16_t u4;
	uint8_t u5;
	uint8_t u6;
	void* p7;
} struct_uuipWCCp_t;
void from_struct_uuipWCCp(struct_uuipWCCp_t *dest, ptr_t src);
void to_struct_uuipWCCp(ptr_t dest, const struct_uuipWCCp_t *src);
typedef struct struct_pLiLLLii_s {
	void* p0;
	unsigned long L1;
	int i2;
	unsigned long L3;
	unsigned long L4;
	unsigned long L5;
	int i6;
	int i7;
} struct_pLiLLLii_t;
void from_struct_pLiLLLii(struct_pLiLLLii_t *dest, ptr_t src);
void to_struct_pLiLLLii(ptr_t dest, const struct_pLiLLLii_t *src);
typedef struct struct_WWpWpWpWp_s {
	uint16_t W0;
	uint16_t W1;
	void* p2;
	uint16_t W3;
	void* p4;
	uint16_t W5;
	void* p6;
	uint16_t W7;
	void* p8;
} struct_WWpWpWpWp_t;
void from_struct_WWpWpWpWp(struct_WWpWpWpWp_t *dest, ptr_t src);
void to_struct_WWpWpWpWp(ptr_t dest, const struct_WWpWpWpWp_t *src);
typedef struct struct_ip_s {
	int i0;
	void* p1;
} struct_ip_t;
void from_struct_ip(struct_ip_t *dest, ptr_t src);
void to_struct_ip(ptr_t dest, const struct_ip_t *src);
typedef struct struct_pi_s {
	void* p0;
	int i1;
} struct_pi_t;
void from_struct_pi(struct_pi_t *dest, ptr_t src);
void to_struct_pi(ptr_t dest, const struct_pi_t *src);
typedef struct struct_pp_s {
	void* p0;
	void* p1;
} struct_pp_t;
void from_struct_pp(struct_pp_t *dest, ptr_t src);
void to_struct_pp(ptr_t dest, const struct_pp_t *src);
typedef struct struct_pu_s {
	void* p0;
	uint32_t u1;
} struct_pu_t;
void from_struct_pu(struct_pu_t *dest, ptr_t src);
void to_struct_pu(ptr_t dest, const struct_pu_t *src);
typedef struct struct_ppi_s {
	void* p0;
	void* p1;
	int i2;
} struct_ppi_t;
void from_struct_ppi(struct_ppi_t *dest, ptr_t src);
void to_struct_ppi(ptr_t dest, const struct_ppi_t *src);
typedef struct struct_iip_s {
	int i0;
	int i1;
	void* p2;
} struct_iip_t;
void from_struct_iip(struct_iip_t *dest, ptr_t src);
void to_struct_iip(ptr_t dest, const struct_iip_t *src);
typedef struct struct_puu_s {
	void* p0;
	uint32_t u1;
	uint32_t u2;
} struct_puu_t;
void from_struct_puu(struct_puu_t *dest, ptr_t src);
void to_struct_puu(ptr_t dest, const struct_puu_t *src);
typedef struct struct_pii_s {
	void* p0;
	int i1;
	int i2;
} struct_pii_t;
void from_struct_pii(struct_pii_t *dest, ptr_t src);
void to_struct_pii(ptr_t dest, const struct_pii_t *src);
typedef struct struct_piiL_s {
	void* p0;
	int i1;
	int i2;
	unsigned long L3;
} struct_piiL_t;
void from_struct_piiL(struct_piiL_t *dest, ptr_t src);
void to_struct_piiL(ptr_t dest, const struct_piiL_t *src);
typedef struct struct_piip_s {
	void* p0;
	int i1;
	int i2;
	void* p3;
} struct_piip_t;
void from_struct_piip(struct_piip_t *dest, ptr_t src);
void to_struct_piip(ptr_t dest, const struct_piip_t *src);
typedef struct struct_ppup_s {
	void* p0;
	void* p1;
	uint32_t u2;
	void* p3;
} struct_ppup_t;
void from_struct_ppup(struct_ppup_t *dest, ptr_t src);
void to_struct_ppup(ptr_t dest, const struct_ppup_t *src);
typedef struct struct_ipip_s {
	int i0;
	void* p1;
	int i2;
	void* p3;
} struct_ipip_t;
void from_struct_ipip(struct_ipip_t *dest, ptr_t src);
void to_struct_ipip(ptr_t dest, const struct_ipip_t *src);
typedef struct struct_iiiiiLi_s {
	int i0;
	int i1;
	int i2;
	int i3;
	int i4;
	unsigned long L5;
	int i6;
} struct_iiiiiLi_t;
void from_struct_iiiiiLi(struct_iiiiiLi_t* dest, ptr_t src);
void to_struct_iiiiiLi(ptr_t dest, const struct_iiiiiLi_t* src);
typedef struct struct_iLLLiiiiiiiLLiiLiiiiLic_s {
	int i0;
	unsigned long L1;
	unsigned long L2;
	unsigned long L3;
	int i4;
	int i5;
	int i6;
	int i7;
	int i8;
	int i9;
	int i10;
	unsigned long L11;
	unsigned long L12;
	int i13;
	int i14;
	unsigned long L15;
	int i16;
	int i17;
	int i18;
	int i19;
	unsigned long L20;
	int i21;
	int8_t c22;
} struct_iLLLiiiiiiiLLiiLiiiiLic_t;
void from_struct_iLLLiiiiiiiLLiiLiiiiLic(struct_iLLLiiiiiiiLLiiLiiiiLic_t* dest, ptr_t src);
void to_struct_iLLLiiiiiiiLLiiLiiiiLic(ptr_t dest, const struct_iLLLiiiiiiiLLiiLiiiiLic_t* src);
typedef struct struct_LLLLiiiLLilliLL_s {
	unsigned long L0;
	unsigned long L1;
	unsigned long L2;
	unsigned long L3;
	int i4;
	int i5;
	int i6;
	int i7;
	unsigned long L8;
	unsigned long L9;
	int i10;
	long l11;
	long l12;
	int i13;
	unsigned long L14;
	unsigned long L15;
} struct_LLLLiiiLLilliLL_t;
void from_struct_LLLLiiiLLilliLL(struct_LLLLiiiLLilliLL_t* dest, ptr_t src);
void to_struct_LLLLiiiLLilliLL(ptr_t dest, const struct_LLLLiiiLLilliLL_t* src);
typedef struct struct_iiuuLip_s {
	int i0;
	int i1;
	uint32_t u2;
	uint32_t u3;
	unsigned long L4;
	int i5;
	void* p6;
} struct_iiuuLip_t;
void from_struct_iiuuLip(struct_iiuuLip_t *dest, ptr_t src);
void to_struct_iiuuLip(ptr_t dest, const struct_iiuuLip_t *src);
typedef struct struct_pLiiiLLLii_s {
	void* p0;
	unsigned long L1;
	int i2;
	int i3;
	int i4;
	unsigned long L5;
	unsigned long L6;
	unsigned long L7;
	int i8;
	int i9;
} struct_pLiiiLLLii_t;
void from_struct_pLiiiLLLii(struct_pLiiiLLLii_t *dest, ptr_t src);
void to_struct_pLiiiLLLii(ptr_t dest, const struct_pLiiiLLLii_t *src);
typedef struct struct_LiiuL_s {
	unsigned long L0;
	int i1;
	int i2;
	uint32_t u3;
	unsigned long L4;
} struct_LiiuL_t;
void from_struct_LiiuL(struct_LiiuL_t *dest, ptr_t src);
void to_struct_LiiuL(ptr_t dest, const struct_LiiuL_t *src);
typedef struct struct_WWWWWWWWWuip_s {
	uint16_t W0;
	uint16_t W1;
	uint16_t W2;
	uint16_t W3;
	uint16_t W4;
	uint16_t W5;
	uint16_t W6;
	uint16_t W7;
	uint16_t W8;
	uint32_t u9;
	int i10;
	void* p11;
	unsigned long L4;
} struct_WWWWWWWWWuip_t;
void from_struct_WWWWWWWWWuip(struct_WWWWWWWWWuip_t *dest, ptr_t src);
void to_struct_WWWWWWWWWuip(ptr_t dest, const struct_WWWWWWWWWuip_t *src);
typedef struct struct_LiiwwwwwwwwL_s {
	unsigned long L0;
	int i1;
	int i2;
	short w3;
	short w4;
	short w5;
	short w6;
	short w7;
	short w8;
	short w9;
	short w10;
	unsigned long L11;
} struct_LiiwwwwwwwwL_t;
void from_struct_LiiwwwwwwwwL(struct_LiiwwwwwwwwL_t *dest, ptr_t src);
void to_struct_LiiwwwwwwwwL(ptr_t dest, const struct_LiiwwwwwwwwL_t *src);
typedef struct struct_iLiiiiLiiiiLi_s {
	int i0;
	unsigned long L1;
	int i2;
	int i3;
	int i4;
	int i5;
	unsigned long L6;
	int i7;
	int i8;
	int i9;
	int i10;
	unsigned long L11;
	int i12;
} struct_iLiiiiLiiiiLi_t;
void from_struct_iLiiiiLiiiiLi(struct_iLiiiiLiiiiLi_t* dest, ptr_t src);
void to_struct_iLiiiiLiiiiLi(ptr_t dest, const struct_iLiiiiLiiiiLi_t* src);
typedef struct struct_pppiiip_s {
	void* p0;
	void* p1;
	void* p2;
	int i3;
	int i4;
	int i5;
	void* p6;
} struct_pppiiip_t;
void from_struct_pppiiip(struct_pppiiip_t *dest, ptr_t src);
void to_struct_pppiiip(ptr_t dest, const struct_pppiiip_t *src);
typedef struct struct_LWww_s {
	unsigned long L0;
	unsigned short W1;
	short W2;
	short W3;
} struct_LWww_t;
void from_struct_LWww(struct_LWww_t *dest, ptr_t src);
void to_struct_LWww(ptr_t dest, const struct_LWww_t *src);
typedef struct struct_ppuuuuup_s {
	void* p0;
	void* p1;
	uint32_t u2;
	uint32_t u3;
	uint32_t u4;
	uint32_t u5;
	uint32_t u6;
	void* p7;
} struct_ppuuuuup_t;
void from_struct_ppuuuuup(struct_ppuuuuup_t *dest, ptr_t src);
void to_struct_ppuuuuup(ptr_t dest, const struct_ppuuuuup_t *src);
typedef struct struct_ppuii_s {
	void* p0;
	void* p1;
	uint32_t u2;
	int32_t i3;
	int32_t i4;
} struct_ppuii_t;
void from_struct_ppuii(struct_ppuii_t *dest, ptr_t src);
void to_struct_ppuii(ptr_t dest, const struct_ppuii_t *src);
void from_struct_ppuuuuup(struct_ppuuuuup_t *dest, ptr_t src);
void to_struct_ppuuuuup(ptr_t dest, const struct_ppuuuuup_t *src);
typedef struct struct_ppuiiiiiiiiipp_s {
	void* p0;
	void* p1;
	uint32_t u2;
	int32_t i3;
	int32_t i4;
	int32_t i5;
	int32_t i6;
	int32_t i7;
	int32_t i8;
	int32_t i9;
	int32_t i10;
	int32_t i11;
	void* p12;
	void* p13;
} struct_ppuiiiiiiiiipp_t;
void from_struct_ppuiiiiiiiiipp(struct_ppuiiiiiiiiipp_t *dest, ptr_t src);
void to_struct_ppuiiiiiiiiipp(ptr_t dest, const struct_ppuiiiiiiiiipp_t *src);
typedef struct struct_lii_s {
	long l0;
	int i1;
	int i2;
} struct_lii_t;
void from_struct_lii(struct_lii_t *dest, ptr_t src);
void to_struct_lii(ptr_t dest, const struct_lii_t *src);
typedef struct struct_UUUULLLLLLLLLLLLLL_s {
	uint64_t 	  U0;
	uint64_t 	  U1;
	uint64_t 	  U2;
	uint64_t 	  U3;
	unsigned long L4;
	unsigned long L5;
	unsigned long L6;
	unsigned long L7;
	unsigned long L8;
	unsigned long L9;
	unsigned long L10;
	unsigned long L11;
	unsigned long L12;
	unsigned long L13;
	unsigned long L14;
	unsigned long L15;
	unsigned long L16;
	unsigned long L17;
} struct_UUUULLLLLLLLLLLLLL_t;
void from_struct_UUUULLLLLLLLLLLLLL(struct_UUUULLLLLLLLLLLLLL_t *dest, ptr_t src);
void to_struct_UUUULLLLLLLLLLLLLL(ptr_t dest, const struct_UUUULLLLLLLLLLLLLL_t *src);
typedef struct struct_uup_s {
	uint32_t u0;
	uint32_t u1;
	void* p2;
} struct_uup_t;
void from_struct_uup(struct_uup_t *dest, ptr_t src);
void to_struct_uup(ptr_t dest, const struct_uup_t *src);
typedef struct struct_uUp_s {
	uint32_t u0;
	uint64_t U1;
	void* p2;
} struct_uUp_t;
void from_struct_uUp(struct_uUp_t *dest, ptr_t src);
void to_struct_uUp(ptr_t dest, const struct_uUp_t *src);
typedef struct struct_uupuu_s {
	uint32_t u0;
	uint32_t u1;
	void* p2;
	uint32_t u3;
	uint32_t u4;
} struct_uupuu_t;
void from_struct_uupuu(struct_uupuu_t *dest, ptr_t src);
void to_struct_uupuu(ptr_t dest, const struct_uupuu_t *src);
typedef struct struct_uUUUU_s {
	uint32_t u0;
	uint64_t U1;
	uint64_t U2;
	uint64_t U3;
	uint64_t U4;
} struct_uUUUU_t;
void from_struct_uUUUU(struct_uUUUU_t *dest, ptr_t src);
void to_struct_uUUUU(ptr_t dest, const struct_uUUUU_t *src);
typedef struct struct_uuupu_s {
	uint32_t u0;
	uint32_t u1;
	uint32_t u2;
	void* p3;
	uint32_t u4;
} struct_uuupu_t;
void from_struct_uuupu(struct_uuupu_t *dest, ptr_t src);
void to_struct_uuupu(ptr_t dest, const struct_uuupu_t *src);
typedef struct struct_pUUuu_s {
	void* p0;
	uint64_t U1;
	uint64_t U2;
	uint32_t u3;
	uint32_t u4;
} struct_pUUuu_t;
void from_struct_pUUuu(struct_pUUuu_t *dest, ptr_t src);
void to_struct_pUUuu(ptr_t dest, const struct_pUUuu_t *src);
typedef struct struct_uuppuu_s {
	uint32_t u0;
	uint32_t u1;
	void* p2;
	void* p3;
	uint32_t u4;
	uint32_t u5;
} struct_uuppuu_t;
void from_struct_uuppuu(struct_uuppuu_t *dest, ptr_t src);
void to_struct_uuppuu(ptr_t dest, const struct_uuppuu_t *src);
typedef struct struct_uuUp_s {
	uint32_t u0;
	uint32_t u1;
	uint64_t U2;
	void* p3;
} struct_uuUp_t;
void from_struct_uuUp(struct_uuUp_t *dest, ptr_t src);
void to_struct_uuUp(ptr_t dest, const struct_uuUp_t *src);
typedef struct struct_uUuuuu_s {
	uint32_t u0;
	uint64_t U1;
	uint32_t u2;
	uint32_t u3;
	uint32_t u4;
	uint32_t u5;
} struct_uUuuuu_t;
void from_struct_uUuuuu(struct_uUuuuu_t *dest, ptr_t src);
void to_struct_uUuuuu(ptr_t dest, const struct_uUuuuu_t *src);
typedef struct struct_uUuu_s {
	uint32_t u0;
	uint64_t U1;
	uint32_t u2;
	uint32_t u3;
} struct_uUuu_t;
void from_struct_uUuu(struct_uUuu_t *dest, ptr_t src);
void to_struct_uUuu(ptr_t dest, const struct_uUuu_t *src);
#endif // __CONVERTER_H_
