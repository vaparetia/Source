//----------------------------------------------------------------------------
// Win32CRenderBackend.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Primitive/Win32/Win32CIndexBuffer.h"
#include "Renderer/Base/Primitive/Win32/Win32CVertexBuffer.h"
#include "Renderer/Base/Backend/Win32/Win32CTexture.h"
#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"
#include "Renderer/Base/CFrameCapture.h"

#include "Engine/Input/CInputGenerator.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CTaskQueue.h"

//----------------------------------------------------------------------------

using namespace std;

#pragma warning ( disable : 4355 )

//----------------------------------------------------------------------------

//#define DEBUG_PIXEL_SHADERS
//#define DEBUG_VERTEX_SHADERS

//----------------------------------------------------------------------------

void CGPUProfileMarker::PushMarker(const char * pName)
{
   int const kBufferSize = 1024;
   static wchar_t wideString[kBufferSize];
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pName, -1, wideString, kBufferSize);

   D3DPERF_BeginEvent(0xFFFFFFFF, wideString);
}

//----------------------------------------------------------------------------

void CGPUProfileMarker::PopMarker()
{
   D3DPERF_EndEvent();
}

//----------------------------------------------------------------------------

void CGPUProfileMarker::Event(const char * pName)
{
   int const kBufferSize = 1024;
   static wchar_t wideString[kBufferSize];
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pName, -1, wideString, kBufferSize);

   D3DPERF_SetMarker(0xFFFFFFFF, wideString);
}

//----------------------------------------------------------------------------

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   return RenderBackend()->MsgProc( hWnd, uMsg, wParam, lParam );
}

//----------------------------------------------------------------------------
   
