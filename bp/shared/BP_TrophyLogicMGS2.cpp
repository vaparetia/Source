//----------------------------------------------------------------------------
// trophy_data_logic.cpp
//
// Custom game logic for unlocking trophies
//----------------------------------------------------------------------------

#include <string>
#include <math.h>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/BPEEngineAPI.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CStopWatch.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "BP_TrophyLogicMGS2.h"
#include "BP_TrophySystem.h"
#include "BP_FileSupport.h"
#include "BP_SaveLoadMGS.h"

extern "C"
{
#ifndef GOLD_VERSION
   extern void BP_DebugText_Print(const char * pString, ...);
#endif
#include "yamashita/2D/msn.h"

   //The necessary includes for these are not C++ friendly :(
#define MCMAN_MAX_TALES 5
   extern unsigned int BP_GetSnakeTalesClearGcl();
   extern unsigned int BP_GetMissionNumberGcl();
   extern unsigned int BP_GetSelectedPlayerGcl();
   extern unsigned int BP_GetDemoTheaterFlagGcl();
   extern int BP_TTL_CalcTotalVRStageCount();
};

extern "C" int gBP_TrophyDebug;
extern "C" int gBP_PlayingMissions;

extern "C" void ShowWrongUserWarningPresent(int *pResult);

// total number of frames required to unlock Johnny Eavesdrop.
#define BP_FRAMES_FOR_JOHNNY_EAVESDROP       60

//----------------------------------------------------------------------------

namespace
{
   const int skTrophyDestroyCameraCount = 15;
   const int skTrophyTranqEnemyCount = 100;
   const int skTrophyBreakNeckCount = 30;
   const int skTrophyDestroyRadioCount = 10;
   const int skTrophyPipeSteamKillCount = 5;
   const int skTrophyFirstPlaceVRMissions = 50;
   const int skTrophyKOCount = 30;

   int sDebugPrintFirstPlaceVRMissions = 0;

   bool sbISeeVamp = false;
   bool sbRoseKillRaidenDemoTheater = false;

   //Filter only on PS3 whether we have the TITLE_MENU_TANKER_CLEARD | TITLE_MENU_PLANT_CLEARD
   //due to player's own progress and / or own save files listed on the HDD at startup.
   //Can't allow trophies in Casting Theater or Boss Survival to unlock in that case, if the
   //user got in there by having someone else's clear data on his HDD.
   //X360 won't even enumerate data from another user so the filter is unnecessary.
#if BPE_TARGET == BPE_TARGET_PS3
   int sBP_TankerClearedByUser = 0;
   int sBP_PlantClearedByUser = 0;
#else
   int sBP_TankerClearedByUser = 1;
   int sBP_PlantClearedByUser = 1;
#endif
}

// Default 1st high score table generated from msn_def_hiscore.h
// actually use the data in msn_def_hiscore!
static int skDefaultVrMissionHiScores[MSN_STAGE_ID_MAX];
static int skDefaultVrMissionHiScoresFilledIn = false;

#undef MSN_SET_HISCORE
#define MSN_SET_HISCORE _msn_set_hiscore
static void _msn_set_hiscore(int stage, int place, int score)
{
   if (place == 1)
   {
      BPE_VERIFY(stage < MSN_STAGE_ID_MAX, 0, "Bad assumption");
      skDefaultVrMissionHiScores[stage] = score;
   }
}

static void _setup_hiscores()
{
   if (!skDefaultVrMissionHiScoresFilledIn)
   {
#include "yamashita/2D/msn_def_hiscore.h"

      skDefaultVrMissionHiScoresFilledIn = true;
   }
}
#undef MSN_SET_HISCORE

//----------------------------------------------------------------------------

