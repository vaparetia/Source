//----------------------------------------------------------------------------
// NRenderQueue.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "NRenderQueue.h"

//----------------------------------------------------------------------------

#include "cell/dma.h"
#include "cell/spurs/task.h"
#include "ShaderSetup.h"

//----------------------------------------------------------------------------

using namespace NRenderQueue;

EProcessResult NRenderQueue::ProcessCommand(SRenderCmd const * pCommand, uint8* pScratchMemory)
{
   switch(pCommand->mCmd)
   {
   case SRenderCmd::kCmd_Terminate:
      return kPR_Terminate;

   case SRenderCmd::kCmd_SetShader:
      {
         //spu_printf("SetShaderExecute(%d): c: %8.8x s: %8.8x u: %8.8x\n", pCommand->mSetShaderParams.mWorkloadId, pCommand->mSetShaderParams.mMaterialConstantParametersEA, pCommand->mSetShaderParams.mRuntimeSharedParametersEA, pCommand->mSetShaderParams.mRuntimeUnsharedParametersEA);

         int const kDmaTag = 11;

         // DMA shader fragment data
         uint8* shaderFragmentDataLS = pScratchMemory;
         {
            int const alignedShaderSize = BPE_PAD_16(pCommand->mSetShaderParams.mShaderFragmentDataSize);
            if( alignedShaderSize > 0 )
               cellDmaGet(shaderFragmentDataLS, pCommand->mSetShaderParams.mShaderFragmentDataEA, alignedShaderSize, kDmaTag, 0, 0);
            pScratchMemory += alignedShaderSize;
         }

         // DMA shader vertex data
         uint8* shaderVertexDataLS = pScratchMemory;
         {
            int const alignedShaderSize = BPE_PAD_16(pCommand->mSetShaderParams.mShaderVertexDataSize);
            if( alignedShaderSize > 0 )
               cellDmaGet(shaderVertexDataLS, pCommand->mSetShaderParams.mShaderVertexDataEA, alignedShaderSize, kDmaTag, 0, 0);
            pScratchMemory += alignedShaderSize;
         }

         // DMA material constant parameters
         uint8* materialConstantParametersLS = pScratchMemory;
         int const alignedMaterialConstantParametersSize = BPE_PAD_16(pCommand->mSetShaderParams.mMaterialConstantParametersSize);
         if( alignedMaterialConstantParametersSize > 0 )
            cellDmaGet(materialConstantParametersLS, pCommand->mSetShaderParams.mMaterialConstantParametersEA, alignedMaterialConstantParametersSize, kDmaTag, 0, 0);
         pScratchMemory += alignedMaterialConstantParametersSize;

         // DMA shared parameters
         uint8* runtimeSharedParametersLS = pScratchMemory;
         int const alignedRuntimeSharedParametersSize = BPE_PAD_16(pCommand->mSetShaderParams.mRuntimeSharedParametersSize);
         if( alignedRuntimeSharedParametersSize > 0 )
            cellDmaGet(runtimeSharedParametersLS, pCommand->mSetShaderParams.mRuntimeSharedParametersEA, alignedRuntimeSharedParametersSize, kDmaTag, 0, 0);
         pScratchMemory += alignedRuntimeSharedParametersSize;

         // DMA unshared parameters
         uint8* runtimeUnsharedParametersLS = pScratchMemory;
         int const alignedRuntimeUnsharedParametersSize = BPE_PAD_16(pCommand->mSetShaderParams.mRuntimeUnsharedParametersSize);
         if( alignedRuntimeUnsharedParametersSize > 0 )
            cellDmaGet(runtimeUnsharedParametersLS, pCommand->mSetShaderParams.mRuntimeUnsharedParametersEA, alignedRuntimeUnsharedParametersSize, kDmaTag, 0, 0);
         pScratchMemory += alignedRuntimeUnsharedParametersSize;

         // Wait for DMAs to finish
         cellDmaWaitTagStatusAll(1 << kDmaTag);

         SetShader(shaderFragmentDataLS, 
                   shaderVertexDataLS,
                   pCommand->mSetShaderParams.mCommandHoleSize,
                   materialConstantParametersLS, pCommand->mSetShaderParams.mMaterialConstantParametersSize,
                   runtimeSharedParametersLS, pCommand->mSetShaderParams.mRuntimeSharedParametersSize,
                   runtimeUnsharedParametersLS, pCommand->mSetShaderParams.mRuntimeUnsharedParametersSize,
                   pScratchMemory,
                   pCommand->mSetShaderParams.mOutFragmentProgramEA, pCommand->mSetShaderParams.mOutFragmentProgramOffset,
                   pCommand->mSetShaderParams.mOutCommandBufferHoleEA,
                   pCommand->mSetShaderParams.mJumpToNextOffset);
      }
      return kPR_Ok;

   case SRenderCmd::kCmd_FreeParameterBufferRegion:
      {
         int const kDmaTag = 12;

         uint32 value = pCommand->mFreeParameterBufferRegionParams.mFreePtrValue;
         cellDmaSmallPutf(&value, pCommand->mFreeParameterBufferRegionParams.mFreePtrEA, 4, kDmaTag, 0, 0);
         cellDmaWaitTagStatusAll(1 << kDmaTag);
      }
      return kPR_Ok;

   default:
      spu_printf("ProcessCommand: %d\n", pCommand->mCmd);
      return kPR_Ok;
   }
}

