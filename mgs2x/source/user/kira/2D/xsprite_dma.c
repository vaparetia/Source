//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  xsprite_dma.c
  スプライト DMA パケット管理モジュール
  
  $Id: xsprite_dma.c,v 1.5 2002/11/23 12:16:41 Yoshizawa1 Exp $
*/
/*
  sprite_2d.c の関数等で作成されたスプライトの描画 DMA パケットを生成し、
  描画を行う。
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if 0 //BP_XBOX
#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"dmapack.h"
#include	"gameheader.h"
#include	"libutl.h"
#include	"def_dma.h"

#define NO_TEX1

#define DEFAULT_Z 0xffffffff

#define _sprite_dma_c_
#include "sprite_dma.h"

/* 各スプライトモードの DMA パケット初期値オリジナル */
#include "sprite_dma_defaults.c"

extern void *DG_SetDmapackNext( void *addr, void *next );

int DG_WriteTextureChangePacks( void *tag_addr, void *tex_packet );

#ifdef PSX2

#ifdef _DEBUG_
#define DBG(...)  printf(_VA_ARGS_)
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






#define ALL_INVISIBLE  (DG_DMAPACK_INVISIBLE0 | DG_DMAPACK_INVISIBLE1 | \
                        DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3 | \
                        DG_DMAPACK_INVISIBLEMENU)

#define DMA_DISABLE  ALL_INVISIBLE
#define DMA_ENABLE   (~ALL_INVISIBLE)

#define SWAP(a, b)  b ^= a, a ^= b, b ^= a
#define FSWAP(a,b)	{float tmp = a ; a = b ; b = tmp ; }


/*
 * 2D スプライトドライバ管理ワーク
 */
typedef struct {
	void		*dmapack ;
	void		*work ;
	void		*param0 ;
	int			chanl ;
} CallbackParam ;

typedef struct xsprite_dma_Work 
{
  GV_ACT              actor;
  
  SPR_OBJ           * root_object;     /* 親オブジェクトの chain 始点      */

  /* ---- ここから上は変わらない ---- */

  /* テクスチャリスト構造体のポインタ */
  DG_TEXTURE_LIST   * tex_list[MAX_TRI];
  unsigned int        tri_code[MAX_TRI];
  int                 tri_cnt[MAX_TRI];  /* SPR_LoadTexture() の回数 */

  /* 2Dオブジェクト転送用 DMAPACK 構造体 */
  DG_DMAPACK        * tex_dmapack[MAX_CHANL];

  /* 各チャネルの描画開始点 */
  spr_TransRoot       dmaroot[ MAX_CHANL ][2];

  /* 各チャネルの描画環境リセット        */
  spr_TexRecov        tex_recov[MAX_CHANL][2];

  /* 各チャネル、各プライオリティにおける、各 tri 転送パケット */
  spr_TexTrans        dma_tex[MAX_CHANL][SPR_PRI_NUM][MAX_TRI][2];

  /* 各チャネル、各プライオリティ、
     各TRI にぶらさがるオブジェクトの始点と終点 */
  DG_DMATAG         * dma_begin[MAX_CHANL][SPR_PRI_NUM][MAX_TRI + 1][2];
  DG_DMATAG         * dma_end[MAX_CHANL][SPR_PRI_NUM][MAX_TRI + 1][2];

  /* ＸＢＯＸ用拡張 */
  CallbackParam		callback_param[ MAX_CHANL ];
  void				*last ;
  void				*end_packet ;
  void				*packet_begin[MAX_CHANL][SPR_PRI_NUM][MAX_TRI + 1];	/* dma_beginの代わり */
  void				*packet_end[MAX_CHANL][SPR_PRI_NUM][MAX_TRI + 1];	/* dma_endの代わり */

  /* スプライトオブジェクトのメモリ確保/開放関数 */
  void * (*func_malloc)(long64 size);
  void   (*func_free)(void * ptr);

#ifdef DEBUG_MODE
  int      trans_tex_size;  /* デバッグ用: テクスチャ総転送量 */

#endif /* DEBUG_MODE */
} Work;

//static int data_buffer[ 32 * 1024 ];
static int data_buffer[ 48 * 1024 ];/* 増やしました yano 2002.06.25 */

static int disp_mask[] = {
  ~DG_DMAPACK_INVISIBLE0,
  ~DG_DMAPACK_INVISIBLE1,
  ~DG_DMAPACK_INVISIBLE2,
  ~DG_DMAPACK_INVISIBLE3,
  ~DG_DMAPACK_INVISIBLEMENU
};


static Work * sys_work = NULL;
static void DmapackCallback( void *param );

static void init_packet_list(Work * work)
{
#ifndef KP_WINDOWS
  int c, p, t;

  /* 全チャネル、全プライオリティ、全 TRI において、
     対応するオブジェクトを無いことにする */
  for(c = 0; c < MAX_CHANL; c++)
    for(p = 0; p < SPR_PRI_NUM; p++)
      for(t = 0; t < MAX_TRI + 1; t++){
		  work->dma_begin[c][p][t][DG_Clock] = work->dma_end[c][p][t][DG_Clock] = NULL;
		  work->packet_begin[c][p][t] = work->last ;
		  work->packet_end[c][p][t] = work->last ;
		  work->last = DG_SetDmapackNext( work->last, work->end_packet );
	  }
#else
	DG_DMATAG	**dma_begin, **dma_end ;
	void		**packet_begin, **packet_end ;
	void		*last, *end_packet ;
	int			c, p, t;

	dma_begin    = (DG_DMATAG **)work->dma_begin + DG_Clock ;
	dma_end      = (DG_DMATAG **)work->dma_end   + DG_Clock ;
	packet_begin = (void **)work->packet_begin ;
	packet_end   = (void **)work->packet_end ;
	last         = work->last ;
	end_packet   = work->end_packet ;
	for(c=MAX_CHANL; c>0; c--)
	{
	    for(p=SPR_PRI_NUM; p>0; p--)
		{
			for(t=MAX_TRI+1; t>0; t--)
			{
				*dma_begin = NULL ;
				*dma_end   = NULL ;

				*packet_begin = last ;
				*packet_end   = last ;
				last = DG_SetDmapackNext(last, end_packet) ;

				dma_begin += 2 ;
				dma_end   += 2 ;
				packet_begin++ ;
				packet_end++ ;
			}
		}
	}
	work->last = last ;
#endif
}

static void dmatag_link(DG_DMATAG * pre, DG_DMATAG * now)
{
  u_long64 qsiz;

  qsiz = pre->qwc & 0xffff;
  pre->qwc = DMATAG_SET_QWC(DMATAG_ID_NEXT, qsiz);
  pre->addr = now;
  pre->vifcode[0] = SCE_VIF1_SET_NOP(0);
  pre->vifcode[1] =
    (qsiz) ? SCE_VIF1_SET_DIRECT(qsiz, 0) : SCE_VIF1_SET_NOP(0);
}

/*
 * 指定された DMAtag を、描画分として登録する
 */
static void regist_obj_packet(Work * work, DG_DMATAG * dmatag,
			      int chanl, int pri, int tri)
{
  DG_DMATAG * pre;
  u_long64 qsiz;

  pre = work->dma_end[chanl][pri][tri][DG_Clock];
  if(NULL != pre)
    dmatag_link(pre, dmatag);
  else
    work->dma_begin[chanl][pri][tri][DG_Clock] = dmatag;

  work->dma_end[chanl][pri][tri][DG_Clock] = dmatag;

  DG_SetDmapackNext( work->packet_end[chanl][pri][tri], work->last );
}


/*
 * 登録されたパケットを、連続した DMA パケットとして接続する
 */
