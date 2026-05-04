//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  console.c
  テキストコンソール画面演出

  2001/07/14   Y.Kira
  $Id: console.c,v 1.1.1.3 2002/11/19 11:45:18 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gameheader.h"
#include "g_define.h"

#include "g_struct.h"
#include "libdg.h"
#include "libdg.cnf"
#include "font.h"
#include "libfs.h"
#include "libmt.h"
#include "libutl.h"
#include "dmapack.h"
#include "def_dma.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#define _console_c_
#include "photo_config.h"
#include "photo_term.h"
#include "photo_pic.h"
#include "photo_itp.h"
#include "photo_panel.h"
#include "photo_puppet.h"
#include "photo_disp.h"
#include "photo_codes.h"

#include "../codec/codecmem.h"
#include "../codec/cdc_load.h"
#include "console.h"
#include "con_act.h"

#ifdef DEBUG
#define DBG
#else
#define DBG
#endif  /* DEBUG */

/* VT100端末サイズ */
#define TEXT_WIDTH  80
#define TEXT_HEIGHT 25

#define MAX_CHAR  (TEXT_WIDTH * TEXT_HEIGHT)

#define DEF_ATTRIB  0x0700       /* デフォルト文字属性 */

/* フォントの大きさ */
#define FONT_WIDTH  8
#define FONT_HEIGHT 12

#define QSIZ(_type)    ((sizeof(_type) + 15) / 16)
#define DSIZ(_type)    ((sizeof(_type) + 7) / 8)

#define BLINK_TIME 150
#define BLINK_ON   100

#define CUR_R     255
#define CUR_G     255
#define CUR_B     255


static int CONSOLE_PACKET_SIZE = 30000;

typedef ALIGN16_DECL(struct) {

  ALIGN16_DECL(struct) {
    DG_DMATAG  trans0;
    DG_DMATAG  trans1;
  } tex_trans ;
  
  DG_DMATAG    dmatag;

  ALIGN16_DECL(struct) gif_blk {
    DG_GIFTAG    gif_packed;
    ALIGN16_DECL(struct) dat_packed {
      DG_GSREG     alpha;
      DG_GSREG     tex2;
    } packed ;

    DG_GIFTAG    gif_reglist;
    ALIGN16_DECL(struct) dat_reglist {
      /* カーソル表示用 */
      u_long64 curs_prim;
      u_long64 curs_rgba;
      u_long64 curs_xyz0;
      u_long64 curs_xyz1;

      u_long64 clamp;
      u_long64 tex0;

      u_long64 prim;
      u_long64 rgba;
    } reglist ;
  } gif ;
  
  /*
   * ここから下は,
   * テキストイメージにしたがってスプライトを表示するために使用される。
   */
  DG_DMATAG    dma_text;
  DG_GIFTAG    gif_text;
  ALIGN16_DECL(struct) text_prim {
    u_long64 rgba;
    u_long64 uv0;
    u_long64 xyz0;
    u_long64 uv1;
    u_long64 xyz1;
    u_long64 nop;
  } text[ MAX_CHAR ] ;
} text_draw ;

typedef struct console_Work 
{
  GV_ACT_EX    actor;

  int          l2d_handle;   /* 装飾 L2D のハンドル          */

  SPR_OBJ    * region;       /* テキスト領域指定オブジェクト */

  /* テキスト領域の左上座標 */
  float        begin_x;
  float        begin_y;

  /* テキスト領域の大きさ */
  float        width;
  float        height;

  /* 文字サイズ */
  float        pitch_x;
  float        pitch_y;


  /* カーソル位置 */
  int          cursor_x;
  int          cursor_y;

  DG_DMAPACK * dmapack;
  DG_TEXTURE_LIST * tex_list;
  DG_TEX     * dgtex;

  int          base_u;  /* 基準 u 値 */
  int          base_v;  /* 基準 v 値 */


  text_draw  * packet[2];

  /*
   * テキスト表示管理に用いるパラメータ
   */
  int          wait_time;             /* 表示を再開するまでの時間           */
  char       * text;                  /* 表示中のテキストポインタ           */
  short        now_attrib;            /* 現在の属性                         */
  int          blink_cnt;             /* カーソルブリンクのためのカウンタ   */
  int          step;
  
  /* テキストバッファ */
  short        text_buf[ MAX_CHAR ];  /* 上位 8 bit が属性,下位 8bit が文字 */


  /* HDD シーク音 SE 用 */
  int          hdd_time;    /* 持続時間           */
  int          hdd_cnt;     /* SE 発生時間        */
  int          hdd_rep;     /* 繰り返し用カウンタ */

#if 1 //BP ndef PSX2
  float          uv_width  ;
  float          uv_height ;
  void			*packet_mem;
  void          *prim ;
#endif
  
} Work;


