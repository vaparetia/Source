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
extern NEWCHARA NewEMA_CommandGetPosition;
extern NEWCHARA NewGclVarClear;
extern NEWCHARA GM_Scn_SetRouteOffset;
extern NEWCHARA NewCamera;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewVRMap3D_Scn;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA SetVR_BOMBS_NUM;
extern NEWCHARA PL_COM_VitalityAdjust;
extern NEWCHARA PL_COM_BladeKillPlugin;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewPowerSupplyLight;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA NewVRSys;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewVRWindow_Scn;
extern NEWCHARA VR_StageStart;
extern NEWCHARA VR_TimerStart;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewElevator;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA COMVRGameOverProc;
extern NEWCHARA NewPlayerAp;
extern NEWCHARA COM_ChangeClearingRoot;
extern NEWCHARA NewSetKaitaiC4;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewGlassScarBase;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA SetVRStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewElavatorLamp;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewGunCamera;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBombEffectScn;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewGclAssert;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA SetVrStageId;
extern NEWCHARA SetVRBOMBS;
extern NEWCHARA NewPutVentObject;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA NewSetChanlTargetMap;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewDelay;
extern NEWCHARA NewBombArea;
extern NEWCHARA NewPluginElevator;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewVRClear_Scn;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA C4MAN_Init;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA VR_TimerPause;
extern NEWCHARA COM_GetEnemyStSt;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA NewZman;
extern NEWCHARA NewElectricFloor;
extern NEWCHARA NewEnbControl;
extern NEWCHARA COM_SetClearingData;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBlur;
extern NEWCHARA PL_COM_IntrudeSubjectCameraPositionMove;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewTs_SubWindow_S;
extern NEWCHARA VRCLR_GetResult;
extern NEWCHARA NewArraySet;
extern NEWCHARA PL_COM_DestroyBox;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA NewVRGoal;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewSetTimerStatus;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA NewCommander;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA NewDoorPannelSpark;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA COM_GetEnemyPos;
extern NEWCHARA GM_GetGameStatusForScn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA VR_StagePause;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA NewSlowDown;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewInvisibleChara;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewTargetProc;
extern NEWCHARA NewExecProcContinual;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewRainFogPersFast_0019Launch;
extern NEWCHARA NewPlasmaPoly_Demo_6002Launch;
extern NEWCHARA NewBombEffect_6009Launch;
extern NEWCHARA NewTs_Spark_600eLaunch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
/* 178 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command ゲットエマ座標 */
		{ 0x0002EAD3, NewEMA_CommandGetPosition },
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command セットルートオフセット */
		{ 0x0005D91D, GM_Scn_SetRouteOffset },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara ＶＲ全体マップ表示３Ｄ */
		{ 0x000C3D82, NewVRMap3D_Scn },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command セット解体済み爆弾数 */
		{ 0x000FF733, SetVR_BOMBS_NUM },
	/* command プレイヤーライフ値操作 */
		{ 0x00122E63, PL_COM_VitalityAdjust },
	/* command プラグインブレード解除 */
		{ 0x00124BF9, PL_COM_BladeKillPlugin },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* chara 電源パネルライト */
		{ 0x001395B5, NewPowerSupplyLight },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara ＶＲシステム */
		{ 0x001D5983, NewVRSys },
	/* command プレイヤー描画モードセット */
		{ 0x00231FA7, PL_COM_SetObjectFlag },
	/* chara ＶＲウィンドウ */
		{ 0x0024C03E, NewVRWindow_Scn },
	/* command ＶＲステージスタート */
		{ 0x0024C830, VR_StageStart },
	/* command ＶＲタイマースタート */
		{ 0x0026AAB9, VR_TimerStart },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* chara 髭剃りスネークテクスチャ入れ替え */
		{ 0x002DF578, NewShavedSnakeTexReplaceActorScn },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara エレベータ */
		{ 0x0030E9CC, NewElevator },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* chara demo */
		{ 0x0033A20F, NewPolygonDemoStart },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command 特殊ＶＲゲームオーバー */
		{ 0x0037700F, COMVRGameOverProc },
	/* chara プレイヤ登場 */
		{ 0x003780F6, NewPlayerAp },
	/* command チェンジクリアリングルート */
		{ 0x0038D44E, COM_ChangeClearingRoot },
	/* chara 解体Ｃ４ */
		{ 0x003A6F25, NewSetKaitaiC4 },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command プレイヤーモーション振動ＯＦＦ */
		{ 0x003F46EC, NewPlayerMotionVibOff },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* chara ガラス弾痕 */
		{ 0x0046139A, NewGlassScarBase },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command セットＶＲステータス */
		{ 0x004909B5, SetVRStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* chara エレベータパネル制御 */
		{ 0x004CBFC5, NewElavatorLamp },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command はだかモードセット */
		{ 0x004F576C, PL_COM_SetNudeMode },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command プラグインブレード */
		{ 0x0056B5CB, NewPluginBlade },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* chara ガンカメラ */
		{ 0x00577FB3, NewGunCamera },
	/* chara 警備コマンダー */
		{ 0x0059EDA8, NewWcCommander },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara 攻撃コマンダー */
		{ 0x005BB4D4, NewAtCommander },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 爆発 */
		{ 0x006781AF, NewBombEffectScn },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* command ゲームオーバー処理終了 */
		{ 0x006856C1, GM_GameOverEndScn },
	/* chara パッド振動 */
		{ 0x0068CB9C, NewPadVibrationScn },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* command セットステージＩＤ */
		{ 0x006C274D, SetVrStageId },
	/* command セット爆弾数 */
		{ 0x006D61F3, SetVRBOMBS },
	/* chara プット壊れ通気口 */
		{ 0x006E0FA8, NewPutVentObject },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* command 主観移動セット */
		{ 0x006ECC1A, PL_COM_SetSubjectMove },
	/* command カメラマップ設定 */
		{ 0x006F5B0F, NewSetChanlTargetMap },
	/* command 強制モーション終了プロック */
		{ 0x00709DF6, NewForceActEndProc },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* command セットＶＲ発見回数 */
		{ 0x00718756, SetVR_DISCOVERY_CNT },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara 爆弾検知領域 */
		{ 0x0076A03C, NewBombArea },
	/* command プラグインエレベータ */
		{ 0x00770EC3, NewPluginElevator },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command カメラ視界チェック */
		{ 0x007F3C88, NewViewCheckCommand },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command ＶＲゲットタイム */
		{ 0x0080FB82, VR_GetVrTime },
	/* chara ＶＲクリア */
		{ 0x00818B2C, NewVRClear_Scn },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command プラグイン自動ジャンプ */
		{ 0x008FF6AD, NewPluginAutoJump },
	/* command Ｃ４管理初期化 */
		{ 0x0090342E, C4MAN_Init },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command ＶＲタイマーポーズ */
		{ 0x00923E25, VR_TimerPause },
	/* command ゲット敵兵状態拡張版 */
		{ 0x00924573, COM_GetEnemyStSt },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara ＶＲスクリーン */
		{ 0x00A58F8A, NewVRScreen_Scn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 電撃床 */
		{ 0x00AF2208, NewElectricFloor },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command セットクリアリング */
		{ 0x00B1E8D4, COM_SetClearingData },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ブラー */
		{ 0x00B63A33, NewBlur },
	/* command イントルード主観カメラ補正 */
		{ 0x00B6E522, PL_COM_IntrudeSubjectCameraPositionMove },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara サブ画面 */
		{ 0x00C210E3, NewTs_SubWindow_S },
	/* command ＶＲクリア＿結果の取得 */
		{ 0x00C5994D, VRCLR_GetResult },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* command 装備ダンボール破壊 */
		{ 0x00C76D6E, PL_COM_DestroyBox },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 天井君 */
		{ 0x00CBB124, NewCeiling },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara アタッカー */
		{ 0x00CC9A07, NewSigAttacker },
	/* command アイテム収得可能範囲設定 */
		{ 0x00CEC17C, OK_ResetGetBoundary },
	/* chara ＶＲゴール */
		{ 0x00CED375, NewVRGoal },
	/* command 跳弾ＳＥのみ */
		{ 0x00D10066, NewEntrySparkOnlySE },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command セットタイマーステータス */
		{ 0x00D99A89, NewSetTimerStatus },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* chara ドアパネル電撃 */
		{ 0x00E2ED79, NewDoorPannelSpark },
	/* command テイルズステージセット */
		{ 0x00E3C577, SetSnakeTalesStatus },
	/* command ゲット敵兵座標 */
		{ 0x00E3EB1A, COM_GetEnemyPos },
	/* command ゲットゲームステータス */
		{ 0x00E4C507, GM_GetGameStatusForScn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command ＶＲステージポーズ */
		{ 0x00EFFDAC, VR_StagePause },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* chara 速度可変 */
		{ 0x00F26728, NewSlowDown },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara 警備兵 */
		{ 0x00F7F777, NewWatcher },
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* command プレイヤー消去キャラ */
		{ 0x00F92B8E, NewInvisibleChara },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* chara ターゲットプロック */
		{ 0x00FDDA41, NewTargetProc },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
	/* EXTERN NewFogSet_Demo */
		{ 0x01000000, NewFogSet_Demo_0000Launch },
	/* EXTERN DM_ChangeAmbient */
		{ 0x01000006, DM_ChangeAmbient_0006Launch },
	/* EXTERN DM_ChangeParallel */
		{ 0x01000007, DM_ChangeParallel_0007Launch },
	/* EXTERN NewFadeInOutForce_Demo */
		{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewRainFogPersFast */
		{ 0x01000019, NewRainFogPersFast_0019Launch },
	/* EXTERN NewPlasmaPoly_Demo */
		{ 0x01006002, NewPlasmaPoly_Demo_6002Launch },
	/* EXTERN NewBombEffect */
		{ 0x01006009, NewBombEffect_6009Launch },
	/* EXTERN NewTs_Spark */
		{ 0x0100600E, NewTs_Spark_600eLaunch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
	/* EXTERN DM_ExecProc */
		{ 0x010FFF01, DM_ExecProc_fff01Launch },
	/* EXTERN NewDemoFrameCountCall */
		{ 0x010FFF03, NewDemoFrameCountCall_fff03Launch },
	/* EXTERN NewDemoEffectInitialize */
		{ 0x010FFF05, NewDemoEffectInitialize_fff05Launch },
/* gcl chara end */
	{ 0, NULL }
};
