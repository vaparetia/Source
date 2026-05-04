//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  memcall.c
  メモリーコールモード関連の名前表示および選択
  2000/09/13

  $Id: memcall.c,v 1.1.1.3 2002/11/19 11:45:02 Yoshizawa1 Exp $
*/
#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "codec.h"

#include "libfs.h"
#include "libdg.h"
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"

#define _memcall_c_
#include "memcall.h"



#include "codecmem.h"
#include "sprite_2d.h"
#include "c_layout.h"
#include "codec_config.h"
#include "../../kira/2D_action/layout_2d.h"
#include "../../kira/2D_lib/util_2d.h"
#include <stdio.h>

#include "BP_BuildDefines.h"

#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Renderer.h"
#include "BP_Font.h"
#endif

#include "font.h"

#include "cjimaku.h"

#ifdef PSX2
#ifndef _DEBUG_
#define DBG(...)
#else
#define DBG(...)  printf(__VA_ARGS__)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif

#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)
#define DSIZ(_type)   ((sizeof(_type) + 7) / 8)

#define BLIGHT   128
#define DARK     32

#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)

#if 1 //BP_XBOX def KP_XBOX
/* XBOX描画タグ */
typedef struct memcall_MEMCALL_DRAW_SPRT {
#if BP_USE_NEW_FONT_SYSTEM()
   DG_DMAPACK_TEXTURE_DYNAMIC  tex;
#else
   DG_DMAPACK_TEX  tex;
#endif
   DG_DMAPACK_ALPHA alpha;
   DG_DMAPACK_SPRT sprt;
} MEMCALL_DRAW_SPRT;
#endif

struct trans_pack;

typedef struct memcall_MEM_NAME_PANEL {
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO  bp_vinfo;
   unsigned int      bp_tex;
#else
   FONT_VRAMINFO vinfo;
   void        * vram;
   int           vramsize;
   DG_TEX_LIN *tex;
   int clut[ 16 ];
#endif

   int           width, height;



   /* 表示位置とサイズ */
   int           panel_dx;
   int           panel_dy;

   int           panel_w;
   int           panel_h;

   MEMCALL_DRAW_SPRT *packet;
} MEM_NAME_PANEL;




/*
 * 名前表示エリアのラベル群(.l2d より取得)
 */

#if 0
static int name_labels[] = {
  MEM_name_0, MEM_name_1,
  MEM_name_2, MEM_name_3,
  MEM_name_4, MEM_name_5,
  MEM_name_6, MEM_name_7
};

/*
 * 周波数表示エリアのラベル群(.l2d より取得)
 */
static int freq_labels[] = {
  MEM_freq_0, MEM_freq_1,
  MEM_freq_2, MEM_freq_3,
  MEM_freq_4, MEM_freq_5,
  MEM_freq_6, MEM_freq_7
};
#else
static int name_labels[] = {
  MEM_name_0, MEM_name_4,
  MEM_name_1, MEM_name_5,
  MEM_name_2, MEM_name_6,
  MEM_name_3, MEM_name_7
};

/*
 * 周波数表示エリアのラベル群(.l2d より取得)
 */
static int freq_labels[] = {
  MEM_freq_0, MEM_freq_4,
  MEM_freq_1, MEM_freq_5,
  MEM_freq_2, MEM_freq_6,
  MEM_freq_3, MEM_freq_7
};
#endif



/*
 * 字幕転送用パケット
 */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG  dmatag0;
  DG_GIFTAG  giftag0;
  DG_GSREG   gsregs[4];
  DG_GIFTAG  giftag1;
  DG_DMATAG  dmatag1;
} tex_trans ;

/*
 * CLUT 転送用パケット
 */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG   dmatag0;   /* clut 転送関連パラメータを転送する */
  struct dma_packs {
    DG_GIFTAG   giftag0;

    struct gif_params { 
      DG_GSREG    bitbltbuf;
      DG_GSREG    trxpos;
      DG_GSREG    trxreg;
      DG_GSREG    trxdir;
    } params;

    DG_GIFTAG   giftag1;
  } packs;
  DG_DMATAG   dmatag1;   /* ref で clut を転送する      */
  DG_DMATAG   dmatag2;   /* next で描画パケットにつなぐ */
} clut_trans ;


