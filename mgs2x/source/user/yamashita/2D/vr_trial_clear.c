/*******************************************************************************
 * vr_trial_clear - vr_trial_clear.c
 * ＶＲクリア  *NewVRClear
 * 2002/07/03 S.Yamashita
 * $Id: vr_trial_clear.c,v 1.4 2002/11/23 12:36:06 Yoshizawa1 Exp $
 */

/*******************************************************************************
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

#include "../../sigeno/vr/vr.h"
#include "msn.h"
#include "vr_def.h"
#include "layoutman.h"

#define __CHARA_NAME__ "VR Clear"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* スコア項目タイプ */
enum {
	SCORE_MAP = 0,
	SCORE_TIME,
	SCORE_M9,
	SCORE_USP,
	SCORE_SOCOM,
	SCORE_AKS,
	SCORE_M4,
	SCORE_C4,
	SCORE_CLAYMORE,
	SCORE_GRENADE,
	SCORE_RGB6,
	SCORE_PSG1,
	SCORE_PSG1_T,
	SCORE_STINGER,
	SCORE_NIKITA,
	SCORE_WEAPON,
	SCORE_COMBO,
	SCORE_SNEAKING,
	SCORE_NO_KILL,
	SCORE_TOTAL,
	SCORE_MAX,
};

/* レートタイプ */
enum {
	RATE_TIME = 0,
	RATE_WEAPON_HANDGUN,
	RATE_WEAPON_RIFLE,
	RATE_WEAPON_EXPLOSIVE,
	RATE_COMBO,
	RATE_SNEAKING,
	RATE_NO_KILL,
	RATE_MAX,
};
#define VRCLR_RATE_TIME             (50)	/* 時間レート　　　　デフォルト値 */
#define VRCLR_RATE_WEAPON_HANDGUN   (50)	/* 単発系武器レート　デフォルト値 */
#define VRCLR_RATE_WEAPON_RIFLE     (50)	/* 連射系武器レート　デフォルト値 */
#define VRCLR_RATE_WEAPON_EXPLOSIVE (50)	/* 爆発系武器レート　デフォルト値 */
#define VRCLR_RATE_COMBO            (50)	/* コンボレート　　　デフォルト値 */
#define VRCLR_RATE_SNEAKING         (50)	/* 隠密レート　　　　デフォルト値 */
#define VRCLR_RATE_NO_KILL          (50)	/* 不殺レート　　　　デフォルト値 */

/* スコア項目セットタイプ */
enum {
	SCORE_TYPE_01 = 0,	/*            TIME                           NO KILL */
	SCORE_TYPE_02,		/*            TIME  BULLET                   NO KILL */
	SCORE_TYPE_03,		/* MAP SCORE  TIME  BULLET  COMBO                    */
	SCORE_TYPE_04,		/*            TIME  BULLET         SNEAKING  NO KILL */
	SCORE_TYPE_05,		/*            TIME                                   */
	SCORE_TYPE_06,		/*            TIME  BULLET                           */
	SCORE_TYPE_07,		/* MAP SCORE  TIME                                   */
	SCORE_TYPE_08,		/* MAP SCORE  TIME          COMBO                    */
	SCORE_TYPE_MAX,
};

/* コールバックタイプ */
enum {
	CB_OBJ_NEXTSTAGE_RIGHT = 1,		/* ＮＥＸＴ　ＳＴＡＧＥ　で下が押された */
	CB_OBJ_TRYAGAIN_LEFT,			/* ＴＲＹ　ＡＧＡＩＮ　　で上が押された */
	CB_OBJ_TRYAGAIN_RIGHT,			/* ＴＲＹ　ＡＧＡＩＮ　　で下が押された */
	CB_OBJ_EXIT_LEFT,				/* ＥＸＩＴ　　　　　　　で上が押された */

	CB_OBJ_NEXTSTAGE_OK,			/* ＮＥＸＴ　ＳＴＡＧＥ　でＯＫが押された */
	CB_OBJ_TRYAGAIN_OK,				/* ＴＲＹ　ＡＧＡＩＮ　　でＯＫが押された */
	CB_OBJ_EXIT_OK,					/* ＥＸＩＴ　　　　　　　でＯＫが押された */
};

/* フェーズタイプ */
enum {
	PHASE_HIDE = 0,				/* 非表示で待機 */
	PHASE_CLEAR_MOTION,			/* クリアモーション中、一定時間停止 */
	PHASE_CLEAR_EFFECT,			/* クリアエフェクト中、一定時間停止 */
	PHASE_L2D_ACTION_1,			/* クリア画面１を開く */

	PHASE_MAP_SHOW,				/* マップスコアの表示 */
	PHASE_TIME_SHOW,			/* タイムの表示 */
	PHASE_TIME_COUNT,			/* タイムの計算 */
	PHASE_M9_SHOW,				/* Ｍ９の表示 */
	PHASE_M9_COUNT,				/* Ｍ９の計算 */
	PHASE_USP_SHOW,				/* ＵＳＰの表示 */
	PHASE_USP_COUNT,			/* ＵＳＰの計算 */
	PHASE_SOCOM_SHOW,			/* ソコムの表示 */
	PHASE_SOCOM_COUNT,			/* ソコムの計算 */
	PHASE_AKS_SHOW,				/* ＡＫＳの表示 */
	PHASE_AKS_COUNT,			/* ＡＫＳの計算 */
	PHASE_M4_SHOW,				/* Ｍ４の表示 */
	PHASE_M4_COUNT,				/* Ｍ４の計算 */
	PHASE_C4_SHOW,				/* Ｃ４の表示 */
	PHASE_C4_COUNT,				/* Ｃ４の計算 */
	PHASE_CLAYMORE_SHOW,		/* クレイモアの表示 */
	PHASE_CLAYMORE_COUNT,		/* クレイモアの計算 */
	PHASE_GRENADE_SHOW,			/* グレネードの表示 */
	PHASE_GRENADE_COUNT,		/* グレネードの計算 */
	PHASE_RGB6_SHOW,			/* ＲＧＢ６の表示 */
	PHASE_RGB6_COUNT,			/* ＲＧＢ６の計算 */
	PHASE_PSG1_SHOW,			/* ＰＳＧ１の表示 */
	PHASE_PSG1_COUNT,			/* ＰＳＧ１の計算 */
	PHASE_PSG1_T_SHOW,			/* ＰＳＧ１－Ｔの表示 */
	PHASE_PSG1_T_COUNT,			/* ＰＳＧ１－Ｔの計算 */
	PHASE_STINGER_SHOW,			/* スティンガーの表示 */
	PHASE_STINGER_COUNT,		/* スティンガーの計算 */
	PHASE_NIKITA_SHOW,			/* ニキータの表示 */
	PHASE_NIKITA_COUNT,			/* ニキータの計算 */
	PHASE_COMBO_SHOW,			/* コンボの表示 */
	PHASE_COMBO_COUNT,			/* コンボの計算 */
	PHASE_SNEAKING_SHOW,		/* 隠密の表示 */
	PHASE_SNEAKING_COUNT,		/* 隠密の計算 */
	PHASE_NO_KILL_SHOW,			/* 不殺の表示 */
	PHASE_WAIT_1,				/* 一定時間停止 */

	PHASE_L2D_ACTION_2,			/* クリア画面１からクリア画面２へ移行 */
	PHASE_WAIT_2,				/* 順位を表示中 */
	PHASE_CURSOR_ON,			/* カーソルを表示して待機 */
	PHASE_CLOSE,				/* 閉じる */
	PHASE_CLOSE2,				/* 閉じる２ */

	PHASE_BONUS_PICT_FADE_IN,	/* ボーナス絵フェードイン */
	PHASE_BONUS_PICT_SHOW,		/* ボーナス絵表示 */
	PHASE_BONUS_PICT_FADE_OUT,	/* ボーナス絵フェードアウト */

	PHASE_END,					/* 終了 */
};
#ifdef PAL
#define PHASE_CLEAR_EFFECT_COUNT        ( 75)
#define PHASE_L2D_ACTION_1_COUNT        ( 75)
#define PHASE_ITEM_SHOW_COUNT           ( 13)
#define PHASE_ITEM_SHOW2_COUNT          ( 38)
#define PHASE_WAIT_1_COUNT              (250)
#define PHASE_L2D_ACTION_2_COUNT        ( 25)
#define PHASE_WAIT_2_COUNT              ( 25)
#define PHASE_CLOSE_COUNT               ( 19)
#define PHASE_CLOSE2_COUNT              ( 38)
#define PHASE_BONUS_PICT_FADE_IN_COUNT  ( 50)
#define PHASE_BONUS_PICT_SHOW_COUNT     (250)
#define PHASE_BONUS_PICT_FADE_OUT_COUNT ( 50)
#define PHASE_END_COUNT                 (  1)
#else
#define PHASE_CLEAR_EFFECT_COUNT        ( 90)
#define PHASE_L2D_ACTION_1_COUNT        ( 90)
#define PHASE_ITEM_SHOW_COUNT           ( 15)
#define PHASE_ITEM_SHOW2_COUNT          ( 45)
#define PHASE_WAIT_1_COUNT              (300)
#define PHASE_L2D_ACTION_2_COUNT        ( 30)
#define PHASE_WAIT_2_COUNT              ( 30)
#define PHASE_CLOSE_COUNT               ( 22)
#define PHASE_CLOSE2_COUNT              ( 45)
#define PHASE_BONUS_PICT_FADE_IN_COUNT  ( 60)
#define PHASE_BONUS_PICT_SHOW_COUNT     (300)
#define PHASE_BONUS_PICT_FADE_OUT_COUNT ( 60)
#define PHASE_END_COUNT                 (  1)
#endif

/* フラグ */
#define VRCLR_INIT          (0x0001)	/* 初期化時 */
#define VRCLR_TEXT_MODE     (0x0002)	/* テキスト表示モード */
#define VRCLR_BONUS_MODE    (0x0004)	/* ボーナス絵表示モード */
#define VRCLR_FAST_MODE     (0x0008)	/* 高速モード */
#define VRCLR_VRTRIAL       (0x0010)	/* 体験版 */
#define VRCLR_FINAL_STAGE   (0x0020)	/* 最終ステージ */
#define VRCLR_INPUT_OK      (0x0040)	/* 入力ＯＫ状態 */
#define VRCLR_BLINK_A       (0x0080)	/* 点滅 */
#define VRCLR_NEXT_STAGE    (0x4000)	/* 次ステージプロックを実行 */
#define VRCLR_TRY_AGAIN     (0x8000)	/* リスタートプロックを実行 */

/* 設定値 */
#define VRCLR_TEXT_BUF_X          (1024)	/* テキストバッファ幅 */
#define VRCLR_TEXT_BUF_Y          ( 600)	/* テキストバッファ高さ */
#define VRCLR_TIME_COUNT_SPEED    (  60)	/* タイムカウント速度 */
#define VRCLR_SCORE_COUNT_SPEED   ( 100)	/* スコアカウント速度 */
#define VRCLR_SCORE_COUNT_SPEED_2 (  10)	/* スコアカウント速度 */
#define VRCLR_FONT_TEX_WIDTH      (18.0f)	/* フォント テクスチャ幅 */
#define VRCLR_FONT_TEX_HEIGHT     (12.0f)	/* フォント テクスチャ高さ */
#define VRCLR_COMBO_SCORE         (0)		/* 最大コンボ数をスコア計算するか */
#ifdef PAL
#define VRCLR_INPUT_INTERVAL      (5)		/* 入力の制御 */
#else
#define VRCLR_INPUT_INTERVAL      (6)		/* 入力の制御 */
#endif

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME    ( 7040470)		/* vr_clear */
#define L2D_FILENAME2    ( 5646060)		/* vr_clear_trial */

#define OBJECT_VR_TRAINING   (13397873)	/* vr_missions_cleared */
#define OBJECT_VR_TRAINING_T (14406163)	/* vr_training_cleared */
#define OBJECT_CURSOR_1      (13630255)	/* select_ca-sol_next_stage */
#define OBJECT_CURSOR_2      (15869131)	/* select_ca-sol_retry */
#define OBJECT_CURSOR_3      ( 9526212)	/* select_ca-sol_exit */
/* カーソル項目 */
#define OBJECT_NEXTSTAGE ( 9727768)		/* next_stage */
#define OBJECT_TRYAGAIN  ( 5529280)		/* retry */
#define OBJECT_EXIT      ( 3435924)		/* exit */
/* カーソル矩形 */
#define OBJECT_NEXTSTAGE_RECT (13630255)	/* select_ca-sol_next_stage */
#define OBJECT_TRYAGAIN_RECT  (15869131)	/* select_ca-sol_retry */
#define OBJECT_EXIT_RECT      ( 9526212)	/* select_ca-sol_exit */
/* ＡＬＴＥＲＮＡＴＩＶＥ モード名 */
#define OBJECT_MODE    ( 3688677)		/* mode */
#define OBJECT_MODE_T  ( 9235103)		/* mode_trial */
/* レベル */
#define OBJECT_LEVEL_2 (13262995)		/* level_num_10-1 */
#define OBJECT_LEVEL_1 ( 1462951)		/* level_num_1-1 */
/* マップスコア */
#define OBJECT_MAP     (13856618)		/* map_score */
#define OBJECT_MAP_EQ  ( 5875770)		/* equal__01 */
#define OBJECT_MAP_T5  (13593174)		/* map_score_total_num_10000 */
#define OBJECT_MAP_T4  ( 3570513)		/* map_score_total_num_1000 */
#define OBJECT_MAP_T3  (  635865)		/* map_score_total_num_100 */
#define OBJECT_MAP_T2  ( 4738461)		/* map_score_total_num_10 */
#define OBJECT_MAP_T1  ( 6963819)		/* map_score_total_num_1 */
/* タイム */
#define OBJECT_TIM     ( 3912197)		/* time */
#define OBJECT_TIM_6   ( 2751193)		/* time_num_10000000 */
#define OBJECT_TIM_5   ( 4804565)		/* time_num_1000000 */
#define OBJECT_TIM_C1  (15978337)		/* time_num_tenten_01 */
#define OBJECT_TIM_4   ( 6902354)		/* time_num_10000 */
#define OBJECT_TIM_3   ( 1264273)		/* time_num_1000 */
#define OBJECT_TIM_C2  (15978338)		/* time_num_tenten_02 */
#define OBJECT_TIM_2   ( 1590481)		/* time_num_10 */
#define OBJECT_TIM_1   (  573989)		/* time_num_1 */
#define OBJECT_TIM_X   (14950146)		/* peke_02 */
#define OBJECT_TIM_R3  ( 2674206)		/* time_num_10-1-1 */
#define OBJECT_TIM_R2  ( 5755761)		/* time_num_1-1-1-2 */
#define OBJECT_TIM_R1  ( 6804979)		/* time_num_1-1-1 */
#define OBJECT_TIM_EQ  ( 4900706)		/* equal_02 */
#define OBJECT_TIM_T5  ( 1180863)		/* time_total_num_10000 */
#define OBJECT_TIM_T4  ( 7901220)		/* time_total_num_1000 */
#define OBJECT_TIM_T3  (10732671)		/* time_total_num_100 */
#define OBJECT_TIM_T2  ( 8199714)		/* time_total_num_10 */
#define OBJECT_TIM_T1  ( 9693423)		/* time_total_num_1 */
/* 弾数 */
#define OBJECT_BUL     (16235619)		/* bullets */
#define OBJECT_BUL_WP  ( 7936167)		/* <weapons> */
#define OBJECT_BUL_3   ( 7325174)		/* bullets_100 */
#define OBJECT_BUL_2   ( 3374638)		/* bullets_10 */
#define OBJECT_BUL_1   (15834095)		/* bullets_1 */
#define OBJECT_BUL_X   (14950147)		/* peke_03 */
#define OBJECT_BUL_R3  ( 6772317)		/* bullets_num_100 */
#define OBJECT_BUL_R2  ( 7027377)		/* bullets_num_10 */
#define OBJECT_BUL_R1  (  743892)		/* bullets_num_1 */
#define OBJECT_BUL_EQ  ( 5875772)		/* equal__03 */
#define OBJECT_BUL_T5  ( 1860475)		/* bullets_total_num_10000 */
#define OBJECT_BUL_T4  ( 5825306)		/* bullets_total_num_1000 */
#define OBJECT_BUL_T3  ( 5424919)		/* bullets_total_num_100 */
#define OBJECT_BUL_T2  ( 3839543)		/* bullets_total_num_10 */
#define OBJECT_BUL_T1  ( 3790000)		/* bullets_total_num_1 */
/* コンボ */
#define OBJECT_COM     ( 6897845)		/* combo */
#define OBJECT_COM_3   ( 8838715)		/* combo_100 */
#define OBJECT_COM_2   ( 6043376)		/* combo_10 */
#define OBJECT_COM_1   (  188854)		/* combo_1 */
#define OBJECT_COM_X   (14950148)		/* peke_04 */
#define OBJECT_COM_R3  (12109793)		/* combo_num_100 */
#define OBJECT_COM_R2  ( 9291325)		/* combo_num_10 */
#define OBJECT_COM_R1  ( 7106096)		/* combo_num_1 */
#define OBJECT_COM_EQ  ( 5875773)		/* equal__04 */
#define OBJECT_COM_T5  (10532076)		/* combo_total_num_10000 */
#define OBJECT_COM_T4  (15009189)		/* combo_total_num_1000 */
#define OBJECT_COM_T3  (11479083)		/* combo_total_num_100 */
#define OBJECT_COM_T2  (14514495)		/* combo_total_num_10 */
#define OBJECT_COM_T1  ( 8317896)		/* combo_total_num_1 */
/* 隠密 */
#define OBJECT_SNE     ( 1238442)		/* sneaking_bonus */
#define OBJECT_SNE_1   ( 9876572)		/* sneaking_bonus_1 */
#define OBJECT_SNE_X   (14950149)		/* peke_05 */
#define OBJECT_SNE_R3  (11366790)		/* sneaking_bonus_num_100 */
#define OBJECT_SNE_R2  (11889546)		/* sneaking_bonus_num_10 */
#define OBJECT_SNE_R1  (14003034)		/* sneaking_bonus_num_1 */
#define OBJECT_SNE_EQ  ( 5875774)		/* equal__05 */
#define OBJECT_SNE_T5  ( 4565398)		/* sneaking_bonus_total_num_10000 */
#define OBJECT_SNE_T4  ( 3288395)		/* sneaking_bonus_total_num_1000 */
#define OBJECT_SNE_T3  (14258536)		/* sneaking_bonus_total_num_100 */
#define OBJECT_SNE_T2  (13028489)		/* sneaking_bonus_total_num_10 */
#define OBJECT_SNE_T1  (13514338)		/* sneaking_bonus_total_num_1 */
/* 不殺 */
#define OBJECT_NOK     ( 2939873)		/* no_kill_bonus */
#define OBJECT_NOK_EQ  ( 5875775)		/* equal__06 */
#define OBJECT_NOK_T5  (11423658)		/* no_kill_total_num_10000 */
#define OBJECT_NOK_T4  (13988475)		/* no_kill_total_num_1000 */
#define OBJECT_NOK_T3  ( 6204306)		/* no_kill_total_num_100 */
#define OBJECT_NOK_T2  ( 1242459)		/* no_kill_total_num_10 */
#define OBJECT_NOK_T1  ( 5805993)		/* no_kill_total_num_1 */
/* 合計 */
#define OBJECT_TOT     ( 7953555)		/* total */
#define OBJECT_TOT_B   ( 4523772)		/* totalBar-1 */
#define OBJECT_TOT_7   (15872943)		/* total_num_1000000 */
#define OBJECT_TOT_6   (16748955)		/* total_num_100000 */
#define OBJECT_TOT_5   ( 6290571)		/* total_num_10000 */
#define OBJECT_TOT_4   (14352354)		/* total_num_1000 */
#define OBJECT_TOT_3   ( 9885693)		/* total_num_100 */
#define OBJECT_TOT_2   ( 7124670)		/* total_num_10 */
#define OBJECT_TOT_1   ( 7562676)		/* total_num_1 */
/* ランキング */
#define OBJECT_1ST     (   53972)		/* 1st */
#define OBJECT_1ST_7   ( 5651495)		/* 1st_num_1000000 */
#define OBJECT_1ST_6   (12235231)		/* 1st_num_100000 */
#define OBJECT_1ST_5   ( 8246669)		/* 1st_num_10000 */
#define OBJECT_1ST_4   (15462058)		/* 1st_num_1000 */
#define OBJECT_1ST_3   (14114675)		/* 1st_num_100 */
#define OBJECT_1ST_2   ( 2013946)		/* 1st_num_10 */
#define OBJECT_1ST_1   ( 5305814)		/* 1st_num_1 */
#define OBJECT_2ND     (   54820)		/* 2nd */
#define OBJECT_2ND_7   ( 9124903)		/* 2nd_num_1000000 */
#define OBJECT_2ND_6   (12343775)		/* 2nd_num_100000 */
#define OBJECT_2ND_5   ( 8250061)		/* 2nd_num_10000 */
#define OBJECT_2ND_4   (15462164)		/* 2nd_num_1000 */
#define OBJECT_2ND_3   ( 2580343)		/* 2nd_num_100 */
#define OBJECT_2ND_2   ( 3750650)		/* 2nd_num_10 */
#define OBJECT_2ND_1   ( 5360086)		/* 2nd_num_1 */
#define OBJECT_3RD     (   55972)		/* 3rd */
#define OBJECT_3RD_7   (13843495)		/* 3rd_num_1000000 */
#define OBJECT_3RD_6   (12491231)		/* 3rd_num_100000 */
#define OBJECT_3RD_5   ( 8254669)		/* 3rd_num_10000 */
#define OBJECT_3RD_4   (15462308)		/* 3rd_num_1000 */
#define OBJECT_3RD_3   (10968955)		/* 3rd_num_100 */
#define OBJECT_3RD_2   ( 6109946)		/* 3rd_num_10 */
#define OBJECT_3RD_1   ( 5433814)		/* 3rd_num_1 */

