//----------------------------------------------------------------------------
// VTACCompiledShader.cpp
// Copyright 2011
// Armature/Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Material/VTA/VTACCompiledShader.h"

//----------------------------------------------------------------------------

#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/VTA/VTACTexture.h"
#include "Renderer/Base/Backend/VTA/NVTAState.h"

//----------------------------------------------------------------------------

//#include "ppu_intrinsics.h"

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

int const CCompiledShader::Begin() const
{
   return mHeader.mPassCount;
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
#if 0
   SShaderStateData const * pStateData = (SShaderStateData const *)pPassHeader->mStateDataOffset;
   SetState(pPassHeader, pStateData);
#endif
   return kPDT_Mesh;
}

//----------------------------------------------------------------------------

void CCompiledShader::EndPass() const
{
   BPE_VERIFY(sCurrentPass != -1, false, "Must begin pass before ending it");

   SShaderPassHeader const * pPassHeader = GetPassHeader(sCurrentPass);
#if 0
   SShaderStateData const * pStateData = (SShaderStateData const *)pPassHeader->mStateDataOffset;
   ResetState(pPassHeader, pStateData);
#endif
   sCurrentPass = -1;
}

//----------------------------------------------------------------------------

static void BuildParameterList(uint8* parameters, int const size, uint32** pParameters, int * pParameterCount)
{
   uint8* parametersEnd = parameters + size;
   while( parameters < parametersEnd )
   {
      uint32* currentParameterPtr = (uint32*)parameters;
      uint32 const crc = *((uint32*)parameters);
      uint32 const size = *((uint32*)(parameters + 4));

      // skip over crc entry, size entry, and the size of the parameter
      parameters += size + 8;

      int const parameterCount = *pParameterCount;

      for( int i = 0; ; ++i )
      {
         if( i < parameterCount )
         {
            uint32 const parameterCRC = *(pParameters[i]);
            if( parameterCRC == crc )
               break;
         }
         else
         {
            pParameters[parameterCount] = currentParameterPtr;
            (*pParameterCount)++;
            break;
         }
      }

   }
}

//----------------------------------------------------------------------------

namespace
{
   struct SParameterSortPred
   {
      bool operator() (uint32* pLhs, uint32* pRhs) const
      {
         return *pLhs < *pRhs;
      }
   };
}

//----------------------------------------------------------------------------

void CCompiledShader::BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
                                       uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
                                       uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize) const
{
   SShaderPassHeader const * pPassHeader = GetPassHeader(sCurrentPass);
   SShaderVertexData const * pShaderVertexData = (SShaderVertexData const *)pPassHeader->mVertexDataOffset;
   SShaderFragmentData const * pShaderFragmentData = (SShaderFragmentData const *)pPassHeader->mFragmentDataOffset;

   static int const kParameterMergeBufferSize = 1024;
   static uint32* sParameterMergeBuffer[kParameterMergeBufferSize];

   uint32** pParameters = sParameterMergeBuffer;
   int parameterCount = 0;

   BuildParameterList(pMaterialRuntimeParametersUnshared, materialRuntimeParametersUnsharedSize, pParameters, &parameterCount);
   BuildParameterList(pMaterialRuntimeParametersShared, materialRuntimeParametersSharedSize, pParameters, &parameterCount);
   BuildParameterList(pMaterialConstantParameters, materialConstantParametersSize, pParameters, &parameterCount);

   uint32** pParametersEnd = pParameters + parameterCount;

   // Sort list of parameters by their CRC.
   std::sort(pParameters, pParameters + parameterCount, SParameterSortPred());

   SceGxmProgram *fragmentProgram = reinterpret_cast<SceGxmProgram *>( ( (char *) pShaderFragmentData ) + pShaderFragmentData->mFragmentProgramUCodeOffset );
   SceGxmProgram *vertexProgram = reinterpret_cast<SceGxmProgram *>( ( (char *) pShaderVertexData ) + pShaderVertexData->mVertexProgramUCodeOffset );

   SFragmentParameter *fragmentParameters = reinterpret_cast<SFragmentParameter *>( ( (char *) pShaderFragmentData ) + pShaderFragmentData->mFragmentParametersOffset );
   SVertexParameter *vertexParameters = reinterpret_cast<SVertexParameter *>( ( (char *) pShaderVertexData ) + pShaderVertexData->mVertexParametersOffset );
   SVertexSemantic *vertexSemantics = reinterpret_cast<SVertexSemantic *>( ( (char *) pShaderVertexData ) + pShaderVertexData->mVertexProgramSemanticsOffset );

   NVTAState::SetVertexAndFragmentProgram( 
      vertexProgram, 
      fragmentProgram,
      SceGxmShaderPatcherId( pShaderVertexData->mRuntimeShaderPatcherId ),
      SceGxmShaderPatcherId( pShaderFragmentData->mRuntimeShaderPatcherId ),
      fragmentParameters, 
      vertexParameters,
      vertexSemantics,
      pParameters,
      pParametersEnd);

#if 0

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
#endif
}

