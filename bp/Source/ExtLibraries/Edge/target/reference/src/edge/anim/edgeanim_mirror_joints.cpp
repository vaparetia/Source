/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"

/*
 * Reference C++ implementation using vectormath.
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void _edgeAnimMirrorJoints(EdgeAnimJointTransform* joints, 
                          uint8_t* weights, 
                          const EdgeAnimMirrorPair* mirrorSpec,
						  unsigned int count)
{
	for (; count; count--)
	{
		uint32_t spec = mirrorSpec->spec;

		EdgeAnimJointTransform& srcJointA = joints[mirrorSpec->idx0];
		EdgeAnimJointTransform& srcJointB = joints[mirrorSpec->idx1];

		EdgeAnimJointTransform dstJointA;
		EdgeAnimJointTransform dstJointB;

		dstJointA.scale = srcJointB.scale;
		dstJointB.scale = srcJointA.scale;

		for(int loop=3; loop>=0; loop--)
		{
			uint32_t rSpecBits = (spec & 0x000000f0) >> 4;
			uint32_t tSpecBits = (spec & 0x0000000f);

			uint32_t idxR = rSpecBits & 3;
			uint32_t idxT = tSpecBits & 3;

			float rA = srcJointA.rotation.getElem(idxR);
			float tA = srcJointA.translation.getElem(idxT);
			float rB = srcJointB.rotation.getElem(idxR);
			float tB = srcJointB.translation.getElem(idxT);

			if(rSpecBits & 8)
			{
				rA = -rA;
				rB = -rB;
			}
			if(tSpecBits & 8)
			{
				tA = -tA;
				tB = -tB;
			}

			dstJointA.rotation.setElem(loop, rA);
			dstJointA.translation.setElem(loop, tA);
			dstJointB.rotation.setElem(loop, rB);
			dstJointB.translation.setElem(loop, tB);

			spec = spec >> 8;
		}

		srcJointA = dstJointB;
		srcJointB = dstJointA;

		// Swap weights
		uint8_t srcWeightA = weights[mirrorSpec->idx0];
		uint8_t srcWeightB = weights[mirrorSpec->idx1];
		weights[mirrorSpec->idx0] = srcWeightB;
		weights[mirrorSpec->idx1] = srcWeightA;

		mirrorSpec++;
	}
}
