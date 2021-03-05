#include <stdint.h>

#ifdef __x86_64__
// x86_64, 6 64bits general regs and 16 or 8? 128bits float regs
/*
For reference, here is the x86_64 va_list structure
typedef struct {
   unsigned int gp_offset;
   unsigned int fp_offset;
   void *overflow_arg_area;
   void *reg_save_area;
} va_list[1];
*/
#define CREATE_SYSV_VALIST(A) \
  va_list sysv_varargs; \
  sysv_varargs->gp_offset=(6*8); \
  sysv_varargs->fp_offset=(6*8)+(16*16); \
  sysv_varargs->overflow_arg_area=A;
#elif defined(__aarch64__)
// aarch64: 8 64bits general regs and 8 128bits float regs
/*
va_list declared as
typedef struct  va_list {
    void * stack; // next stack param
    void * gr_top; // end of GP arg reg save area
    void * vr_top; // end of FP/SIMD arg reg save area
    int gr_offs; // offset from  gr_top to next GP register arg
    int vr_offs; // offset from  vr_top to next FP/SIMD register arg
} va_list;
*/
#define CREATE_SYSV_VALIST(A) \
  va_list sysv_varargs; \
  sysv_varargs.gr_offs=(8*8); \
  sysv_varargs.vr_offs=(8*16); \
  sysv_varargs.stack=A;
#elif defined(__powerpc64__)
// TODO, is this correct?
#define CREATE_SYSV_VALIST(A) \
  va_list sysv_varargs; \
  sysv_varargs->gpr=8; \
  sysv_varargs->fpr=8; \
  sysv_varargs->overflow_arg_area=A;
#else
#error Unknown architecture!
#endif


#define VARARGS sysv_varargs
#define PREPARE_VALIST CREATE_SYSV_VALIST(emu->scratch)
#define VARARGS_(A) sysv_varargs
#define PREPARE_VALIST_(A) CREATE_SYSV_VALIST(A)

typedef struct x64emu_s x64emu_t;

// 1st pos is of vaarg is 0, not 1!
void myStackAlign(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm, int pos);
void myStackAlignGVariantNew(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm, int pos);
void myStackAlignW(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm, int pos);

// defined in wrapperlibc.c
int of_convert(int);    // x86->arm
int of_unconvert(int);  // arm->x86