#define ACTION_FRAME_1  (  863270)	/* openFrame						フレームを開く　 */
#define ACTION_FRAME_2  ( 11026633)	/* closeFrame_next_stage			フレームを閉じる */
#define ACTION_FRAME_3  ( 14567320)	/* closeFrame_retry					フレームを閉じる */
#define ACTION_FRAME_4  ( 16301274)	/* closeFrame_exit					フレームを閉じる */
#define ACTION_TITLE_1  ( 6759299)	/* vr_sneaking_sneaking				タイトル部分表示 */
#define ACTION_TITLE_2  (13409508)	/* vr_sneaking_eliminate_all		タイトル部分表示 */
#define ACTION_TITLE_3  ( 1685966)	/* vr_weapon_handgun				タイトル部分表示 */
#define ACTION_TITLE_4  ( 7466180)	/* vr_weapon_assault_rifle			タイトル部分表示 */
#define ACTION_TITLE_5  ( 1761089)	/* vr_weapon_c4_claymore			タイトル部分表示 */
#define ACTION_TITLE_6  ( 9346950)	/* vr_weapon_grenade				タイトル部分表示 */
#define ACTION_TITLE_7  ( 3729813)	/* vr_weapon_psg_1					タイトル部分表示 */
#define ACTION_TITLE_8  (13548215)	/* vr_weapon_stinger				タイトル部分表示 */
#define ACTION_TITLE_9  ( 8348515)	/* vr_weapon_nikita					タイトル部分表示 */
#define ACTION_TITLE_10 ( 1665546)	/* vr_weapon_hf_blade				タイトル部分表示 */
#define ACTION_TITLE_11 (13900758)	/* vr_no_weapon						タイトル部分表示 */
#define ACTION_TITLE_12 ( 8659112)	/* vr_first_person_view				タイトル部分表示 */
#define ACTION_TITLE_13 ( 9124177)	/* vr_variety						タイトル部分表示 */
#define ACTION_TITLE_14 (10366027)	/* vr_streaking						タイトル部分表示 */
#define ACTION_TITLE_15 ( 9131515)	/* show_alt_title_01				タイトル部分表示 */
#define ACTION_TITLE_16 ( 10991609)	/* stage_01							タイトル部分表示 */
#define ACTION_ITEM1_1  ( 7033413)	/* show_map_score_01				各スコア項目表示 */
#define ACTION_ITEM2_1  ( 6181688)	/* show_time_01						各スコア項目表示 */
#define ACTION_ITEM2_2  ( 5039947)	/* show_time_01-2					各スコア項目表示 */
#define ACTION_ITEM2_3  ( 6181689)	/* show_time_02						各スコア項目表示 */
#define ACTION_ITEM3_1  (13282088)	/* show_bullets_01					各スコア項目表示 */
#define ACTION_ITEM3_2  (13282089)	/* show_bullets_02					各スコア項目表示 */
#define ACTION_ITEM4_2  (10656924)	/* show_combo_01					各スコア項目表示 */
#define ACTION_ITEM4_1  (10656925)	/* show_combo_02					各スコア項目表示 */
#define ACTION_ITEM5_1  ( 4151389)	/* show_sneaking_bonus_01			各スコア項目表示 */
#define ACTION_ITEM6_1  (12410628)	/* show_no_kill_bonus_01			各スコア項目表示 */
#define ACTION_ITEM6_2  (12410629)	/* show_no_kill_bonus_02			各スコア項目表示 */
#define ACTION_ITEM6_3  (12410630)	/* show_no_kill_bonus_03			各スコア項目表示 */
#define ACTION_MOJI_1   ( 5165116)	/* sneaking_sneaking_moji */
#define ACTION_MOJI_2   ( 7471749)	/* sneaking_eliminate_all_moji */
#define ACTION_MOJI_3   ( 9439861)	/* weapon_mode_all_moji */
#define ACTION_MOJI_4   ( 8964742)	/* first_person_view_moji */
#define ACTION_MOJI_5   ( 4924696)	/* weapon_mode_blade_moji */
#define ACTION_MESSAGE  ( 3830757)	/* show_press_any_button */
#define ACTION_ITEMS_1  ( 3406346)	/* hide_sneaking_sneaking			スコア項目非表示 (SCORE_TYPE_01, (SCORE_TYPE_05)) */
#define ACTION_ITEMS_2  ( 6703602)	/* hide_sneaking_eliminate_all		スコア項目非表示 (SCORE_TYPE_02, (SCORE_TYPE_06)) */
#define ACTION_ITEMS_3  (14993552)	/* hide_weapon_mode_all				スコア項目非表示 (SCORE_TYPE_03, (SCORE_TYPE_07)) */
#define ACTION_ITEMS_4  ( 5306159)	/* hide_first_person_view			スコア項目非表示 (SCORE_TYPE_04)                  */
#define ACTION_ITEMS_5  (3286136)	/* hide_weapon_mode_blade       	スコア項目非表示 (SCORE_TYPE_08)                  */
#define ACTION_RANK_1   (12160472)	/* ranking_01						ランキング表示　 */
#define ACTION_SELECT_1 (11435882)	/* show_select_01					カーソル項目表示 */
#define ACTION_SELECT_2 ( 1659982)	/* show_select_exit					カーソル項目表示 */
#define ACTION_CURSOR_1 ( 9071797)	/* nextstage_retry					カーソル移動　　 */
#define ACTION_CURSOR_2 (  938818)	/* retry_nextstage					カーソル移動　　 */
#define ACTION_CURSOR_3 (13445914)	/* retry_exit						カーソル移動　　 */
#define ACTION_CURSOR_4 ( 7324811)	/* exit_retry						カーソル移動　　 */
#define ACTION_CLOSE_1  (16564290)	/* close_vr_sneaking_sneaking		文字フェードアウト */
#define ACTION_CLOSE_2  (16242275)	/* close_vr_sneaking_eliminate_all	文字フェードアウト */
#define ACTION_CLOSE_3  ( 5454073)	/* close_vr_weapon_handgun			文字フェードアウト */
#define ACTION_CLOSE_4  (13744018)	/* close_vr_weapon_assault_rifle	文字フェードアウト */
#define ACTION_CLOSE_5  (13530931)	/* close_vr_weapon_c4_claymore		文字フェードアウト */
#define ACTION_CLOSE_6  (13115057)	/* close_vr_weapon_grenade			文字フェードアウト */
#define ACTION_CLOSE_7  (  243701)	/* close_vr_weapon_psg_1			文字フェードアウト */
#define ACTION_CLOSE_8  (  539107)	/* close_vr_weapon_stinger			文字フェードアウト */
#define ACTION_CLOSE_9  (14233436)	/* close_vr_weapon_nikita			文字フェードアウト */
#define ACTION_CLOSE_10 ( 4804465)	/* close_vr_weapon_hf_blade			文字フェードアウト */
#define ACTION_CLOSE_11 ( 7396204)	/* close_vr_no_weapon				文字フェードアウト */
#define ACTION_CLOSE_12 ( 1686888)	/* close_vr_first_person_view		文字フェードアウト */
#define ACTION_CLOSE_13 ( 7381121)	/* close_vr_variety					文字フェードアウト */
#define ACTION_CLOSE_14 ( 3861473)	/* close_vr_streaking				文字フェードアウト */
#define ACTION_CLOSE_15 ( 3448845)	/* close_vr_alternative				文字フェードアウト */
#define ACTION_CLOSE_16 ( 7582035)	/* close_stage_01					文字フェードアウト */

/*******************************************************************************
 * macros
 */

#define TEXT_X(c) (VRCLR_TEXT_BUF_X/6 * (c) + 1)
#define TEXT_W    (VRCLR_TEXT_BUF_X/6 - 2)
#define TEXT_Y(r) (VRCLR_TEXT_BUF_Y/25 * (r) + 1)
#define TEXT_H    (VRCLR_TEXT_BUF_Y/25 - 2)

#define ICON_X(c) (TEXT_X(c) * VRDEF_SCREEN_X / VRCLR_TEXT_BUF_X)
#define ICON_W(c) ((TEXT_X(c) + TEXT_W) * VRDEF_SCREEN_X / VRCLR_TEXT_BUF_X)
#define ICON_Y(c) (TEXT_Y(c) * VRDEF_SCREEN_Y / VRCLR_TEXT_BUF_Y)
#define ICON_H(c) ((TEXT_Y(c) + TEXT_H) * VRDEF_SCREEN_Y / VRCLR_TEXT_BUF_Y)

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;				/* アクター */

	int       name;					/* シナリオ名 */
	int       proc_next_stage;		/* 次ステージプロック */
	int       proc_try_again;		/* リスタートプロック */
	int       proc_exit;			/* 終了プロック */

	int       bgmcode[4];
	int       clear_count;
	void      *text_work;			/* テキストワーク */
	SPR_OBJ   *background;			/* 背景スプライト */
	SPR_OBJ   *bonus_pict;			/* ボーナス絵 */

	int       score[SCORE_MAX];		/* 各スコア(4*14) */
	short     rate[RATE_MAX];		/* 各変換レート(2*7) */
	short     flag;					/* フラグ */
	int       tri;					/* ボーナス絵の TRI */
	int       result;				/* 結果順位 */

	float     tex_mod_v0;			/* モードテクスチャのＶ値の原点 */
	float     tex_wea_v0;			/* 武器テクスチャのＶ値の原点 */
	float     tex_dig_u0;			/* 数値テクスチャのＵ値の原点 */
	int       effect_flag;			/* 矢野さんエフェクト用フラグ */

	LAYOUTMAN layoutman;			/* レイアウトマネージャ */

	int       count;				/* カウント */
	short     phase;				/* フェーズ */
	short     count2;				/* カウント２ */
	char      mission;				/* ミッション番号 */
	char      mode;					/* モード番号 */
	char      weapon;				/* 武器番号 */
	char      level;				/* レベル番号 */
	char      player;				/* プレイヤー番号 */
	char      score_type;			/* スコアタイプ */
}
WORK;

/*******************************************************************************
 * extern
 */

void *NewTextScreenControlEx(int text_vram_width, int text_vram_height, int prio, int flag);
void MENU_ClearTextTexture(void *work);
int  MENU_CreateTextTexture(void *work, int x, int y, int width, int height, int pitch, int space, char *message);
void MENU_PutTextScreen(void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col);

void *NewVRClearEffect2( int mode, int *flag  );

/*******************************************************************************
 * local
 */

static void *_work   = NULL;	/* 多重起動防止 */
static int  _se_flag = 0;		/* ＳＥ用 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/	/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_FRAME_2,		1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_01,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_03,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_04,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_05,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_5,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_06,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_6,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_07,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_7,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_08,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_8,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_09,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_9,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_10,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_10,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_11,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_11,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_12,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_12,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_13,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_13,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_14,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_14,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_15,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_15,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_16,				ACTION_ITEM1_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_17,				ACTION_ITEM2_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_18,				ACTION_ITEM2_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_19,				ACTION_ITEM3_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_20,				ACTION_ITEM3_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_21,				ACTION_ITEM4_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_22,				ACTION_ITEM4_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_23,				ACTION_ITEM5_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_24,				ACTION_ITEM6_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_25,				ACTION_ITEM6_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_26,				ACTION_ITEM6_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_27,				ACTION_ITEMS_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_28,				ACTION_ITEMS_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_29,				ACTION_ITEMS_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_30,				ACTION_ITEMS_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_31,				ACTION_ITEMS_5,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_32,				ACTION_RANK_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_33,				ACTION_SELECT_1,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_34,				ACTION_SELECT_2,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_35,				ACTION_CURSOR_1,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_36,				ACTION_CURSOR_2,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_37,				ACTION_CURSOR_3,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_38,				ACTION_CURSOR_4,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_39,				ACTION_CLOSE_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_40,				ACTION_CLOSE_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_41,				ACTION_CLOSE_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_42,				ACTION_CLOSE_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_43,				ACTION_CLOSE_5,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_44,				ACTION_CLOSE_6,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_45,				ACTION_CLOSE_7,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_46,				ACTION_CLOSE_8,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_47,				ACTION_CLOSE_9,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_48,				ACTION_CLOSE_10,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_49,				ACTION_CLOSE_11,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_50,				ACTION_CLOSE_12,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_51,				ACTION_CLOSE_13,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_52,				ACTION_CLOSE_14,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_53,				ACTION_CLOSE_15,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_54,				ACTION_FRAME_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_55,				ACTION_FRAME_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_56,				ACTION_FRAME_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_57,				ACTION_FRAME_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_TITLE_16,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_58,				ACTION_CLOSE_16,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_59,				ACTION_MOJI_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_MESSAGE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_60,				ACTION_MOJI_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_MESSAGE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_61,				ACTION_MOJI_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_MESSAGE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_62,				ACTION_MOJI_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_MESSAGE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_63,				ACTION_MOJI_5,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_MESSAGE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */	/* 色 */		/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		0, 0, 0, 0,					0, 0, 0, 0,		LOM_ICO_LOOP,	LOM_ICO_REL|LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_NEXTSTAGE,		LOM_ICOLIST_00,			5,				LOM_OBJ_INTERP_1,
												OBJECT_TRYAGAIN,		LOM_ICOLIST_00,			5,				LOM_OBJ_INTERP_1,
												OBJECT_EXIT,			LOM_ICOLIST_00,			5,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/	/*オブジェクト名*/	/*入力*/	/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				LOM_PAD_ALLACT,		OBJECT_NEXTSTAGE,	PAD_R,		LOM_EXE_CALLBACK,	CB_OBJ_NEXTSTAGE_RIGHT,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,		OBJECT_TRYAGAIN,	PAD_L,		LOM_EXE_CALLBACK,	CB_OBJ_TRYAGAIN_LEFT,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,		OBJECT_TRYAGAIN,	PAD_R,		LOM_EXE_CALLBACK,	CB_OBJ_TRYAGAIN_RIGHT,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,		OBJECT_EXIT,		PAD_L,		LOM_EXE_CALLBACK,	CB_OBJ_EXIT_LEFT,		LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,		OBJECT_NEXTSTAGE,	PAD_OK,		LOM_EXE_CALLBACK,	CB_OBJ_NEXTSTAGE_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,		OBJECT_TRYAGAIN,	PAD_OK,		LOM_EXE_CALLBACK,	CB_OBJ_TRYAGAIN_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT	,	OBJECT_EXIT,		PAD_OK,		LOM_EXE_CALLBACK,	CB_OBJ_EXIT_OK,			LOM_PAD_PRESS,
												LOM_DATA_END,
 					LOM_DATA_END,

					/*モード名*/				/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,				LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* init */

					LOM_MODE_01,				LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions sneaking sneaking */
					LOM_MODE_02,				LOM_ACTLIST_02,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions sneaking eliminate */
					LOM_MODE_03,				LOM_ACTLIST_03,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon handgun */
					LOM_MODE_04,				LOM_ACTLIST_04,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon assault rifle */
					LOM_MODE_05,				LOM_ACTLIST_05,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon c4 claymore */
					LOM_MODE_06,				LOM_ACTLIST_06,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon grenade */
					LOM_MODE_07,				LOM_ACTLIST_07,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon psg1 */
					LOM_MODE_08,				LOM_ACTLIST_08,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon stinger */
					LOM_MODE_09,				LOM_ACTLIST_09,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon nikita */
					LOM_MODE_10,				LOM_ACTLIST_10,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon hf blade */
					LOM_MODE_11,				LOM_ACTLIST_11,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions weapon no weapon */
					LOM_MODE_12,				LOM_ACTLIST_12,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions first person view */
					LOM_MODE_13,				LOM_ACTLIST_13,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions variety */
					LOM_MODE_14,				LOM_ACTLIST_14,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr missions streaking */
					LOM_MODE_15,				LOM_ACTLIST_15,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - alternative missions */
					LOM_MODE_57,				LOM_ACTLIST_57,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* frame open -> title - vr training */
                                                                                                                                   
					LOM_MODE_16,				LOM_ACTLIST_16,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - map score 1 */
					LOM_MODE_17,				LOM_ACTLIST_17,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - time      1 */
					LOM_MODE_18,				LOM_ACTLIST_18,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - time      2 */
					LOM_MODE_19,				LOM_ACTLIST_19,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - bullet    2 */
					LOM_MODE_20,				LOM_ACTLIST_20,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - bullet    3 */
					LOM_MODE_21,				LOM_ACTLIST_21,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - combo     3 */
					LOM_MODE_22,				LOM_ACTLIST_22,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - combo     4 */
					LOM_MODE_23,				LOM_ACTLIST_23,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - sneaking  3 */
					LOM_MODE_24,				LOM_ACTLIST_24,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - no kill   2 */
					LOM_MODE_25,				LOM_ACTLIST_25,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - no kill   3 */
					LOM_MODE_26,				LOM_ACTLIST_26,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item open - no kill   4 */

					LOM_MODE_59,				LOM_ACTLIST_59,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* moji 1 -> show press any cursor */
					LOM_MODE_60,				LOM_ACTLIST_60,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* moji 2 -> show press any cursor */
					LOM_MODE_61,				LOM_ACTLIST_61,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* moji 3 -> show press any cursor */
					LOM_MODE_62,				LOM_ACTLIST_62,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* moji 4 -> show press any cursor */
					LOM_MODE_63,				LOM_ACTLIST_63,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* moji 5 -> show press any cursor */

					LOM_MODE_27,				LOM_ACTLIST_27,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item close - time, no kill */
					LOM_MODE_28,				LOM_ACTLIST_28,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item close - time, bullet, no kill */
					LOM_MODE_29,				LOM_ACTLIST_29,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item close - map score, time, bullet, combo */
					LOM_MODE_30,				LOM_ACTLIST_30,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item close - time, bullet, sneaking, no kill */
					LOM_MODE_31,				LOM_ACTLIST_31,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* item close - map score, time, combo */

					LOM_MODE_32,				LOM_ACTLIST_32,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* ranking open */

					LOM_MODE_33,				LOM_ACTLIST_33,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* cursor item open 1 */
					LOM_MODE_34,				LOM_ACTLIST_34,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* cursor item open 2 */

					LOM_MODE_35,				LOM_ACTLIST_35,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* cursor move - next stage -> retry      */
					LOM_MODE_36,				LOM_ACTLIST_36,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* cursor move - retry      -> next stage */
					LOM_MODE_37,				LOM_ACTLIST_37,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* cursor move - retry      -> exit       */
					LOM_MODE_38,				LOM_ACTLIST_38,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* cursor move - exit       -> retry      */

					LOM_MODE_39,				LOM_ACTLIST_39,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions sneaking sneaking */
					LOM_MODE_40,				LOM_ACTLIST_40,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions sneaking eliminate */
					LOM_MODE_41,				LOM_ACTLIST_41,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon handgun */
					LOM_MODE_42,				LOM_ACTLIST_42,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon assault rifle */
					LOM_MODE_43,				LOM_ACTLIST_43,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon c4 claymore */
					LOM_MODE_44,				LOM_ACTLIST_44,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon grenade */
					LOM_MODE_45,				LOM_ACTLIST_45,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon psg1 */
					LOM_MODE_46,				LOM_ACTLIST_46,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon stinger */
					LOM_MODE_47,				LOM_ACTLIST_47,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon nikita */
					LOM_MODE_48,				LOM_ACTLIST_48,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon hf blade */
					LOM_MODE_49,				LOM_ACTLIST_49,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions weapon no weapon */
					LOM_MODE_50,				LOM_ACTLIST_50,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions first person view */
					LOM_MODE_51,				LOM_ACTLIST_51,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions variety */
					LOM_MODE_52,				LOM_ACTLIST_52,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr missions streaking */
					LOM_MODE_53,				LOM_ACTLIST_53,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - alternative missions */
					LOM_MODE_58,				LOM_ACTLIST_58,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* fade out - vr training */

					LOM_MODE_54,				LOM_ACTLIST_54,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* close frame */
					LOM_MODE_55,				LOM_ACTLIST_55,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* close frame */
					LOM_MODE_56,				LOM_ACTLIST_56,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* close frame */
					LOM_DATA_END,
