// Based on Realmode X86 Emulator Library, and enhanced to handle 64bits
/****************************************************************************
*
* Realmode X86 Emulator Library
*
* Copyright (c) 1996-1999 SciTech Software, Inc.
* Copyright (c) David Mosberger-Tang
* Copyright (c) 1999 Egbert Eich
* Copyright (c) 2007-2017 SUSE LINUX GmbH; Author: Steffen Winterfeldt
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.  The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Description:
*   Implement the primitive machine operations used by the emulation code
*   in ops.c
*
*   Carry Chain Calculation
*
*   This represents a somewhat expensive calculation which is
*   apparently required to emulate the setting of the OF and AF flag.
*   The latter is not so important, but the former is.  The overflow
*   flag is the XOR of the top two bits of the carry chain for an
*   addition (similar for subtraction).  Since we do not want to
*   simulate the addition in a bitwise manner, we try to calculate the
*   carry chain given the two operands and the result.
*
*   So, given the following table, which represents the addition of two
*   bits, we can derive a formula for the carry chain.
*
*   a   b   cin   r     cout
*   0   0   0     0     0
*   0   0   1     1     0
*   0   1   0     1     0
*   0   1   1     0     1
*   1   0   0     1     0
*   1   0   1     0     1
*   1   1   0     0     1
*   1   1   1     1     1
*
*   Construction of table for cout:
*
*   ab
*   r  \  00   01   11  10
*   |------------------
*   0  |   0    1    1   1
*   1  |   0    0    1   0
*
*   By inspection, one gets:  cc = ab +  r'(a + b)
*
*   That represents alot of operations, but NO CHOICE....
*
*   Borrow Chain Calculation.
*
*   The following table represents the subtraction of two bits, from
*   which we can derive a formula for the borrow chain.
*
*   a   b   bin   r     bout
*   0   0   0     0     0
*   0   0   1     1     1
*   0   1   0     1     1
*   0   1   1     0     1
*   1   0   0     1     0
*   1   0   1     0     0
*   1   1   0     0     0
*   1   1   1     1     1
*
*   Construction of table for cout:
*
*   ab
*   r  \  00   01   11  10
*   |------------------
*   0  |   0    1    0   0
*   1  |   1    1    1   0
*
*   By inspection, one gets:  bc = a'b +  r(a' + b)
*
****************************************************************************/

// This has been heavily modified to fit box64 purpose...
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "x64emu_private.h"
#include "x64run_private.h"
#include "env.h"

extern box64env_t box64env;

/*------------------------- Global Variables ------------------------------*/

