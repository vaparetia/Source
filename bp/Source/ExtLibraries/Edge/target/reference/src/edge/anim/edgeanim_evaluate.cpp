/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <string.h>
#include <math.h>

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_macros.h"
#include "edge/anim/edgeanim_structs_ps3.h"

union floatInt
{
    float f;
    uint8_t uc[4];
};

// get index of first set bit in a word
// TODO: optimize
static inline uint32_t _edgeAnimGetFirstSetBit32(uint32_t v)
{
    static const uint8_t clztab[16] = {0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};

    int r = 32;

    if(v & 0xffff0000)
    {
        v >>= 16;
        r -= 16;
    }
    if(v & 0xff00)
    {
        v >>= 8;
        r -= 8;
    }
    if(v & 0xf0)
    {
        v >>= 4;
        r -= 4;
    }

    r -= clztab[v];

    return r;
}

// get index of last set bit in a word
static inline uint32_t _edgeAnimGetLastSetBit32(int32_t v)
{
    return _edgeAnimGetFirstSetBit32(v & -v);
}

// count set bits in a word
static inline uint32_t _edgeAnimCountBits32(uint32_t v)
{
    uint32_t tmp = v - ((v >> 1) & 0x55555555);
    tmp = (tmp & 0x33333333) + ((tmp >> 2) & 0x33333333);
    uint32_t r = ((tmp + (tmp >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;

    return r;
}

// get index of first set bit in a quadword
// TODO: optimize
static inline uint32_t _edgeAnimGetFirstSetBit128(uint32_t* v)
{
    uint32_t i = 0;
    while((i<4) && (v[i] == 0))
        i++;

    uint32_t r;

    if(i == 4)
        r = 128;
    else
        r = _edgeAnimGetFirstSetBit32(v[i]) + i*32;

    return r;
}

// get index of last set bit in a quadword
// TODO: optimize
static inline uint32_t _edgeAnimGetLastSetBit128(uint32_t* v)
{
    int32_t i = 3;
    while((i>=0) && (v[i] == 0))
        i--;

    uint32_t r;

    if(i < 0)
        r = (uint32_t) -1;
    else
        r = _edgeAnimGetLastSetBit32(v[i]) + i*32;

    return r;
}

// count set bits in a quadword
static inline uint32_t _edgeAnimCountBits128(uint32_t* v)
{
    uint32_t r;

    r = _edgeAnimCountBits32(v[0]);
    r += _edgeAnimCountBits32(v[1]);
    r += _edgeAnimCountBits32(v[2]);
    r += _edgeAnimCountBits32(v[3]);

    return r;
}

// TODO: optimize
static void _edgeAnimDecompressQuat(Vectormath::Aos::Quat* pRes, uint32_t qHi, uint32_t qLo)
{
    static const float kSqrt2 = 1.414213562f;
    static const float kQuatScale = ((1 << 15) - 1) / kSqrt2;
    static const float kQuatOffset = kQuatScale / kSqrt2;

    uint32_t a = (qHi & ((1<<15)-1));
    uint32_t b = ((qLo >> 17) & ((1<<15)-1));
    uint32_t c = ((qLo >> 2) & ((1<<15)-1));
    uint32_t idx = (qLo & 3);

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
    uint32_t qHi = ((uint32_t)compressed[0] << 8)  | 
                   ((uint32_t)compressed[1]);

	uint32_t qLo = ((uint32_t)compressed[2] << 24) | 
                   ((uint32_t)compressed[3] << 16) | 
                   ((uint32_t)compressed[4] << 8)  | 
                   ((uint32_t)compressed[5]);

    _edgeAnimDecompressQuat(pR, qHi, qLo);
}

// TODO: optimize
static Vectormath::Aos::Vector3 _edgeAnimDecompressVector3(const uint8_t* compressed)
{
    floatInt x, y, z;

    x.uc[0] = compressed[0]; x.uc[1] = compressed[1]; x.uc[2] = compressed[2];  x.uc[3] = compressed[3];
    y.uc[0] = compressed[4]; y.uc[1] = compressed[5]; y.uc[2] = compressed[6];  y.uc[3] = compressed[7];
    z.uc[0] = compressed[8]; z.uc[1] = compressed[9]; z.uc[2] = compressed[10]; z.uc[3] = compressed[11];

    return Vectormath::Aos::Vector3(x.f, y.f, z.f);
}

static void _edgeAnimGenerateBitMask(uint32_t* mask, uint32_t bitPos)
{
    static const uint64_t t = ~((uint64_t)0);
    uint64_t t0, t1;

	if(bitPos >= 64) {
		uint32_t shift1 = 128 - bitPos;
		t0 = t;
		t1 = ( shift1 >= 64 ? 0 : t << shift1 );
	}
	else {
		uint32_t shift0 = 64 - bitPos;
		t0 = ( shift0 >= 64 ? 0 : t << shift0 );
		t1 = 0;
	}

    mask[0] = (uint32_t)(t0 >> 32);
    mask[1] = (uint32_t)t0;
    mask[2] = (uint32_t)(t1 >> 32);
    mask[3] = (uint32_t)t1;
}

static float _edgeAnimGetBracketingKeyframes(uintptr_t* keyBaseA, uint32_t* keyOfsA,
											 uintptr_t* keyBaseB, uint32_t* keyOfsB,
                                             uint32_t frameInteger, float frameFraction, 
                                             uint32_t* bitMask, uint32_t* prevBitMask, uint32_t stride, 
                                             uintptr_t bitsAdr, uint32_t bitsBitOfs,
                                             uintptr_t initialBase, uint32_t initialOfs,
											 uintptr_t intraBase, uint32_t intraOfs,
											 uintptr_t finalBase, uint32_t finalOfs,
											 uint32_t* numKeysOut)
{
    // pretty slow, mostly due to little-endianness
    // TODO: optimize
    uint32_t ofs = bitsBitOfs & 7;
    uint8_t* adr = (uint8_t*)(bitsAdr + (bitsBitOfs >> 3));
    uint32_t b0 = adr[0] << (24 + ofs);
    b0 |= adr[1] << (ofs + 16);
    b0 |= adr[2] << (ofs + 8);
    b0 |= adr[3] << (ofs);
    b0 |= adr[4] >> (8 - ofs);

    uint32_t b1 = adr[4] << (24 + ofs);
    b1 |= adr[5] << (ofs + 16);
    b1 |= adr[6] << (ofs + 8);
    b1 |= adr[7] << (ofs);
    b1 |= adr[8] >> (8 - ofs);

    uint32_t b2 = adr[8] << (24 + ofs);
    b2 |= adr[9] << (ofs + 16);
    b2 |= adr[10] << (ofs + 8);
    b2 |= adr[11] << (ofs);
    b2 |= adr[12] >> (8 - ofs);

    uint32_t b3 = adr[12] << (24 + ofs);
    b3 |= adr[13] << (ofs + 16);
    b3 |= adr[14] << (ofs + 8);
    b3 |= adr[15] << (ofs);
    b3 |= adr[16] >> (8 - ofs);

    uint32_t intraBits[4];
    intraBits[0] = b0 & bitMask[0];
    intraBits[1] = b1 & bitMask[1];
    intraBits[2] = b2 & bitMask[2];
    intraBits[3] = b3 & bitMask[3];

    uint32_t prevBits[4];
    prevBits[0] = b0 & prevBitMask[0];
    prevBits[1] = b1 & prevBitMask[1];
    prevBits[2] = b2 & prevBitMask[2];
    prevBits[3] = b3 & prevBitMask[3];

    uint32_t numBits = _edgeAnimCountBits128(intraBits);
    uint32_t numPrevBits = _edgeAnimCountBits128(prevBits);

    uint32_t intraBitsM[4];
    intraBitsM[0] = (intraBits[0] & (~prevBitMask[0])) | (~bitMask[0]);
    intraBitsM[1] = (intraBits[1] & (~prevBitMask[1])) | (~bitMask[1]);
    intraBitsM[2] = (intraBits[2] & (~prevBitMask[2])) | (~bitMask[2]);
    intraBitsM[3] = (intraBits[3] & (~prevBitMask[3])) | (~bitMask[3]);

    uint32_t bBits = frameInteger - _edgeAnimGetLastSetBit128(prevBits) - 1;
    uint32_t aBits = _edgeAnimGetFirstSetBit128(intraBitsM) - frameInteger + 1;

    // get compressed jointA address
    if(numPrevBits == 0)
	{
		*keyBaseA = initialBase;
		*keyOfsA = initialOfs;
	}
    else
	{
		*keyBaseA = intraBase;
		*keyOfsA = intraOfs + (numPrevBits-1) * stride;
	}

    // get compressed jointB address
    if(numPrevBits == numBits)
	{
		*keyBaseB = finalBase;
		*keyOfsB = finalOfs;
	}
    else
	{
		*keyBaseB = intraBase;
		*keyOfsB = intraOfs + numPrevBits * stride;
	}

    // get slerp alpha
    float alpha = (bBits + frameFraction) / (aBits + bBits);

    *numKeysOut = numBits;

    return alpha;
}

static float DecompressBitPackedFloat(uint32_t bits, uint32_t numSignBits, uint32_t numExponentBits, uint32_t numMantissaBits )
{
	float res = 0.0f;

	// Mask tables ( to avoid microcoded shifts where we can! )
	static const uint32_t maskTable[] = 
	{
		0x00000000, 
		0x00000001, 0x00000003, 0x00000007, 0x0000000F,	
		0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
		0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,	
		0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
		0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,	
		0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
		0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,	
		0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF,
	};
	static const uint32_t signMaskTable[] = 
	{
		0x00000001, 0x00000002, 0x00000004,	0x00000008,
		0x00000010,	0x00000020,	0x00000040,	0x00000080,
		0x00000100,	0x00000200,	0x00000400,	0x00000800,
		0x00001000,	0x00002000,	0x00004000,	0x00008000,
		0x00010000,	0x00020000,	0x00040000,	0x00080000,
		0x00100000,	0x00200000,	0x00400000,	0x00800000,
		0x01000000,	0x02000000,	0x04000000,	0x08000000,
		0x10000000,	0x20000000,	0x40000000,	0x80000000
	};
	static const uint32_t signExtendTable[] = 
	{
		0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8,
		0xFFFFFFF0, 0xFFFFFFE0, 0xFFFFFFC0, 0xFFFFFF80,
		0xFFFFFF00, 0xFFFFFE00, 0xFFFFFC00, 0xFFFFF800,
		0xFFFFF000, 0xFFFFE000, 0xFFFFC000, 0xFFFF8000,
		0xFFFF0000, 0xFFFE0000, 0xFFFC0000, 0xFFF80000,
		0xFFF00000, 0xFFE00000, 0xFFC00000, 0xFF800000,
		0xFF000000, 0xFE000000, 0xFC000000, 0xF8000000,
		0xF0000000, 0xE0000000, 0xC0000000, 0x80000000,
		0x00000000
	};


	// Exponent bias table ( to avoid microcoded shifts where we can! )
	// 128 - ( 1U << ( m_numExponentBits - 1 ) )
	static const int expBiasTable[] =
	{
		128, 127, 126, 124, 120,
		112, 96,  64,  0, -128, -384
	};

	// Floating point mode 
	if ( numExponentBits )
	{
		union
		{
			unsigned int	m_floatBits;
			float			m_floatVal;
		} floatConv;

		unsigned int m = ( bits ) & maskTable[ numMantissaBits ];
		int			 e = ( bits >> numMantissaBits ) & maskTable[ numExponentBits ];
		unsigned int s = ( bits >> ( numMantissaBits + numExponentBits ) ) & maskTable[ numSignBits ];

		//if ( numExponentBits )
			e += expBiasTable[ numExponentBits ];

		if ( numMantissaBits <= 23 )
			m <<= ( 23 - numMantissaBits );
		else
			m >>= numMantissaBits - 23;

		// Clamp exponent - is it really necessary? - warning: SPU side doesn't do this
		if ( e < 0 )
		{
			e = 0;
			m = 0;
			s = 0;
		}
		else if ( e >= ( 1 << 8 ) )
		{
			e = ( 1 << 8 ) - 1;
			m = 0xffffffffU >> ( 32 - 23 );
		}

		floatConv.m_floatBits  = 0;
		floatConv.m_floatBits |= s << 31U;
		floatConv.m_floatBits |= e << 23U;
		floatConv.m_floatBits |= m;

		res = floatConv.m_floatVal;
	} // Fixed point ( normalized )
	else if ( numMantissaBits )
	{				
		uint32_t lim = maskTable[ numMantissaBits ];
		float flim = ( float ) lim;

		// Signed fixed point
		if ( numSignBits )
		{
			int val = ( int ) bits;
			// sign extend?
			if ( val & signMaskTable [  numMantissaBits ] )
				val |= signExtendTable[  numMantissaBits ];			
			res = val / flim;
		}
		// Unsigned fixed point
		else 
		{
			res = ( ( uint32_t ) bits ) / flim;
		}
	}

	return res;
}

static uint32_t _edgeAnimGetSpecBitStride(uint32_t spec)
{
	uint32_t numSBits = ((spec >> 31) & 1) + ((spec >> 21) & 1) + ((spec >> 11) & 1);
	uint32_t numEBits = ((spec >> 27) & 15) + ((spec >> 17) & 15) + ((spec >> 7) & 15);
	uint32_t numMBits = ((spec >> 22) & 31) + ((spec >> 12) & 31) + ((spec >> 2) & 31);

	return numSBits + numEBits + numMBits;
}

// high 10 bits of spec used
static float _edgeAnimDecompressFloatBitPacked(const uint8_t* byteAdr, uint32_t bitOfs, uint32_t spec, uint32_t* numBitsOut)
{
	uint32_t numSBits = (spec >> (31)) & 1;
	uint32_t numEBits = (spec >> (27)) & 15;
	uint32_t numMBits = (spec >> (22)) & 31;
	uint32_t numBits = numSBits + numEBits + numMBits;
	uint32_t byteOfs = (bitOfs >> 3);

	uint64_t packed = 0;
	packed  = (uint64_t)byteAdr[ byteOfs + 0 ] << 32;
	packed |= (uint64_t)byteAdr[ byteOfs + 1 ] << 24;
	packed |= (uint64_t)byteAdr[ byteOfs + 2 ] << 16;
	packed |= (uint64_t)byteAdr[ byteOfs + 3 ] << 8;
	packed |= (uint64_t)byteAdr[ byteOfs + 4 ];
	packed >>= 8 - (bitOfs & 7);
	packed >>= 32 - numBits;
	packed &= (1<<numBits)-1;
	float r = DecompressBitPackedFloat((uint32_t)packed, numSBits, numEBits, numMBits);

	*numBitsOut = numBits;
	return r;
}

static Vectormath::Aos::Vector3 _edgeAnimDecompressVector3BitPacked(const uint8_t* byteAdr, uint32_t bitOfs, uint32_t spec)
{
	Vectormath::Aos::Vector3 v;
	for(uint32_t i=0; i<3; i++)
	{
		uint32_t numBits;
		v[i] = _edgeAnimDecompressFloatBitPacked(byteAdr, bitOfs, spec, &numBits);
		spec <<= 10;
		bitOfs += numBits;
	}

	return v;
}

static Vectormath::Aos::Quat _edgeAnimDecompressRotationBitPacked(const uint8_t* byteAdr, uint32_t bitOfs, uint32_t spec)
{
	Vectormath::Aos::Vector3 v = _edgeAnimDecompressVector3BitPacked(byteAdr, bitOfs, spec);

	float fa = v[0];
	float fb = v[1];
	float fc = v[2];
	float sumSq = fa*fa + fb*fb + fc*fc;
	if(sumSq > 1)
		sumSq = 1;
	float fd = sqrtf(1 - sumSq);

	Vectormath::Aos::Quat q;
	uint32_t recomputeIdx = spec & 3;
	switch(recomputeIdx)
	{
	case 0:
		q = Vectormath::Aos::Quat(fd, fa, fb, fc);
		break;
	case 1:
		q = Vectormath::Aos::Quat(fa, fd, fb, fc);
		break;
	case 2:
		q = Vectormath::Aos::Quat(fa, fb, fd, fc);
		break;
	case 3:
		q = Vectormath::Aos::Quat(fa, fb, fc, fd);
		break;
	}

	return q;
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
    uint32_t bitMask[4];
    _edgeAnimGenerateBitMask(bitMask, intraFrameCount);

    // prevBitMask = 1 for bits < frameInteger
    uint32_t prevBitMask[4];
    _edgeAnimGenerateBitMask(prevBitMask, frameInteger);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Rotations
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_R)
	{
		// constant rotations - bit packed
		uint32_t spec = *packingSpec++;
		uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
		const uint8_t* constRDataByteAdr = (const uint8_t*) constRData;
		uint32_t constRDataBitOfs = 0;
		for(uint32_t channelLoop = 0; channelLoop < constRCount; channelLoop++)
		{
			Vectormath::Aos::Quat rot = _edgeAnimDecompressRotationBitPacked(constRDataByteAdr, constRDataBitOfs, spec);

			int32_t constRId = constRTable[channelLoop];
			outputJoints[constRId].rotation = rot;

			constRDataBitOfs += specStride;
		}

		// non-constant rotations - bit packed
		uint32_t initialRBitOfs = 0;
		uint32_t intraRBitOfs = 0;
		uint32_t finalRBitOfs = 0;
		for(uint32_t channelLoop = 0; channelLoop < animRCount; channelLoop++)
		{
			uintptr_t keyAdrA, keyAdrB;
			uint32_t keyBitOfsA, keyBitOfsB;
			uint32_t numKeys;

			uint32_t spec = *packingSpec++;
			uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
			float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyBitOfsA, &keyAdrB, &keyBitOfsB, frameInteger, frameFraction, 
				bitMask, prevBitMask, specStride, intraBitsAdr, intraRBitsOfs,
				initialRAdr, initialRBitOfs, intraRAdr, intraRBitOfs, finalRAdr, finalRBitOfs, &numKeys);

			// decompress jointA, jointB
			Vectormath::Aos::Quat rotA, rotB;
			rotA = _edgeAnimDecompressRotationBitPacked((const uint8_t*) keyAdrA, keyBitOfsA, spec);
			rotB = _edgeAnimDecompressRotationBitPacked((const uint8_t*) keyAdrB, keyBitOfsB, spec);

			// slerp
			uint32_t animRId = animRTable[channelLoop];
			outputJoints[animRId].rotation = Vectormath::Aos::slerp(alpha, rotA, rotB);

			intraRBitsOfs += intraFrameCount;

			initialRBitOfs += specStride;
			intraRBitOfs += numKeys * specStride;
			finalRBitOfs += specStride;
		}
	}
	else
	{
		// constant rotations - 48-bit compressed
		for(uint32_t channelLoop = 0; channelLoop < constRCount; channelLoop++)
		{
			int32_t constRId = constRTable[channelLoop];
			const uint8_t* constR = (uint8_t*)constRData + channelLoop * 6;
			_edgeAnimDecompressRotation(&outputJoints[constRId].rotation, constR);
		}

		// non-constant rotations - 48-bit compressed
		for(uint32_t channelLoop = 0; channelLoop < animRCount; channelLoop++)
		{
			uintptr_t keyAdrA, keyAdrB;
			uint32_t keyOfsA, keyOfsB;
			uint32_t numKeys;

			float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyOfsA, &keyAdrB, &keyOfsB, frameInteger, frameFraction, 
				bitMask, prevBitMask, 6, intraBitsAdr, intraRBitsOfs,
				initialRAdr, 0, intraRAdr, 0, finalRAdr, 0, &numKeys);

			// decompress jointA, jointB
			Vectormath::Aos::Quat rotA, rotB;
			_edgeAnimDecompressRotation(&rotA, (uint8_t*)(keyAdrA + keyOfsA));
			_edgeAnimDecompressRotation(&rotB, (uint8_t*)(keyAdrB + keyOfsB));

			// slerp
			uint32_t animRId = animRTable[channelLoop];
			outputJoints[animRId].rotation = Vectormath::Aos::slerp(alpha, rotA, rotB);

			intraRBitsOfs += intraFrameCount;

			initialRAdr += 6;
			intraRAdr += numKeys * 6;
			finalRAdr += 6;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Translations
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_T)
	{
		// constant translations - bit packed
		uint32_t spec = *packingSpec++;
		uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
		const uint8_t* constTDataByteAdr = (const uint8_t*) constTData;
		uint32_t constTDataBitOfs = 0;
		for(uint32_t channelLoop = 0; channelLoop < constTCount; channelLoop++)
		{
			Vectormath::Aos::Vector3 t = _edgeAnimDecompressVector3BitPacked(constTDataByteAdr, constTDataBitOfs, spec);

			int32_t constTId = constTTable[channelLoop];
			outputJoints[constTId].translation = Vectormath::Aos::Point3(t);

			constTDataBitOfs += specStride;
		}

		// non-constant translations - bit packed
		uint32_t initialTBitOfs = 0;
		uint32_t intraTBitOfs = 0;
		uint32_t finalTBitOfs = 0;
		for(uint32_t channelLoop = 0; channelLoop < animTCount; channelLoop++)
		{
			uintptr_t keyAdrA, keyAdrB;
			uint32_t keyBitOfsA, keyBitOfsB;
			uint32_t numKeys;

			uint32_t spec = *packingSpec++;
			uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
			float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyBitOfsA, &keyAdrB, &keyBitOfsB, frameInteger, frameFraction, 
				bitMask, prevBitMask, specStride, intraBitsAdr, intraTBitsOfs,
				initialTAdr, initialTBitOfs, intraTAdr, intraTBitOfs, finalTAdr, finalTBitOfs, &numKeys);

			// decompress jointA, jointB
			Vectormath::Aos::Vector3 transA, transB;
			transA = _edgeAnimDecompressVector3BitPacked((const uint8_t*) keyAdrA, keyBitOfsA, spec);
			transB = _edgeAnimDecompressVector3BitPacked((const uint8_t*) keyAdrB, keyBitOfsB, spec);

			// lerp
			uint32_t animTId = animTTable[channelLoop];
			Vectormath::Aos::Vector3 lerp = Vectormath::Aos::lerp(alpha, transA, transB);
			outputJoints[animTId].translation = Vectormath::Aos::Point3(lerp);

			intraTBitsOfs += intraFrameCount;

			initialTBitOfs += specStride;
			intraTBitOfs += numKeys * specStride;
			finalTBitOfs += specStride;
		}
	}
	else
	{
		// constant translations - uncompressed float3
		for(uint32_t channelLoop = 0; channelLoop < constTCount; channelLoop++)
		{
			int32_t constTId = constTTable[channelLoop];
			const uint8_t* constT = (uint8_t*)constTData + channelLoop * 12;
			Vectormath::Aos::Point3 trans(_edgeAnimDecompressVector3(constT));
			outputJoints[constTId].translation = trans;
		}

		// non-constant translations - uncompressed float3
		for(uint32_t channelLoop = 0; channelLoop < animTCount; channelLoop++)
		{
			uintptr_t keyAdrA, keyAdrB;
			uint32_t keyOfsA, keyOfsB;
			uint32_t numKeys;

			float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyOfsA, &keyAdrB, &keyOfsB, frameInteger, frameFraction, 
				bitMask, prevBitMask, 12, intraBitsAdr, intraTBitsOfs,
				initialTAdr, 0, intraTAdr, 0, finalTAdr, 0, &numKeys);

			// decompress jointA, jointB
			Vectormath::Aos::Point3 transA(_edgeAnimDecompressVector3((uint8_t*)(keyAdrA + keyOfsA)));
			Vectormath::Aos::Point3 transB(_edgeAnimDecompressVector3((uint8_t*)(keyAdrB + keyOfsB)));

			// lerp
			uint32_t animTId = animTTable[channelLoop];
			outputJoints[animTId].translation = Vectormath::Aos::lerp(alpha, transA, transB);

			intraTBitsOfs += intraFrameCount;

			initialTAdr += 12;
			intraTAdr += numKeys * 12;
			finalTAdr += 12;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Scales
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_S)
	{
		// constant scales - bit packed
		uint32_t spec = *packingSpec++;
		uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
		const uint8_t* constSDataByteAdr = (const uint8_t*) constSData;
		uint32_t constSDataBitOfs = 0;
		for(uint32_t channelLoop = 0; channelLoop < constSCount; channelLoop++)
		{
			Vectormath::Aos::Vector3 s = _edgeAnimDecompressVector3BitPacked(constSDataByteAdr, constSDataBitOfs, spec);

			int32_t constSId = constSTable[channelLoop];
			outputJoints[constSId].scale = Vectormath::Aos::Vector4(s);
			outputJoints[constSId].scale.setW(1);

			constSDataBitOfs += specStride;
		}

		// non-constant scales - bit packed
		uint32_t initialSBitOfs = 0;
		uint32_t intraSBitOfs = 0;
		uint32_t finalSBitOfs = 0;
		for(uint32_t channelLoop = 0; channelLoop < animSCount; channelLoop++)
		{
			uintptr_t keyAdrA, keyAdrB;
			uint32_t keyBitOfsA, keyBitOfsB;
			uint32_t numKeys;

			uint32_t spec = *packingSpec++;
			uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
			float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyBitOfsA, &keyAdrB, &keyBitOfsB, frameInteger, frameFraction, 
				bitMask, prevBitMask, specStride, intraBitsAdr, intraSBitsOfs,
				initialSAdr, initialSBitOfs, intraSAdr, intraSBitsOfs, finalSAdr, finalSBitOfs, &numKeys);

			// decompress jointA, jointB
			Vectormath::Aos::Vector3 scaleA, scaleB;
			scaleA = _edgeAnimDecompressVector3BitPacked((const uint8_t*) keyAdrA, keyBitOfsA, spec);
			scaleB = _edgeAnimDecompressVector3BitPacked((const uint8_t*) keyAdrB, keyBitOfsB, spec);

			// lerp
			uint32_t animSId = animSTable[channelLoop];
			Vectormath::Aos::Vector3 lerp = Vectormath::Aos::lerp(alpha, scaleA, scaleB);
			outputJoints[animSId].scale = Vectormath::Aos::Vector4(lerp);
			outputJoints[animSId].scale.setW(1);

			intraSBitsOfs += intraFrameCount;

			initialSBitOfs += specStride;
			intraSBitOfs += numKeys * specStride;
			finalSBitOfs += specStride;
		}
	}
	else
	{
		// constant scales - uncompressed float3
		for(uint32_t channelLoop = 0; channelLoop < constSCount; channelLoop++)
		{
			int32_t constSId = constSTable[channelLoop];
			const uint8_t* constS = (uint8_t*)constSData + channelLoop * 12;
			Vectormath::Aos::Vector4 scale(_edgeAnimDecompressVector3(constS));
			outputJoints[constSId].scale = scale;
			outputJoints[constSId].scale.setW(1);
		}

		// non-constant scales - uncompressed float3
		for(uint32_t channelLoop = 0; channelLoop < animSCount; channelLoop++)
		{
			uintptr_t keyAdrA, keyAdrB;
			uint32_t keyOfsA, keyOfsB;
			uint32_t numKeys;

			float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyOfsA, &keyAdrB, &keyOfsB, frameInteger, frameFraction, 
				bitMask, prevBitMask, 12, intraBitsAdr, intraSBitsOfs,
				initialSAdr, 0, intraSAdr, 0, finalSAdr, 0, &numKeys);

			// decompress jointA, jointB
			Vectormath::Aos::Vector4 scaleA(_edgeAnimDecompressVector3((uint8_t*)(keyAdrA + keyOfsA)));
			Vectormath::Aos::Vector4 scaleB(_edgeAnimDecompressVector3((uint8_t*)(keyAdrB + keyOfsB)));

			// lerp
			uint32_t animSId = animSTable[channelLoop];
			outputJoints[animSId].scale = Vectormath::Aos::lerp(alpha, scaleA, scaleB);
			outputJoints[animSId].scale.setW(1);

			intraSBitsOfs += intraFrameCount;

			initialSAdr += 12;
			intraSAdr += numKeys * 12;
			finalSAdr += 12;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// User channels
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(outputUserChannels)
	{
		if(animHeader->flags & EDGE_ANIM_FLAG_BIT_PACKED_U)
		{
			// constant user channels - bit packed
			uint32_t spec = *packingSpec++;
			uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
			const uint8_t* constUDataByteAdr = (const uint8_t*) constUData;
			uint32_t constUDataBitAdr = 0;
			for(uint32_t channelLoop = 0; channelLoop < constUCount; channelLoop++)
			{
				uint32_t numBits;
				float user = _edgeAnimDecompressFloatBitPacked(constUDataByteAdr, constUDataBitAdr, spec, &numBits);

				int32_t constUId = constUTable[channelLoop];
				outputUserChannels[constUId] = user;

				constUDataBitAdr += specStride;
			}

			// non-constant user channels - bit packed
			uint32_t initialUBitOfs = 0;
			uint32_t intraUBitOfs = 0;
			uint32_t finalUBitOfs = 0;
			for(uint32_t channelLoop = 0; channelLoop < animUCount; channelLoop++)
			{
				uintptr_t keyAdrA, keyAdrB;
				uint32_t keyBitOfsA, keyBitOfsB;
				uint32_t numKeys;

				uint32_t spec = *packingSpec++;
				uint32_t specStride = _edgeAnimGetSpecBitStride(spec);
				float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyBitOfsA, &keyAdrB, &keyBitOfsB, frameInteger, frameFraction, 
					bitMask, prevBitMask, specStride, intraBitsAdr, intraUBitsOfs,
					initialUAdr, initialUBitOfs, intraUAdr, intraUBitOfs, finalUAdr, finalUBitOfs, &numKeys);

				// decompress jointA, jointB
				uint32_t numBits;
				float userA = _edgeAnimDecompressFloatBitPacked((const uint8_t*) keyAdrA, keyBitOfsA, spec, &numBits);
				float userB = _edgeAnimDecompressFloatBitPacked((const uint8_t*) keyAdrB, keyBitOfsB, spec, &numBits);

				// lerp
				uint32_t animUId = animUTable[channelLoop];
				outputUserChannels[animUId] = userA * (1-alpha) + userB * alpha;

				intraUBitsOfs += intraFrameCount;

				initialUBitOfs += specStride;
				intraUBitOfs += numKeys * specStride;
				finalUBitOfs += specStride;
			}
		}
		else
		{
			// constant user channels - uncompressed floats
			for(uint32_t channelLoop = 0; channelLoop < constUCount; channelLoop++)
			{
				int32_t constUId = constUTable[channelLoop];
				const float* constU = (float*)constUData + channelLoop;
				outputUserChannels[constUId] = constU[0];
			}

			// non-constant user channels - uncompressed floats
			for(uint32_t channelLoop = 0; channelLoop < animUCount; channelLoop++)
			{
				uintptr_t keyAdrA, keyAdrB;
				uint32_t keyOfsA, keyOfsB;
				uint32_t numKeys;

				float alpha = _edgeAnimGetBracketingKeyframes(&keyAdrA, &keyOfsA, &keyAdrB, &keyOfsB, frameInteger, frameFraction, 
					bitMask, prevBitMask, 4, intraBitsAdr, intraUBitsOfs,
					initialUAdr, 0, intraUAdr, 0, finalUAdr, 0, &numKeys);

				float userA = *(float*)(keyAdrA + keyOfsA);
				float userB = *(float*)(keyAdrB + keyOfsB);

				// lerp
				uint32_t animUId = animUTable[channelLoop];
				outputUserChannels[animUId] = userA * (1-alpha) + userB * alpha;

				intraUBitsOfs += intraFrameCount;

				initialUAdr += 4;
				intraUAdr += numKeys * 4;
				finalUAdr += 4;
			}
		}
	}
}
