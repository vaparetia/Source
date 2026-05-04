/*
  memcall.c
  メモリーコールモード関連の名前表示および選択
  2000/09/13

  $Id: memcall.c,v 1.1.1.3 2002/11/19 11:45:08 Yoshizawa1 Exp $
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


#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)  printf(args)
#endif /* _DEBUG_ */


#define TIME_WIDE   30
#define TIME_PART   10
#define TIME_PULL   40

#define TEX_BASE    (ZBUFFER_PAGE()/64)
#define TEX_WIDTH   (BUFFER_WIDTH / 64)
#define CLUT_BASE   (0x100000/64-3)
#define CLUT_WIDTH  (64 / 64)

typedef enum {
  MEM_CLOSE,   /* 閉じ状態         */
  MEM_WIDE,    /* 左右に伸びる工程 */
  MEM_PART,    /* 上下に別れる工程 */
  MEM_PULL,    /* 上下に伸びる工程 */
  MEM_OPEN,    /* 開き状態         */

  MEM_PUSH,    /* 上下に縮む工程   */
  MEM_FIT,     /* 上下を合わせる工程 */
  MEM_SHRINK   /* 左右に縮む工程   */
} MemCallStat;


typedef struct {
  DG_DMATAG  dmatag0;
  DG_GIFTAG  giftag0;
  DG_GSREG   bitbltbuf;
  DG_GSREG   trxpos;
  DG_GSREG   trxreg;
  DG_GSREG   trxdir;
 
  DG_GIFTAG  giftag1;
  DG_DMATAG  dmatag1;
} ClutPacket ALIGN16;

typedef struct {
  DG_DMATAG  dmatag0;
  DG_GIFTAG  giftag0;
  DG_GSREG   gsregs[4];
  DG_GIFTAG  giftag1;
  DG_DMATAG  dmatag1;
} TexPacket ALIGN16;

typedef struct {
  ClutPacket clut_trans;  /* CLUT 転送 */
  TexPacket  tex_trans;   /* Texture 転送 */

  /* 描画 */
  DG_DMATAG  dmatag;
  struct lbl_gif {
    DG_GIFTAG  giftag;
    struct lbl_data {
      DG_GSREG   texflush;
      DG_GSREG   alpha;
      DG_GSREG   colclamp;
      DG_GSREG   tex0;
      DG_GSREG   tex1;
      DG_GSREG   prim;
      
      DG_GSREG   rgbq0;
      DG_GSREG   uv0;
      DG_GSREG   xyz0;
      
      DG_GSREG   rgbq1;
      DG_GSREG   uv1;
      DG_GSREG   xyz1;
    } data;
  } gif;

} LabelPacket ALIGN16;


typedef struct {
  SPR_OBJ  * freq_base;   /* 周波数表示の下地 */

  SPR_OBJ  * freq_pos;   /* 周波数表示の親オブジェクト */
  SPR_OBJ  * freq14;     /* "14" の表示                */
  SPR_OBJ  * freq1_00;   /* 1の位                      */
  SPR_OBJ  * freq0_10;   /* 0.1の位                    */
  SPR_OBJ  * freq0_01;   /* 0.01の位                   */
  SPR_OBJ  * freq_dot;   /* 小数点                     */

  /* 製品版では、名前を字幕として表示する */
  FONT_VRAMINFO vinfo;          /* 文字表示用情報                   */
  void        * bmp;            /* 文字表示ビットマップイメージ     */
  int           width, height;  /* 文字表示エリアピクセルサイズ     */
  int           bmpsiz;         /* ビットマップのデータサイズ       */

  int           disp_x, disp_y; /* 表示位置                         */
  int           disp_w, disp_h; /* 表示サイズ                       */

  DG_DMAPACK  * dmapack;        /* 字幕転送 DMAPACK 構造体          */
  LabelPacket * packet;         /* 字幕転送パケット本体             */

} NameLabel;


typedef struct {
  SPR_OBJ  * base;
  SPR_OBJ  * l_line;
  SPR_OBJ  * r_line;

  NameLabel  l_label;
  NameLabel  r_label;
} UnderLine;

typedef struct Work {
  GV_ACT_EX   actor;

  MemCallStat   status;
  int           cnt;

  SPR_OBJ     * parent;

  SPR_OBJ     * upper_parent;   /* 上枠の表示位置基準点     */
  SPR_OBJ     * upper_lstrip;   /* 上枠                     */

  SPR_OBJ     * lower_parent;   /* 下枠の表示位置基準点      */
  SPR_OBJ     * lower_lstrip;   /* 下枠                      */
  NameLabel     lower_l_label;  /* 下枠に従属する周波数/名前 */
  NameLabel     lower_r_label;  /* 下枠に従属する周波数/名前 */
  
  SPR_OBJ     * uline_parent;   /* 全てのアンダーラインの親  */
  UnderLine     under_lines[3]; /* アンダーライン            */

  int           tx_handle;      /* 使用するテクスチャのハンドル */

  int           selected;       /* 選択されている項目の番号  */

#ifdef TRIAL
  SPR_OBJ     * freq_plate;
#endif
} Work;

typedef struct MemcallList {
  int res_name;   /* リソース番号による名前 */
  int freq;       /* 周波数                 */
  int registed; /* 登録済フラグ           */
} MemcallList;

static Work * now_work = NULL;

/* メモリーコールリスト */
static MemcallList  memcall_list[8] = {
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0},
  { 0, 0, 0}
};

static SPR_POS pos_list[2][4] = {
  {
    {MC_UPPER_X1, MC_UPPER_Y2},
    {MC_UPPER_X1, MC_UPPER_Y1},
    {MC_UPPER_X2, MC_UPPER_Y1},
    {MC_UPPER_X2, MC_UPPER_Y2}
  },
  {
    {MC_LOWER_X1, MC_LOWER_Y2},
    {MC_LOWER_X1, MC_LOWER_Y1},
    {MC_LOWER_X2, MC_LOWER_Y1},
    {MC_LOWER_X2, MC_LOWER_Y2}
  }
};

