//----------------------------------------------------------------------------
// BP_Renderer.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "sysutil/sysutil_screenshot.h"
#include "sysutil/sysutil_sysparam.h"
#endif

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/CMTXFont.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/FrontEnd/RenderTypes.h"

#include "BP_BaseRenderer.h"

#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/System/COSContext.h"
#include "Engine/System/CSPURSManager.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Resource/CResourceCache.h"
#include "Engine/Mechanics/zlib/zlib.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"

#include "BP_RenderDmaPack.h"
#include "BP_RenderGS.h"

#include "BP_RenderPrim.h"
#include "BP_DebugPad.h"
#include "BP_Debug.h"
#include "BP_SoundSupport.h"
#include "BP_Memory.h"
#include "BP_Camera.h"
#include "BP_Misc.h"
#include "BP_SaveLoadMGS.h"
#include "BP_SplashScreen.h"
#include "BP_TrophySystem.h"
#include "BP_TUS.h"
#include "BP_Network.h"
#include "BP_CommonDialog.h"

#if MGS_VERSION == 2
#include "BP_RenderObj.h"
#include "BP_RenderObjOptCmf.h"
#include "BP_RenderEvm.h"
#include "BP_RenderRadar.h"
#include "BP_RenderPatch.h"
#include "BP_RenderComdl.h"
#include "BP_RenderProjector.h"
#elif MGS_VERSION == 3
#include "BP_DebugCulling.h"
#include "BP_DebugCollision.h"
#include "BP_RenderFixModel.h"
#include "BP_RenderModel.h"
#include "BP_RenderClone.h"
#include "BP_RenderOptCmf.h"
#include "BP_RenderProjector.h"
#include "BP_Preshade.h"
#include "BP_LayoutFont.h"
#endif

#include "BP_RichPresenceSystem.h"
#include "BP_MovieSupport.h"

#include "BP_RenderFX.h"
#include "BP_DebugPrim.h"

#include "BP_FileSupport.h"
#include	"BP_TextureTool.h"

#include "BP_EndianSupport.h"

#include "BP_RenderBufferTypes.h"

#if BPE_TARGET == BPE_TARGET_X360
#include "Renderer/Base/Backend/X360/X360CTexture.h"
#elif BPE_TARGET == BPE_TARGET_WIN32
#include "Renderer/Base/Backend/Win32/Win32CTexture.h"
#elif BPE_TARGET == BPE_TARGET_VITA
#include <libperf.h>
#include <np_trophy_setup_dialog.h>
#include <screenshot.h>

#include "Engine/System/VTAThreadPriorities.h"

#endif

// Include for global disk build 'enable'.
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

#define BP_USE_EMBEDDED_RESOURCES 0

#if BP_USE_EMBEDDED_RESOURCES==1
#include "../GameDataSource/Assets.h"
#endif

#if BP_VITA
#  include "Renderer/Base/Backend/VTA/NVTAState.h"
#endif

//----------------------------------------------------------------------------

#include "MGS_Common.h"

//----------------------------------------------------------------------------

#if BPE_TARGET==BPE_TARGET_VITA && MGS_VERSION==2
#  define MGS2_DYNAMIC_FRAMERATE 1
#else
#  define MGS2_DYNAMIC_FRAMERATE 0
#endif

#if BPE_TARGET==BPE_TARGET_VITA
#  define DISABLE_POST_FX
#endif

extern "C"
{
   #include "bp_matrix.h"
   #include "BP_RenderBuffer.h"
   #include "BP_RenderBufferTypes.h"

   extern int BP_FRAMES_PER_SEC();

   extern int BP_IsDebugPaused();

#if MGS_VERSION == 2
   #include "camera.h"

   extern int BP_FRAMES_PER_SEC(void);

#endif

#if MGS_VERSION==3
   extern int gBP_DisableMSAAInReflections;
#endif

   int gBP_SceneBufferAllocSize = 0;
   int gBP_SceneBufferPeakSize = 0;
   char gBP_SceneBufferPeakArea[256];

#if BP_VITA
   int gBP_Vita_LastShaderFlushFrame = -1;
#endif
};

#include "BP_RenderShared.h"
#include "BP_UnifiedSDX.h"

void BP_RenderMainThread(int currentRenderBuffer);

#if BP_PRIM_DEBUGGING
extern "C" int gDumpPrims;
extern "C" int gDumpingPrims;
extern "C" int gClearPrims;
extern "C" int gShowPrims;
void BP_Prim_ClearProfile();
void BP_Prim_ResetProfile();
void BP_Prim_DumpProfile();
#endif

#if BP_VITA && MGS_VERSION==2
enum EVitaFPS60Mode
{
   kVF60_Off,
   kVF60_Whitelist,
   kVF60_AllAreas
};
int gAS_Vita_60FPSMode = kVF60_Whitelist;
#endif
//----------------------------------------------------------------------------


CVector4 gBP_FogColor(0, 0, 0, 0);

int gBP_ShouldDrawMode = 1;

#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_VITA
   #if MGS_VERSION == 3
int gSyncWaitRender = 0;
   #else
int gSyncWaitRender = 0;
   #endif
#else
int gSyncWaitRender = 1;
#endif
u_long64 gFrameTestValue;

//----------------------------------------------------------------------------

#if MGS_VERSION == 3
extern int gAllowDecal_BaseOnStage;
extern int gBP_EnableAlphaTestGEqualOptimization;
#endif

namespace
{
#if MGS_VERSION == 3
   int const kRenderBufferSize = 3 * 1024 * 1024;
#else
   // In MGS2: During "Big Shell" intro (d005p03) the usage is ~4.5mb
   // Need to investigate why.
   int const kRenderBufferSize = 5 * 1024 * 1024;
#endif
   SRenderBackendMetrics gPreviousFrameMetrics;

   int mCurrentRenderBuffer = 0;
   int mLastRenderedBuffer = 0;

#if BP_ENABLE_RENDER_LABELS
   int gRenderLabel_ActiveMask = 0;
   int gRenderLabel_DmaPack = 0;
   int gRenderLabel_PostFx = 0;
#endif

   int gViewportTextureCount = 0;

   enum
   {
      kRME_None,
      kRME_Checkered_Stipple,
      kRME_MSAAx2,
   };
   int gRasterMask_SwitchPixels = 1;
   int gRasterMaskEmulationMode = kRME_MSAAx2;
   int gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage = 0;
   int gNeedInitRasterMask = 1;
#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3
   int gEnableMSAA = 1;
#else
   int gEnableMSAA = 0;
#endif
   int gEnableMSAA_ZCullReload = 1;
   int gForceOneTickPerRenderFrame = 0;

#if BPE_TARGET == BPE_TARGET_PS3
   float       gGameGamma  = 1.150f;   // In game gamma
   float       gMovieGamma = 1.0f;     // Movie playback gamma
   float       gPS3VideoOutGamma = -1; // Current PS3 video out gamma
#endif

   CStopWatch renderWatch;
   real32 gCurrentRenderTime = 0.0f;
   
   CStopWatch sEndframeWatch;
   real32 sCurrentEndframeTime = 0.0f;

#if BP_VITA
   // This is the time the endframe thread waited on the 
   // render thread.
   uint64_t sLastRenderWaitTime;
#endif

   int gIndexBufferPoolUT_InUseSize = 0;

   int gVertexBufferPoolUT_InUseSize = 0;
}

extern "C"
{
int gAllowSlowDrawFrameSkips = 0;
};

int BP_GetCurrentRenderBuffer()
{
   return mCurrentRenderBuffer;
}

//----------------------------------------------------------------------------

namespace DebugStats
{
#if BP_TGS_DEMO()
   int   gDisplayFrameCounter = 0;
   int   gDisplayTime = 0;
   int   gDisplayAreaName = 0;
   int   gDisplayDebugText = 0;
#else
   int   gDisplayFrameCounter = 1;
   int   gDisplayTime = 0;
   int   gDisplayAreaName = 1;
   int   gShowPauseLevel = 0;
   int   gDisplayDebugText = 1;
#endif
   int   gDisplayResourceStats = 0;
   int   gDisplayFPS = 1;
   int   gDisplayMemory = 0;
   int   gDisplayRenderMemory = 0;
   
   int   gDisplayTitleSafeArea = 0;
   float gDisplayTitleSafeAreaPercent = .15f;
   int   gDisplayTitleSafeColor = 0;
}

namespace
{
#ifndef GOLD_VERSION
   int   gShowProfileInfo = 0;
   int   gShowProfileInfoOffset = 0;
   int   gIsProfileBarsInitialized = 0;
#endif
}

int gBP_DebugVertexAnim_Enabled = 1;
int gBP_DebugVertexAnim_ShowInfo = 0;

struct SFrameInfoStruct
{
   SBP_ViewportInfo * pViewportInfo;
   SRenderTarget::SRenderTargetViewPort lastViewportInfo;
   int currRenderTarget;
   bool disableMSAASwitch;
   bool hasExecutedClear;

   void Reset()
   {
      if( gEnableMSAA )
         currRenderTarget = kRT_CurrentFrameBuffer;
      else
         currRenderTarget = kRT_CurrentFrameBuffer_NoMSAA;

      disableMSAASwitch = false;
      hasExecutedClear = false;
      pViewportInfo = NULL;
   }
};

SFrameInfoStruct gFrameInfoStruct;

//----------------------------------------------------------------------------

#if BP_ENABLE_ONSCREEN_PROFILER

int BP_IsProfileInfoActive()
{
   return gShowProfileInfo && gIsProfileBarsInitialized;
}

#endif

//----------------------------------------------------------------------------

namespace
{
   CCompiledShader*  gpCurrentShader = NULL;
   uint8*            gpCurrentParameterBuffer = NULL;
};

void BP_EndShader()
{
   if( gpCurrentShader )
   {
      gpCurrentShader->EndShaderBatch();

      gpCurrentShader->EndPass();
      gpCurrentShader->End();

      gpCurrentShader = NULL;
   }
}

int BP_BeginShader(CCompiledShader* pShader, uint8 * pParameterBuffer, int const parameterBufferSize)
{
   if( gpCurrentShader != pShader || gpCurrentParameterBuffer != pParameterBuffer )
   {
      BP_EndShader();

      gpCurrentShader = pShader;
      gpCurrentParameterBuffer = pParameterBuffer;

      gpCurrentShader->Begin();
      gpCurrentShader->BeginPass(0);

      gpCurrentShader->BeginShaderBatch(pParameterBuffer, parameterBufferSize, NULL, 0, NULL, 0);

      return 1;
   }

   return 0;
}

//----------------------------------------------------------------------------


namespace MiscShader
{
   CCompiledShader*  gpShader[1];
}

void BP_InitMiscShader()
{
   CShaderFileId const shaderId("$/EngineSupport/Shaders/Misc.fx");

   {
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, "");
      assert(pShader);

      MiscShader::gpShader[0] = pShader;
   }
}

//----------------------------------------------------------------------------

void BP_ReloadShaders()

{
   gpCurrentShader = NULL;
   gpRenderBackend->ShaderCache()->FlushAllShaders();
   BP_InitShaders1();
   BP_InitShaders2();
}

//----------------------------------------------------------------------------

void BP_Decompress_Asset(void* inSrcBuffer, int inSourceLength, void* inTargetBuffer, int inTargetLength)
{
   unsigned long targetLength = inTargetLength;
   uncompress((Bytef*)inTargetBuffer, &targetLength, (const Bytef*)inSrcBuffer, inSourceLength);
}

//----------------------------------------------------------------------------

void BP_InitConsoleFont()
{
#if BP_USE_EMBEDDED_RESOURCES!=1
   // Vita loads the default font from a bp_archive and not from a resource embedded in the elf.
   CBaseRenderBackend::SetDefaultFontFromMemory(NULL, -1, NULL, -1);
#else
   CResId resId("$/enginesupport/fonts/****/console.ctxr");

   unsigned long consoleFontTexture = ast_open_file_index(resId.c_str());
   unsigned long consoleFont = ast_open_file_index("$/enginesupport/fonts/console.cfon");

   if (consoleFont != -1 && consoleFontTexture != -1)
      CBaseRenderBackend::SetDefaultFontFromMemory(ast_get_file_data(consoleFont), ast_get_file_length(consoleFont), ast_get_file_data(consoleFontTexture), ast_get_file_length(consoleFontTexture));
#endif
}

//----------------------------------------------------------------------------

void BP_PreLoadShader(char const * const shaderFile)
{
#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3
   CShaderFileId const shaderId(shaderFile);
   CResId resId(shaderId.mResource);

   unsigned long fileIndex = ast_open_file_index(resId.c_str());
   void* memBuffer = (void*)ast_get_file_data(fileIndex);
   uint32 memBufferSize = ast_get_file_length(fileIndex);
   BPE_ASSERT(memBuffer != NULL && memBufferSize > 0, "This file is not stored in the binary!!!");

   RenderBackend()->ShaderCache()->LoadShaderFromMemory(shaderId, (char *)memBuffer, memBufferSize);
#endif
}

//----------------------------------------------------------------------------

void BP_PreLoadShaders()
{
#ifdef _BP_DISK_BUILD

#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3

#if MGS_VERSION == 2
   BP_PreLoadShader("$/enginesupport/shaders/comdl.fx");
   BP_PreLoadShader("$/enginesupport/shaders/debug.fx");
   BP_PreLoadShader("$/enginesupport/shaders/default.fx");
   BP_PreLoadShader("$/enginesupport/shaders/dmapack.fx");
   BP_PreLoadShader("$/enginesupport/shaders/evm.fx");
   BP_PreLoadShader("$/enginesupport/shaders/fx.fx");
   BP_PreLoadShader("$/enginesupport/shaders/kms.fx");
   BP_PreLoadShader("$/enginesupport/shaders/kms_opt_cmf.fx");
   BP_PreLoadShader("$/enginesupport/shaders/misc.fx");
   BP_PreLoadShader("$/enginesupport/shaders/patch.fx");
   BP_PreLoadShader("$/enginesupport/shaders/prim.fx");
   BP_PreLoadShader("$/enginesupport/shaders/projector.fx");
   BP_PreLoadShader("$/enginesupport/shaders/radar.fx");
#elif MGS_VERSION == 3
   BP_PreLoadShader("$/enginesupport/shaders/clone.fx");
   BP_PreLoadShader("$/enginesupport/shaders/debug.fx");
   BP_PreLoadShader("$/enginesupport/shaders/default.fx");
   BP_PreLoadShader("$/enginesupport/shaders/dmapack.fx");
   BP_PreLoadShader("$/enginesupport/shaders/fx.fx");
   BP_PreLoadShader("$/enginesupport/shaders/gs.fx");
   BP_PreLoadShader("$/enginesupport/shaders/mdb.fx");
   BP_PreLoadShader("$/enginesupport/shaders/mdc.fx");
   BP_PreLoadShader("$/enginesupport/shaders/misc.fx");
   BP_PreLoadShader("$/enginesupport/shaders/prim.fx");
   BP_PreLoadShader("$/enginesupport/shaders/projector.fx");
#endif

#endif   // BPE_TARGET
#endif   // _BP_DISK_BUILD
}

//----------------------------------------------------------------------------

void BP_InitShaders1()
{
   BP_PreLoadShaders();

   BP_InitMiscShader();
   BP_InitDebugShader();
   BP_InitFXShader();
   BP_InitDmaPackShader();
}

void BP_InitShaders2()
{
   BP_InitPrimShader();
#if MGS_VERSION == 2
   BP_InitObjShader();
   BP_InitObjCharShader();
   BP_InitObjOptCmfShader();
   BP_InitEvmShader();
   BP_InitRadarShader();
   BP_InitPatchShader();
   BP_InitComdlShader();
   BP_InitProjectorShader();
#elif MGS_VERSION == 3
   BP_InitGSShader();
   BP_InitFixModelShader();
   BP_InitModelShader();
   BP_InitFastModelShader();
   BP_InitCloneShader();
   BP_InitProjectorShader();
#endif
}

//----------------------------------------------------------------------------

//#define DISABLE_ARGB_DEPTHBUFFERS


#if BPE_TARGET == BPE_TARGET_X360
   CBaseTexture::EAntiAliasType const kAAMode = CBaseTexture::kAA_MSAA2x;
   D3DMULTISAMPLE_TYPE const kD3DAntiAliasMode = D3DMULTISAMPLE_2_SAMPLES;
#elif BPE_TARGET == BPE_TARGET_PS3
   CBaseTexture::EAntiAliasType const kAAMode = CBaseTexture::kAA_MSAA2x;
#else
   CBaseTexture::EAntiAliasType const kAAMode = CBaseTexture::kAA_None;
#endif

#if BPE_TARGET == BPE_TARGET_X360
   #if BPE_USE_FLOATINGPOINT_ZBUFFER
      #define BP_DEPTHBUFFER_FORMAT CBaseTexture::kFormat_D24FS8
   #else
      #define BP_DEPTHBUFFER_FORMAT CBaseTexture::kFormat_D24X8
   #endif
#else
   #ifdef DISABLE_ARGB_DEPTHBUFFERS
      #define BP_DEPTHBUFFER_FORMAT CBaseTexture::kFormat_D24X8
   #else
      #define BP_DEPTHBUFFER_FORMAT CBaseTexture::kFormat_A8R8G8B8
   #endif
#endif

namespace RT
{
   struct SRenderTargetInfo
   {
      int                           width;
      int                           height;
      int                           widthDiv;
      int                           heightDiv;
      CBaseTexture::EFormat         format;
      CBaseTexture::EUsage          usage;
      CBaseTexture::EAntiAliasType  aa;
      ERenderMemory                 memory;
      SCreateTextureParams *        pTextureParams;
   };

   SCreateTextureParams gBufferTextureParams;
   SCreateTextureParams gDepthBufferTextureParams;
   SCreateTextureParams gFakeMSAATextureParams;