//----------------------------------------------------------------------------

void CCompiledShader::EndShaderBatch() const
{
#if 0
   RenderBackend()->FragmentProgramAllocator()->AddRSXFreeAllocatedMemoryCmd();
#endif
}

//----------------------------------------------------------------------------

CCompiledShaderPackage::CCompiledShaderPackage(void* pData)
:  mpPackage((SShaderPackageHeader*)pData)
{
   BPE_VERIFY(mpPackage->mFourCC == 'CFX!', false, "Invalid shader file format");
   BPE_VERIFY(mpPackage->mVersion == 4, false, "Invalid shader file version");

   FixupOffsets();

   // Register shaders with patcher
   for ( int combinationIdx = 0; combinationIdx < GetCombinationCount(); ++combinationIdx )
   {
      CCompiledShader * pCombination = Combination(combinationIdx);

      for ( int passIdx = 0; passIdx < pCombination->GetPassCount(); ++passIdx )
      {
         SShaderPassHeader const *pPassHeader = pCombination->GetPassHeader( passIdx );

         SShaderVertexData * pShaderVertexData = (SShaderVertexData *)pPassHeader->mVertexDataOffset;
         SShaderFragmentData * pShaderFragmentData = (SShaderFragmentData *)pPassHeader->mFragmentDataOffset;

         BPE_CTASSERT( sizeof( pShaderVertexData->mRuntimeShaderPatcherId ) == sizeof( SceGxmShaderPatcherId ) );
         BPE_CTASSERT( sizeof( pShaderFragmentData->mRuntimeShaderPatcherId ) == sizeof( SceGxmShaderPatcherId ) );

         pShaderVertexData->mRuntimeShaderPatcherId = uint32( NVTAState::RegisterShaderProgram( ( (char *) pShaderVertexData ) + pShaderVertexData->mVertexProgramUCodeOffset ) );
         pShaderFragmentData->mRuntimeShaderPatcherId = uint32( NVTAState::RegisterShaderProgram( ( (char *) pShaderFragmentData ) + pShaderFragmentData->mFragmentProgramUCodeOffset ) );
      }
   }
}

//----------------------------------------------------------------------------

CCompiledShaderPackage::~CCompiledShaderPackage()
{
   for ( int combinationIdx = 0; combinationIdx < GetCombinationCount(); ++combinationIdx )
   {
      CCompiledShader * pCombination = Combination(combinationIdx);

      for ( int passIdx = 0; passIdx < pCombination->GetPassCount(); ++passIdx )
      {
         SShaderPassHeader const *pPassHeader = pCombination->GetPassHeader( passIdx );

         SShaderVertexData const * pShaderVertexData = (SShaderVertexData const *)pPassHeader->mVertexDataOffset;
         SShaderFragmentData const * pShaderFragmentData = (SShaderFragmentData const *)pPassHeader->mFragmentDataOffset;

         NVTAState::UnregisterShaderProgram( ( (char *) pShaderVertexData ) + pShaderVertexData->mVertexProgramUCodeOffset );
         NVTAState::UnregisterShaderProgram( ( (char *) pShaderFragmentData ) + pShaderFragmentData->mFragmentProgramUCodeOffset );
      }
   }
}

//----------------------------------------------------------------------------

CCompiledShader const * CCompiledShaderPackage::GetCombinationByHash(uint32 const hash) const
{
   int combinationCount = GetCombinationCount();

   for( int i = 0; i < combinationCount; ++i )
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
#if 0
   uint32 const stateDataStartAddress = (uint32)mpPackage + mpPackage->mStateDataOffset;
#endif
   for( int combinationIdx = 0; combinationIdx < GetCombinationCount(); ++combinationIdx )
   {
      CCompiledShader * pCombination = Combination(combinationIdx);

      pCombination->FixupHeader();

      for( int passIdx = 0; passIdx < pCombination->GetPassCount(); ++passIdx )
      {
         SShaderPassHeader * pPassHeader = (SShaderPassHeader *)pCombination->GetPassHeader(passIdx);

#if 0
         pPassHeader->mStateDataOffset += stateDataStartAddress;
#endif
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

//----------------------------------------------------------------------------

void CCompiledShader::FixupHeader()
{
}

