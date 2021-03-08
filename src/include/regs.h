#ifndef __REGS_H_
#define __REGS_H_

enum {
	_AX, _CX, _DX, _BX,
	_SP, _BP, _SI, _DI,
	_R8, _R9,_R10,_R11,
   _R12,_R13,_R14,_R15
};

enum {
    _CS, _DS, _SS, _ES, _FS, _GS
};


typedef union {
	int64_t  sq[1];
	uint64_t q[1];
	int32_t  sdword[2];
	uint32_t dword[2];
	int16_t  sword[4];
	uint16_t word[4];
	int8_t   sbyte[8];
	uint8_t  byte[8];
} reg64_t;

typedef struct {
    uint32_t tag;
} fpu_p_reg_t;

typedef enum {
	ROUND_Nearest = 0,		
	ROUND_Down    = 1,
	ROUND_Up      = 2,	
	ROUND_Chop    = 3
} fpu_round_t;

typedef enum {
	d_none = 0,
	d_add8,
	d_add16,
	d_add32,
	d_add64,
	d_and8,
	d_and16,
	d_and32,
	d_and64,
	d_dec8,
	d_dec16,
	d_dec32,
	d_dec64,
	d_inc8,
	d_inc16,
	d_inc32,
	d_inc64,
	d_imul8,
	d_imul16,
	d_imul32,
	d_imul64,
	d_or8,
	d_or16,
	d_or32,
	d_or64,
	d_mul8,
	d_mul16,
	d_mul32,
	d_mul64,
	d_neg8,
	d_neg16,
	d_neg32,
	d_neg64,
	d_shl8,
	d_shl16,
	d_shl32,
	d_shl64,
	d_shr8,
	d_shr16,
	d_shr32,
	d_shr64,
	d_sar8,
	d_sar16,
	d_sar32,
	d_sar64,
	d_sub8,
	d_sub16, 
	d_sub32,
	d_sub64,
	d_xor8,
	d_xor16,
	d_xor32,
	d_xor64,
	d_cmp8,
	d_cmp16,
	d_cmp32,
	d_cmp64,
	d_tst8,
	d_tst16,
	d_tst32,
	d_tst64,
	d_adc8,
	d_adc16,
	d_adc32,
	d_adc64,
	d_sbb8,
	d_sbb16, 
	d_sbb32,
	d_sbb64,
	d_rol8,
	d_rol16,
	d_rol32,
	d_rol64,
	d_ror8,
	d_ror16,
	d_ror32,
	d_ror64,
	d_rcl8,
	d_rcl16,
	d_rcl32,
	d_rcl64,
	d_rcr8,
	d_rcr16,
	d_rcr32,
	d_rcr64,
	d_unknown	//46
} defered_flags_t;

#pragma pack(push, 1)

typedef union {
	//long double ld;	// works only if 80bits!
	struct {
		uint64_t lower;
		uint16_t upper;
	} l;
} longdouble_t;

typedef struct {
	#ifdef HAVE_LD80BITS
	long double 	ld;
	#else
	longdouble_t 	ld;
	#endif
	uint64_t		ref;
} fpu_ld_t;

typedef struct {
	int64_t			ll;
	int64_t			ref;
} fpu_ll_t;

typedef union {
    struct __attribute__ ((__packed__)) {
        unsigned int _F_CF:1;
		unsigned int _F_res1:1;
        unsigned int _F_PF:1;
		unsigned int _F_res2:1;
        unsigned int _F_AF:1;
		unsigned int _F_res3:1;
        unsigned int _F_ZF:1;
        unsigned int _F_SF:1;
        unsigned int _F_TF:1;
        unsigned int _F_IF:1;
        unsigned int _F_DF:1;
        unsigned int _F_OF:1;
        unsigned int _F_IOPL:2;
        unsigned int _F_NT:1;
        unsigned int _F_dummy:1;
        unsigned int _F_RF:1;
        unsigned int _F_VM:1;
        unsigned int _F_AC:1;
        unsigned int _F_VIF:1; 
        unsigned int _F_VIP:1;
        unsigned int _F_ID:1;
    } f;
    uint32_t    x32;
	uint64_t 	x64;
} x64flags_t;