static void create_packet_flow(Work * work)
{
  int c, p, t;
  int last_tri;
  u_long64 qsiz;
  DG_DMATAG * lasttag;

  for(c = 0; c < MAX_CHANL; c++)
    {
      lasttag = &work->dmaroot[c][DG_Clock].dmatag;
      last_tri = -1;
      
      for(p = 0; p < SPR_PRI_NUM; p++)
	for(t = 0; t < MAX_TRI + 1; t++)
	  {
	    /* その .tri を使用するオブジェクトが
	       一つも描画されていなければ、処理をスキップする */
	    if(NULL == work->dma_begin[c][p][t][DG_Clock]) continue;
	    
	    /* 最後に転送した tri と、今回使用する tri が異なる場合のみ、
	       tri 転送パケットを連結する */
	    if((last_tri != t) && (t < MAX_TRI))
	      {
		dmatag_link(lasttag, &work->dma_tex[c][p][t][DG_Clock]);
		/* 最終転送パケットをテクスチャ転送パケットの nxt にする */
		lasttag = &work->dma_tex[c][p][t][DG_Clock].nxt;
		last_tri = t;  /* 最終転送 tri を、今回連結したものにする */
#ifdef DEBUG_MODE
		if(NULL == work->tex_list[t])
		  {
		    printf("Texture %d was killed.\n", t);
		    HANGUP();
		  }
		/* デバッグモード用の、テクスチャ総転送サイズを加算 */
		work->trans_tex_size += work->tex_list[t]->tex_size;
#endif /* DEBUG_MODE */

	      }
	    /* そのチャネルで最後に転送したパケットの直後に、
	       今回のパケット列を接続する */
	    dmatag_link(lasttag, work->dma_begin[c][p][t][DG_Clock]);
	    lasttag = work->dma_end[c][p][t][DG_Clock];
	  }
      /* 最後のパケットのたどりつく先を、環境復元パケットに指定する */
      dmatag_link(lasttag, &work->tex_recov[c][DG_Clock]);
    }
}


int spr_driver_active_status(void)
{
  if(NULL == sys_work) return -1;
  return 0;
}

/*
 * 指定されたプリミティブの、DMA パケット生成
 */
int sprCreate_DMA_Packet(SPR_OBJ * prim)
{
  SPR_HEADER    * head = &prim->head;  /* って、結局は同じ値になるんだけど。 */
  void          * packet_ptr[2];
  size_t          size = 0, inisize = 0;
  unsigned char * pack;
  void          * init_ptr = NULL;
  void * (*func_malloc)(long64 size);

  func_malloc = sys_work->func_malloc;

  switch(head->id)
    {
    case SP_EMPTY:
      head->packet  = NULL;
      return 0;  /* 何事もなかったかのように振舞う */
    case SP_POINT:
      inisize = size = sizeof(spr_DrawPoint);
      init_ptr = &def_draw_point;
      break;
    case SP_LINE:
      inisize = size = sizeof(spr_DrawLine);
      init_ptr = &def_draw_line;
      break;
    case SP_LINESTRIP:
      if(prim->linestrip.pcnt == 0)
	{
	  head->packet = NULL;
	  return 0;
	}
      /* 確保関数は,オブジェクトが生成された時点で指定されたものを使用する */
      func_malloc = prim->linestrip.func_malloc;
      size = sizeof(spr_DrawLineStrip) +
	sizeof(spr_DrawLS_pos) * prim->linestrip.pcnt;
      inisize = sizeof(spr_DrawLineStrip);
      init_ptr = &def_draw_lstrip;
      break;

    case SP_BOX:
    case SP_BOX_F:
      inisize = size  = sizeof(spr_DrawBox);
      init_ptr = &def_draw_box;
      break;

    case SP_SPRITE:
    case SP_SPRITE_F:
      inisize = size = sizeof(spr_DrawSprite);
      init_ptr = &def_draw_sprite;
      break;

    case SP_ZOOM:
    case SP_ZOOM_F:
      inisize = size = sizeof(spr_DrawZoom);
      init_ptr = &def_draw_zoom;
      break;
    case SP_SPIN:
    case SP_SPIN_F:
      inisize = size = sizeof(spr_DrawSpin);
      init_ptr = &def_draw_spin;
      break;
    case SP_POLY:
    case SP_POLY_F:
      inisize = size = sizeof(spr_DrawPoly);
      init_ptr = &def_draw_poly;
      break;
    case SP_TRIANGLE:
      inisize = size = sizeof(spr_DrawTriangle);
      init_ptr = &def_draw_triangle;
      break;
    }

  if(NULL == (pack = (func_malloc)(size * 2))) return -1;

  /* パケット0/1 の初期化 */
  memcpy(&pack[0],    init_ptr, inisize);
  memcpy(&pack[size], init_ptr, inisize);

  if(prim->head.id == SP_LINESTRIP)
    {
      spr_DrawLS_template * ls[2];
      int i;
      long64 sz;

      ls[0] = (spr_DrawLS_template *)pack;
      ls[1] = (spr_DrawLS_template *)(pack + size);

      /* 頂点数に応じて、転送サイズを書き換える */
      sz = (sizeof(struct _gif_lstrip) +
	    sizeof(spr_DrawLS_pos) * prim->linestrip.pcnt) / 16;
      ls[0]->head.dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, sz);
      ls[1]->head.dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, sz);
      ls[0]->head.dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(sz, 0);
      ls[1]->head.dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(sz, 0);

      sz = prim->linestrip.pcnt;
      ls[0]->head.gif.giftag_ver.tag = SCE_GIF_SET_TAG(sz, 1, 0, 0, 1, 2);
      ls[1]->head.gif.giftag_ver.tag = SCE_GIF_SET_TAG(sz, 1, 0, 0, 1, 2);
    }

  /* 各領域のポインタを取得 */
  packet_ptr[0] = &pack[0];
  packet_ptr[1] = &pack[size];
  head->packet = pack;
  head->pk[0] = packet_ptr[0];
  head->pk[1] = packet_ptr[1];
  head->q_size = size / 16 - 1;   /* DMAtag を除いた qword サイズを求める */
  DBG("q_size = %d\n", head->q_size);
  return 0;
}


#ifdef DEBUG_MODE
/*
 * 現在登録されている 2D オブジェクト領域のポインタを出力する
 */
static void disp_object_list(SPR_OBJ * prim)
{
  SPR_OBJ * bros;
  static int level = 0;
  int i;

  for(bros = prim; bros != NULL; bros = bros->head.next)
    {
      printf("2D object:");
      for(i = 0; i < level; i++) printf(" ");
      printf("%p\n", bros);

      if(bros->head.child != NULL)
	{
	  level++;
	  disp_object_list(bros->head.child);
	  level--;
	}
    }
}

int sprDebugObjList(void)
{
  if(sys_work == NULL) return -1;
  printf("----------\n");
  disp_object_list(sys_work->root_object);
  return 0;
}
#endif

/*
 * スプライトの登録
 */
int sprRegistPrim(SPR_OBJ * prim)
{
  DBG("Regist Sprite: %p\n", prim);
  if(sys_work == NULL)
    {
      DBG("sys_work == NULL\n");
      return -1;
    }
  /* root object のレジストは、parent が NULL のオブジェクトのみを対象とする */
  if(prim->head.parent != NULL) return -1;

  /* root object は、他の root object の兄弟として登録される */
  prim->head.next = sys_work->root_object;
  if(sys_work->root_object != NULL) sys_work->root_object->head.prev = prim;
  sys_work->root_object = prim;

#ifdef _DEBUG_
  {
    SPR_OBJ *o;

    for(o = sys_work->root_object; o != NULL; o = o->head.next)
      printf("Root Object: %p\n", o);
  }
#endif /* _DEBUG_ */

  return 0;
}

/*
 * スプライトの登録抹消: root object としての登録を抹消するだけ。
 */
int sprErasePrim(SPR_OBJ * prim)
{
  SPR_OBJ * bros;
  if(prim->head.parent != NULL) return -1;

  /* 指定されたオブジェクトが、root object に含まれていなければ何もしない */
  for(bros = sys_work->root_object; bros != NULL; bros = bros->head.next)
    {
      if(bros != prim) continue;
  
      /* root object のリンクから、指定されたオブジェクトを削除する */
      if(bros->head.prev == NULL)
	{
	  sys_work->root_object = prim->head.next;
	  DBG("destroy: root object = %p\n", sys_work->root_object);
	}

      /* 子以外どこともつながらないオブジェクトとする */
      prim->head.next = NULL;
      prim->head.prev = NULL;
      break;
    }

  return 0;
}

#ifdef _DEBUG_

