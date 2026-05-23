//------------------------------------------------------------------------------------------
// Win32CRenderBackend.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include <xgraphics.h>

#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CVertexArray.h"

#include "Engine/Mechanics/TComPtr.h"

#include "Engine/Math/CMatrix4.h"
#include "Engine/Math/CVector2.h"

#include "Engine/Primitives/CAABox.h"

#include "Renderer/Base/Backend/X360/CD3DCachedDevice.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Frontend/RenderTypes.h"

//#include <string>

//------------------------------------------------------------------------------------------

class CBaseTexture;
class CTexture;
class CMeshHandle;
class CMatrix4;
class CMeshChunk;
class IResourcePool;
class IEvaluatorFactory;
class CDynamicVertexBufferPool_RT;
class CDynamicIndexBufferPool_RT;
class CDynamicIndexBufferPoolChunk_RT;

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

   enum EPresentationInterval
   {
      kPI_One = D3DPRESENT_INTERVAL_ONE,
      kPI_Two = D3DPRESENT_INTERVAL_TWO,
      kPI_Three = D3DPRESENT_INTERVAL_THREE,
      kPI_Immediate = D3DPRESENT_INTERVAL_IMMEDIATE,
   };

   enum EBlendFunc
   {
      kBF_Zero = D3DBLEND_ZERO,

      kBF_One = D3DBLEND_ONE,

      kBF_SrcColor = D3DBLEND_SRCCOLOR,
      kBF_InvSrcColor = D3DBLEND_INVSRCCOLOR,

      kBF_DstColor = D3DBLEND_DESTCOLOR,
      kBF_InvDstColor = D3DBLEND_INVDESTCOLOR,

      kBF_SrcAlpha = D3DBLEND_SRCALPHA,
      kBF_InvSrcAlpha = D3DBLEND_INVSRCALPHA,

      kBF_DstAlpha = D3DBLEND_DESTALPHA,
      kBF_InvDstAlpha = D3DBLEND_INVDESTALPHA,

      kBF_FixedAlpha = D3DBLEND_BLENDFACTOR,//TODO: 360 extensions allow for this on just alpha I believe
      kBF_InvFixedAlpha = D3DBLEND_INVBLENDFACTOR,//TODO: 360 extensions allow for this on just alpha I believe

      kBF_FixedColor = D3DBLEND_BLENDFACTOR,
      kBF_InvFixedColor = D3DBLEND_INVBLENDFACTOR,
   };

   enum EBlendOp
   {
      kBO_Add = D3DBLENDOP_ADD,
      kBO_Subtract = D3DBLENDOP_SUBTRACT,
      kBO_RevSubtract = D3DBLENDOP_REVSUBTRACT,
      kBO_Min = D3DBLENDOP_MIN,
      kBO_Max = D3DBLENDOP_MAX,
   };

   enum EAlphaFunc
   {
      kAF_Never = D3DCMP_NEVER,
      kAF_Less = D3DCMP_LESS,
      kAF_Equal = D3DCMP_EQUAL,
      kAF_LEqual = D3DCMP_LESSEQUAL,
      kAF_Greater = D3DCMP_GREATER,
      kAF_NotEqual = D3DCMP_NOTEQUAL,
      kAF_GEqual = D3DCMP_GREATEREQUAL,
      kAF_Always = D3DCMP_ALWAYS,
   };

#if BPE_INVERT_ZBUFFER
   enum EDepthFunc
   {
      kDF_Never = D3DCMP_NEVER,
      kDF_Less = D3DCMP_GREATER,
      kDF_Equal = D3DCMP_EQUAL,
      kDF_LEqual = D3DCMP_GREATEREQUAL,
      kDF_Greater = D3DCMP_LESS,
      kDF_NotEqual = D3DCMP_NOTEQUAL,
      kDF_GEqual = D3DCMP_LESSEQUAL,
      kDF_Always = D3DCMP_ALWAYS,
   };
#else
   enum EDepthFunc
   {
      kDF_Never = D3DCMP_NEVER,
      kDF_Less = D3DCMP_LESS,
      kDF_Equal = D3DCMP_EQUAL,
      kDF_LEqual = D3DCMP_LESSEQUAL,
      kDF_Greater = D3DCMP_GREATER,
      kDF_NotEqual = D3DCMP_NOTEQUAL,
      kDF_GEqual = D3DCMP_GREATEREQUAL,
      kDF_Always = D3DCMP_ALWAYS,
   };
