//----------------------------------------------------------------------------
// BP_Misc.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/System/COsContext.h"
#include "Engine/Resource/CResourceCache.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/System/CTaskQueue.h"

#include "MGS_Common.h"
#include "BP_SaveLoadMGS.h"
#include "BP_TrophySystem.h"

#include "CWavFile.h"

#include "libfs.h"

#if MGS_VERSION == 3
#include "libgv.h"
#include "pad.h"
#endif

#define BP_USE_EMBEDDED_RESOURCES 0

#if BP_USE_EMBEDDED_RESOURCES
#include "../GameDataSource/Assets.h"
#endif

#include "BP_BaseRenderer.h"
#include "BP_Camera.h"
#include "BP_DebugPad.h"
#include "BP_DebugPad.h"
#include "BP_FileSupport.h"
#include "BP_Font.h"
#include "BP_Misc.h"
#include "BP_RenderFX.h"
#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_SoundSupport.h"
#include "BP_SplashScreen.h"
#include "BP_RichPresenceSystem.h"

#if BP_PS3
#include <cell/sysmodule.h>
#include <cell/rtc.h>
#include <sysutil/sysutil_common.h>
#include <sys/process.h>

extern "C" void ShowLowDiskSpaceWarning(int *pResult, int kbNeeded);

#else
#include <time.h>
#endif

#if BP_VITA
// sceAppMgrLoadExe (exitspawn)
#include <appmgr.h>
#include <rtc.h>
#endif

extern "C" void ShowExitGameWarning(int *pResult);
extern "C" int BP_IsDebugPaused();
extern "C"
{
#if MGS_VERSION == 2
#include "../../mgs2x/source/user/mode/demo/libdemo.h"
#endif

#if MGS_VERSION == 3
#include "../../source/user/mode/demo/demo.h"
#endif
};

#include "boost/optional.hpp"
#include "BP_CommonDialog.h"

extern "C" void BP_EndFrame();

extern "C" int gBP_UseVitaControlOverrides;

static unsigned long long sRestartDialogResult = 0;

//----------------------------------------------------------------------------
// INPUT
//----------------------------------------------------------------------------

int BP_Pad_IsJapanese()
{
   return( gpOsContext->mControllerRegion == CBaseOsContext::kCR_Japan );
}

int BP_Pad_GetOkAssignment()
{
   if( gpOsContext->mControllerRegion == CBaseOsContext::kCR_USA )
   {
      // PS3 == X / X360 == A
      return BP_PAD_B;
   }
   else
   {
      // PS3 == Circle / X360 == B
      return BP_PAD_A;
   }
}

int BP_Pad_GetSubJctCameraAssignment()
{
   switch (gBP_UseVitaControlOverrides)
   {
   case BP_VITA_CONTROL_SCHEME_ORIG:
   case BP_VITA_CONTROL_SCHEME_V2:
      return BP_PAD_R1;

   case BP_VITA_CONTROL_SCHEME_V1:
      return BP_PAD_L1;
   }

   return 0;
}

int BP_Pad_GetLockOnAssignment()
{
   if (gBP_UseVitaControlOverrides) // both v1 and v2 use the same scheme
   {
#if MGS_VERSION == 3
      // Although the Vita doesn't have this button, we hijack this bit and use it to detect lock on gestures which is 
      // done by double tapping the L1 shoulder button. When a double tap is detected on the L1 button this bit is set.
      // The regular L1 shoulder button is then used for slow movement.
      return BP_PAD_L2; 
#else
      return BP_PAD_L1;
#endif
   }
   else
   {
      return BP_PAD_L1;
   }
}

int BP_Pad_GetShootAssignment()
{
   switch (gBP_UseVitaControlOverrides)
   {
   case BP_VITA_CONTROL_SCHEME_ORIG:
   case BP_VITA_CONTROL_SCHEME_V2:
      return BP_PAD_SQUARE;

   case BP_VITA_CONTROL_SCHEME_V1:
      return BP_PAD_R1;
   }
   
   return 0;
}

int BP_Pad_GetSlowMovementAssignment()
{
#if MGS_VERSION == 3
   switch (gBP_UseVitaControlOverrides)
   {
   case BP_VITA_CONTROL_SCHEME_V1:
      return BP_PAD_SQUARE;

   case BP_VITA_CONTROL_SCHEME_V2:
      return BP_PAD_L1;
      break;
   }
#else
   switch (gBP_UseVitaControlOverrides)
   {
   case BP_VITA_CONTROL_SCHEME_V1:
      return BP_PAD_SQUARE;

   case BP_VITA_CONTROL_SCHEME_V2:
      return BP_PAD_L1;
      break;
   }
#endif

   return 0;
}

#if MGS_VERSION == 2
int BP_Pad_GetShootPressAssignment()
{
   switch (gBP_UseVitaControlOverrides)
   {
   case BP_VITA_CONTROL_SCHEME_ORIG:
   case BP_VITA_CONTROL_SCHEME_V2:
      return PAD_PRESS_Y;
      break;

   case BP_VITA_CONTROL_SCHEME_V1:
      return BP_PAD_R1;
      break;
   }

   return 0;
}
#elif MGS_VERSION == 3
int BP_Pad_GetCameraSwitchAssignment()
{
   if (gBP_UseVitaControlOverrides) // Both v1 and v2 use dpad left
   {
      return PAD_DIGITAL_LEFT;
   }
   else
   {
      return _PAD_R3;
   }
}
#endif

//----------------------------------------------------------------------------

int BP_Pad_GetCancelAssignment()
{
   if( gpOsContext->mControllerRegion == CBaseOsContext::kCR_USA )
   {
      // PS3 == Circle / X360 == B
      return BP_PAD_A;
   }
   else
   {
      // PS3 == X / X360 == A
      return BP_PAD_B;
   }
}

//----------------------------------------------------------------------------

int BP_GetMGSNameKeyboardKeystroke( char * const pC )
{
#if BPE_TARGET == BPE_TARGET_X360
   XINPUT_KEYSTROKE keystroke;
   DWORD ret = XInputGetKeystroke( XUSER_INDEX_ANY, XINPUT_FLAG_KEYBOARD, &keystroke );

   if( ret == ERROR_SUCCESS )
   {
      if( keystroke.Flags & (XINPUT_KEYSTROKE_ALT|XINPUT_KEYSTROKE_CTRL) )
      {
         //Ignore CTRL- and ALT- keystrokes.
         return -1;
      }
      if( keystroke.Flags & (XINPUT_KEYSTROKE_KEYDOWN|XINPUT_KEYSTROKE_REPEAT) )
      {
         if( keystroke.Unicode > 0 && keystroke.Unicode < 0x7f )
         {
            char c = (char)keystroke.Unicode;
            //Only transformation necessary is to convert to all caps.
            if( c >= 'a' && c <= 'z' )
            {
               c = 'A' + (c-'a');
            }
            if( strchr( BP_VALID_MGS_NAME_CHARACTERS, c ) )
            {
               *pC = c;
               return 0;
            }
            if( c == 0x08 || c == 0x0a )
            {
               //0x08 is BS
               //0x0A is LF
               *pC = c;
               return 0;
            }
         }
         else if( keystroke.VirtualKey >= VK_NUMPAD0 && keystroke.VirtualKey <= VK_NUMPAD9 )
         {
            *pC = '0' + (keystroke.VirtualKey - VK_NUMPAD0);
            return 0;
         }
         else if( keystroke.VirtualKey == VK_DIVIDE )
         {
            *pC = '/';
            return 0;
         }
         else if( keystroke.VirtualKey == VK_SEPARATOR ) //keypad enter
         {
            *pC = 0x0a;
            return 0;
         }
         else if( keystroke.VirtualKey == VK_DECIMAL )
         {
            *pC = '.';
            return 0;
         }
      }
   }
   return -1;
#else
   //No keyboard support needed on other platforms.
   return -1;
#endif
}

//----------------------------------------------------------------------------
// MISC
//----------------------------------------------------------------------------

void BP_GetLocalTime(int* year, int * month, int* day, int* hours, int* minutes, int * seconds)
{
#if BP_PS3
   if (!cellSysmoduleIsLoaded(CELL_SYSMODULE_RTC))
      cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);

   CellRtcDateTime clk;
   cellRtcGetCurrentClockLocalTime(&clk);

   if( year )
      *year = clk.year;       // 1-9999
   if( month )
      *month = clk.month;     // 1-12
   if( day )
      *day = clk.day;         // 1-31
   if( hours )
      *hours = clk.hour;     // 0-23
   if( minutes )
      *minutes = clk.minute;  // 0-59
   if(seconds)
      *seconds = clk.second;  // 0-59
