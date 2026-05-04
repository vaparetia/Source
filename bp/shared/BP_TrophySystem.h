//------------------------------------------------------------------------------------------
// BP_TrophySystem.h
//
// Main functions for trophies.
//------------------------------------------------------------------------------------------

#pragma once

// size of mBits == kTRP_Count / 8 + (kTRP_Count % 8 == 0 ? 0 : 1)

#if MGS_VERSION == 2
// Enforced by compile time assert in BP_SaveLoadMGS: kTRP_Count == 46
typedef struct _STrophyDeltaBitfield
{
   unsigned char  mBits[6];
} STrophyDeltaBitfield;
#endif

#if MGS_VERSION == 3
// Enforced by compile time assert in BP_SaveLoadMGS: kTRP_Count == 46
typedef struct _STrophyDeltaBitfield
{
   unsigned char  mBits[6];
} STrophyDeltaBitfield;
#endif

//------------------------------------------------------------------------------------------

enum ETrophies
{
   kTRP_Platinum,  // Do not use, OS unlocks!

#if MGS_VERSION==2
   kTRP_GotDogTag,
   kTRP_DestroyNCameras,
   //kTRP_TitleScreenGunshot,
   //kTRP_NakedCartwheel,
   kTRP_FireExtinuisher,   // new
   kTRP_SeeVampInEndCinematic,   // new
   kTRP_LureEnemyGirlieMagazine,
   kTRP_SendNoPantsMarinePhoto,
   kTRP_PutEnemyInLocker,
   kTRP_MetAmes,
   kTRP_MetEmma,
   kTRP_TranqNEnemies,
   //kTRP_WakeTranqedEnemy,
   kTRP_BreakNNecks,
   kTRP_KONEnemies,
   kTRP_FriendOlga,
   kTRP_CompleteVRMission,
   kTRP_ListenToJohnnyInToilet,  // new
   //kTRP_WatchAllCinematics,
   //kTRP_DoPullUp,
   kTRP_KissPoster,
   kTRP_ShootNRadios,
   kTRP_SteamNEnemies,
   kTRP_GotBandana,
   kTRP_GotStealthCamo1,
   kTRP_GotBrownWig,
   kTRP_GotStealthCamo2,
   kTRP_GotOrangeWig,
   kTRP_SnakeBeater,
   kTRP_WatchRoseKillRaiden,
   //kTRP_TalkToRoseNTimes,
   kTRP_MetPresident,
   kTRP_MetStillman,
   kTRP_MetRevolver,
   kTRP_GotDigitalCamera,
   //kTRP_KillSeagull,
   kTRP_GripLevel3,
   kTRP_BeatOlga,
   kTRP_BeatFatman,
   kTRP_BeatHarrier,
   kTRP_BeatRay,
   kTRP_BeatSolidus,
   kTRP_BeatVamp,
   kTRP_PhotographGhostImages,   // new
   kTRP_BeatBossSurvival,
   kTRP_AllBoxes,
   kTRP_BeatSnakeTales,
   kTRP_FirstInNVRMissions,
   kTRP_BeatGameNoAlert,   // new
   kTRP_GotBlueWig,
   kTRP_BeatGame,
   kTRP_BeatAllVRMissions,
   //kTRP_BeatGameNoKills,
#endif

#if MGS_VERSION==3
   kTRP_KOOcelot,
   kTRP_BeatThePain,
   kTRP_BeatTheFear,
   kTRP_BeatTheEnd,
   kTRP_BeatTheFury,
   kTRP_BeatTheSorrow,
   kTRP_BeatShagohod,
   kTRP_BeatVolgin,
   kTRP_BeatTheBoss, // new
   kTRP_CQCSlam,
   kTRP_CQCSpeak,
   kTRP_CQCHoldUp,
//   kTRP_CQCMeatShield,
   kTRP_CQCSlitThroat,
//   kTRP_CureTutorial,
//   kTRP_CureLeech,
   kTRP_CureBullet,
//   kTRP_CurePoison,
   kTRP_GlimpseOcelot,
   kTRP_AllSecretFirstPersonViews,
   kTRP_ShootOneKerotan,
   kTRP_ThrowUp,
   kTRP_CaptureAnimal,
   kTRP_CollectAllMedicalPlants,
   kTRP_CollectAllFruits,
   kTRP_CollectAllMushrooms,
   kTRP_CollectAllBirds,
   kTRP_CollectAllSnakes,
   kTRP_CollectAllFish,
   kTRP_CollectAllFrogs,
   kTRP_EatSnake,
//   kTRP_CatchMouse,
   kTRP_Camouflage100,
   kTRP_Camouflage90,
//   kTRP_HitGuardWithBarrel,
   kTRP_OcelotTimeParadox,
   kTRP_PoisonGuard,
   kTRP_HealStaminaWithMusic,
   kTRP_AllHiddenRadioChannels,
   kTRP_HitGuardWithBees,
   kTRP_BlowUpMunitionsShed,
//   kTRP_SnakeYellInTurret,
   kTRP_FakeDeathPill,
   kTRP_CatchTsuchinoko,
//   kTRP_CollectAllFlora,
//   kTRP_CollectAllFauna,
   kTRP_CollectAllCamouflage,
   kTRP_CollectAllFacePaint,
   kTRP_KillEndBeforeBattle,
   kTRP_HoldUpTheEnd,
   kTRP_MARKHOR,
   kTRP_ShootAllKerotans,
   kTRP_BeatGame,
   kTRP_BeatGameNoKills,
#endif

   kTRP_Count
};  

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

void BP_TrophySystem_UnlockTrophy(enum ETrophies trophy);

void BP_TrophySystem_Init();
void BP_TrophySystem_Shutdown();
void BP_TrophySystem_SignalUpdateFinished();
int BP_TrophySystem_UpdateInProgress();
void BP_TrophySystem_Disable();   // Prevents trophies from being used (e.g. when save data is from another user).
void BP_TrophySystem_Enable();    // Resets status when returning to title screen.
int BP_TrophySystem_IsDisabled();
char const * const BP_TrophySystem_GetDebugString(enum ETrophies trophy);

int BP_TrophySystem_HDDNeededKB();
int BP_TrophySystem_GetError();
int BP_TrophySystem_IsInsufficientHDDSpaceForApp();

void BP_TrophySystem_UnlockTrophiesFromBitfield( const STrophyDeltaBitfield* bitfield );
void BP_TrophySystem_SaveTrophyToBitfield( enum ETrophies trophyID, STrophyDeltaBitfield* pBitfield );

int BP_Debug_TrophySystem_UnlockTrophy( int data );
int BP_Debug_TrophySystem_UnlockAllTrophies( int data );

#ifdef __cplusplus
};
#endif

//------------------------------------------------------------------------------------------