#endif

   enum EStencilOp
   {
      kSO_Keep = D3DSTENCILOP_KEEP,
      kSO_Zero = D3DSTENCILOP_ZERO,
      kSO_Replace = D3DSTENCILOP_REPLACE,
      kSO_Incr = D3DSTENCILOP_INCRSAT,
      kSO_Decr = D3DSTENCILOP_DECRSAT,
      kSO_IncrWrap = D3DSTENCILOP_INCR,
      kSO_DecrWrap = D3DSTENCILOP_DECR,
      kSO_Invert = D3DSTENCILOP_INVERT,
   };

   enum EWrapMode
   {
      kWM_Wrap = D3DTADDRESS_WRAP,
      kWM_Mirror = D3DTADDRESS_MIRROR,
      kWM_Clamp = D3DTADDRESS_CLAMP,
      kWM_Border = D3DTADDRESS_BORDER,
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
      kCM_None = D3DCULL_NONE,
      kCM_CW = D3DCULL_CW,
      kCM_CCW = D3DCULL_CCW
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

   void SuspendGameRenderThread();
   void ResumeGameRenderThread();

   void AcquireRenderThreadOwnership();
   void ReleaseRenderThreadOwnership();

   // Win32 version is never stereo 3d
   bool IsStereo3D() const { return false; }

   void RestoreRenderTargetColor(CBaseTexture* pRenderTexture, CBaseTexture* pSrcTexture, SRect const & RTRect, SRect const & srcRect);
   void RestoreRenderTargetColor_Internal(CBaseTexture* pRenderTexture, CBaseTexture* pSrcTexture, SRectf const & RTRectf, SRectf const & srcRectf);
   
   //void ResolveRenderTargetQuarter(CBaseTexture * pRenderTextureHalf, CBaseTexture * pFakeMSAARenderTexture);
   void ResolveRenderTargetSixteenth(CBaseTexture * pRenderTextureHalf, CBaseTexture * pFakeMSAARenderTextureHalf, CBaseTexture * pRenderTextureQuarter);

   void ResolvePartialRenderTarget(CBaseTexture * pRenderTexture, SRect const & srcRect);
   void ResolvePartialRenderTarget_Unsafe(CBaseTexture * pRenderTexture, SRect const * pSrcRect);
   void ResolveRenderTarget(CBaseTexture * pRenderTexture);
   void ResolveRenderTargetPredicated(CBaseTexture * pRenderTexture);
   void ResolveDepthStencilPredicated(CBaseTexture * pRenderTexture);
   void ResolveDepthStencilPredicatedMSAA2(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1);
   void ResolveDepthStencilPredicatedMSAA4(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1, CBaseTexture * pRenderTexture2, CBaseTexture * pRenderTexture3);
   void SetRenderTarget(SRenderTarget const & renderTarget);

   SRenderTarget const & GetCurrentRenderTarget() const { return mCurrentRenderTarget; }

   CBaseTexture * GetCurrentRenderTexture() const { return mCurrentRenderTexture; }
   CBaseTexture * GetCurrentRenderDepth() const { return mCurrentDepthTexture; }

   int GetPresentationInterval() const { return mPresentationInterval; }
   void SetPresentationInterval(EPresentationInterval presentationInterval);
   void SetPresentationIntervalNum(int presentationInterval);

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

   void SetGPRAllocation( int numVertexGPR );

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

   // get direct3DDevice
   TComPtr<IDirect3DDevice9> & GetDirect3DDevice() { return mpDirect3DDevice; }
   CD3DCachedDevice * GetCachedDeviceWrapper() { return mCachedDeviceWrapper.get(); }

   CMatrix4 const &  GetViewMatrix() const { return mViewMatrix; }
   CMatrix4 const &  GetCameraMatrix() const { return mCameraMatrix; }
   CMatrix4 const &  GetProjectionTimesViewMatrix() const { return mProjectionTimesViewMatrix; }

   CDynamicVertexBufferPool_RT* GetVertexBufferPool_RT() const { return mpDynamicVertexBufferPool_RT.get(); }
   CDynamicIndexBufferPool_RT* GetIndexBufferPool_RT() const { return mpDynamicIndexBufferPool_RT.get(); }
   CDynamicVertexBufferPool_UT* GetVertexBufferPool_UT() const { return mpDynamicVertexBufferPool_UT.get(); }
   CDynamicIndexBufferPool_UT* GetIndexBufferPool_UT() const { return mpDynamicIndexBufferPool_UT.get(); }

   D3DRECT const * GetTilingRects() const { return mpTilingRects; }

   void RegisterDynamicVertexBuffer_RT(CDynamicVertexBuffer_RT* pObj);
   void RemoveDynamicVertexBuffer_RT(CDynamicVertexBuffer_RT* pObj);
   void RegisterDynamicIndexBuffer_RT(CDynamicIndexBuffer_RT* pObj);
   void RemoveDynamicIndexBuffer_RT(CDynamicIndexBuffer_RT* pObj);

   uint32 GetVBLCount() const { return mVBLCount; }

private:
   friend class CBaseRenderBackend;

private:
   void InitDefaultState();

   void InvalidateObjects();
   void RestoreObjects();

   void FlushProjectionTimesViewMatrix();

   void ResolveDepthStencilPredicated_Internal(CBaseTexture * pRenderTexture, int additionalFlags);
   void FixBeginTilingViewportBug_Internal(SRenderTarget const & renderTarget);

   void BeginFrameResourceTick();

   //Threading support
   //----------------------------------------------------------------------------
public:
   typedef void (*TFnRenderThread)(int currentRenderBuffer);
   
   static void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName); // TODO: should really find a better place for this?

   void StartThreadSystem(TFnRenderThread nextThread);
   void RunThreadFunction(int nextParam);
   void WaitThreadFunction();
   static void VBlankCallback(D3DVBLANKDATA *pData);

   bool TryWaitThreadFunction();

