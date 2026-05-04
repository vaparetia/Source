//----------------------------------------------------------------------------
// BP_Debug.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_Debug.h"

//----------------------------------------------------------------------------

#include "Engine/Graphics/CColor.h"
#include "Engine/Resource/CResourceManager.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "BP_RendererDebug.h"
#include "BP_SoundSupport.h"
#include "BP_Renderer.h"
#include "Engine/System/COsContext.h"
#include "BP_TrophySystem.h"
#include "Renderer/Base/Primitive/CMesh.h"

#if MGS_VERSION==3
#include "BP_GclHooks.h"
#endif

#if BP_360
#include <xtl.h>
#include <xbdm.h>
extern int gCommandLineHadProfileInfo;
extern int gCommandLineActiveController;
extern int gCommandLineProfileStatus;
extern int gCommandLineYAxisInverted;

extern int gBP_DisableMemoryCardSizeCheck;
#endif

#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
extern int gBP_DisableApplicationPause;
#endif

#if BP_VITA
#  include <libperf.h>
#endif

//----------------------------------------------------------------------------

extern "C"
{
   // GM_LEVEL_??? enums are 0 -> 60
   // These enums are 0 -> 6 for easy indexing
   enum EGameDifficulty
   {
      kGameDifficulty_VERYEASY,  // GM_LEVEL_VERYEASY
      kGameDifficulty_EASY,      // GM_LEVEL_EASY
      kGameDifficulty_NORMAL,    // GM_LEVEL_NORMAL
      kGameDifficulty_HARD,      // GM_LEVEL_HARD
      kGameDifficulty_EXTREME,   // GM_LEVEL_EXTREME
      kGameDifficulty_E_EXTREME, // GM_LEVEL_E_EXTREME

      kGameDifficulty_COUNT
   };

   extern char * gBP_X360PlatformRoot;
   extern char * gBPE_X360PlatformRoot;
   extern int gBPE_X360PlatformRootLength;

#ifdef BP_VITA
   extern int gBP_UseVitaControlOverrides;
   extern int gBP_VitaStrafeDoubleClick;
   extern int gBP_VitaAutoDrawWeaponWhenStrafing;
   extern const char* gBP_VitaControlsEnum[];
   extern const char* gBP_VitaScopeToggleModeEnum[];
   extern int gBP_VitaScopeToggleMode;
   extern int gBP_VitaUseFlickForStab;
#endif

   extern void BP_InitStreamDebugMenu();

   extern void BP_SetInitialStage( const char * stagename );
   extern int main_argc;
   extern char **main_argv;

#if MGS_VERSION == 2
   #include "camera.h"
   #include "../user/sonoyama/raiden/pl_work.h"

   int gBP_KP_Cheat_OptCmfMode = kBP_KP_Cheat_OptCmf_Normal;
#endif

#if MGS_VERSION == 3
   #include	"BP_DebugCollision.h"//BP_COLL

#define throw CPP_throw
   #include "../../source/user/morita/player/include/player.h"
#undef throw

   extern int gBP_1stPersonCamera_EnableMovement;
   extern int PL_DebugMenuStatus;
   extern int EQMENU_DebugFlag;
   extern int gBP_DisableTex2D;
   extern int gBP_DisableLayoutAnimeDemo;

   extern int ENE_DBG_EyeClose;
   extern int ENE_DBG_EarClose;
   extern int gBP_ForceEnableDownCamo;

   extern int gBP_ForceDisableTorture;
#endif

   // Pause frame stepping
   int   gBP_PauseFrameStep = 0;
   int   gBP_ShowDebugMenuBlackBackground = 1;

   // Input
   int   gBP_Input_ShowPressure = 0;
#if defined(BP_360) || defined(BP_VITA)
   int   gBP_Input_X360Remap_Enable = 1;
#else
   int   gBP_Input_X360Remap_Enable = 0;
#endif
   int   gBP_Input_X360Remap_DemoZoomMethod = 1;
   int   gBP_Input_X360Remap_QuickHolsterMethod = 0;
   int   gBP_Input_X360Remap_QuickHolsterMethodEnable[2] = {1,0};

#if MGS_VERSION == 2
   int   gBP_Input_HoldSquatToGround_Enable = 1;

   int   gBP_Game_GrenadeExplodeInHand_Enable = 0;
   int   gBP_Game_GrenadeBlast_InnerRange = (int)(1500*1.0f);  // PS2 = 1500
   int   gBP_Game_GrenadeBlast_OuterRange = (int)(3000*1.5f);  // PS2 = 3000
   
   int   gBP_Game_SolidusChoke_Life[kGameDifficulty_COUNT] = { 200, 184, 168, 152, 136, 120 };  // PS2 = { 200, 120, 100, 75, 50, 30 }
   int   gBP_Game_SolidusChoke_Time[kGameDifficulty_COUNT] = { 600, 650, 700, 750, 800, 850 };  // PS2 = { 600, 635, 900, 1200, 1500, 30000 }
#endif

   // Cheats
   int gBP_EnemySightDisabled = -1;
   int gBP_EnemyHearingDisabled = -1;

   int gBP_FlipWeaponItemLists = 0;

   // Trigger pressure is used for zooming with either the scope or sniper rifles.
   int gBP_UseFrontTouchForZooming = 1;
   int gBP_UseJoystickStyleScopeZooming = 1;
   int gBP_TouchZoomLikeJoystick = 1;

#if defined(BP_VITA)
   extern int BP_DebugMenu_Callback_UpdateWeaponItemPanels(int data);
#endif

   int gBP_KillCurrentBoss = FALSE;
   int gBP_DamageCurrentBoss = FALSE;
};

extern int gBP_RadarRenderOffscreen;
extern int gBP_DebugMenuIsActive;;

//----------------------------------------------------------------------------
#if BP_360
extern "C" void BP_TransformIntoCommandLineArgs(char const * InCommandLine, int* OutNumArgs, char*** OutArgs)
{
#ifndef GOLD_VERSION
   DmMapDevkitDrive();
#endif
   int argc = 0;
   int totalLen = 0;
   while( InCommandLine[totalLen] != NULL )
   {
      if( InCommandLine[totalLen] == ' ' )
      {
         ++argc;
      }
      ++totalLen;
   }
   ++argc;

   char ** argv = new char*[argc];
   int iStart=0;
   int iArg = 0;
   int strLength = 0;
   for( int iLen=0; iLen<totalLen; ++iLen )
   {
      if( InCommandLine[iLen] == ' ' )
      {
         argv[iArg] = new char[strLength+1];
         memcpy(argv[iArg], &InCommandLine[iStart], strLength);
         argv[iArg][strLength] = NULL;

         ++iArg;
         strLength = 0;
         ++iLen;
         iStart = iLen;
      }
      ++strLength;
   }
   argv[iArg] = new char[strLength+1];
   memcpy(argv[iArg], &InCommandLine[iStart], strLength);
   argv[iArg][strLength] = NULL;

   *OutNumArgs = argc;
   *OutArgs = argv;
}

extern "C" int BP_PreParseCommandLineArgs(int InStartArg, int InNumArgs, char** InArgs, char const * const pApplicationRootDirectory)
{
   int firstFoundX360SpecificVariable = -1;
   bool didCommandLineArgs_OverwriteDefaultPath = false;
   for( int i=InStartArg; i < InNumArgs; ++i )
   {
      // Be very careful, not to pass real arguments after any -X360XXXX argument
      // all argument after the first -X360XXXX argument are stripped off
      if( !strcmp( InArgs[i], "-X360PlatformRoot" ) )
      {
         if( firstFoundX360SpecificVariable == -1 )
            firstFoundX360SpecificVariable = i;

         ++i;
         gBP_X360PlatformRoot = InArgs[i];
         gBPE_X360PlatformRoot = InArgs[i];
         gBPE_X360PlatformRootLength = strlen(gBPE_X360PlatformRoot)-1;//minus extra '\'

         didCommandLineArgs_OverwriteDefaultPath = true;
      }
      if( !strcmp( InArgs[i], "-X360ControllerNum" ) )
      {
         if( firstFoundX360SpecificVariable == -1 )
            firstFoundX360SpecificVariable = i;

         ++i;
         BPE_VERIFY( i < InNumArgs, false, "-X360ControllerNum must be followed by #" );
         gCommandLineActiveController = atoi( InArgs[i] );
      }
      if( !strcmp( InArgs[i], "-X360ProfileStatus" ) )
      {
         if( firstFoundX360SpecificVariable == -1 )
            firstFoundX360SpecificVariable = i;

         ++i;
         BPE_VERIFY( i < InNumArgs, false, "-X360ProfileValid must be followed by #" );
         gCommandLineProfileStatus = atoi( InArgs[i] );

         gCommandLineHadProfileInfo = 1;
      }
      if( !strcmp( InArgs[i], "-X360YAxisInvert" ) )
      {
         if( firstFoundX360SpecificVariable == -1 )
            firstFoundX360SpecificVariable = i;

         ++i;
         BPE_VERIFY( i < InNumArgs, false, "-X360YAxisInvert must be followed by #" );
         gCommandLineYAxisInverted = atoi( InArgs[i] );

         gCommandLineHadProfileInfo = 1;
      }
   }
   
   // Setup default subdirectory if not overridden by command line args.
   // This is the default path when running off a disk, obviously different for MGS2/3
   if( didCommandLineArgs_OverwriteDefaultPath == false && pApplicationRootDirectory != NULL  )
   {
      CResourceManager::SetRepositoryPath(pApplicationRootDirectory);
   }
   return firstFoundX360SpecificVariable;
}

