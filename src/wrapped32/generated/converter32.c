// Manually created for now
#include "converter32.h"

void from_struct_p(struct_p_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_p(ptr_t d, const struct_p_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
}

void from_struct_l(struct_l_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_long(*(long_t*)src); src += 4;
}
void to_struct_l(ptr_t d, const struct_l_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(long_t*)dest = to_long(src->L0); dest += 4;
}

void from_struct_L(struct_L_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_L(ptr_t d, const struct_L_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
}

void from_struct_ll(struct_ll_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_long(*(long_t*)src); src += 4;
	dest->L1 = from_long(*(long_t*)src); src += 4;
}
void to_struct_ll(ptr_t d, const struct_ll_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(long_t*)dest = to_long(src->L0); dest += 4;
	*(long_t*)dest = to_long(src->L1); dest += 4;
}

void from_struct_LL(struct_LL_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_LL(ptr_t d, const struct_LL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
}
void from_struct_LC(struct_LC_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->C1 = *(uint8_t*)src; src += 1;
}
void to_struct_LC(ptr_t d, const struct_LC_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(uint8_t*)dest = to_ulong(src->C1); dest += 1;
}

void from_struct_liu(struct_liu_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->l0 = from_long(*(long_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
}
void to_struct_liu(ptr_t d, const struct_liu_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(long_t*)dest = to_long(src->l0); dest += 4;
	*(int*)dest = src->i1; dest += 2;
	*(uint32_t*)dest = src->u2; dest += 2;
}

void from_struct_llll(struct_llll_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->l0 = from_long(*(ulong_t*)src); src += 4;
	dest->l1 = from_long(*(ulong_t*)src); src += 4;
	dest->l2 = from_long(*(ulong_t*)src); src += 4;
	dest->l3 = from_long(*(ulong_t*)src); src += 4;
}
void to_struct_llll(ptr_t d, const struct_llll_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_long(src->l0); dest += 4;
	*(ulong_t*)dest = to_long(src->l1); dest += 4;
	*(ulong_t*)dest = to_long(src->l2); dest += 4;
	*(ulong_t*)dest = to_long(src->l3); dest += 4;
}

void from_struct_LLLL(struct_LLLL_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L2 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L3 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_LLLL(ptr_t d, const struct_LLLL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L2); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
}

void from_struct_LLLLLLLLLL(struct_LLLLLLLLLL_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L2 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L3 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L4 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L5 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L6 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L7 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L8 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L9 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_LLLLLLLLLL(ptr_t d, const struct_LLLLLLLLLL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L2); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L4); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L5); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L6); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L7); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L8); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L9); dest += 4;
}

void from_struct_LLLLLLLLLLLLLLLLLL(struct_LLLLLLLLLLLLLLLLLL_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L2 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L3 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L4 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L5 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L6 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L7 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L8 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L9 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L10 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L11 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L12 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L13 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L14 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L15 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L16 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L17 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_LLLLLLLLLLLLLLLLLL(ptr_t d, const struct_LLLLLLLLLLLLLLLLLL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L2); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L4); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L5); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L6); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L7); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L8); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L9); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L10); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L11); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L12); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L13); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L14); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L15); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L16); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L17); dest += 4;
}

void from_struct_LpCC(struct_LpCC_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->C2 = *(uint8_t*)src; src += 1;
	dest->C3 = *(uint8_t*)src; src += 1;
}
void to_struct_LpCC(ptr_t d, const struct_LpCC_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(uint8_t*)dest = src->C2; dest += 1;
	*(uint8_t*)dest = src->C3; dest += 1;
}

void from_struct_h(struct_h_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->h0 = from_hash(*(ulong_t*)src); src += 4;
}
void to_struct_h(ptr_t d, const struct_h_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_hash(src->h0); dest += 4;
}

void from_struct_H(struct_H_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->H0 = from_hash_d(*(ulong_t*)src); src += 4;
}
void to_struct_H(ptr_t d, const struct_H_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_hash_d(src->H0); dest += 4;
}