   SRenderTargetInfo const kRenderTargetSetup[] =
   {
      //kRT_MainBuffer
      { -1, -1, 1, 1,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  kAAMode,   kRM_Video,  &gBufferTextureParams },

      //kRT_DefaultDepthBuffer
      { -1, -1, 1, 1,   BP_DEPTHBUFFER_FORMAT,           CBaseTexture::kUsage_DepthBuffer,   kAAMode,   kRM_Video,  &gDepthBufferTextureParams },

      // Below here are NoMSAA textures
      //////////////////////////////////////////////////////////////////////////

      //kRT_MainBuffer1_NoMSAA
      { -1, -1, 1, 1,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_MainBuffer2_NoMSAA
      { -1, -1, 1, 1,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
      
      //kRT_DefaultDepthBuffer_NoMSAA
      { -1, -1, 1, 1,   BP_DEPTHBUFFER_FORMAT,           CBaseTexture::kUsage_DepthBuffer,   CBaseTexture::kAA_None,   kRM_Video,  &gDepthBufferTextureParams },

      //kRT_TempBuffer_NoMSAA,
      { -1, -1, 1, 1,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempDepthBuffer_NoMSAA,
      { -1, -1, 1, 1,   BP_DEPTHBUFFER_FORMAT,           CBaseTexture::kUsage_DepthBuffer,   CBaseTexture::kAA_None,   kRM_Video,  &gDepthBufferTextureParams },

      // Below here are post fx textures
      //////////////////////////////////////////////////////////////////////////

      //kRT_TempBufferHalfFakeMSAA
      { -1, -1, 2, 2,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_MSAA4x,   kRM_Video,  &gFakeMSAATextureParams },

      //kRT_TempBufferHalf1
      { -1, -1, 2, 2,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBufferHalf2
      { -1, -1, 2, 2,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBufferQuarter1
      { -1, -1, 4, 4,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBufferQuarter2
      { -1, -1, 4, 4,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#if MGS_VERSION == 3
      //kRT_TempBufferQuarter3
      { -1, -1, 4, 4,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBufferQuarter4
      { -1, -1, 4, 4,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBufferQuarter5
      { -1, -1, 4, 4,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBufferQuarterTemp
      { -1, -1, 4, 4,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#endif
      //kRT_TempBuffer8th
      { -1, -1, 8, 8,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer16th
      { -1, -1, 16, 16,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer32th
      { -1, -1, 32, 32,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer64th
      { -1, -1, 64, 46,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer128th
      { -1, -1, 128, 128,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer32px
      { 32, 32, 0, 0,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer16px
      { 16, 16, 0, 0,   CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer8px
      { 8, 8, 0, 0,     CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer4px
      { 4, 4, 0, 0,     CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempBuffer2px
      { 2, 2, 0, 0,     CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#if BPE_TARGET == BPE_TARGET_VITA
      //kRT_TempShadow
      { 256, 256, 0, 0, CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TempShadow2
      { 256, 256, 0, 0, CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TempShadow_Depth
      { 256, 256, 0, 0, BP_DEPTHBUFFER_FORMAT,           CBaseTexture::kUsage_DepthBuffer,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },
#else
      //kRT_TempShadow
      { 256, 256, 0, 0, CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempShadow2
      { 256, 256, 0, 0, CBaseTexture::kFormat_A8R8G8B8,  CBaseTexture::kUsage_RenderTarget,  CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TempShadow_Depth
      { 256, 256, 0, 0, BP_DEPTHBUFFER_FORMAT,           CBaseTexture::kUsage_DepthBuffer,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#endif

      //kRT_CameraScreenShot
#if BPE_TARGET == BPE_TARGET_VITA
      { 800, 608, 0, 0,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },
#else
      { -1, -1, 1, 1,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#endif
#if BPE_TARGET == BPE_TARGET_X360
      //kRT_CameraThumbnailScreenShot
      { 64, 64, 0, 0,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#elif BPE_TARGET == BPE_TARGET_VITA
      //kRT_CameraThumbnailScreenShot
      { 384, 224, 0, 0,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },
#else
      //kRT_CameraThumbnailScreenShot
      { 320, 176, 0, 0,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#endif
#if BPE_TARGET == BPE_TARGET_VITA
#if MGS_VERSION == 2
      //kRT_TankerSS0
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TankerSS1
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TankerSS2
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TankerSS3
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TankerSS4
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },

      //kRT_TankerSS5
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_System,  &gBufferTextureParams },
#endif
#else
#if MGS_VERSION == 2
      //kRT_TankerSS0
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TankerSS1
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TankerSS2
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TankerSS3
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TankerSS4
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },

      //kRT_TankerSS5
      { -1, -1, 2, 2,   CBaseTexture::kFormat_X8R8G8B8,  CBaseTexture::kUsage_RenderTarget,   CBaseTexture::kAA_None,   kRM_Video,  &gBufferTextureParams },
#endif
#endif
   };

   BPE_CTASSERT(BPE_ARRAY_SIZE(kRenderTargetSetup) == kRT_Count);

   CBaseTexture* mpRenderTargets[kRT_Count] = {0};
};

#ifndef BP_RENDER_SINGLE_RENDER_TARGET

CBaseTexture * BP_GetRenderTarget(ERenderTarget const id)
{
   switch(id)
   {
   case kRT_CurrentFrameBuffer_NoMSAA:
      return mCurrentRenderBuffer ? RT::mpRenderTargets[kRT_MainBuffer2_NoMSAA] : RT::mpRenderTargets[kRT_MainBuffer1_NoMSAA];

   case kRT_PreviousFrameBuffer_NoMSAA:
      return mCurrentRenderBuffer ? RT::mpRenderTargets[kRT_MainBuffer1_NoMSAA] : RT::mpRenderTargets[kRT_MainBuffer2_NoMSAA];

   case kRT_CurrentFrameBuffer:
      return RT::mpRenderTargets[kRT_MainBuffer];

   default:
      return RT::mpRenderTargets[id];
      break;
   }
}

// This version should only be called from BP_RenderPrologue
void BP_SetMainRenderTarget_RenderPrologue(int const currentRenderBuffer, int shouldDrawMode)
{
   CBaseTexture* pCurrentRenderTargetTexture;
   if( gEnableMSAA )
   {
      gFrameInfoStruct.currRenderTarget = kRT_MainBuffer;
      pCurrentRenderTargetTexture = BP_GetRenderTarget(kRT_MainBuffer);

      SRenderTarget renderTarget(pCurrentRenderTargetTexture, NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer));
      gpRenderBackend->SetRenderTarget(renderTarget);
      if( shouldDrawMode )
         gpRenderBackend->BeginTiling(renderTarget);
   }
   else
   {
      ERenderTarget const renderTargetId = currentRenderBuffer ? kRT_MainBuffer2_NoMSAA : kRT_MainBuffer1_NoMSAA;
      
      gFrameInfoStruct.currRenderTarget = renderTargetId;

      pCurrentRenderTargetTexture = BP_GetRenderTarget(renderTargetId);

      SRenderTarget renderTarget(pCurrentRenderTargetTexture, NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA));
      gpRenderBackend->SetRenderTarget(renderTarget);
   }
}

void BP_SetMainRenderTarget(int const currentRenderBuffer)
{
   gpRenderBackend->ResolveRenderTargetPredicated(gpRenderBackend->GetCurrentRenderTexture());

   CBaseTexture* pCurrentRenderTargetTexture = BP_GetRenderTarget(currentRenderBuffer ? kRT_MainBuffer2_NoMSAA : kRT_MainBuffer1_NoMSAA);

   SRenderTarget renderTarget(pCurrentRenderTargetTexture, NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA));
   gpRenderBackend->SetRenderTarget(renderTarget);

#if BPE_TARGET == BPE_TARGET_X360
   SRectf SrcDestRect;
   SrcDestRect.x1 = SrcDestRect.y1 = 0.0f;
   SrcDestRect.x2 = SrcDestRect.y2 = 1.0f;
   gpRenderBackend->RestoreRenderTargetColor_Internal(pCurrentRenderTargetTexture, pCurrentRenderTargetTexture, SrcDestRect, SrcDestRect);
#endif
}

#else

int gVTADebugRescale = true;
int gVTADebugRescaleToggle = false;
#if MGS_VERSION == 3
int gVTADebugUberscale = true;
#elif MGS_VERSION == 2
int gVTADebugUberscale = true;
#else
int gVTADebugUberscale = false;
#endif
int gVTADebugUberscaleForce = false;
int gVTADebugUberscaleShader = 0;
int gVTADebugUberscaleQuad = 15;

CBaseTexture * BP_GetRenderTarget(ERenderTarget const id)
{
   switch(id)
   {
   case kRT_CurrentFrameBuffer_Full:
      return gpRenderBackend->GetCurrentDisplayTexture();
   case kRT_CurrentDepthBuffer_Full:
      return gpRenderBackend->GetCurrentDisplayDepth();
   case kRT_CurrentFrameBuffer_Small:
      return gVTADebugRescale ? gpRenderBackend->GetCurrentDrawTexture() : gpRenderBackend->GetCurrentDisplayTexture();
   case kRT_CurrentDepthBuffer_Small:
      return gVTADebugRescale ? gpRenderBackend->GetCurrentDrawDepth() : gpRenderBackend->GetCurrentDisplayDepth();
   case kRT_CurrentFrameBuffer_Low:
      return gpRenderBackend->GetLowresDrawTexture();
   case kRT_CurrentDepthBuffer_Low:
      return gpRenderBackend->GetLowresDrawDepth();
   case kRT_CurrentFrameBuffer_NoMSAA:
   case kRT_CurrentFrameBuffer:
      return gpRenderBackend->GetCurrentDisplayTexture();
   
   case kRT_PreviousFrameBuffer_NoMSAA:
      return gpRenderBackend->GetPreviousDisplayTexture();

   default:
      return RT::mpRenderTargets[id];
      break;
   }
}

#endif

void BP_InitRenderTargets()
{
#if BPE_TARGET == BPE_TARGET_X360
   D3DRECT const * pTilingRects = gpRenderBackend->GetTilingRects();
   RT::gBufferTextureParams.mRenderTargetWidth = pTilingRects[0].x2;
   RT::gBufferTextureParams.mRenderTargetHeight = pTilingRects[0].y2;

   RT::gDepthBufferTextureParams.mRenderTargetWidth = pTilingRects[0].x2;
   RT::gDepthBufferTextureParams.mRenderTargetHeight = pTilingRects[0].y2;
   RT::gDepthBufferTextureParams.mBaseAddress = XGSurfaceSize(RT::gBufferTextureParams.mRenderTargetWidth, RT::gBufferTextureParams.mRenderTargetHeight, D3DFMT_A8R8G8B8, kD3DAntiAliasMode);

   RT::gFakeMSAATextureParams.mCreateRenderTargetTexture = 0;
#elif BPE_TARGET == BPE_TARGET_VITA
   RT::gBufferTextureParams.mNumScenes = 1;
#endif

   for( int i = 0; i < kRT_Count; ++i )
   {
      RT::SRenderTargetInfo const & info = RT::kRenderTargetSetup[i];

      int width = info.width;
      if( width < 0 )
         width = RenderBackend()->GetMainFrameBufferWidth() / info.widthDiv;

      int height = info.height;
      if( height < 0 )
         height = RenderBackend()->GetMainFrameBufferHeight() / info.heightDiv;

      info.pTextureParams->mClearMemory = 0;

#if BPE_TARGET == BPE_TARGET_VITA
      switch ( i )
      {
      case kRT_TempShadow:
      case kRT_TempShadow_Depth:
         info.pTextureParams->mNumScenes = 2;
         break;
#if MGS_VERSION == 3
      case kRT_CameraScreenShot:
      case kRT_CameraThumbnailScreenShot:
         break;
#endif
#if MGS_VERSION == 2
      case kRT_CameraScreenShot:
      case kRT_CameraThumbnailScreenShot:
      case kRT_TankerSS0:
      case kRT_TankerSS1:
      case kRT_TankerSS2:
      case kRT_TankerSS3:
      case kRT_TankerSS4:
      case kRT_TankerSS5:
         break;
#endif
      default:
         continue;
      }
#endif

#if BPE_TARGET == BPE_TARGET_X360    
      if( i >= kRT_MainBuffer )
      {
         info.pTextureParams->mCreateRenderTargetTexture = 0;
      }
      if( i >= kRT_MainBuffer1_NoMSAA )
      {
         // reset mCreateRenderTargetTexture from kRT_MainBuffer
         info.pTextureParams->mCreateRenderTargetTexture = 1;
         // outside of the predicated tiling section
         info.pTextureParams->mRenderTargetWidth = width;
         info.pTextureParams->mRenderTargetHeight = height;
      }
      //
      if( i == kRT_MainBuffer1_NoMSAA )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mClearMemory = 1;
      }
      else if( i == kRT_MainBuffer2_NoMSAA )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mClearMemory = 1;
      }
      else if( i == kRT_DefaultDepthBuffer_NoMSAA )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_MainBuffer2_NoMSAA]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer_NoMSAA )
      {
         info.pTextureParams->mBaseAddress = 0;
      }
      else if( i == kRT_TempDepthBuffer_NoMSAA )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer_NoMSAA]->GetEDRAMEnd();
      }
      // Post Effect buffers
      else if( i == kRT_TempBufferHalfFakeMSAA )
      {
         info.pTextureParams->mBaseAddress = 0;
      }
      else if( i == kRT_TempBufferHalf1 )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_DefaultDepthBuffer_NoMSAA]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBufferHalf2 )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferHalf1]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBufferQuarter1 )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferHalf2]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBufferQuarter2 )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferQuarter1]->GetEDRAMEnd();
      }
#if MGS_VERSION == 3
      else if( i == kRT_TempBufferQuarter3 )
      {
         // Note: this is special usage, shares EDRAM memory with kRT_TempBufferQuarter1
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferHalf2]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBufferQuarter4 )
      {
         // Note: this is special usage, shares EDRAM memory with kRT_TempBufferQuarter1
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferHalf2]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBufferQuarter5 )
      {
         // Note: this is special usage, shares EDRAM memory with kRT_TempBufferQuarter1
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferHalf2]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBufferQuarterTemp )
      {
         // Note: this is special usage, shares EDRAM memory with kRT_TempBufferQuarter1
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferHalf2]->GetEDRAMEnd();
      }
#endif
      //
      else if( i == kRT_TempBuffer8th )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_DefaultDepthBuffer]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer16th )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer8th]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer32th )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer16th]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer64th )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer32th]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer128th )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer64th]->GetEDRAMEnd();
      }
      //
      else if( i == kRT_TempBuffer32px )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBufferQuarter2]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer16px )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer32px]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer8px )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer16px]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer4px )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer8px]->GetEDRAMEnd();
      }
      else if( i == kRT_TempBuffer2px )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempBuffer4px]->GetEDRAMEnd();
      }
      //Shadow buffers
      else if( i == kRT_TempShadow )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_DefaultDepthBuffer]->GetEDRAMEnd();
      }
      else if( i == kRT_TempShadow2 )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_DefaultDepthBuffer]->GetEDRAMEnd();
      }
      else if( i == kRT_TempShadow_Depth )
      {
         info.pTextureParams->mBaseAddress = RT::mpRenderTargets[kRT_TempShadow]->GetEDRAMEnd();
         info.pTextureParams->mHierarchicalZBase = RT::mpRenderTargets[kRT_DefaultDepthBuffer]->GetEDRAMEndHierarchicalZ();
      }
      else if( i == kRT_CameraScreenShot )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
   #if MGS_VERSION == 2
      else if( i == kRT_TankerSS0 )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
      else if( i == kRT_TankerSS1 )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
      else if( i == kRT_TankerSS2 )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
      else if( i == kRT_TankerSS3 )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
      else if( i == kRT_TankerSS4 )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
      else if( i == kRT_TankerSS5 )
      {
         info.pTextureParams->mBaseAddress = 0;
         info.pTextureParams->mHierarchicalZBase = 0;
      }
   #endif
#endif

#if BPE_TARGET != BPE_TARGET_X360 
      if( i != kRT_TempBufferHalfFakeMSAA )
#endif
      {
         RT::mpRenderTargets[i] = CBaseTexture::Create(width, height, 1, info.format, info.usage, info.aa, info.memory, info.pTextureParams);
      }
   }

#if BP_ENABLE_MLAA
   // Clear MLAA buffer
   BP_ClearMLAABuffer();
#endif
}

void BP_FreeRenderTargets()
{
   for( int i = 0; i < kRT_Count; ++i )
   {
      CBaseTexture::DeleteTexture(RT::mpRenderTargets[i]);
      RT::mpRenderTargets[i] = NULL;
   }
}

//----------------------------------------------------------------------------

extern "C"
{
#if MGS_VERSION == 3
   extern int BP_ShowSkeleton;
   extern float gBP_LightMeshDepthBias;
#endif
#if BP_ENABLE_TESTNODE
   int gTestNodeEnabled = 0;
   int gTestNodeStart = 0;
   int gTestNodeEnd = 2000;
   int gTestNodeCur = 0;
   int gTestNodeCount = 0;
#endif
#if BP_ENABLE_TESTTRI
   int gTestTriEnabled = 0;
   int gTestTriActive = 0;
   int gTestTriReset = 0;
   int gTestTriStart = 0;
   int gTestTriEnd = 0;
   int gTestTriCur = 0;
   int gTestTriCount = 0;
#endif
   extern void bp_sddrv_loop_iteration();
};

#if BP_VITA
extern int gDoRazorCaptureReallySoon;
#endif

//----------------------------------------------------------------------------
#if BP_VITA
int BP_ReloadHudSettings(int)
{
   extern int gDoRazorLiveMetrics;
   extern int gReloadHudSettings;

   gDoRazorLiveMetrics = 0;
   gReloadHudSettings = 1;
   return 0;
}
#endif

void BP_InitRendererDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU

   // Camera Debug options
   BP_Camera_InitDebugMenu();

   int const renderMenu = BP_DebugMenu_AddMenu("Render", -1);
   {
#if BPE_TARGET == BPE_TARGET_PS3
      static const char* skFrameRateLimiterEntries[] = { "60 fps", "30 fps", "20 fps", "15 fps", "12 fps", "10 fps" };
      static int ps3VideoOutGammeEnabled = 0;
      BP_DebugMenu_AddEnum(renderMenu, "Frame Rate Limiter", skFrameRateLimiterEntries, &gpRenderBackend->mPresentationInterval, 1, 6);
      BP_DebugMenu_AddFloat(renderMenu, "In Game Gamma", &gGameGamma, 0.8f, 1.2f, 0.01f, 0.1f);
      BP_DebugMenu_AddFloat(renderMenu, "Full Screen Movie Gamma", &gMovieGamma, 0.8f, 1.2f, 0.01f, 0.1f);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(renderMenu, "PS3 video out gamma", &gPS3VideoOutGamma, 0.8f, 1.2f, 0.01f, 0.1f), &ps3VideoOutGammeEnabled );

#elif BPE_TARGET == BPE_TARGET_X360
      static const char* skFrameRateLimiterEntries[] = { "60 fps", "30 fps", "20 fps" };
      BP_DebugMenu_AddEnum(renderMenu, "Frame Rate Limiter", skFrameRateLimiterEntries, &gpRenderBackend->mPresentationInterval, 1, 3);
      BP_DebugMenu_AddInt(renderMenu, "Presentation Threshold", &gpRenderBackend->mPresentationThreshold, 0, 100, 1, 10);
      BP_DebugMenu_AddBool(renderMenu, "Async Swap", &gpRenderBackend->mAsyncSwaps);

      int const x360Menu = BP_DebugMenu_AddMenu("X360 Specific", renderMenu);
      static const char* skTrilinearThresholdEntries[] = { "Full", "1/6th", "1/4th", "1/8th" };
      BP_DebugMenu_AddEnum(x360Menu, "Trilinear Threshold", skTrilinearThresholdEntries, &gpRenderBackend->mTrilinearThreshold, 0, 3);
      // Min 16 for either vertex GPR or pixel GPR, pixel GPR + vertex GPR must = 128 (pixel shader GPR = 128-Vertex shader GPR)
      BP_DebugMenu_AddInt(x360Menu, "Vertex GPR Count", &gpRenderBackend->mVertexShaderGPRCount, 16, 128-16, 1, 4);

      static const char* skUpscaleModeEntries[] = { "Default", "Gaussian" };
      BP_DebugMenu_AddEnum(x360Menu, "Upscale Mode", skUpscaleModeEntries, &gpRenderBackend->mVideoScalerMode, 0, 1);
#elif BPE_TARGET == BPE_TARGET_VITA
#if JADEBUG
      int const nodeMenu = BP_DebugMenu_AddMenu("Debug Node", renderMenu);
#if BP_ENABLE_TESTNODE
      BP_DebugMenu_AddBool(nodeMenu, "Test Node Enabled", &gTestNodeEnabled);
      BP_DebugMenu_AddInt(nodeMenu, "Test Node Start", &gTestNodeStart, 0, 1000000, 1, 25);
      BP_DebugMenu_AddInt(nodeMenu, "Test Node End", &gTestNodeEnd, 0, 1000000, 1, 25);
#endif
#if BP_ENABLE_TESTTRI
      BP_DebugMenu_AddBool(nodeMenu, "Test Tri Enabled", &gTestTriEnabled);
      BP_DebugMenu_AddBool(nodeMenu, "Test Tri Reset", &gTestTriReset);
      BP_DebugMenu_AddInt(nodeMenu, "Test Tri Start", &gTestTriStart, 0, 1000000, 1, 25);
      BP_DebugMenu_AddInt(nodeMenu, "Test Tri End", &gTestTriEnd, 0, 1000000, 1, 25);
#endif
      BP_DebugMenu_AddBool(nodeMenu, "Capture", &gDoRazorCaptureReallySoon);
#endif
#endif
      BP_DebugMenu_AddBool(renderMenu, "Show FPS", &DebugStats::gDisplayFPS);
      BP_DebugMenu_AddBool(renderMenu, "Wait Render Thread", &gSyncWaitRender);
      BP_DebugMenu_AddBool(renderMenu, "Force 1 tick per render frame", &gForceOneTickPerRenderFrame);
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
      BP_DebugMenu_AddBool(renderMenu, "Allow slow draw frame skips", &gAllowSlowDrawFrameSkips);
#endif
      static char const * skRenderMemoryOptions[] = { "Off", "Peaks", "Details" };
      BP_DebugMenu_AddEnum(renderMenu, "Show Render Memory", skRenderMemoryOptions, &DebugStats::gDisplayRenderMemory, 0, 2);
      BP_DebugMenu_AddBool(renderMenu, "Enable Blend Mode Emulation", &gGS_DebugEnableBlendModeEmulation);

      int const rasterMaskMenu = BP_DebugMenu_AddMenu("Raster Mask Emulation", renderMenu);
      static int enableDebugMenuRasterMaskForceCheckBoard_BasedOnStage = 0;
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddBool(rasterMaskMenu, "Stage Force CheckBoard", &gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage ), &enableDebugMenuRasterMaskForceCheckBoard_BasedOnStage );
      static const char* skRasterMaskEmulationModeEntries[] = { "Off", "CheckBoard/Stipple", "MSAAx2 Mask" };
      BP_DebugMenu_AddEnum(rasterMaskMenu, "Mode", skRasterMaskEmulationModeEntries, &gRasterMaskEmulationMode, 0, 2);
      BP_DebugMenu_AddBool(rasterMaskMenu, "Enable pixel switch every frame", &gRasterMask_SwitchPixels);

      int const titleSafeMenu = BP_DebugMenu_AddMenu("Title Safe", renderMenu);
      BP_DebugMenu_AddBool(titleSafeMenu, "Display Title Safe Border", &DebugStats::gDisplayTitleSafeArea);
      BP_DebugMenu_AddFloat(titleSafeMenu, "Title Safe Percentage", &DebugStats::gDisplayTitleSafeAreaPercent, 0.0f, 0.3f, 0.05f, 0.2f);
      static const char* skTitleSafeBorderColor[] = { "White", "Black" };
      BP_DebugMenu_AddEnum(titleSafeMenu, "Title Safe Color", skTitleSafeBorderColor, &DebugStats::gDisplayTitleSafeColor, 0, 1);

      int const msaaMenu = BP_DebugMenu_AddMenu("MSAA", renderMenu);
      BP_DebugMenu_AddBool(msaaMenu, "Enable MSAA", &gEnableMSAA);
      BP_DebugMenu_AddBool(msaaMenu, "Enable ZCull Reload", &gEnableMSAA_ZCullReload);
#if MGS_VERSION==3
      BP_DebugMenu_AddBool(msaaMenu, "Disable MSAA for reflections", &gBP_DisableMSAAInReflections);
#endif

      gBP_SceneBufferPeakArea[0] = '\0';
   }

   int const gameTweaksMenu = BP_DebugMenu_AddMenu("Game Specific Tweaks", renderMenu);
   {
#if MGS_VERSION == 3
      BP_DebugMenu_AddFloat(gameTweaksMenu, "Light Mesh Depth Bias", &gBP_LightMeshDepthBias, 0.0f, 1.0f, 0.01f, 0.1f);
#endif
   }

   BP_Movie_InitDebugMenu();
   BP_FX_InitDebugMenu();

#if MGS_VERSION == 2
   BP_Comdl_InitDebugMenu();
   BP_Obj_InitDebugMenu();
   BP_Evm_InitDebugMenu();
   AS_Patch_InitDebugMenu();

#elif  MGS_VERSION == 3
   BP_DebugMenu_AddBool(renderMenu, "Show Skeleton", &BP_ShowSkeleton);
   BP_Preshade_InitDebugMenu();
   BP_DebugCulling_InitDebugMenu();
   BP_Model_InitDebugMenu();
   BP_FixModel_InitDebugMenu();
   BP_Clone_InitDebugMenu();
   BP_OptCmf_InitDebugMenu();
   BP_DebugColl_InitDebugMenu();
#endif

   BP_Projector_InitDebugMenu();

   BP_Memory_InitDebugMenu();

#if BP_ENABLE_RENDER_LABELS
   {
      int const labelMenu = BP_DebugMenu_AddMenu("Render Labels", renderMenu);
      BP_DebugMenu_AddBool(labelMenu, "DmaPack", &gRenderLabel_DmaPack);
      BP_DebugMenu_AddBool(labelMenu, "PostFx",  &gRenderLabel_PostFx);
   }
#endif
   BP_Prim_InitDebugMenu();

#if defined(BP_VITA)
   {
      int const renderMenu = BP_DebugMenu_GetMenu("Render");
#if MGS2_DYNAMIC_FRAMERATE
      static const char* sk60FPSModes[] = { "Off", "Whitelist", "All Areas" };

      BP_DebugMenu_AddEnum( renderMenu, "60 FPS", sk60FPSModes, &gAS_Vita_60FPSMode, 0, 2);
#endif
      int const rescaleMenu = BP_DebugMenu_AddMenu("Rescaling", renderMenu);
      extern int gVTADebugRescale;
      extern int gVTADebugRescaleToggle;
      extern int gVTADebugRescaleWidth;
      extern int gVTADebugRescaleHeight;
      extern int gVTADebugUberscale;
      extern int gVTADebugUberscaleForce;
      extern int gVTADebugUberscaleShader;
      extern int gVTADebugUberscaleQuad;
      BP_DebugMenu_AddBool(rescaleMenu, "Enable", &gVTADebugRescale);
      BP_DebugMenu_AddBool(rescaleMenu, "Toggle", &gVTADebugRescaleToggle);
      BP_DebugMenu_AddInt(rescaleMenu, "Width", &gVTADebugRescaleWidth, 480, CRenderBackend::skDrawWidth, 8, 16);
      BP_DebugMenu_AddInt(rescaleMenu, "Height", &gVTADebugRescaleHeight, 272, CRenderBackend::skDrawHeight, 4, 8);
      BP_DebugMenu_AddBool(rescaleMenu, "Uberscale", &gVTADebugUberscale);
      BP_DebugMenu_AddBool(rescaleMenu, "Uberscale Force", &gVTADebugUberscaleForce);
      BP_DebugMenu_AddInt(rescaleMenu, "Uberscale Shader", &gVTADebugUberscaleShader, 0, 24, 1, 1);
      BP_DebugMenu_AddInt(rescaleMenu, "Uberscale Quad", &gVTADebugUberscaleQuad, 0, 15, 1, 1);
   }
#endif

   BP_DebugMenu_AddBool(-1, "Show Debug Text", &DebugStats::gDisplayDebugText);
   BP_DebugMenu_AddBool(-1, "Show Frame Counter", &DebugStats::gDisplayFrameCounter);
   BP_DebugMenu_AddBool(-1, "Show Time", &DebugStats::gDisplayTime);
   BP_DebugMenu_AddBool(-1, "Show Area Name", &DebugStats::gDisplayAreaName);
   BP_DebugMenu_AddBool(-1, "Show Pause Level", &DebugStats::gShowPauseLevel);
   BP_DebugMenu_AddBool(-1, "Show Memory", &DebugStats::gDisplayMemory);
   BP_DebugMenu_AddBool(-1, "Show Resources Stats", &DebugStats::gDisplayResourceStats);

   // Performance menu
   {
      int const performance = BP_DebugMenu_AddMenu("Performance", renderMenu);

      static const char* skShowProfileInfoEntries[] = { "Off", "Totals", "Graph & Totals", "Graph & GPU Detail/Totals" };
      BP_DebugMenu_AddEnum(performance, "Show Profile Info", skShowProfileInfoEntries, &gShowProfileInfo, 0, 3);

      BP_DebugMenu_AddInt(performance, "Profile Info Line Offset", &gShowProfileInfoOffset, 0, 100, 1, 8);

      extern int gGS_DebugEnableAlphaTestFail;
      BP_DebugMenu_AddBool(performance, "Enable Alpha Fail Emulation", &gGS_DebugEnableAlphaTestFail);

#if BPE_TARGET == BPE_TARGET_PS3
      extern real32 gTextureLODBias;
      BP_DebugMenu_AddFloat(performance, "Texture LOD Bias", &gTextureLODBias, -16.0f, 16.0f, 0.01f, 0.1f);
#endif

#if BP_VITA
      extern int gDoRazorLiveMetrics;
      extern int gDumpRazorLiveMetrics;
      extern int gLMGroup;
      extern int gLMSpan;
      BP_DebugMenu_AddBool(performance, "Show LiveMetrics", &gDoRazorLiveMetrics);
      BP_DebugMenu_AddInt(performance, "LiveMetrics Group", &gLMGroup, 0, 3, 1, 1);
      BP_DebugMenu_AddBool(performance, "Dump LiveMetrics", &gDumpRazorLiveMetrics);
      BP_DebugMenu_AddInt(performance, "LiveMetrics Span", &gLMSpan, 1, 8, 1, 1);
#if JADEBUG
      BP_DebugMenu_AddAction(performance, "Reload hud_settings", BP_ReloadHudSettings, 0);
#endif
#endif
   }

   // Vertex anim menu
   {
      int const vertexAnimMenu = BP_DebugMenu_AddMenu("Vertex Anim", renderMenu);
      BP_DebugMenu_AddBool(vertexAnimMenu, "Enable", &gBP_DebugVertexAnim_Enabled);
      BP_DebugMenu_AddBool(vertexAnimMenu, "Show Info", &gBP_DebugVertexAnim_ShowInfo);
   }
#endif
}

//----------------------------------------------------------------------------

extern "C" double BP_GetGlobalGameTime()
{
   return DG_TickCount / (300.0/BASE_TICK);
}

extern "C" uint64 BP_GetElapsedMicroSeconds()
{
   return (uint64)(BP_GetGlobalGameTime() * 1000000.0);
}

//----------------------------------------------------------------------------

void BP_IncrementGameTicks()
{
#if (BPE_TARGET == BPE_TARGET_PS3) || (BPE_TARGET == BPE_TARGET_X360) || ( BPE_TARGET == BPE_TARGET_VITA )

   // Track VBL count
   static uint32 prevVBLCount = 0;
   static uint32 currVBLCount = 0;

   // Compute delta ticks since last call
   // NOTE: uint32 automatically takes care of wrap around delta
#  if BPE_TARGET==BPE_TARGET_VITA
   gpRenderBackend->UpdateVBLCount();
#  endif
   currVBLCount = gpRenderBackend->GetVBLCount();
   uint32 deltaTicks = currVBLCount - prevVBLCount;
   prevVBLCount = currVBLCount;

#elif BPE_TARGET == BPE_TARGET_WIN32

   // Track time
   static CStopWatch frameStopWatch;
   static real64 frameTime = 0;
   frameTime += frameStopWatch.GetElapsedTime64();
   frameStopWatch.Reset();

   // Compute accumulated game ticks
   const real64 tickTime = 1.0/BP_FRAMES_PER_SEC(); // Original PS2 game tick time
   int deltaTicks = (int)(frameTime / tickTime);

   // Remove tick time from frame time
   frameTime -= deltaTicks * tickTime;
   if( frameTime < 0 )
   {
      frameTime = 0;
   }

#else
#  error Unknown platform!
#endif

#if defined(_DEBUG)
   // Clamp frame time so game doesn't advance like crazy when paused in the debugger
   if( deltaTicks > 4 )
   {
      deltaTicks = 4;
   }
#endif

#if BP_ENABLE_DEBUG_MENU
   // Apply debug cinema speed?
   if( gBP_DirectOutputStreamIsDemo )
   {
      deltaTicks *= gBP_DirectOutputStreamSpeed;
   }
#endif

   // If frame step, force advance of just 1 game tick
   if( ( gBP_PauseFrameStep ) || ( gForceOneTickPerRenderFrame ) )
   {
      deltaTicks = 1;
   }

   // Do not advance game ticks if debug paused
   if( BP_IsDebugPaused() )
   {
      deltaTicks = 0;
   }

   // Update game ticks
   DG_TickCount += deltaTicks;
}

