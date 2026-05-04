#ifndef _memcall_h_
#define _memcall_h_

/*
 * 名前表示用字幕情報
 */

#define NAME_LABEL_W  (FONT_SIZE_W * 8 * 2)
#define NAME_LABEL_H  (FONT_SIZE_H + FONT_SIZE_H/2)

/*
 * 座標変換マクロ
 */
/* VGA 座標から物理座標系へ       */
#define xV2P(x)  (float)((x) * (float)DRAW_WIDTH / 640.0F)
#define yV2P(y)  (float)((y) * (float)DRAW_HEIGHT / 480.0F)

#define MC_LINE_R   60  /* 76 */
#define MC_LINE_G   220 /* 91 */
#define MC_LINE_B   200 /* 87 */
#define MC_LINE_A   64  /*    */

#define MC_FREQ_LIGHT_R  55
#define MC_FREQ_LIGHT_G  66
#define MC_FREQ_LIGHT_B  69
#define MC_FREQ_LIGHT_A  0

#define MC_FREQ_DARK_R   10
#define MC_FREQ_DARK_G   19
#define MC_FREQ_DARK_B   16
#define MC_FREQ_DARK_A   0




/* 基準ポイント */
#define MC_BASE_X  320.0F
#define MC_BASE_Y  224.0F

#define MC_adjust_x  0
#define MC_adjust_y  0

#define MC_disp_x  xV2S(MC_BASE_X + MC_adjust_x)
#define MC_disp_y  yV2S(MC_BASE_Y + MC_adjust_y)

/* 左列アンダーラインの両端 x 座標 */
#define MC_LEFT_ULINE_X1   xV2S( 47 - MC_BASE_X)
#define MC_LEFT_ULINE_X2   xV2S(316 - MC_BASE_X)

/* 右列アンダーラインの両端 x 座標 */
#define MC_RIGHT_ULINE_X1  xV2S(322 - MC_BASE_X)
#define MC_RIGHT_ULINE_X2  xV2S(591 - MC_BASE_X)

/* 各行の y 座標 */
#define MC_UPPER_Y         yV2S(224 - MC_BASE_Y)
#define MC_ULINE_Y0        yV2S(262 - MC_BASE_Y)
#define MC_ULINE_Y1        yV2S(310 - MC_BASE_Y)
#define MC_ULINE_Y2        yV2S(358 - MC_BASE_Y)
#define MC_LOWER_Y         yV2S(407 - MC_BASE_Y)

/* 上部 linestrip の各頂点 */
#define MC_UPPER_X1        xV2S( 39 - MC_BASE_X)
#define MC_UPPER_Y1        (yV2S(213 - MC_BASE_Y) - MC_UPPER_Y)
#define MC_UPPER_X2        xV2S(600 - MC_BASE_X)
#define MC_UPPER_Y2        (yV2S(224 - MC_BASE_Y) - MC_UPPER_Y)

/* 下部 linestrip の各頂点 */
#define MC_LOWER_X1        MC_UPPER_X1
#define MC_LOWER_Y1        (yV2S(418 - MC_BASE_Y) - MC_LOWER_Y)
#define MC_LOWER_X2        MC_UPPER_X2
#define MC_LOWER_Y2        (yV2S(407 - MC_BASE_Y) - MC_LOWER_Y)

/* 周波数表示の x 座標と幅、高さ */
#define MC_LEFT_FREQ_X     xV2S( 52 - MC_BASE_X)
#define MC_RIGHT_FREQ_X    xV2S(327 - MC_BASE_X)
#define MC_FREQ_Y          yV2S(-22)
#define MC_FREQ_W          xV2S(60)
#define MC_FREQ_H          yV2S(14)

#define MC_FREQ_X_14       0
#define MC_FREQ_X_1_00     (MC_FREQ_W / 10.6F * 3.6F)
#define MC_FREQ_X_dot      (MC_FREQ_W / 10.6F * 6.0F)
#define MC_FREQ_X_0_10     (MC_FREQ_W / 10.6F * 7.0F)
#define MC_FREQ_X_0_01     (MC_FREQ_W / 10.6F * 9.0F)

#define MC_FREQ_W_14       (MC_FREQ_W / 10.6F * 3.6F)
#define MC_FREQ_W_1_00     (MC_FREQ_W / 10.6F * 2.0F)
#define MC_FREQ_W_dot      (MC_FREQ_W / 10.6F * 1.0F)
#define MC_FREQ_W_0_10     (MC_FREQ_W / 10.6F * 2.0F)
#define MC_FREQ_W_0_01     (MC_FREQ_W / 10.6F * 2.0F)

#define MC_FREQ_R          (208/2)
#define MC_FREQ_G          (231/2)
#define MC_FREQ_B          (193/2)

#define MC_FREQ_R_dark     (37/2)
#define MC_FREQ_G_dark     (48/2)
#define MC_FREQ_B_dark     (37/2)

/* 名前表示の絶対 X 座標(これらは相対座標ではない) */
#define MC_LABEL_MARGINE  76
/* 表示位置(左右) */
#define MC_ABS_LEFT_X     xV2P(MC_LABEL_MARGINE + 52)
#define MC_ABS_RIGHT_X    xV2P(MC_LABEL_MARGINE + 327)

/* 表示位置(高さ) */
#define MC_ABS_LEVEL_0    yV2P(254 - NAME_LABEL_H)  /* 最上段 */
#define MC_ABS_LEVEL_1    yV2P(302 - NAME_LABEL_H)
#define MC_ABS_LEVEL_2    yV2P(350 - NAME_LABEL_H)
#define MC_ABS_LEVEL_3    yV2P(398 - NAME_LABEL_H)  /* 最下段 */


#define MEMCALL_PRIORITY 3


/*
#ifdef ZOE_TRIAL_JP
#define TRIAL
#endif
*/







#ifndef _memcall_c_
#define EXT extern
#else
#define EXT
#endif /* _memcall_c_ */


#undef EXT


void * NewMemCallMenu(int tx_handle);
void   MemcallOpen(void);
void   MemcallClose(void);
int    MemcallStatus(void);
int    MemcallFirstFreq(void);
int    MemcallSelect(GV_PAD * pad);
int    MemcallSetName(int pos, int name_res, int freq);

#endif /* _memcall_h_ */
