/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
{
	void _edgeAnimEvaluateRConst(uint32_t constCount, const void* constData, const uint16_t* constTable, __m128* output);
	void _edgeAnimEvaluateSTConst(uint32_t constCount, const void* constData, const uint16_t* constTable, __m128* output);
	void _edgeAnimEvaluateUserConst(uint32_t constCount,const void* constData,const uint16_t* constTable, float* outputUserChannels );
	void _edgeAnimEvaluateR(uint32_t animRCount, const uint16_t* animRTable, uintptr_t initialRAdr, uintptr_t intraRAdr, uintptr_t finalRAdr, uintptr_t intraBitsAdr, const __m128i& bitMask, const __m128i& prevBitMask, uint32_t intraFrameCount, uint32_t frameInteger, float frameFraction, __m128* output);
	void _edgeAnimEvaluateST(uint32_t animCount, const uint16_t* animTable, uintptr_t initialAdr, uintptr_t intraAdr, uintptr_t finalAdr, uintptr_t intraBitsAdr, uint32_t intraBitsOfs, const __m128i& bitMask, const __m128i& prevBitMask, uint32_t intraFrameCount, uint32_t frameInteger, float frameFraction, __m128* output);
	void _edgeAnimEvaluateUser(uint32_t animCount, const uint16_t* animTable, uintptr_t initialAdr, uintptr_t intraAdr, uintptr_t finalAdr, uintptr_t intraBitsAdr, uint32_t intraBitsOfs, const __m128i& bitMask, const __m128i& prevBitMask, uint32_t intraFrameCount, uint32_t frameInteger, float frameFraction, float* outputUserChannels);
	void _edgeAnimEvaluateBitPacked(uint32_t animCount, const uint16_t* animTable, const uint32_t* packingSpec, uintptr_t initialAdr, uintptr_t intraAdr, uintptr_t finalAdr, uintptr_t intraBitsAdr, uint32_t intraBitsOfs, const __m128i& bitMask, const __m128i& prevBitMask, uint32_t intraFrameCount, uint32_t frameInteger, float frameFraction, __m128* output, bool isRotation);
	void _edgeAnimEvaluateBitPackedConst(uint32_t constCount, const void* constData, const uint16_t* constTable, const uint32_t* packingSpec, __m128* output, bool isRotation);
	void _edgeAnimEvaluateBitPackedUser(uint32_t animCount, const uint16_t* animTable, const uint32_t* packingSpec, uintptr_t initialAdr, uintptr_t intraAdr, uintptr_t finalAdr, uintptr_t intraBitsAdr, uint32_t intraBitsOfs, const __m128i& bitMask, const __m128i& prevBitMask, uint32_t intraFrameCount, uint32_t frameInteger, float frameFraction, float* output);
	void _edgeAnimEvaluateBitPackedUserConst(uint32_t constCount, const void* constData, const uint16_t* constTable, const uint32_t* packingSpec, float* output);
};

static __forceinline bool _edgeAnimFindTableSpan(uint16_t entry, const uint16_t* table, uint32_t tableSize, uint32_t span, uint32_t& startIndex, uint32_t& resultIndex)
{
	for( startIndex = 0; startIndex < tableSize; startIndex += span )
	{
		for( resultIndex = 0; ( resultIndex < span ) && ( ( startIndex + resultIndex ) < tableSize ); resultIndex++ )
		{
			if( table[ startIndex + resultIndex ] == entry )
			{
				return true;
			}
		}
	}
	return false;
}

static __forceinline __m128i _edgeAnimGenerateBitMask(uint32_t bitPos)
{
	const __m128i kOn = _mm_set1_epi32( -1 );
	const __m128i selLo = _mm_cmplt_epi32( _mm_set1_epi32( bitPos ), _mm_set1_epi32( 64 ) );

	// Hi (bitPos >= 64)
	__m128i hi0 = _mm_slli_epi64( kOn, 128 - bitPos );
	__m128i hi1 = _mm_or_si128( hi0, _mm_srli_si128( kOn, 8 ) );
	__m128i hi  = _mm_andnot_si128( selLo, hi1 );

	// Lo (bitPos < 64)
	__m128i lo0 = _mm_srli_si128( kOn, 8 );
	__m128i lo1 = _mm_slli_epi64( lo0, 64 - bitPos );
	__m128i lo  = _mm_and_si128( selLo, lo1 );

	return _mm_shuffle_epi32( _mm_or_si128( hi, lo ), _MM_SHUFFLE( 2, 3, 0, 1 ) );
}

