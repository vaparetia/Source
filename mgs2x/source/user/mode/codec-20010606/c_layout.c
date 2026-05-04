/*
  c_layout.c
  無線画面のレイアウトに従い 2D オブジェクトを配置する。

  2000/09/01     Y.Kira

  $Id: c_layout.c,v 1.1.1.3 2002/11/19 11:45:04 Yoshizawa1 Exp $
*/
/*
  無線画面のレイアウトに従い、2D オブジェクトを配置する。
  また、応答があった場合の画面アクション等も制御する。
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "gameheader.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"

#include "cdc_face.h"
#include "codecmem.h"
#include "c_indemo.h"

#define _c_layout_c_
#include "c_layout.h"
#include "sprite_2d.h"
#include "codec_config.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)  printf(args)
#endif /* _DEBUG_*/


typedef enum {
  CS_Close,     /* 閉じている状態                                   */
  CS_Opening,   /* 開く途中                                         */
  CS_MaxOpen,   /* 最大まで開いた状態                               */
  CS_StandBy,   /* 開いた直後、一旦レベルを最大に上げる             */
  CS_Ready,     /* 一旦最大に上げたレベルを、設定されたレベルにする */
  CS_Open,      /* 開いている状態                                   */
  CS_PowerNoise,/* 電源を切る直前の「ボツッ」というノイズ           */
  CS_PowerDown, /* コンデンサの電荷が下がり、レベルが下がっていく   */
  CS_Closing    /* 閉じる途中                                       */
} CodecStep;

#ifdef PAL
#define TIMEBASE  6      /* PAL  */
#else
#define TIMEBASE  5      /* NTSC */
#endif


#define INNER_TIME  40   /* 中枠が開ききるまでに必要な所要時間 */
#define OUTER_TIME  80   /* 外枠が開き切るまでに必要な所要時間 */
#define OUTER_DELAY 40   /* 中枠が動き始めてから外枠が動作を開始するまでの
			     遅延時間 */
#define OUTER_WID   (OUTER_TIME - OUTER_DELAY) /* 外枠が実際に動いている時間 */

#define STANDBY_STEP 10


#define LINE_R  60
#define LINE_G  220
#define LINE_B  200

#define CH          4    /* 描画チャネル */

/* #define NO_FREQ */

typedef struct _Work {
  GV_ACT_EX   actor;

  /* 表示ステータス */
  int        disp_stat;
  int        tx_handle;  /* テクスチャハンドル */
  /*
   * 画面の表示パーツ
   */
  /* メータ類の表示位置基準点 */
  SPR_OBJ   * codec_parts;   /* Empty    : メータ類全体の基準   */

  /* 外枠 */
  SPR_OBJ   * mater_box;     /* Box      : 外側の枠             */

  /* メータ本体 */
  SPR_OBJ   * mater;         /* Sprite   : メータ(非点灯)       */
  SPR_OBJ   * mater_light;   /* Sprite   : メータ(点灯)         */

#ifndef NO_FREQ
  /* 周波数表示 */
  SPR_OBJ   * freq_block;    /* Empty    : 周波数表示ブロック   */
  SPR_OBJ   * freq_fix14;    /* Sprite   : "14" の数字          */
  SPR_OBJ   * freq_dot;      /* Sprite   : 小数点               */
  SPR_OBJ   * freq_num[3];   /* Sprite   : 可変数字             */
#endif /* NO_FREQ */

  /* メータ上部グループ */
  SPR_OBJ   * upper_block;   /* Empty    : メータ上部ブロック   */
  SPR_OBJ   * upper_line;    /* LineStrip: メータ上部の囲い     */
  SPR_OBJ   * upper_ptt;     /* Sprite   : "PTT" の文字         */
  SPR_OBJ   * upper_ptt_light;

  /* メータ下部グループ */
  SPR_OBJ   * lower_block;   /* Empty    : メータ下部ブロック   */
  SPR_OBJ   * lower_line;    /* LineStrip: メータ下部の囲い     */
  SPR_OBJ   * lower_mem;     /* Sprite   : "MEMORY" の文字      */
  SPR_OBJ   * lower_mem_light;
  SPR_OBJ   * lower_tun;     /* Sprite   : "TUNING"の文字と矢印 */
  SPR_OBJ   * lower_tun_light;

  SPR_OBJ   * next_arrow;    /* テキスト送りサイン(下向き矢印)  */

  int         arrow_sw;      /* テキスト送りサイン表示フラグ    */
  int         arrow_cnt;     /* テキスト送りサイン明滅カウンタ  */

  int         light_status;  /* 点灯/非点灯状態ステータス       */
  int         mater_level;   /* 中央メータの明滅                */

  CodecStep   step;

  int         inner_cnt;     /* 上下枠アニメーションのカウンタ */
  int         outer_cnt;     /* 外枠アニメーションのカウンタ   */
  int         standby_cnt;

  int         mater_anim:1;  /* メータの上下を行うかどうかのフラグ */
} Work;


static Work * now_work = NULL;  /* 初期状態では NULL */

static int _disp_freq = 0;