static void _print_tex0(u_long64 tex0)
{
  int cld, csa, csm, cpsm, cbp, tfx, tcc, th, tw, psm, tbw, tbp0;

  tbp0 = tex0 & 0x3fff;
  tbw = (tex0 >> 14) & 0x3f;
  psm = (tex0 >> 20) & 0x3f;
  tw  = (tex0 >> 26) & 0x0f;
  th  = (tex0 >> 30) & 0x0f;
  tcc = (tex0 >> 34) & 1;
  tfx = (tex0 >> 35) & 3;
  cbp = (tex0 >> 37) & 0x3fff;
  cpsm = (tex0 >> 51) & 0x0f;
  csm = (tex0 >> 55) & 1;
  csa = (tex0 >> 56) & 0x1f;
  cld = tex0 >> 61;

  printf("TEX0: %x|%x|%x|%x|%x|%x|%x|%x|%x|%x|%x|%x\n",
	 cld, csa, csm, cpsm, cbp, tfx, tcc, th, tw, psm, tbw, tbp0);
}
#define PRINT_TEX0(tex0)  _print_tex0((tex0))
#else
#define PRINT_TEX0(tex0)
#endif /* _DEBUG_ */
/*
 * 兄弟階層の処理
 *
 * attrib   親から継承した表示属性
 * chanl    表示描画チャネル
 * base_x   親の持つ基準x座標
 * base_y   親の持つ基準y座標
 */
#if 0 //BP_PSX2
#define ChanlBaseX(chanl) SPR_FIXED(2048 - DG_Chanls[(chanl)].width / 2)
#define ChanlBaseY(chanl) SPR_FIXED(2048 - DG_Chanls[(chanl)].height / 2)
#else
//#define ChanlBaseX(chanl) SPR_FIXED(DG_Chanls[(chanl)].width / 2)
//#define ChanlBaseY(chanl) SPR_FIXED(DG_Chanls[(chanl)].height / 2)
#define ChanlBaseX(chanl) SPR_FIXED(0)
#define ChanlBaseY(chanl) SPR_FIXED(0)
#endif

#define ChanlX(x)  ((x) + chanl_x)
#define ChanlY(y)  ((y) + chanl_y)

#define PosX(x)  ((SPR_FIX)((base_scale * (x) * chanl_w) / 512.0F))
#define PosY(y)  ((SPR_FIX)((base_scale * (y) * chanl_h) / 384.0F))

