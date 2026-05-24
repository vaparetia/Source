//----------------------------------------------------------------------------
// CShader.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PSCShader.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/ShaderObjects/CShaderObjectFactory.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshChunkRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Frontend/CRenderViewport.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Engine/Graphics/CLight.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Math/BPEMath.h"


uint32 const CShaderCRCs::GetParameterCRC(const char * pParameterName)
{
   uint32 crcValue = 0xFFFFFFFF;

   if (pParameterName) 
   {
      while(*pParameterName) 
      {
         uint32 tableTemp = (uint32)((crcValue & 0xff) ^ (uint8)*pParameterName);

         for (int bitLoop = 0; bitLoop < 8; bitLoop++) 
         {
            if (tableTemp & 1)
               tableTemp = (tableTemp >> 1) ^ 0xedb88320;
            else
               tableTemp >>= 1;
         }

         crcValue = (crcValue >> 8) ^ tableTemp;
         pParameterName++;
      }
   }

   return crcValue;
}

//----------------------------------------------------------------------------

CShaderCommonCRCs::CShaderCommonCRCs()
:  mExternalShaderParam(         GetParameterCRC("g_ExternalShaderParam"))
,  mWorld(                       GetParameterCRC("g_World"))
,  mJointWorld(                  GetParameterCRC("g_JointWorld"))
,  mView(                        GetParameterCRC("g_View"))
,  mViewProjection(              GetParameterCRC("g_ViewProjection"))
,  mWorldViewProjection(         GetParameterCRC("g_WorldViewProjection"))
,  mProjection(                  GetParameterCRC("g_Projection"))
,  mEyePosition(                 GetParameterCRC("g_EyePosition"))
,  mRenderTargetDimensions(      GetParameterCRC("g_RenderTargetDimensions"))
,  mRenderTargetNearFar(         GetParameterCRC("g_RenderTargetNearFar"))
,  mRenderTargetDepthRange(      GetParameterCRC("g_RenderTargetDepthRange"))
,  mFrameBufferTexture(          GetParameterCRC("g_FrameBufferTexture"))
,  mAdditive(                    GetParameterCRC("g_AdditiveColor"))
,  mModulate(                    GetParameterCRC("g_ModulateColor"))
{
   for( int i = 0; i < kLightCount; ++i )
   {
      mLightColor[i] = GetParameterCRC(CStringExtras::Stringize_s("g_LightColor%d", i));
      mLightPosition[i] = GetParameterCRC(CStringExtras::Stringize_s("g_LightPosition%d", i));
      mLightAttenuation[i] = GetParameterCRC(CStringExtras::Stringize_s("g_LightAttenuation%d", i));
   }
}


//----------------------------------------------------------------------------

namespace
{
   int const kConstantMaterialParametersMaxSize = 16 * 1024;
   uint8 sConstantMaterialParametersTempBuffer[kConstantMaterialParametersMaxSize];
}

CShaderCommonCRCs CShader::sCommonCRC;

//----------------------------------------------------------------------------

CShader::CShader(boost::shared_ptr<CGameObjectComponentProperties const> const & properties)
: mProperties(properties)
, mWorldViewProjectionMatrixCached(CMatrix4::Identity())
, mPassCount(1)
, mSubPassCount(1)
{
   BPE_CTASSERT(BPE_ARRAY_SIZE(mpMaterialConstantParameters) == kSCS_Count);

   mpMaterialConstantParameters[0] = NULL;
   mpMaterialConstantParameters[1] = NULL;
   mMaterialConstantParametersSize[0] = 0;
   mMaterialConstantParametersSize[1] = 0;
}

//----------------------------------------------------------------------------

CShader::CShader()
: mShaderVertexDataBinding()
, mWorldViewProjectionMatrixCached(CMatrix4::Identity())
, mPassCount(1)
, mSubPassCount(1)
{
   BPE_CTASSERT(BPE_ARRAY_SIZE(mpMaterialConstantParameters) == kSCS_Count);

   mpMaterialConstantParameters[0] = NULL;
   mpMaterialConstantParameters[1] = NULL;
   mMaterialConstantParametersSize[0] = 0;
   mMaterialConstantParametersSize[1] = 0;
}

//----------------------------------------------------------------------------

CShader::~CShader()
{
   FreeConstantParameters();
   RenderBackend()->RemoveMaterialPropertiesMapping(*this);
}

//----------------------------------------------------------------------------

void CShader::FreeConstantParameters()
{
   if(mpMaterialConstantParameters[0])
   {
      BPE_FREE_ALIGNED(mpMaterialConstantParameters[0]);
      mpMaterialConstantParameters[0] = NULL;
      mMaterialConstantParametersSize[0] = 0;
   }

   if(mpMaterialConstantParameters[1])
   {
      BPE_FREE_ALIGNED(mpMaterialConstantParameters[1]);
      mpMaterialConstantParameters[1] = NULL;
      mMaterialConstantParametersSize[1] = 0;
   }
}

