#ifndef _c_layout_h_
#define _c_layoyt_h_

#include "codec_config.h"


/* VGA 座標を 2Dモジュール座標に変換するマクロ */
#define xV2S(x)   ((float)(x) * 512.0 / 640.0)
#define yV2S(y)   ((float)(y) * 384.0 / 480.0)

#define LAYOUT_PRIORITY  3   /* レイアウトパーツのスプライトプライオリティ */

/* =========================================================================
 * パーツ定義
 * TRI および、その中に含まれるテクスチャの strcode
 * ========================================================================= */
#define LAYOUT_TRI              6888713         /* codec.tri */

#define LAYOUT_14               8936529
#define LAYOUT_dot              8936937
#define LAYOUT_font             6852885
#define LAYOUT_frame_alp_ovl    4191965
#define LAYOUT_mater            5070504
#define LAYOUT_mater_bg         649813
#define LAYOUT_mem              4283916
#define LAYOUT_mem_dark         16745311
#define LAYOUT_press_alp_ovl    956142
#define LAYOUT_ptt              2645530
#define LAYOUT_ptt_dark         13468539
#define LAYOUT_tune             5871661
#define LAYOUT_tune_dark        3143586

#define LAYOUT_num_alp_ovl      4283922
#define LAYOUT_ota_alp_ovl      1400342




/* =========================================================================
 * パーツ位置指定
 * ========================================================================= */

/* 表示基準位置 */
#define PartsX_center     320
#define PartsY_center     CODEC_PARTS_LEVEL

/* 実際の表示を行う際の位置補正 */
#define PartsX_adjust     0
#define PartsY_adjust     0

/* 最終的な基準位置 */
#define PartsX_base    xV2S(PartsX_center + PartsX_adjust)
#define PartsY_base    yV2S(PartsY_center + PartsY_adjust)


/* テキスト送り矢印 */
#define PartsX_NxtArrow   (xV2S(530) - PartsX_base)
#define PartsY_NxtArrow   (yV2S(368) - PartsY_base)
#define PartsW_NxtArrow   xV2S(14)
#define PartsH_NxtArrow   yV2S(8)



/*
 * 外枠
 */
/* -- 閉じ状態 -- */
#define BoxX1_Close      xV2S(218 - PartsX_center)   /* 左上 */
#define BoxY1_Close      yV2S( 89 - PartsY_center)
#define BoxX2_Close      xV2S(421 - PartsX_center)   /* 右下 */
#define BoxY2_Close      yV2S(182 - PartsY_center)

/* -- 開き状態 -- */
#define BoxX1_Open       xV2S( 69 - PartsX_center)   /* 左上 */
#define BoxY1_Open       yV2S( 58 - PartsY_center)
#define BoxX2_Open       xV2S(570 - PartsX_center)   /* 右下 */ 
#define BoxY2_Open       yV2S(213 - PartsY_center)

/*
 * 周波数表示
 */
/* "14" の表示 */
#define NumX_base        xV2S(312 - PartsX_center)
#define NumY_base        xV2S(137 - PartsY_center)

#define NumX_14          (xV2S(312 - PartsX_center) - NumX_base)
#define NumY_14          (yV2S(137 - PartsY_center) - NumY_base)
#define NumW_14          xV2S(22)
#define NumH_14          yV2S(16)

/* 小数点 */
#define NumX_dot         (xV2S(354 - PartsX_center) - NumX_base)		      
#define NumY_dot         (yV2S(145 - PartsY_center) - NumY_base)
#define NumW_dot         xV2S(8)
#define NumH_dot         yV2S(8)

/* 数字 */
#define NumY_num         (yV2S(130 - PartsY_center) - NumY_base)
#define NumW_num         xV2S(20)
#define NumH_num         yV2S(24)
#define NumTexW          20