void from_struct_pL(struct_pL_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->L1 = from_ulong(*(long_t*)src); src += 4;
}
void to_struct_pL(ptr_t d, const struct_pL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
}

void from_struct_ppppii(struct_ppppii_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->i5 = *(int*)src; src += 4;
}
void to_struct_ppppii(ptr_t d, const struct_ppppii_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p3); dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(int*)dest = src->i5; dest += 4;
}

void from_struct_ppppp(struct_ppppp_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p4 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_ppppp(ptr_t d, const struct_ppppp_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p3); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p4); dest += 4;
}

void from_struct_iiiiiiiiilt(struct_iiiiiiiiilt_t* dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->i3 = *(int*)src; src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->i5 = *(int*)src; src += 4;
	dest->i6 = *(int*)src; src += 4;
	dest->i7 = *(int*)src; src += 4;
	dest->i8 = *(int*)src; src += 4;
	dest->l9 = from_long(*(long_t*)src); src += 4;
	dest->p10 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_iiiiiiiiilt(ptr_t d, const struct_iiiiiiiiilt_t* src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(int*)dest = src->i3; dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(int*)dest = src->i5; dest += 4;
	*(int*)dest = src->i6; dest += 4;
	*(int*)dest = src->i7; dest += 4;
	*(int*)dest = src->i8; dest += 4;
	*(long_t*)dest = to_long(src->l9); dest += 4;
	*(ptr_t*)dest = to_cstring(src->p10); dest += 4;
}
void from_struct_up(struct_up_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->u0 = *(uint32_t*)src; src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_up(ptr_t d, const struct_up_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = src->u0; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
}
void from_struct_upi(struct_upi_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->u0 = *(uint32_t*)src; src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
}
void to_struct_upi(ptr_t d, const struct_upi_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = src->u0; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
}
void from_struct_Ldd(struct_Ldd_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->d1 = *(double*)src; src += 8;
	dest->d2 = *(double*)src; src += 8;
}
void to_struct_Ldd(ptr_t d, const struct_Ldd_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(double*)dest = src->d1; dest += 8;
	*(double*)dest = src->d2; dest += 8;
}

void from_struct_upu(struct_upu_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->u0 = *(uint32_t*)src; src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
}
void to_struct_upu(ptr_t d, const struct_upu_t *src) {	
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = src->u0; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
}

void from_struct_LWWWcc(struct_LWWWcc_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(uint32_t*)src); src += 4;
	dest->W1 = *(uint16_t*)src; src += 2;
	dest->W2 = *(uint16_t*)src; src += 2;
	dest->W3 = *(uint16_t*)src; src += 2;
	dest->c4 = *(char*)src; src += 1;
	dest->c5 = *(char*)src; src += 1;
}
void to_struct_LWWWcc(ptr_t d, const struct_LWWWcc_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = to_ulong(src->L0); dest += 4;
	*(uint16_t*)dest = src->W1; dest += 2;
	*(uint16_t*)dest = src->W2; dest += 2;
	*(uint16_t*)dest = src->W3; dest += 2;
	*(char*)dest = src->c4; dest += 1;
	*(char*)dest = src->c5; dest += 1;
}
void from_struct_pLiL(struct_pLiL_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->L3 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_pLiL(ptr_t d, const struct_pLiL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
}
void from_struct_Lip(struct_Lip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_Lip(ptr_t d, const struct_Lip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
}
void from_struct_Lipi(struct_Lipi_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i3 = *(int*)src; src += 4;
}
void to_struct_Lipi(ptr_t d, const struct_Lipi_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
	*(int*)dest = src->i3; dest += 4;
}
void from_struct_LLii(struct_LLii_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->i3 = *(int*)src; src += 4;
}
void to_struct_LLii(ptr_t d, const struct_LLii_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(int*)dest = src->i3; dest += 4;
}
void from_struct_uuipWCCp(struct_uuipWCCp_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->u0 = *(uint32_t*)src; src += 4;
	dest->u1 = *(uint32_t*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u4 = *(uint16_t*)src; src += 2;
	dest->u5 = *(uint8_t*)src; src += 1;
	dest->u6 = *(uint8_t*)src; src += 1;
	dest->p7 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_uuipWCCp(ptr_t d, const struct_uuipWCCp_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = src->u0; dest += 4;
	*(uint32_t*)dest = src->u1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p3); dest += 4;
	*(uint16_t*)dest = src->u4; dest += 2;
	*(uint8_t*)dest = src->u5; dest += 1;
	*(uint8_t*)dest = src->u6; dest += 1;
	*(ptr_t*)dest = to_ptrv(src->p7); dest += 4;
}

void from_struct_pLiLLLii(struct_pLiLLLii_t *dest, ptr_t s)
{
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->L1 = from_ulong(*(ulong_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->L3 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L4 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L5 = from_ulong(*(ulong_t*)src); src += 4;
	dest->i6 = *(int*)src; src += 4;
	dest->i7 = *(int*)src; src += 4;
}
void to_struct_pLiLLLii(ptr_t d, const struct_pLiLLLii_t *src)
{
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L4); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L5); dest += 4;
	*(int*)dest = src->i6; dest += 4;
	*(int*)dest = src->i7; dest += 4;
}

void from_struct_WWpWpWpWp(struct_WWpWpWpWp_t *dest, ptr_t s)
{
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->W0 = *(uint16_t*)src; src += 2;
	dest->W1 = *(uint16_t*)src; src += 2;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->W3 = *(uint16_t*)src; src += 4;	// align
	dest->p4 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->W5 = *(uint16_t*)src; src += 4;	// align
	dest->p6 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->W7 = *(uint16_t*)src; src += 4;	// align
	dest->p8 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_WWpWpWpWp(ptr_t d, const struct_WWpWpWpWp_t *src)
{
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint16_t*)dest = src->W0; dest += 2;
	*(uint16_t*)dest = src->W1; dest += 2;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
	*(uint16_t*)dest = src->W3; dest += 4;	// align
	*(ptr_t*)dest = to_ptrv(src->p4); dest += 4;
	*(uint16_t*)dest = src->W5; dest += 4;	// align
	*(ptr_t*)dest = to_ptrv(src->p6); dest += 4;
	*(uint16_t*)dest = src->W7; dest += 4;	// align
	*(ptr_t*)dest = to_ptrv(src->p8); dest += 4;
}

void from_struct_pi(struct_pi_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
}
void to_struct_pi(ptr_t d, const struct_pi_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
}

void from_struct_pp(struct_pp_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_pp(ptr_t d, const struct_pp_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
}

void from_struct_pu(struct_pu_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u1 = *(uint32_t*)src; src += 4;
}
void to_struct_pu(ptr_t d, const struct_pu_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(uint32_t*)dest = src->u1; dest += 4;
}

void from_struct_ppi(struct_ppi_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
}
void to_struct_ppi(ptr_t d, const struct_ppi_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
}

void from_struct_ip(struct_ip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_ip(ptr_t d, const struct_ip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
}

void from_struct_iip(struct_iip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_iip(ptr_t d, const struct_iip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
}

void from_struct_puu(struct_puu_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u1 = *(uint32_t*)src; src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
}
void to_struct_puu(ptr_t d, const struct_puu_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(uint32_t*)dest = src->u1; dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
}

void from_struct_pii(struct_pii_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
}
void to_struct_pii(ptr_t d, const struct_pii_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
}

void from_struct_piiL(struct_piiL_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->L3 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_piiL(ptr_t d, const struct_piiL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
}

void from_struct_ipip(struct_ipip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_ipip(ptr_t d, const struct_ipip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p3); dest += 4;
}

void from_struct_piip(struct_piip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_piip(ptr_t d, const struct_piip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p3); dest += 4;
}

void from_struct_ppup(struct_ppup_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
	dest->p3 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_ppup(ptr_t d, const struct_ppup_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p3); dest += 4;
}
void from_struct_iiiiiLi(struct_iiiiiLi_t* dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->i3 = *(int*)src; src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->L5 = from_ulong(*(long_t*)src); src += 4;
	dest->i6 = *(int*)src; src += 4;
}
void to_struct_iiiiiLi(ptr_t d, const struct_iiiiiLi_t* src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(int*)dest = src->i3; dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L5); dest += 4;
	*(int*)dest = src->i6; dest += 4;
}
void from_struct_iLLLiiiiiiiLLiiLiiiiLic(struct_iLLLiiiiiiiLLiiLiiiiLic_t* dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->L1 = from_ulong(*(long_t*)src); src += 4;
	dest->L2 = from_ulong(*(long_t*)src); src += 4;
	dest->L3 = from_ulong(*(long_t*)src); src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->i5 = *(int*)src; src += 4;
	dest->i6 = *(int*)src; src += 4;
	dest->i7 = *(int*)src; src += 4;
	dest->i8 = *(int*)src; src += 4;
	dest->i9 = *(int*)src; src += 4;
	dest->i10 = *(int*)src; src += 4;
	dest->L11 = from_ulong(*(long_t*)src); src += 4;
	dest->L12 = from_ulong(*(long_t*)src); src += 4;
	dest->i13 = *(int*)src; src += 4;
	dest->i14 = *(int*)src; src += 4;
	dest->L15 = from_ulong(*(long_t*)src); src += 4;
	dest->i16 = *(int*)src; src += 4;
	dest->i17 = *(int*)src; src += 4;
	dest->i18 = *(int*)src; src += 4;
	dest->i19 = *(int*)src; src += 4;
	dest->L20 = from_ulong(*(long_t*)src); src += 4;
	dest->i21 = *(int*)src; src += 4;
	dest->c22 = *(int8_t*)src; src += 1;
}
void to_struct_iLLLiiiiiiiLLiiLiiiiLic(ptr_t d, const struct_iLLLiiiiiiiLLiiLiiiiLic_t* src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L2); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(int*)dest = src->i5; dest += 4;
	*(int*)dest = src->i6; dest += 4;
	*(int*)dest = src->i7; dest += 4;
	*(int*)dest = src->i8; dest += 4;
	*(int*)dest = src->i9; dest += 4;
	*(int*)dest = src->i10; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L11); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L12); dest += 4;
	*(int*)dest = src->i13; dest += 4;
	*(int*)dest = src->i14; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L15); dest += 4;
	*(int*)dest = src->i16; dest += 4;
	*(int*)dest = src->i17; dest += 4;
	*(int*)dest = src->i18; dest += 4;
	*(int*)dest = src->i19; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L20); dest += 4;
	*(int*)dest = src->i21; dest += 4;
	*(int8_t*)dest = src->c22; dest += 1;
}
void from_struct_LLLLiiiLLilliLL(struct_LLLLiiiLLilliLL_t* dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(long_t*)src); src += 4;
	dest->L1 = from_ulong(*(long_t*)src); src += 4;
	dest->L2 = from_ulong(*(long_t*)src); src += 4;
	dest->L3 = from_ulong(*(long_t*)src); src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->i5 = *(int*)src; src += 4;
	dest->i6 = *(int*)src; src += 4;
	dest->i7 = *(int*)src; src += 4;
	dest->L8 = from_ulong(*(long_t*)src); src += 4;
	dest->L9 = from_ulong(*(long_t*)src); src += 4;
	dest->i10 = *(int*)src; src += 4;
	dest->l11 = from_long(*(long_t*)src); src += 4;
	dest->l12 = from_long(*(long_t*)src); src += 4;
	dest->i13 = *(int*)src; src += 4;
	dest->L14 = from_ulong(*(long_t*)src); src += 4;
	dest->L15 = from_ulong(*(long_t*)src); src += 4;
}
void to_struct_LLLLiiiLLilliLL(ptr_t d, const struct_LLLLiiiLLilliLL_t* src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L2); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L3); dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(int*)dest = src->i5; dest += 4;
	*(int*)dest = src->i6; dest += 4;
	*(int*)dest = src->i7; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L8); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L9); dest += 4;
	*(int*)dest = src->i10; dest += 4;
	*(ulong_t*)dest = to_long(src->l11); dest += 4;
	*(ulong_t*)dest = to_long(src->l12); dest += 4;
	*(int*)dest = src->i13; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L14); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L15); dest += 4;
}

