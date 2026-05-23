//----------------------------------------------------------------------------
// CRenderBackend.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "boost/scoped_ptr.hpp"

#include "Engine/Resource/IResourcePool.h"
#include "Engine/Graphics/CColorf.h"
#include "Engine/Graphics/CMatrixStack.h"
#include "Engine/System/CGuid.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/System/CSyncCriticalSection.h"

#include "Renderer/Base/Primitive/CMeshChunk.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"

//----------------------------------------------------------------------------

class CCompiledShaderCache;
class CShaderManager;
class CShaderObjectFactory;
class IEvaluatorFactory;
class IGameObjectFactory;
class IResourcePool;
class CMTXFont;
class CDrawableRenderEntity;
class CIndexBuffer;
class CVertexData;

class CDynamicVertexBuffer;
class CDynamicIndexBuffer;

//----------------------------------------------------------------------------

#define BPE_USE_EDGE_GEOM      0

#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_VITA
   #define ENABLE_SEPERATE_RENDER_THREAD 1
#else
   // Dreamcast: single render thread for initial port; KOS threading available if needed later.
   #define ENABLE_SEPERATE_RENDER_THREAD 0
#endif


//----------------------------------------------------------------------------

#if (!GOLD_VERSION && BPE_TARGET != BPE_TARGET_PS3 && BPE_TARGET != BPE_TARGET_DREAMCAST)
#define USE_GPU_PROFILE_MARKERS 1
#endif

#if BPE_TARGET == BPE_TARGET_X360
#define BPE_USE_FLOATINGPOINT_ZBUFFER 1
#define BPE_USE_PREDICATED_TILING 1
#endif

#if BPE_USE_FLOATINGPOINT_ZBUFFER
#define BPE_INVERT_ZBUFFER 0
#else
#define BPE_INVERT_ZBUFFER 0
#endif

#if BPE_INVERT_ZBUFFER
#define BPE_VIEWPORT_MINZ		1.0f
#define BPE_VIEWPORT_MAXZ		0.0f
#define BPE_BIAS_MULTIPLIER    -1.0f
#else
#define BPE_VIEWPORT_MINZ		0.0f
#define BPE_VIEWPORT_MAXZ		1.0f
#define BPE_BIAS_MULTIPLIER    1.0f
#endif

class RENDERER_API CGPUProfileMarker
{
public:
   CGPUProfileMarker(const char * pName) { PushMarker(pName); }
   ~CGPUProfileMarker() { PopMarker(); }

   static void PushMarker(const char* pName);
   static void PopMarker();

   static void Event(const char* pName);
};

#ifdef USE_GPU_PROFILE_MARKERS
   #define BPE_ADD_SCOPED_GPU_PROFILE_MARKER(DESC) CGPUProfileMarker gpuMarker(DESC)
   #define BPE_GPU_PUSH_PROFILE_MARKER(DESC) CGPUProfileMarker::PushMarker(DESC)
   #define BPE_GPU_POP_PROFILE_MARKER() CGPUProfileMarker::PopMarker()
   #define BPE_GPU_EVENT(DESC) CGPUProfileMarker::Event(DESC)
#else
   #define BPE_ADD_SCOPED_GPU_PROFILE_MARKER(DESC)
   #define BPE_GPU_PUSH_PROFILE_MARKER(DESC)
   #define BPE_GPU_POP_PROFILE_MARKER()
   #define BPE_GPU_EVENT(DESC)
#endif

//----------------------------------------------------------------------------
// Basic struct for holding info about the framebuffer output state.
// We'll keep this simple for now, later we can make this a bit fancier as
// we determine our requirements.
struct RENDERER_API SScreenOutputControl
{
   enum EScreenOutputMode
   {
      kOM_BlastFactor,
      kOM_BlastFactor_NoToneMapping,
      kOM_Unprocessed,
      kOM_JKE,

      kOM_Count
   };

   SScreenOutputControl();

   void                                         ResetToDefaultState();  // Reset every frame
   
   void                                         SetOutputMode(EScreenOutputMode const mode);
   EScreenOutputMode                            GetOutputMode() const { return mOutputMode; }

   EShaderColorSpace                            GetShaderColorSpace() const { return mShaderColorSpace; }

