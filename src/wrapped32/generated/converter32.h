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
typedef struct struct_hWcc_s {
	unsigned long L0;
	unsigned short W1;
	char c2;
	char c3;
} struct_hWcc_t;
void from_struct_hWcc(struct_hWcc_t *dest, ptr_t src);
void to_struct_hWcc(ptr_t dest, const struct_hWcc_t *src);

#endif // __CONVERTER_H_