extern "C"
void _edgeAnimEvaluate(EdgeAnimJointTransform* outputJoints,
                       float* outputUserChannels,
                       const EdgeAnimAnimation* animHeader,
                       const void* frameSetData,
                       uint32_t intraFrameCount,
					   uint32_t frameInteger,
                       float frameFraction)
{
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

    EDGE_ASSERT(frameInteger <= intraFrameCount);

	uintptr_t initialRAdr = 0, initialTAdr = 0, initialSAdr = 0, initialUAdr = 0;
	uintptr_t intraRAdr = 0, intraTAdr = 0, intraSAdr = 0, intraUAdr = 0;
	uintptr_t finalRAdr = 0, finalTAdr = 0, finalSAdr = 0, finalUAdr = 0;
	uintptr_t intraBitsAdr = 0;

	// Get offsets to the various data blocks within this frameset
	// Note: if there are any animated channels then there must be at least two framesets, 
	// since the last frameset consists of just the final keys (no intra frames)
	if(animHeader->numFrameSets >= 2)
	{
		uint16_t* dataSizes = (uint16_t*)frameSetData;
		uint32_t sizeInitialRData = dataSizes[0];
		uint32_t sizeInitialTData = dataSizes[1];
		uint32_t sizeInitialSData = dataSizes[2];
		uint32_t sizeInitialUData = dataSizes[3];
		uint32_t sizeIntraRData = dataSizes[4];
		uint32_t sizeIntraTData = dataSizes[5];
		uint32_t sizeIntraSData = dataSizes[6];
		uint32_t sizeIntraUData = dataSizes[7];
		initialRAdr = (uintptr_t)dataSizes + 16;
		initialTAdr = initialRAdr + sizeInitialRData;
		initialSAdr = initialTAdr + sizeInitialTData;
		initialUAdr = initialSAdr + sizeInitialSData;
		intraBitsAdr = initialUAdr + sizeInitialUData;
		intraRAdr = intraBitsAdr + ((animRCount + animTCount + animSCount + animUCount) * intraFrameCount + 7) / 8;
		intraTAdr = intraRAdr + sizeIntraRData;
		intraSAdr = intraTAdr + sizeIntraTData;
		intraUAdr = EDGE_ALIGN(intraSAdr + sizeIntraSData, 4);
		uintptr_t nextFrameSetAdr = EDGE_ALIGN(intraUAdr + sizeIntraUData, 16);
		uint16_t* nextDataSizes = (uint16_t*)nextFrameSetAdr;
		uint32_t sizeFinalRData = nextDataSizes[0];
		uint32_t sizeFinalTData = nextDataSizes[1];
		uint32_t sizeFinalSData = nextDataSizes[2];
		finalRAdr = (uintptr_t)nextDataSizes + 16;
		finalTAdr = finalRAdr + sizeFinalRData;
		finalSAdr = finalTAdr + sizeFinalTData;
		finalUAdr = finalSAdr + sizeFinalSData;
	}

    // get channel tables
    const uint16_t* constRTable = animHeader->channelTables;
    const uint16_t* constTTable = constRTable + EDGE_ALIGN(constRCount, 8);
    const uint16_t* constSTable = constTTable + EDGE_ALIGN(constTCount, 4);
    const uint16_t* constUTable = constSTable + EDGE_ALIGN(constSCount, 4);
    const uint16_t* animRTable = constUTable + EDGE_ALIGN(constUCount, 4);
    const uint16_t* animTTable = animRTable + EDGE_ALIGN(animRCount, 4);
    const uint16_t* animSTable = animTTable + EDGE_ALIGN(animTCount, 4);
    const uint16_t* animUTable = animSTable + EDGE_ALIGN(animSCount, 4);

	// packing specs
	uint32_t* packingSpec = EDGE_OFFSET_GET_POINTER(uint32_t, animHeader->offsetPackingSpecs);

    // get bit addresses of the intra bitstreams
    uint32_t intraRBitsOfs = 0;
    uint32_t intraTBitsOfs = intraRBitsOfs + animRCount * intraFrameCount;
    uint32_t intraSBitsOfs = intraTBitsOfs + animTCount * intraFrameCount;
    uint32_t intraUBitsOfs = intraSBitsOfs + animSCount * intraFrameCount;

    // bitMask = 1 for bits < intraFrameCount
    __m128i bitMask = _edgeAnimGenerateBitMask(intraFrameCount);

    // prevBitMask = 1 for bits < frameInteger
    __m128i prevBitMask = _edgeAnimGenerateBitMask(frameInteger);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Rotations
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_R)
	{
		if( outputJoints )
		{
			// constant rotations - bit packed
			_edgeAnimEvaluateBitPackedConst( constRCount, constRData, constRTable, packingSpec, (__m128*)&outputJoints->rotation, true );

			// non-constant rotations - bit packed
			_edgeAnimEvaluateBitPacked( animRCount, animRTable, packingSpec + 1, initialRAdr, intraRAdr, finalRAdr, intraBitsAdr, intraRBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)&outputJoints->rotation, true );
		}

		packingSpec += ( animRCount + 1 );
	}
	else
	{
		if( outputJoints )
		{
			// constant rotations - 48 bit compressed
			_edgeAnimEvaluateRConst( constRCount, constRData, constRTable, (__m128*)&outputJoints->rotation );

			// non-constant rotations - 48 bit compressed
			_edgeAnimEvaluateR( animRCount, animRTable, initialRAdr, intraRAdr, finalRAdr, intraBitsAdr, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)&outputJoints->rotation );
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Translations
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_T)
	{
		if( outputJoints )
		{
			// constant translations - bit packed
			_edgeAnimEvaluateBitPackedConst( constTCount, constTData, constTTable, packingSpec, (__m128*)&outputJoints->translation, false );

			// non-constant translations - bit packed
			_edgeAnimEvaluateBitPacked( animTCount, animTTable, packingSpec + 1, initialTAdr, intraTAdr, finalTAdr, intraBitsAdr, intraTBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)&outputJoints->translation, false );
		}

		packingSpec += ( animTCount + 1 );
	}
	else
	{
		if( outputJoints )
		{
			// constant translations - uncompressed float3
			_edgeAnimEvaluateSTConst( constTCount, constTData, constTTable, (__m128*)&outputJoints->translation );

			// non-constant translations - uncompressed float3
			_edgeAnimEvaluateST( animTCount, animTTable, initialTAdr, intraTAdr, finalTAdr, intraBitsAdr, intraTBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)&outputJoints->translation );
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Scales
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_S)
	{
		if( outputJoints )
		{
			// constant scales - bit packed
			_edgeAnimEvaluateBitPackedConst( constSCount, constSData, constSTable, packingSpec, (__m128*)&outputJoints->scale, false );

			// non-constant scales - bit packed
			_edgeAnimEvaluateBitPacked( animSCount, animSTable, packingSpec + 1, initialSAdr, intraSAdr, finalSAdr, intraBitsAdr, intraSBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)&outputJoints->scale, false );
		}

		packingSpec += ( animSCount + 1 );
	}
	else
	{
		if( outputJoints )
		{
			// constant scales - uncompressed float3
			_edgeAnimEvaluateSTConst( constSCount, constSData, constSTable, (__m128*)&outputJoints->scale );

			// non-constant scales - uncompressed float3
			_edgeAnimEvaluateST( animSCount, animSTable, initialSAdr, intraSAdr, finalSAdr, intraBitsAdr, intraSBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)&outputJoints->scale );
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// User channels
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_U)
	{
		if(outputUserChannels)
		{
			// constant user channels - bit packed
			_edgeAnimEvaluateBitPackedUserConst( constUCount, constUData, constUTable, packingSpec, outputUserChannels );

			// non-constant user channels - bit packed
			_edgeAnimEvaluateBitPackedUser( animUCount, animUTable, packingSpec + 1, initialUAdr, intraUAdr, finalUAdr, intraBitsAdr, intraUBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, outputUserChannels );
		}

		packingSpec += ( animUCount + 1 );
	}
	else
	{
		if(outputUserChannels)
		{
			// constant user channels - uncompressed floats
			_edgeAnimEvaluateUserConst( constUCount, constUData, constUTable, outputUserChannels );

			// non-constant user channels - uncompressed floats
			_edgeAnimEvaluateUser( animUCount, animUTable, initialUAdr, intraUAdr, finalUAdr, intraBitsAdr, intraUBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, outputUserChannels );
		}
	}
}

