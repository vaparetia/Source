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
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewCamera;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewShadowDropHazard;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewStreamStatus;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA NewVRSys;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA PL_COM_Sneeze;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewGclAtan;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewStreamSetPan;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewGetLeftTime;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewKbdExec;
extern NEWCHARA NewSpotLightImageSet;
extern NEWCHARA SetVRStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewSIG_SetDogTagFlag;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoor;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewHzxResetGroupAdd;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewGclAssert;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA SetVrStageId;
extern NEWCHARA SetVRBOMBS;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewStreamSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewTimer;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA NewZman;
extern NEWCHARA NewPluginLadder;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewHoldEnemy;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewTs_SubWindow_S;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewSetTimerStatus;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA NewCommander;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA New2DSprite_Pause;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA GM_GetGameStatusForScn;
extern NEWCHARA NewProjectorLight;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewForceActCancel;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSetupLadder;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewInvisibleChara;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
/* 131 functions */
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
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command 影投影ハザード */
		{ 0x000DA8BF, NewShadowDropHazard },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* command StreamStatus */
		{ 0x001AAE5F, NewStreamStatus },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara 静止スポットライト設置 */
		{ 0x001B5854, NewPutStaticSpotLight },
	/* chara ＶＲシステム */
		{ 0x001D5983, NewVRSys },
	/* command プレイヤー描画モードセット */
		{ 0x00231FA7, PL_COM_SetObjectFlag },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command プレイヤーくしゃみ発動 */
		{ 0x0027E580, PL_COM_Sneeze },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* chara 遠景ぼかし */
		{ 0x002DAD24, NewFarFocusEffectSet },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command atan */
		{ 0x00325C8E, NewGclAtan },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* command StreamSetPan */
		{ 0x00339BD9, NewStreamSetPan },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command プレイヤーモーション振動ＯＦＦ */
		{ 0x003F46EC, NewPlayerMotionVibOff },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command ゲット残りタイム */
		{ 0x00450C64, NewGetLeftTime },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara USBキーボード実行 */
		{ 0x0047D37A, NewKbdExec },
	/* chara スポットライト背景イメージ指定 */
		{ 0x0048F8F8, NewSpotLightImageSet },
	/* command セットＶＲステータス */
		{ 0x004909B5, SetVRStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command ドッグタグフラグセット */
		{ 0x004BF8E6, NewSIG_SetDogTagFlag },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command はだかモードセット */
		{ 0x004F576C, PL_COM_SetNudeMode },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command エルード移動範囲設定 */
		{ 0x005686AE, NewSetEludeRange },
	/* command プラグインブレード */
		{ 0x0056B5CB, NewPluginBlade },
	/* chara マップ接合 */
		{ 0x0056E234, NewMapConnect },
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
	/* command プレイヤー風邪引かせ */
		{ 0x005BFE1D, NewPlayerSetCold },
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* command ゲームオーバー処理終了 */
		{ 0x006856C1, GM_GameOverEndScn },
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
	/* command 主観移動セット */
		{ 0x006ECC1A, PL_COM_SetSubjectMove },
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
	/* command ＶＲゲットタイム */
		{ 0x0080FB82, VR_GetVrTime },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command パッドチェック */
		{ 0x008B6086, NewPadCheck },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command プラグイン自動ジャンプ */
		{ 0x008FF6AD, NewPluginAutoJump },
	/* chara スポットライト投影モデル */
		{ 0x009307AB, NewSpotDrawObjectSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara タイマー */
		{ 0x009B65F0, NewTimer },
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
	/* command プラグインはしご */
		{ 0x00A741FD, NewPluginLadder },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* chara 船倉兵 */
		{ 0x00B3E388, NewHoldEnemy },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
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
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
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
	/* chara ２Ｄスプライト表示＿ポーズ */
		{ 0x00E39C0D, New2DSprite_Pause },
	/* command テイルズステージセット */
		{ 0x00E3C577, SetSnakeTalesStatus },
	/* command ゲットゲームステータス */
		{ 0x00E4C507, GM_GetGameStatusForScn },
	/* chara プロジェクタライト */
		{ 0x00E4D016, NewProjectorLight },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command 強制モーションキャンセル */
		{ 0x00EAD648, NewForceActCancel },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command はしご設定 */
		{ 0x00F7E492, NewSetupLadder },
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* command プレイヤー消去キャラ */
		{ 0x00F92B8E, NewInvisibleChara },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
/* gcl chara end */
	{ 0, NULL }
};
