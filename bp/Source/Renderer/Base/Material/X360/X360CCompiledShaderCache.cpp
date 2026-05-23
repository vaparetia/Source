//----------------------------------------------------------------------------
// X360CCompiledShaderCache.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "X360CCompiledShaderCache.h"

//----------------------------------------------------------------------------

#include "Engine/Mechanics/CCRC.h"               
#include "Engine/Mechanics/TTokenSet.h"
#include "Engine/Streams/CDiskInputStream.h"
#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/System/CDirList.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/Resource/CResourceCache.h"

#include "Renderer/Base/Backend/X360/CRenderBackendPrivate.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

#include "Renderer/Base/Backend/X360/X360CRenderBackend.h"
#include "Renderer/Base/Backend/X360/X360CTexture.h"

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Engine/System/CStopWatch.h"

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

void CCompiledShaderPass::SetSamplerStates(std::vector<SamplerState_X360> const & aSamplerStates)
{
   for( int ii = 0; ii < aSamplerStates.size(); ++ii )
   {
      SamplerState_X360 const & state = aSamplerStates[ii];
      GetD3DDevice()->SetSamplerState(state.mRegister, (D3DSAMPLERSTATETYPE)state.mType, state.mValue);
   }
}

void CCompiledShaderPass::SetRenderStates(std::vector<RenderState_X360> const & aRenderStates)
{
   for( int ii = 0; ii < aRenderStates.size(); ++ii )
   {
      RenderState_X360 const & state = aRenderStates[ii];
      GetD3DDevice()->SetRenderState((D3DRENDERSTATETYPE)state.mType, state.mValue);
   }
}

void CCompiledShaderPass::SetParam( const SParam_X360& param, const uint32* pParameter )
{
   if( param.mContext & FXLPCONTEXT_VERTEXSHADERCONSTANTF )
   {
      const uint32 foundSize = *(pParameter + 1);
      const uint32 regSize = bpe::min_val(uint8(foundSize>>4), param.mVertexRegisterCount);
      const FLOAT* pParameterData = (FLOAT*)(pParameter + 2);
      GetD3DDevice()->SetVertexShaderConstantF(param.mVertexRegisterIndex, pParameterData, regSize);
   }
   if( param.mContext & FXLPCONTEXT_PIXELSHADERCONSTANTF )
   {
      const uint32 foundSize = *(pParameter + 1);
      const uint32 regSize = bpe::min_val(uint8(foundSize>>4), param.mPixelRegisterCount);
      const FLOAT* pParameterData = (FLOAT*)(pParameter + 2);
      GetD3DDevice()->SetPixelShaderConstantF(param.mPixelRegisterIndex, pParameterData, regSize);
   }
   if( param.mContext & FXLPCONTEXT_VERTEXSHADERCONSTANTI  )
   {
      const uint32 foundSize = *(pParameter + 1);
      const INT* pParameterData = (INT*)(pParameter + 2);
      GetD3DDevice()->SetVertexShaderConstantI(param.mVertexRegisterIndex, pParameterData, foundSize>>4);
   }
   if( param.mContext & FXLPCONTEXT_PIXELSHADERCONSTANTI )
   {
      const uint32 foundSize = *(pParameter + 1);
      const INT* pParameterData = (INT*)(pParameter + 2);
      GetD3DDevice()->SetPixelShaderConstantI(param.mPixelRegisterIndex, pParameterData, foundSize>>4);
   }
   if( param.mContext & FXLPCONTEXT_VERTEXSHADERCONSTANTB )
   {
      const uint32 foundSize = *(pParameter + 1);
      const BOOL* pParameterData = (BOOL*)(pParameter + 2);
      GetD3DDevice()->SetVertexShaderConstantB(param.mVertexRegisterIndex, pParameterData, foundSize>>2);
   }
   if( param.mContext & FXLPCONTEXT_PIXELSHADERCONSTANTB )
   {
      const uint32 foundSize = *(pParameter + 1);
      const BOOL* pParameterData = (BOOL*)(pParameter + 2);
      GetD3DDevice()->SetPixelShaderConstantB(param.mPixelRegisterIndex, pParameterData, foundSize>>2);
   }
   if( param.mContext & FXLPCONTEXT_VERTEXSHADERSAMPLER )
   {
      // ignore size parameter for now (which is at + 1)
      CTexture* pTexture = (CTexture*)*(pParameter + 2);
      GetD3DDevice()->SetTexture(param.mVertexRegisterIndex, pTexture->GetTexture());
   }
   if( param.mContext & FXLPCONTEXT_PIXELSHADERSAMPLER )
   {
      // ignore size parameter for now (which is at + 1)
      CTexture* pTexture = (CTexture*)*(pParameter + 2);
      GetD3DDevice()->SetTexture(param.mPixelRegisterIndex, pTexture->GetTexture());
   }
}

