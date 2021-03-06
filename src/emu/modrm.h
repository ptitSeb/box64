#define F8      *(uint8_t*)(R_RIP++)
#define F8S     *(int8_t*)(R_RIP++)
#define F16     *(uint16_t*)(R_RIP+=2, R_RIP-2)
#define F32     *(uint32_t*)(R_RIP+=4, R_RIP-4)
#define F32S    *(int32_t*)(R_RIP+=4, R_RIP-4)
#define F32S64  (uint64_t)(int64_t)F32S
#define F64     *(uint64_t*)(R_RIP+=8, R_RIP-8)
#define F64S    *(int64_t*)(R_RIP+=8, R_RIP-8)
#define PK(a)   *(uint8_t*)(R_RIP+a)
#ifdef DYNAREC
#define STEP if(step) return 0;
#else
#define STEP
#endif

#define GETED oped=GetEd(emu, rex, nextop)
#define GETGD opgd=GetGd(emu, rex, nextop)
#define GETEB oped=GetEb(emu, rex, nextop)
#define GETGB opgd=GetGb(emu, rex, nextop)
#define GETEW oped=GetEw(emu, rex, nextop)
#define GETGW opgd=GetGd(emu, rex, nextop)
#define GETEX opex=GetEx(emu, rex, nextop)
#define GETGX opgx=GetGx(emu, rex, nextop)
#define ED  oped
#define GD  opgd
#define EB  oped
#define GB  opgd->byte[0]
#define EW  oped
#define GW  opgd
#define EX  opex
#define GX  opgx

#define GOCOND(BASE, PREFIX, CONDITIONAL)       \
    case BASE+0x0:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_OF)) {                 \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x1:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_OF)) {                \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x2:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_CF)) {                 \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x3:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_CF)) {                \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x4:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_ZF)) {                 \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x5:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_ZF)) {                \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x6:                              \
        PREFIX                                  \
        if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) {  \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x7:                              \
        PREFIX                                  \
        if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) { \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x8:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF)) {                 \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0x9:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_SF)) {                \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0xA:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_PF)) {                 \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0xB:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_PF)) {                \
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0xC:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF)) {\
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0xD:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) {\
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0xE:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) {\
            CONDITIONAL                         \
        }                                       \
        break;                                  \
    case BASE+0xF:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) {\
            CONDITIONAL                         \
        }                                       \
        break;
