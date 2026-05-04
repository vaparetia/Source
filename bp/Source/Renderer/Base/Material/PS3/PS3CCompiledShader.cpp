//----------------------------------------------------------------------------
// PS3CCompiledShader.cpp
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PS3CCompiledShader.h"
#include "PS3CCompiledShaderTypes.h"

//----------------------------------------------------------------------------

#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"
#include "Renderer/Base/Backend/PS3/PS3CTexture.h"
#include "Renderer/Renderer_SPU/NRenderQueue.h"

//----------------------------------------------------------------------------

//#include "ppu_intrinsics.h"

//----------------------------------------------------------------------------

namespace
{
   void SetState(SShaderPassHeader const * pPassHeader, SShaderStateData const * pData)
   {
      uint8* pBaseAddress = (uint8*)pData;

      int const numWords = pData->mSetStateCommandBufferSize / 4;
      if( numWords > 0 )
      {
         cell::Gcm::cellGcmReserveMethodSize(numWords);
         void* pStateCommandBuffer = pBaseAddress + sizeof(SShaderStateData);
         memcpy(gCellGcmCurrentContext->current, pStateCommandBuffer, pData->mSetStateCommandBufferSize);
         gCellGcmCurrentContext->current += numWords;
      }

      if( RenderBackend()->GetCurrentRenderTarget().mpDepthBuffer == NULL || !pPassHeader->mPassData.GetFlag(SShaderPassData::kDepthTest) )
      {
         cell::Gcm::cellGcmSetDepthTestEnable(CELL_GCM_FALSE);
      }

      CCompiledShaderCache* pShaderCache = RenderBackend()->ShaderCache();

      // set current color mask
      pShaderCache->mColorMask[0] = pPassHeader->mPassData.GetFlag(SShaderPassData::kColorMaskR) ? true : false;
      pShaderCache->mColorMask[1] = pPassHeader->mPassData.GetFlag(SShaderPassData::kColorMaskG) ? true : false;
      pShaderCache->mColorMask[2] = pPassHeader->mPassData.GetFlag(SShaderPassData::kColorMaskB) ? true : false;
      pShaderCache->mColorMask[3] = pPassHeader->mPassData.GetFlag(SShaderPassData::kColorMaskA) ? true : false;
      
      // set alpha to coverage
      pShaderCache->mAlphaToCoverage = pPassHeader->mPassData.GetFlag(SShaderPassData::kAlphaToCoverage) ? true : false;
      pShaderCache->mForceDisableAA = pPassHeader->mPassData.GetFlag(SShaderPassData::kForceDisableAA) ? true : false;

      if ( pPassHeader->mPassData.GetFlag(SShaderPassData::kWaitForRenderTargetResult) )
      {
         RenderBackend()->GcmIssueWaitForBackEndFinish();
         cell::Gcm::cellGcmSetInvalidateTextureCache( CELL_GCM_INVALIDATE_TEXTURE );
         cell::Gcm::cellGcmSetInvalidateTextureCache( CELL_GCM_INVALIDATE_VERTEX_TEXTURE );
      }
   }

   void ResetState(SShaderPassHeader const * pPassHeader, SShaderStateData const * pData)
   {
      uint8* pBaseAddress = (uint8*)pData;

      int const numWords = pData->mResetStateCommandBufferSize / 4;
      if( numWords > 0 )
      {
         cell::Gcm::cellGcmReserveMethodSize(numWords);
         void* pStateCommandBuffer = pBaseAddress + sizeof(SShaderStateData) + pData->mSetStateCommandBufferSize;
         memcpy(gCellGcmCurrentContext->current, pStateCommandBuffer, pData->mResetStateCommandBufferSize);
         gCellGcmCurrentContext->current += numWords;
      }

      //cell::Gcm::cellGcmSetDepthTestEnable(CELL_GCM_TRUE);

      CCompiledShaderCache* pShaderCache = RenderBackend()->ShaderCache();

      // reset current color mask
      pShaderCache->mColorMask[0] = true;
      pShaderCache->mColorMask[1] = true;
      pShaderCache->mColorMask[2] = true;
      pShaderCache->mColorMask[3] = true;

      // reset alpha to coverage flags
      pShaderCache->mAlphaToCoverage = false;

      // reset force disable AA
      pShaderCache->mForceDisableAA = false;
   }
}

