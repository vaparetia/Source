//----------------------------------------------------------------------------
// X360CRenderBackend.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Primitive/X360/X360CIndexBuffer.h"
#include "Renderer/Base/Primitive/X360/X360CVertexBuffer.h"
#include "Renderer/Base/Backend/X360/X360CTexture.h"
#include "Renderer/Base/Backend/X360/CRenderBackendPrivate.h"
#include "Renderer/Base/CFrameCapture.h"

#include "Renderer/Base/Primitive/ProgShader/PSCMesh.h"

#include "Engine/Input/CInputGenerator.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CTaskQueue.h"

//----------------------------------------------------------------------------

#define SAFE_RELEASE(x) if(x) { (x)->Release(); }

//----------------------------------------------------------------------------

using namespace std;

#pragma warning ( disable : 4355 )

std::vector<IDirect3DResource9*> g_ToReleaseArray;

CTaskQueue* gpTaskQueue_Render;

void FlushRenderTaskQue_c()
{
   bpe_debugger_printf( "Flushing render task que\n" );

   gpTaskQueue_Render->FlushTasks();
}

// Be very careful with this function
// DO NOT use it for checking for completion
// Threads could still be processing 
int GetNumberOfCurrentRenderTasks_c()
{
   return gpTaskQueue_Render->GetCurrentNumberOfTasks();
}

int IsDoneWithRenderTasks_c()
{
   return gpTaskQueue_Render->IsDoneWithAllTasks();
}

namespace
{
   int gIsSuspended = 0;
}

//----------------------------------------------------------------------------

//#define DEBUG_PIXEL_SHADERS
//#define DEBUG_VERTEX_SHADERS

//----------------------------------------------------------------------------

bool FindFrameBufferSize(int const width, int const height, CRenderBackend::SRenderFrameBufferSize::EDisplayAspect const aspect, CRenderBackend::SRenderInitialization const & initFlags, CRenderBackend::SRenderFrameBufferSize* pFoundResolution)
{

   for( int i = 0; i < initFlags.mFrameBufferSizeCount; ++i )
   {
      CRenderBackend::SRenderFrameBufferSize const & frameBufferSize = initFlags.mpFrameBufferSize[i];

      if( frameBufferSize.GetDisplayAspect() == aspect &&
         frameBufferSize.GetWidth(CRenderBackend::SRenderFrameBufferSize::kST_Display) == width &&
         frameBufferSize.GetHeight(CRenderBackend::SRenderFrameBufferSize::kST_Display) == height )
      {
         memcpy(pFoundResolution, &frameBufferSize, sizeof(CRenderBackend::SRenderFrameBufferSize));
         return true;
      }
   }

   return false;
}
#ifdef BPE_DEBUG
extern "C" BOOL D3D__DisableConstantOverwriteCheck;
#endif

//----------------------------------------------------------------------------

//Note on 360 D3DXCompileShader only takes a c-string (no Unicode)
static const char pDefaultShader[] = "                               \
sampler2D TextureSampler : register(s0);                             \
                                                                     \
struct InV																		      \
{																				         \
	float4 Position : POSITION;													\
   float2 TexCoord01 : TEXCOORD0;                                    \
};																				         \
																				         \
struct OutV																		      \
{																				         \
	float4 Position : POSITION;													\
	float2 TexCoord01 : TEXCOORD0;												\
};																				         \
																				         \
struct InP																		      \
{																				         \
	float2 TexCoord01 : TEXCOORD0;												\
};																				         \
																				         \
OutV Main_V( InV In )															   \
{																				         \
	OutV Out = (OutV)0;															   \
																				         \
   float4 outPos = In.Position;                                      \
   outPos.y = 1.0f - outPos.y;                                       \
   outPos.xy *= float2(2.0f,2.0f);                                   \
   outPos.xy -= float2(1.0f,1.0f);                                   \
	Out.Position = outPos;													      \
	Out.TexCoord01 = In.TexCoord01;												\
																				         \
	return Out;																	      \
}																				         \
																				         \
float4 Main_P( InP In ) : COLOR													\
{																				         \
	float4 done = tex2D(TextureSampler, In.TexCoord01.xy);				\
																				         \
	return done;																      \
}																				         \
";

//----------------------------------------------------------------------------
bool CompileShaderDefaults(IDirect3DDevice9* pDevice, IDirect3DVertexShader9** pOldVertexShader, IDirect3DPixelShader9** pOldPixelShader)
{
   ID3DXBuffer* pShaderBuffer = NULL;
   ID3DXBuffer* pErrorBuffer = NULL;
   {
      
      if( FAILED( D3DXCompileShader( pDefaultShader, sizeof(pDefaultShader), NULL, NULL, "Main_V", "vs_3_0", 0, &pShaderBuffer, &pErrorBuffer, NULL ) ) )
      {
         char* errTxt = (char*)pErrorBuffer->GetBufferPointer();
         SAFE_RELEASE( pShaderBuffer );
         SAFE_RELEASE( pErrorBuffer );
         BPE_VERIFYA(false, "X360RenderBackend failed to Init! Failed to compile default system vertex shader Main_V!");
         return false;
      }
      DWORD* pShader = (DWORD*)pShaderBuffer->GetBufferPointer();
      DWORD shaderSize = pShaderBuffer->GetBufferSize()/sizeof(DWORD);
      {
         if( FAILED(pDevice->CreateVertexShader( pShader, pOldVertexShader ) ) )
         {
            SAFE_RELEASE( pShaderBuffer );
            SAFE_RELEASE( pErrorBuffer );
            BPE_VERIFYA(false, "Failed to create default system vertex shader Main_V");
            return false;
         }
      }
   }
   {
      ID3DXBuffer* pShaderBuffer = NULL;
      ID3DXBuffer* pErrorBuffer = NULL;
      if( FAILED( D3DXCompileShader( pDefaultShader, sizeof(pDefaultShader), NULL, NULL, "Main_P", "ps_3_0", 0, &pShaderBuffer, &pErrorBuffer, NULL ) ) )
      {
         SAFE_RELEASE( pShaderBuffer );
         SAFE_RELEASE( pErrorBuffer );
         BPE_VERIFYA(false, "X360RenderBackend failed to Init! Failed to compile default system vertex shader Main_V!");
         return false;
      }
      DWORD* pShader = (DWORD*)pShaderBuffer->GetBufferPointer();
      DWORD shaderSize = pShaderBuffer->GetBufferSize()/sizeof(DWORD);
      {
         if( FAILED(pDevice->CreatePixelShader( pShader, pOldPixelShader ) ) )
         {
            SAFE_RELEASE( pShaderBuffer );
            SAFE_RELEASE( pErrorBuffer );
            BPE_VERIFYA(false, "Failed to create default system pixel shader Main_P");
            return FALSE;
         }
      }
   }
   SAFE_RELEASE( pShaderBuffer );
   SAFE_RELEASE( pErrorBuffer );
   return true;
}

//----------------------------------------------------------------------------

