#ifndef __DYNAREC_RV64_HELPER_H__
#define __DYNAREC_RV64_HELPER_H__

// undef to get Close to SSE Float->int conversions
//#define PRECISE_CVT

#if STEP == 0
#include "dynarec_rv64_pass0.h"
#elif STEP == 1
#include "dynarec_rv64_pass1.h"
#elif STEP == 2
#include "dynarec_rv64_pass2.h"
#elif STEP == 3
#include "dynarec_rv64_pass3.h"
#endif

#include "debug.h"
#include "rv64_emitter.h"
#include "../emu/x64primop.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F16S    *(int16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define F32S64  (uint64_t)(int64_t)F32S
#define F64     *(uint64_t*)(addr+=8, addr-8)
#define PK(a)   *(uint8_t*)(addr+a)
#define PK16(a)   *(uint16_t*)(addr+a)
#define PK32(a)   *(uint32_t*)(addr+a)
#define PK64(a)   *(uint64_t*)(addr+a)
#define PKip(a)   *(uint8_t*)(ip+a)


// Strong mem emulation helpers
// Sequence of Read will trigger a DMB on "first" read if strongmem is 2
// Squence of Write will trigger a DMB on "last" write if strongmem is 1
// Opcode will read
#define SMREAD()    if(!dyn->smread && box64_dynarec_strongmem>1) {SMDMB();}
// Opcode will read with option forced lock
#define SMREADLOCK(lock)    if(lock || (!dyn->smread && box64_dynarec_strongmem>1)) {SMDMB();}
// Opcode migh read (depend on nextop)
#define SMMIGHTREAD()   if(!MODREG) {SMREAD();}
// Opcode has wrote
#define SMWRITE()   dyn->smwrite=1
// Opcode has wrote (strongmem>1 only)
#define SMWRITE2()   if(box64_dynarec_strongmem>1) dyn->smwrite=1
// Opcode has wrote with option forced lock
#define SMWRITELOCK(lock)   if(lock) {SMDMB();} else dyn->smwrite=1
// Opcode migh have wrote (depend on nextop)
#define SMMIGHTWRITE()   if(!MODREG) {SMWRITE();}
// Start of sequence
#define SMSTART()   SMEND()
// End of sequence
#define SMEND()     if(dyn->smwrite && box64_dynarec_strongmem) {FENCE();} dyn->smwrite=0; dyn->smread=0;
// Force a Data memory barrier (for LOCK: prefix)
#define SMDMB()     FENCE(); dyn->smwrite=0; dyn->smread=1

//LOCK_* define
#define LOCK_LOCK   (int*)1

// GETGD    get x64 register in gd
#define GETGD   gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3)
// GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)  if(MODREG) {                          \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    SMREAD()                            \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
                    LD(x1, wback, fixedaddress);    \
                    ed = x1;                            \
                }

// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {SDxw(ed, wback, fixedaddress); SMWRITE();}

#define IFX(A)  if((dyn->insts[ninst].x64.gen_flags&(A)))
#define IFX_PENDOR0  if((dyn->insts[ninst].x64.gen_flags&(X_PEND) || !dyn->insts[ninst].x64.gen_flags))
#define IFXX(A) if((dyn->insts[ninst].x64.gen_flags==(A)))
#define IFX2X(A, B) if((dyn->insts[ninst].x64.gen_flags==(A) || dyn->insts[ninst].x64.gen_flags==(B) || dyn->insts[ninst].x64.gen_flags==((A)|(B))))
#define IFXN(A, B)  if((dyn->insts[ninst].x64.gen_flags&(A) && !(dyn->insts[ninst].x64.gen_flags&(B))))

#define SET_DFNONE(S)    if(!dyn->f.dfnone) {MOV_U12(S, d_none); SD(S, xEmu, offsetof(x64emu_t, df)); dyn->f.dfnone=1;}
#define SET_DF(S, N)     if((N)!=d_none) {MOV_U12(S, (N)); SD(S, xEmu, offsetof(x64emu_t, df)); dyn->f.dfnone=0;} else SET_DFNONE(S)
#define SET_NODF()          dyn->f.dfnone = 0
#define SET_DFOK()          dyn->f.dfnone = 1

#define CLEAR_FLAGS() IFX(X_ALL) {ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_CF) | (1UL<<F_OF) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));}

#ifndef MAYSETFLAGS
#define MAYSETFLAGS()
#endif

#ifndef READFLAGS
#define READFLAGS(A) \

#endif

#ifndef SETFLAGS
#define SETFLAGS(A, B)                                                                          \

#endif
#ifndef JUMP
#define JUMP(A, C)
#endif
#ifndef BARRIER
#define BARRIER(A)
#endif
#ifndef BARRIER_NEXT
#define BARRIER_NEXT(A)
#endif

