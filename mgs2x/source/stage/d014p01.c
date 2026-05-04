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
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewCom_BRK_SPLGT_BreakMessage;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewPutBreakObject;
extern NEWCHARA NewPutRadleObject;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewPutGlassObject;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewPutPotatoObject;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewSigPutModelEvm;
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
extern NEWCHARA NewFlyingWarmScn;
extern NEWCHARA NewHako_purapura;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewPutPlateObject;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewMpegPssMovieStr;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA New2DSprite_Prog_000dLaunch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffect_0012Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewDependArms_101cLaunch;
extern NEWCHARA PL_RaidenEquipmentManager_1020Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA NewEvmLateControl_c100Launch;
extern NEWCHARA DM_SetClipParam_fff02Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 90 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
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
	/* chara プットスポットライトオブジェ */
		{ 0x00365DEE, NewPutSpotLightObject },
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
	/* command プットスポットライトオブジェライト消す */
		{ 0x0049EB5E, NewCom_BRK_SPLGT_BreakMessage },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara プット壊れオブジェ */
		{ 0x00532262, NewPutBreakObject },
	/* chara プット揺れオブジェ */
		{ 0x00532F32, NewPutRadleObject },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* chara プットガラスオブジェ */
		{ 0x005A4809, NewPutGlassObject },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
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
	/* chara プットポテトオブジェ */
		{ 0x006C577D, NewPutPotatoObject },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* chara プットモーションモデル */
		{ 0x0072F23C, NewSigPutModelEvm },
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
	/* chara 電灯虫 */
		{ 0x00B66AE4, NewFlyingWarmScn },
	/* chara 蓋付き段ボール */
		{ 0x00B8B94D, NewHako_purapura },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara プット皿オブジェ */
		{ 0x00DF5435, NewPutPlateObject },
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
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* chara MOVIE再生 */
		{ 0x00FFBECE, NewMpegPssMovieStr },
	/* EXTERN NewFogSet_Demo */
		{ 0x01000000, NewFogSet_Demo_0000Launch },
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
	/* EXTERN New2DSprite_Prog */
		{ 0x0100000D, New2DSprite_Prog_000dLaunch },
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewCrossFadeEffect */
		{ 0x01000012, NewCrossFadeEffect_0012Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN NewDependArms */
		{ 0x0100101C, NewDependArms_101cLaunch },
	/* EXTERN PL_RaidenEquipmentManager */
		{ 0x01001020, PL_RaidenEquipmentManager_1020Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewEvmHairModel_Demo2 */
		{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
	/* EXTERN NewEvmLateControl */
		{ 0x0100C100, NewEvmLateControl_c100Launch },
	/* EXTERN DM_SetClipParam */
		{ 0x010FFF02, DM_SetClipParam_fff02Launch },
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
