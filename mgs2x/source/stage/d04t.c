/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewManhatLight_L1;
extern NEWCHARA NewManhatLight_L2;
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewLineSplash;
extern NEWCHARA NewCreateEquipmentScn;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewMirrorControl;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewManhattan3D;
extern NEWCHARA New_ChimnySmoke;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewPluginStance;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewPutWorldTexFadeCamera;
extern NEWCHARA NewStormyRiverWaveSet;
extern NEWCHARA NewSplashFall_Scn;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewCircleLightSpot;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewSky_Prev;
extern NEWCHARA NewWindowRain;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewSkyColumn;
extern NEWCHARA NewOceanWave;
extern NEWCHARA NewRipple;
extern NEWCHARA NewPutWorldTex;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewRainFogPers;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewRainCamera;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutWorldTexFadeCameraForDoor;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewRotateY_Object;
extern NEWCHARA NewThund_Flash;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewRopeModel2;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewZman;
extern NEWCHARA NewManhatLight;
extern NEWCHARA NewPutVanimeObject;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewBlur;
extern NEWCHARA NewRainParts;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSplashFloor_Scn;
extern NEWCHARA NewBrooklyn_R2;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewPutWorldTexFadeWorld;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewBrooklyn3D;
extern NEWCHARA NewSpotRain;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewModelPatAnimation;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA NewRainCamera_Demo_0004Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewSplashMotion_Demo_0100Launch;
extern NEWCHARA NewSplashRipple_Demo_0101Launch;
extern NEWCHARA NewBodySplash3_0104Launch;
extern NEWCHARA NewEneEquip_1002Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewThunder_Demo_2000Launch;
extern NEWCHARA NewLocalWind_Demo_2002Launch;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA NewPutSTanimeObjectCall_4000Launch;
extern NEWCHARA DM_SendEffectMessage_4001Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA DM_DebugPrint_fff00Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA DM_SetClipParam_fff02Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
/* 110 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* chara マンハット赤灯1 */
		{ 0x000041E7, NewManhatLight_L1 },
	/* chara マンハット赤灯2 */
		{ 0x000041E8, NewManhatLight_L2 },
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* chara ライン水飛沫 */
		{ 0x0001A6DD, NewLineSplash },
	/* chara 装備品セット */
		{ 0x000381AC, NewCreateEquipmentScn },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* chara 鏡面モデル管理 */
		{ 0x000D02FD, NewMirrorControl },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* chara マンハッタン */
		{ 0x001E44B7, NewManhattan3D },
	/* chara 煙突煙 */
		{ 0x00264C7D, New_ChimnySmoke },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* chara 遠景ぼかし */
		{ 0x002DAD24, NewFarFocusEffectSet },
	/* chara 髭剃りスネークテクスチャ入れ替え */
		{ 0x002DF578, NewShavedSnakeTexReplaceActorScn },
	/* command プラグイン強制移動 */
		{ 0x002F3844, NewPluginForceRunMove },
	/* command StreamStopAll */
		{ 0x0032FB70, NewStreamStopAll },
	/* chara demo */
		{ 0x0033A20F, NewPolygonDemoStart },
	/* chara マップ */
		{ 0x0035A2CF, NewSetMap },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara プットテクスチャクロスフェードカメラ */
		{ 0x004847D2, NewPutWorldTexFadeCamera },
	/* chara 嵐の川波 */
		{ 0x004A777D, NewStormyRiverWaveSet },
	/* chara 落下水飛沫 */
		{ 0x004BAAFE, NewSplashFall_Scn },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara 固定ボリュームライト */
		{ 0x0054ECA9, NewCircleLightSpot },
	/* command マップ設定 */
		{ 0x0056EF97, NewMapAbout },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command マップ表示 */
		{ 0x005AE655, NewShowMap },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 新空 */
		{ 0x006273B5, NewSky_Prev },
	/* chara 窓雨 */
		{ 0x006442CB, NewWindowRain },
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
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara 霧雨 */
		{ 0x0068F6CB, NewRainFogPers },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* chara カメラ雨 */
		{ 0x006B8EE3, NewRainCamera },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットテクスチャクロスフェードカメラドア */
		{ 0x007A3DBF, NewPutWorldTexFadeCameraForDoor },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* chara くるくる物体 */
		{ 0x008862F6, NewRotateY_Object },
	/* chara 新雷フラッシュ */
		{ 0x0089AF5E, NewThund_Flash },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara ロープモデル２ */
		{ 0x009BC66F, NewRopeModel2 },
	/* chara 環境光 */
		{ 0x009DCD6B, NewAmbientSet },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara マンハット黄灯 */
		{ 0x00A8560D, NewManhatLight },
	/* chara プット頂点アニメオブジェ */
		{ 0x00A895C4, NewPutVanimeObject },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara ブラー */
		{ 0x00B63A33, NewBlur },
	/* chara 位置指定雨 */
		{ 0x00B80DE5, NewRainParts },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara 床水飛沫 */
		{ 0x00BE0863, NewSplashFloor_Scn },
	/* chara ブルックリン赤灯 */
		{ 0x00C0E06D, NewBrooklyn_R2 },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara プットテクスチャクロスフェードワールド */
		{ 0x00CF777F, NewPutWorldTexFadeWorld },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara ブルックリン */
		{ 0x00D6ED95, NewBrooklyn3D },
	/* chara スポット雨 */
		{ 0x00E41776, NewSpotRain },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command デモローポリ劇場 */
		{ 0x00F8F4E8, NewCom_AddRenameList },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
	/* chara モデル切り替えアニメ */
		{ 0x00FE6F50, NewModelPatAnimation },
	/* EXTERN NewFogSet_Demo */
		{ 0x01000000, NewFogSet_Demo_0000Launch },
	/* EXTERN NewFarFocusEffect */
		{ 0x01000001, NewFarFocusEffect_0001Launch },
	/* EXTERN NewNearFocusEffect */
		{ 0x01000002, NewNearFocusEffect_0002Launch },
	/* EXTERN NewRainCamera_Demo */
		{ 0x01000004, NewRainCamera_Demo_0004Launch },
	/* EXTERN DM_ChangeAmbient */
		{ 0x01000006, DM_ChangeAmbient_0006Launch },
	/* EXTERN DM_ChangeParallel */
		{ 0x01000007, DM_ChangeParallel_0007Launch },
	/* EXTERN NewFadeInOutForce_Demo */
		{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	/* EXTERN NewSplashMotion_Demo */
		{ 0x01000100, NewSplashMotion_Demo_0100Launch },
	/* EXTERN NewSplashRipple_Demo */
		{ 0x01000101, NewSplashRipple_Demo_0101Launch },
	/* EXTERN NewBodySplash3 */
		{ 0x01000104, NewBodySplash3_0104Launch },
	/* EXTERN NewEneEquip */
		{ 0x01001002, NewEneEquip_1002Launch },
	/* EXTERN NewPutAttachments */
		{ 0x01001006, NewPutAttachments_1006Launch },
	/* EXTERN NewThunder_Demo */
		{ 0x01002000, NewThunder_Demo_2000Launch },
	/* EXTERN NewLocalWind_Demo */
		{ 0x01002002, NewLocalWind_Demo_2002Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003000, NewRopeModel3_called_3000Launch },
	/* EXTERN NewPutSTanimeObjectCall */
		{ 0x01004000, NewPutSTanimeObjectCall_4000Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01004001, DM_SendEffectMessage_4001Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN DM_DebugPrint */
		{ 0x010FFF00, DM_DebugPrint_fff00Launch },
	/* EXTERN DM_ExecProc */
		{ 0x010FFF01, DM_ExecProc_fff01Launch },
	/* EXTERN DM_SetClipParam */
		{ 0x010FFF02, DM_SetClipParam_fff02Launch },
	/* EXTERN NewDemoFrameCountCall */
		{ 0x010FFF03, NewDemoFrameCountCall_fff03Launch },
	/* EXTERN NewDemoEffectInitialize */
		{ 0x010FFF05, NewDemoEffectInitialize_fff05Launch },
/* gcl chara end */
	{ 0, NULL }
};