#ifndef DEFAULT
#define DEFAULT      *ok = -1; BARRIER(2)
#endif

#ifndef TABLE64
#define TABLE64(A, V)
#endif

#if STEP < 2
#define GETIP(A)
#define GETIP_(A)
#else
// put value in the Table64 even if not using it for now to avoid difference between Step2 and Step3. Needs to be optimized later...
#define GETIP(A)                                        \
    if(dyn->last_ip && ((A)-dyn->last_ip)<0x1000) {     \
        uint64_t _delta_ip = (A)-dyn->last_ip;          \
        dyn->last_ip += _delta_ip;                      \
        if(_delta_ip) {                                 \
            ADDI(xRIP, xRIP, _delta_ip);                \
        }                                               \
    } else {                                            \
        dyn->last_ip = (A);                             \
        if(dyn->last_ip<0xffffffff) {                   \
            MOV64x(xRIP, dyn->last_ip);                 \
        } else                                          \
            TABLE64(xRIP, dyn->last_ip);                \
    }
#define GETIP_(A)                                       \
    if(dyn->last_ip && ((A)-dyn->last_ip)<0x1000) {     \
        uint64_t _delta_ip = (A)-dyn->last_ip;          \
        if(_delta_ip) {ADDI(xRIP, xRIP, _delta_ip);}    \
    } else {                                            \
        if((A)<0xffffffff) {                            \
            MOV64x(xRIP, (A));                          \
        } else                                          \
            TABLE64(xRIP, (A));                         \
    }
#endif
#define CLEARIP()   dyn->last_ip=0


#define MODREG  ((nextop&0xC0)==0xC0)

