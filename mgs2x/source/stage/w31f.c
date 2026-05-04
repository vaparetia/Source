/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewPluginWaterMode;
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewGclCos;
extern NEWCHARA NewGclSin;
extern NEWCHARA NewStreamIsPlay;
extern NEWCHARA NewEMA_CommandGetPosition;
extern NEWCHARA NewAnmtexSet;
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewTimerEnd2;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEMA_CommandGetLife;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewRipBubbleMan;
extern NEWCHARA NewVentilatorLight;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewScrBubble;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewPluginKageshibari;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewSuperVamp;
extern NEWCHARA NewSK_BossPause;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewSetTimerStatus2;
extern NEWCHARA SK_BossPauseVisibleInvisible;
extern NEWCHARA SK_BossTimeAfterWrite2;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNodeLamp;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewGetLapTime;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewScrWaterFilm;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewGlassScarBase;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPluginFall;
extern NEWCHARA NewKbdExec;
extern NEWCHARA NewSpotLightImageSet;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewCom_BRK_SPLGT_BreakMessage;
extern NEWCHARA NewCodec;
extern NEWCHARA NewGetLeftTime2;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA PL_COM_SetProcWaterInOut;
extern NEWCHARA NewForeach;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewDropBodySplushScn;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewPoolWaterComplex;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA NewEMA_CommandGetFloor;
extern NEWCHARA NewWaterModeSetting;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewEMA_CommandGetLink;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewBloodWall;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA PL_COM_SetForceActLoop;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewBubbleOnCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewSigPutModelEvm;
extern NEWCHARA NewDelay;
extern NEWCHARA NewBombArea;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewScenarioClaymore;
extern NEWCHARA NewPutObject;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewViewCheckCommand;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewWaterDustManager;
extern NEWCHARA NewFlowPaper;
extern NEWCHARA NewDust;
extern NEWCHARA NewSK_BossResult2;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewCheckWaterLevel;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA NewPluginGraspEE;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA AccessMobile;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA EE_COM_GetO2;
extern NEWCHARA GM_COM_SetBossSurvivalStatus;
extern NEWCHARA NewMobileDaemon;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewEmma;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA PL_COM_BulletSplashDaemon;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA PL_COM_SightOnOff;
extern NEWCHARA NewO2Gage;
extern NEWCHARA NewNodeDaemon;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA NewEESwim;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA GM_COM_PauseOff;
extern NEWCHARA NewVampShdwTrgt;
extern NEWCHARA NewEMA_CommandForceMove;
extern NEWCHARA NewWaterMine;
extern NEWCHARA NewUnsetTimerStatus2;
extern NEWCHARA NewPassageWaterSet;
extern NEWCHARA NewArraySet;
extern NEWCHARA SK_BossResultChenge2;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewGasInWater;
extern NEWCHARA GM_SetVolumeParam;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA OK_ResetGetBoundary;
extern NEWCHARA NewEntrySparkOnlySE;
extern NEWCHARA NewScrWater;
extern NEWCHARA NewBreathPointManager;
extern NEWCHARA NewLoDControl;
extern NEWCHARA EE_COM_SetLife;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewAddBreathPoint;
extern NEWCHARA NewDustOnCamera;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA NewDoorPannelSpark;
extern NEWCHARA NewNode;
extern NEWCHARA NewDustArea;
extern NEWCHARA GM_COM_PauseOn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewSteamOnCameraScenario;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA NewEmmaLocker;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA NewSlowDown;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSK_BossTelop2;
extern NEWCHARA NewTimer2;
extern NEWCHARA NewScrTrans;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewScrHexFade;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA PL_COM_SetJumpGravity;
extern NEWCHARA NewExecProcContinual;
/* 187 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 新プラグイン水中 */
		{ 0x00004C00, NewPluginWaterMode },
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command StreamIsPlay */
		{ 0x00020BD1, NewStreamIsPlay },
	/* command ゲットエマ座標 */
		{ 0x0002EAD3, NewEMA_CommandGetPosition },
	/* chara アニメテクスチャセット */
		{ 0x000423C8, NewAnmtexSet },
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
	/* command ゲットエマライフ */
		{ 0x00091159, NewEMA_CommandGetLife },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 泡で波紋管理 */
		{ 0x000B1261, NewRipBubbleMan },
	/* chara 天窓の光 */
		{ 0x000BA748, NewVentilatorLight },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command PlayerStatus */
		{ 0x000ECDFD, NewGetPlayerStatus },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara ゆがみ泡 */
		{ 0x001AF0D5, NewScrBubble },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara 静止スポットライト設置 */
		{ 0x001B5854, NewPutStaticSpotLight },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
	/* command プラグイン影縛り */
		{ 0x0029D24D, NewPluginKageshibari },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* chara プロトヴァンプ */
		{ 0x002B8E16, NewSuperVamp },
	/* chara ボスラッシュポーズ */
		{ 0x002E4EEB, NewSK_BossPause },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command ボスセットタイマーステータス */
		{ 0x002F9FE3, NewSetTimerStatus2 },
	/* command ボスラッシュポーズ表示非表示 */
		{ 0x0033D38E, SK_BossPauseVisibleInvisible },
	/* command ボスラッシュ２結果後書き */
		{ 0x0035804E, SK_BossTimeAfterWrite2 },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* chara プットスポットライトオブジェ */
		{ 0x00365DEE, NewPutSpotLightObject },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ノード端末ディスプレイ */
		{ 0x003B1909, NewNodeLamp },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command ボスゲットラップタイム */
		{ 0x003F0A3F, NewGetLapTime },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ水膜 */
		{ 0x00438FFA, NewScrWaterFilm },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* chara ガラス弾痕 */
		{ 0x0046139A, NewGlassScarBase },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command プラグイン落下 */
		{ 0x00476790, NewPluginFall },
	/* chara USBキーボード実行 */
		{ 0x0047D37A, NewKbdExec },
	/* chara スポットライト背景イメージ指定 */
		{ 0x0048F8F8, NewSpotLightImageSet },
	/* command 無線状態 */
		{ 0x00497BE6, CodecStatus },
	/* command プットスポットライトオブジェライト消す */
		{ 0x0049EB5E, NewCom_BRK_SPLGT_BreakMessage },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* command ボスゲット残りタイム */
		{ 0x004A65BA, NewGetLeftTime2 },
	/* chara キャラ付随水飛抹 */
		{ 0x004BAB34, NewCharaSpreadScn },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command 水中出入り時プロック */
		{ 0x005282A9, PL_COM_SetProcWaterInOut },
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
	/* chara 詳細水飛沫 */
		{ 0x0059F23C, NewDropBodySplushScn },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara バンププール */
		{ 0x005C3A78, NewPoolWaterComplex },
	/* command 追加ＨＺＸグループ登録 */
		{ 0x005CAA88, NewHzxSetGroupAdd },
	/* command ゲットエマ足元座標 */
		{ 0x00608B23, NewEMA_CommandGetFloor },
	/* command 新水中設定 */
		{ 0x0060C857, NewWaterModeSetting },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ハリアー戦で誰がプレイヤーを攻撃したか */
		{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* command ゲットエマ手繋ぎ状況 */
		{ 0x0065180B, NewEMA_CommandGetLink },
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
	/* command 強制モーションループ回数設定 */
		{ 0x006A9667, PL_COM_SetForceActLoop },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara カメラ泡 */
		{ 0x006B921A, NewBubbleOnCamera },
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* command 強制モーション終了プロック */
		{ 0x00709DF6, NewForceActEndProc },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* chara プットモーションモデル */
		{ 0x0072F23C, NewSigPutModelEvm },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara 爆弾検知領域 */
		{ 0x0076A03C, NewBombArea },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara 設置クレイモア */
		{ 0x0078D685, NewScenarioClaymore },
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
	/* chara 水中ゴミマネージャ */
		{ 0x00873375, NewWaterDustManager },
	/* chara 浮遊紙 */
		{ 0x008975E6, NewFlowPaper },
	/* chara 浮遊物 */
		{ 0x0089778A, NewDust },
	/* chara ボスラッシュ２結果 */
		{ 0x008A0C96, NewSK_BossResult2 },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* chara 水面監視水飛沫 */
		{ 0x0090B7EC, NewCheckWaterLevel },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command プラグインエマ手繋ぎ */
		{ 0x00926341, NewPluginGraspEE },
	/* chara スポットライト投影モデル */
		{ 0x009307AB, NewSpotDrawObjectSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* command 携帯呼出 */
		{ 0x00951C01, AccessMobile },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* command 水中エマＯ２ゲット */
		{ 0x0097889D, EE_COM_GetO2 },
	/* command ボスラッシュモードセット */
		{ 0x0099615D, GM_COM_SetBossSurvivalStatus },
	/* chara 携帯端末 */
		{ 0x009A75E7, NewMobileDaemon },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara エマ・エメリッヒ */
		{ 0x009B3B8B, NewEmma },
	/* chara ロープモデル３ */
		{ 0x009BC670, NewRopeModel3 },
	/* command 弾水飛沫処理 */
		{ 0x009E1DF3, PL_COM_BulletSplashDaemon },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* command サイト表示制御 */
		{ 0x00A6D9CF, PL_COM_SightOnOff },
	/* chara Ｏ２ゲージ */
		{ 0x00A7CB42, NewO2Gage },
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
	/* chara 水中エマ */
		{ 0x00BB6852, NewEESwim },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command ポーズリセット */
		{ 0x00BD28C4, GM_COM_PauseOff },
	/* chara ヴァンプ影縛りターゲット */
		{ 0x00BFB0A1, NewVampShdwTrgt },
	/* command エマ強制瞬間移動 */
		{ 0x00C2EB8D, NewEMA_CommandForceMove },
	/* chara 水中機雷 */
		{ 0x00C3515F, NewWaterMine },
	/* command ボスアンセットタイマーステータス */
		{ 0x00C41872, NewUnsetTimerStatus2 },
	/* chara 通路用水面 */
		{ 0x00C5D24C, NewPassageWaterSet },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* command ボスラッシュ２音声交換 */
		{ 0x00C84C1D, SK_BossResultChenge2 },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 水中濁り */
		{ 0x00CB2C3E, NewGasInWater },
	/* command セット音量パラメータ */
		{ 0x00CB6BED, GM_SetVolumeParam },
	/* command メニューフナムシ取り付き状態取得 */
		{ 0x00CB6EC4, NewGetShipwormStatus },
	/* chara 天井君 */
		{ 0x00CBB124, NewCeiling },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command アイテム収得可能範囲設定 */
		{ 0x00CEC17C, OK_ResetGetBoundary },
	/* command 跳弾ＳＥのみ */
		{ 0x00D10066, NewEntrySparkOnlySE },
	/* chara 水中カメラ */
		{ 0x00D16C76, NewScrWater },
	/* chara 息継ぎポイント管理 */
		{ 0x00D2C9AA, NewBreathPointManager },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command 水中エマライフセット */
		{ 0x00D481ED, EE_COM_SetLife },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* command 息継ぎポイント登録 */
		{ 0x00DB65AC, NewAddBreathPoint },
	/* chara カメラダスト */
		{ 0x00DC323F, NewDustOnCamera },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* chara ドアパネル電撃 */
		{ 0x00E2ED79, NewDoorPannelSpark },
	/* command ノード画面起動 */
		{ 0x00E3549B, NewNode },
	/* chara エリアダスト */
		{ 0x00E52073, NewDustArea },
	/* command ポーズセット */
		{ 0x00E74F46, GM_COM_PauseOn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara カメラ前曇り */
		{ 0x00ED4678, NewSteamOnCameraScenario },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* chara エマ・ロッカーの中にいる */
		{ 0x00EEE657, NewEmmaLocker },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* chara 速度可変 */
		{ 0x00F26728, NewSlowDown },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara ボスラッシュ２テロップ表示 */
		{ 0x00F466B6, NewSK_BossTelop2 },
	/* chara ボスタイマー */
		{ 0x00F4BBF5, NewTimer2 },
	/* chara フレーム退避 */
		{ 0x00F62833, NewScrTrans },
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* chara 六角フェード */
		{ 0x00FA4E80, NewScrHexFade },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* command ジャンプ時重力セット */
		{ 0x00FEA0C9, PL_COM_SetJumpGravity },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
