//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  sprite_2d.c
  2D表示系用スプライトライブラリ

  $Id: sprite_2d.c,v 1.1.1.3 2002/11/19 11:43:49 Yoshizawa1 Exp $
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
#include "libdg.h"

#define _sprite_2d_c_
#include "sprite_2d.h"
#include "sprite_dma.h"

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

#endif /* PSX2 */


#define SET_SPR_COLOR( _v, _r, _g, _b, _a ) { (_v).r = _r ; (_v).g = _g ; (_v).b = _b ; (_v).a = _a ; }



/*
 * プリミティブに親オブジェクトを定義する
 */
int sprPrimSetParent(SPR_OBJ * prim, SPR_OBJ * parent)
{
  /*
   * 親子関係の設定
   */
  /*
   * 既に親が登録されている場合は、
   * 親、兄弟とは縁を切り、子だけが残る
   */
  if(prim->head.parent != NULL)
    {
      /* 以前の親との縁を切る */
      if(prim->head.parent->head.child == prim)
	prim->head.parent->head.child = prim->head.next;
      prim->head.parent = NULL;

    }
  else
    {
      /* もともと親が無い root object である場合は、
	 一旦 root object としての登録を抹消する */
      sprErasePrim(prim);
    }

  /* 兄弟との縁を切る */
  if(prim->head.next != NULL)
    prim->head.next->head.prev = prim->head.prev;
  if(prim->head.prev != NULL)
    prim->head.prev->head.next = prim->head.next;
  prim->head.next = NULL;
  prim->head.prev = NULL;
  
  /* 親を登録 */
  if(NULL != (prim->head.parent = parent))
    {
      /* 親として指定されたオブジェクトがある場合 */
      /* 親の子を兄弟と認識 */
      if(NULL != (prim->head.next = parent->head.child))
	prim->head.next->head.prev = prim; /* 兄弟に自分を認知させる */
      parent->head.child = prim; /* 親に自分を末の子として認知させる */
    }
  else
    {
      /* 親として NULL が指定された場合、root object として登録する。 */
      sprRegistPrim(prim);
    }
  return 0;
}


int SPR_SetPriority(SPR_OBJ * prim, SPR_PRI pri)
{
  prim->head.pri = pri;
  return 0;
}


/*
 * プリミティブを任意の属性で初期化する
 */
