//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  layout_conv.c
  オブジェクト補間関数群

  2001/01/12  Y.Kira
  $Id: layout_conv.c,v 1.1.1.3 2002/11/19 11:43:51 Yoshizawa1 Exp $
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

#include "gameheader.h"

#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"  /* 2D モジュールを使用 */


#define _layout_conv_c_
#include "layout_config.h"
#include "layout_data.h"
#include "layout_conv.h"

#define F_PI  ((float)M_PI)

#ifdef DEBUG_MODE
#define _DEBUG_
#endif

#undef _DEBUG_

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

#endif

typedef union RGBA {
  unsigned int l_rgba;
  struct {
    unsigned char r, g, b, a;
  } s_rgba;
} RGBA;

/*
 * 表示属性の設定
 */
static void disp_stat(l2dSprite * spr, l2dStatus * stat)
{
  /* アルファレジスタの更新 */
  if(stat->alpha_flg)  
    {
      spr->obj->head.alpha = stat->alpha;
      spr->obj->head.flags |= SPR_FLAG_ALPHA;
    }
  else
    spr->obj->head.flags &= ~SPR_FLAG_ALPHA;

  /* プライオリティ補正値の更新 */
  spr->pri_adjust = stat->pri_adj;
  SPR_SetPriority(spr->obj, spr->pri_adjust + spr->pri_base);

  /* 水平,垂直反転の設定 */
  {
    int flag = 0;

    if(stat->h_rev) flag |= SPR_FLAG_H_REV;
    if(stat->v_rev) flag |= SPR_FLAG_V_REV;
    spr->obj->head.flags |= flag;
  }

  /* 表示/非表示の切替え */
  if(stat->disp)
    SPR_SHOW(spr->obj);
  else
    SPR_HIDE(spr->obj);
}

/*
 * RGBA 値の補間
 */
static RGBA * rgba_conv(RGBA * ret, l2dVertex * p1, l2dVertex * p2, float rate)
{
  /* いずれの値も設定されていない場合は、(128, 128, 128, 128) が使用される */
  if(!p1->rgba && !p2->rgba)
    {
      ret->l_rgba = 0x80808080;
      return ret;
    }

  /* 新しい値が設定されていない場合 */
  if(!p2->rgba)
    {
      ret->s_rgba.r = p1->r;
      ret->s_rgba.g = p1->g;
      ret->s_rgba.b = p1->b;
      ret->s_rgba.a = p1->a;
      return ret;
    }
  /* 古い値が設定されていない場合 */
  if(!p1->rgba || (rate >= 1.0F))
    {
      p1->r = ret->s_rgba.r = p2->r;
      p1->g = ret->s_rgba.g = p2->g;
      p1->b = ret->s_rgba.b = p2->b;
      p1->a = ret->s_rgba.a = p2->a;
      p1->rgba = 1;
      return ret;
    }

  /* 双方の値が設定されている場合は、レートによる補間を行う */
  ret->s_rgba.r = p1->r + (int)((float)(p2->r - p1->r) * rate + 0.5F);
  ret->s_rgba.g = p1->g + (int)((float)(p2->g - p1->g) * rate + 0.5F);
  ret->s_rgba.b = p1->b + (int)((float)(p2->b - p1->b) * rate + 0.5F);
  ret->s_rgba.a = p1->a + (int)((float)(p2->a - p1->a) * rate + 0.5F);
  return ret;
}

/*
 * 座標値の補間
 */
// returns whether it moves or not
static void pos_conv(SPR_POS * pos,
			  l2dVertex * p1, l2dVertex * p2, float rate)
{
  /* 双方の値が設定されていない場合は、(0,0) とする */
  if(!p1->vert && !p2->vert)
    {
      pos->x = pos->y = 0.0F;
    }
  /* 変化後の座標値が設定されていない場合は、以前の値を維持する */
  if(!p2->vert)
    {
      pos->x = p1->x;
      pos->y = p1->y;
      return;
    }

  /* 変化前の座標値が設定されていない場合は、新しい値をそのまま使用する */
  /* レートが 1.0 以上の場合は、p2 の値をそのまま設定する */
  if((rate >= 1.0F) || !(p1->vert))
    {
#if 1
      p1->x = pos->x = p2->x;
      p1->y = pos->y = p2->y;
      p1->vert = 1;   /* 新たな値を設定されたものとする。 */
#else
      pos->x = p2->x;
      pos->y = p2->y;
#endif
      return;
    }

  /* 双方に値が設定されており、変化レートが 1.0F 未満である場合は、
     両者の中間を直線補間した値を使用する */
  pos->x = p1->x + (p2->x - p1->x) * rate;
  pos->y = p1->y + (p2->y - p1->y) * rate;
}

