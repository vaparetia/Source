//----------------------------------------------------------------------------
// BP_TrophyLogicMGS3.cpp
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "BP_TrophyLogicMGS3.h"
#include "BP_TrophySystem.h"
#include "BP_FileSupport.h"
#include "BP_GclHooks.h"
#include "BP_SaveLoadMGS.h"
#include "BP_TrophyLogicMGS3.h"

extern "C"
{
#ifndef GOLD_VERSION
   extern void BP_DebugText_Print(const char * pString, ...);
#endif
#include "linkvar.h"
   unsigned int BP_GetDemoTheaterFlagGcl();
};

extern "C" int gBP_TrophyDebug;

//----------------------------------------------------------------------------

namespace
{
   const char * const skRadioStationStreamNames[] =
   {
      "m01_surfing_guitar",
      "m02_rock_me_baby",
      "m03_pillow_talk",
      "m04_jumpin_johnny",
      "m05_sailor",
      "m06_salty_catfish",
      "m07_sea_breeze",
      "m08_dont_be_short"
   };

   enum ESecretR1Cutscenes
   {
      kR1CS_s075a_0,
      kR1CS_s112a_0,
      kR1CS_s171b_0,
      kR1CS_s201a_1,
      kR1CS_s223a_0_first,
      kR1CS_s223a_0_second,

      kR1CS_Count
   };

   const char * const skSecretR1CutsceneNames[] =
   {
      "s075a_0",
      "s112a_0",
      "s171b_0",
      "s201a_1",
      "s223a_0_first",
      "s223a_0_second",
   };
}

BPE_CTASSERT( BPE_ARRAY_SIZE( skSecretR1CutsceneNames ) == kR1CS_Count );

void BP_Debug_TrophyRender()
{
#ifndef GOLD_VERSION
   if( gBP_TrophyDebug )
   {
      BP_DebugText_Print( "Healing Radio Stations:" );
      for( int i=0; i < BPE_ARRAY_SIZE(skRadioStationStreamNames); ++i )
      {
         if( bp_linkvars.mHealingRadioStations & (1<<i) )
         {
            BP_DebugText_Print( "* %s", skRadioStationStreamNames[i] );
         }
      }

      BP_DebugText_Print( "Secret R1 Cutscenes:" );
      for( int i=0; i < kR1CS_Count; ++i )
      {
         int cutsceneSeen = ( bp_linkvars.mSecretR1Cutscenes & (1<<i) ) != 0;
         BP_DebugText_Print( "%d %s", cutsceneSeen, skSecretR1CutsceneNames[i] );
      }

      {
         unsigned char shotKerotans[64];
         const int shotKerotansCount = BP_GCL_GetShotKerotanCount( shotKerotans );
         BP_DebugText_Print( "Shot Kerotans: %2d", shotKerotansCount );
         for( int i=0; i < 8; ++i )
         {
            const unsigned char * row = shotKerotans + 8 * i;
            BP_DebugText_Print( "%d %d %d %d %d %d %d %d", row[0],row[1],row[2],row[3],row[4],row[5],row[6],row[7] );
         }
      }

      unsigned int demoTheater = BP_GetDemoTheaterFlagGcl();
      BP_DebugText_Print( "In Demo Theater: %d", demoTheater );
      BP_DebugText_Print( "Trophies Disabled: %d", BP_TrophySystem_IsDisabled() );
      BP_DebugText_Print( "Wrong User Loaded: %d", MGS_SaveStatus_WrongUser() );
      BP_DebugText_Print( "Game Clear Count: %d", GM_GameClearCount );
   }
#endif
}

//----------------------------------------------------------------------------

static void bp_check_radio_station_trophy( const char * const streamName )
{
   for( int i=0; i < BPE_ARRAY_SIZE( skRadioStationStreamNames ); ++i )
   {
      if( !strcmp( skRadioStationStreamNames[i], streamName ) )
      {
         bp_linkvars.mHealingRadioStations |= (1<<i);
         //TODO: should this only be unlocked if the player's stamina
         //is not full when the music starts?
         BP_TrophySystem_UnlockTrophy( kTRP_HealStaminaWithMusic );
      }
   }

   if( bp_linkvars.mHealingRadioStations == ( 1 << BPE_ARRAY_SIZE(skRadioStationStreamNames) ) - 1 )
   {
      BP_TrophySystem_UnlockTrophy( kTRP_AllHiddenRadioChannels );
   }
}