//----------------------------------------------------------------------------

static void update_sound_endframe()
{
	double nowFrameTime = CStopWatch::gGlobalTime.GetElapsedTime();

   BP_Debug_PushCPUMarker( "Sound" );
   {
#if MGS_VERSION==3
      //TODO: the equivalent of this loop cycled every 10ms on PS2 on an iop thread.
      //May need to move this code to a separate OS thread if timing becomes a problem.
      const double kSoundUpdateCycleMs = 10;
      const int kMaxSoundUpdatesPerEndFrame = 3;
#elif MGS_VERSION==2
      //The SdInt thread cycled once every 2.5 ms.  May need to restructure that bit of the update.
      //It was more sensitive to exact tempo timing as well, because the music was midi.
      //See setTimer() in iopMain.c
      const double kSoundUpdateCycleMs = 2.5;
      const int kMaxSoundUpdatesPerEndFrame = BP_FRAMES_PER_SEC()/3; //3 50/60hz frames worth of updates
#endif
      static double sSoundExecutedMs = 0;

      //      double prevSoundExecutedMs = sSoundExecutedMs;
      double elapsedMs = nowFrameTime * 1000.0;
      int soundUpdates = 0;

      if( elapsedMs > sSoundExecutedMs )
      {
         //Force at least one sound update.
         bp_sddrv_loop_iteration();
         sSoundExecutedMs += kSoundUpdateCycleMs;
         ++soundUpdates;

         //Loop to perform the rest.
         for( ; sSoundExecutedMs < elapsedMs; sSoundExecutedMs += kSoundUpdateCycleMs )
         {
            if( soundUpdates++ == kMaxSoundUpdatesPerEndFrame )
            {
               //Keep a cap on the number of sound updates, as DG_TickCount can go up by a few dozen increments
               //at a time while the debugger is stepping.
               sSoundExecutedMs = elapsedMs;
               break;
            }
            bp_sddrv_loop_iteration();
         }
      }
      else
      {
         //We're spinning while loading a stage or something.  Currently, BP_EndFrame() is not throttled
         //by vsync's so we have to work around sound updates surpassing DG_TickCount here instead. (MGS2)
#if MGS_VERSION==3
         //Do force at least one sound update! (MGS3 only-- they spam the sound command queue with volume
         //and rotation commands so it is necessary to clear them out.  In MGS2 we will mess up the midi music timing doing this.)
         //This whole bit will become less messy when we throttle BP_EndFrame() to just once a frame
         //and most likely run the sound command processing on a separate thread anyway. :(
         bp_sddrv_loop_iteration();
#else
         // Force one sound update if debug paused so that sound commands are flushed
         if( BP_IsDebugPaused() )
         {
            bp_sddrv_loop_iteration();
         }
#endif
      }
      BP_Debug_PopCPUMarker();
      //      printf("elapsed %f->%f, SD %d\n", (float)prevSoundExecutedMs, (float)elapsedMs, soundUpdates );
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_EndFrame()
{
   // Update game ticks
   BP_IncrementGameTicks();

#if !defined(BP_VITA)
   update_sound_endframe();
#endif

   gpOsContext->ProcessPlatform();
   BP_UpdateRichPresenceLogic();

   BP_RichPresenceSystem_HeartBeat();
   BP_Network_HeartBeat();
   BP_TUS_HeartBeat();
   BP_CommonDialog_Heartbeat();

}

//----------------------------------------------------------------------------

extern "C" const char * gBP_InitialStage;

static void simple_render_frame()
{
#if BP_VITA
   // Clear the back buffer
   gpRenderBackend->BeginScene();
   gpRenderBackend->SetRenderTarget(
      SRenderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer_Full), NULL, NULL, NULL, 
      BP_GetRenderTarget(kRT_CurrentDepthBuffer_Full)));
   gpRenderBackend->Clear(CBaseRenderBackend::kFlag_Color, CColor::Black(), 1.0f, 0);
   BP_RenderLoadingSpinner();
   gpRenderBackend->EndScene();
#endif
}

extern "C" void BP_InitMain()
{

// Vita does not embed resources into the elf.
#if BP_USE_EMBEDDED_RESOURCES == 1
   ast_set_file_decompression(BP_Decompress_Asset);
#endif

   BP_InitConsoleFont();

   BP_HandleCommandLineArgs();
   BP_InitializeSystems();

   BP_RB_Init(kRenderBufferSize);
   
#if BP_ENABLE_MLAA
   BP_InitMLAA();
#endif

   BP_InitRenderTargets();

#if BP_VITA
   BP_InitGS();
#endif

   BP_InitShaders1();

   BP_InitTweakDebugMenu();
   BP_InitRendererDebugMenu();
   BP_InitGameDebugMenu();

#if BP_ENABLE_TEXTURE_TOOL
   BP_TextureTool_Init();
#endif

#if BP_TGS_DEMO()
   BP_TGS_DEMO_Init();
#endif

#if BPE_TARGET != BPE_TARGET_VITA
   BP_InitLoadingSpinner();
#endif

#if ENABLE_SEPERATE_RENDER_THREAD
   gpRenderBackend->StartThreadSystem(BP_RenderMainThread);
#endif

   BP_InitializeFileSupport();

#if BPE_TARGET == BPE_TARGET_VITA
   BP_InitLoadingSpinner();
#endif
   BP_Init_FS_Region();
   BP_LoadStreamRemappingTables();

   BP_CheckForLicenseFile();

   BP_SetSpinnerInitialLoadBegin();

#if defined(BP_VITA)
   BP_BeginBackgroundLoadSDXArchive();
#endif

#if !BP_TGS_DEMO()

#if BPE_TARGET == BPE_TARGET_PS3
   //BP - PS3: Wait for trophy initialization and initial savedata checking to complete.
   //TROPHY initialization step is responsible for kicking off initial silent savedata load
   //because it is dependent on knowing whether there's HDD space for trophies+savedata before it attempts
   //to install the trophy package.
   //This is as long as we can wait for trophy / initial savedata load to complete.
   //The main game thread kicks off after this function returns.
   while( BP_TrophySystem_UpdateInProgress() || !MGS_SaveStatus_IsDone() )
   {
      CStopWatch::Sleep(1);
   }
   //NOTE: #if 0 the next line if you want to disable preventing the game from starting when no HDD space.
   //Useful for testing "no space" save error case since the usual way of that happening (player receives a PSN message
   //that fills up the HDD after starting the game) is a pain in the ass to create.
#if 1
   //Both trophy initialization and initial savedata load are complete by this point.
   //On PS3, do not allow player to progress if there is insufficient HDD space for trophy and game savedata.
   BP_HandleInsufficientHDD();
#endif

#elif BPE_TARGET == BPE_TARGET_VITA
   int simpleFramesRendered = 0;

   if (!BP_TrophySystem_IsDisabled())
   {
      // Give the common dialog a chance to finish before starting
      while( BP_TrophySystem_UpdateInProgress() )
      {
         // It's possible to get a dialog here if there's too many trophy sets in flash memory
         SceCommonDialogStatus status;

         status = sceNpTrophySetupDialogGetStatus();
         if (status != SCE_COMMON_DIALOG_STATUS_NONE)
         {
            simpleFramesRendered++;
            simple_render_frame();
         }
         if (status == SCE_COMMON_DIALOG_STATUS_FINISHED)
         {
            // Tell the trophy system that it's done updating
            BP_TrophySystem_SignalUpdateFinished();

            SceNpTrophySetupDialogResult result;
            memset(&result, 0, sizeof(result));
            int success = sceNpTrophySetupDialogGetResult(&result);

            success = sceNpTrophySetupDialogTerm();
         }
      }
   }

   if ( simpleFramesRendered < 3 )
   {
      // AS(JM) - If we haven't rendered 3 frames yet, do it.
      // 3 will fill out all our frame buffers.
      for ( int i = simpleFramesRendered; i < 3; ++i )
      {
         simple_render_frame();
      }
   }

   sceScreenShotDisable();
   sceScreenShotSetOverlayImage( 
      (const SceChar8 *) (gpOsContext->mBuildSKU == COsContext::kBS_Japan ? "app0:screenshot_jp.png" : "app0:screenshot_eu.png"),
      480,
      272 );
   sceScreenShotEnable();

#endif

   BP_InitShaders2();

#if defined(BP_VITA)
   BP_JoinBackgroundLoadSDXArchive();
#endif

   BP_SetSpinnerInitialLoadEnd();

#if MGS_VERSION == 3
   BP_LayoutFont_Init();
#endif

#endif //BP_TGS_DEMO()
}

//----------------------------------------------------------------------------

enum
{
   kReg_Projection         =  60,   // 60..63
};

//----------------------------------------------------------------------------

extern void BP_Debug_TrophyRender();
extern void BP_Debug_LoadingSpinnerRender();
#if BPE_TARGET == BPE_TARGET_X360
extern int gBP_X360ShowRichPresence;
#endif

void BP_DebugRender()
{
   BP_EndShader();
   
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);

   // Setup screen space projection matrix
   {
      gpRenderBackend->SetScreenSpaceOrtho();

      real32 projection[16];
      BP_Matrix44_to_VS_Matrix44((real32 const*)&gpRenderBackend->GetProjectionMatrix(), projection);

      gpRenderBackend->SetVertexRegisters(kReg_Projection, 4, (CVector4 const*)projection);
   }

#ifndef GOLD_VERSION

#if BP_ENABLE_DEBUG_MENU
   #if MGS_VERSION == 3
   BP_DebugCulling_UpdateDebugMenu();
   BP_DebugColl_Stats_Draw();
   BP_DebugColl_Stats_Reset();
   BP_Preshade_ShowStats();
   #endif

   BP_Memory_ShowStats();
//   BP_FPS_ShowStats();
#endif

   int const initialY = 16;

   int currentX = 16;
   int currentY = initialY;

   // Display Frame Counter
   if( DebugStats::gDisplayFrameCounter )
   {
      BP_Debug_DrawString(CStringExtras::Stringize_s("Frame: %d", gpRenderBackend->GetFrameCount()), &currentX, &currentY);
   }

   // Display Time
   if( DebugStats::gDisplayTime )
   {
      BP_Debug_DrawString(CStringExtras::Stringize_s("Time: %.2f", (float)BP_GetGlobalGameTime()), &currentX, &currentY);
   }

   if( DebugStats::gDisplayAreaName )
   {
      if( const char * areaName = RenderBackend()->GetCurrentAreaDebugName() )
      {
         BP_Debug_DrawString(CStringExtras::Stringize_s("Area: %s Ver: %s", 
            areaName,
            gpOsContext->mBuildVersionString
            ), &currentX, &currentY);
      }
   }

   if( DebugStats::gShowPauseLevel )
   {
#if MGS_VERSION==2
      BP_Debug_DrawString(CStringExtras::Stringize_s("Pause Level: %08x Pad Flags %08x", GV_PauseLevel, GV_PadData[ 0 ].flag), &currentX, &currentY);
#elif MGS_VERSION==3
      BP_Debug_DrawString(CStringExtras::Stringize_s("Pause Level: %08x", GV_ActorPauseLevelGet()), &currentX, &currentY);
#endif
   }

#if BPE_TARGET == BPE_TARGET_X360
   if( gBP_X360ShowRichPresence )
   {
      CColor richPresenceColor;
      const char* pRichPresenceString = BP_GetActualRichPresenceStringAndColor(richPresenceColor);
      BP_Debug_DrawString(CStringExtras::Stringize_s("RichPresence: %s", pRichPresenceString), &currentX, &currentY, 0, richPresenceColor );
   }
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   {
      CellScreenShotSetParam screenshot_param = {0, 0, 0, 0};
      screenshot_param.photo_title = RenderBackend()->GetCurrentAreaDebugName();
#if MGS_VERSION == 2
      screenshot_param.game_title = "MGS2 confidential";
#else
      screenshot_param.game_title = "MGS3 confidential";
#endif
      screenshot_param.game_comment = "Confidential";

      int ret;
      ret = cellScreenShotSetParameter(&screenshot_param);
   }
#endif


   if( DebugStats::gDisplayResourceStats )
   {
      BP_Debug_DrawString(CStringExtras::Stringize_s("Meshes: %d Textures: %d Viewport Textures: %d", BP_GetLoadedMeshCount(), BP_GetLoadedTextureCount(), gViewportTextureCount), &currentX, &currentY);
   }

   // Display FPS
   if( DebugStats::gDisplayFPS )
   {
      real32 const fps = gPreviousFrameMetrics.mTimeSinceLastFlip ? (1.0f / gPreviousFrameMetrics.mTimeSinceLastFlip) : 0.0f;

      bool const displayWarning = (fps > 5) && (fps < 22);

      if( displayWarning )
      {
         BP_Debug_DrawString(CStringExtras::Stringize_s(
            "FPS: %.0f"
#if MGS2_DYNAMIC_FRAMERATE
            "/%d"
#endif
            " - DROPPING BELOW 30fps"
            , fps
#if MGS2_DYNAMIC_FRAMERATE
            , 30 * ( 1 + BP_Renderer_MGS2_3060_IsAt60() )
#endif
            ), &currentX, &currentY, 0, CColor::Red());
      }
      else
      {
         BP_Debug_DrawString(CStringExtras::Stringize_s(
            "FPS: %.0f"
#if MGS2_DYNAMIC_FRAMERATE
            "/%d"
#endif
            , fps
#if MGS2_DYNAMIC_FRAMERATE
            , 30 * ( 1 + BP_Renderer_MGS2_3060_IsAt60() )
#endif
            ), &currentX, &currentY);
      }

#if MGS_VERSION==2 && BP_VITA
//      BP_Renderer_MGS2_PrintDebug3060Info( &currentX, &currentY );
#endif

#if BP_VITA
      gpRenderBackend->PrintTimers();
#endif
   }

   if( DebugStats::gDisplayMemory )
   {
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
      SMemoryStats const memoryStats = CMemoryAllocator::GetMemoryStatistics(CMemoryAllocator::kMS_Detailed);
      BP_Debug_DrawString(memoryStats.mMemInfoString.c_str(), &currentX, &currentY);
#endif
   }

   if( DebugStats::gDisplayRenderMemory > 0)
   {
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_VITA
      real32 const kOneMeg = 1024.0f * 1024.0f;

      SRenderHWAllocatorStats systemVidStats, videoVidStats;
      gpRenderBackend->GetMemoryStats(kRM_System, &systemVidStats);
      gpRenderBackend->GetMemoryStats(kRM_Video, &videoVidStats);

      // peaks
      if (DebugStats::gDisplayRenderMemory == 1)
      {
         BP_Debug_DrawString(
            CStringExtras::Stringize_s(
               "Render Peaks (MiB):\nScene: %0.2f/%0.2f (%s)\nRender: %0.2f/%0.2f (%s)\nRSX SYS: %0.2f/%0.2f (%s)\nRSX VID: %0.2f/%0.2f (%s)\nIndexUT: %0.2f/%0.2f (%s)\nIndexRT: %0.2f/%0.2f (%s)\nVertexUT: %0.2f/%0.2f (%s)\nVertexRT: %0.2f/%0.2f (%s)\n",
               gBP_SceneBufferPeakSize / kOneMeg,
               gBP_SceneBufferAllocSize / kOneMeg,
               gBP_SceneBufferPeakArea,
               BP_RB_DebugGetPeakSize() / kOneMeg,
               BP_RB_DebugGetTotalSize() / kOneMeg,
               BP_RB_DebugGetPeakArea(),
               systemVidStats.mPeakAllocatedSize / kOneMeg,
               systemVidStats.mTotalSize / kOneMeg,
               systemVidStats.mPeakArea,
               videoVidStats.mPeakAllocatedSize / kOneMeg,
               videoVidStats.mTotalSize / kOneMeg,
               videoVidStats.mPeakArea,
               gpRenderBackend->GetIndexBufferPool_UT()->GetPeakSize() / kOneMeg,
               gpRenderBackend->GetIndexBufferPool_UT()->GetTotalSize() / kOneMeg,
               gpRenderBackend->GetIndexBufferPool_UT()->GetPeakArea(),
               gpRenderBackend->GetIndexBufferPool_RT()->GetPeakSize() / kOneMeg,
               gpRenderBackend->GetIndexBufferPool_RT()->GetTotalSize() / kOneMeg,
               gpRenderBackend->GetIndexBufferPool_RT()->GetPeakArea(),
               gpRenderBackend->GetVertexBufferPool_UT()->GetPeakSize() / kOneMeg,
               gpRenderBackend->GetVertexBufferPool_UT()->GetTotalSize() / kOneMeg,
               gpRenderBackend->GetVertexBufferPool_UT()->GetPeakArea(),
               gpRenderBackend->GetVertexBufferPool_RT()->GetPeakSize() / kOneMeg,
               gpRenderBackend->GetVertexBufferPool_RT()->GetTotalSize() / kOneMeg,
               gpRenderBackend->GetVertexBufferPool_RT()->GetPeakArea()),
            &currentX,
            &currentY );
      }
#endif

      // details
      if (DebugStats::gDisplayRenderMemory == 2)
      {
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_VITA
         BP_Debug_DrawString( CStringExtras::Stringize_s( "Peaks: Scene: %0.2f Render: %0.2f RSX SYS: %0.2f RSX VID: %0.2f IndexUT %0.2f IndexRT %0.2f VertexUT %0.2f VertexRT %0.2f\n",
            gBP_SceneBufferPeakSize / kOneMeg,
            BP_RB_DebugGetPeakSize() / kOneMeg,
            systemVidStats.mPeakAllocatedSize / kOneMeg,
            videoVidStats.mPeakAllocatedSize / kOneMeg,
            gpRenderBackend->GetIndexBufferPool_UT()->GetPeakSize() / kOneMeg,
            gpRenderBackend->GetIndexBufferPool_RT()->GetPeakSize() / kOneMeg,
            gpRenderBackend->GetVertexBufferPool_UT()->GetPeakSize() / kOneMeg,
            gpRenderBackend->GetVertexBufferPool_RT()->GetPeakSize() / kOneMeg),
            &currentX,
            &currentY );

         BP_Debug_DrawString(CStringExtras::Stringize_s("RSX SYS: T: %0.2f MB (%d) A: %0.2f MB (%d) PF: %0.2f MB (%d) F: %0.2f MB (%d)\n", 
            systemVidStats.mTotalSize / kOneMeg, 
            systemVidStats.mTotalHandles, 
            systemVidStats.mAllocatedSize / kOneMeg, 
            systemVidStats.mAllocatedHandles, 
            systemVidStats.mPendingFreeSize / kOneMeg, 
            systemVidStats.mPendingFreeBlocks, 
            (systemVidStats.mTotalSize - systemVidStats.mAllocatedSize) / kOneMeg, 
            (systemVidStats.mTotalHandles - systemVidStats.mAllocatedHandles)), 
            &currentX, 
            &currentY);

         BP_Debug_DrawString(CStringExtras::Stringize_s("RSX VID: T: %0.2f MB (%d) A: %0.2f MB (%d) PF: %0.2f MB (%d) F: %0.2f MB (%d)\n", 
            videoVidStats.mTotalSize / kOneMeg, 
            videoVidStats.mTotalHandles, 
            videoVidStats.mAllocatedSize / kOneMeg, 
            videoVidStats.mAllocatedHandles, 
            videoVidStats.mPendingFreeSize / kOneMeg, 
            videoVidStats.mPendingFreeBlocks, 
            (videoVidStats.mTotalSize - videoVidStats.mAllocatedSize) / kOneMeg, 
            (videoVidStats.mTotalHandles - videoVidStats.mAllocatedHandles)), 
            &currentX, 
            &currentY);


#if 0
         BP_Debug_DrawString(CStringExtras::Stringize_s("Frame allocator usage: %.02f KB", BP_GetFrameAllocatorUsage() / 1024.0f), &currentX, &currentY);
         BP_Debug_DrawString(CStringExtras::Stringize_s("Edge shared failed alloc size: %.02f KB", BP_GetEdgeSharedBufferFailedAllocSize() / 1024.0f), &currentX, &currentY);
#endif

#endif

         BP_Debug_DrawString(CStringExtras::Stringize_s("Render Buffer: %d/%d (%d%% Used)", BP_RB_DebugGetUsedSize(), BP_RB_DebugGetTotalSize(), BP_RB_DebugGetUsedSize() * 100 / BP_RB_DebugGetTotalSize()), &currentX, &currentY);

         BP_Debug_DrawString(CStringExtras::Stringize_s("Index Buffer Pool (RT): Size - %d/%d (%d%% Used)", 
            gpRenderBackend->GetIndexBufferPool_RT()->GetInUseSize(), 
            gpRenderBackend->GetIndexBufferPool_RT()->GetTotalSize(), 
            gpRenderBackend->GetIndexBufferPool_RT()->GetInUseSize() * 100 / gpRenderBackend->GetIndexBufferPool_RT()->GetTotalSize()
            ), &currentX, &currentY);

         BP_Debug_DrawString(CStringExtras::Stringize_s("Vertex Buffer Pool (RT): Size - %d/%d (%d%% Used)", 
            gpRenderBackend->GetVertexBufferPool_RT()->GetInUseSize(), 
            gpRenderBackend->GetVertexBufferPool_RT()->GetTotalSize(), 
            gpRenderBackend->GetVertexBufferPool_RT()->GetInUseSize() * 100 / gpRenderBackend->GetVertexBufferPool_RT()->GetTotalSize()
            ), &currentX, &currentY);

         BP_Debug_DrawString(CStringExtras::Stringize_s("Index Buffer Pool (UT): Size - %d/%d (%d%% Used)", 
            gIndexBufferPoolUT_InUseSize, 
            gpRenderBackend->GetIndexBufferPool_UT()->GetTotalSize(), 
            gIndexBufferPoolUT_InUseSize * 100 / gpRenderBackend->GetIndexBufferPool_UT()->GetTotalSize()
            ), &currentX, &currentY);


         BP_Debug_DrawString(CStringExtras::Stringize_s("Vertex Buffer Pool (UT): Size - %d/%d (%d%% Used)", 
            gVertexBufferPoolUT_InUseSize, 
            gpRenderBackend->GetVertexBufferPool_UT()->GetTotalSize(), 
            gVertexBufferPoolUT_InUseSize * 100 / gpRenderBackend->GetVertexBufferPool_UT()->GetTotalSize()
            ), &currentX, &currentY);
      }
   }
   if( DebugStats::gDisplayTitleSafeArea )
   {
      float halfLineWidth = 2.0f/1280.0f;
      float halfTitleSafeArea = DebugStats::gDisplayTitleSafeAreaPercent/2.0f;
      float x0 = halfTitleSafeArea-halfLineWidth;
      x0 *= 2.0f;
      x0 -= 1.0f;
      float x1 = halfTitleSafeArea+halfLineWidth;
      x1 *= 2.0f;
      x1 -= 1.0f;

      float x2 = (1.0f-halfTitleSafeArea)-halfLineWidth;
      x2 *= 2.0f;
      x2 -= 1.0f;

      float x3 = (1.0f-halfTitleSafeArea)+halfLineWidth;
      x3 *= 2.0f;
      x3 -= 1.0f;

      float y0 = halfTitleSafeArea-halfLineWidth;
      y0 *= 2.0f;
      y0 -= 1.0f;
      float y1 = halfTitleSafeArea+halfLineWidth;
      y1 *= 2.0f;
      y1 -= 1.0f;

      float y2 = (1.0f-halfTitleSafeArea)-halfLineWidth;
      y2 *= 2.0f;
      y2 -= 1.0f;
      float y3 = (1.0f-halfTitleSafeArea)+halfLineWidth;
      y3 *= 2.0f;
      y3 -= 1.0f;
      CVector4 lineColor;
      if( DebugStats::gDisplayTitleSafeColor == 0 )
      {
         lineColor = CVector4(1.0f,1.0f,1.0f,1.0f);
      }
      else
      {
         lineColor = CVector4(0.0f,0.0f,0.0f,1.0f);
      }
      BP_DrawRectTextureModulate((CBaseTexture*)&gpRenderBackend->GetWhiteMap(), x0, y0, x3-x0, y1-y0, lineColor, CVector4(0, 0, 0, 0), 0, 1);
      BP_DrawRectTextureModulate((CBaseTexture*)&gpRenderBackend->GetWhiteMap(), x2, y0, x3-x2, y3-y0, lineColor, CVector4(0, 0, 0, 0), 0, 1);
      BP_DrawRectTextureModulate((CBaseTexture*)&gpRenderBackend->GetWhiteMap(), x0, y2, x3-x0, y3-y2, lineColor, CVector4(0, 0, 0, 0), 0, 1);
      BP_DrawRectTextureModulate((CBaseTexture*)&gpRenderBackend->GetWhiteMap(), x0, y0, x1-x0, y3-y0, lineColor, CVector4(0, 0, 0, 0), 0, 1);
      BP_EndShader();
   }

