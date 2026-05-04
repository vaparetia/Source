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
extern NEWCHARA NewGclVarClear;
extern NEWCHARA VRCLR_SetStars;
extern NEWCHARA GM_Scn_SetRouteOffset;
extern NEWCHARA NewCamera;
extern NEWCHARA VRCLR_SetScore;
extern NEWCHARA NewEntrySteamAttackTargetSmall;
extern NEWCHARA NewEntrySteamAttackTargetMiddle;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewVRMap3D_Scn;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA PL_COM_BladeKillPlugin;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA PL_COM_ChangePrevWeaponScn;
extern NEWCHARA NewVRSys;
extern NEWCHARA NewCheckBoxHidden;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewVRWindow_Scn;
extern NEWCHARA VR_StageStart;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewGeneralPoly_Scn;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewElevator;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA COM_SetPicIdBreakComputerMonitor;
extern NEWCHARA NewPlayerAp;
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
extern NEWCHARA SK_ScnBombReset;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewSetCheckSquareOfCamera;
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
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewDelay;
extern NEWCHARA NewBombArea;
extern NEWCHARA NewPluginElevator;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewSetLockerMotion;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewVRClear_Scn;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewLockerDaemon;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewLocker;
extern NEWCHARA NewSetCameraProc;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewLockerStatus;
extern NEWCHARA NewSetPlayerLockerMotion;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA VR_PhotoClear;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA PL_GetDGCamSubject;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA PL_COM_ChangePrevItemScn;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA Msn_GetContinueCount;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewBreakComputerW24d;
extern NEWCHARA NewNodeDaemon;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA NewVRGoal;
extern NEWCHARA Msn_GetRetryCount;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewSetTimerStatus;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA NewCommander;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA VR_StagePause;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA NewMallocCameraSquareWork;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewParrot2;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewInvisibleChara;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 154 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command ＶＲクリア＿星の数の設定 */
		{ 0x0005B165, VRCLR_SetStars },
	/* command セットルートオフセット */
		{ 0x0005D91D, GM_Scn_SetRouteOffset },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＶＲクリア＿スコアの設定 */
		{ 0x0009BF85, VRCLR_SetScore },
	/* command 跳弾水蒸気小 */
		{ 0x0009C02F, NewEntrySteamAttackTargetSmall },
	/* command 跳弾水蒸気中 */
		{ 0x0009C107, NewEntrySteamAttackTargetMiddle },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara ＶＲ全体マップ表示３Ｄ */
		{ 0x000C3D82, NewVRMap3D_Scn },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* command プラグインブレード解除 */
		{ 0x00124BF9, PL_COM_BladeKillPlugin },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command シナリオ前武器変更 */
		{ 0x001D32ED, PL_COM_ChangePrevWeaponScn },
	/* chara ＶＲシステム */
		{ 0x001D5983, NewVRSys },
	/* chara ダンボール柄トラップ設定 */
		{ 0x00209027, NewCheckBoxHidden },
	/* command プレイヤー描画モードセット */
		{ 0x00231FA7, PL_COM_SetObjectFlag },
	/* chara ＶＲウィンドウ */
		{ 0x0024C03E, NewVRWindow_Scn },
	/* command ＶＲステージスタート */
		{ 0x0024C830, VR_StageStart },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* chara 汎用アニメポリゴン */
		{ 0x002A088B, NewGeneralPoly_Scn },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara エレベータ */
		{ 0x0030E9CC, NewElevator },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command 壊れコンピュータＩＤ入れ換え */
		{ 0x00363BE8, COM_SetPicIdBreakComputerMonitor },
	/* chara プレイヤ登場 */
		{ 0x003780F6, NewPlayerAp },
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
	/* command 全体マップ爆弾配置解除 */
		{ 0x0050396F, SK_ScnBombReset },
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
	/* command カメラチェック面登録 */
		{ 0x00661BEE, NewSetCheckSquareOfCamera },
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
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* command 主観移動セット */
		{ 0x006ECC1A, PL_COM_SetSubjectMove },
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
	/* command ロッカーモーション */
		{ 0x0077FFC8, NewSetLockerMotion },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command ＶＲゲットタイム */
		{ 0x0080FB82, VR_GetVrTime },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara ＶＲクリア */
		{ 0x00818B2C, NewVRClear_Scn },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* chara ロッカー管理 */
		{ 0x0085E8F4, NewLockerDaemon },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* chara ロッカー */
		{ 0x008AA572, NewLocker },
	/* command カメラ撮影時プロック */
		{ 0x008B12D1, NewSetCameraProc },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command ロッカー状態 */
		{ 0x008B976D, NewLockerStatus },
	/* command プレイヤーロッカーモーション */
		{ 0x009049ED, NewSetPlayerLockerMotion },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command ＶＲポーズ中クリア */
		{ 0x0093EEEA, VR_PhotoClear },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* command ゲットデジカメ被写体情報 */
		{ 0x009837C4, PL_GetDGCamSubject },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* command シナリオ前装備変更 */
		{ 0x009CF455, PL_COM_ChangePrevItemScn },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* command ミッションズコンティニュー回数取得 */
		{ 0x009E5A92, Msn_GetContinueCount },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara ＶＲスクリーン */
		{ 0x00A58F8A, NewVRScreen_Scn },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* chara 壊れコンピュータW24D */
		{ 0x00A97EB2, NewBreakComputerW24d },
	/* chara ノードシステム */
		{ 0x00AB381C, NewNodeDaemon },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
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
	/* command ミッションズリトライ回数取得 */
		{ 0x00D0E799, Msn_GetRetryCount },
	/* command 跳弾ＳＥのみ */
		{ 0x00D10066, NewEntrySparkOnlySE },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command セットタイマーステータス */
		{ 0x00D99A89, NewSetTimerStatus },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command テイルズステージセット */
		{ 0x00E3C577, SetSnakeTalesStatus },
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
	/* command カメラチェック面ワーク確保 */
		{ 0x00F15E47, NewMallocCameraSquareWork },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara オウム */
		{ 0x00F52ED4, NewParrot2 },
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
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