void rv64_epilog();
void* rv64_next(x64emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define native_pass        STEPNAME(native_pass)

#define dynarec64_00       STEPNAME(dynarec64_00)
#define dynarec64_0F       STEPNAME(dynarec64_0F)
#define dynarec64_64       STEPNAME(dynarec64_64)
#define dynarec64_65       STEPNAME(dynarec64_65)
#define dynarec64_66       STEPNAME(dynarec64_66)
#define dynarec64_67       STEPNAME(dynarec64_67)
#define dynarec64_D8       STEPNAME(dynarec64_D8)
#define dynarec64_D9       STEPNAME(dynarec64_D9)
#define dynarec64_DA       STEPNAME(dynarec64_DA)
#define dynarec64_DB       STEPNAME(dynarec64_DB)
#define dynarec64_DC       STEPNAME(dynarec64_DC)
#define dynarec64_DD       STEPNAME(dynarec64_DD)
#define dynarec64_DE       STEPNAME(dynarec64_DE)
#define dynarec64_DF       STEPNAME(dynarec64_DF)
#define dynarec64_F0       STEPNAME(dynarec64_F0)
#define dynarec64_660F     STEPNAME(dynarec64_660F)
#define dynarec64_6664     STEPNAME(dynarec64_6664)
#define dynarec64_66F0     STEPNAME(dynarec64_66F0)
#define dynarec64_F20F     STEPNAME(dynarec64_F20F)
#define dynarec64_F30F     STEPNAME(dynarec64_F30F)

#define geted           STEPNAME(geted)
#define geted32         STEPNAME(geted32)
#define geted16         STEPNAME(geted16)
#define jump_to_epilog  STEPNAME(jump_to_epilog)
#define jump_to_next    STEPNAME(jump_to_next)
#define ret_to_epilog   STEPNAME(ret_to_epilog)
#define retn_to_epilog  STEPNAME(retn_to_epilog)
#define iret_to_epilog  STEPNAME(iret_to_epilog)
#define call_c          STEPNAME(call_c)
#define call_n          STEPNAME(call_n)
#define grab_segdata    STEPNAME(grab_segdata)
#define emit_cmp8       STEPNAME(emit_cmp8)
#define emit_cmp16      STEPNAME(emit_cmp16)
#define emit_cmp32      STEPNAME(emit_cmp32)
#define emit_cmp8_0     STEPNAME(emit_cmp8_0)
#define emit_cmp16_0    STEPNAME(emit_cmp16_0)
#define emit_cmp32_0    STEPNAME(emit_cmp32_0)
#define emit_test8      STEPNAME(emit_test8)
#define emit_test16     STEPNAME(emit_test16)
#define emit_test32     STEPNAME(emit_test32)
#define emit_add32      STEPNAME(emit_add32)
#define emit_add32c     STEPNAME(emit_add32c)
#define emit_add8       STEPNAME(emit_add8)
#define emit_add8c      STEPNAME(emit_add8c)
#define emit_sub32      STEPNAME(emit_sub32)
#define emit_sub32c     STEPNAME(emit_sub32c)
#define emit_sub8       STEPNAME(emit_sub8)
#define emit_sub8c      STEPNAME(emit_sub8c)
#define emit_or32       STEPNAME(emit_or32)
#define emit_or32c      STEPNAME(emit_or32c)
#define emit_xor32      STEPNAME(emit_xor32)
#define emit_xor32c     STEPNAME(emit_xor32c)
#define emit_and32      STEPNAME(emit_and32)
#define emit_and32c     STEPNAME(emit_and32c)
#define emit_or8        STEPNAME(emit_or8)
#define emit_or8c       STEPNAME(emit_or8c)
#define emit_xor8       STEPNAME(emit_xor8)
#define emit_xor8c      STEPNAME(emit_xor8c)
#define emit_and8       STEPNAME(emit_and8)
#define emit_and8c      STEPNAME(emit_and8c)
#define emit_add16      STEPNAME(emit_add16)
#define emit_add16c     STEPNAME(emit_add16c)
#define emit_sub16      STEPNAME(emit_sub16)
#define emit_sub16c     STEPNAME(emit_sub16c)
#define emit_or16       STEPNAME(emit_or16)
#define emit_or16c      STEPNAME(emit_or16c)
#define emit_xor16      STEPNAME(emit_xor16)
#define emit_xor16c     STEPNAME(emit_xor16c)
#define emit_and16      STEPNAME(emit_and16)
#define emit_and16c     STEPNAME(emit_and16c)
#define emit_inc32      STEPNAME(emit_inc32)
#define emit_inc16      STEPNAME(emit_inc16)
#define emit_inc8       STEPNAME(emit_inc8)
#define emit_dec32      STEPNAME(emit_dec32)
#define emit_dec16      STEPNAME(emit_dec16)
#define emit_dec8       STEPNAME(emit_dec8)
#define emit_adc32      STEPNAME(emit_adc32)
#define emit_adc32c     STEPNAME(emit_adc32c)
#define emit_adc8       STEPNAME(emit_adc8)
#define emit_adc8c      STEPNAME(emit_adc8c)
#define emit_adc16      STEPNAME(emit_adc16)
#define emit_adc16c     STEPNAME(emit_adc16c)
#define emit_sbb32      STEPNAME(emit_sbb32)
#define emit_sbb32c     STEPNAME(emit_sbb32c)
#define emit_sbb8       STEPNAME(emit_sbb8)
#define emit_sbb8c      STEPNAME(emit_sbb8c)
#define emit_sbb16      STEPNAME(emit_sbb16)
#define emit_sbb16c     STEPNAME(emit_sbb16c)
#define emit_neg32      STEPNAME(emit_neg32)
#define emit_neg16      STEPNAME(emit_neg16)
#define emit_neg8       STEPNAME(emit_neg8)
#define emit_shl32      STEPNAME(emit_shl32)
#define emit_shl32c     STEPNAME(emit_shl32c)
#define emit_shr32      STEPNAME(emit_shr32)
#define emit_shr32c     STEPNAME(emit_shr32c)
#define emit_sar32c     STEPNAME(emit_sar32c)
#define emit_rol32c     STEPNAME(emit_rol32c)
#define emit_ror32c     STEPNAME(emit_ror32c)
#define emit_shrd32c    STEPNAME(emit_shrd32c)
#define emit_shld32c    STEPNAME(emit_shld32c)

#define emit_pf         STEPNAME(emit_pf)

#define x87_do_push     STEPNAME(x87_do_push)
#define x87_do_push_empty STEPNAME(x87_do_push_empty)
#define x87_do_pop      STEPNAME(x87_do_pop)
#define x87_get_current_cache   STEPNAME(x87_get_current_cache)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_neoncache STEPNAME(x87_get_neoncache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_get_st_empty  STEPNAME(x87_get_st)
#define x87_refresh     STEPNAME(x87_refresh)
#define x87_forget      STEPNAME(x87_forget)
#define x87_reget_st    STEPNAME(x87_reget_st)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_swapreg     STEPNAME(x87_swapreg)
#define x87_setround    STEPNAME(x87_setround)
#define x87_restoreround STEPNAME(x87_restoreround)
#define sse_setround    STEPNAME(sse_setround)
#define mmx_get_reg     STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_get_reg     STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)
#define sse_forget_reg   STEPNAME(sse_forget_reg)
#define sse_purge07cache STEPNAME(sse_purge07cache)

#define fpu_pushcache   STEPNAME(fpu_pushcache)
#define fpu_popcache    STEPNAME(fpu_popcache)
#define fpu_reset       STEPNAME(fpu_reset)
#define fpu_reset_cache STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache  STEPNAME(fpu_purgecache)
#define mmx_purgecache  STEPNAME(mmx_purgecache)
#define x87_purgecache  STEPNAME(x87_purgecache)
#ifdef HAVE_TRACE
#define fpu_reflectcache STEPNAME(fpu_reflectcache)
#endif

#define CacheTransform       STEPNAME(CacheTransform)
#define rv64_move64     STEPNAME(rv64_move64)
#define rv64_move32     STEPNAME(rv64_move32)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
//uintptr_t geted32(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, rex_t rex, int* l, int s, int delta);

/* setup r2 to address pointed by */
//uintptr_t geted16(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, int s);


// generic x64 helper
void jump_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
//void ret_to_epilog(dynarec_rv64_t* dyn, int ninst);
//void retn_to_epilog(dynarec_rv64_t* dyn, int ninst, int n);
//void iret_to_epilog(dynarec_rv64_t* dyn, int ninst, int is64bits);
//void call_c(dynarec_rv64_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int save_reg);
//void call_n(dynarec_rv64_t* dyn, int ninst, void* fnc, int w);
//void grab_segdata(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, int reg, int segment);
//void emit_cmp8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
//void emit_cmp16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
//void emit_cmp32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
//void emit_cmp8_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_cmp16_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_cmp32_0(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
//void emit_test8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
//void emit_test16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
//void emit_test32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_add32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_add32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
//void emit_add8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_add8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_sub32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
//void emit_sub8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sub8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
//void emit_or32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_or32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
//void emit_xor32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_xor32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
//void emit_and32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_and32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
//void emit_or8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_or8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_xor8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_xor8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_and8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_and8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_add16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_add16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_sub16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sub16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_or16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_or16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_xor16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_xor16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_and16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_and16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_inc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
//void emit_inc16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_inc8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_dec32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
//void emit_dec16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_dec8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_adc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_adc32c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_adc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_adc8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
//void emit_adc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_adc16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_sbb32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_sbb32c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_sbb8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sbb8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
//void emit_sbb16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sbb16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_neg32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
//void emit_neg16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_neg8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_shl32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_shl32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_shr32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_shr32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_sar32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_rol32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_ror32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_shrd32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
//void emit_shld32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);

void emit_pf(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);

// x87 helper
// cache of the local stack counter, to avoid upadte at every call
//void x87_stackcount(dynarec_rv64_t* dyn, int ninst, int scratch);
// fpu push. Return the Dd value to be used
//int x87_do_push(dynarec_rv64_t* dyn, int ninst, int s1, int t);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
//void x87_do_push_empty(dynarec_rv64_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
//void x87_do_pop(dynarec_rv64_t* dyn, int ninst, int s1);
// get cache index for a x87 reg, return -1 if cache doesn't exist
//int x87_get_current_cache(dynarec_rv64_t* dyn, int ninst, int st, int t);
// get cache index for a x87 reg, create the entry if needed
//int x87_get_cache(dynarec_rv64_t* dyn, int ninst, int populate, int s1, int s2, int a, int t);
// get neoncache index for a x87 reg
//int x87_get_neoncache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
//int x87_get_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int t);
// get vfpu register for a x87 reg, create the entry if needed. Do not fetch the Stx if not already in cache
//int x87_get_st_empty(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int t);
// refresh a value from the cache ->emu (nothing done if value is not cached)
//void x87_refresh(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
//void x87_forget(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
//void x87_reget_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// swap 2 x87 regs
//void x87_swapreg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to cw flags, return reg to restore flags
//int x87_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
//void x87_restoreround(dynarec_rv64_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
//int sse_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);

//void CacheTransform(dynarec_rv64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val);
void rv64_move32(dynarec_rv64_t* dyn, int ninst, int reg, int32_t val);

#if STEP < 2
#define CHECK_CACHE()   0
#else
#define CHECK_CACHE()   (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif

// common coproc helpers
// reset the cache
void fpu_reset(dynarec_rv64_t* dyn);
// reset the cache with n
void fpu_reset_cache(dynarec_rv64_t* dyn, int ninst, int reset_n);
// propagate stack state
void fpu_propagate_stack(dynarec_rv64_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3);
// purge MMX cache
void mmx_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1);
// purge x87 cache
void x87_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3);
#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);
#endif
void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07);

uintptr_t dynarec64_00(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_64(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
//uintptr_t dynarec64_65(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep,int* ok, int* need_epilog);
//uintptr_t dynarec64_66(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_67(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_D8(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_D9(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DA(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DB(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DC(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DD(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DE(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DF(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_660F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_6664(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
//uintptr_t dynarec64_66F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
//uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);

#if STEP < 2
#define PASS2(A)
#else
#define PASS2(A)   A
#endif

#if STEP < 3
#define PASS3(A)
#else
#define PASS3(A)   A
#endif

#if STEP < 3
#define MAYUSE(A)   (void)A
#else
#define MAYUSE(A)
#endif

#endif //__DYNAREC_RV64_HELPER_H__