#if BP_VITA
#if BP_ENABLE_TESTNODE || BP_ENABLE_TESTTRI
   BP_DebugText_Print(
#if BP_ENABLE_TESTNODE && BP_ENABLE_TESTTRI
      "Nodes %d   Tris %d", gTestNodeCount, gTestTriCount
#elif BP_ENABLE_TESTNODE
      "Nodes %d", gTestNodeCount
#elif BP_ENABLE_TESTTRI
      "Tris %d", gTestNodeCount
#endif
      );
#endif

   extern int gDoRazorLiveMetrics;
   if (gDoRazorLiveMetrics)
   {
      gpRenderBackend->PrintLiveMetrics();
   }
#if BP_PRIM_DEBUGGING
   if (gShowPrims)
   {
      BP_Prim_DumpProfile();
   }
#endif
#if JADEBUG
   if (gShowModels)
   {
      BP_Model_DumpProfile();
   }
#endif
#endif
   if( DebugStats::gDisplayDebugText )
   {
   BP_DebugText_Render(gpRenderBackend->GetMainFrameBufferWidth() - currentX, initialY, CMTXFont::kRF_JustifyRight);
   }

   BP_DebugText_Clear();

#if BP_ENABLE_DEBUG_MENU
   BP_Debug_TrophyRender();
   BP_Debug_LoadingSpinnerRender();
   BP_DebugMenu_Render();
#endif

#endif
}

//----------------------------------------------------------------------------

#if BP_ENABLE_RENDER_LABELS
SBP_RenderLabel* gpLastRenderLable_DEBUG = NULL;
void BP_RenderLabel(char* pData)
{
   SBP_RenderLabel* pLabelData = (SBP_RenderLabel*)pData;
   gpLastRenderLable_DEBUG = pLabelData;
   BPE_ADD_SCOPED_GPU_PROFILE_MARKER(pLabelData->string);

   if( pLabelData->flags & gRenderLabel_ActiveMask )
   {
      BP_DebugText_Print("%s (%8.8x)", pLabelData->string, pLabelData->flags);
   }
}

#endif

//----------------------------------------------------------------------------

void BP_InitRasterMask()
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET
#if BPE_TARGET == BPE_TARGET_PS3

   if( !gNeedInitRasterMask )
      return;

   gNeedInitRasterMask = 0;

#if MARCO
   BP_DebugText_Print("BP_InitRasterMask");
#endif

   uint32_t mask[32];

   if( gRasterMask_SwitchPixels == 0 || gpRenderBackend->GetFrameCount() & 1 )
   {
      for( int i = 0; i < 32; ++i )
      {
         if( i & 1 )
            mask[i] = 0x55555555;
         else
            mask[i] = 0xAAAAAAAA;
      }
   }
   else
   {
      for( int i = 0; i < 32; ++i )
      {
         if( i & 1 )
            mask[i] = 0xAAAAAAAA;
         else
            mask[i] = 0x55555555;
      }
   }

   cell::Gcm::cellGcmSetPolygonStipplePattern(mask);
#endif

#endif // !BP_RENDER_SINGLE_RENDER_TARGET
}

inline bool CanDoMSAARasterMaskTrick(CBaseTexture::EAntiAliasType colorBufferAAType)
{
   return gRasterMaskEmulationMode == kRME_MSAAx2 && gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage == 0 && colorBufferAAType == CBaseTexture::kAA_MSAA2x;
}

void BP_SetRasterMask(int enable)
{
#if BPE_TARGET == BPE_TARGET_VITA

   gpRenderBackend->SetRasterMask(enable);

#else

   if( gRasterMaskEmulationMode != kRME_None && enable )
   {
      BP_InitRasterMask();

      SRenderTarget const & originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
      if( CanDoMSAARasterMaskTrick(originalRenderTarget.mpColorBuffer[0]->GetAntiAliasType()) )
      {

#if BPE_TARGET == BPE_TARGET_PS3
         // MSAAx2 pixels are invisible pattern
         int const bitValue = 
            ((1<<0) | (0<<1)) << 0 |
            ((1<<0) | (0<<1)) << 4 |
            ((1<<0) | (0<<1)) << 8 |
            ((1<<0) | (0<<1)) << 12;
         cell::Gcm::cellGcmSetAntiAliasingControl(true, CELL_GCM_FALSE, CELL_GCM_FALSE, bitValue );
#elif BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_WIN32
         // MSAAx2 pixels are invisible pattern
         int const bitValue = 
            ((1<<0) | (0<<1)) << 0 |
            ((1<<0) | (0<<1)) << 4 |
            ((1<<0) | (0<<1)) << 8 |
            ((1<<0) | (0<<1)) << 12;
         gpRenderBackend->GetDirect3DDevice()->SetRenderState(D3DRS_MULTISAMPLEMASK, bitValue);
#else
         gpRenderBackend->SetStencilEnable(true);
#endif

      }
      else
      {
#if BPE_TARGET == BPE_TARGET_PS3
         cell::Gcm::cellGcmSetPolygonStippleEnable(1);
#elif BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_WIN32
         // MSAAx0 Checkerboard pattern
         int bitValue;
         if( gRasterMask_SwitchPixels == 0 || gpRenderBackend->GetFrameCount() & 1 )
         {
            bitValue = 
               (0xF) << 0 |
               (0x0) << 4 |
               (0x0) << 8 |
               (0xF) << 12;
         }
         else
         {
            bitValue = 
               (0x0) << 0 |
               (0xF) << 4 |
               (0xF) << 8 |
               (0x0) << 12;
         }
         gpRenderBackend->GetDirect3DDevice()->SetRenderState(D3DRS_MULTISAMPLEMASK, bitValue);
#else
         gpRenderBackend->SetStencilEnable(true);
#endif
      }
   }
   else
   {
#if BPE_TARGET == BPE_TARGET_PS3
      SRenderTarget const & originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
      if( CanDoMSAARasterMaskTrick(originalRenderTarget.mpColorBuffer[0]->GetAntiAliasType()) )
      {
         cell::Gcm::cellGcmSetAntiAliasingControl(true, CELL_GCM_FALSE, CELL_GCM_FALSE, 0xFFFFFFFF );
      }
      else
      {
         cell::Gcm::cellGcmSetPolygonStippleEnable(0);
      }
#elif BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_WIN32
      gpRenderBackend->GetDirect3DDevice()->SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
#else
      gpRenderBackend->SetStencilEnable(false);
#endif
   }
#endif
}

//----------------------------------------------------------------------------

void BP_UpdateDynamicTexture_Command(char* pData)
{
   SBP_UpdateDynamicTexture* pPacket = (SBP_UpdateDynamicTexture*)pData;
   switch(pPacket->bitsPerPixel)
   {
   case 4:
      BP_UpdateDynamicTextureCLUT16(pPacket->bp_tex, pPacket->vram, pPacket->clut);
      break;

   case 8:
      BP_UpdateDynamicTextureCLUT256(pPacket->bp_tex, pPacket->vram, pPacket->clut);
      break;

   case 32:
      BP_UpdateDynamicTextureRGBA(pPacket->bp_tex, pPacket->vram);
      break;
   }
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 2
#define SBP_LAST_LOWRES_CHANNEL 1
#elif MGS_VERSION == 3
#define SBP_LAST_LOWRES_CHANNEL 0
#else
#define SBP_LAST_LOWRES_CHANNEL -2
#endif

void BP_FrameBuffer( char* pData )
{
   SBP_FrameBuffer *pPacket = (SBP_FrameBuffer *) pData;

#if BP_VITA
   if (pPacket->channel > SBP_LAST_LOWRES_CHANNEL)
   {
      gpRenderBackend->SetRenderTarget(
         SRenderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer_Full), NULL, NULL, NULL, 
                       BP_GetRenderTarget(kRT_CurrentDepthBuffer_Full)));
   }
   else
   {
      gpRenderBackend->SetRenderTarget(
         SRenderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer_Small), NULL, NULL, NULL, 
                       BP_GetRenderTarget(kRT_CurrentDepthBuffer_Small)));
   }
#endif
}

void BP_SetViewport( char* pData )
{
   SRenderTarget target = gpRenderBackend->GetCurrentRenderTarget();   

   SBP_SetViewport *pPacket = (SBP_SetViewport *) pData;

   int backWidth = target.mpColorBuffer[0] ? target.mpColorBuffer[0]->GetAntiAliasWidth(target.mpColorBuffer[0]->GetAntiAliasType()) : gpRenderBackend->GetBackBufferWidth();
   int backHeight = target.mpColorBuffer[0] ? target.mpColorBuffer[0]->GetAntiAliasHeight(target.mpColorBuffer[0]->GetAntiAliasType()) : gpRenderBackend->GetBackBufferHeight();

   float hscale, vscale;

   if (backWidth > 512)
      hscale = (float)backWidth / DRAW_WIDTH;
   else
      hscale = 1.0f;

   if (backWidth > 512)
      vscale = (float)backHeight / DRAW_HEIGHT;
   else
      vscale = 1.0f;
#if MGS_VERSION == 2
   int x = ( pPacket->x + DRAW_WIDTH/2 - pPacket->width/2 ) * hscale;
   int y = ( pPacket->y + DRAW_HEIGHT/2 - pPacket->height/2 ) * vscale;
#else
   int x = pPacket->x * hscale;
   int y = pPacket->y * vscale;
#endif

   int width = pPacket->width * hscale;
   int height = pPacket->height * vscale;

   width = bpe::max_val(1, width);
   height = bpe::max_val(1, height);

   if ( x == 0 && y == 0 && width == backWidth && height == backHeight )
   {
      target.mRenderTargetViewPort.mViewportEnabled = 0;
   }
   else
   {
      target.mRenderTargetViewPort.mViewportEnabled = 1;
   }

   target.mRenderTargetViewPort.mViewportX = x;
   target.mRenderTargetViewPort.mViewportY = y;
   target.mRenderTargetViewPort.mViewportWidth = width;
   target.mRenderTargetViewPort.mViewportHeight = height;

   gpRenderBackend->SetRenderTarget( target );

   gFrameInfoStruct.lastViewportInfo = target.mRenderTargetViewPort;
}

//----------------------------------------------------------------------------

void BP_ClearViewport(char* pData)
{
   gFrameInfoStruct.hasExecutedClear = true;

   SBP_ClearViewport* pPacket = (SBP_ClearViewport*)pData;
   
   gpRenderBackend->SetStencilEnable(false);

   int clearStencilFlags = 0;
#if BPE_TARGET != BPE_TARGET_PS3
   // Only need stencil clear on non PS3 platforms for raster mask emulation.
   clearStencilFlags = CRenderBackend::kFlag_Stencil;
#endif

#if MGS_VERSION == 3
   int bpClearFlags = 0;

   if( pPacket->clearFlags & DG_VIEWPORT_CLEARFRAME )
      bpClearFlags |= CRenderBackend::kFlag_Color;

   if( pPacket->clearFlags & DG_VIEWPORT_CLEARDEPTH )
   {
      bpClearFlags |= CRenderBackend::kFlag_Depth|clearStencilFlags;
      gNeedInitRasterMask = 1;
   }

   CColor const clearColor = BP_DecodeColor(pPacket->clearColor);

   if( bpClearFlags )
      gpRenderBackend->Clear(bpClearFlags, clearColor, 0.0f);

#endif

#if MGS_VERSION == 2
   CColor const clearColor = BP_DecodeColor(pPacket->clearColor);

   if( pPacket->clearFlags == 2 )
   {
      gpRenderBackend->Clear(CRenderBackend::kFlag_Depth|clearStencilFlags, clearColor, 0.0f);
   }
   else
   {
      gpRenderBackend->Clear(CRenderBackend::kFlag_Color|CRenderBackend::kFlag_Depth|clearStencilFlags, clearColor, 0.0f);
   }

   gNeedInitRasterMask = 1;

#endif
}

//----------------------------------------------------------------------------

void BP_FrameFirstPacket()
{
   gFrameInfoStruct.Reset();
#if BP_ENABLE_TESTTRI
   gTestTriCur = 0;
   gTestTriActive = gTestTriEnabled;
   if (gTestTriReset)
   {
      gTestTriStart = 0;
   }
#endif

#if BP_PRIM_DEBUGGING
   if (gDumpingPrims)
      gDumpingPrims = 0;
   if (gDumpPrims)
   {
      gDumpingPrims = 1;
      gDumpPrims = 0;
   }
   if (gClearPrims)
   {
      gClearPrims = 0;
      BP_Prim_ResetProfile();
   }
   else
   {
      BP_Prim_ClearProfile();
   }
#endif
#if JADEBUG
   if (gClearModels)
   {
      gClearModels = 0;
      BP_Model_ResetProfile();
   }
   else
   {
      BP_Model_SwapProfile();
   }
#endif
}

void BP_FrameNewViewportInfoPacket(char* pData)
{
   gFrameInfoStruct.pViewportInfo = (SBP_ViewportInfo*)pData;
}

//----------------------------------------------------------------------------

SBP_ViewportInfo * gpViewportInfo;
extern "C" void BP_SetViewportInfoPacket(SBP_ViewportInfo * pData)
{
   gpViewportInfo = pData;
}

extern "C" SBP_ViewportInfo * BP_GetCurrentViewportInfo()
{
   return gpViewportInfo;
}

//----------------------------------------------------------------------------

void BP_RestoreFrameInitTestValue()
{
   BP_GS_SetTest(gFrameTestValue);
}

//----------------------------------------------------------------------------

void BP_MSAA_FinishTilingAndResolve()
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   gFrameInfoStruct.currRenderTarget = kRT_CurrentFrameBuffer_NoMSAA;
   gFrameInfoStruct.disableMSAASwitch = true;

   CBaseTexture * pCurrentColorBuffer = BP_GetRenderTarget(kRT_CurrentFrameBuffer);
   CBaseTexture::EAntiAliasType antiAliasType = pCurrentColorBuffer->GetAntiAliasType();
   
   CBaseTexture * pCurrentDepthBuffer0 = BP_GetRenderTarget(kRT_DefaultDepthBuffer);
   CBaseTexture * pCurrentDepthBuffer1 = NULL;

#if defined(BP_360)
   // We do not create any texture memory for kRT_MainBuffer or kRT_DefaultDepthBuffer1, this allows us to save 7 megs (3.5 color, 3.5 depth)
   // instead we reuse the textures kRT_CurrentFrameBuffer_NoMSAA && kRT_DefaultDepthBuffer_NoMSAA that would normally represent this
   // texture anyways without all the EDRAM separate texture memory madness
   // This is only to save memory, we still have to copy the color/depth into EDRAM...
   pCurrentColorBuffer = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
   pCurrentDepthBuffer1 = BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA);
   if( gFrameInfoStruct.hasExecutedClear )
   {
      gpRenderBackend->ResolveRenderTargetPredicated(pCurrentColorBuffer);
      gpRenderBackend->ResolveDepthStencilPredicatedMSAA2(pCurrentDepthBuffer0, pCurrentDepthBuffer1);
      }
      else
      {
      // When exiting survival viewer we get one frame single frame with no clear commands
      // Because the EDRAM memory is being interpreted in different layouts based on if we are in MSAA mode or not on X360
      // The correct thing to do is to leave leave the previous frames results in the buffer
      // the textures, to do this we just use kRT_PreviousFrameBuffer_NoMSAA
      pCurrentColorBuffer = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
      }
#endif

   gpRenderBackend->EndTiling();

   CBaseTexture* pCurrentFrameBuffer_NoMSAA = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
   SRenderTarget rt(pCurrentFrameBuffer_NoMSAA, NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA));

#if MGS_VERSION == 3
   float pCurrentFrameBuffer_NoMSAA_Width_as_float = (float)pCurrentFrameBuffer_NoMSAA->GetWidth();
   float pCurrentFrameBuffer_NoMSAA_Height_as_float = (float)pCurrentFrameBuffer_NoMSAA->GetHeight();
   SRectf uvRect = { 
      gFrameInfoStruct.lastViewportInfo.mViewportX / pCurrentFrameBuffer_NoMSAA_Width_as_float, 
      gFrameInfoStruct.lastViewportInfo.mViewportY / pCurrentFrameBuffer_NoMSAA_Height_as_float, 
      (gFrameInfoStruct.lastViewportInfo.mViewportX + gFrameInfoStruct.lastViewportInfo.mViewportWidth) / pCurrentFrameBuffer_NoMSAA_Width_as_float, 
      (gFrameInfoStruct.lastViewportInfo.mViewportY + gFrameInfoStruct.lastViewportInfo.mViewportHeight) / pCurrentFrameBuffer_NoMSAA_Height_as_float     
   };

   rt.mRenderTargetViewPort = gFrameInfoStruct.lastViewportInfo;
   gpRenderBackend->SetRenderTarget(rt);
   BP_ResolveColorAndDepth(pCurrentColorBuffer, pCurrentDepthBuffer0, pCurrentDepthBuffer1, NULL, NULL, gEnableMSAA_ZCullReload, antiAliasType, uvRect);

#else
   SRectf uvRect = { 0.0f, 0.0f, 1.0f, 1.0f };

   gpRenderBackend->SetRenderTarget(rt);
   BP_ResolveColorAndDepth(pCurrentColorBuffer, pCurrentDepthBuffer0, pCurrentDepthBuffer1, NULL, NULL, gEnableMSAA_ZCullReload, antiAliasType, uvRect);
#endif

   rt.mRenderTargetViewPort = gFrameInfoStruct.lastViewportInfo;
   gpRenderBackend->SetRenderTarget(rt);
#endif
}

//----------------------------------------------------------------------------

void BP_FrameInitPacket(char* pData)
{
   SBP_FrameInitPacket* pPacket = (SBP_FrameInitPacket*)pData;
   gFrameTestValue = pPacket->testValue;
   BP_GS_SetTest(pPacket->testValue);
   BP_GS_SetupAlphaTestSinglePass(false);
   
   gBP_FogColor = BP_DecodeColorF(pPacket->fogColor);

   // Need to do this to make sure any prims in this viewport pick up the correct fog color.
   BP_Prim_BeginViewport();
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   // Special case handling when MSAA is enabled
   if( gEnableMSAA && !gFrameInfoStruct.disableMSAASwitch )
   {
      if( gFrameInfoStruct.pViewportInfo->hasMSAAStuff )
      {
         SRenderTarget renderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer), NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer));
         renderTarget.mRenderTargetViewPort = gFrameInfoStruct.lastViewportInfo;

         if( gFrameInfoStruct.currRenderTarget != kRT_CurrentFrameBuffer )
         {
            gFrameInfoStruct.currRenderTarget = kRT_CurrentFrameBuffer;     
            gpRenderBackend->SetRenderTarget(renderTarget);
         }
         gpRenderBackend->BeginTiling(renderTarget);
      }
      else if( gFrameInfoStruct.pViewportInfo->hasNonMSAAStuff )
      {
         if( gFrameInfoStruct.currRenderTarget != kRT_CurrentFrameBuffer_NoMSAA )
         {
            BP_MSAA_FinishTilingAndResolve();

            SRenderTarget rt(BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA), NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA));
            rt.mRenderTargetViewPort = gFrameInfoStruct.lastViewportInfo;
            
            gpRenderBackend->SetRenderTarget(rt);
         }
      }
      else
      {
         SRenderTarget renderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer), NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer));
         renderTarget.mRenderTargetViewPort = gFrameInfoStruct.lastViewportInfo;

         if( gFrameInfoStruct.currRenderTarget != kRT_CurrentFrameBuffer )
         {
            gFrameInfoStruct.currRenderTarget = kRT_CurrentFrameBuffer;         
            gpRenderBackend->SetRenderTarget(renderTarget);
         }
         gpRenderBackend->BeginTiling(renderTarget);
      }
   }

#endif //!BP_RENDER_SINGLE_RENDER_TARGET

#if BP_VITA
   BP_PostFx_ResetDmapackFX();
#endif
}

//----------------------------------------------------------------------------

void BP_FrameMSAAEndPacket()
{
   if( gFrameInfoStruct.pViewportInfo->hasNonMSAAStuff )
   {
      if( gFrameInfoStruct.currRenderTarget == kRT_CurrentFrameBuffer )
      {
         BP_MSAA_FinishTilingAndResolve();
      }
   }
}

//----------------------------------------------------------------------------

void BP_FrameStoreBegin(char* pData)
{
   SBP_FrameStorePacket* pPacket = (SBP_FrameStorePacket*)pData;
#if BP_VITA
   CBaseTexture *tex = gpRenderBackend->GetCurrentRenderTexture();
   gpRenderBackend->SetRenderTarget(SRenderTarget((CBaseTexture*)pPacket->bpTexture, NULL, NULL, NULL, 
                                                  gpRenderBackend->GetLowresDrawDepth()));
#  if MGS_VERSION==3
   if (~pPacket->clearflags & DG_VIEWPORT_CLEARFRAME)
   {
      BP_DrawFullscreenTexture(tex, true, false, 1);
   }
#  endif
#endif
}

//----------------------------------------------------------------------------

void BP_FrameStoreEnd(char* pData)
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET
#  if MGS_VERSION == 3

   SBP_FrameStorePacket* pPacket = (SBP_FrameStorePacket*)pData;

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   CBaseTexture* pSourceTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);

   int const sourceWidth = pSourceTexture->GetWidth();
   int const sourceHeight = pSourceTexture->GetHeight();

   SRect srcRect(
      pPacket->x0 * sourceWidth / DRAW_WIDTH,
      pPacket->y0 * sourceHeight / DRAW_HEIGHT,
      pPacket->x1 * sourceWidth / DRAW_WIDTH,
      pPacket->y1 * sourceHeight / DRAW_HEIGHT
   );

   // Resolve everything we have rendered so far on X360
   gpRenderBackend->ResolvePartialRenderTarget(pSourceTexture, srcRect);

   CBaseTexture* pDestTexture = (CBaseTexture*)pPacket->bpTexture;
   
   BP_CopyTextureRect(pSourceTexture, pDestTexture, srcRect.x1, srcRect.y1, srcRect.x2, srcRect.y2);

   // Resolve "copy operation" on X360.
   gpRenderBackend->ResolveRenderTarget(pDestTexture);

   gpRenderBackend->SetRenderTarget(originalRenderTarget);

   gFrameInfoStruct.disableMSAASwitch = false; // It is okay to switch back to MSAA mode if we back up the previous frame
#  else
   SBP_FrameStorePacket* pPacket = (SBP_FrameStorePacket*)pData;

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   CBaseTexture* pSourceTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);

   int const sourceWidth = pSourceTexture->GetWidth();
   int const sourceHeight = pSourceTexture->GetHeight();

   SRect srcRect(
      pPacket->x0 * sourceWidth / DRAW_WIDTH,
      pPacket->y0 * sourceHeight / DRAW_HEIGHT,
      pPacket->x1 * sourceWidth / DRAW_WIDTH,
      pPacket->y1 * sourceHeight / DRAW_HEIGHT
      );

   CBaseTexture* pDestTexture = (CBaseTexture*)pPacket->bpTexture;

   BP_CopyTextureRect(pSourceTexture, pDestTexture, srcRect.x1, srcRect.y1, srcRect.x2, srcRect.y2);

   // Resolve "copy operation" on X360.
   gpRenderBackend->ResolveRenderTarget(pDestTexture);

   gpRenderBackend->SetRenderTarget(originalRenderTarget);

   gFrameInfoStruct.disableMSAASwitch = false; // It is okay to switch back to MSAA mode if we back up the previous frame
#  endif // MGS_VERSION
#endif // BP_RENDER_SINGLE_RENDER_TARGET
}

//----------------------------------------------------------------------------

void BP_CopyRenderTarget(ERenderTarget const dest)
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Copy Render Target");

   SRenderTarget const original = gpRenderBackend->GetCurrentRenderTarget();

   CBaseTexture* pSourceTexture = original.mpColorBuffer[0];
   CBaseTexture* pDestTexture = BP_GetRenderTarget(dest);

   gpRenderBackend->ResolveRenderTarget(pSourceTexture);

   BP_CopyTexture(pSourceTexture, pDestTexture);
   gpRenderBackend->ResolveRenderTarget(pDestTexture);

   gpRenderBackend->SetRenderTarget(original);

#endif
}

//----------------------------------------------------------------------------

void BP_RestoreFogColor()
{
   gpRenderBackend->SetVertexRegisters(kSReg_FogColor, 1, &gBP_FogColor);
}

//----------------------------------------------------------------------------

