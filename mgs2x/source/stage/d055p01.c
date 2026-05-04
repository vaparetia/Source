/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewPluginWaterMode;
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewCreateEquipmentScn;
extern NEWCHARA NewAnmtexSet;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewVentilatorLight;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewScrBubble;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA NewWaterLevelControl;
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
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewSpotLightImageSet;
extern NEWCHARA NewCom_BRK_SPLGT_BreakMessage;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewPoolWaterComplex;
extern NEWCHARA NewWaterModeSetting;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewBubbleOnCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewWaterDustManager;
extern NEWCHARA NewFlowPaper;
extern NEWCHARA NewDust;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewFogSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewPassageWaterSet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewScrWater;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewDustOnCamera;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewDoorPannelSpark;
extern NEWCHARA NewDustArea;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewScrTrans;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewExecProcContinual;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA New2DSprite_Prog_000dLaunch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffect_0012Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA NewScrWater_Demo_0014Launch;
extern NEWCHARA NewRainFogPersFast_0019Launch;
extern NEWCHARA NewNyou_0022Launch;
extern NEWCHARA NewSplashMotion_Demo_0100Launch;
extern NEWCHARA NewSplashRipple_Demo_0101Launch;
extern NEWCHARA NewWallTidal_0108Launch;
extern NEWCHARA NewRipBubbleMan_DEMO_0201Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewEmmaEquip_1015Launch;
extern NEWCHARA NewDemoArmControl_101bLaunch;
extern NEWCHARA NewDependArms_101cLaunch;
extern NEWCHARA NewBreathDemo_101eLaunch;
extern NEWCHARA PL_RaidenEquipmentManager_1020Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA NewMesgBomb3_6013Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA NewSplushSurfaceMan_a003Launch;
extern NEWCHARA NewRippleMan_a004Launch;
extern NEWCHARA NewWaterLevelControl_Demo_b000Launch;
extern NEWCHARA NewDropBodySplush_b001Launch;
extern NEWCHARA NewBloodWater_demo_b007Launch;
extern NEWCHARA NewEvmLateControl_c100Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 127 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 新プラグイン水中 */
		{ 0x00004C00, NewPluginWaterMode },
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* chara 装備品セット */
		{ 0x000381AC, NewCreateEquipmentScn },
	/* chara アニメテクスチャセット */
		{ 0x000423C8, NewAnmtexSet },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 天窓の光 */
		{ 0x000BA748, NewVentilatorLight },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara ゆがみ泡 */
		{ 0x001AF0D5, NewScrBubble },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara 静止スポットライト設置 */
		{ 0x001B5854, NewPutStaticSpotLight },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
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
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara スポットライト背景イメージ指定 */
		{ 0x0048F8F8, NewSpotLightImageSet },
	/* command プットスポットライトオブジェライト消す */
		{ 0x0049EB5E, NewCom_BRK_SPLGT_BreakMessage },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara バンププール */
		{ 0x005C3A78, NewPoolWaterComplex },
	/* command 新水中設定 */
		{ 0x0060C857, NewWaterModeSetting },
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
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara カメラ泡 */
		{ 0x006B921A, NewBubbleOnCamera },
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
	/* chara 水中ゴミマネージャ */
		{ 0x00873375, NewWaterDustManager },
	/* chara 浮遊紙 */
		{ 0x008975E6, NewFlowPaper },
	/* chara 浮遊物 */
		{ 0x0089778A, NewDust },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* chara スポットライト投影モデル */
		{ 0x009307AB, NewSpotDrawObjectSet },
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
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara 通路用水面 */
		{ 0x00C5D24C, NewPassageWaterSet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 水中カメラ */
		{ 0x00D16C76, NewScrWater },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* chara カメラダスト */
		{ 0x00DC323F, NewDustOnCamera },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* chara ドアパネル電撃 */
		{ 0x00E2ED79, NewDoorPannelSpark },
	/* chara エリアダスト */
		{ 0x00E52073, NewDustArea },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara フレーム退避 */
		{ 0x00F62833, NewScrTrans },
	/* command デモローポリ劇場 */
		{ 0x00F8F4E8, NewCom_AddRenameList },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
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
	/* EXTERN NewCrossFadeEffectCustom */
		{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	/* EXTERN NewScrWater_Demo */
		{ 0x01000014, NewScrWater_Demo_0014Launch },
	/* EXTERN NewRainFogPersFast */
		{ 0x01000019, NewRainFogPersFast_0019Launch },
	/* EXTERN NewNyou */
		{ 0x01000022, NewNyou_0022Launch },
	/* EXTERN NewSplashMotion_Demo */
		{ 0x01000100, NewSplashMotion_Demo_0100Launch },
	/* EXTERN NewSplashRipple_Demo */
		{ 0x01000101, NewSplashRipple_Demo_0101Launch },
	/* EXTERN NewWallTidal */
		{ 0x01000108, NewWallTidal_0108Launch },
	/* EXTERN NewRipBubbleMan_DEMO */
		{ 0x01000201, NewRipBubbleMan_DEMO_0201Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN NewEmmaEquip */
		{ 0x01001015, NewEmmaEquip_1015Launch },
	/* EXTERN NewDemoArmControl */
		{ 0x0100101B, NewDemoArmControl_101bLaunch },
	/* EXTERN NewDependArms */
		{ 0x0100101C, NewDependArms_101cLaunch },
	/* EXTERN NewBreathDemo */
		{ 0x0100101E, NewBreathDemo_101eLaunch },
	/* EXTERN PL_RaidenEquipmentManager */
		{ 0x01001020, PL_RaidenEquipmentManager_1020Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewEvmHairModel_Demo2 */
		{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	/* EXTERN NewMesgBomb3 */
		{ 0x01006013, NewMesgBomb3_6013Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
	/* EXTERN NewSplushSurfaceMan */
		{ 0x0100A003, NewSplushSurfaceMan_a003Launch },
	/* EXTERN NewRippleMan */
		{ 0x0100A004, NewRippleMan_a004Launch },
	/* EXTERN NewWaterLevelControl_Demo */
		{ 0x0100B000, NewWaterLevelControl_Demo_b000Launch },
	/* EXTERN NewDropBodySplush */
		{ 0x0100B001, NewDropBodySplush_b001Launch },
	/* EXTERN NewBloodWater_demo */
		{ 0x0100B007, NewBloodWater_demo_b007Launch },
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
