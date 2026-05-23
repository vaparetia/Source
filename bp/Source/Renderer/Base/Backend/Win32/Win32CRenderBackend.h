//------------------------------------------------------------------------------------------
// Win32CRenderBackend.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "d3d9.h"

#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CVertexArray.h"

#include "Engine/Mechanics/TComPtr.h"

#include "Engine/Math/CMatrix4.h"
#include "Engine/Math/CVector2.h"

#include "Engine/Primitives/CAABox.h"

#include "Renderer/Base/Backend/Win32/CD3DCachedDevice.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Frontend/RenderTypes.h"

#include <string>

#define RENDERBACKEND_SUPPORTS_ALPHA_TEST() 1
#define RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET() 1

//------------------------------------------------------------------------------------------

class CBaseTexture;
class CMeshHandle;
class CMatrix4;
class CMeshChunk;
class IResourcePool;
class IEvaluatorFactory;
class CDynamicVertexBufferPool_RT;
class CDynamicIndexBufferPool_RT;
class CDynamicVertexBufferPool_UT;
class CDynamicIndexBufferPool_UT;
class CDynamicIndexBufferPoolChunk_RT;
class CDynamicIndexBufferPoolChunk_UT;

//----------------------------------------------------------------------------

static const D3DPRIMITIVETYPE gD3DPrimitiveMapping[] =
{
   D3DPT_TRIANGLELIST,
   D3DPT_TRIANGLESTRIP,
   D3DPT_TRIANGLEFAN,
   D3DPT_LINELIST,
   D3DPT_LINESTRIP,
   D3DPT_POINTLIST
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gD3DPrimitiveMapping) == CMeshChunk::kPrimitive_Count);


struct SVertexDeclKey
{
   SVertexDeclKey(CShaderVertexDataBinding const & binding, CVertexData const & data)
      :  mBinding(binding)
      ,  mData(data)
   {
   }

   bool const operator < (SVertexDeclKey const & rhs) const;

   CShaderVertexDataBinding   mBinding;
   CVertexData                mData;
};

//----------------------------------------------------------------------------

class RENDERER_API CRenderBackend : public CBaseRenderBackend
{
   typedef CBaseRenderBackend inherited;

   typedef std::map<SVertexDeclKey, TComPtr<IDirect3DVertexDeclaration9> > TDeclarationMap;
public:

   enum EBlendFunc
   {
      kBF_Zero,

      kBF_One,

      kBF_SrcColor,
      kBF_InvSrcColor,

      kBF_DstColor,
      kBF_InvDstColor,

      kBF_SrcAlpha,
      kBF_InvSrcAlpha,

      kBF_DstAlpha,
      kBF_InvDstAlpha,

      kBF_FixedAlpha,
      kBF_InvFixedAlpha,

      kBF_FixedColor,
      kBF_InvFixedColor,

      kBF_Count
   };

   enum EBlendOp
   {
      kBO_Add,
      kBO_Subtract,
      kBO_RevSubtract,
      kBO_Min,
      kBO_Max,

      kBO_Count
   };

   enum EAlphaFunc
   {
      kAF_Never,
      kAF_Less,
      kAF_Equal,
      kAF_LEqual,
      kAF_Greater,
      kAF_NotEqual,
      kAF_GEqual,
      kAF_Always,

      kAF_Count
   };

   enum EDepthFunc
   {
      kDF_Never,
      kDF_Less,
      kDF_Equal,
      kDF_LEqual,
      kDF_Greater,
      kDF_NotEqual,
      kDF_GEqual,
      kDF_Always,

      kDF_Count
   };

   enum EStencilOp
   {
      kSO_Keep,
      kSO_Zero,
      kSO_Replace,
      kSO_Incr,
      kSO_Decr,
      kSO_IncrWrap,
      kSO_DecrWrap,
      kSO_Invert,

      kSO_Count
   };

   enum EWrapMode
   {
      kWM_Wrap,
      kWM_Mirror,
      kWM_Clamp,
      kWM_Border,

      kWM_Count
   };

   enum EFilterMode
   {
      kFM_Nearest,
      kFM_Linear,
      kFM_Nearest_Nearest,
      kFM_Linear_Nearest,
      kFM_Nearest_Linear,
      kFM_Linear_Linear,

      kFM_Count
   };

   enum ECullMode
   {
      kCM_None,
      kCM_CW,
      kCM_CCW
   };

public:
   explicit CRenderBackend(IResourcePool & resourcePool,
                           SRenderInitialization const &initFlags);

   ~CRenderBackend();
   
   void Clear(int const clearFlags, CColor const color, real32 const z = 1.0f, int const stencil = 0);

   void BeginScene();
   void EndScene();
   void Present();
   void ShutDown();

   void BeginTiling(SRenderTarget const & renderTarget);
   void EndTiling();

   void ResolveRenderTarget(CBaseTexture * pRenderTexture) {}
   void ResolvePartialRenderTarget(CBaseTexture * pRenderTexture, SRect const & srcRect) {}
   void ResolveRenderTargetPredicated(CBaseTexture * pRenderTexture) {}
   void ResolveDepthStencilPredicated(CBaseTexture * pRenderTexture) {}
   void ResolveDepthStencilPredicatedMSAA2(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1) {}
   void ResolveDepthStencilPredicatedMSAA4(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1, CBaseTexture * pRenderTexture2, CBaseTexture * pRenderTexture3) {}

   // Win32 version is never stereo 3d
   bool IsStereo3D() const { return false; }

   void SetRenderTarget(SRenderTarget const & renderTarget);

