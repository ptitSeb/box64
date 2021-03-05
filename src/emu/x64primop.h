#ifndef __X64PRIMOP_H_
#define __X64PRIMOP_H_

typedef struct x64emu_s x64emu_t;

// Based on libx86emu

uint16_t     aaa16 (x64emu_t *emu, uint16_t d);
uint16_t     aas16 (x64emu_t *emu, uint16_t d);
uint16_t     aad16 (x64emu_t *emu, uint16_t d, uint8_t base);
uint16_t     aam16 (x64emu_t *emu, uint8_t d, uint8_t base);
uint8_t      adc8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     adc16 (x64emu_t *emu, uint16_t d, uint16_t s);
uint32_t     adc32 (x64emu_t *emu, uint32_t d, uint32_t s);
uint64_t     adc64 (x64emu_t *emu, uint64_t d, uint64_t s);

static inline uint8_t add8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add8;
	return (uint8_t)emu->res;
}

static inline uint16_t add16(x64emu_t *emu, uint16_t d, uint16_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add16;
	return (uint16_t)emu->res;
}

static inline uint32_t add32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add32;
    return emu->res;
}

static inline uint64_t add64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add64;
    return emu->res;
}

static inline uint8_t and8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d & s;
	emu->df = d_and8;

	return emu->res;
}

static inline uint16_t and16(x64emu_t *emu, uint16_t d, uint16_t s)
{
    emu->res = d & s;
	emu->df = d_and16;

    return emu->res;
}

static inline uint32_t and32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d & s;
	emu->df = d_and32;

	return emu->res;
}

static inline uint64_t and64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	emu->res = d & s;
	emu->df = d_and64;

	return emu->res;
}

uint8_t      cmp8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     cmp16 (x64emu_t *emu, uint16_t d, uint16_t s);
uint32_t     cmp32 (x64emu_t *emu, uint32_t d, uint32_t s);
uint64_t     cmp64 (x64emu_t *emu, uint64_t d, uint64_t s);
uint8_t      daa8  (x64emu_t *emu, uint8_t d);
uint8_t      das8  (x64emu_t *emu, uint8_t d);

static inline uint8_t dec8(x64emu_t *emu, uint8_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec8;
	return (uint8_t)emu->res;
}

static inline uint16_t dec16(x64emu_t *emu, uint16_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec16;
	return (uint16_t)emu->res;

}

static inline uint32_t dec32(x64emu_t *emu, uint32_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec32;

	return emu->res;
}

static inline uint64_t dec64(x64emu_t *emu, uint64_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec64;

	return emu->res;
}

static inline uint8_t inc8(x64emu_t *emu, uint8_t d)
{
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc8;
	return (uint8_t)emu->res;
}

static inline uint16_t inc16(x64emu_t *emu, uint16_t d)
{
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc16;
	return (uint16_t)emu->res;
}

static inline uint32_t inc32(x64emu_t *emu, uint32_t d)
{
	if(emu->df == d_shr32) {
		// workaround for some wine trickery
		uint32_t cnt = emu->op2;
        if (cnt > 0) {
            uint32_t cc = emu->op1 & (1 << (cnt - 1));
			CONDITIONAL_SET_FLAG(cc, F_CF);
		}
	}
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc32;
	return emu->res;
}

static inline uint64_t inc64(x64emu_t *emu, uint64_t d)
{
	if(emu->df == d_shr64) {
		// workaround for some wine trickery
		uint64_t cnt = emu->op2;
        if (cnt > 0) {
            uint64_t cc = emu->op1 & (1L << (cnt - 1));
			CONDITIONAL_SET_FLAG(cc, F_CF);
		}
	}
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc64;
	return emu->res;
}

static inline uint8_t or8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d | s;
	emu->df = d_or8;
	return emu->res;
}

static inline uint16_t or16(x64emu_t *emu, uint16_t d, uint16_t s)
{
	emu->res = d | s;
	emu->df = d_or16;
	/* set the carry flag to be bit 8 */
	return emu->res;
}

static inline uint32_t or32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d | s;
	emu->df = d_or32;
	return emu->res;
}

static inline uint64_t or64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	emu->res = d | s;
	emu->df = d_or64;
	return emu->res;
}

static inline uint8_t neg8(x64emu_t *emu, uint8_t s)
{
	emu->res = (uint8_t)-s;
	emu->op1 = s;
	emu->df = d_neg8;
	return emu->res;
}

static inline uint16_t neg16(x64emu_t *emu, uint16_t s)
{
	emu->res = (uint16_t)-s;
	emu->op1 = s;
	emu->df = d_neg16;
	return emu->res;
}

static inline uint32_t neg32(x64emu_t *emu, uint32_t s)
{
	emu->res = (uint32_t)-s;
	emu->op1 = s;
	emu->df = d_neg32;
	return emu->res;
}

