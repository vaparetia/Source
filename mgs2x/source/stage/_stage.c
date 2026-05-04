/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewCergeiEri_demo_3012Launch;
extern NEWCHARA NewPutWorldTexFadeWorld;
extern NEWCHARA NewSolMant_called_3013Launch;
extern NEWCHARA NewRopeModel3_called_3014Launch;
extern NEWCHARA NewHandcuff_called_3015Launch;
extern NEWCHARA NewRopeModel3_called_3017Launch;
extern NEWCHARA NewRopeModel3_called_3019Launch;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA ENEMEM_GclCopy;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA GM_GetDirForGCL;
extern NEWCHARA NewZakoTelop;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewBombEffectScn;
extern NEWCHARA NewTitleKonami;
extern NEWCHARA NewClearCodeScr;
extern NEWCHARA NewDummyPlayer;
extern NEWCHARA NewWaterLineFall;
extern NEWCHARA NewEquipMenuControl;
extern NEWCHARA NewGetFortunePos;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA ComMaxPlantDogtags;
extern NEWCHARA NewExtinguisher;
extern NEWCHARA NewVRPause_Scn;
extern NEWCHARA NewGetFortuneBombPos;
extern NEWCHARA NewPutTexturePlant;
extern NEWCHARA NewRipBubbleMan;
extern NEWCHARA NewWeaponSplash;
extern NEWCHARA NewSIG_SetDogTagFlag;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewGclSin;
extern NEWCHARA GM_COM_SnatchEquip;
extern NEWCHARA PL_COM_SetProcWaterInOut;
extern NEWCHARA NewTdgCameraInitialize;
extern NEWCHARA NewVRWall;
extern NEWCHARA PL_COM_ChangePrevWeaponScn;
extern NEWCHARA NewManhattan3D_Fog;
extern NEWCHARA NewWebSite;
extern NEWCHARA NewPutGlassObject;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewEMA_CommandGetLife;
extern NEWCHARA C4MAN_Init;
extern NEWCHARA NewSelScrMan;
extern NEWCHARA COM_SetClearingData;
extern NEWCHARA NewFortHideObject;
extern NEWCHARA NewDelay;
extern NEWCHARA NewBossrushScr;
extern NEWCHARA PL_COM_SightOnOff;
extern NEWCHARA NewEMA_CommandStreamPlayEma;
extern NEWCHARA NewTheaterScr;
extern NEWCHARA NewVRChild;
extern NEWCHARA NewCmdKamomeVisible;
extern NEWCHARA SIG_DogTagZeroClear;
extern NEWCHARA VR_ScnAddEnemy;
extern NEWCHARA NewHostageCommander;
extern NEWCHARA NewSpecialMenu_Eng;
extern NEWCHARA COM_ChangeClearingRoot;
extern NEWCHARA NewGetOrgaSrcDst;
extern NEWCHARA NewEasyLayout;
extern NEWCHARA NewSkyColumn;
extern NEWCHARA NewStrmFader;
extern NEWCHARA GM_SetVolumeParam;
extern NEWCHARA GM_Scn_SetRouteOffset;
extern NEWCHARA VR_GoalIn;
extern NEWCHARA NewRadarMovie;
extern NEWCHARA NewStreamStatus;
extern NEWCHARA NewSpotRain;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewVRCubeLife;
extern NEWCHARA NewEESwim;
extern NEWCHARA NewEmmaSlater;
extern NEWCHARA NewTimer;
extern NEWCHARA COM_SetPicIdBreakComputerMonitor;
extern NEWCHARA VRCLR_GetResult;
extern NEWCHARA VR_SetTargetSpeedRate;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewBloodWall;
extern NEWCHARA PL_COM_BladeKillPlugin;
extern NEWCHARA NewCypher;
extern NEWCHARA VR_TimerStart;
extern NEWCHARA NewSpecialScr;
extern NEWCHARA PL_COM_VisibleO2Gage;
extern NEWCHARA GM_GetGameStatusForScn;
extern NEWCHARA NewVRWallMarker_Scn;
extern NEWCHARA VRMap3D_ResetBomb;
extern NEWCHARA NewSetupLadder;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewSeaSlater;
extern NEWCHARA NewSplushMan;
extern NEWCHARA VR_SetTargetHideAppearCycle;
extern NEWCHARA NewVRTarget1;
extern NEWCHARA NewDogtagMng;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA NewExecProcContinual;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA ComNPCSnakeSetStage;
extern NEWCHARA PL_COM_InvisiblePlayer;
extern NEWCHARA COM_GetEnemyStSt;
extern NEWCHARA PL_COM_BladeEffectOn;
extern NEWCHARA NewVRSelect_Scn;
extern NEWCHARA NewKamomeManager;
extern NEWCHARA NewEvmLateControl_c100Launch;
extern NEWCHARA NewTargetProc;
extern NEWCHARA VR_SetTargetNGScore;
extern NEWCHARA NewGeneralPoly_Scn;
extern NEWCHARA NewSetCheckCharaOfCamera;
extern NEWCHARA NewMallocCameraCharaWork;
extern NEWCHARA NewGhost;
extern NEWCHARA NewSetCheckSquareOfCamera;
extern NEWCHARA NewEmmaLocker;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA PL_COM_SetNoUse;
extern NEWCHARA NewDoor;
extern NEWCHARA NewDustOnCamera;
extern NEWCHARA NewDefCommander;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewCorpseWalk;
extern NEWCHARA NewRotateY_Object;
extern NEWCHARA NewPreviousStory;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewEMA_CommandGetLink;
extern NEWCHARA NewIceSpray_mng;
extern NEWCHARA NewStreamStop;
extern NEWCHARA NewRain;
extern NEWCHARA NewAddBreathPoint;
extern NEWCHARA NewScreenShotControl;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA NewDEMOSetKaitaiC4;
extern NEWCHARA NewRopeModel2;
extern NEWCHARA NewGetOrgaLife;
extern NEWCHARA NewCom_FRT_CEL_Control;
extern NEWCHARA NewCamera;
extern NEWCHARA NewPutSTanimeObjectCall_4000Launch;
extern NEWCHARA NewVRBook;
extern NEWCHARA ComSetReferenceData;
extern NEWCHARA DM_SendEffectMessage_4001Launch;
extern NEWCHARA NewInterPoly_Demo_4002Launch;
extern NEWCHARA Command_GetFallFloorStatus;
extern NEWCHARA NewWaterModeSetting;
extern NEWCHARA NewVADemo_4004Launch;
extern NEWCHARA ENEMEM_GclSet;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA GM_SeSetVolPanGcl;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewSigBreakStage;
extern NEWCHARA NewLocalWind;
extern NEWCHARA NewPutVanimeObject;
extern NEWCHARA Msn_SetDefaultData;
extern NEWCHARA NewSetSubjectShotLen;
extern NEWCHARA NewStageOutline_Scn;
extern NEWCHARA NewFortCeiling;
extern NEWCHARA NewFontInit;
extern NEWCHARA NewResidentResourceSet;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewCameraOscillation;
extern NEWCHARA NewVRClear_Scn;
extern NEWCHARA NewCheckSlater;
extern NEWCHARA NewPluginGraspEE;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewRemoveTrapBind;
extern NEWCHARA NewBetweenCamera;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetTimerStatus2;
extern NEWCHARA NewPluginSuperBlow;
extern NEWCHARA NewCypher4snipe;
extern NEWCHARA NewUnsetTimerStatus2;
extern NEWCHARA NewGclAssert;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA NewKbdExec;
extern NEWCHARA NewRayConsol_d000Launch;
extern NEWCHARA NewRayEye_d001Launch;
extern NEWCHARA NewWaterPollute_demo_d002Launch;
extern NEWCHARA COM_ClearingCancel;
extern NEWCHARA NewWcFlush_Scn;
extern NEWCHARA NewRayMissileShower_d003Launch;
extern NEWCHARA NewBlinkFloor;
extern NEWCHARA NewRayMonoEye_Demo_d004Launch;
extern NEWCHARA NewPhotoTerminal;
extern NEWCHARA NewRayMissileForDemo_d005Launch;
extern NEWCHARA NewRayFootSplash_d006Launch;
extern NEWCHARA NewRayOozeBloodDemo_d007Launch;
extern NEWCHARA PDRAY_OozeBloodAddDemo_d008Launch;
extern NEWCHARA NewSetBeltConveyer;
extern NEWCHARA New2DSprite_Pause;
extern NEWCHARA PDRAY_CLOUD_DeleteDensity_d009Launch;
extern NEWCHARA NewGclGetRTC;
extern NEWCHARA NewCartridgeUspALL;
extern NEWCHARA NewVRFloorPanel_Scn;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewHzxResetGroupAdd;
extern NEWCHARA NewDemoRayFallBlood_d00aLaunch;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewFogWave_d00bLaunch;
extern NEWCHARA NewPutMonitor;
extern NEWCHARA NewStreamSetPan;
extern NEWCHARA NewBodySplashScn;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA NewResidentAreaSet;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewRadarZoomRateControl;
extern NEWCHARA NewSetLockerMotion;
extern NEWCHARA NewSolidus;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewGetLeftTime;
extern NEWCHARA NewBubbleOnCamera;
extern NEWCHARA NewWatertightDoorStatus;
extern NEWCHARA NewVRWallScarBase;
extern NEWCHARA ComMaxTankerDogtags;
extern NEWCHARA NewGetFortuneBulletPos;
extern NEWCHARA NewLoadGameScr;
extern NEWCHARA NewVampShdwTrgt;
extern NEWCHARA CodecStatus;
extern NEWCHARA GetNPCSnakeLife;
extern NEWCHARA NewScenarioClaymore;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewShortSparkLine;
extern NEWCHARA NewGetLeftTime2;
extern NEWCHARA NewEMA_CommandGetMiceVolPan;
extern NEWCHARA NewVRTarget1life;
extern NEWCHARA PL_COM_DestroyBox;
extern NEWCHARA ComGetLoaddataTime;
extern NEWCHARA NewAttachment;
extern NEWCHARA NewTs_SubWindow_S;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewAttachment2;
extern NEWCHARA NewAttachment3;
extern NEWCHARA NewPutWorldTexFadeCamera;
extern NEWCHARA UTL_EFT_AddBoundFromGCL;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA CPE_CamPoseInit;
extern NEWCHARA NewModelPatAnimation;
extern NEWCHARA NewScrShimmer;
extern NEWCHARA NewSetChanlTargetMap;
extern NEWCHARA NewFallFloor;
extern NEWCHARA NewHako_purapura;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewUserDogtagSet;
extern NEWCHARA NewMpegPssMovieStr;
extern NEWCHARA PL_COM_WtDoorHandle2TurnProc;
extern NEWCHARA NewVrObject;
extern NEWCHARA KR_GetHostageTarget;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA GM_COM_SetBossSurvivalStatus;
extern NEWCHARA NewMzTwindoor;
extern NEWCHARA NewSetCameraProc;
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewBeltConveyerBelt;
extern NEWCHARA PL_COM_FloorHeight;
extern NEWCHARA NewStormyRiverWaveSet;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewManhatLight_L1;
extern NEWCHARA NewManhatLight_L2;
extern NEWCHARA GM_ComMapChange;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA NewPassingSplash;
extern NEWCHARA NewNode;
extern NEWCHARA NewAnmtexSet;
extern NEWCHARA NewEntrySteamAttackTargetSmall;
extern NEWCHARA GLLPhotoCheck_Scn;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA VR_TimerPause;
extern NEWCHARA NewEmma;
extern NEWCHARA NewVrPitFall;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewSigDefender;
extern NEWCHARA PL_COM_OnCorpseStand;
extern NEWCHARA PL_COM_SetRecoverValue;
extern NEWCHARA NewGetTimerStatus;
extern NEWCHARA GM_COM_RegainEquip;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewBrooklyn3D_Fog;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA NewEntrySteamAttackTargetMiddle;
extern NEWCHARA NewZman;
extern NEWCHARA NewBombArea;
extern NEWCHARA NewTengMonitorControlW42a;
extern NEWCHARA NewGlassScarBase;
extern NEWCHARA NewBridgeExplosionScn;
extern NEWCHARA PL_SprayCheckWide;
extern NEWCHARA NewPluginWaterMode;
extern NEWCHARA NewCmdDeadKamomeInvisible;
extern NEWCHARA NewGclLangUpdate;
extern NEWCHARA NewScrTrans;
extern NEWCHARA NewClearedGameMenuScr;
extern NEWCHARA NewWatertightDoor;
extern NEWCHARA NewEntryVRWallScar;
extern NEWCHARA NewTimerEnd;
extern NEWCHARA NewCmdFatInvisible;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewEMA_CommandStreamPlaySna;
extern NEWCHARA NewFloorLightMan;
extern NEWCHARA NewBubbleManyColorSet;
extern NEWCHARA NewHarBGM_Manager;
extern NEWCHARA NewTitleScrMan;
extern NEWCHARA NewSetPlayerLockerMotion;
extern NEWCHARA NewCom_DestroyLight;
extern NEWCHARA NewTimer2;
extern NEWCHARA NewCylinderImage;
extern NEWCHARA NewElectricFloor;
extern NEWCHARA NewGclAbs;
extern NEWCHARA NewCameraPoseEnemyManager;
extern NEWCHARA NewPluginBeltConveyer;
extern NEWCHARA NewTngCommander;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA NewSlitLight2;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewReflectionWaterSurface2Set;
extern NEWCHARA NewCmdKamomeInvisible;
extern NEWCHARA NewBrooklyn3D;
extern NEWCHARA NewHarItemSet;
extern NEWCHARA NewHar_CommandGetClaster;
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewBubbleLineScn;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewGunCamera;
extern NEWCHARA NewHarKasacka;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewFadeInOut_Demo_0003Launch;
extern NEWCHARA NewAlbumMng;
extern NEWCHARA NewRainCamera_Demo_0004Launch;
extern NEWCHARA GM_OnlyCheckControl;
extern NEWCHARA NewFlush_0005Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewContrast_Demo_0009Launch;
extern NEWCHARA PL_COM_CautionSquatOnly;
extern NEWCHARA GM_GetFallTypeForGCL;
extern NEWCHARA VR_SetTargetStrength;
extern NEWCHARA PL_COM_SetMissileBounding;
extern NEWCHARA COM_GetEnemyThinkStatus;
extern NEWCHARA New_Figure_Raven;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewPutBigGlassObject;
extern NEWCHARA PL_SprayCheckNormal;
extern NEWCHARA NewContrastForce_Demo_000aLaunch;
extern NEWCHARA NewSK_ScopeSight_000bLaunch;
extern NEWCHARA VR_StagePause;
extern NEWCHARA NewScrCrack_000cLaunch;
extern NEWCHARA New2DSprite_Prog_000dLaunch;
extern NEWCHARA NewCigarette_Demo_000eLaunch;
extern NEWCHARA GM_COM_PauseOff;
extern NEWCHARA NewLoadRestart;
extern NEWCHARA NewLensFlare_Demo_000fLaunch;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewSubjectDemoPlayer;
extern NEWCHARA NewStreamPauseControl;
extern NEWCHARA NewSigRadPoint;
extern NEWCHARA NewKac_CommandGetKasLife;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSigSinkTanker;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA PL_GetDGCamSubject;
extern NEWCHARA NewCrossFadeEffect_0012Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA NewScrWater_Demo_0014Launch;
extern NEWCHARA NewEMA_CommandForceMove;
extern NEWCHARA NewScrConcentrateBlur_0015Launch;
extern NEWCHARA NewPutTexNoTarget;
extern NEWCHARA NewFortHangLight;
extern NEWCHARA NewRaySight_0016Launch;
extern NEWCHARA NewVtrSight_0017Launch;
extern NEWCHARA NewCom_BRK_SPLGT_BreakMessage;
extern NEWCHARA NewSvPlantCamera;
extern NEWCHARA Msn_ResetQuickWindow;
extern NEWCHARA OK_FogStatusSet_0018Launch;
extern NEWCHARA NewRainFogPersFast_0019Launch;
extern NEWCHARA NewKac_CommandGetKasStat;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewInvisibleChara;
extern NEWCHARA New_ChimnySmoke;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewFlashLight;
extern NEWCHARA NewDEMODigitalCamera_001aLaunch;
extern NEWCHARA NewMCCheckScr;
extern NEWCHARA NewPsg1Sight_001cLaunch;
extern NEWCHARA NewBosKasacka;
extern NEWCHARA NewDEMO_Equip_001dLaunch;
extern NEWCHARA NewSky_Prev;
extern NEWCHARA DM_ControlChange_ffff0Launch;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewVRWallLife;
extern NEWCHARA NewDemoSun_001fLaunch;
extern NEWCHARA NewRainParts;
extern NEWCHARA PL_COM_GetElevatorStatus;
extern NEWCHARA NewCameraSwingMode;
extern NEWCHARA NewUbSvCamera;
extern NEWCHARA PL_COM_VitalityAdjust;
extern NEWCHARA NewWindowRain;
extern NEWCHARA NewScnAiRaySight_0020Launch;
extern NEWCHARA NewPutSpeechObject;
extern NEWCHARA VR_StageStart;
extern NEWCHARA NewExplosionControl_Demo_0021Launch;
extern NEWCHARA NewPadDemoPlay;
extern NEWCHARA COM_GetEnemyPos;
extern NEWCHARA NewNyou_0022Launch;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewFortuneTear_0023Launch;
extern NEWCHARA NewScrGoggles_demo_0024Launch;
extern NEWCHARA NewGllPhoto;
extern NEWCHARA NewGetOrgaPos;
extern NEWCHARA NewScrDrop_demo_0025Launch;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewScrWaterFilm_0026Launch;
extern NEWCHARA NewPutBottleObject;
extern NEWCHARA NewSigRouteModel2;
extern NEWCHARA NewPutTVObject;
extern NEWCHARA NewOrgaPutSearchLightObject;
extern NEWCHARA NewCheckBoxHidden;
extern NEWCHARA PL_COM_BladeNoCheckSegment;
extern NEWCHARA NewScrBubble;
extern NEWCHARA NewTitleBackground;
extern NEWCHARA PL_COM_SetForceActLoop;
extern NEWCHARA NewDropBodySplushScn;
extern NEWCHARA NewBeltObjCheckInsideTrap;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSplashMotion_Demo_0100Launch;
extern NEWCHARA NewDivingGoggles;
extern NEWCHARA NewSplashRipple_Demo_0101Launch;
extern NEWCHARA NewFootSplash_0103Launch;
extern NEWCHARA NewPutHangLight;
extern NEWCHARA NewRainFogPersFast2_0031Launch;
extern NEWCHARA NewBodySplash3_0104Launch;
extern NEWCHARA NewDiveSplash_Parent_0105Launch;
extern NEWCHARA NewSphereSplush_0106Launch;
extern NEWCHARA NewRainFogPersDemo_0033Launch;
extern NEWCHARA NewPutPotatoObject;
extern NEWCHARA NewWaveSplash_Demo_0107Launch;
extern NEWCHARA NewWallTidal_0108Launch;
extern NEWCHARA NewWaterWindSplush_DEMO_0109Launch;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewBlur;
extern NEWCHARA ComLoadOptions;
extern NEWCHARA NewSigGunSpin;
extern NEWCHARA NewPutHoloObject;
extern NEWCHARA NewAutoMenuSelectManagerSet;
extern NEWCHARA NewBgmFader;
extern NEWCHARA NewPuddle;
extern NEWCHARA NewOrga;
extern NEWCHARA NewLensFlr_GameScn;
extern NEWCHARA NewPassageWaterSet;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewWindManager;
extern NEWCHARA SK_BossPauseVisibleInvisible;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewLockerCP;
extern NEWCHARA NewVentilatorLight;
extern NEWCHARA NewVRFloorMarker_Scn;
extern NEWCHARA NewGWB_LinesModel;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewWaterDropManager;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewPutMagazineObject;
extern NEWCHARA SetGameStatusVROnly;
extern NEWCHARA PL_COM_BulletSplashDaemon;
extern NEWCHARA NewW25Defender;
extern NEWCHARA NewDoorPannelSpark;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewO2Gage;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewVRMap3D_Scn;
extern NEWCHARA NewRand;
extern NEWCHARA ENEMEM_GclReset;
extern NEWCHARA NewSpark_6000Launch;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA ComSetProductCode;
extern NEWCHARA NewPlasmaPoly_Demo_6002Launch;
extern NEWCHARA NewDemoBulletCall_6005Launch;
extern NEWCHARA NewOpticalCamouflageBreakDemo2_6006Launch;
extern NEWCHARA NewDebris_Cm_Demo_6007Launch;
extern NEWCHARA NewDebris_Tex_Demo_6008Launch;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewBombEffect_6009Launch;
extern NEWCHARA NewGetNewItemScr;
extern NEWCHARA NewRainCamera;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA ComNTankerDogtags;
extern NEWCHARA NewPrezNikita;
extern NEWCHARA NewRainFogPers;
extern NEWCHARA NewFlyingSmoke_600aLaunch;
extern NEWCHARA NewConfirmSaveScr;
extern NEWCHARA NewBombGasEffect_600bLaunch;
extern NEWCHARA NewSK_BossResult;
extern NEWCHARA NewPlayerDisap;
extern NEWCHARA NewTs_Spark_600eLaunch;
extern NEWCHARA NewLineSplash;
extern NEWCHARA NewGetConfig;
extern NEWCHARA NewRainSlow;
extern NEWCHARA NewBossHarrier;
extern NEWCHARA NewSetBeltConveyerMode;
extern NEWCHARA NewKac_CommandGetPosition;
extern NEWCHARA NewMesgBomb2_6010Launch;
extern NEWCHARA NewGeneralSprite2_6011Launch;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSonicWave_6012Launch;
extern NEWCHARA NewPutWorldTexFadeCameraForDoor;
extern NEWCHARA NewMesgBomb3_6013Launch;
extern NEWCHARA SK_BossTimeAfterWrite;
extern NEWCHARA NewFortBarrierDemo_6014Launch;
extern NEWCHARA NewDebris_Cm_Demo2_6015Launch;
extern NEWCHARA NewDebris_Tex_Demo2_6016Launch;
extern NEWCHARA NewDemoHarrierMissileFire2_6017Launch;
extern NEWCHARA TNG_GetDestroyNum;
extern NEWCHARA NewCheckEneTrap;
extern NEWCHARA NewDemoStageFire_6018Launch;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDemoSolidusDashFire_6019Launch;
extern NEWCHARA NewGclCos;
extern NEWCHARA NewTimerEnd2;
extern NEWCHARA NewDropShadow_1000Launch;
extern NEWCHARA NewShadow_1001Launch;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewEneEquip_1002Launch;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewRipBubbleMan_DEMO_0201Launch;
extern NEWCHARA NewRaidenMaskBubbleDemo_0202Launch;
extern NEWCHARA NewRaincoat_called_1005Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewPutAttachments_1007Launch;
extern NEWCHARA NewParrotFeather_601aLaunch;
extern NEWCHARA NewGbsHandDemo_1008Launch;
extern NEWCHARA NewDemoSolidusDashFire2_601bLaunch;
extern NEWCHARA NewGbsFaceDemo_1009Launch;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewPoolWaterComplex;
extern NEWCHARA NewMAOParticle_601cLaunch;
extern NEWCHARA NewGetHoloStatus;
extern NEWCHARA NewKamomeFeather_601dLaunch;
extern NEWCHARA NewPutWorldTex;
extern NEWCHARA NewHexagonalPattern_601fLaunch;
extern NEWCHARA VMP_GetVmpStatus;
extern NEWCHARA NewBodyShadowVolume_Demo_100aLaunch;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA ComStoreLoadedVariable;
extern NEWCHARA NewPutAttachments_100cLaunch;
extern NEWCHARA NewBulletBarriar;
extern NEWCHARA NewEyeControl_100dLaunch;
extern NEWCHARA NewTargetTrap;
extern NEWCHARA NewDemoArkms_100fLaunch;
extern NEWCHARA NewSaveGameScr;
extern NEWCHARA NewDemoBodyPlasma_6020Launch;
extern NEWCHARA NewDemoElectricFloor_6021Launch;
extern NEWCHARA NewFortElevatorButton;
extern NEWCHARA NewDemoSolidusMissileFire_6022Launch;
extern NEWCHARA NewDemoSolidusMissileSmoke_6023Launch;
extern NEWCHARA NewFatmanPuppetIKSupportSet;
extern NEWCHARA NewDemoBladeSpark_6024Launch;
extern NEWCHARA NewDemoSolidusSnakearmFlow_6025Launch;
extern NEWCHARA NewSlowParamMan;
extern NEWCHARA NewDemoSolidusBladeLight_6026Launch;
extern NEWCHARA NewDemoSolidusEnergyPrim_6027Launch;
extern NEWCHARA NewIrs2;
extern NEWCHARA NewDemoBladeFlow_6028Launch;
extern NEWCHARA ComGetNowTime;
extern NEWCHARA NewPlasmaEvade_6029Launch;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewArmsEffectControl_1010Launch;
extern NEWCHARA NewMallocCameraSquareWork;
extern NEWCHARA AccessMobile;
extern NEWCHARA NewDemoArkms_NYPD_1011Launch;
extern NEWCHARA NewCreateEquipment_1012Launch;
extern NEWCHARA NewPutIceboxObject;
extern NEWCHARA PL_COM_GetBeltConvStatus;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewFortEquip_1014Launch;
extern NEWCHARA NewEmmaEquip_1015Launch;
extern NEWCHARA NewCom_BRK_SPLGT_LightMessage;
extern NEWCHARA NewEyeAnimSEALS_Demo_1016Launch;
extern NEWCHARA NewFortSling_1017Launch;
extern NEWCHARA NewDemoPlasmaLineColor_602aLaunch;
extern NEWCHARA NewOffMan;
extern NEWCHARA NewFortBulletDemo_1018Launch;
extern NEWCHARA PL_COM_SubjectArmChain;
extern NEWCHARA NewPutAttachments_Scn;
extern NEWCHARA NewLinerGunInitEffect_1019Launch;
extern NEWCHARA NewBreakComputerW24d;
extern NEWCHARA NewDemoCypherPlasma_602cLaunch;
extern NEWCHARA NewManhattan3D;
extern NEWCHARA NewScrHexFade;
extern NEWCHARA NewWindNoise;
extern NEWCHARA NewSteamOnCameraScenario;
extern NEWCHARA NewOrgFaceEft_101aLaunch;
extern NEWCHARA NewPluginLadder;
extern NEWCHARA NewDemoArmControl_101bLaunch;
extern NEWCHARA NewDependArms_101cLaunch;
extern NEWCHARA NewDemoDoll;
extern NEWCHARA NewDependArms_Aks_101dLaunch;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewBreathDemo_101eLaunch;
extern NEWCHARA NewPutAttachments_101fLaunch;
extern NEWCHARA NewDebris_Cm_DemoBlood_6030Launch;
extern NEWCHARA NewDebris_Tex_DemoBlood_6031Launch;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewLockerDaemon;
extern NEWCHARA NewForceActCancel;
extern NEWCHARA NewPlayerAp;
extern NEWCHARA NewSigRouteModel;
extern NEWCHARA NewCartridgeM4_demo_gunALL;
extern NEWCHARA NewOptionScr;
extern NEWCHARA NewRunTelop_Trial_a002Launch;
extern NEWCHARA NewVRSys;
extern NEWCHARA NewSplushSurfaceMan_a003Launch;
extern NEWCHARA GM_GetStageHappeningForScn;
extern NEWCHARA PL_RaidenEquipmentManager_1020Launch;
extern NEWCHARA NewRippleMan_a004Launch;
extern NEWCHARA NewDependArms_USP_SP_1021Launch;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewScnEvm_SkirtA_1022Launch;
extern NEWCHARA NewStageFireScn;
extern NEWCHARA NewScnEvm_SkirtB_1023Launch;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA NewFortSling2_1024Launch;
extern NEWCHARA NewPutPlateObject;
extern NEWCHARA NewCharaBreath;
extern NEWCHARA NewFortWallLight;
extern NEWCHARA NewWine_1027Launch;
extern NEWCHARA NewSvCamera;
extern NEWCHARA NewDemoArkms_CitMale_1028Launch;
extern NEWCHARA NewDemoArkms_CitFemale_1029Launch;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewExtraUmiSaraDraw;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA UTL_EFT_ExecCallback;
extern NEWCHARA NewDependArms_Kill_102aLaunch;
extern NEWCHARA NewVRSky_Scn;
extern NEWCHARA NewPutRadleObject;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewHugeSeaSurfaceSet;
extern NEWCHARA PL_COM_Sneeze;
extern NEWCHARA NewFlyingWarmScn;
extern NEWCHARA NewGclGetPrezLife;
extern NEWCHARA NewPDRayStageEnvironmentSet;
extern NEWCHARA NewSpinModel;
extern NEWCHARA NewDust;
extern NEWCHARA NewSpotDropHazard;
extern NEWCHARA NewPadControl;
extern NEWCHARA GM_SetZoneFlagForScn;
extern NEWCHARA NewElevator;
extern NEWCHARA NewPutElevator;
extern NEWCHARA NewTengMonitorControlW44a;
extern NEWCHARA NewEMA_CommandGetPosition;
extern NEWCHARA NewPDRayServerSet;
extern NEWCHARA NewVramInitialize;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_BOMBS_NUM;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewBloodWater_Scn;
extern NEWCHARA NewVRWindow_Scn;
extern NEWCHARA NewSlowDown;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewGclDiffDirABS;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA NewPluginKageshibari;
extern NEWCHARA GM_GetPosForGCL;
extern NEWCHARA NewCom_FRT_HNG_GetBreakFlag;
extern NEWCHARA NewPutSeSound;
extern NEWCHARA VR_SetTargetScore;
extern NEWCHARA DM_DebugPrint_fff00Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA DM_SetClipParam_fff02Launch;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewVRSparkBase;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewGclInsideChk4P;
extern NEWCHARA VR_PhotoClear;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA NewAllSlater;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSetTimerStatus;
extern NEWCHARA NewOceanWave;
extern NEWCHARA NewShadowDropHazard;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewFadeObj;
extern NEWCHARA NewSelect;
extern NEWCHARA NewPadCancel;
extern NEWCHARA PL_COM_SetJumpGravity;
extern NEWCHARA NewBlood_Demo_7001Launch;
extern NEWCHARA NewBlood2_Demo_7002Launch;
extern NEWCHARA AN_Blood_Mist_7003Launch;
extern NEWCHARA NewTraffic_Demo_07d1Launch;
extern NEWCHARA COMVRGameOverProc;
extern NEWCHARA COM_GetCaptureModelName;
extern NEWCHARA NewSK_BossTelop;
extern NEWCHARA NewSeaSurfaceSet;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewNodeFrame;
extern NEWCHARA PL_COM_IntrudeSubjectCameraPositionMove;
extern NEWCHARA NewSniperVamp;
extern NEWCHARA NewBloodWeep_700bLaunch;
extern NEWCHARA NewPeterBlood_700cLaunch;
extern NEWCHARA NewDemoSplashBlood_700dLaunch;
extern NEWCHARA NewBeltObject;
extern NEWCHARA NewComPUT_ELV_GetPos;
extern NEWCHARA NewDemoDummyPointBlood_700eLaunch;
extern NEWCHARA NewPutHideObject;
extern NEWCHARA NewBlood_Demo2_700fLaunch;
extern NEWCHARA NewSoundTest;
extern NEWCHARA NewScrWater;
extern NEWCHARA NewShadowDrawObjectSet;
extern NEWCHARA NewRadar;
extern NEWCHARA NewCheckMapIsDisplay;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA SK_ScnBombReset;
extern NEWCHARA NewBeltObjectManager;
extern NEWCHARA NewVRGoal;
extern NEWCHARA NewStreamIsPlay;
extern NEWCHARA NewDeleteCheckCharaOfCamera;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewShadowDrawObject2Set;
extern NEWCHARA NewThunder_Demo_2000Launch;
extern NEWCHARA COM_GetEnemyStatus2;
extern NEWCHARA NewLocalWind_Demo_2002Launch;
extern NEWCHARA NewRipple;
extern NEWCHARA NewSlowParamMan_prog_2003Launch;
extern NEWCHARA SK_RootPositionChange;
extern NEWCHARA NewLocalWind2_Demo_2004Launch;
extern NEWCHARA NewBodySlater;
extern NEWCHARA GM_COM_ChangeMapConnectStatus;
extern NEWCHARA NewPutFlagObject;
extern NEWCHARA NewEnding;
extern NEWCHARA NewDemoSlater;
extern NEWCHARA AccessPhotoTerm;
extern NEWCHARA NewPutSwingObject;
extern NEWCHARA NewTRAP_C4;
extern NEWCHARA PL_COM_ChangePrevItemScn;
extern NEWCHARA COM_GetEnemyStatus;
extern NEWCHARA NewGasInWater;
extern NEWCHARA NewCircleLightSpot;
extern NEWCHARA PL_CHARA_ScnVibCamera;
extern NEWCHARA NewParallelSet;
extern NEWCHARA COM_GetCaptureName;
extern NEWCHARA NewHoldEnemy;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewW32Commander;
extern NEWCHARA NewIPUDisplayPanelSet;
extern NEWCHARA NewSigTengA;
extern NEWCHARA NewDemoSnakeArm;
extern NEWCHARA NewSigTengB;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA SetVrStageId;
extern NEWCHARA NewPlayerRecoverCold;
extern NEWCHARA NewTengMonitorControl;
extern NEWCHARA NewEmmaClaymore;
extern NEWCHARA NewSmokeBlurEffect_7100Launch;
extern NEWCHARA ComStoreOptions;
extern NEWCHARA NewCommonSmoke_7101Launch;
extern NEWCHARA NewGetOrgaActFlag;
extern NEWCHARA NewDynamicFlow_Demo_7102Launch;
extern NEWCHARA NewLineSmoke_7103Launch;
extern NEWCHARA NewPutBreakObject;
extern NEWCHARA HLD_GetHoldPosition;
extern NEWCHARA NewDemoHarrierMissileSmokeLine2_7104Launch;
extern NEWCHARA NewDemoHarEffect_7105Launch;
extern NEWCHARA NewProjectorLight;
extern NEWCHARA NewDemoHarrierDamageSmoke_7106Launch;
extern NEWCHARA NewDemoRisingSmoke_7107Launch;
extern NEWCHARA NewSmokeReactionClient_7108Launch;
extern NEWCHARA NewDemoRisingSmokeFix_7109Launch;
extern NEWCHARA NewWaterLevelControl_Demo_b000Launch;
extern NEWCHARA NewUSPLightOn;
extern NEWCHARA EE_COM_SetLife;
extern NEWCHARA NewDropBodySplush_b001Launch;
extern NEWCHARA OK_PutSplushSurface_b002Launch;
extern NEWCHARA NewSplushTidal_b003Launch;
extern NEWCHARA NewCheckWaterLevel;
extern NEWCHARA NewParrot2;
extern NEWCHARA NewSeaSurfaceReactionObject_b004Launch;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewAutoSplush_b005Launch;
extern NEWCHARA NewAutoSplush_EftCtrl_b006Launch;
extern NEWCHARA NewSTSelect_Scn;
extern NEWCHARA NewBloodWater_demo_b007Launch;
extern NEWCHARA NewWatertightDoorManager;
extern NEWCHARA NewRisingSmokeScn;
extern NEWCHARA NewDemoRisingSmokeDummy_710aLaunch;
extern NEWCHARA EE_COM_GetO2;
extern NEWCHARA COM_GetEnemyAction;
extern NEWCHARA NewPutSearchLightObject;
extern NEWCHARA NewFatmanDemoScratchSmoke_710cLaunch;
extern NEWCHARA NewComPUT_ELV_EraseBomb;
extern NEWCHARA NewSmokeStripControl_710dLaunch;
extern NEWCHARA NewSmokeMitsukoshiMan_710eLaunch;
extern NEWCHARA NewWaterDustManager;
extern NEWCHARA NewPeterBlood_Gas_710fLaunch;
extern NEWCHARA NewSK_Result;
extern NEWCHARA NewHar_CommandGetPosition;
extern NEWCHARA NewSK_BossPause;
extern NEWCHARA NewLockerStatus;
extern NEWCHARA NewDebugPause;
extern NEWCHARA GM_SetNoiseGcl;
extern NEWCHARA VR_SetComboLimit;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewElavatorLamp;
extern NEWCHARA GM_SetMicParam;
extern NEWCHARA NewGclAtan;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA NewBreakComputerW12b;
extern NEWCHARA NewDynamicFloor;
extern NEWCHARA NewPutPaperObject;
extern NEWCHARA NewWaterFall;
extern NEWCHARA NewGetPlayerMapName;
extern NEWCHARA NewFortDynamicFlow;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA NewNPCSnake;
extern NEWCHARA NewSpotLightImageSet;
extern NEWCHARA NewPDRaySet;
extern NEWCHARA NewCreateEquipmentScn;
extern NEWCHARA NewW32Defender;
extern NEWCHARA NewLocker;
extern NEWCHARA NewManhatLight;
extern NEWCHARA GM_ComSetMapChangeTrap;
extern NEWCHARA NewSetKaitaiC4;
extern NEWCHARA NewTrueMirror;
extern NEWCHARA NewPutSnakeHideObject;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewGetLapTime;
extern NEWCHARA NewDustArea;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewSpecialMenu_Page;
extern NEWCHARA NewBuildingWindow;
extern NEWCHARA SetVRBOMBS;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewGollgon;
extern NEWCHARA NewCodec;
extern NEWCHARA NewFlowPaper;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewNodeDaemon;
extern NEWCHARA NewCommander;
extern NEWCHARA NewSplashFloor_Scn;
extern NEWCHARA NewStreamSet;
extern NEWCHARA NewPutTreeObject;
extern NEWCHARA NewSetPlayerBeltConveyerProc;
extern NEWCHARA NewFatman;
extern NEWCHARA PIC_TankerPicSaveScn;
extern NEWCHARA NewEMA_CommandGetFloor;
extern NEWCHARA NewFlour;
extern NEWCHARA VR_SetTrgScale;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewMobileDaemon;
extern NEWCHARA COM_GetEnemyNoticeMode;
extern NEWCHARA NewOilSpread;
extern NEWCHARA NewFortune;
extern NEWCHARA NewEntrySteamAttackTargetLarge;
extern NEWCHARA NewFlotingBridgeSet;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewReadSavedVariable;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA NewCorpGcl;
extern NEWCHARA GM_UnSetZoneFlagForScn;
extern NEWCHARA NewNodeLamp;
extern NEWCHARA NewExtraUmiDraw;
extern NEWCHARA NewEMA_CommandStreamStop;
extern NEWCHARA NewPluginElevator;
extern NEWCHARA NewWaterMine;
extern NEWCHARA NewFakeGameOverSet;
extern NEWCHARA PL_CHARA_SetSubjectLimit;
extern NEWCHARA NewPutVentObject;
extern NEWCHARA NewBreathPointManager;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA GM_COM_PauseOn;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA SK_BossResultChenge;
extern NEWCHARA NewThund_Flash;
extern NEWCHARA NewKan;
extern NEWCHARA NewSplashRotate;
extern NEWCHARA SetVRStatus;
extern NEWCHARA GCL_DG_SetTmpLight2;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_GetDemoSnakeArmHangPos;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA PL_COM_BladeSlashCheck;
extern NEWCHARA NewPowerSupplyLight;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA HSTG_GetHostageStatus;
extern NEWCHARA TAKABE_RiseWaveEx_b040Launch;
extern NEWCHARA NewRopeModel3_called_3004Launch;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewPluginFall;
extern NEWCHARA ComNPlantDogtags;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewGeneralSprite_Scn;
extern NEWCHARA NewHostage;
extern NEWCHARA NewKillPlayer;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA FRT_AIM_InitAimSpot;
extern NEWCHARA NewGameInit;
extern NEWCHARA NewRaidenMaskBubbleScn;
extern NEWCHARA NewBrooklyn_R2;
extern NEWCHARA NewScrWaterFilm;
extern NEWCHARA NewTrampleSlater;
extern NEWCHARA VR_SetBlastRange;
extern NEWCHARA NewCartridgeM4_demoALL;
extern NEWCHARA NewSplashFall_Scn;
extern NEWCHARA NewBeltConveyerManager;
extern NEWCHARA NewSuperVamp;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA NewWavingClothModelW_called_3010Launch;
extern NEWCHARA NewEvmMMOrga_called_3011Launch;
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	{ 0x000016CB, NewRain },
	{ 0x000041E7, NewManhatLight_L1 },
	{ 0x000041E8, NewManhatLight_L2 },
	{ 0x00004C00, NewPluginWaterMode },
	{ 0x0000D1C5, NewStingerSightFlag },
	{ 0x000129C5, GM_UnSetZoneFlagForScn },
	{ 0x000190B3, NewGclAbs },
	{ 0x00019A53, NewGclCos },
	{ 0x0001A4F3, NewSigRadPoint },
	{ 0x0001A6DD, NewLineSplash },
	{ 0x0001D98E, NewGclSin },
	{ 0x00020BD1, NewStreamIsPlay },
	{ 0x000223D1, NewVramInitialize },
	{ 0x000255B6, PL_SprayCheckNormal },
	{ 0x0002EAD3, NewEMA_CommandGetPosition },
	{ 0x000381AC, NewCreateEquipmentScn },
	{ 0x00040715, NewCom_FRT_HNG_GetBreakFlag },
	{ 0x000423C8, NewAnmtexSet },
	{ 0x00053DA0, NewGclVarClear },
	{ 0x0005603B, NewTimerEnd2 },
	{ 0x0005872D, NewFatmanPuppetIKSupportSet },
	{ 0x0005B611, NewEMA_CommandGetState },
	{ 0x0005C815, NewShadowDrawObjectSet },
	{ 0x0005D91D, GM_Scn_SetRouteOffset },
	{ 0x00061ADE, NewCamera },
	{ 0x0006A23B, NewCameraOscillation },
	{ 0x0006E0FC, COM_GetEnemyThinkStatus },
	{ 0x00075D93, GetNPCSnakeStatus },
	{ 0x0007E215, NewCheckSlater },
	{ 0x00084719, NewComPUT_ELV_GetPos },
	{ 0x0008BFFC, NewPluginSneeze },
	{ 0x00091159, NewEMA_CommandGetLife },
	{ 0x0009C02F, NewEntrySteamAttackTargetSmall },
	{ 0x0009C0E8, NewEntrySteamAttackTargetLarge },
	{ 0x0009C107, NewEntrySteamAttackTargetMiddle },
	{ 0x000A4D33, NewFortune },
	{ 0x000A7CFB, NewSelScrMan },
	{ 0x000ACF09, NewEntryWallScar },
	{ 0x000B1261, NewRipBubbleMan },
	{ 0x000B6ED3, NewMzTwindoor },
	{ 0x000BA748, NewVentilatorLight },
	{ 0x000C02F4, NewSpotDropHazard },
	{ 0x000C3D82, NewVRMap3D_Scn },
	{ 0x000D02FD, NewMirrorControl },
	{ 0x000DA8BF, NewShadowDropHazard },
	{ 0x000DED8C, NewVrPitFall },
	{ 0x000DFADB, NewGetPlayerStateFlag },
	{ 0x000ECDFD, NewGetPlayerStatus },
	{ 0x000F57BA, NewCom_AddRenameList },
	{ 0x000F9473, NewEMA_CommandStreamPlayEma },
	{ 0x000FF733, SetVR_BOMBS_NUM },
	{ 0x0010CF2F, NewTheaterScr },
	{ 0x00122E63, PL_COM_VitalityAdjust },
	{ 0x00124BF9, PL_COM_BladeKillPlugin },
	{ 0x00128946, NewRaiden },
	{ 0x0012B592, VR_GoalIn },
	{ 0x0012C9D0, NewZakoTelop },
	{ 0x001395B5, NewPowerSupplyLight },
	{ 0x0013CA47, SK_BossTimeAfterWrite },
	{ 0x001418B1, NewTdgCameraInitialize },
	{ 0x0015984A, NewKac_CommandGetKasStat },
	{ 0x00164EE0, NewBulletBarriar },
	{ 0x00170393, NewWaterLineFall },
	{ 0x0017078B, NewHarItemSet },
	{ 0x00175436, NewPutPaperObject },
	{ 0x0017A315, PL_COM_ExchangeCache },
	{ 0x0017EA77, NewTengMonitorControlW42a },
	{ 0x0017EAB7, NewTengMonitorControlW44a },
	{ 0x00182073, NewCom_DestroyLight },
	{ 0x0018D40B, NewJimakuControl },
	{ 0x0018E1F5, VMP_GetVmpStatus },
	{ 0x0018F412, NewTngCommander },
	{ 0x0019358B, PL_COM_CautionSquatOnly },
	{ 0x001AAE5F, NewStreamStatus },
	{ 0x001AF0D5, NewScrBubble },
	{ 0x001AF92A, NewRadar },
	{ 0x001B5854, NewPutStaticSpotLight },
	{ 0x001B970C, NewClearedGameMenuScr },
	{ 0x001C4A30, NewSK_BossTelop },
	{ 0x001C5EB9, NewGetOrgaActFlag },
	{ 0x001CB7B7, PL_COM_GetDemoSnakeArmHangPos },
	{ 0x001CBD93, NewFatman },
	{ 0x001D32ED, PL_COM_ChangePrevWeaponScn },
	{ 0x001D5983, NewVRSys },
	{ 0x001D9C73, NewEMA_CommandStreamPlaySna },
	{ 0x001DE43B, NewVRCubeLife },
	{ 0x001DFDFB, NewVRWallLife },
	{ 0x001E44B7, NewManhattan3D },
	{ 0x001F119A, NewEnding },
	{ 0x001F2A23, NewUserDogtagSet },
	{ 0x001F4349, NewCmdKamomeInvisible },
	{ 0x001F567B, ComNPlantDogtags },
	{ 0x002055CA, NewCheckMapIsDisplay },
	{ 0x00206929, NewShadowDrawObject2Set },
	{ 0x00209027, NewCheckBoxHidden },
	{ 0x0022AE7D, NewSK_Result },
	{ 0x00231FA7, PL_COM_SetObjectFlag },
	{ 0x00247947, GM_ComMapChange },
	{ 0x0024C03E, NewVRWindow_Scn },
	{ 0x0024C830, VR_StageStart },
	{ 0x0024D360, NewLoadGameScr },
	{ 0x00262B46, NewSTSelect_Scn },
	{ 0x00264C7D, New_ChimnySmoke },
	{ 0x00264D3F, NewTdgCameraStatus },
	{ 0x0026AAB9, VR_TimerStart },
	{ 0x00274B60, NewSaveGameScr },
	{ 0x00276926, NewCartridgeM4_demoALL },
	{ 0x0027E580, PL_COM_Sneeze },
	{ 0x002981FA, NewPlayerResetInvincible },
	{ 0x0029C817, NewWaterLevelControl },
	{ 0x0029D24D, NewPluginKageshibari },
	{ 0x002A088B, NewGeneralPoly_Scn },
	{ 0x002A0B46, NewKan },
	{ 0x002A4246, NewMap3D },
	{ 0x002B8E16, NewSuperVamp },
	{ 0x002C4B35, PL_COM_SetRecoverValue },
	{ 0x002D77D4, GM_GetPosForGCL },
	{ 0x002DAD24, NewFarFocusEffectSet },
	{ 0x002DF578, NewShavedSnakeTexReplaceActorScn },
	{ 0x002E2ACB, NewGetFortunePos },
	{ 0x002E4EEB, NewSK_BossPause },
	{ 0x002EB280, NewRadarZoomRateControl },
	{ 0x002EFA26, NewVRTarget1life },
	{ 0x002F3844, NewPluginForceRunMove },
	{ 0x002F9FE3, NewSetTimerStatus2 },
	{ 0x0030E9CC, NewElevator },
	{ 0x00311E8E, NewHostage },
	{ 0x003124A2, PL_CHARA_ScnVibCamera },
	{ 0x003129D0, NewHar_CommandGetClaster },
	{ 0x0031D58B, NewStrmFader },
	{ 0x003259E5, NewSigDefender },
	{ 0x00325C8E, NewGclAtan },
	{ 0x0032FB70, NewStreamStopAll },
	{ 0x00339BD9, NewStreamSetPan },
	{ 0x0033A20F, NewPolygonDemoStart },
	{ 0x0033D38E, SK_BossPauseVisibleInvisible },
	{ 0x00344E5B, GM_GetFallTypeForGCL },
	{ 0x003470C9, PL_COM_SetMissileBounding },
	{ 0x0034CC13, NewReadSavedVariable },
	{ 0x00358D2F, SK_ScnBombSet },
	{ 0x0035A2CF, NewSetMap },
	{ 0x003625B1, NewPutMagazineObject },
	{ 0x00363BE8, COM_SetPicIdBreakComputerMonitor },
	{ 0x003640F6, NewPlayerDisap },
	{ 0x00365DEE, NewPutSpotLightObject },
	{ 0x00368F7D, HSTG_GetHostageStatus },
	{ 0x00374069, NewHarBGM_Manager },
	{ 0x00374BBE, NewSigGunSpin },
	{ 0x00374CB5, TNG_GetDestroyNum },
	{ 0x0037700F, COMVRGameOverProc },
	{ 0x003780F6, NewPlayerAp },
	{ 0x0038D44E, COM_ChangeClearingRoot },
	{ 0x00397FD4, NewPreviousStory },
	{ 0x003A6F25, NewSetKaitaiC4 },
	{ 0x003A9224, NewRand },
	{ 0x003AF581, SK_SpecialGameOver },
	{ 0x003B1909, NewNodeLamp },
	{ 0x003C2410, NewRaidenMaskBubbleScn },
	{ 0x003C56AA, NewNamePrintManager },
	{ 0x003D3B06, NewPlayerRecoverCold },
	{ 0x003D5D04, NewAlbumMng },
	{ 0x003D9D2E, NewResidentAreaSet },
	{ 0x003DFD0B, UTL_EFT_ExecCallback },
	{ 0x003E7B6F, PL_COM_InvisiblePlayer },
	{ 0x003EF35F, NewLensFlr_GameScn },
	{ 0x003F0A3F, NewGetLapTime },
	{ 0x003F46EC, NewPlayerMotionVibOff },
	{ 0x003F55FC, NewBrooklyn3D_Fog },
	{ 0x003F9531, NewMCCheckScr },
	{ 0x003FCD73, PL_CHARA_SetSubjectLimit },
	{ 0x0040F2AE, NewPutIceboxObject },
	{ 0x00413144, NewBeltObject },
	{ 0x004152DF, NewBodySlater },
	{ 0x0041CF4E, NewEntryNewSpark },
	{ 0x0041F1F7, NewBeltConveyerManager },
	{ 0x0042181A, UTL_EFT_AddBoundFromGCL },
	{ 0x00438FFA, NewScrWaterFilm },
	{ 0x0043F718, NewSetCamera },
	{ 0x004481ED, EE_COM_GetLife },
	{ 0x004508C2, NewCom_BRK_SPLGT_LightMessage },
	{ 0x00450C64, NewGetLeftTime },
	{ 0x0045E12E, NewStreamPauseControl },
	{ 0x004610F8, NewGetOrgaPos },
	{ 0x0046139A, NewGlassScarBase },
	{ 0x004683B0, SK_BossResultChenge },
	{ 0x00469B3A, NewPluginStance },
	{ 0x0046B814, GM_COM_ChangeMapConnectStatus },
	{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	{ 0x00470DEF, NewAutoMenuSelectManagerSet },
	{ 0x00475CA4, NewPluginSuperBlow },
	{ 0x00476790, NewPluginFall },
	{ 0x0047D37A, NewKbdExec },
	{ 0x004847D2, NewPutWorldTexFadeCamera },
	{ 0x0048F8F8, NewSpotLightImageSet },
	{ 0x004909B5, SetVRStatus },
	{ 0x00492A40, NewSigRouteModel },
	{ 0x00494239, NewGetNewItemScr },
	{ 0x00497BE6, CodecStatus },
	{ 0x0049D6F1, NewPutAttachments_Scn },
	{ 0x0049EB5E, NewCom_BRK_SPLGT_BreakMessage },
	{ 0x004A0018, NewEasyLayout },
	{ 0x004A243A, NewCodec },
	{ 0x004A3697, NewBeltObjCheckInsideTrap },
	{ 0x004A65BA, NewGetLeftTime2 },
	{ 0x004A777D, NewStormyRiverWaveSet },
	{ 0x004A8B88, NewSetBeltConveyerMode },
	{ 0x004A8DF9, NewSetBeltConveyer },
	{ 0x004A90A1, GLLPhotoCheck_Scn },
	{ 0x004B5435, NewPutFlagObject },
	{ 0x004BAAFE, NewSplashFall_Scn },
	{ 0x004BAB34, NewCharaSpreadScn },
	{ 0x004BF8E6, NewSIG_SetDogTagFlag },
	{ 0x004CBFC5, NewElavatorLamp },
	{ 0x004CEFC1, NewAllocDummyMemory },
	{ 0x004CFE6E, NewVRSelect_Scn },
	{ 0x004E1BD6, NewTrueMirror },
	{ 0x004E8280, PL_COM_WtDoorHandle2TurnProc },
	{ 0x004F576C, PL_COM_SetNudeMode },
	{ 0x004FB2CA, NewPutSnakeHideObject },
	{ 0x004FE997, NewParallelSet },
	{ 0x00502332, NewWaterFall },
	{ 0x00502681, NewBridgeExplosionScn },
	{ 0x0050396F, SK_ScnBombReset },
	{ 0x005141AF, GM_SetMicParam },
	{ 0x0051DC64, NewStingerFlag },
	{ 0x00522DB5, NewSvCamera },
	{ 0x005282A9, PL_COM_SetProcWaterInOut },
	{ 0x0053209A, NewPutHideObject },
	{ 0x00532262, NewPutBreakObject },
	{ 0x00532F32, NewPutRadleObject },
	{ 0x0053BAF5, NewFortDynamicFlow },
	{ 0x0053CFFA, NewBossHarrier },
	{ 0x0053E970, NewSigBreakStage },
	{ 0x00540AC2, GM_GetStageHappeningForScn },
	{ 0x00542B2D, NewForeach },
	{ 0x00547041, NewRisingSmokeScn },
	{ 0x0054B365, NewDoorLamp },
	{ 0x0054ECA9, NewCircleLightSpot },
	{ 0x0055080C, NewTitleBackground },
	{ 0x00552C76, NewIceSpray_mng },
	{ 0x0055B942, NewDoor },
	{ 0x005686AE, NewSetEludeRange },
	{ 0x0056B5CB, NewPluginBlade },
	{ 0x0056E234, NewMapConnect },
	{ 0x0056EF97, NewMapAbout },
	{ 0x005738F0, VR_SetTargetStrength },
	{ 0x00577FB3, NewGunCamera },
	{ 0x0057A485, NewHzxResetGroupAdd },
	{ 0x0059EDA8, NewWcCommander },
	{ 0x0059F23C, NewDropBodySplushScn },
	{ 0x005A4652, NewSetGameOverVox },
	{ 0x005A4809, NewPutGlassObject },
	{ 0x005AE655, NewShowMap },
	{ 0x005B316E, NewSaveVariable },
	{ 0x005B6E21, NewSpecialMenu_Eng },
	{ 0x005BB4D4, NewAtCommander },
	{ 0x005BFE1D, NewPlayerSetCold },
	{ 0x005C0BAE, NewPadDemoPlay },
	{ 0x005C2EAE, NewGhost },
	{ 0x005C3A78, NewPoolWaterComplex },
	{ 0x005C526A, ComLoadOptions },
	{ 0x005CAA88, NewHzxSetGroupAdd },
	{ 0x005CB281, NewManhattan3D_Fog },
	{ 0x005DA36D, NewCypher4snipe },
	{ 0x005DAD0E, NewExtraUmiSaraDraw },
	{ 0x005DE390, VR_ScnAddEnemy },
	{ 0x005EC9C1, NewGollgon },
	{ 0x005ED347, NewVRPause_Scn },
	{ 0x005F8469, NewRemoveTrapBind },
	{ 0x005FBE23, NewWindNoise },
	{ 0x00604185, NewGclDiffDirABS },
	{ 0x00608B23, NewEMA_CommandGetFloor },
	{ 0x0060C857, NewWaterModeSetting },
	{ 0x0060CF19, NewReflectionWaterSurface2Set },
	{ 0x0061274D, NewVRTarget1 },
	{ 0x0061514E, NewSeaSlater },
	{ 0x0061676D, NewEMA_CommandStreamStop },
	{ 0x006263F9, UTL_EFT_InitBound },
	{ 0x006273B5, NewSky_Prev },
	{ 0x0062B1FD, NewBgmFader },
	{ 0x00635875, GM_GameOverStartScn },
	{ 0x006376D3, ComNPCSnakeSetStage },
	{ 0x00641A7B, NewPlantSunMain },
	{ 0x006442CB, NewWindowRain },
	{ 0x00645113, NewCorpGcl },
	{ 0x00645C5B, NewEmmaClaymore },
	{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
	{ 0x0064AFEF, NewWallScarBase },
	{ 0x0064FE6E, NewPassingSplash },
	{ 0x0065180B, NewEMA_CommandGetLink },
	{ 0x00657385, NewSkyColumn },
	{ 0x00661BEE, NewSetCheckSquareOfCamera },
	{ 0x00662F7C, NewBodySplashScn },
	{ 0x00663387, PL_COM_GetBeltConvStatus },
	{ 0x0066B097, NewFogControl },
	{ 0x0066BA4C, NewOceanWave },
	{ 0x0066BA66, NewRipple },
	{ 0x0066F439, NewSetSubjectShotLen },
	{ 0x00675145, NewTrampleSlater },
	{ 0x006760E9, NewCharaBreath },
	{ 0x006779AD, NewPutWorldTex },
	{ 0x006781AF, NewBombEffectScn },
	{ 0x0068392D, NewCartridgeM4_demo_gunALL },
	{ 0x00683BCC, NewBloodWall },
	{ 0x006856C1, GM_GameOverEndScn },
	{ 0x0068CB9C, NewPadVibrationScn },
	{ 0x0068F6CB, NewRainFogPers },
	{ 0x00690610, NewCinemaScreen },
	{ 0x006926D0, NewHostageCommander },
	{ 0x006A030A, NewScrShimmer },
	{ 0x006A0CDA, NewPutHangLight },
	{ 0x006A725A, NewPadControl },
	{ 0x006A9667, PL_COM_SetForceActLoop },
	{ 0x006AD20F, NewDummyPlayer },
	{ 0x006AE654, NewSeaSurfaceSet },
	{ 0x006B237D, NewGclAssert },
	{ 0x006B8EE3, NewRainCamera },
	{ 0x006B8FE4, NewBloodCamera },
	{ 0x006B921A, NewBubbleOnCamera },
	{ 0x006C274D, SetVrStageId },
	{ 0x006C577D, NewPutPotatoObject },
	{ 0x006D61F3, SetVRBOMBS },
	{ 0x006D841B, GM_ComSetMapChangeTrap },
	{ 0x006D85BE, NewSK_BossResult },
	{ 0x006DDD6A, NewSpecialMenu_Page },
	{ 0x006DE9BC, Msn_ResetQuickWindow },
	{ 0x006E0FA8, NewPutVentObject },
	{ 0x006E6572, NewDogtagMng },
	{ 0x006E7F78, NewMenuShipwormSet },
	{ 0x006ECC1A, PL_COM_SetSubjectMove },
	{ 0x006EEB2F, NewSpecialScr },
	{ 0x006F5B0F, NewSetChanlTargetMap },
	{ 0x006F7D4D, NewGameInit },
	{ 0x006FA0A2, NewSlitLight2 },
	{ 0x006FA5F5, GM_COM_SnatchEquip },
	{ 0x006FDA0F, NewFortHideObject },
	{ 0x00709DF6, NewForceActEndProc },
	{ 0x0070A68A, NewBGMManager },
	{ 0x00727B5E, NewSplashRotate },
	{ 0x0072EAD4, NewEquipMenuControl },
	{ 0x0072F23C, NewSigPutModelEvm },
	{ 0x0073E084, PL_COM_FloorHeight },
	{ 0x00741B7A, NewPutHoloObject },
	{ 0x00743C9F, NewDelay },
	{ 0x0074CD16, NewFlotingBridgeSet },
	{ 0x0074E86B, NewGclLangUpdate },
	{ 0x00755E2E, VR_SetTargetScore },
	{ 0x007561AE, NewOrgaPutSearchLightObject },
	{ 0x0075E815, NewPutTVObject },
	{ 0x0076A03C, NewBombArea },
	{ 0x00770EC3, NewPluginElevator },
	{ 0x0077318D, NewGclGetRTC },
	{ 0x0077568D, NewVRWallMarker_Scn },
	{ 0x007798F6, NewOrga },
	{ 0x0077C494, NewDebugRouteViewer },
	{ 0x0077FFC8, NewSetLockerMotion },
	{ 0x0078D685, NewScenarioClaymore },
	{ 0x00797EDF, PL_COM_BladeSlashCheck },
	{ 0x007A3DBF, NewPutWorldTexFadeCameraForDoor },
	{ 0x007A8EC1, NewPutObject },
	{ 0x007A94A9, NewMallocCameraCharaWork },
	{ 0x007AA13A, NewStreamSet },
	{ 0x007AD92B, NewCartridgeUspALL },
	{ 0x007B35E0, CodecMemCallSet },
	{ 0x007C7E31, ComNTankerDogtags },
	{ 0x007DDA2E, GCL_DG_SetTmpLight2 },
	{ 0x007E3320, NewOptionScr },
	{ 0x007F3C88, NewViewCheckCommand },
	{ 0x007F7ACF, NewGclGetPrezLife },
	{ 0x0080B977, NewSystemCallback },
	{ 0x0080FB82, VR_GetVrTime },
	{ 0x008155F1, NewClearCodeScr },
	{ 0x0081584F, NewTdgCameraStatusOff },
	{ 0x0081767C, NewTitleKonami },
	{ 0x00818B2C, NewVRClear_Scn },
	{ 0x0081E7F6, CPE_CamPoseInit },
	{ 0x0082A05E, ComStoreLoadedVariable },
	{ 0x0082BDC0, NewGclVecLen },
	{ 0x0082CB77, SK_RootPositionChange },
	{ 0x0083C0C3, NewSniperVamp },
	{ 0x0083D857, NewDynamicLightPosManager },
	{ 0x0083DD4D, NewHarKasacka },
	{ 0x008422F6, NewSetEludeParam },
	{ 0x00843C09, VR_NoContinue },
	{ 0x00848E57, NewSetNikitaTimer },
	{ 0x0084B931, NewBetweenCamera },
	{ 0x0085E8F4, NewLockerDaemon },
	{ 0x0086301D, NewVRChild },
	{ 0x0086D1CF, ENEMEM_GclCopy },
	{ 0x0086D382, NewDemoSnakeArm },
	{ 0x00873375, NewWaterDustManager },
	{ 0x0087A1C0, NewSelect },
	{ 0x008826B9, NewDemoDoll },
	{ 0x008862F6, NewRotateY_Object },
	{ 0x0088CBF1, GM_SeSetVolPanGcl },
	{ 0x0088ED39, VR_SetTargetNGScore },
	{ 0x0088F9B0, ENEMEM_GclAllReset },
	{ 0x008975E6, NewFlowPaper },
	{ 0x0089778A, NewDust },
	{ 0x0089AF5E, NewThund_Flash },
	{ 0x0089C365, NewLockerCP },
	{ 0x0089FEC2, NewVRWallScarBase },
	{ 0x008A43F9, NewVRSparkBase },
	{ 0x008AA572, NewLocker },
	{ 0x008AC901, NewPutTexNoTarget },
	{ 0x008B12D1, NewSetCameraProc },
	{ 0x008B19F0, NewPlayerSetInvincible },
	{ 0x008B6086, NewPadCheck },
	{ 0x008B976D, NewLockerStatus },
	{ 0x008C4A5B, NewScreenShotControl },
	{ 0x008C58F7, NewExtinguisher },
	{ 0x008DFE63, PL_COM_OnCorpseStand },
	{ 0x008E0676, GM_GetDirForGCL },
	{ 0x008E298D, NewPadCancel },
	{ 0x008F27AC, NewDEMOSetKaitaiC4 },
	{ 0x008FF6AD, NewPluginAutoJump },
	{ 0x00901FD7, VR_SetBlastRange },
	{ 0x0090342E, C4MAN_Init },
	{ 0x009049ED, NewSetPlayerLockerMotion },
	{ 0x0090B7EC, NewCheckWaterLevel },
	{ 0x00912BFE, NewFortCeiling },
	{ 0x00915101, NewCameraPoseEnemyManager },
	{ 0x009173E7, SK_ScnBreakPointSet },
	{ 0x009208DC, PL_COM_VisibleO2Gage },
	{ 0x00923E25, VR_TimerPause },
	{ 0x00924573, COM_GetEnemyStSt },
	{ 0x00926341, NewPluginGraspEE },
	{ 0x00926ACE, VR_SetTargetSpeedRate },
	{ 0x0092A625, ComSetProductCode },
	{ 0x0092EB54, NewGetConfig },
	{ 0x009307AB, NewSpotDrawObjectSet },
	{ 0x0093EEEA, VR_PhotoClear },
	{ 0x0094193B, PL_COM_GetElevatorStatus },
	{ 0x00944E11, NewSetMenuStatus },
	{ 0x0094C147, NewCorpseWalk },
	{ 0x00951C01, AccessMobile },
	{ 0x009530AE, NewNodeFrame },
	{ 0x00961325, NewPutSpeechObject },
	{ 0x0096E361, NewAttachment_Server },
	{ 0x00971FDA, NewWcFlush_Scn },
	{ 0x009786F7, NewRadarMovie },
	{ 0x0097889D, EE_COM_GetO2 },
	{ 0x0098137C, ComGetLoaddataTime },
	{ 0x009837C4, PL_GetDGCamSubject },
	{ 0x00987E81, NewLocalWind },
	{ 0x009890D0, COM_GetEnemyNoticeMode },
	{ 0x0098BD5B, NewSplushMan },
	{ 0x0099615D, GM_COM_SetBossSurvivalStatus },
	{ 0x009A75E7, NewMobileDaemon },
	{ 0x009A97B2, GM_OnlyCheckControl },
	{ 0x009AFF54, NewItemBox },
	{ 0x009B3B8B, NewEmma },
	{ 0x009B3FD5, NewCypher },
	{ 0x009B65F0, NewTimer },
	{ 0x009BC66F, NewRopeModel2 },
	{ 0x009BC670, NewRopeModel3 },
	{ 0x009C403E, PIC_TankerPicSaveScn },
	{ 0x009CF455, PL_COM_ChangePrevItemScn },
	{ 0x009DCD6B, NewAmbientSet },
	{ 0x009DD632, GM_SetNoiseGcl },
	{ 0x009E1DF3, PL_COM_BulletSplashDaemon },
	{ 0x009E3C4F, NewVRFloorPanel_Scn },
	{ 0x009E63C2, COM_GetCaptureModelName },
	{ 0x009E8AEB, NewConfirmSaveScr },
	{ 0x009F11BC, NewNPCSnake },
	{ 0x00A0A179, NewBubbleLineScn },
	{ 0x00A151B1, NewBeltObjectManager },
	{ 0x00A1A665, NewGetOrgaLife },
	{ 0x00A3D28A, NewBossrushScr },
	{ 0x00A3E408, ENEMEM_GclSet },
	{ 0x00A53AA7, NewFogSet },
	{ 0x00A58F8A, NewVRScreen_Scn },
	{ 0x00A58FA5, NewIrs2 },
	{ 0x00A60221, PL_COM_GolCapOn },
	{ 0x00A63D3B, NewZman },
	{ 0x00A6A1EE, PL_COM_GolCapOff },
	{ 0x00A6D9CF, PL_COM_SightOnOff },
	{ 0x00A710DB, NewW32Defender },
	{ 0x00A7195F, GetNPCSnakeLife },
	{ 0x00A741FD, NewPluginLadder },
	{ 0x00A7CB42, NewO2Gage },
	{ 0x00A8560D, NewManhatLight },
	{ 0x00A85CEB, NewComPUT_ELV_EraseBomb },
	{ 0x00A895C4, NewPutVanimeObject },
	{ 0x00A97A70, NewBreakComputerW12b },
	{ 0x00A97EB2, NewBreakComputerW24d },
	{ 0x00A9ED03, NewSlowParamMan },
	{ 0x00AB381C, NewNodeDaemon },
	{ 0x00AB5A2A, NewResidentResourceSet },
	{ 0x00ABFD5F, NewWatertightDoorManager },
	{ 0x00ACE3FF, NewPDRaySet },
	{ 0x00AD8864, NewFortWallLight },
	{ 0x00AE0F36, NewPDRayServerSet },
	{ 0x00AF0A7A, NewSpinModel },
	{ 0x00AF2208, NewElectricFloor },
	{ 0x00AF4CF6, NewKamomeManager },
	{ 0x00AFA5E7, NewBlinkFloor },
	{ 0x00AFDEE4, NewTimerEnd },
	{ 0x00B030E4, NewEnbControl },
	{ 0x00B10086, VR_SetComboLimit },
	{ 0x00B1ABD8, NewWatertightDoorStatus },
	{ 0x00B1E8D4, COM_SetClearingData },
	{ 0x00B2A6B7, NewEvmHairModel },
	{ 0x00B2FD6C, NewShortSparkLine },
	{ 0x00B35703, NewAutoJumpSetting },
	{ 0x00B3AA52, NewBeltConveyerBelt },
	{ 0x00B3D54E, GM_SetZoneFlagForScn },
	{ 0x00B3E388, NewHoldEnemy },
	{ 0x00B431C3, NewPutSearchLightObject },
	{ 0x00B44BBB, NewSetExchangeSeCode },
	{ 0x00B4E108, NewVRSky_Scn },
	{ 0x00B4E35C, NewVRWall },
	{ 0x00B53158, NewBloodWater_Scn },
	{ 0x00B63A33, NewBlur },
	{ 0x00B63E47, VR_SetTrgScale },
	{ 0x00B66AE4, NewFlyingWarmScn },
	{ 0x00B6E522, PL_COM_IntrudeSubjectCameraPositionMove },
	{ 0x00B74FD7, SIG_DogTagZeroClear },
	{ 0x00B75003, NewUbSvCamera },
	{ 0x00B7B5A7, ComLocalResourceSet },
	{ 0x00B7F8E7, Msn_SetDefaultData },
	{ 0x00B80DE5, NewRainParts },
	{ 0x00B89202, NewFadeObj },
	{ 0x00B8B5F7, PL_COM_BladeNoCheckSegment },
	{ 0x00B8B94D, NewHako_purapura },
	{ 0x00B93D5E, NewTengMonitorControl },
	{ 0x00B97D41, ENEMEM_GclReset },
	{ 0x00BA92F5, GM_COM_RegainEquip },
	{ 0x00BB6852, NewEESwim },
	{ 0x00BBAD24, NewNearFocusEffectSet },
	{ 0x00BCB4A2, GM_GameOverChoice },
	{ 0x00BD28C4, GM_COM_PauseOff },
	{ 0x00BD400B, NewShadowControl },
	{ 0x00BD4C61, NewWigTexReplaceActorScn },
	{ 0x00BD673F, NewRainSlow },
	{ 0x00BD9CC1, NewW32Commander },
	{ 0x00BDD6A9, NewTargetTrap },
	{ 0x00BE0863, NewSplashFloor_Scn },
	{ 0x00BED0FF, NewWaterDropManager },
	{ 0x00BF97ED, NewPluginBeltConveyer },
	{ 0x00BFB0A1, NewVampShdwTrgt },
	{ 0x00C0895A, NewCmdDeadKamomeInvisible },
	{ 0x00C09E6C, NewAllSlater },
	{ 0x00C0E06D, NewBrooklyn_R2 },
	{ 0x00C13513, NewVRBook },
	{ 0x00C1451B, NewCylinderImage },
	{ 0x00C1BC23, NewSubjectDemoPlayer },
	{ 0x00C1EB06, HLD_GetHoldPosition },
	{ 0x00C1FF0F, NewSigSinkTanker },
	{ 0x00C210E3, NewTs_SubWindow_S },
	{ 0x00C27387, KR_GetHostageTarget },
	{ 0x00C2AD32, NewPDRayStageEnvironmentSet },
	{ 0x00C2EB8D, NewEMA_CommandForceMove },
	{ 0x00C3189A, NewKillPlayer },
	{ 0x00C34B14, NewPutMonitor },
	{ 0x00C3515F, NewWaterMine },
	{ 0x00C3C46C, NewFakeGameOverSet },
	{ 0x00C41872, NewUnsetTimerStatus2 },
	{ 0x00C45437, NewPutBottleObject },
	{ 0x00C547A7, NewFloorLightMan },
	{ 0x00C5994D, VRCLR_GetResult },
	{ 0x00C5D24C, NewPassageWaterSet },
	{ 0x00C622A4, NewDemoSlater },
	{ 0x00C64ECB, VRMap3D_ResetBomb },
	{ 0x00C6F28E, NewCmdFatInvisible },
	{ 0x00C7280A, NewDefCommander },
	{ 0x00C74F97, NewArraySet },
	{ 0x00C76D6E, PL_COM_DestroyBox },
	{ 0x00C7853F, VR_SetTargetHideAppearCycle },
	{ 0x00C7EE15, NewCmdKamomeVisible },
	{ 0x00C88AFA, NewSoundManager },
	{ 0x00C8DBCC, NewPuddle },
	{ 0x00C9DD51, NewSkyUtil },
	{ 0x00CB2C3E, NewGasInWater },
	{ 0x00CB6BED, GM_SetVolumeParam },
	{ 0x00CB6EC4, NewGetShipwormStatus },
	{ 0x00CB7DE8, NewFlashLight },
	{ 0x00CBA568, NewPutBigGlassObject },
	{ 0x00CBB124, NewCeiling },
	{ 0x00CBD98B, NewBosKasacka },
	{ 0x00CC87A2, NewLayout2D_Player },
	{ 0x00CC9A07, NewSigAttacker },
	{ 0x00CD2045, ComGetNowTime },
	{ 0x00CDB878, NewPrezNikita },
	{ 0x00CDC7BD, NewWeaponSplash },
	{ 0x00CEC17C, OK_ResetGetBoundary },
	{ 0x00CED375, NewVRGoal },
	{ 0x00CF777F, NewPutWorldTexFadeWorld },
	{ 0x00D10066, NewEntrySparkOnlySE },
	{ 0x00D110E2, NewFortHangLight },
	{ 0x00D1279A, NewPutTreeObject },
	{ 0x00D1699D, NewGetPlayerMapName },
	{ 0x00D16C76, NewScrWater },
	{ 0x00D20FDE, NewHugeSeaSurfaceSet },
	{ 0x00D219B8, SetGameStatusVROnly },
	{ 0x00D2BD87, NewWindManager },
	{ 0x00D2C9AA, NewBreathPointManager },
	{ 0x00D30863, NewStreamStop },
	{ 0x00D4090D, NewLoDControl },
	{ 0x00D44DC7, ComStoreOptions },
	{ 0x00D481ED, EE_COM_SetLife },
	{ 0x00D51601, NewCheckEneTrap },
	{ 0x00D5D6EF, NewExtraUmiDraw },
	{ 0x00D5FF97, NewCameraSwingMode },
	{ 0x00D612A2, NewEmmaSlater },
	{ 0x00D6ED95, NewBrooklyn3D },
	{ 0x00D72C78, NewBuildingWindow },
	{ 0x00D75A89, NewGetTimerStatus },
	{ 0x00D8CFF4, ComMaxTankerDogtags },
	{ 0x00D90540, NewPutElevator },
	{ 0x00D978AC, COM_GetEnemyStatus2 },
	{ 0x00D99A89, NewSetTimerStatus },
	{ 0x00D99FB0, NewW25Defender },
	{ 0x00D9C789, GM_GetSngCode },
	{ 0x00DA97FB, NewTrapSwitchCommand },
	{ 0x00DABA9E, NewPutTexturePlant },
	{ 0x00DB03A6, AccessPhotoTerm },
	{ 0x00DB65AC, NewAddBreathPoint },
	{ 0x00DBA0D1, NewHar_CommandGetPosition },
	{ 0x00DBD1FA, NewFortElevatorButton },
	{ 0x00DC2F1B, NewIPUDisplayPanelSet },
	{ 0x00DC323F, NewDustOnCamera },
	{ 0x00DC80BF, New_Figure_Raven },
	{ 0x00DC83C5, GM_LoadPack },
	{ 0x00DCB281, ComSetReferenceData },
	{ 0x00DDD2A3, Command_GetFallFloorStatus },
	{ 0x00DDF457, NewEntryVRWallScar },
	{ 0x00DDF5CA, NewCommander },
	{ 0x00DE0400, NewAttachment },
	{ 0x00DE0401, NewAttachment2 },
	{ 0x00DE0402, NewAttachment3 },
	{ 0x00DE07C0, NewFontInit },
	{ 0x00DE4430, PL_COM_SetNoUse },
	{ 0x00DEDDE1, NewBubbleManyColorSet },
	{ 0x00DF5435, NewPutPlateObject },
	{ 0x00E00FD1, NewDivingGoggles },
	{ 0x00E0344C, COM_GetCaptureName },
	{ 0x00E0568B, NewVRFloorMarker_Scn },
	{ 0x00E0DBBA, New2DSprite },
	{ 0x00E0DCBA, NewGeneralSprite_Scn },
	{ 0x00E1C26C, NewGetOrgaSrcDst },
	{ 0x00E22388, NewFadeInOut_Scn },
	{ 0x00E2488B, NewCom_FRT_CEL_Control },
	{ 0x00E29ADB, NewStartScenarioDemo },
	{ 0x00E2A088, NewDebugPause },
	{ 0x00E2ED79, NewDoorPannelSpark },
	{ 0x00E32FC7, NewPutSwingObject },
	{ 0x00E3429D, NewOilSpread },
	{ 0x00E3549B, NewNode },
	{ 0x00E39C0D, New2DSprite_Pause },
	{ 0x00E3C577, SetSnakeTalesStatus },
	{ 0x00E3EB1A, COM_GetEnemyPos },
	{ 0x00E3EEA2, NewGetFortuneBulletPos },
	{ 0x00E41776, NewSpotRain },
	{ 0x00E48F2F, NewPutSeSound },
	{ 0x00E4C507, GM_GetGameStatusForScn },
	{ 0x00E4D016, NewProjectorLight },
	{ 0x00E52073, NewDustArea },
	{ 0x00E5FF03, PL_COM_SubjectArmChain },
	{ 0x00E6B658, COM_GetEnemyStatus },
	{ 0x00E73AC4, NewDeleteCheckCharaOfCamera },
	{ 0x00E74F46, GM_COM_PauseOn },
	{ 0x00E76D74, NewGclVariableMove },
	{ 0x00E79927, NewEndScenarioDemo },
	{ 0x00E9021D, NewTRAP_C4 },
	{ 0x00E96D82, NewCodecDaemon },
	{ 0x00EA5215, NewFallFloor },
	{ 0x00EAD648, NewForceActCancel },
	{ 0x00EB7EDA, NewGllPhoto },
	{ 0x00EBEC24, NewWatertightDoor },
	{ 0x00EC084B, NewStageFireScn },
	{ 0x00EC40C7, NewSetCheckCharaOfCamera },
	{ 0x00ECC9DE, NewFlour },
	{ 0x00ED1E0B, NewVrObject },
	{ 0x00ED4678, NewSteamOnCameraScenario },
	{ 0x00EE8700, NewKac_CommandGetPosition },
	{ 0x00EE8B90, GM_SeSetModeGcl },
	{ 0x00EEE657, NewEmmaLocker },
	{ 0x00EFD440, NewGetFortuneBombPos },
	{ 0x00EFFDAC, VR_StagePause },
	{ 0x00F005A3, COM_ClearingCancel },
	{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	{ 0x00F15E47, NewMallocCameraSquareWork },
	{ 0x00F26728, NewSlowDown },
	{ 0x00F2DFCD, NewLoadRestart },
	{ 0x00F2EEDC, GM_SdSetGcl },
	{ 0x00F37560, NewBloodSpread },
	{ 0x00F4BBF5, NewTimer2 },
	{ 0x00F52ED4, NewParrot2 },
	{ 0x00F5BF46, NewSigRouteModel2 },
	{ 0x00F5C132, NewGetHoloStatus },
	{ 0x00F62833, NewScrTrans },
	{ 0x00F63B18, NewSvPlantCamera },
	{ 0x00F6ADA8, NewUSPLightOn },
	{ 0x00F706CD, NewOffMan },
	{ 0x00F74020, NewPhotoTerminal },
	{ 0x00F7E492, NewSetupLadder },
	{ 0x00F7F777, NewWatcher },
	{ 0x00F8F4E8, NewCom_AddRenameList },
	{ 0x00F91A08, NewDynamicFloor },
	{ 0x00F91B9F, NewDynamicSegment },
	{ 0x00F92B8E, NewInvisibleChara },
	{ 0x00F9711E, NewGWB_LinesModel },
	{ 0x00F9C62D, PL_SprayCheckWide },
	{ 0x00FA4E80, NewScrHexFade },
	{ 0x00FAD3AE, NewEMA_CommandGetMiceVolPan },
	{ 0x00FAEDF1, NewSolidus },
	{ 0x00FB029A, NewSKLightEff },
	{ 0x00FBEB7F, GM_COM_IsGameOver },
	{ 0x00FC14A5, NewTitleScrMan },
	{ 0x00FC6185, NewGclInsideChk4P },
	{ 0x00FCA1A4, PL_COM_BladeEffectOn },
	{ 0x00FDDA41, NewTargetProc },
	{ 0x00FE39E8, FRT_AIM_InitAimSpot },
	{ 0x00FE5C5B, NewSigTengA },
	{ 0x00FE5C5C, NewSigTengB },
	{ 0x00FE5F3D, NewWebSite },
	{ 0x00FE6730, NewSoundTest },
	{ 0x00FE6F50, NewModelPatAnimation },
	{ 0x00FEA0C9, PL_COM_SetJumpGravity },
	{ 0x00FEF2D9, NewSetPlayerBeltConveyerProc },
	{ 0x00FF5B55, ComMaxPlantDogtags },
	{ 0x00FFBECE, NewMpegPssMovieStr },
	{ 0x00FFC196, NewStageOutline_Scn },
	{ 0x00FFD75E, COM_GetEnemyAction },
	{ 0x00FFE7A7, NewKac_CommandGetKasLife },
	{ 0x00FFED03, NewExecProcContinual },
	{ 0x01000000, NewFogSet_Demo_0000Launch },
	{ 0x01000001, NewFarFocusEffect_0001Launch },
	{ 0x01000002, NewNearFocusEffect_0002Launch },
	{ 0x01000003, NewFadeInOut_Demo_0003Launch },
	{ 0x01000004, NewRainCamera_Demo_0004Launch },
	{ 0x01000005, NewFlush_0005Launch },
	{ 0x01000006, DM_ChangeAmbient_0006Launch },
	{ 0x01000007, DM_ChangeParallel_0007Launch },
	{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	{ 0x01000009, NewContrast_Demo_0009Launch },
	{ 0x0100000A, NewContrastForce_Demo_000aLaunch },
	{ 0x0100000B, NewSK_ScopeSight_000bLaunch },
	{ 0x0100000C, NewScrCrack_000cLaunch },
	{ 0x0100000D, New2DSprite_Prog_000dLaunch },
	{ 0x0100000E, NewCigarette_Demo_000eLaunch },
	{ 0x0100000F, NewLensFlare_Demo_000fLaunch },
	{ 0x01000011, NewBlur_Demo_0011Launch },
	{ 0x01000012, NewCrossFadeEffect_0012Launch },
	{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	{ 0x01000014, NewScrWater_Demo_0014Launch },
	{ 0x01000015, NewScrConcentrateBlur_0015Launch },
	{ 0x01000016, NewRaySight_0016Launch },
	{ 0x01000017, NewVtrSight_0017Launch },
	{ 0x01000018, OK_FogStatusSet_0018Launch },
	{ 0x01000019, NewRainFogPersFast_0019Launch },
	{ 0x0100001A, NewDEMODigitalCamera_001aLaunch },
	{ 0x0100001C, NewPsg1Sight_001cLaunch },
	{ 0x0100001D, NewDEMO_Equip_001dLaunch },
	{ 0x0100001F, NewDemoSun_001fLaunch },
	{ 0x01000020, NewScnAiRaySight_0020Launch },
	{ 0x01000021, NewExplosionControl_Demo_0021Launch },
	{ 0x01000022, NewNyou_0022Launch },
	{ 0x01000023, NewFortuneTear_0023Launch },
	{ 0x01000024, NewScrGoggles_demo_0024Launch },
	{ 0x01000025, NewScrDrop_demo_0025Launch },
	{ 0x01000026, NewScrWaterFilm_0026Launch },
	{ 0x01000031, NewRainFogPersFast2_0031Launch },
	{ 0x01000033, NewRainFogPersDemo_0033Launch },
	{ 0x01000100, NewSplashMotion_Demo_0100Launch },
	{ 0x01000101, NewSplashRipple_Demo_0101Launch },
	{ 0x01000103, NewFootSplash_0103Launch },
	{ 0x01000104, NewBodySplash3_0104Launch },
	{ 0x01000105, NewDiveSplash_Parent_0105Launch },
	{ 0x01000106, NewSphereSplush_0106Launch },
	{ 0x01000107, NewWaveSplash_Demo_0107Launch },
	{ 0x01000108, NewWallTidal_0108Launch },
	{ 0x01000109, NewWaterWindSplush_DEMO_0109Launch },
	{ 0x01000201, NewRipBubbleMan_DEMO_0201Launch },
	{ 0x01000202, NewRaidenMaskBubbleDemo_0202Launch },
	{ 0x010007D1, NewTraffic_Demo_07d1Launch },
	{ 0x01001000, NewDropShadow_1000Launch },
	{ 0x01001001, NewShadow_1001Launch },
	{ 0x01001002, NewEneEquip_1002Launch },
	{ 0x01001005, NewRaincoat_called_1005Launch },
	{ 0x01001006, NewPutAttachments_1006Launch },
	{ 0x01001007, NewPutAttachments_1007Launch },
	{ 0x01001008, NewGbsHandDemo_1008Launch },
	{ 0x01001009, NewGbsFaceDemo_1009Launch },
	{ 0x0100100A, NewBodyShadowVolume_Demo_100aLaunch },
	{ 0x0100100C, NewPutAttachments_100cLaunch },
	{ 0x0100100D, NewEyeControl_100dLaunch },
	{ 0x0100100F, NewDemoArkms_100fLaunch },
	{ 0x01001010, NewArmsEffectControl_1010Launch },
	{ 0x01001011, NewDemoArkms_NYPD_1011Launch },
	{ 0x01001012, NewCreateEquipment_1012Launch },
	{ 0x01001013, NewEyeControl2_1013Launch },
	{ 0x01001014, NewFortEquip_1014Launch },
	{ 0x01001015, NewEmmaEquip_1015Launch },
	{ 0x01001016, NewEyeAnimSEALS_Demo_1016Launch },
	{ 0x01001017, NewFortSling_1017Launch },
	{ 0x01001018, NewFortBulletDemo_1018Launch },
	{ 0x01001019, NewLinerGunInitEffect_1019Launch },
	{ 0x0100101A, NewOrgFaceEft_101aLaunch },
	{ 0x0100101B, NewDemoArmControl_101bLaunch },
	{ 0x0100101C, NewDependArms_101cLaunch },
	{ 0x0100101D, NewDependArms_Aks_101dLaunch },
	{ 0x0100101E, NewBreathDemo_101eLaunch },
	{ 0x0100101F, NewPutAttachments_101fLaunch },
	{ 0x01001020, PL_RaidenEquipmentManager_1020Launch },
	{ 0x01001021, NewDependArms_USP_SP_1021Launch },
	{ 0x01001022, NewScnEvm_SkirtA_1022Launch },
	{ 0x01001023, NewScnEvm_SkirtB_1023Launch },
	{ 0x01001024, NewFortSling2_1024Launch },
	{ 0x01001027, NewWine_1027Launch },
	{ 0x01001028, NewDemoArkms_CitMale_1028Launch },
	{ 0x01001029, NewDemoArkms_CitFemale_1029Launch },
	{ 0x0100102A, NewDependArms_Kill_102aLaunch },
	{ 0x01002000, NewThunder_Demo_2000Launch },
	{ 0x01002002, NewLocalWind_Demo_2002Launch },
	{ 0x01002003, NewSlowParamMan_prog_2003Launch },
	{ 0x01002004, NewLocalWind2_Demo_2004Launch },
	{ 0x01003000, NewRopeModel3_called_3000Launch },
	{ 0x01003004, NewRopeModel3_called_3004Launch },
	{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	{ 0x01003010, NewWavingClothModelW_called_3010Launch },
	{ 0x01003011, NewEvmMMOrga_called_3011Launch },
	{ 0x01003012, NewCergeiEri_demo_3012Launch },
	{ 0x01003013, NewSolMant_called_3013Launch },
	{ 0x01003014, NewRopeModel3_called_3014Launch },
	{ 0x01003015, NewHandcuff_called_3015Launch },
	{ 0x01003017, NewRopeModel3_called_3017Launch },
	{ 0x01003019, NewRopeModel3_called_3019Launch },
	{ 0x01004000, NewPutSTanimeObjectCall_4000Launch },
	{ 0x01004001, DM_SendEffectMessage_4001Launch },
	{ 0x01004002, NewInterPoly_Demo_4002Launch },
	{ 0x01004004, NewVADemo_4004Launch },
	{ 0x01006000, NewSpark_6000Launch },
	{ 0x01006002, NewPlasmaPoly_Demo_6002Launch },
	{ 0x01006005, NewDemoBulletCall_6005Launch },
	{ 0x01006006, NewOpticalCamouflageBreakDemo2_6006Launch },
	{ 0x01006007, NewDebris_Cm_Demo_6007Launch },
	{ 0x01006008, NewDebris_Tex_Demo_6008Launch },
	{ 0x01006009, NewBombEffect_6009Launch },
	{ 0x0100600A, NewFlyingSmoke_600aLaunch },
	{ 0x0100600B, NewBombGasEffect_600bLaunch },
	{ 0x0100600E, NewTs_Spark_600eLaunch },
	{ 0x01006010, NewMesgBomb2_6010Launch },
	{ 0x01006011, NewGeneralSprite2_6011Launch },
	{ 0x01006012, NewSonicWave_6012Launch },
	{ 0x01006013, NewMesgBomb3_6013Launch },
	{ 0x01006014, NewFortBarrierDemo_6014Launch },
	{ 0x01006015, NewDebris_Cm_Demo2_6015Launch },
	{ 0x01006016, NewDebris_Tex_Demo2_6016Launch },
	{ 0x01006017, NewDemoHarrierMissileFire2_6017Launch },
	{ 0x01006018, NewDemoStageFire_6018Launch },
	{ 0x01006019, NewDemoSolidusDashFire_6019Launch },
	{ 0x0100601A, NewParrotFeather_601aLaunch },
	{ 0x0100601B, NewDemoSolidusDashFire2_601bLaunch },
	{ 0x0100601C, NewMAOParticle_601cLaunch },
	{ 0x0100601D, NewKamomeFeather_601dLaunch },
	{ 0x0100601F, NewHexagonalPattern_601fLaunch },
	{ 0x01006020, NewDemoBodyPlasma_6020Launch },
	{ 0x01006021, NewDemoElectricFloor_6021Launch },
	{ 0x01006022, NewDemoSolidusMissileFire_6022Launch },
	{ 0x01006023, NewDemoSolidusMissileSmoke_6023Launch },
	{ 0x01006024, NewDemoBladeSpark_6024Launch },
	{ 0x01006025, NewDemoSolidusSnakearmFlow_6025Launch },
	{ 0x01006026, NewDemoSolidusBladeLight_6026Launch },
	{ 0x01006027, NewDemoSolidusEnergyPrim_6027Launch },
	{ 0x01006028, NewDemoBladeFlow_6028Launch },
	{ 0x01006029, NewPlasmaEvade_6029Launch },
	{ 0x0100602A, NewDemoPlasmaLineColor_602aLaunch },
	{ 0x0100602C, NewDemoCypherPlasma_602cLaunch },
	{ 0x01006030, NewDebris_Cm_DemoBlood_6030Launch },
	{ 0x01006031, NewDebris_Tex_DemoBlood_6031Launch },
	{ 0x01007001, NewBlood_Demo_7001Launch },
	{ 0x01007002, NewBlood2_Demo_7002Launch },
	{ 0x01007003, AN_Blood_Mist_7003Launch },
	{ 0x0100700B, NewBloodWeep_700bLaunch },
	{ 0x0100700C, NewPeterBlood_700cLaunch },
	{ 0x0100700D, NewDemoSplashBlood_700dLaunch },
	{ 0x0100700E, NewDemoDummyPointBlood_700eLaunch },
	{ 0x0100700F, NewBlood_Demo2_700fLaunch },
	{ 0x01007100, NewSmokeBlurEffect_7100Launch },
	{ 0x01007101, NewCommonSmoke_7101Launch },
	{ 0x01007102, NewDynamicFlow_Demo_7102Launch },
	{ 0x01007103, NewLineSmoke_7103Launch },
	{ 0x01007104, NewDemoHarrierMissileSmokeLine2_7104Launch },
	{ 0x01007105, NewDemoHarEffect_7105Launch },
	{ 0x01007106, NewDemoHarrierDamageSmoke_7106Launch },
	{ 0x01007107, NewDemoRisingSmoke_7107Launch },
	{ 0x01007108, NewSmokeReactionClient_7108Launch },
	{ 0x01007109, NewDemoRisingSmokeFix_7109Launch },
	{ 0x0100710A, NewDemoRisingSmokeDummy_710aLaunch },
	{ 0x0100710C, NewFatmanDemoScratchSmoke_710cLaunch },
	{ 0x0100710D, NewSmokeStripControl_710dLaunch },
	{ 0x0100710E, NewSmokeMitsukoshiMan_710eLaunch },
	{ 0x0100710F, NewPeterBlood_Gas_710fLaunch },
	{ 0x01008000, DM_SendEffectMessage_8000Launch },
	{ 0x01009000, NewPadVibration2_9000Launch },
	{ 0x0100A002, NewRunTelop_Trial_a002Launch },
	{ 0x0100A003, NewSplushSurfaceMan_a003Launch },
	{ 0x0100A004, NewRippleMan_a004Launch },
	{ 0x0100B000, NewWaterLevelControl_Demo_b000Launch },
	{ 0x0100B001, NewDropBodySplush_b001Launch },
	{ 0x0100B002, OK_PutSplushSurface_b002Launch },
	{ 0x0100B003, NewSplushTidal_b003Launch },
	{ 0x0100B004, NewSeaSurfaceReactionObject_b004Launch },
	{ 0x0100B005, NewAutoSplush_b005Launch },
	{ 0x0100B006, NewAutoSplush_EftCtrl_b006Launch },
	{ 0x0100B007, NewBloodWater_demo_b007Launch },
	{ 0x0100B040, TAKABE_RiseWaveEx_b040Launch },
	{ 0x0100C100, NewEvmLateControl_c100Launch },
	{ 0x0100D000, NewRayConsol_d000Launch },
	{ 0x0100D001, NewRayEye_d001Launch },
	{ 0x0100D002, NewWaterPollute_demo_d002Launch },
	{ 0x0100D003, NewRayMissileShower_d003Launch },
	{ 0x0100D004, NewRayMonoEye_Demo_d004Launch },
	{ 0x0100D005, NewRayMissileForDemo_d005Launch },
	{ 0x0100D006, NewRayFootSplash_d006Launch },
	{ 0x0100D007, NewRayOozeBloodDemo_d007Launch },
	{ 0x0100D008, PDRAY_OozeBloodAddDemo_d008Launch },
	{ 0x0100D009, PDRAY_CLOUD_DeleteDensity_d009Launch },
	{ 0x0100D00A, NewDemoRayFallBlood_d00aLaunch },
	{ 0x0100D00B, NewFogWave_d00bLaunch },
	{ 0x010FFF00, DM_DebugPrint_fff00Launch },
	{ 0x010FFF01, DM_ExecProc_fff01Launch },
	{ 0x010FFF02, DM_SetClipParam_fff02Launch },
	{ 0x010FFF03, NewDemoFrameCountCall_fff03Launch },
	{ 0x010FFF04, NewControlBoy_fff04Launch },
	{ 0x010FFF05, NewDemoEffectInitialize_fff05Launch },
	{ 0x010FFFF0, DM_ControlChange_ffff0Launch },
/* gcl chara end */
	{ 0, NULL }
};