static int sprPrimInit(SPR_ID id, int chanl,
		       SPR_OBJ * prim, SPR_OBJ * parent)
{
  int flags;
  int ret;

  prim->head.parent = NULL;
  prim->head.child  = NULL;
  prim->head.next   = NULL;
  prim->head.prev   = NULL;

  prim->head.func_free = spr_get_func_free(); /* 開放関数を設定 */

  prim->head.id    = id;
  prim->head.pri   = 0;
  prim->head.flags = SPR_FLAG_HIDDEN;
  prim->head.scale = 1.0F;

  prim->head.chanl = chanl;
  prim->head.name  = -1;    /* プログラム中で生成したオブジェクトは、
			       name ID を持たない                     */
  prim->head.alpha = SCE_GS_SET_ALPHA( 0,1,0,1,0x00 ) ;

  flags = 0;
  switch(id)
    {
    case SP_EMPTY:
      prim->empty.pos.x = 0;
      prim->empty.pos.y = 0;
      break;

    case SP_POINT:
      prim->point.pos.x = 0;
      prim->point.pos.y = 0;
      prim->point.col.r = 0;
      prim->point.col.g = 0;
      prim->point.col.b = 0;
      prim->point.col.a = 0;
      break;

    case SP_LINE:
      prim->line.pos[0].x = prim->line.pos[1].x = 0;
      prim->line.pos[0].y = prim->line.pos[1].y = 0;
      prim->line.col[0].r = prim->line.col[1].r = 0;
      prim->line.col[0].g = prim->line.col[1].g = 0;
      prim->line.col[0].b = prim->line.col[1].b = 0;
      prim->line.col[0].a = prim->line.col[1].a = 0;
      break;
    case SP_LINESTRIP:
      /* 頂点確保関数のポインタを得る */
      prim->linestrip.func_malloc = spr_get_func_malloc();
      prim->linestrip.pcnt = 0;
      prim->linestrip.plist = NULL;
      break;

    case SP_BOX:
    case SP_BOX_F:
      prim->box.rect.begin.x  = prim->box.rect.end.x   = 0;
      prim->box.rect.begin.y  = prim->box.rect.end.y   = 0;
      prim->box.col.r = 0;
      prim->box.col.g = 0;
      prim->box.col.b = 0;
      prim->box.col.a = 0;
      break;

    case SP_SPRITE:
    case SP_SPRITE_F:
      flags = SPR_FLAG_NO_TEX;
      prim->sprite.pos.x = 0;
      prim->sprite.pos.y = 0;
      //prim->sprite.col = (SPR_COLOR){255, 255, 255, 128};
	  SET_SPR_COLOR( prim->sprite.col, 255, 255, 255, 128 );
      break;

    case SP_ZOOM:
    case SP_ZOOM_F:
      flags = SPR_FLAG_NO_TEX;
      prim->zoom.rect.begin.x = 0;
      prim->zoom.rect.begin.y = 0;
      prim->zoom.rect.end.x = 0;
      prim->zoom.rect.end.y = 0;
      //prim->zoom.col = (SPR_COLOR){255, 255, 255, 128};
	  SET_SPR_COLOR( prim->zoom.col, 255, 255, 255, 128 );
      break;

    case SP_SPIN:
    case SP_SPIN_F:
      flags = SPR_FLAG_NO_TEX;
      prim->spin.pos.x = prim->spin.pos.y = 0;
      prim->spin.center.x = prim->spin.center.y = 0;
      //prim->spin.col = (SPR_COLOR){255, 255, 255, 128};
	  SET_SPR_COLOR( prim->spin.col, 255, 255, 255, 128 );
      break;
    case SP_POLY:
    case SP_POLY_F:
      {
	int i;

	flags = SPR_FLAG_NO_TEX;
	for(i = 0; i < 4; i++)
	  {
	    prim->poly.vert[i].x = prim->poly.vert[i].y = 0;
	    //prim->poly.col[i] = (SPR_COLOR){255, 255, 255, 128};
		SET_SPR_COLOR( prim->poly.col[i], 255, 255, 255, 128 );
	  }
      }
      break;
    case SP_TRIANGLE:
      {
	int i;

	flags = SPR_FLAG_NO_TEX;
	for(i = 0; i < 3; i++)
	  {
	    prim->triangle.vert[i].x = prim->triangle.vert[i].y = 0;
	    //prim->triangle.col[i] = (SPR_COLOR){255, 255, 255, 128};
		SET_SPR_COLOR( prim->triangle.col[i], 255, 255, 255, 128 );
	  }
      }
      break;
    default:
      return -1;
    }
  prim->head.flags |=  flags;

  /*
   * テクスチャが必要な種類のプリミティブであれば、
   * 初期状態としてテクスチャを未設定にする。
   */
  if(flags & SPR_FLAG_NO_TEX)
    {
      prim->ex_hd.tex.dgtex = NULL;
      prim->ex_hd.tex.pabe  = 0;
    }

  if(sprCreate_DMA_Packet(prim))  /* 描画に必要な DMA パケットを生成する */
    return -1;   /* DMA パケットが生成できなければエラー */

  /* DMA パケットの生成にも問題がなければ、親子関係を構築する */
  sprPrimSetParent(prim, parent);

  return 0;
}

/*
 * 2Dオブジェクト領域の確保と初期化を合わせたコンビニエンス関数
 */
SPR_OBJ * SPR_Create_2D_Object(SPR_ID id, int chanl, SPR_OBJ * parent)
{
  SPR_OBJ * prim;
  size_t siz = sizeof(SPR_OBJ);

  if(spr_driver_active_status()) return NULL;

  switch(id)
    {
    case SP_EMPTY:     siz = sizeof(SPR_EMPTY);     break;
    case SP_POINT:     siz = sizeof(SPR_POINT);     break;
    case SP_LINE:      siz = sizeof(SPR_LINE);      break;
    case SP_LINESTRIP: siz = sizeof(SPR_LINESTRIP); break;
    case SP_BOX:       siz = sizeof(SPR_BOX);       break;
    case SP_BOX_F:     siz = sizeof(SPR_BOX);       break;
    case SP_SPRITE:    siz = sizeof(SPR_SPRITE);    break;
    case SP_SPRITE_F:  siz = sizeof(SPR_SPRITE);    break;
    case SP_ZOOM:      siz = sizeof(SPR_ZOOM);      break;
    case SP_ZOOM_F:    siz = sizeof(SPR_ZOOM);      break;
    case SP_SPIN:      siz = sizeof(SPR_SPIN);      break;
    case SP_SPIN_F:    siz = sizeof(SPR_SPIN);      break;
    case SP_POLY:      siz = sizeof(SPR_POLY);      break;
    case SP_POLY_F:    siz = sizeof(SPR_POLY);      break;
    case SP_TRIANGLE:  siz = sizeof(SPR_TRIANGLE);  break;
    default: return NULL;
    }

  if(NULL == (prim = spr_malloc(siz))) return NULL;

  if(sprPrimInit(id, chanl, prim, parent))
    {
      spr_free(prim);
      return NULL;
    }
#if 0
  sprDebugObjList();
#endif
  return prim;
}

