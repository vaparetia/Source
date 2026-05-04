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
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewShadowDrawObject2Set;
extern NEWCHARA NewCartridgeM4_demoALL;
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
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewCartridgeM4_demo_gunALL;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewSeaSurfaceSet;
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
extern NEWCHARA NewKamomeManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewExecProcContinual;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewContrastForce_Demo_000aLaunch;
extern NEWCHARA NewLensFlare_Demo_000fLaunch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffect_0012Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA OK_FogStatusSet_0018Launch;
extern NEWCHARA NewRainFogPersFast2_0031Launch;
extern NEWCHARA NewArmsEffectControl_1010Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewFortEquip_1014Launch;
extern NEWCHARA NewEyeAnimSEALS_Demo_1016Launch;
extern NEWCHARA NewFortSling_1017Launch;
extern NEWCHARA NewFortBulletDemo_1018Launch;
extern NEWCHARA NewLinerGunInitEffect_1019Launch;
extern NEWCHARA NewDemoArmControl_101bLaunch;
extern NEWCHARA NewDependArms_101cLaunch;
extern NEWCHARA NewLocalWind2_Demo_2004Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA NewRopeModel3_called_3014Launch;
extern NEWCHARA NewPlasmaPoly_Demo_6002Launch;
extern NEWCHARA NewDebris_Cm_Demo_6007Launch;
extern NEWCHARA NewDebris_Tex_Demo_6008Launch;
extern NEWCHARA NewMesgBomb3_6013Launch;
extern NEWCHARA NewFortBarrierDemo_6014Launch;
extern NEWCHARA NewDebris_Tex_Demo2_6016Launch;
extern NEWCHARA NewLineSmoke_7103Launch;
extern NEWCHARA NewDemoRisingSmoke_7107Launch;
extern NEWCHARA NewSmokeStripControl_710dLaunch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 115 functions */
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
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara 影投影モデル２ */
		{ 0x00206929, NewShadowDrawObject2Set },
	/* chara M4弾 */
		{ 0x00276926, NewCartridgeM4_demoALL },
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
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
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
	/* chara フォグコントローラ */
		{ 0x0066B097, NewFogControl },
	/* chara M4_DEMO_GUN弾 */
		{ 0x0068392D, NewCartridgeM4_demo_gunALL },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara プラント海面 */
		{ 0x006AE654, NewSeaSurfaceSet },
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
	/* chara かもめマネージャ */
		{ 0x00AF4CF6, NewKamomeManager },
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
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 汎用空 */
		{ 0x00C9DD51, NewSkyUtil },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
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
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
	/* EXTERN NewFarFocusEffect */
		{ 0x01000001, NewFarFocusEffect_0001Launch },
	/* EXTERN NewNearFocusEffect */
		{ 0x01000002, NewNearFocusEffect_0002Launch },
	/* EXTERN DM_ChangeAmbient */
		{ 0x01000006, DM_ChangeAmbient_0006Launch },
	/* EXTERN DM_ChangeParallel */
		{ 0x01000007, DM_ChangeParallel_0007Launch },
	/* EXTERN NewFadeInOutForce_Demo */
		{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	/* EXTERN NewContrastForce_Demo */
		{ 0x0100000A, NewContrastForce_Demo_000aLaunch },
	/* EXTERN NewLensFlare_Demo */
		{ 0x0100000F, NewLensFlare_Demo_000fLaunch },
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewCrossFadeEffect */
		{ 0x01000012, NewCrossFadeEffect_0012Launch },
	/* EXTERN NewCrossFadeEffectCustom */
		{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	/* EXTERN OK_FogStatusSet */
		{ 0x01000018, OK_FogStatusSet_0018Launch },
	/* EXTERN NewRainFogPersFast2 */
		{ 0x01000031, NewRainFogPersFast2_0031Launch },
	/* EXTERN NewArmsEffectControl */
		{ 0x01001010, NewArmsEffectControl_1010Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN NewFortEquip */
		{ 0x01001014, NewFortEquip_1014Launch },
	/* EXTERN NewEyeAnimSEALS_Demo */
		{ 0x01001016, NewEyeAnimSEALS_Demo_1016Launch },
	/* EXTERN NewFortSling */
		{ 0x01001017, NewFortSling_1017Launch },
	/* EXTERN NewFortBulletDemo */
		{ 0x01001018, NewFortBulletDemo_1018Launch },
	/* EXTERN NewLinerGunInitEffect */
		{ 0x01001019, NewLinerGunInitEffect_1019Launch },
	/* EXTERN NewDemoArmControl */
		{ 0x0100101B, NewDemoArmControl_101bLaunch },
	/* EXTERN NewDependArms */
		{ 0x0100101C, NewDependArms_101cLaunch },
	/* EXTERN NewLocalWind2_Demo */
		{ 0x01002004, NewLocalWind2_Demo_2004Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewEvmHairModel_Demo2 */
		{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003014, NewRopeModel3_called_3014Launch },
	/* EXTERN NewPlasmaPoly_Demo */
		{ 0x01006002, NewPlasmaPoly_Demo_6002Launch },
	/* EXTERN NewDebris_Cm_Demo */
		{ 0x01006007, NewDebris_Cm_Demo_6007Launch },
	/* EXTERN NewDebris_Tex_Demo */
		{ 0x01006008, NewDebris_Tex_Demo_6008Launch },
	/* EXTERN NewMesgBomb3 */
		{ 0x01006013, NewMesgBomb3_6013Launch },
	/* EXTERN NewFortBarrierDemo */
		{ 0x01006014, NewFortBarrierDemo_6014Launch },
	/* EXTERN NewDebris_Tex_Demo2 */
		{ 0x01006016, NewDebris_Tex_Demo2_6016Launch },
	/* EXTERN NewLineSmoke */
		{ 0x01007103, NewLineSmoke_7103Launch },
	/* EXTERN NewDemoRisingSmoke */
		{ 0x01007107, NewDemoRisingSmoke_7107Launch },
	/* EXTERN NewSmokeStripControl */
		{ 0x0100710D, NewSmokeStripControl_710dLaunch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
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