void bp_trophy_begin_stream( int top )
{
   if( const char * const fullPath = BP_FindStreamName( top ) )
   {
      //Strip out the path and extension from the stream name.
      char streamName[FILENAME_MAX];
      strcpy( streamName, strrchr( fullPath, '/' ) + 1 );
      *(strstr( streamName, ".sdt" )) = 0;

      //Don't allow (most?) cinematic unlocks if we're in the demo theater
      unsigned int demoTheater = BP_GetDemoTheaterFlagGcl();

      if( !strcmp( streamName, "v130_010_p010" ) && !demoTheater )
      {
         //Beginning of stream where Snake splints his arm
// DISABLED!         BP_TrophySystem_UnlockTrophy( kTRP_CureTutorial );
      }
      else if( !strcmp( streamName, "m140_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated the pain cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatThePain );
      }
      else if( !strcmp( streamName, "m190_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated the fear cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatTheFear );
      }
      else if( !strcmp( streamName, "m250_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated the end cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatTheEnd );
      }
      else if( !strcmp( streamName, "m280_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated the fury cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatTheFury );
      }
      else if( !strcmp( streamName, "m480_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated volgin cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatVolgin );
      }
      else if( !strcmp( streamName, "m570_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated shagohod cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatShagohod );
      }
      else if( !strcmp( streamName, "m620_010_p010" ) && !demoTheater )
      {
         //Beginning of defeated the boss cutscene
         BP_TrophySystem_UnlockTrophy( kTRP_BeatTheBoss );
      }

      bp_check_radio_station_trophy( streamName );
   }
   else
   {
      printf("Warning: could not get stream name for %08x\n", top );
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

      //Don't allow (most?) cinematic unlocks if we're in the demo theater
      unsigned int demoTheater = BP_GetDemoTheaterFlagGcl();

      if( !strcmp( streamName, "v080_060_r020_01" ) && !demoTheater )
      {
         //End of radio convo w/ Zero after KO Ocelot
         BP_TrophySystem_UnlockTrophy( kTRP_KOOcelot );
      }
      else if( !strcmp( streamName, "m380_020_r010_01" ) && !demoTheater )
      {
         //End of radio convo w/ Zero after waking from sorrow's river
         BP_TrophySystem_UnlockTrophy( kTRP_BeatTheSorrow );
      }
   }
   else
   {
      printf("Warning: could not get stream name for %08x\n", top );
   }
}

void bp_trophy_stream_update( const char * const streamName, const int streamTick, const float manualZoom, const float fpvRotX, const float fpvRotY, const int fpv )
{
//   printf("bp_trophy_stream_update: %s, %d, %f %f %f %d\n", streamName, streamTick, manualZoom, fpvRotX, fpvRotY, fpv );
   int beforeSecretR1Cutscenes = bp_linkvars.mSecretR1Cutscenes;
   unsigned int demoTheater = BP_GetDemoTheaterFlagGcl();

   //None of these fpv trophies should unlock in the demo theater.
   if( fpv && !demoTheater )
   {
      if( !strcmp( streamName, "m260_050_p050.sdt" ) )
      {
         //R1 must be pressed as The Boss rides off. (s075a_0)
         bp_linkvars.mSecretR1Cutscenes |= ( 1 << kR1CS_s075a_0 );
      }
      else if( !strcmp( streamName, "m320_020_p010.sdt" ) )
      {
         if( streamTick < 141000 )
         {
            //Press R1 after Ocelot walks out of the room. "buggy" (s112a_0)
            bp_linkvars.mSecretR1Cutscenes |= ( 1 << kR1CS_s112a_0 );
         }
         //There's another normal R1 FPV later in the stream.
      }
      else if( !strcmp( streamName, "m570_010_p010.sdt" ) )
      {
         //Press R1 right as Eva goes to hug Snake.(s171b_0)
         bp_linkvars.mSecretR1Cutscenes |= ( 1 << kR1CS_s171b_0 );
      }
      else if( !strcmp( streamName, "m625_015_p015.sdt" ) )
      {
         //Press R1 as the horse is yelling or whatever horses do. (s201a_1)
         bp_linkvars.mSecretR1Cutscenes |= ( 1 << kR1CS_s201a_1 );
      }
      else if( !strcmp( streamName, "m680_060_05x.sdt" ) )
      {
         if( streamTick < 25000 )
         {
            //Press R1 as they shake hands (s223a_0)
            bp_linkvars.mSecretR1Cutscenes |= ( 1 << kR1CS_s223a_0_first );

            if( fpvRotY < -200.f )
            {
               //Glancing slightly to the left and Ocelot is fully visible.
               BP_TrophySystem_UnlockTrophy( kTRP_GlimpseOcelot );
            }
         }
         else
         {
            //When snake salutes the grave stone of the boss(s223a_0)
            bp_linkvars.mSecretR1Cutscenes |= ( 1 << kR1CS_s223a_0_second );
         }
      }
   }

   if( beforeSecretR1Cutscenes != bp_linkvars.mSecretR1Cutscenes )
   {
      if( bp_linkvars.mSecretR1Cutscenes == ((1<<kR1CS_Count)-1) )
      {
         BP_TrophySystem_UnlockTrophy( kTRP_AllSecretFirstPersonViews );
      }
   }
}

