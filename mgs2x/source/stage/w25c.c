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
extern NEWCHARA NewEMA_CommandGetPosition;
extern NEWCHARA NewEMA_CommandGetState;
extern NEWCHARA NewCamera;
extern NEWCHARA GetNPCSnakeStatus;
extern NEWCHARA NewEMA_CommandGetLife;
extern NEWCHARA NewEntrySteamAttackTargetLarge;
extern NEWCHARA NewEntrySteamAttackTargetMiddle;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewGetPlayerStateFlag;
extern NEWCHARA NewGetPlayerStatus;
extern NEWCHARA NewRaiden;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA PL_COM_CautionSquatOnly;
extern NEWCHARA NewRadar;
extern NEWCHARA GM_ComMapChange;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewMap3D;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewGclAtan;
extern NEWCHARA PL_COM_SetMissileBounding;
extern NEWCHARA SK_ScnBombSet;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA SK_SpecialGameOver;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewPlayerMotionVibOff;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewSetCamera;
extern NEWCHARA EE_COM_GetLife;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPluginFall;
extern NEWCHARA CodecStatus;
extern NEWCHARA NewCodec;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA SK_ScnBombInit;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewRisingSmokeScn;
extern NEWCHARA NewDoorLamp;
extern NEWCHARA NewDoor;
extern NEWCHARA NewSetEludeRange;
extern NEWCHARA NewMapConnect;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewHzxSetGroupAdd;
extern NEWCHARA NewWindNoise;
extern NEWCHARA NewGclDiffDirABS;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewPlantSunMain;
extern NEWCHARA NewHar_CommandGetWhoAtackPlayer;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewEMA_CommandGetLink;
extern NEWCHARA NewFogControl;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewPadVibrationScn;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewSeaSurfaceSet;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewMenuShipwormSet;
extern NEWCHARA NewForceActEndProc;
extern NEWCHARA NewBGMManager;
extern NEWCHARA PL_COM_FloorHeight;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA CodecMemCallSet;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewGclVecLen;
extern NEWCHARA SK_RootPositionChange;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewSetEludeParam;
extern NEWCHARA NewSetNikitaTimer;
extern NEWCHARA ENEMEM_GclCopy;
extern NEWCHARA NewPadCheck;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewPluginAutoJump;
extern NEWCHARA NewCheckWaterLevel;
extern NEWCHARA SK_ScnBreakPointSet;
extern NEWCHARA COM_GetEnemyStSt;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewItemBox;
extern NEWCHARA NewCypher;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA NewZman;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA PL_COM_SightOnOff;
extern NEWCHARA NewKamomeManager;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewEvmHairModel;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewW32Commander;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewSkyUtil;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewLoDControl;
extern NEWCHARA NewExtraUmiDraw;
extern NEWCHARA NewW25Defender;
extern NEWCHARA GM_GetSngCode;
extern NEWCHARA Command_GetFallFloorStatus;
extern NEWCHARA NewCommander;
extern NEWCHARA PL_COM_SetNoUse;
extern NEWCHARA New2DSprite;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewDoorPannelSpark;
extern NEWCHARA COM_GetEnemyStatus;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewFallFloor;
extern NEWCHARA NewStageFireScn;
extern NEWCHARA SK_ScnSelectStageLimitAutoSet;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewDynamicSegment;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA GM_COM_IsGameOver;
extern NEWCHARA NewExecProcContinual;
/* 130 functions */
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
	/* command ゲットエマ座標 */
		{ 0x0002EAD3, NewEMA_CommandGetPosition },
	/* command ゲットエマ状態 */
		{ 0x0005B611, NewEMA_CommandGetState },
	/* chara カメラ */
		{ 0x00061ADE, NewCamera },
	/* command ＮＰＣスネークの状態 */
		{ 0x00075D93, GetNPCSnakeStatus },
	/* command ゲットエマライフ */
		{ 0x00091159, NewEMA_CommandGetLife },
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
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* command プレイヤー張付きはしゃがみだけ */
		{ 0x0019358B, PL_COM_CautionSquatOnly },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command キャラマップチェンジ */
		{ 0x00247947, GM_ComMapChange },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
	/* chara 全体マップ表示3D */
		{ 0x002A4246, NewMap3D },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command atan */
		{ 0x00325C8E, NewGclAtan },
	/* command ミサイル移動範囲設定 */
		{ 0x003470C9, PL_COM_SetMissileBounding },
	/* command 全体マップ爆弾配置 */
		{ 0x00358D2F, SK_ScnBombSet },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
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
	/* command 水中エマライフゲット */
		{ 0x004481ED, EE_COM_GetLife },
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
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command 全体マップ爆弾配置初期化 */
		{ 0x00515453, SK_ScnBombInit },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* chara テスト昇り煙 */
		{ 0x00547041, NewRisingSmokeScn },
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
	/* command 絶対角度差 */
		{ 0x00604185, NewGclDiffDirABS },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 太陽 */
		{ 0x00641A7B, NewPlantSunMain },
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
	/* command 強制モーション終了プロック */
		{ 0x00709DF6, NewForceActEndProc },
	/* chara ＢＧＭマネージャー */
		{ 0x0070A68A, NewBGMManager },
	/* command プレイヤー床高さ取得 */
		{ 0x0073E084, PL_COM_FloorHeight },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command 無線メモリー設定 */
		{ 0x007B35E0, CodecMemCallSet },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command VecLen */
		{ 0x0082BDC0, NewGclVecLen },
	/* command 全体マップリアルタイム設定 */
		{ 0x0082CB77, SK_RootPositionChange },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* command エルード設定 */
		{ 0x008422F6, NewSetEludeParam },
	/* command ニキータタイマー設定 */
		{ 0x00848E57, NewSetNikitaTimer },
	/* command 敵兵メモリーコピー */
		{ 0x0086D1CF, ENEMEM_GclCopy },
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
	/* command ゲット敵兵状態拡張版 */
		{ 0x00924573, COM_GetEnemyStSt },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara アイテム */
		{ 0x009AFF54, NewItemBox },
	/* chara サイファ */
		{ 0x009B3FD5, NewCypher },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
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
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara 海上巡回コマンダー */
		{ 0x00BD9CC1, NewW32Commander },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 汎用空 */
		{ 0x00C9DD51, NewSkyUtil },
	/* command メニューフナムシ取り付き状態取得 */
		{ 0x00CB6EC4, NewGetShipwormStatus },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara ＬＯＤ制御 */
		{ 0x00D4090D, NewLoDControl },
	/* chara 拡張海表示 */
		{ 0x00D5D6EF, NewExtraUmiDraw },
	/* chara タイトロープ兵 */
		{ 0x00D99FB0, NewW25Defender },
	/* command 再生ＢＧＭ取得 */
		{ 0x00D9C789, GM_GetSngCode },
	/* command 崩落床状態取得 */
		{ 0x00DDD2A3, Command_GetFallFloorStatus },
	/* chara コマンダー */
		{ 0x00DDF5CA, NewCommander },
	/* command 不可装備設定 */
		{ 0x00DE4430, PL_COM_SetNoUse },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* chara ドアパネル電撃 */
		{ 0x00E2ED79, NewDoorPannelSpark },
	/* command ゲット敵兵状態 */
		{ 0x00E6B658, COM_GetEnemyStatus },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara 崩落床 */
		{ 0x00EA5215, NewFallFloor },
	/* chara ステージ置き炎 */
		{ 0x00EC084B, NewStageFireScn },
	/* command 全体マップ選択可能範囲設定 */
		{ 0x00F0F504, SK_ScnSelectStageLimitAutoSet },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
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