/* 下位オブジェクト座標値係数の補間 */
int magni_conv(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;
  l2dStatus * stat2;
  float mag;

  stat1 = spr->stat + form1;   /* 旧状態 */
  stat2 = spr->stat + form2;   /* 新状態 */

  mag = ((stat2->magni - stat1->magni) * rate + stat1->magni);
  spr->magni = mag;
  SPR_MAG(spr->obj, mag);
  //DBG("magnification: (%8.3f - %8.3f) * %8.3f + %8.3f = %8.3f\n",
  //    stat2->magni, stat1->magni, rate, stat1->magni , mag);
  if (stat1->magni != stat2->magni && rate < 1.0f)
  {
     return 1;
  }
  return 0;
}

int lconvEmpty(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos;

  stat1 = spr->stat + form1;   /* 旧状態 */
  stat2 = spr->stat + form2;   /* 新状態 */

  /*
   * Empty オブジェクトでは、座標値と表示属性のみを扱う。
   * RGBA値、テクスチャ関連パラメータは操作対象としない。
   */
  /* 頂点補間 */
  pos_conv(&pos, spr->vertex, stat2->vertex, rate);

  /* 補間結果の座標値を 2D オブジェクトの実体に設定 */
  SPR_SetPosEmpty(spr->obj, &pos);

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);

  return 0;
}

/*
 * 点オブジェクトの補間
 */
int lconvPoint(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos;
  RGBA rgba;
  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  /*
   * 点オブジェクトでは、座標値、表示属性、RGBA 値の補間を行う
   */
  /* 座標値の補間 */
  pos_conv(&pos, spr->vertex, stat2->vertex, rate);
  SPR_SetPosPoint(spr->obj, &pos);

  /* RGBA値の補間 */
  rgba_conv(&rgba, spr->vertex, stat2->vertex, rate);
  SPR_SetColorPoint(spr->obj,
		    rgba.s_rgba.r, rgba.s_rgba.g,
		    rgba.s_rgba.b, rgba.s_rgba.a);

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性の設定 */
  disp_stat(spr, stat2);
  
  return 0;
}

int lconvLine(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos[2];
  RGBA rgba;
  int i;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  /* 座標値、RGBA 値の補間 */
  for(i = 0; i < POS_VTX_LINE; i++)
    {
      pos_conv(&pos[i], spr->vertex + i, stat2->vertex + i, rate);
      rgba_conv(&rgba, spr->vertex + i, stat2->vertex + i, rate);
      /* RGBA 値はこの段階で設定してしまう */
      SPR_SetColorLine(spr->obj, i, 
		       rgba.s_rgba.r, rgba.s_rgba.g,
		       rgba.s_rgba.b, rgba.s_rgba.a);
    }
  SPR_SetPosLine(spr->obj, &pos[0], &pos[1]);  /* 座標値の設定 */

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);

  return 0;
}

int lconvLineStrip(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos;
  RGBA rgba;
  int i;
  
  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;
  /*
   * 各頂点の RGBA 値、座標値の補間
   */
  for(i = 0; i < spr->vertex_num; i++)
    {
      pos_conv(&pos, spr->vertex + i, stat2->vertex + i, rate);    /* 座標   */
      rgba_conv(&rgba, spr->vertex + i, stat2->vertex + i, rate);  /* RGBA値 */
      SPR_SetColorLineStrip(spr->obj, i,
		       rgba.s_rgba.r, rgba.s_rgba.g,
		       rgba.s_rgba.b, rgba.s_rgba.a);
      SPR_SetPosLineStrip(spr->obj, i, 1, &pos);
    }
  
  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);
  return 0;
}

int lconvBox(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_RECT rect;
  RGBA rgba;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  /* BOX のRGBA 値は一つだけ */
  rgba_conv(&rgba, spr->vertex, stat2->vertex, rate);
  pos_conv(&rect.begin, spr->vertex, stat2->vertex, rate);
  pos_conv(&rect.end, spr->vertex + 1, stat2->vertex + 1, rate);

  SPR_SetPosBox(spr->obj, &rect);  /* 座標値の設定 */
  SPR_SetColorBox(spr->obj, 
		  rgba.s_rgba.r, rgba.s_rgba.g,
		  rgba.s_rgba.b, rgba.s_rgba.a);
  
  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);
  return 0;
}

/*
 * テクスチャ補間(Sprite, Zoom 共通)
 */