extern "C" void BP_CleanupTransformIntoCommandLineArgs(int InNumArgs, char** InArgs)
{

   for( int ii=0;ii<InNumArgs;++ii )
{
      delete [] InArgs[ii];
   }

   delete [] InArgs;
   InArgs = NULL;
   InNumArgs = 0;
   }
#endif

//----------------------------------------------------------------------------

#if BP_WIN32

// Handy runtime set/remove hardware breakpoint functions from:
//    http://www.codeproject.com/KB/debug/hardwarebreakpoint.aspx

HANDLE SetHardwareBreakpoint(HANDLE hThread,HWBRK_TYPE Type,HWBRK_SIZE Size,void* s);
bool RemoveHardwareBreakpoint(HANDLE hBrk);

//----------------------------------------------------------------------------

class HWBRK
{
public:
   void* a;
   HANDLE hT;
   HWBRK_TYPE Type;
   HWBRK_SIZE Size;
   HANDLE hEv;
   int iReg;
   int Opr;
   bool SUCC;

   HWBRK()
   {
      Opr = 0;
      a = 0;
      hT = 0;
      hEv = 0;
      iReg = 0;
      SUCC = false;
   }
};

//----------------------------------------------------------------------------

void SetBits(DWORD_PTR& dw, int lowBit, int bits, int newValue)
{
   DWORD_PTR mask = (1 << bits) - 1; 
   dw = (dw & ~(mask << lowBit)) | (newValue << lowBit);
}

//----------------------------------------------------------------------------

static DWORD WINAPI th(LPVOID lpParameter)
{
   HWBRK* h = (HWBRK*)lpParameter;
   int j = 0;
   int y = 0;

   j = SuspendThread(h->hT);
   y = GetLastError();

   CONTEXT ct = {0};
   ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
   j = GetThreadContext(h->hT,&ct);
   y = GetLastError();

   int FlagBit = 0;

   bool Dr0Busy = false;
   bool Dr1Busy = false;
   bool Dr2Busy = false;
   bool Dr3Busy = false;
   if (ct.Dr7 & 1)
      Dr0Busy = true;
   if (ct.Dr7 & 4)
      Dr1Busy = true;
   if (ct.Dr7 & 16)
      Dr2Busy = true;
   if (ct.Dr7 & 64)
      Dr3Busy = true;

   if (h->Opr == 1)
   {
      // Remove
      if (h->iReg == 0)
      {
         FlagBit = 0;
         ct.Dr0 = 0;
         Dr0Busy = false;
      }
      if (h->iReg == 1)
      {
         FlagBit = 2;
         ct.Dr1 = 0;
         Dr1Busy = false;
      }
      if (h->iReg == 2)
      {
         FlagBit = 4;
         ct.Dr2 = 0;
         Dr2Busy = false;
      }
      if (h->iReg == 3)
      {
         FlagBit = 6;
         ct.Dr3 = 0;
         Dr3Busy = false;
      }

      ct.Dr7 &= ~(1 << FlagBit);
   }
   else
   {
      if (!Dr0Busy)
      {
         h->iReg = 0;
         ct.Dr0 = (DWORD_PTR)h->a;
         Dr0Busy = true;
      }
      else
         if (!Dr1Busy)
         {
            h->iReg = 1;
            ct.Dr1 = (DWORD_PTR)h->a;
            Dr1Busy = true;
         }
         else
            if (!Dr2Busy)
            {
               h->iReg = 2;
               ct.Dr2 = (DWORD_PTR)h->a;
               Dr2Busy = true;
            }
            else
               if (!Dr3Busy)
               {
                  h->iReg = 3;
                  ct.Dr3 = (DWORD_PTR)h->a;
                  Dr3Busy = true;
               }
               else
               {
                  h->SUCC = false;
                  j = ResumeThread(h->hT);
                  y = GetLastError();
                  SetEvent(h->hEv);
                  return 0;
               }
               ct.Dr6 = 0;
               int st = 0;
               if (h->Type == HWBRK_TYPE_CODE)
                  st = 0;
               if (h->Type == HWBRK_TYPE_READWRITE)
                  st = 3;
               if (h->Type == HWBRK_TYPE_WRITE)
                  st = 1;
               int le = 0;
               if (h->Size == HWBRK_SIZE_1)
                  le = 0;
               if (h->Size == HWBRK_SIZE_2)
                  le = 1;
               if (h->Size == HWBRK_SIZE_4)
                  le = 3;
               if (h->Size == HWBRK_SIZE_8)
                  le = 2;

               SetBits(ct.Dr7, 16 + h->iReg*4, 2, st);
               SetBits(ct.Dr7, 18 + h->iReg*4, 2, le);
               SetBits(ct.Dr7, h->iReg*2,1,1);
   }

   ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
   j = SetThreadContext(h->hT,&ct);
   y = GetLastError();

   ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
   j = GetThreadContext(h->hT,&ct);
   y = GetLastError();

   j = ResumeThread(h->hT);
   y = GetLastError();

   h->SUCC = true;

   SetEvent(h->hEv);
   return 0;
}

//----------------------------------------------------------------------------

HANDLE SetHardwareBreakpoint(HANDLE hThread,HWBRK_TYPE Type,HWBRK_SIZE Size,void* s)
{
   HWBRK* h = new HWBRK;
   h->a = s;
   h->Size = Size;
   h->Type = Type;
   h->hT = hThread;

   if (hThread == GetCurrentThread())
   {
      DWORD pid = GetCurrentThreadId();
      h->hT = OpenThread(THREAD_ALL_ACCESS,0,pid);
   }

   h->hEv = CreateEvent(0,0,0,0);
   h->Opr = 0; // Set Break
   HANDLE hY = CreateThread(0,0,th,(LPVOID)h,0,0);
   WaitForSingleObject(h->hEv,INFINITE);
   CloseHandle(h->hEv);
   h->hEv = 0;

   if (hThread == GetCurrentThread())
   {
      CloseHandle(h->hT);
   }
   h->hT = hThread;

   if (!h->SUCC)
   {
      delete h;
      return 0;
   }

   return (HANDLE)h;
}

//----------------------------------------------------------------------------

bool RemoveHardwareBreakpoint(HANDLE hBrk)
{
   HWBRK* h = (HWBRK*)hBrk;
   if (!h)
      return false;

   bool C = false;
   if (h->hT == GetCurrentThread())
   {
      DWORD pid = GetCurrentThreadId();
      h->hT = OpenThread(THREAD_ALL_ACCESS,0,pid);
      C = true;
   }

   h->hEv = CreateEvent(0,0,0,0);
   h->Opr = 1; // Remove Break
   HANDLE hY = CreateThread(0,0,th,(LPVOID)h,0,0);
   WaitForSingleObject(h->hEv,INFINITE);
   CloseHandle(h->hEv);
   h->hEv = 0;

   if (C)
   {
      CloseHandle(h->hT);
   }

   delete h;
   return true;
}

//----------------------------------------------------------------------------

int BP_SetHardwareBreakpoint(void* addr, enum HWBRK_SIZE size, enum HWBRK_TYPE type)
{
   return (int)SetHardwareBreakpoint(GetCurrentThread(), type, size, addr);      
}

//----------------------------------------------------------------------------

void BP_RemoveHardwareBreakpoint(int handle)
{
   RemoveHardwareBreakpoint((HANDLE)handle);
}

#endif   //#if BP_WIN32
 
//----------------------------------------------------------------------------

void BP_HandleCommandLineArgs()
{
#ifndef GOLD_VERSION
   for( int i=1; i < main_argc; ++i )
   {
      if( !strcmp( main_argv[i], "-stage" ) )
      {
         if( i+1 < main_argc )
         {
            BP_SetInitialStage( main_argv[i+1] );
         }
         else
         {
            printf("Warning: -stage arg must be followed by stage name.\n");
         }
      }
   }
#endif
}

//----------------------------------------------------------------------------