void from_struct_iiuuLip(struct_iiuuLip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
	dest->u3 = *(uint32_t*)src; src += 4;
	dest->L4 = from_ulong(*(long_t*)src); src += 4;
	dest->i5 = *(int*)src; src += 4;
	dest->p6 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_iiuuLip(ptr_t d, const struct_iiuuLip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
	*(uint32_t*)dest = src->u3; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L4); dest += 4;
	*(int*)dest = src->i5; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p6); dest += 4;
}

void from_struct_pLiiiLLLii(struct_pLiiiLLLii_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->L1 = from_ulong(*(long_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->i3 = *(int*)src; src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->L5 = from_ulong(*(long_t*)src); src += 4;
	dest->L6 = from_ulong(*(long_t*)src); src += 4;
	dest->L7 = from_ulong(*(long_t*)src); src += 4;
	dest->i8 = *(int*)src; src += 4;
	dest->i9 = *(int*)src; src += 4;
}
void to_struct_pLiiiLLLii(ptr_t d, const struct_pLiiiLLLii_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(int*)dest = src->i3; dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L5); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L6); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L7); dest += 4;
	*(int*)dest = src->i8; dest += 4;
	*(int*)dest = src->i9; dest += 4;
}

void from_struct_LiiuL(struct_LiiuL_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(long_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->u3 = *(uint32_t*)src; src += 4;
	dest->L4 = from_ulong(*(long_t*)src); src += 4;
}
void to_struct_LiiuL(ptr_t d, const struct_LiiuL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(uint32_t*)dest = src->u3; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L4); dest += 4;
}

