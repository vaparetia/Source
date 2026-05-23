//----------------------------------------------------------------------------
// Win32CCompiledShaderCache.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Win32CCompiledShaderCache.h"

//----------------------------------------------------------------------------

#include "Engine/Mechanics/CCRC.h"               
#include "Engine/Mechanics/TTokenSet.h"
#include "Engine/Streams/CDiskInputStream.h"
#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/System/CDirList.h"
#include "Engine/System/CFileUtils.h"

#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

#include "Renderer/Base/Material/Win32/CEffectStateManager.h"
#include "Renderer/Base/Backend/Win32/Win32CRenderBackend.h"
#include "Renderer/Base/Backend/Win32/Win32CTexture.h"

#include "d3dx9core.h"

//----------------------------------------------------------------------------

//#define VERBOSE

//----------------------------------------------------------------------------
namespace
{
   int const kMergedParameterBufferSize = 16 * 1024;
   uint8 sMergedParameterBuffer[kMergedParameterBufferSize];

   int const kParameterMergeBufferSize = 1024;
   uint32* sParameterMergeBuffer[kParameterMergeBufferSize];
}

//----------------------------------------------------------------------------

CCompiledShader::CCompiledShader(TComPtr<ID3DXEffect> const & effect, D3DXHANDLE technique)
:  mEffect(effect)
,  mTechnique(technique)
,  mCurrentPass(-1)
{
   D3DXTECHNIQUE_DESC desc;
   mEffect->GetTechniqueDesc(technique, &desc);

   mPasses.resize(desc.Passes, CCompiledShaderPass());

   for( int i = 0; i < mPasses.size(); ++i )
   {
      D3DXHANDLE pass = mEffect->GetPass(mTechnique, i);
      D3DXHANDLE scriptAnnotation = mEffect->GetAnnotationByName(pass, "Script");
      if( scriptAnnotation != NULL )
      {
         LPCSTR scriptString = NULL;
         if( SUCCEEDED(mEffect->GetString(scriptAnnotation, &scriptString)) )
         {
            mPasses[i].SetPassScript(scriptString);
         }
      }
   }

   BuildParameterLookup();
}

//----------------------------------------------------------------------------

int const CCompiledShader::Begin()
{
   UINT passes = 0;
   mEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE);
   return passes;
}

//----------------------------------------------------------------------------

void CCompiledShader::End()
{
   mEffect->End();
}

//----------------------------------------------------------------------------

int const CCompiledShader::BeginPass(int const currentPass)
{
   BPE_VERIFY(mCurrentPass == -1, false, "Must end previous pass before beginning a new one");
   mCurrentPass = currentPass;

   mEffect->BeginPass(mCurrentPass);

   mPasses[mCurrentPass].EvaluatePassScript();
   return mPasses[mCurrentPass].mDrawType;
}

//----------------------------------------------------------------------------

void CCompiledShader::EndPass()
{
   BPE_VERIFY(mCurrentPass != -1, false, "Must begin pass before ending it");

   mPasses[mCurrentPass].ResetPassScript();

   mCurrentPass = -1;

   mEffect->EndPass();
}

//----------------------------------------------------------------------------

void BuildParameterList(uint8* parameters, int const size, uint32** pParameters, int * pParameterCount)
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

struct SParameterSortPred
{
   bool operator() (uint32* pLhs, uint32* pRhs) const
   {
      return *pLhs < *pRhs;
   }
};