   EShaderFramebufferFormat                     GetFrameBufferFormat() const { return mFrameBufferFormat; }

   void                                         SetPostProcessShader(boost::shared_ptr<CShader> const & shader);
   boost::shared_ptr<CShader> const &           GetPostProcessShader() const;

   // Output modulate color is in gamma space.
   // Shader setup does any required conversions.
   CColorf                             mOutputModulateColor;

private:
   EScreenOutputMode                   mOutputMode;
   EShaderColorSpace                   mShaderColorSpace;
   EShaderFramebufferFormat            mFrameBufferFormat;
   boost::shared_ptr<CShader>          mPostProcessShader;
};

//----------------------------------------------------------------------------

static real32 const skGamma = 2.0f;

enum ERenderFonts
{
   kRenderFonts_Console,
   kRenderFonts_Count
};

//----------------------------------------------------------------------------

struct SRenderBackendMetrics
{
   SRenderBackendMetrics()
   {
      // Reset all values
      // Don't turn this into a class!
      memset(this, 0, sizeof(SRenderBackendMetrics));
   }

   int   mNumParametersSet;
   int   mNumTexturesSet;
   int   mNumRenderPrimitiveCalls;

   // Time spend waiting for VBlank (time we can fill with work)
   real32   mVBlankWaitTime;
   real32   mTimeSinceLastFlip;

   int   mParameterBufferRingBufferPeakSize;
   int   mParameterBufferRingBufferStallUS;

   int   mFragmentProgramRingBufferPeakSize;
   int   mFragmentProgramRingBufferStallUS;
};

//----------------------------------------------------------------------------

struct SRenderTarget
{
   enum EBackBufferMode
   {
      kBBM_NoBackBuffer,
      kBBM_BackBufferFill,
      kBBM_BackBufferCentered
   };
   
   enum EStereoViewportSelection
   {
      kSVS_None,
      kSVS_Left,
      kSVS_Right,
      kSVS_GapArea
   };

   SRenderTarget()
   :  mpDepthBuffer( NULL )
   ,  mMinZ( BPE_VIEWPORT_MINZ )
   ,  mMaxZ( BPE_VIEWPORT_MAXZ )
   ,  mBindBackBuffer(kBBM_BackBufferFill)
   ,  mStereoViewport(kSVS_None)
   ,  mScissorEnabled(0)
   ,  mPixelCentersHalf(0)
   ,  mForceAAMode(CBaseTexture::kAA_Invalid)
   {
      mpColorBuffer[0] = NULL;
      mpColorBuffer[1] = NULL;
      mpColorBuffer[2] = NULL;
      mpColorBuffer[3] = NULL;
      mpDepthBuffer = NULL;
   }

   SRenderTarget(
      CBaseTexture * pColor0, 
      CBaseTexture * pColor1, 
      CBaseTexture * pColor2, 
      CBaseTexture * pColor3, 
      CBaseTexture * pDepth,
      EBackBufferMode bindBackBuffer = kBBM_NoBackBuffer,
      EStereoViewportSelection stereoViewport = kSVS_None)
   :  mpDepthBuffer(pDepth)
   ,  mMinZ( BPE_VIEWPORT_MINZ )
   ,  mMaxZ( BPE_VIEWPORT_MAXZ )
   ,  mBindBackBuffer(bindBackBuffer)
   ,  mStereoViewport(stereoViewport)
   ,  mScissorEnabled(0)
   ,  mPixelCentersHalf(0)
   ,  mForceAAMode(CBaseTexture::kAA_Invalid)
   {
      mpColorBuffer[0] = pColor0;
      mpColorBuffer[1] = pColor1;
      mpColorBuffer[2] = pColor2;
      mpColorBuffer[3] = pColor3;
   }

