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
void edgeAnimMatrices3x4ToJoints(EdgeAnimJointTransform* outputJoints, 
                                 const void* inputMatrices, 
                                 unsigned int count)
{
    int count4 = (count + 3) >> 2;

    const float* inputMat3x4 = (const float*) inputMatrices;

    for (; count4; count4--) {
        for(int i=0; i<4; i++) {
            Vectormath::Aos::Transform3 mat;
            mat.setCol0(Vectormath::Aos::Vector3(inputMat3x4[0], inputMat3x4[4], inputMat3x4[8]));
            mat.setCol1(Vectormath::Aos::Vector3(inputMat3x4[1], inputMat3x4[5], inputMat3x4[9]));
            mat.setCol2(Vectormath::Aos::Vector3(inputMat3x4[2], inputMat3x4[6], inputMat3x4[10]));
            mat.setCol3(Vectormath::Aos::Vector3(inputMat3x4[3], inputMat3x4[7], inputMat3x4[11]));

            float sx = length(mat.getCol0());
            float sy = length(mat.getCol1());
            float sz = length(mat.getCol2());

            Vectormath::Aos::Vector4 scale(sx,sy,sz,1.0f);      
            Vectormath::Aos::Quat rotation(mat.getUpper3x3());
            Vectormath::Aos::Point3 translation = Vectormath::Aos::Point3(mat.getTranslation());

            outputJoints->rotation = normalize(rotation);
            outputJoints->translation = translation;
            outputJoints->scale = scale;

            inputMat3x4 += 12;
            outputJoints++;
        }
    }
}
