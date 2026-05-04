//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node.c
  ノード(端末)画面／表示部
  2000/12/12      Y.Kira

  $Id: node.c,v 1.3 2002/12/20 07:58:25 takaki Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gameheader.h"

#include "g_define.h"
#include "libfs.h"
#include "dmapack.h"
#include "def_dma.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#include "../../kira/2D_action/layout_data.h"
/* #include "layout_2d.h" */
#include "stream.h"

#define _node_c_
#include "node_config.h"
#include "node.h"
#include "node_map.h"
#include "../codec/cdc_load.h" /* 車輪の再発明を防ぐ: 無線のロード機構を流用 */
#include "../codec/codecmem.h"
#include "../codec/codec.h"
#include "node_bg.h"

#include "BP_BuildDefines.h"

#include "BP_Misc.h"

#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Renderer.h"
#include "BP_Font.h"
#endif

#include "BP_LocalizedTextByEnum.h"

extern void ShowExitGameWarning(int *pResult);

#include "font.h"

extern int sd_set_cli ( int );
extern void *GetLocalResource( int ref_id, int offset );

#ifdef PSX2
#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif  /* DEBUG */
#else
#define DBG printf
#endif

#ifdef PSX2
#ifdef NO_TEXT
#define LOCATE(x, y, mode)
#define PRINTF(...)
#else
#define LOCATE(x, y, mode)   MENU_Locate(x, y, mode)
#define PRINTF(...)      MENU_Printf(__VA_ARGS__)
#endif
#else
#define LOCATE(x, y, mode)
#define PRINTF
#endif

// BP_ENGINE - Switching to windows mode to defer deletion 1 frame
#ifndef KP_WINDOWS
#define KP_WINDOWS
#endif

// #define NO_MENU_INFO


#define NODE_FDTIME             100    /* フェードアウトに要する時間 */
#define NODE_DOWNLOAD_TIMEOUT   300    /* ダウンロード終了後,
					  この時間ボタンを押さなかったら
					  勝手に進行する */


#define ITEM_CHGTIME   (5 * 10)    /* メニュー項目が切り替わる際の所要時間 */

#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)
#define DSIZ(_type)   ((sizeof(_type) + 7) / 8)

#define BLIGHT   128
#define DARK     32

#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
//#define TEX_BASE	(TEXTURE_TOP_PAGE()/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)

#define FONT_WIDTH  24
#define FONT_HEIGHT 24

#define FONT_LSKIP 4
#define INFO_TEX_WIDTH  (FONT_WIDTH * 30)
#define INFO_TEX_HEIGHT FONT_BUFFER_HEIGHT( 2, FONT_LSKIP )   /* 最大 2 行分 */
#define INFO_DISP_WIDTH (16 * 30)
#define INFO_DISP_HEIGHT  40

#define TEX_WIDTH	((INFO_TEX_WIDTH + 63)/64)

#define INFO_DISP_X  60
#define INFO_DISP_Y  340

typedef struct node_NODE_PANEL
{
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO  bp_vinfo;
   unsigned int      bp_tex;
#else
  FONT_VRAMINFO   vinfo;
  void          * vram;           /* 描画用仮想ビットマップエリア     */
  int             vramsize;       /* 描画領域のデータサイズ   */
  DG_TEX_LIN *tex;
#endif

  int             width, height;  /* 描画領域のテクセルサイズ */

  /* 表示位置, サイズ */
  /* 表示位置 */
  int             panel_dx;
  int             panel_dy;

  /* 表示サイズ */
  int             panel_w;
  int             panel_h;

  void  *packet_mem ;

} NODE_PANEL;



typedef struct node_work {
  GV_ACT_EX      actor;

  void (*act_func)(struct _work *);  /* その時点で有効な Act 関数のポインタ */
  /* システム関連の値 */
  char      old_chanl_flg[ 5 ];       /* 画面の表示状態を保存するエリア      */
  GV_PAD  * pad;                      /* パッドステータス構造体へのポインタ  */

  /* 起動時のフラグ */
  int       flags;


  /* 画面レイアウトについて */
  int       skin_name;                /* ノードのスキンとなるレイアウト名    */
  int       frame_name;               /* 枠レイアウトの名前                  */
  int       handle;                   /* スキンレイアウトの管理ハンドル      */

  int       frame_handle;             /* 枠アニメーションの管理ハンドル      */
  int       frame_mode;		      /* 枠アニメーションの状態管理          */

  /* パッドステータスに関するパラメータ */
  int       rep_cnt;                  /* オートリピート用カウンタ            */
  int       pre_stat;                 /* 直前のフレームのステータス          */
  int       rep_stat;                 /* オートリピートを効かせたステータス  */

  /* 進行、状態に関連するパラメータ */
  int       step;                     /* 各状態の進行をあらわす値            */
  int       mcur_main;                /* メインメニューカーソル位置          */
  int       mcur_main_screen;
  int       mcur_opt;                 /* オプション画面カーソル位置          */
  int       wait_cnt;                 /* フレーム待ちカウンタ                */
  int       selected;                 /* 選択された際に押されていたボタン    */

  /* 全体マップ関連 */
  int       load_cnt;                 /* ロードカウンタ                      */
  int       pre_cnt;                  /* 直前のパーセンテージ                */
  int       change_cnt;               /* パーセンテージ切り替わり中のカウンタ*/
  int       load_wait;                /* 演出上、プログレスバーの成長までの
					ウェイト */

  int       map_name;                 /* 全体マップレイアウト名称            */
  void    * map_act;                  /* 全体マップ表示 Act のポインタ       */
  int       proc_id;                  /* 全体マップロード済処理を行うproc_id */

  /* オプション関連 */
  int       opt_tmp;                  /* オプション内容変更用テンポラリ      */

  /* 画面切替え関連 */
  int       node_file_ID;             /* ロードするファイルの ID             */
  int       fade_cnt;                 /* フェードアウトカウンタ              */
  SPR_OBJ * fade_mask;                /* フェードアウトの際に使用するマスク  */

  int      timeout_cnt;               /* 操作タイムアウト用カウンタ          */


  /*
   * 操作に伴う補間関連
   */
  int       pre_item;                 /* 直前の選択項目     */
  int       moveitem_cnt;             /* 項目間移動カウンタ */
  int       itemchg_cnt;              /* 項目変更カウンタ   */

  /*
   * レイアウト連動用変数
   */
  int      next_evoke;                /* 現在のアクション終了後に実行する、
					 フレームレイアウトのアクション名 */

  /* ダウンロード中の処理に関連する変数 */
  int      download_secnt;

  /* 説明字幕用の部品 */
#ifndef NO_MENU_PANEL
  NODE_PANEL   panel;
  DG_DMAPACK * dmapack;   /* 字幕転送用 DMAPACK */
  int panel_lines;
  int info_disp:1;
#endif

  /*
   * 以下のフラグを毎フレーム監視し、非0 であればノードを起動する
   */
  int accept:1;  /* 0: アクセス要求無し  /  1: アクセス要求あり */
  int download:1;/* 0: ダウンロード不要  /  1: 要ダウンロード   */

  int name_entry:1; /* 0: ネームエントリ不要 / 1: ネームエントリ起動 */
  int name_entry_end:1; /* 1: で、ネームエントリ用キャラ終了 */
  int name_entry_only:1 ;

  /* オートリピート用のフラグ */
  int pad_blank:1; /* 0:パッド内容を反映 / 1:そのフレームのパッド内容は無効 */

  /* 背景がまだ生きているかどうか */
  int bg_dead:1;   /* 0: まだ生きている / 1: 死にに入る */

  int cancelUsed:1;

#ifdef KP_XBOX
  /* オプション画面とのやりとり変数 */
  int x_flag;
#endif

} Node_c_Work;

#define Work Node_c_Work

Work * node_c_now_work = NULL;  /* 現在動作中のノードプロセス */

#define now_work node_c_now_work

#if !BP_USE_NEW_FONT_SYSTEM()
static ALIGN16_PRE int ALIGN16_POST clut_buf[16];   /* 説明字幕用 CLUT */
#endif


/*
 * メインメニューでのカーソル制御アクション指定配列
 */
static struct 
{
  int           cur_pos;
  int           cap_num;   /* 説明字幕リソース */
  int           menu_item;
  int           menu_sw;
  int           menu_colon;  /* コロン */
  int           blight;
  int           dark;
  int           num;
  int           status[4];

  int           disp;

  /*
    int           sw_true;
    int           sw_false;
  */
  u_long64 opt_flag;
  u_long64 opt_flag2;

} menu_list[] = {

  { POS_curVIB, 0,
    OBJ_optVIB,   OBJ_swVIB, OBJ_colonVIB,     /* 振動切替え  */
    KEY_selVIB,   KEY_unVIB,
    2, { SW_vibOFF,    SW_vibON}, 0,
    GM_CONFIG_VIBRATION_OFF, 0 },

  { POS_curRAD, 1,
    OBJ_optRAD,   OBJ_swRAD, OBJ_colonRAD,     /* レーダー    */
    KEY_selRAD,   KEY_unRAD,
    3, {SW_radOFF1, SW_radTYPE1, SW_radTYPE2 }, 1,
    GM_CONFIG_RADAR_OFF, GM_CONFIG_RADAR_OFF_INTRUDE },


  { POS_curBLD, 2,
    OBJ_optBLD,   OBJ_swBLD, OBJ_colonBLD,     /* 出血        */
    KEY_selBLD,   KEY_unBLD,
    2, { SW_bldOFF,    SW_bldON}, 1,
    GM_CONFIG_BLOOD_OFF, 0 },

  { POS_curSND, 3,
    OBJ_optSND,   OBJ_swSND, OBJ_colonSND,     /* サウンド切替え */
    KEY_selSND,   KEY_unSND,
    2, {SW_sndMONO,   SW_sndSTEREO}, 1,
    GM_CONFIG_CUTSCENES_LETTERBOXED, 0 },

  { POS_cur5_1CH, 4,
    OBJ_opt5_1CH, OBJ_sw5_1CH, OBJ_colon5_1CH, /* 部分 5.1ch 対応 */
    KEY_sel5_1CH, KEY_un5_1CH,
    2, {SW_5_1chON,  SW_5_1chOFF}, 1,
    GM_CONFIG_SOUND_5_1CHANL, 0 },

  { POS_curCAP, 5,
    OBJ_optCAP,   OBJ_swCAP, OBJ_colonCAP,     /* 字幕 ON/OFF */
    KEY_selCAP,   KEY_unCAP,
    2, {SW_capOFF,    SW_capON}, 1,
    GM_CONFIG_CAPTION_OFF, 0 },

  { POS_curOWN, 6,
    OBJ_optOWN,   OBJ_swOWN, OBJ_colonOWN,     /* 主観上下挙動 */
    KEY_selOWN,   KEY_unOWN,
    2, {SW_ownREV,   SW_ownNORM}, 1,
    GM_CONFIG_SHUKAN_REVERSE, 0 },

  { POS_curITEM, 7,
    OBJ_optITEM,   OBJ_swITEM, OBJ_colonITEM,  /* アイテムウィンドウ */
    KEY_selITEM,   KEY_unITEM,
    2, {SW_itemLINEAR, SW_itemGROUP}, 1,
    GM_CONFIG_OLD_TYPE_MENU, 0 },

  { POS_curQUICK, 8,
    OBJ_optQUICK, OBJ_swQUICK, OBJ_colonQUICK, /* クィックチェンジ */
    KEY_selQUICK, KEY_unQUICK,
    2, {SW_quickUNEQUIP, SW_quickPREV}, 1,
    GM_CONFIG_MENU_QCHANGE_EX, 0 },

  { POS_curSCRN, 9,
    OBJ_optSCRN,  -1, -1,                /* スクリーン位置調整 */
    KEY_selSCRN, KEY_unSCRN,
    0, {0}, 1,
    -1, 0 },

  { POS_curCOL, 10, 
    OBJ_optCOL,   -1, -1,                /* カラー調整 */
    KEY_selCOL, KEY_unCOL,
    0, {0}, 0, 
    -1, 0 },

  { POS_curEXIT, 11,
    OBJ_optEXIT,  -1, -1,                /* 脱出 */
    KEY_selEXIT, KEY_unEXIT,
    0, {0}, 1, -1, 0 },

  { -1, -1, -1, -1, -1, -1, -1, -1, {0},  -1, 0 },
};