extern "C"
{
   int gBP_DebugStageAction = -1;

#if defined(BP_360) || defined(BP_WIN32)
   // X360, WIN32
   float gBP_VibrationLgMin = 0.0f;
   float gBP_VibrationLgMax = 0.80f;
   float gBP_VibrationLgThreshold = 0.05f;
   float gBP_VibrationSmScale = 1.f;
#else
   // PS3
   float gBP_VibrationLgMin = 0.16f;
   float gBP_VibrationLgMax = 0.92f;
   float gBP_VibrationLgThreshold = 0.05f;
   float gBP_VibrationSmScale = 1.f;
#endif

   int gBP_TrophyDebug = 0;
   int gBP_DebugUnlockTrophy = 0;
   int gBP_MakeBadCloudData = 0;
   int gBP_Transfarring_FailDeleteSlotData = 0;
   int gBP_Transfarring_FailGetSlotData = 0;
   int gBP_Transfarring_FailSetSlotData = 0;
   int gBP_Transfarring_FailGetSlotInfos = 0;
   int gBP_Transfarring_FailIncrementSlotVariable = 0;
}

extern int gBP_LoadingSpinnerDebug;

int BP_DebugStageActionCallback(int data)
{
   // This is handled inside gamed.c
   gBP_DebugStageAction = data;

   return 1;
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 2

extern "C"
{
   extern int PL_PAD_ACTION;
   extern int PL_PAD_SUBJECT;
   extern int PL_PAD_STOP_AIM;
   extern int PL_PAD_SQUAT;
   extern int PL_PAD_WEAPON;
   extern int PL_PAD_PRESS_WEAPON;
   extern int PL_PAD_PUNCH;
   extern int PL_PAD_CHANGE_WEAPON;
   extern int PL_PAD_PEEP;
   extern int PL_PAD_PEEP_R;
   extern int PL_PAD_PEEP_L;
   extern int PL_PAD_PRESS_PEEP_R;
   extern int PL_PAD_PRESS_PEEP_L;
   extern int PL_PAD_CAPUTRE;
   extern int PL_PAD_LOCKON;
   extern int PL_PAD_KNOCK;
   extern int PL_PAD_HANG;
   extern int PL_PAD_BEYOND;
   extern int PL_PAD_READY_RUN;
   extern int PL_PAD_BEYOND_DOWN;
   extern int PL_PAD_LOCKER_ZOOM;
   extern int PL_PAD_PRESS_LOCKER;
   extern int PL_PAD_ZOOMIN;
   extern int PL_PAD_PRESS_ZOOMIN;
   extern int PL_PAD_ZOOMOUT;
   extern int PL_PAD_PRESS_ZOOMOUT;
   extern int PL_PAD_ITEMUSE;
};

struct SBP_PadConfig
{
   int PL_PAD_ACTION;
   int PL_PAD_SUBJECT;
   int PL_PAD_STOP_AIM;
   int PL_PAD_SQUAT;
   int PL_PAD_WEAPON;
   int PL_PAD_PRESS_WEAPON;
   int PL_PAD_PUNCH;
   int PL_PAD_CHANGE_WEAPON;
   int PL_PAD_PEEP;
   int PL_PAD_PEEP_R;
   int PL_PAD_PEEP_L;
   int PL_PAD_PRESS_PEEP_R;
   int PL_PAD_PRESS_PEEP_L;
   int PL_PAD_CAPUTRE;
   int PL_PAD_LOCKON;
   int PL_PAD_KNOCK;
   int PL_PAD_HANG;
   int PL_PAD_BEYOND;
   int PL_PAD_READY_RUN;
   int PL_PAD_BEYOND_DOWN;
   int PL_PAD_LOCKER_ZOOM;
   int PL_PAD_PRESS_LOCKER;
   int PL_PAD_ZOOMIN;
   int PL_PAD_PRESS_ZOOMIN;
   int PL_PAD_ZOOMOUT;
   int PL_PAD_PRESS_ZOOMOUT;
   int PL_PAD_ITEMUSE;
} gBP_PadConfig;

void BP_SavePadConfig()
{
   gBP_PadConfig.PL_PAD_ACTION        = PL_PAD_ACTION;
   gBP_PadConfig.PL_PAD_SUBJECT       = PL_PAD_SUBJECT;
   gBP_PadConfig.PL_PAD_STOP_AIM      = PL_PAD_STOP_AIM;
   gBP_PadConfig.PL_PAD_SQUAT         = PL_PAD_SQUAT;
   gBP_PadConfig.PL_PAD_WEAPON        = PL_PAD_WEAPON;
   gBP_PadConfig.PL_PAD_PRESS_WEAPON  = PL_PAD_PRESS_WEAPON;
   gBP_PadConfig.PL_PAD_PUNCH         = PL_PAD_PUNCH;
   gBP_PadConfig.PL_PAD_CHANGE_WEAPON = PL_PAD_CHANGE_WEAPON;
   gBP_PadConfig.PL_PAD_PEEP          = PL_PAD_PEEP;
   gBP_PadConfig.PL_PAD_PEEP_R        = PL_PAD_PEEP_R;
   gBP_PadConfig.PL_PAD_PEEP_L        = PL_PAD_PEEP_L;
   gBP_PadConfig.PL_PAD_PRESS_PEEP_R  = PL_PAD_PRESS_PEEP_R;
   gBP_PadConfig.PL_PAD_PRESS_PEEP_L  = PL_PAD_PRESS_PEEP_L;
   gBP_PadConfig.PL_PAD_CAPUTRE       = PL_PAD_CAPUTRE;
   gBP_PadConfig.PL_PAD_LOCKON        = PL_PAD_LOCKON;
   gBP_PadConfig.PL_PAD_KNOCK         = PL_PAD_KNOCK;
   gBP_PadConfig.PL_PAD_HANG          = PL_PAD_HANG;
   gBP_PadConfig.PL_PAD_BEYOND        = PL_PAD_BEYOND;
   gBP_PadConfig.PL_PAD_READY_RUN     = PL_PAD_READY_RUN;
   gBP_PadConfig.PL_PAD_BEYOND_DOWN   = PL_PAD_BEYOND_DOWN;
   gBP_PadConfig.PL_PAD_LOCKER_ZOOM   = PL_PAD_LOCKER_ZOOM;
   gBP_PadConfig.PL_PAD_PRESS_LOCKER  = PL_PAD_PRESS_LOCKER;
   gBP_PadConfig.PL_PAD_ZOOMIN        = PL_PAD_ZOOMIN;
   gBP_PadConfig.PL_PAD_PRESS_ZOOMIN  = PL_PAD_PRESS_ZOOMIN;
   gBP_PadConfig.PL_PAD_ZOOMOUT       = PL_PAD_ZOOMOUT;
   gBP_PadConfig.PL_PAD_PRESS_ZOOMOUT = PL_PAD_PRESS_ZOOMOUT;
   gBP_PadConfig.PL_PAD_ITEMUSE       = PL_PAD_ITEMUSE;
}

void BP_RestorePadConfig()
{
   PL_PAD_ACTION        = gBP_PadConfig.PL_PAD_ACTION;
   PL_PAD_SUBJECT       = gBP_PadConfig.PL_PAD_SUBJECT;
   PL_PAD_STOP_AIM      = gBP_PadConfig.PL_PAD_STOP_AIM;
   PL_PAD_SQUAT         = gBP_PadConfig.PL_PAD_SQUAT;
   PL_PAD_WEAPON        = gBP_PadConfig.PL_PAD_WEAPON;
   PL_PAD_PRESS_WEAPON  = gBP_PadConfig.PL_PAD_PRESS_WEAPON;
   PL_PAD_PUNCH         = gBP_PadConfig.PL_PAD_PUNCH;
   PL_PAD_CHANGE_WEAPON = gBP_PadConfig.PL_PAD_CHANGE_WEAPON;
   PL_PAD_PEEP          = gBP_PadConfig.PL_PAD_PEEP;
   PL_PAD_PEEP_R        = gBP_PadConfig.PL_PAD_PEEP_R;
   PL_PAD_PEEP_L        = gBP_PadConfig.PL_PAD_PEEP_L;
   PL_PAD_PRESS_PEEP_R  = gBP_PadConfig.PL_PAD_PRESS_PEEP_R;
   PL_PAD_PRESS_PEEP_L  = gBP_PadConfig.PL_PAD_PRESS_PEEP_L;
   PL_PAD_CAPUTRE       = gBP_PadConfig.PL_PAD_CAPUTRE;
   PL_PAD_LOCKON        = gBP_PadConfig.PL_PAD_LOCKON;
   PL_PAD_KNOCK         = gBP_PadConfig.PL_PAD_KNOCK;
   PL_PAD_HANG          = gBP_PadConfig.PL_PAD_HANG;
   PL_PAD_BEYOND        = gBP_PadConfig.PL_PAD_BEYOND;
   PL_PAD_READY_RUN     = gBP_PadConfig.PL_PAD_READY_RUN;
   PL_PAD_BEYOND_DOWN   = gBP_PadConfig.PL_PAD_BEYOND_DOWN;
   PL_PAD_LOCKER_ZOOM   = gBP_PadConfig.PL_PAD_LOCKER_ZOOM;
   PL_PAD_PRESS_LOCKER  = gBP_PadConfig.PL_PAD_PRESS_LOCKER;
   PL_PAD_ZOOMIN        = gBP_PadConfig.PL_PAD_ZOOMIN;
   PL_PAD_PRESS_ZOOMIN  = gBP_PadConfig.PL_PAD_PRESS_ZOOMIN;
   PL_PAD_ZOOMOUT       = gBP_PadConfig.PL_PAD_ZOOMOUT;
   PL_PAD_PRESS_ZOOMOUT = gBP_PadConfig.PL_PAD_PRESS_ZOOMOUT;
   PL_PAD_ITEMUSE       = gBP_PadConfig.PL_PAD_ITEMUSE;
}

void BP_OverridePadConfig()
{
   PL_PAD_WEAPON        = PAD_R1;                              // was PS2_PAD_SQUARE
   PL_PAD_PRESS_WEAPON  = PAD_PRESS_R1;                        // was PS2_PAD_PRESS_SQUARE

   PL_PAD_SUBJECT       = PAD_L1;                              // was PAD_R1
   PL_PAD_LOCKER_ZOOM   = PAD_L1;                              // was PAD_R1
   PL_PAD_PRESS_LOCKER  = PAD_PRESS_L1;                        // was PAD_PRESS_R1

   PL_PAD_LOCKON        = PS2_PAD_SQUARE;                      // was PAD_L1
   PL_PAD_READY_RUN     = (PS2_PAD_CROSS | PS2_PAD_SQUARE);    // was (PS2_PAD_CROSS | PAD_L1)
}

int gBP_Controls_Override = 0;

int BP_DebugMenu_ActionCallback_OverrideControls(int data)
{
   if( gBP_Controls_Override )
   {
      BP_SavePadConfig();
   }
   else
   {
      BP_RestorePadConfig();
   }

   extern void PL_PadSetPatternA( void );
   PL_PadSetPatternA();

   return 0;
}

extern "C"
{
   int gBP_NikitaMissilesCheat = 0;
   int gBP_ShowCeilingsCheat = 0;
};

#endif   //#if MGS_VERSION == 2

//----------------------------------------------------------------------------

int gBP_InvincibleMode = 0;

#if MGS_VERSION == 3

int gBP_HUD_DisableGauges = 0;

int BP_DebugMenu_Callback_ChangePlayerDebugState(int data)
{
   if( gBP_HUD_DisableGauges )
      PL_DebugMenuStatus |= PL_DBG_S_GRIP_OFF|PL_DBG_S_O2_OFF|PL_DBG_S_GAUGE_OFF|PL_DBG_S_HUNGER_OFF|PL_DBG_S_STAMINA_OFF|PL_DBG_S_ELUDE_OFF;
   else
      PL_DebugMenuStatus &= ~(PL_DBG_S_GRIP_OFF|PL_DBG_S_O2_OFF|PL_DBG_S_GAUGE_OFF|PL_DBG_S_HUNGER_OFF|PL_DBG_S_STAMINA_OFF|PL_DBG_S_ELUDE_OFF);

   PL_DebugMenuStatus &= ~(PL_DBG_S_MUTEKIMODE1|PL_DBG_S_MUTEKIMODE2);

   switch(gBP_InvincibleMode)
   {
   case 0: //Off
      break;

   case 1: //Mode1
      PL_DebugMenuStatus |= PL_DBG_S_MUTEKIMODE1;
      break;

   case 2: //Mode2:
      PL_DebugMenuStatus |= PL_DBG_S_MUTEKIMODE2;
      break;

   case 3: //Mode1&2
      PL_DebugMenuStatus |= PL_DBG_S_MUTEKIMODE1|PL_DBG_S_MUTEKIMODE2;
      break;
   }

   return 0;
}

#elif MGS_VERSION == 2 

extern "C" int PlayerDebugMenuStatus;
int gBP_HUD_DisableRadar = 0;
int gBP_HUD_Disable = 0;
extern "C" int gBP_HUD_DisableTimer = 0;

int BP_DebugMenu_Callback_ChangePlayerDebugState(int data)
{
   if( gBP_HUD_DisableRadar )
      GM_Configuration |= GM_CONFIG_RADAR_OFF;
   else
      GM_Configuration &= ~GM_CONFIG_RADAR_OFF;

   PlayerDebugMenuStatus &= ~PDMS_DISPLAY_OFF;
   
   if( gBP_HUD_Disable )
      PlayerDebugMenuStatus |= PDMS_DISPLAY_OFF;

   PlayerDebugMenuStatus &= ~(PDMS_MUTEKIMODE1|PDMS_MUTEKIMODE2);

   switch(gBP_InvincibleMode)
   {
   case 0: //Off
      break;

   case 1: //Mode1
      PlayerDebugMenuStatus |= PDMS_MUTEKIMODE1;
      break;

   case 2: //Mode2:
      PlayerDebugMenuStatus |= PDMS_MUTEKIMODE2;
      break;
   }

   return 0;
}

#endif

unsigned char gBP_ValidLanguageTable_USA[] = 
{
   0,//kPre

   1,//kL_English
   0,//kL_Japanese
   1,//kL_French
   0,//kL_German
   0,//kL_Italian
   0,//kL_Portuguese
   1,//kL_Spanish
   0,//kL_Dutch
   0,//kL_Russian

   0,//kPost
};
unsigned char gBP_ValidLanguageTable_Europe[] = 
{
   0,//kPre

   1,//kL_English
   0,//kL_Japanese
   1,//kL_French
   1,//kL_German
   1,//kL_Italian
   0,//kL_Portuguese
   1,//kL_Spanish
   0,//kL_Dutch
   0,//kL_Russian

   0,//kPost
};
unsigned char gBP_ValidLanguageTable_Japan[] = 
{
   0,//kPre

   0,//kL_English
   1,//kL_Japanese
   0,//kL_French
   0,//kL_German
   0,//kL_Italian
   0,//kL_Portuguese
   0,//kL_Spanish
   0,//kL_Dutch
   0,//kL_Russian

   0,//kPost
};
int gBP_DebugMenuLanguage = 0;
int gBP_DebugMenuLangaugeLast = 0;
// This is the same function for MGS2 & 3
extern "C" int NewGclLangUpdate( void );
int BP_DebugMenu_Callback_ChangeLanguage(int data)
{
   // figure out what value we should really be based on sku
   int directionPos = gBP_DebugMenuLanguage - gBP_DebugMenuLangaugeLast > 0 ? 1 : -1;
   unsigned char* pValidTable = NULL;
   switch( gpOsContext->mBuildSKU )
   {
   case CBaseOsContext::kBS_USA:
      {
         pValidTable = gBP_ValidLanguageTable_USA;
      }
      break;
   case CBaseOsContext::kBS_Japan:
      {
         pValidTable = gBP_ValidLanguageTable_Japan;
      }
      break;
   case CBaseOsContext::kBS_Europe:
      {
         pValidTable = gBP_ValidLanguageTable_Europe;
      }
      break;
   }
   while( pValidTable[gBP_DebugMenuLanguage] == 0 )
   {
      gBP_DebugMenuLanguage += directionPos;
      if( gBP_DebugMenuLanguage < 0 )
      {
         gBP_DebugMenuLanguage = (CBaseOsContext::kL_Count-1)+2;
      }
      else if( gBP_DebugMenuLanguage > (CBaseOsContext::kL_Count-1)+2 )
      {
         gBP_DebugMenuLanguage = 0;
      }
   }

   gBP_DebugMenuLangaugeLast = gBP_DebugMenuLanguage;
   gpOsContext->mLanguage = CBaseOsContext::ELanguage(gBP_DebugMenuLanguage-1);//we have extra Pre value

   // This is the same function for MGS2 & 3
   NewGclLangUpdate();

   return 0;
}

//----------------------------------------------------------------------------

#if BP_ENABLE_DEBUG_PRIM
extern "C" int BP_DebugNav_ShowPath;
extern "C" int BP_DebugNav_DepthTest;
#endif

#if MGS_VERSION==3
extern "C" int BP_MGSavePointOverride;
#endif

#if BP_360
extern int gBP_X360ShowRichPresence;
#endif

#if MGS_VERSION==3
extern "C" int BP_MGSavePointOverride;
#if BP_PS3
extern "C" int gBP_SVM_Model_HourOverride;
#endif
#endif //MGS_VERSION==3

//----------------------------------------------------------

void BP_InitGameDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   // Stage options
   {
      int const stageMenu = BP_DebugMenu_AddMenu("Stage", -1);
      BP_DebugMenu_AddAction(stageMenu, "Restart...", &BP_DebugStageActionCallback, 0);
      BP_DebugMenu_AddAction(stageMenu, "Reload...", &BP_DebugStageActionCallback, 1);
      BP_DebugMenu_AddAction(stageMenu, "Jump to 'select'...", &BP_DebugStageActionCallback, 2);
   }

#if MGS_VERSION == 2
   // Game play
   {
      int const gameMenu = BP_DebugMenu_AddMenu("Gameplay", -1);
      BP_DebugMenu_AddBool(gameMenu, "Grenade - Explode In Hand - Enable", &gBP_Game_GrenadeExplodeInHand_Enable);
      BP_DebugMenu_AddInt(gameMenu, "Grenade - Blast - Inner Range", &gBP_Game_GrenadeBlast_InnerRange, 0, 50000, 100, 1000);
      BP_DebugMenu_AddInt(gameMenu, "Grenade - Blast - Outer Range", &gBP_Game_GrenadeBlast_OuterRange, 0, 50000, 100, 1000);

      BP_DebugMenu_AddSeparator(gameMenu);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Life - VERY EASY", &gBP_Game_SolidusChoke_Life[kGameDifficulty_VERYEASY],  0, 200, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Life -      EASY", &gBP_Game_SolidusChoke_Life[kGameDifficulty_EASY],      0, 200, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Life -    NORMAL", &gBP_Game_SolidusChoke_Life[kGameDifficulty_NORMAL],    0, 200, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Life -      HARD", &gBP_Game_SolidusChoke_Life[kGameDifficulty_HARD],      0, 200, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Life -   EXTREME", &gBP_Game_SolidusChoke_Life[kGameDifficulty_EXTREME],   0, 200, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Life - E_EXTREME", &gBP_Game_SolidusChoke_Life[kGameDifficulty_E_EXTREME], 0, 200, 1, 10);

      BP_DebugMenu_AddSeparator(gameMenu);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Time - VERY EASY", &gBP_Game_SolidusChoke_Time[kGameDifficulty_VERYEASY],  0, 3000, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Time -      EASY", &gBP_Game_SolidusChoke_Time[kGameDifficulty_EASY],      0, 3000, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Time -    NORMAL", &gBP_Game_SolidusChoke_Time[kGameDifficulty_NORMAL],    0, 3000, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Time -      HARD", &gBP_Game_SolidusChoke_Time[kGameDifficulty_HARD],      0, 3000, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Time -   EXTREME", &gBP_Game_SolidusChoke_Time[kGameDifficulty_EXTREME],   0, 3000, 1, 10);
      BP_DebugMenu_AddInt(gameMenu, "SolidusChokeBoss - Time - E_EXTREME", &gBP_Game_SolidusChoke_Time[kGameDifficulty_E_EXTREME], 0, 3000, 1, 10);
   }