/* 名前表示用の CLUT */
static unsigned int _text_clut[32] ALIGN16;

static void memcall_set_name(Work * work, int pos, int name_res, int freq);
static void set_name_blight(Work * work, int pos, int status);

int MemcallSetName(int pos, int name_res, int freq)
{
  if((pos < 0) || (pos > 7)) return -1;
  DBG("Registed: area %d   ResourceID %d  Freq %d\n", pos, name_res, freq);
  memcall_list[pos].res_name = name_res;
  memcall_list[pos].freq     = freq;
  memcall_list[pos].registed = 1;
  if(now_work != NULL)
    {
      memcall_set_name(now_work, pos, name_res, freq);
      set_name_blight(now_work, pos, 0);
    }
  return 0;
}

static void setup_clut_trans(ClutPacket * clut)
{
  static ClutPacket def_clut_trans = {
    dmatag0:
    {
      qwc: DMATAG_SET_QWC(DMATAG_ID_CNT, 6),
      vifcode:
      {
	SCE_VIF1_SET_NOP(0),
	SCE_VIF1_SET_DIRECT(6, 0)
      }
    },
    giftag0:
    {
      tag: SCE_GIF_SET_TAG(4, 0, 0, 0, SCE_GIF_PACKED, 1),
      regs: GS_REGS_AD
    },
    bitbltbuf:
    {
      reg: SCE_GS_BITBLTBUF,
      data: SCE_GS_SET_BITBLTBUF(0, 0, 0, CLUT_BASE, 1, SCE_GS_PSMCT32)
    },
    trxpos:
    {
      reg: SCE_GS_TRXPOS,
      data: SCE_GS_SET_TRXPOS(0, 0, 0, 0, 0) /* (0,0) に左上→右下転送 */
    },
    trxreg:
    {
      reg: SCE_GS_TRXREG,
      data: SCE_GS_SET_TRXREG(8, 2)
    },
    trxdir:
    {
      reg: SCE_GS_TRXDIR,
      data: SCE_GS_SET_TRXDIR(0)
    },
    
    giftag1:
    {
      tag: SCE_GIF_SET_TAG( 4, 1, 0, 0, SCE_GIF_IMAGE, 0),
      regs: 0x00000000
    },
    
    /* CLUT イメージ転送 */
    dmatag1:
    {
      qwc: DMATAG_SET_QWC(DMATAG_ID_REF, 4),
      addr: _text_clut,
      vifcode:
      {
	SCE_VIF1_SET_NOP(0),
	SCE_VIF1_SET_DIRECT(4, 0)
      }
    }
  };

  *clut = def_clut_trans;
}

static void setup_tex_trans(TexPacket * tex, void * bmp, int width, int height)
{
  DG_MakeLoadImagePacket(tex, SCE_GS_PSMT4HH,
			 width, height, TEX_BASE * 64, TEX_WIDTH * 64, bmp);
}

#define Qsize(_v)   (sizeof(_v) / sizeof(u_long128))


/*
 * 名前表示字幕部の転送パケットを、XYZ2 レジスタ以外全て設定する
 */
static void setup_trans_packet(NameLabel * label, LabelPacket * packet, void *bmp)
{
  /* CLUT 転送部の設定 */
  setup_clut_trans(&packet->clut_trans);

  /* Texture 転送部の設定 */
  setup_tex_trans(&packet->tex_trans, label->bmp, label->width, label->height);

  /* 描画部の設定 */
  packet->dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, Qsize(struct lbl_gif));
  packet->dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(Qsize(struct lbl_gif), 0);

  packet->gif.giftag.tag =
    SCE_GIF_SET_TAG(Qsize(struct lbl_data), 1, 0, 0, 0, 1);
  packet->gif.giftag.regs = GS_REGS_AD;

  /* テクスチャを必ずフラッシュすること */
  packet->gif.data.texflush.reg = SCE_GS_TEXFLUSH;
  packet->gif.data.texflush.data = 1;

  packet->gif.data.alpha.reg = SCE_GS_ALPHA_1;
  packet->gif.data.alpha.data = SCE_GS_SET_ALPHA(0, 2, 2, 1, 100);

  packet->gif.data.colclamp.reg = SCE_GS_COLCLAMP;
  packet->gif.data.colclamp.data = 1;

  packet->gif.data.tex0.reg = SCE_GS_TEX0_1;
  packet->gif.data.tex0.data = SCE_GS_SET_TEX0(TEX_BASE, TEX_WIDTH,
					       SCE_GS_PSMT4HH,
					       9, 8, 1, 0,
					       CLUT_BASE, SCE_GS_PSMCT32,
					       0, 0, 2);
  
  packet->gif.data.tex1.reg = SCE_GS_TEX1_1;
  packet->gif.data.tex1.data = SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0);
  
  packet->gif.data.prim.reg = SCE_GS_PRIM;
  packet->gif.data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
					       0, 1, 0, 1, 0, 1, 0, 0);
  packet->gif.data.rgbq0.reg = SCE_GS_RGBAQ;
  packet->gif.data.rgbq0.data =
    SCE_GS_SET_RGBAQ(MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 0x80, 0);

  packet->gif.data.uv0.reg = SCE_GS_UV;
  packet->gif.data.uv0.data = SCE_GS_SET_UV(8, 8);

  packet->gif.data.rgbq1.reg = SCE_GS_RGBAQ;
  packet->gif.data.rgbq1.data =
    SCE_GS_SET_RGBAQ(MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 0x80, 0);

  packet->gif.data.uv1.reg = SCE_GS_UV;
  packet->gif.data.uv1.data = SCE_GS_SET_UV(label->width * 16 + 8,
					    label->height * 16 + 8);

  packet->gif.data.xyz0.reg = SCE_GS_XYZ2;
  packet->gif.data.xyz1.reg = SCE_GS_XYZ2;
}