#else
#if BP_VITA
   SceDateTime clk;
   sceRtcGetCurrentClockLocalTime(&clk);
   
   if( year )
      *year = clk.year;       // 1-9999
   if( month )
      *month = clk.month;     // 1-12
   if( day )
      *day = clk.day;         // 1-31
   if( hours )
      *hours = clk.hour;     // 0-23
   if( minutes )
      *minutes = clk.minute;  // 0-59
   if(seconds)
      *seconds = clk.second;  // 0-59
#else
   time_t rawtime;
   struct tm *timeinfo;
   time(&rawtime);
   timeinfo = localtime(&rawtime);

   if( year )
      *year = timeinfo->tm_year + 1900;   // years since 1900
   if( month )
      *month = timeinfo->tm_mon + 1;      // 0-11
   if( day )
      *day = timeinfo->tm_mday;           // 1-31
   if( hours )
      *hours = timeinfo->tm_hour;         // 0-23
   if( minutes )
      *minutes = timeinfo->tm_min;        // 0-59
   if(seconds)
      *seconds = timeinfo->tm_sec;        // 0-69
#endif
#endif
}

//----------------------------------------------------------------------------

namespace
{
   CStopWatch gAppWatch;
   int gAppTimeDebugCounter = 0;
   real32 gLastAppTime;
}

void BP_MiscGlobalAppTimeDebug(const char* pInfo)
{
   if( gAppTimeDebugCounter == 0 )
   {
      gAppWatch.Reset();
      gLastAppTime = 0.0f;
   }

   ++gAppTimeDebugCounter;

   real32 const currentAppTime = gAppWatch.GetElapsedTime();
   printf("BP-APPTIME[%s]: %f seconds, time since last: %f seconds\n", pInfo, currentAppTime, currentAppTime - gLastAppTime);
   gLastAppTime = currentAppTime;
}

extern CTaskQueue* gpTaskQueue_Render;

int BP_CanLoadNewTexture()
{
#if BPE_TARGET == BPE_TARGET_X360

   // If we're not currently in the splash screen we let the original code logic deal with stalling whenever too many textures are being loaded simultaneously.
   if( BP_Splash_IsDone() )
      return 1;

   // If we are inside the splash screen and there are currently already PTC decompression jobs in progress, we don't risk possibly stalling the main thread.
   // This prevents hitches during the splash screen sequence.
   if( gpTaskQueue_Render->GetCurrentNumberOfTasks() > 0 )
      return 0;
#endif

   return 1;
}

#ifdef BP_360
int BP_GetDefaultYAxisFlipVaraible()
{
   COsContext* pOsContext = (COsContext*)gpOsContext;
   return pOsContext->IsYAxisInverted();
}
#endif

//----------------------------------------------------------------------------
// FPS
//----------------------------------------------------------------------------

#if !BP_VITA
int BP_IsPAL(void)
{
   if ( RenderBackend()->GetScreenRefreshRate() == 50 )
      return TRUE;
   else
      return FALSE;
}

//----------------------------------------------------------------------------

// Use for linear equations such as: Var = Var + Value
static inline
float BP_FPS_AdjustFloat_Mult( const float value, const float originalRefreshRate )
{
   const float screenRefreshRate = RenderBackend()->GetScreenRefreshRate();
   const float mult              = originalRefreshRate / screenRefreshRate;
   return ( value * mult );
}

//----------------------------------------------------------------------------

// Use for non linear feedback equations such as: Var = Var * Value
static inline
float BP_FPS_AdjustFloat_Pow( const float value, const float originalRefreshRate )
{
   const float screenRefreshRate = RenderBackend()->GetScreenRefreshRate();
   const float mult              = originalRefreshRate / screenRefreshRate;
   return powf( value, mult );
}

//----------------------------------------------------------------------------

static inline
int BP_FPS_AdjustInt_Mult( const int value, const float originalRefreshRate )
{
   const float f = BP_FPS_AdjustFloat_Mult( (float)value, originalRefreshRate );
   const int   i = (int)( f + 0.5f );
   return i;
}

//----------------------------------------------------------------------------

static inline
int BP_FPS_AdjustInt_Pow( const int value, const float originalRefreshRate )
{
   const float f = BP_FPS_AdjustFloat_Pow( (float)value, originalRefreshRate );
   const int   i = (int)( f + 0.5f );
   return i;
}

//----------------------------------------------------------------------------

// Use for linear equations such as: Frame++, if Frame > ?
static inline
int BP_FPS_AdjustInt_Frames( const int value, const float originalRefreshRate )
{
   const float screenRefreshRate = RenderBackend()->GetScreenRefreshRate();
   const float mult              = screenRefreshRate / originalRefreshRate;
   const float f                 = ( value * mult );
   const int   i = (int)( f + 0.5f );
   return i;
}

//----------------------------------------------------------------------------

float BP_FPS_AdjustFloat_Mult_30( const float value )
{
   return BP_FPS_AdjustFloat_Mult( value, 30.0f );
}

//----------------------------------------------------------------------------

float BP_FPS_AdjustFloat_Pow_30( const float value )
{
   return BP_FPS_AdjustFloat_Pow( value, 30.0f );
}

//----------------------------------------------------------------------------

int BP_FPS_AdjustInt_Mult_30( const int value )
{
   return BP_FPS_AdjustInt_Mult( value, 30.0f );
}

//----------------------------------------------------------------------------

int BP_FPS_AdjustInt_Pow_30( const int value )
{
   return BP_FPS_AdjustInt_Pow( value, 30.0f );
}

//----------------------------------------------------------------------------

int BP_FPS_AdjustInt_Frames_30( const int value )
{
   return BP_FPS_AdjustInt_Frames( value, 30.0f );
}

#endif

//----------------------------------------------------------------------------
// TGS DEMO
//----------------------------------------------------------------------------

#if BP_TGS_DEMO()

enum EBPTGSState
{
   kTGS_DEMO_State_Game,
   kTGS_DEMO_State_EndIn,
   kTGS_DEMO_State_EndIdle,
   kTGS_DEMO_State_EndOut,
   kTGS_DEMO_State_EndTitleIn,
};

#define TGS_DEMO_REBOOT_PAD   (BP_PAD_L1 | BP_PAD_L2 | BP_PAD_R1 | BP_PAD_R2 | BP_PAD_START | BP_PAD_SELECT)
#define TGS_DEMO_REBOOT_TIME  2.0f

int   gBP_TGS_DEMO_State = kTGS_DEMO_State_Game;
float gBP_TGS_DEMO_Time  = 0.0f;
float gBP_TGS_DEMO_Alpha = 0.0f;

float gBP_TGS_DEMO_EndIn_Time   = 1.0f;
float gBP_TGS_DEMO_EndIdle_Time = 3.0f;
float gBP_TGS_DEMO_EndOut_Time  = 1.0f;

float gBP_TGS_DEMO_Reboot_Time  = 0.0f;

TResource<CBaseTexture>* gBP_TGS_DEMO_pEndTexture = NULL;

#define BP_TGS_DEMO_JINGLE

static CWavFile * spTGSWavFile = NULL;

void BP_TGS_DEMO_Init()
{
#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3
   {
      CResId const resId("$/misc/tgs_demo/****/tgs_end.ctxr");

      unsigned long textureIdx = ast_open_file_index(resId.c_str());

      unsigned char const * pTextureDataMemory = ast_get_file_data(textureIdx);
      unsigned int textureDataSize = ast_get_file_length(textureIdx);
      CBaseTexture* pTexture = CBaseTexture::Create((void*)pTextureDataMemory, textureDataSize);
      ast_close_file_index(textureIdx);

      CResource resource(new CResourceReference(pTexture, gpResources, resId));
      gpResources->ManuallyAddToResourceMap(resId, resource);
      gpResources->ResourceCache()->CacheResource(resource, resId);

      gBP_TGS_DEMO_pEndTexture = new TResource<CBaseTexture>( gpResources->GetResource( resId ) );
   }
#endif
}