//----------------------------------------------------------------------------

void CShader::BuildConstantParameterBuffer()
{
   FreeConstantParameters();

   for( int i = 0; i < kSCS_Count; ++i )
   {
      // Build parameters using our temporary buffer
      CShaderParameterBuffer parameterBuffer(sConstantMaterialParametersTempBuffer);
      BuildMaterialConstantParameters((EShaderColorSpace)i, parameterBuffer);

      // Then we allocate the real buffer and copy the data in (if there was any data written).
      int const parametersSize = parameterBuffer.GetSize();
      if( parametersSize > 0 )
      {
         int const alignedSize = (parametersSize + 15) & ~15;
         mMaterialConstantParametersSize[i] = parametersSize;
         mpMaterialConstantParameters[i] = (uint8*)BPE_MALLOC_ALIGNED(16, alignedSize);
         memcpy(mpMaterialConstantParameters[i], sConstantMaterialParametersTempBuffer, parametersSize);
      }
   }
   
}

//----------------------------------------------------------------------------

void CShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   // Does nothing, could make this pure virtual
}

//----------------------------------------------------------------------------

void CShader::BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   switch(parameterType)
   {
   case kPT_Shared:
      {
         // viewport transform
         parameters.AddParameter(sCommonCRC.mView, RenderBackend()->GetViewMatrix());

         // projection transform
         parameters.AddParameter(sCommonCRC.mProjection, RenderBackend()->GetProjectionMatrix());

         // viewport * projection transform
         parameters.AddParameter(sCommonCRC.mViewProjection, RenderBackend()->GetProjectionTimesViewMatrix() );

         // eye position
         parameters.AddParameter(sCommonCRC.mEyePosition, RenderBackend()->GetCameraMatrix().GetTranslation());

         // viewport target width, height, 1/width, 1/height
         {
            real32 const width = static_cast<real32>(RenderBackend()->GetViewWidth());
            real32 const height = static_cast<real32>(RenderBackend()->GetViewHeight());

            CVector4 const data(width, height, 1.0f / width, 1.0f / height);

            parameters.AddParameter(sCommonCRC.mRenderTargetDimensions, data);
         }

         CRenderer * pRenderer = Renderer();
         if( pRenderer )
         {
            CRenderViewport const * pViewport = pRenderer->GetCurrentViewport();
            if( pViewport )
            {
               if( pViewport->GetRenderTarget().mpColorBuffer[0] != NULL )
               {
                  parameters.AddTexture(sCommonCRC.mFrameBufferTexture, pViewport->GetRenderTarget().mpColorBuffer[0], true, true);
               }

               // Set Render Target Near/Far
               {
                  real32 const nearZ = pViewport->GetNearClipPlane();
                  real32 const farZ = pViewport->GetFarClipPlane();

                  CVector4 data( nearZ, farZ, farZ - nearZ, farZ * nearZ );
                  parameters.AddParameter(sCommonCRC.mRenderTargetNearFar, data );
               }

               // Set Render Target Depth Range
               {
                  SRenderTarget const & renderTarget = pViewport->GetRenderTarget();

                  real32 const min = renderTarget.mMinZ;
                  real32 const max = renderTarget.mMaxZ;

                  real32 const range = max - min;
                  
                  CVector4 data(min, max, range, 1.0f / range);
                  parameters.AddParameter(sCommonCRC.mRenderTargetDepthRange, data);
               }
            }
         }
      }
      break;
   
   case kPT_Unshared:
      {
         CMatrix34 const & objectTransform = object->mpObject->GetTransform();
         CMatrix4 const objectTransform44 = CMatrix4::FromMatrix34(objectTransform);

         parameters.AddParameter(sCommonCRC.mWorld, objectTransform44);

         {
            mWorldViewProjectionMatrixCached = RenderBackend()->GetProjectionTimesViewMatrix()  * objectTransform44;

            parameters.AddParameter(sCommonCRC.mWorldViewProjection, mWorldViewProjectionMatrixCached);

#if BPE_TARGET == BPE_TARGET_DREAMCAST
            // DC has no GPU vertex shader; pass the model matrix to RenderPrimitives
            // for CPU-side MVP transform.
            RenderBackend()->SetModelMatrix(objectTransform44);
#endif
         }

         parameters.AddParameterGamma(sCommonCRC.mAdditive, CColor::Black(), colorSpace);

         parameters.AddParameterGamma(sCommonCRC.mModulate, CColor::White(), colorSpace);

         parameters.AddParameter(sCommonCRC.mExternalShaderParam, CVector4::Zero());
      }
      break;
   }
}

//----------------------------------------------------------------------------

void CShader::InitializeEvaluators(CEvaluatorUpdateData &/*updateData*/) const
{
}

//----------------------------------------------------------------------------

