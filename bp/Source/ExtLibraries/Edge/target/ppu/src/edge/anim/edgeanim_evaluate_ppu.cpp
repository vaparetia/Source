/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu2vmx.h>
#include <string.h>

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_macros.h"
#include "edge/anim/edgeanim_structs_ps3.h"


/*static __inline vec_float4 spu_sel(vec_float4 a, vec_float4 b, vec_uchar16 pattern)
{
  return ((vec_float4)(si_selb((qword)(a), (qword)(b), (qword)(pattern))));
}*/

static int32_t _edgeAnimFindTableEntry(uint16_t entry, const uint16_t* table, uint32_t tableSize)
{
	uint32_t loop;
	for(loop = 0; loop < tableSize; loop++)
	{
		if(table[loop] == entry)
			break;
	}

	return (loop < tableSize ? (int32_t)loop : -1);
}

static uint32_t _edgeAnimCountEnabledBits(uint32_t adr, uint32_t startBit, uint32_t numBits)
{
    const vec_uchar16 zero  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    const vec_uchar16 allOnes = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    const vec_uchar16 shufSum  = {0x03,0x07,0x0b,0x0f,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};

    uint32_t adr0 = (adr + (startBit >> 3)) & (~15);
    uint32_t adr1 = (adr + ((startBit + numBits) >> 3)) & (~15);

    int32_t qwc = (adr1 - adr0) >> 4;

    int32_t bitOfs0 = ((adr << 3) + startBit) & 127;
    vec_uchar16 msk0a = spu_slqwbytebc(allOnes, 128-bitOfs0);
    vec_uchar16 msk0b = spu_slqw(msk0a, 128-bitOfs0);
    msk0b = spu_nor(msk0b, msk0b);

    int32_t bitOfs1 = (bitOfs0 + numBits) & 127;
    vec_uchar16 msk1a = spu_slqwbytebc(allOnes, 128-bitOfs1);
    vec_uchar16 msk1b = spu_slqw(msk1a, 128-bitOfs1);

    if(qwc == 0)
        msk0b = spu_and(msk0b, msk1b);

    const vec_uchar16* __restrict__ qwAdr = (vec_uchar16*)adr0;

    uint32_t numEnabledBits = 0;

    vec_uchar16 qw0 = *qwAdr++;
    vec_uchar16 qw = spu_and(qw0, msk0b);
    vec_uchar16 n = spu_cntb(qw);
    vec_uchar16 sum = (vec_uchar16)spu_sumb(n, zero);
    vec_uchar16 sum2 = spu_shuffle(sum, sum, shufSum);
    numEnabledBits += si_to_uint((vec_uchar16)spu_sumb(sum2, zero));

    for(int32_t i=0; i<qwc-1; i++) {
        qw = *qwAdr++;
        n = spu_cntb(qw);
        sum = (vec_uchar16)spu_sumb(n, zero);
        sum2 = spu_shuffle(sum, sum, shufSum);
        numEnabledBits += si_to_uint((vec_uchar16)spu_sumb(sum2, zero));
    }

    if(qwc > 0) {
        vec_uchar16 qw1 = *qwAdr++;
        qw = spu_and(qw1, msk1b);
        n = spu_cntb(qw);
        sum = (vec_uchar16)spu_sumb(n, zero);
        sum2 = spu_shuffle(sum, sum, shufSum);
        numEnabledBits += si_to_uint((vec_uchar16)spu_sumb(sum2, zero));
    }

    return numEnabledBits;
}

static inline int32_t _edgeAnimGetFirstSetBit(vec_uchar16 bits)
{
	vec_uint4 clz = spu_cntlz((vec_uint4)bits);
	uint32_t shift = spu_extract( spu_cntlz( spu_gather( (vec_uchar16)spu_cmpgt((vec_uint4)bits, (vec_uint4)vec_splat_u32((unsigned int)0) ) ) ), 0) - 16;
	return (int32_t)( (shift<<3) + spu_extract( spu_slqwbyte( clz, shift ), 0 ) );
}

