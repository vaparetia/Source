//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/******************************************************************************
 * vr_pause - vr_map_3d.c
 * ＶＲ全体マップ表示３Ｄ  *NewVRMap3D
 * 2002/05/27 S.Yamashita
 * $Id: vr_map_3d.c,v 1.1.1.3 2002/11/19 11:51:43 Yoshizawa1 Exp $
 */

/******************************************************************************
 * include
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libdg.cnf"
#include "libmt.h"
#include "gameheader.h"
#include "def_dma.h"
#include "libutl.h"

#include "debugmenu.h"

#include "../../mode/codec/codecmem.h"
#include "../../sigeno/vr/vr.h"
#include "msn.h"
#include "layoutman.h"
#include "vr_def.h"
#include "vr_pause2.h"

#include "vr_map_3d.h"
#include "vr_world_map_layout.h"

#include "BP_EndianSupport.h"
#include "BP_Camera.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

#include "mode/menu/xtextscn.h"

#define __CHARA_NAME__ "VR Map 3D"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/******************************************************************************
 * define
 */

#define FLAG_SIGHT_SET      (0x0001)
#define FLAG_MENU_RADIO_SET (0x0002)
#define FLAG_LAYOUT_FAILED  (0x0004)
#define FLAG_CHILD_START    (0x0008)

#define MAP_CHANL   (2)
#define MAP_DG_FLAG (DG_FLAG_INVISIBLE | DG_FLAG_INVISIBLE3 | DG_FLAG_ONEPIECE | DG_FLAG_NOFOG)

#define FONT_WIDTH (16)
#define FONT_HEIGHT (17)
#define SYS_FONT_WIDTH  (   24)
#define SYS_FONT_HEIGHT (   24)
#define STAGE_FONT_X ( 46 )
#define STAGE_FONT_Y ( 50 )
#define SK_R            (110)
#define SK_G            (130)
#define SK_B            (120)
#define SK_A            (128)

#define MAP_AMB_ACT_R   (80)
#define MAP_AMB_ACT_G   (90)
#define MAP_AMB_ACT_B   (88)
#define MAP_AMB_INACT_R (0)
#define MAP_AMB_INACT_G (0)
#define MAP_AMB_INACT_B (32)
#define MAP_AMB_PLY_R   (178)
#define MAP_AMB_PLY_G   (8)
#define MAP_AMB_PLY_B   (58)
#define MAP_COLOR_R     (90)
#define MAP_COLOR_G     (120)
#define MAP_COLOR_B     (24)
#define MAP_LIGHT_X     (-1.0F)
#define MAP_LIGHT_Y     (-5.0F)
#define MAP_LIGHT_Z     (-1.0F)
#define MAP_LIGHT_ANIM_TIME ( DIRECT_TICK( 300 ) )
#define MAP_FADE_TIME     ( DIRECT_TICK( 75 ) )
#define MAP_FADE_MAX      104

#define DEFAULT_ROT_X_TANKER  (    0)
#define DEFAULT_ROT_Y_TANKER  (    0)
#define DEFAULT_ROT_X         (  -64)
#define DEFAULT_ROT_Y         ( -384)
#define DEFAULT_ROT_X_ARSENAL (  -64)
#define DEFAULT_ROT_Y_ARSENAL ( -384)

#define MAP_OBJ_X            ( 300000.0F)
#define MAP_OBJ_Y            ( 300000.0F)
#define MAP_OBJ_Z            (-300000.0F)
#define MAP_CAMERA_X_TANKER  (11000.0F + MAP_OBJ_X)
#define MAP_CAMERA_Y_TANKER  (17000.0F + MAP_OBJ_Y)
#define MAP_CAMERA_Z_TANKER  (23000.0F + MAP_OBJ_Z)
#define MAP_LOOKAT_X_TANKER  ( 1000.0F + MAP_OBJ_X)
#define MAP_LOOKAT_Y_TANKER  (    0.0F + MAP_OBJ_Y)
#define MAP_LOOKAT_Z_TANKER  ( 3000.0F + MAP_OBJ_Z)
#define MAP_CAMERA_X         ( -700.0F + MAP_OBJ_X)
#define MAP_CAMERA_Y         (14800.0F + MAP_OBJ_Y)
#define MAP_CAMERA_Z         (20000.0F + MAP_OBJ_Z)
#define MAP_LOOKAT_X         ( -700.0F + MAP_OBJ_X)
#define MAP_LOOKAT_Y         ( -200.0F + MAP_OBJ_Y)
#define MAP_LOOKAT_Z         (    0.0F + MAP_OBJ_Z)
#define MAP_CAMERA_X_ARSENAL ( -700.0F + MAP_OBJ_X)
#define MAP_CAMERA_Y_ARSENAL (14800.0F + MAP_OBJ_Y)
#define MAP_CAMERA_Z_ARSENAL (20000.0F + MAP_OBJ_Z)
#define MAP_LOOKAT_X_ARSENAL ( -700.0F + MAP_OBJ_X)
#define MAP_LOOKAT_Y_ARSENAL ( -200.0F + MAP_OBJ_Y)
#define MAP_LOOKAT_Z_ARSENAL (    0.0F + MAP_OBJ_Z)

#define MAP_CAMERA_ZOOM_TANKER   (2.5F)
#define MAP_CAMERA_ZOOM          (2.5F)
#define MAP_CAMERA_ZOOM_ARSENAL  (2.2F)

/******************************************************************************
 * work
 */

/* モデル情報 */
typedef struct
{
	DG_DEF  *def;		/* モデル定義 */
	DG_OBJS *objs;		/* モデル */
	int     queue;		/* 状態 */
	int     strcode;	/* 文字列コード */
} MODEL_SET;

/* フォント情報 */
typedef struct
{
	int  r;			/* 色Ｒ */
	int  g;			/* 色Ｇ */
	int  b;			/* 色Ｂ */
	int  a;			/* 色Ａ */
	void *work;		/* ワーク */
} Font;

/* ワーク */
typedef struct Work
{
	GV_ACT_EX    actor;								/* アクター */
	int          name;								/* シナリオ名 */
	int          flag;								/* フラグ */
	int          mode;								/* 動作モード */
	int          model_cnt;							/* モデル数 */
	MODEL_SET    *models;							/* モデル情報 */
	float        rot_x;								/* モデルのＸ軸回転 */
	float        rot_y;								/* モデルのＹ軸回転 */
	FMATRIX      light_active[2];					/* 通常アクティブ光源 */
	FMATRIX      light_inactive[2];					/* 非アクティブ光源 */
	FMATRIX      light_player[2];					/* 現在位置光源 */
	int          light_cnt;							/* ライトアニメーションのカウンター */
	float        map_zoom;							/* カメラズーム */

	int          active_stage;						/* アクティブステージ */
	int          bomb_stage;						/* 爆弾ステージ */

	Font         stage_font;						/* フォント情報 */
	SPR_OBJ      *fade;								/* フェードスプライト */
	int          fade_cnt;							/* フェードイン・アウトのカウンター */
	Pos          pos;								/* 位置情報 */

	void         (*act_func)(struct Work * work);	/* 実行 Act 関数 */
	int          step;								/* 処理ステップ */

	int          ch_flag;							
	int          ch_width;							
	int          ch_height;							
	int          ch_bg_clear_flag;					
} Work;

/*******************************************************************************
 * local
 */

static int  VRMap3D_BombStage = 0;	/* 爆弾ステージ */

#ifdef DEBUG_MODE
static int  allmap_flag        = 1;
static char *debugmenu_items[] = {"ON", "OFF"};
static int  debugmenu_values[] = {   1,     0};

static GM_DEBUG_MENU debug_menu = {
	NULL,
	".VRMAP",
	"DISPLAY",
	debugmenu_items,
	debugmenu_values,
	(int *)&allmap_flag,	/* target */
	0x01,					/* mask */
	NULL,					/* func */
	0,						/* strid */
	0,						/* type */
	2,						/* max */
	0,0
};
#endif	/* DEBUG_MODE */

/******************************************************************************
 * extern
 */

extern void *NewTextScreenControlEx(int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag);
extern void MENU_ClearTextTexture(void *work);
extern void MENU_PutTextScreen(void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col);
extern void MENU_PutTextScreenF(void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col);
extern void TS_SubWindowSleep(void);
extern void TS_SubWindowWake(void);
extern void GM_JimakuHide(void);

/******************************************************************************
 * static
 */

static void act_pause(Work *work);
static void act_waiting(Work *work);
static void set_ambient(FMATRIX *mat, int r, int g, int b);
static void change_light_matrix(DG_OBJS *objs, FMATRIX *mat);

/******************************************************************************
 * static
 */
/******************************************************************************
 * シグナル処理
 */
