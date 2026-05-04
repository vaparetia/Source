/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimMirrorJoints(EdgeAnimJointTransform* joints, 
                          uint8_t* weights, 
                          const EdgeAnimMirrorPair* mirrorSpec,
						  unsigned int count)
{
	EDGE_ASSERT(EDGE_IS_ALIGNED(joints, 16U));
	EDGE_ASSERT(count);

	for(; count; count--)
	{
		// Load Mirror Spec
		uint32_t idx0 = mirrorSpec->idx0;
		uint32_t idx1 = mirrorSpec->idx1;
		uint32_t spec = mirrorSpec->spec;

		// Source Joint Pointers
		float* pSrcRotA = (float*) &joints[idx0].rotation;
		float* pSrcRotB = (float*) &joints[idx1].rotation;
		float* pSrcTransA = (float*) &joints[idx0].translation;
		float* pSrcTransB = (float*) &joints[idx1].translation;
		float* pSrcScaleA = (float*) &joints[idx0].scale;
		float* pSrcScaleB = (float*) &joints[idx1].scale;

		// Destination Joints + Pointers
		__m128 dstRotA, dstTransA, dstScaleA;
		__m128 dstRotB, dstTransB, dstScaleB;
		float* pDstRotA = (float*) &dstRotA;
		float* pDstRotB = (float*) &dstRotB;
		float* pDstTransA = (float*) &dstTransA;
		float* pDstTransB = (float*) &dstTransB;
		dstScaleA = _mm_load_ps( pSrcScaleA );
		dstScaleB = _mm_load_ps( pSrcScaleB );

		// Sign Mask
		__m128i signMask = _mm_set1_epi32( spec );
		signMask = _mm_unpacklo_epi8( signMask, _mm_setzero_si128() );
		signMask = _mm_unpacklo_epi16( signMask, _mm_setzero_si128() );
		signMask = _mm_shuffle_epi32( signMask, _MM_SHUFFLE( 0, 1, 2, 3 ) );

		__m128i signMaskTransI = _mm_slli_epi32( _mm_srli_epi32( signMask, 3 ), 31 );
		__m128i signMaskRotI = _mm_slli_epi32( _mm_srli_epi32( signMask, 7 ), 31 );
		__m128 signTrans = *(__m128*)&signMaskTransI;
		__m128 signRot = *(__m128*)&signMaskRotI;

		// Source Element Indices
		uint32_t idxTrans3 = ( spec ) & 0x3;
		uint32_t idxTrans2 = ( spec >>  8 ) & 0x3;
		uint32_t idxTrans1 = ( spec >> 16 ) & 0x3;
		uint32_t idxTrans0 = ( spec >> 24 ) & 0x3;

		uint32_t idxRot3 = ( spec >>  4 ) & 0x3;
		uint32_t idxRot2 = ( spec >> 12 ) & 0x3;
		uint32_t idxRot1 = ( spec >> 20 ) & 0x3;
		uint32_t idxRot0 = ( spec >> 28 ) & 0x3;

		// Copy Elements
		pDstRotA[3] = pSrcRotA[ idxRot3 ];
		pDstRotA[2] = pSrcRotA[ idxRot2 ];
		pDstRotA[1] = pSrcRotA[ idxRot1 ];
		pDstRotA[0] = pSrcRotA[ idxRot0 ];
		pDstRotB[3] = pSrcRotB[ idxRot3 ];
		pDstRotB[2] = pSrcRotB[ idxRot2 ];
		pDstRotB[1] = pSrcRotB[ idxRot1 ];
		pDstRotB[0] = pSrcRotB[ idxRot0 ];

		pDstTransA[3] = pSrcTransA[ idxTrans3 ];
		pDstTransA[2] = pSrcTransA[ idxTrans2 ];
		pDstTransA[1] = pSrcTransA[ idxTrans1 ];
		pDstTransA[0] = pSrcTransA[ idxTrans0 ];
		pDstTransB[3] = pSrcTransB[ idxTrans3 ];
		pDstTransB[2] = pSrcTransB[ idxTrans2 ];
		pDstTransB[1] = pSrcTransB[ idxTrans1 ];
		pDstTransB[0] = pSrcTransB[ idxTrans0 ];

		// Apply Sign Mask
		dstRotA = _mm_xor_ps( dstRotA, signRot );
		dstRotB = _mm_xor_ps( dstRotB, signRot );

		dstTransA = _mm_xor_ps( dstTransA, signTrans );
		dstTransB = _mm_xor_ps( dstTransB, signTrans );

		// Writeout Joints
		_mm_store_ps( pSrcRotB, dstRotA );
		_mm_store_ps( pSrcRotA, dstRotB );
		_mm_store_ps( pSrcTransB, dstTransA );
		_mm_store_ps( pSrcTransA, dstTransB );
		_mm_store_ps( pSrcScaleB, dstScaleA );
		_mm_store_ps( pSrcScaleA, dstScaleB );

		// Writeout weights
		uint8_t srcWeightA = weights[idx0];
		uint8_t srcWeightB = weights[idx1];
		weights[idx0] = srcWeightB;
		weights[idx1] = srcWeightA;

		mirrorSpec++;
	}
}
