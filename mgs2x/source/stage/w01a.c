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
extern NEWCHARA NewGclAbs;
extern NEWCHARA NewGclCos;
extern NEWCHARA NewGclSin;
extern NEWCHARA NewStreamIsPlay;
extern NEWCHARA NewCamera;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewEntrySteamAttackTargetSmall;
extern NEWCHARA NewEntrySteamAttackTargetLarge;
extern NEWCHARA NewEntrySteamAttackTargetMiddle;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerRecoverCold;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewCom_BRK_SPLGT_LightMessage;
extern NEWCHARA NewPluginStance;
extern NEWCHARA GM_COM_ChangeMapConnectStatus;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewSIG_SetDogTagFlag;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoor;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewWindowRain;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewSetCheckSquareOfCamera;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewSetChanlTargetMap;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewSetLockerMotion;
extern NEWCHARA NewPutObject;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewLockerDaemon;
extern NEWCHARA GM_SeSetVolPanGcl;
extern NEWCHARA NewLocker;
extern NEWCHARA NewSetCameraProc;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewLockerStatus;
extern NEWCHARA NewExtinguisher;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSetPlayerLockerMotion;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewLocalWind;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewZman;
extern NEWCHARA NewWatertightDoorManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewWatertightDoorStatus;
extern NEWCHARA COM_SetClearingData;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewFlyingWarmScn;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewTs_SubWindow_S;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewPuddle;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA NewLoDControl;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewCommander;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA New2DSprite_Pause;
extern NEWCHARA COM_GetEnemyPos;
extern NEWCHARA GM_GetGameStatusForScn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewForceActCancel;
extern NEWCHARA NewWatertightDoor;
extern NEWCHARA NewSteamOnCameraScenario;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA COM_ClearingCancel;
extern NEWCHARA NewMallocCameraSquareWork;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 128 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command abs */
		{ 0x000190B3, NewGclAbs },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command StreamIsPlay */
		{ 0x00020BD1, NewStreamIsPlay },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
	/* command 跳弾水蒸気小 */
		{ 0x0009C02F, NewEntrySteamAttackTargetSmall },
	/* command 跳弾水蒸気大 */
		{ 0x0009C0E8, NewEntrySteamAttackTargetLarge },
	/* command 跳弾水蒸気中 */
		{ 0x0009C107, NewEntrySteamAttackTargetMiddle },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* chara 遠景ぼかし */
		{ 0x002DAD24, NewFarFocusEffectSet },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* chara プットスポットライトオブジェ */
		{ 0x00365DEE, NewPutSpotLightObject },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command プレイヤー風邪治し */
		{ 0x003D3B06, NewPlayerRecoverCold },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command プットスポットライトオブジェライト管理 */
		{ 0x004508C2, NewCom_BRK_SPLGT_LightMessage },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 即時マップ接合変更 */
		{ 0x0046B814, GM_COM_ChangeMapConnectStatus },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command 無線状態 */
		{ 0x00497BE6, CodecStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* chara キャラ付随水飛抹 */
		{ 0x004BAB34, NewCharaSpreadScn },
	/* command ドッグタグフラグセット */
		{ 0x004BF8E6, NewSIG_SetDogTagFlag },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* chara マップ接合 */
		{ 0x0056E234, NewMapConnect },
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
	/* command プレイヤー風邪引かせ */
		{ 0x005BFE1D, NewPlayerSetCold },
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 窓雨 */
		{ 0x006442CB, NewWindowRain },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* command カメラチェック面登録 */
		{ 0x00661BEE, NewSetCheckSquareOfCamera },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* command カメラマップ設定 */
		{ 0x006F5B0F, NewSetChanlTargetMap },
	/* command 強制モーション終了プロック */
		{ 0x00709DF6, NewForceActEndProc },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* chara プットモーションモデル */
		{ 0x0072F23C, NewSigPutModelEvm },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* command ロッカーモーション */
		{ 0x0077FFC8, NewSetLockerMotion },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command 無線メモリー設定 */
		{ 0x007B35E0, CodecMemCallSet },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* chara ロッカー管理 */
		{ 0x0085E8F4, NewLockerDaemon },
	/* command ＳＥセットＶＰ */
		{ 0x0088CBF1, GM_SeSetVolPanGcl },
	/* chara ロッカー */
		{ 0x008AA572, NewLocker },
	/* command カメラ撮影時プロック */
		{ 0x008B12D1, NewSetCameraProc },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command ロッカー状態 */
		{ 0x008B976D, NewLockerStatus },
	/* chara 消火器 */
		{ 0x008C58F7, NewExtinguisher },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command プレイヤーロッカーモーション */
		{ 0x009049ED, NewSetPlayerLockerMotion },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara ローカル風 */
		{ 0x00987E81, NewLocalWind },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 水密ドア管理 */
		{ 0x00ABFD5F, NewWatertightDoorManager },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 水密ドア状態 */
		{ 0x00B1ABD8, NewWatertightDoorStatus },
	/* command セットクリアリング */
		{ 0x00B1E8D4, COM_SetClearingData },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara 電灯虫 */
		{ 0x00B66AE4, NewFlyingWarmScn },
	/* chara 近景ぼかし */
		{ 0x00BBAD24, NewNearFocusEffectSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara サブ画面 */
		{ 0x00C210E3, NewTs_SubWindow_S },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 水溜り管理 */
		{ 0x00C8DBCC, NewPuddle },
	/* chara 天井君 */
		{ 0x00CBB124, NewCeiling },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara アタッカー */
		{ 0x00CC9A07, NewSigAttacker },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* chara ２Ｄスプライト表示＿ポーズ */
		{ 0x00E39C0D, New2DSprite_Pause },
	/* command ゲット敵兵座標 */
		{ 0x00E3EB1A, COM_GetEnemyPos },
	/* command ゲットゲームステータス */
		{ 0x00E4C507, GM_GetGameStatusForScn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command 強制モーションキャンセル */
		{ 0x00EAD648, NewForceActCancel },
	/* chara 水密ドア */
		{ 0x00EBEC24, NewWatertightDoor },
	/* chara カメラ前曇り */
		{ 0x00ED4678, NewSteamOnCameraScenario },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command クリアリングキャンセル */
		{ 0x00F005A3, COM_ClearingCancel },
	/* command カメラチェック面ワーク確保 */
		{ 0x00F15E47, NewMallocCameraSquareWork },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara 警備兵 */
		{ 0x00F7F777, NewWatcher },
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