static inline uint64_t neg64(x64emu_t *emu, uint64_t s)
{
	emu->res = (uint64_t)-s;
	emu->op1 = s;
	emu->df = d_neg64;
	return emu->res;
}

static inline uint8_t not8(x64emu_t *emu, uint8_t s)
{
	return ~s;
}

static inline uint16_t not16(x64emu_t *emu, uint16_t s)
{
	return ~s;
}

static inline uint32_t not32(x64emu_t *emu, uint32_t s)
{
	return ~s;
}

static inline uint64_t not64(x64emu_t *emu, uint64_t s)
{
	return ~s;
}

uint8_t      rcl8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     rcl16 (x64emu_t *emu, uint16_t d, uint8_t s);
uint32_t     rcl32 (x64emu_t *emu, uint32_t d, uint8_t s);
uint64_t     rcl64 (x64emu_t *emu, uint64_t d, uint8_t s);
uint8_t      rcr8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     rcr16 (x64emu_t *emu, uint16_t d, uint8_t s);
uint32_t     rcr32 (x64emu_t *emu, uint32_t d, uint8_t s);
uint64_t     rcr64 (x64emu_t *emu, uint64_t d, uint8_t s);
uint8_t      rol8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     rol16 (x64emu_t *emu, uint16_t d, uint8_t s);
uint32_t     rol32 (x64emu_t *emu, uint32_t d, uint8_t s);
uint64_t     rol64 (x64emu_t *emu, uint64_t d, uint8_t s);
uint8_t      ror8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     ror16 (x64emu_t *emu, uint16_t d, uint8_t s);
uint32_t     ror32 (x64emu_t *emu, uint32_t d, uint8_t s);
uint64_t     ror64 (x64emu_t *emu, uint64_t d, uint8_t s);
uint16_t     shld16 (x64emu_t *emu, uint16_t d, uint16_t fill, uint8_t s);
uint32_t     shld32 (x64emu_t *emu, uint32_t d, uint32_t fill, uint8_t s);
uint64_t     shld64 (x64emu_t *emu, uint64_t d, uint64_t fill, uint8_t s);
uint16_t     shrd16 (x64emu_t *emu, uint16_t d, uint16_t fill, uint8_t s);
uint32_t     shrd32 (x64emu_t *emu, uint32_t d, uint32_t fill, uint8_t s);
uint64_t     shrd64 (x64emu_t *emu, uint64_t d, uint64_t fill, uint8_t s);
uint8_t      sbb8  (x64emu_t *emu, uint8_t d, uint8_t s);
uint16_t     sbb16 (x64emu_t *emu, uint16_t d, uint16_t s);
uint32_t     sbb32 (x64emu_t *emu, uint32_t d, uint32_t s);
uint64_t     sbb64 (x64emu_t *emu, uint64_t d, uint64_t s);

static inline uint8_t shl8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->df = d_shl8;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = d << s;

	return (uint8_t)emu->res;
}

static inline uint16_t shl16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	emu->df = d_shl16;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = d << s;
	return (uint16_t)emu->res;
}

static inline uint32_t shl32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	emu->df = d_shl32;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = d << s;

	return emu->res;
}

static inline uint64_t shl64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	emu->df = d_shl64;
	emu->op1 = d;

	s &= 0x3f;
	emu->op2 = s;
	emu->res = d << s;

	return emu->res;
}

static inline uint8_t shr8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->df = d_shr8;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = d >> s;

	return (uint8_t)emu->res;
}

static inline uint16_t shr16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	emu->df = d_shr16;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = d >> s;

	return (uint16_t)emu->res;
}

static inline uint32_t shr32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	RESET_FLAGS(emu);	// TODO: Defered this one?

	emu->df = d_shr32;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = d >> s;

    return emu->res;
}

static inline uint64_t shr64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	RESET_FLAGS(emu);	// TODO: Defered this one?

	emu->df = d_shr64;
	emu->op1 = d;

	s &= 0x3f;
	emu->op2 = s;
	emu->res = d >> s;

    return emu->res;
}

static inline uint8_t sar8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->df = d_sar8;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = (uint8_t)(((int8_t)d)>>s);

	return (uint8_t)emu->res;
}

static inline uint16_t sar16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	emu->df = d_sar16;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = (uint16_t)(((int16_t)d)>>s);

	return (uint16_t)emu->res;
}

static inline uint32_t sar32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	emu->df = d_sar32;
	emu->op1 = d;

	s &= 0x1f;
	emu->op2 = s;
	emu->res = (uint32_t)(((int32_t)d)>>s);

	return emu->res;
}

static inline uint64_t sar64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	emu->df = d_sar64;
	emu->op1 = d;

	s &= 0x3f;
	emu->op2 = s;
	emu->res = (uint64_t)(((int64_t)d)>>s);

	return emu->res;
}

static inline uint8_t sub8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub8;
	return (uint8_t)emu->res;
}