static void setup_priority(Work * work)
{
  int i;

  SPR_SetPriority(work->mater_box,   LAYOUT_PRIORITY);
  SPR_SetPriority(work->mater,       LAYOUT_PRIORITY);
  SPR_SetPriority(work->mater_light, LAYOUT_PRIORITY);

  SPR_SetPriority(work->freq_fix14,  LAYOUT_PRIORITY);
  SPR_SetPriority(work->freq_dot,    LAYOUT_PRIORITY);

  for(i = 0; i < 3; i++) SPR_SetPriority(work->freq_num[i], LAYOUT_PRIORITY);

  SPR_SetPriority(work->upper_line,      LAYOUT_PRIORITY);
  SPR_SetPriority(work->upper_ptt,       LAYOUT_PRIORITY);
  SPR_SetPriority(work->upper_ptt_light, LAYOUT_PRIORITY);


  SPR_SetPriority(work->lower_line,      LAYOUT_PRIORITY);
  SPR_SetPriority(work->lower_mem,       LAYOUT_PRIORITY);
  SPR_SetPriority(work->lower_mem_light, LAYOUT_PRIORITY);
  SPR_SetPriority(work->lower_tun,       LAYOUT_PRIORITY);
  SPR_SetPriority(work->lower_tun_light, LAYOUT_PRIORITY);

  /* テキスト送りサインは、もっとも手前に表示する */
  SPR_SetPriority(work->next_arrow,      7);
}

/*
 * メータ類の点灯、非点灯それぞれの状態は、テクスチャの切替えで表現する。
 *
 * ただし、中央のメータについては、非点灯状態のものに点灯状態のオブジェクトを
 * 重ねて表示する。
 */

/*
 * 無線画面の各パーツに相当する 2D オブジェクトを生成する
 */
static int create_parts(Work * work)
{
  /*
   * オブジェクトの生成
   */

  /* メータ類全体 */
  work->codec_parts = SPR_Create_2D_Object(SP_EMPTY,    CH, NULL);

  /* テキスト送り矢印 */
  work->next_arrow  = SPR_Create_2D_Object(SP_SPRITE,   -1, work->codec_parts);

  /* 外側の枠、中央のメータ */
  work->mater_box   = SPR_Create_2D_Object(SP_BOX,      -1, work->codec_parts);

  /* 周波数表示 */
#ifndef NO_FREQ
  work->freq_block  = SPR_Create_2D_Object(SP_EMPTY,    -1, work->codec_parts);
  work->freq_fix14  = SPR_Create_2D_Object(SP_SPRITE,   -1, work->freq_block);
  work->freq_dot    = SPR_Create_2D_Object(SP_SPRITE,   -1, work->freq_block);

  /* 可変数字用オブジェクトの生成 */
  {
    int i;
    
    for(i = 0; i < 3; i++)
      work->freq_num[i] =
	SPR_Create_2D_Object(SP_SPRITE, -1, work->freq_block);
  }
#endif /* NO_FREQ */

  /* メータ上部グループ */
  work->upper_block = SPR_Create_2D_Object(SP_EMPTY,    -1, work->codec_parts);
  work->upper_line  = SPR_Create_2D_Object(SP_LINESTRIP,-1, work->upper_block);
  work->upper_ptt   = SPR_Create_2D_Object(SP_SPRITE,   -1, work->upper_block);
  work->upper_ptt_light = SPR_Create_2D_Object(SP_SPRITE,   -1, work->upper_ptt);

  /* メータ下部グループ */
  work->lower_block = SPR_Create_2D_Object(SP_EMPTY,    -1, work->codec_parts);
  work->lower_line  = SPR_Create_2D_Object(SP_LINESTRIP,-1, work->lower_block);
  work->lower_mem   = SPR_Create_2D_Object(SP_SPRITE,   -1, work->lower_block);
  work->lower_mem_light   = SPR_Create_2D_Object(SP_SPRITE,   -1, work->lower_mem);
  work->lower_tun   = SPR_Create_2D_Object(SP_SPRITE,   -1, work->lower_block);
  work->lower_tun_light   = SPR_Create_2D_Object(SP_SPRITE,   -1, work->lower_tun);

  /* 中央のメータは、下部グループに従属する */
  work->mater       = SPR_Create_2D_Object(SP_SPRITE,   -1, work->lower_block);
  work->mater_light = SPR_Create_2D_Object(SP_SPRITE,   -1, work->mater);

  setup_priority(work);
  return 0;
}

/*
 * 各パーツの初期座標やサイズの設定を行う
 */
