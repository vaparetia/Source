//----------------------------------------------------------------------------
// CAnimationPoseTransforms.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class CMatrix34;
class CSkeleton;

//----------------------------------------------------------------------------

class ENGINE_API CAnimationPoseTransforms
{
public:
   CAnimationPoseTransforms(CSkeleton const * const pSkeleton);
   ~CAnimationPoseTransforms();

public:
   CMatrix34 * mpTransforms;
   uint32      mJointCount;   // unaligned joint count (actual allocation is 4 aligned)

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CAnimationPoseTransforms);
};