static int tex_conv(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus *stat1, *stat2;
  SPR_OBJ * obj;
  float u, v;
  float uw, vh;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  /* 新しい状態でテクスチャが定義されていれば、新たなテクスチャを設定する */
  if(stat2->tex_code >= 0)
    if((stat2->tex_code != spr->tex_code) ||
       (stat2->tex_handle != spr->tex_handle))
      {
	int ret;
	DBG("tri_handle: %d  code: 0x%06x\n",
	    stat2->tex_handle, stat2->tex_code);
	ret = SPR_ObjSetTexture(spr->obj, stat2->tex_code, stat2->tex_handle);
	DBG("ret = %d\n", ret);
	spr->tex_code = stat2->tex_code;
	spr->tex_handle = stat2->tex_handle;
	/* 本来の UV 値を設定する */
	spr->u = spr->obj->ex_hd.tex.pu;
	spr->v = spr->obj->ex_hd.tex.pv;
	spr->w = spr->obj->ex_hd.tex.pw;
	spr->h = spr->obj->ex_hd.tex.ph;
      }
  
  /* テクスチャが設定されていない場合は、以下の処理は無駄なので行わない */
  if(spr->tex_code < 0) return 0;

  /* UV 値の補間 */ 
  if(stat1->tex_code < 0)
    {
      /* 直前のステータスでテクスチャが指定されていない場合、
	 新たなテクスチャパラメータをそのまま指定する   */
      u = stat2->pos_u;
      v = stat2->pos_v;
      uw = stat2->size_u;
      vh = stat2->size_v;
    }
  else
    {
      /* 直前のステータスとの間で、テクスチャ関連パラメタの補間を行う */
      u = stat1->pos_u + (stat2->pos_u - stat1->pos_u) * rate;
      v = stat1->pos_v + (stat2->pos_v - stat1->pos_v) * rate;
      uw = stat1->size_u + (stat2->size_u - stat1->size_u) * rate;
      vh = stat1->size_v + (stat2->size_v - stat1->size_v) * rate;
    }
  obj = spr->obj;

  /* UV 値計算 */
  /* 表示に用いる UV 値は、本来の UV 値及びサイズより 0.5texel 内側を指定 */
#if 0 //BP_PS2 def PSX2
  obj->ex_hd.tex.u = (int)((float)spr->u + (float)spr->w * u) + 8;
  obj->ex_hd.tex.v = (int)((float)spr->v + (float)spr->h * v) + 8;
  obj->ex_hd.tex.w = (int)((float)spr->w * uw) - 16 ;
  obj->ex_hd.tex.h = (int)((float)spr->h * vh) - 16 ;
#else
  /* ＸＢＯＸは浮動小数点でデータを保持するため */
  if (!(obj->ex_hd.tex.dgtex->BP_flag & DG_TEXFLAG_NO_UV_PULLIN))
  {
     obj->ex_hd.tex.u = ((float)spr->u + (float)spr->w * u) + 0.5f ;
     obj->ex_hd.tex.v = ((float)spr->v + (float)spr->h * v) + 0.5f ;
     obj->ex_hd.tex.w = ((float)spr->w * uw) - 1.0f ;
     obj->ex_hd.tex.h = ((float)spr->h * vh) - 1.0f ;
  }
  else
  {
     obj->ex_hd.tex.u = ((float)spr->u + (float)spr->w * u);
     obj->ex_hd.tex.v = ((float)spr->v + (float)spr->h * v);
     obj->ex_hd.tex.w = ((float)spr->w * uw);
     obj->ex_hd.tex.h = ((float)spr->h * vh);
  }
#endif

  return 0;
}

int lconvSprite(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos;
  float width, height; /* 表示サイズ計算用テンポラリ */
  RGBA rgba;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;
#if 1
  pos_conv(&pos, spr->vertex, stat2->vertex, rate);
  rgba_conv(&rgba, spr->vertex, stat2->vertex, rate);
#else
  pos_conv(&pos, stat1->vertex, stat2->vertex, rate);
  rgba_conv(&rgba, stat1->vertex, stat2->vertex, rate);
#endif
  SPR_SetPosSprite(spr->obj, &pos);
  spr->obj->sprite.col.r = rgba.s_rgba.r;
  spr->obj->sprite.col.g = rgba.s_rgba.g;
  spr->obj->sprite.col.b = rgba.s_rgba.b;
  spr->obj->sprite.col.a = rgba.s_rgba.a;

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);
  tex_conv(spr, form1, form2, rate);

  /* 表示サイズの設定、補間 */
#if 1
  width  = spr->width  + (stat2->size_w - spr->width ) * rate;
  height = spr->height + (stat2->size_h - spr->height) * rate;
#else
  width  = stat1->size_w + (stat2->size_w - stat1->size_w) * rate;
  height = stat1->size_h + (stat2->size_h - stat1->size_h) * rate;
#endif

  SPR_SetSizeSprite(spr->obj, width, height);

  if(rate >= 1.0F)
    {
      spr->width  = stat2->size_w;
      spr->height = stat2->size_h;
    }

  return 0;
}