/*
 * 名前表示の座標を設定する
 */
static void setup_name_position(NameLabel * label, int side, int line)
{
  LabelPacket * packet;
  int y_pos[] = {
    MC_ABS_LEVEL_0, MC_ABS_LEVEL_1,
    MC_ABS_LEVEL_2, MC_ABS_LEVEL_3
  };
    
  int x, y;
  u_long xyz0, xyz1;

  x = (!side) ? MC_ABS_LEFT_X : MC_ABS_RIGHT_X;
  y = y_pos[line];

  x += 2048 - DRAW_WIDTH / 2;
  y += 2048 - DRAW_HEIGHT / 2;
  
  packet = label->packet;

  xyz0 = SCE_GS_SET_XYZ(x * 16, y * 16, 1);
  xyz1 = SCE_GS_SET_XYZ((x + xV2P(NAME_LABEL_W)) * 16,
			(y + yV2P(NAME_LABEL_H)) * 16, 1);

  packet[0].gif.data.xyz0.data = xyz0;
  packet[1].gif.data.xyz0.data = xyz0;

  packet[0].gif.data.xyz1.data = xyz1;
  packet[1].gif.data.xyz1.data = xyz1;
}



/*
 * 周波数/名前表示を初期化する
 */
static void setup_name_label(NameLabel * label, int side, SPR_OBJ * parent)
{
  SPR_POS pos;

  label->freq_base = SPR_Create_2D_Object(SP_SPRITE, -1, parent);

  /*
   * 周波数表示の下地を用意
   */
  pos.x = (!side) ? MC_LEFT_FREQ_X : MC_RIGHT_FREQ_X;
  pos.y = MC_FREQ_Y;

  /* 位置、サイズ決定 */
  SPR_SetPosSprite(label->freq_base, &pos);
  SPR_SetSizeSprite(label->freq_base, MC_FREQ_W, MC_FREQ_H);

  /* RGB 値指定 */
  label->freq_base->sprite.col.r = MC_FREQ_DARK_R;
  label->freq_base->sprite.col.g = MC_FREQ_DARK_G;
  label->freq_base->sprite.col.b = MC_FREQ_DARK_B;
  label->freq_base->sprite.col.a = MC_FREQ_DARK_A;

  /* 表示属性設定 */
  label->freq_base->head.alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 64);
  label->freq_base->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
  SPR_SetPriority(label->freq_base, MEMCALL_PRIORITY);

  /*
   * 周波数表示そのものを用意
   */

  /*
   * 名前を表示する字幕を用意
   */
  label->bmpsiz = NAME_LABEL_W * NAME_LABEL_H * 4 / 8;
  label->width  = NAME_LABEL_W;
  label->height = NAME_LABEL_H;

  label->bmp = codecMalloc(label->bmpsiz);
  ASSERT(label->bmp != NULL);

  /* 表示域をクリア */
  memset((char*)label->bmp, 0x00, label->bmpsiz);

  font_set_vraminfo( &label->vinfo,
		     label->bmp,
		     label->width, label->height, 0, 12, 0);

  label->packet = codecMalloc(sizeof(LabelPacket) * 2);
  ASSERT(label->packet != NULL);

  /* 転送パケットを初期化 */
  setup_trans_packet(label, &label->packet[0], label->bmp);
  setup_trans_packet(label, &label->packet[1], label->bmp);

  /* 名前表示用字幕の DMA 転送準備 */
  label->dmapack = DG_MakeDmapack2(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
				   DG_DMAPACK_PHASE_LAST, 130);
  ASSERT(label->dmapack != NULL);

  label->dmapack->packet[0] = &label->packet[0];
  label->dmapack->packet[1] = &label->packet[1];
  
  DG_QueueDmapack(label->dmapack);

  /* 指示があるまで表示しない */
  label->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;

  SPR_HIDE(label->freq_base);
}

static void release_name_label(NameLabel * label)
{
  SPR_Destroy_2D_Object(label->freq_base);
  DG_DequeueDmapack(label->dmapack);
  DG_FreeDmapack(label->dmapack);
  codecFree(label->packet);
  codecFree(label->bmp);
}

static void hide_name_label(NameLabel * label)
{
  SPR_HIDE(label->freq_base);
  label->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
}

static void show_name_label(NameLabel * label)
{
  SPR_SHOW(label->freq_base);
  label->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
}


static void setup_label_name(NameLabel * label, char * name)
{
  FONT_DRAWINFO dr;

  memset((char *)label->bmp, 0x00, label->bmpsiz);
  if(name != NULL)
    {
      font_open_drawinfo(&dr, &label->vinfo);
      font_set_locate(&dr, 0, RUBI_HEIGHT);
      font_draw_string(&dr, name);
    }
}

