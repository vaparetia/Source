//----------------------------------------------------------------------------
// CAnimationBlendTree.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CAnimationBlendTree.h"

//----------------------------------------------------------------------------

#include "Engine/Animation/CAnimation.h"

#include "../../ExtLibraries/Edge/Branch/target/ppu/include/edge/anim/edgeanim_ppu.h"

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(SAnimationBlendLeaf) == sizeof(EdgeAnimBlendLeaf));
BPE_CTASSERT(sizeof(SAnimationBlendBranch) == sizeof(EdgeAnimBlendBranch));

//----------------------------------------------------------------------------

CAnimationBlendTree::CAnimationBlendTree()
{
}

//----------------------------------------------------------------------------
// Add an animation leaf and a branch for blending if required.
// This function is useful for adding multiple animations without having to worry about blending.
void CAnimationBlendTree::AddAnimation(CAnimation const * const pAnimation, SAnimationLocomotionState * pLocomotionState, real32 const time, real32 const weight)
{
   if( weight <= 0.0f )
   {
      return;
   }

   // Add animation leaf
   EdgeAnimAnimation * pEdgeAnimation = (EdgeAnimAnimation *)pAnimation->mpData;

   SAnimationBlendLeaf leafData;
   EdgeAnimBlendLeaf leaf(pEdgeAnimation, pEdgeAnimation->sizeHeader, time, 0, (uint32_t)pLocomotionState);
   memcpy(&leafData, &leaf, sizeof(EdgeAnimBlendLeaf));

   int const leafIndex = mLeaves.size();

   mLeaves.push_back(leafData);

   mBranchInfo.mTotalWeight += weight;
   
   real32 const currentRelativeWeight = weight / mBranchInfo.mTotalWeight;

   int const lastNodeIndex = mBranchInfo.mNodeIndex;
   ENodeType const lastNodeType = mBranchInfo.mNodeType;

   // Add blend branch if required.
   switch( lastNodeType )
   {
   case kNT_Invalid:
      mBranchInfo.mNodeIndex = leafIndex;
      mBranchInfo.mNodeType = kNT_Leaf;
      break;
   
   case kNT_Leaf:
      {
         mBranchInfo.mNodeIndex = mBranches.size();
         mBranchInfo.mNodeType = kNT_Branch;

         SAnimationBlendBranch branchData;
         EdgeAnimBlendBranch branch(EDGE_ANIM_BLENDOP_BLEND_LINEAR, lastNodeIndex | EDGE_ANIM_BLEND_TREE_INDEX_LEAF, leafIndex | EDGE_ANIM_BLEND_TREE_INDEX_LEAF, currentRelativeWeight);
         memcpy(&branchData, &branch, sizeof(EdgeAnimBlendBranch));

         mBranches.push_back(branchData);
      }
      break;
   
   case kNT_Branch:
      {
         mBranchInfo.mNodeIndex = mBranches.size();
         mBranchInfo.mNodeType = kNT_Branch;

         SAnimationBlendBranch branchData;
         EdgeAnimBlendBranch branch(EDGE_ANIM_BLENDOP_BLEND_LINEAR, lastNodeIndex | EDGE_ANIM_BLEND_TREE_INDEX_BRANCH, leafIndex | EDGE_ANIM_BLEND_TREE_INDEX_LEAF, currentRelativeWeight);
         memcpy(&branchData, &branch, sizeof(EdgeAnimBlendBranch));

         mBranches.push_back(branchData);
      }
      break;
   }
}
//----------------------------------------------------------------------------

