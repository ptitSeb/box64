// Manually created for now
#ifndef __CONVERTER_H_
#define __CONVERTER_H_

#include "box32.h"

typedef struct struct_p_s {
	void *p0;
} struct_p_t;
void from_struct_p(struct_p_t *dest, ptr_t src);
void to_struct_p(ptr_t dest, const struct_p_t *src);
typedef struct struct_L_s {
	unsigned long L0;
} struct_L_t;
void from_struct_L(struct_L_t *dest, ptr_t src);
void to_struct_L(ptr_t dest, const struct_L_t *src);
typedef struct struct_LL_s {
	unsigned long L0;
	unsigned long L1;
} struct_LL_t;
void from_struct_LL(struct_LL_t *dest, ptr_t src);
void to_struct_LL(ptr_t dest, const struct_LL_t *src);
typedef struct struct_LLLL_s {
	unsigned long L0;
	unsigned long L1;
	unsigned long L2;
	unsigned long L3;
} struct_LLLL_t;
void from_struct_LLLL(struct_LLLL_t *dest, ptr_t src);
void to_struct_LLLL(ptr_t dest, const struct_LLLL_t *src);
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

#endif // __CONVERTER_H_