static void setup_underline(UnderLine * ul, float y, SPR_OBJ * parent)
{
  SPR_POS pos, begin, end;
  u_long   alpha;

  /* アンダーラインオブジェクトの生成 */
  ul->base = SPR_Create_2D_Object(SP_EMPTY, -1, parent);
  ul->l_line = SPR_Create_2D_Object(SP_LINE, -1, ul->base);
  ul->r_line = SPR_Create_2D_Object(SP_LINE, -1, ul->base);

  /* 周波数表示プレート */
  setup_name_label(&ul->l_label, 0, ul->base);
  setup_name_label(&ul->r_label, 1, ul->base);

  hide_name_label(&ul->l_label);
  hide_name_label(&ul->r_label);


  /* プライオリティの設定 */
  SPR_SetPriority(ul->l_line, MEMCALL_PRIORITY);
  SPR_SetPriority(ul->r_line, MEMCALL_PRIORITY);
  
  /* 頂点色設定 */
  SPR_SetColorLine(ul->l_line, 0, MC_LINE_R, MC_LINE_G, MC_LINE_B, MC_LINE_A);
  SPR_SetColorLine(ul->l_line, 1, MC_LINE_R, MC_LINE_G, MC_LINE_B, MC_LINE_A);

  SPR_SetColorLine(ul->r_line, 0, MC_LINE_R, MC_LINE_G, MC_LINE_B, MC_LINE_A);
  SPR_SetColorLine(ul->r_line, 1, MC_LINE_R, MC_LINE_G, MC_LINE_B, MC_LINE_A);

  /* アンダーラインオブジェクトの表示位置等指定 */
  pos.x = 0, pos.y = y;
  SPR_SetPosEmpty(ul->base, &pos);

  begin.x = MC_LEFT_ULINE_X1;
  end.x   = MC_LEFT_ULINE_X2;
  begin.y = end.y = 0.0F;
  SPR_SetPosLine(ul->l_line, &begin, &end);

  begin.x = MC_RIGHT_ULINE_X1;
  end.x   = MC_RIGHT_ULINE_X2;
  begin.y = end.y = 0.0F;
  SPR_SetPosLine(ul->r_line, &begin, &end);

  alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 64);
  ul->l_line->head.alpha = alpha;
  ul->r_line->head.alpha = alpha;

  ul->l_line->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
  ul->r_line->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;

  SPR_SHOW(ul->l_line);
  SPR_SHOW(ul->r_line);
  SPR_SHOW(ul->base);
}

static void set_pos_under_line(UnderLine * ul, float x, float y)
{
  SPR_POS pos;

  pos.x = x;
  pos.y = y;
  SPR_SetPosEmpty(ul->base, &pos);
}

static void hide_freq_and_name(Work * work)
{
  int i;

  for(i = 0; i < 3; i++)
    {
      hide_name_label(&work->under_lines[i].l_label);
      hide_name_label(&work->under_lines[i].r_label);
    }
  hide_name_label(&work->lower_l_label);
  hide_name_label(&work->lower_r_label);
}

static void show_freq_and_name(Work * work)
{
  int i;

  for(i = 0; i < 3; i++)
    {
      show_name_label(&work->under_lines[i].l_label);
      show_name_label(&work->under_lines[i].r_label);
    }
  show_name_label(&work->lower_l_label);
  show_name_label(&work->lower_r_label);
}

/* 現在登録されている名前を、メモリーコールのリストに追加する */
static void allregist_memcall_list(Work * work)
{
  int i;

  for(i = 0; i < 8; i++)
    {
      if(memcall_list[i].registed)
	{
	  DBG("MemCall Name Regist!!!!!!!!!!!!!!!!!\n");
	  memcall_set_name(work, i,
			   memcall_list[i].res_name,
			   memcall_list[i].freq);
	}
      else
	{
	  DBG("--------------------\n");
	  memcall_set_name(work, i, -1, 0);
	}
      set_name_blight(work, i, 0);
    }
}


/* 必要なオブジェクトを用意する */
static void create_memcall_objects(Work * work)
{
  int i;
  u_long alpha;

  alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 64);

  /*
   * メモリーコールモード全ての親となるオブジェクトの作成
   */
  work->parent = SPR_Create_2D_Object(SP_EMPTY, 4, NULL);
  /* 上下枠の移動基準点となる Empty オブジェクトを生成する */
  work->upper_parent = SPR_Create_2D_Object(SP_EMPTY, -1, work->parent);
  work->lower_parent = SPR_Create_2D_Object(SP_EMPTY, -1, work->parent);
  /* アンダーラインの親になる Empty オブジェクトを作成する */
  work->uline_parent = SPR_Create_2D_Object(SP_EMPTY, -1, work->parent);


  /* オブジェクト本体 */
  work->upper_lstrip = SPR_Create_2D_Object(SP_LINESTRIP, -1, work->upper_parent);
  work->lower_lstrip = SPR_Create_2D_Object(SP_LINESTRIP, -1, work->lower_parent);

  SPR_SetPriority(work->upper_lstrip, MEMCALL_PRIORITY);
  SPR_SetPriority(work->lower_lstrip, MEMCALL_PRIORITY);

  setup_underline(&work->under_lines[0], MC_ULINE_Y0, work->uline_parent);
  setup_underline(&work->under_lines[1], MC_ULINE_Y1, work->uline_parent);
  setup_underline(&work->under_lines[2], MC_ULINE_Y2, work->uline_parent);

  setup_name_label(&work->lower_l_label, 0, work->lower_parent);
  setup_name_label(&work->lower_r_label, 1, work->lower_parent);

  /* 名前の表示位置を設定 */
  for(i = 0; i < 3; i++)
    {
      setup_name_position(&work->under_lines[i].l_label, 0, i);
      setup_name_position(&work->under_lines[i].r_label, 1, i);
    }
  setup_name_position(&work->lower_l_label, 0, 3);
  setup_name_position(&work->lower_r_label, 1, 3);
#ifdef TRIAL   /* 体験版暫定処理: 名前と周波数の表示はテクスチャを用いる */
  /* 周波数プレート */
  work->freq_plate = SPR_Create_2D_Object(SP_SPRITE, -1, work->under_lines[0].l_label.freq_base);
  SPR_SetPosSprite(work->freq_plate, &(SPR_POS){0.0F, 0.0F});
  SPR_SetSizeSprite(work->freq_plate, MC_FREQ_W, MC_FREQ_H);
  SPR_ObjSetTexture(work->freq_plate, GV_StrCode("num_alp_ovl"), work->tx_handle);
  SPR_SetPriority(work->freq_plate, MEMCALL_PRIORITY + 1);
  work->freq_plate->head.alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 100);
  work->freq_plate->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
  work->freq_plate->sprite.col.r = 128;
  work->freq_plate->sprite.col.g = 128;
  work->freq_plate->sprite.col.b = 128;
  work->freq_plate->sprite.col.a = 128;
  SPR_SHOW(work->freq_plate);