#define PARITY(x)   (((emu->x64emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)
#define XOR2(x) 	(((x) ^ ((x)>>1)) & 0x1)

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Implements the AAA instruction and side effects.
****************************************************************************/
uint16_t aaa16(x64emu_t *emu, uint16_t d)
{
	uint16_t	res;
	CHECK_FLAGS(emu);
	if ((d & 0xf) > 0x9 || ACCESS_FLAG(F_AF)) {
		d += 0x6;
		d += 0x100;
		SET_FLAG(F_AF);
		SET_FLAG(F_CF);
	} else {
		CLEAR_FLAG(F_CF);
		CLEAR_FLAG(F_AF);
	}
	res = (uint16_t)(d & 0xFF0F);
	CLEAR_FLAG(F_SF);
	CONDITIONAL_SET_FLAG(res == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	return res;
}

/****************************************************************************
REMARKS:
Implements the AAS instruction and side effects.
****************************************************************************/
uint16_t aas16(x64emu_t *emu, uint16_t d)
{
	uint16_t	res;
	CHECK_FLAGS(emu);
	if ((d & 0xf) > 0x9 || ACCESS_FLAG(F_AF)) {
		d -= 0x6;
		d -= 0x100;
		SET_FLAG(F_AF);
		SET_FLAG(F_CF);
	} else {
		CLEAR_FLAG(F_CF);
		CLEAR_FLAG(F_AF);
	}
	res = (uint16_t)(d & 0xFF0F);
	CLEAR_FLAG(F_SF);
	CONDITIONAL_SET_FLAG(res == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	return res;
}

/****************************************************************************
REMARKS:
Implements the AAD instruction and side effects.
****************************************************************************/
uint16_t aad16(x64emu_t *emu, uint16_t d, uint8_t base)
{
	uint16_t l;
	uint8_t hb, lb;

	RESET_FLAGS(emu);

	hb = (uint8_t)((d >> 8) & 0xff);
	lb = (uint8_t)((d & 0xff));
	l = (uint16_t)((lb + base * hb) & 0xFF);

	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_AF);
	CLEAR_FLAG(F_OF);
	CONDITIONAL_SET_FLAG(l & 0x80, F_SF);
	CONDITIONAL_SET_FLAG((l&0xff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(l & 0xff), F_PF);
	return l;
}

/****************************************************************************
REMARKS:
Implements the AAM instruction and side effects.
****************************************************************************/
uint16_t aam16(x64emu_t *emu, uint8_t d, uint8_t base)
{
    uint16_t h, l;

	RESET_FLAGS(emu);

	h = (uint16_t)(d / base);
	l = (uint16_t)(d % base);
	l |= (uint16_t)(h << 8);

	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_AF);
	CLEAR_FLAG(F_OF);
	CONDITIONAL_SET_FLAG(l & 0x80, F_SF);
	CONDITIONAL_SET_FLAG((l&0xff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(l & 0xff), F_PF);
    return l;
}

/****************************************************************************
REMARKS:
Implements the ADC instruction and side effects.
****************************************************************************/
uint8_t adc8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	uint32_t res;   /* all operands in native machine order */
	uint32_t cc;

	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
		res = 1 + d + s;
	else
		res = d + s;

	CONDITIONAL_SET_FLAG(res & 0x100, F_CF);
	CONDITIONAL_SET_FLAG((res & 0xff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the carry chain  SEE NOTE AT TOP. */
	cc = (s & d) | ((~res) & (s | d));
	CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
	CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
	return (uint8_t)res;
}

uint16_t adc16(x64emu_t *emu, uint16_t d, uint16_t s)
{
	uint32_t res;   /* all operands in native machine order */
	uint32_t cc;

	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
		res = 1 + d + s;
	else
		res = d + s;

	CONDITIONAL_SET_FLAG(res & 0x10000, F_CF);
	CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the carry chain  SEE NOTE AT TOP. */
	cc = (s & d) | ((~res) & (s | d));
	CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
	CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
	return (uint16_t)res;
}

uint32_t adc32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	uint64_t res;   /* all operands in native machine order */
	uint64_t cc;

	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
		res = 1LL + d + s;
	else
		res = (uint64_t)d + s;

	CONDITIONAL_SET_FLAG(res & 0x100000000LL, F_CF);
	CONDITIONAL_SET_FLAG((res & 0xffffffff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the carry chain  SEE NOTE AT TOP. */
	cc = (s & d) | ((~res) & (s | d));
	CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
	CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
	return (uint32_t)res;

}

uint64_t adc64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	uint64_t lo;	/* all operands in native machine order */
	uint64_t hi;
	uint64_t res;
	uint64_t cc;

	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF)) {
		lo = 1 + (d & 0xFFFFFFFF) + (s & 0xFFFFFFFF);
		res = 1 + d + s;
		}
	else {
		lo = (d & 0xFFFFFFFF) + (s & 0xFFFFFFFF);
		res = d + s;
		}
	hi = (lo >> 32) + (d >> 32) + (s >> 32);

	CONDITIONAL_SET_FLAG(hi & 0x100000000L, F_CF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(res & 0x8000000000000000LL, F_SF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the carry chain  SEE NOTE AT TOP. */
	cc = (s & d) | ((~res) & (s | d));
	CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
	CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
	return res;
}

/****************************************************************************
REMARKS:
Implements the CMP instruction and side effects.
****************************************************************************/
uint8_t cmp8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	uint32_t res;   /* all operands in native machine order */
	uint32_t bc;

	RESET_FLAGS(emu);

	res = d - s;
	CLEAR_FLAG(F_CF);
	CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
	CONDITIONAL_SET_FLAG((res & 0xff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return d;
}

uint16_t cmp16(x64emu_t *emu, uint16_t d, uint16_t s)
{
	uint32_t res;   /* all operands in native machine order */
	uint32_t bc;

	RESET_FLAGS(emu);

	res = d - s;
	CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
	CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return d;
}

uint32_t cmp32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	uint32_t res;   /* all operands in native machine order */
	uint32_t bc;

	RESET_FLAGS(emu);

	res = d - s;
	CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return d;
}

uint64_t cmp64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	uint64_t res;   /* all operands in native machine order */
	uint64_t bc;

	RESET_FLAGS(emu);

	res = d - s;
	CONDITIONAL_SET_FLAG(res & 0x8000000000000000LL, F_SF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return d;
}

/****************************************************************************
REMARKS:
Implements the DAA instruction and side effects.
****************************************************************************/
uint8_t daa8(x64emu_t *emu, uint8_t d)
{
	uint32_t res = d;
	CHECK_FLAGS(emu);
	int cf = ACCESS_FLAG(F_CF);
	CLEAR_FLAG(F_CF);
	if ((d & 0xf) > 9 || ACCESS_FLAG(F_AF)) {
		res += 6;
		SET_FLAG(F_AF);
		CONDITIONAL_SET_FLAG(cf || d&0x100, F_CF);
	} else
		CLEAR_FLAG(F_AF);
	if (d > 0x99 || cf) {
		res += 0x60;
		SET_FLAG(F_CF);
	} else
		CLEAR_FLAG(F_CF);
	CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
	CONDITIONAL_SET_FLAG((res & 0xFF) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	return (uint8_t)res;
}

/****************************************************************************
REMARKS:
Implements the DAS instruction and side effects.
****************************************************************************/
uint8_t das8(x64emu_t *emu, uint8_t d)
{
	uint32_t res = d;
	CHECK_FLAGS(emu);
	uint32_t newcf = 0;
	if ((d & 0xf) > 9 || ACCESS_FLAG(F_AF)) {
		res -= 6;
		newcf = ACCESS_FLAG(F_CF) || (d < 6);
		SET_FLAG(F_AF);
	} else
		CLEAR_FLAG(F_AF);
	if (d > 0x99 || ACCESS_FLAG(F_CF)) {
		res -= 0x60;
		newcf = 1;
	}
	CONDITIONAL_SET_FLAG(newcf, F_CF);
	CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
	CONDITIONAL_SET_FLAG((res & 0xFF) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	return res;
}

/****************************************************************************
REMARKS:
Implements the RCL instruction and side effects.
****************************************************************************/
uint8_t rcl8(x64emu_t *emu, uint8_t d, uint8_t s)
{
    unsigned int res, cnt, mask, cf;
	CHECK_FLAGS(emu);
	s = s&0x1f;

    /* s is the rotate distance.  It varies from 0 - 8. */
	/* have

       CF  B_7 B_6 B_5 B_4 B_3 B_2 B_1 B_0 

       want to rotate through the carry by "s" bits.  We could 
       loop, but that's inefficient.  So the width is 9,
       and we split into three parts:

       The new carry flag   (was B_n)
       the stuff in B_n-1 .. B_0
       the stuff in B_7 .. B_n+1

       The new rotate is done mod 9, and given this,
       for a rotation of n bits (mod 9) the new carry flag is
       then located n bits from the MSB.  The low part is 
       then shifted up cnt bits, and the high part is or'd
       in.  Using CAPS for new values, and lowercase for the 
       original values, this can be expressed as:

       IF n > 0 
       1) CF <-  b_(8-n)
       2) B_(7) .. B_(n)  <-  b_(8-(n+1)) .. b_0
       3) B_(n-1) <- cf
       4) B_(n-2) .. B_0 <-  b_7 .. b_(8-(n-1))
	 */
	res = d;
	if ((cnt = s % 9) != 0) {
        /* extract the new CARRY FLAG. */
        /* CF <-  b_(8-n)             */
        cf = (d >> (8 - cnt)) & 0x1;

        /* get the low stuff which rotated 
           into the range B_7 .. B_cnt */
        /* B_(7) .. B_(n)  <-  b_(8-(n+1)) .. b_0  */
        /* note that the right hand side done by the mask */
		res = (d << cnt) & 0xff;

        /* now the high stuff which rotated around 
           into the positions B_cnt-2 .. B_0 */
        /* B_(n-2) .. B_0 <-  b_7 .. b_(8-(n-1)) */
        /* shift it downward, 7-(n-2) = 9-n positions. 
           and mask off the result before or'ing in. 
         */
        mask = (1 << (cnt - 1)) - 1;
        res |= (d >> (9 - cnt)) & mask;

        /* if the carry flag was set, or it in.  */
		if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
            /*  B_(n-1) <- cf */
            res |= 1 << (cnt - 1);
        }
        /* set the new carry flag, based on the variable "cf" */
		CONDITIONAL_SET_FLAG(cf, F_CF);
        /* OVERFLOW is set *IFF* cnt==1, then it is the 
           xor of CF and the most significant bit.  Blecck. */
		if(BOX64ENV(cputype))
		   CONDITIONAL_SET_FLAG((cf ^ (res >> 7)) & 0x1, F_OF);
	   else
		   CONDITIONAL_SET_FLAG((XOR2(d >> 6)), F_OF);
    } else if(s) {
		if(BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (res >> 7)) & 0x1, F_OF);
	}
	return (uint8_t)res;
}

uint16_t rcl16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	unsigned int res, cnt, mask, cf;
	CHECK_FLAGS(emu);
	s = s&0x1f;

	res = d;
	if ((cnt = s % 17) != 0) {
		cf = (d >> (16 - cnt)) & 0x1;
		res = (d << cnt) & 0xffff;
		mask = (1 << (cnt - 1)) - 1;
		res |= (d >> (17 - cnt)) & mask;
		if (ACCESS_FLAG(F_CF)) {
			res |= 1 << (cnt - 1);
		}
		CONDITIONAL_SET_FLAG(cf, F_CF);
		if(BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((cf ^ (res >> 15)) & 0x1, F_OF);
		else
			CONDITIONAL_SET_FLAG((XOR2(d >> 14)), F_OF);
	} else if(s) {
		res = d;
		if(BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (res >> 15)) & 0x1, F_OF);
	}
	return (uint16_t)res;
}

uint32_t rcl32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	uint32_t res, cnt, mask, cf;
	CHECK_FLAGS(emu);
	s = s&0x1f;

	res = d;
	if ((cnt = s) != 0) {
		cf = (d >> (32 - cnt)) & 0x1;
		res = (d << cnt);
		mask = (1 << (cnt - 1)) - 1;
		res |= (d >> (33 - cnt)) & mask;
		if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
			res |= 1 << (cnt - 1);
		}
		CONDITIONAL_SET_FLAG(cf, F_CF);
		if(BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((cf ^ (res >> 31)) & 0x1, F_OF);
		else
			CONDITIONAL_SET_FLAG((XOR2(d >> 30)), F_OF);
	}
	return res;
}

uint64_t rcl64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	uint64_t res, cnt, mask, cf;
	CHECK_FLAGS(emu);
	s = s&0x3f;

	res = d;
	if ((cnt = s) != 0) {
		cf = (d >> (64 - cnt)) & 0x1;
		res = (d << cnt);
		mask = (1LL << (cnt - 1)) - 1;
		res |= (d >> (65 - cnt)) & mask;
		if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
			res |= 1LL << (cnt - 1);
		}
		CONDITIONAL_SET_FLAG(cf, F_CF);
		if(BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((cf ^ (res >> 63)) & 0x1, F_OF);
		else
			CONDITIONAL_SET_FLAG((XOR2(d >> 62)), F_OF);
	}
	return res;
}
/****************************************************************************
REMARKS:
Implements the RCR instruction and side effects.
****************************************************************************/
uint8_t rcr8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	uint32_t	res, cnt;
	uint32_t	mask, cf, ocf = 0;
	CHECK_FLAGS(emu);
	s = s&0x1f;

	/* rotate right through carry */
    /* 
       s is the rotate distance.  It varies from 0 - 8.
       d is the byte object rotated.  

       have 

       CF  B_7 B_6 B_5 B_4 B_3 B_2 B_1 B_0 

       The new rotate is done mod 9, and given this,
       for a rotation of n bits (mod 9) the new carry flag is
       then located n bits from the LSB.  The low part is 
       then shifted up cnt bits, and the high part is or'd
       in.  Using CAPS for new values, and lowercase for the 
       original values, this can be expressed as:

       IF n > 0 
       1) CF <-  b_(n-1)
       2) B_(8-(n+1)) .. B_(0)  <-  b_(7) .. b_(n)
       3) B_(8-n) <- cf
       4) B_(7) .. B_(8-(n-1)) <-  b_(n-2) .. b_(0)
	 */
	res = d;
	if ((cnt = s % 9) != 0) {
        /* extract the new CARRY FLAG. */
        /* CF <-  b_(n-1)              */
		ocf = ACCESS_FLAG(F_CF) != 0;
		/* OVERFLOW is set *IFF* cnt==1, then it is the 
		xor of CF and the most significant bit.  Blecck. */
		/* parenthesized... */
		if(!BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((ocf ^ (d >> 7)) & 0x1, F_OF);
		if (cnt == 1) {
            cf = d & 0x1;
            /* note hackery here.  Access_flag(..) evaluates to either
               0 if flag not set
               non-zero if flag is set.
               doing access_flag(..) != 0 casts that into either 
			   0..1 in any representation of the flags register
               (i.e. packed bit array or unpacked.)
             */
        } else
            cf = (d >> (cnt - 1)) & 0x1;

        /* B_(8-(n+1)) .. B_(0)  <-  b_(7) .. b_n  */
        /* note that the right hand side done by the mask
           This is effectively done by shifting the 
           object to the right.  The result must be masked,
           in case the object came in and was treated 
           as a negative number.  Needed??? */

        mask = (1 << (8 - cnt)) - 1;
        res = (d >> cnt) & mask;

        /* now the high stuff which rotated around 
           into the positions B_cnt-2 .. B_0 */
        /* B_(7) .. B_(8-(n-1)) <-  b_(n-2) .. b_(0) */
        /* shift it downward, 7-(n-2) = 9-n positions. 
           and mask off the result before or'ing in. 
         */
        res |= (d << (9 - cnt));

        /* if the carry flag was set, or it in.  */
		if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
            /*  B_(8-n) <- cf */
            res |= 1 << (8 - cnt);
        }
        /* set the new carry flag, based on the variable "cf" */
		CONDITIONAL_SET_FLAG(cf, F_CF);

	}
	if(s && BOX64ENV(cputype))
		CONDITIONAL_SET_FLAG((XOR2(res >> 6)), F_OF);
	return (uint8_t)res;
}