static int setup_pos_and_size(Work * work)
{
  static SPR_POS upper_line[] = {
    /* 上ブロック LineStrip 頂点 */
    {ApartsX1_line, ApartsY2_line},
    {ApartsX1_line, ApartsY1_line},
    {ApartsX2_line, ApartsY1_line},
    {ApartsX2_line, ApartsY2_line}
  }, lower_line[] = {
    /* 下ブロック LineStrip 頂点 */
    {BpartsX1_line, BpartsY2_line},
    {BpartsX1_line, BpartsY1_line},
    {BpartsX2_line, BpartsY1_line},
    {BpartsX2_line, BpartsY2_line}
  };

  /* テキスト送り矢印の座標設定 */
  SPR_SetPosSprite(work->next_arrow,
		   &(SPR_POS){PartsX_NxtArrow, PartsY_NxtArrow});

  work->next_arrow->sprite.col.r = 60;
  work->next_arrow->sprite.col.g = 220;
  work->next_arrow->sprite.col.b = 200;
  work->next_arrow->sprite.col.a = 64;
  

  /* テキスト送り矢印のサイズ設定 */
  SPR_SetSizeSprite(work->next_arrow, PartsW_NxtArrow, PartsH_NxtArrow);

  /*
   * 枠などの Box, LineStrip 設定
   */
  /* LineStrip の頂点数設定 */
  SPR_SetLineStripVertexNumber(work->upper_line, 4);
  SPR_SetLineStripVertexNumber(work->lower_line, 4);

  /* 座標設定 */
  SPR_SetPosLineStrip(work->upper_line, 0, 4, upper_line);
  SPR_SetPosLineStrip(work->lower_line, 0, 4, lower_line);

  {
    int i;

    for(i = 0; i < 4; i++)
      {
	SPR_SetColorLineStrip(work->upper_line, i, LINE_R, LINE_G, LINE_B, 0);
	SPR_SetColorLineStrip(work->lower_line, i, LINE_R, LINE_G, LINE_B, 0);
      }
  }
  /*
   * その他文字等の座標およびサイズ設定
   */
  /* 座標設定 */
  SPR_SetPosSprite(work->upper_ptt,
		   &(SPR_POS){ApartsX_PTT,ApartsY_PTT});  /* "PTT" */

  SPR_SetPosSprite(work->upper_ptt_light, &(SPR_POS){0.0F, 0.0F});  /* "PTT" */

  SPR_SetPosSprite(work->lower_mem,
		   &(SPR_POS){BpartsX_memory, BpartsY_memory}); /* "MEMORY" */

  SPR_SetPosSprite(work->lower_mem_light,&(SPR_POS){0.0F, 0.0F}); /*"MEMORY"*/

  SPR_SetPosSprite(work->lower_tun,
		   &(SPR_POS){BpartsX_tuning, BpartsY_tuning}); /* "TUNING" */
  SPR_SetPosSprite(work->lower_tun_light, &(SPR_POS){0.0F, 0.0F});/*"TUNING"*/


  /* サイズ設定 */
  SPR_SetSizeSprite(work->upper_ptt, ApartsW_PTT, ApartsH_PTT); /* "PTT"    */
  SPR_SetSizeSprite(work->upper_ptt_light, ApartsW_PTT, ApartsH_PTT); /* "PTT"    */

  /* memory */
  SPR_SetSizeSprite(work->lower_mem, BpartsW_memory, BpartsH_memory);
  SPR_SetSizeSprite(work->lower_mem_light, BpartsW_memory, BpartsH_memory);

  /* "TUNING" */
  SPR_SetSizeSprite(work->lower_tun, BpartsW_tuning, BpartsH_tuning);
  SPR_SetSizeSprite(work->lower_tun_light, BpartsW_tuning, BpartsH_tuning);

  /* 中央のメータ(非点灯のみ) */
  SPR_SetPosSprite(work->mater, &(SPR_POS){PartsX_Mater, PartsY_Mater});
  SPR_SetSizeSprite(work->mater, PartsW_Mater, PartsH_Mater);


#ifndef NO_FREQ
  /*
   * 周波数表示グループの座標設定
   */
  /* 固定 "14" */
  SPR_SetPosSprite(work->freq_fix14, &(SPR_POS){NumX_14, NumY_14});

  /* dot */
  SPR_SetPosSprite(work->freq_dot, &(SPR_POS){NumX_dot, NumY_dot});

  /* 数字 [0] (整数部) */
  SPR_SetPosSprite(work->freq_num[0], &(SPR_POS){NumX_num_1_00, NumY_num});

  /* 数字 [1] (小数第1位) */
  SPR_SetPosSprite(work->freq_num[1], &(SPR_POS){NumX_num_0_10, NumY_num});

  /* 数字 [2] (小数第2位) */
  SPR_SetPosSprite(work->freq_num[2], &(SPR_POS){NumX_num_0_01, NumY_num});

  /* 周波数表示基準点座標 */
  SPR_SetPosEmpty(work->freq_block, &(SPR_POS){NumX_base, NumY_base});


  /* 周波数表示オブジェクトのサイズ設定 */
  SPR_SetSizeSprite(work->freq_fix14, NumW_14, NumH_14);
  SPR_SetSizeSprite(work->freq_dot, NumW_dot, NumH_dot);

  {
    int i;

    for(i = 0; i < 3; i++)
      SPR_SetSizeSprite(work->freq_num[i], NumW_num, NumH_num);
  }
#endif /* NO_FREQ */

  /*
   * 無線表示パーツ全体の表示中心設定
   */
  SPR_SetPosEmpty(work->codec_parts, 
		  &(SPR_POS){PartsX_base, PartsY_base});
  return 0;
}

static int select_num_texture(SPR_OBJ * obj, int num, int handle)
{
  int ret;

  /* 数字の変化は UV アニメーションになる(新実装) */
  ret = SPR_ObjSetTexture(obj, LAYOUT_font, handle);
  obj->ex_hd.tex.u += SPR_FIXED(num * NumTexW);
  obj->ex_hd.tex.w = SPR_FIXED(NumTexW - 1);

  return ret;
}