/*
 * SPR_Create2D_Object() 関数で確保した2Dオブジェクト領域の開放
 *
 * 子孫となっている全てのオブジェクトが、SPR_Create_2D_Object() で
 * 生成されている必要がある。
 */
int SPR_Destroy_2D_Object(SPR_OBJ * prim)
{
  SPR_OBJ *bros, *next;

  if(spr_driver_active_status()) return -1;

  if(NULL == prim) return -1;


  DBG("[1]\n");

  /* 子を全て殺す */
  bros = prim->head.child;
  while(bros != NULL)
    {
      next = bros->head.next;
      SPR_Destroy_2D_Object(bros);
      bros = next;
    }
  DBG("[2]\n");


  /* 親、兄弟と縁を断つ */
  if(prim->head.parent != NULL)
    {
      /* 親オブジェクトがある場合 */
      if(prim->head.prev == NULL)
	prim->head.parent->head.child = prim->head.next;
    }
  
  if(prim->head.next != NULL) prim->head.next->head.prev = prim->head.prev;
  if(prim->head.prev != NULL) prim->head.prev->head.next = prim->head.next;
  DBG("[3]\n");


  /* root object であれば、root object としての登録を抹消 */
  sprErasePrim(prim);

  DBG("[4] sprite id = %d\n", prim->head.id);

  /* LineStrip の場合は頂点リストを殺す */
  if((prim->head.id == SP_LINESTRIP) &&
     (prim->linestrip.plist != NULL))
    {
      (prim->head.func_free)(prim->linestrip.plist);
      DBG("LineStrip Vertex buffer free.(%p)\n", prim->linestrip.plist);
    }

  /* DMA 領域を開放する */
  if((prim->head.id != SP_EMPTY) &&
     (prim->head.packet != NULL))
    {
      (prim->head.func_free)(prim->head.packet);
      DBG("DMA packet free.(%p)\n", prim->head.packet);
    }

  DBG("[5]\n");
  /* 領域を殺す */
  (prim->head.func_free)(prim);
  DBG("Prim Free(%p)\n", prim);
  DBG("[6]\n");
  return 0;
}

/* Empty の座標指定 */
int SPR_SetPosEmpty(SPR_OBJ * prim, SPR_POS * pos)
{
  if(prim->head.id != SP_EMPTY) return -1;
  prim->empty.pos = *pos;
  return 0;
}


/* 折れ線の頂点数指定 */
int SPR_SetLineStripVertexNumber(SPR_OBJ * prim, int num)
{
  struct spr_lstrip_pos * plist;
  int i;

  if(prim->head.id != SP_LINESTRIP) return -1;

  if(NULL == (plist = (prim->linestrip.func_malloc)(sizeof(struct spr_lstrip_pos) * num)))
    return -1;

  /* 以前に設定された頂点リストがある場合 */
  if(prim->linestrip.pcnt > 0)
    {
      /* 古い頂点リストの内容は保持される */
      for(i = 0; i < prim->linestrip.pcnt && i < num; i++)
	plist[i] = prim->linestrip.plist[i];

      /* 古い頂点リスト領域を開放する */
      (prim->head.func_free)(prim->linestrip.plist);

      /* 古い DMA パケット領域を開放する */
      (prim->head.func_free)(prim->head.packet);
    }

  prim->linestrip.pcnt = num;
  prim->linestrip.plist = plist;

  DBG("---<A>---\n");
  /*
   * 新しい頂点数に基づき、DMA パケットを生成する
   */
  sprCreate_DMA_Packet(prim);

  DBG("---<B>---\n");

  return 0;
}

/* 点の座標設定 */
int SPR_SetPosPoint(SPR_OBJ * prim, SPR_POS * pos)
{
  if(prim->head.id != SP_POINT) return -1;
  prim->point.pos = *pos;
  return 0;
}