uint16_t rcr16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	uint32_t res, cnt;
	uint32_t	mask, cf, ocf = 0;
	CHECK_FLAGS(emu);
	s = s&0x1f;

	/* rotate right through carry */
	res = d;
	if ((cnt = s % 17) != 0) {
		ocf = ACCESS_FLAG(F_CF) != 0;
		if(!BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((ocf ^ (d >> 15)) & 0x1, F_OF);
		if (cnt == 1) {
			cf = d & 0x1;
		} else
			cf = (d >> (cnt - 1)) & 0x1;
		mask = (1 << (16 - cnt)) - 1;
		res = (d >> cnt) & mask;
		res |= (d << (17 - cnt));
		if (ACCESS_FLAG(F_CF)) {
			res |= 1 << (16 - cnt);
		}
		CONDITIONAL_SET_FLAG(cf, F_CF);
	}
	if(s && BOX64ENV(cputype))
		CONDITIONAL_SET_FLAG((XOR2(res >> 14)), F_OF);
	return (uint16_t)res;
}

uint32_t rcr32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	uint32_t res, cnt;
	uint32_t mask, cf, ocf = 0;
	CHECK_FLAGS(emu);
	s = s&0x1f;

	/* rotate right through carry */
	res = d;
	if ((cnt = s) != 0) {
		ocf = ACCESS_FLAG(F_CF) != 0;
		if(!BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((ocf ^ (d >> 31)) & 0x1, F_OF);
		if (cnt == 1) {
			cf = d & 0x1;
		} else {
			cf = (d >> (cnt - 1)) & 0x1;
		}
		mask = (1 << (32 - cnt)) - 1;
		res = (d >> cnt) & mask;
		if (cnt != 1)
			res |= (d << (33 - cnt));
		if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
			res |= 1 << (32 - cnt);
		}
		CONDITIONAL_SET_FLAG(cf, F_CF);
	}
	if(s && BOX64ENV(cputype))
		CONDITIONAL_SET_FLAG((XOR2(res >> 30)), F_OF);
	return res;
}