static int setup_nums_textures(Work * work)
{
  int i, n;

  n = _disp_freq;
  for(i = 0; i < 3; i++)
    {
      select_num_texture(work->freq_num[2 - i], n % 10, work->tx_handle);
      n = n / 10;
    }
  return 0;
}


/* 点灯ステータスに従い、各オブジェクトのテクスチャを設定する。
 * また、現在の周波数に合わせ、周波数表示の各数字のテクスチャを設定する。*/
static int setup_textures(Work * work)
{
  /* 数字 */
  setup_nums_textures(work);

  /* "PTT" のテクスチャ指定 */
  SPR_ObjSetTexture(work->upper_ptt, LAYOUT_ptt_dark, work->tx_handle);
  SPR_ObjSetTexture(work->upper_ptt_light, LAYOUT_ptt, work->tx_handle);


  /* "MEMORY" のテクスチャ指定 */
  SPR_ObjSetTexture(work->lower_mem, LAYOUT_mem_dark, work->tx_handle);
  SPR_ObjSetTexture(work->lower_mem_light, LAYOUT_mem, work->tx_handle);


  /* "TUNING" のテクスチャ指定 */
  SPR_ObjSetTexture(work->lower_tun, LAYOUT_tune_dark, work->tx_handle);
  SPR_ObjSetTexture(work->lower_tun_light, LAYOUT_tune, work->tx_handle);

  /*
   * 周波数表示
   */
  /* 固定 "14" */
  SPR_ObjSetTexture(work->freq_fix14, LAYOUT_14, work->tx_handle);

  /* 小数点 */
  SPR_ObjSetTexture(work->freq_dot, LAYOUT_dot, work->tx_handle);

  /* テキスト送り矢印テクスチャ */
  SPR_ObjSetTexture(work->next_arrow, LAYOUT_press_alp_ovl, work->tx_handle);

  return 0;
}

static int setup_all_object(Work * work)
{
  int i;

  /* オブジェクト自体の生成 */
  create_parts(work);

  /* 座標の設定 */
  setup_pos_and_size(work);

  /* テクスチャの設定 */
  work->light_status = 0;  /* 全ての点灯状態を OFF に */
  setup_textures(work);

  SPR_SetColorBox(work->mater_box, LINE_R, LINE_G, LINE_B, 0);

  /* アルファブレンディング設定 */
  {
    unsigned long alpha;

    alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 64);

    work->mater->head.alpha       = alpha;
    work->mater_light->head.alpha = alpha;
    work->mater_box->head.alpha   = alpha;
    work->upper_line->head.alpha  = alpha;
    work->upper_ptt->head.alpha   = alpha;
    work->upper_ptt_light->head.alpha   = alpha;

    work->lower_line->head.alpha  = alpha;
    work->lower_mem->head.alpha   = alpha; 
    work->lower_mem_light->head.alpha   = alpha;
    work->lower_tun->head.alpha   = alpha;
    work->lower_tun_light->head.alpha   = alpha;

    work->freq_fix14->head.alpha  = alpha;
    work->freq_dot->head.alpha    = alpha;

    for(i = 0; i < 3; i++)
      work->freq_num[i]->head.alpha = alpha;

    work->next_arrow->head.alpha   = alpha;
    work->next_arrow->head.flags  |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;

    /* アルファブレンディングを有効に */
    /* メータ類 */
    work->mater->head.flags       |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->mater_light->head.flags |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->mater_box->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;

#ifndef NO_FREQ
    /* 周波数表示 */
    work->freq_fix14->head.flags  |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->freq_dot->head.flags    |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    for(i = 0; i < 3; i++)
      work->freq_num[i]->head.flags |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;;
#endif /* NO_FREQ */

    /* グループ A */
    work->upper_line->head.flags  |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->upper_ptt->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->upper_ptt_light->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;

    /* グループ B */
    work->lower_line->head.flags  |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->lower_mem->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->lower_mem_light->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->lower_tun->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
    work->lower_tun_light->head.flags   |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
  }

  /* 点灯時のオブジェクトは、デフォルトでは非表示にする */
  SPR_HIDE(work->upper_ptt_light);
  SPR_HIDE(work->lower_mem_light);
  SPR_HIDE(work->lower_tun_light);

  /* 常に見えているオブジェクトを、最上位以外全て可視状態にする */
  /* 上パーツ */
  SPR_SHOW(work->upper_line);   /* ライン       */
  SPR_SHOW(work->upper_ptt);    /* "PTT"        */
  SPR_SHOW(work->upper_block);  /* 上パーツ代表 */

  /* 下パーツ */
  SPR_SHOW(work->lower_line);   /* ライン         */
  SPR_SHOW(work->lower_mem);    /* "MEMORY"       */
  SPR_SHOW(work->lower_tun);    /* "TUNING"       */
  SPR_SHOW(work->lower_block);  /* 下パーツ代表   */

  /* テキスト送り矢印は、非表示にしておく。 */
  SPR_HIDE(work->next_arrow);   /* テキスト送り矢印 */

#ifndef NO_FREQ
  /* 周波数表示 */
  SPR_SHOW(work->freq_block);
  SPR_SHOW(work->freq_fix14);
  SPR_SHOW(work->freq_dot);
  for(i = 0; i < 3; i++) SPR_SHOW(work->freq_num[i]);