#endif

#if MGS_VERSION == 2
   // Controls
   {
      int const controlsMenu = BP_DebugMenu_AddMenu("Controls", -1);
      static const char* gBP_Input_X360Remap_DemoZoomMethod_Enum[] = { "Hold RB = Zoom Mode, RT = Zoom Amount", "RT = Zoom Amount" };
      static const char* gBP_Input_X360Remap_QuickHolsterMethod_Enum[] = { "Push stick beyond threshold", "Push forward with time threshold" };

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_AddBool(controlsMenu, "Squat To Ground - Enable", &gBP_Input_HoldSquatToGround_Enable);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddInt(controlsMenu, "Squat To Ground - Hold Frames", &PL_SQUAT_TO_GROUND_FRAMES, 1, 120, 1, 5 ), &gBP_Input_HoldSquatToGround_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_AddBool(controlsMenu, "Enable X360 remap", &gBP_Input_X360Remap_Enable);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddEnum(controlsMenu, "Cutscene camera zoom method", gBP_Input_X360Remap_DemoZoomMethod_Enum, &gBP_Input_X360Remap_DemoZoomMethod, 0, 1), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Locker Zoom - Fast   - Stick Threshold", &PL_PAD_ZOOM_LOCKER_FAST_TH,   0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Locker Zoom - Medium - Stick Threshold", &PL_PAD_ZOOM_LOCKER_MEDIUM_TH, 0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      
      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Locker Zoom - Fast   - Speed", &PL_ZOOM_LOCKER_FAST_SPEED,   0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Locker Zoom - Medium - Speed", &PL_ZOOM_LOCKER_MEDIUM_SPEED, 0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Locker Zoom - Slow   - Speed", &PL_ZOOM_LOCKER_SLOW_SPEED,   0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Spray - Stick Y - Min Threshold", &PL_PAD_SPRAY_MIN_TH, 0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Spray - Stick Y - Max Threshold", &PL_PAD_SPRAY_MAX_TH, 0.0f, 1.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Scope/PGS1/PGS1-T - Zoom In Speed", &PL_SCOPE_ZOOM_IN_SPEED, 0.0f, 10.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Scope/PGS1/PGS1-T - Zoom Out Speed", &PL_SCOPE_ZOOM_OUT_SPEED, 0.0f, 10.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddEnum(controlsMenu, "Quick holster - Method", gBP_Input_X360Remap_QuickHolsterMethod_Enum, &gBP_Input_X360Remap_QuickHolsterMethod, 0, 1), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Quick holster - Stick Magnitude Threshold", &PL_PAD_QUICK_HOLSTER_STICK_MAG_TH, 0.0f, 1.0f, 0.001f, 0.01f ), &gBP_Input_X360Remap_QuickHolsterMethodEnable[0] );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Quick holster - Stick Y Threshold", &PL_PAD_QUICK_HOLSTER_STICK_Y_TH, 0.0f, 1.0f, 0.001f, 0.01f ), &gBP_Input_X360Remap_QuickHolsterMethodEnable[1] );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Quick holster - Stick X Threshold", &PL_PAD_QUICK_HOLSTER_STICK_X_TH, 0.0f, 1.0f, 0.001f, 0.01f ), &gBP_Input_X360Remap_QuickHolsterMethodEnable[1] );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "Quick holster - Time Threshold",    &PL_PAD_QUICK_HOLSTER_TIME_TH,    0.0f, 1.0f, 0.001f, 0.01f ), &gBP_Input_X360Remap_QuickHolsterMethodEnable[1] );

      BP_DebugMenu_AddSeparator(controlsMenu);