uint64_t rcr64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	uint64_t res, cnt;
	uint64_t mask, cf, ocf = 0;
	CHECK_FLAGS(emu);
	s = s&0x3f;

	/* rotate right through carry */
	res = d;
	if ((cnt = s) != 0) {
		ocf = ACCESS_FLAG(F_CF) != 0;
		if(!BOX64ENV(cputype))
			CONDITIONAL_SET_FLAG((ocf ^ (d >> 63)) & 0x1, F_OF);
		if (cnt == 1) {
			cf = d & 0x1;
		} else {
			cf = (d >> (cnt - 1)) & 0x1;
		}
		mask = (1LL << (64 - cnt)) - 1;
		res = (d >> cnt) & mask;
		if (cnt != 1)
			res |= (d << (65 - cnt));
		if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
			res |= 1LL << (64 - cnt);
		}
		CONDITIONAL_SET_FLAG(cf, F_CF);
	}
	if(s && BOX64ENV(cputype))
		CONDITIONAL_SET_FLAG((XOR2(res >> 62)), F_OF);
	return res;
}
/****************************************************************************
REMARKS:
Implements the ROL instruction and side effects.
****************************************************************************/
uint8_t rol8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x1f;
	if(!s) return d;
	CHECK_FLAGS(emu);
	/* OF flag is set if s == 1; OF = CF _XOR_ MSB of result */
	
	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d>>6), F_OF);

	if((cnt = s % 8) != 0) {
	d = (d << cnt) + ((d >> (8 - cnt)) & ((1 << cnt) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG((d + (d >> 7)) & 1, F_OF);

	/* set new CF; note that it is the LSB of the result */
	CONDITIONAL_SET_FLAG(d & 0x1, F_CF);

	return d;
}

uint16_t rol16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x1f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d>>14), F_OF);
	if((cnt = s % 16) != 0) {
	d = (d << cnt) + ((d >> (16 - cnt)) & ((1 << cnt) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG((d + (d >> 15)) & 1, F_OF);

	CONDITIONAL_SET_FLAG(d & 0x1, F_CF);

	return d;
}

uint32_t rol32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x1f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d>>30), F_OF);
	if((cnt = s % 32) != 0) {
	d = (d << cnt) + ((d >> (32 - cnt)) & ((1 << cnt) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG((d + (d >> 31)) & 1, F_OF);

	CONDITIONAL_SET_FLAG(d & 0x1, F_CF);

	return d;
}

uint64_t rol64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x3f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d>>62), F_OF);
	if((cnt = s % 64) != 0) {
	d = (d << cnt) + ((d >> (64 - cnt)) & ((1L << cnt) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG((d + (d >> 63)) & 1, F_OF);

	CONDITIONAL_SET_FLAG(d & 0x1, F_CF);

	return d;
}

/****************************************************************************
REMARKS:
Implements the ROR instruction and side effects.
****************************************************************************/
uint8_t ror8(x64emu_t *emu, uint8_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x1f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(((d >> 7)^d)&1, F_OF);

	if((cnt = s % 8) != 0) {
		d = (d << (8 - cnt)) + ((d >> (cnt)) & ((1 << (8 - cnt)) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d >> 6), F_OF);

	/* set new CF; note that it is the MSB of the result */
	CONDITIONAL_SET_FLAG(d & (1 << 7), F_CF);

	return d;
}

uint16_t ror16(x64emu_t *emu, uint16_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x1f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(((d >> 15)^d)&1, F_OF);

	if((cnt = s % 16) != 0) {
		d = (d << (16 - cnt)) + ((d >> (cnt)) & ((1 << (16 - cnt)) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d >> 14), F_OF);

	/* set new CF; note that it is the MSB of the result */
	CONDITIONAL_SET_FLAG(d & (1 << 15), F_CF);

	return d;
}

uint32_t ror32(x64emu_t *emu, uint32_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x1f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(((d >> 31)^d)&1, F_OF);

	if((cnt = s % 32) != 0) {
		d = (d << (32 - cnt)) + ((d >> (cnt)) & ((1 << (32 - cnt)) - 1));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d >> 30), F_OF);

	/* set new CF; note that it is the MSB of the result */
	CONDITIONAL_SET_FLAG(d & (1 << 31), F_CF);

	return d;
}

uint64_t ror64(x64emu_t *emu, uint64_t d, uint8_t s)
{
	unsigned cnt;

	s = s&0x3f;
	if(!s) return d;
	CHECK_FLAGS(emu);

	if(!BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(((d >> 63)^d)&1, F_OF);

	if((cnt = s % 64) != 0) {
		d = (d << (64 - cnt)) + ((d >> (cnt)) & ((1L << (64 - cnt)) - 1L));
	}

	if(BOX64ENV(cputype)) CONDITIONAL_SET_FLAG(XOR2(d >> 62), F_OF);

	/* set new CF; note that it is the MSB of the result */
	CONDITIONAL_SET_FLAG(d & (1LL << 63), F_CF);

	return d;
}
/****************************************************************************
REMARKS:
Implements the SHLD instruction and side effects.
****************************************************************************/
uint16_t shld16 (x64emu_t *emu, uint16_t d, uint16_t fill, uint8_t s)
{
	unsigned int cnt, res, cf;

	s = s&0x1f;
	cnt = s % 16;
	if(!s)
		return d;
	RESET_FLAGS(emu);
	if (s < 16) {
		res = (d << cnt) | (fill >> (16-cnt));
		cf = d & (1 << (16 - cnt));
		CONDITIONAL_SET_FLAG(cf, F_CF);
		CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	} else {
		res = (fill << (cnt)) | (d >> (16 - cnt));
		if(s==16)
			cf = d & 1;
		else
			cf = fill & (1 << (16 - cnt));
		if(BOX64ENV(cputype) && (s>16)) {
			CLEAR_FLAG(F_CF);
		} else {
			CONDITIONAL_SET_FLAG(cf, F_CF);
		}
		CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	}
	if (BOX64ENV(cputype)) {
		if(s>15)
			CONDITIONAL_SET_FLAG(ACCESS_FLAG(F_CF), F_OF);
		else
			CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (res>>15))&1, F_OF);
	} else {
		CONDITIONAL_SET_FLAG(XOR2(d>>14), F_OF);
	}
	if (BOX64ENV(cputype))
		SET_FLAG(F_AF);
	else
		CLEAR_FLAG(F_AF);
	return (uint16_t)res;
}

uint32_t shld32 (x64emu_t *emu, uint32_t d, uint32_t fill, uint8_t s)
{
	unsigned int cnt, res, cf;

	s = s&0x1f;
	if(!s)
		return d;
	RESET_FLAGS(emu);
	cnt = s % 32;
	if (cnt > 0) {
		res = (d << cnt) | (fill >> (32-cnt));
		cf = d & (1 << (32 - cnt));
		CONDITIONAL_SET_FLAG(cf, F_CF);
		CONDITIONAL_SET_FLAG(!res, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	} else {
		res = d;
	}
	if (BOX64ENV(cputype)) {
		CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (res>>31))&1, F_OF);
	} else {
		CONDITIONAL_SET_FLAG(XOR2(d>>30), F_OF);
	}
	if (BOX64ENV(cputype))
		SET_FLAG(F_AF);
	else
		CLEAR_FLAG(F_AF);
	return res;
}

uint64_t shld64 (x64emu_t *emu, uint64_t d, uint64_t fill, uint8_t s)
{
	uint64_t cnt, res, cf;

	s = s&0x3f;
	if(!s)
		return d;
	RESET_FLAGS(emu);
	cnt = s % 64;
	if (cnt > 0) {
		res = (d << cnt) | (fill >> (64-cnt));
		cf = d & (1LL << (64 - cnt));
		CONDITIONAL_SET_FLAG(cf, F_CF);
		CONDITIONAL_SET_FLAG(!res, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x8000000000000000LL, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	} else {
		res = d;
	}
	if (BOX64ENV(cputype)) {
		CONDITIONAL_SET_FLAG((ACCESS_FLAG(F_CF) ^ (res>>63))&1, F_OF);
	} else {
		CONDITIONAL_SET_FLAG(XOR2(d>>62), F_OF);
	}
	if (BOX64ENV(cputype))
		SET_FLAG(F_AF);
	else
		CLEAR_FLAG(F_AF);
	return res;
}

/****************************************************************************
REMARKS:
Implements the SHRD instruction and side effects.
****************************************************************************/
uint16_t shrd16 (x64emu_t *emu, uint16_t d, uint16_t fill, uint8_t s)
{
	unsigned int cnt, res, cf;

	s = s&0x1f;
	if(!s)
		return d;
	RESET_FLAGS(emu);
	cnt = s % 16;
	if (s < 16) {
		if (cnt > 0) {
			cf = d & (1 << (cnt - 1));
			res = (d >> cnt) | (fill << (16 - cnt));
			CONDITIONAL_SET_FLAG(cf, F_CF);
			CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
			CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
			CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
		} else {
			res = d;
		}

	} else {
		if(s==16)
			cf = d & (1 << 15);
		else
			cf = fill & (1 << (cnt - 1));
		res = (fill >> cnt) | (d << (16 - cnt));
		if(BOX64ENV(cputype) && (s>16)) {
			CLEAR_FLAG(F_CF);
		} else {
			CONDITIONAL_SET_FLAG(cf, F_CF);
		}
		CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	#if 0
		res = 0;
		CLEAR_FLAG(F_CF);
		CLEAR_FLAG(F_OF);
		SET_FLAG(F_ZF);
		CLEAR_FLAG(F_SF);
		CLEAR_FLAG(F_PF);
	#endif
    }
	if (BOX64ENV(cputype)) {
		CONDITIONAL_SET_FLAG(XOR2(res>>14), F_OF);
	} else {
		CONDITIONAL_SET_FLAG((fill^(d>>15))&1, F_OF);
	}
	if (BOX64ENV(cputype))
		SET_FLAG(F_AF);
	else
		CLEAR_FLAG(F_AF);
	return (uint16_t)res;
}

uint32_t shrd32 (x64emu_t *emu, uint32_t d, uint32_t fill, uint8_t s)
{
	unsigned int cnt, res, cf;

	s = s&0x1f;
	if(!s)
		return d;
	RESET_FLAGS(emu);
	cnt = s % 32;
	if (cnt > 0) {
		cf = d & (1 << (cnt - 1));
		res = (d >> cnt) | (fill << (32 - cnt));
		CONDITIONAL_SET_FLAG(cf, F_CF);
		CONDITIONAL_SET_FLAG(!res, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	} else {
		res = d;
	}
	if (BOX64ENV(cputype)) {
		CONDITIONAL_SET_FLAG(XOR2(res>>30), F_OF);
	} else {
		CONDITIONAL_SET_FLAG((fill^(d>>31))&1, F_OF);
	}
	if (BOX64ENV(cputype))
		SET_FLAG(F_AF);
	else
		CLEAR_FLAG(F_AF);
	return res;
}

uint64_t shrd64 (x64emu_t *emu, uint64_t d, uint64_t fill, uint8_t s)
{
	unsigned int cnt;
	uint64_t res, cf;

	s = s&0x3f;
	if(!s)
		return d;
	RESET_FLAGS(emu);
	cnt = s % 64;
	if (cnt > 0) {
		cf = d & (1LL << (cnt - 1));
		res = (d >> cnt) | (fill << (64 - cnt));
		CONDITIONAL_SET_FLAG(cf, F_CF);
		CONDITIONAL_SET_FLAG(!res, F_ZF);
		CONDITIONAL_SET_FLAG(res & 0x8000000000000000LL, F_SF);
		CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
	} else {
		res = d;
	}
	if (BOX64ENV(cputype)) {
		CONDITIONAL_SET_FLAG(XOR2(res>>62), F_OF);
	} else {
		CONDITIONAL_SET_FLAG((fill^(d>>63))&1, F_OF);
	}
	if (BOX64ENV(cputype))
		SET_FLAG(F_AF);
	else
		CLEAR_FLAG(F_AF);
	return res;
}
/****************************************************************************
REMARKS:
Implements the SBB instruction and side effects.
****************************************************************************/
uint8_t sbb8(x64emu_t *emu, uint8_t d, uint8_t s)
{
    uint32_t res;   /* all operands in native machine order */
    uint32_t bc;
	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
		res = d - s - 1;
	else
		res = d - s;
	CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
	CONDITIONAL_SET_FLAG((res & 0xff) == 0, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return (uint8_t)res;
}

uint16_t sbb16(x64emu_t *emu, uint16_t d, uint16_t s)
{
    uint32_t res;   /* all operands in native machine order */
    uint32_t bc;
	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
        res = d - s - 1;
    else
        res = d - s;
   	CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
   	CONDITIONAL_SET_FLAG((res & 0xffff) == 0, F_ZF);
   	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return (uint16_t)res;
}

uint32_t sbb32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	uint32_t res;   /* all operands in native machine order */
	uint32_t bc;
	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
        res = d - s - 1;
    else
        res = d - s;
  	CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
  	CONDITIONAL_SET_FLAG(!res, F_ZF);
  	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return res;
}

uint64_t sbb64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	uint64_t res;   /* all operands in native machine order */
	uint64_t bc;
	CHECK_FLAGS(emu);

	if (ACCESS_FLAG(F_CF))
        res = d - s - 1;
    else
        res = d - s;
  	CONDITIONAL_SET_FLAG(res & 0x8000000000000000LL, F_SF);
  	CONDITIONAL_SET_FLAG(!res, F_ZF);
  	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);

	/* calculate the borrow chain.  See note at top */
	bc = (res & (~d | s)) | (~d & s);
	CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
	CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
	return res;
}

/****************************************************************************
REMARKS:
Implements the TEST instruction and side effects.
****************************************************************************/
void test8(x64emu_t *emu, uint8_t d, uint8_t s)
{
    uint8_t res;   /* all operands in native machine order */
	RESET_FLAGS(emu);

    res = d & s;

	CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_AF);	/* AF == dont care */
	CLEAR_FLAG(F_CF);
	CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
}

