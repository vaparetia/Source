#pragma once

//----------------------------------------------------------------------------
// BP_Renderer.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

#if 0 //MARCO
#define BP_ENABLE_SHADER_RELOAD  1
#endif

#if BP_VITA
#  define BP_RENDER_SINGLE_RENDER_TARGET 1
#endif

//----------------------------------------------------------------------------

enum ERenderTarget
{
   //
   kRT_MainBuffer,
   kRT_DefaultDepthBuffer,
   
   //
   kRT_MainBuffer1_NoMSAA,
   kRT_MainBuffer2_NoMSAA,
   kRT_DefaultDepthBuffer_NoMSAA,
   kRT_TempBuffer_NoMSAA,
   kRT_TempDepthBuffer_NoMSAA,
   
   // Below here are for special fx
   kRT_TempBufferHalfFakeMSAA,
   kRT_TempBufferHalf1,
   kRT_TempBufferHalf2,
   kRT_TempBufferQuarter1,
   kRT_TempBufferQuarter2,
#if MGS_VERSION == 3
   // These are for glare fx
   kRT_TempBufferQuarter3,
   kRT_TempBufferQuarter4,
   kRT_TempBufferQuarter5,
   kRT_TempBufferQuarterTemp, // Use for Quarter blurs
#endif

   kRT_TempBuffer8th,
   kRT_TempBuffer16th,
   kRT_TempBuffer32th,
   kRT_TempBuffer64th,
   kRT_TempBuffer128th,

   kRT_TempBuffer32px,
   kRT_TempBuffer16px,
   kRT_TempBuffer8px,
   kRT_TempBuffer4px,
   kRT_TempBuffer2px,

   kRT_TempShadow,
   kRT_TempShadow2,
   kRT_TempShadow_Depth,

   kRT_CameraScreenShot,
   kRT_CameraThumbnailScreenShot,

#if MGS_VERSION == 2
   // These are special case textures for camera screenshots when in the tanker and vr missions
   // in original these were allocated as a script command look for tank_jpeg_cam script command in gcl files (NewResidentAreaSet)
   // it is much easier/faster to deal with platform textures and not encode/decode jpeg images all the time, if this becomes a memory issue
   // we can change this, takes about 22 megs video memory
   kRT_TankerSS0,
   kRT_TankerSS1,
   kRT_TankerSS2,
   kRT_TankerSS3,
   kRT_TankerSS4,
   kRT_TankerSS5,
#endif

   kRT_Count,

   // These are special case textures which when looked up return the correct buffer.
   kRT_CurrentFrameBuffer,

   kRT_CurrentFrameBuffer_NoMSAA,
   kRT_PreviousFrameBuffer_NoMSAA,

#if BP_VITA
   kRT_CurrentFrameBuffer_Full,
   kRT_CurrentFrameBuffer_Small,
   kRT_CurrentFrameBuffer_Low,
   kRT_CurrentDepthBuffer_Full,
   kRT_CurrentDepthBuffer_Small,
   kRT_CurrentDepthBuffer_Low,
#endif

   // Aliases below this point
   kRT_MonoMini = kRT_TempBufferHalf1,
};

//----------------------------------------------------------------------------

// Converts a PS2 screen coordinate to a new pixel coordinate.

#if BP_VITA
#define BP_REAL_SCREEN_X 960
#define BP_REAL_SCREEN_Y 544
#else
#define BP_REAL_SCREEN_X 1280
#define BP_REAL_SCREEN_Y 720
#endif

#define BP_ADJUST_SCREEN_X(value) ( (value) * BP_REAL_SCREEN_X / DRAW_WIDTH)
#define BP_ADJUST_SCREEN_Y(value) ( (value) * BP_REAL_SCREEN_Y / DRAW_HEIGHT)

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#define BP_BUFFEREDTEXTURE_MAX_TEXTURE 3
typedef struct _SBP_BufferedTexture
{
   int prevIndex;
   int currIndex;
   int lastLockedFrame;
   unsigned int aTextureHandle[BP_BUFFEREDTEXTURE_MAX_TEXTURE];
} SBP_BufferedTexture;

//----------------------------------------------------------------------------

void BP_RenderMain(int currentBuffer, int shouldDrawBuffer);
void BP_RenderPumpLoad();
void BP_WaitForLastRenderThreadToComplete();

void BP_InitShaders1();
void BP_InitShaders2();
void BP_ReloadShaders();

int BP_GetCurrentRenderBuffer();