#endif /* NO_FREQ */

  /* 全体枠 */
  SPR_SHOW(work->mater_box);    /* メータ全体の枠 */

  /* 表示パーツ全体 */
  SPR_HIDE(work->codec_parts);


  return 0;
}


/*
 * 開き途中 / 閉じ途中の、メータ部分のオブジェクト面積、
 * 位置、およびテクスチャ UV 値の設定を行う
 */
static void set_mater_param(Work * work)
{
  int step;

  /* 現在の上下パーツの開き加減を得る */
  {
    float dist;
    float u, d;

    u = work->upper_block->empty.pos.y +
      work->upper_ptt->sprite.pos.y +
      work->upper_ptt->sprite.dh + 1;

    d = work->lower_block->empty.pos.y + work->lower_mem->sprite.pos.y - 6;

    dist = d - u;
    if(dist < 0) dist = -dist;

    step = (int)(dist / (PartsH_Mater / Mater_Lines));
    if(step > Mater_Lines) step = Mater_Lines;
  }
  
  
  /* 上下パーツの開き加減によって、メータオブジェクトの
     面積とテクスチャ UV値を変更する */
  {
    SPR_POS pos;
    float height, lh;
    
    /* 表示位置を求める */
    lh = PartsH_Mater / Mater_Lines;
    height = lh * step;
    if(step >= Mater_Lines) height = PartsH_Mater;
    if(step < 3) /* サイズが 3本以下の場合は表示しない */
      {
	SPR_HIDE(work->mater);
	return;
      }

    pos.x = PartsX_Mater;
    pos.y = PartsY_Mater + PartsH_Mater - height;
    SPR_SetPosSprite(work->mater, &pos);

    /* 表示サイズを設定する */
    SPR_SetSizeSprite(work->mater, PartsW_Mater, height);

    /* 全域にテクスチャを貼る */
    SPR_ObjSetTexture(work->mater, LAYOUT_mater_bg, work->tx_handle);
    
    /* テクスチャの高さを無理矢理調整し、v 値を変更する */
    /* v の変更 */
    work->mater->ex_hd.tex.v +=
      (work->mater->ex_hd.tex.h / Mater_Lines)*(Mater_Lines - step);

    /* 高さの変更 */
    work->mater->ex_hd.tex.h = (work->mater->ex_hd.tex.h / Mater_Lines) * step;
  }

  /* 基本的にこの関数の処理が必要な場合は、点灯状態にしない */
  /* SPR_HIDE(work->mater_light); */
}

/* メータの明滅制御 */
static void mater_light_param(Work *work)
{
  SPR_POS  pos;
  float    y, h;
  int lev;

  lev = work->mater_level;
  if(lev < 0) lev = 0;
  if(lev > Mater_Lines) lev = Mater_Lines;
  if(lev == 0)
    {
      SPR_HIDE(work->mater_light);
      return;
    }
  /* 位置とサイズを、メータのレベルに合わせる */
  y = (Mater_Lines - lev) * (PartsH_Mater / Mater_Lines);

  pos.y = y;
  pos.x = 0;
  SPR_SetPosSprite(work->mater_light, &pos);

  h = PartsH_Mater - y;
  SPR_SetSizeSprite(work->mater_light, PartsW_Mater, h);


  /*
   * テクスチャを設定し、UV値とサイズを調整する
   */
  SPR_ObjSetTexture(work->mater_light, LAYOUT_mater, work->tx_handle);

  /* この時点で、テクスチャ全体の UV およびサイズが収録されているが、
     その値を操作する。 */
  
  y = (Mater_Lines - lev) *
    (work->mater_light->ex_hd.tex.h / Mater_Lines);

  work->mater_light->ex_hd.tex.v += y;
  work->mater_light->ex_hd.tex.h -= y;

  /* メータ点灯状態のオブジェクトを可視状態にする
   * 点灯状態オブジェクトを可視にしても、メータオブジェクトが可視でなければ
   * 表示されない。*/
  SPR_SHOW(work->mater_light);
}

int codecActionStatus(void)
{
  int status;

  if(NULL == now_work) return -1;
  status =  (now_work->step == CS_Open ||
	     now_work->step == CS_Close) ? 0 : 1;
  return status;
}

int codecDispArrow(int sw)
{
  now_work->arrow_sw = sw;
  /* now_work->arrow_cnt = 0; */
}

int codecSetFreq(int freq)
{
  _disp_freq = freq % 1000;
  return _disp_freq;
}


int codecOpenFrame(int fade)
{
  if(NULL == now_work) return -1;

  now_work->inner_cnt = 0;
  now_work->outer_cnt = 0;

  now_work->step = (!fade) ? CS_Opening : CS_Open;

  return 0;
}

int codecCloseFrame(void)
{
  if(NULL == now_work) return -1;

  now_work->inner_cnt   = 0;
  now_work->outer_cnt   = 0;
  now_work->standby_cnt = 0;
  now_work->step = CS_PowerNoise;

  return 0;
}

int codecSetLevel(int level)
{
  if(NULL == now_work) return -1;

  if(level > Mater_Lines) level = Mater_Lines;
  if(level < 0) level = 0;
  now_work->mater_level = level;

  return 0;
}