private:
   static DWORD __stdcall RenderThread( LPVOID InThis );

private:

   D3DPRESENT_PARAMETERS      mPresentParameters;
   XVIDEO_MODE                mUserVideoMode;
   D3DVIDEO_SCALER_PARAMETERS mVideoScalerParms;

   TComPtr<IDirect3D9>        mpDirect3D;
   TComPtr<IDirect3DDevice9>  mpDirect3DDevice;
   int                        mBackBufferIndex;
   CTexture*                  mpCurrentBackBufferTexture;

   CTexture*                  mpBackBufferTexture[3];
   CTexture*                  mpBackBufferDepthTexture;

   boost::scoped_ptr<CD3DCachedDevice> mCachedDeviceWrapper;
   boost::scoped_ptr<CDynamicVertexBufferPool_RT> mpDynamicVertexBufferPool_RT;
   boost::scoped_ptr<CDynamicIndexBufferPool_RT> mpDynamicIndexBufferPool_RT;
   boost::scoped_ptr<CDynamicVertexBufferPool_UT> mpDynamicVertexBufferPool_UT;
   boost::scoped_ptr<CDynamicIndexBufferPool_UT> mpDynamicIndexBufferPool_UT;
   std::vector<IDirect3DResource9 *> mArrayToRelease0;
   std::vector<IDirect3DResource9 *> mArrayToRelease1;
   std::vector<IDirect3DResource9 *> mArrayToRelease2;

   std::vector<CDynamicVertexBuffer_RT*> mArrayDynamicVertexBuffer_RT;
   std::vector<CDynamicIndexBuffer_RT*> mArrayDynamicIndexBuffer_RT;

   SRenderTarget              mCurrentRenderTarget;
   CBaseTexture *             mCurrentRenderTexture;
   CBaseTexture *             mCurrentDepthTexture;
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

   bool mIsTiling;
   int mTilingRectCount;
   D3DRECT mpTilingRects[8];

   TComPtr<IDirect3DVertexShader9> mDefaultVertexShader;
   TComPtr<IDirect3DPixelShader9> mDefaultPixelShader;

   CStopWatch  mFrameTimer;
   volatile uint32      mVBLCount;

   //Threading support
   //----------------------------------------------------------------------------
   TFnRenderThread   mpRenderThreadFunction;
   int               mRenderThreadFunctionParameter;
   HANDLE            mRT_ThreadDieEvent;
   HANDLE            mRT_DoneEvent;
   HANDLE            mRT_GoEvent;

   DWORD             mRT_ThreadID;
   HANDLE            mRT_Thread;

   CSyncCriticalSection mD3DThreadOwnerShip;

public:
   int mPresentationInterval;
   int mPresentationThreshold;
   int mAsyncSwaps;
   int mTrilinearThreshold;
   int mVideoScalerMode; // 0 default 1 gausian
   int mVertexShaderGPRCount;
};

//----------------------------------------------------------------------------

inline void CGPUProfileMarker::PushMarker(const char * pName)
{
#if USE_GPU_PROFILE_MARKERS
   PIXBeginNamedEvent(0xFFFFFFFF, pName);
#endif
}

