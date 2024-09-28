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
	dest->p10 = *(void**)src; src += 4;
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
	dest->p1 = *(void**)src; src += 4;
}
void to_struct_up(ptr_t d, const struct_up_t *src) {
	if (!src) return;
	uint8_t* dest = (uint8_t*)from_ptrv(d);
	*(uint32_t*)dest = src->u0; dest += 4;
	*(ptr_t*)dest = to_ptrv(src->p1); dest += 4;
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
