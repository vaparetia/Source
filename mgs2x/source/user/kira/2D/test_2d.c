//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  test_2d.c
  2D 表示系テスト

  $Id: test_2d.c,v 1.1.1.3 2002/11/19 11:43:50 Yoshizawa1 Exp $
*/
/*
  2D 表示系のテスト用モジュール。
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
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

#include        "g_define.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"

#include "sprite_2d.h"

#ifdef PSX2

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(...)
#endif /* _DEBUG_ */

#else

#include <stdarg.h>
#ifdef _DEBUG_
static inline void DBG( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, format );
	vsprintf( buff, format, argptr );
	va_end( argptr );
}
#else
static void inline DBG( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	//vsprintf( buff, f, argptr );
	va_end( argptr );
}
#endif /* _DEBUG_ */

#define SET_SPR_COLOR( _v, _r, _g, _b, _a ) { (_v).r = _r ; (_v).g = _g ; (_v).b = _b ; (_v).a = _a ; }

#endif



typedef struct {
  GV_ACT       actor;

  SPR_OBJ   * test_empty;     /* Empty オブジェクトのテスト     */
  SPR_OBJ   * test_point;     /* Point オブジェクトのテスト     */
  SPR_OBJ   * test_line;      /* Line オブジェクトのテスト      */
  SPR_OBJ   * test_linestrip; /* LineStrip オブジェクトのテスト */
  SPR_OBJ   * test_box;       /* Box オブジェクトのテスト       */
  SPR_OBJ   * test_sprite;    /* Sprite オブジェクトのテスト    */
  SPR_OBJ   * test_zoom;      /* Zoom オブジェクトのテスト      */
  SPR_OBJ   * test_spin;      /* Spin オブジェクトのテスト      */
  SPR_OBJ   * test_poly;      /* Poly オブジェクトのテスト      */

  SPR_OBJ   * dup_empty;      /* test_empty の複製 */

  SPR_POS pos;

  int   handle;
} Work;

static void Act(Work * work)
{

  work->test_spin->spin.spin += M_PI / 30;

  if(GV_PadData[1].status & PAD_U) work->pos.y -= 1.0;
  if(GV_PadData[1].status & PAD_D) work->pos.y += 1.0;
  if(GV_PadData[1].status & PAD_R) work->pos.x += 1.0;
  if(GV_PadData[1].status & PAD_L) work->pos.x -= 1.0;
  if(GV_PadData[1].status & PAD_A)
    SPR_HIDE(work->test_empty);
  else
    SPR_SHOW(work->test_empty);

  if(GV_PadData[1].status & PAD_X)
    SPR_HIDE(work->test_point);
  else
    SPR_SHOW(work->test_point);

  if(GV_PadData[1].status & PAD_Y)
    SPR_HIDE(work->test_line);
  else
    SPR_SHOW(work->test_line);

  SPR_SetPosEmpty(work->test_empty, &work->pos);
}

static void Die(Work * work)
{
  SPR_KillTexture(work->handle);
}

