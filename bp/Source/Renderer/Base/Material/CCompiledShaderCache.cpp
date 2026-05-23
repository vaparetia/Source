//----------------------------------------------------------------------------
// CCompiledShaderCache.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CCompiledShaderCache.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderViewport.h"

//----------------------------------------------------------------------------

CBaseCompiledShaderPass::CBaseCompiledShaderPass()
:  mDrawType(kPDT_Mesh)
,  mRenderTarget(kPRT_Invalid)
,  mpCustomTarget(NULL)
{
}

//----------------------------------------------------------------------------

CBaseCompiledShaderPass::~CBaseCompiledShaderPass()
{
}

//----------------------------------------------------------------------------

void CBaseCompiledShaderPass::SetPassScript(std::string const & passScript)
{
   std::vector<std::string> tokens;

   CStringExtras::Tokenize(passScript, tokens, ";");

   for( int i = 0; i < tokens.size(); ++i )
   {
      std::vector<std::string> varValue;
      CStringExtras::Tokenize(tokens[i], varValue, "=");

      if( varValue.size() == 2 )
      {
         if( CStringExtras::CompareCaseInsensitive(varValue[0], "RenderColorTarget") )
         {
            if( CStringExtras::CompareCaseInsensitive(varValue[1], "g_FrameBufferTexture"))
            {
               mRenderTarget = kPRT_FrameBuffer;
            }
            else if( CStringExtras::CompareCaseInsensitive(varValue[1], "g_BackBuffer") )
            {
               mRenderTarget = kPRT_BackBuffer;
            }
            else
            {
               mRenderTarget = kPRT_Custom;
               mCustomTarget = varValue[1];
            }
         }
         else if( CStringExtras::CompareCaseInsensitive(varValue[0], "RenderTargetMask") )
         {
            std::vector<std::string> values;
            CStringExtras::Tokenize(varValue[1], values, ",");
            if( values.size() == 4 )
            {
               mRenderTarget = kPRT_FrameBufferMasked;

               for( int i = 0; i < 4; ++i )
               {
                  mRenderTargetMask[i] = CStringExtras::CompareCaseInsensitive(values[i], "true");
               }
            }

         }
         else if( CStringExtras::CompareCaseInsensitive(varValue[0], "Draw") )
         {
            if( CStringExtras::CompareCaseInsensitive(varValue[1], "FullScreenQuad"))
            {
               mDrawType = kPDT_FullscreenQuad;
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

void CBaseCompiledShaderPass::EvaluatePassScript()
{
   switch(mRenderTarget)
   {
   case kPRT_BackBuffer:
      RenderBackend()->SetRenderTarget(SRenderTarget());
      break;

   case kPRT_FrameBuffer:
      RenderBackend()->SetRenderTarget(Renderer()->GetCurrentViewport()->GetRenderTarget());
      break;

   case kPRT_FrameBufferMasked:
      {
         SRenderTarget maskedRenderTarget = Renderer()->GetCurrentViewport()->GetRenderTarget();

         for( int i = 0; i < 4; ++i )
         {
            if( !mRenderTargetMask[i] )
            {
               maskedRenderTarget.mpColorBuffer[i] = NULL;
            }
         }
         RenderBackend()->SetRenderTarget(maskedRenderTarget);
      }
      break;

   case kPRT_Custom:
      {
         if( mpCustomTarget == NULL )
            mpCustomTarget = RenderBackend()->ShaderCache()->GetRenderTarget(mCustomTarget.c_str());

         RenderBackend()->SetRenderTarget(SRenderTarget(mpCustomTarget,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL));
      }
      break;
   }
}

//----------------------------------------------------------------------------

void CBaseCompiledShaderPass::ResetPassScript()
{
   if( mRenderTarget == kPRT_FrameBufferMasked )
   {
      RenderBackend()->SetRenderTarget(Renderer()->GetCurrentViewport()->GetRenderTarget());
   }
}

//----------------------------------------------------------------------------

CBaseCompiledShaderCache::CBaseCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

CBaseCompiledShaderCache::~CBaseCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseCompiledShaderCache::GetRenderTarget(const char * renderTargetTextureName) const
{
   TNameTextureMap::const_iterator found = mNameTextureMap.find(std::string(renderTargetTextureName));
   if( found != mNameTextureMap.end())
   {
      return found->second.get();
   }
   else
   {
      return NULL;
   }
}

//----------------------------------------------------------------------------

void CBaseCompiledShaderCache::ClearRenderTargets()
{
   SRenderTarget const oldRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   for( TNameTextureMap::const_iterator it = mNameTextureMap.begin(); it != mNameTextureMap.end(); ++it )
   {
      CBaseTexture* pTexture = it->second.get();
      gpRenderBackend->SetRenderTarget(SRenderTarget(pTexture, NULL, NULL, NULL, NULL));
      gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor::Black());
   }

   gpRenderBackend->SetRenderTarget(oldRenderTarget);
}
