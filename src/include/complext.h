#ifndef __COMPLEX_T__H_
#define __COMPLEX_T__H_

typedef struct complex_s { double r; double i;} complex_t;
typedef struct complexf_s { float r; float i;} complexf_t;

#endif //__COMPLEX_T__H_

#ifdef COMPLEX_IMPL
#ifndef __COMPLEX_T_IMPL_H_
#define __COMPLEX_T_IMPL_H_
inline complexf_t to_complexf(x64emu_t* emu, int i) {
    complexf_t ret; 
    ret.r = emu->xmm[i].f[0];
    ret.i = emu->xmm[i+1].f[0];
    return ret;
}
inline complex_t to_complex(x64emu_t* emu, int i) {
    complex_t ret;
    ret.r = emu->xmm[i].d[0];
    ret.i = emu->xmm[i+1].d[0];
    return ret;
}
inline void from_complexf(x64emu_t* emu, complexf_t v) {
    emu->xmm[0].f[0]=v.r;
    emu->xmm[0].f[1]=v.i;
}
inline void from_complex(x64emu_t* emu, complex_t v) {
    emu->xmm[0].d[0]=v.r; 
    emu->xmm[1].d[0]=v.i;
}
#endif // __COMPLEX_T_IMPL_H_
#endif  // COMPLEX_IMPL