static int SignalFunc(
	void *work,
	int  signal,
	int  value)
{
	Work *pWork;

	pWork = (Work *)work;

	switch(signal)
	{
	case VRMAP_CHILD_END:
		pWork->flag &= ~FLAG_CHILD_START;
		break;

	case GV_SIGNAL_KILL:
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor(work);
		return -1;
	}

	return 0;
}

/******************************************************************************
 * 文字列コードからモードを取得
 * 文字列コードから列挙番号へ変換
 */
#define J_RES_START ( 62)	/* 全体マップ用ステージ名日本語リソースの開始位置 */
#define E_RES_START (114)	/* 全体マップ用ステージ名英語リソースの開始位置 */

static int GetModeFromStrcode(	/* モード */
	int strcode)	/* 文字列コード */
{
	SY_PRINTF3("GetModeFromStrcode\n");

	switch(strcode)
	{
	case TANKER_BODY:
	case TANKER_00A :	/* 船尾甲板 */
	case TANKER_01A :	/* 船橋一階居住区 */
	case TANKER_01F :	/* 船橋一階リフレッシュルーム */
	case TANKER_01B :	/* 船橋二階居住区 */
	case TANKER_01C :	/* 船橋三階居住区 */
	case TANKER_01D :	/* 船橋四階居住区 */
	case TANKER_01E :	/* 船橋五階操舵室 */
	case TANKER_00B :	/* 航海甲板ウィング */
	case TANKER_02A :	/* 機関室 */
	case TANKER_03B :	/* 第二甲板右舷 */
	case TANKER_03A :	/* 第二甲板左舷 */
	case TANKER_03_O:
	case TANKER_04A :	/* 第一船倉 */
	case TANKER_04B :	/* 第二船倉 */
	case TANKER_04C :	/* 第三船倉 */
		return SK_TANKER;

	case PLANT_A    :	/* Ａ脚ポンプ施設 */
	case PLANT_AB   :	/* ＡＢ連絡橋 */
	case PLANT_B    :	/* Ｂ脚変電室 */
	case PLANT_BC   :	/* ＢＣ連絡橋 */
	case PLANT_C    :	/* Ｃ脚食堂 */
	case PLANT_CD   :	/* ＣＤ連絡橋 */
	case PLANT_D    :	/* Ｄ脚第一沈殿池 */
	case PLANT_DE   :	/* ＤＥ連絡橋 */
	case PLANT_E    :	/* Ｅ脚貨物集配施設 */
	case PLANT_EF   :	/* ＥＦ連絡橋 */
	case PLANT_F    :	/* Ｆ脚倉庫 */
	case PLANT_FA   :	/* ＦＡ連絡橋 */
	case PLANT_S1   :	/* シェル１中央棟 */
	case PLANT_DG   :	/* シェル１シェル２連絡橋 */
	case PLANT_G    :	/* Ｇ脚油処理施設 */
	case PLANT_GH   :	/* ＧＨ連絡橋 */
	case PLANT_H    :	/* Ｈ脚倉庫 */
	case PLANT_HI   :	/* ＨＩ連絡橋 */
	case PLANT_I    :	/* Ｉ脚機材組立施設 */
	case PLANT_IJ   :	/* ＩＪ連絡橋 */
	case PLANT_J    :	/* Ｊ脚発電施設 */
	case PLANT_JK   :	/* ＪＫ連絡橋*/
	case PLANT_K    :	/* Ｋ脚生物化学実験施設 */
	case PLANT_KL   :	/* ＫＬ連絡橋 */
	case PLANT_L    :	/* Ｌ脚汚水処理場 */
	case PLANT_LG   :	/* ＬＧ連絡橋 */
	case PLANT_S2   :	/* シェル２中央棟 */
	case PLANT_L2   :	/* Ｌ脚下部オイルフェンス */
	case PLANT_OTHER:	/* Ｈ脚下部オイルフェンス */
		return SK_PLANT;

	case ARSENAL_41 :	/* アーセナルギア胃' */
	case ARSENAL_42 :	/* アーセナルギア空腸 */
	case ARSENAL_43 :	/* アーセナルギア上行結腸 */
	case ARSENAL_44 :	/* アーセナルギア回腸 */
	case ARSENAL_45 :	/* アーセナルギアＳ状結腸 */
	case ARSENAL_46 :	/* アーセナルギア直腸 */
		return SK_ARSENAL;

	default         :
		return SK_OTHER;
	}

	ASSERT(0);
	return 0;
}

static int StrcodeToEnumJ(	/* 列挙番号 */
	int strcode)	/* 文字列コード */
{
	SY_PRINTF3("StrcodeToEnumJ\n");

	switch(strcode)
	{
	case TANKER_BODY: break;
	case TANKER_00A : return (J_RES_START +  0); 	/* 船尾甲板 */
	case TANKER_01A : return (J_RES_START +  1); 	/* 船橋一階居住区 */
	case TANKER_01F : return (J_RES_START +  2); 	/* 船橋一階リフレッシュルーム */
	case TANKER_01B : return (J_RES_START +  3); 	/* 船橋二階居住区 */
	case TANKER_01C : return (J_RES_START +  4); 	/* 船橋三階居住区 */
	case TANKER_01D : return (J_RES_START +  5); 	/* 船橋四階居住区 */
	case TANKER_01E : return (J_RES_START +  6); 	/* 船橋五階操舵室 */
	case TANKER_00B : return (J_RES_START +  7); 	/* 航海甲板ウィング */
	case TANKER_02A : return (J_RES_START +  8); 	/* 機関室 */
	case TANKER_03B : return (E_RES_START + 53);	/* 第二甲板右舷 */
	case TANKER_03A : return (E_RES_START + 54);	/* 第二甲板左舷 */
	case TANKER_03_O: break;
	case TANKER_04A : return (J_RES_START + 10); 	/* 第一船倉 */
	case TANKER_04B : return (J_RES_START + 11); 	/* 第二船倉 */
	case TANKER_04C : return (J_RES_START + 12); 	/* 第三船倉 */

	case PLANT_A    : return (J_RES_START + 13); 	/* Ａ脚ポンプ施設 */
	case PLANT_AB   : return (J_RES_START + 14); 	/* ＡＢ連絡橋 */
	case PLANT_B    : return (J_RES_START + 15); 	/* Ｂ脚変電室 */
	case PLANT_BC   : return (J_RES_START + 16); 	/* ＢＣ連絡橋 */
	case PLANT_C    : return (J_RES_START + 17); 	/* Ｃ脚食堂 */
	case PLANT_CD   : return (J_RES_START + 18); 	/* ＣＤ連絡橋 */
	case PLANT_D    : return (J_RES_START + 19); 	/* Ｄ脚第一沈殿池 */
	case PLANT_DE   : return (J_RES_START + 20); 	/* ＤＥ連絡橋 */
	case PLANT_E    : return (J_RES_START + 21); 	/* Ｅ脚貨物集配施設 */
	case PLANT_EF   : return (J_RES_START + 22); 	/* ＥＦ連絡橋 */
	case PLANT_F    : return (J_RES_START + 23); 	/* Ｆ脚倉庫 */
	case PLANT_FA   : return (J_RES_START + 24); 	/* ＦＡ連絡橋 */
	case PLANT_S1   : return (J_RES_START + 25); 	/* シェル１中央棟 */
	case PLANT_DG   : return (J_RES_START + 26); 	/* シェル１シェル２連絡橋 */
	case PLANT_G    : return (J_RES_START + 27); 	/* Ｇ脚油処理施設 */
	case PLANT_GH   : return (J_RES_START + 28); 	/* ＧＨ連絡橋 */
	case PLANT_H    : return (J_RES_START + 29); 	/* Ｈ脚倉庫 */
	case PLANT_HI   : return (J_RES_START + 30); 	/* ＨＩ連絡橋 */
	case PLANT_I    : return (J_RES_START + 31); 	/* Ｉ脚機材組立施設 */
	case PLANT_IJ   : return (J_RES_START + 32); 	/* ＩＪ連絡橋 */
	case PLANT_J    : return (J_RES_START + 33); 	/* Ｊ脚発電施設 */
	case PLANT_JK   : return (J_RES_START + 34); 	/* ＪＫ連絡橋*/
	case PLANT_K    : return (J_RES_START + 35); 	/* Ｋ脚生物化学実験施設 */
	case PLANT_KL   : return (J_RES_START + 36); 	/* ＫＬ連絡橋 */
	case PLANT_L    : return (J_RES_START + 37); 	/* Ｌ脚汚水処理場 */
	case PLANT_LG   : return (J_RES_START + 38); 	/* ＬＧ連絡橋 */
	case PLANT_S2   : return (J_RES_START + 39); 	/* シェル２中央棟 */
	case PLANT_L2   : return (J_RES_START + 40); 	/* Ｌ脚下部オイルフェンス */
	case PLANT_OTHER: return (J_RES_START + 41); 	/* Ｈ脚下部オイルフェンス */

	case ARSENAL_41 : return (J_RES_START + 42); 	/* アーセナルギア胃' */
	case ARSENAL_42 : return (J_RES_START + 43); 	/* アーセナルギア空腸 */
	case ARSENAL_43 : return (J_RES_START + 44); 	/* アーセナルギア上行結腸 */
	case ARSENAL_44 : return (J_RES_START + 45); 	/* アーセナルギア回腸 */
	case ARSENAL_45 : return (J_RES_START + 46); 	/* アーセナルギアＳ状結腸 */
	case ARSENAL_46 : return (J_RES_START + 47); 	/* アーセナルギア直腸 */

	default         : return 0xff;
	}

	ASSERT(0);
	return 0;
}