void BP_TGS_DEMO_JumpToTitleScreen()
{
#if MGS_VERSION == 2
   // Jump to title screen
   GM_SetArea( GM_SaveArea, "n_title" ) ;
   GM_ChangeResidentArea( "r_title" ) ;
   GCL_ChangeSenerioCode( 1574688 ) ;	/* scenerio.gcx */
   GM_Result = 9999 ;
   GM_LoadRequest = GM_REQ_DEBUG_RESET | 0x1 ;
#endif

#if MGS_VERSION == 3
   // Jump to title screen
   GM_SetArea( GM_SaveArea, "title" );
   GM_SetLoadRequest( GM_REQ_DEBUG_RESET | GM_REQ_LOAD );
   GM_Result = 9999;
#endif
}

void BP_TGS_DEMO_End( int fade )
{
   // Set state
   if( fade )
   {
      // Fade in
      gBP_TGS_DEMO_Alpha = 0.0f;
      gBP_TGS_DEMO_Time  = 0.0f;
      gBP_TGS_DEMO_State = kTGS_DEMO_State_EndIn;

#ifdef BP_TGS_DEMO_JINGLE
      if( !spTGSWavFile )
      {
         printf("CONSTRUCT TGS JINGLE\n");
         spTGSWavFile = new CWavFile();
#if MGS_VERSION==2
         if( BP_Area_JP() )
         {
            spTGSWavFile->Load( "misc/mgs2j_vc050501.wav" );
            gBP_TGS_DEMO_EndIdle_Time = 25.f;
         }
         else
         {
            //"Euro Game Expo" version
            spTGSWavFile->Load( "misc/mgs2e_vc030120.wav" );
            gBP_TGS_DEMO_EndIdle_Time = 8.5f;
         }
#else
         spTGSWavFile->Load( "misc/mgs3_to_be_continued.wav" );
         gBP_TGS_DEMO_EndIdle_Time = 16.5f;
#endif
      }

      spTGSWavFile->PlayOneShot();
      spTGSWavFile->SetVolume(1.f);
#endif
   }
   else
   {
      // Skip fade in and jump straight to end to avoid fading in over gray background
      gBP_TGS_DEMO_Alpha = 1.0f;
      gBP_TGS_DEMO_Time  = 0.0f;
      gBP_TGS_DEMO_State = kTGS_DEMO_State_EndIdle;
      BP_TGS_DEMO_JumpToTitleScreen();
   }
}

int BP_TGS_DEMO_CutsceneBegin()
{
#if MGS_VERSION == 2
   // Start of olga death cinema?
   if(      ( strcmp( gBP_Demo_StreamName, "t06a1D.sdt" ) == 0 )     // 1st cinema as soon as Olga is dead
         || ( strcmp( gBP_Demo_StreamName, "t06a2D.sdt" ) == 0 ) )   // 2nd cinema of Olga dead (games jumps straight to this if holding buttons after killing olga)
   {
      GM_SdSet( SNG_FOUTS_SS );  // Fade out and stop music
      GM_SdSet( SE_ALL_STOP );   // Stop all sound effects
      BP_TGS_DEMO_End( 1 );
      return 1;
   }
#endif

#if MGS_VERSION == 3
   // Meet Sokolov cinema?
   if( strcmp( gBP_Demo_StreamName, "v080_010_p010.sdt" ) == 0 )
   {
      BP_TGS_DEMO_End( 1 );
      return 1;
   }
#endif

   return 0;
}

void BP_TGS_DEMO_CutsceneEnd()
{
}

void BP_TGS_DEMO_CutscenePlaying()
{
}

int BP_TGS_DEMO_IsLoadComplete()
{
#if MGS_VERSION == 2
   return GM_LoadComplete;
#endif

#if MGS_VERSION == 3
   return GM_IsLoadComplete();
#endif
}

void BP_TGS_DEMO_Update()
{
   float deltaTime = 1.0f / RenderBackend()->GetScreenRefreshRate();

   switch( gBP_TGS_DEMO_State )
   {
      case kTGS_DEMO_State_Game:
         {
            gBP_TGS_DEMO_Alpha = 0.0f;
            gBP_TGS_DEMO_Time  = 0.0f;
         }
         break;

      case kTGS_DEMO_State_EndIn:
         {
            gBP_TGS_DEMO_Time += deltaTime;
            if( gBP_TGS_DEMO_Time >= gBP_TGS_DEMO_EndIn_Time )
            {
               gBP_TGS_DEMO_Alpha = 1.0f;
               gBP_TGS_DEMO_Time  = 0.0f;
               gBP_TGS_DEMO_State = kTGS_DEMO_State_EndIdle;
               BP_TGS_DEMO_JumpToTitleScreen();
            }
            else
            {
               gBP_TGS_DEMO_Alpha = gBP_TGS_DEMO_Time / gBP_TGS_DEMO_EndIn_Time;
            }
         }
         break;

      case kTGS_DEMO_State_EndIdle:
         {
            gBP_TGS_DEMO_Time += deltaTime;
#if MGS_VERSION == 3
            int userQuit = ( GV_PadDataDirect(0)->press & BP_Pad_GetOkAssignment() ) || ( GV_PadDataDirect(0)->press & BP_Pad_GetCancelAssignment() );
#else
            int userQuit = ( GV_PadDataDirect[0].press & BP_Pad_GetOkAssignment() ) || ( GV_PadDataDirect[0].press & BP_Pad_GetCancelAssignment() );
#endif
            if( ( ( gBP_TGS_DEMO_Time >= gBP_TGS_DEMO_EndIdle_Time ) || userQuit ) && ( BP_TGS_DEMO_IsLoadComplete() ) )
            {
               gBP_TGS_DEMO_Alpha = 0.0f;
               gBP_TGS_DEMO_Time  = 0.0f;
               gBP_TGS_DEMO_State = kTGS_DEMO_State_EndOut;
            }
         }
         break;

      case kTGS_DEMO_State_EndOut:
         {
            gBP_TGS_DEMO_Time += deltaTime;
            if( gBP_TGS_DEMO_Time >= gBP_TGS_DEMO_EndOut_Time )
            {
               gBP_TGS_DEMO_Alpha = 0.0f;
               gBP_TGS_DEMO_Time  = 0.0f;
               gBP_TGS_DEMO_State = kTGS_DEMO_State_EndTitleIn;
#ifdef BP_TGS_DEMO_JINGLE
               delete spTGSWavFile;
               spTGSWavFile = NULL;
               printf("DESTRUCT TGS JINGLE\n");
#endif
            }
            else
            {
               gBP_TGS_DEMO_Alpha = gBP_TGS_DEMO_Time / gBP_TGS_DEMO_EndOut_Time;
#ifdef BP_TGS_DEMO_JINGLE
               const float vol = 1.f - gBP_TGS_DEMO_Alpha;
               printf("TGS JINGLE VOLUME OUT %f\n", vol );
               spTGSWavFile->SetVolume(vol);
#endif
            }
         }
         break;

      case kTGS_DEMO_State_EndTitleIn:
         {
            gBP_TGS_DEMO_Time += deltaTime;
            if( gBP_TGS_DEMO_Time >= gBP_TGS_DEMO_EndOut_Time )
            {
               gBP_TGS_DEMO_Alpha = 0.0f;
               gBP_TGS_DEMO_Time  = 0.0f;
               gBP_TGS_DEMO_State = kTGS_DEMO_State_Game;
            }
            else
            {
               gBP_TGS_DEMO_Alpha = gBP_TGS_DEMO_Time / gBP_TGS_DEMO_EndOut_Time;
            }
         }
         break;
   }

   // Reboot?
   if( BP_DebugPad_Status( 0, TGS_DEMO_REBOOT_PAD ) == TGS_DEMO_REBOOT_PAD )
   {
      gBP_TGS_DEMO_Reboot_Time += deltaTime;
      if( gBP_TGS_DEMO_Reboot_Time >= TGS_DEMO_REBOOT_TIME )
      {
         BP_TGS_DEMO_Reboot();
      }
   }
   else
   {
      gBP_TGS_DEMO_Reboot_Time = 0.0f;
   }
}