#ifdef BP_VITA
      BP_DebugMenu_SetCallback(BP_DebugMenu_AddEnum(controlsMenu, "Vita Control Scheme", gBP_VitaControlsEnum, &gBP_UseVitaControlOverrides, 0, 2), &BP_DebugMenu_ActionCallback_OverrideControls, 0);
#endif
      BP_DebugMenu_AddBool(controlsMenu, "Use Front Touch For Zoom", &gBP_UseFrontTouchForZooming);
      BP_DebugMenu_AddBool(controlsMenu, "Joystick Zoom Style", &gBP_TouchZoomLikeJoystick);

// SBroumley - 7/7/2011 - removing 3rd person camera prototype options as requested by Andy since we aren't going to implement
#if 0
      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_AddBool(controlsMenu, "Inherit camera rotation", &gBP_Camera_InheritRot);
      BP_DebugMenu_SetCallback( BP_DebugMenu_AddBool(controlsMenu, "Override controls", &gBP_Controls_Override), &BP_DebugMenu_ActionCallback_OverrideControls, 0);

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_AddBool(controlsMenu, "Override 1st person camera", &gBP_1stPersonCamera_Override);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddEnum(controlsMenu, "- move mode", PL_SubjectMove_Enum, &gBP_1stPersonCamera_Move, 0, 3), &gBP_1stPersonCamera_Override);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddBool(controlsMenu, "- toggle mode", &gBP_1stPersonCamera_Toggle), &gBP_1stPersonCamera_Override);

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_AddBool(controlsMenu, "Override 3rd person camera", &gBP_3rdPersonCamera_Override);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddInt(controlsMenu, "- distance", &gBP_3rdPersonCamera_Dist, 0, 10000, 10, 100 ), &gBP_3rdPersonCamera_Override);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "- horizontal rot speed", &gBP_3rdPersonCamera_HSpeed, 0, 10.0f, 0.1f, 1.0f ), &gBP_3rdPersonCamera_Override);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "- vertical rot speed", &gBP_3rdPersonCamera_VSpeed, 0, 10.0f, 0.1f, 1.0f ), &gBP_3rdPersonCamera_Override);
#endif
   }
#endif


   // Controls
