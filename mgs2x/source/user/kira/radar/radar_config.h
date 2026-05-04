#ifndef _radar_config_h_
#define _radar_config_h_

#define RADAR_D_MODE	0x00000001		// Alert などとメータ
#define RADAR_D_HZX		0x00000002		// 壁表示
#define RADAR_D_PLAYER	0x00000004		// プレイヤー光点／視界表示
#define RADAR_D_ENEMY	0x00000008		// 敵兵表示
#define RADAR_D_MINE	0x00000010		// 地雷表示

#define RADAR_D_NOISE	0x00004000		// ノイズ表示中
#define RADAR_D_SUBWIN	0x00008000		// 子画面に乗っ取られている
#define RADAR_D_OUTDIR	0x00010000		// (拡張) ハリヤー方向表示
#define RADAR_D_C4AREA	0x00020000		// (拡張) C4エリア表示
#define RADAR_D_AIRAREA	0x00040000		// (拡張) 空気エリア表示

#define RADAR_FRAME_DISPLAY \
	( RADAR_D_HZX | RADAR_D_PLAYER | RADAR_D_MINE | RADAR_D_ENEMY \
	  | RADAR_D_OUTDIR | RADAR_D_C4AREA | RADAR_D_AIRAREA | RADAR_D_NOISE )


#define RADAR_STATUS_NODE	0x00000001

/* =========================================================================
 * レーダの各種定数定義
 * ========================================================================= */

#define CLOSE_SPEED	(SC_RATE_H*3.0F)
#define RADAR_VIEW_W	(20000.0F)	/* 見える範囲(1/1000m) */

/* 横方向 */
#define RADAR_ZOOM_RATE_W  (RADAR_WINDOW_W / RADAR_VIEW_W)

/* 縦方向 */
/*
  #define RADAR_ZOOM_RATE_H  (( RADAR_ZOOM_RATE_W * DRAW_HEIGHT / DRAW_WIDTH ) \
  / ( ASPECT_Y() ) )
*/
#if 0
#ifndef PAL
//#define RADAR_ZOOM_RATE_H  (( RADAR_ZOOM_RATE_W * DRAW_HEIGHT / DRAW_WIDTH ) \  * ( ASPECT_Y() ) )
#else
#define RADAR_ZOOM_RATE_H  (0.006000f)
#endif
// #define RADAR_ZOOM_RATE_H  ( RADAR_ZOOM_RATE_W * DRAW_HEIGHT / DRAW_WIDTH )
#endif

#if 1
/* change by K.Takabe 2002/05/21 */
#if 0 //BP_PS2 def PSX2
//#define RADAR_ZOOM_RATE_H  (( RADAR_ZOOM_RATE_W * DRAW_HEIGHT / DRAW_WIDTH ) * ( ASPECT_Y() ) )
//#define RADAR_ZOOM_RATE_H  ( RADAR_ZOOM_RATE_W * RADAR_WINDOW_H / ( RADAR_WINDOW_W * ASPECT_Y() ) )
#define RADAR_ZOOM_RATE_H  (RADAR_WINDOW_H / ( RADAR_VIEW_W / ASPECT_Y() ) )
#else
/*xbox*/
#define RADAR_ZOOM_RATE_H  (RADAR_WINDOW_H / ( RADAR_VIEW_W / ASPECT_Y() ) )
//#define RADAR_ZOOM_RATE_H  ( RADAR_ZOOM_RATE_W * RADAR_WINDOW_H / ( RADAR_WINDOW_W * ASPECT_Y() ) )
#endif
#endif
/*レーダー中心のスクリーン座標*/
#define RADAR_WINDOW_CX (RADAR_WINDOW_X0 + (RADAR_WINDOW_W/2))
#define RADAR_WINDOW_CY (RADAR_WINDOW_Y0 + (RADAR_WINDOW_H/2))

#define RADAR_FRAME (0)

/*キャラの写る範囲*/

#define UPPER_RANGE			( 500.0F)
#define DOWN_RANGE			( -1250.0F)

#define RADAR_ZOOM_RATE_Y	( 2.0F / ( UPPER_RANGE - DOWN_RANGE ) )

