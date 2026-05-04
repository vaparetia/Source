//----------------------------------------------------------------------------
// CAnimationSystem_Helper.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "../../ExtLibraries/Edge/Branch/target/spu/include/edge/anim/edgeanim_spu.h"

//----------------------------------------------------------------------------

void edgeAnimJointsToMatrices4x3(void* outputMatrices, const EdgeAnimJointTransform* inputJoints, int count)
{
   int count4 = (count + 3) >> 2;

   float* outputMat3x4 = (float*) outputMatrices;

   for (; count4; count4--) 
   {
      for(int i = 0; i < 4; i++) 
      {
         Vectormath::Aos::Transform3 transform(inputJoints->rotation, Vectormath::Aos::Vector3(inputJoints->translation) );

         Vectormath::Aos::Vector3 col0 = transform.getCol0() * inputJoints->scale.getX();
         Vectormath::Aos::Vector3 col1 = transform.getCol1() * inputJoints->scale.getY();
         Vectormath::Aos::Vector3 col2 = transform.getCol2() * inputJoints->scale.getZ();
         Vectormath::Aos::Vector3 col3 = transform.getCol3();

         outputMat3x4[0] = col0.getX();
         outputMat3x4[1] = col0.getY();
         outputMat3x4[2] = col0.getZ();
         
         outputMat3x4[3] = col1.getX();
         outputMat3x4[4] = col1.getY();
         outputMat3x4[5] = col1.getZ();
         
         outputMat3x4[6] = col2.getX();
         outputMat3x4[7] = col2.getY();
         outputMat3x4[8] = col2.getZ();
         
         outputMat3x4[9] = col3.getX();
         outputMat3x4[10] = col3.getY();
         outputMat3x4[11] = col3.getZ();

         outputMat3x4 += 12;
         inputJoints++;
      }
   }
}
