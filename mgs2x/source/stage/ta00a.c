/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRain;
extern NEWCHARA NewManhatLight_L1;
extern NEWCHARA NewManhatLight_L2;
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewGclAbs;
extern NEWCHARA NewGclCos;
extern NEWCHARA NewLineSplash;
extern NEWCHARA NewGclSin;
extern NEWCHARA NewGclVarClear;
extern NEWCHARA VRCLR_SetStars;
extern NEWCHARA GM_Scn_SetRouteOffset;
extern NEWCHARA NewCamera;
extern NEWCHARA NewCameraOscillation;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA VRCLR_SetScore;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewVRMap3D_Scn;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA SetVR_BOMBS_NUM;
extern NEWCHARA PL_COM_BladeKillPlugin;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewCom_DestroyLight;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA NewVRSys;
extern NEWCHARA NewManhattan3D;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewVRWindow_Scn;
extern NEWCHARA VR_StageStart;
extern NEWCHARA New_ChimnySmoke;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA VR_TimerStart;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewGclAtan;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewSetMap;
extern NEWCHARA COMVRGameOverProc;
extern NEWCHARA NewPlayerAp;
extern NEWCHARA NewSetKaitaiC4;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerRecoverCold;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPutWorldTexFadeCamera;
extern NEWCHARA SetVRStatus;
extern NEWCHARA NewSigRouteModel;
extern NEWCHARA NewCodec;
extern NEWCHARA NewStormyRiverWaveSet;
extern NEWCHARA NewSplashFall_Scn;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewCircleLightSpot;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewWcCommander;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA NewWindNoise;
extern NEWCHARA NewGclDiffDirABS;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewSky_Prev;
extern NEWCHARA NewBgmFader;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewSkyColumn;
extern NEWCHARA NewOceanWave;
extern NEWCHARA NewRipple;
extern NEWCHARA NewPutWorldTex;
extern NEWCHARA NewBombEffectScn;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewRainFogPers;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewGclAssert;
extern NEWCHARA NewRainCamera;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA SetVrStageId;
extern NEWCHARA SetVRBOMBS;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewBombArea;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutWorldTexFadeCameraForDoor;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewMallocCameraCharaWork;
extern NEWCHARA NewStreamSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewVRClear_Scn;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA NewDemoDoll;
extern NEWCHARA NewRotateY_Object;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewThund_Flash;
extern NEWCHARA NewSetCameraProc;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA VR_TimerPause;
extern NEWCHARA VR_PhotoClear;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA PL_GetDGCamSubject;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel2;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA PL_COM_ChangePrevItemScn;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA COM_GetCaptureModelName;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA NewZman;
extern NEWCHARA NewManhatLight;
extern NEWCHARA NewPutVanimeObject;
extern NEWCHARA NewWatertightDoorManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewWatertightDoorStatus;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBlur;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewSplashFloor_Scn;
extern NEWCHARA NewBrooklyn_R2;
extern NEWCHARA VRCLR_GetResult;
extern NEWCHARA VRMap3D_ResetBomb;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA NewWeaponSplash;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA NewVRGoal;
extern NEWCHARA NewPutWorldTexFadeWorld;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewBrooklyn3D;
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
extern NEWCHARA COM_GetEnemyPos;
extern NEWCHARA GM_GetGameStatusForScn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewWatertightDoor;
extern NEWCHARA NewSetCheckCharaOfCamera;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA VR_StagePause;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewWatcher;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewInvisibleChara;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewModelPatAnimation;
extern NEWCHARA NewExecProcContinual;
/* 192 functions */
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
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command ＶＲクリア＿星の数の設定 */
		{ 0x0005B165, VRCLR_SetStars },
	/* command セットルートオフセット */
		{ 0x0005D91D, GM_Scn_SetRouteOffset },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* chara カメラ上下振動 */
		{ 0x0006A23B, NewCameraOscillation },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
	/* command ＶＲクリア＿スコアの設定 */
		{ 0x0009BF85, VRCLR_SetScore },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara ＶＲ全体マップ表示３Ｄ */
		{ 0x000C3D82, NewVRMap3D_Scn },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* command セット解体済み爆弾数 */
		{ 0x000FF733, SetVR_BOMBS_NUM },
	/* command プラグインブレード解除 */
		{ 0x00124BF9, PL_COM_BladeKillPlugin },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 光源消し */
		{ 0x00182073, NewCom_DestroyLight },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara ＶＲシステム */
		{ 0x001D5983, NewVRSys },
	/* chara マンハッタン */
		{ 0x001E44B7, NewManhattan3D },
	/* command プレイヤー描画モードセット */
		{ 0x00231FA7, PL_COM_SetObjectFlag },
	/* chara ＶＲウィンドウ */
		{ 0x0024C03E, NewVRWindow_Scn },
	/* command ＶＲステージスタート */
		{ 0x0024C830, VR_StageStart },
	/* chara 煙突煙 */
		{ 0x00264C7D, New_ChimnySmoke },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command ＶＲタイマースタート */
		{ 0x0026AAB9, VR_TimerStart },
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
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
   /* chara プットスポットライトオブジェ */
      { 0x00365DEE, NewPutSpotLightObject }, // BP <- Added in manually!
	/* command 特殊ＶＲゲームオーバー */
		{ 0x0037700F, COMVRGameOverProc },
	/* chara プレイヤ登場 */
		{ 0x003780F6, NewPlayerAp },
	/* chara 解体Ｃ４ */
		{ 0x003A6F25, NewSetKaitaiC4 },
	/* command rand */
		{ 0x003A9224, NewRand },
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
	/* chara プットテクスチャクロスフェードカメラ */
		{ 0x004847D2, NewPutWorldTexFadeCamera },
	/* command セットＶＲステータス */
		{ 0x004909B5, SetVRStatus },
	/* chara ルートデモ人形 */
		{ 0x00492A40, NewSigRouteModel },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* chara 嵐の川波 */
		{ 0x004A777D, NewStormyRiverWaveSet },
	/* chara 落下水飛沫 */
		{ 0x004BAAFE, NewSplashFall_Scn },
	/* chara キャラ付随水飛抹 */
		{ 0x004BAB34, NewCharaSpreadScn },
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
	/* chara 固定ボリュームライト */
		{ 0x0054ECA9, NewCircleLightSpot },
	/* command エルード移動範囲設定 */
		{ 0x005686AE, NewSetEludeRange },
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
	/* command プレイヤー風邪引かせ */
		{ 0x005BFE1D, NewPlayerSetCold },
	/* chara ＢＧＭ風音 */
		{ 0x005FBE23, NewWindNoise },
	/* command 絶対角度差 */
		{ 0x00604185, NewGclDiffDirABS },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 新空 */
		{ 0x006273B5, NewSky_Prev },
	/* chara ＢＧＭフェーダー */
		{ 0x0062B1FD, NewBgmFader },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 天球 */
		{ 0x00657385, NewSkyColumn },
	/* chara 波面 */
		{ 0x0066BA4C, NewOceanWave },
	/* chara 波紋 */
		{ 0x0066BA66, NewRipple },
	/* chara プットテクスチャ */
		{ 0x006779AD, NewPutWorldTex },
	/* chara 爆発 */
		{ 0x006781AF, NewBombEffectScn },
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
	/* chara 爆弾検知領域 */
		{ 0x0076A03C, NewBombArea },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットテクスチャクロスフェードカメラドア */
		{ 0x007A3DBF, NewPutWorldTexFadeCameraForDoor },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command カメラチェックキャラワーク確保 */
		{ 0x007A94A9, NewMallocCameraCharaWork },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
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
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* chara デモ人形 */
		{ 0x008826B9, NewDemoDoll },
	/* chara くるくる物体 */
		{ 0x008862F6, NewRotateY_Object },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* chara 新雷フラッシュ */
		{ 0x0089AF5E, NewThund_Flash },
	/* command カメラ撮影時プロック */
		{ 0x008B12D1, NewSetCameraProc },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command プラグイン自動ジャンプ */
		{ 0x008FF6AD, NewPluginAutoJump },
	/* command ＶＲタイマーポーズ */
		{ 0x00923E25, VR_TimerPause },
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
	/* chara ロープモデル２ */
		{ 0x009BC66F, NewRopeModel2 },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* command シナリオ前装備変更 */
		{ 0x009CF455, PL_COM_ChangePrevItemScn },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* command ゲット捕まえられ兵の名前 */
		{ 0x009E63C2, COM_GetCaptureModelName },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara ＶＲスクリーン */
		{ 0x00A58F8A, NewVRScreen_Scn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
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
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ブラー */
		{ 0x00B63A33, NewBlur },
	/* chara 近景ぼかし */
		{ 0x00BBAD24, NewNearFocusEffectSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* chara 床水飛沫 */
		{ 0x00BE0863, NewSplashFloor_Scn },
	/* chara ブルックリン赤灯 */
		{ 0x00C0E06D, NewBrooklyn_R2 },
	/* command ＶＲクリア＿結果の取得 */
		{ 0x00C5994D, VRCLR_GetResult },
	/* command ＶＲ全体マップ表示３Ｄ＿爆弾解除 */
		{ 0x00C64ECB, VRMap3D_ResetBomb },
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
	/* command アイテム収得可能範囲設定 */
		{ 0x00CEC17C, OK_ResetGetBoundary },
	/* chara ＶＲゴール */
		{ 0x00CED375, NewVRGoal },
	/* chara プットテクスチャクロスフェードワールド */
		{ 0x00CF777F, NewPutWorldTexFadeWorld },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* chara ブルックリン */
		{ 0x00D6ED95, NewBrooklyn3D },
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
	/* chara 水密ドア */
		{ 0x00EBEC24, NewWatertightDoor },
	/* command カメラチェックキャラ登録 */
		{ 0x00EC40C7, NewSetCheckCharaOfCamera },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command ＶＲステージポーズ */
		{ 0x00EFFDAC, VR_StagePause },
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
	/* chara モデル切り替えアニメ */
		{ 0x00FE6F50, NewModelPatAnimation },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