#endif /* TRIAL */

#if 0
  /* オタコンの名前を得る */
  {
    char * name;

    /* 日本語なら「オタコン」、英語なら "Otacon" */
    name = GM_GetResource(4, 0);
    setup_label_name(&work->under_lines[0].l_label, name);
  }
#endif

  allregist_memcall_list(work);


  /* 親オブジェクトの位置設定 */
  SPR_SetPosEmpty(work->parent, &(SPR_POS){MC_disp_x, MC_disp_y});

  /* 基準点の座標を設定する */
  SPR_SetPosEmpty(work->upper_parent, &(SPR_POS){0.0F, MC_UPPER_Y});
  SPR_SetPosEmpty(work->lower_parent, &(SPR_POS){0.0F, MC_LOWER_Y});

  /*
   * 各行アンダーライン作成
   */
  SPR_SetPosEmpty(work->uline_parent, &(SPR_POS){0.0F, 0.0F});

  /* linestrip 頂点数設定 */
  SPR_SetLineStripVertexNumber(work->upper_lstrip, 4);
  SPR_SetLineStripVertexNumber(work->lower_lstrip, 4);

  /* linestrip 色設定 */
  for(i = 0; i < 4; i++)
    {
      SPR_SetColorLineStrip(work->upper_lstrip, i,
			  MC_LINE_R, MC_LINE_G, MC_LINE_B, MC_LINE_A);
      SPR_SetColorLineStrip(work->lower_lstrip, i,
			  MC_LINE_R, MC_LINE_G, MC_LINE_B, MC_LINE_A);
    }
  work->upper_lstrip->head.alpha = alpha;
  work->lower_lstrip->head.alpha = alpha;
  work->upper_lstrip->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
  work->lower_lstrip->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;

  SPR_SHOW(work->upper_lstrip);
  SPR_SHOW(work->lower_lstrip);
  SPR_SHOW(work->upper_parent);
  SPR_SHOW(work->lower_parent);
  SPR_SHOW(work->uline_parent);
  
  hide_freq_and_name(work);
  
  /* 親オブジェクトのレベルで、全体を非表示にする */
  SPR_HIDE(work->parent);
}

/*
 * 閉じ状態を設定する
 */
static void setup_closed(Work * work)
{
  static SPR_POS ver_list[] = {
    { 0.0F, 0.0F},
    { 0.0F, 0.0F},
    { 0.0F, 0.0F},
    { 0.0F, 0.0F}
  };

  /* 閉じ状態では全てを表示しない */
  SPR_HIDE(work->parent);

  /* linestrip の座標設定 */
  SPR_SetPosLineStrip(work->upper_lstrip, 0, 4, ver_list);
  SPR_SetPosLineStrip(work->upper_lstrip, 0, 4, ver_list);

  work->status = MEM_CLOSE;
}

/*
 * 開き状態を設定する
 */
static void setup_opened(Work * work)
{
  SPR_SHOW(work->upper_parent);
  SPR_SHOW(work->lower_parent);
  SPR_SHOW(work->uline_parent);
  
  show_freq_and_name(work);

  /* linestrip 頂点座標指定 */
  SPR_SetPosLineStrip(work->upper_lstrip, 0, 4, pos_list[0]);
  SPR_SetPosLineStrip(work->lower_lstrip, 0, 4, pos_list[1]);

  /* 上下枠位置設定 */
  SPR_SetPosEmpty(work->upper_parent, &(SPR_POS){0.0F, MC_UPPER_Y});
  SPR_SetPosEmpty(work->lower_parent, &(SPR_POS){0.0F, MC_LOWER_Y});

  /* アンダーライン位置設定 */
  set_pos_under_line(&work->under_lines[0], 0.0F, MC_ULINE_Y0);
  set_pos_under_line(&work->under_lines[1], 0.0F, MC_ULINE_Y1);
  set_pos_under_line(&work->under_lines[2], 0.0F, MC_ULINE_Y2);

  work->status = MEM_OPEN;
}

/* 左右に延びるシーケンス途中を設定 */
static void setup_wide_sequence(Work * work, float rate)
{ 
  SPR_POS pos[4];
  int i;

  if(rate == 0.0F)
    {
      SPR_HIDE(work->parent);
      return;
    }
  /* 左右に伸びる間、上の枠だけが表示される。 */
  SPR_SHOW(work->parent);
  SPR_SHOW(work->upper_parent);

  SPR_HIDE(work->lower_parent);
  SPR_HIDE(work->uline_parent);

  hide_freq_and_name(work);

  pos[0].x = pos[1].x = MC_UPPER_X1 * rate;
  pos[2].x = pos[3].x = MC_UPPER_X2 * rate;

  /* 左右に伸びるシーケンスにおいては
     高さが 0 であるため、y には全て 0 を代入する */
  for(i = 0; i < 4; i++) pos[i].y = 0.0F;
  SPR_SetPosLineStrip(work->upper_lstrip, 0, 4, pos);

  /* 念のため、上枠の基準点を (0,0) に設定する */
  SPR_SetPosEmpty(work->upper_parent, &(SPR_POS){0.0F, 0.0F});
}

