#include <stddef.h>

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F16S    *(int16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define F32S64  (uint64_t)(int64_t)F32S
#define F64     *(uint64_t*)(addr+=8, addr-8)
#define F64S    *(int64_t*)(addr+=8, addr-8)
#define PK(a)   *(uint8_t*)(addr+a)
#define PARITY(x)   (((emu->x64emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)

#ifdef DYNAREC
#define STEP  CheckExec(emu, addr); if(step && !ACCESS_FLAG(F_TF)) return 0;
#define STEP2 CheckExec(emu, addr); if(step && !ACCESS_FLAG(F_TF)) {R_RIP = addr; return 0;}
#define STEP3 CheckExec(emu, addr); if(*step) (*step)++;
#else
#define STEP
#define STEP2
#define STEP3
#endif

#ifdef TEST_INTERPRETER
#define GETED(D)            oped=TestEd(test, &addr, rex, nextop, D)
#define GETE4(D)            oped=TestEd4(test, &addr, rex, nextop, D)
#define GETE8(D)            oped=TestEd8(test, &addr, rex, nextop, D)
#define GETET(D)            oped=TestEdt(test, &addr, rex, nextop, D)
#define GETE8xw(D)          oped=TestEd8xw(test, rex.w, &addr, rex, nextop, D)
#define GETGD               opgd=GetGd(test->emu, &addr, rex, nextop)
#define GETEB(D)            oped=TestEb(test, &addr, rex, nextop, D)
#define GETGB               opgd=GetGb(test->emu, &addr, rex, nextop)
#define GETEW(D)            oped=TestEw(test, &addr, rex, nextop, D)
#define GETGW               opgd=GetGw(test->emu, &addr, rex, nextop)
#define GETEX(D)            opex=TestEx(test, &addr, rex, nextop, D, 16)
#define GETEX4(D)           opex=TestEx(test, &addr, rex, nextop, D, 4)
#define GETEX8(D)           opex=TestEx(test, &addr, rex, nextop, D, 8)
#define GETGX               opgx=GetGx(test->emu, &addr, rex, nextop)
#define GETGY               opgy=GetGy(test->emu, &addr, rex, nextop)
#define GETEY               opey=TestEy(test, &addr, rex, nextop)
#define GETEM(D)            opem=TestEm(test, &addr, rex, nextop, D)
#define GETGM               opgm=GetGm(test->emu, &addr, rex, nextop)
#define GETVX               opvx=&test->emu->xmm[vex.v]
#define GETVY               opvy=&test->emu->ymm[vex.v]
#define GETVD               opvd=&test->emu->regs[vex.v]
#else
#define GETED(D)            oped=GetEd(emu, &addr, rex, nextop, D)
#define GETE4(D)            GETED(D)
#define GETE8(D)            GETED(D)
#define GETET(D)            GETED(D)
#define GETE8xw(D)          GETED(D)
#define GETGD               opgd=GetGd(emu, &addr, rex, nextop)
#define GETEB(D)            oped=GetEb(emu, &addr, rex, nextop, D)
#define GETGB               opgd=GetGb(emu, &addr, rex, nextop)
#define GETEW(D)            oped=GetEw(emu, &addr, rex, nextop, D)
#define GETGW               opgd=GetGw(emu, &addr, rex, nextop)
#define GETEX(D)            opex=GetEx(emu, &addr, rex, nextop, D)
#define GETEX4(D)           GETEX(D)
#define GETEX8(D)           GETEX(D)
#define GETGX               opgx=GetGx(emu, &addr, rex, nextop)
#define GETGY               opgy=GetGy(emu, &addr, rex, nextop)
#define GETEY               opey=(opex>=&emu->xmm[0] && opex<=&emu->xmm[15])?((sse_regs_t*)((uintptr_t)opex+offsetof(x64emu_t, ymm)-offsetof(x64emu_t, xmm))):((sse_regs_t*)((uintptr_t)opex+16))
#define GETEM(D)            opem=GetEm(emu, &addr, rex, nextop, D)
#define GETGM               opgm=GetGm(emu, &addr, rex, nextop)
#define GETVX               opvx=&emu->xmm[vex.v]
#define GETVY               opvy=&emu->ymm[vex.v]
#define GETVD               opvd=&emu->regs[vex.v]
#endif
#define ED  oped
#define GD  opgd
#define VD  opvd
#define EB  oped
#define GB  opgd->byte[0]
#define EW  oped
#define GW  opgd
#define EX  opex
#define GX  opgx
#define VX  opvx
#define EY  opey
#define GY  opgy
#define VY  opvy
#define EM  opem
#define GM  opgm
#define FAKEED(D)           GetEd(emu, &addr, rex, nextop, D)
#define GETEA(D)            GetEA(emu, &addr, rex, nextop, D)
#define _GETED(D)           oped=GetEd(emu, &addr, rex, nextop, D)
#define _GETEB(D)           oped=GetEb(emu, &addr, rex, nextop, D)
#define _GETEX(D)           opex=GetEx(emu, &addr, rex, nextop, D)

#define MODREG  ((nextop&0xC0)==0xC0)

#if defined(__riscv)
#define NAN_PROPAGATION(dest, src, break_or_continue) \
    if (isnan(dest)) {                                \
        break_or_continue;                            \
    } else if (isnan(src)) {                          \
        (dest) = (src);                               \
        break_or_continue;                            \
    }
#else
#define NAN_PROPAGATION(dest, src, break_or_continue)
#endif

#define MARK_NAN_VF_2(A, B) for(int idx=0; idx<4; ++idx) mask_nan[idx] = isnanf(A->f[idx]) || isnanf(B->f[idx])
#define CHECK_NAN_VF(A) for(int idx=0; idx<4; ++idx) if(!mask_nan[idx] && isnanf(A->f[idx])) A->f[idx] = -NAN

#define MARK_NAN_VD_2(A, B) for(int idx=0; idx<2; ++idx) mask_nan[idx] = isnan(A->d[idx]) || isnan(B->d[idx])
#define CHECK_NAN_VD(A) for(int idx=0; idx<2; ++idx) if(!mask_nan[idx] && isnan(A->d[idx])) A->d[idx] = -NAN

#define MARK_NAN_F_2(A, B) is_nan = isnanf(A->f[0]) || isnanf(B->f[0])
#define CHECK_NAN_F(A) if(!is_nan && isnanf(A->f[0])) A->f[0] = -NAN

#define MARK_NAN_D_2(A, B) is_nan = isnan(A->d[0]) || isnan(B->d[0])
#define CHECK_NAN_D(A) if(!is_nan && isnan(A->d[0])) A->d[0] = -NAN

#define GOCOND(BASE, PREFIX, COND, NOTCOND, POST)\
    case BASE+0x0:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_OF)) {                 \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x1:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_OF)) {                \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x2:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_CF)) {                 \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x3:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_CF)) {                \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x4:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_ZF)) {                 \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x5:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_ZF)) {                \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x6:                              \
        PREFIX                                  \
        if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) {  \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x7:                              \
        PREFIX                                  \
        if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) { \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x8:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF)) {                 \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0x9:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_SF)) {                \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0xA:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_PF)) {                 \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0xB:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_PF)) {                \
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0xC:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF)) {\
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0xD:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) {\
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0xE:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) {\
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;                                  \
    case BASE+0xF:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) {\
            COND                                \
        } else {                                \
            NOTCOND                             \
        }                                       \
        POST                                    \
        break;
