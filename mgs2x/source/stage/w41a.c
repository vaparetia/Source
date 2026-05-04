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
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewShadowDrawObjectSet;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewPluginSneeze;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA PL_COM_VitalityAdjust;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewTngCommander;
extern NEWCHARA NewRadar;
extern NEWCHARA NewPlayerResetInvincible;
extern NEWCHARA NewMap3D;
extern NEWCHARA PL_COM_SetRecoverValue;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA PL_CHARA_ScnVibCamera;
extern NEWCHARA NewStrmFader;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA COM_ChangeClearingRoot;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNodeLamp;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewSigRouteModel;
extern NEWCHARA NewEasyLayout;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA PL_COM_SetNudeMode;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewDoor;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewPlayerSetCold;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA PL_COM_SetForceActLoop;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewSetChanlTargetMap;
extern NEWCHARA GM_COM_SnatchEquip;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewSetLockerMotion;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewStreamSet;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA NewLockerDaemon;
extern NEWCHARA NewDemoSnakeArm;
extern NEWCHARA NewLocker;
extern NEWCHARA NewPlayerSetInvincible;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewLockerStatus;
extern NEWCHARA NewSetPlayerLockerMotion;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA PL_COM_VisibleO2Gage;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewRopeModel3;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewO2Gage;
extern NEWCHARA NewNodeDaemon;
extern NEWCHARA NewEnbControl;
extern NEWCHARA COM_SetClearingData;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA PL_COM_SetPadType;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBlur;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewSubjectDemoPlayer;
extern NEWCHARA NewTs_SubWindow_S;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewCeiling;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA NewTrapSwitchCommand;
extern NEWCHARA NewCommander;
extern NEWCHARA NewAttachment2;
extern NEWCHARA NewAttachment3;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewStartScenarioDemo;
extern NEWCHARA NewNode;
extern NEWCHARA New2DSprite_Pause;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewEndScenarioDemo;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewForceActCancel;
extern NEWCHARA GM_SeSetModeGcl;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewDynamicFloor;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewSigTengA;
extern NEWCHARA NewExecProcContinual;
/* 132 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command cos */
		{ 0x00019A53, NewGclCos },
	/* command sin */
		{ 0x0001D98E, NewGclSin },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara 影投影モデル */
		{ 0x0005C815, NewShadowDrawObjectSet },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command プラグインくしゃみ */
		{ 0x0008BFFC, NewPluginSneeze },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command プレイヤー状態取得 */
		{ 0x000DFADB, NewGetPlayerStateFlag },
	/* command プレイヤーライフ値操作 */
		{ 0x00122E63, PL_COM_VitalityAdjust },
	/* chara プレイヤー */
		{ 0x00128946, NewRaiden },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara 天狗兵コマンダー */
		{ 0x0018F412, NewTngCommander },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command プレイヤー無敵解除 */
		{ 0x002981FA, NewPlayerResetInvincible },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* command Ｏ２ゲージ回復力設定 */
		{ 0x002C4B35, PL_COM_SetRecoverValue },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* chara シナリオカメラ振動 */
		{ 0x003124A2, PL_CHARA_ScnVibCamera },
	/* chara 台詞発生源君 */
		{ 0x0031D58B, NewStrmFader },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command チェンジクリアリングルート */
		{ 0x0038D44E, COM_ChangeClearingRoot },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* command ゲームオーバー背景設定 */
		{ 0x003AF581, SK_SpecialGameOver },
	/* chara ノード端末ディスプレイ */
		{ 0x003B1909, NewNodeLamp },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* chara カメラ設定 */
		{ 0x0043F718, NewSetCamera },
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara ルートデモ人形 */
		{ 0x00492A40, NewSigRouteModel },
	/* chara 簡易レイアウトプレイヤ */
		{ 0x004A0018, NewEasyLayout },
	/* command 無線設定 */
		{ 0x004A243A, NewCodec },
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
	/* chara ドア */
		{ 0x0055B942, NewDoor },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command プレイヤー風邪引かせ */
		{ 0x005BFE1D, NewPlayerSetCold },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* command ハリアー戦で誰がプレイヤーを攻撃したか */
		{ 0x00646DE9, NewHar_CommandGetWhoAtackPlayer },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
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
	/* chara メニューフナムシ */
		{ 0x006E7F78, NewMenuShipwormSet },
	/* command カメラマップ設定 */
		{ 0x006F5B0F, NewSetChanlTargetMap },
	/* command 武器アイテム取られ */
		{ 0x006FA5F5, GM_COM_SnatchEquip },
	/* command 強制モーション終了プロック */
		{ 0x00709DF6, NewForceActEndProc },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* command ロッカーモーション */
		{ 0x0077FFC8, NewSetLockerMotion },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command 無線メモリー設定 */
		{ 0x007B35E0, CodecMemCallSet },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* chara ロッカー管理 */
		{ 0x0085E8F4, NewLockerDaemon },
	/* chara デモ蛇手 */
		{ 0x0086D382, NewDemoSnakeArm },
	/* chara ロッカー */
		{ 0x008AA572, NewLocker },
	/* command プレイヤー無敵セット */
		{ 0x008B19F0, NewPlayerSetInvincible },
	/* command パッドチェック */
		{ 0x008B6086, NewPadCheck },
	/* command ロッカー状態 */
		{ 0x008B976D, NewLockerStatus },
	/* command プレイヤーロッカーモーション */
		{ 0x009049ED, NewSetPlayerLockerMotion },
	/* command 全体マップ壊れ設定 */
		{ 0x009173E7, SK_ScnBreakPointSet },
	/* command Ｏ２ゲージ表示 */
		{ 0x009208DC, PL_COM_VisibleO2Gage },
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
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* chara Ｏ２ゲージ */
		{ 0x00A7CB42, NewO2Gage },
	/* chara ノードシステム */
		{ 0x00AB381C, NewNodeDaemon },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command セットクリアリング */
		{ 0x00B1E8D4, COM_SetClearingData },
	/* chara マルチウェイト髪の毛モデル */
		{ 0x00B2A6B7, NewEvmHairModel },
	/* command プレイヤーパッド設定 */
		{ 0x00B414AE, PL_COM_SetPadType },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ブラー */
		{ 0x00B63A33, NewBlur },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* chara 近景ぼかし */
		{ 0x00BBAD24, NewNearFocusEffectSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara 影管理 */
		{ 0x00BD400B, NewShadowControl },
	/* chara 主観デモプレイヤー */
		{ 0x00C1BC23, NewSubjectDemoPlayer },
	/* chara サブ画面 */
		{ 0x00C210E3, NewTs_SubWindow_S },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* command メニューフナムシ取り付き状態取得 */
		{ 0x00CB6EC4, NewGetShipwormStatus },
	/* chara 天井君 */
		{ 0x00CBB124, NewCeiling },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command トラップ切り替え */
		{ 0x00DA97FB, NewTrapSwitchCommand },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* chara 装備品Ｂ */
		{ 0x00DE0401, NewAttachment2 },
	/* chara 装備品Ｃ */
		{ 0x00DE0402, NewAttachment3 },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command シナリオデモ開始 */
		{ 0x00E29ADB, NewStartScenarioDemo },
	/* command ノード画面起動 */
		{ 0x00E3549B, NewNode },
	/* chara ２Ｄスプライト表示＿ポーズ */
		{ 0x00E39C0D, New2DSprite_Pause },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* command シナリオデモ終了 */
		{ 0x00E79927, NewEndScenarioDemo },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command 強制モーションキャンセル */
		{ 0x00EAD648, NewForceActCancel },
	/* command ＳＥセットモード */
		{ 0x00EE8B90, GM_SeSetModeGcl },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* chara 透明床 */
		{ 0x00F91A08, NewDynamicFloor },
	/* chara 透明壁 */
		{ 0x00F91B9F, NewDynamicSegment },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* command ゲームオーバーチェック */
		{ 0x00FBEB7F, GM_COM_IsGameOver },
	/* chara 天狗兵Ａ */
		{ 0x00FE5C5B, NewSigTengA },
	/* chara プロック連続実行 */
		{ 0x00FFED03, NewExecProcContinual },
/* gcl chara end */
	{ 0, NULL }
};