void CCompiledShader::BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
                                       uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
                                       uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize)
{
   uint32** pParameters = sParameterMergeBuffer;
   int parameterCount = 0;

   // Build list of unique parameters, if a parameter is already in the list, it will not be added again.
   BuildParameterList(pMaterialRuntimeParametersUnshared, materialRuntimeParametersUnsharedSize, pParameters, &parameterCount);
   BuildParameterList(pMaterialRuntimeParametersShared, materialRuntimeParametersSharedSize, pParameters, &parameterCount);
   BuildParameterList(pMaterialConstantParameters, materialConstantParametersSize, pParameters, &parameterCount);

   uint32** pParametersEnd = pParameters + parameterCount;

   // Sort list of parameters by their CRC.
   std::sort(pParameters, pParameters + parameterCount, SParameterSortPred());

   // Set parameters
   {
      uint32** pCurrentParameter = pParameters;

      for( int i = 0; i < mParameters.size(); ++i )
      {
         SParameter const parameter = mParameters[i];
         uint32 const parameterCRC = parameter.mCRC;

         while( pCurrentParameter < pParametersEnd )
         {
            uint32 const dataCRC = *(*(pCurrentParameter));
            
            if( dataCRC == parameterCRC )
            {
               uint32* pParameter = *pCurrentParameter;

               uint32 const foundSize = *(pParameter + 1);
               void* pParameterData = (pParameter + 2);
               mEffect->SetValue((D3DXHANDLE)parameter.mHandle, pParameterData, foundSize);

               ++pCurrentParameter;
               break;
            }
            else if( dataCRC < parameterCRC )
            {
               pCurrentParameter++;
            }
            else // (dataCRC > parameterCRC)
            {
               break;
            }
         }
      }
   }

   // Set texture parameters
   {
      uint32** pCurrentParameter = pParameters;

      for( int i = 0; i < mTextureParameters.size(); ++i )
      {
         SParameter const parameter = mTextureParameters[i];
         uint32 const parameterCRC = parameter.mCRC;

         while( pCurrentParameter < pParametersEnd )
         {
            uint32 const dataCRC = *(*(pCurrentParameter));

            if( dataCRC == parameterCRC )
            {
               uint32* pParameter = *pCurrentParameter;

               // ignore size parameter for now (which is at + 1)

               CTexture* pTexture = (CTexture*)*(pParameter + 2);
               mEffect->SetTexture((D3DXHANDLE)parameter.mHandle, pTexture->GetTexture().GetPtr());

               ++pCurrentParameter;
               break;
            }
            else if( dataCRC < parameterCRC )
            {
               pCurrentParameter++;
            }
            else // (dataCRC > parameterCRC)
            {
               break;
            }
         }
      }
   }
   
   mEffect->CommitChanges();

}

//----------------------------------------------------------------------------

void CCompiledShader::BuildParameterLookup()
{
   for( int i = 0; ; ++i )
   {
      D3DXHANDLE parameter = mEffect->GetParameter(NULL, i);
      if( parameter == NULL )
         break;

      D3DXPARAMETER_DESC desc;
      mEffect->GetParameterDesc(parameter, &desc);

      if( desc.Elements == 0 )
      {
         HandleParameter(parameter, desc.Type, desc.Name, desc.Semantic, false);
      }
      else
      {
         if( mEffect->IsParameterUsed(parameter, mTechnique) )
         {
            for( int elementIdx = 0; elementIdx < desc.Elements; ++elementIdx )
            {
               D3DXHANDLE elementParameter = mEffect->GetParameterElement(parameter, elementIdx);
               std::string const elementParameterName = CStringExtras::Stringize("%s[%d]", desc.Name, elementIdx);
               HandleParameter(elementParameter, desc.Type, elementParameterName.c_str(), desc.Semantic, true);
            }
         }
      }
   }

   std::sort(mParameters.begin(), mParameters.end());
   std::sort(mTextureParameters.begin(), mTextureParameters.end());
}

//----------------------------------------------------------------------------