   bool const operator == (SRenderTarget const & rhs) const
   {
      return mpColorBuffer[0] == rhs.mpColorBuffer[0] &&
             mpColorBuffer[1] == rhs.mpColorBuffer[1] &&
             mpColorBuffer[2] == rhs.mpColorBuffer[2] &&
             mpColorBuffer[3] == rhs.mpColorBuffer[3] &&
             mpDepthBuffer == rhs.mpDepthBuffer &&
             mMinZ == rhs.mMinZ &&
             mMaxZ == rhs.mMaxZ &&
             mBindBackBuffer == rhs.mBindBackBuffer && 
             mStereoViewport == rhs.mStereoViewport &&
             mScissorEnabled == rhs.mScissorEnabled && 
             mScissorX == rhs.mScissorX && 
             mScissorY == rhs.mScissorY && 
             mScissorWidth == rhs.mScissorWidth && 
             mScissorHeight == rhs.mScissorHeight &&
             mPixelCentersHalf == rhs.mPixelCentersHalf &&
             mRenderTargetViewPort.mViewportEnabled == rhs.mRenderTargetViewPort.mViewportEnabled &&
             mRenderTargetViewPort.mViewportX == rhs.mRenderTargetViewPort.mViewportX && 
             mRenderTargetViewPort.mViewportY == rhs.mRenderTargetViewPort.mViewportY && 
             mRenderTargetViewPort.mViewportWidth == rhs.mRenderTargetViewPort.mViewportWidth && 
             mRenderTargetViewPort.mViewportHeight == rhs.mRenderTargetViewPort.mViewportHeight;
   }

   CBaseTexture*                 mpColorBuffer[4];
   CBaseTexture*                 mpDepthBuffer;
   real32                        mMinZ;
   real32                        mMaxZ;
   EBackBufferMode               mBindBackBuffer;
   EStereoViewportSelection      mStereoViewport;
   
   int                           mScissorEnabled;
   int                           mScissorX, mScissorY, mScissorWidth, mScissorHeight;
   
   int                           mPixelCentersHalf;

   struct SRenderTargetViewPort
   {
      int                        mViewportEnabled;
      int                        mViewportX, mViewportY, mViewportWidth, mViewportHeight;

      SRenderTargetViewPort()
      : mViewportEnabled(0)
      {
      }
   };
   SRenderTargetViewPort         mRenderTargetViewPort;

   CBaseTexture::EAntiAliasType  mForceAAMode;
};

//----------------------------------------------------------------------------

class RENDERER_API CBaseRenderBackend
{
   friend class CRenderBackend;
public:
   enum EClearFlags
   {
      kFlag_Color = 1,
      kFlag_Depth = 2,
      kFlag_Stencil = 4
   };

   enum ECreationFlags
   {
      kCF_None                      = 0,
      kCF_Fullscreen                = 1 << 0,
      kCF_DontStoreWindowPosition   = 1 << 1,
      kCF_WaitForVSync              = 1 << 2,   // This is the default when fullscreen

      kCF_Count
   };

   enum EFogMode
   {
      kFGM_Linear,
      kFGM_Exp,
      kFGM_Exp2,

      kFGM_Count
   };

   enum EStereoMode
   {
      kSM_Monoscopic,
      kSM_Stereoscopic
   };

   enum EInitializeDisplayResult
   {
      kIDR_Failed,
      kIDR_Succeded,
      kIDR_AlreadyInSameMode
   };

   struct SRenderFrameBufferSize
   {
      enum EDisplayAspect
      {
         kDA_Fullscreen,
         kDA_Widescreen
      };
      
      enum ESizeType
      {
         kST_Display,
         kST_FrameBuffer,
         kST_AltFrameBuffer,

         kST_Count
      };

      SRenderFrameBufferSize(EDisplayAspect const displayAspect, 
                             int const displayWidth, int const displayHeight, 
                             int const frameBufferWidth, int const frameBufferHeight, 
                             int const altFrameBufferWidth, int const altFrameBufferHeight,
                             real32 const aspect)
      :  mDisplayAspect(displayAspect)
      ,  mAspect(aspect)
      {
         mWidth[kST_Display] = displayWidth;
         mHeight[kST_Display] = displayHeight;

         mWidth[kST_FrameBuffer] = frameBufferWidth;
         mHeight[kST_FrameBuffer] = frameBufferHeight;

         mWidth[kST_AltFrameBuffer] = altFrameBufferWidth;
         mHeight[kST_AltFrameBuffer] = altFrameBufferHeight;
      }

