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
extern NEWCHARA NewStingerSightFlag;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA ComNPlantDogtags;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewSaveGameScr;
extern NEWCHARA NewWaterLevelControl;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewSky_Prev;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewSkyColumn;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewRainFogPers;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewSeaSurfaceSet;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewBubbleOnCamera;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA ComNTankerDogtags;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewThund_Flash;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewConfirmSaveScr;
extern NEWCHARA NewZman;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewSigSinkTanker;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewWindManager;
extern NEWCHARA NewDustOnCamera;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA NewRainCamera_Demo_0004Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewScrWater_Demo_0014Launch;
extern NEWCHARA NewScrConcentrateBlur_0015Launch;
extern NEWCHARA NewRaySight_0016Launch;
extern NEWCHARA NewSplashMotion_Demo_0100Launch;
extern NEWCHARA NewBodySplash3_0104Launch;
extern NEWCHARA NewSphereSplush_0106Launch;
extern NEWCHARA NewWaveSplash_Demo_0107Launch;
extern NEWCHARA NewWallTidal_0108Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewDemoArkms_100fLaunch;
extern NEWCHARA NewThunder_Demo_2000Launch;
extern NEWCHARA NewSlowParamMan_prog_2003Launch;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA NewEvmHairModel_Demo_3007Launch;
extern NEWCHARA NewDebris_Cm_Demo_6007Launch;
extern NEWCHARA NewDebris_Tex_Demo_6008Launch;
extern NEWCHARA NewBombGasEffect_600bLaunch;
extern NEWCHARA NewMesgBomb2_6010Launch;
extern NEWCHARA NewSonicWave_6012Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA NewPadVibration2_9000Launch;
extern NEWCHARA NewSplushSurfaceMan_a003Launch;
extern NEWCHARA NewRippleMan_a004Launch;
extern NEWCHARA NewWaterLevelControl_Demo_b000Launch;
extern NEWCHARA NewDropBodySplush_b001Launch;
extern NEWCHARA NewSeaSurfaceReactionObject_b004Launch;
extern NEWCHARA NewAutoSplush_b005Launch;
extern NEWCHARA NewAutoSplush_EftCtrl_b006Launch;
extern NEWCHARA TAKABE_RiseWaveEx_b040Launch;
extern NEWCHARA NewRayConsol_d000Launch;
extern NEWCHARA NewRayEye_d001Launch;
extern NEWCHARA DM_DebugPrint_fff00Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA DM_SetClipParam_fff02Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewControlBoy_fff04Launch;
extern NEWCHARA DM_ControlChange_ffff0Launch;
/* 108 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* chara 雨 */
		{ 0x000016CB, NewRain },
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
	/* command プラント編取得済みドッグタグ数 */
		{ 0x001F567B, ComNPlantDogtags },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* chara セーブゲーム画面マネージャー */
		{ 0x00274B60, NewSaveGameScr },
	/* command 水位設定 */
		{ 0x0029C817, NewWaterLevelControl },
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
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command スティンガー性能 */
		{ 0x0051DC64, NewStingerFlag },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
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
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 天球 */
		{ 0x00657385, NewSkyColumn },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara 霧雨 */
		{ 0x0068F6CB, NewRainFogPers },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* chara プラント海面 */
		{ 0x006AE654, NewSeaSurfaceSet },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara カメラ泡 */
		{ 0x006B921A, NewBubbleOnCamera },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* command タンカー編取得済みドッグタグ数 */
		{ 0x007C7E31, ComNTankerDogtags },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
	/* chara 新雷フラッシュ */
		{ 0x0089AF5E, NewThund_Flash },
	/* chara デモキャンセルチェック */
		{ 0x008E298D, NewPadCancel },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara 装備品サーバー */
		{ 0x0096E361, NewAttachment_Server },
	/* chara セーブ確認画面 */
		{ 0x009E8AEB, NewConfirmSaveScr },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* chara 沈没タンカー */
		{ 0x00C1FF0F, NewSigSinkTanker },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 風制御 */
		{ 0x00D2BD87, NewWindManager },
	/* chara カメラダスト */
		{ 0x00DC323F, NewDustOnCamera },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara 追跡血 */
		{ 0x00F37560, NewBloodSpread },
	/* command 武器ライト */
		{ 0x00FB029A, NewSKLightEff },
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
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewScrWater_Demo */
		{ 0x01000014, NewScrWater_Demo_0014Launch },
	/* EXTERN NewScrConcentrateBlur */
		{ 0x01000015, NewScrConcentrateBlur_0015Launch },
	/* EXTERN NewRaySight */
		{ 0x01000016, NewRaySight_0016Launch },
	/* EXTERN NewSplashMotion_Demo */
		{ 0x01000100, NewSplashMotion_Demo_0100Launch },
	/* EXTERN NewBodySplash3 */
		{ 0x01000104, NewBodySplash3_0104Launch },
	/* EXTERN NewSphereSplush */
		{ 0x01000106, NewSphereSplush_0106Launch },
	/* EXTERN NewWaveSplash_Demo */
		{ 0x01000107, NewWaveSplash_Demo_0107Launch },
	/* EXTERN NewWallTidal */
		{ 0x01000108, NewWallTidal_0108Launch },
	/* EXTERN NewPutAttachments */
		{ 0x01001006, NewPutAttachments_1006Launch },
	/* EXTERN NewDemoArkms */
		{ 0x0100100F, NewDemoArkms_100fLaunch },
	/* EXTERN NewThunder_Demo */
		{ 0x01002000, NewThunder_Demo_2000Launch },
	/* EXTERN NewSlowParamMan_prog */
		{ 0x01002003, NewSlowParamMan_prog_2003Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003000, NewRopeModel3_called_3000Launch },
	/* EXTERN NewEvmHairModel_Demo */
		{ 0x01003007, NewEvmHairModel_Demo_3007Launch },
	/* EXTERN NewDebris_Cm_Demo */
		{ 0x01006007, NewDebris_Cm_Demo_6007Launch },
	/* EXTERN NewDebris_Tex_Demo */
		{ 0x01006008, NewDebris_Tex_Demo_6008Launch },
	/* EXTERN NewBombGasEffect */
		{ 0x0100600B, NewBombGasEffect_600bLaunch },
	/* EXTERN NewMesgBomb2 */
		{ 0x01006010, NewMesgBomb2_6010Launch },
	/* EXTERN NewSonicWave */
		{ 0x01006012, NewSonicWave_6012Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN NewPadVibration2 */
		{ 0x01009000, NewPadVibration2_9000Launch },
	/* EXTERN NewSplushSurfaceMan */
		{ 0x0100A003, NewSplushSurfaceMan_a003Launch },
	/* EXTERN NewRippleMan */
		{ 0x0100A004, NewRippleMan_a004Launch },
	/* EXTERN NewWaterLevelControl_Demo */
		{ 0x0100B000, NewWaterLevelControl_Demo_b000Launch },
	/* EXTERN NewDropBodySplush */
		{ 0x0100B001, NewDropBodySplush_b001Launch },
	/* EXTERN NewSeaSurfaceReactionObject */
		{ 0x0100B004, NewSeaSurfaceReactionObject_b004Launch },
	/* EXTERN NewAutoSplush */
		{ 0x0100B005, NewAutoSplush_b005Launch },
	/* EXTERN NewAutoSplush_EftCtrl */
		{ 0x0100B006, NewAutoSplush_EftCtrl_b006Launch },
	/* EXTERN TAKABE_RiseWaveEx */
		{ 0x0100B040, TAKABE_RiseWaveEx_b040Launch },
	/* EXTERN NewRayConsol */
		{ 0x0100D000, NewRayConsol_d000Launch },
	/* EXTERN NewRayEye */
		{ 0x0100D001, NewRayEye_d001Launch },
	/* EXTERN DM_DebugPrint */
		{ 0x010FFF00, DM_DebugPrint_fff00Launch },
	/* EXTERN DM_ExecProc */
		{ 0x010FFF01, DM_ExecProc_fff01Launch },
	/* EXTERN DM_SetClipParam */
		{ 0x010FFF02, DM_SetClipParam_fff02Launch },
	/* EXTERN NewDemoFrameCountCall */
		{ 0x010FFF03, NewDemoFrameCountCall_fff03Launch },
	/* EXTERN NewControlBoy */
		{ 0x010FFF04, NewControlBoy_fff04Launch },
	/* EXTERN DM_ControlChange */
		{ 0x010FFFF0, DM_ControlChange_ffff0Launch },
/* gcl chara end */
	{ 0, NULL }
};