#if MGS_VERSION == 3
   {
      int const controlsMenu = BP_DebugMenu_AddMenu("Controls", -1);

#ifdef BP_VITA
      BP_DebugMenu_AddEnum(controlsMenu, "Vita Control Scheme", gBP_VitaControlsEnum, &gBP_UseVitaControlOverrides, 0, 2);
      BP_DebugMenu_AddEnum(controlsMenu, "Vita Scope Toggle Mode", gBP_VitaScopeToggleModeEnum, &gBP_VitaScopeToggleMode, 0, 2);
      BP_DebugMenu_AddBool(controlsMenu, "Vita Flick to Knife Stab", &gBP_VitaUseFlickForStab);
#endif

      BP_DebugMenu_AddBool(controlsMenu, "Enable X360 remap", &gBP_Input_X360Remap_Enable);
      BP_DebugMenu_AddBool(controlsMenu, "Show button pressures", &gBP_Input_ShowPressure);
      BP_DebugMenu_AddBool(controlsMenu, "Use Front Touch For Zoom", &gBP_UseFrontTouchForZooming);
      BP_DebugMenu_AddBool(controlsMenu, "Use Joystick Style Zooming", &gBP_UseJoystickStyleScopeZooming);
#ifdef BP_VITA
      BP_DebugMenu_AddBool(controlsMenu, "Double tap L to strafe", &gBP_VitaStrafeDoubleClick);
      BP_DebugMenu_AddBool(controlsMenu, "Auto draw weapon when strafing", &gBP_VitaAutoDrawWeaponWhenStrafing);
#endif

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_AddInt(controlsMenu, "Locked move side anim angle", &PL_PAD_PARA_MOVE_SIDE_ANGLE, 0, 90, 1, 5);

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddInt(controlsMenu, "Locker Zoom - Fast - Speed", &PL_PAD_ZOOM_LOCKER_FAST_SPEED, 0, 255, 1, 10 ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddInt(controlsMenu, "Locker Zoom - Slow - Speed", &PL_PAD_ZOOM_LOCKER_SLOW_SPEED, 0, 255, 1, 10 ), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "PL_PAD_CQC_SLIT_THROAT_TIME",      &PL_PAD_CQC_SLIT_THROAT_TIME,    0.0f, 5.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "PL_PAD_CQC_DOUBLE_CLICK_TIME",     &PL_PAD_CQC_DOUBLE_CLICK_TIME,   0.0f, 5.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );

      BP_DebugMenu_AddSeparator(controlsMenu);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "PL_PAD_GRENADE_LONG_THROW_TIME",   &PL_PAD_GRENADE_LONG_THROW_TIME, 0.0f, 5.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "PL_PAD_SONAR_PING_TIME",           &PL_PAD_SONAR_PING_TIME,         0.0f, 5.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddFloat(controlsMenu, "PL_PAD_KNIFE_STAB_TIME",           &PL_PAD_KNIFE_STAB_TIME,         0.0f, 5.0f, 0.01f, 0.1f ), &gBP_Input_X360Remap_Enable );
   }
#endif

   // HUD options
   {
      int const hudMenu = BP_DebugMenu_AddMenu("UI", -1);

#if MGS_VERSION == 3
      BP_DebugMenu_AddBool(hudMenu, "Disable EQMenu", &EQMENU_DebugFlag);
      BP_DebugMenu_SetCallback(BP_DebugMenu_AddBool(hudMenu, "Disable Gauges", &gBP_HUD_DisableGauges), &BP_DebugMenu_Callback_ChangePlayerDebugState, 0);

      BP_DebugMenu_AddBool(hudMenu, "Disable Tex 2D", &gBP_DisableTex2D);
      BP_DebugMenu_AddBool(hudMenu, "Disable Layout Anime Demo", &gBP_DisableLayoutAnimeDemo);
#else
      BP_DebugMenu_SetCallback(BP_DebugMenu_AddBool(hudMenu, "Disable HUD", &gBP_HUD_Disable), &BP_DebugMenu_Callback_ChangePlayerDebugState, 0);
      BP_DebugMenu_SetCallback(BP_DebugMenu_AddBool(hudMenu, "Disable Radar", &gBP_HUD_DisableRadar), &BP_DebugMenu_Callback_ChangePlayerDebugState, 0);

      BP_DebugMenu_AddBool(hudMenu, "Disable Timer", &gBP_HUD_DisableTimer);
      BP_DebugMenu_AddBool(hudMenu, "Radar Offscreen", &gBP_RadarRenderOffscreen);
#endif

#if defined(BP_VITA)
      BP_DebugMenu_SetCallback(BP_DebugMenu_AddBool(hudMenu, "Flip Weapon/Item Lists", &gBP_FlipWeaponItemLists), 
         &BP_DebugMenu_Callback_UpdateWeaponItemPanels, 0);
#endif
   }

#if MGS_VERSION == 3
   // Navigation
   {
#if BP_ENABLE_DEBUG_PRIM
      int const navMenu = BP_DebugMenu_AddMenu("Navigation", -1);
      BP_DebugMenu_AddBool(navMenu, "Show path", &BP_DebugNav_ShowPath);
      BP_DebugMenu_AddBool(navMenu, "Use Depth Test", &BP_DebugNav_DepthTest);
#endif
   }
#endif

   BP_SoundSupport_InitDebugMenu();

   BP_DebugMenu_AddInt(-1, "Cinematic FF Speed", &gBP_DirectOutputStreamSpeed, 1, 8, 1, 1);

   int const vibMenu = BP_DebugMenu_AddMenu("Vibration", -1);
   {
      BP_DebugMenu_AddFloat(vibMenu, "Large Motor Min", &gBP_VibrationLgMin, 0.f, 1.f, 0.01f, 0.1f);
      BP_DebugMenu_AddFloat(vibMenu, "Large Motor Max", &gBP_VibrationLgMax, 0.f, 1.f, 0.01f, 0.1f);
      BP_DebugMenu_AddFloat(vibMenu, "Large Motor Threshold", &gBP_VibrationLgThreshold, 0.f, 1.f, 0.01f, 0.1f);
#ifdef BP_360
      BP_DebugMenu_AddSeparator(vibMenu);
      BP_DebugMenu_AddFloat(vibMenu, "Small Motor Scale", &gBP_VibrationSmScale, 0.f, 1.f, 0.01f, 0.1f);
#endif
   }

   int const trophyMenu = BP_DebugMenu_AddMenu("Trophy", -1);
   {
      BP_DebugMenu_AddBool(trophyMenu, "Enable Trophy Debug", &gBP_TrophyDebug);
      static char const * trophyNames[kTRP_Count - 1]; // exclude Platinum
      for( int i=1; i < kTRP_Count; i++ )
      {
         trophyNames[i-1] = BP_TrophySystem_GetDebugString( (ETrophies)i );
      }
      BP_DebugMenu_AddEnum(trophyMenu, "Trophy to Unlock", trophyNames, &gBP_DebugUnlockTrophy, 0, kTRP_Count - 2);
      BP_DebugMenu_AddAction(trophyMenu, "Unlock Selected Trophy", BP_Debug_TrophySystem_UnlockTrophy, 0);
      BP_DebugMenu_AddAction(trophyMenu, "Unlock All Trophies", BP_Debug_TrophySystem_UnlockAllTrophies, 0);
   }
   BP_DebugMenu_AddBool(-1, "Show Loading Spinner State", &gBP_LoadingSpinnerDebug);

   BP_DebugMenu_AddBool(-1, "Show Debug Menu Background", &gBP_ShowDebugMenuBlackBackground);
   BP_DebugMenu_AddBool(-1, "Debug Menu Active", &gBP_DebugMenuIsActive);

#if MGS_VERSION==3
   // Script data debugging
   {
      int scriptMenu = BP_DebugMenu_AddMenu("GCL Scripts", -1);
      BP_DebugMenu_AddBool( scriptMenu, "Spew Var References", &gBP_SpewVarReferences );
      BP_DebugMenu_AddBool( scriptMenu, "Break on missing Var Code", &gBP_BreakOnMissingVarCodeMapping );
   }
