//----------------------------------------------------------------------------
// CAnimationSystem.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CAnimationSystem.h"

//----------------------------------------------------------------------------

#include "Engine/Animation/CAnimation.h"
#include "Engine/Animation/CAnimationBlendTree.h"
#include "Engine/Animation/CAnimationPoseTransforms.h"
#include "Engine/Animation/CSkeleton.h"
#include "Engine/Animation/CAnimationSystem_ProcessBlendTree.h"
#include "Engine/Animation/CAnimationSystem_Helper.h"
#include "Engine/Math/CQuaternion.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "Engine/System/CSPURSManager.h"
#include "cell/spurs.h"
#endif

//----------------------------------------------------------------------------

#include "../../ExtLibraries/Edge/Branch/target/ppu/include/edge/anim/edgeanim_ppu.h"
#include "../../ExtLibraries/Edge/Branch/target/spu/include/edge/anim/edgeanim_spu.h"

//----------------------------------------------------------------------------

#define SPURS_SPU_NUM   5
#define EDGE_ANIM_EXTERNAL_POSE_CACHE_MASK  -1      // all SPUs need external pose cache space in main memory
#define EDGE_ANIM_EXTERNAL_POSE_CACHE   (128*1024)  // arbitrarily allocate 128KB for each SPUs pose stack extension in main memory

//----------------------------------------------------------------------------

namespace
{
   bool                 gAnimationSystemInitialized = false;

   EdgeAnimPpuContext * gEdgeAnimPpuContext = NULL;
   void *               gExternalSpuStorage = NULL;
   void *               gScratchBuffer = NULL;

#if BPE_TARGET == BPE_TARGET_PS3
   uint64_t             gCommandList[1024];
   int                  gCurrentCommandIndex = 0;

   bool                 gJobChainInitialized = false;
   CellSpursJobChain    gJobChain;