static inline int32_t _edgeAnimGetLastSetBit(vec_uchar16 bits)
{
	static const vec_char16 kLastBitTable = (vec_char16){96, 64, 96, -33, 96, 64, 96, 32, 96, 64, 96, 0, 96, 64, 96, 32}; 
	uint32_t gbb_nzbytes = spu_extract( spu_gather( spu_cmpgt((vec_uint4)bits, (vec_uint4)vec_splat_u32((unsigned int)0)) ), 0 );
	int32_t iIndex = (int32_t)spu_extract( (vec_char16)spu_rlqwbyte(kLastBitTable, gbb_nzbytes), 3 );
	vec_uint4 last_nzword = spu_rlqwbytebc((vec_uint4)bits, iIndex);
	return iIndex + spu_extract( spu_cntlz( spu_and(last_nzword, spu_sub(0, last_nzword)) ), 0 );
}

static void _edgeAnimDecompressQuat(Vectormath::Aos::Quat* pRes, uint64_t q)
{
	static const float kSqrt2 = 1.414213562f;
	static const float kQuatScale = ((1 << 15) - 1) / kSqrt2;
	static const float kQuatOffset = kQuatScale / kSqrt2;

	unsigned int a = (unsigned int)((q >> 32) & ((1<<15)-1));
	unsigned int b = (unsigned int)((q >> 17) & ((1<<15)-1));
	unsigned int c = (unsigned int)((q >> 2) & ((1<<15)-1));
	unsigned int idx = (unsigned int)(q & 3);

	float fa = ((float)a - kQuatOffset) / kQuatScale;
	float fb = ((float)b - kQuatOffset) / kQuatScale;
	float fc = ((float)c - kQuatOffset) / kQuatScale;
	float fd = sqrtf(1 - fa*fa - fb*fb - fc*fc);

	switch(idx)
	{
	case 0:
		*pRes = Vectormath::Aos::Quat(fd, fa, fb, fc);
		break;
	case 1:
		*pRes = Vectormath::Aos::Quat(fa, fd, fb, fc);
		break;
	case 2:
		*pRes = Vectormath::Aos::Quat(fa, fb, fd, fc);
		break;
	case 3:
		*pRes = Vectormath::Aos::Quat(fa, fb, fc, fd);
		break;
	}
}

static void _edgeAnimDecompressRotation(Vectormath::Aos::Quat* pR, const uint8_t* compressed)
{
	uint64_t q64 = ((uint64_t)compressed[0] << 40) | 
				   ((uint64_t)compressed[1] << 32) | 
				   ((uint64_t)compressed[2] << 24) | 
				   ((uint64_t)compressed[3] << 16) | 
				   ((uint64_t)compressed[4] << 8) | 
				   ((uint64_t)compressed[5]);

	_edgeAnimDecompressQuat(pR, q64);
}

static vec_float4 _edgeAnimDecompressFloat3(const uint8_t* compressed)
{
    const vec_uchar16 allOnes = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

    vec_uint4 q0 = ((vec_uint4*)compressed)[0];
    vec_uint4 q1 = ((vec_uint4*)compressed)[1];
    uint32_t shift = uint32_t(compressed) & 15;
	vec_uint4 mask = spu_slqwbyte((vec_uint4)allOnes, shift);
	vec_uint4 shifted0 = spu_rlqwbyte(q0, shift);
	vec_uint4 shifted1 = spu_rlqwbyte(q1, shift);
    vec_float4 v = (vec_float4)spu_sel(shifted1, shifted0, mask);

	return v;
}

static uint32_t _edgeAnimGetFrameSetIndex(const EdgeAnimAnimation* anim, float frame)
{
	uint32_t frameInteger = (uint32_t)frame;

	const EdgeAnimFrameSetInfo* infoArray = EDGE_OFFSET_GET_POINTER(EdgeAnimFrameSetInfo, anim->offsetFrameSetInfoArray);

    unsigned int leftIndex = 0;
    unsigned int rightIndex = anim->numFrameSets - 1;             

    while (__builtin_expect((leftIndex + 1 != rightIndex), true)) {
        int midIndex = (leftIndex + rightIndex) >> 1;
        if (frameInteger < infoArray[midIndex].baseFrame)
            rightIndex = midIndex;
        else
            leftIndex = midIndex;
    }

    return leftIndex;
}