      EDisplayAspect GetDisplayAspect() const { return mDisplayAspect; }
      real32 GetAspect() const { return mAspect; }
      int GetWidth(ESizeType const type) const { return mWidth[type]; }
      int GetHeight(ESizeType const type) const { return mHeight[type]; }

   private:
      EDisplayAspect mDisplayAspect;
      real32         mAspect;

      int            mWidth[kST_Count];
      int            mHeight[kST_Count];
   };

   struct RENDERER_API SRenderInitialization
   {
      SRenderInitialization();

      SRenderFrameBufferSize* mpFrameBufferSize;
      int                     mFrameBufferSizeCount;
      uint32                  mCreationFlags;
      int                     mIndexBufferPoolSize;
      int                     mIndexBufferPoolChunkCount;
      int                     mVertexBufferPoolSize;
      int                     mVertexBufferPoolChunkCount;
      BPE_HANDLE              mWindowHandle;
   };
   
public:
   ~CBaseRenderBackend();

   // this function returns whether we're currently in stereo 3d mode.
   bool IsStereo3D() const { return mIsStereo3D; }
   
   // use this function to query of the connected display supports s3d
   bool SupportsStereo3D() const;

   // will return false if couldn't be switched into this mode
   EInitializeDisplayResult InitializeDisplay(EStereoMode const stereoMode);

   // Returns size of display in inches
   real32 GetDisplaySize() const;

   CDrawableRenderEntity* GetFakeRenderEntity() const { return mFakeRenderEntity.get(); }

   IResourcePool * const ResourcePool() { return mpResourcePool; }
   CCompiledShaderCache * ShaderCache() { return mShaderCache.get(); }
   
   real64 const   GetUniverseTime() const { return mUniverseTime; }
   void           SetUniverseTime(real64 const universeTime)   { mUniverseTime = universeTime; }
   uint32 const   GetFrameCount() const { return mFrameCount; }

   IGameObjectFactory & ShaderObjectFactory();
   void AddMaterialPropertiesMapping(boost::shared_ptr<CShader> const & material);
   void RemoveMaterialPropertiesMapping(CShader const & material);
   bool ModifyComponentProperties(CGuid const &componentEditorId, void* pPropertiesData, uint32 const propertiesDataSize, IEvaluatorFactory const &factory);

   void              SetCameraMatrix(CMatrix34 const &matrix);

   CMatrix4 const &  GetProjectionMatrix() const { return mProjectionMatrix; }

   // Returns width of currently bound render target
   int const GetViewWidth() const { return mRenderTargetWidth; }
   // Returns height of currently bound render target
   int const GetViewHeight() const { return mRenderTargetHeight; }

   // Returns width of current viewport
   int const GetViewportWidth() const { return mViewportWidth; }
   // Returns height of currently bound render target
   int const GetViewportHeight() const { return mViewportHeight; }

   void SetViewportJitter(real32 const pixX, real32 const pixY)   { mViewportJitterX = pixX / mRenderTargetWidth; mViewportJitterY = pixY / mRenderTargetHeight; };

   int const GetBackBufferWidth() const { return GetFrameBufferSize().GetWidth(SRenderFrameBufferSize::kST_Display); }
   int const GetBackBufferHeight() const { return GetFrameBufferSize().GetHeight(SRenderFrameBufferSize::kST_Display); }

   int const GetMainFrameBufferWidth() const { return GetFrameBufferSize().GetWidth(mRenderTargetFrameBufferSizeType); }
   int const GetMainFrameBufferHeight() const { return GetFrameBufferSize().GetHeight(mRenderTargetFrameBufferSizeType); }

   void SetRenderTargetFrameBufferSizeType(SRenderFrameBufferSize::ESizeType const type) { mRenderTargetFrameBufferSizeType = type; }
   SRenderFrameBufferSize::ESizeType GetRenderTargetFrameBufferSizeType() const { return mRenderTargetFrameBufferSizeType; }

   SRenderFrameBufferSize const & GetFrameBufferSize() const { return mIsStereo3D ? mFrameBufferSizeStereo : mFrameBufferSizeMono; }

   real32 const GetScreenAspectRatio() const;
   real32 const GetScreenRefreshRate() const;
   bool        Is43Aspect() const;
   bool        IsLowResolution() const;      // Returns true if resolution is < 1280
   bool        IsInterlaced() const;         // Returns true if display is 480i/576i (PS3 only)