   void PrepareJobChainForAdd(int const commandsToAdd)
   {
      // Need to keep the END at the very end of the command list.
      int const commandListSize = BPE_ARRAY_SIZE(gCommandList) - 1;

      // Check if there is more room in the job chain.
      if( gJobChainInitialized && (gCurrentCommandIndex + commandsToAdd) >= BPE_ARRAY_SIZE(gCommandList) )
      {
         // The job chain is full, need to shut it down and wait for it to finish.
         int res = cellSpursShutdownJobChain(&gJobChain);
         BPE_ASSERT_NO_MSG(res == CELL_OK);
         
         res = cellSpursJoinJobChain(&gJobChain);
         BPE_ASSERT_NO_MSG(res == CELL_OK);

         gJobChainInitialized = false;
      }
   
      // Check if we need to initialize the job chain.
      if( !gJobChainInitialized )
      {
         gJobChainInitialized = true;

         const unsigned int MAX_CONTENTION = 1;
         const uint8_t WORKLOAD_PRIORITIES[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

         static CellSpursJobChainAttribute attr;
         int res = cellSpursJobChainAttributeInitialize(&attr, gCommandList, sizeof(CellSpursJob256), 16, WORKLOAD_PRIORITIES, MAX_CONTENTION, true, 0, 1, false, 256, 0);
         BPE_ASSERT_NO_MSG(res == CELL_OK);

         res = cellSpursCreateJobChainWithAttribute(gpSpursManager->mpSPURS_Animation.get(), &gJobChain, &attr);
         BPE_ASSERT_NO_MSG(res == CELL_OK);

         // initialize command list with ends
         for( int i = 0; i < BPE_ARRAY_SIZE(gCommandList); ++i )
         {
            gCommandList[i] = CELL_SPURS_JOB_COMMAND_END;
         }

         gCurrentCommandIndex = 0;
      }
   }

   void KickJobChain()
   {
      int res = cellSpursRunJobChain(&gJobChain);
      BPE_ASSERT_NO_MSG(res == CELL_OK);
   }
#endif
}

//----------------------------------------------------------------------------

CAnimationSystem::CAnimationSystem()
{
   BPE_VERIFY(gAnimationSystemInitialized == false, false, "Animation system already initialized");
   gAnimationSystemInitialized = true;

   gEdgeAnimPpuContext = (EdgeAnimPpuContext*)BPE_MALLOC_ALIGNED(16, sizeof(*gEdgeAnimPpuContext));
   gExternalSpuStorage = BPE_MALLOC_ALIGNED(16, edgeAnimComputeExternalStorageSize( SPURS_SPU_NUM, EDGE_ANIM_EXTERNAL_POSE_CACHE_MASK, EDGE_ANIM_EXTERNAL_POSE_CACHE));
   edgeAnimPpuInitialize(gEdgeAnimPpuContext, SPURS_SPU_NUM, EDGE_ANIM_EXTERNAL_POSE_CACHE_MASK, EDGE_ANIM_EXTERNAL_POSE_CACHE, gExternalSpuStorage);

   // animation scratch buffer when running on PC
   gScratchBuffer = (uint8_t*)BPE_MALLOC_ALIGNED(16, EDGE_ANIM_EXTERNAL_POSE_CACHE);
}

//----------------------------------------------------------------------------

CAnimationSystem::~CAnimationSystem()
{
   gAnimationSystemInitialized = false;

   BPE_FREE_ALIGNED(gScratchBuffer);

   edgeAnimPpuFinalize(gEdgeAnimPpuContext);
   BPE_FREE_ALIGNED(gEdgeAnimPpuContext);
   BPE_FREE_ALIGNED(gExternalSpuStorage);
}

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_PS3

#include "cell/atomic.h"

extern char _binary_job_Animation_Job_SPU_bin_start[];
extern char _binary_job_Animation_Job_SPU_bin_size[];

#endif

void CAnimationSystem::ProcessBlendTree(CAnimationPoseTransforms * pOutPoseWS, 
                                        CSkeleton const * const pSkeleton, 
                                        CAnimationBlendTree const * const pBlendTree,
                                        NAnimationSystem::ERootMotionExtraction const rootNodeAdjustment,
                                        CMatrix34 * pOutRootDelta)
{
   EdgeAnimJointTransform rootJoint;
   {
      rootJoint.rotation = Vectormath::Aos::Quat::identity();
      rootJoint.translation = Vectormath::Aos::Point3(0.0f);
      rootJoint.scale = Vectormath::Aos::Vector4(1.0f);
   }

   EdgeAnimSkeleton const * pEdgeSkeleton = (EdgeAnimSkeleton const *)pSkeleton->mpData;

   uint16 rootNodeId = pBlendTree->mBranchInfo.mNodeIndex;
   switch(pBlendTree->mBranchInfo.mNodeType)
   {
   case CAnimationBlendTree::kNT_Leaf: 
      rootNodeId |= EDGE_ANIM_BLEND_TREE_INDEX_LEAF;
      break;
   case CAnimationBlendTree::kNT_Branch:
      rootNodeId |= EDGE_ANIM_BLEND_TREE_INDEX_BRANCH;
      break;
   }

   EdgeAnimBlendBranch const * pBranches = (EdgeAnimBlendBranch const *)pBlendTree->mBranches.data();
   uint32 const branchCount = pBlendTree->mBranches.size();

   EdgeAnimBlendLeaf const * pLeaves = (EdgeAnimBlendLeaf const *)pBlendTree->mLeaves.data();
   uint32 const leafCount = pBlendTree->mLeaves.size();

#if BPE_TARGET == BPE_TARGET_PS3
   PrepareJobChainForAdd(2);

   static CMatrix34 EDGE_ALIGNED(16) rootDelta(CMatrix34::Identity());

   static int EDGE_ALIGNED(16) isDone;

   static CellSpursJob256 job;
   memset(&job, 0, sizeof(job));

   job.header.eaBinary = (uintptr_t)_binary_job_Animation_Job_SPU_bin_start;
   job.header.sizeBinary =  CELL_SPURS_GET_SIZE_BINARY(_binary_job_Animation_Job_SPU_bin_size);

   int userDataCount = 0;

   // HACK:
   static EdgeAnimBlendBranch EDGE_ALIGNED(16) sBranchesAligned[64];
   static EdgeAnimBlendLeaf EDGE_ALIGNED(16) sLeavesAligned[64];

   int const branchDataSize = branchCount * sizeof(EdgeAnimBlendBranch);
   memcpy(sBranchesAligned, pBranches, branchDataSize);

   int const leavesDataSize = leafCount * sizeof(EdgeAnimBlendLeaf);
   memcpy(sLeavesAligned, pLeaves, leavesDataSize);

   int startInputDmaList = userDataCount;
   int res = cellSpursJobGetInputList(&job.workArea.dmaList[userDataCount++], EDGE_ALIGN(branchCount * sizeof(EdgeAnimBlendBranch), 16U), (uint32_t)sBranchesAligned);
   BPE_ASSERT_NO_MSG(res == CELL_OK);
   res = cellSpursJobGetInputList(&job.workArea.dmaList[userDataCount++], EDGE_ALIGN(leafCount * sizeof(EdgeAnimBlendLeaf), 16U), (uint32_t)(sLeavesAligned));
   BPE_ASSERT_NO_MSG(res == CELL_OK);

   job.header.sizeDmaList = (userDataCount - startInputDmaList) * sizeof(job.workArea.dmaList[0]);

   for(int listLoop = startInputDmaList; listLoop < userDataCount; listLoop++) 
   {
      job.header.sizeInOrInOut += (job.workArea.dmaList[listLoop] >> 32);
   }

   // read-only cached buffer contains the global context (pose cache per spu etc) and the skeleton (since we assume it's shared by many characters)
   uint32_t startCacheDmaList = userDataCount;
   res = cellSpursJobGetCacheList(&job.workArea.dmaList[userDataCount++], EDGE_ALIGN(sizeof(*gEdgeAnimPpuContext), 16), reinterpret_cast<uintptr_t>(gEdgeAnimPpuContext));
   BPE_ASSERT_NO_MSG(res == CELL_OK);
   res = cellSpursJobGetCacheList(&job.workArea.dmaList[userDataCount++], EDGE_ALIGN(pEdgeSkeleton->sizeTotal, 16), reinterpret_cast<uintptr_t>(pEdgeSkeleton));
   BPE_ASSERT_NO_MSG(res == CELL_OK);
   job.header.sizeCacheDmaList = (userDataCount - startCacheDmaList) * sizeof(job.workArea.dmaList[0]);

   // output buffer contains world matrices (warning: for the internal buffer, numMatrices must aligned to a multiple of 4)
   unsigned int numJointsAligned = EDGE_ALIGN(pEdgeSkeleton->numJoints, 4U);
   job.header.sizeOut = EDGE_ALIGN(numJointsAligned * 12 * sizeof(float), 16U);

   // Scratch buffer
   // TODO: use edgeAnimGetPoseSize to calculate correct scratch size. Scratch must be at least: 3 * (maxSizeEvalBuffer + maxSizeUserBuffer + sizePose).
   // Assuming default eval buffer of 8k and userBuffer of 0, this allows for 273 joints currently.
   job.header.sizeScratch = 64000U >> 4U;

   // add root joint (must be aligned to 16 bytes / match code in cellSpursJobMain on the SPU side)
   BPE_ASSERT(EDGE_IS_ALIGNED(&job.workArea.userData[userDataCount], 16), "root joint storage not aligned");
   memcpy(&job.workArea.userData[userDataCount], &rootJoint, sizeof(rootJoint));    
   userDataCount += EDGE_ALIGN(sizeof(rootJoint), sizeof(job.workArea.userData[0])) / sizeof(job.workArea.userData[0]);

   // additional parameters (must match code in cellSpursJobMain on the SPU side)
   assert(EDGE_IS_ALIGNED(pOutPoseWS->mpTransforms, 16));
   job.workArea.userData[userDataCount++] = (static_cast<uint64_t>(branchCount) << (32+16)) | (static_cast<uint64_t>(leafCount) << 32) | (static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pOutPoseWS->mpTransforms)));

   job.workArea.userData[userDataCount++] = (static_cast<uint64_t>(rootNodeId) << (32 + 16)) | (static_cast<uint64_t>(rootNodeAdjustment) << 32) | (uint64_t)(&rootDelta);
   job.workArea.userData[userDataCount++] = (uint64_t)&isDone << 32;

   BPE_ASSERT(userDataCount <= (sizeof(job.workArea.userData) / sizeof(job.workArea.userData[0])), "userDataCount overflow");

   gCommandList[gCurrentCommandIndex + 0] = CELL_SPURS_JOB_COMMAND_JOB(&job);
   gCommandList[gCurrentCommandIndex + 1] = CELL_SPURS_JOB_COMMAND_SYNC;
   gCurrentCommandIndex += 2;

   isDone = 0;

   KickJobChain();

   // Wait for job to be processed.
   while( !cellAtomicNop32((uint32_t*)&isDone) )
   {
   }

   if( pOutRootDelta )
   {
      *pOutRootDelta = rootDelta;
   }