static const int menu_cursor_remap[]=
{
   //0,    //radar        //0,    //vibration
   1,    //blood        //1,    //radar
   2,    //caption      //2,    //blood
   5,    //own view     //3,    //sound
   6,    //item view    //4,    //5.1
   8,    //capt //5,    //caption
   9,    //own        //6,    //own view
   11,    //item    //7,    //item window
};


static const int MaxNumOptions = 7;


static struct 
{
  int res_option;       /* 「オプションの説明」リソース名       */
  int res_radar;        /* 「レーダーの説明」リソース名         */
  int res_qchange;      /* 「クイックチェンジの説明」リソース名 */
  int line_ownview;     /* 主観操作説明文行数                   */
  int line_qchange[2];   /* クイックチェンジ説明文行数           */
} pal_info_res[] = {
  /* 英語 */
  { 0x00a7d31a  /* "オプションの説明" */,
    0x0001ee1c  /* "レーダーの説明" */,
    0x00191c61  /* "クイックチェンジの説明" */,
    1, { 1, 2 } },
 
  /* フランス語 */
  { 0x009f2c05  /* "オプションの説明フランス語" */,
    0x0007980b  /* "レーダーの説明フランス語" */,
    0x0064511f  /* "クイックチェンジの説明フランス語" */,
    1, { 1, 1 } },

  /* ドイツ語 */
  { 0x00b5ab53  /* "オプションの説明ドイツ語" */,
    0x00b7056e  /* "レーダーの説明ドイツ語" */,
    0x00fc1c9c  /* "クイックチェンジの説明ドイツ語" */,
    1, { 1, 2 } },

  /* イタリア語 */
  { 0x00ddc584  /* "オプションの説明イタリア語" */,
    0x0046318a  /* "レーダーの説明イタリア語" */,
    0x00a2ea9e  /* "クイックチェンジの説明イタリア語" */,
    1, { 2, 2 } },

  /* スペイン語 */
  { 0x00941040  /* "オプションの説明スペイン語" */,
    0x00fc7c45  /* "レーダーの説明スペイン語" */,
    0x0059355a  /* "クイックチェンジの説明スペイン語" */,
    2, { 1, 2 } },
  /* 韓国語 */
  { 0,
	0,
	0,
	0, { 0, 0 } },
  /* 日本語 */
  { 0x001dd032  /* "オプションの説明日本語" */,
    0x00a49088  /* "レーダーの説明日本語" */,
    0x007021ce  /* "クイックチェンジの説明日本語" */,
    1, { 1, 1 } }, 

};


/* ローカル関数プロトタイプ */
static void NodeAccessWait(Work * work);
static void NodeAccessReset(Work * work);
static void NodeMain(Work * work);

static void _BP_ReArrangeNodeOptions(Work *work);

extern l2dStatus* BP_GetStatus(void * parts, int code);

#ifndef NO_MENU_PANEL

#if !BP_USE_NEW_FONT_SYSTEM()
static void  set_vram_to_image( NODE_PANEL *panel )
{
	int x, y ;
	unsigned char *s ;
	unsigned short *p ;

	/* DG_TEXLIN_FORMAT_A1R5G5B5用にCLUTを変更されたものを使う set_16bit_clutで変更すること　*/
	/* work->vramに展開された4ビットフォントを16ビットテクスチャとして展開する */
	s = ( unsigned char * )panel->vram;
	p = ( unsigned short * )panel->tex->image;
	for( y = 0; y < panel->height; y++ ){
		for( x = 0; x < panel->width; x += 2 ){
			p[ 0 ] = clut_buf[ *s & 0xF ];
			p[ 1 ] = clut_buf[ *s >> 4 ];
			p += 2;
			s ++;
		}
	}
   
   DG_LinerTextureSetImageDirty(panel->tex);
}

void set_16bit_clut( void *clut_buf )
{
	// CLUTを16ビット形式に変換
      int i;
	typedef union {
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		};
		unsigned int value;
	} CLUT;
      
	CLUT *clut = ( CLUT * )clut_buf ;
	for( i = 0; i < 16; i++ ) {
		unsigned int v;

		v = ( ( ( clut->r >> 3 ) << 10 )
			 | ( ( clut->g >> 3 ) << 5 )
			 | ( ( clut->b ) >> 3 ) << 0 ) | 0x8000;
		clut->value = v;
		clut ++;
	}
	( ( CLUT * )clut_buf )[ 0 ].value = 0;
}

#endif


/*
 * 名前表示パネルの字幕転送パケットを準備する
 */
static void setup_panel_packet(Work * work, NODE_PANEL * panel)
{
   void *prim ;
   float x0, y0, x1, y1;

   x0 = panel->panel_dx;
   y0 = panel->panel_dy;
   x1 = x0 + panel->panel_w;
   y1 = y0 + panel->panel_h;

   prim = panel->packet_mem = codecMalloc( 64 ) ;

   if ( prim == NULL ) 
      return ;

#if BP_USE_NEW_FONT_SYSTEM()
   prim = DG_SetDmapackTextureDynamic01(prim, NULL, panel->bp_tex);
#else
   panel->tex = DG_MakeLinerTexture( panel->width, panel->height, DG_TEXLIN_FORMAT_A1R5G5B5 ) ;
   
   if ( panel->tex == NULL ) 
      return ;
   
   set_vram_to_image( panel ) ;
   
   prim = DG_SetDmapackTexLin( prim, panel->tex );
#endif

   prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 128) ) ;
   prim = DG_SetDmapackSprt( prim, x0, y0, 0.0f, 0.0f, x1, y1, 1.0f, 1.0f, DG_MakeDmaPackColorFromInt(SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0)) ) ;
   prim = DG_SetDmapackEnd( prim ) ;

   printf("Need %d packet phototerm/photo_disp.c!!\n" , (int)prim - (int)panel->packet_mem );
}

void info_size_adjust(Work * work)
{
   NODE_PANEL * panel = &(work->panel);
   u_long64 xyz0, xyz1;
   float x0, y0, x1, y1;
   void		*prim ;

   switch(work->panel_lines)
   {
   case 1:    /* 一行のままの場合 */
      x0 = panel->panel_dx;
      y0 = panel->panel_dy - panel->panel_h / 8;
      x1 = x0 + panel->panel_w;
      y1 = y0 + panel->panel_h;

      prim = panel->packet_mem ;
#if BP_USE_NEW_FONT_SYSTEM()
      prim = DG_SetDmapackTextureDynamic01( prim, NULL, panel->bp_tex );
#else
      prim = DG_SetDmapackTexLin( prim, panel->tex );
#endif
      prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 128) ) ;
      prim = DG_SetDmapackSprt( prim,
         x0, y0, 0.0f, 0.0f,
         x1, y1, 1.0f, 1.0f,
         DG_MakeDmaPackColorFromInt(SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0)) ) ;
      prim = DG_SetDmapackEnd( prim ) ;

      break;

   case 2:    /* 二行表示の場合 */

      x0 = panel->panel_dx ;
      y0 = panel->panel_dy - panel->panel_h / 8;
      x1 = x0 + panel->panel_w;
      y1 = y0 + panel->panel_h * 9 / 10;

      prim = panel->packet_mem ;
#if BP_USE_NEW_FONT_SYSTEM()
      prim = DG_SetDmapackTextureDynamic01( prim, NULL, panel->bp_tex );
#else
      prim = DG_SetDmapackTexLin( prim, panel->tex ) ;
#endif
      prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 128) ) ;
      prim = DG_SetDmapackSprt( prim,
         x0, y0, 0.0f, 0.0f,
         x1, y1, 1.0f, 1.0f,
         DG_MakeDmaPackColorFromInt(SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0)) ) ;
      prim = DG_SetDmapackEnd( prim ) ;

      break;
   }
}



/*
 * 説明字幕用パネルを生成する
 */
static void setup_info_panel(Work * work, int x, int y, int width, int height, int pw, int ph)
{
  NODE_PANEL * panel = &(work->panel);
  DG_DMAPACK * dmapack;

  /* ビットマップの幅, 高さ */
  panel->width  = pw;
  panel->height = ph;

  /* ビットマップの描画位置 */
  panel->panel_dx = x;
  panel->panel_dy = y * DRAW_HEIGHT / 384; 

  /* 描画上の幅,高さ */
  panel->panel_w = width * DRAW_WIDTH / 512;
  panel->panel_h = height * DRAW_HEIGHT / 384;

#if BP_USE_NEW_FONT_SYSTEM()
  panel->bp_tex = BP_AllocDynamicTexture(BP_FONT_CONVERT_ORIGINAL_X(panel->width), BP_FONT_CONVERT_ORIGINAL_Y(panel->height), 1);
  BP_font_set_vraminfo_texture(&panel->bp_vinfo, panel->bp_tex, 0, BP_FONT_CONVERT_ORIGINAL_Y(FONT_LSKIP), FONT_NO_KINSOKU);
#else
  font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop() );

  panel->width = (panel->width/64+1)*64 ;

  /* 描画字幕ビットマップ領域を確保 */
  panel->vramsize = panel->width * panel->height * 4 / 8;
  panel->vram = codecMalloc(panel->vramsize);
  memset(panel->vram, 0x00, panel->vramsize);

  /* vram 情報の設定 */
  font_set_vraminfo(&(panel->vinfo), panel->vram,
		    panel->width, panel->height, 0, FONT_LSKIP, FONT_NO_KINSOKU);

  /* 描画 CLUT を生成 */
  font_set_clut4( clut_buf, 0, FONT_RGB(200, 200, 200), FONT_RGB(0, 0, 0));
  clut_buf[0] &= 0x00ffffff;  /* 透明部分はアルファ 0 */
  set_16bit_clut( clut_buf ) ; /* DG_TEXLIN_FORMAT_A1R5G5B5用にCLUTを変更 */
#endif

  /* 転送パケットを生成 */
  setup_panel_packet(work, panel);

  /* 描画パケットを DMAPACK に登録する */
  dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
			   DG_DMAPACK_PHASE_LAST);

  dmapack->autopacket = panel->packet_mem ;

  work->dmapack = dmapack;

  DG_QueueDmapack(dmapack);
}

/*
 * 字幕部分を破棄する
 */
static void destroy_info_panel(Work * work)
{
	if ( work->dmapack )
   {
		DG_DequeueDmapack(work->dmapack);
		DG_FreeDmapack(work->dmapack);
	}

#if BP_USE_NEW_FONT_SYSTEM()
   if(work->panel.bp_tex)
      BP_FreeDynamicTexture(work->panel.bp_tex);
#else
	/*フォントテクスチャーの開放*/
	if ( work->panel.tex ){
		DG_FreeLinerTexture( work->panel.tex ) ;
	}
   if ( work->panel.vram ) 
      codecDelayedFree(work->panel.vram);
#endif

   /*DMAパケットの開放*/
   if ( work->panel.packet_mem ) {
      codecDelayedFree(work->panel.packet_mem);
   }
}

#endif

/*
 * メニューレイアウトのシグナルハンドル
 */
static void MenuSignalHandle(void * workp, int signal, int value)
{
  Work * work = workp;

  DBG("menu signal: 0x%06x\n", signal);
  /* メニュレイアウトは、受け取ったシグナル名と同じアクション名を
     枠レイアウトに実行させる。 */
  work->next_evoke = signal;
}