void BP_TGS_DEMO_Render()
{
   // Not visible?
   if( ( gBP_TGS_DEMO_State == kTGS_DEMO_State_Game ) || ( gBP_TGS_DEMO_pEndTexture == NULL ) )
   {
      return;
   }

   // Set render modes
   gpRenderBackend->SetAlphaTestEnable(false);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);

   gpRenderBackend->SetDepthCompareEnabled(false);

   // Compute color
   float c = 0.0f;
   float a = 0.0f;
   switch( gBP_TGS_DEMO_State )
   {
   case kTGS_DEMO_State_EndIn:
      {
         // Opaque fade in from black
         c = gBP_TGS_DEMO_Alpha;
         a = 1.0f;
      }
      break;

   case kTGS_DEMO_State_EndIdle:
      {
         // Opaque
         c = 1.0f;
         a = 1.0f;
      }
      break;

   case kTGS_DEMO_State_EndOut:
      {
         // Opaque fade to black
         c = 1.0f - gBP_TGS_DEMO_Alpha;
         a = 1.0f;
      }
      break;

   case kTGS_DEMO_State_EndTitleIn:
      {
         // Alpha fade out black
         c = 0.0f;
         a = 1.0f - gBP_TGS_DEMO_Alpha;
      }
      break;
   }

   // Render quad
   BP_DrawFullscreenTextureModulate(gBP_TGS_DEMO_pEndTexture->Ptr(), CVector4( c, c, c, a ), 0 /*isFilter*/, 0 /*isRGBA*/);
}

//----------------------------------------------------------------------------

void BP_TGS_DEMO_Reboot()
{
   gpOsContext->mShouldTerminateApplication = true;
   gpOsContext->mShutdownType = COsContext::kST_ReturnToBootloader;
}

//----------------------------------------------------------------------------

