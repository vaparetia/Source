//------------------------------------------------------------------------------------------
// BP_TrophySystem.cpp
//
// Misc non-platform specific trophy functions.
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BP_TrophySystem.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"

extern "C" int gBP_DebugUnlockTrophy;

namespace
{
	//------------------------------------------------------------------------------------------

	static char const * const sDebugTrophyNames[] =
	{
      "Platinum",

#if MGS_VERSION==2
   "GotDogTag",
   "DestroyNCameras",
   //"TitleScreenGunshot",
   "FireExtinuisher",   // new
   "SeeVampInEndCinematic",   // new
   "LureEnemyGirlieMagazine",
   "SendNoPantsMarinePhoto",
   "PutEnemyInLocker",
   "MetAmes",
   "MetEmma",
   "TranqNEnemies",
   //"WakeTranqedEnemy",
   "BreakNNecks",
   "KONEnemies",
   "FriendOlga",
   "CompleteVRMission",
   "ListenToJohnnyInToilet",
   "KissPoster",
   "ShootNRadios",
   "SteamNEnemies",
   "GotBandana",
   "GotStealthCamo1",
   "GotBrownWig",
   "GotStealthCamo2",
   "GotOrangeWig",
   "SnakeBeater",
   "WatchRoseKillRaiden",
   "MetPresident",
   "MetStillman",
   "MetRevolver",
   "GotDigitalCamera",
   "GripLevel3",
   "BeatOlga",
   "BeatFatman",
   "BeatHarrier",
   "BeatRay",
   "BeatSolidus",
   "BeatVamp",
   "PhotographGhostImages",
   "BeatBossSurvival",
   "AllBoxes",
   "BeatSnakeTales",
   "FirstInNVRMissions",
   "BeatGameNoAlert",
   "GotBlueWig",
   "BeatGame",
   "BeatAllVRMissions",
#endif

#if MGS_VERSION==3
   "KOOcelot",
   "BeatThePain",
   "BeatTheFear",
   "BeatTheEnd",
   "BeatTheFury",
   "BeatTheSorrow",
   "BeatShagohod",
   "BeatVolgin",
   "BeatTheBoss",
   "CQCSlam",
   "CQCSpeak",
   "CQCHoldUp",
   "CQCSlitThroat",
   "CureBullet",
   "GlimpseOcelot",
   "AllSecretFirstPersonViews",
   "ShootOneKerotan",
   "ThrowUp",
   "CaptureAnimal",
   "CollectAllMedicalPlants",
   "CollectAllFruits",
   "CollectAllMushrooms",
   "CollectAllBirds",
   "CollectAllSnakes",
   "CollectAllFish",
   "CollectAllFrogs",
   "EatSnake",
   "Camouflage100",
   "Camouflage90",
   "OcelotTimeParadox",
   "PoisonGuard",
   "HealStaminaWithMusic",
   "AllHiddenRadioChannels",
   "HitGuardWithBees",
   "BlowUpMunitionsShed",
   "FakeDeathPill",
   "CatchTsuchinoko",
   "CollectAllCamouflage",
   "CollectAllFacePaint",
   "KillEndBeforeBattle",
   "HoldUpTheEnd",
   "MARKHOR",
   "ShootAllKerotans",
   "BeatGame",
   "BeatGameNoKills",
#endif

   };

   BPE_CTASSERT( BPE_ARRAY_SIZE( sDebugTrophyNames ) == kTRP_Count );
}

//----------------------------------------------------------------------------

extern "C" char const * const BP_TrophySystem_GetDebugString(ETrophies trophy)
{
	return sDebugTrophyNames[trophy];
}

//----------------------------------------------------------------------------

extern "C" int BP_Debug_TrophySystem_UnlockTrophy( int data )
{
   (void)data;

   BP_TrophySystem_UnlockTrophy( (ETrophies)(gBP_DebugUnlockTrophy + 1) );

   return 0;
}

//----------------------------------------------------------------------------

extern "C" int BP_Debug_TrophySystem_UnlockAllTrophies( int data )
{
   (void)data;

   // Don't unlock the platinum trophy
   for( int i=0; i < kTRP_Count; i++ )
   {
      ETrophies trophy = (ETrophies)i;
      if( trophy != kTRP_Platinum )
      {
         BP_TrophySystem_UnlockTrophy( trophy );
      }
   }

   return 0;
}