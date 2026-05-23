//----------------------------------------------------------------------------
// CAnimationBlendTree.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

class CAnimation;

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/StlExtras/reserved_vector.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Math/CQuaternion.h"

//----------------------------------------------------------------------------

#include "Engine/Animation/NAnimationBlendTree.h"

//----------------------------------------------------------------------------

struct SAnimationBlendLeaf
{
   char data[16];          // sizeof(EdgeAnimBlendLeaf)
};

struct SAnimationBlendBranch
{
   char data[16];          // sizeof(EdgeAnimBlendBranch)
};

class ENGINE_API CAnimationBlendTree
{
public:
   enum ENodeType
   {
      kNT_Leaf,
      kNT_Branch,
      kNT_Invalid = -1
   };

   struct SBranchInfo
   {
      SBranchInfo()
         : mTotalWeight(0.0f)
         , mNodeType(kNT_Invalid)
         , mNodeIndex(-1)
      {
      }

      real32    mTotalWeight;
      ENodeType mNodeType;
      int       mNodeIndex;      
   };

public:
   CAnimationBlendTree();

   // NOTE: pLocomotionState must either be NULL or by a pointer to data that will stay around until the blend tree is processed (if it is done asynchronously until the next frame).
   void AddAnimation(CAnimation const * const pAnimation, SAnimationLocomotionState * pLocomotionState, real32 const time, real32 const weight);

   void AddAdditiveBlendTree(CAnimationBlendTree const * const pAnimation, real32 const weighting );

   // Placeholder function for basic CCEAdditiveBlend2_customAnimationNode multiple additives.
   // TODO: Rework additive blend tree implementation.
   void AddAdditiveBranch(SBranchInfo const &leftBranchInfo, SBranchInfo const &rightBranchInfo);
   
   bool const HasNodes() const { return !mBranches.empty() || !mLeaves.empty(); }
public:
   SBranchInfo    mBranchInfo;

   bpe::reserved_vector<SAnimationBlendBranch, 64> mBranches;
   bpe::reserved_vector<SAnimationBlendLeaf, 64>   mLeaves;
};