void BP_DisplayCurrentBuffer(int shouldDrawMode)
{
   CBaseTexture * pCurrentBuffer = NULL;

#if BPE_TARGET!=BPE_TARGET_VITA 
   if( shouldDrawMode )
   {
      if( gFrameInfoStruct.currRenderTarget == kRT_CurrentFrameBuffer_NoMSAA )
      {
         pCurrentBuffer = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
         gpRenderBackend->ResolveRenderTarget(pCurrentBuffer);
      }
      else
      {
         // In this case we never had to draw anything that was not MSAA
         // TODO: it sucks to have to draw to kRT_MainBufferX_NoMSAA, just to then put the result in front buffer...

         BP_MSAA_FinishTilingAndResolve();

         pCurrentBuffer = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
         gpRenderBackend->ResolveRenderTarget(pCurrentBuffer);
      }
   }
   else
   {
      pCurrentBuffer = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
      gpRenderBackend->ResolveRenderTarget(pCurrentBuffer);
   }

   // Clear to fill areas that not covered by letter boxing
   if( gpRenderBackend->GetFrameBufferSize().GetDisplayAspect() == CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen )
   {
      // NOTE: This code ONLY executes when running in SD resolutions that are fullscreen mode.
      SRenderTarget rt(NULL, NULL, NULL, NULL, NULL, SRenderTarget::kBBM_BackBufferFill);
      gpRenderBackend->SetRenderTarget(rt);
      gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor::Black());
   }

      SRenderTarget rt(NULL, NULL, NULL, NULL, NULL, SRenderTarget::kBBM_BackBufferCentered);
      gpRenderBackend->SetRenderTarget(rt);
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetAlphaTestEnable(false);
      gpRenderBackend->SetDepthCompareEnabled(false);
      gpRenderBackend->SetStencilEnable(false);

   int isFilter = 0;
   
   if( gpRenderBackend->GetBackBufferWidth() != pCurrentBuffer->GetWidth() ||
       gpRenderBackend->GetBackBufferHeight() != pCurrentBuffer->GetHeight() )
   {
      isFilter = 1;
   }

   BP_DrawFullscreenTexture(pCurrentBuffer, isFilter, 0);

   BP_RenderLoadingSpinner();

#endif
}

//----------------------------------------------------------------------------

void BP_DebugRender_ProfileBarsText()
{
#if BP_ENABLE_ONSCREEN_PROFILER
   static bool sFirstRender = true;
   if( sFirstRender )
   {
      sFirstRender = false;
   }
   else
   {
      if( BP_IsProfileInfoActive() )
      {
         float previousTime = 0.0f;

         // Show GPU markers
         if( gShowProfileInfo >= 3 )
         {
            BP_DebugText_Print("GPU");
            int const firstVisibleRsxMarker = gShowProfileInfoOffset;
            for( int currentRsxMarker = 0; currentRsxMarker < ProfileBars::GetNumGPUMarker(); ++currentRsxMarker )
            {
               char const * pName;
               float timeMS;
               ProfileBars::GetGPUMarkerDisplayData(currentRsxMarker, &pName, &timeMS, previousTime);

               if( currentRsxMarker >= firstVisibleRsxMarker )
               {
                  BP_DebugText_Print("%s: %3.3f ms", pName, timeMS);
               }
            }
            BP_DebugText_Print("");
         }

         // Show GPU totals
         if( ProfileBars::GetNumGPUMarker() > 0 )
         {
            float totalMS = ProfileBars::GetGPUTotalTime();
            real32 const total720pFP = totalMS * (1280.0f * 720.0f) / (gpRenderBackend->GetMainFrameBufferWidth() * gpRenderBackend->GetMainFrameBufferHeight());

            BP_DebugText_Print("GPU Total: %.3f ms", totalMS);
            BP_DebugText_Print("GPU Total (720p FP): ~%3.3f ms", total720pFP);
            BP_DebugText_Print("");
         }

         // Show CPU markers
         if( gShowProfileInfo >= 3 )
         {
            int const firstVisiblePpuMarker = gShowProfileInfoOffset;

            for( int iCPU = 0; iCPU < ProfileBars::GetNumCPU(); ++iCPU )
            {
               if( ProfileBars::GetNumCPUMarker(iCPU) )
               {
                  int markersDisplayed = 0;
                  float totalMS = 0;

                  BP_DebugText_Print("CPU %d", iCPU);

                  for( int currentPpuMarker = 0; currentPpuMarker < ProfileBars::GetNumCPUMarker(iCPU); ++currentPpuMarker )
                  {
                     char const * pName;
                     float timeMS;
                     ProfileBars::GetCPUMarkerDisplayData(iCPU, currentPpuMarker, &pName, &timeMS, previousTime);

                     if( currentPpuMarker >= firstVisiblePpuMarker && *pName != 0 )
                     {
                        totalMS += timeMS;
                        ++markersDisplayed;
                        BP_DebugText_Print("%s: %3.3f ms", pName, timeMS);
                     }
                  }

                  // Only display total if it makes sense!
                  if( markersDisplayed > 1 )
                     BP_DebugText_Print("Total: %3.3f ms", totalMS);

                  BP_DebugText_Print("");
               }
            }
         }

         BP_DebugText_Print("Render CPU: %3.3f ms", gCurrentRenderTime * 1000.0f);
         BP_DebugText_Print("Game CPU: %3.3f ms", sCurrentEndframeTime * 1000.0f);
      }
   }
#endif
}

//----------------------------------------------------------------------------

#if BP_ENABLE_PROFILE_MARKERS

SBP_ProfileRegionMarker* gpLastProfileRegionMarker = NULL;

void BP_Render_PushProfileRegionMarker(void* pData)
{
   gpLastProfileRegionMarker = (SBP_ProfileRegionMarker*)pData;

#if BPE_TARGET==BPE_TARGET_VITA
   sceRazorCpuPushMarkerWithHud( gpLastProfileRegionMarker->string, gpLastProfileRegionMarker->color, SCE_RAZOR_MARKER_ENABLE_HUD );
#endif
   BPE_GPU_PUSH_PROFILE_MARKER(gpLastProfileRegionMarker->string);
}

void BP_Render_PopProfileRegionMarker()
{
   BPE_GPU_POP_PROFILE_MARKER();

#if BPE_TARGET==BPE_TARGET_VITA
   sceRazorCpuPopMarker();
#endif

   // Only leaf level profile markers get added to the PPU/GPU profile graph
   if( gpLastProfileRegionMarker )
   {
      PROFILE_ADD_PPU(gpLastProfileRegionMarker->color, gpLastProfileRegionMarker->string);
      PROFILE_ADD_GPU(gpLastProfileRegionMarker->color, gpLastProfileRegionMarker->string);

      // Ensure commands are executed as fast as possible up to this point!
#if BPE_TARGET == BPE_TARGET_PS3
      cell::Gcm::cellGcmFlush();
#endif
   }

   gpLastProfileRegionMarker = NULL;
}
#endif

//----------------------------------------------------------------------------

void BP_Render_PushProfileMarker(void* pData)
{
   BP_Debug_PushCPUMarker( (char const *) pData );

   BPE_GPU_PUSH_PROFILE_MARKER((char*)pData);
}

void BP_Render_PopProfileMarker()
{
   BP_Debug_PopCPUMarker();

   BPE_GPU_POP_PROFILE_MARKER();
}

//----------------------------------------------------------------------------

int gDemoDie_FirstFrame = 1;
CStopWatch demoDieTimer;
// AS(JM) - Three "diddemodie" flags. One for each thread, and we pass
// this flag from thread to thread as it's set
extern "C" int gAS_DidDemoDie_Game = 0;
extern "C" int gAS_DidDemoDie_Endframe = 0;
extern "C" int gAS_DidDemoDie_Render = 0;

void BP_DrawDemoFade(int shouldDrawMode)
{
   if( gAS_DidDemoDie_Render && !shouldDrawMode)
   {
      if( gDemoDie_FirstFrame )
      {
         gDemoDie_FirstFrame = 0;
         demoDieTimer.Reset();
      }

      BP_GS_SetAlphaTestEnabled( false );
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);

      gpRenderBackend->SetDepthCompareEnabled(false);

      real32 const alpha = bpe::min_val( 1.0f, bpe::max_val(0.0f, demoDieTimer.GetElapsedTime() / 0.25f) );

      BP_DrawFullscreenTextureModulate(&gpRenderBackend->GetWhiteMap(), CVector4(0, 0, 0, alpha), 0, 1);
   }
}

//----------------------------------------------------------------------------

void BP_RenderPrologue(int currentRenderBuffer, int shouldDrawMode)
{
   renderWatch.Reset();

   PROFILE_GPU_BEGIN_FRAME();

#if BP_ENABLE_SHADER_RELOAD

#if MGS_VERSION == 3
   if( GV_PadPress(GV_PadData(0)) & _PAD_R3 )
#else
   if ( GV_PadData[0].press & PAD_AR )
#endif
      BP_ReloadShaders();
#endif

#if BP_ENABLE_RENDER_LABELS
   // Update mask of active render labels
   gRenderLabel_ActiveMask = 0;
   gRenderLabel_ActiveMask |= gRenderLabel_DmaPack ? kRL_DmaPack : 0;
   gRenderLabel_ActiveMask |= gRenderLabel_PostFx ? kRL_PostFx : 0;
#endif

   if( currentRenderBuffer != -1 )
   {
      mCurrentRenderBuffer = currentRenderBuffer;
   }

   gPreviousFrameMetrics = gpRenderBackend->Metrics();

   BP_SoundSupport_BeginFrame();

   BP_Prim_FrameBegin();

#if MGS_VERSION == 2
   BP_Comdl_FrameBegin();
   BP_Obj_FrameBegin();
   BP_Evm_FrameBegin();
#elif MGS_VERSION == 3
   BP_FixModel_FrameBegin();
   BP_Model_FrameBegin();
   BP_Clone_FrameBegin();
#endif

   BP_Camera_Update();

#if BP_ENABLE_TEXTURE_TOOL
   BP_TextureTool_BeginFrame();
#endif

   BP_Debug_BeginFrame();

#if BP_VITA
   if (gVTADebugRescaleToggle && (BP_GetFrameCount() % 30 == 0))
      gVTADebugRescale = !gVTADebugRescale;
#endif

   gpRenderBackend->BeginScene();
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

#if BPE_TARGET == BPE_TARGET_PS3
   // Update gamma according to debug menu
   {
      float gamma = BP_Movie_IsPlayingFullscreen() ? gMovieGamma : gGameGamma;
      if( gPS3VideoOutGamma != gamma )
      {
         gPS3VideoOutGamma = gamma;
         cellVideoOutSetGamma(CELL_VIDEO_OUT_PRIMARY, gPS3VideoOutGamma);
      }
   }
#endif

   BP_SetMainRenderTarget_RenderPrologue(mCurrentRenderBuffer, shouldDrawMode);

#endif

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
}

//----------------------------------------------------------------------------

void BP_RenderEpilogue(int shouldDrawMode)
{
#if BP_VITA
   // force a scene transition to the display buffer if there isn't one yet
   gpRenderBackend->SetRenderTarget(
      SRenderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer_Full), NULL, NULL, NULL, 
                    BP_GetRenderTarget(kRT_CurrentDepthBuffer_Full)));
#endif

   BP_SoundSupport_EndFrame();

   BP_Prim_FrameEnd();
#if MGS_VERSION == 2
   BP_Comdl_FrameEnd();
   BP_Obj_FrameEnd();
   BP_Evm_FrameEnd();
#elif MGS_VERSION == 3
   BP_FixModel_FrameEnd();
   BP_Model_FrameEnd();
   BP_Clone_FrameEnd();
#endif

#if BP_ENABLE_TEXTURE_TOOL
   BP_TextureTool_EndFrame();
#endif

   BP_Debug_EndFrame();

#if BP_ENABLE_DEBUG_PRIM
   BP_DebugPrim_Render();
   BP_DebugPrim_Clear();
#endif

   BP_DisplayCurrentBuffer(shouldDrawMode);

   BP_DrawDemoFade(shouldDrawMode);

   BP_RenderLoadingSpinner();

   BP_Splash_UpdateAndRender();

#if BP_TGS_DEMO()
   BP_TGS_DEMO_Render();
#endif

#if BP_ENABLE_TESTTRI
   gTestTriCount = gTestTriCur;
   // keep the debug menu from getting culled
   gTestTriActive = false;
   if (gTestTriReset)
   {
      gTestTriReset = 0;
      gTestTriEnd = gTestTriCount;
   }
#endif

   BP_DebugRender();

#if BP_ENABLE_ONSCREEN_PROFILER
   if( gShowProfileInfo >= 2 )
   {
      gpRenderBackend->SetAlphaTestEnable(false);
      ProfileBars::Render();
   }
#endif

   // Our finalize time spend rendering
   PROFILE_ADD_GPU(kProfileColor_Finalize, "Finalize");
   PROFILE_ADD_PPU(kProfileColor_Finalize, "Finalize");

   PROFILE_GPU_END_FRAME();

   // Grab CPU render time before calling EndScene (which stalls on VSync)
   gCurrentRenderTime = renderWatch.GetElapsedTime();

   gpRenderBackend->EndScene();

   if ( !shouldDrawMode )
   {
#if BP_VITA
      if ( gpRenderBackend->GetFrameCount() != gBP_Vita_LastShaderFlushFrame + 1 )
      {
         NVTAState::UnregisterAllTemporaryPrograms();
      }

      // Always set the last flush frame so that consecutive frames don't cause the flush
      gBP_Vita_LastShaderFlushFrame = gpRenderBackend->GetFrameCount();
#endif
   }

   PROFILE_GPU_END(0x00000000, "EndLastMarker");
   // Finalize last shader used, so it doesn't get carried over to the next frame!
   BP_EndShader();
}

//----------------------------------------------------------------------------

void BP_RenderCommands(int currentRenderBuffer)
{
   SRenderCommand* pLastCmd0_DEBUG = NULL;
   SRenderCommand* pLastCmd1_DEBUG = NULL;
   SRenderCommand* pCmd = BP_RB_GetFirstCommand(currentRenderBuffer);
   if( pCmd != NULL )
   {
      for( ; pCmd != NULL; pCmd = pCmd->mpNext)
      {
         if ( !BP_RB_IsCommandValid( currentRenderBuffer, pCmd ) )
         {
            printf( "****** BEG ERROR ******\n" );
            printf( "Invalid command pCmd: 0x%8.8x\n", pCmd );
            printf( "Prior commands: 0x%8.8x 0x%8.8x\n", pLastCmd0_DEBUG, pLastCmd1_DEBUG );
            printf( "****** END ERROR ******\n" );
         }

         switch(pCmd->mId)
         {
   #if BP_ENABLE_RENDER_LABELS
         case kCmd_Label:
            BP_RenderLabel(pCmd->mData);
            break;
   #endif

         case kCmd_PushProfileMarker:
            BP_Render_PushProfileMarker(pCmd->mData);
            break;
         
         case kCmd_PopProfileMarker:
            BP_Render_PopProfileMarker();
            break;

            case kCmd_PushProfileRegionMarker:
   #if BP_ENABLE_PROFILE_MARKERS
               BP_Render_PushProfileRegionMarker(pCmd->mData);
   #endif
               break;

            case kCmd_PopProfileRegionMarker:
   #if BP_ENABLE_PROFILE_MARKERS
               BP_Render_PopProfileRegionMarker();
   #endif
               break;

         case kCmd_ClearViewport:
            BP_ClearViewport(pCmd->mData);
            break;

         case kCmd_SetViewport:
            BP_SetViewport( pCmd->mData );
            break;

         case kCmd_DmaPack_AutoPacket:
            BP_RenderDmaPack_AutoPacket(pCmd->mData);
            break;

         case kCmd_DmaPack_Direct:
            BP_RenderDmaPack_Direct(pCmd->mData);
            break;

         case kCmd_Prim_StartPass:
            BP_Prim_StartPass(pCmd->mData);
            break;

         case kCmd_Prim_InitPacket:
            BP_Prim_InitPacket(pCmd->mData);
            break;

         case kCmd_Prim_LocalParam:
            BP_Prim_LocalParam(pCmd->mData);
            break;

         case kCmd_Prim_RenderPacket:
            BP_Prim_RenderPacket(pCmd->mData);
            break;

         case kCmd_Prim_BeginOffscreen:
            BP_Prim_BeginOffscreen();
            break;

         case kCmd_Prim_EndOffscreen:
            BP_Prim_EndOffscreen();
            break;

         case kCmd_FrameFirstPacket:
            BP_FrameFirstPacket();
            break;

         case kCmd_FrameNewViewportInfoPacket:
            BP_FrameNewViewportInfoPacket(pCmd->mData);
            break;

         case kCmd_FrameInitPacket:
            BP_FrameInitPacket(pCmd->mData);
            break;

         case kCmd_FrameMSAAEndPacket:
            BP_FrameMSAAEndPacket();
            break;

         case kCmd_FrameStore_Begin:
            BP_FrameStoreBegin(pCmd->mData);
            break;

         case kCmd_FrameStore_End:
            BP_FrameStoreEnd(pCmd->mData);
            break;

         case kCmd_CopyRenderTargetToQuarterTemp:
   #ifndef DISABLE_POST_FX
            BP_CopyRenderTarget(kRT_TempBufferHalf1);
   #endif
            break;

         case kCmd_RestoreFogColor:
            BP_RestoreFogColor();
            break;

         case kCmd_SetRasterMask:
            BP_SetRasterMask((int)pCmd->mData);
            break;

         case kCmd_UpdateDynamicTexture:
            BP_UpdateDynamicTexture_Command(pCmd->mData);
            break;

         case kCmd_PostFx_Mono:
            BP_PostFx_Mono(pCmd->mData);
            break;

         case kCmd_PostFx_Blinds:
            BP_PostFx_Blinds(pCmd->mData);
            break;

   #if MGS_VERSION == 2
         case kCmd_Obj_InitPacket:
            BP_Obj_InitPacket(pCmd->mData);
            break;

         case kCmd_Obj_SortChainInitPacket:
            BP_Obj_SortChainInitPacket();
            break;

         case kCmd_Obj_SortChainEndPacket:
            BP_Obj_SortChainEndPacket();
            break;

         case kCmd_Obj_LocalParam:
            BP_Obj_LocalParam(pCmd->mData);
            break;

#if BP_VITA
         case kCmd_Obj_PreRenderVita:
            BP_Obj_PreRenderVita( pCmd->mData );
            break;
#endif

         case kCmd_Obj_PreRender:
            BP_Obj_PreRender( pCmd->mData );
            break;

         case kCmd_Obj_Render:
            BP_Obj_Render(pCmd->mData);
            break;

         case kCmd_Obj_RenderMulti:
            BP_Obj_RenderMulti(pCmd->mData);
            break;

         case kCmd_Obj_OptCmf_InitPacket:
            BP_Obj_OptCmf_InitPacket(pCmd->mData);
            break;

         case kCmd_Obj_OptCmf_PassParam:
            BP_Obj_OptCmf_PassParam(pCmd->mData);
            break;

         case kCmd_Obj_OptCmf_LocalParam:
            BP_Obj_OptCmf_LocalParam(pCmd->mData);
            break;

         case kCmd_Obj_OptCmf_Render:
            BP_Obj_OptCmf_Render(pCmd->mData);
            break;
         //Shadow Stage
         case kCmd_Projector_ShadowStage_StartPacket:
            BP_Projector_ShadowStage_StartPacket();
            break;

         case kCmd_Projector_ShadowStage_InitPacket:
            BP_Projector_ShadowStage_InitPacket(pCmd->mData);
            break;

         case kCmd_Projector_ShadowStage_PostZScreenPacket:
            BP_Projector_ShadowStage_PostZScreenPacket();
            break;

         case kCmd_Projector_ShadowStage_ShadowProjStart:
            BP_Projector_ShadowStage_ShadowProjStart(pCmd->mData);
            break;

         case kCmd_Projector_ShadowStage_EndPacket:
            BP_Projector_ShadowStage_EndPacket();
            break;
         //Spot Stage
         case kCmd_Projector_SpotStage_StartPacket:
            BP_Projector_SpotStage_StartPacket();
            break;

         case kCmd_Projector_SpotStage_InitPacket:
            BP_Projector_SpotStage_InitPacket(pCmd->mData);
            break;

         case kCmd_Projector_SpotStage_PostZScreenPacket:
            BP_Projector_SpotStage_PostZScreenPacket(pCmd->mData);
            break;

         case kCmd_Projector_SpotStage_ShadowProjStart:
            BP_Projector_SpotStage_ShadowProjStart(pCmd->mData);
            break;

         case kCmd_Projector_SpotStage_EndPacket:
            BP_Projector_SpotStage_EndPacket();
            break;

         case kCmd_Obj_Projector_LocalParam:
            BP_Projector_LocalParam(pCmd->mData);
            break;

         case kCmd_Obj_Projector_Render:
            BP_Projector_Render(pCmd->mData);
            break;
        
         case kCmd_Evm_InitPacket:
            BP_Evm_InitPacket(pCmd->mData);
            break;

         case kCmd_Evm_LocalParam:
            BP_Evm_LocalParam(pCmd->mData);
            break;

         case kCmd_Evm_Render:
            BP_Evm_Render(pCmd->mData);
            break;
         
         case kCmd_Evm_RenderMulti:
            BP_Evm_RenderMulti(pCmd->mData);
            break;

         case kCmd_RenderRadar:
            BP_RenderRadar(pCmd->mData);
            break;

         case kCmd_Patch_InitFrame:
            BP_PatchInitFrame(pCmd->mData);
            break;

         case kCmd_Patch_Init:
            BP_PatchInit(pCmd->mData);
            break;

         case kCmd_Patch_InitRender:
            BP_PatchInitRender(pCmd->mData);
            break;

         case kCmd_Patch_Render:
            BP_PatchRender(pCmd->mData);
            break;

         case kCmd_Patch_FinalizeFrame:
            BP_PatchFinalizeFrame(pCmd->mData);
            break;

         case kCmd_Comdl_InitPacket:
            BP_Comdl_InitPacket(pCmd->mData);
            break;

         case kCmd_Comdl_Render:
            BP_Comdl_Render(pCmd->mData);
            break;

         case kCmd_PostFx_FarFocus:
            BP_PostFx_MGS2_FarFocus(pCmd->mData);
            break;

         case kCmd_PostFX_VRClear:
            BP_PostFX_MGS2_VRClear(pCmd->mData);
            break;

         case kCmd_PostFX_CodexInOut:
            BP_PostFX_MGS2_CodexInOut(pCmd->mData);
            break;

         case kCmd_PostFx_ReduceScreen:
            BP_PostFx_MGS2_ReduceScreen(pCmd->mData);
            break;

   #elif MGS_VERSION == 3
         case kCmd_SetTexture:
            BP_SetTexturePacket(pCmd->mData);
            break;

         case kCmd_MDB_EndModel:
            BP_FixModel_EndModel();
            break;

         case kCmd_MDB_FastInitPacket:
            BP_FixModel_FastInitPacket(pCmd->mData);
            break;

         case kCmd_MDB_FastLocalParam:
            BP_FixModel_FastLocalParam(pCmd->mData);
            break;

         case kCmd_MDB_FastWrapPacket:
            BP_FixModel_FastWrapPacket(pCmd->mData);
            break;

         case kCmd_MDB_InitPacket:
            BP_FixModel_InitPacket(pCmd->mData);
            break;

         case kCmd_MDB_LocalParam:
            BP_FixModel_LocalParam(pCmd->mData);
            break;

         case kCmd_MDB_WrapPacket:
            BP_FixModel_WrapPacket(pCmd->mData);
            break;

         case kCmd_MDB_Render:
            BP_FixModel_Render(pCmd->mData);
            break;

         case kCmd_MDB_RenderMultiTex:
            BP_FixModel_RenderMultiTex(pCmd->mData);
            break;

         case kCmd_MDC_InitPacket:
            BP_Model_InitPacket(pCmd->mData);
            break;
         
         case kCmd_MDC_EndPacket:
            BP_Model_EndPacket();
            break;

         case kCmd_MDC_LocalParam:
            BP_Model_LocalParam(pCmd->mData);
            break;

         case kCmd_MDC_Render:
            BP_Model_Render(pCmd->mData);
            break;

         case kCmd_MDC_RenderMultiTex:
            BP_Model_RenderMultiTex(pCmd->mData);
            break;

         case kCmd_OptCmf_InitPacket:
            BP_OptCmf_LocalParam(pCmd->mData);
            break;

         case kCmd_OptCmf_PassParam:
            BP_OptCmf_PassParam(pCmd->mData);
            break;

         case kCmd_Clone_InitPacket:
            BP_Clone_InitPacket(pCmd->mData);
            break;

         case kCmd_Clone_Render:
            BP_Clone_Render(pCmd->mData);
            break;

         case kCmd_PostFx_ScopeBlur:
            BP_PostFx_ScopeBlur(pCmd->mData);
            break;

         case kCmd_PostFx_GlareNew:
            BP_PostFx_MGS3_GlareNew(pCmd->mData);
            break;

         case kCmd_PostFx_GhostSmoke:
            BP_PostFX_MGS3_GhostSmoke(pCmd->mData);
            break;

         case kCmd_PostFx_FarFocus:
            BP_PostFx_MGS3_FarFocus(pCmd->mData);
            break;

         case kCmd_PostFx_Contrast:
            BP_PostFx_MGS3_Contrast(pCmd->mData);
            break;

         //Texture projection stage
         case kCmd_Projector_TextureStage_StartPacket:
            BP_Projector_TextureStage_StartPacket(pCmd->mData);
            break;

         //Shadow Stage
         case kCmd_Projector_ShadowStage_StartPacket:
            BP_Projector_ShadowStage_StartPacket();
            break;

         case kCmd_Projector_ShadowStage_InitPacket:
            BP_Projector_ShadowStage_InitPacket();
            break;

         case kCmd_Projector_ShadowStage_PostZScreenPacket:
            BP_Projector_ShadowStage_PostZScreenPacket();
            break;

         case kCmd_Projector_ShadowStage_ShadowProjStart:
            BP_Projector_ShadowStage_ShadowProjStart();
            break;

         case kCmd_Projector_ShadowStage_EndPacket:
            BP_Projector_ShadowStage_EndPacket();
            break;
         //Spot Stage
         case kCmd_Projector_SpotStage_StartPacket:
            BP_Projector_SpotStage_StartPacket();
            break;

         case kCmd_Projector_SpotStage_InitPacket:
            BP_Projector_SpotStage_InitPacket();
            break;

         case kCmd_Projector_SpotStage_PostZScreenPacket:
            BP_Projector_SpotStage_PostZScreenPacket(pCmd->mData);
            break;

         case kCmd_Projector_SpotStage_ShadowProjStart:
            BP_Projector_SpotStage_ShadowProjStart();
            break;

         case kCmd_Projector_SpotStage_EndPacket:
            BP_Projector_SpotStage_EndPacket();
            break;

         case kCmd_PostFx_MGS3_LensflareNew:
            BP_PostFx_MGS3_LensflareNew(pCmd->mData);
            break;

         case kCmd_PostFx_MGS3_ShockwaveFlutter:
#if BP_VITA
            BP_PostFx_ShockwaveFlutter(pCmd->mData);
#endif
            break;
   #endif

         case kCmd_FrameBuffer:
            BP_FrameBuffer(pCmd->mData);
            break;

#if BP_VITA
         case kCmd_MSAA_Particles:
            BP_FX_Halfres_MSAA(pCmd->mData);
            break;
#endif

         case kCmd_PostFx_Offscreen:
#if BP_VITA
            BP_PostFx_Offscreen(pCmd->mData);
#endif
            break;

#if BP_VITA
#if MGS_VERSION == 2
         case kCmd_ProjectorNew_StartPacket:
            BP_ProjectorNew_StartPacket(pCmd->mData);
            break;

         case kCmd_ProjectorNew_InitPacket:
            BP_ProjectorNew_InitPacket(pCmd->mData);
            break;

         case kCmd_ProjectorNew_ShadowProjStart:
            BP_ProjectorNew_ShadowProjStart(pCmd->mData);
            break;

         case kCmd_ProjectorNew_SpotProjStart:
            BP_ProjectorNew_SpotProjStart(pCmd->mData);
            break;

         case kCmd_ProjectorNew_EndPacket:
            BP_ProjectorNew_EndPacket();
            break;

         case kCmd_ProjectorNew_Dummy:
            break;
#endif
#endif

         default:
            BPE_VERIFYA(false, "Unsupported render command");
            break;
         }

         pLastCmd1_DEBUG = pLastCmd0_DEBUG;
         pLastCmd0_DEBUG = pCmd;
      }
   }
   else
   {
      // This happens the very first frame in MGS2/3
      gFrameInfoStruct.hasExecutedClear = true;
#if BP_VITA
      gpRenderBackend->SetRenderTarget(SRenderTarget(BP_GetRenderTarget(kRT_CurrentFrameBuffer_Full), NULL, NULL, NULL,
                                                     BP_GetRenderTarget(kRT_CurrentDepthBuffer_Full)));
#endif
      gpRenderBackend->Clear(CBaseRenderBackend::kFlag_Color, CColor::Black(), 0.0f);
   }
}