static int StrcodeToEnumE(	/* 列挙番号 */
	int strcode)	/* 文字列コード */
{
	SY_PRINTF3("StrcodeToEnumE\n");

	switch(strcode)
	{
	case TANKER_BODY: break;
	case TANKER_00A : return (E_RES_START + 39); 	/* 船尾甲板 */
	case TANKER_01A : return (E_RES_START + 40); 	/* 船橋一階居住区 */
	case TANKER_01F : return (E_RES_START + 41); 	/* 船橋一階リフレッシュルーム */
	case TANKER_01B : return (E_RES_START + 42); 	/* 船橋二階居住区 */
	case TANKER_01C : return (E_RES_START + 43); 	/* 船橋三階居住区 */
	case TANKER_01D : return (E_RES_START + 44); 	/* 船橋四階居住区 */
	case TANKER_01E : return (E_RES_START + 45); 	/* 船橋五階操舵室 */
	case TANKER_00B : return (E_RES_START + 46); 	/* 航海甲板ウィング */
	case TANKER_02A : return (E_RES_START + 47); 	/* 機関室 */
	case TANKER_03B : return (E_RES_START + 48); 	/* 第二甲板右舷 */
	case TANKER_03A : return (E_RES_START + 49); 	/* 第二甲板左舷 */
	case TANKER_03_O: break;
	case TANKER_04A : return (E_RES_START + 50); 	/* 第一船倉 */
	case TANKER_04B : return (E_RES_START + 51); 	/* 第二船倉 */
	case TANKER_04C : return (E_RES_START + 52); 	/* 第三船倉 */

	case PLANT_A    : return (E_RES_START +  0); 	/* Ａ脚ポンプ施設 */
	case PLANT_AB   : return (E_RES_START +  1); 	/* ＡＢ連絡橋 */
	case PLANT_B    : return (E_RES_START +  2); 	/* Ｂ脚変電室 */
	case PLANT_BC   : return (E_RES_START +  3); 	/* ＢＣ連絡橋 */
	case PLANT_C    : return (E_RES_START +  4); 	/* Ｃ脚食堂 */
	case PLANT_CD   : return (E_RES_START +  5); 	/* ＣＤ連絡橋 */
	case PLANT_D    : return (E_RES_START +  6); 	/* Ｄ脚第一沈殿池 */
	case PLANT_DE   : return (E_RES_START +  7); 	/* ＤＥ連絡橋 */
	case PLANT_E    : return (E_RES_START +  8); 	/* Ｅ脚貨物集配施設 */
	case PLANT_EF   : return (E_RES_START +  9); 	/* ＥＦ連絡橋 */
	case PLANT_F    : return (E_RES_START + 10); 	/* Ｆ脚倉庫 */
	case PLANT_FA   : return (E_RES_START + 11); 	/* ＦＡ連絡橋 */
	case PLANT_S1   : return (E_RES_START + 12); 	/* シェル１中央棟 */
	case PLANT_DG   : return (E_RES_START + 13); 	/* シェル１シェル２連絡橋 */
	case PLANT_G    : return (E_RES_START + 14); 	/* Ｇ脚油処理施設 */
	case PLANT_GH   : return (E_RES_START + 15); 	/* ＧＨ連絡橋 */
	case PLANT_H    : return (E_RES_START + 16); 	/* Ｈ脚倉庫 */
	case PLANT_HI   : return (E_RES_START + 17); 	/* ＨＩ連絡橋 */
	case PLANT_I    : return (E_RES_START + 18); 	/* Ｉ脚機材組立施設 */
	case PLANT_IJ   : return (E_RES_START + 19); 	/* ＩＪ連絡橋 */
	case PLANT_J    : return (E_RES_START + 20); 	/* Ｊ脚発電施設 */
	case PLANT_JK   : return (E_RES_START + 21); 	/* ＪＫ連絡橋*/
	case PLANT_K    : return (E_RES_START + 22); 	/* Ｋ脚生物化学実験施設 */
	case PLANT_KL   : return (E_RES_START + 23); 	/* ＫＬ連絡橋 */
	case PLANT_L    : return (E_RES_START + 24); 	/* Ｌ脚汚水処理場 */
	case PLANT_LG   : return (E_RES_START + 25); 	/* ＬＧ連絡橋 */
	case PLANT_S2   : return (E_RES_START + 26); 	/* シェル２中央棟 */
	case PLANT_L2   : return (E_RES_START + 27); 	/* Ｌ脚下部オイルフェンス */
	case PLANT_OTHER: return (E_RES_START + 28); 	/* Ｈ脚下部オイルフェンス */

	case ARSENAL_41 : return (E_RES_START + 29); 	/* アーセナルギア胃' */
	case ARSENAL_42 : return (E_RES_START + 30); 	/* アーセナルギア空腸 */
	case ARSENAL_43 : return (E_RES_START + 31); 	/* アーセナルギア上行結腸 */
	case ARSENAL_44 : return (E_RES_START + 32); 	/* アーセナルギア回腸 */
	case ARSENAL_45 : return (E_RES_START + 33); 	/* アーセナルギアＳ状結腸 */
	case ARSENAL_46 : return (E_RES_START + 34); 	/* アーセナルギア直腸 */

	default         : return 0xff;
	}

	ASSERT(0);
	return 0;
}

/******************************************************************************
 * ２次元座標の取得
 */
static void SK_TransPersOneChanl(
	FVECTOR *res,
	FVECTOR *pos,
	int     chanl)
{
	FMATRIX *eye_pers;

	eye_pers = &DG_Chanl(chanl)->eye_pers;
	pos->vw  = 1.0f;
	_sceVu0ApplyMatrix(res, eye_pers, pos);
}

static void SK_TransPersOne(
	FVECTOR *res,
	FVECTOR *pos)
{
	float w;

	SK_TransPersOneChanl(res, pos, MAP_CHANL);
	w = DG_FABS(res->vw);
	res->vx /= w;
	res->vy /= w;
	res->vz /= w;
	res->vx  = res->vx * (float)(DRAW_WIDTH  / 2) + (float)(DRAW_WIDTH  / 2);
	res->vy  = res->vy * (float)(DRAW_HEIGHT / 2) + (float)(DRAW_HEIGHT / 2);
}

/******************************************************************************
 * チャンネル状態の設定
 */