LOM_DATA_END,
};

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * ＳＥを鳴らす
 */
void SdSCoutry1(void)
{
	if(_se_flag == 0)
		GM_SdSet(SD_S_COUNTRV1);

	_se_flag = (_se_flag + 1) % 3;
}

/*******************************************************************************
 * 表示・非表示の設定
 */
void SetShow(
	WORK *work,		/* ワーク */
	int  strcode,	/* 文字列コード */
	int  flag)		/* 表示フラグ */
{
	int flags;

	if(flag == 0)
	{
		LOM_Spr_GetFlags(&work->layoutman, strcode, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(&work->layoutman, strcode, flags);
	}
	else
	{
		LOM_Spr_GetFlags(&work->layoutman, strcode, &flags); flags &= ~SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(&work->layoutman, strcode, flags);
	}
}

/*******************************************************************************
 * スプライトの表示設定
 */
static void ShowSprite(
	LAYOUTMAN     *layoutman,	/* レイアウトマネージャ */
	int           strcode,		/* 文字列コード */
	int           show)			/* 表示フラグ */
{
	int flags;

	if(show == 0)
	{
		LOM_Spr_GetFlags(layoutman, strcode, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(layoutman, strcode,  flags);
	}
	else
	{
		LOM_Spr_GetFlags(layoutman, strcode, &flags); flags &= ~SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(layoutman, strcode,  flags);
	}
}

/*******************************************************************************
 * スプライトの色の設定
 */
static void SetSpriteColor(
	LAYOUTMAN    *layoutman,		/* レイアウトマネージャ */
	int          strcode,			/* 文字列コード */
	int          active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int          step,				/* 変化度 */
	unsigned int inactive_color,	/* 非アクティブ色 */
	unsigned int active_color)		/* アクティブ色 */
{
	SPR_COLOR    sprcol;
	unsigned int color;
	int          add;

	{
		SPR_OBJ *spr;
		if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
			return;
	}

	if(active == 0) color = inactive_color;
	else            color = active_color;

	LOM_Spr_GetColor(layoutman, strcode, &sprcol);
	{
		if((abs(add = (color & 0x000000ff) - sprcol.r)) <= step)
		                 sprcol.r  = (color & 0x000000ff);
		else if(add > 0) sprcol.r += step;
		else             sprcol.r -= step;

		if((abs(add = ((color & 0x0000ff00) >> 8) - sprcol.g)) <= step)
		                 sprcol.g  = ((color & 0x0000ff00) >> 8);
		else if(add > 0) sprcol.g += step;
		else             sprcol.g -= step;

		if((abs(add = ((color & 0x00ff0000) >> 16) - sprcol.b)) <= step)
		                 sprcol.b  = ((color & 0x00ff0000) >> 16);
		else if(add > 0) sprcol.b += step;
		else             sprcol.b -= step;
	}
	LOM_Spr_SetColor(layoutman, strcode, &sprcol);
}

/*******************************************************************************
 * スプライトのアルファの設定
 */
static void SetSpriteAlpha(
	LAYOUTMAN     *layoutman,		/* レイアウトマネージャ */
	int           strcode,			/* 文字列コード */
	int           active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int           step,				/* 変化度 */
	unsigned char inactive_alpha,	/* 非アクティブアルファ */
	unsigned char active_alpha)		/* アクティブアルファ */
{
	SPR_COLOR     sprcol;
	unsigned char alpha;
	int           add;

	{
		SPR_OBJ *spr;
		if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
			return;
	}

	if(active == 0) alpha = inactive_alpha;
	else            alpha = active_alpha;

	LOM_Spr_GetColor(layoutman, strcode, &sprcol);
	{
		if(abs(add = alpha - sprcol.a) <= step)
		                 sprcol.a  = alpha;
		else if(add > 0) sprcol.a += step;
		else             sprcol.a -= step;
	}
	LOM_Spr_SetColor(layoutman, strcode, &sprcol);

	if(sprcol.a == 0) ShowSprite(layoutman, strcode, 0);
	else              ShowSprite(layoutman, strcode, 1);
}

/*******************************************************************************
 * 
 */
static void AddWeaponTotal(
	WORK *work)		/* ワーク */
{
	work->score[SCORE_WEAPON] = 0;
	if(work->phase                 <= PHASE_M9_COUNT      ) return;
	if(work->score[SCORE_M9]       >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_M9];
	if(work->phase                 <= PHASE_USP_COUNT     ) return;
	if(work->score[SCORE_USP]      >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_USP];
	if(work->phase                 <= PHASE_SOCOM_COUNT   ) return;
	if(work->score[SCORE_SOCOM]    >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_SOCOM];
	if(work->phase                 <= PHASE_AKS_COUNT     ) return;
	if(work->score[SCORE_AKS]      >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_AKS];
	if(work->phase                 <= PHASE_M4_COUNT      ) return;
	if(work->score[SCORE_M4]       >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_M4];
	if(work->phase                 <= PHASE_C4_COUNT      ) return;
	if(work->score[SCORE_C4]       >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_C4];
	if(work->phase                 <= PHASE_CLAYMORE_COUNT) return;
	if(work->score[SCORE_CLAYMORE] >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_CLAYMORE];
	if(work->phase                 <= PHASE_GRENADE_COUNT ) return;
	if(work->score[SCORE_GRENADE]  >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_GRENADE];
	if(work->phase                 <= PHASE_RGB6_COUNT    ) return;
	if(work->score[SCORE_RGB6]     >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_RGB6];
	if(work->phase                 <= PHASE_PSG1_COUNT    ) return;
	if(work->score[SCORE_PSG1]     >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_PSG1];
	if(work->phase                 <= PHASE_PSG1_T_COUNT  ) return;
	if(work->score[SCORE_PSG1_T]   >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_PSG1_T];
	if(work->phase                 <= PHASE_STINGER_COUNT ) return;
	if(work->score[SCORE_STINGER]  >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_STINGER];
	if(work->phase                 <= PHASE_NIKITA_COUNT  ) return;
	if(work->score[SCORE_NIKITA]   >  0                   ) work->score[SCORE_WEAPON] += work->score[SCORE_NIKITA];
}

/*******************************************************************************
 * 
 */
static void DrawWeapon(
	WORK *work,		/* ワーク */
	int  y,			/* Ｙ位置 */
	char *item,		/* 項目名 */
	int  final,		/* 最終カウント */
	int  count,		/* カウント */
	int  rate)		/* レート */
{
	char buf[256];
	memset (buf, 0x00, sizeof(buf));

	if(count > final)
		count = final;

	sprintf(buf, "%s", item);
	MENU_CreateTextTexture(work->text_work, TEXT_X(1), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);

	if(count < 0)
	{
		/* 結果表示モード */
		sprintf(buf, ":");
		MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		sprintf(buf, "= %07d", final);
		MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
	}
	else
	{
		/* 計算モード */
		sprintf(buf, ": %07d", final - count);
		MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		if(rate > 0)
		{
			sprintf(buf, "x %d", rate);
			MENU_CreateTextTexture(work->text_work, TEXT_X(3), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		}
		sprintf(buf, "= %07d", (count * rate) + work->score[SCORE_WEAPON]);
		MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
	}
}

/*******************************************************************************
 * 
 */
static void DrawTime(
	WORK *work,		/* ワーク */
	int  y,			/* Ｙ位置 */
	char *item,		/* 項目名 */
	int  final,		/* 最終カウント */
	int  count,		/* カウント */
	int  rate)		/* レート */
{
	int  min, sec, dec;
	char buf[256];
	memset (buf, 0x00, sizeof(buf));

	if(count > final)
		count = final;

	sprintf(buf, "%s", item);
	MENU_CreateTextTexture(work->text_work, TEXT_X(1), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);

	if(count < 0)
	{
		/* 結果表示モード */
		sprintf(buf, ":");
		MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		sprintf(buf, "= %07d", final);
		MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
	}
	else
	{
		/* 計算モード */
		min = ((final - count) / (VRDEF_FRAMERATE * 60)) % 100;
		sec = ((final - count) % (VRDEF_FRAMERATE * 60)) / VRDEF_FRAMERATE;
		dec = ((final - count) % VRDEF_FRAMERATE) * 100 / VRDEF_FRAMERATE;

		sprintf(buf, ": %02d:%02d:%02d", min, sec, dec);
		MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		sprintf(buf, "x %d", rate * 60);
		MENU_CreateTextTexture(work->text_work, TEXT_X(3), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
#ifdef PAL
		sprintf(buf, "= %07d", (int)((count * ((rate * 6.0f) / 5.0f)) + 0.5f));
#else
		sprintf(buf, "= %07d", count * rate);
#endif
		MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
	}
}

/*******************************************************************************
 * 
 */
static void DrawScore(
	WORK *work,		/* ワーク */
	int  y,			/* Ｙ位置 */
	char *item,		/* 項目名 */
	int  final,		/* 最終カウント */
	int  count,		/* カウント */
	int  rate)		/* レート */
{
	char buf[256];
	memset (buf, 0x00, sizeof(buf));

	if(count > final)
		count = final;

	sprintf(buf, "%s", item);
	MENU_CreateTextTexture(work->text_work, TEXT_X(1), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);

	if(count < 0)
	{
		/* 結果表示モード */
		sprintf(buf, ":");
		MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		sprintf(buf, "= %07d", final);
		MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
	}
	else
	{
		/* 計算モード */
		sprintf(buf, ": %07d", final - count);
		MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		if(rate > 0)
		{
			sprintf(buf, "x %d", rate);
			MENU_CreateTextTexture(work->text_work, TEXT_X(3), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
		}
		sprintf(buf, "= %07d", count * rate);
		MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(y), TEXT_W, TEXT_H, 0, 0, buf);
	}
}

/*******************************************************************************
 * 
 */
static void DrawStageInfo(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	char buf[256];
	memset(buf, 0x00, sizeof(buf));

	/* ミッション */
	if     (work->mission == MSN_MISSION_VR         ) sprintf(buf, "VR MISSIONS");
	else if(work->mission == MSN_MISSION_ALTERNATIVE) sprintf(buf, "ALTERNATIVE MISSIONS");
	MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(1), TEXT_W * 4, TEXT_H, 0, 0, buf);

	/* モード */
	if(work->mode == MSN_MODE_SNEAKING)
	{
		if     (work->weapon == MSN_WEAPON_SNEAKING     ) sprintf(buf, "SNEAKING MODE - SNEAKING");
		else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL) sprintf(buf, "SNEAKING MODE - ELIMINATE ALL");
	}
	else if(work->mode == MSN_MODE_WEAPON)
	{
		if     (work->weapon == MSN_WEAPON_HANDGUN      ) sprintf(buf, "WEAPON MODE - HANDGUN");
		else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE) sprintf(buf, "WEAPON MODE - ASSAULT RIFLE");
		else if(work->weapon == MSN_WEAPON_C4_CLAYMORE  ) sprintf(buf, "WEAPON MODE - C4/CLAYMORE");
		else if(work->weapon == MSN_WEAPON_GRENADE      ) sprintf(buf, "WEAPON MODE - GRENADE");
		else if(work->weapon == MSN_WEAPON_PSG1         ) sprintf(buf, "WEAPON MODE - PSG-1");
		else if(work->weapon == MSN_WEAPON_STINGER      ) sprintf(buf, "WEAPON MODE - STINGER");
		else if(work->weapon == MSN_WEAPON_NIKITA       ) sprintf(buf, "WEAPON MODE - NIKITA");
		else if(work->weapon == MSN_WEAPON_HF_BLADE     ) sprintf(buf, "WEAPON MODE - HF. BLADE");
	}
	else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW) sprintf(buf, "FIRST PERSON VIEW MODE");
	else if(work->mode == MSN_MODE_VARIETY          ) sprintf(buf, "VARIETY MODE");
	else if(work->mode == MSN_MODE_STREAKING        ) sprintf(buf, "STREAKING MODE");
	else if(work->mode == MSN_MODE_BOMB_DISPOSAL    ) sprintf(buf, "BOMB DISPOSAL MODE");
	else if(work->mode == MSN_MODE_ELIMINATE        ) sprintf(buf, "ELIMINATE MODE");
	else if(work->mode == MSN_MODE_HOLD_UP          ) sprintf(buf, "HOLD UP MODE");
	else if(work->mode == MSN_MODE_ESCAPE           ) sprintf(buf, "ESCAPE MODE");
	else if(work->mode == MSN_MODE_PHOTOGRAPH       ) sprintf(buf, "PHOTOGRAPH MODE");
	MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(2), TEXT_W * 4, TEXT_H, 0, 0, buf);

	/* レベル・プレイヤー */
	if     (work->player == MSN_PLAYER_SNAKE   ) sprintf(buf, "LEVEL %02d [SNAKE]", work->level);
	else if(work->player == MSN_PLAYER_RAIDEN  ) sprintf(buf, "LEVEL %02d [RAIDEN]", work->level);
	else if(work->player == MSN_PLAYER_TUXEDO  ) sprintf(buf, "LEVEL %02d [TUXEDO SNAKE]", work->level);
	else if(work->player == MSN_PLAYER_PLISKIN ) sprintf(buf, "LEVEL %02d [PLISKIN]", work->level);
	else if(work->player == MSN_PLAYER_NINJA   ) sprintf(buf, "LEVEL %02d [NINJA RAIDEN]", work->level);
	else if(work->player == MSN_PLAYER_PREVIOUS) sprintf(buf, "LEVEL %02d [PREVIOUS SNAKE]", work->level);
	MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(3), TEXT_W * 4, TEXT_H, 0, 0, buf);
#endif
}

/*******************************************************************************
 * 
 */
