/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/anim/edgeanim_spu.h"

/**
 * @brief   Evaluate all non-constant user (scalar) channels for a given frame in a frameset.
 *
 * @internal
 * 
 * @note    A reference C++ implementation of this function is available in
 *          target/win32/src/edge/anim/edgeanim_evaluate.cpp
 *
 */
void _edgeAnimEvaluateUser(
		vec_uint4 intraUserBitsAdrv,
		vec_uchar16 bitMaskXXXX,
		vec_uchar16 bitMaskYYYY,
		vec_uchar16 bitMaskZZZZ,
		vec_uchar16 bitMaskWWWW,
		vec_uchar16 prevBitMaskXXXX,
		vec_uchar16 prevBitMaskYYYY,
		vec_uchar16 prevBitMaskZZZZ,
		vec_uchar16 prevBitMaskWWWW,
		vec_uint4 frameIntegerv,
		vec_float4 frameFractionv,
		vec_uint4 intraUserAdrv,
		vec_uint4 initialUserAdrv,
		vec_uint4 finalUserAdrv,
		uint32_t count, 
		float* userOutput,
		const uint16_t* __restrict__ channelIds,
		uint32_t intraFrameCount)
{
	const vec_uchar16 zero  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	const vec_uint4 one = {1, 1, 1, 1};
	const vec_float4 fOne = {1, 1, 1, 1};
	const vec_uint4 thirtyOne = {31, 31, 31, 31};
	const vec_uchar16 allOnes   = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

	const vec_uchar16 shufAAAA = {0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03};
	const vec_uchar16 shufBBBB = {0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07};
	const vec_uchar16 shufCCCC = {0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b};
	const vec_uchar16 shufDDDD = {0x0c,0x0d,0x0e,0x0f,0x0c,0x0d,0x0e,0x0f,0x0c,0x0d,0x0e,0x0f,0x0c,0x0d,0x0e,0x0f};
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufDHLPDHLPDHLPDHLP = {0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F};
	const vec_uchar16 shufd000Dh00DHl0DHLp = {0x13,0x00,0x00,0x00,0x03,0x17,0x80,0x80,0x03,0x07,0x1b,0x80,0x03,0x07,0x0b,0x1f};
	const vec_uchar16 selABcd   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	const vec_ushort8 mask_0X0X0X0X = {0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF};

	uint32_t count4 = (count + 3) >> 2;

	for(; count4; count4--) {

		vec_uint4 bitAdr = spu_rlmask(intraUserBitsAdrv, -3);
		vec_uint4 bitOfs = spu_and(intraUserBitsAdrv, 127u);
		
		vec_uint4 bitAdr0 = bitAdr;
		vec_uint4 bitAdr1 = spu_slqwbyte(bitAdr, 4);
		vec_uint4 bitAdr2 = spu_slqwbyte(bitAdr, 8);
		vec_uint4 bitAdr3 = spu_slqwbyte(bitAdr, 12);
		
		int32_t bitOfs0 = si_to_int((vec_char16)bitOfs);
		int32_t bitOfs1 = si_to_int((vec_char16)spu_slqwbyte(bitOfs, 4));
		int32_t bitOfs2 = si_to_int((vec_char16)spu_slqwbyte(bitOfs, 8));
		int32_t bitOfs3 = si_to_int((vec_char16)spu_slqwbyte(bitOfs, 12));
		
		vec_uchar16 d0a = (vec_uchar16)si_lqd((vec_char16)bitAdr0, 0x00);
		vec_uchar16 d0b = (vec_uchar16)si_lqd((vec_char16)bitAdr0, 0x10);
		vec_uchar16 d1a = (vec_uchar16)si_lqd((vec_char16)bitAdr1, 0x00);
		vec_uchar16 d1b = (vec_uchar16)si_lqd((vec_char16)bitAdr1, 0x10);
		vec_uchar16 d2a = (vec_uchar16)si_lqd((vec_char16)bitAdr2, 0x00);
		vec_uchar16 d2b = (vec_uchar16)si_lqd((vec_char16)bitAdr2, 0x10);
		vec_uchar16 d3a = (vec_uchar16)si_lqd((vec_char16)bitAdr3, 0x00);
		vec_uchar16 d3b = (vec_uchar16)si_lqd((vec_char16)bitAdr3, 0x10);
		
		vec_uchar16 d0as = spu_rlqwbytebc(d0a, bitOfs0);
		vec_uchar16 d0bs = spu_rlqwbytebc(d0b, bitOfs0);
		vec_uchar16 d1as = spu_rlqwbytebc(d1a, bitOfs1);
		vec_uchar16 d1bs = spu_rlqwbytebc(d1b, bitOfs1);
		vec_uchar16 d2as = spu_rlqwbytebc(d2a, bitOfs2);
		vec_uchar16 d2bs = spu_rlqwbytebc(d2b, bitOfs2);
		vec_uchar16 d3as = spu_rlqwbytebc(d3a, bitOfs3);
		vec_uchar16 d3bs = spu_rlqwbytebc(d3b, bitOfs3);
		
		vec_uchar16 d0as0 = spu_rlqw(d0as, bitOfs0);
		vec_uchar16 d0bs0 = spu_rlqw(d0bs, bitOfs0);
		vec_uchar16 d1as0 = spu_rlqw(d1as, bitOfs1);
		vec_uchar16 d1bs0 = spu_rlqw(d1bs, bitOfs1);
		vec_uchar16 d2as0 = spu_rlqw(d2as, bitOfs2);
		vec_uchar16 d2bs0 = spu_rlqw(d2bs, bitOfs2);
		vec_uchar16 d3as0 = spu_rlqw(d3as, bitOfs3);
		vec_uchar16 d3bs0 = spu_rlqw(d3bs, bitOfs3);
		
		vec_uchar16 msk0a = spu_slqwbytebc(allOnes, bitOfs0);
		vec_uchar16 msk1a = spu_slqwbytebc(allOnes, bitOfs1);
		vec_uchar16 msk2a = spu_slqwbytebc(allOnes, bitOfs2);
		vec_uchar16 msk3a = spu_slqwbytebc(allOnes, bitOfs3);
		
		vec_uchar16 msk0b = spu_slqw(msk0a, bitOfs0);
		vec_uchar16 msk1b = spu_slqw(msk1a, bitOfs1);
		vec_uchar16 msk2b = spu_slqw(msk2a, bitOfs2);
		vec_uchar16 msk3b = spu_slqw(msk3a, bitOfs3);
		
		vec_uchar16 ib0 = spu_sel(d0bs0, d0as0, msk0b);
		vec_uchar16 ib1 = spu_sel(d1bs0, d1as0, msk1b);
		vec_uchar16 ib2 = spu_sel(d2bs0, d2as0, msk2b);
		vec_uchar16 ib3 = spu_sel(d3bs0, d3as0, msk3b);
		
		vec_uchar16 ibs0 = spu_shuffle(ib0, ib1, shufAaBb);
		vec_uchar16 ibs1 = spu_shuffle(ib0, ib1, shufCcDd);
		vec_uchar16 ibs2 = spu_shuffle(ib2, ib3, shufBbAa);
		vec_uchar16 ibs3 = spu_shuffle(ib2, ib3, shufDdCc);
		vec_uchar16 ibx = spu_sel(ibs0, ibs2, selABcd);
		vec_uchar16 iby = spu_shuffle(ibs0, ibs2, shufCDab);
		vec_uchar16 ibz = spu_sel(ibs1, ibs3, selABcd);
		vec_uchar16 ibw = spu_shuffle(ibs1, ibs3, shufCDab);
		
		vec_uchar16 bx = spu_and(ibx, bitMaskXXXX);
		vec_uchar16 by = spu_and(iby, bitMaskYYYY);
		vec_uchar16 bz = spu_and(ibz, bitMaskZZZZ);
		vec_uchar16 bw = spu_and(ibw, bitMaskWWWW);
		
		vec_uchar16 pbx = spu_and(ibx, prevBitMaskXXXX);
		vec_uchar16 pby = spu_and(iby, prevBitMaskYYYY);
		vec_uchar16 pbz = spu_and(ibz, prevBitMaskZZZZ);
		vec_uchar16 pbw = spu_and(ibw, prevBitMaskWWWW);
		
		vec_uchar16 nbx = spu_cntb(bx);
		vec_uchar16 nby = spu_cntb(by);
		vec_uchar16 nbz = spu_cntb(bz);
		vec_uchar16 nbw = spu_cntb(bw);
		vec_uchar16 npbx = spu_cntb(pbx);
		vec_uchar16 npby = spu_cntb(pby);
		vec_uchar16 npbz = spu_cntb(pbz);
		vec_uchar16 npbw = spu_cntb(pbw);
		vec_ushort8 sumbx = spu_sumb(nbx, npbx);
		vec_ushort8 sumby = spu_sumb(nby, npby);
		vec_ushort8 sumbz = spu_sumb(nbz, npbz);
		vec_ushort8 sumbw = spu_sumb(nbw, npbw);
		vec_ushort8 numBits = spu_add(sumbx, sumby);
		numBits = spu_add(numBits, sumbz );
		numBits = spu_add(numBits, sumbw );
		vec_ushort8 numPrevBits = (vec_ushort8)spu_rlmask((vec_uint4)numBits, -16);
		numBits = spu_and(numBits, mask_0X0X0X0X );
		
		vec_uint4 tz0 = spu_cntlz(spu_and((vec_uint4)pbx, spu_sub(0, (vec_uint4)pbx)));
		vec_uint4 tz1 = spu_cntlz(spu_and((vec_uint4)pby, spu_sub(0, (vec_uint4)pby)));
		vec_uint4 tz2 = spu_cntlz(spu_and((vec_uint4)pbz, spu_sub(0, (vec_uint4)pbz)));
		vec_uint4 tz3 = spu_cntlz(spu_and((vec_uint4)pbw, spu_sub(0, (vec_uint4)pbw)));
		vec_uint4 tzc0 = spu_cmpeq(tz0, 32);
		vec_uint4 tzc1 = spu_cmpeq(tz1, 32);
		vec_uint4 tzc2 = spu_cmpeq(tz2, 32);
		vec_uint4 tzc3 = spu_cmpeq(tz3, 32);
		tz0 = spu_sel(spu_sub(thirtyOne, tz0), tz0, tzc0);
		tz1 = spu_sel(spu_sub(thirtyOne, tz1), tz1, tzc1);
		tz2 = spu_sel(spu_sub(thirtyOne, tz2), tz2, tzc2);
		tz3 = spu_sel(spu_sub(thirtyOne, tz3), tz3, tzc3);
		vec_uint4 tzm1 = spu_and(tzc3, tzc2);
		vec_uint4 tzm2 = spu_and(tzm1, tzc1);
		vec_uint4 tzs1 = spu_add(tz3, spu_and(tz2, tzc3));
		vec_uint4 tzs2 = spu_add(tzs1, spu_and(tz1, tzm1));
		vec_uint4 tzs3 = spu_add(tzs2, spu_and(tz0, tzm2));
		vec_uint4 bBits = spu_sub(tzs3, spu_sub(spu_splats(128u), frameIntegerv));
		
		vec_uint4 lz0 = spu_cntlz((vec_uint4)spu_orc(spu_andc(bx, prevBitMaskXXXX), bitMaskXXXX));  
		vec_uint4 lz1 = spu_cntlz((vec_uint4)spu_orc(spu_andc(by, prevBitMaskYYYY), bitMaskYYYY));  
		vec_uint4 lz2 = spu_cntlz((vec_uint4)spu_orc(spu_andc(bz, prevBitMaskZZZZ), bitMaskZZZZ));  
		vec_uint4 lz3 = spu_cntlz((vec_uint4)spu_orc(spu_andc(bw, prevBitMaskWWWW), bitMaskWWWW));  
		vec_uint4 lzc0 = spu_cmpeq(lz0, 32);
		vec_uint4 lzc1 = spu_cmpeq(lz1, 32);
		vec_uint4 lzc2 = spu_cmpeq(lz2, 32);
		vec_uint4 lzm1 = spu_and(lzc0, lzc1);
		vec_uint4 lzm2 = spu_and(lzm1, lzc2);
		vec_uint4 lzs1 = spu_add(lz0, spu_and(lz1, lzc0));
		vec_uint4 lzs2 = spu_add(lzs1, spu_and(lz2, lzm1));
		vec_uint4 lzs3 = spu_add(lzs2, spu_and(lz3, lzm2));
		vec_uint4 aBits = spu_sub(spu_add(lzs3, one), frameIntegerv);

		// Get addresses of the intra frame data bracketing each channel
		vec_uchar16 numIntraShuf = (vec_uchar16)spu_shuffle(numBits, numPrevBits, shufd000Dh00DHl0DHLp);
		vec_uint4 numIntraSum = (vec_uint4)spu_sumb(numIntraShuf, zero);
		vec_uint4 intraOfs = spu_sl(numIntraSum, 2);
		vec_uint4 rightIntraAdr = spu_add(intraUserAdrv, intraOfs);
		vec_uint4 leftIntraAdr = spu_add(rightIntraAdr, -4);

		// If there is no left/right intra frame data, use initial/final data
		vec_uint4 leftSelMask = spu_cmpgt((vec_uint4)numPrevBits, (vec_uint4)zero);
		vec_uint4 rightSelMask = spu_cmpeq((vec_uint4)numPrevBits, (vec_uint4)numBits);
		vec_uint4 leftAdr = spu_sel(initialUserAdrv, leftIntraAdr, leftSelMask);
		vec_uint4 rightAdr = spu_sel(rightIntraAdr, finalUserAdrv, rightSelMask);

		// Compute tweening factors
		vec_float4 bBitsf = spu_convtf(bBits, 0);
		vec_float4 aBitsf = spu_convtf(aBits, 0);
		vec_float4 tween0 = spu_add(frameFractionv, bBitsf);
		vec_float4 tmp0 = spu_add(bBitsf, aBitsf);
		vec_float4 tmp1 = (vec_float4)si_frest((qword)tmp0);
		vec_float4 tmp2 = (vec_float4)si_fi((qword)tmp0, (qword)tmp1);
		vec_float4 tmp3 = spu_nmsub(tmp0, tmp2, fOne);
		vec_float4 tween1 = spu_madd(tmp3, tmp2, tmp2);
		vec_float4 tween  = spu_mul(tween0, tween1);

		// Load 
		vec_char16 left0Adr = (vec_char16)leftAdr;
		vec_char16 left1Adr = (vec_char16)spu_slqwbyte(leftAdr, 4);
		vec_char16 left2Adr = (vec_char16)spu_slqwbyte(leftAdr, 8);
		vec_char16 left3Adr = (vec_char16)spu_slqwbyte(leftAdr, 12);
		vec_char16 right0Adr = (vec_char16)rightAdr;
		vec_char16 right1Adr = (vec_char16)spu_slqwbyte(rightAdr, 4);
		vec_char16 right2Adr = (vec_char16)spu_slqwbyte(rightAdr, 8);
		vec_char16 right3Adr = (vec_char16)spu_slqwbyte(rightAdr, 12);

		vec_uint4 q0a = (vec_uint4)si_lqd(left0Adr, 0x00);
		vec_uint4 q1a = (vec_uint4)si_lqd(left1Adr, 0x00);
		vec_uint4 q2a = (vec_uint4)si_lqd(left2Adr, 0x00);
		vec_uint4 q3a = (vec_uint4)si_lqd(left3Adr, 0x00);
		vec_uint4 q4a = (vec_uint4)si_lqd(right0Adr, 0x00);
		vec_uint4 q5a = (vec_uint4)si_lqd(right1Adr, 0x00);
		vec_uint4 q6a = (vec_uint4)si_lqd(right2Adr, 0x00);
		vec_uint4 q7a = (vec_uint4)si_lqd(right3Adr, 0x00);

		vec_float4 left0 = (vec_float4)spu_rlqwbyte(q0a, si_to_uint(left0Adr));
		vec_float4 left1 = (vec_float4)spu_rlqwbyte(q1a, si_to_uint(left1Adr));
		vec_float4 left2 = (vec_float4)spu_rlqwbyte(q2a, si_to_uint(left2Adr));
		vec_float4 left3 = (vec_float4)spu_rlqwbyte(q3a, si_to_uint(left3Adr));
		vec_float4 right0 = (vec_float4)spu_rlqwbyte(q4a, si_to_uint(right0Adr));
		vec_float4 right1 = (vec_float4)spu_rlqwbyte(q5a, si_to_uint(right1Adr));
		vec_float4 right2 = (vec_float4)spu_rlqwbyte(q6a, si_to_uint(right2Adr));
		vec_float4 right3 = (vec_float4)spu_rlqwbyte(q7a, si_to_uint(right3Adr));

		// Lerp 
		vec_float4 t0 = spu_shuffle(tween, tween, shufAAAA);
		vec_float4 t1 = spu_shuffle(tween, tween, shufBBBB);
		vec_float4 t2 = spu_shuffle(tween, tween, shufCCCC);
		vec_float4 t3 = spu_shuffle(tween, tween, shufDDDD);

		vec_float4 out0Tmp = spu_madd(right0, t0, left0);
		vec_float4 out0 = spu_nmsub(left0, t0, out0Tmp);

		vec_float4 out1Tmp = spu_madd(right1, t1, left1);
		vec_float4 out1 = spu_nmsub(left1, t1, out1Tmp);

		vec_float4 out2Tmp = spu_madd(right2, t2, left2);
		vec_float4 out2 = spu_nmsub(left2, t2, out2Tmp);

		vec_float4 out3Tmp = spu_madd(right3, t3, left3);
		vec_float4 out3 = spu_nmsub(left3, t3, out3Tmp);

		// Store
		// TODO: optimise
		userOutput[channelIds[0]] = si_to_float((vec_char16)out0);
		userOutput[channelIds[1]] = si_to_float((vec_char16)out1);
		userOutput[channelIds[2]] = si_to_float((vec_char16)out2);
		userOutput[channelIds[3]] = si_to_float((vec_char16)out3);

		vec_uint4 intraUserInc0 = (vec_uint4)spu_sumb((vec_uchar16)spu_shuffle(numBits, numBits, shufDHLPDHLPDHLPDHLP), zero);
		vec_uint4 intraUserInc = spu_sl(intraUserInc0, 2);

		// Advance pointers
		intraUserBitsAdrv = spu_add(intraUserBitsAdrv, intraFrameCount*4);
		initialUserAdrv = spu_add(initialUserAdrv, 4*4);
		intraUserAdrv = spu_add(intraUserAdrv, intraUserInc);
		finalUserAdrv = spu_add(finalUserAdrv, 4*4);
		channelIds += 4;
	}
}