#endif

   // Cheats
   {
      int cheatsMenu = BP_DebugMenu_AddMenu("Cheats", -1);

#if MGS_VERSION == 3
      BP_DebugMenu_AddBool(cheatsMenu, "Enemy Sight Disabled", &ENE_DBG_EyeClose);
      BP_DebugMenu_AddBool(cheatsMenu, "Enemy Hearing Disabled", &ENE_DBG_EarClose);

      static const char* invincibleModes[] = { "Off", "Mode 1 (Disable Damage & Reaction)", "Mode 2 (Disable Damage)", "Mode 1 & 2 (?)" };
      int invincibleEntry = BP_DebugMenu_AddEnum(cheatsMenu, "Invincible Mode", invincibleModes, &gBP_InvincibleMode, 0, 3 );
      BP_DebugMenu_SetCallback(invincibleEntry, &BP_DebugMenu_Callback_ChangePlayerDebugState, 0);

      BP_DebugMenu_AddBool( cheatsMenu, "Show Unlockable Camo", &gBP_ForceEnableDownCamo );

#elif MGS_VERSION==2
      static const char* sightHearMode[] = { "Default", "Enabled", "Disabled" };
      BP_DebugMenu_AddEnum(cheatsMenu, "Enemy Sight Mode", sightHearMode, &gBP_EnemySightDisabled, -1, 1);
      BP_DebugMenu_AddEnum(cheatsMenu, "Enemy Hearing Mode", sightHearMode, &gBP_EnemyHearingDisabled, -1, 1);

      static const char* invincibleModes[] = { "Off", "Mode 1 (No damage and reaction)", "Mode 2 (No damage)" };
      int invincibleEntry = BP_DebugMenu_AddEnum(cheatsMenu, "Invincible Mode", invincibleModes, &gBP_InvincibleMode, 0, 2 );
      BP_DebugMenu_SetCallback(invincibleEntry, &BP_DebugMenu_Callback_ChangePlayerDebugState, 0);

      static const char *skNikitaModes[] = { "Normal", "No time limit", "No speedup", "No time limit & no speedup" };
      BP_DebugMenu_AddEnum(cheatsMenu, "Nikita missiles", skNikitaModes, &gBP_NikitaMissilesCheat, 0, 3);

      BP_DebugMenu_AddBool(cheatsMenu, "Show Ceilings", &gBP_ShowCeilingsCheat);
#endif

      BP_DebugMenu_AddBool(cheatsMenu, "Damage Current Boss", &gBP_DamageCurrentBoss);
      BP_DebugMenu_AddBool(cheatsMenu, "Kill Current Boss", &gBP_KillCurrentBoss);
   }

   // KP requested options
   {
      int KP_cheatsMenu = BP_DebugMenu_AddMenu("KP_Cheats", -1);
#if MGS_VERSION==2
      static const char* optCmfModes[] = { "Normal", "Movie Green", "Movie Invisible" };
      int invincibleEntry = BP_DebugMenu_AddEnum(KP_cheatsMenu, "OptCmfMovieMode Mode", optCmfModes, &gBP_KP_Cheat_OptCmfMode, 0, 2 );
#endif
#if BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET == BPE_TARGET_X360
      BP_DebugMenu_AddBool(KP_cheatsMenu, "Disable Application Pause", &gBP_DisableApplicationPause);
#endif
   }

   // Misc
   {
      int const miscMenu = BP_DebugMenu_AddMenu("Misc", -1);
      static const char* languageModes[] = { "Pre", "English", "Japanese", "French", "German", "Italian", "Portuguese", "Spanish", "Dutch", "Russian", "Post" };
      int enumMenuEntry_Language = BP_DebugMenu_AddEnum(miscMenu, "Language", languageModes, &gBP_DebugMenuLanguage, 0, (CBaseOsContext::kL_Count-1)+2 );
      gBP_DebugMenuLangaugeLast = gBP_DebugMenuLanguage = gpOsContext->mLanguage+1;
      BP_DebugMenu_SetCallback( enumMenuEntry_Language, &BP_DebugMenu_Callback_ChangeLanguage, 0 );
      // X360 specific
#if BP_360
      {
         BP_DebugMenu_AddBool(miscMenu, "Show RichPresence", &gBP_X360ShowRichPresence);

         int const saveLoadMenu = BP_DebugMenu_AddMenu("SaveLoad", miscMenu);
         BP_DebugMenu_AddBool(saveLoadMenu, "Disable free space check on device select", &gBP_DisableMemoryCardSizeCheck);
      }
#endif

#if MGS_VERSION==3
      BP_DebugMenu_AddInt(miscMenu, "MG Save Point Override", &BP_MGSavePointOverride, -1, 0x7f, 1, 1);
#if BP_PS3
      BP_DebugMenu_AddInt(miscMenu, "SVM Model Hour Override", &gBP_SVM_Model_HourOverride, -1, 23, 1, 1);
      BP_DebugMenu_AddBool(miscMenu, "Disable Nightmare Sequence", &gBP_ForceDisableTorture);
#endif
#endif //MGS_VERSION==3

      int const transfarringMenu = BP_DebugMenu_AddMenu("Transfarring", miscMenu);
      {
         BP_DebugMenu_AddBool(transfarringMenu, "Make Bad Cloud Data", &gBP_MakeBadCloudData);

         static char const* errorTUS[] = 
         { 
            "No Override", 
            "Authentication Error", 
            "No Such Title",
            "Service Not Started",
            "Service Ended",
            "Service Maintenance",
         };

         BP_DebugMenu_AddEnum(transfarringMenu, "Force Fail GetSlotInfos", errorTUS, &gBP_Transfarring_FailGetSlotInfos, 0, 5);
         BP_DebugMenu_AddEnum(transfarringMenu, "Force Fail GetSlotData", errorTUS, &gBP_Transfarring_FailGetSlotData, 0, 5);
         BP_DebugMenu_AddEnum(transfarringMenu, "Force Fail SetSlotData", errorTUS, &gBP_Transfarring_FailSetSlotData, 0, 5);
         BP_DebugMenu_AddEnum(transfarringMenu, "Force Fail DeleteSlotData", errorTUS, &gBP_Transfarring_FailDeleteSlotData, 0, 5);
         BP_DebugMenu_AddEnum(transfarringMenu, "Force Fail IncrementSlotVar", errorTUS, &gBP_Transfarring_FailIncrementSlotVariable, 0, 5);
      }
   }

   BP_InitStreamDebugMenu();
#endif
}

//----------------------------------------------------------------------------

void BP_Debug_BeginFrame()
{  
#if MGS_VERSION == 2
   // Call every frame since game resets config in between rooms
   if( gBP_Controls_Override )
   {
      BP_OverridePadConfig();
   }

   // Enable/disable menu items
   gBP_Input_X360Remap_QuickHolsterMethodEnable[0] = gBP_Input_X360Remap_Enable && (gBP_Input_X360Remap_QuickHolsterMethod == 0);
   gBP_Input_X360Remap_QuickHolsterMethodEnable[1] = gBP_Input_X360Remap_Enable && (gBP_Input_X360Remap_QuickHolsterMethod == 1);

#endif   //#if MGS_VERSION == 2
}

//----------------------------------------------------------------------------

int DEBUG_PRESSURE_X = 64;
int DEBUG_PRESSURE_Y = 232;

void BP_Debug_EndFrame()
{
#if MGS_VERSION == 3
   // Draw input pressure values?
   if( gBP_Input_ShowPressure )
   {
      // End previous shader
      BP_EndShader();

      // Draw non zero pressures
      int x = DEBUG_PRESSURE_X;
      int y = DEBUG_PRESSURE_Y;
      GV_PAD* pad = PlayerPad.enable ? &PlayerPad.pad : GV_PadData( 0 );
      for( int i = 0; i < GV_PAD_MAX_PRESSURE; i++ )
      {
         if( pad->pressure[i] )
         {
            BP_Debug_DrawString( CStringExtras::Stringize_s( "Pad.pressure[%d] = %d", i, pad->pressure[i] ), &x, &y );
         }
      }

      // End previous shader
      BP_EndShader();
   }
#endif
}

//----------------------------------------------------------------------------
extern "C" int BP_GetTweakValueFromDifficulty( const int* values )
{
   // Lookup game difficulty
   const int difficulty = GM_GameLevel;

   // Lookup value
   if( difficulty >= GM_LEVEL_E_EXTREME )
   {
      return values[kGameDifficulty_E_EXTREME];
   }
   else if( difficulty >= GM_LEVEL_EXTREME )
   {
      return values[kGameDifficulty_EXTREME];
   }
   else if( difficulty >= GM_LEVEL_HARD )
   {
      return values[kGameDifficulty_HARD];
   }
   else if( difficulty >= GM_LEVEL_NORMAL )
   {
      return values[kGameDifficulty_NORMAL];
   }
   else if( difficulty >= GM_LEVEL_EASY )
   {
      return values[kGameDifficulty_EASY];
   }
   else
   {
      return values[kGameDifficulty_VERYEASY];
   }
}

//----------------------------------------------------------------------------

extern "C" int BP_GCL_SetVar_Override( unsigned int code, int value )
{
#if MGS_VERSION == 2
   switch( code )
   {
      // "w51a" Solidus Choke boss duration time?
      // \CP4\dev\MGS2\mgs2x\scn_utf8\varinit.h
      // ????????????????
      // eval( $w:w51a_???????? = (60*10) );
      case 0x11000334:
      {
         // Override
         value = BP_GetTweakValueFromDifficulty( gBP_Game_SolidusChoke_Time );
      }
      break;
   }
#endif

   return value;
}

//----------------------------------------------------------------------------