int  BP_TGS_DEMO_IsShowingEndScreen()
{
   return( gBP_TGS_DEMO_State != kTGS_DEMO_State_Game );
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------

float gBP_SoftReset_Reboot_Time  = 0.0f;
#if BPE_TARGET == BPE_TARGET_VITA
#define SOFTRESET_REBOOT_PAD   (BP_PAD_L1 | BP_PAD_R1 | BP_PAD_CIRCLE | BP_PAD_SQUARE | BP_PAD_CROSS )
#else
#define SOFTRESET_REBOOT_PAD   (BP_PAD_L1 | BP_PAD_L2 | BP_PAD_R1 | BP_PAD_R2 | BP_PAD_START | BP_PAD_SELECT)
#endif
#define SOFTRESET_REBOOT_PAD_REBOOT_TIME  2.0f

void BP_CheckForSoftReset()
{
   float deltaTime = 1.0f / RenderBackend()->GetScreenRefreshRate();

   // Reboot?
   if( BP_DebugPad_Status( 0, SOFTRESET_REBOOT_PAD ) == SOFTRESET_REBOOT_PAD )
   {
      gBP_SoftReset_Reboot_Time += deltaTime;
      if( gBP_SoftReset_Reboot_Time >= SOFTRESET_REBOOT_PAD_REBOOT_TIME )
      {
         BP_SoftResetReboot();
      }
   }
   else
   {
      gBP_SoftReset_Reboot_Time = 0.0f;
   }
}


//----------------------------------------------------------------------------

void BP_SoftResetReboot()
{
   int confirmRet;
   ShowExitGameWarning( &confirmRet );

   sRestartDialogResult = 0;

   if( confirmRet == 0 )   //yes
   {
      gpOsContext->mShouldTerminateApplication = true;
      gpOsContext->mShutdownType = COsContext::kST_RestartGameSession;
   }
}

void BP_RelaunchTheBootLoaderFromUI()
{
	gpOsContext->mShouldTerminateApplication = true;
   gpOsContext->mShutdownType = COsContext::kST_ReturnToBootloader;
}

void BP_RelaunchTheGameFromUI()
{
   gpOsContext->mShouldTerminateApplication = true;
   gpOsContext->mShutdownType = COsContext::kST_RestartGameSession;

#if BPE_TARGET == BPE_TARGET_VITA
   if ( !sRestartDialogResult )
   {
      sRestartDialogResult = BP_CommonDialog_WantsMessageDialog( BP_GetCustomOverrideString( "SAVELOAD", "RESTARTGAME_CONFIRM" ), kMDL_YesNo );
      printf( "BP_RelaunchTheGameFromUI: %d\n", uint32( sRestartDialogResult ) );
   }
#endif
}

void BP_RelaunchTheGameFromUINoPrompt()
{
   gpOsContext->mShouldTerminateApplication = true;
   gpOsContext->mShutdownType = COsContext::kST_RestartGameSession;
}

// PS3/X360 System
//----------------------------------------------------------------------------

void BP_HandleInsufficientHDD()
{
#if BPE_TARGET == BPE_TARGET_PS3
   if( BP_TrophySystem_IsInsufficientHDDSpaceForApp() )
   {
      //We cannot continue.
      printf("BP_HandleInsufficientHDD(): add'l space required: %d KB.\n", MGS_SaveStatus_ExtraSpaceReq() );
      ShowLowDiskSpaceWarning(NULL, MGS_SaveStatus_ExtraSpaceReq());  // BANG!  Exit the game, this is the first try to read and we're out of space.
   }
#endif
}

extern "C"
{
   char* gLaunchDataX360 = NULL;
}
void BP_ShutdownCheck()
{
   // Lookup app status since OS context is destroyed by BP_ShutdownSystems()
   const bool isSafeToShutDown = gpOsContext->IsSafeToShutDown();
   const bool terminateApp     = gpOsContext->mShouldTerminateApplication;
   COsContext::EShutdownType const shutdownType =
                                 gpOsContext->mShutdownType;

#if BPE_TARGET == BPE_TARGET_VITA
   // I hate this if. It breaks like, every Armature coding standard.
   if ( sRestartDialogResult != 0 )
   {
      if ( BP_CommonDialog_IsStillProcessing( sRestartDialogResult ) ) 
      {
         // Waiting for more

         return;
      }
      else if ( BP_CommonDialog_GetResult( sRestartDialogResult ) == kCDR_OK )
      {
         sRestartDialogResult = 0;
      }
      else
      {
         // Cancel pressed, so cancel
         sRestartDialogResult = 0;
         gpOsContext->mShouldTerminateApplication = false;
         return;
      }
   }
#endif

#if BPE_TARGET == BPE_TARGET_PS3

   // make it possible to kill the app w/o actually shutting down cleanly.
   if( terminateApp )
   {
      if( isSafeToShutDown )  // wait for the save game and trophy system to finish saving first
      {
         printf("BP - SHUTTING DOWN!\n");

         // Shut down bp systems
         BP_ShutdownSystems();

         // Reboot?
         switch ( shutdownType )
         {
         case COsContext::kST_RestartGameSession:
            {
               // Restart the game.
#if MGS_VERSION == 3
               char *pSELF = "MGS3.self";
#elif MGS_VERSION == 2
               char *pSELF = "MGS2.self";
#else
#  error Unknown version!
#endif
               printf("Restart Game Session - Launching Executable: %s\n", pSELF);
               OsContext()->_Exitspawn(pSELF, NULL);
            }
            break;

         case COsContext::kST_ReturnToBootloader:
            {
               // Exit to boot loader
               char *pSELF = NULL;
               if (OsContext()->mBootType == COsContext::kBT_Disc)
               {
                  // Use EBOOT.BIN on disc
                  pSELF = "EBOOT.BIN";
               }
               else
               {
                  // Use BOOTLOADER.self on HDD (cannot use EBOOT.BIN on HDD as per SDK docs)
                  pSELF = "BOOTLOADER.self";
               }
               printf("Exit to Bootloader: Launching Executable: %s\n", pSELF);
               OsContext()->_Exitspawn(pSELF, NULL);
            }
            break;

         case COsContext::kST_LaunchPWForInvite:
            {
               // Launch MGS:PW with invite message
               char *pSELF = "MGS_PW.self";               
               char * argv[2] = { "BOOT_LAUNCHER_INVITATION", NULL };
               printf("In-game invite - Launching Executable: %s  Args: %s\n", pSELF, argv[0]);
               OsContext()->_Exitspawn(pSELF, argv);
            }
            break;

         case COsContext::kST_StandardShutdown:
         default:
            // Exit to XMB
            printf("Exiting to XMB\n");
            _Exit(0);
            break;
         }

      }
      else
      {
         printf("BP - WAITING TO SHUT DOWN...\n");
      }
   }

#endif


#if BPE_TARGET == BPE_TARGET_X360

   // make it possible to kill the app w/o actually shutting down cleanly.
   if( terminateApp )
   {
      if( isSafeToShutDown )  // wait for the save game and trophy system to finish saving first
      {
         printf("BP - SHUTTING DOWN!\n");

         // Shut down bp systems
         BP_ShutdownSystems();

         switch ( shutdownType )
         {
         case COsContext::kST_RestartGameSession:
            {
               // Restart the game.
#if MGS_VERSION == 3
               char * exePath = "MGS3.xex";
#elif MGS_VERSION == 2
               char * exePath = "MGS2.xex";
#else
#  error Unknown version!
#endif
               printf("Restart Game Session - Launching Executable: %s\n", exePath);
               if( gLaunchDataX360 )
               {
                  XSetLaunchData( gLaunchDataX360, strlen(gLaunchDataX360) );
               }
               XLaunchNewImage( exePath, XLAUNCH_FLAG_MATCH_MIN_VERSION );
            }
            break;
         case COsContext::kST_ReturnToBootloader:
            {
               // Exit to boot loader
               char * exePath = "default.xex";
               printf("Exit to Bootloader: Launching Executable: %s\n", exePath);
               XLaunchNewImage( exePath, XLAUNCH_FLAG_MATCH_MIN_VERSION );
            }
            break;
         case COsContext::kST_StandardShutdown:
         default:
            {
               // Exit to dash board
               printf("Exiting to dashboard\n");
               XLaunchNewImage( XLAUNCH_KEYWORD_DASH, 0 );
            }
            break;
         }
      }
      else
      {
         printf("BP - WAITING TO SHUT DOWN...\n");
      }
   }

#endif

#if BPE_TARGET == BPE_TARGET_VITA

   // make it possible to kill the app w/o actually shutting down cleanly.
   if( terminateApp )
   {
      if( isSafeToShutDown )  // wait for the save game and trophy system to finish saving first
      {
         int ret = 0;
         printf("BP - sceAppMgrLoadExec termination occuring.\n");
         printf("WARNING: sceAppMgrLoadExec doesn't work when debugging!\n");

         // AndyO: BP system shutdown disabled on Vita. 
         // Believe clean shutdown not required on Vita as OS can terminate w/out warning, but not checked TRCs.
         //BP_ShutdownSystems();

         switch ( shutdownType )
         {
         case COsContext::kST_RestartGameSession:
            {
               // Restart the game
#if MGS_VERSION == 3
               char * exePath = "app0:MGS3.self";               
#elif MGS_VERSION == 2
               char * exePath = "app0:MGS2.self";
#else
#  error Unknown version!
#endif
               printf("Restart Game Session - Launching Executable: %s\n", exePath);
               char *const argv[] = {NULL};
               ret = sceAppMgrLoadExec(exePath, argv, NULL);
            }
            break;
         case COsContext::kST_ReturnToBootloader:
            {
               // Exit to boot loader
               char * exePath = "app0:eboot.bin";
               printf("Exit to Bootloader: Launching Executable: %s\n", exePath);
               char *const argv[] = {NULL};
               ret = sceAppMgrLoadExec(exePath, argv, NULL);
            }
            break;
         case COsContext::kST_StandardShutdown:
         default:
            {
               // Exit to launcher?
               printf("Exit (kST_StandardShutdown), not allowed on PSP2!\n");
            }
            break;
         }
         printf("ERROR: sceAppMgrLoadExec(0x%08x) did not terminate.\n", ret);
      }
      else
      {
         printf("BP - WAITING TO SHUT DOWN...\n");
      }
   }

#endif
}

//----------------------------------------------------------------------------

int BP_sys_prx_load_module(char const * pRelativePRXPath)
{
#if BPE_TARGET == BPE_TARGET_PS3
   int const sprxId = OsContext()->sys_prx_load_module(pRelativePRXPath);
   return sprxId;
#else
   // Not supported
   BP_BREAK;
   return -1;
#endif
}

//----------------------------------------------------------------------------

#include "Engine/Mechanics/CCRC.h"

unsigned const char* BP_LookupStringOveride(uint32 const hash);

extern "C" char* BP_GetOverrideString(char* inputString)
{
   if( inputString != NULL )
   {
      uint32 const hash = CCRC::CalculateCRC32AsString(inputString);
      
      unsigned const char* overrideString = BP_LookupStringOveride(hash);
      if( overrideString )
         return (char*)overrideString;
   }

   return inputString;
}

//----------------------------------------------------------------------------


static char const *get_language_string()
{
   switch ( gpOsContext->mLanguage )
   {
   case COsContext::kL_Italian:
      return "IT";
   case COsContext::kL_Spanish:
      return "SP";
   case COsContext::kL_Japanese:
      return "JP";
   case COsContext::kL_German:
      return "GR";
   case COsContext::kL_French:
      return "FR";
   default:
      return "EN";
   }
}

//----------------------------------------------------------------------------

extern "C" char const *BP_GetCustomOverrideString( char const *subsystem, char const *component )
{
   char lookup[ 256 ];

   sprintf( lookup, "%s_%s_%s", subsystem, get_language_string(), component );
   char *override = BP_GetOverrideString( lookup );

   if ( override == lookup )
   {
      // If it returns our lookup string, return the component, 
      // since we're just returning a pointer and lookup is on the 
      // stack

      return component;
   }
   else
   {
      return override;
   }
}

//----------------------------------------------------------------------------

namespace Loading
{
#if BPE_TARGET == BPE_TARGET_X360
   real32 const kLoadingSpinnerDefaultShowTime = 4.5f;
#elif BP_VITA
   real32 const kLoadingSpinnerDefaultShowTime = 8.0f;
#else
   real32 const kLoadingSpinnerDefaultShowTime = 14.0f;
#endif

   real32 const kCoolDownTime = 1.0f;
   real32 const kSpinnerFrameRate = 60.0f;
   real32 const kSpinnerAlphaFadeTime = 0.5f;
   real32 const kSpinnerAlphaFadeSpeed = 1.0f / (kSpinnerAlphaFadeTime * kSpinnerFrameRate);
   real32 const kSpinnerMinimumFadeInTime = 0.25f;

   enum EState
   {
      kState_Idle,
      kState_PreFadeIn,
      kState_EnterFadeIn,
      kState_FadingIn,
      kState_FadingInCoolDown,
      kState_FadingOut,
      kState_BeginCoolDown,
      kState_CoolDown,
   };
   
   EState gCurrentState = kState_FadingIn;

   int gIsLoading = kLoadFlag_Stage;
   
   CStopWatch gSpinnerAnimationTime;

   real32 gSpinnerAlpha = 1.0f;

   CStopWatch gLoadingTimer;
   CStopWatch gCoolDownTimer;
   CStopWatch gFadeInCurrentTime;

   CBaseTexture const * gpTexture = NULL;
   boost::optional< TLockedResource<CBaseTexture> > gTextureOwner;
};

int gBP_LoadingSpinnerDebug = 0;

#include "BP_RendererDebug.h"

#if MARCO && !GOLD_VERSION
#define BP_SPINNER_PRINT(...) BP_DebugText_Print(__VA_ARGS__)
#else
#define BP_SPINNER_PRINT(...)
#endif

void BP_InitLoadingSpinner()
{
   CResId const resId("$/misc/loading/****/loading.ctxr");
   CResId const resId_jp("$/misc/loading/****/loading_jp.ctxr");
   CResId const resId_sa("$/misc/loading/****/loading_sa.ctxr");
   CResId const * pRegionIDRegion;
   
   if ( OsContext()->IsDownloadableVersion() )
   {
      pRegionIDRegion = &resId_sa;
   }
   else
   {
      pRegionIDRegion = OsContext()->mBuildSKU == COsContext::kBS_Japan ? &resId_jp : &resId;
   }

#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3
   {
      unsigned long textureIdx = ast_open_file_index(pRegionIDRegion->c_str());

      unsigned char const * pTextureDataMemory = ast_get_file_data(textureIdx);
      unsigned int textureDataSize = ast_get_file_length(textureIdx);

      Loading::gpTexture = CBaseTexture::Create((void*)pTextureDataMemory, textureDataSize);

      ast_close_file_index(textureIdx);
   }

#elif BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_VITA
   
   // TODO: Armature - Add vita resource embedding into executable.

   // When loading data from the resource system we have a TResource to keep owner ship.
   Loading::gTextureOwner = TLockedResource<CBaseTexture>(gpResources->GetResource(*pRegionIDRegion));
   Loading::gpTexture = Loading::gTextureOwner->GetPtr();
#else

#error "Unknown platform"

#endif
}

void BP_SetSpinnerLoadFlag(int loadFlags, int state)
{
   using namespace Loading;

   int originalLoadingState = gIsLoading;

   if( state )
   {
      gIsLoading |= loadFlags;
   }
   else
   {
      gIsLoading &= ~loadFlags;
   }
}

void BP_SetSpinnerInitialLoadBegin()
{
   BP_SetSpinnerLoadFlag( kLoadFlag_InitialLoad, 1 );
   Loading::gCurrentState = Loading::kState_EnterFadeIn;
   Loading::gSpinnerAlpha = 1.0f;
}

void BP_SetSpinnerInitialLoadEnd()
{
   BP_SetSpinnerLoadFlag( kLoadFlag_InitialLoad, 0 );
}

void BP_RenderLoadingSpinner()
{
   using namespace Loading;

   if ( Loading::gpTexture == NULL )
   {
      // Can't render an unloaded spinner!
      return; 
   }

   real32 const kVisibleThreshold = 1.0f / 255.0f;

   switch(gCurrentState)
   {
   case kState_Idle:
      {
         BP_SPINNER_PRINT("kState_Idle\n");

         if( gIsLoading )
         {
            gCurrentState = kState_PreFadeIn;
            gLoadingTimer.Reset();
         }
      }
      break;

   case kState_PreFadeIn:
      {
         if( gIsLoading )
         {
            real32 const currentTime = gLoadingTimer.GetElapsedTime();
            BP_SPINNER_PRINT("kState_PreFadeIn: Timer: %f s\n", currentTime);

            if( currentTime > kLoadingSpinnerDefaultShowTime )
            {
               gSpinnerAnimationTime.Reset();
               gCurrentState = kState_EnterFadeIn;
            }
         }
         else
         {
            gCurrentState = kState_BeginCoolDown;
         }
      }
      break;
   
   case kState_EnterFadeIn:
      {
         gFadeInCurrentTime.Reset();
         gCurrentState = kState_FadingIn;
      }
      // fallthrough intentional

   case kState_FadingIn:
      {
         BP_SPINNER_PRINT("kState_FadingIn\n");

         gSpinnerAlpha += kSpinnerAlphaFadeSpeed;

         if( !gIsLoading )
            gCurrentState = kState_FadingInCoolDown;
      }
      break;
   
   case kState_FadingInCoolDown:
      {
         BP_SPINNER_PRINT("kState_FadingInCoolDown\n");
         gSpinnerAlpha += kSpinnerAlphaFadeSpeed;

         if( gIsLoading )
         {
            gCurrentState = kState_EnterFadeIn;
         }
         else
         {
            // Prevent fading out until at least minimum amount of time has passed.
            if( gFadeInCurrentTime.GetElapsedTime() > kSpinnerMinimumFadeInTime )
            {
               gCurrentState = kState_FadingOut;
            }
         }
      }
      break;

   case kState_FadingOut:
      {
         BP_SPINNER_PRINT("kState_FadingOut\n");

         if( gIsLoading )
         {
            gCurrentState = kState_EnterFadeIn;
         }
         else
         {
            gSpinnerAlpha -= kSpinnerAlphaFadeSpeed;

            if( gSpinnerAlpha < 0.0f )
            {
               gCurrentState = kState_BeginCoolDown;
            }
         }
      }
      break;

   case kState_BeginCoolDown:
      {
         gCoolDownTimer.Reset();
         gCurrentState = kState_CoolDown;
      }

   case kState_CoolDown:
      {
         real32 const currentTime = gCoolDownTimer.GetElapsedTime();
         BP_SPINNER_PRINT("kState_CoolDown: Timer: %f s\n", currentTime);

         if( gIsLoading )
            gCurrentState = kState_PreFadeIn;
         else if( currentTime > kCoolDownTime )
            gCurrentState = kState_Idle;
      }
      break;
   }

   gSpinnerAlpha = MathUtils::ClampZeroOne(gSpinnerAlpha);

   if( gSpinnerAlpha < kVisibleThreshold )
      return;

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif
   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);
   
   gpRenderBackend->SetDepthCompareEnabled(false);

   gpRenderBackend->SetStencilEnable(false);

   CBaseTexture* pTexture = (CBaseTexture*)gpTexture;
   
   int const viewportWidth = 1280;
   int const viewportHeight = 720;

   // 15% safe zone
   real32 const safeZoneFactor = 0.15f;

   // texture width/Height
   int const width = pTexture->GetWidth();
   int const height = pTexture->GetHeight();

   // Allows shifting of loading texture
   int const xOffset = 25; // current texture has no text within 25 pixels of right side
   int const yOffset = 0;

   // aligned to bottom right of screen (title safe corner)
   int const x = viewportWidth - (viewportWidth * safeZoneFactor * 0.5f) - width + xOffset;
   int const y = viewportHeight - (viewportHeight * safeZoneFactor * 0.5f) - height + yOffset;

   CVector4 const uvRect( 0.5f / pTexture->GetWidth(), (pTexture->GetHeight() + 0.5f) / pTexture->GetHeight(), (pTexture->GetWidth() + 0.5f) / pTexture->GetWidth(), 0.5f / pTexture->GetHeight() );

   real32 const viewX = -1.0f + 2.0f * x / viewportWidth;
   real32 const viewY = -1.0f + 2.0f * (viewportHeight - y) / viewportHeight;
   real32 const viewWidth = 2.0f * width / viewportWidth;
   real32 const viewHeight = -2.0f * height / viewportHeight;

   real32 const kCycleSpeed = 2.0f;
   
   real32 const t = fabsf((2.0f * fmodf(gSpinnerAnimationTime.GetElapsedTime(), kCycleSpeed) / kCycleSpeed) - 1.0f);

   real32 const alpha = gSpinnerAlpha;
   real32 const color = MathUtils::SmoothLerp(t, 0.75f, 1.0f);

   BP_DrawRectTextureModulate(pTexture, viewX, viewY, viewWidth, viewHeight, CVector4(color, color, color, alpha), uvRect, 1, 1);
}

