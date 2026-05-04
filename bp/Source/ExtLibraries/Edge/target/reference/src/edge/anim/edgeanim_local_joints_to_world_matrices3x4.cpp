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
void edgeAnimLocalJointsToWorldMatrices3x4(void* outputMatrices,
										   const EdgeAnimJointTransform* inputJoints,
										   const EdgeAnimJointTransform* rootJoint,
										   const unsigned short* jointLinkage, 
										   unsigned int count)
{
	const Vectormath::Aos::Vector4 unitScale(1.0f, 1.0f, 1.0f, 1.0f);
	const Vectormath::Aos::Vector4 wAxis( 0.0f, 0.0f, 0.0f, 1.0f );

	Vectormath::Aos::Transform3 rootMatrix = Vectormath::Aos::Transform3( rootJoint->rotation, Vectormath::Aos::Vector3( rootJoint->translation ) );
	rootMatrix.setCol0(rootMatrix.getCol0() * rootJoint->scale.getX());
	rootMatrix.setCol1(rootMatrix.getCol1() * rootJoint->scale.getY());
	rootMatrix.setCol2(rootMatrix.getCol2() * rootJoint->scale.getZ());

	int count4 = (count + 3) >> 2;

	for(; count4; count4--, jointLinkage+=8) {
		for(int i=0; i<4; i++) {
			const int idxJoint = jointLinkage[2*i+0]; 
			const int idxParent = jointLinkage[2*i+1] & 0x7fff;
			const int idxJointMat = 12 * idxJoint;
			const int idxParentMat = 12 * idxParent;
			const bool isRoot = idxParent >= 0x4000;
			const bool compensateParent = (jointLinkage[2*i+1] & 0x8000) == 0x8000;

			const EdgeAnimJointTransform& local = inputJoints[idxJoint];
			const Vectormath::Aos::Vector4& parentScale = isRoot ? rootJoint->scale : inputJoints[idxParent].scale;
			const Vectormath::Aos::Vector4 scaleCompensate = compensateParent ? Vectormath::Aos::recipPerElem( parentScale ) : unitScale;

			// Parent Matrix
			Vectormath::Aos::Transform3 parent;
			if( isRoot )
			{
				parent = rootMatrix;
			}
			else
			{
				float *parentMat3x4 = (float*) outputMatrices + idxParentMat;
				Vectormath::Aos::Vector4 row0( parentMat3x4[0], parentMat3x4[1], parentMat3x4[2], parentMat3x4[3] );
				Vectormath::Aos::Vector4 row1( parentMat3x4[4], parentMat3x4[5], parentMat3x4[6], parentMat3x4[7] );
				Vectormath::Aos::Vector4 row2( parentMat3x4[8], parentMat3x4[9], parentMat3x4[10], parentMat3x4[11] );
				parent.setRow( 0, row0 );
				parent.setRow( 1, row1 );
				parent.setRow( 2, row2 );
				parent.setRow( 3, wAxis );
			}

			// Compute Output Matrix
			// worldMatrix = worldParent * localTranslate * localScaleCompensate * localRotation * localScale

			// Apply localTranslate * localScaleCompensate
			Vectormath::Aos::Transform3 worldMatrix(
				parent.getCol0() * scaleCompensate.getX(),
				parent.getCol1() * scaleCompensate.getY(),
				parent.getCol2() * scaleCompensate.getZ(),
				Vectormath::Aos::Vector3( parent * local.translation ) );

			// Apply localRotation * localScale
			Vectormath::Aos::Transform3 localRotationScale = Vectormath::Aos::Transform3::rotation( local.rotation );
			localRotationScale.setCol0( localRotationScale.getCol0() * local.scale.getX() );
			localRotationScale.setCol1( localRotationScale.getCol1() * local.scale.getY() );
			localRotationScale.setCol2( localRotationScale.getCol2() * local.scale.getZ() );

			worldMatrix *= localRotationScale;

			// Output
			const Vectormath::Aos::Vector4 row0 = worldMatrix.getRow(0);
			const Vectormath::Aos::Vector4 row1 = worldMatrix.getRow(1);
			const Vectormath::Aos::Vector4 row2 = worldMatrix.getRow(2);
			float* outputMat3x4 = (float*) outputMatrices + idxJointMat;
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
		}
	}
}