/* 点の色設定 */
int SPR_SetColorPoint(SPR_OBJ * prim, int r, int g, int b, int a)
{
  if(prim->head.id != SP_POINT) return -1;
  prim->point.col.r = r;
  prim->point.col.g = g;
  prim->point.col.b = b;
  prim->point.col.a = a;
  return 0;
}


/* 線分の座標設定 */
int SPR_SetPosLine(SPR_OBJ * prim, SPR_POS *begin, SPR_POS *end)
{
  if(prim->head.id != SP_LINE) return -1;

  prim->line.pos[0] = *begin;
  prim->line.pos[1] = *end;

  return 0;
}

/* 線分の色設定 */
int SPR_SetColorLine(SPR_OBJ * prim, int p, int r, int g, int b, int a)
{
  if(prim->head.id != SP_LINE) return -1;

  prim->line.col[p].r = r;
  prim->line.col[p].g = g;
  prim->line.col[p].b = b;
  prim->line.col[p].a = a;

  return 0;
}

/* 折れ線の頂点座標指定
 * 指定した頂点番号から指定した個数の頂点を設定する */
int SPR_SetPosLineStrip(SPR_OBJ * prim, int begin, int num, SPR_POS * pos)
{
  int i;

  if(prim->head.id != SP_LINESTRIP) return -1;
  if(begin >= prim->linestrip.pcnt) return -1;
  if((begin + num - 1) >= prim->linestrip.pcnt) return -1;
  for(i = 0; i < num; i++)
    {
      prim->linestrip.plist[begin + i].pos.x = pos[i].x;
      prim->linestrip.plist[begin + i].pos.y = pos[i].y;
    }
  return 0;
}

/* 折れ線の頂点色指定 */
int SPR_SetColorLineStrip(SPR_OBJ * prim, int p, int r, int g, int b, int a)
{
  if(prim->head.id != SP_LINESTRIP) return -1;

  prim->linestrip.plist[p].col.r = r;
  prim->linestrip.plist[p].col.g = g;
  prim->linestrip.plist[p].col.b = b;
  prim->linestrip.plist[p].col.a = a;

  return 0;
}

/* 長方形の座標指定 */
int SPR_SetPosBox(SPR_OBJ * prim, SPR_RECT * rect)
{
  if(prim->head.id != SP_BOX && prim->head.id != SP_BOX_F) return -1;
  prim->box.rect = *rect;
  return 0;
}

/* 長方形の色設定 */
int SPR_SetColorBox(SPR_OBJ * prim, int r, int g, int b, int a)
{
   if(prim->head.id != SP_BOX && prim->head.id != SP_BOX_F) return -1;
  prim->box.col.r = r;
  prim->box.col.g = g;
  prim->box.col.b = b;
  prim->box.col.a = a;
  
  return 0;
}

/* 等倍矩形スプライトの座標指定 */
int SPR_SetPosSprite(SPR_OBJ * prim, SPR_POS * pos)
{
  if(prim->head.id != SP_SPRITE && prim->head.id != SP_SPRITE_F) return -1;
  prim->sprite.pos = *pos;
  return 0;
}

/* サイズ指定矩形ビットマップの表示サイズ指定 */
int SPR_SetSizeSprite(SPR_OBJ * prim, float w, float h)
{
   if(prim->head.id != SP_SPRITE && prim->head.id != SP_SPRITE_F) return -1;
  prim->sprite.dw = w;
  prim->sprite.dh = h;
  return 0;
}

/* サイズ指定矩形ビットマップの表示 RGB値指定 */
int SPR_SetColorSprite(SPR_OBJ * prim, int r, int g, int b, int a)
{
   if(prim->head.id != SP_SPRITE && prim->head.id != SP_SPRITE_F) return -1;
  prim->sprite.col.r = r;
  prim->sprite.col.g = g;
  prim->sprite.col.b = b;
  prim->sprite.col.a = a;
  
  return 0;
}


/* 回転ビットマップの表示サイズ指定 */
int SPR_SetSizeSpin(SPR_OBJ * prim, float w, float h)
{
  if(prim->head.id != SP_SPIN && prim->head.id != SP_SPIN_F) return -1;

  prim->spin.dw = w;
  prim->spin.dh = h;

  return 0;
}

