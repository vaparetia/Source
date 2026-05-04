//----------------------------------------------------------------------------
// BP_RenderDmaPack.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"



//----------------------------------------------------------------------------
// These get set up by the functions below as hints for the rendering code actually using them

union SBlendAttributes
{
   unsigned char data;
   struct
   {
      uint32 BlendModeDrawCount : 2;
      uint32 ForceRGBWhite : 1;
      uint32 AlphaBlendEnabled : 1;
      uint32 SrcAlphaBlend : 1;
      uint32 StandardBlend : 1;
   };
};
extern SBlendAttributes gGSBlend;

#define gGS_ForceRGBWhite gGSBlend.ForceRGBWhite
#define gGS_BlendModeDrawCount gGSBlend.BlendModeDrawCount
#define gGS_AlphaBlendEnabled gGSBlend.AlphaBlendEnabled
#define gGS_SrcAlphaBlend gGSBlend.SrcAlphaBlend
#define gGS_IsStandardAlphaBlend gGSBlend.StandardBlend

//----------------------------------------------------------------------------

extern int gGS_DebugEnableBlendModeEmulation;
extern int gGS_Vita_CurrentSpecialBlend;
//extern int gGS_Vita_CurrentAlphaFunc;

static const int skGS_Vita_NumSpecialBlends = 2;

//----------------------------------------------------------------------------

#if BP_VITA
void BP_InitGS();
#endif

int BP_GS_GetCurrentVitaAlphaFunc();
void BP_GS_SetAlpha_UseLastBlendMode(int iBlendPass);
void BP_GS_SetAlpha_NormalBlend(uint64 data);
void BP_GS_SetAlpha_NoBlend(uint64 data);
void BP_GS_SetAlpha_Default(uint64 data, int iBlendPass);
void BP_GS_SetAlpha_OffscreenPrim(uint64 data, int iBlendPass);

typedef void (*TFnSetAlphaFunc)(uint64 data, int iBlendPass);
extern TFnSetAlphaFunc gCurrentAlphaFunc;

static inline void BP_GS_SetAlphaFunc(TFnSetAlphaFunc func)
{
   gCurrentAlphaFunc = func;
}

static inline void BP_GS_SetAlpha(uint64 data, int iBlendPass=0)
{
   gCurrentAlphaFunc(data, iBlendPass);
}
uint64 BP_GS_OptimizeAlpha(uint64 data, int AlwaysOne);
void BP_GS_SetClamp(int texUnit, uint64 data);

void BP_GS_SetAlphaTestFunc( int const func, int const refValue );
void BP_GS_SetAlphaTestEnabled( bool const enabled );

//----------------------------------------------------------------------------

extern int gGS_AlphaFailPassStart;
extern int gGS_AlphaFailPassCount;
struct SBP_TestState;

enum EAlphaTestPass
{
   kPass_First,

   kPass_NoDepthWrite = kPass_First,
   kPass_DepthWrite,

   kPass_Count
};

void BP_GS_SetTest(uint64 data);
void BP_GS_SetupAlphaFailPass(int currentPass);
EAlphaTestPass BP_GS_GetPassFor_SingleAlphaValue(int alphaValue);
void BP_GS_SetupAlphaFailPass_SingleAlphaValue(int alphaValue);
void BP_GS_SetupAlphaFailPass_Optimize(int currentPass, SBP_TestState* ppOutTestState);
int BP_GS_AlphaFailPass_Optimize(int minAlphaValue, int maxAlphaValue, int minNonZeroAlpha, SBP_TestState** ppOutTestState);
void BP_GS_SetupAlphaTestSinglePass(bool forceWriteDepth);
void BP_GS_SetupAlphaTestSinglePass_ForceDisableAlphaTest(bool forceWriteDepth);
