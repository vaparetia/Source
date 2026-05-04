//----------------------------------------------------------------------------
// Win32CCompiledShaderCache.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/TComPtr.h"
#include "Engine/Resource/CResourceManager.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"

struct ID3DXEffect;
struct ID3DXEffectStateManager;
#ifndef D3DXHANDLE
typedef LPCSTR D3DXHANDLE;
#endif

//----------------------------------------------------------------------------

class CCompiledShaderPass : public CBaseCompiledShaderPass
{
public:
   CCompiledShaderPass()
   {
   }
};

//----------------------------------------------------------------------------

class RENDERER_API CCompiledShader
{
public:
   struct SParameter
   {
      SParameter(uint32 const crc, uint32 const handle)
      : mCRC(crc)
      , mHandle(handle)
      {
      }

      bool const operator < (SParameter const & rhs) const
      {
         return mCRC < rhs.mCRC;
      }

      uint32 mCRC;
      uint32 mHandle;
   };

public:
   CCompiledShader(TComPtr<ID3DXEffect> const & effect, D3DXHANDLE technique);
   
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
   void HandleParameter(D3DXHANDLE parameter, int /*D3DXPARAMETER_TYPE*/ const type, char const * pName, char const * pSemantic, bool const isArrayElement);

public:
   TComPtr<ID3DXEffect>             mEffect;
   D3DXHANDLE                       mTechnique;

   std::vector<SParameter>          mParameters;
   std::vector<SParameter>          mTextureParameters;

   std::vector<CCompiledShaderPass> mPasses;
   int                              mCurrentPass;
};

//----------------------------------------------------------------------------

class RENDERER_API CCompiledShaderCache : public CBaseCompiledShaderCache
{
   friend class CVertexData;
public:
   typedef std::map<uint32, CCompiledShader>       TShaderCombinationMap;  // key is CRC hash from CShaderFileId
   typedef std::map<uint32, TShaderCombinationMap> TShaderMap;             // key is CRC of defines

public:
   CCompiledShaderCache();
   virtual ~CCompiledShaderCache();

   void  SetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   void  SetShader(CCompiledShader const *  pCurrentShader)      { mpCurrentShader = const_cast<CCompiledShader *>(pCurrentShader); };

   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
   void  FlushAllShaders();

   CCompiledShader * CurrentShader() { return mpCurrentShader; }

public:
   CCompiledShader*                 mpCurrentShader;
   TComPtr<ID3DXEffectStateManager> mEffectStateManager;

private:
   TShaderMap        mShaders;
};

//----------------------------------------------------------------------------