static void check_node(Work * work, SPR_OBJ * prim, float base_scale,
		       int attrib, int chanl, SPR_FIX base_x, SPR_FIX base_y)
{
  SPR_OBJ * bros;
  int now_attrib;
  SPR_FIX now_x, now_y ;
  int	pri;
  SPR_FIX chanl_x, chanl_y;
  SPR_FIX chanl_w, chanl_h;
  SPR_FIX x[4], y[4];
  int now_chanl;
  u_long64 xyz, rgb;
  DG_TEX * tex = NULL;
  int alpsw, txsw;
  u_long64 prim_d, alpha, tex0, tex1, tex2, clamp, pabe, g;
  int priv;
  float scale;
  float	u_scale = 0.0f, v_scale = 0.0f;

  now_x = now_y = 0;
  tex1 = SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0);

  priv = DG_PrivilegeMode;
  priv = (!priv) ? SPR_FLAG_PRIV : 0;

  for(bros = prim; bros != NULL; bros = bros->head.next)
    {
      /*
       * 描画領域チャネルの継承
       */
      /* 継承されたチャネルが負の値である場合、
	 オブジェクトに設定してあるチャネルを使用する。 */
      if((now_chanl = chanl) < 0) now_chanl = bros->head.chanl;
      if(now_chanl < 0) continue;   /* 異常なチャネルであれば処理を行わない */
      chanl_x = ChanlBaseX(now_chanl);
      chanl_y = ChanlBaseY(now_chanl);

      chanl_w = SPR_FIXED(DG_Chanls[now_chanl].width);
      chanl_h = SPR_FIXED(DG_Chanls[now_chanl].height);

      /*
       * 属性のテストと継承
       */
      /* 親から継承された属性と、オブジェクト固有の属性から、
	 この場で使用され、なおかつ子に継承される属性を求める */
      now_attrib = (attrib & SPR_FLAG_HIDDEN) | bros->head.flags;

      /*
       * 親オブジェクトの基準点に、各頂点の座標を加算した、絶対座標値を求める
       */
      /* 基準点の絶対座標値を求め、それを子に継承する基準座標とする */
      if(!(now_attrib & SPR_FLAG_HIDDEN))
	{
	  pri = bros->head.pri & 7;
	  alpsw = (now_attrib & SPR_FLAG_ALPHA) ? 1 : 0;
	  txsw  = ((bros->head.id != SP_EMPTY) &&
		   (bros->head.id != SP_POINT) &&
		   (bros->head.id != SP_LINE) &&
		   (bros->head.id != SP_LINESTRIP) &&
		   (bros->head.id != SP_BOX) &&
         (bros->head.id != SP_BOX_F) &&
		   !(bros->head.flags & SPR_FLAG_NO_TEX)) ? 1 : 0;
	  g = ((bros->head.id == SP_LINE) || (bros->head.id == SP_LINESTRIP) ||
	       (bros->head.id == SP_POLY) || (bros->head.id == SP_TRIANGLE));
	  prim_d = SCE_GS_SET_PRIM(0, g, txsw, 0, alpsw, 0, 1, 0, 0);

	  alpha = (alpsw)
	    ? bros->head.alpha : SCE_GS_SET_ALPHA(2, 2, 2, 0, 128);
	  pabe = (alpha && txsw) ? bros->ex_hd.tex.pabe : 0;
	  if(txsw)
	    {
	      tex = bros->ex_hd.tex.dgtex;
	      if(NULL == tex) txsw = 0;
	      else
		{
		  clamp = tex->tex_trans.clamp.data;
		  tex0 = tex->tex_trans.tex0.data;
		  tex2 = tex->tex_trans.tex2.data;
		  /* PRINT_TEX0(tex0); */
		  /* テクスチャのテクセル座標ＵＶから正規化ＵＶ座標への変換パラメータを算出 */
		  u_scale = 1.0f / (float)(1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f)) ;
		  v_scale = 1.0f / (float)(1 << ((tex->tex_trans.tex0.data >> 30) & 0x0f)) ;

		}
	    }
	  
	  /* SP_EMPTY 以外、表示チャネルとして指定されているチャネルの
	     パケットに順次つなぐ */
	  if(bros->head.id != SP_EMPTY &&
	     ((bros->head.flags | priv) & SPR_FLAG_PRIV))
	    {
	      int tri;

	      tri = (txsw) ? bros->ex_hd.tex.tri : MAX_TRI;

	      /* リストに描画対象となるパケットを登録する */
	      regist_obj_packet(work,
				bros->head.pk[DG_Clock], now_chanl, pri, tri);
	    }

     // BP added then commented out
     // add this line if you want to zoom into the layout
     // work->last = DG_SetDmapackViewMapping(work->last, 0.05f*DRAW_WIDTH, 0.05f*DRAW_HEIGHT, 0.95f*DRAW_WIDTH, 0.95f*DRAW_HEIGHT);
     work->last = DG_SetDmapackViewMapping(work->last, 0.0f, 0.0f, DRAW_WIDTH, DRAW_HEIGHT);

	  /* アルファ禁止フラグをセット */
	  if ( bros->head.flags & SPR_FLAG_NOALPHATEST ) {
		  work->last = DG_SetDmapackModeEnable( work->last, DG_DMAPACK_MODE_NO_ALPHATEST );
	  }

	  switch (bros->head.id){
		case SP_EMPTY:
	      now_x = base_x + PosX(bros->empty.pos.x);
	      now_y = base_y + PosY(bros->empty.pos.y);
	      break;
	    case SP_POINT:
	      {
			  spr_DrawPoint * point = bros->head.packet;
			  now_x = base_x + PosX(bros->point.pos.x);
			  now_y = base_y + PosY(bros->point.pos.y);

			  work->last = DG_SetDmapackAlpha( work->last, alpha );
			  work->last = DG_SetDmapackPoint( work->last, ChanlX( now_x ), ChanlY( now_y ), DG_RGBATODMARGBA( bros->point.col ) );
	      }
	      break;

	    case SP_LINE:
	      {
			  SPR_FIX x, y;
			  spr_DrawLine * line = bros->head.packet;
		
			  now_x = base_x + PosX(bros->line.pos[0].x);
			  now_y = base_y + PosY(bros->line.pos[0].y);
			  x = base_x + PosX(bros->line.pos[1].x);
			  y = base_y + PosY(bros->line.pos[1].y);

			  work->last = DG_SetDmapackAlpha( work->last, alpha );
			  work->last = DG_SetDmapackLine_F( work->last,
											 ChanlX( now_x ), ChanlY( now_y ),
											 DG_RGBATODMARGBA( bros->line.col[0] ),
											 ChanlX( x ), ChanlY( y ),
											 DG_RGBATODMARGBA( bros->line.col[1] ) );
	      }
	      break;
	    case SP_LINESTRIP:
	      {
			  spr_DrawLS_template * lstr[2];
			  int i;
			  SPR_FIX x, y;

			  now_x = base_x + PosX(bros->linestrip.plist[0].pos.x);
			  now_y = base_y + PosY(bros->linestrip.plist[0].pos.y);

			  work->last = DG_SetDmapackTex( work->last, NULL );
			  work->last = DG_SetDmapackAlpha( work->last, alpha );
			  work->last = DG_SetDmapackLineStrip_F( work->last, bros->linestrip.pcnt );
			  for ( i = 0 ; i < bros->linestrip.pcnt ; i++ ){
				  x = base_x + PosX(bros->linestrip.plist[i].pos.x);
				  y = base_y + PosY(bros->linestrip.plist[i].pos.y);
				  work->last = DG_SetDmapackVertex_F( work->last,
												   ChanlX( x ), ChanlY( y ), 0.0f, 0.0f,
												   DG_RGBATODMARGBA( bros->linestrip.plist[i].col ) );
			  }
#if 0
		lstr[DG_Clock] = bros->head.pk[DG_Clock];
		now_x = base_x + PosX(bros->linestrip.plist[0].pos.x);
		now_y = base_y + PosY(bros->linestrip.plist[0].pos.y);

		/* 全頂点の処理 */
		for(i = 0; i < bros->linestrip.pcnt;i++)
		  {
		    x = base_x + PosX(bros->linestrip.plist[i].pos.x);
		    y = base_y + PosY(bros->linestrip.plist[i].pos.y);

		    xyz = SCE_GS_SET_XYZ(ChanlX(x), ChanlY(y), DEFAULT_Z);

		    rgb = SCE_GS_SET_RGBAQ(bros->linestrip.plist[i].col.r,
					   bros->linestrip.plist[i].col.g,
					   bros->linestrip.plist[i].col.b,
					   bros->linestrip.plist[i].col.a, 0);

		    lstr[DG_Clock]->pos[i].xyz = xyz;
		    lstr[DG_Clock]->pos[i].rgbq = rgb;
		  }

		/* ALPHA の設定 */
		lstr[DG_Clock]->head.gif.data.alpha.data = alpha;

		/* PRIM の設定 */
		prim_d |= (SCE_GS_PRIM_LINESTRIP & 7);
		lstr[DG_Clock]->head.gif.giftag.tag = 
		  SCE_GIF_SET_TAG(1, 0, 1, prim_d, 0, 1);
#endif
	      }
	      break;
	    case SP_BOX:
       case SP_BOX_F:
	      {
			  spr_DrawBox * box = bros->head.packet;
		
			  now_x = base_x + PosX(bros->box.rect.begin.x);
			  now_y = base_y + PosY(bros->box.rect.begin.y);

			  x[0] = ChanlX( now_x );
			  y[0] = ChanlY( now_y );
			  x[3] = ChanlX( base_x + PosX(bros->box.rect.end.x) );
			  y[3] = ChanlY( base_y + PosY(bros->box.rect.end.y) );
			  x[1] = x[3] ;
			  y[1] = y[0] ;
			  x[2] = x[0] ;
			  y[2] = y[3] ;
           
			  work->last = DG_SetDmapackAlpha( work->last, alpha );
#ifdef ORIG_PATH
			  work->last = DG_SetDmapackLine_F( work->last,
											 x[0], y[0], DG_RGBATODMARGBA( bros->box.col ),
											 x[1], y[1], DG_RGBATODMARGBA( bros->box.col ) );
#ifndef KP_WINDOWS
			  work->last = DG_SetDmapackLine_F( work->last,
											 x[1], y[1], DG_RGBATODMARGBA( bros->box.col ),
											 x[3], y[3], DG_RGBATODMARGBA( bros->box.col ) );
#else
			  work->last = DG_SetDmapackLine_F( work->last,
											 x[1], y[1],   DG_RGBATODMARGBA( bros->box.col ),
											 x[3], y[3]+1, DG_RGBATODMARGBA( bros->box.col ) );
#endif
			  work->last = DG_SetDmapackLine_F( work->last,
											 x[0], y[0], DG_RGBATODMARGBA( bros->box.col ),
											 x[2], y[2], DG_RGBATODMARGBA( bros->box.col ) );
			  work->last = DG_SetDmapackLine_F( work->last,
											 x[2], y[2], DG_RGBATODMARGBA( bros->box.col ),
											 x[3], y[3], DG_RGBATODMARGBA( bros->box.col ) );
#else
           work->last = DG_SetDmapackLineStrip_F( work->last, 5 );
           work->last = DG_SetDmapackVertex_F( work->last, x[0], y[0], 0.0f, 0.0f, DG_RGBATODMARGBA( bros->box.col ) ) ;
           work->last = DG_SetDmapackVertex_F( work->last, x[1], y[1], 0.0f, 0.0f, DG_RGBATODMARGBA( bros->box.col ) ) ;
           work->last = DG_SetDmapackVertex_F( work->last, x[3], y[3], 0.0f, 0.0f, DG_RGBATODMARGBA( bros->box.col ) ) ;
           work->last = DG_SetDmapackVertex_F( work->last, x[2], y[2], 0.0f, 0.0f, DG_RGBATODMARGBA( bros->box.col ) ) ;
           work->last = DG_SetDmapackVertex_F( work->last, x[0], y[0], 0.0f, 0.0f, DG_RGBATODMARGBA( bros->box.col ) ) ;
#endif

	      }
	      break;
	    case SP_SPRITE:
	      {
			  spr_DrawSprite * sprite = bros->head.packet;
		
			  now_x = base_x + PosX(bros->sprite.pos.x);
			  now_y = base_y + PosY(bros->sprite.pos.y);
		
			  x[0] = ChanlX(now_x);
			  y[0] = ChanlY(now_y);
			  x[1] = x[0] + PosX(bros->sprite.dw);
			  y[1] = y[0] + PosY(bros->sprite.dh);

			  if ( txsw ){
				  float u0, v0;
				  float u1, v1;
				  u0 = bros->ex_hd.tex.u * u_scale ;
				  v0 = bros->ex_hd.tex.v * v_scale ;
				  u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
				  v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

				  if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
				  if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
				  work->last = DG_SetDmapackTex( work->last, tex );
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackSprt( work->last,
												 x[0], y[0], u0, v0,
												 x[1], y[1], u1, v1,
												 DG_RGBATODMARGBA( bros->sprite.col ) );
			  } else {
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackBox( work->last,
												x[0], y[0],
												x[1], y[1],
												DG_RGBATODMARGBA( bros->sprite.col ) );
			  }
	      }
	      break;
	      
       case SP_SPRITE_F:
          {
             spr_DrawSprite * sprite = bros->head.packet;

             now_x = base_x + PosX(bros->sprite.pos.x);
             now_y = base_y + PosY(bros->sprite.pos.y);

             x[0] = ChanlX(now_x);
             y[0] = ChanlY(now_y);
             x[1] = x[0] + PosX(bros->sprite.dw);
             y[1] = y[0] + PosY(bros->sprite.dh);

             if ( txsw ){
                float u0, v0;
                float u1, v1;
                u0 = bros->ex_hd.tex.u * u_scale ;
                v0 = bros->ex_hd.tex.v * v_scale ;
                u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
                v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

                if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
                if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
                work->last = DG_SetDmapackTex( work->last, tex );
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackSprt_F( work->last,
                   x[0], y[0], u0, v0,
                   x[1], y[1], u1, v1,
                   DG_RGBATODMARGBA( bros->sprite.col ),
                   bros->head.flags );
             } else {
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackBox_F( work->last,
                   x[0], y[0],
                   x[1], y[1],
                   DG_RGBATODMARGBA( bros->sprite.col ) );
             }
          }
          break;

	    case SP_ZOOM:
	      {
			  spr_DrawZoom * zoom = bros->head.packet;
		
			  now_x = base_x + PosX(bros->zoom.rect.begin.x);
			  now_y = base_y + PosY(bros->zoom.rect.begin.y);
			  x[0] = ChanlX(now_x);
			  y[0] = ChanlY(now_y);
			  x[1] = ChanlX(base_x + PosX(bros->zoom.rect.end.x));
			  y[1] = ChanlY(base_y + PosY(bros->zoom.rect.end.y));

			  if ( txsw ){
				  float u0, v0;
				  float u1, v1;
				  u0 = bros->ex_hd.tex.u * u_scale ;
				  v0 = bros->ex_hd.tex.v * v_scale ;
				  u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
				  v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

				  if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
				  if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
				  work->last = DG_SetDmapackTex( work->last, tex );
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackSprt( work->last,
												 x[0], y[0], u0, v0,
												 x[1], y[1], u1, v1,
												DG_RGBATODMARGBA( bros->zoom.col ) );
			  } else {
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackBox( work->last,
												x[0], y[0],
												x[1], y[1],
												DG_RGBATODMARGBA( bros->zoom.col ) );
			  }
	      }
	      break;

       case SP_ZOOM_F:
          {
             spr_DrawZoom * zoom = bros->head.packet;

             now_x = base_x + PosX(bros->zoom.rect.begin.x);
             now_y = base_y + PosY(bros->zoom.rect.begin.y);
             x[0] = ChanlX(now_x);
             y[0] = ChanlY(now_y);
             x[1] = ChanlX(base_x + PosX(bros->zoom.rect.end.x));
             y[1] = ChanlY(base_y + PosY(bros->zoom.rect.end.y));

             if ( txsw ){
                float u0, v0;
                float u1, v1;
                u0 = bros->ex_hd.tex.u * u_scale ;
                v0 = bros->ex_hd.tex.v * v_scale ;
                u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
                v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

                if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
                if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
                work->last = DG_SetDmapackTex( work->last, tex );
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackSprt_F( work->last,
                   x[0], y[0], u0, v0,
                   x[1], y[1], u1, v1,
                   DG_RGBATODMARGBA( bros->zoom.col ),
                   bros->head.flags);
             } else {
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackBox_F( work->last,
                   x[0], y[0],
                   x[1], y[1],
                   DG_RGBATODMARGBA( bros->zoom.col ) );
             }
          }
          break;

	    case SP_SPIN:
	      {
			  spr_DrawSpin * spin = bros->head.packet;
			  float s_sin, s_cos, yscale, ooyscale;

			  now_x = base_x + PosX(bros->spin.pos.x);
			  now_y = base_y + PosY(bros->spin.pos.y);

			  /* 回転中心を原点とした場合の、各頂点をテンポラリに設定 */

           // BP widescreen FIX
           // convert from 4:3 to 16:9
           // then rotate
           // then convert back
           // TODO: investigate whether it should be 4:3 or 512:400 or 512:448 or...
           // eyeballing it, 4:3 gave the best results
           // --SteveM
           yscale = (4.0f/3.0f)/(16.0f/9.0f);
           ooyscale = 1.0f/yscale;

			  /* 論理座標で回転させる */
			  x[0] = -bros->spin.center.x;
			  y[0] = -bros->spin.center.y * yscale;

			  x[1] = x[0] + bros->spin.dw;
			  y[1] = y[0];

			  x[2] = x[1];
			  y[2] = y[0] + bros->spin.dh * yscale;

			  x[3] = x[0];
			  y[3] = y[2];

		
			  /* 回転処理 */
			  /* 回転に必要な sin, cos の値を得る */
			  s_sin = vu0_Sin(bros->spin.spin);
			  s_cos = vu0_Cos(bros->spin.spin);

			  {
				  float tx, ty;

				  tx = x[0] * s_cos - y[0] * s_sin;
				  ty = x[0] * s_sin + y[0] * s_cos;
				  x[0] = tx, y[0] = ty * ooyscale;
		
				  tx = x[1] * s_cos - y[1] * s_sin;
				  ty = x[1] * s_sin + y[1] * s_cos;
				  x[1] = tx, y[1] = ty * ooyscale;
		
				  tx = x[2] * s_cos - y[2] * s_sin;
				  ty = x[2] * s_sin + y[2] * s_cos;
				  x[2] = tx, y[2] = ty * ooyscale;
		
				  tx = x[3] * s_cos - y[3] * s_sin;
				  ty = x[3] * s_sin + y[3] * s_cos;
				  x[3] = tx, y[3] = ty * ooyscale;
			  }

			  /* 回転結果を、表示位置に平行移動し、物理座標に変換する */
			  x[0] = ChanlX(base_x + PosX(x[0] + bros->spin.pos.x));
			  y[0] = ChanlY(base_y + PosY(y[0] + bros->spin.pos.y));

			  x[1] = ChanlX(base_x + PosX(x[1] + bros->spin.pos.x));
			  y[1] = ChanlY(base_y + PosY(y[1] + bros->spin.pos.y));

			  x[2] = ChanlX(base_x + PosX(x[2] + bros->spin.pos.x));
			  y[2] = ChanlY(base_y + PosY(y[2] + bros->spin.pos.y));

			  x[3] = ChanlX(base_x + PosX(x[3] + bros->spin.pos.x));
			  y[3] = ChanlY(base_y + PosY(y[3] + bros->spin.pos.y));

			  if ( txsw ){
				  float u0, v0;
				  float u1, v1;
				  u0 = bros->ex_hd.tex.u * u_scale ;
				  v0 = bros->ex_hd.tex.v * v_scale ;
				  u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
				  v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

				  if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
				  if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);

				  work->last = DG_SetDmapackTex( work->last, tex );
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackRSprt( work->last,
												  x[0], y[0], x[1], y[1], 
												  x[3], y[3], x[2], y[2], 
												  u0, v0, u1, v1,
												  DG_RGBATODMARGBA( bros->spin.col ) );
			  } else {
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackQuad( work->last,
												 x[0], y[0], DG_RGBATODMARGBA( bros->spin.col ),
												 x[1], y[1], DG_RGBATODMARGBA( bros->spin.col ),
												 x[3], y[3], DG_RGBATODMARGBA( bros->spin.col ),
												 x[2], y[2], DG_RGBATODMARGBA( bros->spin.col ) );
			  }
	      }
	      break;
       case SP_SPIN_F:
          {
             spr_DrawSpin * spin = bros->head.packet;
             float s_sin, s_cos, yscale, ooyscale;

             now_x = base_x + PosX(bros->spin.pos.x);
             now_y = base_y + PosY(bros->spin.pos.y);

             /* 回転中心を原点とした場合の、各頂点をテンポラリに設定 */

             // BP widescreen FIX
             // convert from 4:3 to 16:9
             // then rotate
             // then convert back
             // TODO: investigate whether it should be 4:3 or 512:400 or 512:448 or...
             // eyeballing it, 4:3 gave the best results
             // --SteveM
             yscale = (4.0f/3.0f)/(16.0f/9.0f);
             ooyscale = 1.0f/yscale;

             /* 論理座標で回転させる */
             x[0] = -bros->spin.center.x;
             y[0] = -bros->spin.center.y * yscale;

             x[1] = x[0] + bros->spin.dw;
             y[1] = y[0];

             x[2] = x[1];
             y[2] = y[0] + bros->spin.dh * yscale;

             x[3] = x[0];
             y[3] = y[2];


             /* 回転処理 */
             /* 回転に必要な sin, cos の値を得る */
             s_sin = vu0_Sin(bros->spin.spin);
             s_cos = vu0_Cos(bros->spin.spin);

             {
                float tx, ty;

                tx = x[0] * s_cos - y[0] * s_sin;
                ty = x[0] * s_sin + y[0] * s_cos;
                x[0] = tx, y[0] = ty * ooyscale;

                tx = x[1] * s_cos - y[1] * s_sin;
                ty = x[1] * s_sin + y[1] * s_cos;
                x[1] = tx, y[1] = ty * ooyscale;

                tx = x[2] * s_cos - y[2] * s_sin;
                ty = x[2] * s_sin + y[2] * s_cos;
                x[2] = tx, y[2] = ty * ooyscale;

                tx = x[3] * s_cos - y[3] * s_sin;
                ty = x[3] * s_sin + y[3] * s_cos;
                x[3] = tx, y[3] = ty * ooyscale;
             }

             /* 回転結果を、表示位置に平行移動し、物理座標に変換する */
             x[0] = ChanlX(base_x + PosX(x[0] + bros->spin.pos.x));
             y[0] = ChanlY(base_y + PosY(y[0] + bros->spin.pos.y));

             x[1] = ChanlX(base_x + PosX(x[1] + bros->spin.pos.x));
             y[1] = ChanlY(base_y + PosY(y[1] + bros->spin.pos.y));

             x[2] = ChanlX(base_x + PosX(x[2] + bros->spin.pos.x));
             y[2] = ChanlY(base_y + PosY(y[2] + bros->spin.pos.y));

             x[3] = ChanlX(base_x + PosX(x[3] + bros->spin.pos.x));
             y[3] = ChanlY(base_y + PosY(y[3] + bros->spin.pos.y));

             if ( txsw ){
                float u0, v0;
                float u1, v1;
                u0 = bros->ex_hd.tex.u * u_scale ;
                v0 = bros->ex_hd.tex.v * v_scale ;
                u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
                v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

                if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
                if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);

                work->last = DG_SetDmapackTex( work->last, tex );
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackRSprt_F( work->last,
                   x[0], y[0], x[1], y[1], 
                   x[3], y[3], x[2], y[2], 
                   u0, v0, u1, v1,
                   DG_RGBATODMARGBA( bros->spin.col ) );
             } else {
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackQuad( work->last,
                   x[0], y[0], DG_RGBATODMARGBA( bros->spin.col ),
                   x[1], y[1], DG_RGBATODMARGBA( bros->spin.col ),
                   x[3], y[3], DG_RGBATODMARGBA( bros->spin.col ),
                   x[2], y[2], DG_RGBATODMARGBA( bros->spin.col ) );
             }
          }
          break;

       case SP_POLY:
	      {
			  spr_DrawPoly * poly = bros->head.packet;
		
			  now_x = base_x + PosX(bros->poly.vert[0].x);
			  now_y = base_y + PosY(bros->poly.vert[0].y);

			  x[0] = ChanlX(now_x);
			  y[0] = ChanlY(now_y);
		
			  x[1] = ChanlX(base_x + PosX(bros->poly.vert[1].x));
			  y[1] = ChanlY(base_y + PosY(bros->poly.vert[1].y));
		
			  x[2] = ChanlX(base_x + PosX(bros->poly.vert[2].x));
			  y[2] = ChanlY(base_y + PosY(bros->poly.vert[2].y));
		
			  x[3] = ChanlX(base_x + PosX(bros->poly.vert[3].x));
			  y[3] = ChanlY(base_y + PosY(bros->poly.vert[3].y));

			  if ( txsw ){
				  float u0, v0;
				  float u1, v1;
				  u0 = bros->ex_hd.tex.u * u_scale ;
				  v0 = bros->ex_hd.tex.v * v_scale ;
				  u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
				  v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

				  if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
				  if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
				  work->last = DG_SetDmapackTex( work->last, tex );
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  /* これだけ完全ではないので注意！（多分使われてないだろうけど・・・） */
				  work->last = DG_SetDmapackRSprt( work->last,
												  x[0], y[0], x[1], y[1],
												  x[3], y[3], x[2], y[2],
												  u0, v0, u1, v1,
												  DG_RGBATODMARGBA( bros->poly.col[0] ) );
			  } else {
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackQuad( work->last,
												 x[0], y[0], DG_RGBATODMARGBA( bros->poly.col[0] ),
												 x[1], y[1], DG_RGBATODMARGBA( bros->poly.col[1] ),
												 x[2], y[2], DG_RGBATODMARGBA( bros->poly.col[3] ),
												 x[3], y[3], DG_RGBATODMARGBA( bros->poly.col[2] ) );
			  }

	      }
	      break;
       case SP_POLY_F:
          {
             spr_DrawPoly * poly = bros->head.packet;

             now_x = base_x + PosX(bros->poly.vert[0].x);
             now_y = base_y + PosY(bros->poly.vert[0].y);

             x[0] = ChanlX(now_x);
             y[0] = ChanlY(now_y);

             x[1] = ChanlX(base_x + PosX(bros->poly.vert[1].x));
             y[1] = ChanlY(base_y + PosY(bros->poly.vert[1].y));

             x[2] = ChanlX(base_x + PosX(bros->poly.vert[2].x));
             y[2] = ChanlY(base_y + PosY(bros->poly.vert[2].y));

             x[3] = ChanlX(base_x + PosX(bros->poly.vert[3].x));
             y[3] = ChanlY(base_y + PosY(bros->poly.vert[3].y));

             if ( txsw ){
                float u0, v0;
                float u1, v1;
                u0 = bros->ex_hd.tex.u * u_scale ;
                v0 = bros->ex_hd.tex.v * v_scale ;
                u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
                v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

                if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
                if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
                work->last = DG_SetDmapackTex( work->last, tex );
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                /* これだけ完全ではないので注意！（多分使われてないだろうけど・・・） */
                work->last = DG_SetDmapackRSprt_F( work->last,
                   x[0], y[0], x[1], y[1],
                   x[3], y[3], x[2], y[2],
                   u0, v0, u1, v1,
                   DG_RGBATODMARGBA( bros->poly.col[0] ) );
             } else {
                work->last = DG_SetDmapackAlpha( work->last, alpha );
                work->last = DG_SetDmapackQuad_F( work->last,
                   x[0], y[0], DG_RGBATODMARGBA( bros->poly.col[0] ),
                   x[1], y[1], DG_RGBATODMARGBA( bros->poly.col[1] ),
                   x[2], y[2], DG_RGBATODMARGBA( bros->poly.col[3] ),
                   x[3], y[3], DG_RGBATODMARGBA( bros->poly.col[2] ) );
             }
          }
          break;
	    case SP_TRIANGLE:
	      {
			  spr_DrawTriangle * triangle = bros->head.packet;
		
			  now_x = base_x + PosX(bros->triangle.vert[0].x);
			  now_y = base_y + PosY(bros->triangle.vert[0].y);

			  x[0] = ChanlX(now_x);
			  y[0] = ChanlY(now_y);
		
			  x[1] = ChanlX(base_x + PosX(bros->triangle.vert[1].x));
			  y[1] = ChanlY(base_y + PosY(bros->triangle.vert[1].y));
		
			  x[2] = ChanlX(base_x + PosX(bros->triangle.vert[2].x));
			  y[2] = ChanlY(base_y + PosY(bros->triangle.vert[2].y));

			  if ( txsw ){
				  float u0, v0;
				  float u1, v1;
				  u0 = bros->ex_hd.tex.u * u_scale ;
				  v0 = bros->ex_hd.tex.v * v_scale ;
				  u1 = ( bros->ex_hd.tex.u + bros->ex_hd.tex.w ) * u_scale ;
				  v1 = ( bros->ex_hd.tex.v + bros->ex_hd.tex.h ) * v_scale ;

				  if(bros->head.flags & SPR_FLAG_H_REV) FSWAP(u0, u1);
				  if(bros->head.flags & SPR_FLAG_V_REV) FSWAP(v0, v1);
				  work->last = DG_SetDmapackTex( work->last, tex );
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  /* これだけ完全ではないので注意！（多分使われてないだろうけど・・・） */
				  work->last = DG_SetDmapackRSprt( work->last,
												  x[0], y[0], x[1], y[1],
												  x[2], y[2], x[2], y[2],
												  u0, v0, u1, v1,
												  DG_RGBATODMARGBA( bros->triangle.col[0] ) );
			  } else {
				  work->last = DG_SetDmapackAlpha( work->last, alpha );
				  work->last = DG_SetDmapackTriangle( work->last,
													 x[0], y[0], DG_RGBATODMARGBA( bros->triangle.col[0] ),
													 x[1], y[1], DG_RGBATODMARGBA( bros->triangle.col[1] ),
													 x[2], y[2], DG_RGBATODMARGBA( bros->triangle.col[2 /* BP - WAS 3, should be 2 */ ] ) );
           }
		  }
	      break;
	  }

	  /* アルファ禁止フラグをリセット */
	  if ( bros->head.flags & SPR_FLAG_NOALPHATEST ) {
		  work->last = DG_SetDmapackModeDisable( work->last, DG_DMAPACK_MODE_NO_ALPHATEST );
	  }

	  if(bros->head.id != SP_EMPTY &&
	     ((bros->head.flags | priv) & SPR_FLAG_PRIV)){
		  int tri;

	      tri = (txsw) ? bros->ex_hd.tex.tri : MAX_TRI;

		  work->packet_end[now_chanl][pri][tri] = work->last ;
		  work->last = DG_SetDmapackNext( work->last, work->end_packet );
	  }

	}
      /*
       * 子オブジェクトの処理
       */
      /* 子が無い場合は次の兄弟に移行 */
      if(bros->head.child == NULL) continue;
      scale = base_scale * bros->head.scale;
      check_node(work, bros->head.child, scale,
		 now_attrib, now_chanl, now_x, now_y); 
    }
}