void CCompiledShader::HandleParameter(D3DXHANDLE parameter, int /*D3DXPARAMETER_TYPE*/ const type, char const * pName, char const * pSemantic, bool const isArrayElement)
{
   // if this parameter is a 2d texture and it's semantic says "RenderColorTarget" then we deal with it in a special way.
   if( (type == D3DXPT_TEXTURE || type == D3DXPT_TEXTURE2D) && 
      pSemantic && 
      CStringExtras::CompareCaseInsensitive(pSemantic, "RenderColorTarget") )
   {
      // if the render target has a viewport ratio indication we need to create a render target
      D3DXHANDLE viewportRatioParam = mEffect->GetAnnotationByName(parameter, "ViewportRatio");
      if( viewportRatioParam != NULL )
      {          
         CVector2 viewportRatio = CVector2(1.0f, 1.0f);
         mEffect->GetValue(viewportRatioParam, &viewportRatio, sizeof(real32) * 2);

         int renderTargetWidth = static_cast<int>(viewportRatio[kAX] * RenderBackend()->GetViewWidth());
         int renderTargetHeight = static_cast<int>(viewportRatio[kAY] * RenderBackend()->GetViewHeight());

         CTexture::EFormat const defaultFormat = CTexture::kFormat_A8R8G8B8;

         TNameTextureMap & nameTextureMap = RenderBackend()->ShaderCache()->mNameTextureMap;
         TNameTextureMap::iterator found = nameTextureMap.find(pName);
         if( found == nameTextureMap.end() )
         {
            boost::shared_ptr<CTexture> pTexture(static_cast<CTexture*>(RenderBackend()->CreateRenderTarget(renderTargetWidth, renderTargetHeight, CBaseTexture::kAA_None, defaultFormat)));
            found = RenderBackend()->ShaderCache()->mNameTextureMap.insert(std::make_pair(pName, pTexture)).first;
            bpe_debugger_printf("Creating new render target for texture '%s' (w:%d h:%d)\n", pName, renderTargetWidth, renderTargetHeight);
         }

         CTexture & texture = static_cast<CTexture &>(*found->second);
         mEffect->SetTexture(parameter, texture.GetTexture().GetPtr());
      }

      uint32 const parameterCRC = CShaderCRCs::GetParameterCRC(pName);

#ifdef VERBOSE
      bpe_debugger_printf("Sampler: %s (%8.8x)\n", pName, parameterCRC);
#endif
      mTextureParameters.push_back(SParameter(parameterCRC, (uint32)parameter));
   }
   else
   {
      if( isArrayElement || mEffect->IsParameterUsed(parameter, mTechnique) )
      {
         switch(type)
         {
         case D3DXPT_TEXTURE:
         case D3DXPT_TEXTURE2D:
         case D3DXPT_TEXTURE3D:
         case D3DXPT_TEXTURECUBE:
            {
               uint32 const parameterCRC = CShaderCRCs::GetParameterCRC(pName);
#ifdef VERBOSE
               bpe_debugger_printf("Sampler: %s (%8.8x)\n", pName, parameterCRC);
#endif
               mTextureParameters.push_back(SParameter(parameterCRC, (uint32)parameter));

            }
            break;

         case D3DXPT_SAMPLER:
         case D3DXPT_SAMPLER1D:
         case D3DXPT_SAMPLER2D:
         case D3DXPT_SAMPLER3D:
         case D3DXPT_SAMPLERCUBE:
            break;

         case D3DXPT_FLOAT:
            {
               uint32 const parameterCRC = CShaderCRCs::GetParameterCRC(pName);
#ifdef VERBOSE
               bpe_debugger_printf("Parameter: %s (%8.8x)\n", pName, parameterCRC);
#endif
               mParameters.push_back(SParameter(parameterCRC, (uint32)parameter));
            }
            break;

         default:
            BPE_VERIFYA(false, "Invalid parameter type");
            break;
         }
      }
   }
}

//----------------------------------------------------------------------------

CCompiledShaderCache::CCompiledShaderCache()
:  mpCurrentShader(NULL)
,  mEffectStateManager(new CEffectStateManager())
{
   FlushAllShaders();
}

//----------------------------------------------------------------------------

