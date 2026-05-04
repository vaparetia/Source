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
extern NEWCHARA NewEMA_CommandGetPosition;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewCheckSlater;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewEMA_CommandGetLife;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewRipBubbleMan;
extern NEWCHARA NewVentilatorLight;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewScrBubble;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPutStaticSpotLight;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewKan;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewElevator;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNodeLamp;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewBodySlater;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewScrWaterFilm;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewKbdExec;
extern NEWCHARA NewSpotLightImageSet;
extern NEWCHARA NewCodec;
extern NEWCHARA NewCharaSpreadScn;
extern NEWCHARA NewElavatorLamp;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_WtDoorHandle2TurnProc;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewWaterFall;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA PL_COM_SetProcWaterInOut;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewDropBodySplushScn;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewEMA_CommandGetFloor;
extern NEWCHARA NewWaterModeSetting;
extern NEWCHARA NewSeaSlater;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewEMA_CommandGetLink;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewTrampleSlater;
extern NEWCHARA NewBloodWall;
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
extern NEWCHARA NewPluginElevator;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewFlowPaper;
extern NEWCHARA NewDust;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewScreenShotControl;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA NewPluginGraspEE;
extern NEWCHARA NewSpotDrawObjectSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewEmma;
extern NEWCHARA PL_COM_BulletSplashDaemon;
extern NEWCHARA NewBubbleLineScn;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA PL_COM_SightOnOff;
extern NEWCHARA NewO2Gage;
extern NEWCHARA NewNodeDaemon;
extern NEWCHARA NewWatertightDoorManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewWatertightDoorStatus;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBloodWater_Scn;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA NewEESwim;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewAllSlater;
extern NEWCHARA NewEMA_CommandForceMove;
extern NEWCHARA NewWaterMine;
extern NEWCHARA NewPassageWaterSet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewGasInWater;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewScrWater;
extern NEWCHARA NewBreathPointManager;
extern NEWCHARA NewLoDControl;
extern NEWCHARA EE_COM_SetLife;
extern NEWCHARA NewEmmaSlater;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewAddBreathPoint;
extern NEWCHARA NewDustOnCamera;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA NewNode;
extern NEWCHARA NewDustArea;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewWatertightDoor;
extern NEWCHARA NewSteamOnCameraScenario;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewScrTrans;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 149 functions */
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
	/* command ゲットエマ座標 */
		{ 0x0002EAD3, NewEMA_CommandGetPosition },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* chara 船虫踏み潰し管理 */
		{ 0x0007E215, NewCheckSlater },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
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
	/* chara あきかん */
		{ 0x002A0B46, NewKan },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara エレベータ */
		{ 0x0030E9CC, NewElevator },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ノード端末ディスプレイ */
		{ 0x003B1909, NewNodeLamp },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* chara 体舟虫 */
		{ 0x004152DF, NewBodySlater },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ水膜 */
		{ 0x00438FFA, NewScrWaterFilm },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara USBキーボード実行 */
		{ 0x0047D37A, NewKbdExec },
	/* chara スポットライト背景イメージ指定 */
		{ 0x0048F8F8, NewSpotLightImageSet },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
	/* chara キャラ付随水飛抹 */
		{ 0x004BAB34, NewCharaSpreadScn },
	/* chara エレベータパネル制御 */
		{ 0x004CBFC5, NewElavatorLamp },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command 水密ドアハンドル２回転時プロックコール */
		{ 0x004E8280, PL_COM_WtDoorHandle2TurnProc },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* chara パイプ流水 */
		{ 0x00502332, NewWaterFall },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command 水中出入り時プロック */
		{ 0x005282A9, PL_COM_SetProcWaterInOut },
	/* chara ドアランプ */
		{ 0x0054B365, NewDoorLamp },
	/* chara ドア */
		{ 0x0055B942, NewDoor },
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
	/* command ゲットエマ足元座標 */
		{ 0x00608B23, NewEMA_CommandGetFloor },
	/* command 新水中設定 */
		{ 0x0060C857, NewWaterModeSetting },
	/* chara 舟虫 */
		{ 0x0061514E, NewSeaSlater },
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
	/* chara 潰れふなむし */
		{ 0x00675145, NewTrampleSlater },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
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
	/* command プラグインエレベータ */
		{ 0x00770EC3, NewPluginElevator },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* chara 浮遊紙 */
		{ 0x008975E6, NewFlowPaper },
	/* chara 浮遊物 */
		{ 0x0089778A, NewDust },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* chara スクリーンショット */
		{ 0x008C4A5B, NewScreenShotControl },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command プラグインエマ手繋ぎ */
		{ 0x00926341, NewPluginGraspEE },
	/* chara スポットライト投影モデル */
		{ 0x009307AB, NewSpotDrawObjectSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara エマ・エメリッヒ */
		{ 0x009B3B8B, NewEmma },
	/* command 弾水飛沫処理 */
		{ 0x009E1DF3, PL_COM_BulletSplashDaemon },
	/* chara 連続発生線状泡 */
		{ 0x00A0A179, NewBubbleLineScn },
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
	/* chara 水密ドア管理 */
		{ 0x00ABFD5F, NewWatertightDoorManager },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 水密ドア状態 */
		{ 0x00B1ABD8, NewWatertightDoorStatus },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara 水中血 */
		{ 0x00B53158, NewBloodWater_Scn },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* chara 水中エマ */
		{ 0x00BB6852, NewEESwim },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara フナ虫リーダー */
		{ 0x00C09E6C, NewAllSlater },
	/* command エマ強制瞬間移動 */
		{ 0x00C2EB8D, NewEMA_CommandForceMove },
	/* chara 水中機雷 */
		{ 0x00C3515F, NewWaterMine },
	/* chara 通路用水面 */
		{ 0x00C5D24C, NewPassageWaterSet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 水中濁り */
		{ 0x00CB2C3E, NewGasInWater },
	/* chara 天井君 */
		{ 0x00CBB124, NewCeiling },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 水中カメラ */
		{ 0x00D16C76, NewScrWater },
	/* chara 息継ぎポイント管理 */
		{ 0x00D2C9AA, NewBreathPointManager },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* command 水中エマライフセット */
		{ 0x00D481ED, EE_COM_SetLife },
	/* chara エマ虫 */
		{ 0x00D612A2, NewEmmaSlater },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* command 息継ぎポイント登録 */
		{ 0x00DB65AC, NewAddBreathPoint },
	/* chara カメラダスト */
		{ 0x00DC323F, NewDustOnCamera },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* command ノード画面起動 */
		{ 0x00E3549B, NewNode },
	/* chara エリアダスト */
		{ 0x00E52073, NewDustArea },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara 水密ドア */
		{ 0x00EBEC24, NewWatertightDoor },
	/* chara カメラ前曇り */
		{ 0x00ED4678, NewSteamOnCameraScenario },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara フレーム退避 */
		{ 0x00F62833, NewScrTrans },
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
