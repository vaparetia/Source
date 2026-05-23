#include "StdAfx.h"
#include "Engine/Animation/CAnimationSystem_ProcessBlendTree.h"
#include "cell/dma.h"
#include "edge/anim/edgeanim_spu.h"
#include "spu_printf.h"

//----------------------------------------------------------------------------

void bpe_log_assert_failure(char const * const fileName, unsigned long lineNum,
                            char const * const prefix, char const * const cond, char const * const msg )
{
   spu_printf(msg);
}

//----------------------------------------------------------------------------------------------

bool bpe_query_to_halt_program(char const * const fileName, unsigned long const lineNum,
                               char const * const prefix, char const * const cond, char const * const msg)
{
   spu_printf(msg);
   return true;
}

//----------------------------------------------------------------------------

void cellSpursJobMain2(CellSpursJobContext2* jobContext, CellSpursJob256 *jobAnim)
{
   // Input buffers
   struct __attribute__(aligned((16))) 
   {        
      const EdgeAnimBlendBranch* blendTreeBranches;
      const EdgeAnimBlendLeaf* blendTreeLeaves;
   } spursBuffers;
   
   BPE_ASSERT(sizeof(spursBuffers) >= jobContext->numIoBuffer * sizeof(void*), "Input buffer mismatch");
   cellSpursJobGetPointerList((void**)(void*)&spursBuffers, &jobAnim->header, jobContext);

   // Cached read-only buffers (expected to be shared across multiple characters)
   BPE_ASSERT(2 >= jobContext->numCacheBuffer, "wrong number of cache buffers");
   const EdgeAnimPpuContext* ppuContext = (const EdgeAnimPpuContext*) jobContext->cacheBuffer[0];
   const EdgeAnimSkeleton* skeleton = (const EdgeAnimSkeleton* ) jobContext->cacheBuffer[1];

   // Output buffer for matrices
   Vectormath::Aos::Transform3* outputMatrices = (Vectormath::Aos::Transform3*) jobContext->oBuffer;

   // Retrieve parameters, stored after the DMA lists element
   // warning: the following formula is only right because we have on DMA list element per buffer.
   const int numDmaListElements = jobContext->numIoBuffer + jobContext->numCacheBuffer;
   const uint32_t* params = (const uint32_t*)(void*)(&jobAnim->workArea.userData[numDmaListElements]);

   // Root joint
   const EdgeAnimJointTransform* rootJoint = reinterpret_cast<const EdgeAnimJointTransform*>(params);
   params += sizeof(EdgeAnimJointTransform) / sizeof(*params);

   // Other parameters
   uint32_t numBlendBranchesAndLeaves = *params++;
   uint32_t numBlendBranches = numBlendBranchesAndLeaves >> 16;
   uint32_t numBlendLeaves = numBlendBranchesAndLeaves & 0xFFFF;

   uint32_t eaOutputMatrices = *params++; 

   uint32_t rootNodeIdAndRootNodeAdjustment = *params++;
   uint32_t rootNodeId = rootNodeIdAndRootNodeAdjustment >> 16;
   NAnimationSystem::ERootMotionExtraction rootMotionExtraction = (NAnimationSystem::ERootMotionExtraction)(rootNodeIdAndRootNodeAdjustment & 0xFFFF);

   uint32_t eaRootDelta = *params++;
   uint32_t eaIsDone = *params++;

   static float EDGE_ALIGNED(16) rootDelta[12];

   AnimationSystem_ProcessBlendTree(outputMatrices, rootMotionExtraction, &rootDelta, ppuContext, cellSpursGetCurrentSpuId(), skeleton, spursBuffers.blendTreeBranches, numBlendBranches, spursBuffers.blendTreeLeaves, numBlendLeaves, rootNodeId, rootJoint, jobContext->sBuffer, jobAnim->header.sizeScratch << 4U);

   // Dma output buffer
   cellDmaLargePut(outputMatrices, eaOutputMatrices, skeleton->numJoints * 12 * sizeof(float), jobContext->dmaTag, 0, 0);
   
   // Dma root delta
   if( eaRootDelta )
   {
      cellDmaPut(rootDelta, eaRootDelta, 12 * sizeof(float), jobContext->dmaTag, 0, 0);
   }

   uint32_t isDoneValue = 1;

   cellDmaSmallPutf(&isDoneValue, eaIsDone, 4, jobContext->dmaTag, 0, 0);
}

