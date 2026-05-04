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
extern NEWCHARA NewGclCos;
extern NEWCHARA NewGclSin;
extern NEWCHARA NewCreateEquipmentScn;
extern NEWCHARA NewAnmtexSet;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewCheckSlater;
extern NEWCHARA NewFortune;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewRaiden;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewGetFortunePos;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNodeLamp;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA GM_COM_AddSeJimaku;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewCom_BRK_SPLGT_LightMessage;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewCom_BRK_SPLGT_BreakMessage;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewFortDynamicFlow;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoor;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewHzxResetGroupAdd;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewTrampleSlater;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewFortHideObject;
extern NEWCHARA NewDelay;
extern NEWCHARA NewGclLangUpdate;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewFortCeiling;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewO2Gage;
extern NEWCHARA NewFortWallLight;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewFloorLightMan;
extern NEWCHARA NewDemoSlater;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewFortHangLight;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewPutElevator;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewFortElevatorButton;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewCom_FRT_CEL_Control;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA NewOilSpread;
extern NEWCHARA NewGetFortuneBulletPos;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewGetFortuneBombPos;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA FRT_AIM_InitAimSpot;
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
extern NEWCHARA NewScrConcentrateBlur_0015Launch;
extern NEWCHARA NewFortuneTear_0023Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewFortEquip_1014Launch;
extern NEWCHARA NewFortSling_1017Launch;
extern NEWCHARA NewFortBulletDemo_1018Launch;
extern NEWCHARA NewLinerGunInitEffect_1019Launch;
extern NEWCHARA NewDemoArmControl_101bLaunch;
extern NEWCHARA NewDependArms_101cLaunch;
extern NEWCHARA NewBreathDemo_101eLaunch;
extern NEWCHARA PL_RaidenEquipmentManager_1020Launch;
extern NEWCHARA NewDependArms_Kill_102aLaunch;
extern NEWCHARA NewLocalWind2_Demo_2004Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA NewDebris_Cm_Demo_6007Launch;
extern NEWCHARA NewDebris_Tex_Demo_6008Launch;
extern NEWCHARA NewMesgBomb2_6010Launch;
extern NEWCHARA NewMesgBomb3_6013Launch;
extern NEWCHARA NewFortBarrierDemo_6014Launch;
extern NEWCHARA NewBlood2_Demo_7002Launch;
extern NEWCHARA NewCommonSmoke_7101Launch;
extern NEWCHARA NewLineSmoke_7103Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 162 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* chara 装備品セット */
		{ 0x000381AC, NewCreateEquipmentScn },
	/* chara アニメテクスチャセット */
		{ 0x000423C8, NewAnmtexSet },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* chara 船虫踏み潰し管理 */
		{ 0x0007E215, NewCheckSlater },
	/* chara ボスフォーチュン */
		{ 0x000A4D33, NewFortune },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara 静止スポットライト設置 */
		{ 0x001B5854, NewPutStaticSpotLight },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* chara 髭剃りスネークテクスチャ入れ替え */
		{ 0x002DF578, NewShavedSnakeTexReplaceActorScn },
	/* command ゲットフォーチュン座標 */
		{ 0x002E2ACB, NewGetFortunePos },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* chara demo */
		{ 0x0033A20F, NewPolygonDemoStart },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* chara プットスポットライトオブジェ */
		{ 0x00365DEE, NewPutSpotLightObject },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ノード端末ディスプレイ */
		{ 0x003B1909, NewNodeLamp },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command ＳＥ字幕登録 */
		{ 0x004147A5, GM_COM_AddSeJimaku },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command プットスポットライトオブジェライト管理 */
		{ 0x004508C2, NewCom_BRK_SPLGT_LightMessage },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command プットスポットライトオブジェライト消す */
		{ 0x0049EB5E, NewCom_BRK_SPLGT_BreakMessage },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara フォーチュン戦巻き上げ煙 */
		{ 0x0053BAF5, NewFortDynamicFlow },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command 追加ＨＺＸグループ削除 */
		{ 0x0057A485, NewHzxResetGroupAdd },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ハリアー戦で誰がプレイヤーを攻撃したか */
		{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 潰れふなむし */
		{ 0x00675145, NewTrampleSlater },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* chara フォーチュン戦ライデン隠れオブジェ */
		{ 0x006FDA0F, NewFortHideObject },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command 字幕言語設定 */
		{ 0x0074E86B, NewGclLangUpdate },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command 無線メモリー設定 */
		{ 0x007B35E0, CodecMemCallSet },
	/* command カメラ視界チェック */
		{ 0x007F3C88, NewViewCheckCommand },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* chara フォーチュン戦天井崩れ */
		{ 0x00912BFE, NewFortCeiling },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* chara スポットライト投影モデル */
		{ 0x009307AB, NewSpotDrawObjectSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* chara Ｏ２ゲージ */
		{ 0x00A7CB42, NewO2Gage },
	/* chara フォーチュン戦壁ライト */
		{ 0x00AD8864, NewFortWallLight },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara 床照り返し管理 */
		{ 0x00C547A7, NewFloorLightMan },
	/* chara デモ虫 */
		{ 0x00C622A4, NewDemoSlater },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* command メニューフナムシ取り付き状態取得 */
		{ 0x00CB6EC4, NewGetShipwormStatus },
	/* chara 天井君 */
		{ 0x00CBB124, NewCeiling },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command 跳弾ＳＥのみ */
		{ 0x00D10066, NewEntrySparkOnlySE },
	/* chara フォーチュン戦揺れライト */
		{ 0x00D110E2, NewFortHangLight },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* chara フォーチュン戦昇降機 */
		{ 0x00D90540, NewPutElevator },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* chara フォーチュン戦昇降機スイッチ */
		{ 0x00DBD1FA, NewFortElevatorButton },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command フォーチュン戦天井崩れ表示管理 */
		{ 0x00E2488B, NewCom_FRT_CEL_Control },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* chara フォーチュン戦オイル燃え */
		{ 0x00E3429D, NewOilSpread },
	/* command ゲットフォーチュン弾座標 */
		{ 0x00E3EEA2, NewGetFortuneBulletPos },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command ゲットフォーチュングレネード座標 */
		{ 0x00EFD440, NewGetFortuneBombPos },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command デモローポリ劇場 */
		{ 0x00F8F4E8, NewCom_AddRenameList },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* command フォーチュン索敵初期化 */
		{ 0x00FE39E8, FRT_AIM_InitAimSpot },
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
	/* EXTERN NewScrConcentrateBlur */
		{ 0x01000015, NewScrConcentrateBlur_0015Launch },
	/* EXTERN NewFortuneTear */
		{ 0x01000023, NewFortuneTear_0023Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN NewFortEquip */
		{ 0x01001014, NewFortEquip_1014Launch },
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
	/* EXTERN NewBreathDemo */
		{ 0x0100101E, NewBreathDemo_101eLaunch },
	/* EXTERN PL_RaidenEquipmentManager */
		{ 0x01001020, PL_RaidenEquipmentManager_1020Launch },
	/* EXTERN NewDependArms_Kill */
		{ 0x0100102A, NewDependArms_Kill_102aLaunch },
	/* EXTERN NewLocalWind2_Demo */
		{ 0x01002004, NewLocalWind2_Demo_2004Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewEvmHairModel_Demo2 */
		{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	/* EXTERN NewDebris_Cm_Demo */
		{ 0x01006007, NewDebris_Cm_Demo_6007Launch },
	/* EXTERN NewDebris_Tex_Demo */
		{ 0x01006008, NewDebris_Tex_Demo_6008Launch },
	/* EXTERN NewMesgBomb2 */
		{ 0x01006010, NewMesgBomb2_6010Launch },
	/* EXTERN NewMesgBomb3 */
		{ 0x01006013, NewMesgBomb3_6013Launch },
	/* EXTERN NewFortBarrierDemo */
		{ 0x01006014, NewFortBarrierDemo_6014Launch },
	/* EXTERN NewBlood2_Demo */
		{ 0x01007002, NewBlood2_Demo_7002Launch },
	/* EXTERN NewCommonSmoke */
		{ 0x01007101, NewCommonSmoke_7101Launch },
	/* EXTERN NewLineSmoke */
		{ 0x01007103, NewLineSmoke_7103Launch },
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