static float _edgeAnimGetBracketingKeyframes(uint8_t** keyA, uint8_t** keyB,
									  		 uint32_t frameInteger, float frameFraction, uint32_t intraFrameCount,
									  		 uint32_t channelId,	uint32_t stride, uint32_t intraBitsAdr, uint32_t intraBitsOfs,
									  		 uint32_t initialAdr, uint32_t intraAdr, uint32_t finalAdr)
{
    const vec_uchar16 zero = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    const vec_uchar16 allOnes = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    const vec_uchar16 shufSum = {0x03,0x07,0x0b,0x0f,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};

	// prevBitMask = 1 for bits < frameInteger
	int32_t mskShft = 128 - frameInteger;
	vec_uchar16 prevBitMask = spu_slqwbytebc(allOnes, mskShft);
	prevBitMask = spu_slqw(prevBitMask, mskShft);

	// bitMask = 1 for bits < intraFrameCount
	mskShft = 128 - intraFrameCount;
	vec_uchar16 bitMask = spu_slqwbytebc(allOnes, mskShft);
	bitMask = spu_slqw(bitMask, mskShft);

	// count number of enabled intra bits for all previous channels to get the start address of the intra frame R data
	uint32_t numEnabledIntraBits = _edgeAnimCountEnabledBits(intraBitsAdr, intraBitsOfs, channelId * intraFrameCount);
	intraAdr += numEnabledIntraBits * stride;
	intraBitsOfs += channelId * intraFrameCount;

	vec_uchar16* adr = (vec_uchar16*)(intraBitsAdr + (intraBitsOfs >> 3));
	uint32_t shift = ((intraBitsAdr << 3) + intraBitsOfs) & 127u;

	// get intra bits, left justified
	vec_uchar16 d0a = adr[0];
	vec_uchar16 d0b = adr[1];
	vec_uchar16 d0as = spu_rlqwbytebc(d0a, shift);
	vec_uchar16 d0bs = spu_rlqwbytebc(d0b, shift);
	vec_uchar16 d0as0 = spu_rlqw(d0as, shift);
	vec_uchar16 d0bs0 = spu_rlqw(d0bs, shift);
	vec_uchar16 msk0a = spu_slqwbytebc(allOnes, shift);
	vec_uchar16 msk0b = spu_slqw(msk0a, shift);
	vec_uchar16 ib0 = spu_sel(d0bs0, d0as0, msk0b);
	vec_uchar16 intraBits = spu_and(ib0, bitMask);

	// get prev intra bits
	vec_uchar16 prevBits = spu_and(ib0, prevBitMask);

	// count enabled intra bits
	vec_uchar16 sum = (vec_uchar16)spu_sumb(spu_cntb(intraBits), zero);
	vec_uchar16 sum2 = spu_shuffle(sum, sum, shufSum);
	uint32_t numBits = si_to_uint((vec_uchar16)spu_sumb(sum2, zero));

	// count prev enabled intra bits
	sum = (vec_uchar16)spu_sumb(spu_cntb(prevBits), zero);
	sum2 = spu_shuffle(sum, sum, shufSum);
	uint32_t numPrevBits = si_to_uint((vec_uchar16)spu_sumb(sum2, zero));

	// get number of contiguously disabled bits before this frame
	uint32_t bBits = frameInteger - _edgeAnimGetLastSetBit(prevBits) - 1;

	// get number of contiguously disabled bits after this frame
	vec_uchar16 intraBitsM = spu_orc(spu_andc(intraBits, prevBitMask), bitMask);
	uint32_t aBits = _edgeAnimGetFirstSetBit(intraBitsM) - frameInteger + 1;

	// get compressed jointA address
	if(numPrevBits == 0)
		*keyA = (uint8_t*)initialAdr + channelId * stride;
	else
		*keyA = (uint8_t*)intraAdr + (numPrevBits-1) * stride;

	// get compressed jointB address
	if(numPrevBits == numBits)
		*keyB = (uint8_t*)finalAdr + channelId * stride;
	else
		*keyB = (uint8_t*)intraAdr + numPrevBits * stride;

	// get slerp alpha
	float alpha = (bBits + frameFraction) / (aBits + bBits);

	return alpha;
}

