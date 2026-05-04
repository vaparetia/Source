//----------------------------------------------------------------------------
// BP_Misc.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// INPUT
//----------------------------------------------------------------------------

#if MGS_VERSION==2
#define BP_VALID_MGS_NAME_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/_.&!:? "
#elif MGS_VERSION==3
#define BP_VALID_MGS_NAME_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/_.&!:? \'"
#endif

int BP_Pad_IsJapanese();
int BP_Pad_GetOkAssignment();
int BP_Pad_GetCancelAssignment();
int BP_GetMGSNameKeyboardKeystroke( char * const pC );

int BP_Pad_GetSubJctCameraAssignment();
int BP_Pad_GetLockOnAssignment();
int BP_Pad_GetShootAssignment();
int BP_Pad_GetSlowMovementAssignment();

#if MGS_VERSION == 2
int BP_Pad_GetShootPressAssignment();
#elif MGS_VERSION == 3
int BP_Pad_GetCameraSwitchAssignment();
#endif

//----------------------------------------------------------------------------
// MISC
//----------------------------------------------------------------------------

void BP_GetLocalTime(int* year, int * month, int* day, int* hours, int* minutes, int * seconds);
void BP_CheckForHardShutdown(int bWaitUntilShutdown);
void BP_SetTimeBased_PAL_NTSC_Globals();

void BP_MiscGlobalAppTimeDebug(const char* pInfo);
int BP_CanLoadNewTexture();

#ifdef BP_360
int BP_GetDefaultYAxisFlipVaraible();
#endif

void BP_CheckForSoftReset();
void BP_SoftResetReboot();

void BP_RelaunchTheGameFromUI();
void BP_RelaunchTheBootLoaderFromUI();
void BP_RelaunchTheGameFromUINoPrompt();

void BP_ConsoleScreenSaverSuspend();
void BP_ConsoleScreenSaverResume();

int BP_CheckForLicenseFile();

//----------------------------------------------------------------------------
// Looading spinner
//----------------------------------------------------------------------------

enum ELoadingFlags
{
   kLoadFlag_Stage            =  (1 << 0),
   kLoadFlag_SaveLoad         =  (1 << 1),
   kLoadFlag_LoadScript       =  (1 << 2),
   kLoadFlag_PhotoAlbum       =  (1 << 3),
   kLoadFlag_ScriptedLoad1    =  (1 << 4),
   kLoadFlag_CloudSync        =  (1 << 5),
   kLoadFlag_InitialLoad      =  (1 << 6),
   kLoadFlag_CloudSyncMgs2Mission =  (1 << 7),
};

void BP_InitLoadingSpinner();
void BP_SetSpinnerLoadFlag(int loadFlags, int state);
void BP_SetSpinnerInitialLoadBegin();
void BP_SetSpinnerInitialLoadEnd();
void BP_RenderLoadingSpinner();
void BP_Debug_LoadingSpinnerRender();

//----------------------------------------------------------------------------
// FPS
//----------------------------------------------------------------------------

#if defined(BP_VITA)
#  define BP_IsPAL() (0)

// AS - In MGS3 there are specific circumstances that are running twice as slow when the game is running at 30FPS 
// instead of 60FPS. (UI animations for example). This value is therefore used on the Vita platform. On other platforms, 
// it has no effect.
#  define BP_FPS30_UI_TIME_SCALE(x) ((x)*2.0f)

#define BP_FPS_AdjustFloat_Mult_30(x) (x)
#define BP_FPS_AdjustFloat_Pow_30(x) (x)

#define BP_FPS_AdjustInt_Mult_30(x) (x)
#define BP_FPS_AdjustInt_Pow_30(x) (x)
#define BP_FPS_AdjustInt_Frames_30(x) (x)

#else

int BP_IsPAL();
#  define BP_FPS30_UI_TIME_SCALE(x) (x)

float BP_FPS_AdjustFloat_Mult_30( const float value );
float BP_FPS_AdjustFloat_Pow_30( const float value );

int BP_FPS_AdjustInt_Mult_30( const int value );
int BP_FPS_AdjustInt_Pow_30( const int value );
int BP_FPS_AdjustInt_Frames_30( const int value );

#endif

//----------------------------------------------------------------------------
// TGS DEMO
//----------------------------------------------------------------------------

#if BP_TGS_DEMO()

void BP_TGS_DEMO_Init();
int  BP_TGS_DEMO_CutsceneBegin();
void BP_TGS_DEMO_CutsceneEnd();
void BP_TGS_DEMO_CutscenePlaying();
void BP_TGS_DEMO_Update();
void BP_TGS_DEMO_Render();
void BP_TGS_DEMO_Reboot();
int  BP_TGS_DEMO_IsShowingEndScreen();

#endif

//----------------------------------------------------------------------------
// PS3 System
//----------------------------------------------------------------------------

void BP_HandleInsufficientHDD();
void BP_ShutdownCheck();
int BP_sys_prx_load_module(char const * pRelativePRXPath); // Special version for loading SPRX from disc or HDD patch. Filename is relative to root filesystem (i.e. mylib.sprx). Returns 'sys_prx_id_t'

// returns 1 if this is running a downloadable version of the game (i.e. PSN) and 0 if it's the disk version of the game. NOTE: DiskPatch still returns 0!
int BP_IsDownloadableVersion();

//----------------------------------------------------------------------------
// X360 specific
//----------------------------------------------------------------------------
enum kCheckDeviceNewGameModeRetValues
{
   kCDNGMRT_NothingRequired,
   kCDNGMRT_ShowDeviceInvalidThenDeviceSelector,
   kCDNGMRT_ShowDeviceSelector,
};
void BP_UpdateRichPresenceLogic();
int CheckDeviceNewGameMode(int requiredSpace);

char* BP_GetOverrideString(char* inputString);
char const *BP_GetCustomOverrideString( char const *subsystem, char const *component );


//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