//----------------------------------------------------------------------------

inline void CGPUProfileMarker::PopMarker()
{
#if USE_GPU_PROFILE_MARKERS
   PIXEndNamedEvent();
#endif
}

//----------------------------------------------------------------------------

inline void CGPUProfileMarker::Event(const char * pName)
{
#if USE_GPU_PROFILE_MARKERS
   PIXSetMarker(0xFFFFFFFF, pName);
#endif
}

//----------------------------------------------------------------------------


inline void CRenderBackend::RegisterDynamicVertexBuffer_RT(CDynamicVertexBuffer_RT* pObj)
{
   RegisterWithArray(pObj, &mArrayDynamicVertexBuffer_RT);
}

inline void CRenderBackend::RemoveDynamicVertexBuffer_RT(CDynamicVertexBuffer_RT* pObj)
{
   RemoveFromArray(pObj, &mArrayDynamicVertexBuffer_RT);
}

inline void CRenderBackend::RegisterDynamicIndexBuffer_RT(CDynamicIndexBuffer_RT* pObj)
{
   RegisterWithArray(pObj, &mArrayDynamicIndexBuffer_RT);
}

inline void CRenderBackend::RemoveDynamicIndexBuffer_RT(CDynamicIndexBuffer_RT* pObj)
{
   RemoveFromArray(pObj, &mArrayDynamicIndexBuffer_RT);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::BeginTiling(SRenderTarget const & renderTarget)
{
#if BPE_USE_PREDICATED_TILING
   if( !mIsTiling )
   {
      mpDirect3DDevice->BeginTiling(D3DTILING_SKIP_FIRST_TILE_CLEAR|D3DTILING_CPU5, mTilingRectCount, mpTilingRects, NULL, 1.0f, 0);
      mIsTiling = true;
      FixBeginTilingViewportBug_Internal(renderTarget);
   }
#endif
}

//----------------------------------------------------------------------------

inline void CRenderBackend::EndTiling()
{
#if BPE_USE_PREDICATED_TILING
   if( mIsTiling )
   {
      mpDirect3DDevice->EndTiling(0, NULL, NULL, NULL, 1.0f, 0, NULL);
      mIsTiling = false;;
   }
#endif
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

inline void CRenderBackend::SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV)
{
   GetCachedDeviceWrapper()->SetSamplerState(texUnit, D3DSAMP_ADDRESSU, wrapU);
   GetCachedDeviceWrapper()->SetSamplerState(texUnit, D3DSAMP_ADDRESSV, wrapV);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector)
{
   GetCachedDeviceWrapper()->SetVertexShaderConstantF(startRegister, (real32 const*)pVector, numVectors);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetIndexData(CIndexBuffer const * indexBuffer)
{
   mBoundIndexBufferOffset = 0;
   GetCachedDeviceWrapper()->SetIndices(indexBuffer->mpCurrBuffer);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetIndexData(CDynamicIndexBufferPoolChunk_RT const * pChunk)
{
   CDynamicIndexBufferPool_RT const * pPool = pChunk->GetPool();
   mBoundIndexBufferOffset = pChunk->GetStartIndex();
   GetCachedDeviceWrapper()->SetIndices(pPool->GetIndexBuffer()->mpCurrBuffer);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetIndexData(CDynamicIndexBufferPoolChunk_UT const * pChunk)
{
   CDynamicIndexBufferPool_UT const * pPool = pChunk->GetPool();
   mBoundIndexBufferOffset = pChunk->GetStartIndex();
   GetCachedDeviceWrapper()->SetIndices(pPool->GetIndexBuffer()->mpCurrBuffer);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::ForceVertexDataRebind()
{
   mBoundVertexDataHash_1 = 0xDEADBEEF;
   mBoundVertexDataHash_2 = 0xFEE7C0DE;
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetCullMode(ECullMode const mode)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_CULLMODE, mode);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetPresentationInterval(EPresentationInterval presentationInterval)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_PRESENTINTERVAL, presentationInterval);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetPresentationIntervalNum(int presentationInterval)
{
   switch( presentationInterval )
   {
   case 1:
      SetPresentationInterval(kPI_One);
      break;
   case 2:
      SetPresentationInterval(kPI_Two);
      break;
   case 3:
      SetPresentationInterval(kPI_Three);
      break;
   }
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetShadeMode(int isSmooth)
{
#if 0 //X360 does not support this!!!
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_SHADEMODE, isSmooth ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
#endif
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetAlphaTestEnable(bool enable)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHATESTENABLE, enable);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetAlphaFunc(EAlphaFunc func, int refValue)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHAFUNC, func);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHAREF, refValue);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetStencilEnable(bool enable)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILENABLE, enable);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBackStencilEnable(bool enable)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, enable);
}