extern "C" void BP_UpdatePlayerPadRemap()
{  
#if MGS_VERSION == 2
   // Disable remap if playing a paddemo cinema so playback is correct
   if( GM_GameStatus & STATE_PAD_DEMO )
   {
      PlayerPad.enable = 0;
   }
   else
   {
      PlayerPad.enable = gBP_Input_X360Remap_Enable;
   }
#endif

#if MGS_VERSION == 3
   // Disable remap if playing a paddemo cinema so playback is correct
   if( GM_CheckGameStatus( STATE_PAD_DEMO ) )
   {
      PlayerPad.enable = 0;
   }
   else
   {
      PlayerPad.enable = gBP_Input_X360Remap_Enable;
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Debug_PushCPUMarker( char const *name )
{
#if BP_VITA && !defined(GOLD_VERSION)
   sceRazorCpuPushMarkerWithHud( name, 0, SCE_RAZOR_MARKER_ENABLE_HUD );
#endif
}

//----------------------------------------------------------------------------

void BP_Debug_PopCPUMarker()
{
#if BP_VITA && !defined(GOLD_VERSION)
   sceRazorCpuPopMarker();
#endif
}

//----------------------------------------------------------------------------
#if BP_ENABLE_TESTNODE
void BP_Debug_BeginTestNode()
{
   gTestNodeCur = 0;
}

int BP_Debug_EvalTestNode()
{
   gTestNodeCur++;
   if (gTestNodeEnabled)
   {
      if (gTestNodeCur >= gTestNodeStart && gTestNodeCur <= gTestNodeEnd)
         return true;
      else
         return false;
   }
   else
      return true;
}

void BP_Debug_EndTestNode()
{
   gTestNodeCount = gTestNodeCur;
}

#endif
#if BP_ENABLE_TESTTRI
const void *BP_Debug_EvalTestTri(int is32, const void *indices, uint32_t *indexCount)
{
   const void *d;
   if (gTestTriActive)
   {
      int ntris = *indexCount / 3;
      int first, start, end;
      
      first = start = gTestTriCur;
      end = gTestTriCur + ntris - 1;
      gTestTriCur += ntris;

      if (start > gTestTriEnd)
         return NULL;
      else if (end < gTestTriStart)
         return NULL;
      else if (start >= gTestTriStart && end <= gTestTriEnd) // no clipping needed
         return indices;
      else
      {
         // index buffer intersects the range, figure out the new start/end
         if (start < gTestTriStart)
            start = gTestTriStart;
         if (end > gTestTriEnd)
            end = gTestTriEnd;
         if (end < start)
            return NULL;
         *indexCount = (end - start + 1) * 3;
         d = indices;
         if (start > first)
         {
            int skip = start - first;
            unsigned long b = (unsigned long)d;
            if (is32)
               b += skip * 4 * 3;
            else
               b += skip * 2 * 3;
            d = (const void *)b;
         }
         return d;
      }
   }
   else
   {
      gTestTriCur += *indexCount / 3;
      return indices;
   }
}
#endif
//----------------------------------------------------------------------------
#if JADEBUG || BP_PRIM_DEBUGGING
#include "Renderer/base/Primitive/CMesh.h"

extern "C" char *BP_SplatBasename(char *dst, const char *src)
{
   const char *last = NULL;
   const char *p = src;

   while (*p)
   {
      if (*p == '/' || *p == '\\')
         last = p;
      p++;
   }

   if (last)
   {
      last++;
      memcpy(dst, last, p-last);
      dst[p-last] = 0;
      return dst + (p-last);
   }
   else
   {
      memcpy(dst, src, p-src);
      dst[p-src] = 0;
      return dst + (p-src);
   }
}
#endif

#if JADEBUG

extern "C" {

struct _BPModelProfile
{
   unsigned int model;
   int nmodel;
   int nmesh;
   int nunit;
   int nrenderedunit;
   int npack;
   int nrenderedpack;
   int nsemiunit;
};

int gShowModels = 0;
int gClearModels = 0;

BPModelProfile model_prof_list[256];
BPModelProfile model_prof_show[256];
BPModelProfile *model_prof_end = model_prof_list;
BPModelProfile *model_prof_ptr = model_prof_list;
BPModelProfile *model_show_end;

int BP_Model_ProfCtrl(int d)
{
   if (!d)
      ;
   else if (d < 0)
   {
      if (model_prof_ptr > model_prof_list)
         model_prof_ptr--;
   }
   else if (d > 0)
   {
      if (model_prof_ptr < model_prof_end-1)
         model_prof_ptr++;
   }
   return 0;
}

void BP_Model_ResetProfile()
{
   model_prof_end = model_prof_list;
   model_prof_ptr = model_prof_list;
}

void BP_Model_SwapProfile()
{
   model_show_end = model_prof_show + (model_prof_end - model_prof_list);
   memcpy(model_prof_show, model_prof_list, sizeof(BPModelProfile) * (model_show_end - model_prof_show));
}

void BP_Model_ClearProfile()
{
   BPModelProfile *p = model_prof_list;

   while (p < model_prof_end)
   {
      p->nmodel = 0;
      p->nmesh = 0;
      p->nunit = 0;
      p->nrenderedunit = 0;
      p->npack = 0;
      p->nrenderedpack = 0;
      p->nsemiunit = 0;
      p++;
   }
}

void BP_Model_DumpProfile()
{
   int n = 0;
   BPModelProfile *p = model_prof_show;
   char *str;
   BP_DebugText_Print("%40s %3s %2s %17s %13s", "FILE", "MOD", "M", "UNITS", "PACKETS");
   const char *fname;
   const char *endchr;
   while (p < model_show_end && n++ < 32)
   {
      fname = BP_GetDebugNameFromMesh(p->model);
      str = strrchr((char*)fname, '/');
      if (!str)
         str = strrchr((char*)fname, '\\');
      if (!str)
         str = "_UNKNOWN";
      str++;
      endchr = strrchr(str, '.');
      BP_DebugText_Print("%40.*s %3d %2d %5d/%5d/%5d %6d/%6d", endchr-str, str, p->nmodel, p->nmesh, p->nsemiunit, p->nrenderedunit, p->nunit, p->nrenderedpack, p->npack);
      p++;
   }
}

BPModelProfile *BP_AddModelProfile(unsigned int model, int n_mesh, int n_units)
{
   BPModelProfile *p = model_prof_list;

   while (p < model_prof_end)
   {
      if (p->model == model)
      {
         p->nmodel++;
         p->nmesh += n_mesh;
         p->nunit += n_units;
         return p;
      }
      p++;
   }
   if (p == model_prof_end)
   {
      p->model = model;
      p->nmodel = 1;
      p->nmesh = n_mesh;
      p->nunit = n_units;
      p->npack = 0;
      p->nrenderedunit = 0;
      p->nrenderedpack = 0;
      p->nsemiunit = 0;
      model_prof_end++;
   }
   if (model_prof_end > &model_prof_list[255])
      model_prof_end = &model_prof_list[255];
   return p;
}

void BP_IncrementModelSemitransUnitCount(BPModelProfile *prof, int rendered)
{
   if (prof)
      prof->nsemiunit += rendered;
}

void BP_IncrementModelRenderedUnitCount(BPModelProfile *prof, int rendered)
{
   if (prof)
      prof->nrenderedunit += rendered;
}

void BP_IncrementModelSeenPacketCount(BPModelProfile *prof, int seen)
{
   if (prof)
      prof->npack += seen;
}

void BP_IncrementModelRenderedPacketCount(BPModelProfile *prof, int rendered)
{
   if (prof)
      prof->nrenderedpack += rendered;
}

void BP_GetTexAlphaRange(void *tex, int *tex_alpha)
{
   DG_TEX *pTex = (DG_TEX *)tex;
   CBaseTexture const *pTexture = (CBaseTexture*)pTex->BP_TextureHandle;
   if (pTexture)
   {
      tex_alpha[0] = pTexture->mMinRGBA&0x000000FF;
      tex_alpha[1] = pTexture->mMaxRGBA&0x000000FF;
   }
   else
   {
      tex_alpha[0] = tex_alpha[1] = 0;
   }
}

void BP_GetPacketAlphaRange(unsigned mesh, int unit, int packet, int *vert_alpha)
{
   CMesh const *pMesh = (CMesh*)mesh;
   int const chunkStart = pMesh->GetChunkStartForUnit( unit );
   CMeshChunk const *pChunk = &pMesh->GetMeshChunks()[chunkStart + packet];

   vert_alpha[0] = pChunk->mVertColorMin & 0x000000FF;
   vert_alpha[1] = pChunk->mVertColorMax & 0x000000FF;
}

}
#endif
//----------------------------------------------------------------------------

#ifndef GOLD_VERSION

char const *BP_Debug_GetModelName( int bp_model )
{
   CMesh *pMesh = (CMesh *) bp_model;

   if ( pMesh )
   {
      return pMesh->mDebugName.c_str();
   }
   else
   {
      return "(null)";
   }
}

#endif