/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_spu.h"

extern "C"
{
	void _edgeAnimEvaluateR(vec_uint4 intraRBitsAdrv, vec_uchar16 bitMaskXXXX, vec_uchar16 bitMaskYYYY, vec_uchar16 bitMaskZZZZ, vec_uchar16 bitMaskWWWW, vec_uchar16 prevBitMaskXXXX, vec_uchar16 prevBitMaskYYYY, vec_uchar16 prevBitMaskZZZZ, vec_uchar16 prevBitMaskWWWW, vec_uint4 frameIntegerv, vec_float4 frameFractionv, vec_uint4 intraRAdrv, vec_uint4 initialRAdrv, vec_uint4 finalRAdrv, uint32_t count, vec_float4* output, const uint16_t* __restrict__ channelIds, uint32_t intraFrameCount);
	void _edgeAnimEvaluateST(vec_uint4 intraTBitsAdrv, vec_uchar16 bitMaskXXXX, vec_uchar16 bitMaskYYYY, vec_uchar16 bitMaskZZZZ, vec_uchar16 bitMaskWWWW, vec_uchar16 prevBitMaskXXXX, vec_uchar16 prevBitMaskYYYY, vec_uchar16 prevBitMaskZZZZ, vec_uchar16 prevBitMaskWWWW, vec_uint4 frameIntegerv, vec_float4 frameFractionv, vec_uint4 intraTAdrv, vec_uint4 initialTAdrv, vec_uint4 finalTAdrv, uint32_t count, vec_float4* output, const uint16_t* __restrict__ channelIds, uint32_t intraFrameCount);
	void _edgeAnimEvaluateUser(vec_uint4 intraUserBitsAdrv, vec_uchar16 bitMaskXXXX, vec_uchar16 bitMaskYYYY, vec_uchar16 bitMaskZZZZ, vec_uchar16 bitMaskWWWW, vec_uchar16 prevBitMaskXXXX, vec_uchar16 prevBitMaskYYYY, vec_uchar16 prevBitMaskZZZZ, vec_uchar16 prevBitMaskWWWW, vec_uint4 frameIntegerv, vec_float4 frameFractionv, vec_uint4 intraUserAdrv, vec_uint4 initialUserAdrv, vec_uint4 finalUserAdrv, uint32_t count, float* userOutput, const uint16_t* __restrict__ channelIds, uint32_t intraFrameCount);
	void _edgeAnimEvaluateRConst(vec_float4* output, const vec_uint4* constRData, uint32_t count, const uint16_t* constRTable, const vec_uchar16* shufTable);
	void _edgeAnimEvaluateBitPacked(vec_uint4 intraBitsAdr, vec_uchar16 bitMaskXXXX, vec_uchar16 bitMaskYYYY, vec_uchar16 bitMaskZZZZ, vec_uchar16 bitMaskWWWW, vec_uchar16 prevBitMaskXXXX, vec_uchar16 prevBitMaskYYYY, vec_uchar16 prevBitMaskZZZZ, vec_uchar16 prevBitMaskWWWW, vec_uint4 frameIntegerv, vec_float4 frameFractionv, uint32_t intraKeysAdr, uint32_t initialKeysAdr, uint32_t finalKeysAdr, uint32_t count, vec_float4* output, const uint16_t* channelIds, uint32_t intraFrameCount, const uint32_t* specs, bool rotation);
	void _edgeAnimEvaluateBitPackedConst(uint32_t keysAdr, uint32_t count, vec_float4* output, const uint16_t* channelIds, const uint32_t* spec, bool rotation);
	void _edgeAnimEvaluateBitPackedUser(vec_uint4 intraBitsAdr, vec_uchar16 bitMaskXXXX, vec_uchar16 bitMaskYYYY, vec_uchar16 bitMaskZZZZ, vec_uchar16 bitMaskWWWW, vec_uchar16 prevBitMaskXXXX, vec_uchar16 prevBitMaskYYYY, vec_uchar16 prevBitMaskZZZZ, vec_uchar16 prevBitMaskWWWW, vec_uint4 frameIntegerv, vec_float4 frameFractionv, uint32_t intraKeysAdr, uint32_t initialKeysAdr, uint32_t finalKeysAdr, uint32_t count, float* output, const uint16_t* channelIds, uint32_t intraFrameCount, const uint32_t* specs);
	void _edgeAnimEvaluateBitPackedUserConst(uint32_t keysAdr, uint32_t count, float* output, const uint16_t* channelIds, const uint32_t* spec);
};

