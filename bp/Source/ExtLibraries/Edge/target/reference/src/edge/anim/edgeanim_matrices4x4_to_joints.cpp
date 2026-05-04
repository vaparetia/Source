/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"

/*
 * Reference C++ implementation using vectormath.
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimMatrices4x4ToJoints(EdgeAnimJointTransform* outputJoints, 
                                 const void* inputMatrices, 
                                 unsigned int count)
{
	Vectormath::Aos::Transform3* inputTransforms = (Vectormath::Aos::Transform3*)inputMatrices;
    int count4 = (count + 3) >> 2;

    for (; count4; count4--) {
        for(int i=0; i<4; i++) {
            float sx = length(inputTransforms->getCol0());
            float sy = length(inputTransforms->getCol1());
            float sz = length(inputTransforms->getCol2());

            Vectormath::Aos::Vector4 scale(sx,sy,sz,1.0f);      
            Vectormath::Aos::Quat rotation(inputTransforms->getUpper3x3());
            Vectormath::Aos::Point3 translation = Vectormath::Aos::Point3(inputTransforms->getTranslation());

            outputJoints->rotation = normalize(rotation);
            outputJoints->translation = translation;
            outputJoints->scale = scale;

            inputTransforms++;
            outputJoints++;
        }
    }
}