/*
 * テクスチャ u, v 値と、幅、高さおよび属性の設定
 */
int SPR_ObjSetTexture(SPR_OBJ * prim, int code, int handle)
{
  DG_TEX * tex;
  SPR_FIX w, h;
  SPR_FIX u, v, tw, th;
  int width, height, iu, iv;

  if((prim->head.id == SP_EMPTY) ||
     (prim->head.id == SP_POINT) || 
     (prim->head.id == SP_LINE) ||
     (prim->head.id == SP_LINESTRIP) ||
     (prim->head.id == SP_BOX))
    return -1;

  if(NULL == sys_work) return -1;
  if(NULL == (tex = DG_GetTexture2(sys_work->tri_code[handle], code))) return -1;
  /* if(NULL == (tex = DG_GetTexture(code))) return -1; */

  /* テクスチャの幅と高さを求めておく */
  w = SPR_FIXED(1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f));
  h = SPR_FIXED(1 << ((tex->tex_trans.tex0.data >> 30) & 0x0f));

  /* uv値の計算 */
  DG_GetTexelInfo(&width, &height, &iu, &iv, tex);

  u = tex->u_offset * w;
  v = tex->v_offset * h;

    /* テクスチャの幅、高さ */
  tw = tex->u_scale * w;
  th = tex->v_scale * h;

  prim->ex_hd.tex.u = u ;
  prim->ex_hd.tex.v = v ;
  prim->ex_hd.tex.w = tw ;
  prim->ex_hd.tex.h = th ;

  prim->ex_hd.tex.pu = SPR_FIXED(iu);
  prim->ex_hd.tex.pv = SPR_FIXED(iv);
  prim->ex_hd.tex.pw = SPR_FIXED(width);
  prim->ex_hd.tex.ph = SPR_FIXED(height);

  prim->ex_hd.tex.tri = handle;
  prim->ex_hd.tex.dgtex = tex;
  prim->head.flags &= ~SPR_FLAG_NO_TEX;
  return 0;
}