static void _edgeAnimEvaluate(EdgeAnimJointTransform* outputJoint,
					   		  float* outputUserChannel,
					   		  const EdgeAnimAnimation* animHeader,
					   		  const EdgeAnimSkeleton* skel,
                       		  const void* frameSetData,
                       		  uint32_t intraFrameCount,
					   		  uint32_t jointIndex,
					   		  uint32_t userChannelIndex,
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
    uint32_t intraRBitsOfs = 0;
    uint32_t intraTBitsOfs = intraRBitsOfs + animRCount * intraFrameCount;
    uint32_t intraSBitsOfs = intraTBitsOfs + animTCount * intraFrameCount;
    uint32_t intraUserBitsOfs = intraSBitsOfs + animSCount * intraFrameCount;

	if(outputJoint)
	{
		// set output joint to base pose
	    const EdgeAnimJointTransform* basePose = EDGE_OFFSET_GET_POINTER(EdgeAnimJointTransform, skel->offsetBasePose);
		*outputJoint = basePose[jointIndex];

		// rotation
		int32_t constRId = _edgeAnimFindTableEntry(jointIndex, constRTable, constRCount);
		if(constRId >= 0)
		{
			// channel is constant - get value from decompressed constant value
			const uint8_t* constR = (uint8_t*)constRData + constRId * 6;
			_edgeAnimDecompressRotation(&outputJoint->rotation, constR);
		}
		else
		{
			int32_t animRId = _edgeAnimFindTableEntry(jointIndex, animRTable, animRCount);
			if(animRId >= 0)
			{
				// channel is animated - slerp keyframes
				uint8_t *keyA, *keyB;
				float alpha = _edgeAnimGetBracketingKeyframes(
					&keyA, &keyB,	frameInteger, frameFraction, intraFrameCount, animRId, 6, 
					intraBitsAdr, intraRBitsOfs, initialRAdr, intraRAdr, finalRAdr);

				// decompress jointA, jointB
				Vectormath::Aos::Quat rotA, rotB;
				_edgeAnimDecompressRotation(&rotA, keyA);
				_edgeAnimDecompressRotation(&rotB, keyB);

				// slerp
				outputJoint->rotation = Vectormath::Aos::slerp(alpha, rotA, rotB);
			}
		}

		// translation
		int32_t constTId = _edgeAnimFindTableEntry(jointIndex, constTTable, constTCount);
		if(constTId >= 0)
		{
			// channel is constant - get value from constant value
			const uint8_t* constT = (uint8_t*)constTData + constTId * 12;
			outputJoint->translation = Vectormath::Aos::Point3(_edgeAnimDecompressFloat3(constT));
		}
		else
		{
			int32_t animTId = _edgeAnimFindTableEntry(jointIndex, animTTable, animTCount);
			if(animTId >= 0)
			{
				// channel is animated - lerp keyframes
				uint8_t *keyA, *keyB;
				float alpha = _edgeAnimGetBracketingKeyframes(
					&keyA, &keyB, frameInteger, frameFraction, intraFrameCount, animTId, 12, 
					intraBitsAdr, intraTBitsOfs, initialTAdr, intraTAdr, finalTAdr);

				// decompress jointA, jointB
				Vectormath::Aos::Point3 transA(_edgeAnimDecompressFloat3(keyA));
				Vectormath::Aos::Point3 transB(_edgeAnimDecompressFloat3(keyB));

				// lerp
				outputJoint->translation = Vectormath::Aos::lerp(alpha, transA, transB);
			}
		}

		// scale
		int32_t constSId = _edgeAnimFindTableEntry(jointIndex, constSTable, constSCount);
		if(constSId >= 0)
		{
			// channel is constant - get value from constant value
			const uint8_t* constS = (uint8_t*)constSData + constSId * 12;
			outputJoint->scale = Vectormath::Aos::Vector4(_edgeAnimDecompressFloat3(constS));
			outputJoint->scale.setW(1);
		}
		else
		{
			int32_t animSId = _edgeAnimFindTableEntry(jointIndex, animSTable, animSCount);
			if(animSId >= 0)
			{
				// channel is animated - lerp keyframes
				uint8_t *keyA, *keyB;
				float alpha = _edgeAnimGetBracketingKeyframes(
					&keyA, &keyB, frameInteger, frameFraction, intraFrameCount, animSId, 12, 
					intraBitsAdr, intraSBitsOfs, initialSAdr, intraSAdr, finalSAdr);

				// decompress jointA, jointB
				Vectormath::Aos::Vector4 scaleA(_edgeAnimDecompressFloat3(keyA));
				Vectormath::Aos::Vector4 scaleB(_edgeAnimDecompressFloat3(keyB));

				// lerp
				outputJoint->scale = Vectormath::Aos::lerp(alpha, scaleA, scaleB);
				outputJoint->scale.setW(1);
			}
		}
	}

	// user channel
	if(outputUserChannel)
	{
		// set output joint to zero by default
		*outputUserChannel = 0.0f;

		int32_t constUserId = _edgeAnimFindTableEntry(userChannelIndex, constUTable, constUCount);

		if(constUserId >= 0)
		{
			// channel is constant - get value from constant value
			const float* constUser = ((float*)constUData) + constUserId;
			*outputUserChannel = *constUser;
		}
		else
		{
			int32_t animUserId = _edgeAnimFindTableEntry(userChannelIndex, animUTable, animUCount);
			if(animUserId >= 0)
			{
				// channel is animated - lerp keyframes
				uint8_t *keyA, *keyB;
				float alpha = _edgeAnimGetBracketingKeyframes(
					&keyA, &keyB, frameInteger, frameFraction, intraFrameCount, animUserId, 4, 
					intraBitsAdr, intraUserBitsOfs, initialUAdr, intraUAdr, finalUAdr);

				float userA, userB;
				userA = *(float*)keyA;
				userB = *(float*)keyB;

				// lerp
				*outputUserChannel = userA * (1-alpha) + userB * alpha;
			}
		}
	}
}