//----------------------------------------------------------------------------

void BP_RenderNoDrawMode()
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET
   
   CBaseTexture* pLastBuffer;
   CBaseTexture* pCurrBuffer;
   if( mLastRenderedBuffer )
   {
      pLastBuffer = BP_GetRenderTarget(kRT_MainBuffer2_NoMSAA);
      pCurrBuffer = BP_GetRenderTarget(kRT_MainBuffer1_NoMSAA);
   }
   else
   {
      pLastBuffer = BP_GetRenderTarget(kRT_MainBuffer1_NoMSAA);
      pCurrBuffer = BP_GetRenderTarget(kRT_MainBuffer2_NoMSAA);
   }

   BP_CopyTexture(pLastBuffer, pCurrBuffer);
#else
#if BP_VITA
   BP_CopyTexture(gpRenderBackend->GetPreviousDisplayTexture(), gpRenderBackend->GetCurrentDisplayTexture());
#endif
#endif
}

//----------------------------------------------------------------------------

void BP_RenderMainThread(int currentRenderBuffer)
{
   if( currentRenderBuffer == -1 )
   {
      //BP_DebugText_Print("SPECIAL RENDER MODE");
      BP_RenderPrologue(-1, 0/*gShouldDrawMode*/);
      BP_RenderNoDrawMode();
      BP_RenderEpilogue(0/*gShouldDrawMode*/);
   }
   else
   {
      BP_RB_LockBufferForRead( currentRenderBuffer );

      BP_RenderPrologue(currentRenderBuffer, gBP_ShouldDrawMode);

      if( gBP_ShouldDrawMode )
      {
         gDemoDie_FirstFrame = 1;
         // Set gAS_DidDemoDie_Render to 0 when we're drawing frames again
         gAS_DidDemoDie_Render = 0;

         BP_RenderCommands(currentRenderBuffer);
         mLastRenderedBuffer = currentRenderBuffer;
      }
      else
      {
         BP_RenderNoDrawMode();
      }

      BP_RenderEpilogue(gBP_ShouldDrawMode);

      BP_RB_UnlockBufferForRead( currentRenderBuffer );

      // Add dummy marker that gets skipped during rendering, this acts to bound the game update time so it doesn't include everything we do above this point (including vsync stall!)
      PROFILE_ADD_PPU(0, "");
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_ProcessDebugInput()
{
#if BP_ENABLE_DEBUG_MENU
#if !defined(MARCO) && BPE_TARGET == BPE_TARGET_WIN32
   BP_DebugMenu_ProcessInput(0);
#else
   BP_DebugMenu_ProcessInput(1);
#endif
#endif
}

//----------------------------------------------------------------------------

#if BP_ENABLE_MEMORY_TRACKING
void DumpMemoryStats();
#endif

extern "C" void BP_RenderMain(int currentRenderBuffer, int shouldDrawBuffer)
{
#if BP_ENABLE_MEMORY_TRACKING
   if ( GV_PadDataDirect[1].press & PAD_AR )
   {
      DumpMemoryStats();
   }
#endif
   
   BP_Render_HelpDrainUltWork();

   // Have to grab these after the update is done, otherwise these buffers get Reset!
   {
      gIndexBufferPoolUT_InUseSize = gpRenderBackend->GetIndexBufferPool_UT()->GetInUseSize();

      gVertexBufferPoolUT_InUseSize = gpRenderBackend->GetVertexBufferPool_UT()->GetInUseSize();
   }

   // Hack to force checkerboard raster mask emulation for any problem levels
#if MGS_VERSION == 3
   if( strcmp( RenderBackend()->GetCurrentAreaDebugName(), "s081a_0" ) == 0 )
   {
      gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage = 1;
   }
   else
   {
      gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage = 0;
   }

   if( strcmp( RenderBackend()->GetCurrentAreaDebugName(), "s171b") == 0 || strcmp( RenderBackend()->GetCurrentAreaDebugName(), "s171b_0") == 0 )
   {
      gBP_EnableAlphaTestGEqualOptimization = 1;
   }
   else
   {
      gBP_EnableAlphaTestGEqualOptimization = 0;
   }

   if( strcmp( RenderBackend()->GetCurrentAreaDebugName(), "s223a_0" ) == 0 )
   {
      gAllowDecal_BaseOnStage = 1;
   }
   else
   {
      gAllowDecal_BaseOnStage = 0;
   }

#elif MGS_VERSION == 2
   if( strcmp( GM_GetArea(), "d021p01" ) == 0 )
   {
      gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage = 1;
   }
   else
   {
      gForceReasterMaskEmulation_kRME_Checkered_Stipple_BaseOnStage = 0;
   }
#endif

   sCurrentEndframeTime = sEndframeWatch.GetElapsedTime();
#if BP_VITA && MGS_VERSION == 2
   // sLastRenderWaitTime, at this point, is an absolute time
   // later in this function, it will turn into a diff
   sLastRenderWaitTime = sceKernelGetProcessTimeWide();
#endif

   bool renderNewFrame = true;

#if ENABLE_SEPERATE_RENDER_THREAD
   BP_Debug_PushCPUMarker( "BP_RenderMain - Threadwait 1" );
   if( shouldDrawBuffer )
   {
      // In "draw" mode stall for the previous frame to finish so we can start a new frame.
      gpRenderBackend->WaitThreadFunction();
      renderNewFrame = true;
   }
   else
   {
      if( gBP_ShouldDrawMode )
      {
         gpRenderBackend->WaitThreadFunction();
         renderNewFrame = true;
      }
      else
      {
         // In the "undraw" mode: Check if previous frame is finished, in that case start a new frame.
		 
		 // VITA_TODO: Restore original undraw behavior.
		 // the following line was commented out because if we let the render thread go all willy-nilly, we'd eventually
		 // try to overwrite its buffers
//         renderNewFrame = gpRenderBackend->TryWaitThreadFunction();
         gpRenderBackend->WaitThreadFunction();
         renderNewFrame = true;
      }
   }

   BP_Renderer_MGS2_3060_SetRenderTimeUS( gpRenderBackend->GetThreadLastCPUTimeUs(), gpRenderBackend->VitaReadGPUCanDo60FPS() );
   BP_Debug_PopCPUMarker();

#endif

   gBP_ShouldDrawMode = shouldDrawBuffer;
   
   // AS(JM) - copy the endframe's version of diddemodie to the
   // render thread if the endframe's is set. When complete, 
   // unset the endframe's version.
   // This is safe because at this point, we're running from endframe
   // and we just waited for the render thread to catch up.
   if ( gAS_DidDemoDie_Endframe )
   {
      gAS_DidDemoDie_Render = 1;
      gAS_DidDemoDie_Endframe = 0;
   }

#if MGS_VERSION==2
   DG_Renderframe_FrameCount = DG_Endframe_FrameCount;
#endif

   if( renderNewFrame )
   {
      // At this point the main thread is guaranteed to "right here" and the render thread is guranteed to be idle.
      BP_ShutdownCheck();

#if BP_ENABLE_ONSCREEN_PROFILER
      // Initialize profile bars
      if( gShowProfileInfo && !gIsProfileBarsInitialized )
      {
         gIsProfileBarsInitialized = 1;
#if defined(BP_PS3) // TODO: investigate make this platform generic in some way
         ProfileBars::Init(gpSpursManager->mpSPURS_Main.get(), 5, gpSpursManager->mpSPURS_System.get(), 1);
#elif defined(BP_360)
         ProfileBars::Init(gpRenderBackend->GetDirect3DDevice().GetPtr());
#endif
      }

      // Start profile bars if they are enabled.
      if( BP_IsProfileInfoActive() )
      {
         // Render on screen text for all markers
         if( shouldDrawBuffer )         
            BP_DebugRender_ProfileBarsText();

         ProfileBars::Enable(true);

         ProfileBars::StartFrame(gShowProfileInfo >= 2 ? true : false);
         ProfileBars::SetTargetFps( BP_FRAMES_PER_SEC() / gpRenderBackend->GetPresentationInterval() );
      }
      else
         ProfileBars::Enable(false);
#endif

      // Process debug controller input AFTER the render thread went idle
#if MGS_VERSION==3
      BP_Debug_PushCPUMarker( "BP_ProcessDebugInput" );
      BP_ProcessDebugInput();
      BP_Debug_PopCPUMarker();
#endif

      // Update movies
      {
         BP_Movie_RenderActiveMovies(shouldDrawBuffer);
         PROFILE_ADD_PPU(0xFFFFFFFF, "Movie");
         PROFILE_ADD_GPU(0xFFFFFFFF, "Movie");
      }

#if ENABLE_SEPERATE_RENDER_THREAD
      BP_Debug_PushCPUMarker( "BP_RenderMain - Threadwait 2" );
      gpRenderBackend->RunThreadFunction(currentRenderBuffer);

      // Only do sync wait on thread to finish if in "draw" mode.
      if( 
         (
         gSyncWaitRender 
#if BP_ENABLE_TEXTURE_TOOL
         ||
         BP_TextureTool_GetEnable()
#endif
         )
         && shouldDrawBuffer )
         gpRenderBackend->WaitThreadFunction();
      BP_Debug_PopCPUMarker();
#else
      bool shouldRender = true;

      // If we shouldn't render, just render less often to reduce the vsync stalls while still allowing the frame delayed memory free's to occur.
      if( !shouldDrawBuffer )
      {
         shouldRender = false;

         // Handle lack of texture memory flush by calling 'present' at around 30fps
         static CStopWatch sw;
         if (sw.GetElapsedMilliseconds() > 33)
         {
            shouldRender = true;
            sw.Reset();
         }
      }

      if( shouldRender ) 
         BP_RenderMainThread(currentRenderBuffer);
#endif
   }

#if MGS_VERSION==2 && BP_VITA
   // AS(JM) - Here's where sLastRenderWaitTime becomes a diff
   sLastRenderWaitTime = sceKernelGetProcessTimeWide() - sLastRenderWaitTime;
#endif


   // After this function exits the "game" time begins.
   sEndframeWatch.Reset();
}

//----------------------------------------------------------------------------

// This only waits for the render thread to finish execution
extern "C" void BP_WaitForLastRenderThreadToComplete()
{
#if ENABLE_SEPERATE_RENDER_THREAD
   gpRenderBackend->WaitThreadFunction();
#endif
}

//----------------------------------------------------------------------------

// This waits for both the render thread to finish execution
// and for the GPU to finish execution
extern "C" void BP_WaitForLastRenderToComplete()
{
   BP_WaitForLastRenderThreadToComplete();
#if BPE_TARGET == BPE_TARGET_PS3
   gpRenderBackend->GcmFinish();
#endif
}

//----------------------------------------------------------------------------

extern "C" int BP_LoadScreenShotJPEG_Internal(void* pJPEG, int JPEGSize, ERenderTarget renderTarget)
{
#if BPE_TARGET == BPE_TARGET_X360
   // JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(renderTarget);
      CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
      IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture();
      IDirect3DSurface9* pD3DSurfaceDestNew;
      pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
      D3DXLoadSurfaceFromFileInMemory(pD3DSurfaceDestNew, NULL, NULL, pJPEG, JPEGSize, NULL, D3DX_DEFAULT, 0, NULL);
      //
      pD3DSurfaceDestNew->Release();
   }
#elif BPE_TARGET == BPE_TARGET_WIN32
   // JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(renderTarget);
      CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
      IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture().GetPtr();
      IDirect3DSurface9* pD3DSurfaceDestNew;
      pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
      D3DXLoadSurfaceFromFileInMemory(pD3DSurfaceDestNew, NULL, NULL, pJPEG, JPEGSize, NULL, D3DX_DEFAULT, 0, NULL);
      //
      pD3DSurfaceDestNew->Release();
   }
#elif BPE_TARGET == BPE_TARGET_PS3
   // JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(renderTarget);
      pCTextureDestNew->LoadFromJPEG(pJPEG, JPEGSize);
   }
#elif BPE_TARGET == BPE_TARGET_VITA
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(renderTarget);
      pCTextureDestNew->LoadFromJPEG(pJPEG, JPEGSize);
   }
#else
#error "Unknown platform!"
#endif

   return 0;
}

extern "C" int BP_LoadScreenShot_DigitalCamera_FromJPEG(void* pJPEG, int JPEGSize)
{
   return BP_LoadScreenShotJPEG_Internal(pJPEG, JPEGSize, kRT_CameraScreenShot);
}

#if MGS_VERSION == 2

extern "C" int BP_LoadScreenShot_DigitalCamera_FromJPEG_MGS2(void* pJPEG_)
{
   char* pJPEG = (char*)pJPEG_;
   int JPEGSize = *(int*)pJPEG;
   pJPEG += sizeof(int);
   return BP_LoadScreenShot_DigitalCamera_FromJPEG(pJPEG, JPEGSize);
}

extern "C" void BP_CaptureScreenShot_Tanker_FromJPEG_MemBuffer(int photoNum, void* pMemory_)
{
   char* pMemory = (char*)pMemory_;
   char* const pUniqueID = "BP_MEMJPEG";
   // Ensure this is our new JPEG format
   int uniqueIDLength = strlen(pUniqueID);
   if( pMemory[uniqueIDLength] != NULL || strcmp(pMemory, pUniqueID) != 0 )
   {
      // the data is corrupt or is still from the old KP assets
      printf("BP_CaptureScreenShotTanker_FromJPEG_MemBuffer reading corrupt MEMJPEG!\n");
      return;
   }
   pMemory += uniqueIDLength+1; // advance past pUniqueID and NULL character
   // Extract photoSize from the next 4 bytes
   int photoSize = *(int*)pMemory;
   BP_LE_SwapSInt_Inp(&photoSize);
   pMemory += sizeof(int);
   // Load up JPEG
   BP_LoadScreenShotJPEG_Internal(pMemory, photoSize, ERenderTarget(kRT_TankerSS0+photoNum));
}

extern "C" void BP_CaptureScreenShotTanker(int photoNum)
{

   // this function does not save out file to png/jpeg buffers
   CBaseTexture* pCTextureRenderTarget = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);

#if BPE_TARGET == BPE_TARGET_X360
   CTexture* pTexturePlatformRenderTarget = (CTexture*)pCTextureRenderTarget;
   IDirect3DTexture9* pD3DTextureRenderTarget = (IDirect3DTexture9*)pTexturePlatformRenderTarget->GetTexture();
   IDirect3DSurface9* pD3DSurfaceRenderTarget;
   pD3DTextureRenderTarget->GetSurfaceLevel(0, &pD3DSurfaceRenderTarget);

   CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_TankerSS0+photoNum));
   CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
   IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture();
   IDirect3DSurface9* pD3DSurfaceDestNew;
   pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
   D3DXLoadSurfaceFromSurface(pD3DSurfaceDestNew, NULL, NULL, pD3DSurfaceRenderTarget, NULL, NULL, D3DX_DEFAULT, 0);
   //
   pD3DSurfaceRenderTarget->Release();
   pD3DSurfaceDestNew->Release();
#elif BPE_TARGET == BPE_TARGET_WIN32
   CTexture* pTexturePlatformRenderTarget = (CTexture*)pCTextureRenderTarget;
   IDirect3DTexture9* pD3DTextureRenderTarget = (IDirect3DTexture9*)pTexturePlatformRenderTarget->GetTexture().GetPtr();
   IDirect3DSurface9* pD3DSurfaceRenderTarget;
   pD3DTextureRenderTarget->GetSurfaceLevel(0, &pD3DSurfaceRenderTarget);

   CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_TankerSS0+photoNum));
   CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
   IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture().GetPtr();
   IDirect3DSurface9* pD3DSurfaceDestNew;
   pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
   D3DXLoadSurfaceFromSurface(pD3DSurfaceDestNew, NULL, NULL, pD3DSurfaceRenderTarget, NULL, NULL, D3DX_DEFAULT, 0);
   //
   pD3DSurfaceRenderTarget->Release();
   pD3DSurfaceDestNew->Release();
#elif BPE_TARGET == BPE_TARGET_PS3
   // PS3
   CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_TankerSS0+photoNum));
   gpRenderBackend->SetRenderTarget(SRenderTarget(pCTextureDestNew, NULL, NULL, NULL, NULL));
   BP_DrawFullscreenTexture(pCTextureRenderTarget, 1, 1);
   // wait for GPU finish
   gpRenderBackend->GcmFinish();
#elif BPE_TARGET == BPE_TARGET_VITA
   CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_TankerSS0+photoNum));
   gpRenderBackend->SetRenderTarget(SRenderTarget(pCTextureDestNew, NULL, NULL, NULL, NULL));
   BP_DrawFullscreenTexture(pCTextureRenderTarget, 1, 1);
   gpRenderBackend->EndCapture();
#else
#  error Unknown platform
#endif
}
#endif
//----------------------------------------------------------------------------

extern "C" void BP_AquireRenderThread()
{
   gpRenderBackend->AcquireRenderThreadOwnership();
}

//----------------------------------------------------------------------------

extern "C" void BP_ReleaseRenderThread()
{
   gpRenderBackend->ReleaseRenderThreadOwnership();
}

//----------------------------------------------------------------------------

extern "C" void BP_CaptureScreenShot(void** pJPEG, int* pJPEGSize, void** pPNG, int* pPNGSize)
{
   CBaseTexture* pCTextureRenderTarget = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);

#if BPE_TARGET == BPE_TARGET_VITA
   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraScreenShot));
      gpRenderBackend->SetRenderTarget(SRenderTarget(pCTextureDestNew, NULL, NULL, NULL, NULL));
      BP_DrawFullscreenTexture(pCTextureRenderTarget, 1, 0);
   }
   // Render PNG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraThumbnailScreenShot));
      gpRenderBackend->SetRenderTarget(SRenderTarget(pCTextureDestNew, NULL, NULL, NULL, NULL));
      BP_DrawFullscreenTexture(pCTextureRenderTarget, 1, 0);
   }
   gpRenderBackend->EndCapture();
   // Save JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraScreenShot));

      void* pJPEG2;
      int JPEGSize2;
      pCTextureDestNew->SaveToJPEG(&pJPEG2, &JPEGSize2);
      *pJPEGSize = JPEGSize2;
      *pJPEG = malloc(JPEGSize2);
      memcpy(*pJPEG, pJPEG2, JPEGSize2);
   }
   // Save PNG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraThumbnailScreenShot));

      void* pPNG2;
      int PNGSize2;
      pCTextureDestNew->SaveToPNG(&pPNG2, &PNGSize2);
      *pPNGSize = PNGSize2;
      *pPNG = malloc(PNGSize2);
      memcpy(*pPNG, pPNG2, PNGSize2);
   }
#elif BPE_TARGET == BPE_TARGET_X360
   CTexture* pTexturePlatformRenderTarget = (CTexture*)pCTextureRenderTarget;
   IDirect3DTexture9* pD3DTextureRenderTarget = (IDirect3DTexture9*)pTexturePlatformRenderTarget->GetTexture();
   IDirect3DSurface9* pD3DSurfaceRenderTarget;
   pD3DTextureRenderTarget->GetSurfaceLevel(0, &pD3DSurfaceRenderTarget);

   // JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraScreenShot));
      CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
      IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture();
      IDirect3DSurface9* pD3DSurfaceDestNew;
      pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
      D3DXLoadSurfaceFromSurface(pD3DSurfaceDestNew, NULL, NULL, pD3DSurfaceRenderTarget, NULL, NULL, D3DX_DEFAULT, 0);

      LPD3DXBUFFER d3dBuffer = NULL;
      D3DXSaveSurfaceToFileInMemory(&d3dBuffer, D3DXIFF_JPG, pD3DSurfaceDestNew, NULL, NULL);
      *pJPEGSize = d3dBuffer->GetBufferSize();
      *pJPEG = malloc(d3dBuffer->GetBufferSize());
      memcpy(*pJPEG, d3dBuffer->GetBufferPointer(), d3dBuffer->GetBufferSize());
      //
      pD3DSurfaceDestNew->Release();
      d3dBuffer->Release();
   }
   // PNG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraThumbnailScreenShot));
      CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
      IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture();
      IDirect3DSurface9* pD3DSurfaceDestNew;
      pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
      D3DXLoadSurfaceFromSurface(pD3DSurfaceDestNew, NULL, NULL, pD3DSurfaceRenderTarget, NULL, NULL, D3DX_DEFAULT, 0);

      LPD3DXBUFFER d3dBuffer = NULL;
      D3DXSaveSurfaceToFileInMemory(&d3dBuffer, D3DXIFF_PNG, pD3DSurfaceDestNew, NULL, NULL);
      *pPNGSize = d3dBuffer->GetBufferSize();
      *pPNG = malloc(d3dBuffer->GetBufferSize());
      memcpy(*pPNG, d3dBuffer->GetBufferPointer(), d3dBuffer->GetBufferSize());
      //
      pD3DSurfaceDestNew->Release();
      d3dBuffer->Release();
   }
   //
   pD3DSurfaceRenderTarget->Release();
#elif BPE_TARGET == BPE_TARGET_WIN32
   CTexture* pTexturePlatformRenderTarget = (CTexture*)pCTextureRenderTarget;
   IDirect3DTexture9* pD3DTextureRenderTarget = (IDirect3DTexture9*)pTexturePlatformRenderTarget->GetTexture().GetPtr();
   IDirect3DSurface9* pD3DSurfaceRenderTarget;
   pD3DTextureRenderTarget->GetSurfaceLevel(0, &pD3DSurfaceRenderTarget);
   // JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraScreenShot));
      CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
      IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture().GetPtr();
      IDirect3DSurface9* pD3DSurfaceDestNew;
      pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
      D3DXLoadSurfaceFromSurface(pD3DSurfaceDestNew, NULL, NULL, pD3DSurfaceRenderTarget, NULL, NULL, D3DX_DEFAULT, 0);

      LPD3DXBUFFER d3dBuffer = NULL;
      D3DXSaveSurfaceToFileInMemory(&d3dBuffer, D3DXIFF_JPG, pD3DSurfaceDestNew, NULL, NULL);
      *pJPEGSize = d3dBuffer->GetBufferSize();
      *pJPEG = malloc(d3dBuffer->GetBufferSize());
      memcpy(*pJPEG, d3dBuffer->GetBufferPointer(), d3dBuffer->GetBufferSize());
      //
      pD3DSurfaceDestNew->Release();
      d3dBuffer->Release();
   }
   // PNG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraThumbnailScreenShot));
      CTexture* pTexturePlatformDestNew = (CTexture*)pCTextureDestNew;
      IDirect3DTexture9* pD3DTextureDestNew = (IDirect3DTexture9*)pTexturePlatformDestNew->GetTexture().GetPtr();
      IDirect3DSurface9* pD3DSurfaceDestNew;
      pD3DTextureDestNew->GetSurfaceLevel(0, &pD3DSurfaceDestNew);
      D3DXLoadSurfaceFromSurface(pD3DSurfaceDestNew, NULL, NULL, pD3DSurfaceRenderTarget, NULL, NULL, D3DX_DEFAULT, 0);

      LPD3DXBUFFER d3dBuffer = NULL;
      D3DXSaveSurfaceToFileInMemory(&d3dBuffer, D3DXIFF_JPG, pD3DSurfaceDestNew, NULL, NULL);
      *pPNGSize = d3dBuffer->GetBufferSize();
      *pPNG = malloc(d3dBuffer->GetBufferSize());
      memcpy(*pPNG, d3dBuffer->GetBufferPointer(), d3dBuffer->GetBufferSize());
      //
      pD3DSurfaceDestNew->Release();
      d3dBuffer->Release();
   }
   //
   pD3DSurfaceRenderTarget->Release();