/* フレームへの指令 */
#define FRAME_SIGNAL_UP		0x00000b10		/* Up */
#define FRAME_SIGNAL_DOWN	0x0023cb4e		/* Down */
#define FRAME_SIGNAL_LEFT	0x0027a134		/* Left */
#define FRAME_SIGNAL_RIGHT	0x00562979		/* Right */
#define FRAME_SIGNAL_RIGHTUP	0x00a5f068		/* RightUp */
#define FRAME_SIGNAL_LEFTDOWN	0x00664561		/* LeftDown */



static void FrameSignalHandle(void * workp, int signal, int value)
{
  Work * work = workp;

  DBG("frame signal: 0x%06x\n", signal);

  /* 全ての子に対し,シグナルを発行する */
  switch(signal)
    {
    case FRAME_SIGNAL_UP:
      GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_UPPER, value);
      work->frame_mode=1;
      break;
    case FRAME_SIGNAL_DOWN:
      GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_LOWER, value);
      work->frame_mode=1;
      break;
    case FRAME_SIGNAL_LEFT:
      GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_LEFT, value);
      work->frame_mode=1;
      break;
    case FRAME_SIGNAL_RIGHT:
      GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_RIGHT, value);
      work->frame_mode=1;
      break;
    case FRAME_SIGNAL_RIGHTUP:
      GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_RIGHTUP, value);
      work->frame_mode=1;
      break;
    case FRAME_SIGNAL_LEFTDOWN:
      GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_LEFTDOWN, value);
      work->frame_mode=1;
      break;
	default:
	  ASSERT( 0 );
    }
}


static void BGMsg_Stop(Work *work)
{
  if(!work->frame_mode) return;

  GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_STOP, 0);

  work->frame_mode=0;
}


/*
 * パッドのステータスを、オートリピートが効いているものとして扱う
 */
static int repeat_status(Work * work)
{
  GV_PAD * pad;
  int ret;
  pad = work->pad;

  if(!pad->status)
    {
      work->pre_stat = 0;
      work->rep_cnt = 0;
      return 0;
    }
  
  if(work->rep_cnt < REP_MAX)
    {
      if(pad->status == work->pre_stat)
	{
	  work->pre_stat = pad->status;
	  work->rep_cnt++;
	  ret = 0;
	}
      else
	{
	  /* 直前のフレームとステータスが異なっている場合、
	   * 異なるステータスを持つものだけを残し、*/
	  work->rep_cnt = 0;
	  ret = (pad->status ^ work->pre_stat) & pad->status;
	}
    }
  else
    {
      if(!work->pad_blank)
	{
	  ret = pad->status;
	  work->pad_blank = 1;
	}
      else
	{
	  ret = 0;
	  work->pad_blank = 0;
	}
    }
    
  work->pre_stat = pad->status;
  return ret;
}


static void ActSwitch(Work * work, void (*func)(Work *))
{
  work->act_func = func;
  work->step = 0;
}



/*
 * ダウンロードのパーセンテージ表示のためのオブジェクト状態更新を行う
 */
static void setup_parcent(Work * work, int p)
{
  static int code[] = {
    KEY_num0, KEY_num1, KEY_num2, KEY_num3, KEY_num4,
    KEY_num5, KEY_num6, KEY_num7, KEY_num8, KEY_num9
  };
  int num;
  void * prg_num001; /* 1の位   */
  void * prg_num010; /* 10の位  */
  void * prg_num100; /* 100の位 */
  SPR_OBJ * obj_num001;
  SPR_OBJ * obj_num010;
  SPR_OBJ * obj_num100;

  prg_num001 = L2D_GetParts(work->frame_handle, SCRN_prg_num001);
  prg_num010 = L2D_GetParts(work->frame_handle, SCRN_prg_num010);
  prg_num100 = L2D_GetParts(work->frame_handle, SCRN_prg_num100);

  if(NULL == prg_num001) return;
  if(NULL == prg_num010) return;
  if(NULL == prg_num100) return;

  obj_num001 = L2D_GetObject(work->frame_handle, SCRN_prg_num001);
  obj_num010 = L2D_GetObject(work->frame_handle, SCRN_prg_num010);
  obj_num100 = L2D_GetObject(work->frame_handle, SCRN_prg_num100);

  /* 1の位 */
  num = p % 10;
  L2D_MorfObject(prg_num001, code[num], code[num], 1.0F);
  
  /* 10の位 */
  num = (p / 10) % 10;
  if(!num) SPR_HIDE(obj_num010);
  L2D_MorfObject(prg_num010, code[num], code[num], 1.0F);
  
  /* 100の位 */
  num = (p / 100) % 10;
  if(!num) SPR_HIDE(obj_num100);
  L2D_MorfObject(prg_num100, code[num], code[num], 1.0F);
}

static int start_fadein(Work * work)
{
  int alpha;

  if(NULL == work->fade_mask) return 1;
  work->fade_cnt += TIME_BASE;
  if(work->fade_cnt >= NODE_FDTIME)
    {
      SPR_Destroy_2D_Object(work->fade_mask);
      work->fade_mask = NULL;
      return 1;
    }
  alpha = 128 - 128 * work->fade_cnt / NODE_FDTIME;
  if(alpha < 0) alpha = 0;
  work->fade_mask->sprite.col.a = alpha;
  return 0;
}

/*
 * 全体マップダウンロード時のメイン処理
 */
static void NodeMapDownload(Work * work)
{
  enum {
    STEP_INIT,         /* 初期化(ダイアログ表示など)         */
    STEP_FDIN,         /* 背景対策のため、フェードインで入る */
    STEP_OPENING,      /* ダウンロード画面を開いている       */
    STEP_DOWNLOADING,  /* ロード中演出(プログレスバーなど)   */
    STEP_COMPLETE,     /* 完了表示                           */
    STEP_WAIT,         /* ボタン入力待ち                     */
    STEP_EXIT          /* ダウンロード画面終了               */
  };

  /*
   * 枠もしくはメニューが初期化中であれば待つ
   */
  if( work->handle >= 0 ){
	  /* PS2 */
	  /* node_menu.l2d,node_frame.l2dの両方を使用 */
	  if((L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) ||
		 (L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle))) {

		  printf( "node.c : L2D_STAT_BUSY!! handle%d frame_handle%d\n",
				 L2D_STAT_BUSY == L2D_ActionStatus(work->handle),
				 L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle) ) ;
		  return;
	  }
  } else {
	  /* XBOX */
	  /* node_frame.l2dだけを使用 */
	  if( L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle) ){
		  printf( "node.c : L2D_STAT_BUSY!! frame_handle%d\n",L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle));
		  return;
	  }
  }

  switch(work->step)
    {
    case STEP_INIT:     /* 初期化                   */

      work->step = STEP_FDIN;
      work->load_cnt = 0;

      break;

    case STEP_FDIN:
      if(!start_fadein(work)) break;
      work->step = STEP_OPENING;
      /* ダウンロードのアクションはフレームレイアウト中に存在する */
      L2D_EvokeAction(work->frame_handle, SCRN_openDown);
      GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_WINOPN01);

      printf("openDownload--------------\n");
      break;
    case STEP_OPENING:      /* ダウンロード画面を開いている */
      DBG("STEP_OPENING\n");

      if(L2D_STAT_BUSY != L2D_ActionStatus(work->frame_handle))
	{
	  /* 開き終ったら、完全な状態をセットする */
	  L2D_EvokeAction(work->frame_handle, SCRN_setMapDownload);
	  work->step = STEP_DOWNLOADING;
	  work->load_wait = 20;
	  work->download_secnt = 0;
	  work->pre_cnt = 0;
	  work->change_cnt = 0;
	}
      break;
      
    case STEP_DOWNLOADING:  /* ダウンロード中演出       */
      DBG("STEP_DOWNLOADING\n");
      {
	int p;
	void * parts;
   SPR_OBJ *pTempSprite,*pTempSprite2;
   float progressRightEdge; 

   // BP JG - set frame to correct sizes
   pTempSprite = L2D_GetObject(work->frame_handle,0x0f31);
   if ( pTempSprite ) pTempSprite->sprite.dw = 326;
   pTempSprite = L2D_GetObject(work->frame_handle,0xf53);
   if ( pTempSprite ) pTempSprite->sprite.dh = 75;

	/*
	 * プログレスバーパーツの状態遷移をプログラムから制御する。
	 */
	if(work->load_wait > 0)
	  {
	    work->load_wait -= TIME_BASE;
	    break;
	  }

	work->download_secnt += TIME_BASE;
	if(work->download_secnt >= 30)
	  {
	    work->download_secnt -= 30;
	    GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_N_LOAD01);
	  }
	
	p = work->load_cnt +
	  (work->load_cnt - work->pre_cnt) * work->change_cnt / 60;

	if(p > 100)
	  {
	    p = 100;
	    work->step = STEP_COMPLETE;
	  }

	/* ここにダウンロード中のレイアウト制御処理をいれる。 */
	parts = L2D_GetParts(work->frame_handle, SCRN_progressEdge);
	ASSERT(NULL != parts);
	if(NULL != parts)
	  L2D_MorfObject(parts, KEY_progress0, KEY_progress100,(float)p / 100.0F);

	parts = L2D_GetParts(work->frame_handle, SCRN_progress);
	ASSERT(NULL != parts);
	if(NULL != parts)
	  L2D_MorfObject(parts, KEY_progress0, KEY_progress100,(float)p / 100.0F);


   progressRightEdge = 407;
   pTempSprite = L2D_GetObject(work->frame_handle,0x796a09);
   if ( pTempSprite ) pTempSprite->sprite.pos.x = progressRightEdge;

   //BP JG - correct progress bar to the scope it needs to be.
   pTempSprite = L2D_GetObject(work->frame_handle,SCRN_progress);
   pTempSprite2 = L2D_GetObject(work->frame_handle,SCRN_progressEdge);
   if ( pTempSprite2 )
      pTempSprite2->sprite.dh = 8;
   if ( pTempSprite && pTempSprite2 )
   {
      pTempSprite2->sprite.pos.x = pTempSprite->sprite.pos.x+pTempSprite->sprite.dw;
      if ( pTempSprite2->sprite.pos.x > progressRightEdge ) pTempSprite2->sprite.pos.x=progressRightEdge;
      if ( pTempSprite->sprite.pos.x+pTempSprite->sprite.dw > progressRightEdge ) pTempSprite->sprite.dw=progressRightEdge-pTempSprite->sprite.pos.x;
   }

	setup_parcent(work, p);

	work->change_cnt += TIME_BASE;
	if(work->change_cnt <= 60) break;

	/*
	 * パーセンテージに合わせ、数字を設定する
	 */
	// setup_parcent(work, p);

	work->pre_cnt = p; /* work->load_cnt; */
	work->change_cnt = 0;
	work->load_cnt = p + (BP_PS2_rand() >> 16) % 41 + 8;
	// work->load_cnt = p + (BP_PS2_rand() >> 16) % 11 + 8;
	work->load_wait = 20;
      }
      break;
    case STEP_COMPLETE:     /* ダウンロード完了         */
      DBG("STEP_COMPLETE\n");
      /*
       * ここでダウンロード完了の表示を行う
       */
      L2D_EvokeAction(work->frame_handle, SCRN_setMapComplete);
      GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_N_COMP01);  /* 完了 SE */
      
      work->step = STEP_WAIT;
      work->timeout_cnt = 0;
      break;

    case STEP_WAIT:         /* ボタン入力待ち           */
      work->timeout_cnt += TIME_BASE;
      if((work->pad->press & (PAD_OK | PAD_CANCEL)) ||
	 (work->timeout_cnt >= NODE_DOWNLOAD_TIMEOUT))
	{
	  L2D_EvokeAction(work->frame_handle, SCRN_closeDown);
	  GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_WINCLS01);  /* 完了 SE */
	  work->step = STEP_EXIT;
	}
      break;

    case STEP_EXIT:         /* ダウンロード画面終了処理 */ 
      DBG("STEP_EXIT\n");
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle))
	break;
      ActSwitch(work, NodeMain);
      break;
    }
}