void BP_Debug_LoadingSpinnerRender()
{
#ifndef GOLD_VERSION
   if( gBP_LoadingSpinnerDebug )
   {
      BP_DebugText_Print("Loading Spinner: %d %08x %f", Loading::gCurrentState, Loading::gIsLoading, Loading::gLoadingTimer.GetElapsedTime() );
   }
#endif
}

//----------------------------------------------------------------------------

struct SAreaPresenceMap
{
   const char* areaName;
   int         presenceID;
};

#if MGS_VERSION==3
static SAreaPresenceMap const skAreaPresenceMap[] =
{
   { "title",     kRP_MGS3_MAINMENU    },
   { "mg1",       kRP_MGS3_MG1         },
   { "mg2",       kRP_MGS3_MG2         },
   { "theater",   kRP_MGS3_DEMOTHEATER },
};

static int gCurrentlyInTheater = 0;
#else
static SAreaPresenceMap const skAreaPresenceMap[] =
{
   { "n_title",   kRP_MGS2_MAINMENU    },
   { "select",    kRP_MGS2_MAINMENU    },
   { "mselect",   kRP_MGS2_MAINMENU    },
}; 

static int gCurrentlyInSnakeTales = 0;
#endif

//All MGS2 stages
/*
a00a
a00b
a00c
a01a
a01b
a01c
a01d
a01e
a01f
a02a
a02b
a03a
a03b
a12a
a12b
a13a
a13b
a13c
a14a
a14b
a15a
a15b
a16a
a17a
a18a
a19a
a20a
a20b
a20c
a20e
a21a
a21b
a22a
a22b
a23a
a23b
a24a
a24b
a24c
a24d
a24f
a24g
a25a
a25d
a28a
a31a
a31b
a31c
a41a
a41b
a42a
a43a
a45a
a46a
a61a
boss
d001p01
d001p02
d005p01
d005p03
d00t
d010p01
d012p01
d014p01
d01t
d021p01
d036p03
d036p05
d045p01
d046p01
d04t
d053p01
d055p01
d05t
d063p01
d065p02
d070p01
d070p09
d070px9
d078p01
d080p01
d080p06
d080p07
d080p08
d082p01
d10t
d11t
d12t
d12t3
d12t4
d13t
d14t
ending
init
mselect
museum
n_title
r_plt0
r_plt1
r_plt10
r_plt11
r_plt12
r_plt13
r_plt2
r_plt3
r_plt4
r_plt5
r_plt6
r_plt7
r_plt8
r_plt9
r_plt_s
r_rai_b
r_sna_b
r_title
r_tnk0
r_tnk_r
r_vr_1
r_vr_b
r_vr_p
r_vr_r
r_vr_rp
r_vr_s
r_vr_sp
r_vr_t
r_vr_x
select
sp01a
sp02a
sp03a
sp06a
sp07a
sp08a
sp21a
sp22a
sp24a
sp25a
sselect
st01a
st02a
st03a
st04a
st05a
ta00a
ta01a
ta01b
ta01c
ta01d
ta01e
ta01f
ta02a
ta12a
ta20a
ta22a
ta24a
ta31a
ta42a
tales
tsp03a
tvs03a
tvs05a
tvs06a
tvs08a
twp03a
twp34a
twp43a
vs01a
vs02a
vs03a
vs04a
vs05a
vs06a
vs07a
vs08a
vs09a
vs10a
w00a
w00b
w00c
w01a
w01b
w01c
w01d
w01e
w01f
w02a
w03a
w03b
w04a
w04b
w04c
w11a
w11b
w11c
w12a
w12b
w12c
w13a
w13b
w14a
w15a
w15b
w16a
w16b
w17a
w18a
w19a
w20a
w20b
w20c
w20d
w21a
w21b
w22a
w23a
w23b
w24a
w24b
w24c
w24d
w24e
w25a
w25b
w25c
w25d
w28a
w31a
w31b
w31c
w31d
w31f
w32a
w32b
w41a
w42a
w43a
w44a
w45a
w46a
w51a
w61a
webdemo
wmovie
wp01a
wp02a
wp03a
wp04a
wp05a
wp11a
wp12a
wp13a
wp14a
wp15a
wp21a
wp22a
wp23a
wp24a
wp25a
wp31a
wp32a
wp33a
wp34a
wp35a
wp41a
wp42a
wp43a
wp44a
wp45a
wp51a
wp52a
wp53a
wp54a
wp55a
wp61a
wp62a
wp63a
wp64a
wp65a
wp71a
wp72a
wp73a
wp74a
wp75a
*/
// Notes:
// Boss - before boss survival mode
// wp - weapon vr
// sp - first person vr
// st - streaking vr
// wmovie - almost anywhere a full screen movie is played
// webdemo - multiple places, fake web page with embedded images
// 

