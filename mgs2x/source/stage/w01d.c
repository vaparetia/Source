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
extern NEWCHARA NewCamera;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewEntrySteamAttackTargetMiddle;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewMzTwindoor;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewBulletBarriar;
extern NEWCHARA NewPutPaperObject;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA NewCheckBoxHidden;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerRecoverCold;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewSIG_SetDogTagFlag;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewSvCamera;
extern NEWCHARA NewPutBreakObject;
extern NEWCHARA NewPutRadleObject;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoor;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewWindowRain;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBombEffectScn;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPutHangLight;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewPutPotatoObject;
extern NEWCHARA NewSetChanlTargetMap;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewSetCameraProc;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewExtinguisher;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewIrs2;
extern NEWCHARA NewZman;
extern NEWCHARA NewEnbControl;
extern NEWCHARA COM_SetClearingData;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewHako_purapura;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewTs_SubWindow_S;
extern NEWCHARA NewKillPlayer;
extern NEWCHARA NewPutBottleObject;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewSetTimerStatus;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewCommander;
extern NEWCHARA NewPutPlateObject;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA New2DSprite_Pause;
extern NEWCHARA COM_GetEnemyPos;
extern NEWCHARA GM_GetGameStatusForScn;
extern NEWCHARA COM_GetEnemyStatus;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewTRAP_C4;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewFlour;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 127 functions */
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
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
	/* command 跳弾水蒸気中 */
		{ 0x0009C107, NewEntrySteamAttackTargetMiddle },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 両開き扉 */
		{ 0x000B6ED3, NewMzTwindoor },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* chara ブレットバリア */
		{ 0x00164EE0, NewBulletBarriar },
	/* chara プット紙オブジェ */
		{ 0x00175436, NewPutPaperObject },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara ダンボール柄トラップ設定 */
		{ 0x00209027, NewCheckBoxHidden },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command プレイヤー風邪治し */
		{ 0x003D3B06, NewPlayerRecoverCold },
	/* command プレイヤーモーション振動ＯＦＦ */
		{ 0x003F46EC, NewPlayerMotionVibOff },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command 無線状態 */
		{ 0x00497BE6, CodecStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command ドッグタグフラグセット */
		{ 0x004BF8E6, NewSIG_SetDogTagFlag },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara 監視カメラ */
		{ 0x00522DB5, NewSvCamera },
	/* chara プット壊れオブジェ */
		{ 0x00532262, NewPutBreakObject },
	/* chara プット揺れオブジェ */
		{ 0x00532F32, NewPutRadleObject },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command エルード移動範囲設定 */
		{ 0x005686AE, NewSetEludeRange },
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
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 窓雨 */
		{ 0x006442CB, NewWindowRain },
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
	/* chara プット揺れライト */
		{ 0x006A0CDA, NewPutHangLight },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara プットポテトオブジェ */
		{ 0x006C577D, NewPutPotatoObject },
	/* command カメラマップ設定 */
		{ 0x006F5B0F, NewSetChanlTargetMap },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
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
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command カメラ撮影時プロック */
		{ 0x008B12D1, NewSetCameraProc },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* chara 消火器 */
		{ 0x008C58F7, NewExtinguisher },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
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
	/* chara 複数赤外線 */
		{ 0x00A58FA5, NewIrs2 },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command セットクリアリング */
		{ 0x00B1E8D4, COM_SetClearingData },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara 蓋付き段ボール */
		{ 0x00B8B94D, NewHako_purapura },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara サブ画面 */
		{ 0x00C210E3, NewTs_SubWindow_S },
	/* command プレイヤー瞬殺 */
		{ 0x00C3189A, NewKillPlayer },
	/* chara プット瓶オブジェ */
		{ 0x00C45437, NewPutBottleObject },
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
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* chara プット皿オブジェ */
		{ 0x00DF5435, NewPutPlateObject },
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
	/* command ゲット敵兵状態 */
		{ 0x00E6B658, COM_GetEnemyStatus },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara トラップ専用C4 */
		{ 0x00E9021D, NewTRAP_C4 },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara メリケン粉 */
		{ 0x00ECC9DE, NewFlour },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
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
