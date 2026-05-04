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
extern NEWCHARA NewAnmtexSet;
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewTimerEnd2;
extern NEWCHARA NewFatmanPuppetIKSupportSet;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewRadar;
extern NEWCHARA NewFatman;
extern NEWCHARA NewCmdKamomeInvisible;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewMap3D;
extern NEWCHARA GM_GetPosForGCL;
extern NEWCHARA NewSK_BossPause;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewSetTimerStatus2;
extern NEWCHARA NewGclAtan;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA SK_BossPauseVisibleInvisible;
extern NEWCHARA NewReadSavedVariable;
extern NEWCHARA SK_BossTimeAfterWrite2;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewSetKaitaiC4;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA PL_COM_InvisiblePlayer;
extern NEWCHARA NewGetLapTime;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewGetLeftTime;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPluginFall;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewGetLeftTime2;
extern NEWCHARA NewSIG_SetDogTagFlag;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombReset;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA NewWindNoise;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewCorpGcl;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBombEffectScn;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewSeaSurfaceSet;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewDelay;
extern NEWCHARA NewBombArea;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA GM_SeSetVolPanGcl;
extern NEWCHARA NewSK_BossResult2;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA PL_COM_OnCorpseStand;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA NewCheckWaterLevel;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA COM_GetEnemyStSt;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA GM_COM_SetBossSurvivalStatus;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewTimer;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA PL_COM_BulletSplashDaemon;
extern NEWCHARA NewFogSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewPutVanimeObject;
extern NEWCHARA NewKamomeManager;
extern NEWCHARA NewTimerEnd;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewAutoJumpSetting;
extern NEWCHARA GM_SetZoneFlagForScn;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA GM_COM_PauseOff;
extern NEWCHARA NewUnsetTimerStatus2;
extern NEWCHARA NewCmdFatInvisible;
extern NEWCHARA NewArraySet;
extern NEWCHARA SK_BossResultChenge2;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewExtraUmiDraw;
extern NEWCHARA NewSetTimerStatus;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA GM_COM_PauseOn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSK_BossTelop2;
extern NEWCHARA NewTimer2;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewScrHexFade;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 154 functions */
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
	/* chara アニメテクスチャセット */
		{ 0x000423C8, NewAnmtexSet },
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command ボスタイマーエンド */
		{ 0x0005603B, NewTimerEnd2 },
	/* chara ファットマン死体ＩＫ補正 */
		{ 0x0005872D, NewFatmanPuppetIKSupportSet },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara ファットマン */
		{ 0x001CBD93, NewFatman },
	/* command カモメ非表示 */
		{ 0x001F4349, NewCmdKamomeInvisible },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* command コントロール座標取得 */
		{ 0x002D77D4, GM_GetPosForGCL },
	/* chara ボスラッシュポーズ */
		{ 0x002E4EEB, NewSK_BossPause },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command ボスセットタイマーステータス */
		{ 0x002F9FE3, NewSetTimerStatus2 },
	/* command atan */
		{ 0x00325C8E, NewGclAtan },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* command ボスラッシュポーズ表示非表示 */
		{ 0x0033D38E, SK_BossPauseVisibleInvisible },
	/* command save_var_read */
		{ 0x0034CC13, NewReadSavedVariable },
	/* command ボスラッシュ２結果後書き */
		{ 0x0035804E, SK_BossTimeAfterWrite2 },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* chara 解体Ｃ４ */
		{ 0x003A6F25, NewSetKaitaiC4 },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command プレイヤー不可視 */
		{ 0x003E7B6F, PL_COM_InvisiblePlayer },
	/* command ボスゲットラップタイム */
		{ 0x003F0A3F, NewGetLapTime },
	/* command プレイヤーモーション振動ＯＦＦ */
		{ 0x003F46EC, NewPlayerMotionVibOff },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command ゲット残りタイム */
		{ 0x00450C64, NewGetLeftTime },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command プラグイン落下 */
		{ 0x00476790, NewPluginFall },
	/* command 無線状態 */
		{ 0x00497BE6, CodecStatus },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command ボスゲット残りタイム */
		{ 0x004A65BA, NewGetLeftTime2 },
	/* command ドッグタグフラグセット */
		{ 0x004BF8E6, NewSIG_SetDogTagFlag },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command 全体マップ爆弾配置解除 */
		{ 0x0050396F, SK_ScnBombReset },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command エルード移動範囲設定 */
		{ 0x005686AE, NewSetEludeRange },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
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
	/* chara 死体君 */
		{ 0x00645113, NewCorpGcl },
	/* command ハリアー戦で誰がプレイヤーを攻撃したか */
		{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
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
	/* chara プラント海面 */
		{ 0x006AE654, NewSeaSurfaceSet },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara 爆弾検知領域 */
		{ 0x0076A03C, NewBombArea },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
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
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* command ＳＥセットＶＰ */
		{ 0x0088CBF1, GM_SeSetVolPanGcl },
	/* chara ボスラッシュ２結果 */
		{ 0x008A0C96, NewSK_BossResult2 },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command プレイヤー死体上立たせ */
		{ 0x008DFE63, PL_COM_OnCorpseStand },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command プラグイン自動ジャンプ */
		{ 0x008FF6AD, NewPluginAutoJump },
	/* chara 水面監視水飛沫 */
		{ 0x0090B7EC, NewCheckWaterLevel },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command ゲット敵兵状態拡張版 */
		{ 0x00924573, COM_GetEnemyStSt },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* command ボスラッシュモードセット */
		{ 0x0099615D, GM_COM_SetBossSurvivalStatus },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara タイマー */
		{ 0x009B65F0, NewTimer },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* command 弾水飛沫処理 */
		{ 0x009E1DF3, PL_COM_BulletSplashDaemon },
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* chara プット頂点アニメオブジェ */
		{ 0x00A895C4, NewPutVanimeObject },
	/* chara かもめマネージャ */
		{ 0x00AF4CF6, NewKamomeManager },
	/* command タイマーエンド */
		{ 0x00AFDEE4, NewTimerEnd },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 自動ジャンプ設定 */
		{ 0x00B35703, NewAutoJumpSetting },
	/* command セットゾーンフラグ */
		{ 0x00B3D54E, GM_SetZoneFlagForScn },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command ポーズリセット */
		{ 0x00BD28C4, GM_COM_PauseOff },
	/* command ボスアンセットタイマーステータス */
		{ 0x00C41872, NewUnsetTimerStatus2 },
	/* command ファットマン非表示 */
		{ 0x00C6F28E, NewCmdFatInvisible },
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
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command 跳弾ＳＥのみ */
		{ 0x00D10066, NewEntrySparkOnlySE },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* chara 拡張海表示 */
		{ 0x00D5D6EF, NewExtraUmiDraw },
	/* command セットタイマーステータス */
		{ 0x00D99A89, NewSetTimerStatus },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
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
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
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
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* chara 六角フェード */
		{ 0x00FA4E80, NewScrHexFade },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