static void map_chanl_on(
	Work *work)		/* ワーク */
{
	/* 現在のチャネル状態を保存 */
	work->ch_flag          = DG_Chanls[MAP_CHANL].flag;
	work->ch_width         = DG_Chanls[MAP_CHANL].width;
	work->ch_height        = DG_Chanls[MAP_CHANL].height;
	work->ch_bg_clear_flag = DG_Chanls[MAP_CHANL].bg_clear_flag;

	/* チャンネル状態を設定 */
	DG_SetDrawEnv(&DG_Chanls[MAP_CHANL], 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
	DG_Chanls[MAP_CHANL].flag          = 1;
	DG_Chanls[MAP_CHANL].bg_clear_flag = 2;
}

static void map_chanl_off(
	Work *work)		/* ワーク */
{
	/* チャネル状態を元に戻す */
	DG_Chanls[MAP_CHANL].flag          = work->ch_flag;
	DG_Chanls[MAP_CHANL].width         = work->ch_width;
	DG_Chanls[MAP_CHANL].height        = work->ch_height;
	DG_Chanls[MAP_CHANL].bg_clear_flag = work->ch_bg_clear_flag;
}

/******************************************************************************
 * Act の設定
 */
static void change_act(
	Work *work,						/* ワーク */
	void (*act_func)(Work * work))	/* 実行 Act 関数 */
{
	work->act_func = act_func;
	work->step     = 0;
}

/******************************************************************************
 * カメラの設定
 */
static void camera_setup(
	Work *work)		/* ワーク */
{
	FVECTOR eye;
	FVECTOR base;

   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system

	switch(work->mode)
	{
	case SK_TANKER:
		eye.vx  = MAP_CAMERA_X_TANKER;
		eye.vy  = MAP_CAMERA_Y_TANKER;
		eye.vz  = MAP_CAMERA_Z_TANKER;
		eye.vw  = 0.0F;
		base.vx = MAP_LOOKAT_X_TANKER;
		base.vy = MAP_LOOKAT_Y_TANKER;
		base.vz = MAP_LOOKAT_Z_TANKER;
		base.vw = 0.0f;

		DG_SetCamera2(&DG_Chanls[MAP_CHANL], &eye, &base, work->map_zoom);
		break;

	case SK_PLANT:
	default:
		eye.vx  = MAP_CAMERA_X;
		eye.vy  = MAP_CAMERA_Y;
		eye.vz  = MAP_CAMERA_Z;
		eye.vw  = 0.0F;
		base.vx = MAP_LOOKAT_X;
		base.vy = MAP_LOOKAT_Y;
		base.vz = MAP_LOOKAT_Z;
		base.vw = 0.0f;

		DG_SetCamera2(&DG_Chanls[MAP_CHANL], &eye, &base, work->map_zoom);
		break;

	case SK_ARSENAL:
		eye.vx  = MAP_CAMERA_X_ARSENAL;
		eye.vy  = MAP_CAMERA_Y_ARSENAL;
		eye.vz  = MAP_CAMERA_Z_ARSENAL;
		eye.vw  = 0.0F;
		base.vx = MAP_LOOKAT_X_ARSENAL;
		base.vy = MAP_LOOKAT_Y_ARSENAL;
		base.vz = MAP_LOOKAT_Z_ARSENAL;
		base.vw = 0.0f;

		DG_SetCamera2(&DG_Chanls[MAP_CHANL], &eye, &base, work->map_zoom);
		break;
	}
}

/******************************************************************************
 * モデルの表示準備
 */
static void model_setup(
	Work *work)		/* ワーク */
{
	int i;

	for(i = 0; i < work->model_cnt; i++)
	{
		DG_QueueObjs(work->models[i].objs);
		work->models[i].queue = 1;

		/* 表示グループを、マップオブジェクトと同じものにする */
		DG_AddCurrentGroup2(MAP_CHANL, work->models[i].objs->group_id);
	}
}

/******************************************************************************
 * フォントの色の決定
 */
static inline int FontColor(
	Work *pWork,	/* ワーク */
	int  div)		/* アルファ値を割る値 */
{
	u_int color;

	color = SK_R | (SK_G << 8) | (SK_B << 16) | ((pWork->stage_font.a / div) << 24); 

	return color;
}

/******************************************************************************
 * 文字列の表示
 */
static void StringDisp(
	Work *pWork)	/* ワーク */
{
	int   disp_width; 
	int   poly_width;
	float x1 , y1 , x2 , y2 , u1 , v1;
	u_int color;

	disp_width = 17 * FONT_WIDTH;
	poly_width = 16 * SYS_FONT_WIDTH;

	x1 = (float)(STAGE_FONT_X);
	y1 = (float)(STAGE_FONT_Y + FONT_HEIGHT - 4);
	x2 = (float)(x1 + disp_width);
	y2 = (float)(y1 + FONT_HEIGHT + 4);
	u1 = (float)poly_width;
	v1 = (float)SYS_FONT_HEIGHT;
	color = FontColor(pWork, 1); MENU_PutTextScreenF(pWork->stage_font.work, x1       , y1       , x2       , y2       , 1, 0, (int)u1 + 1, (int)v1 + 4, color);
	color = FontColor(pWork, 2); MENU_PutTextScreenF(pWork->stage_font.work, x1 + 0.5f, y1 + 0.5f, x2 + 0.5f, y2 + 0.5f, 1, 0, (int)u1 + 1, (int)v1 + 4, color);

	y1 = (float)(STAGE_FONT_Y);
	x2 = (float)(STAGE_FONT_X + (22 * (FONT_WIDTH - 3)));
	y2 = (float)(y1 + FONT_HEIGHT - 3);
	u1 = (float)(22 * SYS_FONT_WIDTH);
	color = FontColor(pWork, 1); MENU_PutTextScreenF(pWork->stage_font.work, x1       , y1       , x2       , y2       , 1, (int)v1 + 6, (int)u1 + 1, ((int)v1 * 2) + 6, color);
	color = FontColor(pWork, 2); MENU_PutTextScreenF(pWork->stage_font.work, x1 + 0.5f, y1 + 0.5f, x2 + 0.5f, y2 + 0.5f, 1, (int)v1 + 6, (int)u1 + 1, ((int)v1 * 2) + 6, color);
}

/******************************************************************************
 * 文字列の設定
 */
static void StringCreate(
	Work *pWork)	/* ワーク */
{
	int  enum_num;
	int  poly_width;
	void *ptr;

	if(pWork->pos.position == -1)
		return;

	if(pWork->pos.position == -2) enum_num = StrcodeToEnumJ(ARSENAL_46);
	else                          enum_num = StrcodeToEnumJ(pWork->models[pWork->pos.position].strcode);
	if(enum_num == 0xff)
		return;

	poly_width = 16 * SYS_FONT_WIDTH;
	ptr = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource(6, enum_num));
	MENU_CreateTextTexture(pWork->stage_font.work, 1, 0, poly_width, SYS_FONT_HEIGHT + 6, 0, 6, 0, ptr);

	if(pWork->pos.position == -2) enum_num = StrcodeToEnumE(ARSENAL_46);
	else                          enum_num = StrcodeToEnumE(pWork->models[pWork->pos.position].strcode);
	if(enum_num == 0xff)
		return;

	poly_width = 22 * SYS_FONT_WIDTH;
	ptr = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource(6, enum_num));
	MENU_CreateTextTexture(pWork->stage_font.work, 1, 4 + SYS_FONT_HEIGHT, poly_width, SYS_FONT_HEIGHT, 0, 0, 0, ptr);
}

/******************************************************************************
 * 
 */