/*
 * メインメニュー画面を作成
 */
static void node_create_main_menu(Work * work)
{
  int i;
  void * parts;
  void * colon;
  SPR_OBJ * obj;

  //return;
  /*
   * メニューの各項目の明度を設定する
   */
  /* 一旦全暗転 */
  for(i = 0; menu_list[i].menu_item >= 0; i++)
    {
      parts = L2D_GetParts(work->handle, menu_list[i].menu_item);
      colon = L2D_GetParts(work->handle, menu_list[i].menu_colon);
      obj   = L2D_GetObject(work->handle, menu_list[i].menu_item);
      ASSERT(parts != NULL);
      if(NULL != parts)
	{
	  int stat1, stat2, stat;
	  float rate;

	  rate = 1.0F;
	  stat1 = menu_list[i].dark;
	  stat2 = menu_list[i].dark;
	  if(i == work->mcur_main)
	    {
	      if(!work->moveitem_cnt)
		{
		  stat1 = menu_list[i].blight;
		  stat2 = menu_list[i].blight;
		  rate = 1.0F;
		}
	      else 
		{
		  stat1 = menu_list[i].dark;
		  stat2 = menu_list[i].blight;
		  rate = (float)work->moveitem_cnt / (float)ITEM_CHGTIME;
		}

         // BP FIX - don't adjust LETTERBOXED based on 5.1 or vice versa
#if 0
	      /*
	       * これから合わせる先が 5.1ch で、現在の設定がモノラルであれば
	       * 点灯させない
	       */
	      if((menu_list[i].opt_flag == GM_CONFIG_SOUND_5_1CHANL) &&
		 (work->opt_tmp & GM_CONFIG_CUTSCENES_LETTERBOXED))
		stat1 = stat2 = menu_list[i].dark;
#endif
	    }
	  else if(work->moveitem_cnt && (i == work->pre_item))
	    {
	      stat1 = menu_list[i].blight;
	      stat2 = menu_list[i].dark;
	      rate = (float)work->moveitem_cnt / (float)ITEM_CHGTIME;

         // BP FIX - don't adjust LETTERBOXED based on 5.1 or vice versa
#if 0
	      if((menu_list[i].opt_flag == GM_CONFIG_SOUND_5_1CHANL)
		 && (work->opt_tmp & GM_CONFIG_CUTSCENES_LETTERBOXED))
		stat1 = stat2 = menu_list[i].dark;
#endif
	    }

	  /*
	    stat = menu_list[i].dark;
	    if((i == work->mcur_main) && !work->moveitem_cnt)
	    stat = menu_list[i].blight;
	  */

	  /* 明暗状態を設定 */
	  L2D_MorfObject(parts, stat1, stat2, rate);

	  if(NULL != colon)
	    L2D_MorfObject(colon, stat1, stat2, rate);

	  /* 表示状態を設定 */
	  if(!menu_list[i].disp) obj->sprite.col.a = 0;

	  /* スイッチ部分のオブジェクトがあれば、状態を設定する */
	  if(menu_list[i].menu_sw < 0) continue;

	  /* 状態の設定 */
	  parts = L2D_GetParts(work->handle, menu_list[i].menu_sw);
	  obj = L2D_GetObject(work->handle, menu_list[i].menu_sw);
	  ASSERT(parts != NULL);
	  if(NULL == parts) continue;

	  /* 表示状態の設定 */
	  if(!menu_list[i].disp) SPR_HIDE(obj);

	  stat = 0;
	  if(!(work->opt_tmp & menu_list[i].opt_flag)) stat++;
	  if(menu_list[i].opt_flag2)
	    if(work->opt_tmp & menu_list[i].opt_flag2) stat++;
	  stat = menu_list[i].status[stat];
	  L2D_MorfObject(parts, stat, stat, 1.0F);
	}
    }

  /* 現在のカーソル位置に合わせ、状態設定を行う */
  parts = L2D_GetParts(work->handle, CUR_curs);
  if(NULL != parts)
    {
      if(!work->moveitem_cnt)
	{
	  L2D_MorfObject(parts,
			 menu_list[work->mcur_main].cur_pos,
			 menu_list[work->mcur_main].cur_pos, 1.0F);
	}
      else
	{
	  float rate;
	  
	  rate = (float)work->moveitem_cnt / (float)ITEM_CHGTIME;
	  L2D_MorfObject(parts,
			 menu_list[work->pre_item].cur_pos,
			 menu_list[work->mcur_main].cur_pos, rate);
	}
      if(NULL != (parts = L2D_GetParts(work->handle, CUR_curs_parent)))
	L2D_MorfObject(parts,
		       0x00927bc4  /* "default" */,
		       0x00927bc4  /* "default" */, 1.0F);
    }


   _BP_ReArrangeNodeOptions(work);
}

/*
 * メニュー項目に設定されているアルファ値を,
 * メニュースイッチオブジェクトに反映させる
 */
static void node_alpha_sync(Work * work)
{
  int i;
  SPR_OBJ * item;
  SPR_OBJ * sw;

  if(work->handle < 0) return;   /* 既にハンドルが開放されていたら何もしない */

  for(i = 0; menu_list[i].menu_item >= 0; i++)
    {
      if(menu_list[i].menu_sw < 0) continue;
      
      /* アルファのコピー */
      item = L2D_GetObject(work->handle, menu_list[i].menu_item);
      if(NULL == item) continue;
      sw = L2D_GetObject(work->handle, menu_list[i].menu_sw);
      if(NULL == sw) continue;
      sw->sprite.col.a = item->sprite.col.a;
    }

  _BP_ReArrangeNodeOptions(work);

}


/*
 * 画面の表示チャネルを復元する
 */
static void node_recov(Work * work)
{
  int i;

  for(i = 0; i <= 4; i++)
    {
      printf("chanl %d = %d\n", i, work->old_chanl_flg[i]);
      DG_Chanls[i].flag = work->old_chanl_flg[i];
    }
}

static void node_termination(Work * work)
{
  printf("Node finish.\n");
  /* 背景を終了させる */
  GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_KILL, 0);
  if( work->handle >= 0 ){
	  L2D_ReleaseLayout(work->handle);  /* レイアウトを開放 */
  }
  L2D_ReleaseLayout(work->frame_handle);
  work->handle = -1;
  work->frame_handle = -1;
  DG_SetPrivilegeMode(0);  /* 特権モードから復帰 */
}

/*
 * ノードのメイン画面
 */
#if 1 //BP_PS2 def PSX2

static void NodeMain(Work * work)
{
  enum {
    STEP_INIT,  /* 初期化                             */
    STEP_FDIN,  /* フェードイン                       */
    STEP_SCRN,  /* アクション終了待ち                 */
    STEP_SCRN_WAIT,
    STEP_WAIT,  /* マップロード or オプション選択待機 */
    STEP_MOVEITEM,
    STEP_SELECTED,
    STEP_CHGITEM,
    STEP_EXIT,
    STEP_FDOUT,
    STEP_PRE_RECOV,
#ifdef KP_WINDOWS
    STEP_PRE_RECOV2,
#endif
    STEP_RECOV,
    STEP_QUIT,
    STEP_FINISH
  };

  switch(work->step)
    {
    case STEP_INIT:  /* ノードメインメニュー画面初期化 */
      DBG("STEP_INIT\n");
      {
	int act_name;
	int max;


	if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;

#ifndef NO_MENU_INFO
	/* 説明用字幕を用意する */
	setup_info_panel(work,
			 INFO_DISP_X, INFO_DISP_Y,
			 INFO_DISP_WIDTH, INFO_DISP_HEIGHT,
			 INFO_TEX_WIDTH, INFO_TEX_HEIGHT);  /* 仮の位置 */
#endif

	/* デフォルトのカーソル位置を EXIT に */
	max = 0; while(menu_list[max].menu_item >= 0) max++;
	work->mcur_main = max - 1;
	
	/* メインメニュー表示アクションを起動
	 * ダウンロード表示の有無によって,起動するアクションを使い分ける
	 */
	act_name = SCRN_openOptSlant1;
	if(work->download) act_name = SCRN_openOptSlant2;
	{
	  int stat = L2D_EvokeAction(work->handle, act_name);
	  printf("EvokeAction Status = %d\n", stat);
	}
	work->opt_tmp = GM_Configuration;   /* 操作用オプション */
	work->step = STEP_FDIN;  /* アクション終了待ちへ移行 */
      }
      break;

    case STEP_FDIN:
      if(!start_fadein(work)) break;
      work->step = STEP_SCRN;
      break;
    case STEP_SCRN:
      DBG("STEP_SCRN\n");
      /* おわるまで待つ */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      work->step = STEP_SCRN_WAIT;
      node_create_main_menu(work);
      {
	void * parts;
#if 0
	parts = L2D_GetParts(work->handle, OBJ_Prog);
	L2D_MorfObject(parts, KEY_default, KEY_default, 1.0F);
#endif
	parts = L2D_GetParts(work->handle, OBJ_ROOT);
	L2D_MorfObject(parts, KEY_default, KEY_default, 1.0F);
      }
      // L2D_EvokeAction(work->handle, SCRN_selectOpt);
      break;
    case STEP_SCRN_WAIT:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      node_create_main_menu(work);
#ifndef NO_MENU_INFO
      /* 現在のカーソル位置に合わせて、説明字幕を設定する */
      {
	char * info;
	int lines = 1;
	/* リソースから文字列を取得 */
	if(menu_list[work->mcur_main].cap_num >= 0)
	  {
	    switch(menu_list[work->mcur_main].opt_flag)
       {
       case GM_CONFIG_RADAR_OFF:
          {
             int num = 0;
		  
             if(GM_CONFIG_RADAR_OFF & work->opt_tmp) num = 2;
             else
                if(GM_CONFIG_RADAR_OFF_INTRUDE & work->opt_tmp) num = 1;
             info = BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(pal_info_res[GM_Language-1].res_radar, num));
          }
          break;
       case GM_CONFIG_MENU_QCHANGE_EX:
          {
             int num = 0;
             if(GM_CONFIG_MENU_QCHANGE_EX & work->opt_tmp) num = 1;
             info = BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(pal_info_res[GM_Language-1].res_qchange, num));
             lines = pal_info_res[GM_Language-1].line_qchange[num];
          }
          break;

       case GM_CONFIG_CUTSCENES_LETTERBOXED:
          {
             info = BP_GetStringForEnum(kBP_GLS_Options_ExplainRealTimeCutScenes);
          }
          break;
       default:
          info = BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(pal_info_res[GM_Language-1].res_option,
             menu_list[work->mcur_main].cap_num));

          if(menu_list[work->mcur_main].cap_num == 6 /*OWN VIEW*/ )
             lines = pal_info_res[GM_Language-1].line_ownview;

          break;
       }

       if(NULL != info)
       {
#if BP_USE_NEW_FONT_SYSTEM()
          BP_FONT_DRAWINFO dr;
          BP_font_begin_render_texture(&work->panel.bp_vinfo, 1);
          BP_font_open_drawinfo(&dr, &work->panel.bp_vinfo);
          BP_font_set_color(&dr, 200, 200, 200, 128);
          BP_font_draw_string(&dr, info);
          BP_font_end_render_texture(&work->panel.bp_vinfo);
#else
          FONT_DRAWINFO dr;
          memset(work->panel.vram, 0x00, work->panel.vramsize);
          font_open_drawinfo(&dr, &(work->panel.vinfo));
          font_set_locate(&dr, 0, 0);
          font_set_color(&dr, 0);
          font_draw_string(&dr, info);
          set_vram_to_image( &work->panel ) ;
#endif

          printf("dr.xtop = %d\n", dr.xtop);
		/* 説明字幕の表示行数に合わせて、
		   表示領域のサイズ、UV 値を変動させる */
          work->panel_lines = lines;
          info_size_adjust(work);
       }
   }
   work->info_disp = 1;
      }