/* 上下にわかれるシーケンス途中を設定 */
static void setup_part_sequence(Work * work, float rate)
{
  SPR_POS pos[4];
  int i;
 
  SPR_SHOW(work->upper_parent);
  SPR_SHOW(work->lower_parent);
  SPR_HIDE(work->uline_parent);

  hide_freq_and_name(work);

  /* 上枠の頂点設定 */
  for(i = 0; i < 4; i++)
    {
      pos[i].x = pos_list[0][i].x;
      pos[i].y = pos_list[0][i].y * rate;
    }
  SPR_SetPosLineStrip(work->upper_lstrip, 0, 4, pos);

  /* 下枠の頂点設定 */
  for(i = 0; i < 4; i++)
    {
      pos[i].x = pos_list[1][i].x;
      pos[i].y = pos_list[1][i].y * rate;
    }
  SPR_SetPosLineStrip(work->lower_lstrip, 0, 4, pos);

  SPR_SetPosEmpty(work->upper_parent, &(SPR_POS){0.0F, 0.0F});
  SPR_SetPosEmpty(work->lower_parent, &(SPR_POS){0.0F, 0.0F});
}

/* 上下に延びるシーケンス途中を設定 */
static void setup_pull_sequence(Work * work, float rate)
{
  SPR_POS pos;

  /* 上下に伸びる工程においては上下枠およびアンダーラインがすべて表示される */
  SPR_SHOW(work->upper_parent);
  SPR_SHOW(work->lower_parent);
  SPR_SHOW(work->uline_parent);

  hide_freq_and_name(work);

  /* 上下枠の頂点位置設定 */
  SPR_SetPosLineStrip(work->upper_lstrip, 0, 4, pos_list[0]);
  SPR_SetPosLineStrip(work->lower_lstrip, 0, 4, pos_list[1]);

  /* 各パーツ表示位置設定 */
  pos.x = 0.0F;
 
  /* 上枠 */
  pos.y = MC_UPPER_Y * rate;
  SPR_SetPosEmpty(work->upper_parent, &pos);

  /* 下枠 */
  pos.y = MC_LOWER_Y * rate;
  SPR_SetPosEmpty(work->lower_parent, &pos);

  /* アンダーライン */
  set_pos_under_line(&work->under_lines[0], 0.0F, MC_ULINE_Y0 * rate);
  set_pos_under_line(&work->under_lines[1], 0.0F, MC_ULINE_Y1 * rate);
  set_pos_under_line(&work->under_lines[2], 0.0F, MC_ULINE_Y2 * rate);
}


static void setup_freq_obj_attrib(SPR_OBJ * obj,
				  int r, int g, int b, int a)
{
  obj->sprite.col.r = r;
  obj->sprite.col.g = g;
  obj->sprite.col.b = b;
  obj->sprite.col.a = a;

  /* 表示属性設定 */
  obj->head.alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 0x80);
  obj->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
  SPR_SetPriority(obj, MEMCALL_PRIORITY + 1);

}


static void memcall_set_name(Work * work, int pos, int name_res, int freq)
{
  NameLabel * label;
  char * name;

  DBG("entering memcall_set_name()\n");
  if(work == NULL) return;

  DBG("[1]\n");
  /* 指定された項目に、名前を設定する */
  if(pos < 6)
    label = (!(pos & 1))
      ? (&work->under_lines[pos / 2].l_label)
      : (&work->under_lines[pos / 2].r_label);
  else
    label = (!(pos & 1))
      ? &work->lower_l_label
      : &work->lower_r_label;

  DBG("[2] name_res = %d\n");

  if(name_res < 0)
    {
      label->freq_pos = NULL;
      return;
    }
  DBG("[3]\n");
  DBG("==================================================================\n");


  DBG("MemCall: area %d  ResourceID:%d\n", pos, name_res);
  name = GM_GetResource(4, name_res);

  setup_label_name(label, name);

  /* 指定された項目の周波数表示を作成する */
  label->freq_pos = SPR_Create_2D_Object(SP_EMPTY,  -1, label->freq_base);
  label->freq_dot = SPR_Create_2D_Object(SP_SPRITE, -1, label->freq_pos);
  label->freq14   = SPR_Create_2D_Object(SP_SPRITE, -1, label->freq_pos);
  label->freq1_00 = SPR_Create_2D_Object(SP_SPRITE, -1, label->freq_pos);
  label->freq0_10 = SPR_Create_2D_Object(SP_SPRITE, -1, label->freq_pos);
  label->freq0_01 = SPR_Create_2D_Object(SP_SPRITE, -1, label->freq_pos);

  setup_freq_obj_attrib(label->freq_dot, MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 128);
  setup_freq_obj_attrib(label->freq14,   MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 128);
  setup_freq_obj_attrib(label->freq1_00, MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 128);
  setup_freq_obj_attrib(label->freq0_10, MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 128);
  setup_freq_obj_attrib(label->freq0_01, MC_FREQ_R, MC_FREQ_G, MC_FREQ_B, 128);

  /* 周波数表示の表示座標を指定 */
  SPR_SetPosEmpty(label->freq_pos, &(SPR_POS){0.0F, 0.0F});

  /* 周波数表示の各文字の位置を指定 */
  SPR_SetPosSprite(label->freq14, &(SPR_POS){MC_FREQ_X_14, MC_FREQ_H * 0.05F});
  SPR_SetPosSprite(label->freq1_00, &(SPR_POS){MC_FREQ_X_1_00, 0.0F});
  SPR_SetPosSprite(label->freq_dot,
		   &(SPR_POS){MC_FREQ_X_dot,  MC_FREQ_H / 2.0F});
  SPR_SetPosSprite(label->freq0_10, &(SPR_POS){MC_FREQ_X_0_10, 0.0F});
  SPR_SetPosSprite(label->freq0_01, &(SPR_POS){MC_FREQ_X_0_01, 0.0F});
  
  /* 周波数表示の各文字のサイズを指定 */
  SPR_SetSizeSprite(label->freq14, MC_FREQ_W_14, MC_FREQ_H * 0.85F);
  SPR_SetSizeSprite(label->freq1_00, MC_FREQ_W_1_00, MC_FREQ_H);
  SPR_SetSizeSprite(label->freq_dot, MC_FREQ_W_dot, MC_FREQ_H / 2.0F);
  SPR_SetSizeSprite(label->freq0_10, MC_FREQ_W_0_10, MC_FREQ_H);
  SPR_SetSizeSprite(label->freq0_01, MC_FREQ_W_0_01, MC_FREQ_H);

  /* 周波数表示の各文字のテクスチャを指定("14", "." などの固定部分) */
  SPR_ObjSetTexture(label->freq14,   LAYOUT_14,  work->tx_handle);
  SPR_ObjSetTexture(label->freq_dot, LAYOUT_dot, work->tx_handle);

  /* 周波数表示の各数字のテクスチャを指定(周波数によって変動) */
  {
    /* 各数字のインデックスを得る */
    SPR_TEX * tex;
    int num1_00 = (freq / 100) % 10;
    int num0_10 = (freq / 10) % 10;
    int num0_01 = freq % 10;

    DBG("freq num =  %d %d %d\n", num1_00, num0_10, num0_01);
    /* テクスチャを設定 */
    SPR_ObjSetTexture(label->freq1_00, LAYOUT_font, work->tx_handle);
    SPR_ObjSetTexture(label->freq0_10, LAYOUT_font, work->tx_handle);
    SPR_ObjSetTexture(label->freq0_01, LAYOUT_font, work->tx_handle);

    /* 数字の値によって UV 値を変更 */
    tex = &(label->freq1_00->ex_hd.tex);
    tex->u += (tex->w / 10) * num1_00;
    tex->w = tex->w / 10;

    tex = &(label->freq0_10->ex_hd.tex);
    tex->u += (tex->w / 10) * num0_10;
    tex->w = tex->w / 10;

    tex = &(label->freq0_01->ex_hd.tex);
    tex->u += (tex->w / 10) * num0_01;
    tex->w = tex->w / 10;
  }

  SPR_SHOW(label->freq14);
  SPR_SHOW(label->freq_dot);
  SPR_SHOW(label->freq1_00);
  SPR_SHOW(label->freq0_10);
  SPR_SHOW(label->freq0_01);

  SPR_SHOW(label->freq_pos);
}

