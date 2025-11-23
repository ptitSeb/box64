#ifndef __MY_ALIGN__H_
#define __MY_ALIGN__H_
#include <stdint.h>

#include "mysignal.h"

typedef struct x64_va_list_s {
   unsigned int gp_offset;
   unsigned int fp_offset;
   void *overflow_arg_area;
   void *reg_save_area;
} x64_va_list_t[1];

#define X64_VA_MAX_REG  (6*8)
#define X64_VA_MAX_XMM  ((6*8)+(8*16))

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
#define CREATE_SYSV_VALIST(A)             \
  va_list sysv_varargs;                   \
  sysv_varargs->gp_offset=X64_VA_MAX_REG; \
  sysv_varargs->fp_offset=X64_VA_MAX_XMM; \
  sysv_varargs->reg_save_area=(A);        \
  sysv_varargs->overflow_arg_area=A

#define CONVERT_VALIST(A)                 \
  va_list sysv_varargs;                   \
  sysv_varargs->gp_offset=(A)->gp_offset; \
  sysv_varargs->fp_offset=(A)->fp_offset; \
  sysv_varargs->reg_save_area=(A)->reg_save_area;  \
  sysv_varargs->overflow_arg_area=(A)->overflow_arg_area;

#define CREATE_VALIST_FROM_VAARG(STACK, SCRATCH, N) \
  va_list sysv_varargs;                             \
  sysv_varargs->gp_offset=(6-(((N)<6)?(N):6))*8;    \
  sysv_varargs->fp_offset=(6*8);                    \
  sysv_varargs->reg_save_area=(SCRATCH);            \
  sysv_varargs->overflow_arg_area=(STACK);          \
  {                                                 \
    uint64_t *p = (uint64_t*)(SCRATCH);             \
    p[0]=R_RDI; p[1]=R_RSI; p[2]=R_RDX;             \
    p[3]=R_RCX; p[4]=R_R8; p[5]=R_R9;               \
    memcpy(&p[6], emu->xmm, 8*16);                  \
  }

#define PREFER_CONVERT_VAARG

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
  sysv_varargs.__gr_offs=(8*8); \
  sysv_varargs.__vr_offs=(8*16); \
  sysv_varargs.__stack=(A)

#define CONVERT_VALIST(A)                                         \
  va_list sysv_varargs;                                           \
  sysv_varargs.__gr_offs=-(6*8)+(A)->gp_offset;                   \
  sysv_varargs.__vr_offs=-(8*16)+((A)->fp_offset-X64_VA_MAX_REG); \
  sysv_varargs.__stack=(A)->overflow_arg_area;                    \
  sysv_varargs.__gr_top=(A)->reg_save_area + X64_VA_MAX_REG;      \
  sysv_varargs.__vr_top=(A)->reg_save_area + X64_VA_MAX_XMM;

#define CREATE_VALIST_FROM_VAARG(STACK, SCRATCH, N)                     \
  va_list sysv_varargs;                                                 \
  sysv_varargs.__gr_offs=-(6*8)+(8*(((N)<6)?(N):6));                    \
  sysv_varargs.__vr_offs=-(8*16);                                       \
  sysv_varargs.__stack=(STACK);                                         \
  sysv_varargs.__gr_top=(void*)((uintptr_t)(SCRATCH) + X64_VA_MAX_REG); \
  sysv_varargs.__vr_top=(void*)((uintptr_t)(SCRATCH) + X64_VA_MAX_XMM); \
  {                                                                     \
    uintptr_t *p = (uintptr_t*)(SCRATCH);                               \
    p[0]=R_RDI; p[1]=R_RSI; p[2]=R_RDX;                                 \
    p[3]=R_RCX; p[4]=R_R8; p[5]=R_R9;                                   \
    memcpy(&p[6], emu->xmm, 8*16);                                      \
  }

#define PREFER_CONVERT_VAARG

#elif defined(__sw_64__) /* or Alpha */
/*
typdef struct {
  char* __base;
  int __offset;
}va_list;
*/

// the follow three macro is not fully compatiable with SW64/Alpha
// so don't expect va function works well.
#define CREATE_SYSV_VALIST(A)   \
  va_list sysv_varargs;         \
  sysv_varargs.__offset=0;      \
  sysv_varargs.__base=(A)

#define CREATE_VALIST_FROM_VALIST(VA, SCRATCH)                          \
  va_list sysv_varargs;                                                 \
  {                                                                     \
    uintptr_t *p = (uintptr_t*)(SCRATCH);                               \
    int n = (X64_VA_MAX_REG - (VA)->gp_offset)/8;                       \
    if(n) memcpy(&p[0], (VA)->reg_save_area+X64_VA_MAX_REG-n*8, n*8);   \
    memcpy(&p[n], (VA)->overflow_arg_area, 20*8);                       \
    sysv_varargs.__offset = (VA)->gp_offset;                            \
    sysv_varargs.__base = (char*)p;                                     \
  }

#define CREATE_VALIST_FROM_VAARG(STACK, SCRATCH, N)                     \
  va_list sysv_varargs;                                                 \
  {                                                                     \
    uintptr_t *p = (uintptr_t*)(SCRATCH);                               \
    p[0]=R_RDI; p[1]=R_RSI; p[2]=R_RDX;                                 \
    p[3]=R_RCX; p[4]=R_R8; p[5]=R_R9;                                   \
    memcpy(&p[8+N], STACK, 20*8);                                       \
    sysv_varargs.__offset = N*8;                                        \
    sysv_varargs.__base = (char*)p;                                     \
  }