/* 回転ビットマップの回転中心指定 */
int SPR_SetCenterSpin(SPR_OBJ * prim, SPR_POS * center)
{
   if(prim->head.id != SP_SPIN && prim->head.id != SP_SPIN_F) return -1;
  prim->spin.center = *center;
  return 0;
}

/* 回転ビットマップの回転角指定 */
int SPR_SetRollSpin(SPR_OBJ * prim, float spin)
{
   if(prim->head.id != SP_SPIN && prim->head.id != SP_SPIN_F) return -1;

  prim->spin.spin = spin;

  return 0;
}

/* 対角指定矩形ビットマップの座標指定 */
int SPR_SetPosZoom(SPR_OBJ * prim, SPR_RECT * rect)
{
  if(prim->head.id != SP_ZOOM && prim->head.id != SP_ZOOM_F) return -1;
  prim->zoom.rect = *rect;

  return 0;
}

/* サイズ指定矩形ビットマップの表示 RGB値指定 */
int SPR_SetColorZoom(SPR_OBJ * prim, int r, int g, int b, int a)
{
   if(prim->head.id != SP_ZOOM && prim->head.id != SP_ZOOM_F) return -1;
  prim->zoom.col.r = r;
  prim->zoom.col.g = g;
  prim->zoom.col.b = b;
  prim->zoom.col.a = a;
  
  return 0;
}

/* 回転スプライトの中心座標指定 */
int SPR_SetPosSpin(SPR_OBJ * prim, SPR_POS * pos)
{
   if(prim->head.id != SP_SPIN && prim->head.id != SP_SPIN_F) return -1;
  prim->spin.pos = *pos;
  return 0;
}

/* 回転スプライトの表示 RGB値指定 */
int SPR_SetColorSpin(SPR_OBJ * prim, int r, int g, int b, int a)
{
   if(prim->head.id != SP_SPIN && prim->head.id != SP_SPIN_F) return -1;
  prim->spin.col.r = r;
  prim->spin.col.g = g;
  prim->spin.col.b = b;
  prim->spin.col.a = a;
  
  return 0;
}

/* 任意4頂点指定スプライトの頂点座標指定 */
int SPR_SetPosPoly(SPR_OBJ * prim, SPR_POS * pos)
{
  int i;

  if(prim->head.id != SP_POLY && prim->head.id != SP_POLY_F) return -1;

  for(i = 0; i < 4; i++)
    prim->poly.vert[i] = pos[i];
  return 0;
}

/*
 * 任意4頂点指定スプライトの頂点色指定
 */
int SPR_SetColorPoly(SPR_OBJ * prim, int p, int r, int g, int b, int a)
{
   if(prim->head.id != SP_POLY && prim->head.id != SP_POLY_F) return -1;

  prim->poly.col[p].r = r;
  prim->poly.col[p].g = g;
  prim->poly.col[p].b = b;
  prim->poly.col[p].a = a;

  return 0;
}

/*
 * 三角形オブジェクトの頂点座標指定
 */
int SPR_SetPosTriangle(SPR_OBJ * prim, SPR_POS * pos)
{
  int i;

  if(prim->head.id != SP_TRIANGLE) return -1;

  for(i = 0; i < 3; i++)
    prim->triangle.vert[i] = pos[i];
  return 0;
}

/*
 * 三角形オブジェクトの頂点色指定
 */
int SPR_SetColorTriangle(SPR_OBJ * prim, int p, int r, int g, int b, int a)
{
  if(prim->head.id != SP_TRIANGLE) return -1;

  prim->triangle.col[p].r = r;
  prim->triangle.col[p].g = g;
  prim->triangle.col[p].b = b;
  prim->triangle.col[p].a = a;

  return 0;
}