#define NumX_num_1_00    (xV2S(335 - PartsX_center) - NumX_base)   /* 1.00 */
#define NumX_num_0_10    (xV2S(362 - PartsX_center) - NumX_base)   /* 0.10 */
#define NumX_num_0_01    (xV2S(382 - PartsX_center) - NumX_base)   /* 0.01 */


/*
 * グループA: 上パーツ構成部品
 */
/* 上パーツは、常に全体枠の上辺と等間隔を保つ */
#define GroupA_X_Open    xV2S(0)
#define GroupA_Y_Open    (yV2S(7) + BoxY1_Open)
#define GroupA_X_Close   xV2S(0)
#define GroupA_Y_Close   (yV2S(7) + BoxY1_Close)

/* 枠のライン */
#define ApartsX1_line    (xV2S(225 - PartsX_center) - GroupA_X_Open)
#define ApartsY1_line    yV2S(0)
#define ApartsX2_line    (xV2S(414 - PartsX_center) - GroupA_X_Open)
#define ApartsY2_line    yV2S(10)

/* PTT */
#define ApartsX_PTT      (xV2S(236 - PartsX_center) - GroupA_X_Close)
#define ApartsY_PTT      (yV2S(103 - PartsY_center) - GroupA_Y_Close)
#define ApartsW_PTT      xV2S(44)
#define ApartsH_PTT      yV2S(12)


/*
 * グループB: 下パーツ構成部品
 */
/* 下パーツは、常に全体枠の下辺と等間隔を保つ */
#define GroupB_X_Open    xV2S(0)
#define GroupB_Y_Open    (BoxY2_Open - yV2S(7))
#define GroupB_X_Close   xV2S(0)
#define GroupB_Y_Close   (BoxY2_Close - yV2S(7))

/* 枠のライン */
#define BpartsX1_line    (xV2S(225 - PartsX_center) - GroupB_X_Open)
#define BpartsY1_line    yV2S(0)
#define BpartsX2_line    (xV2S(414 - PartsX_center) - GroupB_X_Open)
#define BpartsY2_line    yV2S(-10)

/* MEMORY */
#define BpartsX_memory   (xV2S(235 - PartsX_center) - GroupB_X_Close)
#define BpartsY_memory   (yV2S(159.2F - PartsY_center) - GroupB_Y_Close)
#define BpartsW_memory   60                      /* xV2S(74) */
#define BpartsH_memory   (10.0F * 384.0F/448.0F) /* yV2S(11) */

/* < TUNING > */
#define BpartsX_tuning   (xV2S(326.25F - PartsX_center) - GroupB_X_Close)
#define BpartsY_tuning   (yV2S(159.2F - PartsY_center) - GroupB_Y_Close)
#define BpartsW_tuning   62   /* xV2S(78) */
#define BpartsH_tuning   (10.0F * 384.0F/448.0F) /* yV2S(11) */


/* メータ */
#define PartsX_Mater     (xV2S(234 - PartsX_center) - GroupB_X_Open)
#define PartsY_Mater     (yV2S( 87 - PartsY_center) - GroupB_Y_Open)
#define PartsW_Mater     xV2S(174)
#define PartsH_Mater     yV2S(100)

#define Mater_Lines        9    /* メータのラインの本数 */


/* 点灯/非点灯状態 */
#define CODEC_LIGHT_PTT     0x0001
#define CODEC_LIGHT_MEMORY  0x0002
#define CODEC_LIGHT_LEFT    0x0010
#define CODEC_LIGHT_RIGHT   0x0020
#define CODEC_LIGHT_TUNING  0x0040


int codecActionStatus(void);
int codecSetFreq(int freq);
int codecCloseFrame(void);
int codecOpenFrame(int fade);
int codecSetLevel(int level);
int codecSetLight(int status);
int codecLayoutDisp(int sw);

#define CalcMovePos(min, max, total, cnt) ((min) + (cnt) * ((max) - (min)) / total)

#endif /* _c_layout_h_ */