LRESULT CRenderBackend::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch( uMsg )
   {
      case WM_SETCURSOR:
         {
            // Hide windows cursor if fullscreen, otherwise set the cursor the default arrow.
            if( mPresentParameters.Windowed )
            {               
               SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            else
            {
               SetCursor(NULL);
            }
            return TRUE;
         }
         break;

      case WM_CHAR:
         {
            //InputGenerator()->AddEvent( CInputEvent( CInputEvent::kType_Char, static_cast<int>( wParam ) ) );
            return 0;
         }
         break;

      case WM_KEYDOWN:
         {
            //InputGenerator()->AddEvent(CInputEvent(CInputEvent::kType_KeyDown, CBaseInputGenerator::ConvertVirtualKey(static_cast<int>(wParam))));
            return 0;
         }
         break;

      case WM_KEYUP:
         {
            //InputGenerator()->AddEvent(CInputEvent(CInputEvent::kType_KeyUp, CBaseInputGenerator::ConvertVirtualKey(static_cast<int>(wParam))));
            return 0;
         }
         break;
         
      case WM_CLOSE:
         StoreWindowPosition();
         OsContext()->mShouldTerminateApplication = true;
         break;

      case WM_DESTROY:
         OsContext()->mShouldTerminateApplication = true;
         break;

      case WM_LBUTTONUP:
      case WM_LBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_ERASEBKGND:
      case WM_PAINT:
         {
            if( mHostedInWindow )
            {
               HWND parentWindow = ::GetParent(hWnd);
               SendMessage(parentWindow, uMsg, wParam, lParam);
            }
         }
         break;

      case WM_SIZE:
         {
         }
         break;
   }

   return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//----------------------------------------------------------------------------

void CRenderBackend::StoreWindowPosition() const
{
   if (((mInitFlags.mCreationFlags & kCF_DontStoreWindowPosition) == 0) &&
      ((mInitFlags.mCreationFlags & kCF_Fullscreen) == 0))
   {
      if( ::IsIconic((HWND)OsContext()->mMainWindow) == 0 )
      {
         RECT rect;
         ::GetWindowRect((HWND)OsContext()->mMainWindow, &rect);
         std::string const appPath(CSystemVar::GetAppRegistryPath());
         CSystemVar::SetRegString(appPath.c_str(), "d3dWinPosX", CStringExtras::Stringize_s("%d", rect.left));
         CSystemVar::SetRegString(appPath.c_str(), "d3dWinPosY", CStringExtras::Stringize_s("%d", rect.top));
      }
   }
}

//----------------------------------------------------------------------------

CRenderBackend::CRenderBackend(IResourcePool & resourcePool,
                               SRenderInitialization const &initFlags)

:  CBaseRenderBackend(resourcePool, initFlags)
,  mpDirect3D(NULL)
,  mpDirect3DDevice(NULL)
,  mpBackBuffer(NULL)
,  mCachedDeviceWrapper(NULL)
,  mHostedInWindow(false)
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
{
   // Assume the first one provided is the preferred one for Win32.
   if( initFlags.mFrameBufferSizeCount > 0 )
   {
      mFrameBufferSizeMono = initFlags.mpFrameBufferSize[0];
   }

   bool const fullscreen = (mInitFlags.mCreationFlags & kCF_Fullscreen) != 0;
   
   BPE_VERIFY(mInitFlags.mWindowHandle == NULL, false, "Not currently supported");

   // Register the window class.
   WNDCLASSEX wc = 
   {
      sizeof(WNDCLASSEX), 
      CS_CLASSDC, 
      ::MsgProc, 
      0L, 
      0L, 
      GetModuleHandle( NULL ), 
      NULL, 
      NULL, 
      NULL, 
      NULL,
      "CSD3DWND", 
      NULL
   };

   RegisterClassEx( &wc );

   // Create the application's window.
   DWORD flags = 0;

   if( mHostedInWindow )
      flags |= WS_CHILD;
   else
      flags |= WS_OVERLAPPEDWINDOW;

     // We're going to reposition the window if not fullscreen or hosted inside another window, so don't show it now
   if( fullscreen || mHostedInWindow )
   {
      flags |= WS_VISIBLE;
   }

   OsContext()->mMainWindow = CreateWindow("CSD3DWND", 
                                           "Bluepoint Engine", 
                                           flags,
                                           0, 0, GetBackBufferWidth(), GetBackBufferHeight(),
                                           mHostedInWindow ? (HWND)mInitFlags.mWindowHandle : GetDesktopWindow(), 
                                           NULL, 
                                           wc.hInstance, 
                                           NULL);

   if( !fullscreen && !mHostedInWindow)
   {
      RECT rect;
      RECT clientRect;
      ::GetWindowRect((HWND)OsContext()->mMainWindow, &rect);
      ::GetClientRect((HWND)OsContext()->mMainWindow, &clientRect);

      // Get the previous position
      int posX = 100;
      int posY = 100;

      UINT windowFlags = SWP_NOZORDER | SWP_SHOWWINDOW;
      if ((mInitFlags.mCreationFlags & kCF_DontStoreWindowPosition) == 0)
      {
         std::string const appPath(CSystemVar::GetAppRegistryPath());
         std::string const previousPosX(CSystemVar::GetRegString(appPath.c_str(), "d3dWinPosX"));
         std::string const previousPosY(CSystemVar::GetRegString(appPath.c_str(), "d3dWinPosY"));
         if (previousPosX.size() && previousPosY.size())
         {
            posX = atoi(previousPosX.c_str());
            posY = atoi(previousPosY.c_str());
         }
      }
      else
      {
         // We're not repositioning the window
         windowFlags |= SWP_NOMOVE;
      }

      ::SetWindowPos((HWND)OsContext()->mMainWindow, NULL, posX, posY, 
                     GetBackBufferWidth() + (rect.right - rect.left) - (clientRect.right - clientRect.left),
                     GetBackBufferHeight() + (rect.bottom - rect.top) - (clientRect.bottom - clientRect.top),
                     windowFlags );
   }

   mpDirect3D = TComPtr<IDirect3D9>(Direct3DCreate9(D3D_SDK_VERSION));
   
   // Set up the structure used to create the D3DDevice.
   ZeroMemory( &mPresentParameters, sizeof(D3DPRESENT_PARAMETERS) );
   
   mPresentParameters.BackBufferWidth        = GetBackBufferWidth();
   mPresentParameters.BackBufferHeight       = GetBackBufferHeight();
   mPresentParameters.BackBufferFormat       = D3DFMT_X8R8G8B8;
   
   // Create one backbuffer and a zbuffer
   mPresentParameters.BackBufferCount        = 2;
   mPresentParameters.EnableAutoDepthStencil = FALSE;
   mPresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;

   mPresentParameters.hDeviceWindow = (HWND)OsContext()->mMainWindow;
   mPresentParameters.Windowed = !fullscreen;
   
   // Set up how the backbuffer is "presented" to the frontbuffer each frame
   mPresentParameters.SwapEffect             = D3DSWAPEFFECT_DISCARD;
   
   if ((mInitFlags.mCreationFlags & kCF_WaitForVSync) != 0)
   {
      mPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
   }
   else
   {
      mPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
   }
   mPresentParameters.FullScreen_RefreshRateInHz = fullscreen ? 60 : 0;

   UINT adapterToUse = D3DADAPTER_DEFAULT;
   D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;
   
#define PERFHUD_SUPPORT

   // get monitor handle associated with the window that we created.
   HMONITOR windowMonitor = MonitorFromWindow((HWND)OsContext()->mMainWindow, MONITOR_DEFAULTTONEAREST);

   // Look for 'NVIDIA NVPerfHUD' adapter
   // If it is present, override default settings
   for( int adapter = 0; adapter < mpDirect3D->GetAdapterCount(); ++adapter)
   {
      D3DADAPTER_IDENTIFIER9 identifier;
      mpDirect3D->GetAdapterIdentifier(adapter, 0, &identifier);

      // this exposes the refresh rate used by the display, theoretically we should be 
      // setting "mRefreshRate" to this value (if we use this adapter), but we don't have any CRTs to test this on right now.
      D3DDISPLAYMODE displayMode;
      mpDirect3D->GetAdapterDisplayMode(adapter, &displayMode);

#ifdef PERFHUD_SUPPORT
      if (strstr(identifier.Description, "PerfHUD") != NULL)
      {
         adapterToUse = adapter;
         deviceType = D3DDEVTYPE_REF;
         break;
      }
#endif

      // get monitor handle for this adapter
      HMONITOR monitor = mpDirect3D->GetAdapterMonitor(adapter);
      
      // if the adapter monitor handle and our window monitor handle match up, we found the right adapter!
      if( monitor == windowMonitor )
      {
         adapterToUse = adapter;
         // don't break out here and keep searching anyway, just in case we find a Perf hud adapter!
      }
   }

   DWORD behaviorFlags = 0;
#ifdef DEBUG_VERTEX_SHADERS
   behaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING; 
#else
   behaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
#endif
   IDirect3DDevice9* pDevice = NULL;
   HRESULT result = mpDirect3D->CreateDevice( adapterToUse, 
#ifdef DEBUG_PIXEL_SHADERS
                             D3DDEVTYPE_REF, 
#else
                             deviceType, 
#endif
                             (HWND)OsContext()->mMainWindow,
                             D3DCREATE_FPU_PRESERVE | D3DCREATE_PUREDEVICE |
                             behaviorFlags,
                             &mPresentParameters, 
                             &pDevice );

   if( FAILED(result) )
   {
      bpe_message_box("Error initializing renderer (Can't create Direct3dDevice, might be cause of resolution)", "BPE Renderer Error");
      exit(0);
   }

   mpDirect3DDevice = TComPtr<IDirect3DDevice9>(pDevice);
   mCachedDeviceWrapper.reset(new CD3DCachedDevice(mpDirect3DDevice));
   
   RestoreObjects();

   InitDefaultObjects();

   mpDynamicVertexBufferPool_RT.reset(new CDynamicVertexBufferPool_RT(initFlags.mVertexBufferPoolSize, initFlags.mVertexBufferPoolChunkCount));
   mpDynamicIndexBufferPool_RT.reset(new CDynamicIndexBufferPool_RT(initFlags.mIndexBufferPoolSize, initFlags.mIndexBufferPoolChunkCount));
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

   ::CloseWindow((HWND)OsContext()->mMainWindow);
}

//----------------------------------------------------------------------------

void CRenderBackend::InitDefaultState()
{
   SetPerspectiveProjection();
}

//----------------------------------------------------------------------------

real32 const CRenderBackend::GetFullScreenAspectRatio()
{
   // Will use the display device associated with this thread
   // Might go a bit funny on multi-mon where window is initial positioned on a
   // different display device to the full screen device.
   DEVMODE maxRes;
   DEVMODE temp;
   temp.dmSize = sizeof(DEVMODE);
   for (DWORD loop = 0; EnumDisplaySettings(NULL, loop, &temp); loop++)
   {
      if ((loop == 0) || ((temp.dmPelsHeight >= maxRes.dmPelsHeight) && (temp.dmPelsWidth > maxRes.dmPelsWidth)))
      {
         // Highest res
         memcpy(&maxRes, &temp, sizeof(DEVMODE));
      }
   }

   // Assumes the monitor has square pixels
   real32 const aspectRatio = maxRes.dmPelsWidth / (real32) maxRes.dmPelsHeight;
   return aspectRatio;
}

//----------------------------------------------------------------------------

real32 const CRenderBackend::GetClientWindowAspectRatio()
{
   RECT clientRect;
   ::GetClientRect((HWND)OsContext()->mMainWindow, &clientRect);
   return (clientRect.right - clientRect.left) / static_cast<real32>(clientRect.bottom - clientRect.top);
}

//----------------------------------------------------------------------------

TComPtr<IDirect3DDevice9>& CRenderBackend::GetDirect3DDevice()
{
   return mpDirect3DDevice;
}

//----------------------------------------------------------------------------

void CRenderBackend::InvalidateObjects()
{
   mpBackBuffer = TComPtr<IDirect3DSurface9>(NULL);
   
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
      mpBackBuffer = TComPtr<IDirect3DSurface9>( pBackBuffer );
   }
   
   InitDefaultState();
}

