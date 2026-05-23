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
void edgeAnimJointsToMatrices3x4(void* outputMatrices, 
                                 const EdgeAnimJointTransform* inputJoints, 
                                 unsigned int count)
{
    int count4 = (count + 3) >> 2;

	float* outputMat3x4 = (float*) outputMatrices;

    for (; count4; count4--) {
        for(int i=0; i<4; i++) {
            Vectormath::Aos::Transform3 transform(inputJoints->rotation, Vectormath::Aos::Vector3(inputJoints->translation) );

            transform.setCol0(transform.getCol0() * inputJoints->scale.getX());
            transform.setCol1(transform.getCol1() * inputJoints->scale.getY());
            transform.setCol2(transform.getCol2() * inputJoints->scale.getZ());

			Vectormath::Aos::Vector4 row0 = transform.getRow(0);
			Vectormath::Aos::Vector4 row1 = transform.getRow(1);
			Vectormath::Aos::Vector4 row2 = transform.getRow(2);

			outputMat3x4[0] = row0.getX();
			outputMat3x4[1] = row0.getY();
			outputMat3x4[2] = row0.getZ();
			outputMat3x4[3] = row0.getW();
			outputMat3x4[4] = row1.getX();
			outputMat3x4[5] = row1.getY();
			outputMat3x4[6] = row1.getZ();
			outputMat3x4[7] = row1.getW();
			outputMat3x4[8] = row2.getX();
			outputMat3x4[9] = row2.getY();
			outputMat3x4[10] = row2.getZ();
			outputMat3x4[11] = row2.getW();

            outputMat3x4+=12;
            inputJoints++;
        }
    }
}
