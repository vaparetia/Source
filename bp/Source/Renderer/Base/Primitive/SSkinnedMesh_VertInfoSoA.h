//----------------------------------------------------------------------------
// SSkinnedMesh_VertInfoSoA.h
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"

//----------------------------------------------------------------------------
// SoA layout for initial skinned meshes.
// Data is laid out in xxxx,yyyy,zzzz  in the
// order of pos, normal, binormal, sbinormal.
// This works much better with SSE.
struct SSkinnedMesh_VertInfoSoA
{
   enum EDataOrder   // For reference
   {
      kPos,
      kNorm,
      kBinorm,
      kSbinorm
   };

   real32   mX[4];
   real32   mY[4];
   real32   mZ[4];
   real32   mWeights[3];   // Weights for 4 bones, bone 4 weight is 1 - (mWeights[0] + mWeights[1] + mWeights[2])
   uint32   mBoneIndices;
};