typedef ALIGN16_DECL(struct) trans_pack {
  tex_trans     trans;      /* 字幕テクスチャ転送用 */

  DG_DMATAG     dmatag;

  ALIGN16_DECL(struct) gif_packets {
    DG_GIFTAG     giftag0;
    ALIGN16_DECL(struct) gif_packed {
      DG_GSREG      texflush;
      DG_GSREG      alpha;
    } packed ;

    DG_GIFTAG     giftag1;
    ALIGN16_DECL(struct) gif_reglist {
      u_long64 clamp;
      u_long64 tex0;
      u_long64 prim;
      u_long64 rgbq;
    
      u_long64 uv0;
      u_long64 xyz0;
    
      u_long64 uv1;
      u_long64 xyz1;
    } reglist ;
  } gifs ;
} trans_pack ;


typedef ALIGN16_DECL(struct) {

  SPR_OBJ        * name_area;/* 名前表示領域矩形をあらわすSP_BOX    */
  SPR_OBJ        * freq_area;/* 周波数表示領域矩形をあらわすSP_BOX */

  SPR_OBJ        * freq_base; /* 周波数表示の親オブジェクト */

  MEM_NAME_PANEL   panel;

} list_parson ;


typedef struct {
  GV_ACT_EX     actor;

  int           l2d_handle;  /* 無線レイアウトが稼働しているハンドル */

  SPR_OBJ     * all_parent;  /* 周波数表示全ての親 */



  DG_DMAPACK  * dmapack;

  /*
   * 名前表示領域と,周波数表示領域
   */
  list_parson   parson[8];

  /*
   * CLUT 転送部パケット
   */
  ALIGN16_PRE unsigned int    ALIGN16_POST clut_buf[64];  /* CLUT マスタ */
  
  
  /* 選択関連 */
  int           selected;
#if 1 //BP_XBOX def KP_XBOX
  /* DMAPACKのパケット本体 */
  int packet[ ( sizeof( MEMCALL_DRAW_SPRT ) * 8
	       + sizeof( DG_DMAPACK_TAG ) ) / sizeof( int ) ];
#endif
  
} Work;

static Work * now_work = NULL;

/* 表示エリアと記録エリアのリンクをとる */
static int    disp_link[8];
static int    item_link[8];


/* -------------------------------------------------------------------------
 * 対外関数
 * ----------------------------------------------------------------------- */
/*
 * 人名の文字列ポインタを取得
 */
char * MemCallGetRes(int area)
{
  if((area < 0) || (area > 7)) return NULL;
  return GM_GetResource(4, CODEC_registed_list[area].name_res);
}

/*
 * 登録されている項目数を得る
 */
int MemCallCount(void)
{
  int cnt = 0;
  int i;

  for(i = 0; i < 8; i++)
    if(CODEC_registed_list[i].registed) cnt++;

  return cnt;
}


/*
 * 指定エリアに設定された人物の周波数を得る
 */
int MemCallGetFreq(int area)
{
  int idx = disp_link[area];
  if((idx < 0) || (idx > 7)) return -1;

  printf("get frequency area %d = %d\n", idx, CODEC_registed_list[idx].freq);

  return CODEC_registed_list[idx].freq;
}

/*
 * 指定エリアに,人名と周波数を登録する
 * (指定したエリアが、必ず表示エリアと対応するとは限らない)
 */
int MemCallSetName(int area, int freq, int name_id)
{
  char * name;

  if((area < 0) || (area > 7)) return -1;

#ifdef DEBUG_MODE
//  printf("Area %d : freq = %d\n", area, freq);
//  name = GM_GetResource(4, name_id);
//  printf("regist memcall: %s\n", name);
#endif
  CODEC_registed_list[area].freq = freq;
  CODEC_registed_list[area].name_res = name_id;
  CODEC_registed_list[area].registed = 1;
  return 0;
}

/*
 * 選択項目を指定したエリアにする
 */
int MemCallSelect(int area)
{
  Work * work = now_work;
  int idx = disp_link[area];

  if((area < 0) || (area > 7)) return -1;
  if(idx < 0) return -1;
  if(0 > CODEC_registed_list[idx].name_res) return -1;
  work->selected = area;
  return 0;
}

#if !BP_USE_NEW_FONT_SYSTEM()
static void send_draw_pattern( MEM_NAME_PANEL *work )
{
	/* work->vramに展開された4ビットフォントを16ビットテクスチャとして展開する */
	int x, y;
	unsigned char *s = ( unsigned char * )work->vram;
	unsigned short *p = ( unsigned short * )work->tex->image;

	for( y = 0; y < work->height; y++ ){
		for( x = 0; x < work->width; x += 2 ){
		    p[ 0 ] = ( *s & 0xF ) ? 0xE318 : 0;
		    p[ 1 ] = ( *s >> 4 ) ? 0xE318 : 0;
		    p += 2;
		    s ++;
		}
	}
}
#endif