#elif BPE_TARGET==BPE_TARGET_PS3
   // PS3
   // Render JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraScreenShot));
      gpRenderBackend->SetRenderTarget(SRenderTarget(pCTextureDestNew, NULL, NULL, NULL, NULL));
      BP_DrawFullscreenTexture(pCTextureRenderTarget, 1, 1);
   }
   // Render PNG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraThumbnailScreenShot));
      gpRenderBackend->SetRenderTarget(SRenderTarget(pCTextureDestNew, NULL, NULL, NULL, NULL));
      BP_DrawFullscreenTexture(pCTextureRenderTarget, 1, 1);
   }
   // wait for GPU finish
   gpRenderBackend->GcmFinish();
   // Save JPEG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraScreenShot));

      void* pJPEG2;
      int JPEGSize2;
      pCTextureDestNew->SaveToJPEG(&pJPEG2, &JPEGSize2);
      *pJPEGSize = JPEGSize2;
      *pJPEG = malloc(JPEGSize2);
      memcpy(*pJPEG, pJPEG2, JPEGSize2);
   }
   // Save PNG
   {
      CBaseTexture* pCTextureDestNew = BP_GetRenderTarget(ERenderTarget(kRT_CameraThumbnailScreenShot));

      void* pPNG2;
      int PNGSize2;
      pCTextureDestNew->SaveToPNG(&pPNG2, &PNGSize2);
      *pPNGSize = PNGSize2;
      *pPNG = malloc(PNGSize2);
      memcpy(*pPNG, pPNG2, PNGSize2);
   }
#else
#  error Unknown platform
#endif
}

//----------------------------------------------------------------------------

extern "C" void BP_RenderClearCommands(int currentBuffer)
{
   // Last thing before we return to the game
   BP_RB_SetCurrentBufferIndex(currentBuffer);
   BP_RB_Clear();
}

//----------------------------------------------------------------------------

unsigned int BP_GetFrameCount()
{
   return gpRenderBackend->GetFrameCount();
}

//----------------------------------------------------------------------------

unsigned int BP_AllocDynamicTexture(int const width, int const height, int const doClear)
{
   CBaseTexture* pTexture = CBaseTexture::Create(width, height, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, kRM_System);
   
   BPE_ASSERT( pTexture->GetWidth() == width && pTexture->GetHeight() == height, "Width and height don't match inputs!" );

   if( doClear )
   {
      void* pMemory;
      int pitch;
      pTexture->Lock(&pMemory, &pitch);
      memset(pMemory, 0, pitch * pTexture->GetHeight());
      pTexture->Unlock();
   }

   return (unsigned int)pTexture;
}

//----------------------------------------------------------------------------

void BP_AllocDynamicTexture_Buffered(int const width, int const height, int const doClear, SBP_BufferedTexture* pBT)
{
   pBT->lastLockedFrame = -1;
   pBT->prevIndex = 0;
   pBT->currIndex = 1;

   for( int i=0; i < BP_BUFFEREDTEXTURE_MAX_TEXTURE; ++i )
   {
      pBT->aTextureHandle[i] = BP_AllocDynamicTexture(width, height, doClear);
   }
}

//----------------------------------------------------------------------------

SBP_BufferedTexture* BP_AllocDynamicTexture_Buffered_Pointer(int const width, int const height, int const doClear)
{
   SBP_BufferedTexture* pResult = new SBP_BufferedTexture;
   BP_AllocDynamicTexture_Buffered(width, height, doClear, pResult);
   return pResult;
}

//----------------------------------------------------------------------------

SBP_BufferedTexture* BP_AllocDynamicTexture_SingleBuffered_Pointer(int const width, int const height, int const doClear)
{
   SBP_BufferedTexture* pResult = new SBP_BufferedTexture;

   pResult->lastLockedFrame = -1;
   pResult->prevIndex = -1;
   pResult->currIndex = 0;

   pResult->aTextureHandle[0] = BP_AllocDynamicTexture(width, height, doClear);
   for( int i=1; i < BP_BUFFEREDTEXTURE_MAX_TEXTURE; ++i )
   {
      pResult->aTextureHandle[i] = NULL;
   }
   return pResult;
}

//----------------------------------------------------------------------------

void BP_LockDynamicTexture_NonLinear(unsigned int texture, void** pData, int* pitch)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;
   BPE_VERIFY(pTexture != NULL, false, "BP_LockDynamicTexture: Must pass in valid texture");
#if BPE_TARGET != BPE_TARGET_X360
   pTexture->Lock(pData, pitch);
#else
   BPE_VERIFY(pTexture->GetFormat() == CBaseTexture::kFormat_A8R8G8B8, false, "BP_LockDynamicTexture: Must pass in CBaseTexture::kFormat_A8R8G8B8 texture");
   char* pTexData;
   pTexture->Lock((void**)&pTexData, pitch);
   int width = pTexture->GetWidth();
   int height = pTexture->GetHeight();
   *pitch = width*4;
   *pData = malloc(width*height*4);
   XGUntileTextureLevel(width, height, 0, XGGetGpuFormat(D3DFMT_A8R8G8B8), XGTILE_NONPACKED, *(char**)pData, width*4, NULL, pTexData, NULL);
   pTexture->Unlock();
#endif
}

//----------------------------------------------------------------------------

void BP_UnlockDynamicTexture_NonLinear(unsigned int texture, void** pData)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;
   BPE_VERIFY(pTexture != NULL, false, "BP_LockDynamicTexture: Must pass in valid texture");
#if BPE_TARGET != BPE_TARGET_X360
   pTexture->Unlock();
#else
   BPE_VERIFY(pTexture->GetFormat() == CBaseTexture::kFormat_A8R8G8B8, false, "BP_LockDynamicTexture: Must pass in CBaseTexture::kFormat_A8R8G8B8 texture");
   void* pTexData;
   int pitch;
   pTexture->Lock(&pTexData, &pitch);
   int width = pTexture->GetWidth();
   int height = pTexture->GetHeight();
   XGTileTextureLevel(width, height, 0, XGGetGpuFormat(D3DFMT_A8R8G8B8), XGTILE_NONPACKED, pTexData, NULL, *(char**)pData, width*4, NULL);
   pTexture->Unlock();
   free(*pData);
#endif
}

//----------------------------------------------------------------------------

void BP_UpdateDynamicTextureCLUT16(unsigned int texture, void* vram, void* paletteVram)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;

   uint32* pTextureMem = NULL;
   int texturePitch = 0;

   pTexture->Lock((void**)&pTextureMem, &texturePitch);
   texturePitch >>= 2;
   
   int const height = pTexture->GetHeight();
   int const width = pTexture->GetWidth();
   
   char* sourceData = (char*)vram;
   uint32* clut = (uint32*)paletteVram;

   for( int y = 0; y < height; ++y )
   {
      for( int x = 0; x < width; x += 2 )
      {
         unsigned char s = *sourceData++;

         pTextureMem[x + 0] = clut[s & 0xF];
         pTextureMem[x + 1] = clut[s >> 4];
      }

      pTextureMem += texturePitch;
   }

   pTexture->Unlock();
}

//----------------------------------------------------------------------------

void BP_UpdateDynamicTextureCLUT256(unsigned int texture, void* vram, void* paletteVram)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;

   uint32* pTextureMem = NULL;
   int texturePitch = 0;

   pTexture->Lock((void**)&pTextureMem, &texturePitch);
   texturePitch >>= 2;

   int const height = pTexture->GetHeight();
   int const width = pTexture->GetWidth();

   char* sourceData = (char*)vram;
   uint32* clut = (uint32*)paletteVram;

   for( int y = 0; y < height; ++y )
   {
      for( int x = 0; x < width; ++x )
      {
         unsigned char s = *sourceData++;
         pTextureMem[x] = clut[s]; 
      }

      pTextureMem += texturePitch;
   }

   pTexture->Unlock();
}

//----------------------------------------------------------------------------

void BP_UpdateDynamicTextureRGBA(unsigned int texture, void* vram)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;

   unsigned char* pTextureMem = NULL;
   int texturePitch = 0;

   pTexture->Lock((void**)&pTextureMem, &texturePitch);
   
   int const height = pTexture->GetHeight();
   int const width = pTexture->GetWidth();
   
   unsigned char* src = (unsigned char*)vram;

   for( int y = 0; y < height; ++y )
   {
      // copy the current row.
      unsigned char* dst = ( unsigned char* )pTextureMem;
      for( int x = 0; x < width; ++x, src += 4, dst += 4 )
      {
#if defined( BP_WIN32 ) || defined(BP_VITA)
         dst[ 0 ] = src[ 0 ];
         dst[ 1 ] = src[ 1 ];
         dst[ 2 ] = src[ 2 ];
         dst[ 3 ] = src[ 3 ];
#else
         dst[ 0 ] = src[ 3 ];
         dst[ 1 ] = src[ 2 ];
         dst[ 2 ] = src[ 1 ];
         dst[ 3 ] = src[ 0 ];
#endif
      }

      // advance to the next row.
      pTextureMem += texturePitch;
   }

   pTexture->Unlock();   
}

//----------------------------------------------------------------------------

void BP_FreeDynamicTexture(unsigned int texture)
{
   if( texture )
   {
      CBaseTexture* pTexture = (CBaseTexture*)texture;
      CBaseTexture::DeleteTexture(pTexture);
   }
}

//----------------------------------------------------------------------------

void BP_FreeDynamicTexture_Buffered(SBP_BufferedTexture* pBT)
{
   for( int i=0; i<BP_BUFFEREDTEXTURE_MAX_TEXTURE; ++i )
   {
      BP_FreeDynamicTexture(pBT->aTextureHandle[i]);
      pBT->aTextureHandle[i] = NULL;
   }
}

//----------------------------------------------------------------------------

void BP_FreeDynamicTexture_Buffered_Pointer(SBP_BufferedTexture* pBT)
{
   if( pBT )
   {
      BP_FreeDynamicTexture_Buffered(pBT);
      delete pBT;
   }
}

//----------------------------------------------------------------------------

void BP_Debug_ChangeTextureToCheckerboard(unsigned int texture)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;
   
   char* pMemory;
   int pitch;
   pTexture->Lock((void**)&pMemory, &pitch);
  
   for( int y = 0; y < pTexture->GetHeight(); ++y )
   {
      for( int x = 0; x < pTexture->GetWidth(); ++x )
      {
         uint32* pColor = (uint32*)(pMemory + pitch * y + (x * 4));
         *pColor = ( (x + y) & 1 ) ? 0xFFFFFFFF : 0xFF000000;
      }
   }

   pTexture->Unlock();
}

//----------------------------------------------------------------------------

int BP_GetDynamicTextureWidth(unsigned int texture)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;
   return pTexture ? pTexture->GetWidth() : 0;
}

//----------------------------------------------------------------------------

int BP_GetDynamicTextureHeight(unsigned int texture)
{
   CBaseTexture* pTexture = (CBaseTexture*)texture;
   return pTexture ? pTexture->GetHeight() : 0;
}

//----------------------------------------------------------------------------

void BP_ReplaceTexture(unsigned int sourceTexture, unsigned int destTexture)
{
   if( sourceTexture && destTexture )
   {
      // if we don't do this we change a CBaseTexture while it is in use by the render thread
      BP_WaitForLastRenderThreadToComplete(); 

      CBaseTexture* pSource = (CBaseTexture*)sourceTexture;
      CBaseTexture* pDest = (CBaseTexture*)destTexture;
      pDest->CloneTexture(pSource);
   }
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 3

void BP_SetTexturePacket(char* pData)
{
   SBP_SetTexture* pPacket = (SBP_SetTexture*)pData;
   DG_TEX_BP* pTex = pPacket->tex;

   CBaseTexture const* pTexture = (CBaseTexture *)pTex->BP_TextureHandle;
   if( pTexture )
   {
      BP_GS_SetAlpha(pTex->tex_trans.alpha.data);
      BP_SetTexture(pTex, 0);
   
      real32 temp[4];
      temp[0] = 1.0f/pTexture->GetWidth(); temp[1] = 1.0f/pTexture->GetHeight(); temp[2] = 0.0f; temp[3] = 0.0f;
      gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, (CVector4 const*)temp);
   }
   else
   {
      BP_DebugText_Print("Invalid BP_SetTexturePacket: NULL ptr");
   }
}

#elif MGS_VERSION == 2

extern "C" DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format )
{
   int		image_size, palette_size, total_size, pitch;
   void	*addr ;

   switch ( format )
   {
     case DG_TEXLIN_FORMAT_A8R8G8B8:
        image_size = width * height * 4 ;
        palette_size = 0 ;
        pitch = width * 4 ;
        break ;
     case DG_TEXLIN_FORMAT_A1R5G5B5:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;
     case DG_TEXLIN_FORMAT_A4R4G4B4:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;

     case DG_TEXLIN_FORMAT_P8:
     case DG_TEXLIN_FORMAT_L8:
        image_size = width * height ;
        palette_size = 256 * 4 ;
        pitch = width ;
        break ;

     case DG_TEXLIN_FORMAT_A8:
        image_size = width * height ;
        palette_size = 256 * 4 ;
        pitch = width ;
        break ;

     case DG_TEXLIN_FORMAT_R5G6B5:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;

     case DG_TEXLIN_FORMAT_X1R5G5B5:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;
   }

   total_size = image_size + palette_size ;

   addr = GV_Malloc(total_size);
   
   if ( addr == NULL )
   {
      ASSERT( 0 );
      return ( NULL );
   }

   DG_TEX_LIN* tex = (DG_TEX_LIN*)GV_Malloc( sizeof(DG_TEX_LIN) );

   if ( tex == NULL )
   {
      ASSERT( 0 );
      BPE_FREE_ALIGNED(addr);
      return ( NULL );
   }

   tex->width = width ;
   tex->height = height ;
   tex->format = format ;
   tex->flag = 0 ;
   tex->image = addr ;
   tex->image_dirty = 1;
   tex->BP_TextureHandle = (unsigned int)CBaseTexture::Create(width, height, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, kRM_System);

   return tex;
}

//----------------------------------------------------------------------------

extern "C" void DG_MakeLinerTexture2( DG_TEX_LIN *tex, int width, int height, int format, void *addr )
{
   int		image_size, palette_size, total_size, pitch ;

   switch ( format )
   {
     case DG_TEXLIN_FORMAT_A8R8G8B8:
        image_size = width * height * 4 ;
        palette_size = 0 ;
        pitch = width * 4 ;
        break ;
     case DG_TEXLIN_FORMAT_A1R5G5B5:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;
     case DG_TEXLIN_FORMAT_A4R4G4B4:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;
     case DG_TEXLIN_FORMAT_P8:
     case DG_TEXLIN_FORMAT_L8:
        image_size = width * height ;
        palette_size = 256 * 4 ;
        pitch = width ;
        break ;
     case DG_TEXLIN_FORMAT_A8:
        image_size = width * height ;
        palette_size = 256 * 4 ;
        pitch = width ;
        break ;
     case DG_TEXLIN_FORMAT_R5G6B5:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;
     case DG_TEXLIN_FORMAT_X1R5G5B5:
        image_size = width * height * 2 ;
        palette_size = 0 ;
        pitch = width * 2 ;
        break ;
   }

   tex->width = width ;
   tex->height = height ;
   tex->format = format ;
   tex->flag = 0 ;
   tex->image = addr;
   tex->image_dirty = 0;
   tex->BP_TextureHandle = (unsigned int)CBaseTexture::Create(width, height, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, kRM_System);
}

//----------------------------------------------------------------------------

extern "C" void DG_FreeLinerTexture( DG_TEX_LIN *tex )
{
   if ( tex == NULL ) 
      return;

   CBaseTexture* pTexture = (CBaseTexture*)tex->BP_TextureHandle;
   if( pTexture )
   {
      CBaseTexture::DeleteTexture(pTexture);
      tex->BP_TextureHandle = 0;
   }

   GV_DelayedFree( tex->image );
   tex->image = NULL;

   GV_DelayedFree( tex );
}

//----------------------------------------------------------------------------

extern "C" void DG_FreeLinerTexture2( DG_TEX_LIN *tex )
{
   // this is a complement to DG_MakeLinerTexture2
   // tex->image is allocated outside the TEX_LIN
   // also, tex is a member and not an allocated pointer

   CBaseTexture* pTexture = (CBaseTexture*)tex->BP_TextureHandle;
   if( pTexture )
   {
      CBaseTexture::DeleteTexture(pTexture);
      tex->BP_TextureHandle = 0;
   }
}

//----------------------------------------------------------------------------

#define BP_PACK_TEX_COLOR(r, g, b, a) ((uint32)a << 24 | (uint32)r << 16 | (uint32)g << 8 | (uint32)b)

void BP_SetLinerTexture(DG_TEX_LIN* tex, int texUnit)
{
   CBaseTexture * pTexture = NULL;

   if( tex->BP_TextureHandle )
   {
      pTexture = (CBaseTexture*)tex->BP_TextureHandle;
      
      if( tex->image_dirty )
      {
         tex->image_dirty = 0;

         int const format = tex->format;

         uint8* pSourceImageU8 = (uint8*)tex->image;
         uint16* pSourceImageU16 = (uint16*)tex->image;
         uint32* pSourceImageU32 = (uint32*)tex->image;

         uint32* pPalette = (uint32*)(pSourceImageU8 + (tex->width*tex->height));

         uint32* pTextureMem = NULL;
         int texturePitch;
         
         pTexture->Lock((void**)&pTextureMem, &texturePitch);

         for(int y = 0; y < tex->height; ++y )
         {
            for( int x = 0; x < tex->width; ++x )
            {
               uint32* pOutColor = pTextureMem + (texturePitch >> 2) * y + x;

               int const sourceOffset = y * tex->width + x;

               switch( format )
               {
               case DG_TEXLIN_FORMAT_A8R8G8B8:
                  {
                     uint32 argb = *(pSourceImageU32 + sourceOffset);
                     uint8 const a = (argb & 0xff000000)>>24;
                     uint8 const r = (argb & 0x00ff0000)>>16;
                     uint8 const g = (argb & 0x0000ff00)>>8;
                     uint8 const b = (argb & 0x000000ff)>>0;

                     *pOutColor = BP_PACK_TEX_COLOR(r, g, b, a);
                  }
                  break;

               case DG_TEXLIN_FORMAT_A1R5G5B5:
                  {
                     uint16 argb = *(pSourceImageU16 + sourceOffset);
                     uint8 const a = (argb & 0x8000) * 255 / 0x8000;
                     uint8 const r = (argb & 0x7c00) * 255 / 0x7c00;
                     uint8 const g = (argb & 0x03e0) * 255 / 0x03e0;
                     uint8 const b = (argb & 0x001f) * 255 / 0x001f;

                     *pOutColor = BP_PACK_TEX_COLOR(r, g, b, a);
                  }
                  break;

               case DG_TEXLIN_FORMAT_A4R4G4B4:
                  *pOutColor = 0xFF0000FF;
                  break ;
               
               case DG_TEXLIN_FORMAT_P8:
                  {
                     uint8 const index = *(pSourceImageU8 + sourceOffset);
                     *pOutColor = pPalette[index];
                  }
                  break;

               case DG_TEXLIN_FORMAT_L8:
                  *pOutColor = BP_PACK_TEX_COLOR(255, 255, 0, 255);
                  break ;
               
               case DG_TEXLIN_FORMAT_A8:
                  {
                     uint8 const alpha = *(pSourceImageU8 + sourceOffset);
                     *pOutColor = BP_PACK_TEX_COLOR(255, 255, 255, alpha);
                  }
                  break ;
               
               case DG_TEXLIN_FORMAT_R5G6B5:
                  {
                     uint16 argb = *(pSourceImageU16 + sourceOffset);

                     uint8 const a = 128;
                     uint8 const r = (argb & 0xf800) * 255 / 0xf800;
                     uint8 const g = (argb & 0x07e0) * 255 / 0x07e0;
                     uint8 const b = (argb & 0x001f) * 255 / 0x001f;

                     *pOutColor = BP_PACK_TEX_COLOR(r, g, b, a);
                  }
                  break ;
               
               case DG_TEXLIN_FORMAT_X1R5G5B5:
                  {
                     uint16 argb = *(pSourceImageU16 + sourceOffset);

                     uint8 const a = 128;
                     uint8 const r = (argb & 0x7c00) * 255 / 0x7c00;
                     uint8 const g = (argb & 0x03e0) * 255 / 0x03e0;
                     uint8 const b = (argb & 0x001f) * 255 / 0x001f;

                     *pOutColor = BP_PACK_TEX_COLOR(r, g, b, a);
                  }
                  break;
               }
            }
         }

         pTexture->Unlock();
      }
   }
   else
   {
      pTexture = (CBaseTexture *)&gpRenderBackend->GetWhiteMap();
   }

   gpRenderBackend->SetTexture(texUnit, pTexture);
   gpRenderBackend->SetTextureFilter(texUnit, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);

}

#endif // MGS_VERSION == 2

//----------------------------------------------------------------------------

void BP_AllocViewportTexture(unsigned int * outTexture, int width, int height, int hasAlpha)
{
   BPE_VERIFY(*outTexture == 0, false, "Didn't free previous texture!");
   
   CBaseTexture* pTexture = CBaseTexture::Create(width, height, 1, hasAlpha ? CBaseTexture::kFormat_A8R8G8B8 : CBaseTexture::kFormat_X8R8G8B8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_System);
   *outTexture = (unsigned int)pTexture;
   ++gViewportTextureCount;
}

//----------------------------------------------------------------------------

void BP_FreeViewportTexture(unsigned int * texture)
{
   CBaseTexture* pTexture = (CBaseTexture*)*texture;
   if( pTexture )
   {
      --gViewportTextureCount;
      CBaseTexture::DeleteTexture(pTexture);
      *texture = 0;
   }
}

unsigned int BP_GetRenderTarget_c(unsigned int id)
{
   return (unsigned int)BP_GetRenderTarget((ERenderTarget)id);
}

//----------------------------------------------------------------------------

#ifndef GOLD_VERSION
char const *BP_GetDebugNameFromMesh( unsigned mesh )
{
   return ((CMesh *) mesh )->mDebugName.c_str();
}
#endif

//----------------------------------------------------------------------------

#if BP_VITA
#  define BP_ENABLE_VITA_ULT 1
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_VITA_ULT

#include <sce_atomic.h>

#include <boost/circular_buffer.hpp>
#include "Engine/System/CSyncEvent.h"

//----------------------------------------------------------------------------
// CVTAWorkQueue
//
// This is a queue whose items are pushed from a main thread and popped from a
// worker thread using condition variables.
//
// Inspired by:
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
//----------------------------------------------------------------------------

class CVTAWorkQueue
{
public:
   // SQueueItem is simply a function ptr and its parameters
   struct SQueueItem
   {
      typedef void (*TQueueFunc)( SULTParam const *param );

      SQueueItem( TQueueFunc func, void const *work, int const size )
         : mFunc( func )
      {
         memcpy( &mParam, work, size );
      }

      SQueueItem()
         : mFunc( NULL )
      {
      }

      TQueueFunc mFunc;
      SULTParam mParam;
   };