void from_struct_WWWWWWWWWuip(struct_WWWWWWWWWuip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->W0 = *(uint16_t*)src; src += 2;
	dest->W1 = *(uint16_t*)src; src += 2;
	dest->W2 = *(uint16_t*)src; src += 2;
	dest->W3 = *(uint16_t*)src; src += 2;
	dest->W4 = *(uint16_t*)src; src += 2;
	dest->W5 = *(uint16_t*)src; src += 2;
	dest->W6 = *(uint16_t*)src; src += 2;
	dest->W7 = *(uint16_t*)src; src += 2;
	dest->W8 = *(uint16_t*)src; src += 2;
	dest->u9 = *(uint32_t*)src; src += 4;
	dest->i10 = *(int*)src; src += 4;
	dest->p11 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_WWWWWWWWWuip(ptr_t d, const struct_WWWWWWWWWuip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint16_t*)dest = src->W0; dest += 2;
	*(uint16_t*)dest = src->W1; dest += 2;
	*(uint16_t*)dest = src->W2; dest += 2;
	*(uint16_t*)dest = src->W3; dest += 2;
	*(uint16_t*)dest = src->W4; dest += 2;
	*(uint16_t*)dest = src->W5; dest += 2;
	*(uint16_t*)dest = src->W6; dest += 2;
	*(uint16_t*)dest = src->W7; dest += 2;
	*(uint16_t*)dest = src->W8; dest += 2;
	*(uint32_t*)dest = src->u9; dest += 4;
	*(int*)dest = src->i10; dest += 4;
	*(ulong_t*)dest = to_ptrv(src->p11); dest += 4;
}

