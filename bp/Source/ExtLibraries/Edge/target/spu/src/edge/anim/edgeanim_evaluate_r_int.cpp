/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/anim/edgeanim_spu.h"

/** 
* @brief   Evaluate all non-constant rotation channels for a given frame in a frameset
*          SPU intrinisics implementation.
*
*          For each channel, the intra-frame bitstream is decoded to determine the two 
*          bracketing keyframes. These are then decompressed and slerped. The compression 
*          used is "smallest 3" (48 bits per quaternion).
*
*          (See LibEdge-Overview_e.doc "Animation Encoding and Compression" for more details 
*          on the frameset data structure)
*
*          Here is a more detailed flow of the process:
*
*          Let frameFraction be the fractional value of the evaluation frame
*          Let tableR be the table of non-constant rotation channel indices
*          For each non-constant rotation channel
*            Load the intra-frame bits for this channel (0 - 128 bits)
*            Let keyRData be the starting address of the intra-frame compressed key-frames for this channel
*            Let totalBits be the total number of enabled intra-frame bits
*            Let prevBits be the number of enabled intra-frame bits previous to this frame
*            If prevBits = 0 then
*              Let keyA be the initial key-frame decompressed
*            Else
*              Let keyA be the key-frame decompressed at keyRData + sizeof(R) * prevBits
*            If prevBits = totalBits then
*              Let keyB be the final key-frame decompressed
*            Else
*              Let keyB be the key-frame decompressed at keyRData + sizeof(R) * (prevBits+1)
*            Let bBits be the number of contiguous disabled bits before, and including, this frame’s bit
*            Let aBits be the number of contiguous disabled bits after this frame’s bit
*            Let alpha be equal to (bBits + frameFraction) / (aBits + bBits + 1)
*            Let interp be the slerp between keyA and keyB with alpha
*            Write interp to the output pose, using the mapping from tableR
*
* @note    A reference C++ implementation of this function is available in
*          target/reference/src/edge/anim/edgeanim_evaluate.cpp
*
* @note    This function is not called unless you change the makefile which 
*          uses asm implementation by default.
*/

#warning sub-optimal version - intrinisics implementation instead of asm