/*
 * 現在のテキストバッファの内容を DMA パケットに反映する
 */
static void make_packet(Work * work)
{
  int num;
  int x, y;
  float cx, cy;
  float u, v;
  int code;
  int alpha;

  alpha = 128;
  if((work->blink_cnt += TIME_BASE) >= BLINK_ON) alpha = 0;
  if(work->blink_cnt >= BLINK_TIME) work->blink_cnt -= BLINK_TIME;

  /*カーソルの表示*/
  {
    cx = work->begin_x + work->width  * work->cursor_x / TEXT_WIDTH;
    cy = work->begin_y + work->height * work->cursor_y / TEXT_HEIGHT;

	work->prim = work->packet_mem ;
	work->prim = DG_SetDmapackAlpha( work->prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
	work->prim = DG_SetDmapackBox( work->prim,
								   cx                , cy                ,
								   cx + work->pitch_x, cy + work->pitch_y,
								   DG_MakeDMAPackColor(CUR_R, CUR_G, CUR_B, 0x80) ) ;

	work->prim = DG_SetDmapackTex( work->prim, work->dgtex ) ;
  }

  num = 0;
  for(y = 0; y < TEXT_HEIGHT; y++) {
      cy = work->begin_y + work->height * y / TEXT_HEIGHT;

      for(x = 0; x < TEXT_WIDTH; x++) {
	    int r, g, b;

		if((code = work->text_buf[ y * TEXT_WIDTH + x ]) == ' ') continue;

		cx = work->begin_x + work->width * x / TEXT_WIDTH;

	    b = (code & 0x100) >> 1;
	    r = (code & 0x200) >> 2;
	    g = (code & 0x400) >> 3;

		code = (code & 0xff);
      
      if (code >= ' ' && code < ' ' + 96)
      {
         code -= ' ';

         /* 表示座標の確定 */
         v = (((float)(code / 0x20) * FONT_HEIGHT)) + work->base_v;
         u = (((float)(code & 0x1f) * FONT_WIDTH )) + work->base_u;
         work->prim = DG_SetDmapackSprt( work->prim,
            cx                , cy                ,
            (u             )/work->uv_width, (v              )/work->uv_height,
            cx + work->pitch_x, cy + work->pitch_y,
            (u + FONT_WIDTH)/work->uv_width, (v + FONT_HEIGHT)/work->uv_height,
            DG_MakeDMAPackColor( r,g,b, 0x80) ) ;
      }
	
		num++;
	}
  }
  
  work->prim = DG_SetDmapackEnd( work->prim ) ;
  
  if( ((int)work->prim - (int)work->packet_mem) > CONSOLE_PACKET_SIZE )
  {
     printf("Need %d packet phototerm/console.c!!\n" , (int)work->prim - (int)work->packet_mem );
     BP_BREAK;
  }
}

/*
 * 次の表示文字を出力するテキスト座標を求める
 */
static void locate(Work * work, int x, int y)
{
  work->cursor_x = x;
  work->cursor_y = y;
}

/*
 * 全テキスト消去
 */
static void clear(Work * work)
{
  int i;
  for(i = 0; i < MAX_CHAR; i++) work->text_buf[i] = ' ';
}

/*
 * テキストの ROLL UP
 */
static void roll_up(Work * work)
{
  int i;

  /* 位置行持ち上げる */
  for(i = 0; i < TEXT_WIDTH * (TEXT_HEIGHT - 1); i++)
    work->text_buf[i] = work->text_buf[i + TEXT_WIDTH];

  /* 最も下段の行をクリアする */
  for(i = MAX_CHAR - TEXT_WIDTH; i < MAX_CHAR; i++) work->text_buf[i] = ' ';
}


/*
 * デフォルトの属性で、現在のカーソル位置に文字を書き込む
 */
static void put_char(Work * work, int c)
{
  if((c & 0xff) == 0xd2) c = 0x7e | (c & 0xff00); /* JIS X 0201 KANA の 'メ' */
  if((c & 0xff) == '~')  c = 0x7c | (c & 0xff00); /* '~' */

  work->text_buf[ work->cursor_y * TEXT_WIDTH + work->cursor_x ] =
    (c & 0xff) | (work->now_attrib & 0xff00);

  work->cursor_x++;
  if(work->cursor_x >= TEXT_WIDTH)
    {
      work->cursor_x = 0;
      work->cursor_y++;
    }
  if(work->cursor_y >= TEXT_HEIGHT)
    {
      roll_up(work);
      work->cursor_y = TEXT_HEIGHT - 1;
    }
}

static int get_sound(int snd)
{
  static int se_list[] = {
    SD_A_PC_BOOT1,  /* ブート音     */
    SD_A_PC_HDSEK,  /* HDD シーク音 */
    SD_A_PC_START,  /* 起動音       */
    SD_A_PC_ALART,  /* 警告音       */
    SD_A_PC_OKOK1,  /* 端末了承音   */
  };

  if(snd >= 5) return -1;
  return se_list[snd];
}

/*
 * ハードディスクシーク音のモードを切替える
 */
static void hdd_mode(Work * work, int hdd_id)
{
  work->hdd_rep  = 0; /* SE リピート用のカウンタをリセットする */
  switch(hdd_id)
    {
    case 0:  /* HDD停止        */
      work->hdd_rep = -1;   /* シーク音を中断 */
      break;

    case 1:  /* 持続時間 0.5秒 */
      work->hdd_time = 150;
      work->hdd_cnt = 0;
      break;

    case 2:  /* 持続時間 1.0秒 */
      work->hdd_time = 300;
      work->hdd_cnt = 0;
      break;

    case 9:  /* 無限連続       */
      work->hdd_time = -1;
      work->hdd_cnt  = 0;
      break;
    }
}


/*
 * カーソル位置に文字列を出力する
 * 非0 を返した場合は,一旦フレームを終了する
 */
static char * putstr(Work * work, char * str)
{
  int doing = 0;
  char * ptr = str;

  /* wait_time が残っている間,出力を抑止する。 */
  if(work->wait_time > 0)
    {
      if((work->wait_time -= TIME_BASE) < 0)
	work->wait_time = 0;
      return ptr;
    }

  /*
   * doing が 非0 になるまで繰り返す
   */
  while(!doing && *ptr)
    switch(*ptr)
      {
      case '\\':  /* エスケープシーケンス開始 */
	ptr++;
	switch(*ptr)
	  {
	  case 'L':   /* クリア   */
	    ptr++;
	    clear(work);
	    work->cursor_x = work->cursor_y = 0;
	    doing = 1;
	    work->now_attrib = DEF_ATTRIB;
	    break;
	  case 'C':   /* 色変え   */
	    ptr++;
	    if((*ptr >= '0') && (*ptr <= '7'))
	      work->now_attrib = (*ptr - '0') << 8;
	    ptr++;
	    break;
	  case 'W':   /* 時間待ち */
	    ptr++;
	    {
	      int t = 0;
	      
	      while(*ptr != '\\')
		{
		  if(*ptr < '0' || *ptr > '9') break;
		  t = t * 10;
		  t = t + *ptr - '0';
		  ptr++;
		}
	      ptr++;
	      work->wait_time = t;
	      doing = 1;
	    }
	    break;
	  case 'S':   /* SE 発生 */
	    ptr++;
	    {
	      int snd = 0;
	      
	      while(*ptr != '\\')
		{
		  if(*ptr < '0' || *ptr > '9') break;
		  snd = snd * 10;
		  snd = snd + *ptr - '0';
		  ptr++;
		}
	      ptr++;
	      snd = get_sound(snd);
	      GM_SdSet(snd);
	    }
	    break;
	  case 'H':   /* HDD 音パターン切替え */
	    ptr++;
	    hdd_mode(work, *ptr - '0');
	    ptr++;
	    break;
	  }
	break;
      case '\n':      /* 改行     */
	work->cursor_x = 0;
	work->cursor_y ++;
	if(work->cursor_y >= TEXT_HEIGHT)
	  {
	    roll_up(work);
	    work->cursor_y = TEXT_HEIGHT - 1;
	    doing = 1;  /* roll_up を伴う改行の場合, 一旦フレームを終らせる */
	  }
	ptr++;
	break;
      case '\t':      /* 水平タブ */
	ptr++;
	break;
      default:
	put_char(work, *ptr);
	ptr++;
	break;
      }

  if(!*ptr) return NULL;  /* NULL が返ったら、そのテキストは終了 */
  return ptr;
}

static void text_disp(Work * work)
{
  char * next;

  next = putstr(work, work->text);
  if(NULL == next)
    GV_CallParentSignalFunc(work, CON_SIGNAL_FINISH, 0);
  work->text = next;
  
}

static void Act(Work * work)
{
  enum {
    STEP_INIT,
    STEP_DISP
  };

  /* HDD SE処理(SE が停止していない限り実行) */
  if(work->hdd_rep >= 0)
    {
      work->hdd_cnt += TIME_BASE;
      if((work->hdd_cnt < work->hdd_time) || (work->hdd_time < 0))
	{
	  work->hdd_rep += TIME_BASE;
	  if(work->hdd_rep >= 10)
	    {
	      work->hdd_rep -= 10;
	      GM_SdSet(SD_A_PC_HDSEK);
	    }
	}
    }

  switch(work->step)
    {
    case STEP_INIT:   /* 初期化 */
      {
	float x, y, w, h;

	GM_SdSet(SD_A_PC_BOOT1);

	work->region = L2D_GetObject(work->l2d_handle, PHOTO_Console);

	x = work->region->box.rect.begin.x;
	y = work->region->box.rect.begin.y;

	w = work->region->box.rect.end.x - x;
	h = work->region->box.rect.end.y - y;

	/* 取得した座標とサイズは2D表示系用仮想座標系なので、
	   物理座標系に修正する */
#if 0 //BP def def PSX2
	work->begin_x = 2048.0F - DRAW_WIDTH / 2 +
	  x * DRAW_WIDTH  / SPR_SCRN_WIDTH ;

	work->begin_y = 2048.0F - DRAW_HEIGHT / 2 +
	  y * DRAW_HEIGHT / SPR_SCRN_HEIGHT;
#else
	/* XBOX は左上が (0,0) */
	work->begin_x = x * DRAW_WIDTH  / SPR_SCRN_WIDTH ;
	work->begin_y = y * DRAW_HEIGHT / SPR_SCRN_HEIGHT;
#endif
	
	work->width   = w * DRAW_WIDTH  / SPR_SCRN_WIDTH;
	work->height  = h * DRAW_HEIGHT / SPR_SCRN_HEIGHT;

	/* 文字の配置単位 */
	work->pitch_x = work->width  / (float)TEXT_WIDTH;
	work->pitch_y = work->height / (float)TEXT_HEIGHT;

	printf("pitch_x = %f  pitch_y = %f\n", work->pitch_x, work->pitch_y);
	
	work->step = STEP_DISP;
      }
      break;

    case STEP_DISP:
      text_disp(work);    /* 指定されたテキストの表示 */
      
      make_packet(work);  /* テキスト表示パケットの生成 */
      break;
    }

  /* テキストが終端に行き付いたら終了 */
  if(NULL == work->text)
    {
      GV_DestroyActor(work);
    }
  else
    if(GV_PadDataDirect[0].press & PAD_CANCEL)
      {
	/* 途中キャンセル時は途中で終る */
	GV_CallParentSignalFunc(work, CON_SIGNAL_FINISH, 1);
	GV_DestroyActor(work);
      }
}

static void Die(Work * work)
{
  GM_SdSet(SE_JOUCHUU_OFF);  /* SE 中断 */
  L2D_ReleaseLayout(work->l2d_handle);  /* レイアウト開放 */
  DG_DequeueDmapack(work->dmapack);
  DG_FreeDmapack(work->dmapack);
#if 0 //BP def def PSX2
  codecDelayedFree(work->packet[0]);
#else
  codecDelayedFree(work->packet_mem);
#endif
}

static int CreateDMApacket(Work * work, text_draw * packet, int side)
{
#if 0 //BP def def PSX2
  /*
   * テクスチャ転送部書き込み
   */
  DG_WriteTextureChangePacks(&(packet->tex_trans),
			     &(work->tex_list->tex_packet[side]));

  packet->dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_CNT, QSIZ(struct gif_blk));
  packet->dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(QSIZ(struct gif_blk), 0);


  /*
   * 表示準備
   */
  packet->gif.gif_packed.tag = SCE_GIF_SET_TAG(QSIZ(struct dat_packed), 1,
					       0, 0, 0, 1);
  packet->gif.gif_packed.regs = GS_REGS_AD;

  packet->gif.packed.alpha.reg  = SCE_GS_ALPHA_1;
  packet->gif.packed.alpha.data = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);

  packet->gif.packed.tex2.reg   = SCE_GS_TEX2_1;
  packet->gif.packed.tex2.data  = work->dgtex->tex_trans.tex2.data;


  /*
   * 表示条件定義
   */
  packet->gif.gif_reglist.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 8);
  packet->gif.gif_reglist.regs = 0x10685510;

  packet->gif.reglist.curs_prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
						  0, 0, 0, 1, 0, 1, 0, 0);
  packet->gif.reglist.curs_rgba = SCE_GS_SET_RGBAQ(CUR_R, CUR_G, CUR_B, 0, 0);
  packet->gif.reglist.curs_xyz0 = SCE_GS_SET_XYZ(0, 0, -1);
  packet->gif.reglist.curs_xyz1 = SCE_GS_SET_XYZ(0, 0, -1);

  packet->gif.reglist.clamp = SCE_GS_SET_CLAMP(0, 0, 0, 0, 0, 0);
  packet->gif.reglist.tex0 = work->dgtex->tex_trans.tex0.data;
  packet->gif.reglist.prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
					     0, 1, 0, 1, 0, 1, 0, 0);
  packet->gif.reglist.rgba = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);


  /*
   * 初期状態では表示テキスト無し
   */
  /* 何もせずにリターン */
  packet->dma_text.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, 0);
  packet->dma_text.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dma_text.vifcode[1] = SCE_VIF1_SET_NOP(0);