//----------------------------------------------------------------------------

void CRenderBackend::BeginScene()
{
   InternalBeginScene();

   InternalBeginFrameResourceTick();

   if( !mFirstFrame )
   {
      BPE_ASSERT(!GetDirect3DDevice().IsNull(), "no direct 3d device");

      CStopWatch timer;
      HRESULT result = D3DDeviceUncached()->Present( NULL, NULL, NULL, NULL );
      mMetrics.mVBlankWaitTime = timer.GetElapsedTime();

      if( result == D3DERR_DEVICELOST )
      {
         result = D3DDeviceUncached()->TestCooperativeLevel();
         if( result == D3DERR_DEVICENOTRESET )
         {
            while( FAILED( result ) )
            {
               InvalidateObjects();

               D3DDeviceUncached()->EvictManagedResources();

               result = D3DDeviceUncached()->Reset( &mPresentParameters );

               if( SUCCEEDED( result ) )
               {
                  RestoreObjects();
               }
            }
         }
      }
      
      if( FAILED( result ) )
         return;
   }
   
   mFirstFrame = false;

   D3DDevice()->BeginScene();

   CAPTURE_BEGIN();
}

//----------------------------------------------------------------------------

void CRenderBackend::EndScene()
{
   CAPTURE_END();

   D3DDevice()->EndScene();
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
      D3DDeviceUncached()->Clear( 0, NULL, d3dClearFlags, color.GetARGB(), z, stencil );
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::Present()
{
   InternalPresent();

   MSG msg; 

   if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
   {
      // Check for a quit message
      if( msg.message != WM_QUIT )
      {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
   }

   mpDynamicVertexBufferPool_RT->FrameReset();
   mpDynamicIndexBufferPool_RT->FrameReset();
}

void CRenderBackend::ShutDown()
{
   Present();
}

//----------------------------------------------------------------------------
   
void CRenderBackend::SetRenderTarget(SRenderTarget const & renderTarget)
{
   // early out
   if( mCurrentRenderTarget == renderTarget )
      return;

   mCurrentRenderTarget = renderTarget;

   bool validColorSurfaces = false;

   LPDIRECT3DSURFACE9 pRenderTargetSurfaces[] = { NULL, NULL, NULL, NULL };
   for( int i = 0; i < BPE_ARRAY_SIZE(mCurrentRenderTarget.mpColorBuffer); ++i )
   {
      CTexture* pColorBuffer = (CTexture*)mCurrentRenderTarget.mpColorBuffer[i];

      if(pColorBuffer == NULL )
         break;
      
      validColorSurfaces = true;
      pRenderTargetSurfaces[i] = pColorBuffer->Surface().GetPtr();
   }

   bool boundBackBuffer = false;

   // only bind device back buffer if user isn't binding a depth buffer (if user only binds depth buffer thats ok)
   if( !validColorSurfaces && renderTarget.mBindBackBuffer != SRenderTarget::kBBM_NoBackBuffer )
   {
      pRenderTargetSurfaces[0] = mpBackBuffer.GetPtr();
      boundBackBuffer = true;
   }

   LPDIRECT3DSURFACE9 pDepthBufferSurface = NULL;
   if( mCurrentRenderTarget.mpDepthBuffer )
   {
      pDepthBufferSurface = ((CTexture*)mCurrentRenderTarget.mpDepthBuffer)->Surface().GetPtr();
   }

   // get dimensions of current render target
   {
      D3DSURFACE_DESC renderTargetDesc;
      if( pRenderTargetSurfaces[0] != NULL )
      {
         pRenderTargetSurfaces[0]->GetDesc( &renderTargetDesc );
         mRenderTargetWidth = renderTargetDesc.Width;
         mRenderTargetHeight = renderTargetDesc.Height;
      }
      else if( pDepthBufferSurface != NULL )
      {
         pDepthBufferSurface->GetDesc( &renderTargetDesc );
         mRenderTargetWidth = renderTargetDesc.Width;
         mRenderTargetHeight = renderTargetDesc.Height;
      }
   }

   HRESULT res;
   for( int i = 0; i < BPE_ARRAY_SIZE(pRenderTargetSurfaces); ++i )
   {
      res = D3DDevice()->SetRenderTarget(i, pRenderTargetSurfaces[i]);
   }

   res = D3DDevice()->SetDepthStencilSurface( pDepthBufferSurface );

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

   D3DDevice()->SetViewport( &vp );

   if( renderTarget.mScissorEnabled )
   {
      D3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
      
      RECT scissorRect;

      scissorRect.left = renderTarget.mScissorX;
      scissorRect.right = renderTarget.mScissorX + renderTarget.mScissorWidth;

      scissorRect.top = renderTarget.mScissorY;
      scissorRect.bottom = renderTarget.mScissorY + renderTarget.mScissorHeight;

      D3DDeviceUncached()->SetScissorRect(&scissorRect);
   }
   else
   {
      D3DDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
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

   D3DDevice()->SetRenderState( D3DRS_ZENABLE, FALSE );
   D3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
   D3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
   D3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

   D3DDevice()->SetVertexShader( NULL );
   D3DDevice()->SetFVF( D3DFVF_XYZRHW|D3DFVF_DIFFUSE );

   D3DDevice()->SetPixelShader( NULL );
   D3DDevice()->SetTexture( 0, NULL );

   D3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &vertices[0], sizeof( SVertex ) );

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

   D3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
   D3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
   D3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

   D3DDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
   D3DDevice()->SetVertexShader( NULL );
   D3DDevice()->SetFVF( D3DFVF_XYZRHW|D3DFVF_DIFFUSE );

   D3DDevice()->SetPixelShader( NULL );
   D3DDevice()->SetTexture( 0, NULL );

   D3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &vertices[0], sizeof( SVertex ) );

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
   
   D3DDevice()->SetFVF(FVF);
   
   D3DDevice()->DrawPrimitiveUP(gD3DPrimitiveMapping[type], 
                                CMeshChunk::GetPrimitiveCount(type, numVertices), 
                                pData, 
                                vertexSize );
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const /*allowAniso*/)
{
   pTexture->SetCurrentTextureParams(texUnit);

   D3DDevice()->SetTexture(texUnit, ((CTexture const *)pTexture)->GetTexture().GetPtr());

   CAPTURE_TEXTURE(texUnit, (CTexture const *)pTexture);
}

//----------------------------------------------------------------------------

static const uint32 gTextureAddressMap[] = 
{
   D3DTADDRESS_WRAP,
   D3DTADDRESS_MIRROR,
   D3DTADDRESS_CLAMP,
   D3DTADDRESS_BORDER
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gTextureAddressMap) == CRenderBackend::kWM_Count);