static void DrawScreen(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	MENU_ClearTextTexture(work->text_work);

	/* ステージ情報 */
	DrawStageInfo(work);
	if(work->phase <= PHASE_L2D_ACTION_1) return;

	/* マップスコア */
	if((work->score_type == SCORE_TYPE_03) || (work->score_type == SCORE_TYPE_07))
	{
		DrawScore(work,  5, "MAP SCORE", work->score[SCORE_MAP], -2, 0);
	}
	if(work->phase <= PHASE_MAP_SHOW) goto DRAW_TOTAL;

	/* タイム */
	{
	if     (work->phase == PHASE_TIME_SHOW ) DrawTime (work,  6, "TIME", VR_TIME                , 0          , work->rate[RATE_TIME]);
	else if(work->phase == PHASE_TIME_COUNT) DrawTime (work,  6, "TIME", VR_TIME                , work->count, work->rate[RATE_TIME]);
	else                                     DrawTime (work,  6, "TIME", work->score[SCORE_TIME], -1         , work->rate[RATE_TIME]);
	}
	if(work->phase <= PHASE_TIME_COUNT) goto DRAW_TOTAL;

	/* 武器 */
	if((work->score_type == SCORE_TYPE_02) || (work->score_type == SCORE_TYPE_03) || (work->score_type == SCORE_TYPE_04) || (work->score_type == SCORE_TYPE_06))
	{
	if     (work->phase == PHASE_M9_SHOW       ) { AddWeaponTotal(work); DrawWeapon (work,  7, "M9"      , GM_WeaponNum(WP_m92     ), 0          , work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_M9_COUNT      ) { AddWeaponTotal(work); DrawWeapon (work,  7, "M9"      , GM_WeaponNum(WP_m92     ), work->count, work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_USP_SHOW      ) { AddWeaponTotal(work); DrawWeapon (work,  7, "USP"     , GM_WeaponNum(WP_Usp     ), 0          , work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_USP_COUNT     ) { AddWeaponTotal(work); DrawWeapon (work,  7, "USP"     , GM_WeaponNum(WP_Usp     ), work->count, work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_SOCOM_SHOW    ) { AddWeaponTotal(work); DrawWeapon (work,  7, "SOCOM"   , GM_WeaponNum(WP_Socom   ), 0          , work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_SOCOM_COUNT   ) { AddWeaponTotal(work); DrawWeapon (work,  7, "SOCOM"   , GM_WeaponNum(WP_Socom   ), work->count, work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_AKS_SHOW      ) { AddWeaponTotal(work); DrawWeapon (work,  7, "AKS"     , GM_WeaponNum(WP_Aks     ), 0          , work->rate[RATE_WEAPON_RIFLE    ]); }
	else if(work->phase == PHASE_AKS_COUNT     ) { AddWeaponTotal(work); DrawWeapon (work,  7, "AKS"     , GM_WeaponNum(WP_Aks     ), work->count, work->rate[RATE_WEAPON_RIFLE    ]); }
	else if(work->phase == PHASE_M4_SHOW       ) { AddWeaponTotal(work); DrawWeapon (work,  7, "M4"      , GM_WeaponNum(WP_m4      ), 0          , work->rate[RATE_WEAPON_RIFLE    ]); }
	else if(work->phase == PHASE_M4_COUNT      ) { AddWeaponTotal(work); DrawWeapon (work,  7, "M4"      , GM_WeaponNum(WP_m4      ), work->count, work->rate[RATE_WEAPON_RIFLE    ]); }
	else if(work->phase == PHASE_C4_SHOW       ) { AddWeaponTotal(work); DrawWeapon (work,  7, "C4"      , GM_WeaponNum(WP_C4Bomb  ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_C4_COUNT      ) { AddWeaponTotal(work); DrawWeapon (work,  7, "C4"      , GM_WeaponNum(WP_C4Bomb  ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_CLAYMORE_SHOW ) { AddWeaponTotal(work); DrawWeapon (work,  7, "CLAYMORE", GM_WeaponNum(WP_Claymore), 0          , work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_CLAYMORE_COUNT) { AddWeaponTotal(work); DrawWeapon (work,  7, "CLAYMORE", GM_WeaponNum(WP_Claymore), work->count, work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_GRENADE_SHOW  ) { AddWeaponTotal(work); DrawWeapon (work,  7, "GRENADE" , GM_WeaponNum(WP_Grenade ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_GRENADE_COUNT ) { AddWeaponTotal(work); DrawWeapon (work,  7, "GRENADE" , GM_WeaponNum(WP_Grenade ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_RGB6_SHOW     ) { AddWeaponTotal(work); DrawWeapon (work,  7, "RGB6"    , GM_WeaponNum(WP_Rgb6    ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_RGB6_COUNT    ) { AddWeaponTotal(work); DrawWeapon (work,  7, "RGB6"    , GM_WeaponNum(WP_Rgb6    ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_PSG1_SHOW     ) { AddWeaponTotal(work); DrawWeapon (work,  7, "PSG1"    , GM_WeaponNum(WP_Psg1    ), 0          , work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_PSG1_COUNT    ) { AddWeaponTotal(work); DrawWeapon (work,  7, "PSG1"    , GM_WeaponNum(WP_Psg1    ), work->count, work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_PSG1_T_SHOW   ) { AddWeaponTotal(work); DrawWeapon (work,  7, "PSG1-T"  , GM_WeaponNum(WP_Psg1T   ), 0          , work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_PSG1_T_COUNT  ) { AddWeaponTotal(work); DrawWeapon (work,  7, "PSG1-T"  , GM_WeaponNum(WP_Psg1T   ), work->count, work->rate[RATE_WEAPON_HANDGUN  ]); }
	else if(work->phase == PHASE_STINGER_SHOW  ) { AddWeaponTotal(work); DrawWeapon (work,  7, "STINGER" , GM_WeaponNum(WP_Stinger ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_STINGER_COUNT ) { AddWeaponTotal(work); DrawWeapon (work,  7, "STINGER" , GM_WeaponNum(WP_Stinger ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_NIKITA_SHOW   ) { AddWeaponTotal(work); DrawWeapon (work,  7, "NIKITA"  , GM_WeaponNum(WP_Nikita  ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else if(work->phase == PHASE_NIKITA_COUNT  ) { AddWeaponTotal(work); DrawWeapon (work,  7, "NIKITA"  , GM_WeaponNum(WP_Nikita  ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE]); }
	else                                         { AddWeaponTotal(work); DrawWeapon (work,  7, "WEAPON"  , work->score[SCORE_WEAPON], -2         , 0); }
	}
	if(work->phase <= PHASE_NIKITA_COUNT) goto DRAW_TOTAL;

#if VRCLR_COMBO_SCORE
	/* コンボ */
	if((work->score_type == SCORE_TYPE_03) || (work->score_type == SCORE_TYPE_08))
	{
	if     (work->phase == PHASE_COMBO_SHOW ) DrawScore (work,  8, "COMBO", VR_COMBO_CHAIN_MAX      , 0          , work->rate[RATE_COMBO]);
	else if(work->phase == PHASE_COMBO_COUNT) DrawScore (work,  8, "COMBO", VR_COMBO_CHAIN_MAX      , work->count, work->rate[RATE_COMBO]);
	else                                      DrawScore (work,  8, "COMBO", work->score[SCORE_COMBO], -1         , work->rate[RATE_COMBO]);
	}
	if(work->phase <= PHASE_COMBO_COUNT) goto DRAW_TOTAL;
#endif

	/* 隠密 */
	if(work->score_type == SCORE_TYPE_04)
	{
		if((3 - VR_DiscoverCount) > 0)
		{
		if     (work->phase == PHASE_SNEAKING_SHOW ) DrawScore (work,  9, "SNEAKING", (3 - VR_DiscoverCount)     , 0          , work->rate[RATE_SNEAKING]);
		else if(work->phase == PHASE_SNEAKING_COUNT) DrawScore (work,  9, "SNEAKING", (3 - VR_DiscoverCount)     , work->count, work->rate[RATE_SNEAKING]);
		else                                         DrawScore (work,  9, "SNEAKING", work->score[SCORE_SNEAKING], -1         , work->rate[RATE_SNEAKING]);
		}
		else
		{
		                                             DrawScore (work,  9, "SNEAKING", work->score[SCORE_SNEAKING], -1         , work->rate[RATE_SNEAKING]);
		}
	}
	if(work->phase <= PHASE_SNEAKING_COUNT) goto DRAW_TOTAL;

	/* 不殺 */
	if((work->score_type == SCORE_TYPE_01) || (work->score_type == SCORE_TYPE_02) || (work->score_type == SCORE_TYPE_04))
	{
		DrawScore(work,  10, "NO KILL", work->score[SCORE_NO_KILL], -2, 0);
	}
	if(work->phase <= PHASE_NO_KILL_SHOW) goto DRAW_TOTAL;

	/* 合計 */
DRAW_TOTAL:
	DrawScore(work, 12, "TOTAL", work->score[SCORE_TOTAL], -2, 0);
#endif
}

/*******************************************************************************
 * 
 */
static void DrawScreen2(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	char buf[256];
	memset (buf, 0x00, sizeof(buf));

	MENU_ClearTextTexture(work->text_work);

	/* ステージ情報 */
	DrawStageInfo(work);

	/* 順位 */
	sprintf(buf, "1ST");
	MENU_CreateTextTexture(work->text_work, TEXT_X(1), TEXT_Y(5), TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, ":");
	MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(5), TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, "%07d", MSN_HISCORE2(VR_STAGE_ID, 1));
	MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(5), TEXT_W, TEXT_H, 0, 0, buf);

	sprintf(buf, "2ND");
	MENU_CreateTextTexture(work->text_work, TEXT_X(1), TEXT_Y(6), TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, ":");
	MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(6), TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, "%07d", MSN_HISCORE2(VR_STAGE_ID, 2));
	MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(6), TEXT_W, TEXT_H, 0, 0, buf);

	sprintf(buf, "3RD");
	MENU_CreateTextTexture(work->text_work, TEXT_X(1), TEXT_Y(7), TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, ":");
	MENU_CreateTextTexture(work->text_work, TEXT_X(2), TEXT_Y(7), TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, "%07d", MSN_HISCORE2(VR_STAGE_ID, 3));
	MENU_CreateTextTexture(work->text_work, TEXT_X(4), TEXT_Y(7), TEXT_W, TEXT_H, 0, 0, buf);

	/* 合計 */
	DrawScore(work, 12, "TOTAL", work->score[SCORE_TOTAL], -2, 0);

	if(work->phase <= PHASE_WAIT_2) return;

	/* 選択項目 */
	if(!(work->flag & VRCLR_FINAL_STAGE))
	{
	sprintf(buf, "NEXT STAGE");
	MENU_CreateTextTexture(work->text_work, TEXT_X(3), TEXT_Y(21) - TEXT_H / 2, TEXT_W, TEXT_H, 0, 0, buf);
	}
	sprintf(buf, "TRY AGAIN");
	MENU_CreateTextTexture(work->text_work, TEXT_X(4) + TEXT_W / 4, TEXT_Y(21) - TEXT_H / 2, TEXT_W, TEXT_H, 0, 0, buf);
	sprintf(buf, "EXIT");
	MENU_CreateTextTexture(work->text_work, TEXT_X(5) + TEXT_W / 4, TEXT_Y(21) - TEXT_H / 2, TEXT_W, TEXT_H, 0, 0, buf);
#endif
}

/*******************************************************************************
 * カウンターの設定
 */
void SetCounter(
	WORK *work,		/* ワーク */
	int  value,		/* 数値 */
	int  strcode7,	/* ７桁目用オブジェクトの文字列コード */
	int  strcode6,	/* ６桁目用オブジェクトの文字列コード */
	int  strcode5,	/* ５桁目用オブジェクトの文字列コード */
	int  strcode4,	/* ４桁目用オブジェクトの文字列コード */
	int  strcode3,	/* ３桁目用オブジェクトの文字列コード */
	int  strcode2,	/* ２桁目用オブジェクトの文字列コード */
	int  strcode1)	/* １桁目用オブジェクトの文字列コード */
{
	int i;

	if(value < 0)
		return;

	/* １００００００の位 */
	if(strcode7 > 1)
	{
		i = value / 1000000;
		if(i > 9) i = 9;
		LOM_SprTex_SetU(&work->layoutman, strcode7, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);
	}
	value %= 1000000;

	/* １０００００の位 */
	if(strcode6 > 1)
	{
		i = value / 100000;
		LOM_SprTex_SetU(&work->layoutman, strcode6, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);
	}
	value %= 100000;

	/* １００００の位 */
	if(strcode5 > 1)
	{
		i = value / 10000;
		LOM_SprTex_SetU(&work->layoutman, strcode5, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);
	}
	value %= 10000;

	/* １０００の位 */
	if(strcode4 > 1)
	{
		i = value / 1000;
		LOM_SprTex_SetU(&work->layoutman, strcode4, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);
	}
	value %= 1000;

	/* １００の位 */
	if(strcode3 > 1)
	{
		i = value / 100;
		LOM_SprTex_SetU(&work->layoutman, strcode3, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);
	}
	value %= 100;

	/* １０の位 */
	if(strcode2 > 1)
	{
		i = value / 10;
		LOM_SprTex_SetU(&work->layoutman, strcode2, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);
	}
	value %= 10;

	/* １の位 */
	if(strcode1 > 1)
	{
		LOM_SprTex_SetU(&work->layoutman, strcode1, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * value);
	}
}

/*******************************************************************************
 * タイムカウンターの設定
 */
static void _SetTimer(
	WORK *work,		/* ワーク */
	int  value)		/* 数値 */
{
	int i;
	int min;
	int sec;

	if(value < 0)
		return;

	min = (value / (VRDEF_FRAMERATE * 60)) % 100;
	value %= VRDEF_FRAMERATE * 60;
	sec = value / VRDEF_FRAMERATE;
	value %= VRDEF_FRAMERATE;

	/* 分 １０の位 */
	i = min / 10;
	min %= 10;
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIM_6, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);

	/* 分 １の位 */
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIM_5, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * min);

	/* 秒 １０の位 */
	i = sec / 10;
	sec %= 10;
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIM_4, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);

	/* 秒 １の位 */
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIM_3, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * sec);

	/* １／１００秒 １０の位 */
	value = (value * 100) / VRDEF_FRAMERATE;
	i = value / 10;
	value %= 10;
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIM_2, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * i);

	/* １／１００秒 １の位 */
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIM_1, work->tex_dig_u0 + VRCLR_FONT_TEX_WIDTH * value);
}

/*******************************************************************************
 * 
 */
static void SetWeapon(
	WORK *work,			/* ワーク */
	int  final,			/* 最終カウント */
	int  count,			/* カウント */
	int  rate,			/* レート */
	int  flag,
	int  strcode3,		/* ３桁目用オブジェクトの文字列コード */
	int  strcode2,		/* ２桁目用オブジェクトの文字列コード */
	int  strcode1,		/* １桁目用オブジェクトの文字列コード */
	int  strcode3R,		/* ３桁目用オブジェクトの文字列コード */
	int  strcode2R,		/* ２桁目用オブジェクトの文字列コード */
	int  strcode1R,		/* １桁目用オブジェクトの文字列コード */
	int  strcode7T,		/* ７桁目用オブジェクトの文字列コード */
	int  strcode6T,		/* ６桁目用オブジェクトの文字列コード */
	int  strcode5T,		/* ５桁目用オブジェクトの文字列コード */
	int  strcode4T,		/* ４桁目用オブジェクトの文字列コード */
	int  strcode3T,		/* ３桁目用オブジェクトの文字列コード */
	int  strcode2T,		/* ２桁目用オブジェクトの文字列コード */
	int  strcode1T)		/* １桁目用オブジェクトの文字列コード */
{
	if(flag != 0)
	{
		/* 結果表示モード */
		if(flag != -2)
		SetCounter(work, final,         0,         0,         0,         0, strcode3 , strcode2 , strcode1 );
		if(rate > 0)
		SetCounter(work,  rate,         0,         0,         0,         0, strcode3R, strcode2R, strcode1R);
		SetCounter(work, count, strcode7T, strcode6T, strcode5T, strcode4T, strcode3T, strcode2T, strcode1T);

		work->score[SCORE_TOTAL] += count;
	}
	else
	{
		if(count > final)
			count = final;

		/* 計算モード */
		SetCounter(work,  final                                     ,         0,         0,         0,         0, strcode3 , strcode2 , strcode1 );
		if(rate > 0)
		SetCounter(work,           rate                             ,         0,         0,         0,         0, strcode3R, strcode2R, strcode1R);
		SetCounter(work,  (count * rate) + work->score[SCORE_WEAPON], strcode7T, strcode6T, strcode5T, strcode4T, strcode3T, strcode2T, strcode1T);

		work->score[SCORE_TOTAL] += (count * rate) + work->score[SCORE_WEAPON];
	}
}

/*******************************************************************************
 * 
 */
static void SetTime(
	WORK *work,			/* ワーク */
	int  final,			/* 最終カウント */
	int  count,			/* カウント */
	int  rate,			/* レート */
	int  flag)
{
	if(flag != 0)
	{
		/* 結果表示モード */
		_SetTimer  (work, final);
		SetCounter(work, rate * 60, 0, 0,             0,             0, OBJECT_TIM_R3, OBJECT_TIM_R2, OBJECT_TIM_R1);
		SetCounter(work, count    , 0, 0, OBJECT_TIM_T5, OBJECT_TIM_T4, OBJECT_TIM_T3, OBJECT_TIM_T2, OBJECT_TIM_T1);

		work->score[SCORE_TOTAL] += count;
	}
	else
	{
		if(count > final)
			count = final;

		/* 計算モード */
		_SetTimer  (work, final);
		SetCounter(work, rate * 60   , 0, 0,             0,             0, OBJECT_TIM_R3, OBJECT_TIM_R2, OBJECT_TIM_R1);
#ifdef PAL
		SetCounter(work, (int)((count * ((rate * 6.0f) / 5.0f)) + 0.5f), 0, 0, OBJECT_TIM_T5, OBJECT_TIM_T4, OBJECT_TIM_T3, OBJECT_TIM_T2, OBJECT_TIM_T1);
		work->score[SCORE_TOTAL] += (int)((count * ((rate * 6.0f) / 5.0f)) + 0.5f);
#else
		SetCounter(work, count * rate, 0, 0, OBJECT_TIM_T5, OBJECT_TIM_T4, OBJECT_TIM_T3, OBJECT_TIM_T2, OBJECT_TIM_T1);
		work->score[SCORE_TOTAL] += count * rate;
#endif
	}
}

/*******************************************************************************
 * 
 */
static void SetScore(
	WORK *work,			/* ワーク */
	int  final,			/* 最終カウント */
	int  count,			/* カウント */
	int  rate,			/* レート */
	int  flag,
	int  strcode3,		/* ３桁目用オブジェクトの文字列コード */
	int  strcode2,		/* ２桁目用オブジェクトの文字列コード */
	int  strcode1,		/* １桁目用オブジェクトの文字列コード */
	int  strcode3R,		/* ３桁目用オブジェクトの文字列コード */
	int  strcode2R,		/* ２桁目用オブジェクトの文字列コード */
	int  strcode1R,		/* １桁目用オブジェクトの文字列コード */
	int  strcode7T,		/* ７桁目用オブジェクトの文字列コード */
	int  strcode6T,		/* ６桁目用オブジェクトの文字列コード */
	int  strcode5T,		/* ５桁目用オブジェクトの文字列コード */
	int  strcode4T,		/* ４桁目用オブジェクトの文字列コード */
	int  strcode3T,		/* ３桁目用オブジェクトの文字列コード */
	int  strcode2T,		/* ２桁目用オブジェクトの文字列コード */
	int  strcode1T)		/* １桁目用オブジェクトの文字列コード */
{
	if(flag != 0)
	{
		/* 結果表示モード */
		if(flag != -2)
		SetCounter(work, final,         0,         0,         0,         0, strcode3 , strcode2 , strcode1 );
		if(rate > 0)
		SetCounter(work,  rate,         0,         0,         0,         0, strcode3R, strcode2R, strcode1R);
		SetCounter(work, count, strcode7T, strcode6T, strcode5T, strcode4T, strcode3T, strcode2T, strcode1T);

		if(strcode1T != OBJECT_TOT_1)
			work->score[SCORE_TOTAL] += count;
	}
	else
	{
		if(count > final)
			count = final;

		/* 計算モード */
		SetCounter(work,         final,         0,         0,         0,         0, strcode3 , strcode2 , strcode1 );
		if(rate > 0)
		SetCounter(work,          rate,         0,         0,         0,         0, strcode3R, strcode2R, strcode1R);
		SetCounter(work,  count * rate, strcode7T, strcode6T, strcode5T, strcode4T, strcode3T, strcode2T, strcode1T);

		work->score[SCORE_TOTAL] += count * rate;
	}
}

/*******************************************************************************
 * 
 */
static void SetStageInfo(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	if(work->flag & VRCLR_VRTRIAL)
	{
		LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE_T, work->tex_mod_v0 + VRCLR_FONT_TEX_HEIGHT * (work->level - 1));
	}
	else
	{
		if(work->mission == MSN_MISSION_ALTERNATIVE)
		{
			if     (work->mode == MSN_MODE_BOMB_DISPOSAL) LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE, work->tex_mod_v0 + VRCLR_FONT_TEX_HEIGHT * 6);
			else if(work->mode == MSN_MODE_ELIMINATE    ) LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE, work->tex_mod_v0 + VRCLR_FONT_TEX_HEIGHT * 7);
			else if(work->mode == MSN_MODE_HOLD_UP      ) LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE, work->tex_mod_v0 + VRCLR_FONT_TEX_HEIGHT * 8);
			else if(work->mode == MSN_MODE_ESCAPE       ) LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE, work->tex_mod_v0 + VRCLR_FONT_TEX_HEIGHT * 9);
			else if(work->mode == MSN_MODE_PHOTOGRAPH   ) LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE, work->tex_mod_v0 + VRCLR_FONT_TEX_HEIGHT * 10);
		}
	}
	SetCounter(work, work->level, 0, 0, 0, 0, 0, OBJECT_LEVEL_2, OBJECT_LEVEL_1);
#endif
}

/*******************************************************************************
 * 
 */
static void SetScreen(
	WORK *work)		/* ワーク */
{
	work->score[SCORE_TOTAL] = 0;

	/* ステージ情報 */
	SetStageInfo(work);
	if(work->phase <= PHASE_L2D_ACTION_1) return;

	/* マップスコア */
	if((work->score_type == SCORE_TYPE_03) || (work->score_type == SCORE_TYPE_07) || (work->score_type == SCORE_TYPE_08))
	{
		SetShow(work, OBJECT_MAP   , 1);
		SetShow(work, OBJECT_MAP_EQ, 1);
		SetShow(work, OBJECT_MAP_T5, 1);
		SetShow(work, OBJECT_MAP_T4, 1);
		SetShow(work, OBJECT_MAP_T3, 1);
		SetShow(work, OBJECT_MAP_T2, 1);
		SetShow(work, OBJECT_MAP_T1, 1);
		SetScore(work, 0, work->score[SCORE_MAP], 0, 2,
			0, 0, 0, 0, 0, 0, 0, 0, OBJECT_MAP_T5, OBJECT_MAP_T4, OBJECT_MAP_T3, OBJECT_MAP_T2, OBJECT_MAP_T1);
	}
	if(work->phase <= PHASE_MAP_SHOW) goto DRAW_TOTAL_2;

	/* タイム */
	{
		SetShow(work, OBJECT_TIM   , 1);
		SetShow(work, OBJECT_TIM_6 , 1);
		SetShow(work, OBJECT_TIM_5 , 1);
		SetShow(work, OBJECT_TIM_C1, 1);
		SetShow(work, OBJECT_TIM_4 , 1);
		SetShow(work, OBJECT_TIM_3 , 1);
		SetShow(work, OBJECT_TIM_C2, 1);
		SetShow(work, OBJECT_TIM_2 , 1);
		SetShow(work, OBJECT_TIM_1 , 1);
		SetShow(work, OBJECT_TIM_X , 1);
		SetShow(work, OBJECT_TIM_R3, 1);
		SetShow(work, OBJECT_TIM_R2, 1);
		SetShow(work, OBJECT_TIM_R1, 1);
		SetShow(work, OBJECT_TIM_EQ, 1);
		SetShow(work, OBJECT_TIM_T5, 1);
		SetShow(work, OBJECT_TIM_T4, 1);
		SetShow(work, OBJECT_TIM_T3, 1);
		SetShow(work, OBJECT_TIM_T2, 1);
		SetShow(work, OBJECT_TIM_T1, 1);
	if     (work->phase == PHASE_TIME_SHOW ) SetTime(work, VR_TIME, 0                      , work->rate[RATE_TIME], 0);
	else if(work->phase == PHASE_TIME_COUNT) SetTime(work, VR_TIME, work->count            , work->rate[RATE_TIME], 0);
	else                                     SetTime(work, VR_TIME, work->score[SCORE_TIME], work->rate[RATE_TIME], 1);
	}
	if(work->phase <= PHASE_TIME_COUNT) goto DRAW_TOTAL_2;

	/* 武器 */
	if((work->score_type == SCORE_TYPE_02) || (work->score_type == SCORE_TYPE_03) || (work->score_type == SCORE_TYPE_04) || (work->score_type == SCORE_TYPE_06))
	{
		if     (work->phase == PHASE_M9_SHOW       ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_m92     ), 0          , work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  0); }
		else if(work->phase == PHASE_M9_COUNT      ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_m92     ), work->count, work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  0); }
		else if(work->phase == PHASE_USP_SHOW      ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Usp     ), 0          , work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  1); }
		else if(work->phase == PHASE_USP_COUNT     ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Usp     ), work->count, work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  1); }
		else if(work->phase == PHASE_SOCOM_SHOW    ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Socom   ), 0          , work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  2); }
		else if(work->phase == PHASE_SOCOM_COUNT   ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Socom   ), work->count, work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  2); }
		else if(work->phase == PHASE_AKS_SHOW      ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Aks     ), 0          , work->rate[RATE_WEAPON_RIFLE    ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  3); }
		else if(work->phase == PHASE_AKS_COUNT     ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Aks     ), work->count, work->rate[RATE_WEAPON_RIFLE    ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  3); }
		else if(work->phase == PHASE_M4_SHOW       ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_m4      ), 0          , work->rate[RATE_WEAPON_RIFLE    ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  4); }
		else if(work->phase == PHASE_M4_COUNT      ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_m4      ), work->count, work->rate[RATE_WEAPON_RIFLE    ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  4); }
		else if(work->phase == PHASE_C4_SHOW       ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_C4Bomb  ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  5); }
		else if(work->phase == PHASE_C4_COUNT      ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_C4Bomb  ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  5); }
		else if(work->phase == PHASE_CLAYMORE_SHOW ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Claymore), 0          , work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  6); }
		else if(work->phase == PHASE_CLAYMORE_COUNT) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Claymore), work->count, work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  6); }
		else if(work->phase == PHASE_GRENADE_SHOW  ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Grenade ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  7); }
		else if(work->phase == PHASE_GRENADE_COUNT ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Grenade ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  7); }
		else if(work->phase == PHASE_RGB6_SHOW     ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Rgb6    ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  8); }
		else if(work->phase == PHASE_RGB6_COUNT    ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Rgb6    ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  8); }
		else if(work->phase == PHASE_PSG1_SHOW     ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Psg1    ), 0          , work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  9); }
		else if(work->phase == PHASE_PSG1_COUNT    ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Psg1    ), work->count, work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT *  9); }
		else if(work->phase == PHASE_PSG1_T_SHOW   ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Psg1T   ), 0          , work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT * 10); }
		else if(work->phase == PHASE_PSG1_T_COUNT  ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Psg1T   ), work->count, work->rate[RATE_WEAPON_HANDGUN  ], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT * 10); }
		else if(work->phase == PHASE_STINGER_SHOW  ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Stinger ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT * 11); }
		else if(work->phase == PHASE_STINGER_COUNT ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Stinger ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT * 11); }
		else if(work->phase == PHASE_NIKITA_SHOW   ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Nikita  ), 0          , work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT * 12); }
		else if(work->phase == PHASE_NIKITA_COUNT  ) { AddWeaponTotal(work); SetWeapon(work, GM_WeaponNum(WP_Nikita  ), work->count, work->rate[RATE_WEAPON_EXPLOSIVE], 0, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1); LOM_SprTex_SetV(&work->layoutman, OBJECT_BUL_WP, work->tex_wea_v0 + VRCLR_FONT_TEX_HEIGHT * 12); }
		else
		{
			SetShow(work, OBJECT_BUL   , 1);
			SetShow(work, OBJECT_BUL_WP, 0);
			SetShow(work, OBJECT_BUL_3 , 0);
			SetShow(work, OBJECT_BUL_2 , 0);
			SetShow(work, OBJECT_BUL_1 , 0);
			SetShow(work, OBJECT_BUL_X , 0);
			SetShow(work, OBJECT_BUL_R3, 0);
			SetShow(work, OBJECT_BUL_R2, 0);
			SetShow(work, OBJECT_BUL_R1, 0);
			SetShow(work, OBJECT_BUL_EQ, 1);
			SetShow(work, OBJECT_BUL_T5, 1);
			SetShow(work, OBJECT_BUL_T4, 1);
			SetShow(work, OBJECT_BUL_T3, 1);
			SetShow(work, OBJECT_BUL_T2, 1);
			SetShow(work, OBJECT_BUL_T1, 1);
			AddWeaponTotal(work);
			SetWeapon(work, 0, work->score[SCORE_WEAPON], 0, 2, OBJECT_BUL_3, OBJECT_BUL_2, OBJECT_BUL_1, OBJECT_BUL_R3, OBJECT_BUL_R2, OBJECT_BUL_R1, 0, 0, OBJECT_BUL_T5, OBJECT_BUL_T4, OBJECT_BUL_T3, OBJECT_BUL_T2, OBJECT_BUL_T1);
		}
	}
	if(work->phase <= PHASE_NIKITA_COUNT) goto DRAW_TOTAL_2;