CCompiledShaderCache::~CCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::FlushAllShaders()
{
   mpCurrentShader = NULL;
   mShaders.clear();
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::SetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   mpCurrentShader = GetShader(shaderFileId, pDefines);
}

//----------------------------------------------------------------------------

CCompiledShader *  CCompiledShaderCache::GetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   uint32 const definesCRC = CCRC::CalculateCRC32AsString(pDefines);
   return GetShader(shaderFileId, definesCRC, pDefines);
}

//----------------------------------------------------------------------------

CCompiledShader * CCompiledShaderCache::GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines)
{
   CCompiledShader *pCurrentShader = NULL;
   
   TShaderMap::iterator foundFileIt = mShaders.find(shaderFileId.mHash);

   if( foundFileIt != mShaders.end() )
   {
      TShaderCombinationMap::iterator foundCombinationIt = foundFileIt->second.find(definesCRC);
      if (foundCombinationIt != foundFileIt->second.end())
      {
         // Found shader, return immediately.
         pCurrentShader = &foundCombinationIt->second;
         return pCurrentShader;
      }
   }
   else
   {
      // shaderFileId not found, add empty file entry to prevent special case below
      std::pair<TShaderMap::iterator, bool> inserted = mShaders.insert(TShaderMap::value_type(shaderFileId.mHash, TShaderCombinationMap()));
      foundFileIt = inserted.first;
   }

   // Shader not found, compile!
   {
      std::string path = CResourceManager::GetLocalPath(shaderFileId.mResource);

      std::vector<D3DXMACRO> defineMacros;
      std::vector<std::string> defineStrings;
      
      CStringExtras::Tokenize(pDefines, defineStrings, "=;");
      for( int i = 0; i < (defineStrings.size() / 2); ++i )
      {
         D3DXMACRO macro;

         macro.Name = defineStrings[i*2 + 0].c_str();
         macro.Definition = defineStrings[i*2 + 1].c_str();
         defineMacros.push_back(macro);
      }

      D3DXMACRO const kPlatformMacro = { "BPE_PLATFORM", "0" };
      defineMacros.push_back( kPlatformMacro );

      D3DXMACRO const kNullTerminator = { NULL, NULL };
      defineMacros.push_back(kNullTerminator);

      LPD3DXEFFECT effect = NULL;
      LPD3DXBUFFER errorMsgs = NULL;
      bool shaderTechniqueWasBound = false;

      D3DXCreateEffectFromFile(D3DDeviceUncached(), path.c_str(), &defineMacros[0], NULL, D3DXSHADER_PACKMATRIX_ROWMAJOR, NULL, &effect, &errorMsgs);

      if( effect != NULL )
      {
         effect->SetStateManager(mEffectStateManager.GetPtr());
         D3DXHANDLE technique = NULL;
         effect->FindNextValidTechnique(NULL, &technique);

         if( technique != NULL )
         {
            effect->SetTechnique(technique);

            // Insert combination into file mapping
            foundFileIt->second.insert(TShaderCombinationMap::value_type(definesCRC, CCompiledShader(TComPtr<ID3DXEffect>(effect), technique)));
            shaderTechniqueWasBound = true;
         }
         else
         {
            effect->Release();
         }
      }

      if( errorMsgs )
      {
         bpe_console_printf("%s\n", errorMsgs->GetBufferPointer());
         bpe_debugger_printf("%s\n", errorMsgs->GetBufferPointer());
         errorMsgs->Release();
      }
      else if ( !shaderTechniqueWasBound )
      {
         // Sometimes it appears the shader compiler can get into a state where 
         // it doesn't feel like compiling the shader for an indeterminate amount of time
         // after it fails for awhile.

         bpe_debugger_and_console_printf( "%s: Unknown compile error!\n", path.c_str() );
      }
      else
      {
         // No error messages, rebind shader (call function to prevent code duplication)
         pCurrentShader = GetShader(shaderFileId, pDefines);
      }
   }
   return pCurrentShader;
}

//----------------------------------------------------------------------------