#elif defined(__loongarch64) || defined(__powerpc64__) || defined(__riscv)
#define CREATE_SYSV_VALIST(A) \
  va_list sysv_varargs = (va_list)A
// not creating CONVERT_VALIST(A) on purpose
// this one will create a VA_LIST from x64_va_list using only GPRS (NOFLOAT) and 100 stack element
#define CREATE_VALIST_FROM_VALIST(VA, SCRATCH)                          \
  va_list sysv_varargs;                                                 \
  {                                                                     \
    if((VA)->fp_offset!=X64_VA_MAX_XMM) printf_log(LOG_DEBUG, "Warning: %s: CREATE_VALIST_FROM_VALIST with %d XMM register!\n", __FUNCTION__, (X64_VA_MAX_XMM - (VA)->fp_offset)/16);\
    uintptr_t *p = (uintptr_t*)(SCRATCH);                               \
    int n = (X64_VA_MAX_REG - (VA)->gp_offset)/8;                       \
    if(n) memcpy(&p[0], (VA)->reg_save_area+X64_VA_MAX_REG-n*8, n*8);   \
    memcpy(&p[n], (VA)->overflow_arg_area, 20*8);                       \
    sysv_varargs = (va_list)p;                                          \
  }
// this is an approximation, and if the va_list have some float/double, it will fail!
// if the funciton needs more than 100 args, it will also fail
#define CREATE_VALIST_FROM_VAARG(STACK, SCRATCH, N)                     \
  va_list sysv_varargs;                                                 \
  {                                                                     \
    uintptr_t *p = (uintptr_t*)(SCRATCH);                               \
    p[0]=R_RDI; p[1]=R_RSI; p[2]=R_RDX;                                 \
    p[3]=R_RCX; p[4]=R_R8; p[5]=R_R9;                                   \
    memcpy(&p[6], STACK, 20*8);                                         \
    sysv_varargs = (va_list)&p[N];                                      \
  }
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
void myStackAlignScanf(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int pos);
void myStackAlignW(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int xmm, int pos);
void myStackAlignScanfW(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int pos);
#ifndef CONVERT_VALIST
void myStackAlignValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va);
void myStackAlignWValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va);
void myStackAlignScanfValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va);
void myStackAlignScanfWValist(x64emu_t* emu, const char* fmt, uint64_t* mystack, x64_va_list_t va);
void myStackAlignGVariantNewVa(x64emu_t* emu, const char* fmt, uint64_t* scratch, x64_va_list_t* b);
void myStackAlignGVariantNew(x64emu_t* emu, const char* fmt, uint64_t* st, uint64_t* mystack, int pos);
#endif

struct x64_stat64 {                   /* x86_64       arm64 */
    uint64_t st_dev;                    /* 0   */   /* 0   */
    uint64_t st_ino;                    /* 8   */   /* 8   */
    uint64_t st_nlink;                  /* 16  */   /* 20  */
    uint32_t st_mode;                   /* 24  */   /* 16  */
    uint32_t st_uid;                    /* 28  */   /* 24  */
    uint32_t st_gid;                    /* 32  */   /* 28  */
    int __pad0;                         /* 36  */   /* --- */
    uint64_t st_rdev;                   /* 40  */   /* 32  */
    int64_t st_size;                    /* 48  */   /* 48  */
    int64_t st_blksize;                 /* 56  */   /* 56  */
    uint64_t st_blocks;                 /* 64  */   /* 64  */
    struct timespec st_atim;            /* 72  */   /* 72  */
    struct timespec st_mtim;            /* 88  */   /* 88  */
    struct timespec st_ctim;            /* 104 */   /* 104 */
    uint64_t __glibc_reserved[3];       /* 120 */   /* 120 */
} __attribute__((packed));              /* 144 */   /* 128 */

void AlignStat64(const void* source, void* dest);
void UnalignStat64(const void* source, void* dest);

// defined in wrapperlibc.c
int of_convert(int);    // x86->arm
int of_unconvert(int);  // arm->x86

void UnalignEpollEvent(void* dest, void* source, int nbr); // Arm -> x86
void AlignEpollEvent(void* dest, void* source, int nbr); // x86 -> Arm

void UnalignSemidDs(void *dest, const void* source);
void AlignSemidDs(void *dest, const void* source);

void* align_xcb_connection(void* src);
void unalign_xcb_connection(void* src, void* dst);
void* add_xcb_connection(void* src);
void del_xcb_connection(void* src);
void register_xcb_display(void* d, void* xcb);
void unregister_xcb_display(void* d);

uintptr_t getVArgs(x64emu_t* emu, int pos, uintptr_t* b, int N);
void setVArgs(x64emu_t* emu, int pos, uintptr_t* b, int N, uintptr_t a);

// longjmp / setjmp
typedef struct jump_buff_x64_s {
    uint64_t save_rbx;
    uint64_t save_rbp;
    uint64_t save_r12;
    uint64_t save_r13;
    uint64_t save_r14;
    uint64_t save_r15;
    uint64_t save_rsp;
    uint64_t save_rip;
} jump_buff_x64_t;

typedef struct __jmp_buf_tag_s {
    jump_buff_x64_t __jmpbuf;
    int              __mask_was_saved;
    #ifdef ANDROID
    union {
      sigset_t         __saved_mask;
      sigset64_t         __saved_mask64;
    };
    #else
    __sigset_t       __saved_mask;
    #endif
} __jmp_buf_tag_t;

#endif  //__MY_ALIGN__H_