void from_struct_LiiwwwwwwwwL(struct_LiiwwwwwwwwL_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(long_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->w3 = *(short*)src; src += 2;
	dest->w4 = *(short*)src; src += 2;
	dest->w5 = *(short*)src; src += 2;
	dest->w6 = *(short*)src; src += 2;
	dest->w7 = *(short*)src; src += 2;
	dest->w8 = *(short*)src; src += 2;
	dest->w9 = *(short*)src; src += 2;
	dest->w10 = *(short*)src; src += 2;
	dest->L11 = from_ulong(*(long_t*)src); src += 4;
}
void to_struct_LiiwwwwwwwwL(ptr_t d, const struct_LiiwwwwwwwwL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_ulong(src->L0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(short*)dest = src->w3; dest += 2;
	*(short*)dest = src->w4; dest += 2;
	*(short*)dest = src->w5; dest += 2;
	*(short*)dest = src->w6; dest += 2;
	*(short*)dest = src->w7; dest += 2;
	*(short*)dest = src->w8; dest += 2;
	*(short*)dest = src->w9; dest += 2;
	*(short*)dest = src->w10; dest += 2;
	*(ulong_t*)dest = to_ulong(src->L11); dest += 4;
}

void from_struct_iLiiiiLiiiiLi(struct_iLiiiiLiiiiLi_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->i0 = *(int*)src; src += 4;
	dest->L1 = from_ulong(*(long_t*)src); src += 4;
	dest->i2 = *(int*)src; src += 4;
	dest->i3 = *(int*)src; src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->i5 = *(int*)src; src += 4;
	dest->L6 = from_ulong(*(long_t*)src); src += 4;
	dest->i7 = *(int*)src; src += 4;
	dest->i8 = *(int*)src; src += 4;
	dest->i9 = *(int*)src; src += 4;
	dest->i10 = *(int*)src; src += 4;
	dest->L11 = from_ulong(*(long_t*)src); src += 4;
	dest->i12 = *(int*)src; src += 4;
}
void to_struct_iLiiiiLiiiiLi(ptr_t d, const struct_iLiiiiLiiiiLi_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(int*)dest = src->i0; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L1); dest += 4;
	*(int*)dest = src->i2; dest += 4;
	*(int*)dest = src->i3; dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(int*)dest = src->i5; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L6); dest += 4;
	*(int*)dest = src->i7; dest += 4;
	*(int*)dest = src->i8; dest += 4;
	*(int*)dest = src->i9; dest += 4;
	*(int*)dest = src->i10; dest += 4;
	*(ulong_t*)dest = to_ulong(src->L11); dest += 4;
	*(int*)dest = src->i12; dest += 4;
}