typedef enum {
    F_CF = 0,
	F_res1,
    F_PF,
	F_res2,
    F_AF,
	F_res3,
    F_ZF,
    F_SF,
    F_TF,
    F_IF,
    F_DF,
    F_OF,
    F_IOPL, // double
    F_NT = 14,
    F_dummy,
    F_RF,
    F_VM,
    F_AC,
    F_VIF,
    F_VIP,
    F_ID,
	F_LAST
} flags_names_t;


typedef union {
    struct __attribute__ ((__packed__)) {
        unsigned int F87_IE:1;
        unsigned int F87_DE:1;
        unsigned int F87_ZE:1;
        unsigned int F87_OE:1;
        unsigned int F87_UE:1;
        unsigned int F87_PE:1;
        unsigned int F87_SF:1;
        unsigned int F87_ES:1;
        unsigned int F87_C0:1;
		unsigned int F87_C1:1;
		unsigned int F87_C2:1;
		unsigned int F87_TOP:3;
		unsigned int F87_C3:1;
		unsigned int F87_B:1;
    } f;
    uint16_t    x16;
} x87flags_t;

typedef union {
	uint64_t	q;
	int64_t		sq;
	double		d;
	float		f[2];
	uint32_t	ud[2];
	int32_t 	sd[2];
	uint16_t 	uw[4];
	int16_t 	sw[4];
	uint8_t 	ub[8];
	int8_t 		sb[8];
} mmx87_regs_t;

typedef union {
	uint64_t q[2];
	int64_t sq[2];
	double d[2];
	float f[4];
	uint32_t ud[4];
	int32_t sd[4];
	uint16_t uw[8];
	int16_t sw[8];
	uint8_t ub[16];
	int8_t sb[16];
} sse_regs_t;
#pragma pack(pop)

#define R_RIP emu->ip.q[0]
#define R_RAX emu->regs[_AX].q[0]
#define R_RBX emu->regs[_BX].q[0]
#define R_RCX emu->regs[_CX].q[0]
#define R_RDX emu->regs[_DX].q[0]
#define R_RDI emu->regs[_DI].q[0]
#define R_RSI emu->regs[_SI].q[0]
#define R_RSP emu->regs[_SP].q[0]
#define R_RBP emu->regs[_BP].q[0]
#define R_R8 emu->regs[_R8].q[0]
#define R_R9 emu->regs[_R9].q[0]
#define R_R10 emu->regs[_R10].q[0]
#define R_R11 emu->regs[_R11].q[0]
#define R_R12 emu->regs[_R12].q[0]
#define R_R13 emu->regs[_R13].q[0]
#define R_R14 emu->regs[_R14].q[0]
#define R_R15 emu->regs[_R15].q[0]
#define R_EAX emu->regs[_AX].dword[0]
#define R_EBX emu->regs[_BX].dword[0]
#define R_ECX emu->regs[_CX].dword[0]
#define R_EDX emu->regs[_DX].dword[0]
#define R_EDI emu->regs[_DI].dword[0]
#define R_ESI emu->regs[_SI].dword[0]
#define R_ESP emu->regs[_SP].dword[0]
#define R_EBP emu->regs[_BP].dword[0]
#define R_AX emu->regs[_AX].word[0]
#define R_BX emu->regs[_BX].word[0]
#define R_CX emu->regs[_CX].word[0]
#define R_DX emu->regs[_DX].word[0]
#define R_DI emu->regs[_DI].word[0]
#define R_SI emu->regs[_SI].word[0]
#define R_SP emu->regs[_SP].word[0]
#define R_BP emu->regs[_BP].word[0]
#define R_AL emu->regs[_AX].byte[0]
#define R_AH emu->regs[_AX].byte[1]
#define R_CX emu->regs[_CX].word[0]
#define R_CL emu->regs[_CX].byte[0]
#define R_CS emu->segs[_CS]
#define R_DS emu->segs[_DS]
#define R_SS emu->segs[_SS]
#define R_ES emu->segs[_ES]
#define R_FS emu->segs[_FS]
#define R_GS emu->segs[_GS]

#define ACCESS_FLAG(F)  emu->eflags.f._##F
#define SET_FLAG(F)     emu->eflags.f._##F = 1
#define CLEAR_FLAG(F)   emu->eflags.f._##F = 0
#define CONDITIONAL_SET_FLAG(COND, F)   emu->eflags.f._##F = (COND)?1:0

#endif //__REGS_H_