inline void CRenderBackend::SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass )
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILFAIL, fail);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILZFAIL, depthFail);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILPASS, depthPass);
}

inline void CRenderBackend::SetBackStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass )
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_CCW_STENCILFAIL, fail);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_CCW_STENCILZFAIL, depthFail);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_CCW_STENCILPASS, depthPass);
}

inline void CRenderBackend::SetStencilMask(uint32 mask)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILWRITEMASK, mask);
}

inline void CRenderBackend::SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILFUNC, func);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILREF, ref);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILMASK, mask);
}

inline void CRenderBackend::SetBackStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_CCW_STENCILFUNC, func);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILREF, ref);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_STENCILMASK, mask);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::RenderPrimitives(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount, uint32 const indexBufferOffset, uint32 const indexCount)
{
   ++mMetrics.mNumRenderPrimitiveCalls;

   GetCachedDeviceWrapper()->DrawIndexedPrimitive(gD3DPrimitiveMapping[type],
      0,
      vertexBufferOffset, 
      vertexCount,  
      mBoundIndexBufferOffset + indexBufferOffset, 
      CMeshChunk::GetPrimitiveCount(type, indexCount) );
}

//----------------------------------------------------------------------------

inline void CRenderBackend::RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount)
{
   ++mMetrics.mNumRenderPrimitiveCalls;

   GetCachedDeviceWrapper()->DrawPrimitive(gD3DPrimitiveMapping[type], vertexBufferOffset, CMeshChunk::GetPrimitiveCount(type, vertexCount));
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBlendOp(EBlendOp const blendOp)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_BLENDOP, blendOp);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_BLENDOPALPHA, blendOp);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_BLENDOP, colorBlendOp);
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_BLENDOPALPHA, alphaBlendOp);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetGPRAllocation( int numVertexGPR )
{
   mpDirect3DDevice->SetShaderGPRAllocation(0, numVertexGPR, 128-numVertexGPR);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetDepthCompareEnabled(bool const enable)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_ZENABLE, enable ? TRUE : FALSE);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetDepthFunc(EDepthFunc const func)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_ZFUNC, func);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetDepthWriteEnabled(bool const enable)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_ZWRITEENABLE, enable ? TRUE : FALSE);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetLineWidth(real32 const width)
{
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_LINEWIDTH, *((DWORD*)&width));
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetLineSmoothEnabled(bool const enable)
{
   // Not supported on 360
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color )
{
   mFogColor = color;
   mFogMode = fogMode;

   float fogScale = 1.0f/(farZ - nearZ);
   float fogParam0 = farZ * fogScale + 1.0f;
   float fogParam1 = -fogScale;

   mFogParams = CVector2( fogParam0, fogParam1 );
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetColorAlphaWriteEnabled(bool const color, bool const alpha)
{
   DWORD mask = 0;
   if( color )
   {
      mask |= D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
   }
   if( alpha )
   {
      mask |= D3DCOLORWRITEENABLE_ALPHA;
   }
   GetCachedDeviceWrapper()->SetRenderState(D3DRS_COLORWRITEENABLE, mask);
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode)
{
   if( enable )
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);

      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SRCBLEND, srcBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_DESTBLEND, dstBlendMode);
   }
   else
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode)
{
   if( enable )
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SRCBLEND, rgbSrcBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_DESTBLEND, rgbDstBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SRCBLENDALPHA, aSrcBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_DESTBLENDALPHA, aDstBlendMode);
   }
   else
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode, const uint32 rgbaFixed)
{
   if( enable )
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);

      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SRCBLEND, srcBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_DESTBLEND, dstBlendMode);

      GetCachedDeviceWrapper()->SetRenderState(D3DRS_BLENDFACTOR, rgbaFixed);
   }
   else
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode, const uint32 rgbaFixed)
{
   if( enable )
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SRCBLEND, rgbSrcBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_DESTBLEND, rgbDstBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SRCBLENDALPHA, aSrcBlendMode);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_DESTBLENDALPHA, aDstBlendMode);

      GetCachedDeviceWrapper()->SetRenderState(D3DRS_BLENDFACTOR, rgbaFixed);
   }
   else
   {
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      GetCachedDeviceWrapper()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}