static void Act(Work * work)
{
  int i;
  int alpha;

  for(i = 0; i < 8; i++)
    {
      alpha = (work->selected == i) ? BLIGHT : DARK;
#if 1 //BP_XBOX def KP_XBOX
      work->parson[i].panel.packet->sprt.rgba
	  = ( unsigned int )SCE_GS_SET_RGBAQ(alpha, alpha, alpha, 0x80, 0);
#endif
    }
}

static void Die(Work * work)
{
   int i;

   DG_DequeueDmapack(work->dmapack);
   DG_FreeDmapack(work->dmapack);

   for(i = 0; i < 8; i++)
   {
#if BP_USE_NEW_FONT_SYSTEM()
      BP_FreeDynamicTexture(work->parson[i].panel.bp_tex);
#else
      codecDelayedFree(work->parson[i].panel.vram);
      DG_FreeLinerTexture( work->parson[i].panel.tex );
#endif
      if(CODEC_registed_list[i].registed)
         SPR_Destroy_2D_Object(work->parson[i].freq_base);
   }

   font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop() );

   if(work == now_work) now_work = NULL;

#ifdef DEBUG_MODE
   {
      long64 siz = codecMemGetSize();
      printf("memlist.c: Die(): used = %ld\n", siz);
   }
#endif /* DEBUG_MODE */
}

/*
 * 名前表示パネルの字幕転送パケットを準備する
 */
