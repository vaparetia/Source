 /* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef _MSC_VER
#include <vectormath/cpp/floatInVec.h>
#endif
#include "edge/anim/edgeanim_structs_ps3.h"

/*
 * Reference C++ implementation using vectormath.
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimLocalJointsToWorldJoints(EdgeAnimJointTransform* outputJoints, 
                                      const EdgeAnimJointTransform* inputJoints,
                                      const EdgeAnimJointTransform* rootJoint,
                                      const unsigned short* jointLinkage, 
                                      unsigned int count)
{
	EDGE_ASSERT( inputJoints != outputJoints ); // Joint Aliasing Not Permitted

    const Vectormath::Aos::Vector4 unitScale(1.0f, 1.0f, 1.0f, 1.0f);

    int count4 = (count + 3) >> 2;

    for(; count4; count4--, jointLinkage+=8) {
        for(int i=0; i<4; i++) {
            int idxJoint = jointLinkage[2*i+0]; 
            int idxParent = jointLinkage[2*i+1] & 0x7fff;
            bool compensateParent = (jointLinkage[2*i+1] & 0x8000) == 0x8000;

            EdgeAnimJointTransform world;
            const EdgeAnimJointTransform& __restrict__ local = inputJoints[idxJoint];
			const EdgeAnimJointTransform& __restrict__ parentLocal = (idxParent >= 0x4000)? *rootJoint: inputJoints[idxParent];
            const EdgeAnimJointTransform& __restrict__ parentWorld = (idxParent >= 0x4000)? *rootJoint: outputJoints[idxParent];

			Vectormath::Aos::Vector4 scaleCompensate = compensateParent ? Vectormath::Aos::recipPerElem(parentLocal.scale) : unitScale;

            // Rotation
            world.rotation = parentWorld.rotation * local.rotation;
			world.rotation = Vectormath::Aos::normalize(world.rotation);

            // Scale
			world.scale = Vectormath::Aos::mulPerElem(local.scale, parentWorld.scale);
			world.scale = Vectormath::Aos::mulPerElem(world.scale, scaleCompensate);

            // Translation
            Vectormath::Aos::Vector3 ts = Vectormath::Aos::mulPerElem(Vectormath::Aos::Vector3(local.translation), parentWorld.scale.getXYZ());
            Vectormath::Aos::Vector4 q = Vectormath::Aos::Vector4(parentWorld.rotation);

#if (defined(__SPU__) || defined(_MSC_VER))
            float
#else 
            Vectormath::floatInVec 
#endif      
            w = q.getW();
            Vectormath::Aos::Vector3 c = Vectormath::Aos::cross(q.getXYZ(), ts) + w * ts;
            Vectormath::Aos::Vector3 qts = ts + 2.0f * Vectormath::Aos::cross(q.getXYZ(), c);

            world.translation = parentWorld.translation + qts;

            outputJoints[idxJoint] = world;
        }
    }
}
