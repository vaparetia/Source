//----------------------------------------------------------------------------
// CAnimationSystem_ProcessBlendTree.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"

//----------------------------------------------------------------------------

#ifdef SPU
#include "cell/dma.h"
#endif

#include "../../ExtLibraries/Edge/Branch/target/ppu/include/edge/anim/edgeanim_ppu.h"
#include "../../ExtLibraries/Edge/Branch/target/spu/include/edge/anim/edgeanim_spu.h"

#include "CAnimationSystem_Helper.h"
#include "NAnimationBlendTree.h"

#include "CAnimationSystem_ProcessBlendTree.h"

//----------------------------------------------------------------------------

namespace
{
   void edgeJointTransformToMatrix34(EdgeAnimJointTransform const & joint, void* matrix)
   {
      float* outputMat3x4 = (float*)matrix;

      Vectormath::Aos::Transform3 transform(joint.rotation, Vectormath::Aos::Vector3(joint.translation) );

      transform.setCol0(transform.getCol0() * joint.scale.getX());
      transform.setCol1(transform.getCol1() * joint.scale.getY());
      transform.setCol2(transform.getCol2() * joint.scale.getZ());

      Vectormath::Aos::Vector4 row0 = transform.getRow(0);
      Vectormath::Aos::Vector4 row1 = transform.getRow(1);
      Vectormath::Aos::Vector4 row2 = transform.getRow(2);

      outputMat3x4[0] = row0.getX();
      outputMat3x4[1] = row1.getX();
      outputMat3x4[2] = row2.getX();
      outputMat3x4[3] = row0.getY();
      outputMat3x4[4] = row1.getY();
      outputMat3x4[5] = row2.getY();
      outputMat3x4[6] = row0.getZ();
      outputMat3x4[7] = row1.getZ();
      outputMat3x4[8] = row2.getZ();
      outputMat3x4[9] = row0.getW();
      outputMat3x4[10] = row1.getW();
      outputMat3x4[11] = row2.getW();
   }

   void leaf_callback_ExtractRootMotion( EdgeAnimSpuContext *pSPUContext,
                                         EdgeAnimBlendLeaf const * const pLeaf,
                                         EdgeAnimAnimation const *pAnim,
                                         EdgeAnimSkeleton const * const pSkel,
                                         int const pipelineStage,
                                         unsigned const dmaTag,
                                         void *pUserScratchBuffer )
   {
      SAnimationLocomotionState* pLocomotionState = (SAnimationLocomotionState*)pUserScratchBuffer;

      // prefetch stage
      if( pipelineStage == -1 )
      {
#ifdef SPU
         cellDmaGet(pLocomotionState, pLeaf->userVal, sizeof(SAnimationLocomotionState), dmaTag, 0, 0);
#else
         memcpy(pLocomotionState, (void*)pLeaf->userVal, sizeof(SAnimationLocomotionState));
#endif
         return;
      }

      // final stage
      if (pipelineStage == 0)
      {

#ifdef SPU
         cellDmaWaitTagStatusAll(1 << dmaTag);
#endif
         EdgeAnimPoseInfo poseLocal;
         edgeAnimPoseStackGetPose(pSPUContext, &poseLocal, 0);
         
         EdgeAnimJointTransform & rootJoint = poseLocal.jointArray[0];

         Vectormath::Aos::Quat loopedRotation = rootJoint.rotation;
         Vectormath::Aos::Point3 loopedTranslation = rootJoint.translation;

         Vectormath::Aos::Quat & lastEvalRotationStorage = reinterpret_cast<Vectormath::Aos::Quat &>(pLocomotionState->mLastEvalRotation);
         Vectormath::Aos::Point3 & lastEvalTranslationStorage = reinterpret_cast<Vectormath::Aos::Point3 &>(pLocomotionState->mLastEvalTranslation);

         Vectormath::Aos::Quat previousRotation = lastEvalRotationStorage;
         Vectormath::Aos::Point3 previousTranslation = lastEvalTranslationStorage;

         lastEvalRotationStorage = rootJoint.rotation;
         lastEvalTranslationStorage = rootJoint.translation;

         if(pLocomotionState->mLoopCount != 0) 
         {
            Vectormath::Aos::Quat const & animationDeltaRotation = reinterpret_cast<Vectormath::Aos::Quat const &>(pLocomotionState->mAnimationDeltaRotation);
            Vectormath::Aos::Vector3 const & animationDeltaTranslation = reinterpret_cast<Vectormath::Aos::Vector3 const &>(pLocomotionState->mAnimationDeltaTranslation);

            if(pLocomotionState->mLoopCount > 0) 
            {
               for(int loop = 0; loop < pLocomotionState->mLoopCount; loop++) 
               {
                  Vectormath::Aos::Transform3 deltaTransform(animationDeltaRotation, animationDeltaTranslation);
                  loopedTranslation = deltaTransform * loopedTranslation;
                  loopedRotation = loopedRotation * animationDeltaRotation;
               }
            }
            else 
            {
               for(int32_t loop = 0; loop < -pLocomotionState->mLoopCount; loop++) 
               {
                  Vectormath::Aos::Quat endDeltaConj = Vectormath::Aos::conj(animationDeltaRotation);
                  Vectormath::Aos::Transform3 deltaTransform(endDeltaConj, Vectormath::Aos::Vector3(0,0,0));
                  loopedTranslation = deltaTransform * (loopedTranslation - animationDeltaTranslation);
                  loopedRotation = endDeltaConj * loopedRotation;
               }
            }
         }

         if( !pLocomotionState->mLastEvalValid )
         {
            pLocomotionState->mLastEvalValid = true;

            previousRotation = loopedRotation;
            previousTranslation = loopedTranslation;
         }

         Vectormath::Aos::Transform3 conjRot(Vectormath::Aos::conj(previousRotation), Vectormath::Aos::Vector3(0,0,0));
         Vectormath::Aos::Quat deltaRotation = loopedRotation * Vectormath::Aos::conj(previousRotation);
         Vectormath::Aos::Vector3 deltaTranslation = conjRot * (loopedTranslation - previousTranslation);

         rootJoint.rotation = Vectormath::Aos::normalize(deltaRotation);
         rootJoint.translation = Vectormath::Aos::Point3(deltaTranslation);

#ifdef SPU
         // dma updated locomotion state
         cellDmaPut(pLocomotionState, pLeaf->userVal, sizeof(SAnimationLocomotionState), dmaTag, 0, 0);
         cellDmaWaitTagStatusAll(1 << dmaTag);
#else
         memcpy((void*)pLeaf->userVal, (void*)pLocomotionState, sizeof(SAnimationLocomotionState));
#endif
      }
   }
}