#if VRCLR_COMBO_SCORE
	/* コンボ */
	if((work->score_type == SCORE_TYPE_03) || (work->score_type == SCORE_TYPE_08))
	{
	if     (work->phase == PHASE_COMBO_SHOW ) SetScore(work, VR_COMBO_CHAIN_MAX      , 0          , work->rate[RATE_COMBO], 0, OBJECT_COM_3, OBJECT_COM_2, OBJECT_COM_1, OBJECT_COM_R3, OBJECT_COM_R2, OBJECT_COM_R1, 0, 0, OBJECT_COM_T5, OBJECT_COM_T4, OBJECT_COM_T3, OBJECT_COM_T2, OBJECT_COM_T1);
	else if(work->phase == PHASE_COMBO_COUNT) SetScore(work, VR_COMBO_CHAIN_MAX      , work->count, work->rate[RATE_COMBO], 0, OBJECT_COM_3, OBJECT_COM_2, OBJECT_COM_1, OBJECT_COM_R3, OBJECT_COM_R2, OBJECT_COM_R1, 0, 0, OBJECT_COM_T5, OBJECT_COM_T4, OBJECT_COM_T3, OBJECT_COM_T2, OBJECT_COM_T1);
	else                                      SetScore(work, VR_COMBO_CHAIN_MAX, work->score[SCORE_COMBO], work->rate[RATE_COMBO], 1, OBJECT_COM_3, OBJECT_COM_2, OBJECT_COM_1, OBJECT_COM_R3, OBJECT_COM_R2, OBJECT_COM_R1, 0, 0, OBJECT_COM_T5, OBJECT_COM_T4, OBJECT_COM_T3, OBJECT_COM_T2, OBJECT_COM_T1);
	}
	if(work->phase <= PHASE_COMBO_COUNT) goto DRAW_TOTAL_2;
#endif

	/* 隠密 */
	if(work->score_type == SCORE_TYPE_04)
	{
		SetShow(work, OBJECT_SNE   , 1);
		SetShow(work, OBJECT_SNE_1 , 0);
		SetShow(work, OBJECT_SNE_X , 0);
		SetShow(work, OBJECT_SNE_R3, 0);
		SetShow(work, OBJECT_SNE_R2, 0);
		SetShow(work, OBJECT_SNE_R1, 0);
		SetShow(work, OBJECT_SNE_EQ, 0);
		SetShow(work, OBJECT_SNE_T5, 0);
		SetShow(work, OBJECT_SNE_T4, 0);
		SetShow(work, OBJECT_SNE_T3, 1);
		SetShow(work, OBJECT_SNE_T2, 1);
		SetShow(work, OBJECT_SNE_T1, 1);
		if((3 - VR_DiscoverCount) > 0)
		{
		if     (work->phase == PHASE_SNEAKING_SHOW ) SetScore(work, (3 - VR_DiscoverCount)     , 0          , work->rate[RATE_SNEAKING], 0, 0, 0, OBJECT_SNE_1, OBJECT_SNE_R3, OBJECT_SNE_R2, OBJECT_SNE_R1, 0, 0, OBJECT_SNE_T5, OBJECT_SNE_T4, OBJECT_SNE_T3, OBJECT_SNE_T2, OBJECT_SNE_T1);
		else if(work->phase == PHASE_SNEAKING_COUNT) SetScore(work, (3 - VR_DiscoverCount)     , work->count, work->rate[RATE_SNEAKING], 0, 0, 0, OBJECT_SNE_1, OBJECT_SNE_R3, OBJECT_SNE_R2, OBJECT_SNE_R1, 0, 0, OBJECT_SNE_T5, OBJECT_SNE_T4, OBJECT_SNE_T3, OBJECT_SNE_T2, OBJECT_SNE_T1);
		else                                         SetScore(work, (3 - VR_DiscoverCount), work->score[SCORE_SNEAKING], work->rate[RATE_SNEAKING], 1, 0, 0, OBJECT_SNE_1, OBJECT_SNE_R3, OBJECT_SNE_R2, OBJECT_SNE_R1, 0, 0, OBJECT_SNE_T5, OBJECT_SNE_T4, OBJECT_SNE_T3, OBJECT_SNE_T2, OBJECT_SNE_T1);
		}
		else
		{
		                                             SetScore(work, 0, work->score[SCORE_SNEAKING], work->rate[RATE_SNEAKING], 1, 0, 0, OBJECT_SNE_1, OBJECT_SNE_R3, OBJECT_SNE_R2, OBJECT_SNE_R1, 0, 0, OBJECT_SNE_T5, OBJECT_SNE_T4, OBJECT_SNE_T3, OBJECT_SNE_T2, OBJECT_SNE_T1);
		}
	}
	if(work->phase <= PHASE_SNEAKING_COUNT) goto DRAW_TOTAL_2;

	/* 不殺 */
	if((work->score_type == SCORE_TYPE_01) || (work->score_type == SCORE_TYPE_02) || (work->score_type == SCORE_TYPE_04))
	{
		SetShow(work, OBJECT_NOK   , 1);
		SetShow(work, OBJECT_NOK_EQ, 1);
		SetShow(work, OBJECT_NOK_T5, 1);
		SetShow(work, OBJECT_NOK_T4, 1);
		SetShow(work, OBJECT_NOK_T3, 1);
		SetShow(work, OBJECT_NOK_T2, 1);
		SetShow(work, OBJECT_NOK_T1, 1);
		SetScore(work, 0, work->score[SCORE_NO_KILL], 0, 2,
			0, 0, 0, 0, 0, 0, 0, 0, OBJECT_NOK_T5, OBJECT_NOK_T4, OBJECT_NOK_T3, OBJECT_NOK_T2, OBJECT_NOK_T1);
	}
	if(work->phase <= PHASE_NO_KILL_SHOW) goto DRAW_TOTAL_2;

	/* 合計 */
DRAW_TOTAL_2:
	SetShow(work, OBJECT_TOT  , 1);
	SetShow(work, OBJECT_TOT_B, 1);
	SetShow(work, OBJECT_TOT_7, 1);
	SetShow(work, OBJECT_TOT_6, 1);
	SetShow(work, OBJECT_TOT_5, 1);
	SetShow(work, OBJECT_TOT_4, 1);
	SetShow(work, OBJECT_TOT_3, 1);
	SetShow(work, OBJECT_TOT_2, 1);
	SetShow(work, OBJECT_TOT_1, 1);
	SetScore(work, 0, work->score[SCORE_TOTAL], 0, 2,
			0, 0, 0, 0, 0, 0, OBJECT_TOT_7, OBJECT_TOT_6, OBJECT_TOT_5, OBJECT_TOT_4, OBJECT_TOT_3, OBJECT_TOT_2, OBJECT_TOT_1);
}

/*******************************************************************************
 * 
 */
static void SetScreen2(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	int flag;

	/* ステージ情報 */
	SetStageInfo(work);

	/* 順位 */
	SetCounter(work, MSN_HISCORE2(VR_STAGE_ID, 1), OBJECT_1ST_7, OBJECT_1ST_6, OBJECT_1ST_5, OBJECT_1ST_4, OBJECT_1ST_3, OBJECT_1ST_2, OBJECT_1ST_1);
	SetCounter(work, MSN_HISCORE2(VR_STAGE_ID, 2), OBJECT_2ND_7, OBJECT_2ND_6, OBJECT_2ND_5, OBJECT_2ND_4, OBJECT_2ND_3, OBJECT_2ND_2, OBJECT_2ND_1);
	SetCounter(work, MSN_HISCORE2(VR_STAGE_ID, 3), OBJECT_3RD_7, OBJECT_3RD_6, OBJECT_3RD_5, OBJECT_3RD_4, OBJECT_3RD_3, OBJECT_3RD_2, OBJECT_3RD_1);

	if(MSN_HISCORE(VR_STAGE_ID, 1) & MSN_OVERWRITE)
	{
		if(work->result == 1) flag = 1;
		else                  flag = 0;

		SetSpriteColor(&work->layoutman, OBJECT_1ST_7, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_1ST_6, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_1ST_5, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_1ST_4, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_1ST_3, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_1ST_2, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_1ST_1, flag, 256, VRDEF_I_1STPLAC, VRDEF_A_1STPLAC);
	}
	if(MSN_HISCORE(VR_STAGE_ID, 2) & MSN_OVERWRITE)
	{
		if(work->result == 2) flag = 1;
		else                  flag = 0;

		SetSpriteColor(&work->layoutman, OBJECT_2ND_7, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_2ND_6, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_2ND_5, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_2ND_4, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_2ND_3, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_2ND_2, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_2ND_1, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
	}
	if(MSN_HISCORE(VR_STAGE_ID, 3) & MSN_OVERWRITE)
	{
		if(work->result == 3) flag = 1;
		else                  flag = 0;

		SetSpriteColor(&work->layoutman, OBJECT_3RD_7, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_3RD_6, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_3RD_5, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_3RD_4, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_3RD_3, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_3RD_2, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_3RD_1, flag, 256, VRDEF_I_CLEARED, VRDEF_A_CLEARED);
	}
	/* 合計 */
	SetScore(work, 0, work->score[SCORE_TOTAL], 0, 2,
			0, 0, 0, 0, 0, 0, OBJECT_TOT_7, OBJECT_TOT_6, OBJECT_TOT_5, OBJECT_TOT_4, OBJECT_TOT_3, OBJECT_TOT_2, OBJECT_TOT_1);

	if(work->result == 1)
	{
		SetSpriteColor(&work->layoutman, OBJECT_TOT_7, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_6, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_5, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_4, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_3, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_2, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_1, 0, 256, VRDEF_A_1STPLAC, VRDEF_A_1STPLAC);
	}
	else if((work->result == 2) || (work->result == 3))
	{
		SetSpriteColor(&work->layoutman, OBJECT_TOT_7, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_6, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_5, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_4, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_3, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_2, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_TOT_1, 0, 256, VRDEF_A_CLEARED, VRDEF_A_CLEARED);
	}

	/* 選択項目 */
	if(work->flag & VRCLR_FINAL_STAGE)
		SetShow(work, OBJECT_NEXTSTAGE, 0);
#endif
}

/*******************************************************************************
 * フェーズの移行
 */
static void NextPhase(
	WORK *work)		/* ワーク */
{
	work->phase++;
	work->count = 0;
	_se_flag = 0;
	SY_PRINTF1("NEXT PHASE: %d\n", work->phase);
}
static void SetPhase(
	WORK  *work,	/* ワーク */
	short phase)	/* フェーズ */
{
	work->phase = phase;
	work->count = 0;
	_se_flag = 0;
	SY_PRINTF1("SET PHASE: %d\n", work->phase);
}

/*******************************************************************************
 * レイアウトマネージャのコールバック
 */
static int LOMCallback(		/* 1: 成功:処理を続行 */
							/* 2: 成功:処理を中断 */
							/* 0: 失敗 */
	void *pWork,	/* ワーク */
	int  param)		/* パラメータ */
{
	WORK *work = (WORK *)pWork;

	SY_PRINTF3("LOMCallback\n");
	SY_PRINTF1("Callback: %d\n", param);

	if(!(work->flag & VRCLR_INPUT_OK))
		return 1;
	switch(param)
	{
	case CB_OBJ_NEXTSTAGE_RIGHT:
		if(!(work->flag & VRCLR_INPUT_OK))
			return 1;
		GM_SdSet(SD_S_CUR01);

		LOM_SetCurObj (&work->layoutman, OBJECT_TRYAGAIN, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_35, 1, 1, 0);
		/* 入力一時ＮＧ */
		work->flag  &= ~VRCLR_INPUT_OK;
		work->count  = -VRCLR_INPUT_INTERVAL;
		break;

	case CB_OBJ_TRYAGAIN_LEFT:
		if(!(work->flag & VRCLR_FINAL_STAGE))
		{
			if(!(work->flag & VRCLR_INPUT_OK))
				return 1;
			GM_SdSet(SD_S_CUR01);

			LOM_SetCurObj (&work->layoutman, OBJECT_NEXTSTAGE, 1);
			LOM_SetCurMode(&work->layoutman, LOM_MODE_36, 1, 1, 0);
			/* 入力一時ＮＧ */
			work->flag  &= ~VRCLR_INPUT_OK;
			work->count  = -VRCLR_INPUT_INTERVAL;
		}
		break;

	case CB_OBJ_TRYAGAIN_RIGHT:
		if(!(work->flag & VRCLR_INPUT_OK))
			return 1;
		GM_SdSet(SD_S_CUR01);

		LOM_SetCurObj (&work->layoutman, OBJECT_EXIT, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_37, 1, 1, 0);
		/* 入力一時ＮＧ */
		work->flag  &= ~VRCLR_INPUT_OK;
		work->count  = -VRCLR_INPUT_INTERVAL;
		break;

	case CB_OBJ_EXIT_LEFT:
		if(!(work->flag & VRCLR_INPUT_OK))
			return 1;
		GM_SdSet(SD_S_CUR01);

		LOM_SetCurObj (&work->layoutman, OBJECT_TRYAGAIN, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_38, 1, 1, 0);
		/* 入力一時ＮＧ */
		work->flag  &= ~VRCLR_INPUT_OK;
		work->count  = -VRCLR_INPUT_INTERVAL;
		break;

	case CB_OBJ_NEXTSTAGE_OK:
		if(work->proc_next_stage != 0)
		{
			SY_PRINTF1("Next Stage Proc");
			GM_SdSet(SD_S_START001);

			work->flag |=  VRCLR_NEXT_STAGE;
			work->flag &= ~VRCLR_TRY_AGAIN;

			MSN_RETRY_COUNT    = 0;
			MSN_CONTINUE_COUNT = 0;
			MSN_2DSTATUS &= ~MSN_2DSTAT_QUICK_WINDOW;
			GM_DiazepamCount = 0 ;

			SetPhase(work, PHASE_CLOSE);

			/* 入力ＮＧ */
			work->flag &= ~VRCLR_INPUT_OK;
		}
		break;

	case CB_OBJ_TRYAGAIN_OK:
		if(work->proc_try_again != 0)
		{
			SY_PRINTF1("Try Again Proc");
			GM_SdSet(SD_S_START001);

			work->flag &= ~VRCLR_NEXT_STAGE;
			work->flag |=  VRCLR_TRY_AGAIN;

			MSN_RETRY_COUNT++;
			GM_DiazepamCount = 0 ;

			SetPhase(work, PHASE_CLOSE);

			/* 入力ＮＧ */
			work->flag &= ~VRCLR_INPUT_OK;
		}
		break;

	case CB_OBJ_EXIT_OK:
		if(work->proc_exit != 0)
		{
			SY_PRINTF1("Exit Proc");
			GM_SdSet(SD_S_START01);

			work->flag &= ~VRCLR_NEXT_STAGE;
			work->flag &= ~VRCLR_TRY_AGAIN;

			MSN_RETRY_COUNT    = 0;
			MSN_CONTINUE_COUNT = 0;
			MSN_2DSTATUS &= ~MSN_2DSTAT_QUICK_WINDOW;
			GM_DiazepamCount = 0 ;

			SetPhase(work, PHASE_CLOSE);

			/* 入力ＮＧ */
			work->flag &= ~VRCLR_INPUT_OK;
		}
		break;

	default:
		break;
	}
	return 1;
}

/*******************************************************************************
 * メッセージ処理

mesg ＶＲクリア $s:名前 ＯＦＦ[0]
mesg ＶＲクリア $s:名前 ＦＯＣＵＳ[1]
mesg ＶＲクリア $s:名前 ＯＮ[2]
 */
static int ReceiveMessage(	/* 処理結果 */
	WORK *work)		/* ワーク */
{
	int    i;
	int    msg_count;
	int    ret = -1;
	GV_MSG *msg;

	msg_count = GV_ReceiveMessage(work->name, &msg);
	for(i = 0; i < msg_count; i++, msg++)
	{
		SY_PRINTF2("Message received.\n");

		switch(msg->message[0])
		{
		case 0:		/* ＯＦＦ */
			/* 停止 */
			if(work->text_work ) MENU_ClearTextTexture(work->text_work);
			if(work->background) work->background->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
			if(work->bonus_pict) work->bonus_pict->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
			LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);

			SetPhase(work, PHASE_HIDE);

			/* 矢野さんキャラ第０フェーズ */
			work->effect_flag = 0;

			ret = 0;
			break;

		case 1:		/* ＦＯＣＵＳ */
			SY_PRINTF2("Clear Focus.\n");

			if(work->text_work ) MENU_ClearTextTexture(work->text_work);
			if(work->background) work->background->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
			if(work->bonus_pict) work->bonus_pict->sprite.head.head.flags |=  SPR_FLAG_HIDDEN;
			LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);

			/* 矢野さんキャラ第１フェーズ */
			work->effect_flag = 1;

			break;

		case 2:		/* ＯＮ */
			SY_PRINTF2("Clear On.\n");

			/* 開始 */
			if(work->text_work ) MENU_ClearTextTexture(work->text_work);
			if(work->background) work->background->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;
			if(work->bonus_pict) work->bonus_pict->sprite.head.head.flags |=  SPR_FLAG_HIDDEN;
			LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);

			SetPhase(work, PHASE_CLEAR_EFFECT);

			/* 矢野さんキャラ第２フェーズ */
			work->effect_flag = 2;

			/* ポーズ状態に入る */
			GV_PauseOnActorSystem(GV_PAUSE_MENU);

			ret = 0;
			break;

		default:
			ret = -1;
			break;
		}
	}

	return ret;
}

