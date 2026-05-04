/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewCreateEquipmentScn;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewShadowDrawObjectSet;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewUserDogtagSet;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewFogSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA NewFadeInOut_Demo_0003Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffect_0012Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA NewRainFogPersFast_0019Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewDemoArkms_NYPD_1011Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA PL_RaidenEquipmentManager_1020Launch;
extern NEWCHARA NewDemoArkms_CitMale_1028Launch;
extern NEWCHARA NewDemoArkms_CitFemale_1029Launch;
extern NEWCHARA NewSlowParamMan_prog_2003Launch;
extern NEWCHARA NewLocalWind2_Demo_2004Launch;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA NewRopeModel3_called_3017Launch;
extern NEWCHARA NewRopeModel3_called_3019Launch;
extern NEWCHARA NewDebris_Cm_Demo_6007Launch;
extern NEWCHARA NewMesgBomb3_6013Launch;
extern NEWCHARA NewParrotFeather_601aLaunch;
extern NEWCHARA NewDemoBodyPlasma_6020Launch;
extern NEWCHARA NewDemoBladeSpark_6024Launch;
extern NEWCHARA NewDemoSolidusBladeLight_6026Launch;
extern NEWCHARA NewDemoSolidusEnergyPrim_6027Launch;
extern NEWCHARA NewDemoBladeFlow_6028Launch;
extern NEWCHARA NewDemoPlasmaLineColor_602aLaunch;
extern NEWCHARA NewDemoSplashBlood_700dLaunch;
extern NEWCHARA NewSmokeMitsukoshiMan_710eLaunch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA NewEvmLateControl_c100Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 105 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* chara 装備品セット */
		{ 0x000381AC, NewCreateEquipmentScn },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara 影投影モデル */
		{ 0x0005C815, NewShadowDrawObjectSet },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara ユーザードックタグ */
		{ 0x001F2A23, NewUserDogtagSet },
	/* chara 髭剃りスネークテクスチャ入れ替え */
		{ 0x002DF578, NewShavedSnakeTexReplaceActorScn },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* chara demo */
		{ 0x0033A20F, NewPolygonDemoStart },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 太陽 */
		{ 0x00641A7B, NewPlantSunMain },
	/* command ハリアー戦で誰がプレイヤーを攻撃したか */
		{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara 影管理 */
		{ 0x00BD400B, NewShadowControl },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara 汎用空 */
		{ 0x00C9DD51, NewSkyUtil },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command デモローポリ劇場 */
		{ 0x00F8F4E8, NewCom_AddRenameList },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* EXTERN NewFogSet_Demo */
		{ 0x01000000, NewFogSet_Demo_0000Launch },
	/* EXTERN NewFarFocusEffect */
		{ 0x01000001, NewFarFocusEffect_0001Launch },
	/* EXTERN NewNearFocusEffect */
		{ 0x01000002, NewNearFocusEffect_0002Launch },
	/* EXTERN NewFadeInOut_Demo */
		{ 0x01000003, NewFadeInOut_Demo_0003Launch },
	/* EXTERN DM_ChangeAmbient */
		{ 0x01000006, DM_ChangeAmbient_0006Launch },
	/* EXTERN DM_ChangeParallel */
		{ 0x01000007, DM_ChangeParallel_0007Launch },
	/* EXTERN NewFadeInOutForce_Demo */
		{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewCrossFadeEffect */
		{ 0x01000012, NewCrossFadeEffect_0012Launch },
	/* EXTERN NewCrossFadeEffectCustom */
		{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	/* EXTERN NewRainFogPersFast */
		{ 0x01000019, NewRainFogPersFast_0019Launch },
	/* EXTERN NewPutAttachments */
		{ 0x01001006, NewPutAttachments_1006Launch },
	/* EXTERN NewDemoArkms_NYPD */
		{ 0x01001011, NewDemoArkms_NYPD_1011Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN PL_RaidenEquipmentManager */
		{ 0x01001020, PL_RaidenEquipmentManager_1020Launch },
	/* EXTERN NewDemoArkms_CitMale */
		{ 0x01001028, NewDemoArkms_CitMale_1028Launch },
	/* EXTERN NewDemoArkms_CitFemale */
		{ 0x01001029, NewDemoArkms_CitFemale_1029Launch },
	/* EXTERN NewSlowParamMan_prog */
		{ 0x01002003, NewSlowParamMan_prog_2003Launch },
	/* EXTERN NewLocalWind2_Demo */
		{ 0x01002004, NewLocalWind2_Demo_2004Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003000, NewRopeModel3_called_3000Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewEvmHairModel_Demo2 */
		{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003017, NewRopeModel3_called_3017Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003019, NewRopeModel3_called_3019Launch },
	/* EXTERN NewDebris_Cm_Demo */
		{ 0x01006007, NewDebris_Cm_Demo_6007Launch },
	/* EXTERN NewMesgBomb3 */
		{ 0x01006013, NewMesgBomb3_6013Launch },
	/* EXTERN NewParrotFeather */
		{ 0x0100601A, NewParrotFeather_601aLaunch },
	/* EXTERN NewDemoBodyPlasma */
		{ 0x01006020, NewDemoBodyPlasma_6020Launch },
	/* EXTERN NewDemoBladeSpark */
		{ 0x01006024, NewDemoBladeSpark_6024Launch },
	/* EXTERN NewDemoSolidusBladeLight */
		{ 0x01006026, NewDemoSolidusBladeLight_6026Launch },
	/* EXTERN NewDemoSolidusEnergyPrim */
		{ 0x01006027, NewDemoSolidusEnergyPrim_6027Launch },
	/* EXTERN NewDemoBladeFlow */
		{ 0x01006028, NewDemoBladeFlow_6028Launch },
	/* EXTERN NewDemoPlasmaLineColor */
		{ 0x0100602A, NewDemoPlasmaLineColor_602aLaunch },
	/* EXTERN NewDemoSplashBlood */
		{ 0x0100700D, NewDemoSplashBlood_700dLaunch },
	/* EXTERN NewSmokeMitsukoshiMan */
		{ 0x0100710E, NewSmokeMitsukoshiMan_710eLaunch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
	/* EXTERN NewEvmLateControl */
		{ 0x0100C100, NewEvmLateControl_c100Launch },
	/* EXTERN DM_ExecProc */
		{ 0x010FFF01, DM_ExecProc_fff01Launch },
	/* EXTERN NewDemoFrameCountCall */
		{ 0x010FFF03, NewDemoFrameCountCall_fff03Launch },
	/* EXTERN NewControlBoy */
		{ 0x010FFF04, NewControlBoy_fff04Launch },
	/* EXTERN NewDemoEffectInitialize */
		{ 0x010FFF05, NewDemoEffectInitialize_fff05Launch },
	/* EXTERN DM_ControlChange */
		{ 0x010FFFF0, DM_ControlChange_ffff0Launch },
/* gcl chara end */
	{ 0, NULL }
};
