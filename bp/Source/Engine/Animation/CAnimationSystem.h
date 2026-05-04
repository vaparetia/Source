//----------------------------------------------------------------------------
// CAnimationSystem.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "NAnimationSystem.h"

//----------------------------------------------------------------------------

class CAnimation;
class CAnimationBlendTree;
class CAnimationPoseTransforms;
class CMatrix34;
class CSkeleton;

//----------------------------------------------------------------------------

class ENGINE_API CAnimationSystem
{
public:
   CAnimationSystem();
   ~CAnimationSystem();

   static void ProcessBlendTree(CAnimationPoseTransforms * pOutPoseWS, 
                                CSkeleton const * const pSkeleton, 
                                CAnimationBlendTree const * const pBlendTree,
                                NAnimationSystem::ERootMotionExtraction const rootNodeAdjustment = NAnimationSystem::kRootMotionExtraction_None,
                                CMatrix34 * pOutRootDelta = NULL);

   static void ConvertTransformsToBindSpace(CSkeleton const * pSkeleton,
                                            CAnimationPoseTransforms const * pPoseWS,
                                            CAnimationPoseTransforms * pPoseBS_Out);

   static void CalculateInvBindSpaceTransforms(CSkeleton const * pSkeleton,
                                               CAnimationPoseTransforms * pPoseBSInv);
};