static void setup_panel_packet(Work * work, MEM_NAME_PANEL * panel)
{
#if 1 //BP_XBOX def KP_XBOX
  // パネル描画パケットの設定
  MEMCALL_DRAW_SPRT *p;
  float x, y, w, h;
  int rgba;

  p = panel->packet;

  x = ( float )panel->panel_dx;
  y = ( float )panel->panel_dy;
  w = ( float )panel->panel_w;
  h = ( float )panel->panel_h;

  rgba = SCE_GS_SET_RGBAQ(DARK, DARK, DARK, DARK, 0);

#if BP_USE_NEW_FONT_SYSTEM()
  DG_SetDmapackTextureDynamic01( &p->tex, NULL, panel->bp_tex );
#else
  DG_SetDmapackTexLin( &p->tex, panel->tex );
#endif
  DG_SetDmapackAlpha( &p->alpha, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
  DG_SetDmapackSprt( &p->sprt, x, y, 0.0F, 0.0F, x + w, y + h, 1.0F, 1.0F, DG_MakeDmaPackColorFromInt(rgba) );
#endif
}


/*
 * 名前表示用字幕構造体の中身を整える
 */
static void setup_name_panel(list_parson * parson)
{
  MEM_NAME_PANEL * panel = &(parson->panel);

  /* 領域を表示する */
#ifdef DEBUG_MODE
  // ここの注釈を外すと,名前表示領域ガイドが表示されるようになる。
  // parson->name_area->box.col.a = 128;
#endif

  panel->panel_dx = parson->name_area->box.rect.begin.x;
  panel->panel_dy = parson->name_area->box.rect.begin.y;


  panel->panel_w = parson->name_area->box.rect.end.x - panel->panel_dx;
  panel->panel_h = parson->name_area->box.rect.end.y - panel->panel_dy;

  /* 物理座標に変換 */
  panel->panel_dx = panel->panel_dx * DRAW_WIDTH  / SPR_SCRN_WIDTH;
  panel->panel_dy = panel->panel_dy * DRAW_HEIGHT / SPR_SCRN_HEIGHT;

  /* 物理描画サイズに変換 */
  panel->panel_w = panel->panel_w * DRAW_WIDTH  / SPR_SCRN_WIDTH;
  panel->panel_h = panel->panel_h * DRAW_HEIGHT / SPR_SCRN_HEIGHT;

  /* 640x480 サイズ換算に変換し、これをテクスチャパネルの大きさとする */
  panel->width  = panel->panel_w * 640 / SPR_SCRN_WIDTH + 48; /* もう1文字 */
  panel->height = panel->panel_h * 480 / SPR_SCRN_HEIGHT;

  printf("<a>panel->height = %d\n", panel->height);
  // panel->height = (panel->height + 48) / 48 * 48;
  printf("<b>panel->height = %d\n", panel->height);

  panel->width += panel->width & 1;
  panel->height += panel->height & 1;
  
  // panel->panel_h = panel->height * DRAW_HEIGHT / 480;
#if 1 //BP_XBOX def KP_XBOX
  // XBOXは64のアライメント
  panel->width = ( ( panel->width + 63 ) / 64 ) * 64;
#endif
  printf("++++++++++++++ W:%d  H:%d\n", panel->width, panel->height);

#if BP_USE_NEW_FONT_SYSTEM()
  panel->bp_tex = BP_AllocDynamicTexture(BP_FONT_CONVERT_ORIGINAL_X(panel->width), BP_FONT_CONVERT_ORIGINAL_Y(panel->height), 1);
  BP_font_set_vraminfo_texture(&panel->bp_vinfo, panel->bp_tex, 0, BP_FONT_CONVERT_ORIGINAL_Y(8), FONT_NO_KINSOKU);
#else

  panel->vramsize = panel->width * panel->height * 4 / 8;
  printf("vramsize = %d\n", panel->vramsize);
  panel->vram = codecMalloc(panel->vramsize);

  /* 内容を消去 */
  memset(panel->vram, 0x00, panel->vramsize);
  printf("vramsize = %d\n", panel->vramsize);
  
  /* vram 情報の設定 */
  font_set_vraminfo(&(panel->vinfo), panel->vram,
		    panel->width, panel->height, 0, 8, FONT_NO_KINSOKU );

  panel->tex = DG_MakeLinerTexture( panel->width, panel->height, DG_TEXLIN_FORMAT_A1R5G5B5 );
  memset( panel->tex->image, 0, panel->width * panel->height * sizeof( short ) );
#endif
}


/*
 * 人名表示リストの用意
 */
static void setup_list(Work * work, list_parson * parson)
{
  /* 表示パネルの準備 */
  setup_name_panel(parson);

  /* 表示パネル転送用パケットの作成 */
  setup_panel_packet(work, &(parson->panel));


}

static void all_show(void * workp, SPR_OBJ * obj)
{
  SPR_SHOW(obj);
}

static void setup_freq_panel(Work * work, list_parson * parson, int area)
{
  static int freq_name[] = {
    MEM_num0, MEM_num1, MEM_num2, MEM_num3, MEM_num4,
    MEM_num5, MEM_num6, MEM_num7, MEM_num8, MEM_num9
  };
  void * parts;
  SPR_OBJ * f_base;
  int num;
  int freq;
  int idx;

  /* 登録されていない箇所は非表示 */
  if(!CODEC_registed_list[area].registed) return;
  freq = CODEC_registed_list[area].freq;
  idx = item_link[area];

  printf("-<a> area = %d\n", area);
  parts = L2D_GetParts(work->l2d_handle, MEM_f1_00);
  if(NULL != parts)
    {
      num = (freq / 100) % 10;
      L2D_MorfObject(parts, freq_name[num], freq_name[num], 1.0F);
    }

  printf("-<b>\n");
  parts = L2D_GetParts(work->l2d_handle, MEM_f0_10);
  if(NULL != parts)
    {
      num = (freq / 10) % 10;
      L2D_MorfObject(parts, freq_name[num], freq_name[num], 1.0F);
    }

  printf("-<c>\n");
  parts = L2D_GetParts(work->l2d_handle, MEM_f0_01);
  if(NULL != parts)
    {
      num = freq % 10;
      L2D_MorfObject(parts, freq_name[num], freq_name[num], 1.0F);
    }

  printf("-<d>\n");
  f_base = L2D_GetObject(work->l2d_handle, MEM_f_base);
  if(NULL == f_base) return ;

  printf("-<e>\n");

  parson->freq_base = SPR_DuplicateTree(f_base);

  printf("-<f>\n");
  {
    SPR_OBJ * mark;

    /* 位置合わせ */
    mark = L2D_GetObject(work->l2d_handle, freq_labels[idx]);
    ASSERT(mark != NULL);
    SPR_SetPosEmpty(parson->freq_base, &(mark->box.rect.begin));
  }
  printf("-<g>\n");
  U2D_TreeProc(work, parson->freq_base, all_show);
}


/*
 * clut 転送パケットの内容を設定する
 */
static void setup_clut_trans(Work * work)
{
}


/*
 * 人名表示パケットの連結
 * 全ての人名表示を DG_DMAPACK の一つの転送単位として扱う。
 */
static void link_packet(Work * work)
{
#if 1 //BP_XBOX def KP_XBOX
  // XBOXではタグリストは連続しているので、終端を設定するだけ
  DG_SetDmapackEnd( ( void * )( ( MEMCALL_DRAW_SPRT * )work->packet + 8 ) );
#endif
}

static int GetResources(Work * work, int handle, int selected)
{
  int i;
  int idx;

  if(NULL != now_work) return -1;
  now_work = work;

  work->l2d_handle = handle;
  work->selected   = selected;

  /* 周波数表示全ての親を作成 */
  work->all_parent = SPR_Create_2D_Object(SP_EMPTY, 4, NULL);
  ASSERT(NULL != work->all_parent);

#if 1 //BP_XBOX def KP_XBOX
  {
	  SPR_POS p = { 0.0F, 0.0F };
	  SPR_SetPosEmpty(work->all_parent, &p);
  }
#endif
  SPR_SHOW(work->all_parent);

  /* フォント環境の初期化 */
  font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop());

  /* clut 内容の作成 */
  font_set_clut4( work->clut_buf, 0, 
		  FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) );

  work->clut_buf[0] &= 0x00ffffff;  /* 透明部分はアルファ 0 */


  /* 登録済のものだけを検索し,詰めて表示するための表を作成する*/
  idx = 0;
  for(i = 0; i < 8; i++)
    {
      disp_link[i] = -1;
      item_link[i] = -1;
    }
  for(i = 0; i < 8; i++)
    if(CODEC_registed_list[i].registed)
      {
	disp_link[idx] = i;  /* 正引きインデックス(表示位置からアイテム) */
	item_link[i] = idx;  /* 逆引きインデックス(アイテムから表示位置) */
	idx++;
      }

  /*
   * エリア確定用オブジェクトのポインタを得る
   */
  for(i = 0; i < 8; i++)
    {
      /*
       * 表示領域指標となるオブジェクトのポインタを得る
       */
      work->parson[i].name_area = codecGetObject(name_labels[i]);
      work->parson[i].freq_area = codecGetObject(freq_labels[i]);
#if 1 //BP_XBOX def KP_XBOX
      // パケットバッファはこちらが用意したもの
      work->parson[ i ].panel.packet = ( MEMCALL_DRAW_SPRT * )work->packet + i;
#endif

      setup_list(work, &(work->parson[i]));
      setup_freq_panel(work, &(work->parson[i]), i);

      if(i == work->selected)
	{
#if 1 //BP_XBOX def KP_XBOX
	  work->parson[i].panel.packet->sprt.rgba = 
	    ( unsigned int )SCE_GS_SET_RGBAQ(BLIGHT, BLIGHT, BLIGHT, BLIGHT, 0);
#endif

	}
    }

  /* clut 転送パケットを作成する */
  setup_clut_trans(work);
  /* パケットを繋ぐ(全ての描画パケットは clut 転送パケットの後に続く) */
  link_packet(work);
  /* DMAPACK に登録する */
  {
    DG_DMAPACK * dmapack;

    dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
			     DG_DMAPACK_PHASE_LAST);

    