void BP_Debug_TrophyRender()
{
#ifndef GOLD_VERSION
   if( gBP_TrophyDebug )
   {
      unsigned int selectedPlayerStrId = BP_GetSelectedPlayerGcl();
      unsigned int missionNumber = BP_GetMissionNumberGcl();
      unsigned int demoTheater = BP_GetDemoTheaterFlagGcl();
      BP_DebugText_Print("Player ID: 0x%08x", selectedPlayerStrId);
      BP_DebugText_Print("Mission Number: %d", missionNumber);
      BP_DebugText_Print("Demo Theater: %d", demoTheater);
      BP_DebugText_Print("PlayingMissions: %d", gBP_PlayingMissions );

      BP_DebugText_Print( "Alert Count: %d", GM_AlertCount );

      BP_DebugText_Print( "Broken Neck Count: %d", bp_linkvars.mBrokenNeckCount );
      BP_DebugText_Print( "KO Enemy Count: %d", bp_linkvars.mKOEnemyCount );
      BP_DebugText_Print( "Destroyed camera count: %d", bp_linkvars.mDestroyedCameraCount );
      BP_DebugText_Print( "Destroyed radio count: %d", bp_linkvars.mDestroyedRadioCount );
      BP_DebugText_Print( "Steam kill count: %d", bp_linkvars.mPipeSteamKillCount );
      BP_DebugText_Print( "Tranqed enemy count: %d", bp_linkvars.mTranqedEnemyCount );
      BP_DebugText_Print( "First place VR missions: %d", sDebugPrintFirstPlaceVRMissions );
      BP_DebugText_Print( "Skipped a cinematic: %d", bp_linkvars.mSkippedACinematic );
      BP_DebugText_Print( "Johnny Eavesdrop (Shell 1 Core): %d", bp_linkvars.mJohnnyEavesdrop_vc121101 );
      BP_DebugText_Print( "Johnny Eavesdrop (Sniping): %d", bp_linkvars.mJohnnyEavesdrop_vc120121 );

      _setup_hiscores();

      //VR mission stats
      {
         int clearVRMissions = 0;
         int firstPlaceVRMissions = 0;
         for( int i=VRM_START; i < MSN_STAGE_ID_MAX; ++i )
         {
            if( MSN_GET_CLEARFLAG( i ) )
            {
               ++clearVRMissions;
            }
            //Check current 1st place high score against the default value.
            if( skDefaultVrMissionHiScores[i] < MSN_GET_HISCORE2(i, 1) )
            {
               ++firstPlaceVRMissions;
            }
         }
         BP_DebugText_Print( "VR Missions Clear: %d", clearVRMissions );
         BP_DebugText_Print( "VR Missions 1st Place: %d", firstPlaceVRMissions );
      }
      //Snake tales stats
      {
         unsigned int st_clear_flag = BP_GetSnakeTalesClearGcl();
         int st[MCMAN_MAX_TALES] = { 0 };
         for( int i=0; i < MCMAN_MAX_TALES; ++i )
         {
            if( st_clear_flag & (1<<i) )
            {
               st[i] = 1;
            }
         }
         BP_DebugText_Print( "Snake Tales Clear: %d %d %d %d %d", st[0],st[1],st[2],st[3],st[4] );
      }

      if( sbISeeVamp )
      {
         BP_DebugText_Print( "I SEE VAMP!" );
      }
      else
      {
         BP_DebugText_Print( "I don't see Vamp." );
      }

      BP_DebugText_Print( "Trophies Disabled: %d", BP_TrophySystem_IsDisabled() );
      BP_DebugText_Print( "Wrong User Loaded: %d", MGS_SaveStatus_WrongUser() );
      BP_DebugText_Print( "Tanker cleared by user: %d", sBP_TankerClearedByUser );
      BP_DebugText_Print( "Plant cleared by user: %d", sBP_PlantClearedByUser );
   }
#endif
}

//----------------------------------------------------------------------------