static char* pMGS2TankerStages[]=
{ 
   "d00t",
   "d01t",
   "d04t",
   "d05t",
   "d10t",
   "d11t",
   "d12t",
   "d12t3",
   "d12t4",
   "d13t",
   "d14t",

   "w00a",
   "w01b",
   "w01a",
   "w01f",
   "w01c",
   "w01d",
   "w01e",
   "w00b",
   "w00c",
   "w02a",
   "w03a",
   "w03b",
   "w04a",
   "w04b",
   "w04c",

   0
};

char* pMGS2PlantStages[]=
{ 
   "museum",

   "d001p01",
   "d001p02",
   "d005p01",
   "d005p03",
   "d010p01",
   "d012p01",
   "d014p01",
   "d021p01",
   "d036p03",
   "d036p05",
   "d045p01",
   "d046p01",
   "d053p01",
   "d055p01",
   "d063p01",
   "d065p02",
   "d070p01",
   "d070p09",
   "d070px9",
   "d078p01",
   "d080p01",
   "d080p06",
   "d080p07",
   "d080p08",
   "d082p01",

   "w11a",
   "w11b",
   "w11c",
   "w12a",
   "w12b",
   "w12c",
   "w13a",
   "w13b",
   "w14a",
   "w15a",
   "w15b",
   "w16a",
   "w16b",
   "w17a",
   "w18a",
   "w19a",
   "w20a",
   "w20b",
   "w20c",
   "w20d",
   "w21a",
   "w21b",
   "w22a",
   "w23a",
   "w23b",
   "w24a",
   "w24b",
   "w24c",
   "w24d",
   "w24e",
   "w25a",
   "w25b",
   "w25c",
   "w25d",
   "w28a",
   "w31a",
   "w31b",
   "w31c",
   "w31d",
   "w31f",
   "w32a",
   "w32b",
   "w41a",
   "w42a",
   "w43a",
   "w44a",
   "w45a",
   "w46a",
   "w51a",
   "w61a",

   0
};

void BP_UpdateRichPresenceLogic()
{
#if BPE_TARGET == BPE_TARGET_X360

#if MGS_VERSION==2
   char *pAreaName = GM_GetArea();
   if( pAreaName == NULL || strlen(pAreaName) < 2 )
   {
      return;
   }

   static CStopWatch snakeTalesTimer;
   // snake tales
   if ( GM_VRStatus&GM_VR_SNAKETALES || strcasecmp("tales", pAreaName) == 0 )
   {
      gCurrentlyInSnakeTales = 1;
      snakeTalesTimer.Reset();
      BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_SNAKETALES);
      return;
   }
   
   // vr missions
   if ( pAreaName[0] == 'v' )
   {
      BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_VRMISSION);
      return;
   }
   else if ( pAreaName[0] == 'w' && pAreaName[1] == 'p' ) // weapon VR
   {
      BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_VRMISSION);
      return;
   }
   else if ( pAreaName[0] == 's' && pAreaName[1] == 'p' ) // First person VR
   {
      BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_VRMISSION);
      return;
   }
   else if ( pAreaName[0] == 's' && pAreaName[1] == 't' ) // streaking VR
   {
      BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_VRMISSION);
      return;
   }

   // alt mission
   if ( pAreaName[0] == 'a' )
   {
      // When transitioning between snake tales levels the GM_VR_SNAKETALES gets cleared before level load completes
      if( gCurrentlyInSnakeTales == 0 )
      {
         BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_ALTMISSION);
         return;
      }
   }

   // check all plant stages.
   for (int i=0; pMGS2PlantStages[i]!=0; i++ )
   {
      if ( strcasecmp(pMGS2PlantStages[i],pAreaName) == 0 )
      {
         // player is in the plant
         BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_PLANT);
         return;
      }
   }

   // check all tanker stages.
   for (int i=0; pMGS2TankerStages[i]!=0; i++ )
   {
      if ( strcasecmp(pMGS2TankerStages[i],pAreaName) == 0 )
      {
         // player is in the plant
         BP_RichPresenceSystem_SetActiveContext(kRP_MGS2_TANKER);
         return;
      }
   }

   // Check for special area based overrides.
   for( int i = 0; i < BPE_ARRAY_SIZE(skAreaPresenceMap); ++i )
   {
      SAreaPresenceMap const & areaPresence = skAreaPresenceMap[i];
      if( strcasecmp(areaPresence.areaName, pAreaName) == 0 )
      {
         ERichPresence const presenceId = (ERichPresence)areaPresence.presenceID;

         gCurrentlyInSnakeTales = 0;
         BP_RichPresenceSystem_SetActiveContext(presenceId);
         return;
      }
   }

   if( strcasecmp("boss", pAreaName) == 0 )
   {
      // Very short transition before entering the actual boss stages (which are either tanker, plant stages or vr stages)
      // All it does is show panels of who is fighting against who for less than half a second with no way to interrupt.
      // We're keeping the last rich presence state for this case.
      return;
   }
   else if( strcasecmp("wmovie", pAreaName) == 0 )
   {
      // This stage gets triggered any time a in game fullscreen movie is played
      // This happens during the tanker, plant and at the ending of the game.
      // We're keeping the last rich presence state for this case.
      return;
   }
   else if( strcasecmp("webdemo", pAreaName) == 0 )
   {
      // This is a non interactive "web page" that shows a fullscreen image with super imposed images.
      // It gets triggered during the tanker and the plant.
      // We're keeping the last rich presence state for this case.
      return;
   }
   else if( strcasecmp("sselect", pAreaName) == 0 )
   {
      // This is usually snaketales select but when completing the final snake
      // tales, this stage is used for the credits
      // We're keeping the last rich presence state for this case.
      return;
   }
   else if( strcasecmp("ending", pAreaName) == 0 )
   {
      // This is the end credits and results screen.
      // It gets triggered during snake tales, tanker and the plant.
      // We're keeping the last rich presence state for this case.
      return;
   }

