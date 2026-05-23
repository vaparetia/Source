//----------------------------------------------------------------------------
// CAnimationSystem_ProcessBlendTree.h
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "NAnimationSystem.h"

//----------------------------------------------------------------------------

struct EdgeAnimPpuContext;
struct EdgeAnimJointTransform;
struct EdgeAnimSkeleton;
struct EdgeAnimBlendLeaf;
struct EdgeAnimBlendBranch;

//----------------------------------------------------------------------------

void AnimationSystem_ProcessBlendTree(void * pOutPoseWS, //(CMatrix34*)
                                      NAnimationSystem::ERootMotionExtraction const rootNodeAdjustment,
                                      void * pOutRootDelta, //(CMatrix34*)
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
                                      int const sizeScratchBuffer);

//----------------------------------------------------------------------------