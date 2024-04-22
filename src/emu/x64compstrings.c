#include <stdint.h>

#include "box64stack.h"
#include "x64emu.h"
#include "x64run_private.h"
#include "x64emu_private.h"
#include "x64compstrings.h"

static int overrideIfDataInvalid(sse_regs_t* mem, int lmem, sse_regs_t* reg, int lreg, int j, int i, int imm8)
{
    int valid1 = (i<lreg);
    int valid2 = (j<lmem);
    if(!valid1 && !valid2)
        switch((imm8>>2)&3) {
            case 0b00:
            case 0b01:  return 0;
            case 0b10:
            case 0b11:  return 1;
        }
    if(!valid1 && valid2)
        switch((imm8>>2)&3) {
            case 0b00:
            case 0b01:
            case 0b10:  return 0;
            case 0b11:  return 1;
        }
    if(valid1 && !valid2)
        return 0;
    switch((imm8>>2)&3) {
        case 0b01:  // range
            switch (imm8&3) {
                case 0b00:  // ub
                    return (i&1)?((reg->ub[i]>=mem->ub[j])):((reg->ub[i]<=mem->ub[j]));
                case 0b01:  // uw
                    return (i&1)?((reg->uw[i]>=mem->uw[j])):((reg->uw[i]<=mem->uw[j]));
                case 0b10:  // sb
                    return (i&1)?((reg->sb[i]>=mem->sb[j])):((reg->sb[i]<=mem->sb[j]));
                default:    // sw
                    return (i&1)?((reg->sw[i]>=mem->sw[j])):((reg->sw[i]<=mem->sw[j]));
            }
            break;
        default:    // the others
            switch (imm8&1) {
                case 0:  // byte
                    return (reg->ub[i] == mem->ub[j]);
                default: // word
                    return (reg->uw[i] == mem->uw[j]);
            }
    }
}

uint32_t sse42_compare_string_explicit_len(x64emu_t* emu, sse_regs_t* mem, int lmem, sse_regs_t* reg, int lreg, uint8_t imm8)
{
    // get number of packed byte/word
    int n_packed = (imm8&1)?8:16;
    if(lreg<0) lreg = -lreg;
    if(lmem<0) lmem = -lmem;
    if(lreg>n_packed) lreg = n_packed;
    if(lmem>n_packed) lmem = n_packed;
    // aggregate to intres1
    uint32_t intres1 = 0;
    switch((imm8>>2)&3) {
        case 0b00:  //Equal any
            for(int j=0; j<n_packed; ++j)
                for(int i=0; i<n_packed; ++i) {
                    intres1 |= overrideIfDataInvalid(mem, lmem, reg, lreg, j, i, imm8)<<j;
                }
            break;
        case 0b01:  // Range
            for(int j=0; j<n_packed; ++j)
                for(int i=0; i<n_packed; i+=2) {
                    intres1 |= (overrideIfDataInvalid(mem, lmem, reg, lreg, j, i, imm8) & overrideIfDataInvalid(mem, lmem, reg, lreg, j, i+1, imm8))<<j;
                }
            break;
        case 0b10:  // Equal each
            for(int i=0; i<n_packed; ++i) {
                intres1 |= overrideIfDataInvalid(mem, lmem, reg, lreg, i, i, imm8)<<i;
            }
            break;
        case 0b11:  // Equal ordered
            intres1 = (1<<n_packed)-1;
            for(int j=0; j<n_packed; ++j)
                for(int i=0; i<n_packed-j; ++i) {
                    #if 1
                    if(!overrideIfDataInvalid(mem, lmem, reg, lreg, i+j, i, imm8)) {
                        intres1 &= ~(1<<j);
                        break;
                    }
                    #else
                    intres1 &= (((1<<n_packed)-1)^(1<<j)) | (overrideIfDataInvalid(mem, lmem, reg, lreg, i+j, i, imm8)<<j);
                    #endif
                }
            break;
    }
    // build intres2
    uint32_t intres2 = intres1;
    switch((imm8>>4)&3) {
        case 0b01: intres2 ^= ((1<<n_packed)-1); break;
        case 0b11: intres2 ^= ((1<<lmem)-1); break;
    }
    // and now set the flags
    RESET_FLAGS(emu);
    CONDITIONAL_SET_FLAG(intres2, F_CF);
    CONDITIONAL_SET_FLAG(lmem<n_packed, F_ZF);
    CONDITIONAL_SET_FLAG(lreg<n_packed, F_SF);
    CONDITIONAL_SET_FLAG(intres2&1, F_OF);
    CONDITIONAL_SET_FLAG(((!intres2) && (lmem==n_packed)), F_AF);
    CLEAR_FLAG(F_PF);
    
    return intres2;
}

uint32_t sse42_compare_string_implicit_len(x64emu_t* emu, sse_regs_t* mem, sse_regs_t* reg, uint8_t imm8)
{
    int lmem = 0;
    int lreg = 0;
    // get lmem and lreg
    if(imm8&1) {
        while(lmem<8 && mem->uw[lmem]) ++lmem;
        while(lreg<8 && reg->uw[lreg]) ++lreg;
    } else {
        while(lmem<16 && mem->ub[lmem]) ++lmem;
        while(lreg<16 && reg->ub[lreg]) ++lreg;
    }
    return sse42_compare_string_explicit_len(emu, mem, lmem, reg, lreg, imm8);
}
