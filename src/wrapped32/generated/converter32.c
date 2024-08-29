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
	dest->p0 = *(void**)src; src += 4;
	dest->p1 = *(void**)src; src += 4;
	dest->p2 = *(void**)src; src += 4;
	dest->p3 = *(void**)src; src += 4;
	dest->p4 = *(void**)src; src += 4;
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
