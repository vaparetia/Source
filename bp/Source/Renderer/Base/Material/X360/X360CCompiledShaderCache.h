//----------------------------------------------------------------------------
// X360CCompiledShaderCache.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/TComPtr.h"
#include "Engine/Resource/CResourceManager.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/X360/X360CookedShaderFormat.h"

#include <xtl.h>
#include <fxl.h>

//----------------------------------------------------------------------------
class CCompiledShaderCombinationHolder;

class CCompiledShaderPass : public CBaseCompiledShaderPass
{
public:
   CCompiledShaderPass()
   {
   }
   void Init(Pass_360 const & pass, CCompiledShaderCombinationHolder* pCombinationHolder);

   void SetParam( SParam_X360 const & param, const uint32* pParameter );
   void BeginShaderBatch(uint32** pParameters, uint32** pParametersEnd);
   static void SetSamplerStates(std::vector<SamplerState_X360> const & aSamplerStates);
   static void SetRenderStates(std::vector<RenderState_X360> const & aRenderStates);

   Params_X360* mParams;
   SamplerStates_X360* mSamperStates;
   RenderStates_X360* mRenderStates;

   IDirect3DVertexShader9* mVertexShader;
   IDirect3DPixelShader9* mPixelShader;
};

//----------------------------------------------------------------------------

class CCompiledShader
{
public:
   CCompiledShader(EffectCombination_360 const & combination);
   void Init(EffectCombination_360 const & pass, CCompiledShaderCombinationHolder* pCombinationHolder);

   // Start using compiled shader, returns number of passes
   int const Begin();
   // Finish compiled shader
   void End();

   // Start using pass, returns pass type
   int const BeginPass(int const currentPass);
   void EndPass();

   void BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
      uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
      uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize);

   void EndShaderBatch() {}

   int const GetCurrentPassIndex() const { return mCurrentPass; }
   int const GetPassCount() const { return mPasses.size(); }
private:
   void BuildParameterLookup();

public:

   std::vector<CCompiledShaderPass> mPasses;
   int                              mCurrentPass;
};

//----------------------------------------------------------------------------

class CCompiledShaderCombinationHolder
{
public:
   CCompiledShaderCombinationHolder(Effect_360 const & effect);

   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
private:
   void BuildParameterLookup();

public:

   std::map<uint32, CCompiledShader*> mShaderCombinations;

   std::vector<Params_X360> mParams;
   std::vector<SamplerStates_X360> mSamperStates;
   std::vector<RenderStates_X360> mRenderStates;

   std::vector< TComPtr<IDirect3DVertexShader9> > mVertexShader;
   std::vector< TComPtr<IDirect3DPixelShader9> > mPixelShader;
};

//----------------------------------------------------------------------------

class RENDERER_API CCompiledShaderCache : public CBaseCompiledShaderCache
{
   friend class CVertexData;

public:
   CCompiledShaderCache();
   virtual ~CCompiledShaderCache();

   void  SetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   void  SetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
   void  SetShader(CCompiledShader const *  pCurrentShader)      { mpCurrentShader = const_cast<CCompiledShader *>(pCurrentShader); };

   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
   void LoadShaderFromMemory(CShaderFileId const &shaderFileId, char const * const memBuffer, uint32 const memBufferSize);
   void FlushAllShaders();

   CCompiledShader * CurrentShader() { return mpCurrentShader; }
private:
   void FreeShaders();

public:
   CCompiledShader*                 mpCurrentShader;
   TComPtr<ID3DXEffectStateManager> mEffectStateManager;

private:
   std::map<uint32, CCompiledShaderCombinationHolder*> mShaders;
};

//----------------------------------------------------------------------------