static void PosBombUpdate(
	Work *work)
{
	FVECTOR pos;
	FVECTOR tmp;
	int     i;
	int     x_plus, y_plus, z_plus;

	for(i = 0; i < work->model_cnt; i++)
	{
		if(work->bomb_stage & (0x1 << i))
		{
			/* モデルのマトリクスを格納 */
			DG_SetPos(&work->models[i].objs->world);

			/* 中心座標の取得 */
			tmp.vx  = work->models[i].def->lx - work->models[i].def->ux;
			tmp.vy  = work->models[i].def->ly - work->models[i].def->uy;
			tmp.vz  = work->models[i].def->lz - work->models[i].def->uz;
			_sceVu0DivVector(&tmp, &tmp, 2.0f);

			/* 位置修正 */
			if(work->mode == SK_TANKER)
			{
				switch(i)
				{
				case  0: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_body      */
				case  1: x_plus =     0; y_plus =     0; z_plus =  1200; break;	/* map_tanker_00a       船尾甲板 */
				case  2: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_01a       船橋一階居住区 */
				case  3: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_01f       船橋一階リフレッシュルーム */
				case  4: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_01b       船橋二階居住区 */
				case  5: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_01c       船橋三階居住区 */
				case  6: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_01d       船橋四階居住区 */
				case  7: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_01e       船橋五階操舵室 */
				case  8: x_plus =   100; y_plus =     0; z_plus =  -200; break;	/* map_tanker_00b       航海甲板ウィング */
				case  9: x_plus =   100; y_plus =  -300; z_plus =   700; break;	/* map_tanker_02a       機関室 */
				case 10: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_03b       第二甲板右舷 */
				case 11: x_plus = -1600; y_plus =     0; z_plus =     0; break;	/* map_tanker_03a       第二甲板左舷 */
				case 12: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_03_other  */
				case 13: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_04a       第一船倉 */
				case 14: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_04b       第二船倉 */
				case 15: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_tanker_04c       第三船倉 */
				default: x_plus =     0; y_plus =     0; z_plus =     0; break;
				}
			}
			else if(work->mode == SK_PLANT)
			{
				switch(i)
				{
				case  0: x_plus =   -50; y_plus =  1025; z_plus =     0; break;	/* a_w12                Ａ脚ポンプ施設 */
				case  1: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* ab_w13               ＡＢ連絡橋 */
				case  2: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* b_w14                Ｂ脚変電室 */
				case  3: x_plus =  -500; y_plus =     0; z_plus =     0; break;	/* bc_w15               ＢＣ連絡橋 */
				case  4: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* c_w16                Ｃ脚食堂 */
				case  5: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* cd_w17               ＣＤ連絡橋 */
				case  6: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* d_w18                Ｄ脚第一沈殿池 */
				case  7: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* de_w19               ＤＥ連絡橋 */
				case  8: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* e_w20                Ｅ脚貨物集配施設 */
				case  9: x_plus =   500; y_plus =     0; z_plus =     0; break;	/* ef_w21               ＥＦ連絡橋 */
				case 10: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* f_w22                Ｆ脚倉庫 */
				case 11: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* af_w23               ＦＡ連絡橋 */
				case 12: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* s1_center_w24        シェル１中央棟 */
				case 13: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* dg_w25               シェル１シェル２連絡橋 */
				case 14: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* g_w25                Ｇ脚油処理施設 */
				case 15: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* gh                   ＧＨ連絡橋 */
				case 16: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* h                    Ｈ脚倉庫 */
				case 17: x_plus =  -500; y_plus =     0; z_plus =     0; break;	/* hi                   ＨＩ連絡橋 */
				case 18: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* i                    Ｉ脚機材組立施設 */
				case 19: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* ij                   ＩＪ連絡橋 */
				case 20: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* j                    Ｊ脚発電施設 */
				case 21: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* jk                   ＪＫ連絡橋 */
				case 22: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* k                    Ｋ脚生物化学実験施設 */
				case 23: x_plus =   500; y_plus =     0; z_plus =     0; break;	/* lk_w25               ＫＬ連絡橋 */
				case 24: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* l_w25                Ｌ脚汚水処理場 */
				case 25: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* gl_w25               ＬＧ連絡橋 */
				case 26: x_plus =   -50; y_plus =   850; z_plus =     0; break;	/* s2_center_w31        シェル２中央棟 */
				case 27: x_plus =  1300; y_plus =  -500; z_plus =     0; break;	/* l_w32                Ｌ脚下部オイルフェンス */
				case 28: x_plus = -1300; y_plus =  -500; z_plus =     0; break;	/* other                Ｈ脚下部オイルフェンス */
				default: x_plus =     0; y_plus =     0; z_plus =     0; break;
				}
//				tmp.vx +=     work->models[i].def->ux + BOMB_X_PLUS;
//				tmp.vz +=     work->models[i].def->uz;
//				if(i =    = 0) tmp.vy +=     work->models[i].def->uy + BOMB_A_PLUS;
//				else       tmp.vy +=     work->models[i].def->uy + BOMB_Y_PLUS;
			}
			else if(work->mode == SK_ARSENAL)
			{
				switch(i)
				{
				case  0: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_41_mt           アーセナルギア胃 */
				case  1: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_42_mt           アーセナルギア空腸 */
				case  2: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_43_mt           アーセナルギア上行結腸 */
				case  3: x_plus =     0; y_plus =     0; z_plus =     0; break;	/* map_44_mt           アーセナルギア回腸 */
				case  4: x_plus =     0; y_plus =   800; z_plus =  -400; break;	/* map_45_mt           アーセナルギアＳ状結腸 */
				default: x_plus =     0; y_plus =     0; z_plus =     0; break;
				}
			}
			else
			{
				x_plus = 0;
				y_plus = 0;
				z_plus = 0;
			}
			tmp.vx += work->models[i].def->ux + x_plus;
			tmp.vy += work->models[i].def->uy + y_plus;
			tmp.vz += work->models[i].def->uz + z_plus;

			/* 実際の場所を求める */
			DG_RotVector(&tmp, &tmp, 1);
			_sceVu0AddVector(&tmp, &tmp, (FVECTOR *)work->models[i].objs->world.m[3]);

			/* 透視変換 */
			SK_TransPersOne(&pos , &tmp);
			pos.vx = pos.vx * 512.0f / (float)DRAW_WIDTH;
			pos.vy = pos.vy * 384.0f / (float)DRAW_HEIGHT;

			/* 格納 */
			work->pos.all_point[i].vx = pos.vx;
			work->pos.all_point[i].vy = pos.vy;
		}
	}
}

/******************************************************************************
 * モデルの回転
 */
static void model_rot(
	Work *work)		/* ワーク */
{
	int     i;
	int     dx;
	int     dy;
	int     sign;
	float   target_x;
	float   target_y;
	FVECTOR pos;
	SVECTOR rot;

	if(work->model_cnt == 0)
		return;

    /* 横方向 */
	dx = GV_PadDataDirect[0].right_dx - 128;
	sign = 1;
	if(dx < 0) sign = -1;
	if((dx * sign) <   64) dx =    0;	/* 最低値   64、-64 */
	if(dx          >  128) dx =  128;	/* 最高値  128 */
	if(dx          < -112) dx = -112;	/* 最高値 -112 */
	switch(work->mode)
	{
	case  SK_TANKER:
		target_y = (float)(dx * 10 + DEFAULT_ROT_Y_TANKER);
		break;

	case SK_PLANT:
	default:
		target_y = (float)(dx * 10 + DEFAULT_ROT_Y);
		break;

	case SK_ARSENAL:
		target_y = (float)(dx * 10 + DEFAULT_ROT_Y_ARSENAL);
		break;
	}
	work->rot_y += (target_y - work->rot_y) / 10.0F;

	/* 縦方向 */
	dy = GV_PadDataDirect[0].right_dy - 128;
	sign = 1;
	if(dy < 0) sign = -1;
	if((dy * sign) <  64) dy  =   0;	/* 最低値  64、-64 */
	if(dy          >  28) dy  =  28;	/* 最高値  28 */
	if(dy          < -10) dy  = -10;	/* 最高値 -10 */
	switch(work->mode)
	{
	case  SK_TANKER:
		target_x = (float)(dy * 20 + DEFAULT_ROT_X_TANKER);
		break;

	case SK_PLANT:
	default:
		target_x = (float)(dy * 20 + DEFAULT_ROT_X);
		break;

	case SK_ARSENAL:
		target_x = (float)(dy * 20 + DEFAULT_ROT_X_ARSENAL);
		break;
	}
	work->rot_x += (target_x - work->rot_x) / 10.0F;

	/* モデルの回転 */
	for(i = 0; i < work->model_cnt; i++)
	{
		pos.vx = MAP_OBJ_X;
		pos.vy = MAP_OBJ_Y;
		pos.vz = MAP_OBJ_Z;
		pos.vw = 1.0F;
		rot.vx = (int)work->rot_x;
		rot.vy = (int)work->rot_y;
		rot.vz = rot.pad = 0;
		DG_SetPos2(&pos , &rot);
		DG_GetPos(&work->models[i].objs->world);
	}
}

/******************************************************************************
 * 現在位置光源によるアニメーション
 */
static void player_pos_anim(
	Work *work)		/* ワーク */
{
	int rate;
	int player_r;
	int player_g;
	int player_b;

	if(    (work->pos.position == -1)
		|| (work->pos.position == -2))
		return;

	rate = (work->light_cnt += TIME_BASE);

	if(rate > MAP_LIGHT_ANIM_TIME)
	{
		rate             = MAP_LIGHT_ANIM_TIME;
		work->light_cnt -= MAP_LIGHT_ANIM_TIME;
	}

	player_r = (MAP_AMB_ACT_R - MAP_AMB_PLY_R) * rate / MAP_LIGHT_ANIM_TIME + MAP_AMB_PLY_R;
	player_g = (MAP_AMB_ACT_G - MAP_AMB_PLY_G) * rate / MAP_LIGHT_ANIM_TIME + MAP_AMB_PLY_G;
	player_b = (MAP_AMB_ACT_B - MAP_AMB_PLY_B) * rate / MAP_LIGHT_ANIM_TIME + MAP_AMB_PLY_B;

	set_ambient(work->light_player, player_r, player_g, player_b);
}

/******************************************************************************
 * 
 */
static inline void StatusReset(
	Work *pWork)	/* ワーク */
{
	int tmp;

	tmp  = GM_CheckMenuStatus(MENU_STREAM_CH_0);
	tmp |= GM_CheckMenuStatus(MENU_STREAM_CH_1);

	GM_PopGameStatus();
	GM_PopMenuStatus();

	if(tmp & MENU_STREAM_CH_0) GM_SetMenuStatus(MENU_STREAM_CH_0);
	else                       GM_ResetMenuStatus(MENU_STREAM_CH_0);
	if(tmp & MENU_STREAM_CH_1) GM_SetMenuStatus(MENU_STREAM_CH_1);
	else                       GM_ResetMenuStatus(MENU_STREAM_CH_1);

	TS_SubWindowWake();

	if(pWork->flag & FLAG_MENU_RADIO_SET)
	{
		GM_ResetMenuStatus(MENU_RADIO_DISABLE);
		pWork->flag &= ~FLAG_MENU_RADIO_SET;
	}
}

/******************************************************************************
 * ポーズ中
 */