#endif

  return 0;
}


static int GetResources(Work * work,
			int l2d_name,
			int text_name,
			int pause_level)
{
   float safeZoneOffsetY = 0;

   /*
   * 画面装飾の l2d をロード
   */
  work->blink_cnt = 0;
  {
    void * ptr;

    if(NULL == (ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'))))
      ptr = CDC_GetFileEntry(l2d_name, 'o');
    work->l2d_handle = L2D_SetupLayout2(ptr, 4, 0, SPR_FLAG_PRIV, pause_level, safeZoneOffsetY);
    ASSERT(work->l2d_handle >= 0);
  }

  /* デフォルトアクションを実行(他にアクションは無いものとする) */
  L2D_EvokeAction(work->l2d_handle, CON_DefaultAction);
  work->step = 0;

  work->now_attrib = DEF_ATTRIB;  /* デフォルトの属性に設定 */

  work->wait_time  = 0;           /* 初期には時間待ちしない */

  /*
   * テキストデータの位置を得る
   */
  if(NULL == (work->text = GV_GetCache(GV_CacheID(text_name, 'r'))))
    work->text = CDC_GetFileEntry(text_name, 'r');

  /* テキストバッファをクリア */
  {
    int i;

    for(i = 0; i < MAX_CHAR; i++) work->text_buf[i] = ' ';
  }

  /* DMA パケットの準備 */

  {
    DG_DMAPACK * dmapack;

#if 0 //BP def def PSX2
    work->packet[0] = codecMalloc(sizeof(text_draw) * 2);
    ASSERT(work->packet != NULL);
    work->packet[1] = work->packet[0] + 1;
#endif

    /* テクスチャの準備 */
    work->tex_list = DG_GetTextureList(TEXT_photo_font);
    ASSERT(NULL != work->tex_list);
    work->dgtex = DG_GetTexture2(TEXT_photo_font, TEXT_densou_font_alp_ovl);
    ASSERT(NULL != work->dgtex);
    {
      SPR_FIX w, h;

      w = SPR_FIXED(1 << ((work->dgtex->tex_trans.tex0.data >> 26) & 0x0f));
      h = SPR_FIXED(1 << ((work->dgtex->tex_trans.tex0.data >> 30) & 0x0f));

#if 0 //BP def def PSX2
      work->base_u = work->dgtex->u_offset * w;
      work->base_v = work->dgtex->v_offset * h;
#else
	  /* XBOXのUVは(0.0f,0.0f)->(1,0f,1.0f) */
      work->base_u = 0 ;
      work->base_v = 0 ;
	  work->uv_width  = w ;
	  work->uv_height = h ;
#endif
    }

    dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
			     DG_DMAPACK_PHASE_LAST);
    ASSERT(NULL != dmapack);
    work->dmapack = dmapack;
#if 0 //BP def def PSX2
    CreateDMApacket(work, work->packet[0], 0);
    CreateDMApacket(work, work->packet[1], 1);
    dmapack->packet[0] = work->packet[0];
    dmapack->packet[1] = work->packet[1];
#else
	if ( !(work->packet_mem = codecMalloc( CONSOLE_PACKET_SIZE )) ) {
		printf("ERR!! MALLOC phototerm/console.c!!\n");
		return -1 ;
	}
	work->prim = work->dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( work->prim ) ;
#endif
    DG_QueueDmapack(dmapack);
  }
  return 0;
}

void * NewConsoleDemo(int l2d_name, int text_name, int pause_level)
{
  Work * work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0);
  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, l2d_name, text_name, pause_level))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