void CShader::SetShader(CShaderFileId const &shaderFileId, char const * const pDefines)
{
   BPE_ASSERT(pDefines != NULL, "NULL pDefines!");
   RenderBackend()->ShaderCache()->SetShader(shaderFileId, pDefines);
}

//----------------------------------------------------------------------------

void CShader::SetShaderVertexDataBinding( CShaderVertexDataBinding const & binding )
{
   mShaderVertexDataBinding = binding;
}

//----------------------------------------------------------------------------

void CShader::DrawRenderObjects(SRenderEntry const * pObjects, int const numObjects)
{
   BPE_ASSERT(numObjects > 0, "Must draw at least one object!");

   CCompiledShaderCache * pShaderCache = RenderBackend()->ShaderCache();

   EShaderColorSpace const shaderColorSpace = RenderBackend()->GetScreenOutputControl().GetShaderColorSpace();

   CShaderParameterBufferAllocator::BeginRegion();

   for( int pass = 0; pass < mPassCount; ++pass )
   {
      BindPassShader(pass, pObjects);
      CCompiledShader * pShader = pShaderCache->CurrentShader();
      if( pShader )
      {
#if BPE_TARGET == BPE_TARGET_WIN32
         pShader->BeginShaderBatch(mpMaterialConstantParameters[shaderColorSpace], mMaterialConstantParametersSize[shaderColorSpace],
                                   NULL, 0,
                                   NULL, 0);
#endif

         int const shaderPassCount = pShader->Begin();
         BPE_ASSERT(shaderPassCount == 1, "We currently only support one pass per shader");

         pShader->BeginPass(0);

         CShaderParameterBuffer sharedParameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
         BuildRuntimeParameters(pass, pObjects, kPT_Shared, shaderColorSpace, sharedParameterBuffer);
         CShaderParameterBufferAllocator::FinalizeParameterBuffer(sharedParameterBuffer);

#if BPE_TARGET == BPE_TARGET_WIN32
         pShader->BeginShaderBatch(NULL, 0,
                                   sharedParameterBuffer.GetStartAddress(), sharedParameterBuffer.GetSize(),
                                   NULL, 0);
#endif
         for (int loop = 0; loop < numObjects; loop++)
         {
            SRenderEntry const * pObject = pObjects + loop;

            CShaderParameterBuffer unsharedParameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
            BuildRuntimeParameters(pass, pObject, kPT_Unshared, shaderColorSpace, unsharedParameterBuffer);
            CShaderParameterBufferAllocator::FinalizeParameterBuffer(unsharedParameterBuffer);

#if BPE_TARGET == BPE_TARGET_WIN32
            pShader->BeginShaderBatch(NULL, 0,
                                      NULL, 0,
                                      unsharedParameterBuffer.GetStartAddress(), unsharedParameterBuffer.GetSize());
#else

            pShader->BeginShaderBatch(mpMaterialConstantParameters[shaderColorSpace], mMaterialConstantParametersSize[shaderColorSpace],
                                      sharedParameterBuffer.GetStartAddress(), sharedParameterBuffer.GetSize(),
                                      unsharedParameterBuffer.GetStartAddress(), unsharedParameterBuffer.GetSize());
#endif
            pObject->mpObject->RenderGeometry(mShaderVertexDataBinding);

            pShader->EndShaderBatch();
         }
      }

      if( pShader )
         pShader->EndPass();

      UnbindPassShader(pass, pObjects);

      if( pShader )
         pShader->End();
   }

   CShaderParameterBufferAllocator::EndRegion();
}

//----------------------------------------------------------------------------

void CShader::BeginSinglePassRender()
{
   CShaderParameterBufferAllocator::BeginRegion();

   EShaderColorSpace const shaderColorSpace = RenderBackend()->GetScreenOutputControl().GetShaderColorSpace();

   CCompiledShader * pShader = RenderBackend()->ShaderCache()->CurrentShader();

   pShader->Begin();
   pShader->BeginPass(0);

   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
   SRenderEntry fakeEntry;
   fakeEntry.mpObject = RenderBackend()->GetFakeRenderEntity();
   fakeEntry.mSortKey = 0;
   BuildRuntimeParameters(0, &fakeEntry, kPT_Shared, shaderColorSpace, parameterBuffer);
   BuildRuntimeParameters(0, &fakeEntry, kPT_Unshared, shaderColorSpace, parameterBuffer);
   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   pShader->BeginShaderBatch(mpMaterialConstantParameters[shaderColorSpace], mMaterialConstantParametersSize[shaderColorSpace],
                             parameterBuffer.GetStartAddress(), parameterBuffer.GetSize(),
                             NULL, 0);

   CShaderParameterBufferAllocator::EndRegion();
}

//----------------------------------------------------------------------------

void CShader::EndSinglePassRender()
{
   CCompiledShader * pShader = RenderBackend()->ShaderCache()->CurrentShader();

   pShader->EndShaderBatch();
   pShader->EndPass();
   pShader->End();
}

//----------------------------------------------------------------------------


