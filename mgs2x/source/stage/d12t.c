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
extern NEWCHARA NewCreateEquipmentScn;
extern NEWCHARA NewShadowDrawObjectSet;
extern NEWCHARA NewEntryWallScar;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA PL_COM_ExchangeCache;
extern NEWCHARA NewRadar;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewFarFocusEffectSet;
extern NEWCHARA NewShavedSnakeTexReplaceActorScn;
extern NEWCHARA NewPluginForceRunMove;
extern NEWCHARA NewStreamStopAll;
extern NEWCHARA NewPolygonDemoStart;
extern NEWCHARA NewSetMap;
extern NEWCHARA NewPutSpotLightObject;
extern NEWCHARA NewRand;
extern NEWCHARA NewNamePrintManager;
extern NEWCHARA NewEntryNewSpark;
extern NEWCHARA NewPluginStance;
extern NEWCHARA NewSpotLightImageSet;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewParallelSet;
extern NEWCHARA NewStingerFlag;
extern NEWCHARA NewForeach;
extern NEWCHARA NewMapAbout;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewShowMap;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewReflectionWaterSurface2Set;
extern NEWCHARA UTL_EFT_InitBound;
extern NEWCHARA NewWallScarBase;
extern NEWCHARA NewBloodWall;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewBloodCamera;
extern NEWCHARA NewDelay;
extern NEWCHARA NewDebugRouteViewer;
extern NEWCHARA NewPutObject;
extern NEWCHARA NewSystemCallback;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewDynamicLightPosManager;
extern NEWCHARA NewPadCancel;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewAttachment_Server;
extern NEWCHARA NewRopeModel2;
extern NEWCHARA NewAmbientSet;
extern NEWCHARA NewFogSet;
extern NEWCHARA NewZman;
extern NEWCHARA NewEnbControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewNearFocusEffectSet;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA NewShadowControl;
extern NEWCHARA NewWigTexReplaceActorScn;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewScrWater;
extern NEWCHARA NewIPUDisplayPanelSet;
extern NEWCHARA NewProjectorLight;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewBloodSpread;
extern NEWCHARA NewCom_AddRenameList;
extern NEWCHARA NewSKLightEff;
extern NEWCHARA NewFogSet_Demo_0000Launch;
extern NEWCHARA NewFarFocusEffect_0001Launch;
extern NEWCHARA NewNearFocusEffect_0002Launch;
extern NEWCHARA DM_ChangeAmbient_0006Launch;
extern NEWCHARA DM_ChangeParallel_0007Launch;
extern NEWCHARA NewFadeInOutForce_Demo_0008Launch;
extern NEWCHARA New2DSprite_Prog_000dLaunch;
extern NEWCHARA NewLensFlare_Demo_000fLaunch;
extern NEWCHARA NewBlur_Demo_0011Launch;
extern NEWCHARA NewCrossFadeEffectCustom_0013Launch;
extern NEWCHARA NewVtrSight_0017Launch;
extern NEWCHARA NewEneEquip_1002Launch;
extern NEWCHARA NewPutAttachments_1006Launch;
extern NEWCHARA NewPutAttachments_100cLaunch;
extern NEWCHARA NewEyeControl_100dLaunch;
extern NEWCHARA NewDemoArkms_100fLaunch;
extern NEWCHARA NewArmsEffectControl_1010Launch;
extern NEWCHARA NewCreateEquipment_1012Launch;
extern NEWCHARA NewRopeModel3_called_3000Launch;
extern NEWCHARA NewRopeModel3_called_3004Launch;
extern NEWCHARA NewWavingClothModelW_called_3010Launch;
extern NEWCHARA NewCergeiEri_demo_3012Launch;
extern NEWCHARA NewPutSTanimeObjectCall_4000Launch;
extern NEWCHARA NewGeneralSprite2_6011Launch;
extern NEWCHARA DM_SendEffectMessage_8000Launch;
extern NEWCHARA DM_DebugPrint_fff00Launch;
extern NEWCHARA DM_ExecProc_fff01Launch;
extern NEWCHARA NewDemoFrameCountCall_fff03Launch;
extern NEWCHARA NewDemoEffectInitialize_fff05Launch;
/* 93 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command スティンガーサイト性能 */
		{ 0x0000D1C5, NewStingerSightFlag },
	/* chara 装備品セット */
		{ 0x000381AC, NewCreateEquipmentScn },
	/* chara 影投影モデル */
		{ 0x0005C815, NewShadowDrawObjectSet },
	/* command 弾痕ノーマル */
		{ 0x000ACF09, NewEntryWallScar },
	/* command デモモデルリネーム追加 */
		{ 0x000F57BA, NewCom_AddRenameList },
	/* command ファイル交換処理 */
		{ 0x0017A315, PL_COM_ExchangeCache },
	/* chara レーダー */
		{ 0x001AF92A, NewRadar },
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
	/* chara プットスポットライトオブジェ */
		{ 0x00365DEE, NewPutSpotLightObject },
	/* command rand */
		{ 0x003A9224, NewRand },
	/* chara ドッグタグ名前表示マネージャ */
		{ 0x003C56AA, NewNamePrintManager },
	/* command 跳弾ノーマル */
		{ 0x0041CF4E, NewEntryNewSpark },
	/* command プラグイン姿勢制御 */
		{ 0x00469B3A, NewPluginStance },
	/* chara スポットライト背景イメージ指定 */
		{ 0x0048F8F8, NewSpotLightImageSet },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* chara 平行光 */
		{ 0x004FE997, NewParallelSet },
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
	/* chara キラキラ水面２ */
		{ 0x0060CF19, NewReflectionWaterSurface2Set },
	/* command エフェクトバウンド初期化 */
		{ 0x006263F9, UTL_EFT_InitBound },
	/* chara 弾痕 */
		{ 0x0064AFEF, NewWallScarBase },
	/* chara 壁血 */
		{ 0x00683BCC, NewBloodWall },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* chara カメラ血 */
		{ 0x006B8FE4, NewBloodCamera },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* chara ルート表示 */
		{ 0x0077C494, NewDebugRouteViewer },
	/* chara プットオブジェ */
		{ 0x007A8EC1, NewPutObject },
	/* command システムコールバック */
		{ 0x0080B977, NewSystemCallback },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara ライト位置管理 */
		{ 0x0083D857, NewDynamicLightPosManager },
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
	/* chara フォグ */
		{ 0x00A53AA7, NewFogSet },
	/* chara Ｚフォーカス管理 */
		{ 0x00A63D3B, NewZman },
	/* chara 全薬莢コントロール */
		{ 0x00B030E4, NewEnbControl },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara 近景ぼかし */
		{ 0x00BBAD24, NewNearFocusEffectSet },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* chara 影管理 */
		{ 0x00BD400B, NewShadowControl },
	/* chara カツラテクスチャ差し替え管理 */
		{ 0x00BD4C61, NewWigTexReplaceActorScn },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* chara 水中カメラ */
		{ 0x00D16C76, NewScrWater },
	/* chara ＩＰＵ表示パネル */
		{ 0x00DC2F1B, NewIPUDisplayPanelSet },
	/* chara プロジェクタライト */
		{ 0x00E4D016, NewProjectorLight },
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
	/* EXTERN NewFogSet_Demo */
		{ 0x01000000, NewFogSet_Demo_0000Launch },
	/* EXTERN NewFarFocusEffect */
		{ 0x01000001, NewFarFocusEffect_0001Launch },
	/* EXTERN NewNearFocusEffect */
		{ 0x01000002, NewNearFocusEffect_0002Launch },
	/* EXTERN DM_ChangeAmbient */
		{ 0x01000006, DM_ChangeAmbient_0006Launch },
	/* EXTERN DM_ChangeParallel */
		{ 0x01000007, DM_ChangeParallel_0007Launch },
	/* EXTERN NewFadeInOutForce_Demo */
		{ 0x01000008, NewFadeInOutForce_Demo_0008Launch },
	/* EXTERN New2DSprite_Prog */
		{ 0x0100000D, New2DSprite_Prog_000dLaunch },
	/* EXTERN NewLensFlare_Demo */
		{ 0x0100000F, NewLensFlare_Demo_000fLaunch },
	/* EXTERN NewBlur_Demo */
		{ 0x01000011, NewBlur_Demo_0011Launch },
	/* EXTERN NewCrossFadeEffectCustom */
		{ 0x01000013, NewCrossFadeEffectCustom_0013Launch },
	/* EXTERN NewVtrSight */
		{ 0x01000017, NewVtrSight_0017Launch },
	/* EXTERN NewEneEquip */
		{ 0x01001002, NewEneEquip_1002Launch },
	/* EXTERN NewPutAttachments */
		{ 0x01001006, NewPutAttachments_1006Launch },
	/* EXTERN NewPutAttachments */
		{ 0x0100100C, NewPutAttachments_100cLaunch },
	/* EXTERN NewEyeControl */
		{ 0x0100100D, NewEyeControl_100dLaunch },
	/* EXTERN NewDemoArkms */
		{ 0x0100100F, NewDemoArkms_100fLaunch },
	/* EXTERN NewArmsEffectControl */
		{ 0x01001010, NewArmsEffectControl_1010Launch },
	/* EXTERN NewCreateEquipment */
		{ 0x01001012, NewCreateEquipment_1012Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003000, NewRopeModel3_called_3000Launch },
	/* EXTERN NewRopeModel3_called */
		{ 0x01003004, NewRopeModel3_called_3004Launch },
	/* EXTERN NewWavingClothModelW_called */
		{ 0x01003010, NewWavingClothModelW_called_3010Launch },
	/* EXTERN NewCergeiEri_demo */
		{ 0x01003012, NewCergeiEri_demo_3012Launch },
	/* EXTERN NewPutSTanimeObjectCall */
		{ 0x01004000, NewPutSTanimeObjectCall_4000Launch },
	/* EXTERN NewGeneralSprite2 */
		{ 0x01006011, NewGeneralSprite2_6011Launch },
	/* EXTERN DM_SendEffectMessage */
		{ 0x01008000, DM_SendEffectMessage_8000Launch },
	/* EXTERN DM_DebugPrint */
		{ 0x010FFF00, DM_DebugPrint_fff00Launch },
	/* EXTERN DM_ExecProc */
		{ 0x010FFF01, DM_ExecProc_fff01Launch },
	/* EXTERN NewDemoFrameCountCall */
		{ 0x010FFF03, NewDemoFrameCountCall_fff03Launch },
	/* EXTERN NewDemoEffectInitialize */
		{ 0x010FFF05, NewDemoEffectInitialize_fff05Launch },
/* gcl chara end */
	{ 0, NULL }
};