void test16(x64emu_t *emu, uint16_t d, uint16_t s)
{
	uint16_t res;   /* all operands in native machine order */
	RESET_FLAGS(emu);

	res = d & s;

	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_AF);	/* AF == dont care */
	CLEAR_FLAG(F_CF);
	CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
}

void test32(x64emu_t *emu, uint32_t d, uint32_t s)
{
	uint32_t res;   /* all operands in native machine order */
	RESET_FLAGS(emu);

	res = d & s;

	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_AF);	/* AF == dont care */
	CLEAR_FLAG(F_CF);
	CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
}

void test64(x64emu_t *emu, uint64_t d, uint64_t s)
{
	uint64_t res;   /* all operands in native machine order */
	RESET_FLAGS(emu);

	res = d & s;

	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_AF);	/* AF == dont care */
	CLEAR_FLAG(F_CF);
	CONDITIONAL_SET_FLAG(res & 0x8000000000000000LL, F_SF);
	CONDITIONAL_SET_FLAG(!res, F_ZF);
	CONDITIONAL_SET_FLAG(PARITY(res & 0xff), F_PF);
}

/****************************************************************************
REMARKS:
Implements the IDIV instruction and side effects.
****************************************************************************/

void idiv8(x64emu_t *emu, uint8_t s)
{
    int32_t dvd, quot, mod;
	if(BOX64ENV(cputype)) {
		CHECK_FLAGS(emu);
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_PF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_SF);
	}

	dvd = (int16_t)R_AX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
        return;
	}
	div_t p = div(dvd, (int8_t)s);
	quot = p.quot;
	mod = p.rem;
	if ((int8_t)quot != quot) {
		INTR_RAISE_DIV0(emu);
		return;
	}
	R_AL = (int8_t) quot;
	R_AH = (int8_t) mod;
}