int codecSetMaterAnimDisable(int flag)
{
  if(NULL == now_work) return -1;

  now_work->mater_anim = flag;
  return 0;
}

int codecSetLight(int status)
{
  if(NULL == now_work) return -1;
  now_work->light_status = status;
  return 0;
}
int codecLayoutDisp(int sw)
{
  if(now_work == NULL) return -1;
  if(sw)
    {
      /* テクスチャの設定 */
      SPR_SHOW(now_work->codec_parts);
      now_work->disp_stat = 1;
    }
  else
    {
      SPR_HIDE(now_work->codec_parts);
      now_work->disp_stat = 0;
    }
  printf("disp flag = %d\n", now_work->disp_stat);
  return 0;
}

static void set_blink(Work * work, SPR_OBJ * obj, int flag)
{
  if(work->light_status & flag) SPR_SHOW(obj);
  else
    SPR_HIDE(obj);
}


static void blink_point(Work * work)
{
  int u, w;
  int W;
  SPR_POS pos;

  /* PTT */
  set_blink(work, work->upper_ptt_light, CODEC_LIGHT_PTT);

  /* MEMORY */
  set_blink(work, work->lower_mem_light, CODEC_LIGHT_MEMORY);

  set_blink(work, work->lower_tun_light, CODEC_LIGHT_TUNING);

  pos.x = 0.0F;
  pos.y = 0.0F;
  u = work->lower_tun_light->ex_hd.tex.u;
  w = work->lower_tun_light->ex_hd.tex.w;
  W = BpartsW_tuning;

  if(!(work->light_status & CODEC_LIGHT_RIGHT))
    {
      w -= SPR_FIXED(9);
      W -= 9.0F;
    }
  if(!(work->light_status & CODEC_LIGHT_LEFT))
    {
      w -= SPR_FIXED(9);
      u += SPR_FIXED(9);
      W -= 9.0F;
      pos.x += 9.0F;
    }
  SPR_SetPosSprite(work->lower_tun_light, &pos);
  SPR_SetSizeSprite(work->lower_tun_light, W, BpartsH_tuning);
  work->lower_tun_light->ex_hd.tex.u = u;
  work->lower_tun_light->ex_hd.tex.w = w;
}

/*
 * 完全に開いた状態にパーツを設定する
 */
static void parts_full_open(Work * work)
{
  /* 外側の Box の位置を開き位置に */
  SPR_SetPosBox(work->mater_box,
		&(SPR_RECT){{BoxX1_Open, BoxY1_Open},
		  {BoxX2_Open, BoxY2_Open}});
  
  /* 上下の表示グループを開き位置に */
  SPR_SetPosEmpty(work->upper_block,
		  &(SPR_POS){GroupA_X_Open, GroupA_Y_Open});
  
  SPR_SetPosEmpty(work->lower_block,
		  &(SPR_POS){GroupB_X_Open, GroupB_Y_Open});
  
  
  /*
   * メータの全体を表示する
   */
  /* テクスチャ全域を設定しなおし */
  SPR_SetPosSprite(work->mater,
		   &(SPR_POS){PartsX_Mater, PartsY_Mater});
  SPR_SetSizeSprite(work->mater, PartsW_Mater, PartsH_Mater);
  SPR_ObjSetTexture(work->mater, LAYOUT_mater_bg, work->tx_handle);

#if 0
    set_mater_param(work);  /* 上下ブロックの開き加減によって、
			     中央メータの表示パラメタを変更する */
#endif
  SPR_SHOW(work->mater);
}

static void make_close_layout(Work * work)
{
  /* 外側の Box の位置を閉じ位置に */
  SPR_SetPosBox(work->mater_box,
		&(SPR_RECT){{BoxX1_Close, BoxY1_Close},
		  {BoxX2_Close, BoxY2_Close}});
  
  /* 上下の表示グループを閉じ位置に */
  SPR_SetPosEmpty(work->upper_block,
		  &(SPR_POS){GroupA_X_Close, GroupA_Y_Close});
  SPR_SetPosEmpty(work->lower_block,
		  &(SPR_POS){GroupB_X_Close, GroupB_Y_Close});
  
  SPR_HIDE(work->mater);  /* メータオブジェクトの表示を OFF にする */
}