#endif
      work->step = STEP_WAIT;
      break;
    case STEP_WAIT:  /* メニュー入力待ち */
      {
	int max;

	/* パッド入力にしたがってカーソル位置を更新する */
	work->pre_item = work->mcur_main;
	max = 0; while(menu_list[max].menu_item >= 0) max++;
	if(work->rep_stat & (PAD_U | PAD_D))
	  GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_CUR01);

	do 
   {

	  if(work->rep_stat & PAD_U) work->mcur_main_screen--;
    
	  //if(work->mcur_main == 4)
     {
		  /* 5.1chの項目はなくなったので飛ばす 2002.09.26 YANO */
	//	  work->mcur_main = 3;
	  }
	  if(work->rep_stat & PAD_D) work->mcur_main_screen++;
	  
     //if(work->mcur_main == 4)
     {
		  /* 5.1chの項目はなくなったので飛ばす */
		 // work->mcur_main = 5;
	  }

	  /* カーソル位置が範囲外にあれば、補正する */

	  if(work->mcur_main_screen < 0) work->mcur_main_screen += MaxNumOptions;
	  if(work->mcur_main_screen >= MaxNumOptions) work->mcur_main_screen -= MaxNumOptions;



      work->mcur_main = menu_cursor_remap[work->mcur_main_screen];


	} while(!menu_list[work->mcur_main].disp);

	if(work->mcur_main != work->pre_item)
	  {
	    /* カーソル移動開始 */
	    DG_DequeueDmapack(work->dmapack);
	    work->info_disp = 0;
	    work->step = STEP_MOVEITEM;
	    work->moveitem_cnt = TIME_BASE;
	    node_create_main_menu(work);
	    break;
	  }

#ifndef NO_MENU_INFO
	/* 現在のカーソル位置に合わせて、説明字幕を設定する */
      if(!work->info_disp)
      {
         char * info;
         int lines = 1;

         /* リソースから文字列を取得 */
         if(menu_list[work->mcur_main].cap_num >= 0)
         {
            switch(menu_list[work->mcur_main].opt_flag)
            {
            case GM_CONFIG_RADAR_OFF:
               {
                  int num = 0;

                  if(GM_CONFIG_RADAR_OFF & work->opt_tmp) num = 2;
                  else
                     if(GM_CONFIG_RADAR_OFF_INTRUDE & work->opt_tmp) num = 1;
                  info = BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(pal_info_res[GM_Language-1].res_radar, num));
               }
               break;
            case GM_CONFIG_MENU_QCHANGE_EX:
               {
                  int num = 0;
                  if(GM_CONFIG_MENU_QCHANGE_EX & work->opt_tmp) num = 1;
                  info = BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(pal_info_res[GM_Language-1].res_qchange, num));
                  lines = pal_info_res[GM_Language-1].line_qchange[num];
               }
               break;


            case GM_CONFIG_CUTSCENES_LETTERBOXED:
               {
                  info = BP_GetStringForEnum(kBP_GLS_Options_ExplainRealTimeCutScenes);
               }
               break;


            default:

               if (work->mcur_main==9 )         //screen brightness - no back to title screen.
               {
                  info = BP_GetStringForEnum(kBP_GLS_Options_ExplainBackToTitleScreen);
               }
               else
               {
                  info = BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(pal_info_res[GM_Language-1].res_option, menu_list[work->mcur_main].cap_num));
                  if(menu_list[work->mcur_main].cap_num == 6 /* OWN VIEW */)
                     lines = pal_info_res[GM_Language-1].line_ownview;
               }
               break;
            }


            // printf("info = %s\n", info);
            if(NULL != info)
            {
#if BP_USE_NEW_FONT_SYSTEM()
               BP_FONT_DRAWINFO dr;
               BP_font_begin_render_texture(&work->panel.bp_vinfo, 1);
               BP_font_open_drawinfo(&dr, &work->panel.bp_vinfo);
               BP_font_set_color(&dr, 200, 200, 200, 128);
               BP_font_draw_string(&dr, info);
               BP_font_end_render_texture(&work->panel.bp_vinfo);
#else
               FONT_DRAWINFO dr;

               /* 表示内容消去 */
               memset(work->panel.vram, 0x00, work->panel.vramsize);

               font_open_drawinfo(&dr, &(work->panel.vinfo));
               font_set_locate(&dr, 0, 0);
               font_set_color(&dr, 0);
               font_draw_string(&dr, info);
               set_vram_to_image( &work->panel ) ;
#endif
               work->panel_lines = lines;
               info_size_adjust(work);
            }
         }
         DG_QueueDmapack(work->dmapack);
         work->info_disp = 1;
      }
#endif
	if( ((work->mcur_main!=9)&&(work->pad->press&(PAD_OK|PAD_L|PAD_R))) || ((work->mcur_main==9)&&(work->pad->press&(PAD_OK))) )
	  {
       work->cancelUsed = 0;

       work->selected = work->pad->press;   
       work->step = STEP_SELECTED;
	  }
	else if(work->pad->press & PAD_CANCEL)
	  {
       work->cancelUsed = 1;
	    work->step = STEP_EXIT;
	    GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_TYPING03);
	    // GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_V_CANS02);
	  }
	/* 現在の位置にカーソルが合った状態でメニュー画面を更新する */
	ASSERT(L2D_STAT_BUSY != L2D_ActionStatus(work->handle));
	node_create_main_menu(work);
      }
  break;
    case STEP_MOVEITEM: /* カーソル位置が更新された場合 */
      work->moveitem_cnt += TIME_BASE;
      if(work->moveitem_cnt >= ITEM_CHGTIME)
	{
	  work->moveitem_cnt = 0;
	  work->step = STEP_WAIT;
	}
      node_create_main_menu(work);
      break;
    case STEP_SELECTED: /* 項目が選択された場合 */
      DBG("STEP_SELECTED\n");
      {
	/*
	 * スイッチ形式の項目であれば、スイッチ内容を切替えてメニューに戻る
	 */
	if(menu_list[work->mcur_main].menu_sw >= 0)
	  {
	    int now;

	    now = 0;
	    if(!(work->opt_tmp & menu_list[work->mcur_main].opt_flag)) now++;
	    if(menu_list[work->mcur_main].opt_flag2)
	      if(work->opt_tmp & menu_list[work->mcur_main].opt_flag2) now++;

	    if(work->selected & (PAD_R | PAD_OK)) now++;
	    if(work->selected & PAD_L) now--;
	    if(now >= menu_list[work->mcur_main].num)
	      now -= menu_list[work->mcur_main].num;
	    if(now < 0)
	      now += menu_list[work->mcur_main].num;

	    if(now >= 2)
	      {
		work->opt_tmp |= menu_list[work->mcur_main].opt_flag2;
		now--;
	      }
	    else
	      work->opt_tmp &= ~menu_list[work->mcur_main].opt_flag2;

	    if(now)
	      work->opt_tmp &= ~menu_list[work->mcur_main].opt_flag;
	    else
	      work->opt_tmp |= menu_list[work->mcur_main].opt_flag;

	    work->step = STEP_SCRN_WAIT;

	    /* 5.1ch と STEREO は特別な相関関係にある。 */
	    switch(menu_list[work->mcur_main].menu_item)
	      {
	      case OBJ_optRAD:
	      case OBJ_optQUICK:
#if !BP_USE_NEW_FONT_SYSTEM()
            memset(work->panel.vram, 0x00, work->panel.vramsize);
#endif
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_TYPING03);
            break;


	      case OBJ_optSND:
            if(work->opt_tmp & menu_list[work->mcur_main].opt_flag)
            {
               /* モノラルに設定する */
               //sd_set_cli(SD_MONORAL);
               /* モノラルに設定されたら 5.1ch を強制的に OFF にする */
               work->opt_tmp &= ~GM_CONFIG_SOUND_5_1CHANL;
            }
            else
            {
               //sd_set_cli(SD_STEREO);
            }
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_TYPING03);
            break;

         case OBJ_opt5_1CH:

// BP FIX:
// options are no longer related
// I don't believe that 5.1 exists but that's neither here nor there
#if 0
            /* モノラルであれば、なにが何でも OFF */
            if(work->opt_tmp & GM_CONFIG_CUTSCENES_LETTERBOXED)
               work->opt_tmp &= ~GM_CONFIG_SOUND_5_1CHANL;
            else
#endif
               GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_TYPING03);
            break;

         default:
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_TYPING03);
            /* デフォルトでは何もしない */
            break;
       }

	    break;
	  }
	else if(menu_list[work->mcur_main].menu_item == OBJ_optSCRN)      // this is really the return to title screen 
	  {
        // BP JG - code goes here to return to title screen.
        // AS JM - Do the return here instead of below so we don't see a
        // screen transition
        GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_TYPING03);
        BP_RelaunchTheGameFromUI();
        work->step = STEP_WAIT;  /* 現在は無効 */
//        work->step = STEP_EXIT;  /* 終了 */
	  }
	else if(menu_list[work->mcur_main].menu_item != OBJ_optEXIT)
	  {
	    /*
	     * スイッチでなければ、別の処理が必要になる
	     */
	    work->step = STEP_WAIT;  /* 現在は無効 */
	  }
	else if(!(work->selected & (PAD_L |PAD_R)))
	  {
	    /*
	     * 選択したのが EXIT であれば、
	     * テンポラリの内容をオプションに反映する
	     */
	    work->step = STEP_EXIT;  /* 終了 */
	  }
	else
	  work->step = STEP_WAIT;  /* 現在は無効 */
      }
      break;

    case STEP_CHGITEM:

      break;
    case STEP_EXIT:
      DBG("STEP_EXIT\n");
      GM_Configuration = work->opt_tmp;
      GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_N_START1);  /* 開始 SE */

      /* もし、レーダのモードが ON になっていれば、レーダを使ったものとする。*/
      if(!(GM_Configuration & GM_CONFIG_RADAR_OFF))
	GM_ClearCodeFlag |= GM_CLEAR_RADAR_USED;


      {

#if 0
   l2dSprite * parts;
	int i;

	if(NULL != (parts = L2D_GetParts(work->handle, OBJ_optSCRN)))
	  for(i = 0; i < parts->vertex_num; i++)
	    parts->vertex[i].a = 0;  /* アルファ値を設定 */

	if(NULL != (parts = L2D_GetParts(work->handle, OBJ_optCOL)))
	  for(i = 0; i < parts->vertex_num; i++)
	    parts->vertex[i].a = 0;  /* アルファ値を設定 */
#endif
      }
#ifndef NO_MENU_INFO
      destroy_info_panel(work);
#endif
      L2D_EvokeAction(work->handle, SCRN_closeOptSlant1);
      work->step = STEP_FDOUT;
      break;

    case STEP_FDOUT:

      DBG("STEP_FDOUT\n");
      /* ノード閉じ中であれば閉じおわるまで進まない */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle)) break;
      
      /*
       * 終了したら、完全に暗転したものとする。
       */
      SPR_ResetMemoryManager();  /* 2D描画モジュールのメモリ管理関数を戻す */
      work->fade_cnt = 0;
      work->fade_mask = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
      SPR_SET_FLAGS(work->fade_mask, SPR_FLAG_ALPHA | SPR_FLAG_PRIV);
      SPR_ALPHA(work->fade_mask, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));
      SPR_SetColorSprite(work->fade_mask, 0, 0, 0, 128);
      SPR_SetPosSprite(work->fade_mask, (SPR_POS*)&DG_ZeroVector);
      SPR_SetSizeSprite(work->fade_mask, SPR_SCRN_WIDTH, SPR_SCRN_HEIGHT);
      SPR_SetPriority(work->fade_mask, SPR_PRI_MOST_NEAR);
      SPR_SHOW(work->fade_mask);

      node_termination(work);
#ifndef KP_WINDOWS
      CDC_ReleaseLogicalDirectory();