static void _edgeAnimEvaluateSTConst(const vec_uint4* constData, const uint16_t* channelIds, uint32_t count, vec_float4* output)
{
	const vec_uchar16 shufDab0 = {0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x80,0x80,0x80,0x80};
	const vec_uchar16 shufCDa0 = {0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x80,0x80,0x80,0x80};
	const vec_uchar16 shufBCD0 = {0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x80,0x80,0x80,0x80};
	const vec_uint4 mask_000X = {0x00000000, 0x00000000, 0x00000000, 0xffffffff};

	const vec_float4 fOne = {1, 1, 1, 1};

	uint32_t count4 = (count + 3) >> 2;

	for(; count4; count4--) {
		vec_uint4 qw0 = constData[0];
		vec_uint4 qw1 = constData[1];
		vec_uint4 qw2 = constData[2];

		vec_float4 v0 = (vec_float4)qw0;
		vec_float4 v1 = (vec_float4)spu_shuffle(qw0, qw1, shufDab0);
		vec_float4 v2 = (vec_float4)spu_shuffle(qw1, qw2, shufCDa0);
		vec_float4 v3 = (vec_float4)spu_shuffle(qw2, qw2, shufBCD0);

		// set w component to 1.0f
		v0 = spu_sel(v0, fOne, mask_000X);
		v1 = spu_sel(v1, fOne, mask_000X);
		v2 = spu_sel(v2, fOne, mask_000X);
		v3 = spu_sel(v3, fOne, mask_000X);

		vec_float4* outAdr0 = output + channelIds[0] * 3;
		vec_float4* outAdr1 = output + channelIds[1] * 3;
		vec_float4* outAdr2 = output + channelIds[2] * 3;
		vec_float4* outAdr3 = output + channelIds[3] * 3;

		outAdr0[0] = v0;
		outAdr1[0] = v1;
		outAdr2[0] = v2;
		outAdr3[0] = v3;

		// Advance pointers
		channelIds += 4;
		constData += 3;
	}
}

static void _edgeAnimEvaluateUserConst(const float* constData, const uint16_t* channelIds, uint32_t count, float* output)
{
	for(; count; count--) {
		output[*channelIds++] = *constData++;
	}
}

/**
 * @brief   Evaluate an abitrary (non-integral) frame in an animation frameset. 
 *
 *          - All animated joints and user channels are populated by this function.
 *          - Non-animated joint or channels contain unknown data, and their weight
 *            is set to 0.
 *
 *          This function is the main evaluation call and subsequently calls all 
 *          _edgeAnimEvaluate* functions. 
 *
 * @see     See LibEdge-Overview_e.doc "Animation Encoding and Compression" for more details 
 *          on the frameset data structure.
 *
 * @internal
 *
 * @param   outputJoints            Output joint array.  Must be aligned to 16 bytes. 
 *                                  Due to the SIMD nature of the processing, the 
 *                                  outputJoints array must be a multiple of 4 entries 
 *                                  in size. 
 * @param   outputUserChannels      Pointer to output user channel array. 
 *                                  Must be aligned to 16 bytes.
 *                                  Due to the SIMD nature of the processing, the 
 *                                  outputChannels array must be a multiple of 4 entries 
 *                                  in size. 
 * @param   animHeader              Animation header
 * @param   frameSetData            The frameset raw data block as exported by edge tools
 * @param   intraFrameCount         Number of intra frames inside this frame
 * @param   frameInteger            Frame index. This value must be within the limits of
 *                                  this frameset : 0 <= frameInteger <= intraFrameCount
 * @param   frameFraction           Fractional part of frame index [0,1]. Used for
 *                                  keyframe tweening
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_evaluate.cpp
 *
 */