void bp_trophy_begin_stream( int top )
{
   if( const char * fullPath = BP_FindStreamName( top ) )
   {
      //Strip out the path and extension from the stream name.
      char streamName[FILENAME_MAX];
      strcpy( streamName, strrchr( fullPath, '/' ) + 1 );
      *(strstr( streamName, ".sdt" )) = 0;

      unsigned int demoTheater = BP_GetDemoTheaterFlagGcl();

      if( !strcmp( streamName, "t06a1D" ) || !strcmp( streamName, "t06a2D" ) )
      {
         //Beginning of defeated olga cutscene
         //(There are two versions)
         BP_TrophySystem_UnlockTrophy( kTRP_BeatOlga );
      }
      else if( !strcmp( streamName, "p034_01_p01" ) )
      {
         //Beginning of defeated olga cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatFatman );
      }
      else if( !strcmp( streamName, "p046_01_p01" ) )
      {
         //Beginning of defeated harrier cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatHarrier );
      }
      else if( !strcmp( streamName, "p057_01_p01" ) )
      {
         //Beginning of defeated vamp cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatVamp );
      }
      else if( !strcmp( streamName, "p080_01_p01" ) )
      {
         //Beginning of defeated ray cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatRay );
      }
      else if( !strcmp( streamName, "p082_01_p01" ) && !demoTheater )
      {
         //Beginning of defeated solidus cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatSolidus );
      }
   }
   else
   {
      printf("Warning: could not get stream name for %08x\n", top );
   }
}

void bp_trophy_update_stream_pan( int top, float angle, float volume )
{
   if( const char * fullPath = BP_FindStreamName( top ) )
   {
      // remove the path and extension from the stream name.
      char streamName[ FILENAME_MAX ];
      strcpy( streamName, strrchr( fullPath, '/' ) + 1 );
      *strstr( streamName, ".sdt" ) = 0;

      // check for the correct stream.
      if( !strcmp( streamName, "vc121101" ) )
      {
         if ( volume > 0.5f )
         {
            // increment the johnny eavesdrop counter.
            bp_linkvars.mJohnnyEavesdrop_vc121101 += 1;
            if( bp_linkvars.mJohnnyEavesdrop_vc120121 && bp_linkvars.mJohnnyEavesdrop_vc121101 > BP_FRAMES_FOR_JOHNNY_EAVESDROP )
            {
               BP_TrophySystem_UnlockTrophy( kTRP_ListenToJohnnyInToilet );
            }
         }
      }
   }
}

void bp_trophy_end_stream( int top )
{
   if( const char * fullPath = BP_FindStreamName( top ) )
   {
      //Strip out the path and extension from the stream name.
      char streamName[FILENAME_MAX];
      strcpy( streamName, strrchr( fullPath, '/' ) + 1 );
      *(strstr( streamName, ".sdt" )) = 0;

      if( !strcmp( streamName, "p014_15_p07" ) )
      {
         //End of cutscene where stillman hides in the closet
         BP_TrophySystem_UnlockTrophy( kTRP_MetStillman );
      }
      else if( !strcmp( streamName, "p040_07_p04" ) )
      {
         //End of cutscene followed by the scene where you have to quickly equip the AK-47
         BP_TrophySystem_UnlockTrophy( kTRP_MetAmes );
      }
      else if( !strcmp( streamName, "t13a1D" ) )
      {
         //End of final "meet ocelot" cutscene stream (end of tanker chapter)
         BP_TrophySystem_UnlockTrophy( kTRP_MetRevolver );
      }
      else if( !strcmp( streamName, "p049_11_p02" ) )
      {
         //End of final "meet prez" cutscene stream
         BP_TrophySystem_UnlockTrophy( kTRP_MetPresident );
      }
      else if( !strcmp( streamName, "p058_08_p04" ) )
      {
         //End of final "meet emma" cutscene stream
         BP_TrophySystem_UnlockTrophy( kTRP_MetEmma );
      }
      else if( !strcmp( streamName, "vc107011" ) )
      {
         //End of voice sample when olga gut punches raiden then walks off
         BP_TrophySystem_UnlockTrophy( kTRP_FriendOlga );
      }
      else if( !strcmp( streamName, "vc001301" ) )
      {
         //Call otacon while looking at poster on open locker door in w01a
         BP_TrophySystem_UnlockTrophy( kTRP_SnakeBeater );
      }
      else if( !strcmp( streamName, "vc120121" ) )
      {
         bp_linkvars.mJohnnyEavesdrop_vc120121 = 1;
         if( bp_linkvars.mJohnnyEavesdrop_vc121101 > BP_FRAMES_FOR_JOHNNY_EAVESDROP )
         {
            BP_TrophySystem_UnlockTrophy( kTRP_ListenToJohnnyInToilet );
         }
      }
      else if( !strcmp( streamName, "p082_01_p01" ) )
      {
         if( sbRoseKillRaidenDemoTheater && sBP_TankerClearedByUser && sBP_PlantClearedByUser )
         {
            //Watch Rose kill Raiden on a rooftop (demo theater mode)
            BP_TrophySystem_UnlockTrophy( kTRP_WatchRoseKillRaiden );
         }
      }
   }
   else
   {
      printf("Warning: could not get stream name for %08x\n", top );
   }

   //Reset this flag after every stream finishes playing.
   sbRoseKillRaidenDemoTheater = false;
}