CRenderBackend::CRenderBackend(IResourcePool & resourcePool, SRenderInitialization const &initFlags)
:  CBaseRenderBackend(resourcePool, initFlags)
,  mpDirect3D(NULL)
,  mpDirect3DDevice(NULL)
,  mCachedDeviceWrapper(NULL)
,  mCurrentRenderTarget()
,  mFirstFrame(true)
,  mBoundVertexDataHash_1(0)
,  mBoundVertexDataHash_2(0)
,  mBoundIndexBufferOffset(0)
,  mFogEnabled( false )
,  mFogColor( CColor::Black() )
,  mFogParams( 1.f, 1.f )
,  mCameraMatrix(CMatrix4::Identity())
,  mViewMatrix(CMatrix4::Identity())
,  mProjectionTimesViewMatrix( CMatrix4::Identity() )
,  mCurrentRenderTexture(NULL)
,  mCurrentDepthTexture(NULL)
,  mVBLCount(0)
,  mpRenderThreadFunction(NULL)
,  mRenderThreadFunctionParameter(-1)
,  mPresentationInterval(1)
,  mPresentationThreshold(0)
,  mAsyncSwaps(1)
,  mTrilinearThreshold(3) // Start this off at 1/8th
,  mVideoScalerMode(0)
,  mVertexShaderGPRCount(40)
{
   // Create task queue for rendering purposes.
   {
      int taskProcessors[5] = { 1, 2, 3, 4, 5 };
      const char* pThreadNames[5] = {"PCT decompress 1", "PCT decompress 2", "PCT decompress 3", "PCT decompress 4", "PCT decompress 5"};
      gpTaskQueue_Render = new CTaskQueue(5, taskProcessors, pThreadNames);
   }

   //D3D__DisableConstantOverwriteCheck = TRUE;

   mpDirect3D = TComPtr<IDirect3D9>(Direct3DCreate9(D3D_SDK_VERSION));

   XGetVideoMode(&mUserVideoMode);
   // We always render at 1280x720
   int renderTargetWidth = 1280;
   int renderTargetHeight = 720;
   // Copy off refresh rate
   // for this game pick either 50 or 60 for platform agnostic variable
   float diff60 = fabs(60.0f - mUserVideoMode.RefreshRate);
   float diff50 = fabs(50.0f - mUserVideoMode.RefreshRate);
   if( diff60 < diff50 )
   {
      mRefreshRate = 60.0f;
   }
   else
   {
      mRefreshRate = 50.0f;
   }

   float const aspect = 16.0f / 9.0f;

#if BPE_USE_PREDICATED_TILING
   const D3DRECT pTilingRects[] =
   {
      { 0,     0,    640,  720 },
      { 640,   0,  1280,   720 },
   };
   mIsTiling = false;
#else
   const D3DRECT pTilingRects[] =
   {
      { 0,  0,    1280, 720 },
   };
#endif
   
   mTilingRectCount = ARRAYSIZE( pTilingRects );
   memcpy( mpTilingRects, pTilingRects, mTilingRectCount * sizeof( D3DRECT ) );
   
   // Find frame buffer size for monoscopic setup
   if( !FindFrameBufferSize(1280, 720, SRenderFrameBufferSize::kDA_Widescreen, initFlags, &mFrameBufferSizeMono) )
   {
      BPE_VERIFYA(false, "Unable to find monoscopic frame buffer setup");
   }

   // Set up the structure used to create the D3DDevice.
   ZeroMemory( &mPresentParameters, sizeof(D3DPRESENT_PARAMETERS) );
   mPresentParameters.BackBufferWidth = renderTargetWidth;
   mPresentParameters.BackBufferHeight = renderTargetHeight;
   mPresentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;

   mPresentParameters.FrontBufferFormat = D3DFMT_LE_X8R8G8B8;
   if ((mInitFlags.mCreationFlags & kCF_WaitForVSync) != 0)
   {
      mPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
   }
   else
   {
      mPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
   }
   //mPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
   mPresentParameters.Windowed = FALSE;

   mPresentParameters.DisableAutoBackBuffer = TRUE;
   mPresentParameters.DisableAutoFrontBuffer = TRUE;
   mPresentParameters.EnableAutoDepthStencil = FALSE;
   mPresentParameters.AutoDepthStencilFormat = D3DFMT_D24S8;
   mPresentParameters.DisableAutoBackBuffer = FALSE;
   mPresentParameters.DisableAutoFrontBuffer = FALSE;
   mPresentParameters.EnableAutoDepthStencil = TRUE;

   mPresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;

   mPresentParameters.RingBufferParameters.SecondarySize = 7*(1024*1024);

   //Setup video scalar parameters
   mVideoScalerParms.ScalerSourceRect.x1 = 0;
   mVideoScalerParms.ScalerSourceRect.y1 = 0;
   mVideoScalerParms.ScalerSourceRect.x2 = mPresentParameters.BackBufferWidth;
   mVideoScalerParms.ScalerSourceRect.y2 = mPresentParameters.BackBufferHeight;
   mVideoScalerParms.ScaledOutputWidth = mUserVideoMode.dwDisplayWidth;
   mVideoScalerParms.ScaledOutputHeight = mUserVideoMode.fIsWideScreen ? mUserVideoMode.dwDisplayHeight : (mUserVideoMode.dwDisplayHeight * 3 / 4);
   mVideoScalerParms.FilterProfile = 0;

   UINT adapterToUse = D3DADAPTER_DEFAULT;
   D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

   IDirect3DDevice9* pDevice = NULL;
   HRESULT result = mpDirect3D->CreateDevice( adapterToUse, 
      deviceType, 
      (HWND)OsContext()->mMainWindow,
      D3DCREATE_CREATE_THREAD_ON_4|D3DCREATE_CREATE_THREAD_ON_5,
      &mPresentParameters, 
      &pDevice );

   if( FAILED(result) )
   {
      bpe_message_box("Failed to initialize D3D device!\n", "BPE Renderer Error");
      exit(0);
   }

   mpDirect3DDevice = TComPtr<IDirect3DDevice9>(pDevice);
   mCachedDeviceWrapper.reset(new CD3DCachedDevice(mpDirect3DDevice));

   SCreateTextureParams bufferTextureParams;
   bufferTextureParams.mRenderTargetWidth = mPresentParameters.BackBufferWidth;
   bufferTextureParams.mRenderTargetHeight = mPresentParameters.BackBufferHeight;

   // Create full screen textures
   for( int i = 0; i < BPE_ARRAY_SIZE(mpBackBufferTexture); ++i )
   {
      mpBackBufferTexture[i] = (CTexture *)CBaseTexture::Create(mPresentParameters.BackBufferWidth, mPresentParameters.BackBufferHeight, 1, 
                                                                CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, 
                                                                mPresentParameters.FrontBufferFormat, D3DMULTISAMPLE_NONE, false, &bufferTextureParams);
   }

   mBackBufferIndex = 0;
   mpCurrentBackBufferTexture = mpBackBufferTexture[mBackBufferIndex];

   GetD3DDevice()->SetRenderTarget(0, mpCurrentBackBufferTexture->Surface().GetPtr());
   
   pDevice->SetVerticalBlankCallback(VBlankCallback);

   //
#if BPE_USE_FLOATINGPOINT_ZBUFFER
   CBaseTexture::EFormat depthFormat = CBaseTexture::kFormat_D24FS8;
   D3DFORMAT depthFormatD3D = D3DFMT_D24FS8;
#else
   CBaseTexture::EFormat depthFormat = CBaseTexture::kFormat_D24X8;
   D3DFORMAT depthFormatD3D = D3DFMT_D24X8;
#endif

   bufferTextureParams.mBaseAddress = mpBackBufferTexture[0]->GetEDRAMEnd();
   bufferTextureParams.mCreateDepthTargetTexture = 0; // We don't need an actual texture for the full screen default depth buffer

   mpBackBufferDepthTexture = (CTexture *)CBaseTexture::Create(mPresentParameters.BackBufferWidth, mPresentParameters.BackBufferHeight, 1, 
                                                               depthFormat, CBaseTexture::kUsage_DepthBuffer, CBaseTexture::kAA_None, kRM_Video, 
                                                               depthFormatD3D, D3DMULTISAMPLE_NONE, false, &bufferTextureParams);

   GetD3DDevice()->SetDepthStencilSurface(mpBackBufferDepthTexture->Surface().GetPtr());

   RestoreObjects();

   InitDefaultObjects();
   FlushRenderTaskQue_c();

   mShaderCache.reset( new CCompiledShaderCache() );

   IDirect3DVertexShader9* pDefaultVertexShader; IDirect3DPixelShader9* pDefaultPixelShader;
   CompileShaderDefaults(pDevice, &pDefaultVertexShader, &pDefaultPixelShader);
   mDefaultVertexShader.Reset(pDefaultVertexShader);
   mDefaultPixelShader.Reset(pDefaultPixelShader);

   mpDynamicVertexBufferPool_RT.reset(new CDynamicVertexBufferPool_RT(initFlags.mVertexBufferPoolSize_RT, initFlags.mVertexBufferPoolChunkCount_RT));
   mpDynamicIndexBufferPool_RT.reset(new CDynamicIndexBufferPool_RT(initFlags.mIndexBufferPoolSize_RT, initFlags.mIndexBufferPoolChunkCount_RT));

   mpDynamicVertexBufferPool_UT.reset(new CDynamicVertexBufferPool_UT(initFlags.mVertexBufferPoolSize_UT, initFlags.mVertexBufferPoolChunkCount_UT));
   mpDynamicIndexBufferPool_UT.reset(new CDynamicIndexBufferPool_UT(initFlags.mIndexBufferPoolSize_UT, initFlags.mIndexBufferPoolChunkCount_UT));
}

//----------------------------------------------------------------------------

bool CBaseRenderBackend::SupportsStereo3D() const
{
   return false;
}

//----------------------------------------------------------------------------