void idiv16(x64emu_t *emu, uint16_t s)
{
	int32_t dvd, quot, mod;
	if(BOX64ENV(cputype)) {
		CHECK_FLAGS(emu);
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_PF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_SF);
	}

	dvd = (((int32_t)R_DX) << 16) | R_AX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
		return;
	}
	div_t p = div(dvd, (int16_t)s);
	quot = p.quot;
	mod = p.rem;
	if ((int16_t)quot != quot) {
		INTR_RAISE_DIV0(emu);
		return;
	}

	R_AX = (uint16_t)quot;
	R_DX = (uint16_t)mod;
}

void idiv32(x64emu_t *emu, uint32_t s)
{
	int64_t dvd, quot, mod;
	if(BOX64ENV(cputype)) {
		CHECK_FLAGS(emu);
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_PF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_SF);
	}

	dvd = (((int64_t)R_EDX) << 32) | R_EAX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
		return;
	}
	ldiv_t p = ldiv(dvd, (int32_t)s);
	quot = p.quot;
	mod = p.rem;
	 if ((int32_t)quot != quot) {
		INTR_RAISE_DIV0(emu);
		return;
	}

	R_RAX = (uint32_t)quot;
	R_RDX = (uint32_t)mod;
}

void idiv64(x64emu_t *emu, uint64_t s)
{
	__int128 dvd, quot, mod;
	if(BOX64ENV(cputype)) {
		CHECK_FLAGS(emu);
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_PF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_SF);
	}

	dvd = (((__int128)R_RDX) << 64) | R_RAX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
		return;
	}
	quot = dvd/(int64_t)s;
	mod = dvd%(int64_t)s;
	if ((int64_t)quot != quot) {
		INTR_RAISE_DIV0(emu);
		return;
	}

	R_RAX = (uint64_t)quot;
	R_RDX = (uint64_t)mod;
}