void bp_trophy_stream_update( const char * const streamName, const int streamTick, const float manualZoom, const float manualX, const float manualY )
{
   sbISeeVamp = false;
//   printf("bp_trophy_stream_update: %s, %d, %f %f %f\n", streamName, streamTick, manualZoom, manualX, manualY );
   if( streamName != NULL && !strcmp( streamName, "p082_04_p03.sdt" ) )
   {
      //Catch all cases of Vamp in frame and not obstructed
      //Must be zoomed in as well, otherwise the trophy is free
      if( manualZoom >= 2.0 )  //3 seems to be max zoom
      {
         if( streamTick >= 10000 && streamTick <= 11025 )
         {
            //before throwing dogtag (no way for Vamp not to be in frame)
            BP_TrophySystem_UnlockTrophy( kTRP_SeeVampInEndCinematic );
            sbISeeVamp = true;
         }
         else if( streamTick >= 40950 && streamTick <= 41100 )
         {
            //camera spins around Raiden (no way for Vamp not to be in frame)
            BP_TrophySystem_UnlockTrophy( kTRP_SeeVampInEndCinematic );
            sbISeeVamp = true;
         }
         else if( streamTick >= 55590 && streamTick <= 57100 )
         {
            //talking to Rose 1 (need to look slightly to left)
            if( manualX < 0.7f )
            {
               BP_TrophySystem_UnlockTrophy( kTRP_SeeVampInEndCinematic );
               sbISeeVamp = true;
            }
         }
         else if( streamTick >= 60975 && streamTick <= 61575 )
         {
            //talking to Rose 2 (need to look slightly to left)
            if( manualX < 0.6f )
            {
               BP_TrophySystem_UnlockTrophy( kTRP_SeeVampInEndCinematic );
               sbISeeVamp = true;
            }
         }
      }
   }
}

void bp_trophy_new2dsprite( unsigned int texCode )
{
   //NOTE: these trophies unlock when the character's 2dsprite comes up,
   //or at the end of the cinematic if it's skipped.
   switch( texCode )
   {
   case 0x00c0d688: //us / eu / jp
      {
         //"President Johnson (Paul Lukather)" subtitle
         BP_TrophySystem_UnlockTrophy( kTRP_MetPresident );
      }
      break;
   case 0x00f52239: //us / eu / jp
      {
         //Ocelot flashback during metal gear ray theft of tanker ending cinema (t12a1D)
         BP_TrophySystem_UnlockTrophy( kTRP_MetRevolver );
      }
      break;
   case 0x00c5c615: //us / eu / jp
      {
         //"Emma Emmerich (Jennifer Hale)" subtitle
         BP_TrophySystem_UnlockTrophy( kTRP_MetEmma );
      }
      break;
   }
}