#elif MGS_VERSION==3
   const char* pAreaName = GM_GetArea();

   // Only allow Virtous Mission/"Snake Eater" Mission detection when not currently in the demo theater.
   if( gCurrentlyInTheater == 0 )
   {
      if( strlen(pAreaName) >= 2 )
      {
         if( pAreaName[1] >= '0' && pAreaName[1] <= '9')
         {
            switch(tolower(pAreaName[0]))
            {
            case 'v':
               // stage name of format v<NUMBER>?
               BP_RichPresenceSystem_SetActiveContext(kRP_MGS3_VIRTUOUS);
               return;
            case 's':
               // stage name of format s<NUMBER>?
               BP_RichPresenceSystem_SetActiveContext(kRP_MGS3_SNAKEEATER);
               return;
            }
         }
      }
   }

   // Check for special area based overrides.
   for( int i = 0; i < BPE_ARRAY_SIZE(skAreaPresenceMap); ++i )
   {
      SAreaPresenceMap const & areaPresence = skAreaPresenceMap[i];
      if( strcasecmp(areaPresence.areaName, pAreaName) == 0 )
      {
         ERichPresence const presenceId = (ERichPresence)areaPresence.presenceID;
         
         gCurrentlyInTheater = (presenceId == kRP_MGS3_DEMOTHEATER);
         
         BP_RichPresenceSystem_SetActiveContext(presenceId);
         break;
      }
   }
#endif

#endif
}


//----------------------------------------------------------------------------
#if BPE_TARGET == BPE_TARGET_X360
extern "C" void BP_WaitForLastRenderToComplete();
#endif

extern "C" void ShowExitGameWarning(int *pResult)  // about to perform destructive action
{
#if BPE_TARGET == BPE_TARGET_X360
   BP_SetAllDirectOutputStreamPaused(1);

   gpRenderBackend->SuspendGameRenderThread();
   OSDialogRequest* pRet = new OSDialogRequest(kDRT_ExitGameplay);
   OsContext()->AddDialogRequest(pRet);
   while( !pRet->IsDone() )
   {
      BP_EndFrame();
      Sleep(1);
   }
   gpRenderBackend->ResumeGameRenderThread();

   BP_SetAllDirectOutputStreamPaused(0);

   *pResult = OsContext()->mLastExitGameRequestRealResult;
#else
   *pResult = 0;
#endif
}

#if BPE_TARGET == BPE_TARGET_X360
extern "C" void ShowDeviceSelector_Sync(unsigned int requiredSizeForNewSave)
{
   BP_SetAllDirectOutputStreamPaused(1);

   gpRenderBackend->SuspendGameRenderThread();
   OSDialogRequest* pOSDialogRequest = OsContext()->ShowDeviceUI(requiredSizeForNewSave);
   while( !pOSDialogRequest->IsDone() )
   {
      BP_EndFrame();
      Sleep(1);
   }

   gpRenderBackend->ResumeGameRenderThread();
   BP_SetAllDirectOutputStreamPaused(0);
}

extern "C" void ShowGenericOsDialog_Sync(unsigned int dialogType/*EDialogRequestType*/)
{
   BP_SetAllDirectOutputStreamPaused(1);

   gpRenderBackend->SuspendGameRenderThread();

   OSDialogRequest* pRet = new OSDialogRequest((EDialogRequestType)dialogType);
   OsContext()->AddDialogRequest(pRet);
   
   while( !pRet->IsDone() )
   {
      BP_EndFrame();
      Sleep(1);
   }

   gpRenderBackend->ResumeGameRenderThread();
   BP_SetAllDirectOutputStreamPaused(0);

}

extern "C" int CheckDeviceNewGameMode(int requiredSpace)
{
   COsContext * pOsContext = OsContext();
   if( pOsContext->mSaveLoadDeviceId != XCONTENTDEVICE_ANY )
   {
      XDEVICE_DATA deviceData;
      if( XContentGetDeviceData(pOsContext->mSaveLoadDeviceId, &deviceData) != ERROR_DEVICE_NOT_CONNECTED )
      {
         if( deviceData.ulDeviceFreeBytes >= requiredSpace )
         {
            return kCDNGMRT_NothingRequired;
         }
         else
         {
            return kCDNGMRT_ShowDeviceSelector;
         }
      }
      else
      {
         // They probably want to save, they probably just tripped and accidentally pulled out the memory card
         // We want to show them the device invalid and the device selector in this case
         return kCDNGMRT_ShowDeviceInvalidThenDeviceSelector;
      }
   }  
   else
   {
      // We warned them the couldn't save, they obviously don't care
      return kCDNGMRT_NothingRequired;
   }
}

extern "C" void DoDeviceSelectorLogic_ForSpace(int requiredSpace)
{   
   int result = CheckDeviceNewGameMode(requiredSpace);
   switch( result )
   {
   case kCDNGMRT_NothingRequired:
      return;
      break;
   case kCDNGMRT_ShowDeviceInvalidThenDeviceSelector:
      ShowGenericOsDialog_Sync(3/*kDRT_StorageDeviceUnavailable*/);
      //fallthrough
   case kCDNGMRT_ShowDeviceSelector:
      ShowDeviceSelector_Sync(requiredSpace);
      break;
   }
}
#endif

//----------------------------------------------------------------------------
volatile int gBP_ScreenSaverSuspended = 0;
void BP_ConsoleScreenSaverSuspend()
{
#if BPE_TARGET == BPE_TARGET_X360
   gBP_ScreenSaverSuspended = 1;
   XEnableScreenSaver(FALSE);
   printf("Screensaver disabled.\n");
#endif
}

void BP_ConsoleScreenSaverResume()
{
#if BPE_TARGET == BPE_TARGET_X360
   gBP_ScreenSaverSuspended = 0;
   XEnableScreenSaver(TRUE);
   printf("Screensaver enabled.\n");
#endif
}

//----------------------------------------------------------------------------

void BP_CheckForHardShutdown(int bWaitUntilShutdown)
{
#if ENABLE_SEPERATE_RENDER_THREAD
   // Wait for the rendering thread to finish.
   RenderBackend()->WaitThreadFunction();
#endif

#if defined( BP_PS3 )

   // If we're going into final waiting for shutdown state, disable the loading icon.
   // This prevents rendering issues when both the renderer thread and US are trying to render.
   {
      Loading::gIsLoading = 0;
      Loading::gCurrentState = Loading::kState_Idle;
      Loading::gSpinnerAlpha = 0;
   }

   // Immediately mute all sound to avoid streaming sound errors while we wait.
   //TODO: Do something comparable to BP_MultiStream_SetMasterVol(0.f);
   while (true)
   {
      // ClearAndSwapForOSD will check for game termination and perform shutdown.
      RenderBackend()->ClearAndSwapForOSD();

      // check the callback queue.
      cellSysutilCheckCallback();

      // sleep for a bit.
      sys_timer_usleep(500);

      BP_ShutdownCheck();

      if (!bWaitUntilShutdown)
      {
         bpe_debugger_printf("BP_CheckForHardShutdown return.\n");
         return;
      }
   }


   // should never reach here.
   BP_BREAK;

#elif defined( BP_360 )

#elif defined( BP_WIN32 )

#elif defined( BP_VITA )

#else
#error Unsupported platform!
#endif
}

//----------------------------------------------------------------------------

#if defined(BP_PS3)

#include <np/drm.h>
#include <sys/fs_external.h>

extern SceNpDrmKey skDRMKey_PSN;

int BP_CheckForLicenseFile()
{
   int ret = 0;

   if( OsContext()->GetBootType() == COsContext::kBT_HDD )
   {
      char edatPath[CELL_FS_MAX_FS_PATH_LENGTH] = {};
      sprintf(edatPath, "%s/%s", OsContext()->mGameUsrdirPath, "license.edat");
      
      ret = sceNpDrmIsAvailable(&skDRMKey_PSN, edatPath);
      printf("BP_CheckForLicenseFile (%s) ret: 0x%08x\n", edatPath, ret);
      if (ret != 0 )
      {
         // Function call should never return
         COsContext::TerminateLicenseCheckFailed();
      }
   }

   return ret;
}

#else

int BP_CheckForLicenseFile()
{
   return 0;
}

#endif

//----------------------------------------------------------------------------

int BP_IsDownloadableVersion()
{
   return OsContext()->IsDownloadableVersion() ? 1 : 0;
}