/****************************************************************************
REMARKS:
Implements the DIV instruction and side effects.
****************************************************************************/
void div8(x64emu_t *emu, uint8_t s)
{
	uint32_t dvd, div, mod;
	RESET_FLAGS(emu);
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_OF);
	if(BOX64ENV(cputype)) {
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_PF);
	} else {
		CLEAR_FLAG(F_AF);
		SET_FLAG(F_ZF);
		SET_FLAG(F_PF);
	}

	dvd = R_AX;
    if (s == 0) {
		INTR_RAISE_DIV0(emu);
        return;
    }
	div = dvd / (uint8_t)s;
	mod = dvd % (uint8_t)s;
	if (div > 0xff) {
		INTR_RAISE_DIV0(emu);
        return;
	}
	R_AL = (uint8_t)div;
	R_AH = (uint8_t)mod;
}

void div16(x64emu_t *emu, uint16_t s)
{
	uint32_t dvd, div, mod;
	RESET_FLAGS(emu);
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_OF);
	if(BOX64ENV(cputype)) {
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_PF);
	} else {
		CLEAR_FLAG(F_AF);
		SET_FLAG(F_ZF);
		SET_FLAG(F_PF);
	}

	dvd = (((uint32_t)R_DX) << 16) | R_AX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
        return;
    }
	div = dvd / (uint16_t)s;
	mod = dvd % (uint16_t)s;
	if (div > 0xffff) {
		INTR_RAISE_DIV0(emu);
		return;
	}

	R_AX = (uint16_t)div;
	R_DX = (uint16_t)mod;
}

