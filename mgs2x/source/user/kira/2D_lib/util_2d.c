//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  layout_2d.c
  2D レイアウトエディタおよびコンバータで作成された、2D レイアウトデータの
  再生を行うモジュール。

  2001/01/10  Y.Kira

  $Id: util_2d.c,v 1.1.1.3 2002/11/19 11:43:52 Yoshizawa1 Exp $
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
#include "debugmenu.h"

#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"  /* 2D モジュールを使用 */

#define _util_2d_h_
#include "util_2d.h"

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

/*
 * 指定されたオブジェクト以下のオブジェクトのアルファ値に,
 * 指定された値を設定して回る。元の値は無視する。
 */
int U2D_TreeAlpha(SPR_OBJ * obj, int alpha)
{
  SPR_OBJ * child;
  int cnt = 1;

  switch(obj->head.id)
    {
    case SP_EMPTY: break;
    case SP_POINT:
      obj->point.col.a = alpha;
      break;
    case SP_LINE:
      obj->line.col[0].a = alpha;
      obj->line.col[1].a = alpha;
      break;
    case SP_LINESTRIP:
      {
         int i;

         for(i = 0; i < obj->linestrip.pcnt; i++)
            obj->linestrip.plist[i].col.a = alpha;
      }
      break;
    case SP_BOX:
    case SP_BOX_F:
      obj->box.col.a = alpha;
      break;
    case SP_SPRITE:
    case SP_SPRITE_F:
      obj->sprite.col.a = alpha;
      break;
      
    case SP_ZOOM:
    case SP_ZOOM_F:
      obj->zoom.col.a = alpha;
      break;

    case SP_SPIN:
    case SP_SPIN_F:
      obj->spin.col.a = alpha;
      break;
      
    case SP_POLY:
    case SP_POLY_F:
      {
         int i;

         for(i = 0; i < 4; i++)
            obj->poly.col[i].a = alpha;
      }
      break;

    case SP_TRIANGLE:
      {
         int i;

         for(i = 0; i < 3; i++)
            obj->triangle.col[i].a = alpha;
      }
      break;
    }

  /* 自分に子がいるなら、その子を全て処理する */
  for(child = obj->head.child; NULL != child; child = child->head.next)
    cnt += U2D_TreeAlpha(child, alpha);
  
  return cnt;
}

/*
 * 指定されたオブジェクト以下のオブジェクトのアルファ値に,
 * 指定された係数をかけて回る。元の値は無視する。
 */
int U2D_TreeMulAlpha(SPR_OBJ * obj, int alpha)
{
  SPR_OBJ * child;
  int cnt = 1;

  switch(obj->head.id)
    {
    case SP_EMPTY: break;
    case SP_POINT:
      obj->point.col.a = obj->point.col.a * alpha / 128;
      break;
    case SP_LINE:
      obj->line.col[0].a = obj->line.col[0].a * alpha / 128;
      obj->line.col[1].a = obj->line.col[1].a * alpha / 128;
      break;
    case SP_LINESTRIP:
      {
	int i;

	for(i = 0; i < obj->linestrip.pcnt; i++)
	  obj->linestrip.plist[i].col.a =
	    obj->linestrip.plist[i].col.a * alpha / 128;
      }
      break;
    case SP_BOX:
    case SP_BOX_F:
      obj->box.col.a = obj->box.col.a * alpha / 128;
      break;
    case SP_SPRITE:
    case SP_SPRITE_F:
      obj->sprite.col.a = obj->sprite.col.a * alpha / 128;
      break;
      
    case SP_ZOOM:
    case SP_ZOOM_F:
      obj->zoom.col.a =  obj->zoom.col.a * alpha / 128;
      break;

    case SP_SPIN:
    case SP_SPIN_F:
      obj->spin.col.a =  obj->spin.col.a * alpha / 128;
      break;
      
    case SP_POLY:
    case SP_POLY_F:
      {
	int i;

	for(i = 0; i < 4; i++)
	  obj->poly.col[i].a =  obj->poly.col[i].a * alpha / 128;
      }
      break;

    case SP_TRIANGLE:
      {
	int i;

	for(i = 0; i < 3; i++)
	  obj->triangle.col[i].a =  obj->triangle.col[i].a * alpha / 128;
      }
      break;
    }

  /* 自分に子がいるなら、その子を全て処理する */
  for(child = obj->head.child; NULL != child; child = child->head.next)
    cnt += U2D_TreeAlpha(child, alpha);
  
  return cnt;
}

/*
 * 特定のオブジェクト以下のツリーに対し,同じ関数を適用する
 */
void U2D_TreeProc(void * workp, SPR_OBJ * obj,
		  void (*proc)(void * workp, SPR_OBJ * obj))
{
  SPR_OBJ * child;

  (proc)(workp, obj);

  /* 自分に子がいるなら、その子を全て処理する */
  for(child = obj->head.child; NULL != child; child = child->head.next)
    U2D_TreeProc(workp, child, proc);
}