int lconvZoom(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_RECT rect;
  RGBA rgba;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;
  pos_conv(&rect.begin, spr->vertex, stat2->vertex, rate);
  pos_conv(&rect.end,   spr->vertex + 1, stat2->vertex + 1, rate);
  SPR_SetPosZoom(spr->obj, &rect); /* 座標設定 */

  /* RGBA 値設定 */
  rgba_conv(&rgba, spr->vertex, stat2->vertex, rate);
  spr->obj->zoom.col.r = rgba.s_rgba.r;
  spr->obj->zoom.col.g = rgba.s_rgba.g;
  spr->obj->zoom.col.b = rgba.s_rgba.b;
  spr->obj->zoom.col.a = rgba.s_rgba.a;

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性の設定 */
  disp_stat(spr, stat2);
  tex_conv(spr, form1, form2, rate);

  return 0;
}

int lconvSpin(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos;
  float width, height; /* 表示サイズ計算用テンポラリ */
  RGBA rgba;
  float ang_A, ang_B, ang_C;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  pos_conv(&pos, spr->vertex, stat2->vertex, rate);
  rgba_conv(&rgba, spr->vertex, stat2->vertex, rate);

  SPR_SetPosSpin(spr->obj, &pos);
  spr->obj->spin.col.r = rgba.s_rgba.r;
  spr->obj->spin.col.g = rgba.s_rgba.g;
  spr->obj->spin.col.b = rgba.s_rgba.b;
  spr->obj->spin.col.a = rgba.s_rgba.a;

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);
  tex_conv(spr, form1, form2, rate);

  /* 表示サイズの設定、補間 */
  width  = spr->width  + (stat2->size_w - spr->width ) * rate;
  height = spr->height + (stat2->size_h - spr->height) * rate;

  SPR_SetSizeSpin(spr->obj, width, height);

  if(rate >= 1.0F)
    {
      spr->width  = stat2->size_w;
      spr->height = stat2->size_h;
    }

  /* 回転角の補間 */

  ang_A = stat1->angle;
  ang_B = stat2->angle;
  if(!spr->spin_dir)
    {
      /* 回転方向右(デフォルト) */
      if((ang_C = ang_B - ang_A) < 0.0F) ang_C += 4096.0F;
      ang_A += ang_C * rate;
    }
  else
    {
      /* 回転方向左             */
      if((ang_C = ang_A - ang_B) < 0.0F) ang_C += 4096.0F;
      ang_A -= ang_C * rate;
    }

  spr->obj->spin.spin = ang_A * F_PI * 2 / 4096;

  /* 回転中心の補間 */
  {
    pos.x = (stat2->center_x - stat1->center_x) * rate + stat1->center_x;
    pos.y = (stat2->center_y - stat1->center_y) * rate + stat1->center_y;
    SPR_SetCenterSpin(spr->obj, &pos);
  }
  
  return 0;
}

/*
 * 任意 4頂点オブジェクトの補間
 */
int lconvPoly(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos[4];
  RGBA rgba;
  int i;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  /* 座標値、RGBA 値の補間 */
  for(i = 0; i < POS_VTX_POLY; i++)
    {
      pos_conv(&pos[i], spr->vertex + i, stat2->vertex + i, rate);
      rgba_conv(&rgba, spr->vertex + i, stat2->vertex + i, rate);
      /* RGBA 値はこの段階で設定してしまう */
      SPR_SetColorPoly(spr->obj, i,
		       rgba.s_rgba.r, rgba.s_rgba.g,
		       rgba.s_rgba.b, rgba.s_rgba.a);
    }
  SPR_SetPosPoly(spr->obj, pos);  /* 座標値の設定 */

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);
  tex_conv(spr, form1, form2, rate);

  return 0;
}

/*
 * 三角形オブジェクトの補間
 */
int lconvTriangle(l2dSprite * spr, int form1, int form2, float rate)
{
  l2dStatus * stat1;  /* 状態1(旧状態) */
  l2dStatus * stat2;  /* 状態2(新状態) */
  SPR_POS pos[3];
  RGBA rgba;
  int i;

  stat1 = spr->stat + form1;
  stat2 = spr->stat + form2;

  /* 座標値、RGBA 値の補間 */
  for(i = 0; i < POS_VTX_TRIANGLE; i++)
    {
      pos_conv(&pos[i], spr->vertex + i, stat2->vertex + i, rate);
      rgba_conv(&rgba, spr->vertex + i, stat2->vertex + i, rate);
      /* RGBA 値はこの段階で設定してしまう */
      SPR_SetColorTriangle(spr->obj, i,
			   rgba.s_rgba.r, rgba.s_rgba.g,
			   rgba.s_rgba.b, rgba.s_rgba.a);
    }
  SPR_SetPosTriangle(spr->obj, pos);  /* 座標値の設定 */

  /* 座標値係数 */
  magni_conv(spr, form1, form2, rate);

  /* 表示属性 */
  disp_stat(spr, stat2);
  tex_conv(spr, form1, form2, rate);

  return 0;
}