   void SetProjectionMatrix(CMatrix4 const & projection);

   void SetPerspectiveProjection(CAngle const & fov = CAngle::FromDegrees(55.0f), 
                                 real32 const aspect = 1.33f, 
                                 real32 const minClip = 0.1f, 
                                 real32 const maxClip = 1000.0f);

   // default will create orthographic projection whose dimensions equal the render target
   void SetOrthographicProjection(real32 const width = -1.0f,
                                  real32 const height = -1.0f,
                                  real32 const zNear = -1.0f,
                                  real32 const zFar = 1.0f);

   // Sets up all matrices for rendering in pixel coords with 0,0 being the top left, -1 < z < 1
   void SetScreenSpaceOrtho();

   // Sets up all matrices for rendering in 0 -> 1 coords with 0,0 being the top left.
   void SetUniformOrtho();

   CBaseTexture const & GetWhiteMap() const;
   CBaseTexture const & GetGreyMap() const;  // 0.5, 0.5, 0.5, 1.0
   CBaseTexture const & GetBlackMap() const;
   CBaseTexture const & GetFlatNormalMap() const;

   // this function will create a texture that can be used to render to it (use SetRenderTarget to activate it)
   CBaseTexture * CreateRenderTarget(int width, int height, CBaseTexture::EAntiAliasType const aaFlags, CBaseTexture::EFormat format, ERenderMemory const memory = kRM_Video);

   // this function will create a depth buffer that can be used to render to it (use SetRenderTarget to activate it)
   CBaseTexture * CreateDepthBuffer(int width, int height, CBaseTexture::EAntiAliasType const aaFlags, ERenderMemory const memory = kRM_Video );

   SScreenOutputControl & ScreenOutputControl() { return mScreenOutputControl; }
   SScreenOutputControl const & GetScreenOutputControl() const { return mScreenOutputControl; }

   CVector2 const GetTextExtents(std::string const & text, 
                                 ERenderFonts const font = kRenderFonts_Console) const;

   // render text string to x/y coordinate in pixels (0,0 - top/left)
   void RenderText(const char * const text, 
                   real32 const x, 
                   real32 const y,
                   ERenderFonts const font = kRenderFonts_Console,
                   CColor const & color = CColor::White(),
                   int const flags = 0);

   void RenderQuad(real32 const minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV);
   void RenderFullscreenQuad(real32 const z);

   real32 const GetFontHeight(ERenderFonts const font = kRenderFonts_Console) const;
   TLockedResource<CMTXFont> GetFont(ERenderFonts const font = kRenderFonts_Console) const;

   SRenderBackendMetrics & Metrics() { return mMetrics; }

   void DeleteRenderResourceFrameDelayed(IObject* pObject);

   void RegisterDynamicVertexBuffer(CDynamicVertexBuffer* pObj);
   void RemoveDynamicVertexBuffer(CDynamicVertexBuffer* pObj);
   void RegisterDynamicIndexBuffer(CDynamicIndexBuffer* pObj);
   void RemoveDynamicIndexBuffer(CDynamicIndexBuffer* pObj);

   static void SetDefaultFontFromMemory(const uint8 * CFONBuffer, uint32 const CFONBufferSize, const uint8 * TXTRBuffer, uint32 const TXTRBufferSize);

   void SetCurrentAreaDebugName(char *name) { strcpy(mCurrentAreaDebugName, name); }
   char *GetCurrentAreaDebugName() { return &mCurrentAreaDebugName[0]; }

protected:
   CBaseRenderBackend(IResourcePool & resourcePool,
                      SRenderInitialization const &initFlags);

   // This function is called before any draw calls are submitted. 
   // This function must be called before setting and clearing render targets.
   void InternalBeginScene();
   // This function is called after all rendering is done but before the present.
   // Swap back buffer.
   void InternalPresent();

   void InitDefaultObjects();

   void OnResourcesChanged();

   void UninitializeDisplay();