void from_struct_pppiiip(struct_pppiiip_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p2 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->i3 = *(int*)src; src += 4;
	dest->i4 = *(int*)src; src += 4;
	dest->i5 = *(int*)src; src += 4;
	dest->p6 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_pppiiip(ptr_t d, const struct_pppiiip_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p2); dest += 4;
	*(int*)dest = src->i3; dest += 4;
	*(int*)dest = src->i4; dest += 4;
	*(int*)dest = src->i5; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p6); dest += 4;
}

void from_struct_LWww(struct_LWww_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->L0 = from_ulong(*(uint32_t*)src); src += 4;
	dest->W1 = *(uint16_t*)src; src += 2;
	dest->W2 = *(int16_t*)src; src += 2;
	dest->W3 = *(int16_t*)src; src += 2;
}
void to_struct_LWww(ptr_t d, const struct_LWww_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = to_ulong(src->L0); dest += 4;
	*(uint16_t*)dest = src->W1; dest += 2;
	*(int16_t*)dest = src->W2; dest += 2;
	*(int16_t*)dest = src->W3; dest += 2;
}

void from_struct_ppuuuuup(struct_ppuuuuup_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
	dest->u3 = *(uint32_t*)src; src += 4;
	dest->u4 = *(uint32_t*)src; src += 4;
	dest->u5 = *(uint32_t*)src; src += 4;
	dest->u6 = *(uint32_t*)src; src += 4;
	dest->p7 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_ppuuuuup(ptr_t d, const struct_ppuuuuup_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
	*(uint32_t*)dest = src->u3; dest += 4;
	*(uint32_t*)dest = src->u4; dest += 4;
	*(uint32_t*)dest = src->u5; dest += 4;
	*(uint32_t*)dest = src->u6; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p7); dest += 4;
}