static void Act(Work * work)
{
  /* 設定されている周波数に従い、画面表示上の周波数を変更する */
  if(work->disp_stat) setup_textures(work);

  /* 現在の点灯ステータスに従い、基本パーツのテクスチャを設定する */
  blink_point(work);

  /* テキスト送り矢印の点滅 */
  SPR_HIDE(work->next_arrow);
  if((work->arrow_sw != 0) && (work->arrow_cnt < 12))
    SPR_SHOW(work->next_arrow);
  work->arrow_cnt++;
  work->arrow_cnt &= 0x0f;

  switch(work->step)
    {
    case CS_Close:    /* 閉じ状態   */
      make_close_layout(work);
      work->mater_anim = 0;
      break;

    case CS_Opening:  /* 開く途中   */
      /* 上下枠のアクション */
      if(work->inner_cnt < INNER_TIME)
	{
	  SPR_POS pos;
	  SPR_RECT rect;

	  /* 上ブロックの y 座標を求める */
	  pos.x = CalcMovePos(GroupA_X_Close, GroupA_X_Open, INNER_TIME, work->inner_cnt);
	  pos.y = CalcMovePos(GroupA_Y_Close, GroupA_Y_Open, INNER_TIME, work->inner_cnt);
	  SPR_SetPosEmpty(work->upper_block, &pos);

	  /* 下ブロックの y 座標を求める */
	  pos.x = CalcMovePos(GroupB_X_Close, GroupB_X_Open, INNER_TIME, work->inner_cnt);
	  pos.y = CalcMovePos(GroupB_Y_Close, GroupB_Y_Open, INNER_TIME, work->inner_cnt);
	  SPR_SetPosEmpty(work->lower_block, &pos);


	  /* 外枠の頂点を、閉じ位置から縦方向に動かす */
	  rect.begin.x = BoxX1_Close;
	  rect.begin.y = CalcMovePos(BoxY1_Close, BoxY1_Open, INNER_TIME, work->inner_cnt);
	  rect.end.x = BoxX2_Close;
	  rect.end.y = CalcMovePos(BoxY2_Close, BoxY2_Open, INNER_TIME, work->inner_cnt);
	  SPR_SetPosBox(work->mater_box, &rect);

	  work->inner_cnt += TIMEBASE;
	}
      else
	{
	  /* 中枠をそれぞれ規定位置に設定する */
	  SPR_SetPosEmpty(work->upper_block,
			  &(SPR_POS){GroupA_X_Open, GroupA_Y_Open});
	  
	  SPR_SetPosEmpty(work->lower_block,
			  &(SPR_POS){GroupB_X_Open, GroupB_Y_Open});
	}
      set_mater_param(work);  /* 上下ブロックの開き加減によって、
				 中央メータの表示パラメタを変更する */
      SPR_SHOW(work->mater);
	
      /* 外枠のアニメーション */
      if(work->outer_cnt < OUTER_TIME)
	{
	  /* 外枠は中枠より少し遅れて動作を開始する */
	  if(work->outer_cnt >= OUTER_DELAY)  
	    {
	      SPR_RECT rect;
	      float x;
	      int t;

	      t = work->outer_cnt - OUTER_DELAY;

	      rect.begin.y = BoxY1_Open;
	      rect.end.y   = BoxY2_Open;

	      /* 外枠の進行に伴い、X 座標を可変する */
	      rect.begin.x= CalcMovePos(BoxX1_Close, BoxX1_Open, OUTER_WID, t);
	      rect.end.x = CalcMovePos(BoxX2_Close, BoxX2_Open, OUTER_WID, t);
	      SPR_SetPosBox(work->mater_box, &rect);
	    }
	  work->outer_cnt += TIMEBASE;
	}
      else
	{
	  SPR_SetPosBox(work->mater_box,
			&(SPR_RECT){{BoxX1_Open, BoxY1_Open},
			  {BoxX2_Open, BoxY2_Open}});
	}

      /* 中枠、外枠いずれも処理が終っているならば、開き状態に移行する。 */
      if(work->outer_cnt >= OUTER_TIME && 
	 work->inner_cnt >= INNER_TIME)
	{
	  work->standby_cnt = 0;
	  work->step = CS_MaxOpen;
	}
      break;
    case CS_MaxOpen:
      parts_full_open(work);
      SPR_SHOW(work->mater);
      work->step = CS_StandBy;
      break;
    case CS_StandBy:
      {	
	int t;
	/*
	 * 電源投入直後のレベルの上がり方を再現
	 */
	t = work->standby_cnt / STANDBY_STEP;
	work->mater_level = (t * t) / 10;
	
	if(work->mater_level > (Mater_Lines * 2))
	  {
	    /* work->standby_cnt = 0;*/
	    work->step = CS_Ready;
	  }
	if(work->mater_level > Mater_Lines)
	  work->mater_level = Mater_Lines;
	mater_light_param(work);
	work->standby_cnt += TIMEBASE;
      }
      break;
    case CS_Ready:
      {
	work->step = CS_Open;
      }
      break;
    case CS_Open:     /* 開き状態   */
      /* 
       * 開いている状態では、上のほうがわずかに上下する。
       */
      if(!work->mater_anim)
	{
	  static int c = 0;
	  int r;
	  
	  parts_full_open(work);
	  if(!c)
	    {
	      r = ((rand() >> 16) % 5) - 2;
	      
	      work->standby_cnt = Mater_Lines + r - 2;
	      work->mater_level = work->standby_cnt;
	      
	      /* メータレベルに応じて点灯状態のオブジェクトを重ねる */
	      c = 4;
	    }
	  c--;
	  mater_light_param(work);
	}
      break;


    case CS_PowerNoise:  /* 電源を切った直後の「ボツッ」というノイズ */
      /*
       * 急激に上がり…
       */
      /* 現在の値から最上位にまで持ち上げる */
      work->mater_level = work->standby_cnt;
      work->standby_cnt++;
      if(work->mater_level > Mater_Lines)
	{
	  work->mater_level = Mater_Lines;
	  work->standby_cnt = 0;
	  work->step = CS_PowerDown;
	}
      mater_light_param(work);
      break;
    case CS_PowerDown:   /* コンデンサの電荷が下がり、レベルが下がっていく */
      {
	int t;
	/*
	 * 徐々にさがっていく。
	 */
	t = (150 - work->standby_cnt) / STANDBY_STEP;
	work->mater_level = (t * t) / 10;

	mater_light_param(work);
	if(work->mater_level <= 0)
	  {
	    work->mater_level = 0;
	    work->step = CS_Closing;
	  }
	work->standby_cnt += TIMEBASE;
      }
      break;
    case CS_Closing:  /* 閉じる途中 */
      /*
       * 完全に電荷が失われたら、枠を閉じる
       */
      /* 閉じる場合は、外枠が最初に閉じ始めるため、
       * カウンタの上限値およびディレイ値が逆になる。*/
      if(work->outer_cnt < INNER_TIME)
	{
	  SPR_RECT rect;
	  float x;

	  rect.begin.x = CalcMovePos(BoxX1_Open, BoxX1_Close, INNER_TIME, work->outer_cnt);
	  rect.end.x = CalcMovePos(BoxX2_Open, BoxX2_Close, INNER_TIME, work->outer_cnt);

	  rect.begin.y = BoxY1_Open;
	  rect.end.y = BoxY2_Open;

	  SPR_SetPosBox(work->mater_box, &rect);

	  work->outer_cnt += TIMEBASE;
	}
      /*
	else
	{
	SPR_SetPosBox(work->mater_box,
	&(SPR_RECT){{BoxX1_Close, BoxY1_Open},
	{BoxX2_Close, BoxY2_Open}});
	}
      */
      
      if(work->inner_cnt < OUTER_TIME)
	{
	  if(work->inner_cnt >= OUTER_DELAY)
	    {
	      int t;
	      float y;
	      SPR_POS pos;
	      SPR_RECT rect;

	      t = work->inner_cnt - OUTER_DELAY;

	      /* 上下枠を動かす */
	      pos.x = CalcMovePos(GroupA_X_Open, GroupA_X_Close, OUTER_WID, t);
	      pos.y = CalcMovePos(GroupA_Y_Open, GroupA_Y_Close, OUTER_WID, t);
	      SPR_SetPosEmpty(work->upper_block, &pos);

	      pos.x = CalcMovePos(GroupB_X_Open, GroupB_X_Close, OUTER_WID, t);
	      pos.y = CalcMovePos(GroupB_Y_Open, GroupB_Y_Close, OUTER_WID, t);
	      SPR_SetPosEmpty(work->lower_block, &pos);

	      /* 外枠も合わせて縮める */
	      rect.begin.x = BoxX1_Close;
	      rect.end.x   = BoxX2_Close;
	      rect.begin.y =CalcMovePos(BoxY1_Open, BoxY1_Close, OUTER_WID, t);
	      rect.end.y  = CalcMovePos(BoxY2_Open, BoxY2_Close, OUTER_WID, t);

	      SPR_SetPosBox(work->mater_box, & rect);
	    }
	  work->inner_cnt += TIMEBASE;
	}
      else
	{
	  /* 中枠をそれぞれ規定位置に設定する */
	  SPR_SetPosEmpty(work->upper_block,
			  &(SPR_POS){GroupA_X_Close, GroupA_Y_Close});
	  
	  SPR_SetPosEmpty(work->lower_block,
			  &(SPR_POS){GroupB_X_Close, GroupB_Y_Close});

	  SPR_SetPosBox(work->mater_box,
			&(SPR_RECT){{BoxX1_Close, BoxY1_Close},
			  {BoxX2_Close, BoxY2_Close}});
	}

      set_mater_param(work);  /* 上下ブロックの開き加減によって、
				 中央メータの表示パラメタを変更する */
      SPR_SHOW(work->mater);  /* メータオブジェクトの表示を ON にする */
      
      /* 中枠、外枠いずれも処理が終っているならば、閉じ状態に移行する。 */
      if(work->outer_cnt >= INNER_TIME && work->inner_cnt >= OUTER_TIME)
	work->step = CS_Close;
      
      break;
    }
}