void CRenderBackend::SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV)
{
   CAPTURE_TEXTURE_ADDRESS(texUnit, wrapU, wrapV);
   D3DDevice()->SetSamplerState(texUnit, D3DSAMP_ADDRESSU, gTextureAddressMap[wrapU]);
   D3DDevice()->SetSamplerState(texUnit, D3DSAMP_ADDRESSV, gTextureAddressMap[wrapV]);
}

//----------------------------------------------------------------------------

static const uint32 gMinFilterMap[] =
{
   D3DTEXF_POINT,
   D3DTEXF_LINEAR,
   D3DTEXF_POINT,
   D3DTEXF_LINEAR,
   D3DTEXF_POINT,
   D3DTEXF_LINEAR
};
BPE_CTASSERT(BPE_ARRAY_SIZE(gMinFilterMap) == CRenderBackend::kFM_Count);

static const uint32 gMipFilterMap[] =
{
   D3DTEXF_NONE,
   D3DTEXF_NONE,
   D3DTEXF_POINT,
   D3DTEXF_POINT,
   D3DTEXF_LINEAR,
   D3DTEXF_LINEAR
};
BPE_CTASSERT(BPE_ARRAY_SIZE(gMipFilterMap) == CRenderBackend::kFM_Count);

static const uint32 gMagFilterMap[] =
{
   D3DTEXF_POINT,
   D3DTEXF_LINEAR,
   D3DTEXF_POINT,
   D3DTEXF_LINEAR,
   D3DTEXF_POINT,
   D3DTEXF_LINEAR
};
BPE_CTASSERT(BPE_ARRAY_SIZE(gMagFilterMap) == CRenderBackend::kFM_Count);

