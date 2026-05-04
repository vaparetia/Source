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
extern NEWCHARA NewCamera;
extern NEWCHARA NewScnBreakBody;
extern NEWCHARA PL_COM_BladeKillPlugin;
extern NEWCHARA NewRaiden;
extern NEWCHARA VR_GoalIn;
extern NEWCHARA NewZakoTelop;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewTngCommander;
extern NEWCHARA NewStreamStatus;
extern NEWCHARA NewRadar;
extern NEWCHARA NewVRSys;
extern NEWCHARA PL_COM_SetObjectFlag;
extern NEWCHARA NewVRWindow_Scn;
extern NEWCHARA VR_StageStart;
extern NEWCHARA NewPluginKageshibari;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA PL_CHARA_ScnVibCamera;
extern NEWCHARA NewSigDefender;
extern NEWCHARA NewStreamSetPan;
extern NEWCHARA NewSetMap;
extern NEWCHARA COMVRGameOverProc;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewPluginFall;
extern NEWCHARA NewKbdExec;
extern NEWCHARA SetVRStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewPluginBlade;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewHzxResetGroupAdd;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewAtCommander;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA VR_ScnAddEnemy;
extern NEWCHARA NewGollgon;
extern NEWCHARA NewVRPause_Scn;
extern NEWCHARA NewStreamStart;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA PL_COM_SetForceActLoop;
extern NEWCHARA NewGclAssert;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA SetVrStageId;
extern NEWCHARA SetVRBOMBS;
extern NEWCHARA PL_COM_SetSubjectMove;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA GetGllBreakParts;
extern NEWCHARA NewDelay;
extern NEWCHARA GetGllHeadPos;
extern NEWCHARA NewGclGetRTC;
extern NEWCHARA NewVRWallMarker_Scn;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewStreamSet;
extern NEWCHARA GCL_DG_SetTmpLight2;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewVRClear_Scn;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewVRWallScarBase;
extern NEWCHARA NewVRSparkBase;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA VR_TimerPause;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewVRScreen_Scn;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewFreeThunder;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewVRSky_Scn;
extern NEWCHARA NewFadeObj;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA GM_COM_PauseOff;
extern NEWCHARA VRCLR_GetResult;
extern NEWCHARA NewDefCommander;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewSigAttacker;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA SetGameStatusVROnly;
extern NEWCHARA NewStreamStop;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewEntryVRWallScar;
extern NEWCHARA NewCommander;
extern NEWCHARA PL_COM_SetNoUse;
extern NEWCHARA NewVRFloorMarker_Scn;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA NewSigBreakPartScn;
extern NEWCHARA GM_COM_PauseOn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewVrObject;
extern NEWCHARA VR_StagePause;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewScnPadVib;
extern NEWCHARA NewScrHexFade;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewSigTengB;
extern NEWCHARA NewStageOutline_Scn;
/* 142 functions */
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
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* chara オブジェクト分解 */
		{ 0x000BD712, NewScnBreakBody },
	/* command プラグインブレード解除 */
		{ 0x00124BF9, PL_COM_BladeKillPlugin },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command ＶＲステージクリア */
		{ 0x0012B592, VR_GoalIn },
	/* chara ザコラッシュテロップ表示 */
		{ 0x0012C9D0, NewZakoTelop },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara 天狗兵コマンダー */
		{ 0x0018F412, NewTngCommander },
	/* command StreamStatus */
		{ 0x001AAE5F, NewStreamStatus },
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
	/* command プラグイン影縛り */
		{ 0x0029D24D, NewPluginKageshibari },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara シナリオカメラ振動 */
		{ 0x003124A2, PL_CHARA_ScnVibCamera },
	/* chara 長廊下兵 */
		{ 0x003259E5, NewSigDefender },
	/* command StreamSetPan */
		{ 0x00339BD9, NewStreamSetPan },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command 特殊ＶＲゲームオーバー */
		{ 0x0037700F, COMVRGameOverProc },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command プラグイン落下 */
		{ 0x00476790, NewPluginFall },
	/* chara USBキーボード実行 */
		{ 0x0047D37A, NewKbdExec },
	/* command セットＶＲステータス */
		{ 0x004909B5, SetVRStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
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
	/* command プラグインブレード */
		{ 0x0056B5CB, NewPluginBlade },
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
	/* chara 攻撃コマンダー */
		{ 0x005BB4D4, NewAtCommander },
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* command ＶＲ敵兵追加 */
		{ 0x005DE390, VR_ScnAddEnemy },
	/* chara ゴルルゴン */
		{ 0x005EC9C1, NewGollgon },
	/* chara ＶＲポーズ */
		{ 0x005ED347, NewVRPause_Scn },
	/* command StreamStart */
		{ 0x0060D52E, NewStreamStart },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 太陽 */
		{ 0x00641A7B, NewPlantSunMain },
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
	/* command 強制モーションループ回数設定 */
		{ 0x006A9667, PL_COM_SetForceActLoop },
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
	/* command 強制モーション終了プロック */
		{ 0x00709DF6, NewForceActEndProc },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* command セットＶＲ発見回数 */
		{ 0x00718756, SetVR_DISCOVERY_CNT },
	/* chara プットモーションモデル */
		{ 0x0072F23C, NewSigPutModelEvm },
	/* command ゲットゴルルゴン壊れパーツ */
		{ 0x007334AF, GetGllBreakParts },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command ゲットゴルルゴン頭座標 */
		{ 0x0075513C, GetGllHeadPos },
	/* command 現在時刻取得 */
		{ 0x0077318D, NewGclGetRTC },
	/* chara ＶＲ壁マーカー */
		{ 0x0077568D, NewVRWallMarker_Scn },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command テンプライト */
		{ 0x007DDA2E, GCL_DG_SetTmpLight2 },
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
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* chara ＶＲ弾痕 */
		{ 0x0089FEC2, NewVRWallScarBase },
	/* chara ＶＲ跳弾 */
		{ 0x008A43F9, NewVRSparkBase },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command プラグイン自動ジャンプ */
		{ 0x008FF6AD, NewPluginAutoJump },
	/* command ＶＲタイマーポーズ */
		{ 0x00923E25, VR_TimerPause },
	/* chara スポットライト投影モデル */
		{ 0x009307AB, NewSpotDrawObjectSet },
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
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara 任意稲光 */
		{ 0x00B23A64, NewFreeThunder },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ＶＲ空 */
		{ 0x00B4E108, NewVRSky_Scn },
	/* chara フェードオブジェ */
		{ 0x00B89202, NewFadeObj },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* command ポーズリセット */
		{ 0x00BD28C4, GM_COM_PauseOff },
	/* command ＶＲクリア＿結果の取得 */
		{ 0x00C5994D, VRCLR_GetResult },
	/* chara 長廊下コマンダー */
		{ 0x00C7280A, NewDefCommander },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara アタッカー */
		{ 0x00CC9A07, NewSigAttacker },
	/* command アイテム収得可能範囲設定 */
		{ 0x00CEC17C, OK_ResetGetBoundary },
	/* command ＶＲステージセット */
		{ 0x00D219B8, SetGameStatusVROnly },
	/* command StreamStop */
		{ 0x00D30863, NewStreamStop },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* command ＶＲ弾痕ノーマル */
		{ 0x00DDF457, NewEntryVRWallScar },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* command 不可装備設定 */
		{ 0x00DE4430, PL_COM_SetNoUse },
	/* chara ＶＲ床マーカー */
		{ 0x00E0568B, NewVRFloorMarker_Scn },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* command テイルズステージセット */
		{ 0x00E3C577, SetSnakeTalesStatus },
	/* chara ゴルルゴン部位壊れ */
		{ 0x00E4CEA2, NewSigBreakPartScn },
	/* command ポーズセット */
		{ 0x00E74F46, GM_COM_PauseOn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara ＶＲオブジェ */
		{ 0x00ED1E0B, NewVrObject },
	/* command ＶＲステージポーズ */
		{ 0x00EFFDAC, VR_StagePause },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara パッド振動スクリプト */
		{ 0x00F77507, NewScnPadVib },
	/* chara 六角フェード */
		{ 0x00FA4E80, NewScrHexFade },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* chara 天狗兵Ｂ */
		{ 0x00FE5C5C, NewSigTengB },
	/* chara ステージアウトライン */
		{ 0x00FFC196, NewStageOutline_Scn },
/* gcl chara end */
	{ 0, NULL }
};