void _edgeAnimEvaluate(EdgeAnimJointTransform* outputJoint, 
					  float* outputUserChannel,
					  const EdgeAnimAnimation* anim, 
					  const EdgeAnimSkeleton* skel, 
					  uint32_t jointIndex,
					  uint32_t userChannelIndex,
					  float evalTime)
{
	// there is no ppu implementation of the bitpacked evaluator, it is spu only
	EDGE_ASSERT(anim->offsetPackingSpecs == 0);

	float frame = evalTime * anim->sampleFrequency;

	// clamp to first frame
    if(frame < 0)
        frame = 0;

	// get frame set info
	uint32_t frameSetIndex = _edgeAnimGetFrameSetIndex(anim, frame);
	const EdgeAnimFrameSetInfo* infoArray = EDGE_OFFSET_GET_POINTER(EdgeAnimFrameSetInfo, anim->offsetFrameSetInfoArray);
	uint32_t numIntraFrames = infoArray[frameSetIndex].numIntraFrames;
	uint32_t baseFrame = infoArray[frameSetIndex].baseFrame;

	// get frame set address
	const uint32_t* dmaArray = EDGE_OFFSET_GET_POINTER(uint32_t, anim->offsetFrameSetDmaArray);
	const void* frameSetData = (void*)(dmaArray[frameSetIndex*2 + 1] + (uintptr_t)anim);

	// get integer & fractional parts of frame (relative to frameset)
	float frameSetFrame = frame - baseFrame;
	uint32_t frameInteger = (uint32_t)frameSetFrame;
	float frameFraction = frameSetFrame - (float)frameInteger;

	// clamp to last frame
	// (required for last frameset if we have gone beyond the end of the animation)
	if(frameInteger > numIntraFrames)
	{
		frameInteger = numIntraFrames;
		frameFraction = 1;
	}

	// evaluate
	_edgeAnimEvaluate(outputJoint, outputUserChannel, anim, skel, frameSetData, numIntraFrames, jointIndex, userChannelIndex, frameInteger, frameFraction);
}

extern "C"
void edgeAnimEvaluateJoint(EdgeAnimJointTransform* outputJoint, 
						   const EdgeAnimAnimation* anim, 
					       const EdgeAnimSkeleton* skel, 
					       uint32_t jointIndex,
					       float evalTime)
{
	_edgeAnimEvaluate(outputJoint, NULL, anim, skel, jointIndex, 0, evalTime);
}

extern "C"
float edgeAnimEvaluateUserChannel(const EdgeAnimAnimation* anim, 
								  const EdgeAnimSkeleton* skel, 
								  uint32_t channelIndex,
								  float evalTime)
{
	float r;
	_edgeAnimEvaluate(NULL, &r, anim, skel, 0, channelIndex, evalTime);
	return r;
}