void CRenderBackend::SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter)
{
   D3DDevice()->SetSamplerState(texUnit, D3DSAMP_MINFILTER, gMinFilterMap[minFilter]);
   D3DDevice()->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, gMipFilterMap[minFilter]);

   D3DDevice()->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, gMagFilterMap[magFilter]);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector)
{
   D3DDevice()->SetVertexShaderConstantF(startRegister, (real32 const*)pVector, numVectors);
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::RenderQuad(real32 minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV)
{
   struct SVertex
   { 
      real32 x, y, z;
      real32 u, v;
   };

   SVertex const vertices[] =
   {
      { minX, maxY, z, minU, minV },
      { maxX, maxY, z, maxU, minV },
      { minX, minY, z, minU, maxV },
      { maxX, minY, z, maxU, maxV }
   };

   D3DDevice()->SetFVF( D3DFVF_XYZ|D3DFVF_TEX1 );
   D3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertices[0], sizeof(SVertex) );

   ((CRenderBackend*)this)->ForceVertexDataRebind();
}

//----------------------------------------------------------------------------

void CRenderBackend::SetIndexData(CIndexBuffer const * indexBuffer)
{
   mBoundIndexBufferOffset = 0;
   D3DDevice()->SetIndices(indexBuffer->mBuffer.GetPtr());
   CAPTURE_INDEX_BUFFER(indexBuffer);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetIndexData(CDynamicIndexBufferPoolChunk_RT const * pChunk)
{
   CDynamicIndexBufferPool_RT const * pPool = pChunk->GetPool();
   mBoundIndexBufferOffset = pChunk->GetStartIndex();
   D3DDevice()->SetIndices(pPool->GetIndexBuffer()->mBuffer.GetPtr());
}

//----------------------------------------------------------------------------

void CRenderBackend::SetIndexData(CDynamicIndexBufferPoolChunk_UT const * pChunk)
{
   CDynamicIndexBufferPool_UT const * pPool = pChunk->GetPool();
   mBoundIndexBufferOffset = pChunk->GetStartIndex();
   D3DDevice()->SetIndices(pPool->GetIndexBuffer()->mBuffer.GetPtr());
}

//----------------------------------------------------------------------------

// Forces the cached vertex declaration to get reset

void CRenderBackend::ForceVertexDataRebind()
{
   mBoundVertexDataHash_1 = 0xDEADBEEF;
   mBoundVertexDataHash_2 = 0xFEE7C0DE;
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

   D3DDevice()->SetVertexDeclaration( vertexDeclaration );

   for( int i = 0; i < vertexData.mVertexBuffers.size(); ++i )
   {
      SVertexStream const & stream = vertexData.mVertexBuffers[i];
      stream.mpBuffer->Bind(i, stream.mOffset, stream.mStride);
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitives(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount, uint32 const indexBufferOffset, uint32 const indexCount)
{
   ++mMetrics.mNumRenderPrimitiveCalls;

   D3DDevice()->DrawIndexedPrimitive(gD3DPrimitiveMapping[type],
                                     0,
                                     vertexBufferOffset, 
                                     vertexCount,  
                                     mBoundIndexBufferOffset + indexBufferOffset, 
                                     CMeshChunk::GetPrimitiveCount(type, indexCount) );

   CAPTURE_PRIMITIVES(type, vertexBufferOffset, vertexCount, indexBufferOffset, indexCount);
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount)
{
   ++mMetrics.mNumRenderPrimitiveCalls;

   D3DDevice()->DrawPrimitive(gD3DPrimitiveMapping[type], vertexBufferOffset, CMeshChunk::GetPrimitiveCount(type, vertexCount));

   CAPTURE_PRIMITIVES_NOINDICES(type, vertexBufferOffset, vertexCount);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetShadeMode(int isSmooth)
{
   D3DDevice()->SetRenderState(D3DRS_SHADEMODE, isSmooth ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
}

//----------------------------------------------------------------------------

static const uint32 gAlphaFuncMap[] = 
{
   D3DCMP_NEVER,
   D3DCMP_LESS,
   D3DCMP_EQUAL,
   D3DCMP_LESSEQUAL,
   D3DCMP_GREATER,
   D3DCMP_NOTEQUAL,
   D3DCMP_GREATEREQUAL,
   D3DCMP_ALWAYS
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gAlphaFuncMap) == CRenderBackend::kAF_Count);

void CRenderBackend::SetAlphaTestEnable(bool enable)
{
   D3DDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, enable);
}

void CRenderBackend::SetAlphaFunc(EAlphaFunc func, int refValue)
{
   D3DDevice()->SetRenderState(D3DRS_ALPHAFUNC, gAlphaFuncMap[func]);
   D3DDevice()->SetRenderState(D3DRS_ALPHAREF, refValue);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetStencilEnable(bool enable)
{
   D3DDevice()->SetRenderState(D3DRS_STENCILENABLE, enable);
}

void CRenderBackend::SetBackStencilEnable(bool enable)
{
   D3DDevice()->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, enable);
}

static const uint32 gStencilOpMap[] = 
{
   D3DSTENCILOP_KEEP,
   D3DSTENCILOP_ZERO,
   D3DSTENCILOP_REPLACE,
   D3DSTENCILOP_INCRSAT,
   D3DSTENCILOP_DECRSAT,
   D3DSTENCILOP_INCR,
   D3DSTENCILOP_DECR,
   D3DSTENCILOP_INVERT
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gStencilOpMap) == CRenderBackend::kSO_Count);

void CRenderBackend::SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass )
{
   D3DDevice()->SetRenderState(D3DRS_STENCILFAIL, gStencilOpMap[fail]);
   D3DDevice()->SetRenderState(D3DRS_STENCILZFAIL, gStencilOpMap[depthFail]);
   D3DDevice()->SetRenderState(D3DRS_STENCILPASS, gStencilOpMap[depthPass]);
}

void CRenderBackend::SetBackStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass )
{
   D3DDevice()->SetRenderState(D3DRS_CCW_STENCILFAIL, gStencilOpMap[fail]);
   D3DDevice()->SetRenderState(D3DRS_CCW_STENCILZFAIL, gStencilOpMap[depthFail]);
   D3DDevice()->SetRenderState(D3DRS_CCW_STENCILPASS, gStencilOpMap[depthPass]);
}

