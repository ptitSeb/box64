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
#ifdef DYNAREC
#define STEP if(step) return 0;
#define STEP2 if(step) {R_RIP = addr; return 0;}
#define STEP3 if(*step) (*step)++;
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
#define GETED32(D)          oped=TestEd32O(test, &addr, rex, nextop, D, 0)
#define GETED_OFFS(D, O)    oped=TestEdO(test, &addr, rex, nextop, D, O)
#define GETED_OFFS_16(O)    oped=TestEd16off(test, &addr, rex, nextop, O)
#define GETGD               opgd=GetGd(test->emu, &addr, rex, nextop)
#define GETEB(D)            oped=TestEb(test, &addr, rex, nextop, D)
#define GETEB32(D)          oped=TestEb32O(test, &addr, rex, nextop, D, 0)
#define GETEB_OFFS(D, O)    oped=TestEbO(test, &addr, rex, nextop, D, O)
#define GETGB               opgd=GetGb(test->emu, &addr, rex, nextop)
#define GETEW(D)            oped=TestEw(test, &addr, rex, nextop, D)
#define GETEW32(D)          oped=TestEw32O(test, &addr, rex, nextop, D, 0)
#define GETEW_OFFS(D, O)    oped=TestEdO(test, &addr, rex, nextop, D, O)
#define GETEW_OFFS_16(O)    oped=TestEw16off(test, &addr, rex, nextop, O)
#define GETGW               opgd=GetGw(test->emu, &addr, rex, nextop)
#define GETEX(D)            opex=TestEx(test, &addr, rex, nextop, D)
#define GETEX32(D)          opex=TestEx32O(test, &addr, rex, nextop, D, 0)
#define GETEX_OFFS(D, O)    opex=TestExO(test, &addr, rex, nextop, D, O)
#define GETGX               opgx=GetGx(test->emu, &addr, rex, nextop)
#define GETEM(D)            opem=TestEm(test, &addr, rex, nextop, D)
#define GETEM32(D)          opem=TestEm32O(test, &addr, rex, nextop, D, 0)
#define GETGM               opgm=GetGm(test->emu, &addr, rex, nextop)
#else
#define GETED(D)            oped=GetEd(emu, &addr, rex, nextop, D)
#define GETE4(D)            GETED(D)
#define GETE8(D)            GETED(D)
#define GETET(D)            GETED(D)
#define GETE8xw(D)          GETED(D)
#define GETED32(D)          oped=GetEd32O(emu, &addr, rex, nextop, D, 0)
#define GETED_OFFS(D, O)    oped=GetEdO(emu, &addr, rex, nextop, D, O)
#define GETED_OFFS_16(O)    oped=GetEd16off(emu, &addr, rex, nextop, O)
#define GETGD               opgd=GetGd(emu, &addr, rex, nextop)
#define GETEB(D)            oped=GetEb(emu, &addr, rex, nextop, D)
#define GETEB32(D)          oped=GetEb32O(emu, &addr, rex, nextop, D, 0)
#define GETEB_OFFS(D, O)    oped=GetEbO(emu, &addr, rex, nextop, D, O)
#define GETGB               opgd=GetGb(emu, &addr, rex, nextop)
#define GETEW(D)            oped=GetEw(emu, &addr, rex, nextop, D)
#define GETEW32(D)          oped=GetEw32O(emu, &addr, rex, nextop, D, 0)
#define GETEW_OFFS(D, O)    oped=GetEdO(emu, &addr, rex, nextop, D, O)
#define GETEW_OFFS_16(O)    oped=GetEw16off(emu, &addr, rex, nextop, O)
#define GETGW               opgd=GetGw(emu, &addr, rex, nextop)
#define GETEX(D)            opex=GetEx(emu, &addr, rex, nextop, D)
#define GETEX32(D)          opex=GetEx32O(emu, &addr, rex, nextop, D, 0)
#define GETEX_OFFS(D, O)    opex=GetExO(emu, &addr, rex, nextop, D, O)
#define GETGX               opgx=GetGx(emu, &addr, rex, nextop)
#define GETEM(D)            opem=GetEm(emu, &addr, rex, nextop, D)
#define GETEM32(D)          opem=GetEm32O(emu, &addr, rex, nextop, D, 0)
#define GETGM               opgm=GetGm(emu, &addr, rex, nextop)
#endif
#define ED  oped
#define GD  opgd
#define EB  oped
#define GB  opgd->byte[0]
#define EW  oped
#define GW  opgd
#define EX  opex
#define GX  opgx
#define EM  opem
#define GM  opgm
#define FAKEED(D)           GetEd(emu, &addr, rex, nextop, D)
#define FAKEED32(D)         GetEd32O(emu, &addr, rex, nextop, D, 0)
#define GETEA(D)            GetEA(emu, &addr, rex, nextop, D)
#define GETEA32(D)          GetEA32(emu, &addr, rex, nextop, D)
#define _GETED(D)           oped=GetEd(emu, &addr, rex, nextop, D)
#define _GETED32(D)         oped=GetEd32O(emu, &addr, rex, nextop, D, 0)
#define _GETEB(D)           oped=GetEb(emu, &addr, rex, nextop, D)

#define MODREG  ((nextop&0xC0)==0xC0)

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
