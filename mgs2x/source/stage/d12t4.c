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
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewCartridgeM4_demoALL;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewReflectionWaterSurface2Set;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewCartridgeM4_demo_gunALL;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewCartridgeUspALL;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewSplushMan;
extern NEWCHARA NewRopeModel2;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewZman;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA NewFadeInOut_Demo_0003Launch;
extern NEWCHARA NewRainCamera_Demo_0004Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewContrastForce_Demo_000aLaunch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA NewScrWater_Demo_0014Launch;
extern NEWCHARA NewScrConcentrateBlur_0015Launch;
extern NEWCHARA NewSphereSplush_0106Launch;
extern NEWCHARA NewWaveSplash_Demo_0107Launch;
extern NEWCHARA NewWallTidal_0108Launch;
extern NEWCHARA NewEneEquip_1002Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewPutAttachments_100cLaunch;
extern NEWCHARA NewEyeControl_100dLaunch;
extern NEWCHARA NewDemoArkms_100fLaunch;
extern NEWCHARA NewArmsEffectControl_1010Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewDependArms_USP_SP_1021Launch;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewDebris_Cm_Demo_6007Launch;
extern NEWCHARA NewDebris_Tex_Demo_6008Launch;
extern NEWCHARA NewBombEffect_6009Launch;
extern NEWCHARA NewBombGasEffect_600bLaunch;
extern NEWCHARA NewTs_Spark_600eLaunch;
extern NEWCHARA NewMesgBomb2_6010Launch;
extern NEWCHARA NewBlood2_Demo_7002Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA NewWaterLevelControl_Demo_b000Launch;
extern NEWCHARA NewSplushTidal_b003Launch;
extern NEWCHARA NewRayConsol_d000Launch;
extern NEWCHARA NewRayEye_d001Launch;
extern NEWCHARA NewWaterPollute_demo_d002Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 104 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* chara M4弾 */
		{ 0x00276926, NewCartridgeM4_demoALL },
	/* chara 遠景ぼかし */
		{ 0x002DAD24, NewFarFocusEffectSet },
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
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara キラキラ水面２ */
		{ 0x0060CF19, NewReflectionWaterSurface2Set },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara M4_DEMO_GUN弾 */
		{ 0x0068392D, NewCartridgeM4_demo_gunALL },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* chara USP弾 */
		{ 0x007AD92B, NewCartridgeUspALL },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara 水飛沫管理 */
		{ 0x0098BD5B, NewSplushMan },
	/* chara ロープモデル２ */
		{ 0x009BC66F, NewRopeModel2 },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara 近景ぼかし */
		{ 0x00BBAD24, NewNearFocusEffectSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
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
	/* EXTERN NewRainCamera_Demo */
		{ 0x01000004, NewRainCamera_Demo_0004Launch },
	/* EXTERN DM_ChangeAmbient */
		{ 0x01000006, DM_ChangeAmbient_0006Launch },
	/* EXTERN DM_ChangeParallel */
		{ 0x01000007, DM_ChangeParallel_0007Launch },
	/* EXTERN NewFadeInOutForce_Demo */
		{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	/* EXTERN NewContrastForce_Demo */
		{ 0x0100000A, NewContrastForce_Demo_000aLaunch },
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewCrossFadeEffectCustom */
		{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	/* EXTERN NewScrWater_Demo */
		{ 0x01000014, NewScrWater_Demo_0014Launch },
	/* EXTERN NewScrConcentrateBlur */
		{ 0x01000015, NewScrConcentrateBlur_0015Launch },
	/* EXTERN NewSphereSplush */
		{ 0x01000106, NewSphereSplush_0106Launch },
	/* EXTERN NewWaveSplash_Demo */
		{ 0x01000107, NewWaveSplash_Demo_0107Launch },
	/* EXTERN NewWallTidal */
		{ 0x01000108, NewWallTidal_0108Launch },
	/* EXTERN NewEneEquip */
		{ 0x01001002, NewEneEquip_1002Launch },
	/* EXTERN NewPutAttachments */
		{ 0x01001006, NewPutAttachments_1006Launch },
	/* EXTERN NewPutAttachments */
		{ 0x0100100C, NewPutAttachments_100cLaunch },
	/* EXTERN NewEyeControl */
		{ 0x0100100D, NewEyeControl_100dLaunch },
	/* EXTERN NewDemoArkms */
		{ 0x0100100F, NewDemoArkms_100fLaunch },
	/* EXTERN NewArmsEffectControl */
		{ 0x01001010, NewArmsEffectControl_1010Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN NewDependArms_USP_SP */
		{ 0x01001021, NewDependArms_USP_SP_1021Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003000, NewRopeModel3_called_3000Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewDebris_Cm_Demo */
		{ 0x01006007, NewDebris_Cm_Demo_6007Launch },
	/* EXTERN NewDebris_Tex_Demo */
		{ 0x01006008, NewDebris_Tex_Demo_6008Launch },
	/* EXTERN NewBombEffect */
		{ 0x01006009, NewBombEffect_6009Launch },
	/* EXTERN NewBombGasEffect */
		{ 0x0100600B, NewBombGasEffect_600bLaunch },
	/* EXTERN NewTs_Spark */
		{ 0x0100600E, NewTs_Spark_600eLaunch },
	/* EXTERN NewMesgBomb2 */
		{ 0x01006010, NewMesgBomb2_6010Launch },
	/* EXTERN NewBlood2_Demo */
		{ 0x01007002, NewBlood2_Demo_7002Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
	/* EXTERN NewWaterLevelControl_Demo */
		{ 0x0100B000, NewWaterLevelControl_Demo_b000Launch },
	/* EXTERN NewSplushTidal */
		{ 0x0100B003, NewSplushTidal_b003Launch },
	/* EXTERN NewRayConsol */
		{ 0x0100D000, NewRayConsol_d000Launch },
	/* EXTERN NewRayEye */
		{ 0x0100D001, NewRayEye_d001Launch },
	/* EXTERN NewWaterPollute_demo */
		{ 0x0100D002, NewWaterPollute_demo_d002Launch },
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