/*******************************************************************************
 * シーケンス処理
 */
static void VrSequence( WORK *work )
{

	if ( GM_VRStatus & GM_VR_CLEAR ) {
		if ( work->clear_count == 1 ) {

			if ( GM_VRStatus & GM_VR_POSE_CLEAR ) {
				/* ポーズ中にクリア */
				SY_PRINTF2("Pause Clear On.\n");

				/* 開始 */
				if(work->text_work ) MENU_ClearTextTexture(work->text_work);
				if(work->background) work->background->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;
				if(work->bonus_pict) work->bonus_pict->sprite.head.head.flags |=  SPR_FLAG_HIDDEN;
				LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);

				SetPhase(work, PHASE_CLEAR_EFFECT);

				/* 矢野さんキャラ第３フェーズ */
				work->effect_flag = 3;

				DG_SetPrivilegeMode(0);	/* 特権モード解除 */
				GM_SdSet( SNG_FOUTP_SS ) ;
			}
			/* とりあえず、１位のＢＧＭ呼んでおく */
			GM_VoxStream( work->bgmcode[0], 0 ) ;
		}
		work->clear_count++ ;
		if(work->clear_count > 2)
			work->clear_count = 2;
	}
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	int motion_count;

	if(work->flag & VRCLR_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRCLR_INIT;

		/* 各ＵＶ値の取得 */
		if(work->flag & VRCLR_VRTRIAL)
			LOM_SprTex_GetV(&work->layoutman, OBJECT_MODE_T , &work->tex_mod_v0);
		else
			LOM_SprTex_GetV(&work->layoutman, OBJECT_MODE   , &work->tex_mod_v0);
		LOM_SprTex_GetV(&work->layoutman, OBJECT_BUL_WP , &work->tex_wea_v0);
		LOM_SprTex_GetU(&work->layoutman, OBJECT_LEVEL_2, &work->tex_dig_u0);
		SY_PRINTF1("mod v0: %f\n", work->tex_mod_v0);
		SY_PRINTF1("wea v0: %f\n", work->tex_wea_v0);
		SY_PRINTF1("dig u0: %f\n", work->tex_dig_u0);

		/* 初期モード */
		LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
	}

	/* メッセージ処理 */
	ReceiveMessage(work);

	/* シーケンス処理 */
	VrSequence(work);

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* 高速モード */
	if(GV_PadDataDirect[0].status & PAD_CANCEL)
		work->flag |= VRCLR_FAST_MODE;

	/**************************************************************************
	 * 各フェーズ
	 */
	{
		int draw_text = 0;

		work->count++;
		work->count2++;

		switch(work->phase)
		{
		/**********************************************************************
		 * 非表示で待機
		 */
		case PHASE_HIDE:
			break;

		/**********************************************************************
		 * クリアモーション中、一定時間停止
		 */
		case PHASE_CLEAR_MOTION:
			if(1)
			{
				if(work->flag & VRCLR_FINAL_STAGE) motion_count = 90;
				else                               motion_count = 90;
			}
			else
			{
				if(work->flag & VRCLR_FINAL_STAGE) motion_count = 90;
				else                               motion_count = 90;
			}
			if(work->count >= motion_count)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * クリアエフェクト中、一定時間停止
		 */
		case PHASE_CLEAR_EFFECT:

			/* 黒背景出現 */
			if(work->flag & VRCLR_TEXT_MODE)
			{
				if(work->count <= 15)
					SPR_SetColorSprite(work->background, 0, 0, 0, (96 * work->count) / 15);
			}

			if(work->count >= PHASE_CLEAR_EFFECT_COUNT)
			{
				/* 次のフェーズ */
				NextPhase(work);

				/* フレームが開いて、タイトルが出るアクション */
				GM_SdSet(SD_S_LINEMOV1);
				if(work->flag & VRCLR_VRTRIAL)
				{
					LOM_SetCurMode(&work->layoutman, LOM_MODE_57, 1, 1, 1);
				}
				else
				{
					if(work->mission == MSN_MISSION_VR)
					{
						if(work->mode == MSN_MODE_SNEAKING)
						{
							if     (work->weapon == MSN_WEAPON_SNEAKING     ) LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL) LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
							else ASSERT(0);
						}
						else if(work->mode == MSN_MODE_WEAPON)
						{
							if     (work->weapon == MSN_WEAPON_HANDGUN      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE) LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_C4_CLAYMORE  ) LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_GRENADE      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_06, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_PSG1         ) LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_STINGER      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_08, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_NIKITA       ) LOM_SetCurMode(&work->layoutman, LOM_MODE_09, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_HF_BLADE     ) LOM_SetCurMode(&work->layoutman, LOM_MODE_10, 1, 1, 1);
							else ASSERT(0);
						}
						else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW) LOM_SetCurMode(&work->layoutman, LOM_MODE_12, 1, 1, 1);
						else if(work->mode == MSN_MODE_VARIETY          ) LOM_SetCurMode(&work->layoutman, LOM_MODE_13, 1, 1, 1);
						else if(work->mode == MSN_MODE_STREAKING        ) LOM_SetCurMode(&work->layoutman, LOM_MODE_14, 1, 1, 1);
						else ASSERT(0);
					}
					else if(work->mission == MSN_MISSION_ALTERNATIVE)
					{
						if     (work->mode == MSN_MODE_BOMB_DISPOSAL) LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1);
						else if(work->mode == MSN_MODE_ELIMINATE    ) LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1);
						else if(work->mode == MSN_MODE_HOLD_UP      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1);
						else if(work->mode == MSN_MODE_ESCAPE       ) LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1);
						else if(work->mode == MSN_MODE_PHOTOGRAPH   ) LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1);
						else ASSERT(0);
					}
					else ASSERT(0);
				}

				/* スコアの計算 */
				work->score[SCORE_MAP     ] = VR_SCORE;
#ifdef PAL
				work->score[SCORE_TIME    ] = (int)((VR_TIME * ((work->rate[RATE_TIME] * 6.0f) / 5.0f)) + 0.5f);
#else
				work->score[SCORE_TIME    ] = VR_TIME * work->rate[RATE_TIME];
#endif
				work->score[SCORE_M9      ] = GM_WeaponNum(WP_m92     ) * work->rate[RATE_WEAPON_HANDGUN];
				work->score[SCORE_USP     ] = GM_WeaponNum(WP_Usp     ) * work->rate[RATE_WEAPON_HANDGUN];
				work->score[SCORE_SOCOM   ] = GM_WeaponNum(WP_Socom   ) * work->rate[RATE_WEAPON_HANDGUN];
				work->score[SCORE_AKS     ] = GM_WeaponNum(WP_Aks     ) * work->rate[RATE_WEAPON_RIFLE];
				work->score[SCORE_M4      ] = GM_WeaponNum(WP_m4      ) * work->rate[RATE_WEAPON_RIFLE];
				work->score[SCORE_C4      ] = GM_WeaponNum(WP_C4Bomb  ) * work->rate[RATE_WEAPON_EXPLOSIVE];
				work->score[SCORE_CLAYMORE] = GM_WeaponNum(WP_Claymore) * work->rate[RATE_WEAPON_EXPLOSIVE];
				work->score[SCORE_GRENADE ] = GM_WeaponNum(WP_Grenade ) * work->rate[RATE_WEAPON_EXPLOSIVE];
				work->score[SCORE_RGB6    ] = GM_WeaponNum(WP_Rgb6    ) * work->rate[RATE_WEAPON_EXPLOSIVE];
				work->score[SCORE_PSG1    ] = GM_WeaponNum(WP_Psg1    ) * work->rate[RATE_WEAPON_HANDGUN];
				work->score[SCORE_PSG1_T  ] = GM_WeaponNum(WP_Psg1T   ) * work->rate[RATE_WEAPON_HANDGUN];
				work->score[SCORE_STINGER ] = GM_WeaponNum(WP_Stinger ) * work->rate[RATE_WEAPON_EXPLOSIVE];
				work->score[SCORE_NIKITA  ] = GM_WeaponNum(WP_Nikita  ) * work->rate[RATE_WEAPON_EXPLOSIVE];
				work->score[SCORE_WEAPON  ] = 0;
				work->score[SCORE_COMBO   ] = VR_COMBO_CHAIN_MAX * work->rate[RATE_COMBO];
				work->score[SCORE_SNEAKING] = (3 - VR_DiscoverCount) * work->rate[RATE_SNEAKING];
				work->score[SCORE_NO_KILL ] = (VR_KillCount == 0) ? work->rate[RATE_NO_KILL] : 0;
				work->score[SCORE_TOTAL   ] = 0;
				SY_PRINTF1("NO KILL = %d\n", VR_KillCount);
			}
			break;

		/**********************************************************************
		 * クリア画面１を開く
		 */
		case PHASE_L2D_ACTION_1:

#ifdef PAL
			if(work->count == 8)
#else
			if(work->count == 10)