#if 1 //BP_XBOX def KP_XBOX
    dmapack->autopacket = work->packet;
#endif
    work->dmapack = dmapack;
  }

  /*
   * 用意した各字幕エリアに,人名を描く
   */
  {
     char * name;

#if BP_USE_NEW_FONT_SYSTEM()
     BP_FONT_DRAWINFO dr;
#else
     FONT_DRAWINFO dr;
     font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop() );
#endif

     for(i = 0; i < 8; i++)
     {
       if(!CODEC_registed_list[i].registed)
          continue;

	   /*
	    * 人名の描画
	    */
	   idx = item_link[i];

      name = BP_GCL_LOOKUP_NEW_FONT_STRING(MemCallGetRes(i));

#if BP_USE_NEW_FONT_SYSTEM()
      BP_font_begin_render_texture(&work->parson[idx].panel.bp_vinfo, 1);
      BP_font_open_drawinfo(&dr, &work->parson[idx].panel.bp_vinfo);
      BP_font_set_locate(&dr, 0, BP_FONT_CONVERT_ORIGINAL_Y(RUBI_HEIGHT));
      BP_font_set_color(&dr, 0xc0, 0xc0, 0xc0, 128);
      BP_font_draw_string(&dr, name);

      BP_font_end_render_texture(&work->parson[idx].panel.bp_vinfo);

#else
      font_open_drawinfo(&dr, &work->parson[idx].panel.vinfo);
	   font_set_locate(&dr, 0, RUBI_HEIGHT);
	   // font_set_locate(&dr, 0, 0);
	   font_draw_string(&dr, name);

   	send_draw_pattern( &work->parson[ idx ].panel );
#endif 
    }
  }

  DG_QueueDmapack(work->dmapack);  /* 転送開始 */

  return 0;
}

void * NewMemCallList(int l2d_handle, int selected)
{
  Work * work;

  if(NULL == (work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, l2d_handle, selected))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