void _edgeAnimEvaluateSingle(EdgeAnimJointTransform* outputJoint, 
							 float* outputUserChannel,
							 const EdgeAnimAnimation* anim, 
							 const EdgeAnimSkeleton* skel, 
							 uint16_t jointIndex,
							 uint16_t userChannelIndex,
							 float evalTime)
{
	// there is no implementation of the bitpacked evaluator
	EDGE_ASSERT(anim->offsetPackingSpecs == 0);

	// clamp to first frame
	float frame = evalTime * anim->sampleFrequency;
	if(__builtin_expect(frame < 0, false))
		frame = 0;

	// search current frameset (binary search)
	const EdgeAnimFrameSetInfo* infoArray = EDGE_OFFSET_GET_POINTER(EdgeAnimFrameSetInfo, anim->offsetFrameSetInfoArray);
	unsigned int leftIndex = 0;
	unsigned int rightIndex = anim->numFrameSets - 1;			 
	while (__builtin_expect((leftIndex + 1 < rightIndex), true)) {
		int midIndex = (leftIndex + rightIndex) >> 1;
		bool isSmaller = (uint32_t)frame < infoArray[midIndex].baseFrame;
		rightIndex = isSmaller? midIndex: rightIndex;
		leftIndex = isSmaller? leftIndex: midIndex;						
	}

	unsigned int frameSetIndex = leftIndex;
	uint32_t intraFrameCount = infoArray[frameSetIndex].numIntraFrames;
	uint32_t baseFrame = infoArray[frameSetIndex].baseFrame;

	// get integer & fractional parts of frame (relative to frameset)
	float frameSetFrame = frame - baseFrame;
	uint32_t frameInteger = (uint32_t)frameSetFrame;
	float frameFraction = frameSetFrame - (float)frameInteger;

	// clamp to last frame
	// (required for last frameset if we have gone beyond the end of the animation)
	if(frameInteger > intraFrameCount)
	{
		frameInteger = intraFrameCount;
		frameFraction = 1;
	}

	// get frameset
	const uint32_t* dmaArray = EDGE_OFFSET_GET_POINTER(uint32_t, anim->offsetFrameSetDmaArray);
	const void* frameSetData = (void*)(dmaArray[frameSetIndex*2 + 1] + (uintptr_t)anim);

	// 
	const void* constRData = EDGE_OFFSET_GET_POINTER(void, anim->offsetConstRData);
	const void* constTData = EDGE_OFFSET_GET_POINTER(void, anim->offsetConstTData);
	const void* constSData = EDGE_OFFSET_GET_POINTER(void, anim->offsetConstSData);
	const void* constUData = EDGE_OFFSET_GET_POINTER(void, anim->offsetConstUserData);

	uint32_t constRCount = anim->numConstRChannels;
	uint32_t constTCount = anim->numConstTChannels;
	uint32_t constSCount = anim->numConstSChannels;
	uint32_t constUCount = anim->numConstUserChannels;
	uint32_t animRCount = anim->numAnimRChannels;
	uint32_t animTCount = anim->numAnimTChannels;
	uint32_t animSCount = anim->numAnimSChannels;
	uint32_t animUCount = anim->numAnimUserChannels;

	EDGE_ASSERT(frameInteger <= intraFrameCount);

	uintptr_t initialRAdr = 0, initialTAdr = 0, initialSAdr = 0, initialUAdr = 0;
	uintptr_t intraRAdr = 0, intraTAdr = 0, intraSAdr = 0, intraUAdr = 0;
	uintptr_t finalRAdr = 0, finalTAdr = 0, finalSAdr = 0, finalUAdr = 0;
	uintptr_t intraBitsAdr = 0;

	// Get offsets to the various data blocks within this frameset
	// Note: if there are any animated channels then there must be at least two framesets, 
	// since the last frameset consists of just the final keys (no intra frames)
	if(anim->numFrameSets >= 2)
	{
		uint16_t* dataSizes = (uint16_t*)frameSetData;
		uint32_t sizeInitialRData = dataSizes[0];
		uint32_t sizeInitialTData = dataSizes[1];
		uint32_t sizeInitialSData = dataSizes[2];
		uint32_t sizeInitialUData = dataSizes[3];
		uint32_t sizeIntraRData = dataSizes[4];
		uint32_t sizeIntraTData = dataSizes[5];
		uint32_t sizeIntraSData = dataSizes[6];
		uint32_t sizeIntraUData = dataSizes[7];
		initialRAdr = (uintptr_t)dataSizes + 16;
		initialTAdr = initialRAdr + sizeInitialRData;
		initialSAdr = initialTAdr + sizeInitialTData;
		initialUAdr = initialSAdr + sizeInitialSData;
		intraBitsAdr = initialUAdr + sizeInitialUData;
		intraRAdr = intraBitsAdr + ((animRCount + animTCount + animSCount + animUCount) * intraFrameCount + 7) / 8;
		intraTAdr = intraRAdr + sizeIntraRData;
		intraSAdr = intraTAdr + sizeIntraTData;
		intraUAdr = EDGE_ALIGN(intraSAdr + sizeIntraSData, 4);
		uintptr_t nextFrameSetAdr = EDGE_ALIGN(intraUAdr + sizeIntraUData, 16);
		uint16_t* nextDataSizes = (uint16_t*)nextFrameSetAdr;
		uint32_t sizeFinalRData = nextDataSizes[0];
		uint32_t sizeFinalTData = nextDataSizes[1];
		uint32_t sizeFinalSData = nextDataSizes[2];
		finalRAdr = (uintptr_t)nextDataSizes + 16;
		finalTAdr = finalRAdr + sizeFinalRData;
		finalSAdr = finalTAdr + sizeFinalTData;
		finalUAdr = finalSAdr + sizeFinalSData;
	}

	// get channel tables
	const uint16_t* constRTable = anim->channelTables;
	const uint16_t* constTTable = constRTable + EDGE_ALIGN(constRCount, 8);
	const uint16_t* constSTable = constTTable + EDGE_ALIGN(constTCount, 4);
	const uint16_t* constUTable = constSTable + EDGE_ALIGN(constSCount, 4);
	const uint16_t* animRTable = constUTable + EDGE_ALIGN(constUCount, 4);
	const uint16_t* animTTable = animRTable + EDGE_ALIGN(animRCount, 4);
	const uint16_t* animSTable = animTTable + EDGE_ALIGN(animTCount, 4);
	const uint16_t* animUTable = animSTable + EDGE_ALIGN(animSCount, 4);

	// get bit addresses of the intra bitstreams
	uint32_t intraRBitsOfs = 0;
	uint32_t intraTBitsOfs = intraRBitsOfs + animRCount * intraFrameCount;
	uint32_t intraSBitsOfs = intraTBitsOfs + animTCount * intraFrameCount;
	uint32_t intraUBitsOfs = intraSBitsOfs + animSCount * intraFrameCount;

	// bitMask = 1 for bits < intraFrameCount
	__m128i bitMask = _edgeAnimGenerateBitMask(intraFrameCount);

	// prevBitMask = 1 for bits < frameInteger
	__m128i prevBitMask = _edgeAnimGenerateBitMask(frameInteger);

	// evaluate
	if( outputJoint )
	{
		uint32_t idxStart, idxTarget;
		EdgeAnimJointTransform output[2];

		// set output joint to base pose
		const EdgeAnimJointTransform* basePose = EDGE_OFFSET_GET_POINTER(EdgeAnimJointTransform, skel->offsetBasePose);
		output[1] = basePose[ jointIndex ];

		// rotation
		if( _edgeAnimFindTableSpan( jointIndex, constRTable, constRCount, 8, idxStart, idxTarget ) )
		{
			uint16_t table[8] = { 0,0,0,0,0,0,0,0 };
			table[ idxTarget ] = 1;
			_edgeAnimEvaluateRConst( 8, (uint8_t*)constRData + idxStart * 6, table, (__m128*)(&output->rotation) );
		}
		else if( _edgeAnimFindTableSpan( jointIndex, animRTable, animRCount, 4, idxStart, idxTarget ) )
		{
			uint32_t count = ( idxStart + 4 );
			size_t sz = count * sizeof(uint16_t);
			uint16_t *table = (uint16_t *) memset( alloca( sz ), 0, sz );
			table[ idxStart + idxTarget ] = 1;
			_edgeAnimEvaluateR( count, table, initialRAdr, intraRAdr, finalRAdr, intraBitsAdr, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)(&output->rotation) );
		} 

		// translation
		if( _edgeAnimFindTableSpan( jointIndex, constTTable, constTCount, 4, idxStart, idxTarget ) )
		{
			uint16_t table[4] = { 0,0,0,0 };
			table[ idxTarget ] = 1;
			_edgeAnimEvaluateSTConst( 4, (uint8_t*)constTData + idxStart * 12, table, (__m128*)(&output->translation) );
		}
		else if( _edgeAnimFindTableSpan( jointIndex, animTTable, animTCount, 4, idxStart, idxTarget ) )
		{
			uint32_t count = ( idxStart + 4 );
			size_t sz = count * sizeof(uint16_t);
			uint16_t *table = (uint16_t *) memset( alloca( sz ), 0, sz );
			table[ idxStart + idxTarget ] = 1;
			_edgeAnimEvaluateST( count, table, initialTAdr, intraTAdr, finalTAdr, intraBitsAdr, intraTBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)(&output->translation) );
		}

		// scale
		if( _edgeAnimFindTableSpan( (uint16_t)jointIndex, constSTable, constSCount, 4, idxStart, idxTarget ) )
		{
			uint16_t table[4] = { 0,0,0,0 };
			table[ idxTarget ] = 1;
			_edgeAnimEvaluateSTConst( 4, (uint8_t*)constSData + idxStart * 12, table, (__m128*)(&output->scale) );
		}
		else if( _edgeAnimFindTableSpan( jointIndex, animSTable, animSCount, 4, idxStart, idxTarget ) )
		{
			uint32_t count = ( idxStart + 4 );
			size_t sz = count * sizeof(uint16_t);
			uint16_t *table = (uint16_t *) memset( alloca( sz ), 0, sz );
			table[ idxStart + idxTarget ] = 1;
			_edgeAnimEvaluateST( count, table, initialSAdr, intraSAdr, finalSAdr, intraBitsAdr, intraSBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, (__m128*)(&output->scale) );
		}

		*outputJoint = output[1];
	}

	if( outputUserChannel )
	{
		uint32_t idxStart, idxTarget;

		// set output joint to zero by default
		float output[2] = { 0.0f, 0.0f };

		if( _edgeAnimFindTableSpan( userChannelIndex, constUTable, constUCount, 4, idxStart, idxTarget ) )
		{
			uint16_t uTable[4] = { 0,0,0,0 };
			uTable[ idxTarget ] = 1;
			_edgeAnimEvaluateUserConst( 4, (float*)constUData + idxStart, uTable, output );
		}
		else if( _edgeAnimFindTableSpan( userChannelIndex, animUTable, animUCount, 4, idxStart, idxTarget ) )
		{
			uint32_t count = ( idxStart + 4 );
			size_t sz = count * sizeof(uint16_t);
			uint16_t *table = (uint16_t *) memset( alloca( sz ), 0, sz );
			table[ idxStart + idxTarget ] = 1;
			_edgeAnimEvaluateUser( count, table, initialUAdr, intraUAdr, finalUAdr, intraBitsAdr, intraUBitsOfs, bitMask, prevBitMask, intraFrameCount, frameInteger, frameFraction, output );
		}

		*outputUserChannel = output[1];
	}
}

extern "C"
void edgeAnimEvaluateJoint(EdgeAnimJointTransform* outputJoint, 
						   const EdgeAnimAnimation* anim, 
						   const EdgeAnimSkeleton* skel, 
						   uint32_t jointIndex,
						   float evalTime)
{
	_edgeAnimEvaluateSingle(outputJoint, NULL, anim, skel, (uint16_t)jointIndex, 0, evalTime);
}

extern "C"
float edgeAnimEvaluateUserChannel(const EdgeAnimAnimation* anim, 
								  const EdgeAnimSkeleton* skel, 
								  uint32_t channelIndex,
								  float evalTime)
{
	float r;
	_edgeAnimEvaluateSingle(NULL, &r, anim, skel, 0, (uint16_t)channelIndex, evalTime);
	return r;
}