void BP_SetRasterMask(int enable);

#ifndef GOLD_VERSION
char const *BP_GetDebugNameFromMesh( unsigned mesh );
#endif

unsigned int BP_GetFrameCount();

//----------------------------------------------------------------------------

unsigned int BP_AllocDynamicTexture(int const width, int const height, int const doClear);
void BP_AllocDynamicTexture_Buffered(int const width, int const height, int const doClear, SBP_BufferedTexture* pBT);
SBP_BufferedTexture* BP_AllocDynamicTexture_Buffered_Pointer(int const width, int const height, int const doClear);
SBP_BufferedTexture* BP_AllocDynamicTexture_SingleBuffered_Pointer(int const width, int const height, int const doClear);
void BP_LockDynamicTexture_NonLinear(unsigned int texture, void** pData, int* pitch);
void BP_UnlockDynamicTexture_NonLinear(unsigned int texture, void** pData);
void BP_UpdateDynamicTextureCLUT16(unsigned int texture, void* vram, void* paletteVram);
void BP_UpdateDynamicTextureCLUT256(unsigned int texture, void* vram, void* paletteVram);
void BP_UpdateDynamicTextureRGBA(unsigned int texture, void* vram);
void BP_FreeDynamicTexture(unsigned int texture);
void BP_FreeDynamicTexture_Buffered(SBP_BufferedTexture* pBT);
void BP_FreeDynamicTexture_Buffered_Pointer(SBP_BufferedTexture* pBT);
int BP_GetDynamicTextureWidth(unsigned int texture);
int BP_GetDynamicTextureHeight(unsigned int texture);
void BP_UpdateDynamicTexture_Command(char *pPacket);

void BP_ReplaceTexture(unsigned int sourceTexture, unsigned int destTexture);

void BP_SetTexturePacket(char* pData);

void BP_Debug_ChangeTextureToCheckerboard(unsigned int texture);

void BP_Render_AllocAndLockDynamicVertexBufferChunk_UT( void **pVertexBufferChunk, void **pWritableAddress, int const size );
void BP_Render_UnlockDynamicVertexBufferChunk_UT( void *pVertexBufferChunk );
#if BP_VITA
void *BP_Render_AllocDynamicVertexBufferChunk_UT_Vita_Unsafe( int const size );
#endif
#if MGS_VERSION==2
void BP_Render_AllocAndLockDynamicIndexBufferChunk_UT( int *pIndexBufferChunk, void **pWritableAddress, int const size );
void BP_Render_UnlockDynamicIndexBufferChunk_UT( int const *pIndexBufferChunk );
#endif

//----------------------------------------------------------------------------
// These functions deal with setting, resetting the variable pointed to, to allow for maximum error checking.
void BP_AllocViewportTexture(unsigned int * outTexture, int width, int height, int hasAlpha);
void BP_FreeViewportTexture(unsigned int * texture);

//----------------------------------------------------------------------------

unsigned int BP_GetRenderTarget_c(unsigned int id);

//----------------------------------------------------------------------------

typedef struct _SBP_ViewportInfo
{
   int hasMSAAStuff;
   int hasNonMSAAStuff;
} SBP_ViewportInfo;

void BP_SetViewportInfoPacket(SBP_ViewportInfo * pData);
SBP_ViewportInfo * BP_GetCurrentViewportInfo();


//----------------------------------------------------------------------------

struct _SULTParam
{
   unsigned int p0;
   unsigned int p1;
   unsigned int p2;
   unsigned int p3;
   unsigned int p4;
   unsigned int p5plus[2];
};

typedef struct _SULTParam SULTParam;

typedef void (*TBP_Render_Ult_Function)( SULTParam const * );
void BP_Render_EnableUltFromThisThread( int const dgVariableIndex );
void BP_Render_PostUltWork( TBP_Render_Ult_Function func, void const *param, int const size );
void BP_Render_PostUltWorks( TBP_Render_Ult_Function func, void const *param, int const size, int const count );
void BP_Render_SyncUltWork();
void BP_Render_HelpDrainUltWorkEx( int bool_extraThread );
static inline void BP_Render_HelpDrainUltWork() { BP_Render_HelpDrainUltWorkEx(0); }

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// C++ only stuff below this point
//----------------------------------------------------------------------------

#ifdef __cplusplus

#include "Engine/Math/CVector4.h"
#include "Engine/Graphics/CColor.h"

class CBaseTexture;
class CCompiledShader;

//----------------------------------------------------------------------------

