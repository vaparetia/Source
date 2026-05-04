#ifndef _bomb_sensor_h_
#define _bomb_sensor_h_

#include "sprite_2d.h"    /* 2D表示モジュールは必須 */


#define BOMB_NO_TEXTURE
#undef BOMB_NO_TEXTURE

/*
 * もやの色
 */
/*
#define BOMB_R  64
#define BOMB_G  0
#define BOMB_B  40
#define BOMB_A  32
*/
#define BOMB_R  (20)
#define BOMB_G  (20)
#define BOMB_B  (0)
#define BOMB_A  (32)

/*
 * 爆弾のもや表示用テクスチャ定義
 */
/*

chi01_msk

ただ、複数使う場合、回転させないとバレバレっす

*/
#define BOMB_TRI  RADAR_TRI //GV_StrCode("effect") /* B SENSOR 表示用 TRI の StrCode      */
#define BOMB_TEX (10984814)

//#define BOMB_TEX  GV_StrCode("chi01_msk") /* B SENSOR 表示用テクスチャの StrCode */

#define BOMB_DISP_W   ((float)16)
#define BOMB_DISP_H   ((float)16)


#define BOMB_OBJ_NUMS    32
#define BOMB_DELTA_SPEED 10 /* もやアニメーション用角速度(4096 = 2π[rad]) */

#define RAD_MAX_ABS_X   SPR_PIX_X(4, (RADAR_WINDOW_W/2) )
#define RAD_MAX_ABS_Y   SPR_PIX_Y(4, (RADAR_WINDOW_H/2) )

#define RAD_BOMB_CX     SPR_PIX_X(4, (RADAR_WINDOW_W / 2))
#define RAD_BOMB_CY     SPR_PIX_Y(4, (RADAR_WINDOW_H / 2))

// #define BOMB_MAX_ALPHA  50
#define BOMB_MAX_ALPHA 255

// by koba4 2001/05/31
enum { 
  SK_MODE_BOMB = 0 ,
  SK_MODE_WATER = 0x1 ,
};

typedef struct RADAR_BOMB {
  struct RADAR_BOMB * prev;
  struct RADAR_BOMB * next;

  /* 爆弾は物を見ないので、視界は必要ない。
     しかし、検知されるエリアが限定される */
  unsigned int  map;             /* マップ                         */

  float         range_center;    /* センサに映る上下レンジ用中心   */
  float         range_zoom_rate; /* 上限レンジ用倍率               */
  FVECTOR     * pos;             /* 爆弾位置 */

  float         width;           /* レーダ検知領域サイズ(幅)       */
  float         height;          /* レーダ検知領域サイズ(高さ)     */
  float         center_x;        /* 検知領域北西の、爆弾位置からの相対位置 */
  float         center_y;        /* 検知領域北西の、爆弾位置からの相対位置 */


  float         min_x, min_y;    /* 最小表示座標 */
  float         max_x, max_y;    /* 最大表示座標 */


  unsigned char r, g, b, a;      /* RGBA 値。もやの色を変える(a は無効) */

  int           rate;           /* 2π[rad]を4096とする角度値
				   (表示輝度変化に使用) */

  int           blight;         /* 表示の明るさ */
 
  SPR_OBJ     * empty;  /* 爆弾中心にあたる点 */
  SPR_OBJ     * cloud[ BOMB_OBJ_NUMS ];
  int           pre_rate[ BOMB_OBJ_NUMS ];
  int           spin_dir[ BOMB_OBJ_NUMS ];
  SPR_POS       bomb_obj_pos[ BOMB_OBJ_NUMS ];
#ifndef BOMB_NO_TEXTURE
  struct {
    SPR_FIX  u, v;
    SPR_FIX  w, h;
  } texinfo[ BOMB_OBJ_NUMS ];
#endif
  char			mode; // by koba4 爆弾モード、水中モード
} RADAR_BOMB;

RADAR_BOMB * BOMB_InitBomb(RADAR_BOMB * bomb);
void         BOMB_Remove(RADAR_BOMB * bomb);
void         BOMB_SetArea(RADAR_BOMB * bomb, FVECTOR * pos,
			  float cx, float cy, float width, float height);
void         BOMB_SetBlight(RADAR_BOMB * bomb, int blight);

void       * NewRadarBomb(short * base_rot, SPR_OBJ * parent, float offset_y );

#endif /* _bomb_sensor_h_ */