   explicit CVTAWorkQueue( char const *desc, int const dgVariableIndex )
      : mThreadId( -1 )
      , mThreadIdDrainer( -1 )
      , mThreadEmptyQueueFlag( false, false )
      , mDrainerHelperPlease( false, false )
      , mQueue( skMaxQueueItems )
      , mQueueItemCount( 0 )
      , mFastDrainHead( 0 )
      , mFastDrainCount( 0 )
      , mWorkInProgress( 0 )
      , mShouldSignalEmpty( false )
      , mbIsLocked( false )
      , mDGVariableIndex( dgVariableIndex )
   {
      // Creates the event flag, mutex, and condtion variable
      // Then creates and starts a worker thread

      BPE_CHECK_SCE( sceKernelCreateLwMutex( &mLWMutex, ( std::string( desc ) + " Mutex" ).c_str(), SCE_KERNEL_LW_MUTEX_ATTR_TH_FIFO, 0, NULL ) );
      BPE_CHECK_SCE( sceKernelCreateLwCond( &mLWCond, ( std::string( desc ) + " CV" ).c_str(), SCE_KERNEL_LW_COND_ATTR_TH_FIFO, &mLWMutex, NULL ) );
      mThreadId = BPE_CHECK_SCE( sceKernelCreateThread( 
         desc, 
         StaticThreadFunction, 
         NVtaThreadPriorities::kPriorityUltWorkers, 
         32 * 1024, 
         0,
         SCE_KERNEL_CPU_MASK_USER_ALL,
         NULL ) );

      mThreadIdDrainer = BPE_CHECK_SCE( sceKernelCreateThread(
         ( std::string( desc ) + " DR" ).c_str(),
         StaticThreadFunctionDrainer,
         NVtaThreadPriorities::kPriorityUltDrainers,
         32 * 1024,
         0,
         SCE_KERNEL_CPU_MASK_USER_ALL,
         NULL ) );

      CVTAWorkQueue *pThisPtr = this;

      BPE_CHECK_SCE( sceKernelStartThread( mThreadId, sizeof( pThisPtr ), &pThisPtr ) );
      BPE_CHECK_SCE( sceKernelStartThread( mThreadIdDrainer, sizeof( pThisPtr ), &pThisPtr ) );
   }

   bool LockPushQueueItemsFromMainThread( int const count, int *pUnlockParam )
   {
      BPE_ASSERT( !mbIsLocked, "Trying to re-lock a locked ULT thread" );

      *pUnlockParam = 0;

      sceKernelLockLwMutex( &mLWMutex, 1, NULL );
      
      // If we can push all, return and keep locked
      if( mQueueItemCount + count <= skMaxQueueItems )
      {
         mbIsLocked = true;
         return true;
      }
      else
      {
         // Unlock and return
         sceKernelUnlockLwMutex( &mLWMutex, 1 );

         return false;
      }
   }

   void PushQueueItemLockedFromMainThread( SQueueItem const &item, int *pUnlockParam )
   {
      BPE_ASSERT( mbIsLocked, "Trying push locked work to an unlocked thread" );

      int const oldCount = PushQueueItemUncheckedReturnOldCount( item );

      if ( oldCount == 0 )
      {
         *pUnlockParam = 1;
      }
   }

   void UnlockPushQueueItemsFromMainThread( int unlockParam )
   {
      BPE_ASSERT( mbIsLocked, "Unlocking an unlock thing" );

      mbIsLocked = false;

      if ( unlockParam )
      {
         // If the unlock param is 1, then we need to signal the consumer
         // thread to go

         sceKernelSignalLwCond( &mLWCond );
      }

      sceKernelUnlockLwMutex( &mLWMutex, 1 );
   }

   void PushQueueItemFromMainThread( SQueueItem const &item )
   {
      BPE_ASSERT( !mbIsLocked, "Trying to push unlocked work to a locked thread" );

      sceKernelLockLwMutex( &mLWMutex, 1, NULL );

      // If we can push it, push and unlock,
      // if we can't, unlock then execute
      if ( mQueueItemCount < skMaxQueueItems )
      {
         int const oldCount = PushQueueItemUncheckedReturnOldCount( item );
         
         // Only need to signal the condition var
         // if we added an object to an empty queue
         //
         // otherwise the consumer thread will get to it
         // before checking the condition var
         if ( oldCount == 0 )
         {
            sceKernelSignalLwCond( &mLWCond );
         }
         sceKernelUnlockLwMutex( &mLWMutex, 1 );
      }
      else
      {
         sceKernelUnlockLwMutex( &mLWMutex, 1 );
         item.mFunc( &item.mParam );
      }
   }

   void WaitForWorkToCompleteFromMainThread()
   {
      // Our thread is executing, let's do a double check lock
      // This means we check, lock the mutex, then check again before doing hard
      // work
      //
      // "Hard work" means telling the worker thread to set an event flag when
      // work is complete, and for us to wait for it.
      // 
      // Since that's a syscall, we'd like to minimize that.

      if ( mQueueItemCount + mWorkInProgress )
      {
         sceKernelLockLwMutex( &mLWMutex, 1, NULL );

         if ( mQueueItemCount + mWorkInProgress )
         {
            // We are still locked, so let's tell the other thread we 
            // want to know when it's done, signal it, then wait
            BPE_ASSERT( mShouldSignalEmpty == false, "Signal empty out of whack (1)?" );
            mShouldSignalEmpty = true;

            // Wake the other thread and release the mutex (which actually happens in the opposite order)
            // This kicks the other thread to check the mShouldSignalEmpty var above
            sceKernelSignalLwCond( &mLWCond );
            sceKernelUnlockLwMutex( &mLWMutex, 1 );

            // Now we just wait for the ult thread to idle
            mThreadEmptyQueueFlag.Wait();

            // The other thread should have set signal empty to false by now
            BPE_ASSERT( mShouldSignalEmpty == false, "Signal empty out of whack (2)?" );
            BPE_ASSERT( mWorkInProgress == 0, "Signal empty out of whack (2)?" );
         }
         else
         {
            // The other thread finished its work while we were locking the mutex,
            // so just unlock the mutex and continue.
            sceKernelUnlockLwMutex( &mLWMutex, 1 );
         }
      }
   }

   void HelpConsumeRemainingWorkFromMainThread( bool const wantsExtraThread )
   {
      // Note that this function does not ensure that all work is 
      // complete.  Rather, it just ensures that there is no work remaining
      // to be started.  The last work item may still be in-flight from the 
      // worker thread.

      // Do a quick check that the queue's not empty
      // before doing mutex-y work

      if ( mQueueItemCount )
      {
         // Lock the mutex, drain, unlock and go

         sceKernelLockLwMutex( &mLWMutex, 1, NULL );
         
         TransferQueueItemsToFastDrainUnchecked();
         
         sceKernelUnlockLwMutex( &mLWMutex, 1 );

         if ( wantsExtraThread )
         {
            // Increment the work in progress (the drainer helper will decrement it)
            sceAtomicIncrement32( &mWorkInProgress );

            mDrainerHelperPlease.Set();
         }

         for ( ;; )
         {
            int cnt = sceAtomicIncrement32( &mFastDrainHead );

            if ( cnt < mFastDrainCount )
            {
               mFastDrainItems[ cnt ].mFunc( &( mFastDrainItems[cnt].mParam ) );
            }
            else
            {
               break;
            }
         }
      }
   }

private:
   static SceInt32 StaticThreadFunction( SceSize argSize, void *pArgBlock )
   {
      CVTAWorkQueue **ppWorkQueue = reinterpret_cast<CVTAWorkQueue **>( pArgBlock );

      return (*ppWorkQueue)->ThreadFunction();
   }

   static SceInt32 StaticThreadFunctionDrainer( SceSize argSize, void *pArgBlock )
   {
      CVTAWorkQueue **ppWorkQueue = reinterpret_cast<CVTAWorkQueue **>( pArgBlock );

      return (*ppWorkQueue)->ThreadFunctionDrainer();
   }

   SceInt32 ThreadFunctionDrainer()
   {
      DG_AS_CurrentThreadVarsIndex = mDGVariableIndex;
      GV_AS_EnableScratchpadFromThisThread();

      for ( ;; )
      {
         mDrainerHelperPlease.Wait();

         if ( mFastDrainHead < mFastDrainCount )
         {
            for ( ;; )
            {
               int cnt = sceAtomicIncrement32( &mFastDrainHead );

               if ( cnt < mFastDrainCount )
               {
                  mFastDrainItems[ cnt ].mFunc( &( mFastDrainItems[cnt].mParam ) );
               }
               else
               {
                  break;
               }
            }

         }

         // Decrement work-in-progress.  This had been incremented in the main thread
         // the reason it was done in the main thread was for the situation where
         // this thread didn't wake up until after the entire drain process was complete
         sceAtomicDecrement32( &mWorkInProgress );

         sceKernelLockLwMutex( &mLWMutex, 1, NULL );
         sceKernelSignalLwCond( &mLWCond );
         sceKernelUnlockLwMutex( &mLWMutex, 1 );
      }
   }

   SceInt32 ThreadFunction()
   {
      DG_AS_CurrentThreadVarsIndex = mDGVariableIndex;
      GV_AS_EnableScratchpadFromThisThread();

      BP_Debug_PushCPUMarker( "BP_Renderer_Ult" );

      // Simple loop of lock mutex, wait on condition var (which
      // unlocks mutex and locks again when the var is set), pop
      // item, unlock mutex, and work

      sceKernelLockLwMutex( &mLWMutex, 1, NULL );

      for ( ;; )
      {

         while ( mQueueItemCount == 0 && mFastDrainHead >= mFastDrainCount )
         {
            if ( mShouldSignalEmpty && mWorkInProgress == 0 )
            {
               mShouldSignalEmpty = false;
               mThreadEmptyQueueFlag.Set();
            }

            sceKernelWaitLwCond( &mLWCond, NULL );
         }

         // Note - we are incrementing work in progress before
         // we pop from the queue so that mWorkInProgress + mQueueCount
         // is nonzero
         sceAtomicIncrement32( &mWorkInProgress );

         // If we have real queue items, pop one and go
         if ( mQueueItemCount )
         {
            SQueueItem item;
            PopQueueItemUnchecked( &item );

            sceKernelUnlockLwMutex( &mLWMutex, 1 );

            item.mFunc( &item.mParam );

            sceKernelLockLwMutex( &mLWMutex, 1, NULL );
         }

         // If we have now moved into fast drain mode, unlock the mutex and drain like 
         // there's no tomorrow.
         if ( mFastDrainHead < mFastDrainCount )
         {
            sceKernelUnlockLwMutex( &mLWMutex, 1 );

            for ( ;; )
            {
               int cnt = sceAtomicIncrement32( &mFastDrainHead );

               if ( cnt < mFastDrainCount )
               {
                  mFastDrainItems[ cnt ].mFunc( &( mFastDrainItems[cnt].mParam ) );
               }
               else
               {
                  break;
               }
            }

            sceKernelLockLwMutex( &mLWMutex, 1, NULL );
         }

         sceAtomicDecrement32( &mWorkInProgress );
      }

      BP_Debug_PopCPUMarker();
   }

   void PopQueueItemUnchecked( SQueueItem *pParam );
   int PushQueueItemUncheckedReturnOldCount( SQueueItem const &item );
   void TransferQueueItemsToFastDrainUnchecked();

   SceUID mThreadId;
   SceUID mThreadIdDrainer;
   CSyncEvent mThreadEmptyQueueFlag;
   CSyncEvent mDrainerHelperPlease;
   SceKernelLwMutexWork mLWMutex;
   SceKernelLwCondWork mLWCond;

   static const int skMaxQueueItems = 512;
   boost::circular_buffer< SQueueItem > mQueue;
   volatile int mQueueItemCount;
   SQueueItem mFastDrainItems[ 512 ];
   volatile int mFastDrainHead;
   int mFastDrainCount;
   volatile int mWorkInProgress;
   volatile bool mShouldSignalEmpty;
   bool mbIsLocked;
   int mDGVariableIndex;

};

//----------------------------------------------------------------------------
// PopQueueItemUnchecked
//----------------------------------------------------------------------------
//
// Does a simple pop of a queue item and an atomic decrement of queue item count
// Assumes that the caller has actually checked that the queue is poppable
//
// Turning off inlining on purpose
// So that the state of sQueue is never assumed, as sQueue is not volatile
__attribute__((noinline))
void CVTAWorkQueue::PopQueueItemUnchecked( SQueueItem *pParam )
{
   // Not inlined so that sQueue is never assumed to be not changing
   BPE_ASSERT( mQueueItemCount > 0, "Queue item count invalid" );

   sceAtomicDecrement32( &mQueueItemCount );

   *pParam = mQueue.front();
   mQueue.pop_front();
}

//----------------------------------------------------------------------------

void CVTAWorkQueue::TransferQueueItemsToFastDrainUnchecked()
{
   int const queueItemCount = mQueueItemCount;

   for ( int i = 0; i < queueItemCount; ++i )
   {
      mFastDrainItems[i] = mQueue.front();
      mQueue.pop_front();
   }

   mQueueItemCount = 0;
   mFastDrainHead = 0;
   mFastDrainCount = queueItemCount;
}

//----------------------------------------------------------------------------
// PushQueueItemUncheckedReturnOldCount
//----------------------------------------------------------------------------
//
// Does a simple push of a queue item and an atomic incremetn of queue item count
// Assumes that the caller has actually checked that the queue is pushable
//
// Turning off inlining on purpose
// So that the state of sQueue is never assumed, as sQueue is not volatile
__attribute__((noinline))
int CVTAWorkQueue::PushQueueItemUncheckedReturnOldCount( SQueueItem const &item )
{
   // Not inlined so sQueue is never assumed to not be changing
   int oldCount = sceAtomicIncrement32( &mQueueItemCount );
   mQueue.push_back( item );

   return oldCount;
}

namespace NBP_Render_Ult
{
   static int const skMaxWorkQueues = 2;
   volatile SceInt32 sWorkQueuesInUse = 0;
   __thread CVTAWorkQueue *sThreadWorkQueue = NULL;
}

#endif

//----------------------------------------------------------------------------

void BP_Render_EnableUltFromThisThread( int const dgVariableIndex )
{
#if BP_ENABLE_VITA_ULT
   if ( NBP_Render_Ult::sThreadWorkQueue == NULL )
   {
      SceInt32 currentWorkQueue = sceAtomicIncrement32( &NBP_Render_Ult::sWorkQueuesInUse );

      BPE_VERIFY( currentWorkQueue < NBP_Render_Ult::skMaxWorkQueues, false, "Out of work queues!" );
      char debugString[ 80 ];
      sprintf( debugString, "Aux Work %d", currentWorkQueue );

      // Note - this work queue is never freed, but there are only 2 of them
      NBP_Render_Ult::sThreadWorkQueue = new CVTAWorkQueue( debugString, dgVariableIndex );
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Render_PostUltWork( TBP_Render_Ult_Function func, void const *work, int const size )
{
#if BP_ENABLE_VITA_ULT
   CVTAWorkQueue *pWorkQueue = NBP_Render_Ult::sThreadWorkQueue;
   
   if ( !pWorkQueue )
   {
      // No work queue for this thread, just call the function
      (*func)( ( SULTParam const * ) work );
   }
   else
   {
      pWorkQueue->PushQueueItemFromMainThread( CVTAWorkQueue::SQueueItem( func, work, size ) );
   }
#else
   (*func)( ( SULTParam const * ) work );
#endif
}

//----------------------------------------------------------------------------

void BP_Render_PostUltWorks( TBP_Render_Ult_Function func, void const *work, int const size, int const count )
{
#if BP_ENABLE_VITA_ULT
   CVTAWorkQueue *pWorkQueue = NBP_Render_Ult::sThreadWorkQueue;

   if ( pWorkQueue )
   {
      int unlockParam = 0;
      if ( pWorkQueue->LockPushQueueItemsFromMainThread( count, &unlockParam ) )
      {
         for ( int i = 0; i < count; ++i )
         {
            pWorkQueue->PushQueueItemLockedFromMainThread( CVTAWorkQueue::SQueueItem( 
               func, 
               (SULTParam const *) ( reinterpret_cast<char const *>(work) + size * i ), 
               size ),
               &unlockParam);
         }

         pWorkQueue->UnlockPushQueueItemsFromMainThread( unlockParam );

         return;
      }
   }
#endif
   for ( int i = 0; i < count; ++i )
   {
      (*func)( ( SULTParam const * ) ( reinterpret_cast<char const *>(work) + size * i ) );
   }

}

//----------------------------------------------------------------------------

void BP_Render_HelpDrainUltWorkEx( int bool_extraThread )
{
#if BP_ENABLE_VITA_ULT
   CVTAWorkQueue *pWorkQueue = NBP_Render_Ult::sThreadWorkQueue;
   if ( pWorkQueue )
   {
      pWorkQueue->HelpConsumeRemainingWorkFromMainThread( bool_extraThread != 0 );
   }

   // Just because the queue is empty doesn't mean that work is done.
   BP_Render_SyncUltWork();
#endif   
}

//----------------------------------------------------------------------------

void BP_Render_SyncUltWork()
{
#if BP_ENABLE_VITA_ULT
   CVTAWorkQueue *pWorkQueue = NBP_Render_Ult::sThreadWorkQueue;
   if ( pWorkQueue )
   {
      pWorkQueue->WaitForWorkToCompleteFromMainThread();
   }

#endif
}

//----------------------------------------------------------------------------

void BP_Render_AllocAndLockDynamicVertexBufferChunk_UT( void **pVertexBufferChunk, void **pWritableAddress, int const size )
{
   CDynamicVertexBufferPoolChunk_UT *pChunk = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk( size );

   *pVertexBufferChunk = pChunk;

   if ( pChunk )
   {
      *pWritableAddress = pChunk->Lock();
   }
   else
   {
      *pWritableAddress = NULL;
   }
}

//----------------------------------------------------------------------------

void BP_Render_UnlockDynamicVertexBufferChunk_UT( void *pVertexBufferChunk )
{
   reinterpret_cast< CDynamicVertexBufferPoolChunk_UT * >( pVertexBufferChunk )->Unlock();
}

//----------------------------------------------------------------------------

#if MGS_VERSION==2

void BP_Render_AllocAndLockDynamicIndexBufferChunk_UT( int *pIndexBufferChunk, void **pWritableAddress, int const size )
{
   CDynamicIndexBufferPool_UT::TChunk pChunk = gpRenderBackend->GetIndexBufferPool_UT()->AllocChunk( size );

   CDynamicIndexBufferPool_UT::CopyToInts( pIndexBufferChunk, pChunk );

   if ( !CDynamicIndexBufferPool_UT::IsChunkNull( pChunk ) )
   {
      *pWritableAddress = CDynamicIndexBufferPool_UT::LockChunk( pChunk );
   }
   else
   {
      *pWritableAddress = NULL;
   }
}

//----------------------------------------------------------------------------

void BP_Render_UnlockDynamicIndexBufferChunk_UT( int const *pIndexBufferChunk )
{
   CDynamicIndexBufferPool_UT::UnlockChunk( CDynamicIndexBufferPool_UT::ChunkFromInts( pIndexBufferChunk ) );
}

#endif

#if BP_VITA
void *BP_Render_AllocDynamicVertexBufferChunk_UT_Vita_Unsafe( int const size )
{
   return gpRenderBackend->GetVertexBufferPool_UT()->AllocChunkUnsafeVita( size );
}
#endif

#if MGS2_DYNAMIC_FRAMERATE

namespace NMGS2_3060
{
   uint64 gCanEndframeHit60 = 0;
   uint64 gCanGameframeHit60 = 0;
   uint64 gCanRenderHit60 = 0;
   int    gCanHardwareHit60 = 0;
   int    gGameModeAllows60 = 0;

   bool gAreWeAt60 = false;

   char s30FPSHistory[8 /* MUST BE POWER OF TWO */ ] = { 0 };
   int s30FPSHistoryPivot = 0;

   inline void add_history_item( bool is30fps )
   {
      // Puts an "is 30" into the history round robin

      s30FPSHistory[ s30FPSHistoryPivot ] = is30fps;
      s30FPSHistoryPivot = ( s30FPSHistoryPivot + 1 ) & ( BPE_ARRAY_SIZE( s30FPSHistory ) - 1 );
   }

   inline bool should_switch_to_30()
   {
      // This is to check if, while at 60, if we should switch to 30.
      // We do this by seeing if we had more than a certain number of frames
      // that didn't run at 60 in our history. Note that these don't need to
      // be consecutive frames.

      int count30 = 0;
      static int const sk30fpsFramesToSwitch = 3;

      for ( int i = 0; i < BPE_ARRAY_SIZE( s30FPSHistory ); ++i )
      {
         count30 += s30FPSHistory[i];
      }

      return ( count30 > sk30fpsFramesToSwitch );
   }

   inline bool should_switch_to_60()
   {
      // This is to check if we can switch to 60 from 30.
      // This just checks the history and sees if all frames in the history can run at 60.

      for ( int i = 0; i < BPE_ARRAY_SIZE( s30FPSHistory ); ++i )
      {
         if ( s30FPSHistory[i] )
         {
            return false;
         }
      }

      return true;
   }

   void SetAt60Immediate( bool b )
   {
      // Sets our 60 fps status right now and stomps all over history doing it

      gAreWeAt60 = b;
      memset( s30FPSHistory, !b, sizeof( s30FPSHistory ) );
   }

   void SetFrameCanDo60WithHistory( bool b )
   {
      // Adds this item to the history, and sees if it's enough
      // to cause a switch

      add_history_item( !b );

      if ( b != gAreWeAt60 )
      {
         if ( gAreWeAt60 )
         {
            if ( should_switch_to_30() )
            {
               SetAt60Immediate( false );
            }
         }
         else 
         {
            if ( should_switch_to_60() )
            {
               SetAt60Immediate( true );
            }
         }
      }
   }
}

void BP_Renderer_MGS2_3060_SetEndframeTimeUS( uint64_t const time )
{
   // Note that we subtract the last amount of time that the endframe thread
   // waited on the render thread. If not, then the render thread waiting for vsync
   // is counted in here, which means we never go back to 60
   NMGS2_3060::gCanEndframeHit60 = time - sLastRenderWaitTime;
}

void BP_Renderer_MGS2_3060_SetRenderTimeUS( uint64_t const frameTime, int hardwareCanDo60 )
{
   // Sets flags both for frame time and if the hardware missed a vsync

   NMGS2_3060::gCanRenderHit60 = frameTime;
   NMGS2_3060::gCanHardwareHit60 = hardwareCanDo60;
}

void BP_Renderer_MGS2_3060_SetGameTimeUS( uint64_t const frameTime, int gameModeAllows60 )
{
   NMGS2_3060::gCanGameframeHit60 = frameTime;
   NMGS2_3060::gGameModeAllows60 = gameModeAllows60;
}

static bool is_mgs2_area_whitelisted_for_60( char const *area )
{
   static char const *skAreasFor60FPS[] = 
   {
      "w01a",
      "w01f",
      "w01b",
      "w01c",
      "w01d",
      "w01e",
      "w11a",
      "w11b",
      "w11c",
      "w12b",
      "w14a",
      "w15a",
      "w15b",
      "w16a",
      "w16b",
      "w18a",
      "w20a",
      "w22a",
      "w23b",
      "w24b",
      "w24d",
      "w25c",
      "w28a",
      "w31a",
      "w31d",
      "w41a",
      "w43a",
      "w51a",
      "w61a",
      NULL
   };
   static char sCachedCheckArea[ 9 ] = { 0 };
   static bool sCachedLastResult = false;

   // Just return the prior result if we already checked this area
   if ( strcmp( area, sCachedCheckArea ) != 0 )
   {
      // Otherwise, recache and search
      strcpy( sCachedCheckArea, area );
      sCachedLastResult = false;

      // AS(JM) Check for VR Missions and make them be 60fps
      if ( 
         !strncmp( area, "vs", 2 ) ||
         !strncmp( area, "wp", 2 ) || 
         !strncmp( area, "sp", 2 ) 
         )
      {
         sCachedLastResult = true;
      }
      else
      {
         // Otherwise check whitelisted areas.

         for ( char const **ppStr = skAreasFor60FPS; *ppStr; ++ppStr )
         {
            if ( !strcmp( area, *ppStr ) )
            {
               sCachedLastResult = true;
               break;      
            }
         }
      }
   }

   return sCachedLastResult;
}

void BP_Renderer_MGS2_3060_GameHeartbeat()
{
   if ( gAS_Vita_60FPSMode == kVF60_Off || !NMGS2_3060::gGameModeAllows60 )
   {
      // If 60fps is off or the game doesn't want it, set it off right now

      NMGS2_3060::SetAt60Immediate( false );
   }
   else
   {
      if ( is_mgs2_area_whitelisted_for_60( GM_GetArea() ) || gAS_Vita_60FPSMode == kVF60_AllAreas )
      {
         // Add a history item and try to switch us to or from
         // 60 if possible.

         bool const try60 = 
            ( NMGS2_3060::gCanEndframeHit60 < 15000 ) && 
            ( NMGS2_3060::gCanGameframeHit60 < 16000 ) && 
            ( NMGS2_3060::gCanRenderHit60 < 16100 ) &&
            NMGS2_3060::gCanHardwareHit60;

         NMGS2_3060::SetFrameCanDo60WithHistory( try60 );
      }
      else
      {
         // If we're in an invalid area, then turn off 60 right now

         NMGS2_3060::SetAt60Immediate( false );
      }
   
   }

   gpRenderBackend->SetTargetFPS( NMGS2_3060::gAreWeAt60 ? CRenderBackend::kFPS_60 : CRenderBackend::kFPS_30 );
}

void BP_Renderer_MGS2_3060_PrintDebugInfo( int *pcurrentx, int *pcurrenty)
{
   BP_Debug_DrawString(CStringExtras::Stringize_s( "EF: %d GF: %d R: %d H: %d\n", 
      (int) NMGS2_3060::gCanEndframeHit60,
      (int) NMGS2_3060::gCanGameframeHit60,
      (int) NMGS2_3060::gCanRenderHit60,
      NMGS2_3060::gCanHardwareHit60 ),
      pcurrentx, pcurrenty );

}

int BP_Renderer_MGS2_3060_IsAt60()
{
   // NOTE - this function is directly externed in from libdg.h

   return NMGS2_3060::gAreWeAt60;
}

#endif // MGS2_DYNAMIC_FRAMERATE