extern "C"
void _edgeAnimEvaluate(EdgeAnimJointTransform* outputJoints, 
					   float* outputUserChannels,
					   const EdgeAnimAnimation* animHeader,
					   const void* frameSetData, 
					   uint32_t intraFrameCount,
					   uint32_t frameInteger,
					   float frameFraction)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_EVALUATE);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));

	const vec_uint4 _0123 = {0, 1, 2, 3};
	const vec_uchar16 allOnes   = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	const vec_uchar16 shufAAAA = {0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03};
	const vec_uchar16 shufBBBB = {0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07};
	const vec_uchar16 shufCCCC = {0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b};
	const vec_uchar16 shufDDDD = {0x0c,0x0d,0x0e,0x0f,0x0c,0x0d,0x0e,0x0f,0x0c,0x0d,0x0e,0x0f,0x0c,0x0d,0x0e,0x0f};

	const vec_uchar16 quatShufTable[4] = {
		{0x0c,0x0d,0x0e,0x0f,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b},			// shufDABC
		{0x00,0x01,0x02,0x03,0x0c,0x0d,0x0e,0x0f,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b},			// shufADBC
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x0c,0x0d,0x0e,0x0f,0x08,0x09,0x0a,0x0b},			// shufABDC
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f},			// shufABCD
	};

	const void* constRData = EDGE_OFFSET_GET_POINTER(void, animHeader->offsetConstRData);
	const void* constTData = EDGE_OFFSET_GET_POINTER(void, animHeader->offsetConstTData);
	const void* constSData = EDGE_OFFSET_GET_POINTER(void, animHeader->offsetConstSData);
	const void* constUData = EDGE_OFFSET_GET_POINTER(void, animHeader->offsetConstUserData);

	uint32_t constRCount = animHeader->numConstRChannels;
	uint32_t constTCount = animHeader->numConstTChannels;
	uint32_t constSCount = animHeader->numConstSChannels;
	uint32_t constUCount = animHeader->numConstUserChannels;
	uint32_t animRCount = animHeader->numAnimRChannels;
	uint32_t animTCount = animHeader->numAnimTChannels;
	uint32_t animSCount = animHeader->numAnimSChannels;
	uint32_t animUCount = animHeader->numAnimUserChannels;

	vec_uint4 frameIntegerv = spu_splats(frameInteger);
	vec_float4 frameFractionv = spu_splats(frameFraction);

	EDGE_ASSERT(frameInteger <= intraFrameCount);

	vec_float4* __restrict__ output = (vec_float4* __restrict__) outputJoints;
	float* __restrict__ userOutput = (float* __restrict__) outputUserChannels;

	// Get offsets to the various data blocks within this frameset
	uint16_t* dataSizes = (uint16_t*)frameSetData;
	uint32_t sizeInitialRData = dataSizes[0];
	uint32_t sizeInitialTData = dataSizes[1];
	uint32_t sizeInitialSData = dataSizes[2];
	uint32_t sizeInitialUData = dataSizes[3];
	uint32_t sizeIntraRData = dataSizes[4];
	uint32_t sizeIntraTData = dataSizes[5];
	uint32_t sizeIntraSData = dataSizes[6];
	uint32_t sizeIntraUData = dataSizes[7];
	uint32_t initialRAdr = (uint32_t)dataSizes + 16;
	uint32_t initialTAdr = initialRAdr + sizeInitialRData;
	uint32_t initialSAdr = initialTAdr + sizeInitialTData;
	uint32_t initialUAdr = initialSAdr + sizeInitialSData;
	uint32_t intraBitsAdr = initialUAdr + sizeInitialUData;
	uint32_t intraRAdr = intraBitsAdr + ((animRCount + animTCount + animSCount + animUCount) * intraFrameCount + 7) / 8;
	uint32_t intraTAdr = intraRAdr + sizeIntraRData;
	uint32_t intraSAdr = intraTAdr + sizeIntraTData;
	uint32_t intraUAdr = EDGE_ALIGN(intraSAdr + sizeIntraSData, 4);
	uint32_t nextFrameSetAdr = EDGE_ALIGN(intraUAdr + sizeIntraUData, 16);
	uint16_t* nextDataSizes = (uint16_t*)nextFrameSetAdr;
	uint32_t sizeFinalRData = nextDataSizes[0];
	uint32_t sizeFinalTData = nextDataSizes[1];
	uint32_t sizeFinalSData = nextDataSizes[2];
	uint32_t finalRAdr = (uint32_t)nextDataSizes + 16;
	uint32_t finalTAdr = finalRAdr + sizeFinalRData;
	uint32_t finalSAdr = finalTAdr + sizeFinalTData;
	uint32_t finalUAdr = finalSAdr + sizeFinalSData;

	// get channel tables
	const uint16_t* __restrict__ constRTable = animHeader->channelTables;
	const uint16_t* __restrict__ constTTable = constRTable + EDGE_ALIGN(constRCount, 8);
	const uint16_t* __restrict__ constSTable = constTTable + EDGE_ALIGN(constTCount, 4);
	const uint16_t* __restrict__ constUTable = constSTable + EDGE_ALIGN(constSCount, 4);
	const uint16_t* __restrict__ animRTable = constUTable + EDGE_ALIGN(constUCount, 4);
	const uint16_t* __restrict__ animTTable = animRTable + EDGE_ALIGN(animRCount, 4);
	const uint16_t* __restrict__ animSTable = animTTable + EDGE_ALIGN(animTCount, 4);
	const uint16_t* __restrict__ animUTable = animSTable + EDGE_ALIGN(animSCount, 4);

	// get bit addresses of the intra bitstreams
	uint32_t intraRBitsAdr = intraBitsAdr * 8;
	uint32_t intraTBitsAdr = intraRBitsAdr + animRCount * intraFrameCount;
	uint32_t intraSBitsAdr = intraTBitsAdr + animTCount * intraFrameCount;
	uint32_t intraUBitsAdr = intraSBitsAdr + animSCount * intraFrameCount;

	vec_uint4 intraInc = spu_mulo((vec_ushort8)_0123, intraFrameCount);
	vec_uint4 intraRBitsAdrv = spu_add(intraInc, intraRBitsAdr);
	vec_uint4 intraTBitsAdrv = spu_add(intraInc, intraTBitsAdr);
	vec_uint4 intraSBitsAdrv = spu_add(intraInc, intraSBitsAdr);
	vec_uint4 intraUBitsAdrv = spu_add(intraInc, intraUBitsAdr);

	// prevBitMask = 1 for bits < frameInteger
	int32_t mskShft = 128 - frameInteger;
	vec_uchar16 prevBitMask = spu_slqwbytebc(allOnes, mskShft);
	prevBitMask = spu_slqw(prevBitMask, mskShft);
	vec_uchar16 prevBitMaskXXXX = spu_shuffle(prevBitMask, prevBitMask, shufAAAA);
	vec_uchar16 prevBitMaskYYYY = spu_shuffle(prevBitMask, prevBitMask, shufBBBB);
	vec_uchar16 prevBitMaskZZZZ = spu_shuffle(prevBitMask, prevBitMask, shufCCCC);
	vec_uchar16 prevBitMaskWWWW = spu_shuffle(prevBitMask, prevBitMask, shufDDDD);

	// bitMask = 1 for bits < intraFrameCount
	mskShft = 128 - intraFrameCount;
	vec_uchar16 bitMask = spu_slqwbytebc(allOnes, mskShft);
	bitMask = spu_slqw(bitMask, mskShft);
	vec_uchar16 bitMaskXXXX = spu_shuffle(bitMask, bitMask, shufAAAA);
	vec_uchar16 bitMaskYYYY = spu_shuffle(bitMask, bitMask, shufBBBB);
	vec_uchar16 bitMaskZZZZ = spu_shuffle(bitMask, bitMask, shufCCCC);
	vec_uchar16 bitMaskWWWW = spu_shuffle(bitMask, bitMask, shufDDDD);

	vec_uint4 RInc = spu_mulo((vec_ushort8)_0123, 6);
	vec_uint4 TInc = spu_mulo((vec_ushort8)_0123, 12);
	vec_uint4 SInc = spu_mulo((vec_ushort8)_0123, 12);
	vec_uint4 UInc = spu_mulo((vec_ushort8)_0123, 4);

	vec_uint4 initialRAdrv = spu_add(RInc, initialRAdr);
	vec_uint4 initialTAdrv = spu_add(TInc, initialTAdr);
	vec_uint4 initialSAdrv = spu_add(SInc, initialSAdr);
	vec_uint4 initialUAdrv = spu_add(UInc, initialUAdr);
	vec_uint4 finalRAdrv = spu_add(RInc, finalRAdr);
	vec_uint4 finalTAdrv = spu_add(TInc, finalTAdr);
	vec_uint4 finalSAdrv = spu_add(SInc, finalSAdr);
	vec_uint4 finalUAdrv = spu_add(UInc, finalUAdr);
	vec_uint4 intraRAdrv = spu_splats(intraRAdr);
	vec_uint4 intraTAdrv = spu_splats(intraTAdr);
	vec_uint4 intraSAdrv = spu_splats(intraSAdr);
	vec_uint4 intraUAdrv = spu_splats(intraUAdr);

	uint32_t* packingSpec = EDGE_OFFSET_GET_POINTER(uint32_t, animHeader->offsetPackingSpecs);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Rotations
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_R)
	{
		// bit packed
		_edgeAnimEvaluateBitPackedConst((uint32_t)constRData, constRCount, output, constRTable, packingSpec, true);

		_edgeAnimEvaluateBitPacked(intraRBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraRAdr,
			initialRAdr, finalRAdr, animRCount, output, animRTable, intraFrameCount, packingSpec+1, true);

		packingSpec += animRCount + 1;
	}
	else
	{
		// 48bit "smallest 3" compression
		_edgeAnimEvaluateRConst(output, (vec_uint4*)constRData, constRCount, constRTable, quatShufTable);

		_edgeAnimEvaluateR(intraRBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraRAdrv,
			initialRAdrv, finalRAdrv, animRCount, output, animRTable, intraFrameCount);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Translations
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_T)
	{
		// bit packed
		_edgeAnimEvaluateBitPackedConst((uint32_t)constTData, constTCount, output + 1, constTTable, packingSpec, false);

		_edgeAnimEvaluateBitPacked(intraTBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraTAdr,
			initialTAdr, finalTAdr, animTCount, output + 1, animTTable, intraFrameCount, packingSpec+1, false);

		packingSpec += animTCount + 1;
	}
	else
	{
		// uncompressed floats
		_edgeAnimEvaluateSTConst((vec_uint4*)constTData, constTTable, constTCount, output+1);

		_edgeAnimEvaluateST(intraTBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraTAdrv,
			initialTAdrv, finalTAdrv, animTCount, output + 1, animTTable, intraFrameCount);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Scales
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_S)
	{
		// bit packed
		_edgeAnimEvaluateBitPackedConst((uint32_t)constSData, constSCount, output + 2, constSTable, packingSpec, false);

		_edgeAnimEvaluateBitPacked(intraSBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraSAdr,
			initialSAdr, finalSAdr, animSCount, output + 2, animSTable, intraFrameCount, packingSpec+1, false);

		packingSpec += animSCount + 1;
	}
	else
	{
		// uncompressed floats
		_edgeAnimEvaluateSTConst((vec_uint4*)constSData, constSTable, constSCount, output+2);

		_edgeAnimEvaluateST(intraSBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraSAdrv,
			initialSAdrv, finalSAdrv, animSCount, output + 2, animSTable, intraFrameCount);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// User channels
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_U)
	{
		// bit packed
		_edgeAnimEvaluateBitPackedUserConst((uint32_t)constUData, constUCount, userOutput, constUTable, packingSpec);

		_edgeAnimEvaluateBitPackedUser(intraUBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraUAdr,
			initialUAdr, finalUAdr, animUCount, userOutput, animUTable, intraFrameCount, packingSpec+1);

		packingSpec += animUCount + 1;
	}
	else
	{
		// uncompressed floats
		_edgeAnimEvaluateUserConst((float*)constUData, constUTable, constUCount, userOutput);

		_edgeAnimEvaluateUser(intraUBitsAdrv, bitMaskXXXX, bitMaskYYYY, bitMaskZZZZ, bitMaskWWWW, prevBitMaskXXXX,
			prevBitMaskYYYY, prevBitMaskZZZZ, prevBitMaskWWWW, frameIntegerv, frameFractionv, intraUAdrv,
			initialUAdrv, finalUAdrv, animUCount, userOutput, animUTable, intraFrameCount);
	}
}