#endif
				GM_SdSet(SD_S_WINOPN01);

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_L2D_ACTION_1_COUNT)
			{
				/* 高速モードのリセット */
				work->flag &= ~VRCLR_FAST_MODE;

				/* 次のフェーズ */
				switch(work->score_type)
				{
				case SCORE_TYPE_01: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); break;
				case SCORE_TYPE_02: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); break;
				case SCORE_TYPE_03: SetPhase(work, PHASE_MAP_SHOW ); LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 1); break;
				case SCORE_TYPE_04: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); break;
				case SCORE_TYPE_05: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); break;
				case SCORE_TYPE_06: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); break;
				case SCORE_TYPE_07: SetPhase(work, PHASE_MAP_SHOW ); LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 1); break;
				case SCORE_TYPE_08: SetPhase(work, PHASE_MAP_SHOW ); LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * マップスコアを表示
		 */
		case PHASE_MAP_SHOW:

			if(work->count == 1)
				SdSCoutry1();

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW2_COUNT)
			{
				/* 次のフェーズ */
				switch(work->score_type)
				{
				case SCORE_TYPE_03: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_18, 1, 1, 1); break;
				case SCORE_TYPE_07: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_18, 1, 1, 1); break;
				case SCORE_TYPE_08: SetPhase(work, PHASE_TIME_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_18, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * タイムの表示
		 */
		case PHASE_TIME_SHOW:

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * タイムの計算
		 */
		case PHASE_TIME_COUNT:

			/* カウント */
			if(work->count < VR_TIME)
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += VRCLR_TIME_COUNT_SPEED * 10 - 1;
				work->count += VRCLR_TIME_COUNT_SPEED - 1;
				if(work->count > VR_TIME)
					work->count = VR_TIME;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= VR_TIME)
			{
				/* 次のフェーズ */
				switch(work->score_type)
				{
				case SCORE_TYPE_01: SetPhase(work, PHASE_NO_KILL_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_24, 1, 1, 1); break;
				case SCORE_TYPE_02: SetPhase(work, PHASE_M9_SHOW     ); LOM_SetCurMode(&work->layoutman, LOM_MODE_19, 1, 1, 1); break;
				case SCORE_TYPE_03: SetPhase(work, PHASE_M9_SHOW     ); LOM_SetCurMode(&work->layoutman, LOM_MODE_20, 1, 1, 1); break;
				case SCORE_TYPE_04: SetPhase(work, PHASE_M9_SHOW     ); LOM_SetCurMode(&work->layoutman, LOM_MODE_19, 1, 1, 1); break;
				case SCORE_TYPE_05: SetPhase(work, PHASE_WAIT_1      ); LOM_SetCurMode(&work->layoutman, LOM_MODE_59, 1, 1, 1); break;
				case SCORE_TYPE_06: SetPhase(work, PHASE_M9_SHOW     ); LOM_SetCurMode(&work->layoutman, LOM_MODE_19, 1, 1, 1); break;
				case SCORE_TYPE_07: SetPhase(work, PHASE_WAIT_1      ); LOM_SetCurMode(&work->layoutman, LOM_MODE_61, 1, 1, 1); break;
#if VRCLR_COMBO_SCORE
				case SCORE_TYPE_08: SetPhase(work, PHASE_COMBO_SHOW  ); LOM_SetCurMode(&work->layoutman, LOM_MODE_21, 1, 1, 1); break;
#else
				case SCORE_TYPE_08: SetPhase(work, PHASE_WAIT_1      ); LOM_SetCurMode(&work->layoutman, LOM_MODE_63, 1, 1, 1); break;
#endif
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * Ｍ９の表示
		 */
		case PHASE_M9_SHOW:

			if(work->score[SCORE_M9] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_USP_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * Ｍ９の計算
		 */
		case PHASE_M9_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_m92))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_m92))
					work->count = GM_WeaponNum(WP_m92);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_m92))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＵＳＰの表示
		 */
		case PHASE_USP_SHOW:
		GO_PHASE_USP_SHOW:

			if(work->score[SCORE_USP] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_SOCOM_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＵＳＰの計算
		 */
		case PHASE_USP_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Usp))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count >= GM_WeaponNum(WP_Usp))
					work->count = GM_WeaponNum(WP_Usp);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Usp))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ソコムの表示
		 */
		case PHASE_SOCOM_SHOW:
		GO_PHASE_SOCOM_SHOW:

			if(work->score[SCORE_SOCOM] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_AKS_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ソコムの計算
		 */
		case PHASE_SOCOM_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Socom))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Socom))
					work->count = GM_WeaponNum(WP_Socom);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Socom))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＡＫＳの表示
		 */
		case PHASE_AKS_SHOW:
		GO_PHASE_AKS_SHOW:

			if(work->score[SCORE_AKS] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_M4_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＡＫＳの計算
		 */
		case PHASE_AKS_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Aks))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Aks))
					work->count = GM_WeaponNum(WP_Aks);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Aks))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * Ｍ４の表示
		 */
		case PHASE_M4_SHOW:
		GO_PHASE_M4_SHOW:

			if(work->score[SCORE_M4] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_C4_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * Ｍ４の計算
		 */
		case PHASE_M4_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_m4))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_m4))
					work->count = GM_WeaponNum(WP_m4);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_m4))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * Ｃ４の表示
		 */
		case PHASE_C4_SHOW:
		GO_PHASE_C4_SHOW:

			if(work->score[SCORE_C4] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_CLAYMORE_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * Ｃ４の計算
		 */
		case PHASE_C4_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_C4Bomb))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_C4Bomb))
					work->count = GM_WeaponNum(WP_C4Bomb);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_C4Bomb))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * クレイモアの表示
		 */
		case PHASE_CLAYMORE_SHOW:
		GO_PHASE_CLAYMORE_SHOW:

			if(work->score[SCORE_CLAYMORE] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_GRENADE_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * クレイモアの計算
		 */
		case PHASE_CLAYMORE_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Claymore))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Claymore))
					work->count = GM_WeaponNum(WP_Claymore);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Claymore))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * グレネードの表示
		 */
		case PHASE_GRENADE_SHOW:
		GO_PHASE_GRENADE_SHOW:

			if(work->score[SCORE_GRENADE] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_RGB6_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * グレネードの計算
		 */
		case PHASE_GRENADE_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Grenade))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Grenade))
					work->count = GM_WeaponNum(WP_Grenade);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Grenade))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＲＧＢ６の表示
		 */
		case PHASE_RGB6_SHOW:
		GO_PHASE_RGB6_SHOW:

			if(work->score[SCORE_RGB6] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_PSG1_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＲＧＢ６の計算
		 */
		case PHASE_RGB6_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Rgb6))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Rgb6))
					work->count = GM_WeaponNum(WP_Rgb6);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Rgb6))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＰＳＧ１の表示
		 */
		case PHASE_PSG1_SHOW:
		GO_PHASE_PSG1_SHOW:

			if(work->score[SCORE_PSG1] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_PSG1_T_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＰＳＧ１の計算
		 */
		case PHASE_PSG1_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Psg1))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Psg1))
					work->count = GM_WeaponNum(WP_Psg1);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Psg1))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＰＳＧ１－Ｔの表示
		 */
		case PHASE_PSG1_T_SHOW:
		GO_PHASE_PSG1_T_SHOW:

			if(work->score[SCORE_PSG1_T] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_STINGER_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ＰＳＧ１－Ｔの計算
		 */
		case PHASE_PSG1_T_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Psg1T))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Psg1T))
					work->count = GM_WeaponNum(WP_Psg1T);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Psg1T))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * スティンガーの表示
		 */
		case PHASE_STINGER_SHOW:
		GO_PHASE_STINGER_SHOW:

			if(work->score[SCORE_STINGER] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_NIKITA_SHOW;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * スティンガーの計算
		 */
		case PHASE_STINGER_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Stinger))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Stinger))
					work->count = GM_WeaponNum(WP_Stinger);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Stinger))
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ニキータの表示
		 */
		case PHASE_NIKITA_SHOW:
		GO_PHASE_NIKITA_SHOW:

			if(work->score[SCORE_NIKITA] < 0)
			{
				NextPhase(work);
				NextPhase(work);
				goto GO_PHASE_AFTER_WEAPON;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ニキータの計算
		 */
		case PHASE_NIKITA_COUNT:

			/* カウント */
			if(work->count < GM_WeaponNum(WP_Nikita))
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > GM_WeaponNum(WP_Nikita))
					work->count = GM_WeaponNum(WP_Nikita);
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= GM_WeaponNum(WP_Nikita))
			{
				/* 次のフェーズ */
			GO_PHASE_AFTER_WEAPON:
				switch(work->score_type)
				{
				case SCORE_TYPE_02: SetPhase(work, PHASE_NO_KILL_SHOW ); LOM_SetCurMode(&work->layoutman, LOM_MODE_25, 1, 1, 1); break;
#if VRCLR_COMBO_SCORE
				case SCORE_TYPE_03: SetPhase(work, PHASE_COMBO_SHOW   ); LOM_SetCurMode(&work->layoutman, LOM_MODE_22, 1, 1, 1); break;
#else
				case SCORE_TYPE_03: SetPhase(work, PHASE_WAIT_1       ); LOM_SetCurMode(&work->layoutman, LOM_MODE_61, 1, 1, 1); break;
#endif
				case SCORE_TYPE_04: SetPhase(work, PHASE_SNEAKING_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_23, 1, 1, 1); break;
				case SCORE_TYPE_06: SetPhase(work, PHASE_WAIT_1       ); LOM_SetCurMode(&work->layoutman, LOM_MODE_60, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * コンボの表示
		 */
		case PHASE_COMBO_SHOW:

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * コンボの計算
		 */
		case PHASE_COMBO_COUNT:

			/* カウント */
			if(work->count < VR_COMBO_CHAIN_MAX)
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count += 10;
//				work->count += VRCLR_SCORE_COUNT_SPEED - 1;
				if(work->count > VR_COMBO_CHAIN_MAX)
					work->count = VR_COMBO_CHAIN_MAX;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= VR_COMBO_CHAIN_MAX)
			{
				/* 次のフェーズ */
				switch(work->score_type)
				{
				case SCORE_TYPE_03: SetPhase(work, PHASE_WAIT_1); LOM_SetCurMode(&work->layoutman, LOM_MODE_61, 1, 1, 1); break;
				case SCORE_TYPE_08: SetPhase(work, PHASE_WAIT_1); LOM_SetCurMode(&work->layoutman, LOM_MODE_63, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * 隠密の表示
		 */
		case PHASE_SNEAKING_SHOW:

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * 隠密の計算
		 */
		case PHASE_SNEAKING_COUNT:

			/* カウント */
			if(work->count < 3)
			{
				SdSCoutry1();

				if(GV_PadDataDirect[0].status & PAD_OK)
					work->count = 3;
			}

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= 3)
			{
				/* 次のフェーズ */
				switch(work->score_type)
				{
				case SCORE_TYPE_04: SetPhase(work, PHASE_NO_KILL_SHOW); LOM_SetCurMode(&work->layoutman, LOM_MODE_26, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * 不殺の表示
		 */
		case PHASE_NO_KILL_SHOW:

			if(work->count == 1)
				SdSCoutry1();

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			if(work->count >= PHASE_ITEM_SHOW2_COUNT)
			{
				/* 次のフェーズ */
				switch(work->score_type)
				{
				case SCORE_TYPE_01: SetPhase(work, PHASE_WAIT_1); LOM_SetCurMode(&work->layoutman, LOM_MODE_59, 1, 1, 1); break;
				case SCORE_TYPE_02: SetPhase(work, PHASE_WAIT_1); LOM_SetCurMode(&work->layoutman, LOM_MODE_60, 1, 1, 1); break;
				case SCORE_TYPE_04: SetPhase(work, PHASE_WAIT_1); LOM_SetCurMode(&work->layoutman, LOM_MODE_62, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * 一定時間停止
		 */
		case PHASE_WAIT_1:

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}
#if 0
			if(work->count >= PHASE_WAIT_1_COUNT)
#else
			if(GV_PadDataDirect[0].press != 0)
#endif
			{
				GM_SdSet(SD_S_WIN01);

				NextPhase(work);

				/* 結果 */
				work->result = Msn_SetNewScore(VR_STAGE_ID, work->score[SCORE_TOTAL]);

				/* 次のアクション */
				switch(work->score_type)
				{
				case SCORE_TYPE_01: LOM_SetCurMode(&work->layoutman, LOM_MODE_27, 1, 1, 1); break;
				case SCORE_TYPE_02: LOM_SetCurMode(&work->layoutman, LOM_MODE_28, 1, 1, 1); break;
				case SCORE_TYPE_03: LOM_SetCurMode(&work->layoutman, LOM_MODE_29, 1, 1, 1); break;
				case SCORE_TYPE_04: LOM_SetCurMode(&work->layoutman, LOM_MODE_30, 1, 1, 1); break;
				case SCORE_TYPE_05: LOM_SetCurMode(&work->layoutman, LOM_MODE_27, 1, 1, 1); break;
				case SCORE_TYPE_06: LOM_SetCurMode(&work->layoutman, LOM_MODE_28, 1, 1, 1); break;
				case SCORE_TYPE_07: LOM_SetCurMode(&work->layoutman, LOM_MODE_29, 1, 1, 1); break;
				case SCORE_TYPE_08: LOM_SetCurMode(&work->layoutman, LOM_MODE_31, 1, 1, 1); break;
				default: ASSERT(0);
				}
			}
			break;

		/**********************************************************************
		 * クリア画面１からクリア画面２へ移行
		 */
		case PHASE_L2D_ACTION_2:

			SetScreen(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen(work);
			}

			/* クリア画面１が消えている途中 */
			if(    (work->score_type == SCORE_TYPE_05)
				|| (work->score_type == SCORE_TYPE_06))
			{
				SetShow(work, OBJECT_NOK   , 0);
				SetShow(work, OBJECT_NOK_EQ, 0);
				SetShow(work, OBJECT_NOK_T5, 0);
				SetShow(work, OBJECT_NOK_T4, 0);
				SetShow(work, OBJECT_NOK_T3, 0);
				SetShow(work, OBJECT_NOK_T2, 0);
				SetShow(work, OBJECT_NOK_T1, 0);
			}
			if(work->score_type == SCORE_TYPE_07)
			{
				SetShow(work, OBJECT_BUL   , 0);
				SetShow(work, OBJECT_BUL_WP, 0);
				SetShow(work, OBJECT_BUL_3 , 0);
				SetShow(work, OBJECT_BUL_2 , 0);
				SetShow(work, OBJECT_BUL_1 , 0);
				SetShow(work, OBJECT_BUL_X , 0);
				SetShow(work, OBJECT_BUL_R3, 0);
				SetShow(work, OBJECT_BUL_R2, 0);
				SetShow(work, OBJECT_BUL_R1, 0);
				SetShow(work, OBJECT_BUL_EQ, 0);
				SetShow(work, OBJECT_BUL_T5, 0);
				SetShow(work, OBJECT_BUL_T4, 0);
				SetShow(work, OBJECT_BUL_T3, 0);
				SetShow(work, OBJECT_BUL_T2, 0);
				SetShow(work, OBJECT_BUL_T1, 0);
				SetShow(work, OBJECT_COM   , 0);
				SetShow(work, OBJECT_COM_3 , 0);
				SetShow(work, OBJECT_COM_2 , 0);
				SetShow(work, OBJECT_COM_1 , 0);
				SetShow(work, OBJECT_COM_X , 0);
				SetShow(work, OBJECT_COM_R3, 0);
				SetShow(work, OBJECT_COM_R2, 0);
				SetShow(work, OBJECT_COM_R1, 0);
				SetShow(work, OBJECT_COM_EQ, 0);
				SetShow(work, OBJECT_COM_T5, 0);
				SetShow(work, OBJECT_COM_T4, 0);
				SetShow(work, OBJECT_COM_T3, 0);
				SetShow(work, OBJECT_COM_T2, 0);
				SetShow(work, OBJECT_COM_T1, 0);
			}
#if VRCLR_COMBO_SCORE==0
			if(    (work->score_type == SCORE_TYPE_03)
				|| (work->score_type == SCORE_TYPE_08))
			{
				SetShow(work, OBJECT_COM   , 0);
				SetShow(work, OBJECT_COM_3 , 0);
				SetShow(work, OBJECT_COM_2 , 0);
				SetShow(work, OBJECT_COM_1 , 0);
				SetShow(work, OBJECT_COM_X , 0);
				SetShow(work, OBJECT_COM_R3, 0);
				SetShow(work, OBJECT_COM_R2, 0);
				SetShow(work, OBJECT_COM_R1, 0);
				SetShow(work, OBJECT_COM_EQ, 0);
				SetShow(work, OBJECT_COM_T5, 0);
				SetShow(work, OBJECT_COM_T4, 0);
				SetShow(work, OBJECT_COM_T3, 0);
				SetShow(work, OBJECT_COM_T2, 0);
				SetShow(work, OBJECT_COM_T1, 0);
			}
#endif

			if(work->count >= PHASE_L2D_ACTION_2_COUNT)
			{
				/* 順位の表示を開始 */
				LOM_SetCurMode(&work->layoutman, LOM_MODE_32, 1, 1, 1);
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * 順位を表示中
		 */
		case PHASE_WAIT_2:

			SetScreen2(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen2(work);
			}

			if(work->count >= PHASE_WAIT_2_COUNT)
			{
				NextPhase(work);

				/* カーソルを表示 */
				if(work->flag & VRCLR_FINAL_STAGE) { LOM_SetCurMode(&work->layoutman, LOM_MODE_34, 1, 1, 1); LOM_SetCurObj (&work->layoutman, OBJECT_EXIT, 1); }
				else                               { LOM_SetCurMode(&work->layoutman, LOM_MODE_33, 1, 1, 1); }

				/* 入力ＯＫだけど一時ＮＧ */
				work->flag  &= ~VRCLR_INPUT_OK;
				work->count  = -30;
			}
			break;

		/**********************************************************************
		 * カーソルを表示して待機
		 */
		case PHASE_CURSOR_ON:

			if(work->count == 0)
				work->flag |= VRCLR_INPUT_OK;

			SetScreen2(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen2(work);
			}
			break;

		/**********************************************************************
		 * 閉じる
		 */
		case PHASE_CLOSE:

			SetScreen2(work);

			if(work->flag & VRCLR_TEXT_MODE)
			{
				draw_text = 1;
				DrawScreen2(work);
			}

			if(work->count == 1)
			{
				if(work->flag & VRCLR_VRTRIAL)
				{
					LOM_SetCurMode(&work->layoutman, LOM_MODE_58, 1, 1, 1);
				}
				else
				{
					if(work->mission == MSN_MISSION_VR)
					{
						if(work->mode == MSN_MODE_SNEAKING)
						{
							if     (work->weapon == MSN_WEAPON_SNEAKING     ) LOM_SetCurMode(&work->layoutman, LOM_MODE_39, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL) LOM_SetCurMode(&work->layoutman, LOM_MODE_40, 1, 1, 1);
							else ASSERT(0);
						}
						else if(work->mode == MSN_MODE_WEAPON)
						{
							if     (work->weapon == MSN_WEAPON_HANDGUN      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_41, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE) LOM_SetCurMode(&work->layoutman, LOM_MODE_42, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_C4_CLAYMORE  ) LOM_SetCurMode(&work->layoutman, LOM_MODE_43, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_GRENADE      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_44, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_PSG1         ) LOM_SetCurMode(&work->layoutman, LOM_MODE_45, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_STINGER      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_46, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_NIKITA       ) LOM_SetCurMode(&work->layoutman, LOM_MODE_47, 1, 1, 1);
							else if(work->weapon == MSN_WEAPON_HF_BLADE     ) LOM_SetCurMode(&work->layoutman, LOM_MODE_48, 1, 1, 1);
							else ASSERT(0);
						}
						else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW) LOM_SetCurMode(&work->layoutman, LOM_MODE_50, 1, 1, 1);
						else if(work->mode == MSN_MODE_VARIETY          ) LOM_SetCurMode(&work->layoutman, LOM_MODE_51, 1, 1, 1);
						else if(work->mode == MSN_MODE_STREAKING        ) LOM_SetCurMode(&work->layoutman, LOM_MODE_52, 1, 1, 1);
						else ASSERT(0);
					}
					else if(work->mission == MSN_MISSION_ALTERNATIVE)
					{
						if     (work->mode == MSN_MODE_BOMB_DISPOSAL) LOM_SetCurMode(&work->layoutman, LOM_MODE_53, 1, 1, 1);
						else if(work->mode == MSN_MODE_ELIMINATE    ) LOM_SetCurMode(&work->layoutman, LOM_MODE_53, 1, 1, 1);
						else if(work->mode == MSN_MODE_HOLD_UP      ) LOM_SetCurMode(&work->layoutman, LOM_MODE_53, 1, 1, 1);
						else if(work->mode == MSN_MODE_ESCAPE       ) LOM_SetCurMode(&work->layoutman, LOM_MODE_53, 1, 1, 1);
						else if(work->mode == MSN_MODE_PHOTOGRAPH   ) LOM_SetCurMode(&work->layoutman, LOM_MODE_53, 1, 1, 1);
						else ASSERT(0);
					}
					else ASSERT(0);
				}
			}

			/* カーソルのフェードアウト */
			{
				SPR_COLOR sprcol;

				if(work->flag & VRCLR_NEXT_STAGE)
				{
					SetShow(work, OBJECT_CURSOR_1, 0);

					LOM_Spr_GetColor(&work->layoutman, OBJECT_CURSOR_1, &sprcol);
					sprcol.a = (30 * (PHASE_CLOSE_COUNT - work->count)) / PHASE_CLOSE_COUNT;
					LOM_Spr_SetColor(&work->layoutman, OBJECT_CURSOR_1, &sprcol);
				}
				else if(work->flag & VRCLR_TRY_AGAIN)
				{
					SetShow(work, OBJECT_CURSOR_2, 0);

					LOM_Spr_GetColor(&work->layoutman, OBJECT_CURSOR_2, &sprcol);
					sprcol.a = (30 * (PHASE_CLOSE_COUNT - work->count)) / PHASE_CLOSE_COUNT;
					LOM_Spr_SetColor(&work->layoutman, OBJECT_CURSOR_2, &sprcol);
				}
				else
				{
					SetShow(work, OBJECT_CURSOR_3, 0);

					LOM_Spr_GetColor(&work->layoutman, OBJECT_CURSOR_3, &sprcol);
					sprcol.a = (30 * (PHASE_CLOSE_COUNT - work->count)) / PHASE_CLOSE_COUNT;
					LOM_Spr_SetColor(&work->layoutman, OBJECT_CURSOR_3, &sprcol);
				}
			}

			if(work->count >= PHASE_CLOSE_COUNT)
			{
				NextPhase(work);

				GM_SdSet(SD_S_WINCLS01);
				if     (work->flag & VRCLR_NEXT_STAGE) LOM_SetCurMode(&work->layoutman, LOM_MODE_54, 1, 1, 1);
				else if(work->flag & VRCLR_TRY_AGAIN ) LOM_SetCurMode(&work->layoutman, LOM_MODE_55, 1, 1, 1);
				else                                   LOM_SetCurMode(&work->layoutman, LOM_MODE_56, 1, 1, 1);
			}
			break;

		/**********************************************************************
		 * 閉じる２
		 */
		case PHASE_CLOSE2:

#ifdef PAL
			if(work->count == 8)
#else
			if(work->count == 10)
#endif
				GM_SdSet(SD_S_LINEMOV1);

			/* 黒背景でフェードアウト */
			if(work->background) 
				SPR_SetColorSprite(work->background, 0, 0, 0,
					(128 * work->count) / PHASE_CLOSE2_COUNT);

			if(work->count >= PHASE_CLOSE2_COUNT)
			{
				if(work->flag & VRCLR_BONUS_MODE) SetPhase(work, PHASE_BONUS_PICT_FADE_IN);
				else                              SetPhase(work, PHASE_END);
			}
			break;

		/**********************************************************************
		 * ボーナス絵フェードイン
		 */
		case PHASE_BONUS_PICT_FADE_IN:

			if(work->background)
				work->background->sprite.head.head.flags |= SPR_FLAG_HIDDEN;

			work->bonus_pict->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;

			SPR_SetColorSprite(work->bonus_pict, 255, 255, 255,
				(128 * work->count) / PHASE_BONUS_PICT_FADE_IN_COUNT);

			if(work->count >= PHASE_BONUS_PICT_FADE_IN_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ボーナス絵表示
		 */
		case PHASE_BONUS_PICT_SHOW:

			if(work->count >= PHASE_BONUS_PICT_SHOW_COUNT)
			{
				NextPhase(work);
			}
			break;

		/**********************************************************************
		 * ボーナス絵フェードアウト
		 */
		case PHASE_BONUS_PICT_FADE_OUT:

			SPR_SetColorSprite(work->bonus_pict, 255, 255, 255,
				(128 * (PHASE_BONUS_PICT_FADE_IN_COUNT - work->count)) / PHASE_BONUS_PICT_FADE_IN_COUNT);

			if(work->count >= PHASE_BONUS_PICT_FADE_OUT_COUNT)
			{
				NextPhase(work);

				work->bonus_pict->sprite.head.head.flags |= SPR_FLAG_HIDDEN;

				if(work->background)
					work->background->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;
			}
			break;

		/**********************************************************************
		 * 終了
		 */
		case PHASE_END:
			if(work->count >= PHASE_END_COUNT)
			{
				if     (work->flag & VRCLR_NEXT_STAGE) GM_ExecProc(work->proc_next_stage, NULL);
				else if(work->flag & VRCLR_TRY_AGAIN ) GM_ExecProc(work->proc_try_again, NULL);
				else                                   GM_ExecProc(work->proc_exit, NULL);
			}
			break;
		}

		/* 高速モード */
		if((PHASE_L2D_ACTION_1 < work->phase) && (work->phase < PHASE_WAIT_1))
		{
			if(work->flag & VRCLR_FAST_MODE)
			{
				work->flag &= ~VRCLR_FAST_MODE;
				SetPhase(work, PHASE_WAIT_1);
				SdSCoutry1();
				switch(work->score_type)
				{
				case SCORE_TYPE_01: LOM_SetCurMode(&work->layoutman, LOM_MODE_59, 1, 1, 1); break;
				case SCORE_TYPE_02: LOM_SetCurMode(&work->layoutman, LOM_MODE_60, 1, 1, 1); break;
				case SCORE_TYPE_03: LOM_SetCurMode(&work->layoutman, LOM_MODE_61, 1, 1, 1); break;
				case SCORE_TYPE_04: LOM_SetCurMode(&work->layoutman, LOM_MODE_62, 1, 1, 1); break;
				case SCORE_TYPE_05: LOM_SetCurMode(&work->layoutman, LOM_MODE_59, 1, 1, 1); break;
				case SCORE_TYPE_06: LOM_SetCurMode(&work->layoutman, LOM_MODE_60, 1, 1, 1); break;
				case SCORE_TYPE_07: LOM_SetCurMode(&work->layoutman, LOM_MODE_61, 1, 1, 1); break;
				case SCORE_TYPE_08: LOM_SetCurMode(&work->layoutman, LOM_MODE_63, 1, 1, 1); break;
				default: ASSERT(0);
				}
 			}
		}

		/* テキスト表示 */
		if(draw_text == 1)
		{
			MENU_PutTextScreen(work->text_work,
				0, 0, VRDEF_SCREEN_X, VRDEF_SCREEN_Y,
				0, 0, VRCLR_TEXT_BUF_X, VRCLR_TEXT_BUF_Y,
				VRDEF_ACTIVE);
		}
	}

	/* 点滅 */
	if((PHASE_L2D_ACTION_1 < work->phase) && (work->phase < PHASE_CLOSE))
	{
		unsigned char alpha1, alpha2;
		SPR_COLOR     sprcol;

#ifdef PAL
		if((work->count2 % 100) < 84) alpha1 = 32 + ((96 * (work->count2 % 100)) / 84);
		else                          alpha1 = 32 + ((96 * (16 - ((work->count2 % 100) - 84))) / 16);

		alpha2 = 32 + ((96 * (work->count2 % 50)) / 50);
#else
		if((work->count2 % 120) < 100) alpha1 = 32 + ((96 * (work->count2 % 120)) / 100);
		else                           alpha1 = 32 + ((96 * (20 - ((work->count2 % 120) -  100))) / 20);

		alpha2 = 32 + ((96 * (work->count2 % 60)) / 60);
#endif
		if(work->flag & VRCLR_VRTRIAL)
		{
			LOM_Spr_GetColor(&work->layoutman, OBJECT_VR_TRAINING_T, &sprcol); sprcol.a = alpha1;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_VR_TRAINING_T, &sprcol);
		}
		else
		{
			LOM_Spr_GetColor(&work->layoutman, OBJECT_VR_TRAINING, &sprcol); sprcol.a = alpha1;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_VR_TRAINING, &sprcol);
		}

		if(work->phase > PHASE_WAIT_2)
		{
			if(work->result == 1)
			{
#ifdef PAL
				if((work->count2 % 50) == 0) GM_SdSet(SD_S_LINEMOV1);
#else
				if((work->count2 % 60) == 0) GM_SdSet(SD_S_LINEMOV1);
#endif
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_7, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_7, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_6, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_6, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_5, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_5, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_4, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_4, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_3, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_3, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_2, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_2, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_1ST_1, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_1ST_1, &sprcol);
			}
			else if(work->result == 2)
			{
#ifdef PAL
				if((work->count2 % 50) == 0) GM_SdSet(SD_S_LINEMOV1);
#else
				if((work->count2 % 60) == 0) GM_SdSet(SD_S_LINEMOV1);
#endif
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_7, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_7, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_6, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_6, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_5, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_5, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_4, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_4, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_3, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_3, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_2, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_2, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_2ND_1, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_2ND_1, &sprcol);
			}
			else if(work->result == 3)
			{
#ifdef PAL
				if((work->count2 % 50) == 0) GM_SdSet(SD_S_LINEMOV1);
#else
				if((work->count2 % 60) == 0) GM_SdSet(SD_S_LINEMOV1);
#endif
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_7, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_7, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_6, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_6, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_5, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_5, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_4, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_4, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_3, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_3, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_2, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_2, &sprcol);
				LOM_Spr_GetColor(&work->layoutman, OBJECT_3RD_1, &sprcol); sprcol.a = alpha2;
				LOM_Spr_SetColor(&work->layoutman, OBJECT_3RD_1, &sprcol);
			}
		}
#ifdef PAL
		if((work->count2 % 100) == 0)
#else
		if((work->count2 % 120) == 0)
#endif
			work->count2 = 0;
	}
#endif
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	/* ＴＲＩの解放 */
	if(work->tri != -1)
		if(SPR_KillTexture(work->tri) == -1)
			SY_PRINTF2("Kill TRI Failed.\n");

	/* スプライトの解放 */
	if(work->bonus_pict)
		if(SPR_Destroy_2D_Object(work->bonus_pict) == -1)
			SY_PRINTF2("Destroy Sprite Failed.\n");
	if(work->background)
		if(SPR_Destroy_2D_Object(work->background) == -1)
			SY_PRINTF2("Destroy Sprite Failed.\n");

	/* レイアウトマネージャ */
	DestroyLayoutman(&work->layoutman);

	/* 多重起動防止 */
	_work = NULL;
}

/*******************************************************************************
 * 最終ステージ判定
 */
static int IsFinalStage(
	WORK *work)		/* ワーク */
{
   BP_TODO_BREAK;
#if 0 //BP
	if(work->flag & VRCLR_VRTRIAL)
	{
		if(VR_STAGE_ID == VRDEF_TRIAL_5)
			return 1;
		else
			return 0;
	}
	else
	{
		/* ミッション */
		if(work->mission == MSN_MISSION_VR)
		{
			if(work->mode == MSN_MODE_SNEAKING)
			{
				if(work->weapon == MSN_WEAPON_SNEAKING)
				{
					if(VR_STAGE_ID > VRM_SNK_SNK_END - VRM_SNK_SNK_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
				{
					if(VR_STAGE_ID > VRM_SNK_ELM_END - VRM_SNK_ELM_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
			}
			else if(work->mode == MSN_MODE_WEAPON)
			{
				if(work->weapon == MSN_WEAPON_HANDGUN)
				{
					if(VR_STAGE_ID > VRM_WPN_HGN_END - VRM_WPN_HGN_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
				{
					if(VR_STAGE_ID > VRM_WPN_ASR_END - VRM_WPN_ASR_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
				{
					if(VR_STAGE_ID > VRM_WPN_C4C_END - VRM_WPN_C4C_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_GRENADE)
				{
					if(VR_STAGE_ID > VRM_WPN_GRN_END - VRM_WPN_GRN_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_PSG1)
				{
					if(VR_STAGE_ID > VRM_WPN_PSG_END - VRM_WPN_PSG_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_STINGER)
				{
					if(VR_STAGE_ID > VRM_WPN_STG_END - VRM_WPN_STG_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_NIKITA)
				{
					if(VR_STAGE_ID > VRM_WPN_NKT_END - VRM_WPN_NKT_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
				else if(work->weapon == MSN_WEAPON_HF_BLADE)
				{
					if(VR_STAGE_ID > VRM_WPN_HFB_END - VRM_WPN_HFB_PLAYER_NUM)
						return 1;
					else
						return 0;
				}
			}
			else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
			{
				if(VR_STAGE_ID > VRM_FPV_END - VRM_FPV_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
			else if(work->mode == MSN_MODE_VARIETY)
			{
				if(VR_STAGE_ID > VRM_VRT_END - VRM_VRT_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
			else if(work->mode == MSN_MODE_STREAKING)
			{
				if(VR_STAGE_ID > VRM_STR_END - VRM_STR_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
		}
		else if(work->mission == MSN_MISSION_ALTERNATIVE)
		{
			if(work->mode == MSN_MODE_BOMB_DISPOSAL)
			{
				if(VR_STAGE_ID > ALT_BMB_END - ALT_BMB_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
			else if(work->mode == MSN_MODE_ELIMINATE)
			{
				if(VR_STAGE_ID > ALT_ELM_END - ALT_ELM_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
			else if(work->mode == MSN_MODE_HOLD_UP)
			{
				if(VR_STAGE_ID > ALT_HLD_END - ALT_HLD_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
			else if(work->mode == MSN_MODE_ESCAPE)
			{
				if(VR_STAGE_ID > ALT_ESC_END - ALT_ESC_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
			else if(work->mode == MSN_MODE_PHOTOGRAPH)
			{
				if(VR_STAGE_ID > ALT_PHT_END - ALT_PHT_PLAYER_NUM)
					return 1;
				else
					return 0;
			}
		}
	}
#endif
	return 0;
}

/******************************************************************************
 * 初期化処理

chara ＶＲクリア[NewVRClear_Scn] $s:名前 \
	-rate_A     $i:秒変換レート \
	-rate_B     $i:単発系武器変換レート \
	-rate_C     $i:連射系武器変換レート \
	-rate_D     $i:爆発系武器変換レート \
	-rate_E     $i:コンボ変換レート \
	-rate_F     $i:隠密変換レート \
	-rate_G     $i:不殺変換レート \
	-next_stage $p:ＮＥＸＴＳＴＡＧＥプロック \
	-try_again  $p:ＴＲＹＡＧＡＩＮプロック \
	-exit       $p:ＥＸＩＴプロック \
	-bgM        $t:１位ＢＧＭ $t:２位ＢＧＭ $t:３位ＢＧＭ $t:４位以下ＢＧＭ \
	-pict       $s:tri名  $s:予告テクスチャ名 \
	-score_type $i:スコア項目セット \
	-flag       $i:フラグ

// フラグ
#define VRCLR_TEXT_MODE (0x0002)	// テキスト表示モード
#define VRCLR_VRTRIAL   (0x0010)	// 体験版

// スコア項目セット
#enum {
	SCORE_TYPE_01 = 0,	//            TIME                           NO KILL
	SCORE_TYPE_02,		//            TIME  BULLET                   NO KILL
	SCORE_TYPE_03,		// MAP SCORE  TIME  BULLET  COMBO
	SCORE_TYPE_04,		//            TIME  BULLET         SNEAKING  NO KILL
	SCORE_TYPE_05,		//            TIME
	SCORE_TYPE_06,		//            TIME  BULLET
	SCORE_TYPE_07,		// MAP SCORE  TIME
	SCORE_TYPE_08,		// MAP SCORE  TIME          COMBO
	SCORE_TYPE_MAX,
};
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
   BP_TODO_BREAK;
#if 0 //BP
	SPR_POS spr_pos;
	int     tri = 0, tex = 0;

	SY_PRINTF3("GetResources\n");

	work->name            = name;
	work->proc_next_stage = 0;
	work->proc_try_again  = 0;
	work->proc_exit       = 0;

	work->text_work       = NULL;
	work->background      = NULL;
	work->bonus_pict      = NULL;

	memset(work->score, 0x00, sizeof(work->score));
	work->rate[RATE_TIME            ] = VRCLR_RATE_TIME;
	work->rate[RATE_WEAPON_HANDGUN  ] = VRCLR_RATE_WEAPON_HANDGUN;
	work->rate[RATE_WEAPON_RIFLE    ] = VRCLR_RATE_WEAPON_RIFLE;
	work->rate[RATE_WEAPON_EXPLOSIVE] = VRCLR_RATE_WEAPON_EXPLOSIVE;
	work->rate[RATE_COMBO           ] = VRCLR_RATE_COMBO;
	work->rate[RATE_SNEAKING        ] = VRCLR_RATE_SNEAKING;
	work->rate[RATE_NO_KILL         ] = VRCLR_RATE_NO_KILL;
	work->flag            = 0;

	work->tri             = -1;
	work->tex_mod_v0      = 0.0f;
	work->tex_wea_v0      = 0.0f;
	work->tex_dig_u0      = 0.0f;

	work->phase           = PHASE_HIDE;
	work->count           = 0;
	work->count2          = 0;
	work->result          = 0;
	work->effect_flag     = 0;

	/* ステージモードの取得 */
	Msn_GetCurrentStageInfo(&work->mission, &work->mode, &work->weapon, &work->level, &work->player);
	work->level++;

	/* スコア項目セット */
	if(work->mission == MSN_MISSION_VR)
	{
		if(work->mode == MSN_MODE_SNEAKING)
		{
			if     (work->weapon == MSN_WEAPON_SNEAKING     ) work->score_type = SCORE_TYPE_01;
			else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL) work->score_type = SCORE_TYPE_02;
			else ASSERT(0);
		}
		else if(work->mode == MSN_MODE_WEAPON)
		{
			if     (work->weapon == MSN_WEAPON_HANDGUN      ) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_C4_CLAYMORE  ) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_GRENADE      ) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_PSG1         ) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_STINGER      ) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_NIKITA       ) work->score_type = SCORE_TYPE_03;
			else if(work->weapon == MSN_WEAPON_HF_BLADE     ) work->score_type = SCORE_TYPE_08;
			else ASSERT(0);
		}
		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW) work->score_type = SCORE_TYPE_04;
		else if(work->mode == MSN_MODE_VARIETY          ) work->score_type = SCORE_TYPE_05;
		else if(work->mode == MSN_MODE_STREAKING        ) work->score_type = SCORE_TYPE_05;
		else ASSERT(0);
	}
	else if(work->mission == MSN_MISSION_ALTERNATIVE)
	{
		if     (work->mode == MSN_MODE_BOMB_DISPOSAL) work->score_type = SCORE_TYPE_04;
		else if(work->mode == MSN_MODE_ELIMINATE    ) work->score_type = SCORE_TYPE_04;
		else if(work->mode == MSN_MODE_HOLD_UP      ) work->score_type = SCORE_TYPE_05;
		else if(work->mode == MSN_MODE_ESCAPE       ) work->score_type = SCORE_TYPE_01;
		else if(work->mode == MSN_MODE_PHOTOGRAPH   ) work->score_type = SCORE_TYPE_07;
		else ASSERT(0);
	}
	else ASSERT(0);

	/* シナリオオプション */
	if(name != 0)
	{
		/* flag */
		if(GCL_GetOption('f') != NULL) work->flag = GCL_GetNextInt();

		/* rate */
		if(GCL_GetOption('A') != NULL) work->rate[RATE_TIME            ] = GCL_GetNextInt();
		if(GCL_GetOption('B') != NULL) work->rate[RATE_WEAPON_HANDGUN  ] = GCL_GetNextInt();
		if(GCL_GetOption('C') != NULL) work->rate[RATE_WEAPON_RIFLE    ] = GCL_GetNextInt();
		if(GCL_GetOption('D') != NULL) work->rate[RATE_WEAPON_EXPLOSIVE] = GCL_GetNextInt();
		if(GCL_GetOption('E') != NULL) work->rate[RATE_COMBO           ] = GCL_GetNextInt();
		if(GCL_GetOption('F') != NULL) work->rate[RATE_SNEAKING        ] = GCL_GetNextInt();
		if(GCL_GetOption('G') != NULL) work->rate[RATE_NO_KILL         ] = GCL_GetNextInt();

		/* proc */
		if(GCL_GetOption('n') != NULL) work->proc_next_stage = GCL_GetNextInt();
		if(GCL_GetOption('t') != NULL) work->proc_try_again  = GCL_GetNextInt();
		if(GCL_GetOption('e') != NULL) work->proc_exit       = GCL_GetNextInt();

		/* Clear BGM */
		if(GCL_GetOption('M') != NULL) {
			work->bgmcode[0] = GCL_GetNextInt() ;	/* １位 */
			work->bgmcode[1] = GCL_GetNextInt() ;	/* ２位 */
			work->bgmcode[2] = GCL_GetNextInt() ;	/* ３位 */
			work->bgmcode[3] = GCL_GetNextInt() ;	/* ４位以下 */
			work->clear_count = 0 ;
		}

		/* pict */
		if(GCL_GetOption('p') != NULL)
		{
			tri = GCL_GetNextInt();
			tex = GCL_GetNextInt();
			work->flag |= VRCLR_BONUS_MODE;
		}

		/* score type */
		if(GCL_GetOption('s') != NULL) work->score_type = GCL_GetNextInt();
		if((work->score_type < 0) || (SCORE_TYPE_MAX <= work->score_type))
		{
			SY_PRINTF2("Wrong Value In score_type Parameter,\n");
			ASSERT(0);
		}

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}
	SY_PRINTF1("SCORE TYPE: %d\n", work->score_type);

	/* レイアウトマネージャ */
	if(work->flag & VRCLR_VRTRIAL)
	{
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME2, 4, 0, 0, 0, 0, LOMCallback, NULL, work, VRCLR_INPUT_INTERVAL+9, VRCLR_INPUT_INTERVAL+4, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0)
		{
			SY_PRINTF2("Create Layoutman Failed.\n");
			return 0;
		}
		SY_PRINTF1("Create Layoutman Succeeded.\n");
	}
	else
	{
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, NULL, work, VRCLR_INPUT_INTERVAL+9, VRCLR_INPUT_INTERVAL+4, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0)
		{
			SY_PRINTF2("Create Layoutman Failed.\n");
			return 0;
		}
		SY_PRINTF1("Create Layoutman Succeeded.\n");
	}

	if(LoadLOMData(&work->layoutman, _lom_data) == 0)
	{
		SY_PRINTF2("Load LOM Data Failed.\n");
		return 0;
	}
	SY_PRINTF1("Load LOM Data Succeeded.\n");

	/* ステージ番号修正 */
	if(work->flag & VRCLR_VRTRIAL)
	{
		switch(VR_STAGE_ID)
		{
		case VRDEF_TRIAL_1: work->level = 1; break;
		case VRDEF_TRIAL_2: work->level = 2; break;
		case VRDEF_TRIAL_3: work->level = 3; break;
		case VRDEF_TRIAL_4: work->level = 4; break;
		case VRDEF_TRIAL_5: work->level = 5; break;
		default           : ASSERT(0); break;
		}
	}
	SY_PRINTF1("STAGE INFO: %d-%d-%d-%d-%d\n", work->mission, work->mode, work->weapon, work->level, work->player);

	/* 最終ステージ判定 */
	if(IsFinalStage(work) == 1)
		work->flag |= VRCLR_FINAL_STAGE;

	/* バックグラウンド */
	{
		if((work->background = SPR_Create_2D_Object(SP_SPRITE, 4, NULL)) == NULL)
		{
			SY_PRINTF2("Create Sprite Failed.\n");
			return 0;
		}
		memset(&spr_pos, 0x00, sizeof(spr_pos));
		if(work->flag & VRCLR_TEXT_MODE) SPR_SetPriority(work->background, SPR_PRI_MOST_NEAR - 1);
		else                             SPR_SetPriority(work->background, SPR_PRI_MOST_NEAR);
		SPR_SetPosSprite  (work->background, &spr_pos);
		SPR_SetSizeSprite (work->background, VRDEF_SCREEN_X, VRDEF_SCREEN_Y);
		SPR_SetColorSprite(work->background, 0, 0, 0, 0);
		work->background->sprite.head.head.flags |= SPR_FLAG_ALPHA;
		work->background->sprite.head.head.alpha  = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00);
	}

	/* テキストモード */
	if(work->flag & VRCLR_TEXT_MODE)
	{
		SY_PRINTF1("TEXT MODE\n");

		/* テキストのワーク */
		if((work->text_work = NewTextScreenControlEx(
			VRCLR_TEXT_BUF_X, VRCLR_TEXT_BUF_Y, 240, 0x0004)) == NULL)
		{
			SY_PRINTF2("Create Text Screen Control Failed.\n");
			return 0;
		}
		GV_SetActorChild(work, work->text_work);
	}

	/* ボーナスモード */
	if(work->flag & VRCLR_BONUS_MODE)
	{
		SY_PRINTF1("BONUS MODE\n");

		/* ボーナス絵 */
		if((work->bonus_pict = SPR_Create_2D_Object(SP_SPRITE, 4, NULL)) == NULL)
		{
			SY_PRINTF2("Create Sprite Failed.\n");
			return 0;
		}
		if(tri != 0)
		{
			if((work->tri = SPR_LoadTexture(tri)) == -1)
			{
				SY_PRINTF2("Load TRI[%d] Failed.\n", tri);
				return 0;
			}
			if(tex != 0)
			{
				if(SPR_ObjSetTexture(work->bonus_pict, tex, work->tri) == -1)
				{
					SY_PRINTF2("Set Texture[%d] Failed.\n", tex);
					return 0;
				}
			}
		}
		memset(&spr_pos, 0x00, sizeof(spr_pos));
		SPR_SetPosSprite  (work->bonus_pict, &spr_pos);
		SPR_SetSizeSprite (work->bonus_pict, VRDEF_SCREEN_X, VRDEF_SCREEN_Y);
		SPR_SetColorSprite(work->bonus_pict, 255, 255, 255, 0);
		work->bonus_pict->sprite.head.head.flags |= SPR_FLAG_ALPHA;
		work->bonus_pict->sprite.head.head.alpha  = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00);
	}

	/* 矢野さんキャラ起動（非表示） */
	{
		void *ptr;
		work->effect_flag = 0;
		if((ptr = NewVRClearEffect2(0, &work->effect_flag)) != NULL)
			GV_SetActorChild(work, ptr);
	}

	work->flag |= VRCLR_INIT;

	/* 多重起動防止 */
	_work = work;
#endif
	return 1;
}

/*******************************************************************************
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
void *NewVRClear(void)
{
	WORK *work;

	SY_PRINTF3("NewVRClear\n");
	OPERATOR();

	/* 多重起動防止 */
	if(_work != NULL)
		return(_work);

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_AFTER2,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, 0, GM_CurrentMap))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRClear_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRClear_Scn\n");
	OPERATOR();

	/* 多重起動防止 */
	if(_work != NULL)
		return(_work);

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_AFTER2,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
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
	}

	return (void *)work;
}