static void act_pause(
	Work *work)		/* ワーク */
{
	enum
	{
		STEP_INIT,
		STEP_FDIN,
		STEP_PAUSE,
		STEP_FINISH,
	};

	static float zm;

	int i;
	int alpha;

	player_pos_anim(work);
	PosBombUpdate(work);

	switch(work->step)
	{
    case STEP_INIT:
		work->map_zoom = 0.0F;

		map_chanl_on(work);
		model_setup(work);
		model_rot(work);

		/* フェードイン準備 */
		work->fade->sprite.col.a = 0;
		SPR_SHOW(work->fade);
		work->fade_cnt = 0;
		if(work->pos.position > -1) change_light_matrix(work->models[work->pos.position].objs, work->light_player);

		work->step = STEP_FDIN;
		SY_PRINTF1("act_pause - STEP_FDIN\n");
		/* break 不要 */

	case STEP_FDIN:
		GM_JimakuHide();

		/* フェードイン・ズームイン */
		work->fade_cnt += TIME_BASE;
		alpha = work->fade_cnt * MAP_FADE_MAX / MAP_FADE_TIME;
		switch(work->mode)
		{
		case SK_TANKER : work->map_zoom = (float)(work->fade_cnt * MAP_CAMERA_ZOOM_TANKER ) / (float)MAP_FADE_TIME; break;
		default:
		case SK_PLANT  : work->map_zoom = (float)(work->fade_cnt * MAP_CAMERA_ZOOM        ) / (float)MAP_FADE_TIME; break;
		case SK_ARSENAL: work->map_zoom = (float)(work->fade_cnt * MAP_CAMERA_ZOOM_ARSENAL) / (float)MAP_FADE_TIME; break;
		}
		if(alpha > MAP_FADE_MAX)
			alpha = MAP_FADE_MAX;
		work->fade->sprite.col.a = alpha;
		if(alpha < MAP_FADE_MAX)
			break;

		MENU_ClearTextTexture(work->stage_font.work);
		work->step = STEP_PAUSE;
		SY_PRINTF1("act_pause - STEP_PAUSE\n");
		/* break 不要 */

	case STEP_PAUSE:
		model_rot(work);
		StringCreate(work);
		StringDisp(work);
		break;

	case STEP_FINISH:
		/* フェードアウト・ズームアウト */
		work->fade_cnt -= TIME_BASE;
		alpha = work->fade_cnt * MAP_FADE_MAX / MAP_FADE_TIME;
		work->map_zoom = (float)(work->fade_cnt * zm) / (float)MAP_FADE_TIME;
		if(alpha < 0)
			alpha = 0;
		work->fade->sprite.col.a = alpha;
		camera_setup(work);
		if(alpha > 0)
			break;

		/* モデル */
		for(i = 0; i < work->model_cnt; i++)
		{
			if(work->models[i].queue == 1)
			{
				DG_DequeueObjs(work->models[i].objs);
				work->models[i].queue = 0;
			}
		}

		map_chanl_off(work);
		SPR_HIDE(work->fade);
		change_act(work, act_waiting);
		if(work->pos.position > -1) change_light_matrix( work->models[work->pos.position].objs, work->light_active);
		GM_ResetSightStatus(SGT_Invisible);
		work->flag &= ~FLAG_SIGHT_SET;

		/* レイアウトを閉じる */
		GV_CallChildSignalFunc(work , VRWML_CLOSE, 0);
		break;
	}

	if(    (GV_PauseLevelNoXMB != GV_PAUSE_PAUSE)   //BP_PAUSE - exclude XMB from pause check
		&& (   (work->step == STEP_FDIN )
			|| (work->step == STEP_PAUSE)))
	{
		work->step = STEP_FINISH;
		SY_PRINTF1("act_pause - STEP_FINISH\n");
		zm = work->map_zoom;
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_V_CANS02);
		StatusReset(work);

		/* ＶＲポーズ */
//		GV_CallChildSignalFunc(&work->actor, VRPAU2_OFF, 0);
	}
}

/******************************************************************************
 * 待機中
 */
static void act_waiting(
	Work *work)		/* ワーク */
{
	enum
	{
		STEP_INIT,
		STEP_WAIT,
	};

	int     i;
	FVECTOR pos;
	SVECTOR rot;
	void    *ptr;

	switch(work->step)
	{
	case STEP_INIT:
		work->step      = STEP_WAIT;
		SY_PRINTF1("act_pause - STEP_WAIT\n");
		work->light_cnt = 0;

		/* モデルの回転 */
		switch(work->mode)
		{
		case SK_TANKER:
			work->rot_x = DEFAULT_ROT_X_TANKER;
			work->rot_y = DEFAULT_ROT_Y_TANKER;
			break;

		case SK_PLANT:
		default:
			work->rot_x = DEFAULT_ROT_X;
			work->rot_y = DEFAULT_ROT_Y;
			break;

		case SK_ARSENAL:
			work->rot_x = DEFAULT_ROT_X_ARSENAL;
			work->rot_y = DEFAULT_ROT_Y_ARSENAL;
			break;
		}
		for(i = 0; i < work->model_cnt; i++)
		{
			pos.vx = MAP_OBJ_X;
			pos.vx = MAP_OBJ_Y;
			pos.vx = MAP_OBJ_Z;
			pos.vw = 1.0F;
			rot.vx = (int)work->rot_x;
			rot.vy = (int)work->rot_y;
			rot.vz = rot.pad = 0;
			DG_SetPos2(&pos, &rot);
			DG_GetPos(&work->models[i].objs->world);
		}

		/* break 不要 */

	case STEP_WAIT:

#ifdef DEBUG_MODE
		if(allmap_flag == 0)
			break;
#endif

		if(    (GV_PauseLevelNoXMB == GV_PAUSE_PAUSE)   //BP_PAUSE - exclude XMB from pause check
			&& !(MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE)
			&& !(GV_PadData[0].flag & GV_PAD_RELEASE)
			&& !(work->flag & FLAG_LAYOUT_FAILED)
			&& !(work->flag & FLAG_CHILD_START))
		{
			/* ＶＲ全体マップ表示３Ｄのにぎやかし */
			if((ptr = NewVRWorldMapLayout(&work->pos, &work->pos.position, work->mode, &work->bomb_stage)) == NULL)
			{
				work->flag |= FLAG_LAYOUT_FAILED;
			}
			else
			{
				GV_SetActorChild(work, ptr);
				work->flag |= FLAG_CHILD_START;

				// サブウィンドウ
				TS_SubWindowSleep();

				// ゲーム状態
				GM_PushGameStatus();

				// メニュー状態
				if((GM_MenuStatus & MENU_RADIO_DISABLE) == 0)
					work->flag |= FLAG_MENU_RADIO_SET;
				GM_SetMenuStatus(MENU_RADIO_DISABLE);
				GM_PushMenuStatus();
				GM_SetMenuStatus(MENU_GAGE_OFF);
				GM_SetMenuStatus(MENU_RADAR_OFF);
				GM_SetMenuStatus(MENU_SUBWIN_OFF);
				GM_SetMenuStatus(MENU_MENU_OFF);

				// 字幕表示
				GM_JimakuHide();

				// サイト
				GM_SetSightStatus(SGT_Invisible);
				work->flag |= FLAG_SIGHT_SET;

				/* ＶＲポーズ */
				GV_CallChildSignalFunc(&work->actor, VRPAU2_ON, 0);

				// Act の変更
				change_act(work, act_pause);

				GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);

				// ポーズ解除をできなくする
				GM_GameStatus |= STATE_PAUSE_DISABLE;
			}
		}
		else
		{
			work->flag &= ~FLAG_LAYOUT_FAILED;
		}
		break;
	}
}

/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	Work *work)		/* ワーク */
{
	int i;

	/* 爆弾ステージ */
	work->bomb_stage = VRMap3D_BombStage;

	/* 実行 Act 関数 */
	(work->act_func)(work);

	/* カメラを設定 */
	if((GV_PauseLevelNoXMB == GV_PAUSE_PAUSE) && !(MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE))   //BP_PAUSE - exclude XMB from pause check
		camera_setup(work);

	/* 特別な場合モデルを隠す */
	if(    (GM_MenuStatus & MENU_RADIO_ON)
		|| (GM_MenuStatus & MENU_NODE_ON)
		|| (GM_CheckGameStatus(STATE_PLAY_DEMO)))
	{
		for(i = 0; i < work->model_cnt; i++)
		{
			if(work->models[i].queue == 1)
				DG_DequeueObjs(work->models[i].objs);
			work->models[i].queue = 0;
		}
	}
}

/*******************************************************************************
 * 終了処理
 */
static void Die(
	Work *work)		/* ワーク */
{
	int i;

	/* モデル */
	for(i = 0; i < work->model_cnt; i++)
	{
		if(work->models[i].queue == 1)
			DG_DequeueObjs(work->models[i].objs);
		DG_FreeObjs(work->models[i].objs);
	}
	GV_Free(work->models);

	/* フェードスプライト */
	SPR_Destroy_2D_Object(work->fade);

	/* 設定状態を戻す */
	if(work->flag & FLAG_SIGHT_SET)
		GM_ResetSightStatus(SGT_Invisible);
	if(work->flag & FLAG_MENU_RADIO_SET)
		GM_ResetMenuStatus( MENU_RADIO_DISABLE);
}