   void InternalBeginFrameResourceTick();

protected:
   typedef std::multimap< CGuid, boost::weak_ptr<CShader> > TComponentEditorIdToComponentMap;
   typedef std::pair< TComponentEditorIdToComponentMap::const_iterator, TComponentEditorIdToComponentMap::const_iterator > TComponentEditorIdToComponentMapConstIterPair;
   typedef std::pair< TComponentEditorIdToComponentMap::iterator, TComponentEditorIdToComponentMap::iterator > TComponentEditorIdToComponentMapIterPair;

protected:
   IResourcePool *            mpResourcePool;
   SRenderInitialization      mInitFlags;
   
   bool                       mIsStereo3D;

   SRenderFrameBufferSize     mFrameBufferSizeMono;
   SRenderFrameBufferSize     mFrameBufferSizeStereo;

   // Which size to use for the render target.
   SRenderFrameBufferSize::ESizeType mRenderTargetFrameBufferSizeType;

   real32                     mFrustumNear;
   real32                     mFrustumFar;

   int                        mRenderTargetWidth;
   int                        mRenderTargetHeight;
   int                        mViewportWidth;
   int                        mViewportHeight;
   
   bool                       mInterlaced;
   real32                     mRefreshRate;
   
   CMatrix4                   mProjectionMatrix;

   real32                     mViewportJitterX;
   real32                     mViewportJitterY;

   SScreenOutputControl       mScreenOutputControl;
   
   std::auto_ptr<CCompiledShaderCache>       mShaderCache;
   TComponentEditorIdToComponentMap          mComponentEditorIdToComponentMap;
   boost::scoped_ptr<CShaderObjectFactory>   mpShaderObjectFactory;

   CBaseTexture* mWhiteMap;
   CBaseTexture* mGreyMap;
   CBaseTexture* mBlackMap;
   CBaseTexture* mFlatNormalMap;

   // Default fonts
   bpe::reserved_vector<TLockedResource<CMTXFont>, kRenderFonts_Count>
                                             mFonts;

   SRenderBackendMetrics                     mMetrics;

   std::auto_ptr<CDrawableRenderEntity>      mFakeRenderEntity;

   std::vector<IObject*> mArrayToRelease_Object;
   std::vector<IObject*> mArrayToRelease_Object_0;
   std::vector<IObject*> mArrayToRelease_Object_1;
   std::vector<IObject*> mArrayToRelease_Object_2;

   std::vector<CDynamicVertexBuffer*> mArrayDynamicVertexBuffer;
   std::vector<CDynamicIndexBuffer*> mArrayDynamicIndexBuffer;

   CSyncCriticalSection                      mGeneralCriticalSection;
private:
   uint32                                    mFrameCount;
   real64                                    mUniverseTime;
   bool                                      mAutoUpdateUniverseTime;

   static uint8 const *                      mFontCFONBuffer;
   static uint32                             mFontCFONBufferSize;
   static uint8 const *                      mFontTXTRBuffer;
   static uint32                             mFontTXTRBufferSize;

   char                                      mCurrentAreaDebugName[256];

};

template< class T > void RegisterWithArray(T* pObj, std::vector<T*>* pArray)
{
   BPE_VERIFY(pObj->mGlobalListIndex == -1, false, "RegisterWithArray: pObj must not already be registered in a global list");

   pArray->push_back(pObj);
   pObj->mGlobalListIndex = pArray->size()-1;
}
template< class T > void RemoveFromArray(T* pObj, std::vector<T*>* pArray)
{
   BPE_VERIFY(pObj->mGlobalListIndex != -1, false, "RemoveFromArray: pObj must be registered in a global list");

   if( pArray->size() > 1 )
   {
      // swap with last element
      T* pLastObj = (*pArray)[pArray->size()-1];
      pLastObj->mGlobalListIndex = pObj->mGlobalListIndex;
      (*pArray)[pObj->mGlobalListIndex] = pLastObj;
   }
   pObj->mGlobalListIndex = -1;
   pArray->pop_back();
}

//----------------------------------------------------------------------------

#include BPE_PLATFORM_PATH2(Renderer/Base/Backend, CRenderBackend.h)

//----------------------------------------------------------------------------

extern RENDERER_API CRenderBackend * gpRenderBackend;

//----------------------------------------------------------------------------

inline CRenderBackend * const RenderBackend()
{
   return gpRenderBackend;
}

//------------------------------------------------------------------------------------------