/*実験 レーダーキャラ上限*/
#define R_CTRL_MAX	(64)

/*視力と関係なく表示視界を設定*/
#define RAD_RANGE_FIX (6000.0F)

#define BASE_ALPHA  (0x80 / 8 * 6)



#define SNK_R	(71)
#define SNK_G	(68)
#define SNK_B	(64)

#define SRC_R	(58)
#define SRC_G	(90)
#define SRC_B	(4)

#define ALT_R	(140)
#define ALT_G	(5)
#define ALT_B	(10)

#define ALT_LEVEL_R	(76)
#define ALT_LEVEL_G	(2)
#define ALT_LEVEL_B	(0)

#define ESC_R	(155)
#define ESC_G	(46)
#define ESC_B	(0)


#define ESC_LEVEL_R	(135)
#define ESC_LEVEL_G	(40)
#define ESC_LEVEL_B	(0)

#define JAM_R	(9)
#define JAM_G	(103)
#define JAM_B	(44)

#define JAM_LEVEL_R	(10)
#define JAM_LEVEL_G	(77)
#define JAM_LEVEL_B	(58)

#define RAD_DEFAULT				(0x0000) 
#define RAD_SCN_INVISIBLE	(0x0001) /*シナリオからの非表示*/

#define RAD_STATE_CHAFF (ALERT_MODE_SEARCH + 1)

#define	ALLOW_COL	(0x80)

#define MAX_ROT_TABLE	64

#define RAD_BUF_MAX (81920 / sizeof( u_long128 ))


#define NOISE_1 (0xce0182)
#define NOISE_2 (0xce0183)
#define NOISE_3 (0xce0184)


/********
言葉の定義
レーダー 地形やキャラを表示するもの
アラート表示
********/

/*
 * レーダーの使用禁止チェック
 */

/*ゲーム状態による使用不可*/
#define RAD_STATE_NOUSE (STATE_CHAFF)

/*プレイヤ状態による使用不可*/
//#define RAD_PLAYER_NOUSE (PLAYER_INTRUDE|PLAYER_LOCKER|PLAYER_MENU_OPEN)
#define RAD_PLAYER_NOUSE (PLAYER_MENU_OPEN)

/*
 * パーツの表示フラグ(ModeChange() で使用)
 */
#define RADAR_SHOW_USE   0x0001
#define RADAR_SHOW_EMPTY 0x0002
#define RADAR_SHOW_MODE  0x0004
#define RADAR_SHOW_NOISE 0x0008

#define UWAGAKI (SCE_GS_SET_ALPHA(2,2,0,0,0))
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))
#define KASAN2 (SCE_GS_SET_ALPHA(0,2,2,1,64))
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))


#define TOP_CYCLE1 127
#define TOP_CYCLE2 15

#define BOTTOM_CYCLE1 63
#define BOTTOM_CYCLE2 15

#ifdef PAL
#define CAUTION_BASE  3000.0F
#else
#define CAUTION_BASE  3600.0F
#endif  /* PAL */

#define MENU_MODE_OFF	(MENU_RADAR_OFF|MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN)


/* =========================================================================
 *  列挙子による定数定義
 * ========================================================================= */

/*でばぐ用*/
#ifdef DEBUG_MODE
enum {
  RAD_DEBUG_ARROW,
  RAD_DEBUG_SIGHT,
  RAD_DEBUG_MAX
};
#endif

/*
 * マイクロプログラムエントリの選択
 */
enum {
  SEG_DRAW_START = 0,
  SEG_DRAW_NEXT,
  SEG_DYNAMIC_DRAW_START,
  SEG_DYNAMIC_DRAW_NEXT,
  CHARA_SIGHT_DRAW,
};

enum {
  CHARA_COLOR_PLAYER = 0,
  CHARA_COLOR_ENEMY_BLUE,
  CHARA_COLOR_ENEMY_RED,
  CHARA_COLOR_ENEMY_YELOW,
  MAX_CHARA_COLOR
};

enum {
  RAD_MSG_SCN_INVIS, /* off */
  RAD_MSG_SCN_VIS    /*on*/
};





#endif /* _radar_config_h_ */