/*******************************************************************************
 * 環境光 色の設定
 */
static void set_ambient(
	FMATRIX *mat,	/* 光源マトリクス */
	int     r,		/* 色Ｒ */
	int     g,		/* 色Ｇ */
	int     b)		/* 色Ｂ */
{
	mat[1].m[3][0] = (float)r;
	mat[1].m[3][1] = (float)g;
	mat[1].m[3][2] = (float)b;
}

/*******************************************************************************
 * 平行光 色の設定
 */
static void set_light_color(
	FMATRIX *mat,	/* 光源マトリクス */
	int     r,		/* 色Ｒ */
	int     g,		/* 色Ｇ */
	int     b)		/* 色Ｂ */
{
	mat[1].m[0][0] = (float)r;
	mat[1].m[0][1] = (float)g;
	mat[1].m[0][2] = (float)b;
}

/*******************************************************************************
 * 平行光 方向の設定
 */
static void set_light_vec(
	FMATRIX *mat,	/* 光源マトリクス */
	float   x,		/* 方向Ｘ */
	float   y,		/* 方向Ｙ */
	float   z)		/* 方向Ｚ */
{
	FVECTOR vec;

	vec.vx = x;
	vec.vy = y;
	vec.vz = z;

	_sceVu0Normalize(&vec, &vec);

	mat[0].m[0][0] = vec.vx;
	mat[0].m[1][0] = vec.vy;
	mat[0].m[2][0] = vec.vz;
}

/*******************************************************************************
 * 光源マトリクスの設定
 */
static void set_light_matrixes(
	Work *work)		/* ワーク */
{
	/* 環境光色の設定 */
	set_ambient(work->light_active  , MAP_AMB_ACT_R  , MAP_AMB_ACT_G  , MAP_AMB_ACT_B  );
	set_ambient(work->light_inactive, MAP_AMB_INACT_R, MAP_AMB_INACT_G, MAP_AMB_INACT_B);
	set_ambient(work->light_player  , MAP_AMB_PLY_R  , MAP_AMB_PLY_G  , MAP_AMB_PLY_B  );

	/* 平行光色の設定 */
	set_light_color(work->light_active  , MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B);
	set_light_color(work->light_inactive, MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B);
	set_light_color(work->light_player  , MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B);

	/* 平行光方向の設定 */
	set_light_vec(work->light_active  , MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z);
	set_light_vec(work->light_inactive, MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z);
	set_light_vec(work->light_player  , MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z);
}

/*******************************************************************************
 * 各モデルに光源マトリクスを設定
 */
static void change_light_matrix(
	DG_OBJS *objs,	/* モデル */
	FMATRIX *mat)	/* 光源マトリクス */
{
	int i;

	for(i = 0; i < objs->n_models; i++)
		objs->objs[i].light = mat;
}

/*******************************************************************************
 * モデルを通常アクティブ光源に設定
 */
static void map_all_active(
	Work    *work,		/* ワーク */
	DG_OBJS *objs,		/* モデル */
	int     active)		/* アクティブフラグ */
{
	if(active) change_light_matrix(objs, work->light_active);
	else       change_light_matrix(objs, work->light_inactive);
}

/*******************************************************************************
 * 各モデルのビットフラグを寝かす
 */
static void objs_clear_flag(
	DG_OBJS *objs,			/* モデル */
	int     clear_flag)		/* ビットフラグ */
{
	int i;
	int mask = ~clear_flag;

	objs->flag &= mask;
	for(i = 0; i < objs->n_models; i++)
		objs->objs[i].flag &= mask;
}

/*******************************************************************************
 * 初期化処理２
 */
static int GetResourcesP(	/* 1: 成功 */
							/* 0: 失敗 */
	Work *work,			/* ワーク */
	int  name,			/* シナリオ名 */
	int  where,			/* マップＩＤ */
	int  *table_le,		/* モデル名テーブル */
	int  model_cnt,		/* モデル数 */
	int  player_pos)	/* 現在位置 */
{
	int     i;
	SPR_POS pos = {0.0f, 0.0f};

	SY_PRINTF3("GetResourcesP\n");

#ifdef DEBUG_MODE
	GM_AddDebugMenu(&debug_menu);
#endif

	/* モデル数 */
	work->model_cnt = model_cnt;

	/* 光源マトリクスの設定 */
	set_light_matrixes(work);

	/* モデル用メモリ確保 */
	if(NULL == (work->models = GV_Malloc(sizeof(MODEL_SET) * model_cnt)))
	{
		SY_PRINTF2("No Memory");
		return 0;
	}

	/* フェードスプライト */
	work->fade = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	SPR_SetPosSprite(work->fade, &pos);
	SPR_SetSizeSprite(work->fade, 512.0F, 384.0F);
	work->fade->sprite.col.r =  8;
	work->fade->sprite.col.g = 24;
	work->fade->sprite.col.b =  8;
	work->fade->sprite.col.a =  0;
	work->fade->head.flags  |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
	work->fade->head.alpha   = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
	SPR_SetPriority(work->fade, 1);
	SPR_HIDE(work->fade);

	/* 現在位置の初期化 */
	work->pos.position = -1;

	/* モデル */
	for(i = 0; i < model_cnt; i++)
	{
		work->models[i].queue   = 0;
		work->models[i].strcode = BP_LE_SwapSInt( *(table_le + i) );

		work->models[i].def = GV_GetCache(GV_CacheID(BP_LE_SwapSInt( *(table_le + i)), 'k'));
		if(work->models[i].def == NULL)
		{
			SY_PRINTF2("GetCacheFailed\n");
			return 0;
		}
		work->models[i].objs = DG_MakeObjs(work->models[i].def, MAP_DG_FLAG, MAP_CHANL);
		if(work->models[i].objs == NULL)
		{
			SY_PRINTF2("MakeObjsFailed\n");
			return 0;
		}
		/* モデル作成直後は必ず DG_FLAG_INVISIBLE2 が設定されているので、これをクリアする。 */
		objs_clear_flag(work->models[i].objs, DG_FLAG_INVISIBLE2);

		/* モデルを通常アクティブ光源か非アクティブ光源に設定 */
		map_all_active(work, work->models[i].objs, work->active_stage & (0x1 << i));

		/* 現在位置チェック */
		if(BP_LE_SwapSInt( *(table_le + i) ) == player_pos)
		{
			/* 現在位置の決定 */
			work->pos.position = i;
		}
	}
	/* 特別処理 */
	if(player_pos == ARSENAL_46) work->pos.position = -2;
	SY_PRINTF1("Position: %d\n", work->pos.position);

	/* 初期 Act の設定 */
	change_act(work, act_waiting);

	/* シグナルの登録 */
	GV_SetActorSignalFunc(work, SignalFunc);

	return 1;
}