static int GetResources(Work * work, int where)
{
  int code;

  /* 使用するテクスチャを指定する */
  code = GV_StrCode("codec");
  work->handle = SPR_LoadTexture(code);

  /* 操作座標となる Empty オブジェクトを作成 */
  if(NULL == (work->test_empty = SPR_Create_2D_Object(SP_EMPTY, 4, NULL)))
    return -1;

  /* Box オブジェクトを、Empty オブジェクトとおなじ
     最上位オブジェクトとして作成 */
  work->test_box = SPR_Create_2D_Object(SP_BOX, 4, NULL);
  
#if 1
  {
    SPR_POS ls_pos[] = {
      { 180,   84.8 },
      { 180,   76.8 },
      { 331.2, 76.8 },
      { 331.2, 84.8 }
    };
    DBG("LineStgrip<1>\n");
    /* LineStrip オブジェクトを、Empty, Box とおなじ
       最上位オブジェクトとして作成 */
    work->test_linestrip = SPR_Create_2D_Object(SP_LINESTRIP, 0, NULL);

    DBG("LineStgrip<2>\n");
    
    /* LineStrip オブジェクトの頂点数を設定する */
    SPR_SetLineStripVertexNumber(work->test_linestrip, 4);

    DBG("LineStgrip<3>\n");

    /* LineStrip オブジェクトの座標を指定する */
    SPR_SetPosLineStrip(work->test_linestrip, 0, 4, ls_pos);

    DBG("LineStgrip<4>\n");

    /* LineStrip 頂点色設定 */
    SPR_SetColorLineStrip(work->test_linestrip, 0, 0, 255, 0, 128);
    SPR_SetColorLineStrip(work->test_linestrip, 1, 0, 255, 0, 128);
    SPR_SetColorLineStrip(work->test_linestrip, 2, 0, 255, 0, 128);
    SPR_SetColorLineStrip(work->test_linestrip, 3, 0, 255, 0, 128);
  }
#endif

  /*
   * 以下、Emptyオブジェクトを最上位の親として、
   * その下に各種オブジェクトを作成する。
   */
  work->test_point  = SPR_Create_2D_Object(SP_POINT,  2, work->test_empty);
  work->test_line   = SPR_Create_2D_Object(SP_LINE,   2, work->test_point);
  work->test_sprite = SPR_Create_2D_Object(SP_SPRITE, 2, work->test_point);
  work->test_poly   = SPR_Create_2D_Object(SP_POLY,   2, work->test_line);

  DBG("Spin Sprite\n");
  work->test_spin   = SPR_Create_2D_Object(SP_SPIN,   2, work->test_empty);

  if(work->test_point  == NULL ||
     work->test_line   == NULL ||
     work->test_sprite == NULL ||
     work->test_poly   == NULL ||
     work->test_spin   == NULL) return -1;

  /*
   * 作成したオブジェクトの座標値などを設定する。
   */

  /* 点オブジェクト */
#if 0 //BP_PS2
//#ifdef PSX2
  SPR_SetPosPoint(work->test_point, &(SPR_POS){0.0F, 0.0F});
  SPR_SetColorPoint(work->test_point, 255, 0, 0, 255);

  SPR_SetPosLine(work->test_line,
		 &(SPR_POS){0.0F, -16.0F}, &(SPR_POS){-16.0F, 0.0F});
  SPR_SetPosBox(work->test_box,
		&(SPR_RECT){{55.2F, 71.2F}, {456.0F, 195.2F}});
  
  SPR_SetColorBox(work->test_box, 0, 255, 0, 128);
  SPR_SetColorLine(work->test_line, 0, 0, 255, 0, 255);
  SPR_SetColorLine(work->test_line, 1, 0, 255, 0, 255);


  /* スプライト表示位置の指定 */
  SPR_SetPosSprite(work->test_sprite, &(SPR_POS){64.0F, 64.0F});
  SPR_SetSizeSprite(work->test_sprite, 139.2, 59.2);
#else
  {
	  SPR_POS	tmp_pos = {0.0F, 0.0F};
	  SPR_SetPosPoint(work->test_point, &tmp_pos );
  }
  SPR_SetColorPoint(work->test_point, 255, 0, 0, 255);
  {
	  SPR_POS	tmp_pos1 = {0.0F, -16.0F};
	  SPR_POS	tmp_pos2 = {-16.0F, 0.0F};
	  SPR_SetPosLine(work->test_line, &tmp_pos1, &tmp_pos2);
  }
  {
	  SPR_RECT	tmp_rect = {{55.2F, 71.2F}, {456.0F, 195.2F}};
	  SPR_SetPosBox(work->test_box, &tmp_rect);
  }
  SPR_SetColorBox(work->test_box, 0, 255, 0, 128);
  SPR_SetColorLine(work->test_line, 0, 0, 255, 0, 255);
  SPR_SetColorLine(work->test_line, 1, 0, 255, 0, 255);

  {/* スプライト表示位置の指定 */
	  SPR_POS		tmp_pos = {64.0F, 64.0F};
	  SPR_SetPosSprite(work->test_sprite, &tmp_pos );
	  SPR_SetSizeSprite(work->test_sprite, 139.2, 59.2);
  }
#endif





  /* 任意頂点四角形オブジェクト */
  {
    static SPR_POS pos[] = {
      {0, 0},
      {-32, 32},
      {-64, 0},
      {-32, -32}
    };
    SPR_SetPosPoly(work->test_poly, pos);
  }

  /* 回転スプライト */
#if 0 //BP_PS2
//#ifdef PSX2
  SPR_SetPosSpin(work->test_spin, &(SPR_POS){0.0F, 0.0F});
  SPR_SetSizeSpin(work->test_spin, 64.0F, 64.0F);
  SPR_SetCenterSpin(work->test_spin, &(SPR_POS){32.0F, 32.0F});
#else
  {
	  SPR_POS		tmp_pos1 = {0.0F, 0.0F};
	  SPR_POS		tmp_pos2 = {32.0F, 32.0F};
	  SPR_SetPosSpin(work->test_spin, &tmp_pos1 );
	  SPR_SetSizeSpin(work->test_spin, 64.0F, 64.0F);
	  SPR_SetCenterSpin(work->test_spin, &tmp_pos2 );
  }
#endif

  work->test_spin->spin.spin = 0;

  /* テクスチャを貼ることが可能な三つのオブジェクトにテクスチャを指定する */
  SPR_ObjSetTexture(work->test_sprite, GV_StrCode("mater_alp_ovl"), work->handle);
  SPR_ObjSetTexture(work->test_poly, GV_StrCode("mater_bg_alp_ovl"), work->handle);
  SPR_ObjSetTexture(work->test_spin, GV_StrCode("mater_alp_ovl"), work->handle);

  /*
   * アルファブレンディングのテスト
   */
  /* ブレンディング関数の各値を設定 */
  work->test_sprite->head.alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 64);
  work->test_sprite->head.flags |= SPR_FLAG_ALPHA;


  /* 作成したオブジェクトを可視状態にするが、
   * その親である empty オブジェクトがまだ不可視なので、
   * この時点では全てのオブジェクトが表示されない。 */
  SPR_SHOW(work->test_sprite);
  SPR_SHOW(work->test_point);
  SPR_SHOW(work->test_spin);
  SPR_SHOW(work->test_poly);
  SPR_SHOW(work->test_line);

  work->pos.x = 256.0;
  work->pos.y = 192.0;
  SPR_SetPosEmpty(work->test_empty, &work->pos);
  SPR_SHOW(work->test_empty);
  SPR_SHOW(work->test_box);
  SPR_SHOW(work->test_linestrip);

  work->dup_empty = SPR_DuplicateTree(work->test_empty);
  return 0;
}

void * New2D_Test(int name, int where)
{
  Work * work;

  OPERATOR();

  if(NULL == (work = GV_NewActor(GV_ACTOR_MANAGER, sizeof(Work)))) return NULL;

  if(GetResources(work, where))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  GV_SetActor(work, Act, Die);

  return work;
}