/* オブジェクトツリーのデュプリケイト */
static SPR_OBJ * spr_dup_single_obj(SPR_OBJ * node, SPR_OBJ * objparent)
{
  SPR_OBJ * obj;
  SPR_OBJ * chobj;
  SPR_OBJ * nextobj;
  void * packet;
  void * pk[2];
  size_t siz;
  void * parent;
  void * child;
  void * next;
  void * prev;
  void * (* func_malloc)(long64 siz);
  void   (* func_free)(void * ptr);

  /* 一度正規の手続きを経て、同じ属性のオブジェクトを生成 */
  obj = SPR_Create_2D_Object(node->head.id,    /* 同じ属性     */
			     node->head.chanl, /* 同じチャネル */
			     objparent);    /* 同じ親       */


  if(NULL == obj)
    {
      printf("Warning: <layout_2d.c:SPR_DuplicateTree()> not enough memory.\n");
      return NULL;
    }

  /*
   * 各種属性、パラメータのコピー。
   * DMA パケット領域はコピーしてはいけない。
   */
  /* DMA パケット関連のポインタを一時退避 */
  packet = obj->head.packet;
  pk[0] = obj->head.pk[0];
  pk[1] = obj->head.pk[1];

  /* リンクポインタを一時退避 */
  parent = obj->head.parent;
  child  = obj->head.child;
  next   = obj->head.next;
  prev   = obj->head.prev;

  /* 開放関数のポインタを一時退避 */
  func_free = obj->head.func_free;

  /* LineStrip ならば、頂点バッファ確保用関数のポインタも退避 */
  if(obj->head.id == SP_LINESTRIP)
    func_malloc = obj->linestrip.func_malloc;

  /* 構造体内容をコピー */
  switch(node->head.id)
    {
    case SP_EMPTY:     siz = sizeof(SPR_EMPTY);     break;
    case SP_POINT:     siz = sizeof(SPR_POINT);     break;
    case SP_LINE:      siz = sizeof(SPR_LINE);      break;
    case SP_BOX:       siz = sizeof(SPR_BOX);       break;
    case SP_BOX_F:     siz = sizeof(SPR_BOX);       break;
    case SP_LINESTRIP: siz = sizeof(SPR_LINESTRIP); break;
    case SP_SPRITE:    siz = sizeof(SPR_SPRITE);    break;
    case SP_SPRITE_F:  siz = sizeof(SPR_SPRITE);    break;
    case SP_ZOOM:      siz = sizeof(SPR_ZOOM);      break;
    case SP_ZOOM_F:    siz = sizeof(SPR_ZOOM);      break;
    case SP_SPIN:      siz = sizeof(SPR_SPIN);      break;
    case SP_SPIN_F:    siz = sizeof(SPR_SPIN);      break;
    case SP_POLY:      siz = sizeof(SPR_POLY);      break;
    case SP_POLY_F:    siz = sizeof(SPR_POLY);      break;
    case SP_TRIANGLE:  siz = sizeof(SPR_TRIANGLE);  break;
    default:           siz = 0; break; // BP ADDED - IN CASE OTHERS (like SP_CLIP_
    }
  /* 構造体領域をコピー */
  memcpy(obj, node, siz);

  /* メモリ関連関数ポインタを書き戻す */
  obj->head.func_free = func_free;
  if(obj->head.id == SP_LINESTRIP)
    obj->linestrip.func_malloc = func_malloc;


  /* DMA パケット領域のポインタを書き戻す */
  obj->head.packet = packet;
  obj->head.pk[0] = pk[0];
  obj->head.pk[1] = pk[1];
  
  /* リンクポインタを書き戻す */
  obj->head.parent = parent;
  obj->head.child  = child;
  obj->head.next   = next;
  obj->head.prev   = prev;

  /* 折れ線の場合は、頂点バッファの内容も複製が必要 */
  if(node->head.id == SP_LINESTRIP)
    {
      /* 同じ個数分の頂点バッファを確保 */
      SPR_SetLineStripVertexNumber(obj, node->linestrip.pcnt);

      /* 頂点バッファ内の値をコピー */
      memcpy(obj->linestrip.plist,
	     node->linestrip.plist,
	     node->linestrip.pcnt * sizeof(struct spr_lstrip_pos));
    }
  
  /* 複製元の子を全て複製して、複製先の子のオブジェクトとして登録 */
  for(chobj = node->head.child; chobj != NULL; chobj = chobj->head.next)
    if(NULL == spr_dup_single_obj(chobj, obj))
      {
	/* 今回作ったオブジェクトと,その子を全て破棄 */
	SPR_Destroy_2D_Object(obj);
	return NULL;
      }
  
  /* 複製された子を逆順に並べなおす。
     登録順によって描画順が変わることがあるため。 */
  chobj = obj->head.child;
  while(chobj != NULL)
    {
      nextobj = chobj->head.next;
      chobj->head.next = chobj->head.prev;
      chobj->head.prev = nextobj;
      obj->head.child = chobj;
      chobj = nextobj;
    }
  
  return obj;
}

/*
 * オブジェクトツリーの複製
 */
SPR_OBJ * SPR_DuplicateTree(SPR_OBJ * node)
{
  return spr_dup_single_obj(node, node->head.parent);
}