void CRenderBackend::SetStencilMask(uint32 mask)
{
   D3DDevice()->SetRenderState(D3DRS_STENCILWRITEMASK, mask);
}

void CRenderBackend::SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask)
{
   D3DDevice()->SetRenderState(D3DRS_STENCILFUNC, gAlphaFuncMap[func]);
   D3DDevice()->SetRenderState(D3DRS_STENCILREF, ref);
   D3DDevice()->SetRenderState(D3DRS_STENCILMASK, mask);
}

void CRenderBackend::SetBackStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask)
{
   D3DDevice()->SetRenderState(D3DRS_CCW_STENCILFUNC, gAlphaFuncMap[func]);
   D3DDevice()->SetRenderState(D3DRS_STENCILREF, ref);
   D3DDevice()->SetRenderState(D3DRS_STENCILMASK, mask);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetColorAlphaWriteEnabled(bool const color, bool const alpha)
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
   D3DDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, mask);
}

//----------------------------------------------------------------------------

// must match CBaseRenderBackend::EBlendFunc order
static const uint32 gBlendFuncMap[] = 
{
   D3DBLEND_ZERO, 

   D3DBLEND_ONE,  

   D3DBLEND_SRCCOLOR,
   D3DBLEND_INVSRCCOLOR,

   D3DBLEND_DESTCOLOR,
   D3DBLEND_INVDESTCOLOR,

   D3DBLEND_SRCALPHA,
   D3DBLEND_INVSRCALPHA,

   D3DBLEND_DESTALPHA,
   D3DBLEND_INVDESTALPHA,

   D3DBLEND_BLENDFACTOR,      // NOTE: D3D does not support separate alpha/color blend factor operations, only (RGBA * BlendFactorRGBA) or (RGBA * InvBlendFactorRGBA).
   D3DBLEND_INVBLENDFACTOR,

   D3DBLEND_BLENDFACTOR,
   D3DBLEND_INVBLENDFACTOR
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gBlendFuncMap) == CRenderBackend::kBF_Count);

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode)
{
   if( enable )
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);

      D3DDevice()->SetRenderState(D3DRS_SRCBLEND, gBlendFuncMap[srcBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_DESTBLEND, gBlendFuncMap[dstBlendMode]);
   }
   else
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode)
{
   if( enable )
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
      
      D3DDevice()->SetRenderState(D3DRS_SRCBLEND, gBlendFuncMap[rgbSrcBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_DESTBLEND, gBlendFuncMap[rgbDstBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_SRCBLENDALPHA, gBlendFuncMap[aSrcBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_DESTBLENDALPHA, gBlendFuncMap[aDstBlendMode]);
   }
   else
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode, const uint32 rgbaFixed)
{
   if( enable )
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);

      D3DDevice()->SetRenderState(D3DRS_SRCBLEND, gBlendFuncMap[srcBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_DESTBLEND, gBlendFuncMap[dstBlendMode]);

      D3DDevice()->SetRenderState(D3DRS_BLENDFACTOR, rgbaFixed);
   }
   else
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode, const uint32 rgbaFixed)
{
   if( enable )
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

      D3DDevice()->SetRenderState(D3DRS_SRCBLEND, gBlendFuncMap[rgbSrcBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_DESTBLEND, gBlendFuncMap[rgbDstBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_SRCBLENDALPHA, gBlendFuncMap[aSrcBlendMode]);
      D3DDevice()->SetRenderState(D3DRS_DESTBLENDALPHA, gBlendFuncMap[aDstBlendMode]);

      D3DDevice()->SetRenderState(D3DRS_BLENDFACTOR, rgbaFixed);
   }
   else
   {
      D3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      D3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
   }
}

//----------------------------------------------------------------------------

static const uint32 gBlendOpMap[] = 
{
   D3DBLENDOP_ADD, 
   D3DBLENDOP_SUBTRACT,
   D3DBLENDOP_REVSUBTRACT,
   D3DBLENDOP_MIN,
   D3DBLENDOP_MAX
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gBlendOpMap) == CRenderBackend::kBO_Count);

void CRenderBackend::SetBlendOp(EBlendOp const blendOp)
{
   D3DDevice()->SetRenderState(D3DRS_BLENDOP, gBlendOpMap[blendOp]);
   D3DDevice()->SetRenderState(D3DRS_BLENDOPALPHA, gBlendOpMap[blendOp]);
}

void CRenderBackend::SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp)
{
   D3DDevice()->SetRenderState(D3DRS_BLENDOP, gBlendOpMap[colorBlendOp]);
   D3DDevice()->SetRenderState(D3DRS_BLENDOPALPHA, gBlendOpMap[alphaBlendOp]);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetDepthCompareEnabled(bool const enable)
{
   D3DDevice()->SetRenderState(D3DRS_ZENABLE, enable ? TRUE : FALSE);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetDepthFunc(EDepthFunc const func)
{
   D3DDevice()->SetRenderState(D3DRS_ZFUNC, gAlphaFuncMap[func]);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetDepthWriteEnabled(bool const enable)
{
   D3DDevice()->SetRenderState(D3DRS_ZWRITEENABLE, enable ? TRUE : FALSE);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetCullMode(ECullMode const mode)
{
   switch(mode)
   {
   case kCM_None:
      D3DDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      break;
   case kCM_CW:
      D3DDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
      break;
   case kCM_CCW:
      D3DDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
      break;
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetLineWidth(real32 const width)
{
   // Not supported by D3D
}

//----------------------------------------------------------------------------

void CRenderBackend::SetLineSmoothEnabled(bool const enable)
{
   D3DDevice()->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, enable ? TRUE : FALSE);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color )
{
   mFogColor = color;
   mFogMode = fogMode;

   float fogScale = 1.0f/(farZ - nearZ);
   float fogParam0 = farZ * fogScale + 1.0f;
   float fogParam1 = -fogScale;

   mFogParams = CVector2( fogParam0, fogParam1 );
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

//----------------------------------------------------------------------------

void CRenderBackend::BeginTiling(SRenderTarget const & renderTarget)
{
}

//----------------------------------------------------------------------------

void CRenderBackend::EndTiling()
{
}