void CCompiledShaderPass::BeginShaderBatch(uint32** pParameters, uint32** pParametersEnd)
{
   {
      uint32** pCurrentParameter = pParameters;

      for( int i = 0; i < mParams->mParams.size(); ++i )
      {
         SParam_X360 const & parameter = mParams->mParams[i];
         uint32 const parameterCRC = parameter.mCRC;

         while( pCurrentParameter < pParametersEnd )
         {
            uint32 const dataCRC = *(*(pCurrentParameter));

            if( dataCRC == parameterCRC )
            {
               SetParam(parameter, *pCurrentParameter);

               ++pCurrentParameter;
               break;
            }
            else if( dataCRC < parameterCRC )
            {
               ++pCurrentParameter;
            }
            else//(dataCRC > parameterCRC)
            {
               break;
            }
         }
      }
   }
}

void CCompiledShaderPass::Init(Pass_360 const & pass, CCompiledShaderCombinationHolder* pCombinationHolder)
{
   mParams = &pCombinationHolder->mParams[pass.mParams_Index];
   mSamperStates = &pCombinationHolder->mSamperStates[pass.mSamperStates_Index];
   mRenderStates = &pCombinationHolder->mRenderStates[pass.mRenderStates_Index];
   mVertexShader = pCombinationHolder->mVertexShader[pass.mVertexShader_Index].GetPtr();
   mPixelShader = pCombinationHolder->mPixelShader[pass.mPixelShader_Index].GetPtr();
}

CCompiledShader::CCompiledShader(EffectCombination_360 const & combination)
: mCurrentPass(-1)
{
   //for now we only support the first technique
   Technique_360 const & technique = combination.mTechniques[0];
   mPasses.reserve(technique.mPasses.size());
   for( int iPass=0; iPass<technique.mPasses.size(); ++iPass )
   {
      mPasses.push_back(CCompiledShaderPass());
   }
}

void CCompiledShader::Init(EffectCombination_360 const & combination, CCompiledShaderCombinationHolder* pCombinationHolder)
{
   //for now we only support the first technique
   Technique_360 const & technique = combination.mTechniques[0];
   mPasses.reserve(technique.mPasses.size());
   for( int iPass=0; iPass<technique.mPasses.size(); ++iPass )
   {
      CCompiledShaderPass & compileShaderPass = mPasses[iPass];
      compileShaderPass.Init(technique.mPasses[iPass], pCombinationHolder);
   }
}

int const CCompiledShader::Begin()
{
   //mEffect->BeginTechnique(mTechnique, FXL_RESTORE_DEFAULT_STATE);//FXL_RESTORE_DEFAULT_STATE
   return mPasses.size();
}

void CCompiledShader::End()
{
   //mEffect->EndTechnique();
}

int const CCompiledShader::BeginPass(int const currentPass)
{
   BPE_VERIFY(mCurrentPass == -1, false, "Must end previous pass before beginning a new one");
   CCompiledShaderPass & compiledShaderPass = mPasses[currentPass];
   mCurrentPass = currentPass;

   //mEffect->BeginPassFromIndex(mCurrentPass);
   CCompiledShaderPass::SetSamplerStates(compiledShaderPass.mSamperStates->mSamperStates);
   CCompiledShaderPass::SetRenderStates(compiledShaderPass.mRenderStates->mRenderStates);
   GetD3DDevice()->SetVertexShader(compiledShaderPass.mVertexShader);
   GetD3DDevice()->SetPixelShader(compiledShaderPass.mPixelShader);

   compiledShaderPass.EvaluatePassScript();
   return compiledShaderPass.mDrawType;
}

void CCompiledShader::EndPass()
{
   BPE_VERIFY(mCurrentPass != -1, false, "Must begin pass before ending it");
   CCompiledShaderPass & compiledShaderPass = mPasses[mCurrentPass];
   mCurrentPass = -1;

   compiledShaderPass.ResetPassScript();

   //mEffect->EndPass();
   CCompiledShaderPass::SetSamplerStates(compiledShaderPass.mSamperStates->mSamperStates_Restore);
   CCompiledShaderPass::SetRenderStates(compiledShaderPass.mRenderStates->mRenderStates_Restore);
}

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

   //mEffect->CommitU();
   // New Set parameters method
   mPasses[mCurrentPass].BeginShaderBatch(pParameters, pParametersEnd);
}

//----------------------------------------------------------------------------

CCompiledShaderCache::CCompiledShaderCache()
:  mpCurrentShader(NULL)
{
   FlushAllShaders();
}

//----------------------------------------------------------------------------