#endif
      work->step = STEP_PRE_RECOV;
    case STEP_PRE_RECOV:
      if(!work->bg_dead) break;  /* 背景が死ぬまで待つ */
#ifndef KP_WINDOWS
      work->step = STEP_RECOV;
#else
      work->step = STEP_PRE_RECOV2;
#endif
      break;

#ifdef KP_WINDOWS
    case STEP_PRE_RECOV2:
      CDC_ReleaseLogicalDirectory();	// ここで解放

      
      // 
#if 0
      if ( work->cancelUsed==0 && menu_list[work->mcur_main].menu_item == OBJ_optSCRN )
      {
         // reboot and relaunch
         int confirmRet;
         ShowExitGameWarning( &confirmRet );
         if( confirmRet == 0 )   //yes
         {
            BP_RelaunchTheGameFromUI();
         }
         else
         {
            // regular exit back to game
            work->step = STEP_RECOV;
         }
      }
      else
#endif
      {
         // regular exit back to game
         work->step = STEP_RECOV;
      }


      break ;
#endif

    case STEP_RECOV:
      DBG("STEP_RECOV\n");
      {
	if(codecMemRecov()) break; /* 遅延開放領域が開放しおわるまで進まない */

	node_recov(work);       /* 画面の表示状態 */
	work->step = STEP_QUIT;
      }
      break;
    case STEP_QUIT:
      DBG("STEP_QUIT\n");
      {
	int alpha;
	/* 終ったらフェードインで入る */
	work->fade_cnt += TIME_BASE;
	if((work->fade_cnt >= NODE_FDTIME) ||
	   (work->flags & NODE_FLAG_BLACK_OUT))
	  {
	    GV_PauseOffActorSystem( GV_PAUSE_STOP );
	    work->step = STEP_FINISH;
	    /* BGM 音量上げ */
	    GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SNG_SYUKAN_OFF);
	  }
	alpha = 128 - ((128 * work->fade_cnt) / NODE_FDTIME);
	if(alpha < 0) alpha = 0;
	work->fade_mask->sprite.col.a = alpha;
	printf("fade alpha = %d\n", alpha);
      }
      break;
    case STEP_FINISH:
      /*
       * ノードへのアクセスが終了した場合
       */      
      DBG("STEP_FINISH\n");

      /* 黒マスクオブジェクトを破棄 */
      SPR_Destroy_2D_Object(work->fade_mask);
      GM_ResetMenuStatus(MENU_NODE_ON);  /* ノード中フラグをリセット */
      CODEC_Block(0);  /* 無線禁止を解除 */
      {
	GCL_ARGS  gcl_args;

	gcl_args.argc = 0;
	GCL_ExecProc( work->proc_id, &gcl_args);
      }
      ActSwitch(work, NodeAccessReset);
      break;
    }
  node_alpha_sync(work);/* メニュー項目とスイッチオブジェクトのアルファ同期 */


   _BP_ReArrangeNodeOptions(work);
}

#else //KP_XBOX

static void NodeMain(Work * work)
{
  void *ptr;

  enum {
	  STEP_INIT = 0,  /* 初期化                             */
	  STEP_FDIN,  /* フェードイン                       */
	  STEP_SCRN,  /* アクション終了待ち                 */
	  STEP_SCRN_WAIT,
	  STEP_WAIT,  /* マップロード or オプション選択待機 */
	  STEP_MOVEITEM,
	  STEP_SELECTED,
	  STEP_CHGITEM,
	  STEP_EXIT,
	  STEP_FDOUT,
	  STEP_PRE_RECOV,
#ifdef KP_WINDOWS
	  STEP_PRE_RECOV2,
#endif
	  STEP_RECOV,
	  STEP_QUIT,
	  STEP_FINISH
  };

  printf("node main[%d]\n",work->x_flag );

  switch(work->step){
	case 0:
	  /* オプション画面呼び出し */
	  if( work->x_flag == 0 ){
		  extern void *NewOptionScrForNodeX( void *node_work, int *node_flag );
		  if ( (ptr = NewOptionScrForNodeX( work , &work->x_flag )) != 0 ){
			  GV_SetActorChild( work, ptr );
		  }
	  }
	  /* フェード関係初期化 */
	  start_fadein( work );
	  /* もし、レーダのモードが ON になっていれば、レーダを使ったものとする。*/
	  if(!(GM_Configuration & GM_CONFIG_RADAR_OFF))
		GM_ClearCodeFlag |= GM_CLEAR_RADAR_USED;

	  /* フレームアニメーション */
	  work->next_evoke = GV_StrCode("openSlant1");
	  if(work->download) work->next_evoke = GV_StrCode("openSlant2");

	  work->step = 1;

	  break;
	case 1:
	  /* フェード関係初期化 */
	  /* 毎フレーム必要 */
	  start_fadein( work );

	  break;
	case STEP_FDOUT:
	  DBG("STEP_FDOUT\n");

	  /* ノード閉じ中であれば閉じおわるまで進まない */
	  if( work->handle >= 0 ){
		  if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
	  }
	  if(L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle)) break;
	  
	  /*
	   * 終了したら、完全に暗転したものとする。
	   */
	  SPR_ResetMemoryManager();  /* 2D描画モジュールのメモリ管理関数を戻す */
	  work->fade_cnt = 0;
	  work->fade_mask = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
	  SPR_SET_FLAGS(work->fade_mask, SPR_FLAG_ALPHA | SPR_FLAG_PRIV);
	  SPR_ALPHA(work->fade_mask, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));
	  SPR_SetColorSprite(work->fade_mask, 0, 0, 0, 128);
	  SPR_SetPosSprite(work->fade_mask, (SPR_POS*)&DG_ZeroVector);
	  SPR_SetSizeSprite(work->fade_mask, SPR_SCRN_WIDTH, SPR_SCRN_HEIGHT);
	  SPR_SetPriority(work->fade_mask, SPR_PRI_MOST_NEAR);
	  SPR_SHOW(work->fade_mask);
	  
	  node_termination(work);
#ifndef KP_WINDOWS
	  CDC_ReleaseLogicalDirectory();
#endif
	  work->step = STEP_PRE_RECOV;
	case STEP_PRE_RECOV:
	  if(!work->bg_dead) break;  /* 背景が死ぬまで待つ */
#ifndef KP_WINDOWS
      work->step = STEP_RECOV;
#else
      work->step = STEP_PRE_RECOV2;
#endif
	  break;

#ifdef KP_WINDOWS
    case STEP_PRE_RECOV2:
      CDC_ReleaseLogicalDirectory();	// ここで解放
      work->step = STEP_RECOV;
      break ;
#endif

	case STEP_RECOV:
	  DBG("STEP_RECOV\n");
	  {
		  if(codecMemRecov()) break; /* 遅延開放領域が開放しおわるまで進まない */
		  
		  node_recov(work);       /* 画面の表示状態 */
		  work->step = STEP_QUIT;
	  }
	  break;
	case STEP_QUIT:
	  DBG("STEP_QUIT\n");
	  {
		  int alpha;
		  /* 終ったらフェードインで入る */
		  work->fade_cnt += TIME_BASE;
		  if((work->fade_cnt >= NODE_FDTIME) ||
			 (work->flags & NODE_FLAG_BLACK_OUT))
			{
				GV_PauseOffActorSystem( GV_PAUSE_STOP );
				work->step = STEP_FINISH;
				/* BGM 音量上げ */
				GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SNG_SYUKAN_OFF);
			}
		  alpha = 128 - ((128 * work->fade_cnt) / NODE_FDTIME);
		  if(alpha < 0) alpha = 0;
		  work->fade_mask->sprite.col.a = alpha;
		  printf("fade alpha = %d\n", alpha);
	  }
	  break;
	case STEP_FINISH:
	  /*
	   * ノードへのアクセスが終了した場合
	   */      
	  DBG("STEP_FINISH\n");
	  
	  /* 黒マスクオブジェクトを破棄 */
	  SPR_Destroy_2D_Object(work->fade_mask);
	  GM_ResetMenuStatus(MENU_NODE_ON);  /* ノード中フラグをリセット */
	  CODEC_Block(0);  /* 無線禁止を解除 */
	  {
		  GCL_ARGS  gcl_args;
		  
		  gcl_args.argc = 0;
		  GCL_ExecProc( work->proc_id, &gcl_args);
	  }
	  ActSwitch(work, NodeAccessReset);
	  break;
  }

  if( work->x_flag == 1 ){
	  /* オプション画面起動中 */
	  return ;
  }
  if( work->x_flag == -1 ){
	  extern int PL_COM_SetPadType( void );

	  /* オプション画面終了 */
	  printf("NODE op end\n");
	  /* キーコンフィグ反映 */
	  PL_COM_SetPadType();		  
	  /* ノード閉じフレーム */
	  work->next_evoke = GV_StrCode("closeSlant1");
	  printf("evo[%d]\n", work->next_evoke);

	  work->step = STEP_FDOUT;  /* 終了 */
#if 0
	  GM_ResetMenuStatus(MENU_NODE_ON);  /* ノード中フラグをリセット */
	  CODEC_Block(0);  /* 無線禁止を解除 */
	  {
		  GCL_ARGS  gcl_args;
		  
		  gcl_args.argc = 0;
		  GCL_ExecProc( work->proc_id, &gcl_args);
	  }
	  ActSwitch(work, NodeAccessReset);
#endif

	  work->x_flag = 0;

  }
}

#endif


/*
 * メインのゲーム進行を停止し、前準備各種を行う
 */
static void node_erase(Work * work)
{
  int i;

  /*
   * 各チャネルの表示状態を保存
   */
  for(i = 0; i <= 4; i++)
    {
      work->old_chanl_flg[i] = DG_Chanls[i].flag;
      DG_Chanls[i].flag = 0;  /* 一旦非表示に */
    }

  /*
   * ノードに使用する各チャネルの表示状態を設定
   */
  DG_Chanls[4].flag = 1;   /* メニューチャネルを表示状態にする */

  /* 再生中のストリームを停止する */
  GM_StreamStopAll();

  /*
   * この状態で、メモリを無線同様パケット領域に確保
   */
  codecMemInit();    /* パケット領域に 3MB ほど確保 */

  /* これ以後,2D オブジェクトに使用するメモリ領域は、
     パケット領域に確保した 3MB から確保する。 */
  SPR_SetMemoryManager(codecMalloc, codecDelayedFree);

  /*
    これ以降,動的メモリ分配関数としては、以下のものが使用可能になる。

    void * codecMalloc(size_t size);  確保
    void   codecFree(void * ptr);     開放
    
  */

}

/*
 * ネームエントリ画面を起動する
 */
