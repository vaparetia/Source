/*
		header file for system link variable
*/

#pragma once //BP

#include "BP_TrophySystem.h"

#define GM_GameClearCount		(*( short * )( linkvarbuf + 0 ))
#define GM_TankerClearCount		(*( short * )( linkvarbuf + 2 ))
#define GM_PlantClearCount		(*( short * )( linkvarbuf + 4 ))
#define GM_Configuration		(*( short * )( linkvarbuf + 6 ))
#define GM_Configuration2		(*( int *)( linkvarbuf + 8 ))
#define GM_VRConfiguration		(*( int *)( linkvarbuf + 12 ))
#define GM_GameLevel		(*( short * )( linkvarbuf + 16 ))
#define GM_Result		(*( short * )( linkvarbuf + 18 ))
#define GM_Language		(*( short * )( linkvarbuf + 20 ))
#define GM_ClearFlag		(*( short * )( linkvarbuf + 22 ))
#define GM_ScrAdjX		(*( short * )( linkvarbuf + 24 ))
#define GM_ScrAdjY		(*( short * )( linkvarbuf + 26 ))
#define GM_SaveResidentDir		(( char *)( linkvarbuf + 28 ))
#define GM_SaveAreaDir		(( char *)( linkvarbuf + 44 ))
#define GM_DogTagFlag		(( int *)( linkvarbuf + 60 ))
#define GM_SaveArea		(*( int *)( linkvarbuf + 188 ))
#define GM_SaveMap		(*( int *)( linkvarbuf + 192 ))
#define GM_AreaHistory		(( int *)( linkvarbuf + 196 ))
#define GM_PrevArea		(*( int *)( linkvarbuf + 212 ))
#define GM_SaveX		(*( int *)( linkvarbuf + 216 ))
#define GM_SaveY		(*( int *)( linkvarbuf + 220 ))
#define GM_SaveZ		(*( int *)( linkvarbuf + 224 ))
#define GM_StagePlayTime		(*( int *)( linkvarbuf + 228 ))
#define GM_PlayerPosX		(*( int *)( linkvarbuf + 232 ))
#define GM_PlayerPosY		(*( int *)( linkvarbuf + 236 ))
#define GM_PlayerPosZ		(*( int *)( linkvarbuf + 240 ))
#define GM_PlayerDir		(*( short * )( linkvarbuf + 244 ))
#define GM_PlayerMotion		(*( short * )( linkvarbuf + 246 ))
#define GM_BehindRot		(*( short * )( linkvarbuf + 248 ))
#define GM_Vitality		(*( short * )( linkvarbuf + 250 ))
#define GM_VitalityMax		(*( short * )( linkvarbuf + 252 ))
#define GM_O2		(*( short * )( linkvarbuf + 254 ))
#define GM_O2Max		(*( short * )( linkvarbuf + 256 ))
#define GM_PlayerStance		(*( short * )( linkvarbuf + 258 ))
#define GM_Weapon		(*( short * )( linkvarbuf + 260 ))
#define GM_Item		(*( short * )( linkvarbuf + 262 ))
#define GM_PlayerCold		(*( short * )( linkvarbuf + 264 ))
#define GM_PlayerSneezeTime		(*( short * )( linkvarbuf + 266 ))
#define GM_PlayerColdCount		(*( int *)( linkvarbuf + 268 ))
#define GM_PlayerColdStartTime		(*( int *)( linkvarbuf + 272 ))
#define GM_PlayerStateFlag		(*( short * )( linkvarbuf + 276 ))
#define GM_WeaponPrev		(*( short * )( linkvarbuf + 278 ))
#define GM_ItemPrev		(*( short * )( linkvarbuf + 280 ))
#define GM_AlertMode		(*( short * )( linkvarbuf + 282 ))
#define GM_StartAlertMode		(*( short * )( linkvarbuf + 284 ))
#define GM_SnakeGripMax		(( short * )( linkvarbuf + 286 ))
#define GM_RaidenGripMax		(( short * )( linkvarbuf + 294 ))
#define GM_SnakeChin_Up		(*( short * )( linkvarbuf + 302 ))
#define GM_RaidenChin_Up		(*( short * )( linkvarbuf + 304 ))
#define GM_ContinueCount		(*( short * )( linkvarbuf + 306 ))
#define GM_GameOverCount		(*( short * )( linkvarbuf + 308 ))
#define GM_SaveCount		(*( short * )( linkvarbuf + 310 ))
#define GM_PlayTime		(*( int *)( linkvarbuf + 312 ))
#define GM_LastSave		(*( int *)( linkvarbuf + 316 ))
#define GM_ShootCount		(*( short * )( linkvarbuf + 320 ))
#define GM_AlertCount		(*( short * )( linkvarbuf + 322 ))
#define GM_KillCount		(*( short * )( linkvarbuf + 324 ))
#define GM_DamageCount		(*( short * )( linkvarbuf + 326 ))
#define GM_ClearCode		(( int *)( linkvarbuf + 328 ))
#define GM_MecaKillCount		(*( short * )( linkvarbuf + 344 ))
#define Padding_Dummy		(*( short * )( linkvarbuf + 346 ))
#define _GM_Weapons		(( short * )( linkvarbuf + 348 ))
#define _GM_WeaponsMax		(( short * )( linkvarbuf + 420 ))
#define _GM_Items		(( short * )( linkvarbuf + 492 ))
#define _GM_ItemsMax		(( short * )( linkvarbuf + 588 ))
#define _GM_WeaponsR		(( short * )( linkvarbuf + 684 ))
#define _GM_WeaponsMaxR		(( short * )( linkvarbuf + 756 ))
#define _GM_ItemsR		(( short * )( linkvarbuf + 828 ))
#define _GM_ItemsMaxR		(( short * )( linkvarbuf + 924 ))
#define _GM_WeaponsSaved		(( short * )( linkvarbuf + 1020 ))
#define _GM_ItemsSaved		(( short * )( linkvarbuf + 1092 ))
#define GM_CameraX		(*( int *)( linkvarbuf + 1188 ))
#define GM_CameraY		(*( int *)( linkvarbuf + 1192 ))
#define GM_CameraZ		(*( int *)( linkvarbuf + 1196 ))
#define GM_CamTargX		(*( int *)( linkvarbuf + 1200 ))
#define GM_CamTargY		(*( int *)( linkvarbuf + 1204 ))
#define GM_CamTargZ		(*( int *)( linkvarbuf + 1208 ))
#define GM_CamRotX		(*( int *)( linkvarbuf + 1212 ))
#define GM_CamRotY		(*( int *)( linkvarbuf + 1216 ))
#define GM_AlertLevel		(*( int *)( linkvarbuf + 1220 ))
#define GM_LastCodecFreq		(*( short * )( linkvarbuf + 1224 ))
#define GM_GlobalLoadCount		(*( short * )( linkvarbuf + 1226 ))
#define ENEMEM_EneMem		(( int *)( linkvarbuf + 1228 ))
#define GM_ResetLoadCount		(*( short * )( linkvarbuf + 5452 ))
#define ENEMEM_CurrentNum		(*( short * )( linkvarbuf + 5454 ))
#define GM_TnkerCamStatus		(*( int *)( linkvarbuf + 5456 ))
#define GM_MyName		(( char *)( linkvarbuf + 5460 ))
#define GM_MySexData		(*( int *)( linkvarbuf + 5480 ))
#define GM_MyYearData		(*( int *)( linkvarbuf + 5484 ))
#define GM_MyMonthData		(*( int *)( linkvarbuf + 5488 ))
#define GM_MyDayData		(*( int *)( linkvarbuf + 5492 ))
#define GM_MyBloodData		(*( int *)( linkvarbuf + 5496 ))
#define GM_MyRegionData		(*( int *)( linkvarbuf + 5500 ))
#define GM_StageBreakPoint		(*( int *)( linkvarbuf + 5504 ))
#define GM_SelectStageLimit		(*( int *)( linkvarbuf + 5508 ))
#define GM_StageNum		(*( short * )( linkvarbuf + 5512 ))
#define GM_TitleMenuStatus		(*( short * )( linkvarbuf + 5514 ))
#define GM_ShipwormFlag		(*( short * )( linkvarbuf + 5516 ))
#define GM_ShipwormCorrode		(*( short * )( linkvarbuf + 5518 ))
#define GM_RationUseCount		(*( short * )( linkvarbuf + 5520 ))
#define GM_ClearingCount		(*( short * )( linkvarbuf + 5522 ))
#define GM_RedFindCount		(*( short * )( linkvarbuf + 5524 ))
#define GM_ClearCodeFlag		(*( short * )( linkvarbuf + 5526 ))