void _edgeAnimEvaluateR(
		vec_uint4 intraRBitsAdrv,
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
		vec_uint4 intraRAdrv,
		vec_uint4 initialRAdrv,
		vec_uint4 finalRAdrv,
		uint32_t count, 
		vec_float4* output,
		const uint16_t* __restrict__ channelIds,
		uint32_t intraFrameCount)
{
	// MacLaurin - Acos
	const vec_float4 acos0 = {1.5707963050, 1.5707963050, 1.5707963050, 1.5707963050};
	const vec_float4 acos1 = {0.2145988016, 0.2145988016, 0.2145988016, 0.2145988016};
	const vec_float4 acos2 = {0.0889789874, 0.0889789874, 0.0889789874, 0.0889789874};
	const vec_float4 acos3 = {0.0501743046, 0.0501743046, 0.0501743046, 0.0501743046};

	// Taylor - sin
	const vec_float4 sin0 = {-0.1666666664, -0.1666666664, -0.1666666664, -0.1666666664};
	const vec_float4 sin1 = {0.0083333315, 0.0083333315, 0.0083333315, 0.0083333315};
	const vec_float4 sin2 = {-0.000198409, -0.000198409, -0.000198409, -0.000198409};
	const vec_float4 sin3 = {0.0000027526, 0.0000027526, 0.0000027526, 0.0000027526};

	const vec_float4 fOneMinusEpsilon = {0.999f, 0.999f, 0.999f, 0.999f};
	const vec_float4 fOne = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 fMinusOne = {-1.0f, -1.0f, -1.0f, -1.0f};
	const vec_float4 fHalf = {0.5f, 0.5f, 0.5f, 0.5f};
	static const float sqrt2 = 1.4142135624f;
	const vec_float4 quatScale = {sqrt2, sqrt2, sqrt2, sqrt2};
	const vec_float4 quatBias = {-0.5*sqrt2, -0.5*sqrt2, -0.5*sqrt2, -0.5*sqrt2};
	const vec_uchar16 zero  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	const vec_uint4 one = {1, 1, 1, 1};
	const vec_uint4 thirtyOne = {31, 31, 31, 31};
	const vec_uchar16 allOnes   = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufABcd = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufABCD = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
	const vec_uchar16 shufabcd = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shuf0A0a0000 = {0x80,0x80,0x00,0x01,0x80,0x80,0x10,0x11,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
	const vec_uchar16 shuf00000A0a = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x01,0x80,0x80,0x10,0x11};
	const vec_uchar16 shufBCbc0000 = {0x02,0x03,0x04,0x05,0x12,0x13,0x14,0x15,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
	const vec_uchar16 shuf0000BCbc = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x02,0x03,0x04,0x05,0x12,0x13,0x14,0x15};
	const vec_uchar16 shufDHLPDHLPDHLPDHLP = {0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F};
	const vec_uchar16 shufd000Dh00DHl0DHLp = {0x13,0x00,0x00,0x00,0x03,0x17,0x80,0x80,0x03,0x07,0x1b,0x80,0x03,0x07,0x0b,0x1f};
	const vec_uchar16 selABcd   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	const vec_ushort8 mask_0X0X0X0X = {0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF};

	uint32_t count4 = (count + 3) >> 2;

	for(; count4; count4--) {
		vec_uint4 bitAdr = spu_rlmask(intraRBitsAdrv, -3);
		vec_uint4 bitOfs = spu_and(intraRBitsAdrv, 127u);

		// Get intra frame rotation bits for 4 joints, left justified
		// Note: Currently assuming a limit of 128 intra frames
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

		// swizzle -> xxxx yyyy zzzz wwww
		vec_uchar16 ibs0 = spu_shuffle(ib0, ib1, shufAaBb);
		vec_uchar16 ibs1 = spu_shuffle(ib0, ib1, shufCcDd);
		vec_uchar16 ibs2 = spu_shuffle(ib2, ib3, shufBbAa);
		vec_uchar16 ibs3 = spu_shuffle(ib2, ib3, shufDdCc);
		vec_uchar16 ibx = spu_sel(ibs0, ibs2, selABcd);
		vec_uchar16 iby = spu_shuffle(ibs0, ibs2, shufCDab);
		vec_uchar16 ibz = spu_sel(ibs1, ibs3, selABcd);
		vec_uchar16 ibw = spu_shuffle(ibs1, ibs3, shufCDab);

		// Clear invalid bits (if intra frame count is less than 128)
		vec_uchar16 bx = spu_and(ibx, bitMaskXXXX);
		vec_uchar16 by = spu_and(iby, bitMaskYYYY);
		vec_uchar16 bz = spu_and(ibz, bitMaskZZZZ);
		vec_uchar16 bw = spu_and(ibw, bitMaskWWWW);

		// Get rotation bits prior to current frame
		vec_uchar16 pbx = spu_and(ibx, prevBitMaskXXXX);
		vec_uchar16 pby = spu_and(iby, prevBitMaskYYYY);
		vec_uchar16 pbz = spu_and(ibz, prevBitMaskZZZZ);
		vec_uchar16 pbw = spu_and(ibw, prevBitMaskWWWW);

		// Count enabled bits
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

		// Get number of contiguously disabled bits before (and including) current frame
		// i.e. the number of trailing zeros of prb.
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

		// Get number of contiguously disabled bits after (but not including) current frame
		// i.e. the number of leading zeros of rb
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

		// Get addresses of the intra frame data bracketing each joint
		vec_uchar16 numShuf = (vec_uchar16)spu_shuffle(numBits, numPrevBits, shufd000Dh00DHl0DHLp);
		vec_uint4 numSum = (vec_uint4)spu_sumb(numShuf, zero);
		vec_uint4 intraOfs = spu_add(spu_sl(numSum, 2), spu_sl(numSum, 1));
		vec_uint4 rightIntraAdr = spu_add(intraRAdrv, intraOfs);
		vec_uint4 leftIntraAdr = spu_add(rightIntraAdr, -6);

		// If there is no left/right intra frame data, use initial/final data
		vec_uint4 leftSelMask = spu_cmpgt((vec_uint4)numPrevBits, (vec_uint4)zero);
		vec_uint4 rightSelMask = spu_cmpeq((vec_uint4)numPrevBits, (vec_uint4)numBits);
		vec_uint4 leftAdr = spu_sel(initialRAdrv, leftIntraAdr, leftSelMask);
		vec_uint4 rightAdr = spu_sel(rightIntraAdr, finalRAdrv, rightSelMask);

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

		// Get rotation data pointers
		vec_char16 left0Adr = (vec_char16)leftAdr;
		vec_char16 left1Adr = (vec_char16)spu_slqwbyte(leftAdr, 4);
		vec_char16 left2Adr = (vec_char16)spu_slqwbyte(leftAdr, 8);
		vec_char16 left3Adr = (vec_char16)spu_slqwbyte(leftAdr, 12);
		vec_char16 right0Adr = (vec_char16)rightAdr;
		vec_char16 right1Adr = (vec_char16)spu_slqwbyte(rightAdr, 4);
		vec_char16 right2Adr = (vec_char16)spu_slqwbyte(rightAdr, 8);
		vec_char16 right3Adr = (vec_char16)spu_slqwbyte(rightAdr, 12);

		// Decompress the 8 rotation quaternions to float 4
		// Compression is 'smallest 3' (15.15.15.2)
		vec_uint4 q0a = (vec_uint4)si_lqd(left0Adr, 0x00);
		vec_uint4 q0b = (vec_uint4)si_lqd(left0Adr, 0x10);
		vec_uint4 q1a = (vec_uint4)si_lqd(left1Adr, 0x00);
		vec_uint4 q1b = (vec_uint4)si_lqd(left1Adr, 0x10);
		vec_uint4 q2a = (vec_uint4)si_lqd(left2Adr, 0x00);
		vec_uint4 q2b = (vec_uint4)si_lqd(left2Adr, 0x10);
		vec_uint4 q3a = (vec_uint4)si_lqd(left3Adr, 0x00);
		vec_uint4 q3b = (vec_uint4)si_lqd(left3Adr, 0x10);
		vec_uint4 q4a = (vec_uint4)si_lqd(right0Adr, 0x00);
		vec_uint4 q4b = (vec_uint4)si_lqd(right0Adr, 0x10);
		vec_uint4 q5a = (vec_uint4)si_lqd(right1Adr, 0x00);
		vec_uint4 q5b = (vec_uint4)si_lqd(right1Adr, 0x10);
		vec_uint4 q6a = (vec_uint4)si_lqd(right2Adr, 0x00);
		vec_uint4 q6b = (vec_uint4)si_lqd(right2Adr, 0x10);
		vec_uint4 q7a = (vec_uint4)si_lqd(right3Adr, 0x00);
		vec_uint4 q7b = (vec_uint4)si_lqd(right3Adr, 0x10);

		uint32_t shift0 = si_to_uint(left0Adr) & 15;
		uint32_t shift1 = si_to_uint(left1Adr) & 15;
		uint32_t shift2 = si_to_uint(left2Adr) & 15;
		uint32_t shift3 = si_to_uint(left3Adr) & 15;
		uint32_t shift4 = si_to_uint(right0Adr) & 15;
		uint32_t shift5 = si_to_uint(right1Adr) & 15;
		uint32_t shift6 = si_to_uint(right2Adr) & 15;
		uint32_t shift7 = si_to_uint(right3Adr) & 15;

		// Left justify each compressed quat
		vec_uint4 q0 = spu_or(spu_slqwbyte(q0a, shift0), spu_rlmaskqwbyte(q0b, shift0 - 16));
		vec_uint4 q1 = spu_or(spu_slqwbyte(q1a, shift1), spu_rlmaskqwbyte(q1b, shift1 - 16));
		vec_uint4 q2 = spu_or(spu_slqwbyte(q2a, shift2), spu_rlmaskqwbyte(q2b, shift2 - 16));
		vec_uint4 q3 = spu_or(spu_slqwbyte(q3a, shift3), spu_rlmaskqwbyte(q3b, shift3 - 16));
		vec_uint4 q4 = spu_or(spu_slqwbyte(q4a, shift4), spu_rlmaskqwbyte(q4b, shift4 - 16));
		vec_uint4 q5 = spu_or(spu_slqwbyte(q5a, shift5), spu_rlmaskqwbyte(q5b, shift5 - 16));
		vec_uint4 q6 = spu_or(spu_slqwbyte(q6a, shift6), spu_rlmaskqwbyte(q6b, shift6 - 16));
		vec_uint4 q7 = spu_or(spu_slqwbyte(q7a, shift7), spu_rlmaskqwbyte(q7b, shift7 - 16));

		// Unpack 15-bit components
		vec_uint4 a0i = spu_or(spu_shuffle(q0, q1, shuf0A0a0000), spu_shuffle(q2, q3, shuf00000A0a));
		vec_uint4 a4i = spu_or(spu_shuffle(q4, q5, shuf0A0a0000), spu_shuffle(q6, q7, shuf00000A0a));

		vec_uint4 bc0i = spu_or(spu_shuffle(q0, q1, shufBCbc0000), spu_shuffle(q2, q3, shuf0000BCbc));
		vec_uint4 bc4i = spu_or(spu_shuffle(q4, q5, shufBCbc0000), spu_shuffle(q6, q7, shuf0000BCbc));

		vec_uint4 b0i = spu_and(spu_rlmask(bc0i, -17), 0x7fff);
		vec_uint4 b4i = spu_and(spu_rlmask(bc4i, -17), 0x7fff);
		vec_uint4 c0i = spu_and(spu_rlmask(bc0i, -2), 0x7fff);
		vec_uint4 c4i = spu_and(spu_rlmask(bc4i, -2), 0x7fff);

		vec_float4 a0 = spu_madd(spu_convtf(a0i, 15), quatScale, quatBias);
		vec_float4 a4 = spu_madd(spu_convtf(a4i, 15), quatScale, quatBias);
		vec_float4 b0 = spu_madd(spu_convtf(b0i, 15), quatScale, quatBias);
		vec_float4 b4 = spu_madd(spu_convtf(b4i, 15), quatScale, quatBias);
		vec_float4 c0 = spu_madd(spu_convtf(c0i, 15), quatScale, quatBias);
		vec_float4 c4 = spu_madd(spu_convtf(c4i, 15), quatScale, quatBias);

		// Derived value, d = sqrt(1 - a^2 - b^2 - c^2)
		vec_float4 d0sqr0 = spu_nmsub(a0, a0, fOne);
		vec_float4 d0sqr1 = spu_nmsub(b0, b0, d0sqr0);
		vec_float4 d0sqr2 = spu_nmsub(c0, c0, d0sqr1);
		vec_float4 d0tmp0 = (vec_float4)si_frsqest((qword)d0sqr2);
		vec_float4 d0tmp1 = (vec_float4)si_fi((qword)d0sqr2, (qword)d0tmp0);
		vec_float4 d0tmp2 = spu_mul(d0sqr2, d0tmp1);
		vec_float4 d0tmp3 = spu_mul(d0tmp1, fHalf);
		vec_float4 d0tmp4 = spu_nmsub(d0tmp2, d0tmp1, fOne);
		vec_float4 d0inv = spu_madd(d0tmp3, d0tmp4, d0tmp1);
		vec_float4 d0 = spu_mul(d0inv, d0sqr2);

		vec_float4 d4sqr0 = spu_nmsub(a4, a4, fOne);
		vec_float4 d4sqr1 = spu_nmsub(b4, b4, d4sqr0);
		vec_float4 d4sqr2 = spu_nmsub(c4, c4, d4sqr1);
		vec_float4 d4tmp0 = (vec_float4)si_frsqest((qword)d4sqr2);
		vec_float4 d4tmp1 = (vec_float4)si_fi((qword)d4sqr2, (qword)d4tmp0);
		vec_float4 d4tmp2 = spu_mul(d4sqr2, d4tmp1);
		vec_float4 d4tmp3 = spu_mul(d4tmp1, fHalf);
		vec_float4 d4tmp4 = spu_nmsub(d4tmp2, d4tmp1, fOne);
		vec_float4 d4inv = spu_madd(d4tmp3, d4tmp4, d4tmp1);
		vec_float4 d4 = spu_mul(d4inv, d4sqr2);

		// Get 2-bit derived value index
		vec_uint4 idx0 = spu_and(spu_nor(bc0i, bc0i), 0x3);
		vec_uint4 idx4 = spu_and(spu_nor(bc4i, bc4i), 0x3);

		// Now shuffle components to insert the derived value.
		// Using selects rather than deswizzle+shuffle as I want to keep the quats in swizzled form for the slerp...
		vec_float4 leftRotX, leftRotY, leftRotZ, leftRotW, rightRotX, rightRotY, rightRotZ, rightRotW;

		vec_uint4 sel0a = spu_sl(spu_splats(0xe0000000u), idx0);
		vec_uint4 sel0b = spu_add(sel0a, sel0a);
		vec_uint4 sel0c = spu_add(sel0b, sel0b);
		vec_uint4 selMsk0a = spu_rlmaska(sel0a, -31);
		vec_uint4 selMsk0b = spu_rlmaska(sel0b, -31);
		vec_uint4 selMsk0c = spu_rlmaska(sel0c, -31);
		leftRotY = spu_sel(a0, d0, selMsk0a);
		leftRotX = spu_sel(d0, a0, selMsk0a);
		leftRotZ = spu_sel(b0, leftRotY, selMsk0b);
		leftRotY = spu_sel(leftRotY, b0, selMsk0b);
		leftRotW = spu_sel(c0, leftRotZ, selMsk0c);
		leftRotZ = spu_sel(leftRotZ, c0, selMsk0c);

		vec_uint4 sel4a = spu_sl(spu_splats(0xe0000000u), idx4);
		vec_uint4 sel4b = spu_add(sel4a, sel4a);
		vec_uint4 sel4c = spu_add(sel4b, sel4b);
		vec_uint4 selMsk4a = spu_rlmaska(sel4a, -31);
		vec_uint4 selMsk4b = spu_rlmaska(sel4b, -31);
		vec_uint4 selMsk4c = spu_rlmaska(sel4c, -31);
		rightRotY = spu_sel(a4, d4, selMsk4a);
		rightRotX = spu_sel(d4, a4, selMsk4a);
		rightRotZ = spu_sel(b4, rightRotY, selMsk4b);
		rightRotY = spu_sel(rightRotY, b4, selMsk4b);
		rightRotW = spu_sel(c4, rightRotZ, selMsk4c);
		rightRotZ = spu_sel(rightRotZ, c4, selMsk4c);

		// Slerp each (left, right) rotation pair with the tweening factors computed earlier

		// Slerp rotations : compute dot product
		vec_float4 dotRotX = spu_mul(leftRotX, rightRotX);
		vec_float4 dotRotXY = spu_madd(leftRotY, rightRotY, dotRotX);
		vec_float4 dotRotXYZ = spu_madd(leftRotZ, rightRotZ, dotRotXY);
		vec_float4 dotRot = spu_madd(leftRotW, rightRotW, dotRotXYZ);   
		vec_float4 floatSign = spu_sel(fOne, fMinusOne, spu_cmpgt((vec_float4)zero, dotRot));

		// Slerp rotations : determine target rotation (right or -right)
		vec_float4 targetRotX = spu_mul(rightRotX, floatSign);
		vec_float4 targetRotY = spu_mul(rightRotY, floatSign);
		vec_float4 targetRotZ = spu_mul(rightRotZ, floatSign);
		vec_float4 targetRotW = spu_mul(rightRotW, floatSign);
		vec_float4 cosAngle = spu_mul(dotRot, floatSign);
 
		// Slerp rotations : evaluate angle = acos(cosAngle) (MacLaurin)
		vec_float4 oneMinusCosAngle = spu_sub(fOne, cosAngle);
		vec_float4 rsqTmp = spu_rsqrte(oneMinusCosAngle);	   
		vec_float4 evalTmp = spu_nmsub(rsqTmp, cosAngle, rsqTmp);
		vec_float4 acosEval3 = spu_mul(cosAngle, acos3);
		vec_float4 acosEval2 = spu_nmsub(cosAngle, acosEval3, acos2);
		vec_float4 acosEval1 = spu_nmsub(cosAngle, acosEval2, acos1);
		vec_float4 acosEval0 = spu_nmsub(cosAngle, acosEval1, acos0);
		vec_float4 angle = spu_mul(acosEval0, evalTmp);

		// Slerp rotations : evaluate sina = sin(angle) (Taylor)
		vec_float4 sqAngle = spu_mul(angle, angle);
		vec_float4 sinaEval3 = spu_madd(sin3, sqAngle, sin2);
		vec_float4 sinaEval2 = spu_madd(sinaEval3, sqAngle, sin1);
		vec_float4 sinaEval1 = spu_madd(sinaEval2, sqAngle, sin0);
		vec_float4 sinaEval0 = spu_madd(sinaEval1, sqAngle, fOne);
		vec_float4 sina = spu_mul(sinaEval0, angle);

		// Slerp rotations : evaluate sinb = sin(t*angle) (Taylor)
		vec_float4 tAngle = spu_mul(tween, angle);
		vec_float4 sqTAngle = spu_mul(tAngle, tAngle);
		vec_float4 sinbEval3 = spu_madd(sin3, sqTAngle, sin2);
		vec_float4 sinbEval2 = spu_madd(sinbEval3, sqTAngle, sin1);
		vec_float4 sinbEval1 = spu_madd(sinbEval2, sqTAngle, sin0);
		vec_float4 sinbEval0 = spu_madd(sinbEval1, sqTAngle, fOne);
		vec_float4 sinb = spu_mul(sinbEval0, tAngle);
		
		// Slerp rotations : evaluate sinc = sin((1-t)*angle) (Taylor)
		vec_float4 oneMinusT = spu_sub(fOne, tween);
		vec_float4 oneMinusTAngle = spu_mul(oneMinusT, angle);
		vec_float4 sqOneMinusTAngle = spu_mul(oneMinusTAngle, oneMinusTAngle);
		vec_float4 sincEval3 = spu_madd(sin3, sqOneMinusTAngle, sin2);
		vec_float4 sincEval2 = spu_madd(sincEval3, sqOneMinusTAngle, sin1);
		vec_float4 sincEval1 = spu_madd(sincEval2, sqOneMinusTAngle, sin0);
		vec_float4 sincEval0 = spu_madd(sincEval1, sqOneMinusTAngle, fOne);
		vec_float4 sinc = spu_mul(sincEval0, oneMinusTAngle);   

		// Select either sinc/sina or sinb/sina depending on the quadrant
		vec_float4 invSina = spu_re(sina);
		vec_float4 fLeftTmp = spu_mul(sinc, invSina);
		vec_float4 fRightTmp = spu_mul(sinb, invSina);
		vec_uchar16 shufRange = spu_sel(shufABCD, shufabcd, (vec_uchar16)spu_cmpgt(cosAngle, fOneMinusEpsilon));
		vec_float4 fLeft = spu_shuffle(fLeftTmp, oneMinusT, shufRange);
		vec_float4 fRight = spu_shuffle(fRightTmp, tween, shufRange);
	
		// Actual blend
		vec_float4 tmpRotX = spu_mul(leftRotX, fLeft);
		vec_float4 tmpRotY = spu_mul(leftRotY, fLeft);
		vec_float4 tmpRotZ = spu_mul(leftRotZ, fLeft);
		vec_float4 tmpRotW = spu_mul(leftRotW, fLeft);
		vec_float4 outRotX = spu_madd(targetRotX, fRight, tmpRotX);
		vec_float4 outRotY = spu_madd(targetRotY, fRight, tmpRotY);
		vec_float4 outRotZ = spu_madd(targetRotZ, fRight, tmpRotZ);
		vec_float4 outRotW = spu_madd(targetRotW, fRight, tmpRotW);

		// Unswizzle output rotation
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		vec_float4 outRotX0Y0X1Y1 = spu_shuffle(outRotX, outRotY, shufAaBb);
		vec_float4 outRotZ1W1Z0W0 = spu_shuffle(outRotZ, outRotW, shufBbAa);
		vec_float4 outRot0 = spu_shuffle(outRotX0Y0X1Y1, outRotZ1W1Z0W0, shufABcd);
		vec_float4 outRot1 = spu_shuffle(outRotX0Y0X1Y1, outRotZ1W1Z0W0, shufCDab);
		vec_float4 outRotX2Y2X3Y3 = spu_shuffle(outRotX, outRotY, shufCcDd);
		vec_float4 outRotZ3W3Z2W2 = spu_shuffle(outRotZ, outRotW, shufDdCc);
		vec_float4 outRot2 = spu_shuffle(outRotX2Y2X3Y3, outRotZ3W3Z2W2, shufABcd);
		vec_float4 outRot3 = spu_shuffle(outRotX2Y2X3Y3, outRotZ3W3Z2W2, shufCDab);	 

		// Write results (x4) 
		vec_float4* outAdr0 = output + channelIds[0] * 3;
		vec_float4* outAdr1 = output + channelIds[1] * 3;
		vec_float4* outAdr2 = output + channelIds[2] * 3;
		vec_float4* outAdr3 = output + channelIds[3] * 3;
		*outAdr0 = outRot0;
		*outAdr1 = outRot1;
		*outAdr2 = outRot2;
		*outAdr3 = outRot3;

		// Advance pointers
		vec_uint4 intraRInc0 = (vec_uint4)spu_sumb((vec_uchar16)spu_shuffle(numBits, numBits, shufDHLPDHLPDHLPDHLP), zero);
		vec_uint4 intraRInc = spu_add(spu_sl(intraRInc0, 2), spu_sl(intraRInc0, 1));
		intraRBitsAdrv = spu_add(intraRBitsAdrv, intraFrameCount*4);
		initialRAdrv = spu_add(initialRAdrv, 6*4);
		intraRAdrv = spu_add(intraRAdrv, intraRInc);
		finalRAdrv = spu_add(finalRAdrv, 6*4);
		channelIds += 4;
	}
}