static void NodeNameEntry(Work * work)
{
  enum {
    STEP_INIT,
    STEP_FDIN,
    STEP_WAIT,
	STEP_FDOUT,
	STEP_PRE_RECOV,
#ifdef KP_WINDOWS
	STEP_PRE_RECOV2,
#endif
	STEP_RECOV,
    STEP_QUIT,
    STEP_SHUTDOWN,
    STEP_FINISH
  };

  switch(work->step)
    {
    case STEP_INIT:
      {
	void * name_entry_work;
	extern void *NewNameLayoutByNode( int name, int frame_l2d );

	work->step = STEP_FDIN;
	name_entry_work = NewNameLayoutByNode(0, work->frame_handle);
	if(NULL != name_entry_work)
	  GV_SetActorChild(work, name_entry_work);
	else
	  {
	    printf("node.c : Name Entry Execute failed.\n");
	    work->step = STEP_FINISH;
	  }
      }
      break;

    case STEP_FDIN:
      if(!start_fadein(work)) break;
      work->step = STEP_WAIT;
      break;
    case STEP_WAIT:
      if(work->name_entry_end) {
		  if(work->name_entry_only) {
			  work->step = STEP_FDOUT;
		  }else{
			  work->step = STEP_FINISH;
		  }
	  }
      break;
    case STEP_FDOUT:
      DBG("STEP_FDOUT\n");
      /* ノード閉じ中であれば閉じおわるまで進まない */
	  printf("AAAAAAAAA\n");
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
	  printf("bAAAAAAAAA\n");
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->frame_handle)) break;
	  printf("cAAAAAAAAA\n");

      /*
       * 終了したら、完全に暗転したものとする。
       */
      SPR_ResetMemoryManager();  /* 2D描画モジュールのメモリ管理関数を戻す */
      work->fade_cnt = 0;
      work->fade_mask = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
      SPR_SET_FLAGS(work->fade_mask, SPR_FLAG_ALPHA | SPR_FLAG_PRIV);
      SPR_ALPHA(work->fade_mask, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));
      SPR_SetColorSprite(work->fade_mask, 0, 0, 0, 0);
      SPR_SetPosSprite(work->fade_mask, (SPR_POS*)&DG_ZeroVector);
      SPR_SetSizeSprite(work->fade_mask, SPR_SCRN_WIDTH, SPR_SCRN_HEIGHT);
      SPR_SetPriority(work->fade_mask, SPR_PRI_MOST_NEAR);
      SPR_SHOW(work->fade_mask);
	  work->step = STEP_QUIT;


    case STEP_QUIT:
      DBG("node.c : STEP_QUIT\n");
      {
		  int alpha;
#define NODE_NAME_FDTIME (NODE_FDTIME)
		  /* 終ったらフェードインで入る */
		  work->fade_cnt += TIME_BASE;
		  if( work->fade_cnt >= NODE_NAME_FDTIME )
			{
				GV_PauseOffActorSystem( GV_PAUSE_STOP );
				/* BGM 音量上げ */
				GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SNG_SYUKAN_OFF);

				node_termination(work);
#ifndef KP_WINDOWS
				CDC_ReleaseLogicalDirectory();
#endif
				work->step = STEP_PRE_RECOV;
			}
		  alpha = /*128 -*/ ((128 * work->fade_cnt) / NODE_NAME_FDTIME);
		  if(alpha < 0) alpha = 0;
		  work->fade_mask->sprite.col.a = alpha;
		  printf("fade alpha = %d\n", alpha);
      }
      break;

    case STEP_PRE_RECOV:
      if(!work->bg_dead) break;  /* 背景が死ぬまで待つ */
#ifndef KP_WINDOWS
      work->step = STEP_RECOV;
#else
      work->step = STEP_PRE_RECOV2;
#endif
      break;

#ifdef KP_WINDOWS
    case STEP_PRE_RECOV2:
      CDC_ReleaseLogicalDirectory();	// ここで解放
      work->step = STEP_RECOV;
      break ;
#endif

    case STEP_RECOV:
      DBG("node.c : STEP_RECOV\n");
      {
		  if(codecMemRecov()) break; /* 遅延開放領域が開放しおわるまで進まない */

		  node_recov(work);       /* 画面の表示状態 */
		  work->step = STEP_SHUTDOWN;
      }
      break;

    case STEP_SHUTDOWN:
	  {
		  /*
		   * ノードへのアクセスが終了した場合
		   */      
		  DBG("node.c : STEP_SHUTDOWN\n");
		  
		  /* 黒マスクオブジェクトを破棄 */
		  SPR_Destroy_2D_Object(work->fade_mask);
		  GM_ResetMenuStatus(MENU_NODE_ON);  /* ノード中フラグをリセット */
		  CODEC_Block(0);  /* 無線禁止を解除 */
		  ActSwitch(work, NodeAccessReset);

		  if ( work->proc_id )
			GCL_ExecProc( work->proc_id, NULL ) ;
      
	  }
	  break ;

    case STEP_FINISH:
      {
		  /* 最初のノードアクセスなので、ダウンロード画面を起動 */
		  ActSwitch(work, NodeMapDownload);
      }
      break;
    }
}

/*
 * ノード画面の初期設定を行う。
 */
static int node_setup(Work * work)
{
   float safeZoneOffsetY=0;
#ifdef PSX2
  /* ノード画面用レイアウトデータを設定する */
  DBG("node_setup(): [1]\n");
  {
    void * ptr = GV_GetCache(GV_CacheID(work->skin_name, 'o'));
    if(NULL == ptr) ptr = CDC_GetFileEntry(work->skin_name, 'o');
    work->handle = L2D_SetupLayout2(ptr, 4, 5, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
    /* ロードできなければ何もしない */
    if(work->handle < 0) return -1;
  }

  /* メニューレイアウトのシグナルハンドルを設定する */
  L2D_SetSignalHandle(work->handle, work, MenuSignalHandle);
#endif

  /* 枠レイアウトを設定する */
  {
    void * ptr = GV_GetCache(GV_CacheID(work->frame_name, 'o'));
    if(NULL == ptr) ptr = CDC_GetFileEntry(work->frame_name, 'o');

    work->frame_handle = L2D_SetupLayout2(ptr, 4, 0, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
    /* レイアウトをロードするために必要なメモリが無ければ、
       メニューレイアウトを開放して終了 */
    if(work->frame_handle < 0)
      {
		  if( work->handle >= 0 ){
			  L2D_ReleaseLayout(work->handle);
		  }
		  return -1;
      }
  }

  /* 枠レイアウトのシグナルハンドルを設定する */
  L2D_SetSignalHandle(work->frame_handle, work, FrameSignalHandle);

  /* レイアウトを初期化するアクションを呼び出す */
  DBG("node_setup(): [2]\n");
  // L2D_EvokeAction(work->handle, SCRN_DefaultAction);
  L2D_EvokeAction(work->frame_handle, SCRN_DefaultAction);
  // L2D_EvokeAction(work->frame_handle, SCRN_setBaseOpen);

  work->frame_mode = 0;
  work->bg_dead = 0;
  /* 背景キャラを起動 */
  printf("============ Evoke BG Chara ===============\n");
  {
    void * workp;

    workp = NewNodeBG_P();
    ASSERT(NULL != workp);
    GV_SetActorChild(work, workp);

    GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_OPENGRD, 0);
    GV_CallChildSignalFunc(work, NODE_BG_SIGNAL_OPENPLMAP, 0);
  }

  /* 特権モードとして画面を描画 */
  DBG("node_setup(): [3]\n");
  DG_SetPrivilegeMode(1);  /* 特権モードに移行 */
  GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_N_START1);  /* 開始 SE */

  /* ノード画面が起動してからは、Act() から
     呼び出される先を NodeMain() に切替える */
  DBG("node_setup(): [4]\n");
  if(work->name_entry)
    {
      printf("Execute Name Entry.\n");
      ActSwitch(work, NodeNameEntry);
    }
  else
    {
      if(work->download)
	{
	  printf("Execute Download\n");
	  ActSwitch(work, NodeMapDownload);  /* 要ダウンロードの場合は、
				      ダウンロード画面から開始される。 */
	}
      else
	{
	  printf("Execute Node Option\n");
	  ActSwitch(work, NodeMain);  /* 2回目以降はメインメニューから開始 */
	}
    }
  return 0;
}


/*
 * アクセスフラグをリセットして,再度待機に入る
 */
static void NodeAccessReset(Work * work)
{
  work->accept = 0;
  ActSwitch(work, NodeAccessWait);
}

/*
 * アクセスの待ち受けを行う
 */
static void NodeAccessWait(Work * work)
{
  enum {
    STEP_INIT,   /* 待ち受け初期化                */
    STEP_WAIT,   /* アクセス待ち                  */
    STEP_LOAD,   /* ロード開始待ち */
    STEP_ERASE,  /* 画面フェードアウト開始        */
    STEP_FDOUT,  /* 画面フェードアウト中,ロード中 */
    STEP_SETUP   /* 画面用意中                    */
  };
  work->next_evoke = -1;

  switch(work->step)
    {
    case STEP_INIT:
      DBG("NodeAccessWait() is Initialized.\n");
      // work->accept = 0;
      work->step = STEP_WAIT;
      break;
    case STEP_WAIT:
      {
	void * tmp;
	extern int SK_CheckMapStatus( void );
	if(SK_CheckMapStatus()) break;

	if(!work->accept) break;
	work->accept = 0;   /* リセット */

	/* 安全のため、とりあえず 128kb ほど確保してみて、
	   確保できなければ却下する */
	if(NULL == (tmp = GV_Malloc(128 * 1024))) break;

	work->fade_cnt = 0;
	work->fade_mask = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
	if(NULL == work->fade_mask)
	  {
	    GV_Free(tmp);
	    break;
	  }
	CODEC_Block(1);
	GV_PauseOnActorSystem( GV_PAUSE_STOP );  /* 全体を PAUSE 状態にする */
	GV_Free(tmp);
	GM_SetMenuStatus(MENU_NODE_ON);  /* ノードアクセス中フラグ設定 */

	SPR_SET_FLAGS(work->fade_mask, SPR_FLAG_ALPHA | SPR_FLAG_PRIV);
	SPR_ALPHA(work->fade_mask, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0));
	/* 黒単色透明から開始 */
	SPR_SetColorSprite(work->fade_mask, 0, 0, 0, 0);
	SPR_SetPosSprite(work->fade_mask, (SPR_POS*)&DG_ZeroVector);
	SPR_SetSizeSprite(work->fade_mask, SPR_SCRN_WIDTH, SPR_SCRN_HEIGHT);
	SPR_SetPriority(work->fade_mask, SPR_PRI_MOST_NEAR);
	SPR_SHOW(work->fade_mask);

	/* フェードアウトの準備 */
	work->step = STEP_FDOUT;
	
	/* BGM 音量下げ */
	GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SNG_SYUKAN_ON);

	/* 暗転状態から入る場合は、真っ暗ではじまり、すぐ終る */
	if(work->flags & NODE_FLAG_BLACK_IN)
	  {
	    work->fade_cnt = NODE_FDTIME;
	    SPR_SetColorSprite(work->fade_mask, 0, 0, 0, 128);
	  }
      }
      break;

    case STEP_FDOUT:   /* まず画面をフェードアウトさせる(短時間で) */
      {
	int alpha;
	work->fade_cnt += TIME_BASE;

	if(work->fade_cnt >= NODE_FDTIME)    /* フェードアウト終了 */
	  {
	    node_erase(work);   /* 画面の消去等の準備 */
	    /* 既に画面は消えているので,マスクを消去する */
	    // SPR_Destroy_2D_Object(work->fade_mask);
	    work->fade_cnt = 0;
	    work->step = STEP_LOAD;
	    break;
	  }

	alpha = 128 * work->fade_cnt / NODE_FDTIME;
	if(alpha > 128) alpha = 128;
	work->fade_mask->sprite.col.a = alpha;
	printf("fade alpha = %d\n", alpha);
      }
      break;
    case STEP_LOAD:
      if(cdbios_get_status()) break;
      /*
       * レイアウト等を含むデータのロードを開始する
       */
      BP_SetSpinnerLoadFlag(kLoadFlag_ScriptedLoad1, 1);
      CDC_LoadLogicalDirectory(work->node_file_ID);
      work->step = STEP_ERASE;
      break;
    case STEP_ERASE:
      if(!CDC_SyncCodecDataLoad())
	{
      BP_SetSpinnerLoadFlag(kLoadFlag_ScriptedLoad1, 0);
	  /* ロードが終ったら準備 */
	  CDC_InitLoadData();
	  work->step = STEP_SETUP;
	}
      break;
      
    case STEP_SETUP:
      printf("Node Setup -------------------------------------------~~~~\n");
      node_setup(work);
      break;
    }
}