static inline uint16_t sub16(x64emu_t *emu, uint16_t d, uint16_t s)
{
    emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub16;
	return (uint16_t)emu->res;
}

static inline uint32_t sub32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub32;
	return emu->res;
}

static inline uint64_t sub64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub64;
	return emu->res;
}

void         test8  (x64emu_t *emu, uint8_t d, uint8_t s);
void         test16 (x64emu_t *emu, uint16_t d, uint16_t s);
void         test32 (x64emu_t *emu, uint32_t d, uint32_t s);
void         test64 (x64emu_t *emu, uint64_t d, uint64_t s);

static inline uint8_t xor8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor8;
	return emu->res;
}


static inline uint16_t xor16(x64emu_t *emu, uint16_t d, uint16_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor16;
	return emu->res;
}

static inline uint32_t xor32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor32;
	return emu->res;
}

static inline uint64_t xor64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor64;
	return emu->res;
}

static inline void imul8(x64emu_t *emu, uint8_t s)
{
	emu->df = d_imul8;
	R_AX = emu->res = (int16_t)(int8_t)R_AL * (int8_t)s;
}

static inline void imul16_eax(x64emu_t *emu, uint16_t s)
{
	emu->df = d_imul16;
	emu->res = (int32_t)(int16_t)R_AX * (int16_t)s;
	R_AX = (uint16_t)emu->res;
	R_DX = (uint16_t)(emu->res >> 16);
}

static inline uint16_t imul16(x64emu_t *emu, uint16_t op1, uint16_t op2)
{
	emu->df = d_imul16;
	emu->res = (int32_t)(int16_t)op1 * (int16_t)op2;
	return (uint16_t)emu->res;
}

static inline void imul32_direct(uint32_t *res_lo, uint32_t* res_hi,uint32_t d, uint32_t s)
{
	int64_t res = (int64_t)(int32_t)d * (int32_t)s;

	*res_lo = (uint32_t)res;
	*res_hi = (uint32_t)(res >> 32);
}

static inline uint32_t imul32(x64emu_t *emu, uint32_t op1, uint32_t op2)
{
	emu->df = d_imul32;
	uint32_t _res, _op1;
	imul32_direct(&_res,&_op1,op1,op2);
	emu->res = _res;
	emu->op1 = _op1;
	return emu->res;
}

static inline void imul32_eax(x64emu_t *emu, uint32_t s)
{
	emu->df = d_imul32;
	imul32_direct(&R_EAX,&R_EDX,R_EAX,s);
	emu->res = R_EAX;
	emu->op1 = R_EDX;
}

static inline void imul64_direct(uint64_t *res_lo, uint64_t* res_hi,uint64_t d, uint64_t s)
{
	__int128 res = (__int128)(int64_t)d * (int64_t)s;

	*res_lo = (uint64_t)res;
	*res_hi = (uint64_t)(res >> 64);
}

static inline uint64_t imul64(x64emu_t *emu, uint64_t op1, uint64_t op2)
{
	emu->df = d_imul64;
	imul64_direct(&emu->res,&emu->op1,op1,op2);
	return emu->res;
}

static inline void imul64_rax(x64emu_t *emu, uint64_t s)
{
	emu->df = d_imul64;
	imul64_direct(&R_RAX,&R_RDX,R_RAX,s);
	emu->res = R_RAX;
	emu->op1 = R_RDX;
}

static inline void mul8(x64emu_t *emu, uint8_t s)
{
	emu->df = d_mul8;
	R_AX = emu->res = (uint16_t)(R_AL) * s;
}

static inline void mul16(x64emu_t *emu, uint16_t s)
{
	emu->df = d_mul16;
	emu->res = (uint32_t)R_AX * s;
	R_AX = (uint16_t)emu->res;
	R_DX = (uint16_t)(emu->res >> 16);
}

static inline void mul32_eax(x64emu_t *emu, uint32_t s)
{
	emu->df = d_mul32;
	uint64_t res = (uint64_t)R_EAX * s;
	emu->res = R_EAX = (uint32_t)res;
	emu->op1 = R_EDX = (uint32_t)(res >> 32);
}

static inline void mul64_rax(x64emu_t *emu, uint64_t s)
{
	emu->df = d_mul64;
	__int128 res = (__int128)R_RAX * s;
	emu->res = R_RAX = (uint64_t)res;
	emu->op1 = R_RDX = (uint64_t)(res >> 64);
}

void         idiv8  (x64emu_t *emu, uint8_t s);
void         idiv16 (x64emu_t *emu, uint16_t s);
void         idiv32 (x64emu_t *emu, uint32_t s);
void         idiv64 (x64emu_t *emu, uint64_t s);
void         div8  (x64emu_t *emu, uint8_t s);
void         div16 (x64emu_t *emu, uint16_t s);
void         div32 (x64emu_t *emu, uint32_t s);
void         div64 (x64emu_t *emu, uint64_t s);

#endif //__X64PRIMOP_H_