#if BP_ENABLE_ONSCREEN_PROFILER

#if defined(BP_PS3)
#include "Renderer/Edge/profile_bars.h"
#elif defined(BP_360)
#include "Renderer/Base/Backend/X360/profile_bars.h"
#else
#error profiler bars not supported on unknown platform
#endif

int BP_IsProfileInfoActive();

#if defined(BP_PS3) // TODO: investigate make this platform generic in some way
#define PROFILE_GPU_BEGIN_FRAME()
#define PROFILE_GPU_END_FRAME()
#define PROFILE_ADD_GPU(color, name) if( BP_IsProfileInfoActive() ) { ProfileBars::AddRsxMarker(color, name); cell::Gcm::cellGcmFlush(); }
#define PROFILE_GPU_END(color, name)

#define PROFILE_ADD_PPU(color, name) if( BP_IsProfileInfoActive() ) ProfileBars::AddPpuMarker(color, name)
#elif defined(BP_360)
#define PROFILE_GPU_BEGIN_FRAME() ProfileBars::GPUMarkerPre()
#define PROFILE_GPU_END_FRAME() ProfileBars::GPUMarkerPost()
#define PROFILE_ADD_GPU(color, name) ProfileBars::GPUMarkerPop(color, name); ProfileBars::GPUMarkerPush(color, name)
//#define PROFILE_GPU_START(color, name) ProfileBars::GPUMarkerPush(color, name)
#define PROFILE_GPU_END(color, name) ProfileBars::GPUMarkerPop(color, name)

#define PROFILE_ADD_PPU(color, name) ProfileBars::CPUMarkerPop(color, name); ProfileBars::CPUMarkerPush(color, name)
#endif

#else
#define PROFILE_GPU_BEGIN_FRAME()
#define PROFILE_GPU_END_FRAME()
#define PROFILE_ADD_GPU(color, name)
#define PROFILE_GPU_END(color, name)

#define PROFILE_ADD_PPU(color, name)

#endif

//----------------------------------------------------------------------------

int BP_BeginShader(CCompiledShader* pShader, uint8 * pParameterBuffer, int const parameterBufferSize);
void BP_EndShader();

//----------------------------------------------------------------------------

CBaseTexture* BP_GetRenderTarget(ERenderTarget const id);
#ifndef BP_RENDER_SINGLE_RENDER_TARGET
void BP_SetMainRenderTarget(int const currentRenderBuffer);
#endif

//----------------------------------------------------------------------------

void BP_RestoreFrameInitTestValue();

//----------------------------------------------------------------------------

inline CVector4 const BP_DecodeColorF(uint64 rgba)
{
   return CVector4( ((rgba & 0x000000FF) >> 0) / 255.0f, ((rgba & 0x0000FF00) >> 8) / 255.0f, ((rgba & 0x00FF0000) >> 16) / 255.0f, ((rgba & 0xFF000000) >> 24) / 255.0f );
}

inline CColor const BP_DecodeColor(uint64 rgba)
{
   return CColor( uint8((rgba & 0x000000FF) >> 0), uint8((rgba & 0x0000FF00) >> 8), uint8((rgba & 0x00FF0000) >> 16), uint8((rgba & 0xFF000000) >> 24) );
}

enum ESharedRegister
{
   // 0..15 reserved for shared registers
   kSReg_FogColor       =  0,    //half4
   kSReg_FogParam       =  1,    //half4
   kSReg_LightDir       =  2,    //half4x4
   kSReg_LightCol       =  6,    //half4x4
   kSReg_Sampler0Info   =  10,   //half4
   kSReg_EnvmapParams   =  11,   //half4

};

//----------------------------------------------------------------------------

#if MGS_VERSION == 2
void BP_SetLinerTexture(DG_TEX_LIN* tex, int texUnit);
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if MGS_VERSION == 2 && BP_VITA
   void BP_Renderer_MGS2_3060_SetEndframeTimeUS( uint64_t const frameTime );
   void BP_Renderer_MGS2_3060_SetRenderTimeUS( uint64_t const frameTime, int hardwareCanDo60 );
   void BP_Renderer_MGS2_3060_SetGameTimeUS( uint64_t const frameTime, int gameModeAllows60 );
   void BP_Renderer_MGS2_3060_GameHeartbeat();
   void BP_Renderer_MGS2_3060_PrintDebugInfo( int *, int *);
   int BP_Renderer_MGS2_3060_IsAt60();


#endif

#ifdef __cplusplus
}
#endif