//----------------------------------------------------------------------------

int const CCompiledShader::Begin() const
{
   return mPassCount;
}

//----------------------------------------------------------------------------

void CCompiledShader::End() const
{
}

//----------------------------------------------------------------------------

static int sCurrentPass = -1;

int const CCompiledShader::BeginPass(int const currentPass) const
{
   BPE_VERIFY(sCurrentPass == -1, false, "Must end previous pass before beginning a new one");
   sCurrentPass = currentPass;

   SShaderPassHeader const * pPassHeader = GetPassHeader(sCurrentPass);
   SShaderStateData const * pStateData = (SShaderStateData const *)pPassHeader->mStateDataOffset;
   SetState(pPassHeader, pStateData);

   return kPDT_Mesh;
}

//----------------------------------------------------------------------------

void CCompiledShader::EndPass() const
{
   BPE_VERIFY(sCurrentPass != -1, false, "Must begin pass before ending it");

   SShaderPassHeader const * pPassHeader = GetPassHeader(sCurrentPass);
   SShaderStateData const * pStateData = (SShaderStateData const *)pPassHeader->mStateDataOffset;
   ResetState(pPassHeader, pStateData);

   sCurrentPass = -1;
}

//----------------------------------------------------------------------------

void CCompiledShader::BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
                                       uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
                                       uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize) const
{
   SShaderPassHeader const * pPassHeader = GetPassHeader(sCurrentPass);
   SShaderVertexData const * pShaderVertexData = (SShaderVertexData const *)pPassHeader->mVertexDataOffset;
   SShaderFragmentData const * pShaderFragmentData = (SShaderFragmentData const *)pPassHeader->mFragmentDataOffset;

   // Create command buffer hole
   uint32 commandBufferHoleEA = 0;
   uint32_t jumpToNextOffset = 0;
   {
      // Reserve a bit extra to ensure we can align our start ptr to 16 bytes
      int const wordsToReserve = pPassHeader->mCommandBufferHoleSize >> 2;
      cellGcmReserveMethodSize(gCellGcmCurrentContext, wordsToReserve + 3);

      // Align with NOPS to be 16 byte aligned
      while(((uint32_t)gCellGcmCurrentContext->current & 0xF) != 0)
         cellGcmSetNopCommandUnsafeInline(gCellGcmCurrentContext, 1);

      commandBufferHoleEA = (uint32)gCellGcmCurrentContext->current;
      uint32_t const holeEnd = commandBufferHoleEA + pPassHeader->mCommandBufferHoleSize;
      
      // Add initial jump
      uint32_t jumpOffset;
      cellGcmAddressToOffset(gCellGcmCurrentContext->current, &jumpOffset);
      cellGcmSetJumpCommandUnsafeInline(gCellGcmCurrentContext, jumpOffset);

      cellGcmAddressToOffset(gCellGcmCurrentContext->current, &jumpToNextOffset);

      gCellGcmCurrentContext->current = (uint32_t*)holeEnd;
   }

   NRenderQueue::SRenderCmd cmd;

   cmd.mCmd = NRenderQueue::SRenderCmd::kCmd_SetShader;
   cmd.mSetShaderParams.mShaderFragmentDataEA = (uint32)pShaderFragmentData;
   cmd.mSetShaderParams.mShaderVertexDataEA = (uint32)pShaderVertexData;

   cmd.mSetShaderParams.mMaterialConstantParametersEA = (uint32)pMaterialConstantParameters;
   cmd.mSetShaderParams.mRuntimeSharedParametersEA = (uint32)pMaterialRuntimeParametersShared;
   cmd.mSetShaderParams.mRuntimeUnsharedParametersEA = (uint32)pMaterialRuntimeParametersUnshared;

   // Allocate memory for fragment program
   uint8* pFragmentProgramEA = NULL;
   uint32 fragmentProgramOffset = 0;

   RenderBackend()->FragmentProgramAllocator()->Allocate(pShaderFragmentData->mFragmentProgramUCodeSize, &pFragmentProgramEA, &fragmentProgramOffset);
   cmd.mSetShaderParams.mOutFragmentProgramEA = (uint32)pFragmentProgramEA;
   cmd.mSetShaderParams.mOutFragmentProgramOffset = fragmentProgramOffset;

   cmd.mSetShaderParams.mOutCommandBufferHoleEA = commandBufferHoleEA;

   cmd.mSetShaderParams.mJumpToNextOffset = jumpToNextOffset;

   cmd.mSetShaderParams.mShaderFragmentDataSize = pShaderFragmentData->mSize;
   cmd.mSetShaderParams.mShaderVertexDataSize = pShaderVertexData->mSize;
   cmd.mSetShaderParams.mCommandHoleSize = pPassHeader->mCommandBufferHoleSize;

   cmd.mSetShaderParams.mMaterialConstantParametersSize = materialConstantParametersSize;
   cmd.mSetShaderParams.mRuntimeSharedParametersSize = materialRuntimeParametersSharedSize;
   cmd.mSetShaderParams.mRuntimeUnsharedParametersSize = materialRuntimeParametersUnsharedSize;

   BPE_VERIFY((cmd.mSetShaderParams.mShaderFragmentDataEA & 15) == 0, false, "ShaderFragmentDataEA not 16 byte aligned");
   BPE_VERIFY((cmd.mSetShaderParams.mShaderVertexDataEA & 15) == 0, false, "ShaderVertexDataEA not 16 byte aligned");
   BPE_VERIFY((cmd.mSetShaderParams.mMaterialConstantParametersEA & 15) == 0, false, "MaterialConstantParametersEA not 16 byte aligned");
   BPE_VERIFY((cmd.mSetShaderParams.mRuntimeSharedParametersEA & 15) == 0, false, "RuntimeSharedParametersEA not 16 byte aligned");
   BPE_VERIFY((cmd.mSetShaderParams.mRuntimeUnsharedParametersEA & 15) == 0, false, "RuntimeSharedParametersEA not 16 byte aligned");

   gpRenderBackend->AddFragmentRenderCmd(&cmd);
}

