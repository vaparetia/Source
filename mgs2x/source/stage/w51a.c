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
extern NEWCHARA NewShadowDrawObjectSet;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_VitalityAdjust;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA PL_COM_GetDemoSnakeArmHangPos;
extern NEWCHARA PL_COM_SetRecoverValue;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA PL_CHARA_ScnVibCamera;
extern NEWCHARA NewStrmFader;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewSigRouteModel;
extern NEWCHARA NewEasyLayout;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewRainFogPers;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewStreamSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewDemoSnakeArm;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewPadCancel;
extern NEWCHARA PL_COM_VisibleO2Gage;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewO2Gage;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA PL_COM_SetPadType;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBlur;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSubjectDemoPlayer;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewHugeSeaSurfaceSet;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewAttachment3;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewSteamOnCameraScenario;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewMpegPssMovieStr;
extern NEWCHARA NewExecProcContinual;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA NewRainFogPersDemo_0033Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewArmsEffectControl_1010Launch;
extern NEWCHARA NewEyeControl2_1013Launch;
extern NEWCHARA NewLinerGunInitEffect_1019Launch;
extern NEWCHARA PL_RaidenEquipmentManager_1020Launch;
extern NEWCHARA NewFortSling2_1024Launch;
extern NEWCHARA NewLocalWind2_Demo_2004Launch;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA NewRopeModel3_called_3004Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewEvmHairModel_Demo2_3009Launch;
extern NEWCHARA NewHandcuff_called_3015Launch;
extern NEWCHARA NewMesgBomb3_6013Launch;
extern NEWCHARA NewMAOParticle_601cLaunch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 128 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara 影投影モデル */
		{ 0x0005C815, NewShadowDrawObjectSet },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command プレイヤーライフ値操作 */
		{ 0x00122E63, PL_COM_VitalityAdjust },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command デモ蛇手つかみ位置取得 */
		{ 0x001CB7B7, PL_COM_GetDemoSnakeArmHangPos },
	/* command Ｏ２ゲージ回復力設定 */
		{ 0x002C4B35, PL_COM_SetRecoverValue },
	/* EXTERN NewNearFocusEffect */
		{ 0x002DAD24, NewFarFocusEffectSet },
	/* chara 髭剃りスネークテクスチャ入れ替え */
		{ 0x002DF578, NewShavedSnakeTexReplaceActorScn },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara シナリオカメラ振動 */
		{ 0x003124A2, PL_CHARA_ScnVibCamera },
	/* chara 台詞発生源君 */
		{ 0x0031D58B, NewStrmFader },
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
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* chara ルートデモ人形 */
		{ 0x00492A40, NewSigRouteModel },
	/* chara 簡易レイアウトプレイヤ */
		{ 0x004A0018, NewEasyLayout },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
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
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 太陽 */
		{ 0x00641A7B, NewPlantSunMain },
	/* command ハリアー戦で誰がプレイヤーを攻撃したか */
		{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* command ゲームオーバー処理終了 */
		{ 0x006856C1, GM_GameOverEndScn },
	/* chara パッド振動 */
		{ 0x0068CB9C, NewPadVibrationScn },
	/* chara 霧雨 */
		{ 0x0068F6CB, NewRainFogPers },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
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
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* chara デモ蛇手 */
		{ 0x0086D382, NewDemoSnakeArm },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command パッドチェック */
		{ 0x008B6086, NewPadCheck },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command Ｏ２ゲージ表示 */
		{ 0x009208DC, PL_COM_VisibleO2Gage },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
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
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command プレイヤーパッド設定 */
		{ 0x00B414AE, PL_COM_SetPadType },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ブラー */
		{ 0x00B63A33, NewBlur },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara 影管理 */
		{ 0x00BD400B, NewShadowControl },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara 主観デモプレイヤー */
		{ 0x00C1BC23, NewSubjectDemoPlayer },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 汎用空 */
		{ 0x00C9DD51, NewSkyUtil },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 巨大海面 */
		{ 0x00D20FDE, NewHugeSeaSurfaceSet },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara 装備品Ｃ */
		{ 0x00DE0402, NewAttachment3 },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara カメラ前曇り */
		{ 0x00ED4678, NewSteamOnCameraScenario },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* chara MOVIE再生 */
		{ 0x00FFBECE, NewMpegPssMovieStr },
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
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewCrossFadeEffectCustom */
		{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	/* EXTERN NewRainFogPersDemo */
		{ 0x01000033, NewRainFogPersDemo_0033Launch },
	/* EXTERN NewPutAttachments */
		{ 0x01001006, NewPutAttachments_1006Launch },
	/* EXTERN NewArmsEffectControl */
		{ 0x01001010, NewArmsEffectControl_1010Launch },
	/* EXTERN NewEyeControl2 */
		{ 0x01001013, NewEyeControl2_1013Launch },
	/* EXTERN NewLinerGunInitEffect */
		{ 0x01001019, NewLinerGunInitEffect_1019Launch },
	/* EXTERN PL_RaidenEquipmentManager */
		{ 0x01001020, PL_RaidenEquipmentManager_1020Launch },
	/* EXTERN NewFortSling2 */
		{ 0x01001024, NewFortSling2_1024Launch },
	/* EXTERN NewLocalWind2_Demo */
		{ 0x01002004, NewLocalWind2_Demo_2004Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003000, NewRopeModel3_called_3000Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003004, NewRopeModel3_called_3004Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewEvmHairModel_Demo2 */
		{ 0x01003009, NewEvmHairModel_Demo2_3009Launch },
	/* EXTERN NewHandcuff_called */
		{ 0x01003015, NewHandcuff_called_3015Launch },
	/* EXTERN NewMesgBomb3 */
		{ 0x01006013, NewMesgBomb3_6013Launch },
	/* EXTERN NewMAOParticle */
		{ 0x0100601C, NewMAOParticle_601cLaunch },
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
