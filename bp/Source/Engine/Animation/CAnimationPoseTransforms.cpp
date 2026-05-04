//----------------------------------------------------------------------------
// CAnimationPoseTransforms.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CAnimationPoseTransforms.h"

//----------------------------------------------------------------------------

#include "Engine/Animation/CSkeleton.h"
#include "Engine/Math/CMatrix34.h"

#include "../../ExtLibraries/Edge/Branch/target/ppu/include/edge/anim/edgeanim_ppu.h"

//----------------------------------------------------------------------------

CAnimationPoseTransforms::CAnimationPoseTransforms(CSkeleton const * const pSkeleton)
{
   mJointCount = pSkeleton->GetJointCount();
   uint32 const jointCountAligned = EDGE_ALIGN(mJointCount, 4);
   mpTransforms = (CMatrix34*)BPE_MALLOC_ALIGNED( 16, jointCountAligned * sizeof(CMatrix34) );
   memset(mpTransforms, 0, jointCountAligned * sizeof(CMatrix34));
}

//----------------------------------------------------------------------------

CAnimationPoseTransforms::~CAnimationPoseTransforms()
{
   BPE_FREE_ALIGNED(mpTransforms);
}