void bp_trophy_vr_mission_clear( const int stage )
{
   int i;
   int clearVRMissions = 0;
   int firstPlaceVRMissions = 0;
   const int id_min = 0;
   const int id_max = MSN_STAGE_ID_MAX-1;
   const int total_stages = BP_TTL_CalcTotalVRStageCount();
   if( stage < id_min || stage > id_max )
   {
      return;
   }

   _setup_hiscores();

   if( !gBP_PlayingMissions )
   {
      //Must be a "VR" mission inside Snake Tales.  Don't count these.
      return;
   }

   for( i=id_min; i <= id_max; ++i )
   {
      if( MSN_GET_CLEARFLAG( i ) )
      {
         ++clearVRMissions;
      }
      //Check current 1st place high score against the default value.
      if( skDefaultVrMissionHiScores[i] < MSN_GET_HISCORE2(i, 1) )
      {
         ++firstPlaceVRMissions;
      }
   }

   sDebugPrintFirstPlaceVRMissions = firstPlaceVRMissions;

   if( firstPlaceVRMissions >= skTrophyFirstPlaceVRMissions )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_FirstInNVRMissions );
   }

   if( clearVRMissions >= total_stages )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_BeatAllVRMissions );
   }

   BP_TrophySystem_UnlockTrophy( kTRP_CompleteVRMission );
}

void bp_trophy_snake_tales_clear( const int code )
{
   unsigned int st_clear_flag = BP_GetSnakeTalesClearGcl();

   printf("bp_trophy_snake_tales_clear: code 0x%08X, clear_flag %x\n", code, st_clear_flag );
   if( st_clear_flag == ( (1<<MCMAN_MAX_TALES)-1 ) )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_BeatSnakeTales );
   }
}

void bp_trophy_demo_theater_play( const int argc, const int * const argv )
{
   if( ( argv[0] == 8 ) && ( argv[1] == 7 ) && ( argv[2] == 1 || argv[2] == 19 ) )
   {
      //8 == Defeat Solidus Cinematic
      //7 == Rose
      //1 == Raiden
      //19 == Raiden (Ninja)
      sbRoseKillRaidenDemoTheater = true;
   }
}

void bp_trophy_skipped_cinematic()
{
   bp_linkvars.mSkippedACinematic = 1;
}

void bp_trophy_destroyed_radio()
{
   ++bp_linkvars.mDestroyedRadioCount;
   if( bp_linkvars.mDestroyedRadioCount >= skTrophyDestroyRadioCount )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_ShootNRadios );
   }
}

void bp_trophy_destroyed_camera()
{
   ++bp_linkvars.mDestroyedCameraCount;
   if( bp_linkvars.mDestroyedCameraCount >= skTrophyDestroyCameraCount )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_DestroyNCameras );
   }
}

void bp_trophy_tranqed_enemy()
{
   ++bp_linkvars.mTranqedEnemyCount;
   if( bp_linkvars.mTranqedEnemyCount >= skTrophyTranqEnemyCount )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_TranqNEnemies );
   }
}

void bp_trophy_broke_enemy_neck()
{
   ++bp_linkvars.mBrokenNeckCount;
   if( bp_linkvars.mBrokenNeckCount >= skTrophyBreakNeckCount )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_BreakNNecks );
   }
}

void bp_trophy_ko_enemy()
{
   ++bp_linkvars.mKOEnemyCount;
   if( bp_linkvars.mKOEnemyCount >= skTrophyKOCount )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_KONEnemies );
   }
}

void bp_trophy_steam_enemy()
{
   ++bp_linkvars.mPipeSteamKillCount;
   if( bp_linkvars.mPipeSteamKillCount >= skTrophyPipeSteamKillCount )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_SteamNEnemies );
   }
}

void bp_trophy_set_tanker_cleared_by_user()
{
   sBP_TankerClearedByUser = 1;
}

void bp_trophy_set_plant_cleared_by_user()
{
   sBP_PlantClearedByUser = 1;
}

int bp_trophy_get_tanker_cleared_by_user()
{
   return sBP_TankerClearedByUser;
}

int bp_trophy_get_plant_cleared_by_user()
{
   return sBP_PlantClearedByUser;
}

int bp_trophy_check_warning_cleared_by_user()
{
#if BPE_TARGET==BPE_TARGET_PS3
   //Special warning for case where user enters casting theater or boss survival
   //when these sections were only unlocked by another user's data on the HDD.
   if( !( sBP_TankerClearedByUser && sBP_PlantClearedByUser ) )
   {
      int result;
      ShowWrongUserWarningPresent(&result);
   }
#endif
   
   return 0;
}

//----------------------------------------------------------------------------
