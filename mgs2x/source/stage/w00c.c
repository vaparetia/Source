/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewRain;
extern NEWCHARA NewManhatLight_L1;
extern NEWCHARA NewManhatLight_L2;
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewGclAbs;
extern NEWCHARA NewGclCos;
extern NEWCHARA NewLineSplash;
extern NEWCHARA NewGclSin;
extern NEWCHARA NewStreamIsPlay;
extern NEWCHARA NewCamera;
extern NEWCHARA NewCameraOscillation;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewShadowDropHazard;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewCom_DestroyLight;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA NewManhattan3D;
extern NEWCHARA New_ChimnySmoke;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA GM_GetPosForGCL;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerRecoverCold;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPutWorldTexFadeCamera;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewPutAttachments_Scn;
extern NEWCHARA NewCodec;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewSIG_SetDogTagFlag;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewCircleLightSpot;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA NewWindNoise;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewSky_Prev;
extern NEWCHARA NewBgmFader;
extern NEWCHARA NewCorpGcl;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewSkyColumn;
extern NEWCHARA NewRipple;
extern NEWCHARA NewSetSubjectShotLen;
extern NEWCHARA NewPutWorldTex;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewRainCamera;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutWorldTexFadeCameraForDoor;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewMallocCameraCharaWork;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewRotateY_Object;
extern NEWCHARA GM_SeSetVolPanGcl;
extern NEWCHARA NewThund_Flash;
extern NEWCHARA NewSetCameraProc;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel2;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewZman;
extern NEWCHARA NewPluginLadder;
extern NEWCHARA NewManhatLight;
extern NEWCHARA NewPutVanimeObject;
extern NEWCHARA NewWatertightDoorManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewWatertightDoorStatus;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewPutSearchLightObject;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBlur;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewBrooklyn_R2;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA NewWeaponSplash;
extern NEWCHARA NewPutWorldTexFadeWorld;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewBrooklyn3D;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewCommander;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA New2DSprite_Pause;
extern NEWCHARA COM_GetEnemyPos;
extern NEWCHARA NewPutSeSound;
extern NEWCHARA NewDeleteCheckCharaOfCamera;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewWatertightDoor;
extern NEWCHARA NewSetCheckCharaOfCamera;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSetupLadder;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 143 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* chara 雨 */
		{ 0x000016CB, NewRain },
	/* chara マンハット赤灯1 */
		{ 0x000041E7, NewManhatLight_L1 },
	/* chara マンハット赤灯2 */
		{ 0x000041E8, NewManhatLight_L2 },
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command abs */
		{ 0x000190B3, NewGclAbs },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* chara ライン水飛沫 */
		{ 0x0001A6DD, NewLineSplash },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command StreamIsPlay */
		{ 0x00020BD1, NewStreamIsPlay },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* chara カメラ上下振動 */
		{ 0x0006A23B, NewCameraOscillation },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command 影投影ハザード */
		{ 0x000DA8BF, NewShadowDropHazard },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 光源消し */
		{ 0x00182073, NewCom_DestroyLight },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara 静止スポットライト設置 */
		{ 0x001B5854, NewPutStaticSpotLight },
	/* chara マンハッタン */
		{ 0x001E44B7, NewManhattan3D },
	/* chara 煙突煙 */
		{ 0x00264C7D, New_ChimnySmoke },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* command コントロール座標取得 */
		{ 0x002D77D4, GM_GetPosForGCL },
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
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara プットテクスチャクロスフェードカメラ */
		{ 0x004847D2, NewPutWorldTexFadeCamera },
	/* command 無線状態 */
		{ 0x00497BE6, CodecStatus },
	/* chara プットアタッチコントロール */
		{ 0x0049D6F1, NewPutAttachments_Scn },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* chara キャラ付随水飛抹 */
		{ 0x004BAB34, NewCharaSpreadScn },
	/* command ドッグタグフラグセット */
		{ 0x004BF8E6, NewSIG_SetDogTagFlag },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara 固定ボリュームライト */
		{ 0x0054ECA9, NewCircleLightSpot },
	/* command エルード移動範囲設定 */
		{ 0x005686AE, NewSetEludeRange },
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
	/* chara ＢＧＭ風音 */
		{ 0x005FBE23, NewWindNoise },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 新空 */
		{ 0x006273B5, NewSky_Prev },
	/* chara ＢＧＭフェーダー */
		{ 0x0062B1FD, NewBgmFader },
	/* chara 死体君 */
		{ 0x00645113, NewCorpGcl },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 天球 */
		{ 0x00657385, NewSkyColumn },
	/* chara 波紋 */
		{ 0x0066BA66, NewRipple },
	/* command 主観射程距離設定 */
		{ 0x0066F439, NewSetSubjectShotLen },
	/* chara プットテクスチャ */
		{ 0x006779AD, NewPutWorldTex },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara カメラ雨 */
		{ 0x006B8EE3, NewRainCamera },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットテクスチャクロスフェードカメラドア */
		{ 0x007A3DBF, NewPutWorldTexFadeCameraForDoor },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command カメラチェックキャラワーク確保 */
		{ 0x007A94A9, NewMallocCameraCharaWork },
	/* command 無線メモリー設定 */
		{ 0x007B35E0, CodecMemCallSet },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* chara くるくる物体 */
		{ 0x008862F6, NewRotateY_Object },
	/* command ＳＥセットＶＰ */
		{ 0x0088CBF1, GM_SeSetVolPanGcl },
	/* chara 新雷フラッシュ */
		{ 0x0089AF5E, NewThund_Flash },
	/* command カメラ撮影時プロック */
		{ 0x008B12D1, NewSetCameraProc },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command パッドチェック */
		{ 0x008B6086, NewPadCheck },
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
	/* chara ロープモデル２ */
		{ 0x009BC66F, NewRopeModel2 },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command プラグインはしご */
		{ 0x00A741FD, NewPluginLadder },
	/* chara マンハット黄灯 */
		{ 0x00A8560D, NewManhatLight },
	/* chara プット頂点アニメオブジェ */
		{ 0x00A895C4, NewPutVanimeObject },
	/* chara 水密ドア管理 */
		{ 0x00ABFD5F, NewWatertightDoorManager },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 水密ドア状態 */
		{ 0x00B1ABD8, NewWatertightDoorStatus },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* chara プット投光器オブジェ */
		{ 0x00B431C3, NewPutSearchLightObject },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ブラー */
		{ 0x00B63A33, NewBlur },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara ブルックリン赤灯 */
		{ 0x00C0E06D, NewBrooklyn_R2 },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara アタッカー */
		{ 0x00CC9A07, NewSigAttacker },
	/* chara 武器水飛沫 */
		{ 0x00CDC7BD, NewWeaponSplash },
	/* chara プットテクスチャクロスフェードワールド */
		{ 0x00CF777F, NewPutWorldTexFadeWorld },
	/* command 跳弾ＳＥのみ */
		{ 0x00D10066, NewEntrySparkOnlySE },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* chara ブルックリン */
		{ 0x00D6ED95, NewBrooklyn3D },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
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
	/* chara プットＳＥ鳴らし */
		{ 0x00E48F2F, NewPutSeSound },
	/* command カメラチェックキャラ削除 */
		{ 0x00E73AC4, NewDeleteCheckCharaOfCamera },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara 水密ドア */
		{ 0x00EBEC24, NewWatertightDoor },
	/* command カメラチェックキャラ登録 */
		{ 0x00EC40C7, NewSetCheckCharaOfCamera },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command はしご設定 */
		{ 0x00F7E492, NewSetupLadder },
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