/*
 * 現在登録されている確保関数を使用して、メモリを確保する
 */
void * spr_malloc(long64 size)
{
  Work * work = sys_work;
  if(NULL == work) return NULL;
  return (work->func_malloc)(size);
}

/*
 * 現在登録されている開放関数を使用して、メモリを開放する
 */
void spr_free(void * ptr)
{
  Work * work = sys_work;
  ASSERT(NULL != work);
  (work->func_free)(ptr);
}

/* 現在登録されている確保関数のポインタを得る */
void * (* spr_get_func_malloc(void))()
{
  Work * work = sys_work;
  if(NULL == work) return NULL;
  return work->func_malloc;
}

/* 現在登録されている開放関数のポインタを得る */
void (* spr_get_func_free(void))()
{
  Work * work = sys_work;
  if(NULL == work) return NULL;
  return work->func_free;
}

void * SPR_DefMalloc(long64 size)
{
  return GV_Malloc(size);
}

void SPR_DefFree(void * ptr)
{
  GV_DelayedFree(ptr);
}

int SPR_ResetMemoryManager(void)
{
  Work * work = sys_work;

  if(NULL == work) return -1;
  work->func_malloc = SPR_DefMalloc;
  work->func_free   = SPR_DefFree;
  return 0;
}

int SPR_SetMemoryManager(void * (*func_malloc)(long64 size),
			 void (*func_free)(void * ptr))
{
  Work * work = sys_work;
  if(NULL == work)
    {
      DBG("Could not setup driver.\n");
      return -1;
    }

  if(NULL != func_malloc) work->func_malloc = func_malloc;
  if(NULL != func_free)   work->func_free = func_free;

  return 0;
}