void CAnimationBlendTree::AddAdditiveBlendTree(CAnimationBlendTree const * const pAdditiveBlendTree, real32 const weighting)
{
   SAnimationBlendBranch branchData;

   uint16 leftFlags = EDGE_ANIM_BLEND_TREE_INDEX_LEAF;
   uint16 leftIndex = mBranchInfo.mNodeIndex;

   if (mBranchInfo.mNodeType == kNT_Branch)
   {
      leftFlags = EDGE_ANIM_BLEND_TREE_INDEX_BRANCH;
   }

   int16 rightFlags = EDGE_ANIM_BLEND_TREE_INDEX_LEAF;
   int16 rightIndex = (int16) pAdditiveBlendTree->mBranchInfo.mNodeIndex + mLeaves.size();

   if (pAdditiveBlendTree->mBranchInfo.mNodeType == kNT_Branch)
   {
      rightFlags = EDGE_ANIM_BLEND_TREE_INDEX_BRANCH;
      rightIndex = (int16) pAdditiveBlendTree->mBranchInfo.mNodeIndex + mBranches.size() + 1;
   }

   EdgeAnimBlendBranch branch(EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_RIGHT, leftIndex | leftFlags, rightIndex | rightFlags, weighting );
   memcpy(&branchData, &branch, sizeof(EdgeAnimBlendBranch));

   // Setup initial node
   mBranchInfo.mNodeIndex = mBranches.size();
   mBranchInfo.mNodeType = kNT_Branch;

   // Now add additive branch
   mBranches.push_back(branchData);

   // Now add blend tree
   int16 const numStartingLeaves = (int16) mLeaves.size();
   int16 const numStartingBranches = (int16) mBranches.size();

   // Branches
   foreach (SAnimationBlendBranch const &branch, pAdditiveBlendTree->mBranches)
   {
      EdgeAnimBlendBranch const &edgeBranch =  reinterpret_cast<EdgeAnimBlendBranch const &>(branch);
      EdgeAnimBlendBranch offsetEdgeBranch = edgeBranch;

      if ((offsetEdgeBranch.left & EDGE_ANIM_BLEND_TREE_INDEX_LEAF) != 0) 
      {
         offsetEdgeBranch.left += numStartingLeaves;
      }
      else
      {
         offsetEdgeBranch.left += numStartingBranches;
      }

      if ((offsetEdgeBranch.right & EDGE_ANIM_BLEND_TREE_INDEX_LEAF) != 0) 
      {
         offsetEdgeBranch.right += numStartingLeaves;
      }
      else
      {
         offsetEdgeBranch.right += numStartingBranches;
      }

      mBranches.push_back(reinterpret_cast<SAnimationBlendBranch &>(offsetEdgeBranch));
   }

   // Leaves
   foreach (SAnimationBlendLeaf const &leaf, pAdditiveBlendTree->mLeaves)
   {
      // Leaves don't have indices to offset
      mLeaves.push_back(leaf);
   }
}

//----------------------------------------------------------------------------

void CAnimationBlendTree::AddAdditiveBranch(SBranchInfo const &leftBranchInfo, SBranchInfo const &rightBranchInfo)
{
   SAnimationBlendBranch branchData;

   uint16 leftFlags = EDGE_ANIM_BLEND_TREE_INDEX_LEAF;
   uint16 leftIndex = leftBranchInfo.mNodeIndex;

   if (leftBranchInfo.mNodeType == CAnimationBlendTree::kNT_Branch)
   {
      leftFlags = EDGE_ANIM_BLEND_TREE_INDEX_BRANCH;
   }

   int16 rightFlags = EDGE_ANIM_BLEND_TREE_INDEX_LEAF;
   int16 rightIndex = (int16) rightBranchInfo.mNodeIndex;

   if (rightBranchInfo.mNodeType == CAnimationBlendTree::kNT_Branch)
   {
      rightFlags = EDGE_ANIM_BLEND_TREE_INDEX_BRANCH;
   }

   EdgeAnimBlendBranch branch(EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_RIGHT, leftIndex | leftFlags, rightIndex | rightFlags, rightBranchInfo.mTotalWeight );
   memcpy(&branchData, &branch, sizeof(EdgeAnimBlendBranch));

   // Setup additive branch
   mBranchInfo.mNodeIndex = mBranches.size();
   mBranchInfo.mNodeType = CAnimationBlendTree::kNT_Branch;

   // Add additive branch
   mBranches.push_back(branchData);

   // Reset weight (TODO: Check to see if this makes sense)
   mBranchInfo.mTotalWeight = leftBranchInfo.mTotalWeight + rightBranchInfo.mTotalWeight;
}
