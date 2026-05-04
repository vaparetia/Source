#ifndef _photo_config_h_
#define _photo_config_h_

/* =========================================================================

   管理用定数定義

   ========================================================================= */

#define PHOTO_VOICE_CH   GM_STREAM_CHANNEL_0
#define PHOTO_MENU_CH    MENU_STREAM_CH_0


/* =========================================================================

   画面上での扱いに関するマクロ定義

   ========================================================================= */

/*
 * スプライトと同じ 512 x 384 座標を、画面の物理座用に合わせるマクロ。
 * 値は float 型になる。画面上の座標値やサイズは、このマクロを通して
 * 画面サイズを 512 x 384 として指定すること(PAL 対応のため)。
 */
#define VRx(_x)   ((float)((_x) * DRAW_WIDTH / 512.0F))
#define VRy(_y)   ((float)((_y) * DRAW_HEIGHT / 384.0F))

/*
 * 転送画像の表示サイズ(512 x 384座標上でのサイズ)
 */
#define PHOTO_DISP_WIDTH  VRx(256)
#define PHOTO_DISP_HEIGHT VRy(192)

/*
 * 転送画像の表示位置(512 x 384 座標上でのサイズ)
 */
#define PHOTO_DISP_X    VRx(64)
#define PHOTO_DISP_Y    VRy(64)

#define HOME_X   ((float)(2048 - DRAW_WIDTH / 2))
#define HOME_Y	 ((float)(2048 - DRAW_HEIGHT / 2))

/* =========================================================================

   画像テクスチャに関するマクロ定義

   ========================================================================= */
/*
 * 転送画像のテクセルサイズ(テクセルレベルでのサイズ)
 * 実際のサイズに合わせること。
 */

#define MARGINE_X         8   /* 左右の縁どりの幅 */
#define MARGINE_Y         8   /* 上下の縁どりの幅 */

#define PHOTO_TEX_WIDTH   (DRAW_WIDTH / 2)
#define PHOTO_TEX_HEIGHT  (DRAW_HEIGHT / 2)
/*
#define PHOTO_TEX_U       (PHOTO_TEX_WIDTH + MARGINE_X)
#define PHOTO_TEX_V       (PHOTO_TEX_HEIGHT + MARGINE_Y)
*/
#define PHOTO_TEX_U       MARGINE_X
#define PHOTO_TEX_V       MARGINE_Y

#define PHOTO_TEX_UW      (PHOTO_TEX_WIDTH - MARGINE_X * 2)
#define PHOTO_TEX_VH      (PHOTO_TEX_HEIGHT - MARGINE_Y * 2)


#define PHOTO_PIXELS      (PHOTO_TEX_WIDTH * PHOTO_TEX_HEIGHT)
#define PHOTO_BUFSIZE     (sizeof(unsigned int) * DRAW_WIDTH * DRAW_HEIGHT)
#define PHOTO_TEXSIZE     (sizeof(unsigned short) * PHOTO_PIXELS)

#define PHOTO_MAX_PICT    4   /* 一度に転送の対象とする最大枚数 */
#define PHOTO_ACT_MODE    GV_ACTOR_MANAGER

#endif /* _photo_config_h_ */