static void Die(Work * work)
{
  if(work == now_work) now_work = NULL;
  /* オブジェクトを破壊すれば、その下位のオブジェクトは全て破壊される。
     よって、ここでは際上位オブジェクトを破壊する。*/
  DBG("c_layout:[0]\n");
  SPR_KillTexture(work->tx_handle);
  SPR_Destroy_2D_Object(work->codec_parts);
  DBG("c_layout:[1]\n");
}

static int GetResources(Work * work)
{
  work->disp_stat = 0;
  work->tx_handle = SPR_LoadTexture(LAYOUT_TRI);
  setup_all_object(work);
  work->step = CS_Close;
  make_close_layout(work);  /* 閉じ状態の座標を設定しておく */
  work->mater_level = 0;
  work->arrow_sw = 0;
  work->arrow_cnt = 0;

  /* メモリーコールモードのレイアウト準備 */
  GV_SetActorChild(work, NewMemCallMenu(work->tx_handle));

  return 0;
}

void * NewCodecLayout(void)
{
  Work * work;

  OPERATOR();
  if(now_work != NULL) return NULL;
  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  now_work = work;
  return work;
}

/*
 * シナリオインタフェース。通常用いることはない。
 */
void * NewCodecLayoutSCN(int name, int where)
{
  return NewCodecLayout();
}