#define MAX_LINKVARBUF 5528

extern char *linkvarbuf;

typedef struct _SBP_LinkVars
{
   unsigned int   mCookie;
   unsigned int   mVersion;
   unsigned int   mSize;

   int         mDestroyedCameraCount;
   int         mTranqedEnemyCount;
   int         mKOEnemyCount;
   int         mBrokenNeckCount;
   int         mDestroyedRadioCount;
   int         mPipeSteamKillCount;
   int         mTalkedToRoseCount;
   int         mSkippedACinematic;
   int         mJohnnyEavesdrop_vc121101;
   int         mJohnnyEavesdrop_vc120121;

   // Originator data
   unsigned char  mOriginalConsoleType; // Vita or PS3
   unsigned char  mOriginalConsolePSID[16]; // 16 bytes for the OpenPSID (same length on Vita/PS3)
   unsigned char  mOriginalPSNAccount[16]; // 16 bytes of a PSN ID hash

   // Flag denoting whether it's legal to unlock trophies. 0 indicates validity, since old save games will be zeroed out
   unsigned char  mTrophyValidFlag;

   // Upon transfarring back to the original system, unlock these trophies
   STrophyDeltaBitfield mTrophyDeltaBitfield;
} SBP_LinkVars;

extern SBP_LinkVars bp_linkvars;
extern SBP_LinkVars sv_bp_linkvars;

#define MAX_BP_LINKVARBUF 1024
