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
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewShadowDrawObjectSet;
extern NEWCHARA GM_Scn_SetRouteOffset;
extern NEWCHARA NewCamera;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewVRMap3D_Scn;
extern NEWCHARA NewShadowDropHazard;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA PL_COM_BladeKillPlugin;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA PL_COM_ChangePrevWeaponScn;
extern NEWCHARA NewVRSys;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewVRWindow_Scn;
extern NEWCHARA VR_StageStart;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewPadVib4;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewDM_FrameCheck;
extern NEWCHARA NewMap3D;
extern NEWCHARA GM_GetPosForGCL;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPlayerAp;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA SetVRStatus;
extern NEWCHARA NewSigRouteModel;
extern NEWCHARA NewCodec;
extern NEWCHARA GLLPhotoCheck_Scn;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA NewWindNoise;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewSeaSurfaceSet;
extern NEWCHARA NewGclAssert;
extern NEWCHARA NewRainCamera;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA SetVrStageId;
extern NEWCHARA SetVRBOMBS;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewMallocCameraCharaWork;
extern NEWCHARA NewStreamSet;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewVRClear_Scn;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA GM_SeSetVolPanGcl;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewCheckWaterLevel;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA PIC_TankerPicSaveScn;
extern NEWCHARA PL_COM_ChangePrevItemScn;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA PL_COM_BulletSplashDaemon;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA NewZman;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA NewVRGoal;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewStreamStop;
extern NEWCHARA NewLoDControl;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewCommander;
extern NEWCHARA NewAttachment2;
extern NEWCHARA NewAttachment3;
extern NEWCHARA PL_COM_SetNoUse;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewGllPhoto;
extern NEWCHARA NewSetCheckCharaOfCamera;
extern NEWCHARA VR_StagePause;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA NewSlowDown;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicFloor;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewInvisibleChara;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 156 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* chara 影投影モデル */
		{ 0x0005C815, NewShadowDrawObjectSet },
	/* command セットルートオフセット */
		{ 0x0005D91D, GM_Scn_SetRouteOffset },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara ＶＲ全体マップ表示３Ｄ */
		{ 0x000C3D82, NewVRMap3D_Scn },
	/* command 影投影ハザード */
		{ 0x000DA8BF, NewShadowDropHazard },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
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
	/* command プレイヤー描画モードセット */
		{ 0x00231FA7, PL_COM_SetObjectFlag },
	/* chara ＶＲウィンドウ */
		{ 0x0024C03E, NewVRWindow_Scn },
	/* command ＶＲステージスタート */
		{ 0x0024C830, VR_StageStart },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* chara パッド振動制御 */
		{ 0x002948A0, NewPadVib4 },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
	/* chara ゴルルゴン振動制御 */
		{ 0x002A41C7, NewDM_FrameCheck },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* command コントロール座標取得 */
		{ 0x002D77D4, GM_GetPosForGCL },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* chara プレイヤ登場 */
		{ 0x003780F6, NewPlayerAp },
	/* command rand */
		{ 0x003A9224, NewRand },
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
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command セットＶＲステータス */
		{ 0x004909B5, SetVRStatus },
	/* chara ルートデモ人形 */
		{ 0x00492A40, NewSigRouteModel },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command ゴルルゴン写真判定補助 */
		{ 0x004A90A1, GLLPhotoCheck_Scn },
	/* chara キャラ付随水飛抹 */
		{ 0x004BAB34, NewCharaSpreadScn },
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
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* chara ＢＧＭ風音 */
		{ 0x005FBE23, NewWindNoise },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 太陽 */
		{ 0x00641A7B, NewPlantSunMain },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara フォグコントローラ */
		{ 0x0066B097, NewFogControl },
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
	/* chara プラント海面 */
		{ 0x006AE654, NewSeaSurfaceSet },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* chara カメラ雨 */
		{ 0x006B8EE3, NewRainCamera },
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
	/* chara プットモーションモデル */
		{ 0x0072F23C, NewSigPutModelEvm },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command カメラチェックキャラワーク確保 */
		{ 0x007A94A9, NewMallocCameraCharaWork },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command カメラ視界チェック */
		{ 0x007F3C88, NewViewCheckCommand },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command ＶＲゲットタイム */
		{ 0x0080FB82, VR_GetVrTime },
	/* chara ＶＲクリア */
		{ 0x00818B2C, NewVRClear_Scn },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* command ＳＥセットＶＰ */
		{ 0x0088CBF1, GM_SeSetVolPanGcl },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command パッドチェック */
		{ 0x008B6086, NewPadCheck },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* chara 水面監視水飛沫 */
		{ 0x0090B7EC, NewCheckWaterLevel },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* command デジカメ画像セーブ */
		{ 0x009C403E, PIC_TankerPicSaveScn },
	/* command シナリオ前装備変更 */
		{ 0x009CF455, PL_COM_ChangePrevItemScn },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* command 弾水飛沫処理 */
		{ 0x009E1DF3, PL_COM_BulletSplashDaemon },
	/* chara ＶＲスクリーン */
		{ 0x00A58F8A, NewVRScreen_Scn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* chara 影管理 */
		{ 0x00BD400B, NewShadowControl },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 汎用空 */
		{ 0x00C9DD51, NewSkyUtil },
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
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* command StreamStop */
		{ 0x00D30863, NewStreamStop },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* chara 装備品Ｂ */
		{ 0x00DE0401, NewAttachment2 },
	/* chara 装備品Ｃ */
		{ 0x00DE0402, NewAttachment3 },
	/* command 不可装備設定 */
		{ 0x00DE4430, PL_COM_SetNoUse },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* command テイルズステージセット */
		{ 0x00E3C577, SetSnakeTalesStatus },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara 水中ゴルルゴン */
		{ 0x00EB7EDA, NewGllPhoto },
	/* command カメラチェックキャラ登録 */
		{ 0x00EC40C7, NewSetCheckCharaOfCamera },
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
	/* chara 透明床 */
		{ 0x00F91A08, NewDynamicFloor },
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