//----------------------------------------------------------------------------

void AnimationSystem_ProcessBlendTree(void* pOutPoseWS, 
                                      NAnimationSystem::ERootMotionExtraction const rootNodeAdjustment,
                                      void* pOutRootDelta,
                                      EdgeAnimPpuContext const * pPPUContext,
                                      int const spuId,
                                      EdgeAnimSkeleton const * pSkeleton,
                                      EdgeAnimBlendBranch const * pBranches,
                                      int const numBranches,
                                      EdgeAnimBlendLeaf const * pLeaves,
                                      int const numLeaves,
                                      uint16 const rootNodeId,
                                      EdgeAnimJointTransform const * pRootJoint,
                                      void* pScratchBuffer,
                                      int const sizeScratchBuffer)
{
   EdgeAnimSpuContext spuContext;
   edgeAnimSpuInitialize(&spuContext, pPPUContext, 0, pScratchBuffer, sizeScratchBuffer, pSkeleton->numJoints, pSkeleton->numUserChannels, EDGE_ANIM_EVAL_BUFFER_SIZE, sizeof(SAnimationLocomotionState));

   EdgeAnimLeafCallback leafCallback = NULL;
   if (rootNodeAdjustment != NAnimationSystem::kRootMotionExtraction_None)
      leafCallback = leaf_callback_ExtractRootMotion;

   edgeAnimProcessBlendTree(&spuContext, rootNodeId, pBranches, numBranches, pLeaves, numLeaves, pSkeleton, NULL, 0, NULL, leafCallback);

   // Convert local joints to world joints 
   EdgeAnimPoseInfo poseLocal;
   EdgeAnimPoseInfo poseWorld;
   edgeAnimPoseStackPush(&spuContext);
   edgeAnimPoseStackGetPose(&spuContext, &poseLocal, 1);
   edgeAnimPoseStackGetPose(&spuContext, &poseWorld, 0);

   if( rootNodeAdjustment != NAnimationSystem::kRootMotionExtraction_None && pOutRootDelta )
   {
      EdgeAnimJointTransform rootDelta = poseLocal.jointArray[0];

      edgeJointTransformToMatrix34(rootDelta, pOutRootDelta);

      poseLocal.jointArray[0].rotation = Vectormath::Aos::Quat::identity();
      poseLocal.jointArray[0].translation = Vectormath::Aos::Point3(0.0f);
   }

   edgeAnimLocalJointsToWorldJoints(poseWorld.jointArray, poseLocal.jointArray, pRootJoint, pSkeleton->simdHierarchy, pSkeleton->numSimdHierarchyQuads * 4);

   // Convert world joint to matrices (in the output buffer)
   edgeAnimJointsToMatrices4x3(pOutPoseWS, poseWorld.jointArray, pSkeleton->numJoints);

   // Empty the stack
   edgeAnimPoseStackPop(&spuContext); // pop world joints
   edgeAnimPoseStackPop(&spuContext); // pop local joints

   // Finalize
   edgeAnimSpuFinalize(&spuContext);
}