/*
 * 名前表示を、status が非0 であれば明るく、0 であれば暗くする。
 */
static void set_name_blight(Work * work, int pos, int status)
{
  NameLabel * label;
  int r, g, b;

  if(work == NULL) return;

  /* 指定された項目に、名前を設定する */
  if(pos < 6)
    label = (!(pos & 1))
      ? (&work->under_lines[pos / 2].l_label)
      : (&work->under_lines[pos / 2].r_label);
  else
    label = (!(pos & 1))
      ? &work->lower_l_label
      : &work->lower_r_label;

  /* 周波数表示下地の RGB値設定 */
  r = (!status) ? MC_FREQ_DARK_R : MC_FREQ_LIGHT_R;
  g = (!status) ? MC_FREQ_DARK_G : MC_FREQ_LIGHT_G;
  b = (!status) ? MC_FREQ_DARK_B : MC_FREQ_LIGHT_B;
  
  label->freq_base->sprite.col.r = r;
  label->freq_base->sprite.col.g = g;
  label->freq_base->sprite.col.b = b;

  r = (!status) ? MC_FREQ_R_dark : MC_FREQ_R;
  g = (!status) ? MC_FREQ_G_dark : MC_FREQ_G;
  b = (!status) ? MC_FREQ_B_dark : MC_FREQ_B;

  /* 名前表示の RGB値設定 */
  label->packet[0].gif.data.rgbq0.data = SCE_GS_SET_RGBAQ(r, g, b, 0x80, 0);
  label->packet[0].gif.data.rgbq1.data = SCE_GS_SET_RGBAQ(r, g, b, 0x80, 0);

  label->packet[1].gif.data.rgbq0.data = SCE_GS_SET_RGBAQ(r, g, b, 0x80, 0);
  label->packet[1].gif.data.rgbq1.data = SCE_GS_SET_RGBAQ(r, g, b, 0x80, 0);

  /* 周波数表示の RGB値設定 */
  if(NULL != label->freq_pos)
    {
      setup_freq_obj_attrib(label->freq_dot, r, g, b, 0x80);
      setup_freq_obj_attrib(label->freq14,   r, g, b, 0x80); 
      setup_freq_obj_attrib(label->freq1_00, r, g, b, 0x80); 
      setup_freq_obj_attrib(label->freq0_10, r, g, b, 0x80); 
      setup_freq_obj_attrib(label->freq0_01, r, g, b, 0x80); 
    }
}


void MemcallOpen(void)
{
  Work * work = now_work;
  if(work == NULL) return;
  if(work->status != MEM_CLOSE) return;
  work->status = MEM_WIDE;
  work->cnt = 0;
  /* SPR_SHOW(work->parent); */
}

void MemcallClose(void)
{
  Work * work = now_work;
  if(work == NULL) return;
  if(work->status != MEM_OPEN) return;
  work->status = MEM_PUSH;
  work->cnt = TIME_PULL;
}

int MemcallStatus(void)
{
  Work * work = now_work;
  if(work == NULL) return -1;
  switch(work->status)
    {
    case MEM_CLOSE: return 0;
    case MEM_OPEN: return 1;
    default: return -1;
    }
}

/*
 * 一番最初の項目の周波数を得る
 */
int MemcallFirstFreq(void)
{
  int i;

  for(i = 0; i < 8; i++)
    if(memcall_list[i].registed) return memcall_list[i].freq;
  return -1;
}

/*
 * 指定項目のリソース番号を得る
 */
int MemcallGetRes(int area)
{
  if(!memcall_list[area].registed) return -1;
  return memcall_list[area].res_name;
}

