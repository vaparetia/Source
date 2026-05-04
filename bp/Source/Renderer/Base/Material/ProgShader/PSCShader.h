//----------------------------------------------------------------------------
// CShader.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CCompiledShaderCacheTypes.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"
#include "Engine/System/CGuid.h"
#include "vector"

//----------------------------------------------------------------------------

class CDrawableRenderEntity;
class CEvaluatorUpdateData;
class CLight;
class CLightState;
class CResId;
class CGameObjectComponentProperties;
class CCompiledShader;
class CShaderParameterBuffer;
struct SRenderEntry;

//----------------------------------------------------------------------------

class CShaderCRCs
{
protected:
   CShaderCRCs() {};

public:
   RENDERER_API static uint32 const GetParameterCRC(const char * pParameterName);
};

//----------------------------------------------------------------------------

class CShaderCommonCRCs : public CShaderCRCs
{
public:
   CShaderCommonCRCs();

   uint32   mExternalShaderParam;

   uint32   mWorld;
   uint32   mJointWorld;
   
   uint32   mView;
   uint32   mViewProjection;
   uint32   mWorldViewProjection;
   uint32   mProjection;

   uint32   mEyePosition;

   uint32   mRenderTargetDimensions;
   uint32   mRenderTargetNearFar;
   uint32   mRenderTargetDepthRange;

   static int const kLightCount = 8;
   uint32   mLightColor[kLightCount];
   uint32   mLightPosition[kLightCount];
   uint32   mLightAttenuation[kLightCount];

   uint32   mFrameBufferTexture;

   uint32   mAdditive;
   uint32   mModulate;
};

//----------------------------------------------------------------------------

class RENDERER_API CShader
{
public:
   typedef boost::shared_ptr<CGameObjectComponentProperties const>   TShaderProperties;
   enum EParameterType
   {
      kPT_Shared,    // Parameters are shared among all objects with the same shader instance.
      kPT_Unshared,  // Parameters are unique to the object passed in
   };

   // Creates a specific 'material' instance from properties.
   static boost::shared_ptr<CShader> const   CreateMaterial(TShaderProperties const & properties);

protected:
   explicit CShader(TShaderProperties const & properties);

   CShader();  // Internal use only!
public:
   virtual ~CShader();
 
   // shader implementation
   // This function will get called multiple times between each shader 'bind'
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

   /// begins drawing using single pass shader (inplace)
   virtual void                  BeginSinglePassRender();
   /// ends drawing using single pass shader (inplace)
   virtual void                  EndSinglePassRender();

   /// binds the material
   virtual bool                  CanBatch(SRenderEntry const * pPrevObject, SRenderEntry const * pCurrentObject)     { return false; };
   virtual void                  Bind(SRenderEntry const * pFirstObject) = 0;
   virtual void                  BindPassShader(int const pass, SRenderEntry const * pFirstObject) = 0;
   virtual void                  UnbindPassShader(int const pass, SRenderEntry const * pFirstObject) = 0;

   /// Draws multiple objects that only have transient render state changes
   virtual void                  DrawRenderObjects(SRenderEntry const * pFirstObject, int const numObjects);

   virtual uint64                GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const = 0;

   // set render state related functions
   void                          SetShaderVertexDataBinding(CShaderVertexDataBinding const & flags);
   void                          SetShader(CShaderFileId const &shaderFileId, char const * const pDefines);

   CGameObjectComponentProperties const * const 
                                 GetProperties() const      { return mProperties.get(); }


   void                          BuildConstantParameterBuffer();

private:
   void                          FreeConstantParameters();

protected:
   static CShaderCommonCRCs      sCommonCRC;
   CShaderVertexDataBinding      mShaderVertexDataBinding;

   TShaderProperties             mProperties;
   
   uint8*                        mpMaterialConstantParameters[2];
   int                           mMaterialConstantParametersSize[2];

   int                           mPassCount;
   int                           mSubPassCount;

   mutable CMatrix4              mWorldViewProjectionMatrixCached; // HACK - So we don't need to recalc in sub classes
};

//----------------------------------------------------------------------------