   SRenderTarget const & GetCurrentRenderTarget() const { return mCurrentRenderTarget; }
   CBaseTexture * GetCurrentRenderTexture() const { return NULL; }
   CBaseTexture * GetCurrentRenderDepth() const { return NULL; }

   int GetPresentationInterval() const { return 1; }

   void SetShadeMode(int isSmooth);

   void SetAlphaTestEnable(bool enable);
   void SetAlphaFunc(EAlphaFunc func, int refValue);

   // enable disable stencil testing
   void SetStencilEnable(bool enable);
   // enable disable stencil testing
   void SetBackStencilEnable(bool enable);

   void SetColorAlphaWriteEnabled(bool const color, bool const alpha);

   // stencil operation on stencil fail, depth fail and depth pass
   void SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass );
   // stencil operation on stencil fail, depth fail and depth pass
   void SetBackStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass );
   // write mask
   void SetStencilMask(uint32 mask);
   // stencil test function, test reference value, stencil test mask
   void SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask);
   // stencil test function, test reference value, stencil test mask
   void SetBackStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask);

   void SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode);
   void SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode);

   void SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode, uint32 const fixedColor);
   void SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode, uint32 const fixedColor);

   void SetBlendOp(EBlendOp const blendOp);
   void SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp);

   void SetFogEnabled( bool const enable ) { mFogEnabled = enable; }
   void SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color );
   bool GetFogEnabled() const { return mFogEnabled; }
   CColorf const &GetFogColor() const { return mFogColor; }
   CVector2 const &GetWin32FogParams() const { return mFogParams; }

   void SetDepthCompareEnabled(bool const enable);
   void SetDepthFunc(EDepthFunc const func);

   void SetDepthWriteEnabled(bool const enable);

   void SetCullMode(ECullMode const mode);

   void SetLineWidth(real32 const width);
   void SetLineSmoothEnabled(bool const enable);

   // Coords are in pixels
   void RenderQuad2d(CVector2 const & start,
                             CVector2 const & end,
                             CColor const & color);

   void RenderLine2d(CVector2 const & start,
                     CVector2 const & end,
                     CColor const & color);

   void SetIndexData(CIndexBuffer const * indexBuffer);
   void SetIndexData(CDynamicIndexBufferPoolChunk_RT const * pChunk);
   void SetIndexData(CDynamicIndexBufferPoolChunk_UT const * pChunk);

   void SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2);
   void ForceVertexDataRebind();

   void RenderPrimitives(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount, uint32 const indexBufferOffset, uint32 const indexCount);
   void RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount);

   void RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
                                       CShaderVertexDataBinding binding, 
                                       void const * pData, 
                                       uint32 const numVertices);

   void SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const allowAniso = true);
   void SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV);
   void SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter);

   void SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector);

   // Win32 implementation specific functions
   
   void StoreWindowPosition() const;

   // get direct3DDevice
   TComPtr<IDirect3DDevice9> & GetDirect3DDevice();
   CD3DCachedDevice * GetCachedDeviceWrapper() { return mCachedDeviceWrapper.get(); }

   LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

   CMatrix4 const &  GetViewMatrix() const { return mViewMatrix; }
   CMatrix4 const &  GetCameraMatrix() const { return mCameraMatrix; }
   CMatrix4 const &  GetProjectionTimesViewMatrix() const { return mProjectionTimesViewMatrix; }

   CDynamicVertexBufferPool_RT* GetVertexBufferPool_RT() const { return mpDynamicVertexBufferPool_RT.get(); }
   CDynamicIndexBufferPool_RT* GetIndexBufferPool_RT() const { return mpDynamicIndexBufferPool_RT.get(); }
   CDynamicVertexBufferPool_UT* GetVertexBufferPool_UT() const { return (CDynamicVertexBufferPool_UT*)mpDynamicVertexBufferPool_RT.get(); }
   CDynamicIndexBufferPool_UT* GetIndexBufferPool_UT() const { return (CDynamicIndexBufferPool_UT*)mpDynamicIndexBufferPool_RT.get(); }

   void AcquireRenderThreadOwnership() {}
   void ReleaseRenderThreadOwnership() {}

private:
   friend class CBaseRenderBackend;

private:
   void InitDefaultState();
   
   void InvalidateObjects();
   void RestoreObjects();

   real32 const GetFullScreenAspectRatio();
   real32 const GetClientWindowAspectRatio();

   void FlushProjectionTimesViewMatrix();

private:
   bool                       mHostedInWindow;

   D3DPRESENT_PARAMETERS      mPresentParameters;

   TComPtr<IDirect3D9>        mpDirect3D;
   TComPtr<IDirect3DDevice9>  mpDirect3DDevice;
   TComPtr<IDirect3DSurface9> mpBackBuffer;

   boost::scoped_ptr<CD3DCachedDevice> mCachedDeviceWrapper;
   boost::scoped_ptr<CDynamicVertexBufferPool_RT> mpDynamicVertexBufferPool_RT;
   boost::scoped_ptr<CDynamicIndexBufferPool_RT> mpDynamicIndexBufferPool_RT;

   SRenderTarget              mCurrentRenderTarget;
   bool                       mFirstFrame;

   TDeclarationMap            mDeclarations;
   uint64                     mBoundVertexDataHash_1;
   uint64                     mBoundVertexDataHash_2;
   int                        mBoundIndexBufferOffset;

   bool                       mFogEnabled;
   EFogMode                   mFogMode;
   CColorf                    mFogColor;
   CVector2                   mFogParams;

   CMatrix4                   mCameraMatrix;
   CMatrix4                   mViewMatrix;
   CMatrix4                   mProjectionTimesViewMatrix;
};

//------------------------------------------------------------------------------------------