#else
   AnimationSystem_ProcessBlendTree(pOutPoseWS->mpTransforms, rootNodeAdjustment, pOutRootDelta, gEdgeAnimPpuContext, 0, pEdgeSkeleton, pBranches, branchCount, pLeaves, leafCount, rootNodeId, &rootJoint, gScratchBuffer, EDGE_ANIM_EXTERNAL_POSE_CACHE);
#endif
}

//----------------------------------------------------------------------------
// We can create an optimized version of this later.
void CAnimationSystem::ConvertTransformsToBindSpace(CSkeleton const * pSkeleton, CAnimationPoseTransforms const * pPoseWS, CAnimationPoseTransforms * pPoseBS_Out)
{
   EdgeAnimSkeleton const * pEdgeSkeleton = (EdgeAnimSkeleton const *)pSkeleton->mpData;
   CAnimationPoseTransforms const * pPoseBSInv = pSkeleton->mpPoseBSInv;

   CMatrix34 const *pPoseWS_mat = pPoseWS->mpTransforms;
   CMatrix34 const *pPoseBSInv_mat = pPoseBSInv->mpTransforms;
   CMatrix34 *pPoseBS_Out_mat = pPoseBS_Out->mpTransforms;

   for( int i = 0; i < pEdgeSkeleton->numJoints; ++i )
   {
      CMatrix34::MatrixMultiply_Inplace(*pPoseWS_mat, *pPoseBSInv_mat, *pPoseBS_Out_mat);
      pPoseWS_mat++;
      pPoseBSInv_mat++;
      pPoseBS_Out_mat++;
   }
}

//----------------------------------------------------------------------------

void CAnimationSystem::CalculateInvBindSpaceTransforms(CSkeleton const * pSkeleton, CAnimationPoseTransforms * pPoseBSInv)
{
   EdgeAnimSkeleton const * pEdgeSkeleton = (EdgeAnimSkeleton const *)pSkeleton->mpData;

   EdgeAnimJointTransform* pBasePoseJoints = EDGE_OFFSET_GET_POINTER(EdgeAnimJointTransform, pEdgeSkeleton->offsetBasePose);

   uint32 const numJointsAligned = EDGE_ALIGN(pSkeleton->GetJointCount(), 4U);
   edgeAnimJointsToMatrices4x3(pPoseBSInv->mpTransforms, pBasePoseJoints, numJointsAligned);

   // Calculate inverse
   for( int i = 0; i < pEdgeSkeleton->numJoints; ++i )
   {
      pPoseBSInv->mpTransforms[i] = pPoseBSInv->mpTransforms[i].Inverse();
   }
}