/*
 * 指定項目の周波数を得る
 */
int MemcallGetFreq(int area)
{
  if(!memcall_list[area].registed) return -1;
  return memcall_list[area].freq;
}

/*
 * パッド操作で項目を選択し、その周波数を得る
 */
int MemcallSelect(GV_PAD * pad)
{
  int num;
  int i;

  if(now_work == NULL) return -1;

  /* 項目が一つも登録されていない場合はエラー */
  for(i = 0; i < 8; i++)
    if(memcall_list[i].registed) break;
  if(i == 8) return -1;

  num = now_work->selected;
  if(pad->press & PAD_U)
    {
      do {
	num -= 2,  num &= 7;
	if(!memcall_list[num].registed)
	  if(memcall_list[num ^ 1].registed) num ^= 1;
      } while(!memcall_list[num].registed);
    }
  if(pad->press & PAD_D)
    {
      do {
	num += 2,  num &= 7;
	if(!memcall_list[num].registed)
	  if(memcall_list[num ^ 1].registed) num ^= 1;
      } while(!memcall_list[num].registed);
    }
  if(pad->press & PAD_R)
    {
      if(memcall_list[num ^ 1].registed) num ^= 1;
      else
	do {
	  num++, num &= 7;
	} while(!memcall_list[num].registed);
    }
  if(pad->press & PAD_L)
    {
      if(memcall_list[num ^ 1].registed) num ^= 1;
      else
	do {
	  num--, num &= 7;
	} while(!memcall_list[num].registed);
    }

  now_work->selected = num;

  return memcall_list[num].freq;
}

static void Act(Work * work)
{
  float rate;

  switch(work->status)
    {
    case MEM_CLOSE:   /* 閉じ状態         */
      work->cnt = 0;
      setup_closed(work);
      break;
    case MEM_WIDE:    /* 左右に伸びる工程 */
      rate = (float)work->cnt / TIME_WIDE;
      if(rate >= 1.0F)
	{
	  rate = 1.0F;
	  work->cnt = 0;
	  work->status = MEM_PART;
	}
      else
	work->cnt += TIME_BASE;
      setup_wide_sequence(work, rate);
      break;
    case MEM_PART:    /* 上下に別れる工程 */
      rate = (float)work->cnt / TIME_PART;
      if(rate >= 1.0F)
	{
	  rate = 1.0F;
	  work->cnt = 0;
	  work->status = MEM_PULL;
	}
      else
	work->cnt += TIME_BASE;
      setup_part_sequence(work, rate);
      break;
    case MEM_PULL:    /* 上下に伸びる工程 */
      rate = (float)work->cnt / TIME_PULL;
      if(rate >= 1.0F)
	{
	  int i;

	  rate = 1.0F;
	  work->cnt = 0;
	  work->status = MEM_OPEN;
	  /* 全ての名前を非選択状態に */
	  for(i = 0; i < 8; i++) set_name_blight(work, i, 0);
	  /* 登録されている最初の項目を選択 */
	  work->selected = -1;
	  for(i = 0; i < 8; i++)
	    if(memcall_list[i].registed)
	      {
		work->selected = i;
		break;
	      }
	}
      else
	work->cnt += TIME_BASE;
      setup_pull_sequence(work, rate);
      break;
      
    case MEM_OPEN:    /* 開き状態         */
      {
	int i;

	work->cnt = TIME_PULL;
	setup_opened(work);
	/* 選択されている項目を光らせる */
	for(i = 0; i < 8; i++)
	  set_name_blight(work, i, (i == work->selected));
      }
      break;
    case MEM_PUSH:
      rate = (float)work->cnt / TIME_PULL;
      if(rate < 0.0F)
	{
	  work->cnt = TIME_PART;
	  rate = 0.0F;
	  work->status = MEM_FIT;
	}
      else
	work->cnt -= TIME_BASE;
      setup_pull_sequence(work, rate);
      break;
      
    case MEM_FIT:
      rate = (float)work->cnt / TIME_PART;
      if(rate < 0.0F)
	{
	  work->cnt = TIME_WIDE;
	  rate = 0.0F;
	  work->status = MEM_SHRINK;
	}
      else
	work->cnt -= TIME_BASE;
      setup_part_sequence(work, rate);
      break;
 
    case MEM_SHRINK:
      rate = (float)work->cnt / TIME_WIDE;
      if(rate < 0.0F)
	{
	  work->cnt = 0;
	  rate = 0.0F;
	  work->status = MEM_CLOSE;
	}
      else
	work->cnt -= TIME_BASE;
      setup_wide_sequence(work, rate);
      break;
    }
}

static void Die(Work * work)
{
  int i;

  /* 字幕転送のパケットをすべて開放する */
  for(i = 0; i < 3; i++)
    {
      release_name_label(&work->under_lines[i].l_label);
      release_name_label(&work->under_lines[i].r_label);
    }
  release_name_label(&work->lower_l_label);
  release_name_label(&work->lower_r_label);

  /* 用意したオブジェクトをすべて破棄する */
  SPR_Destroy_2D_Object(work->parent);
  if(work == now_work) now_work = NULL;
}

static int GetResources(Work * work, int tx_handle)
{
  work->tx_handle = tx_handle;
  
  /* 名前表示用の CLUT を作成する */
  font_set_clut4( _text_clut, 0, 
		  FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) );
  
  /* 必要となる 2D オブジェクトの用意 */
  create_memcall_objects(work);
  work->status = MEM_CLOSE;     /* 初期状態では閉じ */
  setup_closed(work);
  return 0;
}

/*
 * プログラムインタフェース
 */
void * NewMemCallMenu(int tx_handle)
{
  Work * work;

  OPERATOR();

  if(now_work != NULL) return NULL;
  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;


  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  if(GetResources(work, tx_handle))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  now_work = work;
  return work;
}