CRenderBackend::EInitializeDisplayResult CBaseRenderBackend::InitializeDisplay(EStereoMode const stereoMode)
{
   return kIDR_Succeded;
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::UninitializeDisplay()
{
}

//----------------------------------------------------------------------------

CRenderBackend::~CRenderBackend()
{
   ShaderCache()->FlushAllShaders();
   InvalidateObjects();

   SetEvent(mRT_GoEvent); // Allow RT to go if it is stalled
   SetEvent(mRT_ThreadDieEvent); // Allow RT to die
  
   WaitForSingleObject(mRT_Thread, INFINITE);
   CloseHandle(mRT_ThreadDieEvent);
   CloseHandle(mRT_DoneEvent);
   CloseHandle(mRT_GoEvent);

   CloseHandle(mRT_Thread);
}

//----------------------------------------------------------------------------

void CRenderBackend::InitDefaultState()
{
   SetPerspectiveProjection();
}

//----------------------------------------------------------------------------

void CRenderBackend::InvalidateObjects()
{
   CBaseTexture::DeleteTexture(mWhiteMap);
   mWhiteMap = NULL;

   CBaseTexture::DeleteTexture(mBlackMap);
   mBlackMap = NULL;

   CBaseTexture::DeleteTexture(mFlatNormalMap);
   mFlatNormalMap = NULL;
}

//----------------------------------------------------------------------------

void CRenderBackend::RestoreObjects()
{
   // get ptr to frame buffer
   {
      LPDIRECT3DSURFACE9 pBackBuffer = NULL;
      D3DDeviceUncached()->GetRenderTarget( 0, &pBackBuffer );
   }

   InitDefaultState();
}

//----------------------------------------------------------------------------

void CRenderBackend::BeginScene()
{
   // Update Async Swaps field.
   {
      static int sLastAsyncSwaps = 0;
      if( mAsyncSwaps != sLastAsyncSwaps )
      {
         sLastAsyncSwaps = mAsyncSwaps;
         D3DDeviceUncached()->SetSwapMode( mAsyncSwaps ? TRUE : FALSE );
      }

      D3DDeviceUncached()->SetRenderState(D3DRS_BUFFER2FRAMES, mAsyncSwaps ? FALSE : TRUE);
   }

   SetPresentationIntervalNum(mPresentationInterval);

   D3DDeviceUncached()->SetRenderState(D3DRS_PRESENTIMMEDIATETHRESHOLD, mPresentationThreshold);

   InternalBeginScene();

#if !ENABLE_SEPERATE_RENDER_THREAD
   BeginFrameResourceTick();
#endif

   GetD3DDevice()->BeginScene();
#if BPE_PIXELCENTER_MATCH_TEXELCENTER
   D3DDeviceUncached()->SetRenderState(D3DRS_HALFPIXELOFFSET, TRUE);
#endif

   SetRenderTarget( SRenderTarget(mpCurrentBackBufferTexture, NULL, NULL, NULL, NULL) );
   //
   SetGPRAllocation(mVertexShaderGPRCount);

   if( mTrilinearThreshold == 0 )
   {
      D3DDeviceUncached()->SetSamplerState(0, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_IMMEDIATE);
      D3DDeviceUncached()->SetSamplerState(1, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_IMMEDIATE);
      D3DDeviceUncached()->SetSamplerState(2, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_IMMEDIATE);
      D3DDeviceUncached()->SetSamplerState(3, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_IMMEDIATE);
   }
   else if( mTrilinearThreshold == 1 )
   {
      D3DDeviceUncached()->SetSamplerState(0, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONESIXTH);
      D3DDeviceUncached()->SetSamplerState(1, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONESIXTH);
      D3DDeviceUncached()->SetSamplerState(2, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONESIXTH);
      D3DDeviceUncached()->SetSamplerState(3, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONESIXTH);
   }
   else if( mTrilinearThreshold == 2 )
   {
      D3DDeviceUncached()->SetSamplerState(0, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONEFOURTH);
      D3DDeviceUncached()->SetSamplerState(1, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONEFOURTH);
      D3DDeviceUncached()->SetSamplerState(2, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONEFOURTH);
      D3DDeviceUncached()->SetSamplerState(3, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_ONEFOURTH);
   }
   else if( mTrilinearThreshold == 3 )
   {
      D3DDeviceUncached()->SetSamplerState(0, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_THREEEIGHTHS);
      D3DDeviceUncached()->SetSamplerState(1, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_THREEEIGHTHS);
      D3DDeviceUncached()->SetSamplerState(2, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_THREEEIGHTHS);
      D3DDeviceUncached()->SetSamplerState(3, D3DSAMP_TRILINEARTHRESHOLD, D3DTRILINEAR_THREEEIGHTHS);
   }

   CAPTURE_BEGIN();
}

//----------------------------------------------------------------------------

void CRenderBackend::EndScene()
{
   CAPTURE_END();

   GetD3DDevice()->EndScene();
   Present();
}

//----------------------------------------------------------------------------

void CRenderBackend::Clear(int const clearFlags, CColor const color, real32 const z, int const stencil)
{
   BPE_ASSERT(!GetDirect3DDevice().IsNull(), "no direct 3d device");

   DWORD d3dClearFlags = (clearFlags & kFlag_Color) ? D3DCLEAR_TARGET : 0;

   IDirect3DSurface9* pDepthSurface = NULL;
   D3DDeviceUncached()->GetDepthStencilSurface(&pDepthSurface);

   if( pDepthSurface != NULL )
   {
      pDepthSurface->Release();
      d3dClearFlags |= (clearFlags & kFlag_Depth) ? D3DCLEAR_ZBUFFER : 0;
      d3dClearFlags |= (clearFlags & kFlag_Stencil) ? D3DCLEAR_STENCIL : 0;
   }

   if( d3dClearFlags )
   {
#if BPE_INVERT_ZBUFFER
      real32 z2 = 1.0f - z;
#else
      real32 z2 = z;
#endif
      D3DDeviceUncached()->Clear( 0, NULL, d3dClearFlags, color.GetARGB(), z2, stencil );
   }
}

//----------------------------------------------------------------------------
void CRenderBackend::VBlankCallback(D3DVBLANKDATA *pData)
{
   //NOTES: do not put a breakpoint in this function
   //this function is a DPC callback, if you put a breakpoint here
   //your debugger will crash!
   //InterlockedIncrement(&sNumVBlank);

   gpRenderBackend->mVBLCount++;
}

//----------------------------------------------------------------------------

void CRenderBackend::Present()
{
   // In Async Swap case stall for back buffer to become available, though we may have already been trashing it!

   {
      // Resolve EDRAM to current back buffer texture.
      D3DDeviceUncached()->Resolve( D3DRESOLVE_RENDERTARGET0|D3DRESOLVE_ALLFRAGMENTS, NULL, mpCurrentBackBufferTexture->GetTexture(), NULL, 0, 0, NULL, 0.0f, 0, NULL );

      // Stall for VSync?
      {
         CStopWatch timer;

         if( !mAsyncSwaps )
         {
            D3DDeviceUncached()->SynchronizeToPresentationInterval();
         }
         else
         {
            int const frontBufferCount = BPE_ARRAY_SIZE(mpBackBufferTexture);

            D3DSWAP_STATUS swapStatus;
            
            for(;;)
            {
               D3DDeviceUncached()->QuerySwapStatus(&swapStatus);

               if( (swapStatus.EnqueuedCount + 1) < frontBufferCount )
                  break;
            }

            //printf("SwapStatus: Enqueued: %d Swaps: %d Stall: %.3fms\n", swapStatus.EnqueuedCount, swapStatus.Swap, timer.GetElapsedTime() * 1000.0f);
         }

         mMetrics.mVBlankWaitTime = timer.GetElapsedTime();
      }

      // Note: if you don't reset GPR allocations to default, you will get GPU/CPU hangs
      SetGPRAllocation(64);

      mVideoScalerParms.FilterProfile = mVideoScalerMode;
      D3DDeviceUncached()->Swap(mpCurrentBackBufferTexture->GetTexture(), &mVideoScalerParms);

      mBackBufferIndex = (mBackBufferIndex + 1) % BPE_ARRAY_SIZE(mpBackBufferTexture);
      mpCurrentBackBufferTexture = mpBackBufferTexture[mBackBufferIndex];

      mMetrics.mTimeSinceLastFlip = mFrameTimer.GetElapsedTime();
      mFrameTimer.Reset();
   }

   ForceVertexDataRebind();
   D3DDeviceUncached()->UnsetAll();

   InternalPresent();
}

//----------------------------------------------------------------------------

void CRenderBackend::ShutDown()
{
   Present();
}

//----------------------------------------------------------------------------
void CRenderBackend::RestoreRenderTargetColor(CBaseTexture* pRenderTexture, CBaseTexture* pSrcTexture, SRect const & RTRect, SRect const & srcRect)
{
   SRectf RTRectf = { 
      (float)RTRect.x1 / (float)pRenderTexture->GetWidth(), (float)RTRect.y2 / (float)pRenderTexture->GetHeight(), 
      (float)RTRect.x2 / (float)pRenderTexture->GetWidth(), (float)RTRect.y2 / (float)pRenderTexture->GetHeight() 
   };
   SRectf srcRectf = { 
      (float)srcRect.x1 / (float)pRenderTexture->GetWidth(), (float)srcRect.y2 / (float)pRenderTexture->GetHeight(), 
      (float)srcRect.x2 / (float)pRenderTexture->GetWidth(), (float)srcRect.y2 / (float)pRenderTexture->GetHeight() 
   };

   RestoreRenderTargetColor_Internal(pRenderTexture, pSrcTexture, RTRectf, srcRectf );
}

//----------------------------------------------------------------------------
void CRenderBackend::RestoreRenderTargetColor_Internal(CBaseTexture* pRenderTexture_, CBaseTexture* pSrcTexture_, SRectf const & RTRectf, SRectf const & srcRectf)
{
   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("RestoreRenderTargetColor");

   CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
   CTexture * pSrcTexture = (CTexture *)pSrcTexture_;
   //Get All current blend states
   D3DBLENDSTATE oldBlendState;
   DWORD oldHalfPixelOffset, oldCullMode, oldZEnable, oldAlphaTestEnable, oldMinFilter, oldMagFilter;
   IDirect3DBaseTexture9* pOldTexture;
   IDirect3DSurface9* pOldRenderTarget;
   D3DVIEWPORT9 oldViewport;
   IDirect3DVertexDeclaration9* pOldVertexDeclaration;
   IDirect3DVertexShader9* pOldVertexShader;
   IDirect3DPixelShader9* pOldPixelShader;

   IDirect3DDevice9* pD3D = GetDirect3DDevice().GetPtr();
   pD3D->GetBlendState(0, &oldBlendState);
   pD3D->GetRenderState(D3DRS_HALFPIXELOFFSET, &oldHalfPixelOffset);
   pD3D->GetRenderState(D3DRS_CULLMODE, &oldCullMode);
   pD3D->GetRenderState(D3DRS_ZENABLE , &oldZEnable);
   pD3D->GetRenderState(D3DRS_ALPHATESTENABLE, &oldAlphaTestEnable);
   pD3D->GetViewport(&oldViewport);

   pD3D->GetSamplerState(0, D3DSAMP_MINFILTER, &oldMinFilter);
   pD3D->GetSamplerState(0, D3DSAMP_MAGFILTER, &oldMagFilter);
   pD3D->GetTexture(0, &pOldTexture);

   pD3D->GetRenderTarget(0, &pOldRenderTarget);

   pD3D->GetVertexDeclaration(&pOldVertexDeclaration);
   pD3D->GetVertexShader(&pOldVertexShader);
   pD3D->GetPixelShader(&pOldPixelShader);
   //Set needed states
   D3DBLENDSTATE newBlendState;
   newBlendState.SrcBlend  = D3DBLEND_ONE;
   newBlendState.BlendOp = D3DBLENDOP_ADD;
   newBlendState.DestBlend = D3DBLEND_ZERO;
   newBlendState.SrcBlendAlpha = D3DBLEND_ONE;
   newBlendState.BlendOpAlpha = D3DBLENDOP_ADD;
   newBlendState.DestBlendAlpha = D3DBLEND_ZERO;
   pD3D->SetBlendState(0, newBlendState);
   pD3D->SetRenderState(D3DRS_HALFPIXELOFFSET, TRUE);
   pD3D->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
   pD3D->SetRenderState(D3DRS_ZENABLE , D3DZB_FALSE);
   pD3D->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

   pD3D->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
   pD3D->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
   pD3D->SetTexture(0, pSrcTexture->GetTexture());

   pD3D->SetRenderTarget(0, pRenderTexture->Surface().GetPtr());
   
   pD3D->SetVertexShader(mDefaultVertexShader.GetPtr());
   pD3D->SetPixelShader(mDefaultPixelShader.GetPtr());
   pD3D->SetFVF(D3DFVF_XYZ|D3DFVF_TEX1);
   //Actual draw code
   {
      struct SVertex
      { 
         real32 x, y, z;
         real32 u, v;
      };

      SVertex const vertices[] =
      {
         { RTRectf.x1, RTRectf.y2, BPE_VIEWPORT_MAXZ, srcRectf.x1, srcRectf.y2 },
         { RTRectf.x2, RTRectf.y2, BPE_VIEWPORT_MAXZ, srcRectf.x2, srcRectf.y2 },
         { RTRectf.x1, RTRectf.y1, BPE_VIEWPORT_MAXZ, srcRectf.x1, srcRectf.y1 },
         { RTRectf.x2, RTRectf.y1, BPE_VIEWPORT_MAXZ, srcRectf.x2, srcRectf.y1 }
      };

      pD3D->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertices[0], sizeof(SVertex) );
   }
   //restore all changed render states
   pD3D->SetBlendState(0, oldBlendState);
   pD3D->SetRenderState(D3DRS_HALFPIXELOFFSET, oldHalfPixelOffset);
   pD3D->SetRenderState(D3DRS_CULLMODE, oldCullMode);
   pD3D->SetRenderState(D3DRS_ZENABLE, oldZEnable);
   pD3D->SetRenderState(D3DRS_ALPHATESTENABLE, oldAlphaTestEnable);

   pD3D->SetSamplerState(0, D3DSAMP_MINFILTER, oldMinFilter);
   pD3D->SetSamplerState(0, D3DSAMP_MAGFILTER, oldMagFilter);
   pD3D->SetTexture(0, pOldTexture);

   pD3D->SetRenderTarget(0, pOldRenderTarget);
   pD3D->SetViewport(&oldViewport);

   pD3D->SetVertexShader(pOldVertexShader);
   pD3D->SetPixelShader(pOldPixelShader);
   pD3D->SetVertexDeclaration(pOldVertexDeclaration);
   //Dec any incremented reference counts
   SAFE_RELEASE(pOldTexture);
   SAFE_RELEASE(pOldRenderTarget);
   SAFE_RELEASE(pOldVertexDeclaration);
   SAFE_RELEASE(pOldVertexShader);
   SAFE_RELEASE(pOldPixelShader);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveRenderTargetSixteenth(CBaseTexture * pRenderTextureHalf_, CBaseTexture * pFakeMSAARenderTextureHalf_, CBaseTexture * pRenderTextureQuarter_)
{
   CTexture * pRenderTextureHalf = (CTexture *)pRenderTextureHalf_;
   CTexture * pRenderTextureQuarter = (CTexture *)pRenderTextureQuarter_;
   CTexture * pFakeMSAARenderTextureHalf = (CTexture *)pFakeMSAARenderTextureHalf_;

   CD3DCachedDevice * pCachedDevice = GetCachedDeviceWrapper();
   IDirect3DDevice9 * pD3D = GetDirect3DDevice().GetPtr();

   BPE_VERIFY(pFakeMSAARenderTextureHalf->GetAntiAliasType() == CBaseTexture::kAA_MSAA4x, false, "Invalid pFakeMSAARenderTextureHalf: Must be multisampled and quarter width/height");
   BPE_VERIFY(pFakeMSAARenderTextureHalf->GetWidth() == pRenderTextureHalf->GetWidth(), false, "pRenderTextureHalf must match pFakeMSAARenderTextureHalf width/height");
   BPE_VERIFY(pFakeMSAARenderTextureHalf->GetHeight() == pRenderTextureHalf->GetHeight(), false, "pRenderTextureHalf must match pFakeMSAARenderTextureHalf width/height");
   BPE_VERIFY(pRenderTextureQuarter->GetWidth() == pRenderTextureHalf->GetWidth()/2, false, "pRenderTextureQuarter must be half pRenderTextureHalf width/height");
   BPE_VERIFY(pRenderTextureQuarter->GetHeight() == pRenderTextureHalf->GetHeight()/2, false, "pRenderTextureQuarter must be half pRenderTextureHalf width/height");
   IDirect3DSurface9 * pCurrRenderTarget;
   pD3D->GetRenderTarget(0, &pCurrRenderTarget);
   IDirect3DSurface9 * pCurrDepthStencil;
   pD3D->GetDepthStencilSurface(&pCurrDepthStencil);
   D3DVIEWPORT9 oldViewport;
   pD3D->GetViewport(&oldViewport);
#ifdef BPE_DEBUG
   D3DSURFACE_DESC desc;
   pCurrRenderTarget->GetDesc(&desc);
   BPE_VERIFY(desc.MultiSampleType == D3DMULTISAMPLE_NONE, false, "Source Rendertarget must not be MSAA");
   //BPE_VERIFY(desc.Width/4 == pRenderTextureHalf->GetWidth(), false, "Source Rendertarget must be quarter size");
   //BPE_VERIFY(desc.Height/4 == pRenderTextureHalf->GetHeight(), false, "Source Rendertarget must be quarter size");
#endif
   // trick the hardware into giving us a free (scrambled) 2x2 downsample as per X360 post process sample app
   pCachedDevice->SetRenderTarget(0, pFakeMSAARenderTextureHalf->Surface().GetPtr());
   pCachedDevice->SetDepthStencilSurface(NULL);
   pD3D->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, pRenderTextureHalf->GetTexture(), NULL, 0, 0, NULL, 1.0f, 0, NULL);

   // now perform a bilinearly-sampled copy to get the next 2x2 downsample (fixes scrambling from first step)
   pCachedDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
   pCachedDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
   pCachedDevice->SetTexture(0, pRenderTextureHalf->GetTexture());
   pCachedDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
   pCachedDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
   pCachedDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
   pCachedDevice->SetRenderState( D3DRS_HALFPIXELOFFSET, TRUE );
   //
   pCachedDevice->SetRenderTarget(0, pRenderTextureQuarter->Surface().GetPtr());

   // set render states
   SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   SetCullMode(CRenderBackend::kCM_None);
   SetDepthCompareEnabled(false);
   SetAlphaTestEnable(false);

   // render full-view rectangle to apply shrink
   pCachedDevice->SetVertexShader(mDefaultVertexShader.GetPtr());
   pCachedDevice->SetPixelShader(mDefaultPixelShader.GetPtr());
   pCachedDevice->SetFVF(D3DFVF_XYZ|D3DFVF_TEX1);
   //Actual draw code
   {
      struct SVertex
      { 
         real32 x, y, z;
         real32 u, v;
      };

      SVertex const vertices[] =
      {
         { 0.0f, 0.0f, BPE_VIEWPORT_MAXZ, 0.0f, 0.0f },
         { 1.0f, 0.0f, BPE_VIEWPORT_MAXZ, 1.0f, 0.0f },
         { 0.0f, 1.0f, BPE_VIEWPORT_MAXZ, 0.0f, 1.0f },
         { 1.0f, 1.0f, BPE_VIEWPORT_MAXZ, 1.0f, 1.0f }
      };

      pCachedDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertices[0], sizeof(SVertex) );
   }

   //------------------------------------
   // resolve half scale texture
   pD3D->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, pRenderTextureQuarter->GetTexture(), NULL, 0, 0, NULL, 1.0f, 0, NULL);

   //Restore any neccessary renderstates
   pCachedDevice->SetRenderState( D3DRS_HALFPIXELOFFSET, FALSE );
   SetDepthCompareEnabled(true);
   SetAlphaTestEnable(true);
   pCachedDevice->SetRenderTarget(0,pCurrRenderTarget);
   pCachedDevice->SetDepthStencilSurface(pCurrDepthStencil);
   pCachedDevice->SetViewport(&oldViewport);
   //
   SAFE_RELEASE(pCurrRenderTarget);
   SAFE_RELEASE(pCurrDepthStencil);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveRenderTarget(CBaseTexture * pRenderTexture_)
{
   CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
   D3DDeviceUncached()->Resolve(D3DRESOLVE_RENDERTARGET0|D3DRESOLVE_ALLFRAGMENTS, NULL, pRenderTexture->GetTexture(), NULL, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolvePartialRenderTarget(CBaseTexture * pRenderTexture, SRect const & srcRect_)
{
   SRect srcRect;
   //must be multiples of 8 (round down starts, round up ends)
   srcRect.x1 = srcRect_.x1 & ~7;
   srcRect.x2 = (srcRect_.x2 + 7) & ~7;
   srcRect.y1 = srcRect_.y1 & ~7;
   srcRect.y2 = (srcRect_.y2 + 7) & ~7;

   //must not exceed bounds
   srcRect.x1 = bpe::max_val(0, srcRect.x1);
   srcRect.x2 = bpe::min_val(pRenderTexture->GetWidth(), srcRect.x2);
   srcRect.y1 = bpe::max_val(0, srcRect.y1);
   srcRect.y2 = bpe::min_val(pRenderTexture->GetHeight(), srcRect.y2);
   ResolvePartialRenderTarget_Unsafe(pRenderTexture, &srcRect);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolvePartialRenderTarget_Unsafe(CBaseTexture * pRenderTexture_, SRect const * pSrcRect)
{
   CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
   D3DDeviceUncached()->Resolve(D3DRESOLVE_RENDERTARGET0|D3DRESOLVE_ALLFRAGMENTS, (D3DRECT const *)pSrcRect, pRenderTexture->GetTexture(), (D3DPOINT const *)pSrcRect, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveRenderTargetPredicated(CBaseTexture * pRenderTexture_)
{
   IDirect3DSurface9* pCurrRenderTarget = NULL;
   D3DDeviceUncached()->GetRenderTarget(0, &pCurrRenderTarget);
   if( pCurrRenderTarget )
   {
      if( pRenderTexture_ )
      {
#if BPE_USE_PREDICATED_TILING
         CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
         if( mIsTiling )
         {
            for( int i = 0; i < ( UINT )mTilingRectCount; ++i )
            {
               D3DDeviceUncached()->SetPredication( D3DPRED_TILE( i ) );
               D3DPOINT* pDestPoint = ( D3DPOINT* )&mpTilingRects[i];

               //D3DDeviceUncached()->Resolve(D3DRESOLVE_RENDERTARGET0|D3DRESOLVE_ALLFRAGMENTS|D3DRESOLVE_CLEARRENDERTARGET|D3DRESOLVE_CLEARDEPTHSTENCIL, &mpTilingRects[i], pRenderTexture->GetTexture(), pDestPoint, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
               D3DDeviceUncached()->Resolve(D3DRESOLVE_ALLFRAGMENTS, &mpTilingRects[i], pRenderTexture->GetTexture(), pDestPoint, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
            }
            D3DDeviceUncached()->SetPredication( 0 ); //Restore predication to default.
         }
         else
         {
            CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
            D3DDeviceUncached()->Resolve(D3DRESOLVE_ALLFRAGMENTS, NULL, pRenderTexture->GetTexture(), NULL, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
         }
#else
         CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
         D3DDeviceUncached()->Resolve(D3DRESOLVE_ALLFRAGMENTS, NULL, pRenderTexture->GetTexture(), NULL, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
#endif
      }
      pCurrRenderTarget->Release();
   }
}

void CRenderBackend::ResolveDepthStencilPredicated_Internal(CBaseTexture * pRenderTexture_, int additionalFlags)
{
   IDirect3DSurface9* pCurrDepthStencil = NULL;
   D3DDeviceUncached()->GetDepthStencilSurface(&pCurrDepthStencil);
   if( pCurrDepthStencil )
   {
      if( pRenderTexture_ )
      {
#if BPE_USE_PREDICATED_TILING
         if( mIsTiling )
         {
            CTexture * pRenderTexture = (CTexture *)pRenderTexture_;

            for( int i = 0; i < ( UINT )mTilingRectCount; ++i )
            {
               D3DDeviceUncached()->SetPredication( D3DPRED_TILE( i ) );
               D3DPOINT* pDestPoint = ( D3DPOINT* )&mpTilingRects[i];

               D3DDeviceUncached()->Resolve(D3DRESOLVE_DEPTHSTENCIL|additionalFlags, &mpTilingRects[i], pRenderTexture->GetTexture(), pDestPoint, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
            }
            D3DDeviceUncached()->SetPredication( 0 ); //Restore predication to default.
         }
         else
         {
            CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
            D3DDeviceUncached()->Resolve(D3DRESOLVE_DEPTHSTENCIL|additionalFlags, NULL, pRenderTexture->GetTexture(), NULL, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
         }
#else
         CTexture * pRenderTexture = (CTexture *)pRenderTexture_;
         D3DDeviceUncached()->Resolve(D3DRESOLVE_DEPTHSTENCIL|additionalFlags, NULL, pRenderTexture->GetTexture(), NULL, 0, 0, NULL, mCurrentRenderTarget.mMinZ, 0, NULL);
#endif
      }
      pCurrDepthStencil->Release();
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveDepthStencilPredicated(CBaseTexture * pRenderTexture)
{
   ResolveDepthStencilPredicated_Internal(pRenderTexture, D3DRESOLVE_FRAGMENT0);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveDepthStencilPredicatedMSAA2(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1)
{
   ResolveDepthStencilPredicated_Internal(pRenderTexture0, D3DRESOLVE_FRAGMENT0);
   ResolveDepthStencilPredicated_Internal(pRenderTexture1, D3DRESOLVE_FRAGMENT1);
}

//----------------------------------------------------------------------------

void CRenderBackend::ResolveDepthStencilPredicatedMSAA4(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1, CBaseTexture * pRenderTexture2, CBaseTexture * pRenderTexture3)
{
   ResolveDepthStencilPredicated_Internal(pRenderTexture0, D3DRESOLVE_FRAGMENT0);
   ResolveDepthStencilPredicated_Internal(pRenderTexture1, D3DRESOLVE_FRAGMENT1);
   ResolveDepthStencilPredicated_Internal(pRenderTexture2, D3DRESOLVE_FRAGMENT2);
   ResolveDepthStencilPredicated_Internal(pRenderTexture3, D3DRESOLVE_FRAGMENT3);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetRenderTarget(SRenderTarget const & renderTarget)
{
   // early out
   if( mCurrentRenderTarget == renderTarget )
      return;

   mCurrentRenderTarget = renderTarget;

   bool validColorSurfaces = false;

   CTexture* pRenderTargetTextures[] = { NULL, NULL, NULL, NULL };
   for( int i = 0; i < BPE_ARRAY_SIZE(mCurrentRenderTarget.mpColorBuffer); ++i )
   {
      CTexture* pColorBuffer = (CTexture*)mCurrentRenderTarget.mpColorBuffer[i];

      if(pColorBuffer == NULL )
         break;

      validColorSurfaces = true;
      pRenderTargetTextures[i] = pColorBuffer;
   }

   bool boundBackBuffer = false;

   // only bind device back buffer if user isn't binding a depth buffer (if user only binds depth buffer thats ok)
   if( !validColorSurfaces && renderTarget.mBindBackBuffer != SRenderTarget::kBBM_NoBackBuffer )
   {
      pRenderTargetTextures[0] = mpCurrentBackBufferTexture;
      boundBackBuffer = true;
   }

   CTexture* pDepthBufferTexture = NULL;
   if( mCurrentRenderTarget.mpDepthBuffer )
   {
      pDepthBufferTexture = (CTexture*)mCurrentRenderTarget.mpDepthBuffer;
   }

   // get dimensions of current render target
   {
      if( pRenderTargetTextures[0] != NULL )
      {
         mRenderTargetWidth = pRenderTargetTextures[0]->GetWidth();
         mRenderTargetHeight = pRenderTargetTextures[0]->GetHeight();
      }
      else if( pDepthBufferTexture != NULL )
      {
         mRenderTargetWidth = pDepthBufferTexture->GetWidth();
         mRenderTargetHeight = pDepthBufferTexture->GetHeight();
      }
   }

   HRESULT res;
   for( int i = 0; i < BPE_ARRAY_SIZE(pRenderTargetTextures); ++i )
   {
      if( pRenderTargetTextures[i] )
         res = GetD3DDevice()->SetRenderTarget(i, pRenderTargetTextures[i]->Surface().GetPtr());
      else
         res = GetD3DDevice()->SetRenderTarget(i, NULL);
   }
   if( pDepthBufferTexture )
      res = GetD3DDevice()->SetDepthStencilSurface( pDepthBufferTexture->Surface().GetPtr() );
   else
      res = GetD3DDevice()->SetDepthStencilSurface( NULL );

   D3DVIEWPORT9 vp;
   vp.X = vp.Y = 0;
   vp.Width = mRenderTargetWidth;
   vp.Height = mRenderTargetHeight;
   vp.MinZ = renderTarget.mMinZ;
   vp.MaxZ = renderTarget.mMaxZ;

   if( boundBackBuffer )
   {
      mRenderTargetWidth = GetBackBufferWidth();
      mRenderTargetHeight = GetBackBufferHeight();

      switch( renderTarget.mBindBackBuffer )
      {
      case SRenderTarget::kBBM_BackBufferFill:
         {
            vp.X = 0;
            vp.Y = 0;
            vp.Width = GetBackBufferWidth();
            vp.Height = GetBackBufferHeight();
         }
         break;

      case SRenderTarget::kBBM_BackBufferCentered:
         {
            int bufferHeight = GetBackBufferHeight();

            if( GetFrameBufferSize().GetDisplayAspect() == SRenderFrameBufferSize::kDA_Fullscreen )
               bufferHeight *= (3.0f / 4.0f);

            int const diffX = GetBackBufferWidth() - GetMainFrameBufferWidth();
            int const diffY = GetBackBufferHeight() - bufferHeight;
            vp.X = diffX / 2;
            vp.Y = diffY / 2;
            vp.Width = GetMainFrameBufferWidth();
            vp.Height = bufferHeight;
         }
         break;
      }
   }

   if( renderTarget.mRenderTargetViewPort.mViewportEnabled )
   {
      vp.X = renderTarget.mRenderTargetViewPort.mViewportX;
      vp.Y = renderTarget.mRenderTargetViewPort.mViewportY;
      vp.Width = renderTarget.mRenderTargetViewPort.mViewportWidth;
      vp.Height = renderTarget.mRenderTargetViewPort.mViewportHeight;
   }

   mViewportWidth = vp.Width;
   mViewportHeight = vp.Height;

   GetD3DDevice()->SetViewport( &vp );

   if( renderTarget.mScissorEnabled )
   {
      GetD3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

      RECT scissorRect;

      scissorRect.left = renderTarget.mScissorX;
      scissorRect.right = renderTarget.mScissorX + renderTarget.mScissorWidth;

      scissorRect.top = renderTarget.mScissorY;
      scissorRect.bottom = renderTarget.mScissorY + renderTarget.mScissorHeight;

      D3DDeviceUncached()->SetScissorRect(&scissorRect);
   }
   else
   {
      GetD3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
   }

   mCurrentRenderTexture = pRenderTargetTextures[0];
   mCurrentDepthTexture = pDepthBufferTexture;
}

void CRenderBackend::FixBeginTilingViewportBug_Internal(SRenderTarget const & renderTarget)
{
   if( renderTarget.mRenderTargetViewPort.mViewportEnabled )
   {
      D3DVIEWPORT9 vp;
      vp.MinZ = renderTarget.mMinZ;
      vp.MaxZ = renderTarget.mMaxZ;
      vp.X = renderTarget.mRenderTargetViewPort.mViewportX;
      vp.Y = renderTarget.mRenderTargetViewPort.mViewportY;
      vp.Width = renderTarget.mRenderTargetViewPort.mViewportWidth;
      vp.Height = renderTarget.mRenderTargetViewPort.mViewportHeight;
      GetD3DDevice()->SetViewport( &vp );
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderQuad2d( CVector2 const & start,
                                  CVector2 const & end,
                                  CColor const & color )
{
   struct SVertex
   { 
      D3DXVECTOR4 p;
      DWORD color;
   };

   SVertex const vertices[] =
   {
      { D3DXVECTOR4( start.GetX(),  end.GetY(),    mFrustumNear, 1.0f ), color.GetARGB() },
      { D3DXVECTOR4( start.GetX(),  start.GetY(),  mFrustumNear, 1.0f ), color.GetARGB() },
      { D3DXVECTOR4( end.GetX(),    end.GetY(),    mFrustumNear, 1.0f ), color.GetARGB() },

      { D3DXVECTOR4( end.GetX(),    start.GetY(),  mFrustumNear, 1.0f ), color.GetARGB() },
      { D3DXVECTOR4( end.GetX(),    end.GetY(),    mFrustumNear, 1.0f ), color.GetARGB() },
      { D3DXVECTOR4( start.GetX(),  start.GetY(),  mFrustumNear, 1.0f ), color.GetARGB() }
   };

   GetD3DDevice()->SetRenderState( D3DRS_ZENABLE, FALSE );
   GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
   GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
   GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

   GetD3DDevice()->SetVertexShader( NULL );
   GetD3DDevice()->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE );
   GetD3DDevice()->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );

   GetD3DDevice()->SetPixelShader( NULL );
   GetD3DDevice()->SetTexture( 0, NULL );

   GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &vertices[0], sizeof( SVertex ) );

   GetD3DDevice()->SetRenderState( D3DRS_VIEWPORTENABLE, TRUE );

   ForceVertexDataRebind();
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderLine2d( CVector2 const & start,
                                  CVector2 const & end,
                                  CColor const & color )
{
   struct SVertex
   { 
      D3DXVECTOR4 p;
      DWORD color;
   };

   SVertex const vertices[] =
   {
      { D3DXVECTOR4( start.GetX(),  start.GetY(),  mFrustumNear, 1.0f ), color.GetARGB() },
      { D3DXVECTOR4( end.GetX(),    end.GetY(),    mFrustumNear, 1.0f ), color.GetARGB() }
   };

   GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
   GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
   GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

   GetD3DDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
   GetD3DDevice()->SetVertexShader( NULL );
   GetD3DDevice()->SetFVF( D3DFVF_XYZ|D3DFVF_DIFFUSE );
   GetD3DDevice()->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );

   GetD3DDevice()->SetPixelShader( NULL );
   GetD3DDevice()->SetTexture( 0, NULL );

   GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &vertices[0], sizeof( SVertex ) );

   GetD3DDevice()->SetRenderState( D3DRS_VIEWPORTENABLE, TRUE );

   ForceVertexDataRebind();
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
                                                    CShaderVertexDataBinding binding, 
                                                    void const * pData, 
                                                    uint32 const numVertices)
{
   ForceVertexDataRebind();

   ++mMetrics.mNumRenderPrimitiveCalls;

   int vertexSize = 0;
   DWORD FVF = 0;

   if( binding.HasStream(kVDU_Position) )
   {
      vertexSize += sizeof(CVector3);
      FVF |= D3DFVF_XYZ;
   }

   if( binding.HasStream(kVDU_Normal) )
   {
      vertexSize += sizeof(CVector3);
      FVF |= D3DFVF_NORMAL;
   }

   if( binding.HasStream(kVDU_Color0) )
   {
      vertexSize += sizeof(uint32);
      FVF |= D3DFVF_DIFFUSE;
   }

   if( binding.HasStream(kVDU_TexCoord0) )
   {
      vertexSize += sizeof(CVector2);
      FVF |= D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0);
   }

   GetD3DDevice()->SetFVF(FVF);

   GetD3DDevice()->DrawPrimitiveUP(gD3DPrimitiveMapping[type], 
      CMeshChunk::GetPrimitiveCount(type, numVertices), 
      pData, 
      vertexSize );
}

//------------------------------------------------------------------------------------------

void CRenderBackend::SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const allowAniso)
{
   pTexture->SetCurrentTextureParams(texUnit);

   GetCachedDeviceWrapper()->SetTexture(texUnit, ((CTexture const *)pTexture)->GetTexture());//can not be inlined because of unknown CTexture class

   int maxAniso = 1;

   if(pTexture->GetMipCount() > 1 && allowAniso )
      maxAniso = 4;

   GetCachedDeviceWrapper()->SetSamplerState(texUnit, D3DSAMP_MAXANISOTROPY, maxAniso);
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::RenderQuad(real32 minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV)
{
   struct SVertex
   { 
      real32 x, y, z;
      real32 u, v;
   };

   GetD3DDevice()->SetFVF( D3DFVF_XYZ|D3DFVF_TEX1 );
   
#define RECTS_PER_VIEW 8
   SVertex vertices[3*RECTS_PER_VIEW];

   real32 currX = minX;
   real32 currY = maxY; // Y is flipped on purpose
   real32 currU = minU;
   real32 currV = minV;

   real32 deltaX = (maxX-minX)/(real32)RECTS_PER_VIEW;
   real32 deltaY = (minY-maxY); // Y is flipped on purpose
   real32 deltaU = (maxU-minU)/(real32)RECTS_PER_VIEW;
   real32 deltaV = (maxV-minV);

   int vertIdx = 0;
   for(int i = 0; i < RECTS_PER_VIEW; ++i)
   {
      // top-left vert
      {
         SVertex & vert = vertices[vertIdx];
         vert.x = currX;
         vert.y = currY;
         vert.z = z;
         vert.u = currU;
         vert.v = currV;
         ++vertIdx;
      }

      // top-right vert
      {
         SVertex & vert = vertices[vertIdx];
         vert.x = currX+deltaX;
         vert.y = currY;
         vert.z = z;
         vert.u = currU+deltaU;
         vert.v = currV;
         ++vertIdx;
      }

      // bottom-left vert
      {
         SVertex & vert = vertices[vertIdx];
         vert.x = currX;
         vert.y = currY+deltaY;
         vert.z = z;
         vert.u = currU;
         vert.v = currV+deltaV;
         ++vertIdx;
      }

      // bottom-right vert is implicitly defined

      currX += deltaX;
      currU += deltaU;
   }
   GetD3DDevice()->DrawPrimitiveUP( D3DPT_RECTLIST, RECTS_PER_VIEW, &vertices[0], sizeof(SVertex) );

   ((CRenderBackend*)this)->ForceVertexDataRebind();
}

//----------------------------------------------------------------------------

bool const SVertexDeclKey::operator < (SVertexDeclKey const & rhs) const
{
   if( mBinding < rhs.mBinding )
      return true;

   if( mBinding > rhs.mBinding )
      return false;

   // Check to allow reserved_vector 'capacity' loop optimization.
   BPE_ASSERT(mData.mAttributes.size() == mData.mAttributes.static_capacity, "size/capacity mismatch.");

   for( int i = 0; i < mData.mAttributes.static_capacity; ++i )
   {
      int const res = memcmp(&mData.mAttributes[i], &rhs.mData.mAttributes[i], sizeof(SVertexAttribute));

      if( res < 0 )
         return true;

      if( res > 0 )
         return false;
   }

   return false;
}

void CRenderBackend::SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2)
{
   CAPTURE_VERTEX_DATA(vertexDataBinding, vertexData);

   if ((vertexDataHash_1 == mBoundVertexDataHash_1) &&
      (vertexDataHash_2 == mBoundVertexDataHash_2))
      return;

   mBoundVertexDataHash_1 = vertexDataHash_1;
   mBoundVertexDataHash_2 = vertexDataHash_2;

   if( vertexData.mpCachedVertexDecl == NULL )
   {
      LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration = NULL;

      SVertexDeclKey const key(vertexDataBinding, vertexData);

      TDeclarationMap::const_iterator const foundDeclaration = mDeclarations.find(key);

      if( foundDeclaration != mDeclarations.end() )
      {
         vertexDeclaration = foundDeclaration->second.GetPtr();
      }
      else
      {
         CVertexData::TDeclaration vertexElements;
         vertexData.BuildDeclaration(vertexDataBinding, vertexElements);

         D3DVERTEXELEMENT9 const endElement = D3DDECL_END();
         vertexElements.push_back( endElement );

         D3DDeviceUncached()->CreateVertexDeclaration( &vertexElements[0], &vertexDeclaration );
         vertexDeclaration->AddRef();
         mDeclarations.insert(TDeclarationMap::value_type(key, TComPtr<IDirect3DVertexDeclaration9>(vertexDeclaration)));
      }
      vertexData.mpCachedVertexDecl = vertexDeclaration;
   }

   GetD3DDevice()->SetVertexDeclaration( vertexData.mpCachedVertexDecl );

   for( int i = 0; i < vertexData.mVertexBuffers.size(); ++i )
   {
      SVertexStream const & stream = vertexData.mVertexBuffers[i];
      stream.mpBuffer->Bind(i, stream.mOffset, stream.mStride);
   }
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetProjectionMatrix(CMatrix4 const &matrix) 
{ 
   mProjectionMatrix = matrix; 
   CRenderBackend *pThis = static_cast<CRenderBackend *>(this);
   pThis->FlushProjectionTimesViewMatrix();
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetCameraMatrix(CMatrix34 const &matrix)
{
   CRenderBackend *pThis = static_cast<CRenderBackend *>(this);

   pThis->mCameraMatrix = CMatrix4::FromMatrix34(matrix);
   pThis->mViewMatrix = pThis->mCameraMatrix.Inverse();

   pThis->FlushProjectionTimesViewMatrix();
}


//----------------------------------------------------------------------------

void CBaseRenderBackend::SetPerspectiveProjection(CAngle const & fov, 
                                                  real32 const aspect, 
                                                  real32 const minClip, 
                                                  real32 const maxClip )
{
   mFrustumNear = minClip;
   mFrustumFar = maxClip;

   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(
      CMatrix4::Translation(CVector3(mViewportJitterX, mViewportJitterY, 0.0f)) * CMatrix4::Perspective(fov, aspect, minClip, maxClip));
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetOrthographicProjection(real32 const width,
                                                   real32 const height,
                                                   real32 const zNear,
                                                   real32 const zFar)
{
   mFrustumNear = zNear;
   mFrustumFar = zFar;

   real32 const realWidth = (width < 0.0f) ? mRenderTargetWidth : width;
   real32 const realHeight = (height < 0.0f) ? mRenderTargetHeight : height;

   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::Orthographic(realWidth, realHeight, zNear, zFar));

}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetScreenSpaceOrtho()
{
   // 1-1 pixel mapping with 0,0 at screen top left
   real32 const offsetX = -1.0f;
   real32 const offsetY = 1.0f;
   real32 const nearZ = -1.0f;
   real32 const farZ = 1.0f;
   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicOffset(real32(GetViewWidth()), 
      real32(-GetViewHeight()),
      offsetX,
      offsetY,
      nearZ, farZ));

   SetCameraMatrix(CMatrix34::Identity());
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetUniformOrtho()
{
   real32 const width = 1.0f;
   real32 const height = -1.0f;
   real32 const offsetX = -1.0f;
   real32 const offsetY = 1.0f;
   real32 const nearZ = -1.0f;
   real32 const farZ = 1.0f;
   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicOffset(width, 
      height,
      offsetX,
      offsetY,
      nearZ, farZ));

   SetCameraMatrix(CMatrix34::Identity());
}

//----------------------------------------------------------------------------

void CRenderBackend::FlushProjectionTimesViewMatrix()
{
   mProjectionTimesViewMatrix = mProjectionMatrix * mViewMatrix;
}

//Threading support
//----------------------------------------------------------------------------

void CRenderBackend::SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
   typedef struct tagTHREADNAME_INFO
   {
      DWORD dwType;     // must be 0x1000
      LPCSTR szName;    // pointer to name (in user address space)
      DWORD dwThreadID; // thread ID (-1 = caller thread)
      DWORD dwFlags;    // reserved for future use, must be zero
   } THREADNAME_INFO;

   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = szThreadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( 0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info );
   }
   __except( EXCEPTION_CONTINUE_EXECUTION ) {
   }
}

void CRenderBackend::StartThreadSystem(TFnRenderThread nextThread)
{
   mpRenderThreadFunction = nextThread;

   mRT_ThreadDieEvent = CreateEvent(NULL, TRUE, FALSE, "mRT_ThreadDieEvent");
   mRT_DoneEvent = CreateEvent(NULL, TRUE, TRUE, "mRT_DoneEvent");
   mRT_GoEvent = CreateEvent(NULL, TRUE, FALSE, "mRT_GoEvent");

   // Give up owner ship over D3D device so other threads can get it.
   mpDirect3DDevice->ReleaseThreadOwnership();

   mRT_Thread = CreateThread(NULL, 0, RenderThread, this, CREATE_SUSPENDED, &mRT_ThreadID);
   SetThreadName(mRT_ThreadID, "mRT_Thread");
   
   //Be careful processor 2 is the same thread the IO runs on, don't use processor 2
   XSetThreadProcessor(mRT_Thread, 3);
   
   ResumeThread(mRT_Thread);
}

void CRenderBackend::RunThreadFunction(int nextParam)
{
   WaitForSingleObject(mRT_DoneEvent, INFINITE);

   mRenderThreadFunctionParameter = nextParam;

#if ENABLE_SEPERATE_RENDER_THREAD
   AcquireRenderThreadOwnership();
   BeginFrameResourceTick();
   ReleaseRenderThreadOwnership();
#endif

   ResetEvent(mRT_DoneEvent);
   SetEvent(mRT_GoEvent);
}

void CRenderBackend::WaitThreadFunction()
{
   WaitForSingleObject(mRT_DoneEvent, INFINITE);
}

bool CRenderBackend::TryWaitThreadFunction()
{
   if( WaitForSingleObject(mRT_DoneEvent, 0) == WAIT_TIMEOUT )
   {
      return false;
   }

   return true;
}

DWORD __stdcall CRenderBackend::RenderThread( LPVOID InThis )
{
   CRenderBackend* pThis = (CRenderBackend*)InThis;
   while( WaitForSingleObject(pThis->mRT_ThreadDieEvent, 0) == WAIT_TIMEOUT )
   {
      // Wait until render thread is woken up with more work to do.
      DWORD waitResult = WAIT_TIMEOUT;
      
      if( !gIsSuspended )
         waitResult = WaitForSingleObject(pThis->mRT_GoEvent, 65);
     
      pThis->AcquireRenderThreadOwnership();

      if( waitResult == WAIT_OBJECT_0 )
      {
         ResetEvent(pThis->mRT_GoEvent);

         if( pThis->mpRenderThreadFunction )
            pThis->mpRenderThreadFunction(pThis->mRenderThreadFunctionParameter);

         // Set event that rendering thread has completed it's work and is waiting to be woken up.
         SetEvent(pThis->mRT_DoneEvent);
      }
      else if( waitResult == WAIT_TIMEOUT )
      {
         if( pThis->mpRenderThreadFunction )
            pThis->mpRenderThreadFunction(-1);
      }

      pThis->ReleaseRenderThreadOwnership();
   }

   return 0;
}

//----------------------------------------------------------------------------

void CRenderBackend::BeginFrameResourceTick()
{
   InternalBeginFrameResourceTick();

   mArrayToRelease0.swap(g_ToReleaseArray);
   for( int ii=0;ii<mArrayToRelease2.size();++ii )
   {
      IDirect3DResource9 * pResource = mArrayToRelease2[ii];
      pResource->Release();
   }
   mArrayToRelease2.clear();
   mArrayToRelease2.swap(mArrayToRelease1);
   mArrayToRelease1.swap(mArrayToRelease0);

   for(int ii=0;ii<mArrayDynamicVertexBuffer_RT.size();++ii)
   {
      CDynamicVertexBuffer_RT * pDynamicVertexBuffer = mArrayDynamicVertexBuffer_RT[ii];
      pDynamicVertexBuffer->FrameReset();
   }

   for(int ii=0;ii<mArrayDynamicIndexBuffer_RT.size();++ii)
   {
      CDynamicIndexBuffer_RT * pDynamicIndexBuffer = mArrayDynamicIndexBuffer_RT[ii];
      pDynamicIndexBuffer->FrameReset();
   }

   //
   mpDynamicVertexBufferPool_RT->FrameReset();
   mpDynamicIndexBufferPool_RT->FrameReset();
   mpDynamicVertexBufferPool_UT->FrameReset();
   mpDynamicIndexBufferPool_UT->FrameReset();
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter)
{
   static const uint32 sMinFilterMap[] =
   {
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_POINT,
      D3DTEXF_LINEAR
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(sMinFilterMap) == CRenderBackend::kFM_Count);

   static const uint32 sMipFilterMap[] =
   {
      D3DTEXF_NONE,
      D3DTEXF_NONE,
      D3DTEXF_POINT,
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_LINEAR
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(sMipFilterMap) == CRenderBackend::kFM_Count);

   static const uint32 sMagFilterMap[] =
   {
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_POINT,
      D3DTEXF_LINEAR,
      D3DTEXF_POINT,
      D3DTEXF_ANISOTROPIC
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(sMagFilterMap) == CRenderBackend::kFM_Count);

   GetCachedDeviceWrapper()->SetSamplerState(texUnit, D3DSAMP_MINFILTER, sMinFilterMap[minFilter]);
   GetCachedDeviceWrapper()->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, sMipFilterMap[minFilter]);

   GetCachedDeviceWrapper()->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, sMagFilterMap[magFilter]);
}

//----------------------------------------------------------------------------

void CRenderBackend::SuspendGameRenderThread()
{
   ++gIsSuspended;
}

//----------------------------------------------------------------------------

void CRenderBackend::ResumeGameRenderThread()
{
   --gIsSuspended;
}

//----------------------------------------------------------------------------

void CRenderBackend::AcquireRenderThreadOwnership()
{
   mD3DThreadOwnerShip.Enter();
   mpDirect3DDevice->AcquireThreadOwnership();   
}

//----------------------------------------------------------------------------

void CRenderBackend::ReleaseRenderThreadOwnership()
{
   mpDirect3DDevice->ReleaseThreadOwnership();
   mD3DThreadOwnerShip.Leave();
}