static void Act(Work * work)
{
  if(L2D_STAT_BUSY != L2D_ActionStatus(work->frame_handle))
  {
     BGMsg_Stop(work);
  }

  /* 枠レイアウトに実行予約アクションがある場合は,
     現在のアクション終了を待ち、そのアクションを起動する。 */
  if(work->next_evoke >= 0){
    if(L2D_STAT_BUSY != L2D_ActionStatus(work->frame_handle))
      {
		  int se_id;
		  int res;
		  res = L2D_EvokeAction(work->frame_handle, work->next_evoke);
		  if( res < 0 ){
			  //ASSERT( 0 );
		  } 
		  se_id = -1;
		  switch(work->next_evoke)
			{
				/* 開き: 右から左 */
			  case 0x00458aec  /* "openSlant1" */:
			  case 0x00458aed  /* "openSlant2" */:
			  case 0x0007601f  /* "openDown" */:   se_id = SD_S_WINOPNR1; break;
				
				/* 開き: 左から右 */
			  case 0x00458aee  /* "openSlant3" */:
			  case 0x00458aef  /* "openSlant4" */: se_id = SD_S_WINOPNL1; break; 
				
				
				/* 閉じ: 右から左 */
			  case 0x00e4fa38  /* "closeSlant2" */:
			  case 0x000a54bd  /* "closeToLeft" */:  se_id = SD_S_WINCLSR1; break;
				
				
				/* 閉じ: 左から右 */
			  case 0x00e4fa37  /* "closeSlant1" */:
			  case 0x00ac9a95  /* "closeToRight" */:
			  case 0x00da47fa  /* "closeDown" */:    se_id = SD_S_WINCLSL1; break;
				
				/* 閉じ: 縦方向のみ */
			  case 0x004873ec  /* "closeToTop" */:
			  case 0x00482c89  /* "closeToBtm" */:   se_id = SD_S_WINCLS01; break;
			  default: se_id = -1; break;
			}
		  if(se_id >= 0)
			GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, se_id);  /* 開き SE */
		  work->next_evoke = -1;
      }
	printf("evo act[%d]\n",work->next_evoke);
}

  _BP_ReArrangeNodeOptions(work);


  /* フレーム待ちカウンタの値がある場合は、空フレームとして扱う */
  if(work->wait_cnt)
    {
		work->wait_cnt--;
		return;
    }
  work->pad = &GV_PadDataDirect[0];
  work->rep_stat = repeat_status(work);
  (work->act_func)(work);
  _BP_ReArrangeNodeOptions(work);


}


static void Die(Work * work)
{
  if(now_work == work) now_work = NULL;

  /* ノード画面のレイアウトを開放 */
#ifdef NODE_LAYOUT
  if(work->handle >= 0) L2D_ReleaseLayout(work->handle);
  if(work->frame_handle >= 0) L2D_ReleaseLayout(work->frame_handle);
#endif /* NODE_LAYOUT */

#ifdef KP_WINDOWS
  CDC_ReleaseLogicalDirectory();
#endif
}

static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case NAME_ENTRY_END:
      work->name_entry_end = 1;
      break;
    case NODE_BG_SIGNAL_DEAD:   /* 背景が死んだことの通知 */
      work->bg_dead = 1;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static int GetResources(Work * work, int name, int map)
{

  /* プロセスシグナルハンドラを登録 */
  GV_SetActorSignalFunc(work, ReceiveSignal);


  /* デバッグ用 */
  work->skin_name = GV_StrCode("node_menu");
  work->frame_name = GV_StrCode("node_frame");

  /* 全体マップの名称を得る */
#if 0
  if(NULL != GCL_GetOption('s')) 
    work->skin_name = GCL_GetNextInt();  /* スキン名取得 */
#endif
  if(NULL != GCL_GetOption('p'))
    work->proc_id = GCL_GetNextInt();    /* ノード起動コールバック取得 */

  if(NULL != GCL_GetOption('f'))
    work->node_file_ID = GCL_GetNextInt();

  work->name_entry_only = 0 ;
  if(NULL != GCL_GetOption('n'))
	work->name_entry_only = 1 ;


  work->map_name = -1;     /* マップはまだ指定されていない       */
  work->map_act   = NULL;  /* 全体マップ表示 Actor はまだ無し    */

  DBG("Map = 0x%08x\n", work->map_name); /* 全体マップ名 */
  
  /* Actor で使用する各種パラメタの初期化 */
  work->wait_cnt  = 0;     /* フレーム待ちカウンタは 0 に        */
  work->mcur_main = 0;     /* ノードメインメニューのカーソル位置 */
  work->mcur_opt  = 0;     /* ノードオプション画面のカーソル位置 */


  work->mcur_main = 0;
  // AS(JM) menu_cursor_remap was being used backwards, or something.
  // Really, it should be set to MaxNumOptions - 1 so that "Exit" is selected
  // by default. MGSTWO-2991
  work->mcur_main_screen = MaxNumOptions - 1; // menu_cursor_remap[work->mcur_main];

  /* オートリピートの初期化 */
  work->rep_stat  = 0;
  work->pre_stat  = 0;
  work->rep_cnt   = 0;

  /* L2Dハンドラの初期化 */
  work->handle = -1;
  work->frame_handle = -1;

  /* 起動予約枠アクションは無し */
  work->next_evoke = -1;

  /* 初期の Act() として、アクセス待ち受け関数を設定する */
  ActSwitch(work, NodeAccessWait); /* 初期状態では、アクセス待ちにしておく */
  work->accept   = 0;               /* アクセス要求無し                     */

  now_work = work;

  return 0;
}

/*
 * ノード端末デーモンの登録
 */
void * NewNodeDaemon(int name, int map)
{
  Work * work;

  /* name が、actor の要求しているものでなければ不許可 */
  if(name != NodeID) return NULL;

  ASSERT(now_work == NULL);  /* 二重登録は不可 */

  if(NULL == (work = GV_NewActorPrio(NODE_ACT_MODE,
				     sizeof(Work), NODE_ACT_PRIO)))
    return NULL;

  /* actor を登録 */
  GV_SetActor(work, Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, name, map))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}

/*
 * ノード画面起動(プログラム用)
 */
int NewNode_for_Prog(int map, int name, int flag)
{
  DBG("entering NewNode()\n");
  if(NULL == now_work) return -1;
  now_work->download = map;
  now_work->name_entry = name;
  now_work->accept = 1;
  now_work->flags = flag;
  return 0;
}

/*
 * ノード画面の起動(シナリオ用)
 */
int NewNode(void)
{
  int map = 0;
  int name = 0;
  int flag = 0;
  if(NULL != GCL_GetOption('m')) map = 1;
  if(NULL != GCL_GetOption('n')) name = 1;
  if(NULL != GCL_GetOption('b'))
    flag |= GCL_GetNextInt();  /* 暗転フラグ取得 */
  
  return NewNode_for_Prog(map, name, flag);
}

static void _BP_ReArrangeNodeOptions(Work *work)
{
   //BP JG - This code is designed to move the menu items around that are build in script.

   float yPosTweak = 6;
   float yTextPos = 71+3;
   float yTextSize = 22;
   SPR_OBJ *pTestObject;
   void * parts;
   l2dStatus * stat1;  /* ??1(???) */

   if ( work->handle < 0 )
   {
      // Layout has been unloaded.  Punt.
      return;
   }

   if ( work->panel_lines==0 )
      return;
 
   // change vibration to ... full frame hud
   pTestObject = L2D_GetObject(work->handle,OBJ_optVIB);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   if ( pTestObject ) SPR_HIDE(pTestObject);
   //pTestObject->head.flags = 0xa001;

   pTestObject = L2D_GetObject(work->handle,OBJ_colonVIB);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   if ( pTestObject ) SPR_HIDE(pTestObject);

   pTestObject = L2D_GetObject(work->handle,OBJ_swVIB);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   if ( pTestObject ) SPR_HIDE(pTestObject);

   // change sound options to ... real time cutscenes
   pTestObject = L2D_GetObject(work->handle,OBJ_optSND);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*2);
   if ( pTestObject ) pTestObject->sprite.dw = 214;
   if ( pTestObject ) pTestObject->sprite.dh = 10;
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   if ( pTestObject ) SPR_HIDE(pTestObject);
   pTestObject = L2D_GetObject(work->handle,OBJ_colonSND);
   if ( pTestObject ) pTestObject->sprite.pos.x = 286;
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*2);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   if ( pTestObject ) SPR_HIDE(pTestObject);
   pTestObject = L2D_GetObject(work->handle,OBJ_swSND);
   if ( pTestObject ) pTestObject->sprite.pos.x = 300;
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*2);
   if ( pTestObject ) pTestObject->sprite.dw = 128;
   if ( pTestObject ) pTestObject->sprite.dh = 10;
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   if ( pTestObject ) SPR_HIDE(pTestObject);

   // move 'radar' up
   pTestObject = L2D_GetObject(work->handle,OBJ_optRAD);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*0);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   pTestObject = L2D_GetObject(work->handle,OBJ_colonRAD);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*0);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;
   pTestObject = L2D_GetObject(work->handle,OBJ_swRAD);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*0);
   if ( pTestObject ) pTestObject->head.flags = 0x2001;

   // move 'blood' up
   pTestObject = L2D_GetObject(work->handle,OBJ_optBLD);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*1);
   pTestObject = L2D_GetObject(work->handle,OBJ_colonBLD);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*1);
   pTestObject = L2D_GetObject(work->handle,OBJ_swBLD);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*1);

   // move 'captions' up
   pTestObject = L2D_GetObject(work->handle,OBJ_optCAP);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*3);
   pTestObject = L2D_GetObject(work->handle,OBJ_colonCAP);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*3);
   pTestObject = L2D_GetObject(work->handle,OBJ_swCAP);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*3);

   // move 'own view' up
   pTestObject = L2D_GetObject(work->handle,OBJ_optOWN);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*4);
   pTestObject = L2D_GetObject(work->handle,OBJ_colonOWN);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*4);
   pTestObject = L2D_GetObject(work->handle,OBJ_swOWN);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*4);

   // move 'item window' up
   pTestObject = L2D_GetObject(work->handle,OBJ_optITEM);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*6);
   if ( pTestObject ) SPR_HIDE(pTestObject);
   pTestObject = L2D_GetObject(work->handle,OBJ_colonITEM);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*6);
   if ( pTestObject ) SPR_HIDE(pTestObject);
   pTestObject = L2D_GetObject(work->handle,OBJ_swITEM);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*6);
   if ( pTestObject ) SPR_HIDE(pTestObject);

   // move 'quick change' up
   pTestObject = L2D_GetObject(work->handle,OBJ_optQUICK);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*5);
   pTestObject = L2D_GetObject(work->handle,OBJ_colonQUICK);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*5);
   pTestObject = L2D_GetObject(work->handle,OBJ_swQUICK);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*5);

   // add screen adjust and position it. we're using this for something else.
   pTestObject = L2D_GetObject(work->handle,OBJ_optSCRN );
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*11);

   // move 'exit' down
   pTestObject = L2D_GetObject(work->handle,OBJ_optEXIT);
   if ( pTestObject ) pTestObject->sprite.pos.y = yTextPos+(yTextSize*11);


   // update the cursor positions
   menu_list[1].cur_pos = POS_curVIB;
   menu_list[2].cur_pos = POS_curRAD;

   //menu_list[3].cur_pos = POS_curITEM;
   //menu_list[5].cur_pos = POS_curBLD;

   //menu_list[3].cur_pos = POS_curBLD;
   menu_list[5].cur_pos = POS_curBLD;


   menu_list[6].cur_pos = POS_curSND;
   
   //menu_list[7].cur_pos = POS_curCAP;
   
   menu_list[8].cur_pos = POS_curCAP;
   menu_list[9].cur_pos = POS_curQUICK;
   menu_list[11].cur_pos = POS_curEXIT;



   // find and adjust the actions for the cursor position.
   parts = L2D_GetParts(work->handle, CUR_curs);
   if ( parts )
   {
      stat1 = BP_GetStatus(parts, POS_curQUICK);            // this one is re-used as the 'Return to title screen' option
      stat1->vertex->y = (yTextPos+3)+(yTextSize*10)-6;
      stat1 = BP_GetStatus(parts, POS_curEXIT);
      stat1->vertex->y = (yTextPos+3)+(yTextSize*11)-3;
   }

}