void div32(x64emu_t *emu, uint32_t s)
{
	uint64_t dvd, div, mod;
	RESET_FLAGS(emu);
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_OF);
	if(BOX64ENV(cputype)) {
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_PF);
	} else {
		CLEAR_FLAG(F_AF);
		SET_FLAG(F_ZF);
		SET_FLAG(F_PF);
	}

	dvd = (((uint64_t)R_EDX) << 32) | R_EAX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
		return;
	}
	div = dvd / (uint32_t)s;
	mod = dvd % (uint32_t)s;
	if (div > 0xffffffff) {
		INTR_RAISE_DIV0(emu);
		return;
	}

	R_RAX = (uint32_t)div;
	R_RDX = (uint32_t)mod;
}

void div64(x64emu_t *emu, uint64_t s)
{
	__int128 dvd, div, mod;
	RESET_FLAGS(emu);
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_OF);
	if(BOX64ENV(cputype)) {
		SET_FLAG(F_AF);
		CLEAR_FLAG(F_ZF);
		CLEAR_FLAG(F_PF);
	} else {
		CLEAR_FLAG(F_AF);
		SET_FLAG(F_ZF);
		SET_FLAG(F_PF);
	}

	dvd = (((__int128)R_RDX) << 64) | R_RAX;
	if (s == 0) {
		INTR_RAISE_DIV0(emu);
		return;
	}
	div = dvd / (unsigned __int128)s;
	mod = dvd % (unsigned __int128)s;
	if (div > 0xffffffffffffffffL) {
		INTR_RAISE_DIV0(emu);
		return;
	}

	R_RAX = (uint64_t)div;
	R_RDX = (uint64_t)mod;
}
