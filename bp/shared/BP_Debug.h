//----------------------------------------------------------------------------
// BP_Debug.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

void BP_HandleCommandLineArgs();
void BP_InitGameDebugMenu();
void BP_Debug_BeginFrame();
void BP_Debug_EndFrame();

#ifdef __cplusplus
extern "C" {
#endif

   //----------------------------------------------------------------------------

   // Pause frame stepping
   extern int   gBP_PauseFrameStep;
   extern int   gBP_ShowDebugMenuBlackBackground;

   // Input
   extern int   gBP_Input_X360Remap_Enable;
   extern int   gBP_Input_X360Remap_DemoZoomMethod;
   extern int   gBP_Input_X360Remap_QuickHolsterMethod;
   extern int   gBP_Input_HoldSquatToGround_Enable;
   
   // Gameplay
   extern int   gBP_Game_GrenadeExplodeInHand_Enable;
   extern int   gBP_Game_GrenadeBlast_InnerRange;
   extern int   gBP_Game_GrenadeBlast_OuterRange;
   
   extern int   gBP_Game_SolidusChoke_Life[];
   extern int   gBP_Game_SolidusChoke_Time[];

   // Cheats
   extern int gBP_EnemySightDisabled;
   extern int gBP_EnemyHearingDisabled;

   // 
#if MGS_VERSION == 2
   enum 
   {
      kBP_KP_Cheat_OptCmf_Normal,
      kBP_KP_Cheat_OptCmf_Green,
      kBP_KP_Cheat_OptCmf_Invisible,
   };
   extern int gBP_KP_Cheat_OptCmfMode;
#endif

   //----------------------------------------------------------------------------
#if BP_360
   void BP_TransformIntoCommandLineArgs(char const * InCommandLine, int* OutNumArgs, char*** OutArgs);
   int BP_PreParseCommandLineArgs(int InStartArg, int InNumArgs, char** InArgs, char const * const pApplicationRootDirectory);
   void BP_CleanupTransformIntoCommandLineArgs(int InNumArgs, char** InArgs);
#endif
   //----------------------------------------------------------------------------
   int BP_GetTweakValueFromDifficulty( const int* values );
   int BP_GCL_SetVar_Override( unsigned int code, int value );

#if BP_WIN32

   // Types of breakpoints
   enum HWBRK_TYPE
   {
      HWBRK_TYPE_CODE,        // Triggers when code is executed
      HWBRK_TYPE_READWRITE,   // Triggers when data is read or written
      HWBRK_TYPE_WRITE,       // Triggers when data is written
   };

   // Size of read/write breakpoints
   enum HWBRK_SIZE
   {
      HWBRK_SIZE_1,           // 1 byte
      HWBRK_SIZE_2,           // 2 bytes
      HWBRK_SIZE_4,           // 4 bytes
      HWBRK_SIZE_8,           // 8 bytes
   };

   // Set/Remove breakpoint functions
   //    - Supports up to 4 hardware breakpoints per thread
   int BP_SetHardwareBreakpoint(void* addr, enum HWBRK_SIZE size, enum HWBRK_TYPE type);
   void BP_RemoveHardwareBreakpoint(int handle);

#endif

   //----------------------------------------------------------------------------

   // X360 remapping
   void BP_UpdatePlayerPadRemap();

   //----------------------------------------------------------------------------


   void BP_Debug_PushCPUMarker( char const *name );
   void BP_Debug_PopCPUMarker();

#if BP_ENABLE_TESTNODE
   extern int gTestNodeEnabled;
   extern int gTestNodeStart;
   extern int gTestNodeEnd;
   extern int gTestNodeCur;
   extern int gTestNodeCount;

   void BP_Debug_BeginTestNode();
   int BP_Debug_EvalTestNode();
   void BP_Debug_EndTestNode();
#endif
#if BP_ENABLE_TESTTRI
   extern int gTestTriEnabled;
   extern int gTestTriActive;
   extern int gTestTriReset;
   extern int gTestTriStart;
   extern int gTestTriEnd;
   extern int gTestTriCur;
   extern int gTestTriCount;

#if (MGS_VERSION == 3) || BP_WIN32
   typedef unsigned int uint32_t;
#endif
   const void *BP_Debug_EvalTestTri(int is32, const void *indices, uint32_t *indexCount);
#endif

#if JADEBUG
   extern int gShowModels;
   extern int gClearModels;

   int BP_Model_ProfCtrl(int d);
   void BP_Model_DumpProfile();
   void BP_Model_ResetProfile();
   void BP_Model_ClearProfile();
   void BP_Model_SwapProfile();
   typedef struct _BPModelProfile BPModelProfile;
   BPModelProfile *BP_AddModelProfile(unsigned int,int,int);
   void BP_IncrementModelRenderedUnitCount(BPModelProfile *prof, int rendered);
   void BP_IncrementModelSemitransUnitCount(BPModelProfile *prof, int rendered);
   void BP_IncrementModelSeenPacketCount(BPModelProfile *prof, int seen);
   void BP_IncrementModelRenderedPacketCount(BPModelProfile *prof, int rendered);

   void BP_GetTexAlphaRange(void *tex, int *tex_alpha);
   void BP_GetPacketAlphaRange(unsigned mesh, int unit, int packet, int *vert_alpha);
#endif

   char const *BP_Debug_GetModelName( int bp_model );

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