/*
 * 指定されたテクスチャリストを取得し、
 * その転送パケットをテクスチャ転送 DMAPACK に設定する。
 */
int SPR_LoadTexture(int code)
{
  DG_TEXTURE_LIST * tex_list;
  Work * work;
  int id;

  DBG("SPR_LoadTexture(%d)\n", code);
  if(NULL == sys_work)
    {
      DBG("Could not setup driver.\n");
      return -1;
    }
  work = sys_work;
  for(id = 0; id < MAX_TRI; id++)
    {
      if(work->tri_code[id] == code)
	{
	  work->tri_cnt[id]++;
	  return id; /* 既にあるならばそれを使う */
	}
      if(work->tex_list[id] == NULL) break;
    }

  if(id == MAX_TRI) return -1;

  if(NULL == (tex_list = DG_GetTextureList(code))) 
    {
      DBG("Could not found textures.\n");
      return -1;
    }

  DBG("Texture found.\n");
  work->tri_code[id] = code;
  work->tex_list[id] = tex_list;
  work->tri_cnt[id]++;
  DBG("texture: %d   handle = %d  count = %d\n", code, id, work->tri_cnt[id]);
  return id;
}

/*
 * 指定されたテクスチャハンドルのテクスチャ割り当てを開放する
 */
int SPR_KillTexture(int handle)
{
  Work * work;

  /* 2Dオブジェクトモジュールが動いていなければエラー */
  if(NULL == (work = sys_work)) return -1;

  /* 指定されたハンドルが上限値より大きい、もしくは負の値であればエラー */
  if((handle < 0) || (handle >= MAX_TRI)) return -1;

  /* 指定されたハンドルにテクスチャが割り当てられていなければエラー */
  if(!work->tri_cnt[handle]) return -1;

  if((--work->tri_cnt[handle]) == 0)
    {
      /* 登録カウンタが 0 になれば、完全に割当を開放する */
      work->tex_list[handle] = NULL;
      work->tri_code[handle] = -1;
      DBG("Kill texture handle %d\n", handle);
    }
  return 0;
}


static void Act(Work * work)
{
  int c, p, t;

#ifdef DEBUG_MODE
  work->trans_tex_size = 0;
#endif /* DEBUG_MODE */
  work->last = data_buffer ;
  work->end_packet = work->last ;
  work->last = DG_SetDmapackEnd( work->last );	/* 終端コードの書き込み */
  
#ifdef KP_WINDOWS	// Skip
	if( DG_CurrentFrameDrawSkip() ){ return ; }
#endif

   if (DG_Arm_SkipThisFrame())
   {
      return;
   }

  for(t = 0; t < MAX_TRI; t++)
    if(work->tex_list[t] != NULL)
      for(c = 0; c < MAX_CHANL; c++)
	for(p = 0; p < SPR_PRI_NUM; p++)
	  {
	    DG_WriteTextureChangePacks(&work->dma_tex[c][p][t][DG_Clock].call,
				       &work->tex_list[t]->tex_packet[DG_Clock]);
	    work->dma_tex[c][p][t][DG_Clock].nxt.qwc =
	      DMATAG_SET_QWC(DMATAG_ID_NEXT, 0);
	    work->dma_tex[c][p][t][DG_Clock].nxt.vifcode[0] =
	      SCE_VIF1_SET_NOP(0);
	    work->dma_tex[c][p][t][DG_Clock].nxt.vifcode[1] =
	      SCE_VIF1_SET_NOP(0);
	  }

  /* DMA パケットリストを初期化 */
  init_packet_list(work);

  /* オブジェクトリストを検索し、リストに登録する */
  check_node(work, work->root_object, 1.0F, 0, -1, 0, 0);

  /* 登録したオブジェクトリストの要所にテクスチャ転送パケットを挟む */
  create_packet_flow(work);


#ifdef DEBUG_MODE
  if(GV_PadDataDirect[0].press & PAD_AL)
    printf("2D Texture total trans size: %d (0x%08x)\n",
	   work->trans_tex_size, work->trans_tex_size);
#endif /* DEBUG_MODE */
}