/******************************************************************************
 * 初期化処理

chara ＶＲ全体マップ表示３Ｄ[NewVRMap3D_Scn] $s:名前 \
	// 以下オプション
	-player   $s:プレイヤ位置ラベル \
	-model    $d:マップモデルテーブル \
	-active   $i:アクティブステージ \
	-bomb     $i:爆弾ステージ \
	-restart  $p:ＲＥＳＴＡＲＴプロック \
	-exit     $p:ＥＸＩＴプロック \	
	-vr_pause								// ＶＲポーズ起動フラグ

// 各テーブルビットフラグ
enum
{
	F_TANKER_BODY = 0x00000001,	// map_tanker_body
	F_TANKER_00A  = 0x00000002,	// map_tanker_00a       船尾甲板
	F_TANKER_01A  = 0x00000004,	// map_tanker_01a       船橋一階居住区
	F_TANKER_01F  = 0x00000008,	// map_tanker_01f       船橋一階リフレッシュルーム
	F_TANKER_01B  = 0x00000010,	// map_tanker_01b       船橋二階居住区
	F_TANKER_01C  = 0x00000020,	// map_tanker_01c       船橋三階居住区
	F_TANKER_01D  = 0x00000040,	// map_tanker_01d       船橋四階居住区
	F_TANKER_01E  = 0x00000080,	// map_tanker_01e       船橋五階操舵室
	F_TANKER_00B  = 0x00000100,	// map_tanker_00b       航海甲板ウィング
	F_TANKER_02A  = 0x00000200,	// map_tanker_02a       機関室
	F_TANKER_03B  = 0x00000400,	// map_tanker_03b       第二甲板右舷
	F_TANKER_03A  = 0x00000800,	// map_tanker_03a       第二甲板左舷
	F_TANKER_03_O = 0x00001000,	// map_tanker_03_other
	F_TANKER_04A  = 0x00002000,	// map_tanker_04a       第一船倉
	F_TANKER_04B  = 0x00004000,	// map_tanker_04b       第二船倉
	F_TANKER_04C  = 0x00008000,	// map_tanker_04c       第三船倉
};

enum
{
	F_PLANT_A     = 0x00000001,	// a_w12                Ａ脚ポンプ施設
	F_PLANT_AB    = 0x00000002,	// ab_w13               ＡＢ連絡橋
	F_PLANT_B     = 0x00000004,	// b_w14                Ｂ脚変電室
	F_PLANT_BC    = 0x00000008,	// bc_w15               ＢＣ連絡橋
	F_PLANT_C     = 0x00000010,	// c_w16                Ｃ脚食堂
	F_PLANT_CD    = 0x00000020,	// cd_w17               ＣＤ連絡橋
	F_PLANT_D     = 0x00000040,	// d_w18                Ｄ脚第一沈殿池
	F_PLANT_DE    = 0x00000080,	// de_w19               ＤＥ連絡橋
	F_PLANT_E     = 0x00000100,	// e_w20                Ｅ脚貨物集配施設
	F_PLANT_EF    = 0x00000200,	// ef_w21               ＥＦ連絡橋
	F_PLANT_F     = 0x00000400,	// f_w22                Ｆ脚倉庫
	F_PLANT_FA    = 0x00000800,	// af_w23               ＦＡ連絡橋
	F_PLANT_S1    = 0x00001000,	// s1_center_w24        シェル１中央棟
	F_PLANT_DG    = 0x00002000,	// dg_w25               シェル１シェル２連絡橋
	F_PLANT_G     = 0x00004000,	// g_w25                Ｇ脚油処理施設
	F_PLANT_GH    = 0x00008000,	// gh                   ＧＨ連絡橋
	F_PLANT_H     = 0x00010000,	// h                    Ｈ脚倉庫
	F_PLANT_HI    = 0x00020000,	// hi                   ＨＩ連絡橋
	F_PLANT_I     = 0x00040000,	// i                    Ｉ脚機材組立施設
	F_PLANT_IJ    = 0x00080000,	// ij                   ＩＪ連絡橋
	F_PLANT_J     = 0x00100000,	// j                    Ｊ脚発電施設
	F_PLANT_JK    = 0x00200000,	// jk                   ＪＫ連絡橋
	F_PLANT_K     = 0x00400000,	// k                    Ｋ脚生物化学実験施設
	F_PLANT_KL    = 0x00800000,	// lk_w25               ＫＬ連絡橋
	F_PLANT_L     = 0x01000000,	// l_w25                Ｌ脚汚水処理場
	F_PLANT_LG    = 0x02000000,	// gl_w25               ＬＧ連絡橋
	F_PLANT_S2    = 0x04000000,	// s2_center_w31        シェル２中央棟
	F_PLANT_L2    = 0x08000000,	// l_w32                Ｌ脚下部オイルフェンス
	F_PLANT_OTHER = 0x10000000,	// other                Ｈ脚下部オイルフェンス
};

enum
{
	F_ARSENAL_41  = 0x00000001,	// map_41_mt           アーセナルギア胃
	F_ARSENAL_42  = 0x00000002,	// map_42_mt           アーセナルギア空腸
	F_ARSENAL_43  = 0x00000004,	// map_43_mt           アーセナルギア上行結腸
	F_ARSENAL_44  = 0x00000008,	// map_44_mt           アーセナルギア回腸
	F_ARSENAL_45  = 0x00000010,	// map_45_mt           アーセナルギアＳ状結腸
};
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	Work *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	int  *table_le    = NULL;
	int  player_pos   = -1;
	int  proc_restart = 0;
	int  proc_exit    = 0;
	int  vr_pause     = 0;
	int  model_cnt    = 0;
	void *ptr;

	SY_PRINTF3("GetResources\n");

	work->flag         = 0;
	work->rot_x        = 0.0f;
	work->rot_y        = 0.0f;
	work->light_cnt    = 0;
	work->map_zoom     = 0.0f;
	work->step         = 0;
	work->fade_cnt     = 0;
	work->active_stage = 0;
	work->bomb_stage   = 0;

	/* シナリオオプション */
	if(GCL_GetOption('m') != NULL) table_le           = GCL_GetNextResource_LE();
	if(GCL_GetOption('p') != NULL) player_pos         = GCL_GetNextInt();
	if(GCL_GetOption('a') != NULL) work->active_stage = GCL_GetNextInt();
	if(GCL_GetOption('b') != NULL) work->bomb_stage   = GCL_GetNextInt();
	if(GCL_GetOption('r') != NULL) proc_restart       = GCL_GetNextInt();
	if(GCL_GetOption('e') != NULL) proc_exit          = GCL_GetNextInt();
	if(GCL_GetOption('v') == NULL)
	{
		SY_PRINTF1("vr_pause parameter(-v) needed.");
		return 0;
	}
	 vr_pause = 1;

	/* 爆弾ステージ */
	VRMap3D_BombStage = work->bomb_stage;

	/* 動作モード */
	work->mode = GetModeFromStrcode(player_pos);
	SY_PRINTF1("Mode: %d\n", work->mode);

	/* モデルの数 */
	if(table_le != NULL)
	{
		while( BP_LE_SwapSInt( *(table_le + model_cnt) ) != 0x20)
			model_cnt++;
	}
	work->name = name;
	SY_PRINTF1("ModelCount: %d\n", model_cnt);

	/* フォント */
	work->stage_font.work = NewTextScreenControlEx(24 * SYS_FONT_WIDTH, (SYS_FONT_HEIGHT * 2) + 14, 240, 0x2, 1/*bufferedTextFlag*/);
	if(work->stage_font.work == NULL)
	{
		SY_PRINTF2("TextScreenControlFailed\n");
		return 0;
	}
	MENU_ClearTextTexture(work->stage_font.work);
	GV_SetActorChild(work, work->stage_font.work);

	work->stage_font.r    = SK_R;
	work->stage_font.g    = SK_G;
	work->stage_font.b    = SK_B;
	work->stage_font.a    = SK_A;

	/* ＶＲポーズ２ */
	if(vr_pause == 1)
	{
		if((ptr = NewVRPause2(proc_restart, proc_exit)) == NULL)
		{
			SY_PRINTF2("NewVRPause2 Failed\n");
			return 0;
		}
		GV_SetActorChild(work, ptr);
	}

	return GetResourcesP(work, name, where, table_le, model_cnt, player_pos);
}

/******************************************************************************
 * public
 */
/******************************************************************************
 * 起動処理
enum
{
	GV_ACTOR_DAEMON,	// システムデーモン
	GV_ACTOR_MANAGER,	// ゲーム進行プロセス
	GV_ACTOR_ASSIST,	// ゲーム補助プロセス

	GV_ACTOR_PREV2,		// メインより前に実行されるキャラクタ (ポーズ時も動作)
	GV_ACTOR_PREV,		// メインより前に実行されるキャラクタ

	GV_ACTOR_PLAYER,	// 通常キャラクタ (プレイヤー)
	GV_ACTOR_USER,		// 通常キャラクタ
	GV_ACTOR_CAMERA,	// カメラ (ポーズ時も動作)

	GV_ACTOR_AFTER,		// メインより後に実行されるキャラクタ
	GV_ACTOR_EFFECT,	// エフェクト
	GV_ACTOR_AFTER2,	// メインより後に実行されるキャラクタ (ポーズ時も動作)

	GV_ACTOR_DAEMON2,	// システムデーモン２
	GV_ACTOR_LEVEL_MAX
};
 */
void *NewVRMap3D_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	Work *work;

	SY_PRINTF3("NewVRMap3D_Scn\n");
	OPERATOR();

	/* アクター生成 */
	if(NULL == (work = GV_NewActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 180)))
		return NULL;

	/* アクター登録 */
	GV_SetActor(&work->actor, Act, Die);
	GV_ActorEX(&work->actor);

	/* ワークの初期化 */
	if(!GetResources(work, name, where))
	{
		SY_PRINTF2("GetResources Failed.\n");
		GV_DestroyActor(work);
		return NULL;
	}

	return work;
}

/******************************************************************************
 *

command ＶＲ全体マップ表示３Ｄ＿爆弾設置[VRMap3D_SetBomb] -b $i:ステージフラグ
 */
void VRMap3D_SetBomb(void)
{
	VRMap3D_BombStage |= GCL_GetOptionValue('b', 0);
}

/******************************************************************************
 *

command ＶＲ全体マップ表示３Ｄ＿爆弾解除[VRMap3D_ResetBomb] -b $i:ステージフラグ
 */
void VRMap3D_ResetBomb(void)
{
	VRMap3D_BombStage &= ~GCL_GetOptionValue('b', 0);
}
