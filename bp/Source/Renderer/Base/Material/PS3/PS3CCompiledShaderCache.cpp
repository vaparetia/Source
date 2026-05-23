//----------------------------------------------------------------------------
// PS3CCompiledShaderCache.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PS3CCompiledShaderCache.h"

//----------------------------------------------------------------------------

#include "Engine/Mechanics/CCRC.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Resource/CResourceCache.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"
#include "Renderer/Base/Material/PS3/PS3CCompiledShader.h"

//----------------------------------------------------------------------------

using namespace cell::Gcm;

//----------------------------------------------------------------------------

CCompiledShaderCache::CCompiledShaderCache()
:  mpCurrentShader(NULL)
,  mAlphaToCoverage(false)
,  mForceDisableAA( false )
{
   mColorMask[0] = mColorMask[1] = mColorMask[2] = mColorMask[3] = true;
}

//----------------------------------------------------------------------------

CCompiledShaderCache::~CCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::SetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   uint32 const definesCRC = CCRC::CalculateCRC32AsString(pDefines);
   SetShader(shaderFileId, definesCRC, pDefines);
}

//----------------------------------------------------------------------------

CCompiledShader * CCompiledShaderCache::GetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   uint32 const definesCRC = CCRC::CalculateCRC32AsString(pDefines);
   return GetShader(shaderFileId, definesCRC, pDefines);
}

//----------------------------------------------------------------------------

CCompiledShader * CCompiledShaderCache::GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines)
{
   TShaderMap::iterator foundFileIt = mShaders.find(shaderFileId.mHash);

   if( foundFileIt == mShaders.end() )
   {
      CStopWatch watch;

      TResource<CCompiledShaderPackage> shaderPackage = gpResources->GetResource( CResId(shaderFileId.mResource) );
      shaderPackage.Lock();

      if( !shaderPackage.IsNull() )
      {
         // dereferencing the resource causes it to be loaded.
         int unused = shaderPackage->GetCombinationCount();

         // Insert combination into file mapping
         foundFileIt = mShaders.insert(TShaderMap::value_type(shaderFileId.mHash, shaderPackage)).first;
      }

      bpe_debugger_printf( "time to load shader (%s %s) %.f ms\n", shaderFileId.mResource.c_str(), pDefines, watch.GetElapsedTime() * 1000.0f );
   }

   CCompiledShader * pCompiledShader = (CCompiledShader*)foundFileIt->second->GetCombinationByHash(definesCRC);

   if (pCompiledShader == NULL )
   {
      bpe_debugger_printf("ERROR: Invalid shader combination: %s (%s)\n", shaderFileId.mResource.c_str(), pDefines);
   }
   
   return pCompiledShader;
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::LoadShaderFromMemory(CShaderFileId const &shaderFileId, char const * const memBuffer, uint32 const memBufferSize)
{
   CStopWatch watch;

   TResource<CCompiledShaderPackage> shaderPackage( new CCompiledShaderPackage( (void*)memBuffer ) );
   shaderPackage.Lock();

   if( !shaderPackage.IsNull() )
   {
      // dereferencing the resource causes it to be loaded.
      int unused = shaderPackage->GetCombinationCount();

      // Insert combination into file mapping
      mShaders.insert(TShaderMap::value_type(shaderFileId.mHash, shaderPackage)).first;
   }

   gpResources->ManuallyAddToResourceMap(CResId( shaderFileId.mResource ), shaderPackage);
   if (gpResources->ResourceCache() != NULL)
      gpResources->ResourceCache()->CacheResource(shaderPackage, CResId( shaderFileId.mResource ));

   bpe_debugger_printf( "time to load memory shader %.f ms\n", watch.GetElapsedTime() * 1000.0f );
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::SetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines)
{
   mpCurrentShader = GetShader(shaderFileId, definesCRC, pDefines);
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::FreeShaders()
{
   mShaders.clear();

   mpCurrentShader = NULL;
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::FlushAllShaders()
{
   FreeShaders();
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::ValidateState()
{
#if 0
   // Set color mask, color mask mrt and aa control.
   {
      // Reserve enough space for the three commands below.
      cellGcmReserveMethodSize(6);

      // setup color mask
      {
         SRenderTarget const & renderTarget = RenderBackend()->GetCurrentRenderTarget();

         uint32 colorMask = 0;

         if( renderTarget.mpColorBuffer[0] != NULL || renderTarget.mpDepthBuffer == NULL )
            colorMask = (mColorMask[0] ? CELL_GCM_COLOR_MASK_R : 0) |
                        (mColorMask[1] ? CELL_GCM_COLOR_MASK_G : 0) |
                        (mColorMask[2] ? CELL_GCM_COLOR_MASK_B : 0) |
                        (mColorMask[3] ? CELL_GCM_COLOR_MASK_A : 0);

         // 2 words
         cell::Gcm::UnsafeInline::cellGcmSetColorMask(colorMask);

         uint32 colorMaskMRT = 0;

         if( BPE_BRANCH_HINT_FALSE(renderTarget.mpColorBuffer[1] != NULL) )
            colorMaskMRT |= (mColorMask[0] ? CELL_GCM_COLOR_MASK_MRT1_R : 0) |
                            (mColorMask[1] ? CELL_GCM_COLOR_MASK_MRT1_G : 0) |
                            (mColorMask[2] ? CELL_GCM_COLOR_MASK_MRT1_B : 0) |
                            (mColorMask[3] ? CELL_GCM_COLOR_MASK_MRT1_A : 0);

         if( BPE_BRANCH_HINT_FALSE(renderTarget.mpColorBuffer[2] != NULL) )
            colorMaskMRT |= (mColorMask[0] ? CELL_GCM_COLOR_MASK_MRT2_R : 0) |
                            (mColorMask[1] ? CELL_GCM_COLOR_MASK_MRT2_G : 0) |
                            (mColorMask[2] ? CELL_GCM_COLOR_MASK_MRT2_B : 0) |
                            (mColorMask[3] ? CELL_GCM_COLOR_MASK_MRT2_A : 0);

         if( BPE_BRANCH_HINT_FALSE(renderTarget.mpColorBuffer[3] != NULL) )
            colorMaskMRT |= (mColorMask[0] ? CELL_GCM_COLOR_MASK_MRT3_R : 0) |
                            (mColorMask[1] ? CELL_GCM_COLOR_MASK_MRT3_G : 0) |
                            (mColorMask[2] ? CELL_GCM_COLOR_MASK_MRT3_B : 0) |
                            (mColorMask[3] ? CELL_GCM_COLOR_MASK_MRT3_A : 0);

         // 2 words
         cell::Gcm::UnsafeInline::cellGcmSetColorMaskMrt(colorMaskMRT);
      }

      // setup antialias control (2 words)
      {
         bool const isAA = ( RenderBackend()->GetCurrentRenderTargetAAType() != CBaseTexture::kAA_None ) && !mForceDisableAA;
         cell::Gcm::UnsafeInline::cellGcmSetAntiAliasingControl( isAA, isAA && mAlphaToCoverage, CELL_GCM_FALSE, 0xFFFF );
      }
   }
#endif
}