/*
 * ドライバの終了処理
 */
static void Die(Work * work)
{
  SPR_OBJ * obj;
  int i;

  DBG("Sprite Driver is DIE!!\n");

  /* テクスチャ転送で始まる DMAPACK による転送を停止する */
  for(i = 0; i < MAX_CHANL; i++)
    {
      DG_DequeueDmapack(work->tex_dmapack[i]); /* 全チャネルに渡って停止 */
      DG_FreeDmapack(work->tex_dmapack[i]);    /* DMAPACK 構造体の開放   */
    }

  /* 登録されているオブジェクトを全て非参照に */
  while(NULL != (obj = work->root_object))
    SPR_Destroy_2D_Object(obj);
  DBG("finish.\n");

  if(sys_work == work) sys_work = NULL;

  GV_SetActorFreeFunc( work, GV_DelayedFree );
}

/* 1チャネル分の DMA 関係パラメタ初期化 */
static int init_dma_param(Work * work, int chanl)
{
  DG_DMAPACK * dmapack;
  int flg, i;

  DBG("init_dma_param(): chanl = %d\n", chanl);
  for(i = 0; i < 2; i++)
    {
      work->dmaroot[chanl][i].dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_NEXT, 2);
      work->dmaroot[chanl][i].dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
      work->dmaroot[chanl][i].dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(2, 0);
      work->dmaroot[chanl][i].dmatag.addr = &work->tex_recov[chanl][i];

      work->dmaroot[chanl][i].giftag.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1);
      work->dmaroot[chanl][i].giftag.regs = GS_REGS_AD;

      work->dmaroot[chanl][i].test.reg = SCE_GS_TEST_1;
      work->dmaroot[chanl][i].test.data =
	SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
    }

  flg = (chanl < 4) ? DG_DMAPACK_NORMAL : DG_DMAPACK_MENU;
  
  /* 2D オブジェクトは基本的に特権 DMAPACK として作成する */
  dmapack = DG_MakeDmapack2(flg | DG_DMAPACK_PRIVILEGE,
			    DG_DMAPACK_PHASE_AFTER, 144);
  
  if(NULL == dmapack) return -1;

  dmapack->flag &= ~DMA_DISABLE;
  dmapack->flag |= (DMA_DISABLE & disp_mask[chanl]);

  dmapack->packet[0] = &work->dmaroot[chanl][0];
  dmapack->packet[1] = &work->dmaroot[chanl][1];

  work->tex_dmapack[chanl] = dmapack;

  /* 描画用コールバックの設定 */
  work->callback_param[ chanl ].work = work ;
  work->callback_param[ chanl ].dmapack = dmapack ;
  work->callback_param[ chanl ].param0 = NULL ;
  work->callback_param[ chanl ].chanl = chanl ;
  DG_SetDmapackCallback( dmapack, DmapackCallback, &work->callback_param[ chanl ] );

  DG_QueueDmapack(dmapack);

  return 0;
}



static int GetResources(Work * work, int mode)
{
  int chanl;
  int id, i;

  /* メモリ確保/開放関数を初期化する。
     初期状態では、SPR_DefMalloc() / SPR_DelayedFree() が用いられる。 */
  work->func_malloc = SPR_DefMalloc;
  work->func_free   = SPR_DefFree;


  /* 登録済オブジェクトをクリアする */
  work->root_object = NULL;
  /* 初期状態においては予約なし */

  /* テクスチャが登録されていない状態にする */
  for(id = 0; id < MAX_TRI; id++)
    {
      work->tex_list[id] = NULL;
      work->tri_code[id] = -1;
      work->tri_cnt[id] = 0;
    }

  /* 描画環境リセットの DMA 転送部を設定する */
  for(chanl = 0; chanl < MAX_CHANL; chanl++)
    {
      for(i = 0; i < 2; i++)
	{
	  work->tex_recov[chanl][i].dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, 3);
	  work->tex_recov[chanl][i].dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
	  work->tex_recov[chanl][i].dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(3, 0);

	  work->tex_recov[chanl][i].giftag.tag = SCE_GIF_SET_TAG(2, 1, 0, 0, 0, 1);
	  work->tex_recov[chanl][i].giftag.regs = GS_REGS_AD;

	  work->tex_recov[chanl][i].test.reg = SCE_GS_TEST_1;
#ifdef PSX2
	  work->tex_recov[chanl][i].test.data = *(u_long64 *)&DG_Chanls[chanl].draw_env[i].datas.test1;
#endif

	  work->tex_recov[chanl][i].pabe.reg = SCE_GS_PABE;
	  work->tex_recov[chanl][i].pabe.data = SCE_GS_SET_PABE(0);
	}
      init_dma_param(work, chanl);
    }

  return 0;
}

void * SPR_Init2D_ObjectDriver(void)
{
  Work * work;
  int mode = 0;

  if(sys_work != NULL) return NULL;

  OPERATOR();
  //work = (Work *)GV_NewActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf1);
  work = (Work *)GV_CreateActor(GV_ACTOR_DAEMON2, GV_CLASS_SYSTEM, sizeof(Work), 0x00);
  if(NULL == work) return NULL;

  if(GetResources(work, mode))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  GV_SetActor(work, Act, Die);

  sys_work = work;
  return work;
}

/* 古いプログラムと互換性を保つための関数 */
void * New2D_ObjectDriver(void)
{
  return SPR_Init2D_ObjectDriver();
}



/* ---------------------------------------------------------------- */
/* DG_DMAPACKコールバック関数 */
static void DmapackCallback( void *addr )
{
	CallbackParam	*param = addr ;
	Work			*work ;
	int				i, j ;
	void	*before = NULL ;

	work = param->work ;

#if 0
	for ( i = 0 ; i < SPR_PRI_NUM ; i++ ){
		for ( j = 0 ; j < MAX_TRI+1 ; j++ ){
			DG_ExecAuto2DPrim( work->packet_begin[param->chanl][i][j] );
		}
	}
#else
	/* リストを全て１つにつなげる */
	for ( i = 0 ; i < SPR_PRI_NUM ; i++ ){
		for ( j = 0 ; j < MAX_TRI+1 ; j++ ){
			if ( before != NULL ){
				DG_SetDmapackNext( before, work->packet_begin[param->chanl][i][j] );
			}
			before = work->packet_end[param->chanl][i][j] ;
		}
	}
	DG_ExecAuto2DPrim( work->packet_begin[param->chanl][0][0] );

#endif

}

/* DG_DMAPACKレベルでの表示プライオリティ変更 */
/* ＸＢＯＸゲームオーバー画面での終了確認Ｌ２Ｄ表示用強制表示プライオリティ変更 */
void SPR_ForcePriorityChange( int chanl, int flag )
{
	if ( sys_work == NULL ) return ;

	if ( flag == 0 ){
		sys_work->tex_dmapack[ chanl ]->priority = 144 ;
		sys_work->tex_dmapack[ chanl ]->phase = DG_DMAPACK_PHASE_AFTER ;
	} else {
		sys_work->tex_dmapack[ chanl ]->priority = 255 ;
		sys_work->tex_dmapack[ chanl ]->phase = DG_DMAPACK_PHASE_LAST ;
	}
	DG_DequeueDmapack( sys_work->tex_dmapack[ chanl ] );
	DG_QueueDmapack( sys_work->tex_dmapack[ chanl ] );
}