CCompiledShaderCache::~CCompiledShaderCache()
{
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::FreeShaders()
{
   for( std::map<uint32, CCompiledShaderCombinationHolder*>::iterator shaderIt = mShaders.begin(); shaderIt != mShaders.end(); ++shaderIt )
   {
      CCompiledShaderCombinationHolder * combinations = shaderIt->second;

      for( std::map<uint32, CCompiledShader*>::iterator combinationIt = combinations->mShaderCombinations.begin(); combinationIt != combinations->mShaderCombinations.end(); ++combinationIt )
         delete combinationIt->second;

      combinations->mShaderCombinations.clear();
      delete combinations;
   }

   mShaders.clear();

   mpCurrentShader = NULL;
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::FlushAllShaders()
{
   FreeShaders();
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::SetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   uint32 const definesCRC = CCRC::CalculateCRC32AsString(pDefines);
   SetShader(shaderFileId, definesCRC, pDefines);
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::SetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines)
{
   mpCurrentShader = GetShader(shaderFileId, definesCRC, pDefines);
}

//----------------------------------------------------------------------------

CCompiledShader * CCompiledShaderCache::GetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   uint32 const definesCRC = CCRC::CalculateCRC32AsString(pDefines);
   return GetShader(shaderFileId, definesCRC, pDefines);
}

CCompiledShader * CCompiledShaderCache::GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines)
{
   CCompiledShader * pCurrentShader = NULL;

   std::map<uint32, CCompiledShaderCombinationHolder*>::iterator foundFileIt = mShaders.find(shaderFileId.mHash);

   if( foundFileIt != mShaders.end() )
   {
      std::map<uint32, CCompiledShader*>::iterator foundCombinationIt = foundFileIt->second->mShaderCombinations.find(definesCRC);
      if (foundCombinationIt != foundFileIt->second->mShaderCombinations.end())
      {
         // Found shader, return immediately.
         pCurrentShader = foundCombinationIt->second;
         return pCurrentShader;
      }
   }
   else
   {
      // shaderFileId not found, compile!
      CStopWatch watch;
      TLockedResource<Effect_360> effect( gpResources->GetResource(CResId( shaderFileId.mResource )) );
      if( !effect.IsNull() )
      {
         mShaders[shaderFileId.mHash] = new CCompiledShaderCombinationHolder(*effect);
      }
      bpe_debugger_printf( "time to load/compile shader (%s %s) %.f ms\n", shaderFileId.mResource.c_str(), pDefines, watch.GetElapsedTime() * 1000.0f );

      // Rebind shader (call function to prevent code duplication)
      pCurrentShader = GetShader(shaderFileId, definesCRC, pDefines);
   }

   return pCurrentShader;
}

//----------------------------------------------------------------------------

void CCompiledShaderCache::LoadShaderFromMemory(CShaderFileId const &shaderFileId, char const * const memBuffer, uint32 const memBufferSize)
{
   CMemoryInputStream memStream((void *)memBuffer, memBufferSize, CMemoryInputStream::kOwner_App);
   uint32 fourCC = memStream.ReadUint32();
   BPE_VERIFY(fourCC == 'EFCT', false, "Invalid effect resource");

   CStopWatch watch;
   TResource<Effect_360> effectRes( new Effect_360( memStream ) );

   gpResources->ManuallyAddToResourceMap(CResId( shaderFileId.mResource ), effectRes);
   if (gpResources->ResourceCache() != NULL)
      gpResources->ResourceCache()->CacheResource(effectRes, CResId( shaderFileId.mResource ));

   BPE_VERIFY( mShaders.find(shaderFileId.mHash) == mShaders.end(), false, "Loading shader from memory multiple times" );
   mShaders[shaderFileId.mHash] = new CCompiledShaderCombinationHolder(*effectRes);
}

//----------------------------------------------------------------------------

CCompiledShaderCombinationHolder::CCompiledShaderCombinationHolder(Effect_360 const & effect)
{
   HRESULT hr;
   //
   mVertexShader.reserve( effect.mVertexShader.size() );
   for( int ii=0; ii < effect.mVertexShader.size(); ++ii )
   {
      VertexShader_X360 const & vertexShader = effect.mVertexShader[ii];

      IDirect3DVertexShader9 * pVertexShader;
      hr = D3DDeviceUncached()->CreateVertexShader((DWORD*)&vertexShader.mVertexShaderCode[0], &pVertexShader);
      mVertexShader.push_back(TComPtr<IDirect3DVertexShader9>(pVertexShader));
   }

   mPixelShader.reserve( effect.mPixelShader.size() );
   for( int ii=0; ii < effect.mPixelShader.size(); ++ii )
   {
      PixelShader_X360 const & pixelShader = effect.mPixelShader[ii];

      IDirect3DPixelShader9 * pPixelShader;
      hr = D3DDeviceUncached()->CreatePixelShader((DWORD*)&pixelShader.mPixelShaderCode[0], &pPixelShader);
      mPixelShader.push_back(TComPtr<IDirect3DPixelShader9>(pPixelShader));
   }
   
   mParams = effect.mParams;
   mSamperStates = effect.mSamperStates;
   mRenderStates = effect.mRenderStates;

   // fill in all CCompiledShader
   for( int iCombination=0; iCombination < effect.mCombinations.size(); ++iCombination )
   {
      CCompiledShader* pCompileShader = new CCompiledShader( effect.mCombinations[iCombination] );
      mShaderCombinations[ effect.mCombinations[iCombination].mHash ] = pCompileShader;
      pCompileShader->Init( effect.mCombinations[iCombination], this );
   }
}
