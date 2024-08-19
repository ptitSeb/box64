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

#endif // __CONVERTER_H_