void from_struct_ppuii(struct_ppuii_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
	dest->i3 = *(int32_t*)src; src += 4;
	dest->i4 = *(int32_t*)src; src += 4;
}
void to_struct_ppuii(ptr_t d, const struct_ppuii_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
	*(int32_t*)dest = src->i3; dest += 4;
	*(int32_t*)dest = src->i4; dest += 4;
}

void from_struct_ppuiiiiiiiiipp(struct_ppuiiiiiiiiipp_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->p0 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p1 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->u2 = *(uint32_t*)src; src += 4;
	dest->i3 = *(int32_t*)src; src += 4;
	dest->i4 = *(int32_t*)src; src += 4;
	dest->i5 = *(int32_t*)src; src += 4;
	dest->i6 = *(int32_t*)src; src += 4;
	dest->i7 = *(int32_t*)src; src += 4;
	dest->i8 = *(int32_t*)src; src += 4;
	dest->i9 = *(int32_t*)src; src += 4;
	dest->i10 = *(int32_t*)src; src += 4;
	dest->i11 = *(int32_t*)src; src += 4;
	dest->p12 = from_ptrv(*(ptr_t*)src); src += 4;
	dest->p13 = from_ptrv(*(ptr_t*)src); src += 4;
}
void to_struct_ppuiiiiiiiiipp(ptr_t d, const struct_ppuiiiiiiiiipp_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ptr_t*)dest = to_ptrv(src->p0); dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
	*(uint32_t*)dest = src->u2; dest += 4;
	*(int32_t*)dest = src->i3; dest += 4;
	*(int32_t*)dest = src->i4; dest += 4;
	*(int32_t*)dest = src->i5; dest += 4;
	*(int32_t*)dest = src->i6; dest += 4;
	*(int32_t*)dest = src->i7; dest += 4;
	*(int32_t*)dest = src->i8; dest += 4;
	*(int32_t*)dest = src->i9; dest += 4;
	*(int32_t*)dest = src->i10; dest += 4;
	*(int32_t*)dest = src->i11; dest += 4;
	*(ptr_t*)dest = to_ptrv_silent(src->p12); dest += 4;
	*(ptr_t*)dest = to_ptrv_silent(src->p13); dest += 4;
}

void from_struct_lii(struct_lii_t *dest, ptr_t s) {
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->l0 = from_long(*(long_t*)src); src += 4;
	dest->i1 = *(int*)src; src += 4;
	dest->i2 = *(int*)src; src += 4;
}
void to_struct_lii(ptr_t d, const struct_lii_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(ulong_t*)dest = to_long(src->l0); dest += 4;
	*(int*)dest = src->i1; dest += 4;
	*(int*)dest = src->i2; dest += 4;
}

void from_struct_UUUULLLLLLLLLLLLLL(struct_UUUULLLLLLLLLLLLLL_t *dest, ptr_t s) {
	if(!s) return;
	uint8_t* src = (uint8_t*)from_ptrv(s);
	dest->U0 = *(uint64_t*)src; src += 8;
	dest->U1 = *(uint64_t*)src; src += 8;
	dest->U2 = *(uint64_t*)src; src += 8;
	dest->U3 = *(uint64_t*)src; src += 8;
	dest->L4 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L5 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L6 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L7 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L8 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L9 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L10 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L11 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L12 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L13 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L14 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L15 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L16 = from_ulong(*(ulong_t*)src); src += 4;
	dest->L17 = from_ulong(*(ulong_t*)src); src += 4;
}
void to_struct_UUUULLLLLLLLLLLLLL(ptr_t d, const struct_UUUULLLLLLLLLLLLLL_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint64_t*)dest = src->U0; dest += 8;
	*(uint64_t*)dest = src->U1; dest += 8;
	*(uint64_t*)dest = src->U2; dest += 8;
	*(uint64_t*)dest = src->U3; dest += 8;
	*(ulong_t*)dest = to_ulong(src->L4); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L5); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L6); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L7); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L8); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L9); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L10); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L11); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L12); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L13); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L14); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L15); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L16); dest += 4;
	*(ulong_t*)dest = to_ulong(src->L17); dest += 4;
}