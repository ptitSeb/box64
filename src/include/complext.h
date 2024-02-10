#ifndef __COMPLEX_T__H_
#define __COMPLEX_T__H_

typedef struct complex_s { double r; double i;} complex_t;
typedef struct complexf_s { float r; float i;} complexf_t;
typedef struct complexl_s { long double r; long double i;} complexl_t;


#endif //__COMPLEX_T__H_

#ifdef COMPLEX_IMPL
#include "emu/x87emu_private.h"
#include "x64emu.h"
#ifndef __COMPLEX_T_IMPL_H_
#define __COMPLEX_T_IMPL_H_
static inline complexf_t to_complexf(x64emu_t* emu, int i) {
    complexf_t ret; 
    ret.r = emu->xmm[i].f[0];
    ret.i = emu->xmm[i].f[1];
    return ret;
}
static inline complex_t to_complex(x64emu_t* emu, int i) {
    complex_t ret;
    ret.r = emu->xmm[i].d[0];
    ret.i = emu->xmm[i+1].d[0];
    return ret;
}
static inline complexl_t to_complexl(x64emu_t* emu, uintptr_t p) {
    complexl_t ret;
    ret.r = *(long double*)p;
    ret.i = *(long double*)(p+16);
    return ret;
}
static inline complex_t to_complexk(x64emu_t* emu, uintptr_t p) {
    complex_t ret;
    ret.r = FromLD((long double*)p);
    ret.i = FromLD((long double*)(p+16));
    return ret;
}
static inline void from_complexf(x64emu_t* emu, complexf_t v) {
    emu->xmm[0].f[0]=v.r;
    emu->xmm[0].f[1]=v.i;
}
static inline void from_complex(x64emu_t* emu, complex_t v) {
    emu->xmm[0].d[0]=v.r; 
    emu->xmm[1].d[0]=v.i;
}
static inline void from_complexl(x64emu_t* emu, complexl_t v) {
    fpu_do_push(emu);
    fpu_do_push(emu);
    ST0.d=FromLD(&v.r); 
    ST(1).d=FromLD(&v.i);
}
static inline void from_complexk(x64emu_t* emu, complex_t v) {
    fpu_do_push(emu);
    fpu_do_push(emu);
    ST0.d=v.r;
    ST1.d=v.i;
}
#endif // __COMPLEX_T_IMPL_H_
#endif  // COMPLEX_IMPL
