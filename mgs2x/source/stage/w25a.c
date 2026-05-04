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
extern NEWCHARA NewStreamIsPlay;
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewTimerEnd2;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEntrySteamAttackTargetLarge;
extern NEWCHARA NewEntrySteamAttackTargetMiddle;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewKac_CommandGetKasStat;
extern NEWCHARA NewHarItemSet;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA GM_ComMapChange;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewSK_BossPause;
extern NEWCHARA NewRadarZoomRateControl;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewSetTimerStatus2;
extern NEWCHARA NewHar_CommandGetClaster;
extern NEWCHARA SK_BossPauseVisibleInvisible;
extern NEWCHARA SK_BossTimeAfterWrite2;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewHarBGM_Manager;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA UTL_EFT_ExecCallback;
extern NEWCHARA NewGetLapTime;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA UTL_EFT_AddBoundFromGCL;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPluginSuperBlow;
extern NEWCHARA NewPluginFall;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewGetLeftTime2;
extern NEWCHARA NewPutFlagObject;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewBridgeExplosionScn;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewBossHarrier;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewHzxResetGroupAdd;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewPadDemoPlay;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA NewRemoveTrapBind;
extern NEWCHARA NewWindNoise;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewSeaSurfaceSet;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewStreamSet;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewHarKasacka;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewSK_BossResult2;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA NewCheckWaterLevel;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewLocalWind;
extern NEWCHARA GM_COM_SetBossSurvivalStatus;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewCypher;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA PL_COM_ChangePrevItemScn;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewIrs2;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA PL_COM_SightOnOff;
extern NEWCHARA NewKamomeManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA GM_COM_PauseOff;
extern NEWCHARA NewTargetTrap;
extern NEWCHARA NewKillPlayer;
extern NEWCHARA NewUnsetTimerStatus2;
extern NEWCHARA NewArraySet;
extern NEWCHARA SK_BossResultChenge2;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewBosKasacka;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewExtraUmiDraw;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewHar_CommandGetPosition;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA NewCommander;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA GM_COM_PauseOn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewTRAP_C4;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewStageFireScn;
extern NEWCHARA NewKac_CommandGetPosition;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSK_BossTelop2;
extern NEWCHARA NewTimer2;
extern NEWCHARA NewDynamicFloor;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewScrHexFade;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewKac_CommandGetKasLife;
extern NEWCHARA NewExecProcContinual;
/* 163 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command StreamIsPlay */
		{ 0x00020BD1, NewStreamIsPlay },
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command ボスタイマーエンド */
		{ 0x0005603B, NewTimerEnd2 },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command 跳弾水蒸気大 */
		{ 0x0009C0E8, NewEntrySteamAttackTargetLarge },
	/* command 跳弾水蒸気中 */
		{ 0x0009C107, NewEntrySteamAttackTargetMiddle },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command ゲットカサッカステータス */
		{ 0x0015984A, NewKac_CommandGetKasStat },
	/* command ハリアアイテムセット */
		{ 0x0017078B, NewHarItemSet },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command キャラマップチェンジ */
		{ 0x00247947, GM_ComMapChange },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* chara ボスラッシュポーズ */
		{ 0x002E4EEB, NewSK_BossPause },
	/* command レーダーズーム設定 */
		{ 0x002EB280, NewRadarZoomRateControl },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command ボスセットタイマーステータス */
		{ 0x002F9FE3, NewSetTimerStatus2 },
	/* command ゲットクラスター座標 */
		{ 0x003129D0, NewHar_CommandGetClaster },
	/* command ボスラッシュポーズ表示非表示 */
		{ 0x0033D38E, SK_BossPauseVisibleInvisible },
	/* command ボスラッシュ２結果後書き */
		{ 0x0035804E, SK_BossTimeAfterWrite2 },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* chara ハリアーＢＧＭマネ */
		{ 0x00374069, NewHarBGM_Manager },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command エフェクトバウンド実行 */
		{ 0x003DFD0B, UTL_EFT_ExecCallback },
	/* command ボスゲットラップタイム */
		{ 0x003F0A3F, NewGetLapTime },
	/* command プレイヤーモーション振動ＯＦＦ */
		{ 0x003F46EC, NewPlayerMotionVibOff },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* command エフェクトバウンド追加 */
		{ 0x0042181A, UTL_EFT_AddBoundFromGCL },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command プラグイン強制吹っ飛ばし */
		{ 0x00475CA4, NewPluginSuperBlow },
	/* command プラグイン落下 */
		{ 0x00476790, NewPluginFall },
	/* command 無線状態 */
		{ 0x00497BE6, CodecStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command ボスゲット残りタイム */
		{ 0x004A65BA, NewGetLeftTime2 },
	/* chara プット旗オブジェ */
		{ 0x004B5435, NewPutFlagObject },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* chara 連絡橋爆発 */
		{ 0x00502681, NewBridgeExplosionScn },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara ボスハリアー */
		{ 0x0053CFFA, NewBossHarrier },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command エルード移動範囲設定 */
		{ 0x005686AE, NewSetEludeRange },
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
	/* chara パッドデモ */
		{ 0x005C0BAE, NewPadDemoPlay },
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* command トラップ削除 */
		{ 0x005F8469, NewRemoveTrapBind },
	/* chara ＢＧＭ風音 */
		{ 0x005FBE23, NewWindNoise },
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
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command 無線メモリー設定 */
		{ 0x007B35E0, CodecMemCallSet },
	/* command カメラ視界チェック */
		{ 0x007F3C88, NewViewCheckCommand },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* chara ハリアー戦カサッカ */
		{ 0x0083DD4D, NewHarKasacka },
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* chara ボスラッシュ２結果 */
		{ 0x008A0C96, NewSK_BossResult2 },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command パッドチェック */
		{ 0x008B6086, NewPadCheck },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command プラグイン自動ジャンプ */
		{ 0x008FF6AD, NewPluginAutoJump },
	/* chara 水面監視水飛沫 */
		{ 0x0090B7EC, NewCheckWaterLevel },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara ローカル風 */
		{ 0x00987E81, NewLocalWind },
	/* command ボスラッシュモードセット */
		{ 0x0099615D, GM_COM_SetBossSurvivalStatus },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara サイファ */
		{ 0x009B3FD5, NewCypher },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* command シナリオ前装備変更 */
		{ 0x009CF455, PL_COM_ChangePrevItemScn },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* chara 複数赤外線 */
		{ 0x00A58FA5, NewIrs2 },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* command サイト表示制御 */
		{ 0x00A6D9CF, PL_COM_SightOnOff },
	/* chara かもめマネージャ */
		{ 0x00AF4CF6, NewKamomeManager },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command ポーズリセット */
		{ 0x00BD28C4, GM_COM_PauseOff },
	/* chara ターゲットトラップ */
		{ 0x00BDD6A9, NewTargetTrap },
	/* command プレイヤー瞬殺 */
		{ 0x00C3189A, NewKillPlayer },
	/* command ボスアンセットタイマーステータス */
		{ 0x00C41872, NewUnsetTimerStatus2 },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* command ボスラッシュ２音声交換 */
		{ 0x00C84C1D, SK_BossResultChenge2 },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 汎用空 */
		{ 0x00C9DD51, NewSkyUtil },
	/* command メニューフナムシ取り付き状態取得 */
		{ 0x00CB6EC4, NewGetShipwormStatus },
	/* chara ボスラッシュカサッカ */
		{ 0x00CBD98B, NewBosKasacka },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* chara 拡張海表示 */
		{ 0x00D5D6EF, NewExtraUmiDraw },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command ゲットハリアー座標 */
		{ 0x00DBA0D1, NewHar_CommandGetPosition },
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
	/* command ポーズセット */
		{ 0x00E74F46, GM_COM_PauseOn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara トラップ専用C4 */
		{ 0x00E9021D, NewTRAP_C4 },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara ステージ置き炎 */
		{ 0x00EC084B, NewStageFireScn },
	/* command ゲットスネーク座標 */
		{ 0x00EE8700, NewKac_CommandGetPosition },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara ボスラッシュ２テロップ表示 */
		{ 0x00F466B6, NewSK_BossTelop2 },
	/* chara ボスタイマー */
		{ 0x00F4BBF5, NewTimer2 },
	/* chara 透明床 */
		{ 0x00F91A08, NewDynamicFloor },
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* chara 六角フェード */
		{ 0x00FA4E80, NewScrHexFade },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* command ゲットカサッカライフ */
		{ 0x00FFE7A7, NewKac_CommandGetKasLife },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