//----------------------------------------------------------------------------

void CCompiledShader::EndShaderBatch() const
{
   RenderBackend()->FragmentProgramAllocator()->AddRSXFreeAllocatedMemoryCmd();
}

//----------------------------------------------------------------------------

CCompiledShaderPackage::CCompiledShaderPackage(void* pData)
:  mpPackage((SShaderPackageHeader*)pData)
{
   BPE_VERIFY(mpPackage->mFourCC == 'CFX!', false, "Invalid shader file format");
   BPE_VERIFY(mpPackage->mVersion == 0, false, "Invalid shader file version");

   FixupOffsets();
}

//----------------------------------------------------------------------------

CCompiledShader const * CCompiledShaderPackage::GetCombinationByHash(uint32 const hash) const
{
   for( int i = 0; i < GetCombinationCount(); ++i )
   {
      if( GetHash(i) == hash )
          return GetCombination(i);
   }

   return NULL;
}

//----------------------------------------------------------------------------

void CCompiledShaderPackage::FixupOffsets()
{
   uint32 const vertexDataStartAddress = (uint32)mpPackage + mpPackage->mVertexDataOffset;
   uint32 const fragmentDataStartAddress = (uint32)mpPackage + mpPackage->mFragmentDataOffset;
   uint32 const stateDataStartAddress = (uint32)mpPackage + mpPackage->mStateDataOffset;

   for( int combinationIdx = 0; combinationIdx < GetCombinationCount(); ++combinationIdx )
   {
      CCompiledShader const * pCombination = GetCombination(combinationIdx);
      for( int passIdx = 0; passIdx < pCombination->mPassCount; ++passIdx )
      {
         SShaderPassHeader * pPassHeader = (SShaderPassHeader *)pCombination->GetPassHeader(passIdx);
         pPassHeader->mStateDataOffset += stateDataStartAddress;
         pPassHeader->mFragmentDataOffset += fragmentDataStartAddress;
         pPassHeader->mVertexDataOffset += vertexDataStartAddress;
      }
   }
}

//----------------------------------------------------------------------------

void CCompiledShaderPackage::Factory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );

   void* pData = BPE_MALLOC_ALIGNED(128, buildData.mSize);
   memcpy(pData, buildData.mpMemory, buildData.mSize);

   returnResource.mpResource = new CCompiledShaderPackage(pData);
}
