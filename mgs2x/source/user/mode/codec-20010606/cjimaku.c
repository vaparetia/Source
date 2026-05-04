/*
	cjimaku.c
		無線機用字幕表示
	2000/07/18	K.Uehara
	2000/09/08      Y.Kira

	$Id: cjimaku.c,v 1.1.1.3 2002/11/19 11:45:05 Yoshizawa1 Exp $
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
#include "font.h"
#include "dmapack.h"
#include "def_dma.h"

#include "cjimaku.h"
#include "codec_config.h"
#include "codecmem.h"
#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)


/* #define NO_CLUT */

#define NO_SIDE     /* ウィンドウ両サイドのぼかしを表現するコードが入るまでは、
		       このマクロを定義しておく。 */


/* テクスチャ転送部 */
typedef struct {
  DG_DMATAG dmatag0;
  DG_GIFTAG giftag0;
  DG_GSREG  gsregs[4];
  DG_GIFTAG giftag1;
  DG_DMATAG dmatag1;
} tex_packet ALIGN16;

/* CLUT転送部 */
typedef struct { 
  /* 転送の前準備を行う */
  DG_DMATAG  dmatag0;

  DG_GIFTAG  giftag0;

  DG_GSREG   bitbltbuf;  /* バッファ間転送設定(転送形式)   */
  DG_GSREG   trxpos;     /* バッファ間転送領域指定(転送先) */
  DG_GSREG   trxreg;     /* バッファ間転送領域指定(サイズ) */
  DG_GSREG   trxdir;     /* 転送開始                       */

  DG_GIFTAG  giftag1;    /* GIF イメージ転送準備           */

  /* イメージ転送       */
  DG_DMATAG  dmatag1;    /* イメージ本体を転送             */
} clut_packet ALIGN16;



typedef struct {
#ifndef NO_CLUT
  clut_packet  clut_trans;   /* CLUT 転送部      */
#endif /* NO_CLUT */

  tex_packet   tex_trans;    /* テキスト部テクスチャ転送部 */

  /* 描画部 */
  DG_DMATAG    dmatag;
  DG_GIFTAG    giftag;

  DG_GSREG     texflush;

  /* 下地の暗い半透明パネル */
  DG_GSREG     panel_alpha;
  DG_GSREG     panel_prim;
  DG_GSREG     panel_rgb;
  DG_GSREG     panel_xyz0;
  DG_GSREG     panel_xyz1;

  /* テキスト本体 */
  DG_GSREG     alpha;    /* アルファブレンディング設定 */
  DG_GSREG     clamp;    /* テキスト領域の CLAMP 値    */
  DG_GSREG     colclamp; /* カラークランプ制御         */
  DG_GSREG     tex0;     /* テクスチャ設定(TEX0)       */
  DG_GSREG     tex1;     /* テクスチャ設定(TEX1)       */
  DG_GSREG     prim;     /* プリミティブ指定           */

  DG_GSREG     rgbq0;    /* 左上頂点色                 */
  DG_GSREG     uv0;      /* 左上 UV 値                 */
  DG_GSREG     xyz0;     /* 左上描画座標               */

  DG_GSREG     rgbq1;    /* 右下頂点色   */
  DG_GSREG     uv1;      /* 右下 UV 値   */
  DG_GSREG     xyz1;     /* 右下描画座標 */
  
} textarea_packet ALIGN16;

typedef struct {
	sceGsTex0 tex;
	sceGsRgbaq rgba;
	sceGsPrim prim;
	u_long nop;
} MENU_SPRT_INIT;

typedef struct {
	DG_PRIM_UV uv1;
	DG_PRIM_XY xy1;
	DG_PRIM_UV uv2;
	DG_PRIM_XY xy2;
} MENU_SPRT;

static unsigned int _text_clut[32]  ALIGN16;

/*
 * テキストビットマップを転送するための DMA パケットを生成
 */
static int codec_Create_DMA_PacketForTextTrans(CODEC_MESG_PANEL * panel,
						int x, int y)
{
  int dx, dy, w, h, ofs;
  DG_DMAPACK * dmapack;
  textarea_packet * packet;
  static textarea_packet def_tex_pack = {
#ifndef NO_CLUT
    /* CLUT 転送部の設定 */
    clut_trans:
    {
      /* 転送前準備パラメタ転送 */
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
      /* 転送前準備 */
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
      },
    },
#endif /* NO_CLUT */
    /* 描画部の設定 */
    dmatag:
    {
      qwc: DMATAG_SET_QWC(DMATAG_ID_RET, 19),
      vifcode:
      {
	SCE_VIF1_SET_NOP(0),
	SCE_VIF1_SET_DIRECT(19, 0)
      }
    },
    giftag:
    {
      tag: SCE_GIF_SET_TAG(18, 1, 0, 0, 0, 1),
      regs: GS_REGS_AD
    },

    /* テクスチャをフラッシュする */
    texflush:
    {
      reg: SCE_GS_TEXFLUSH,
      data: 1
    },
    /* パネル描画部 */
    panel_alpha:
    {
      reg: SCE_GS_ALPHA_1,
      data: SCE_GS_SET_ALPHA(1, 2, 0, 2, 94)
    },
    panel_prim:
    {
      reg: SCE_GS_PRIM,
      data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 0, 0, 0)
    },
    panel_rgb:
    {
      reg: SCE_GS_RGBAQ,
      data: SCE_GS_SET_RGBAQ(0, 0, 0, 64, 0)
    },
    panel_xyz0:{ reg: SCE_GS_XYZ2 },
    panel_xyz1:{ reg: SCE_GS_XYZ2 },

    /* 文字部分の描画 */
    alpha:
    {
      reg: SCE_GS_ALPHA_1,
      data: SCE_GS_SET_ALPHA(0, 2, 2, 1, 100)
    },
    clamp: { reg: SCE_GS_CLAMP_1 },
    colclamp:
    {
      reg: SCE_GS_COLCLAMP,
      data: 1
    },
    tex1:
    {
      reg: SCE_GS_TEX1_1,
      data: SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0)
    },
    tex0:
    {
      reg: SCE_GS_TEX0_1,
      data: SCE_GS_SET_TEX0(TEX_BASE, TEX_WIDTH, SCE_GS_PSMT4HH, 9, 8, 1, 1,
			    CLUT_BASE, SCE_GS_PSMCT32, 0, 0, 2)
    },
    prim:
    {
      reg: SCE_GS_PRIM,
      data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0)
    },
    rgbq0:
    {
      reg: SCE_GS_RGBAQ,
      data: SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0)
    },
    uv0: { reg: SCE_GS_UV },
    xyz0:{ reg: SCE_GS_XYZ2 },

    rgbq1:
    {
      reg: SCE_GS_RGBAQ,
      data: SCE_GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0)
    },
    uv1: { reg: SCE_GS_UV },
    xyz1:{ reg: SCE_GS_XYZ2 }
  };

  if(NULL == (packet = codecMalloc(sizeof(textarea_packet) * 2)))  return -1 ;

  /*
   * デフォルト値のマスタ領域を書き換えてから、その内容を実際の
   * バッファにコピーする
   */
  /* テクスチャ転送部を設定 */
  DG_MakeLoadImagePacket(&def_tex_pack.tex_trans,
			 SCE_GS_PSMT4HH,
			 panel->width, panel->height,
			 TEX_BASE * 64, TEX_WIDTH * 64,
			 panel->vram);

  /*
    テクスチャのパラメタは以下の通り

    転送先バッファベースポイント   : TEX_BASE
    転送先バッファ幅               : TEX_WIDTH
    転送先ピクセル格納フォーマット : SCE_GS_PSMT4HH
  */


  /* CLUT のマスタを作成 */
  font_set_clut4( _text_clut, 0, 
		  FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) );

  
  /* 座標等を設定 */
  ofs = DG_CurrentField ? 0 : 8;
  dx = 2048 - DG_Chanls[4].width / 2 + x;
  dy = 2048 - DG_Chanls[4].height / 2 + y;
  w = panel->width * DG_Chanls[4].width / 640 * 24 / 24;
  h = (panel->height * DG_Chanls[4].height / 480) * 24 / 24;
  panel->panel_dx = dx;
  panel->panel_dy = dy;
  panel->panel_w  = w;
  panel->panel_h  = h;
  def_tex_pack.panel_xyz0.data = SCE_GS_SET_XYZ( dx * 16, dy * 16, -1);
  def_tex_pack.panel_xyz1.data = SCE_GS_SET_XYZ((dx + w) * 16,(dy + h) * 16, -1);
  
  /* 文字の開始位置によって、dx を調整する */
  def_tex_pack.xyz0.data = SCE_GS_SET_XYZ( dx * 16, dy * 16, -1);
  def_tex_pack.xyz1.data = SCE_GS_SET_XYZ((dx + w) * 16,(dy + h) * 16, -1);

  def_tex_pack.uv0.data = SCE_GS_SET_UV(8, 8);
  def_tex_pack.uv1.data = SCE_GS_SET_UV(panel->width * 16 + 8,
					panel->height * 16 + 8);

  def_tex_pack.clamp.data = SCE_GS_SET_CLAMP(2, 2, 0, panel->width, 0, panel->height);


  /* 初期状態を設定 */
  /* 設定の終了したマスタをコピーすることで、パケットを用意する */
  memcpy(packet,     &def_tex_pack, sizeof(textarea_packet));
  memcpy(packet + 1, &def_tex_pack, sizeof(textarea_packet));

  /* DMAPACK として登録 */
  /* このパケットは、メニューチャネル以外で転送を行わない */
  if(NULL == (dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
				       DG_DMAPACK_PHASE_LAST)))
    {
      codecFree(packet);
      return -1;
    }

  dmapack->packet[0] = &packet[0];
  dmapack->packet[1] = &packet[1];

  panel->packet = packet;
  panel->dmapack = dmapack;

  DG_QueueDmapack(dmapack);

  return 0;
}


static void codec_centering( CODEC_MESG_PANEL * panel, int tw)
{
  textarea_packet *packet;
  int dx, dy, w, h;

  packet = panel->packet;
  tw = tw * DRAW_WIDTH / 640;
  dx = 2048 - tw / 2;
  dy = panel->panel_dy;
  w  = panel->panel_w;
  h  = panel->panel_h;
  packet[0].xyz0.data = SCE_GS_SET_XYZ( dx * 16, dy * 16, -1);
  packet[0].xyz1.data = SCE_GS_SET_XYZ((dx + w) * 16,(dy + h) * 16, -1);
  packet[1].xyz0.data = SCE_GS_SET_XYZ( dx * 16, dy * 16, -1);
  packet[1].xyz1.data = SCE_GS_SET_XYZ((dx + w) * 16,(dy + h) * 16, -1);
}

void CODEC_DrawMesgPanel( CODEC_MESG_PANEL *panel, int x, int y )
{
  codec_Create_DMA_PacketForTextTrans(panel, x, y);
}

void CODEC_DrawMessage( CODEC_MESG_PANEL *panel, char *mesg )
{
  FONT_DRAWINFO dr;
  int w;

  memset( ( char * )panel->vram, 0x00, panel->vramsize );
  
  if( mesg != NULL ){
    font_open_drawinfo( &dr, &panel->vinfo );
    font_set_locate(&dr, 0, RUBI_HEIGHT);
    font_draw_string( &dr, mesg );
    w = font_get_draw_width(&dr);
    printf("cjimaku.c: w = %d\n", w);
    codec_centering(panel, w);
    printf("cjimaku.c: w = %d success\n", w);
  }
}

void CODEC_OpenMesgPanel( CODEC_MESG_PANEL *panel, int width, int height )
{
  int vramsize;
  
  vramsize = width * height * 4 / 8;
  
  panel->vramsize = vramsize;
  panel->width = width;
  panel->height = height;
  
  panel->vram = codecMalloc( vramsize );
  ASSERT( panel->vram != NULL );
  
  printf("vram = %p (size = %d) \n", panel->vram, panel->vramsize);

  /* VRAM の設定 */
  font_set_vraminfo( &panel->vinfo,
		     panel->vram,
		     width, height, 0, 12, 0 );

}

/* メッセージパネルを隠す */
void CODEC_HideMesgPanel( CODEC_MESG_PANEL * panel)
{
  panel->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
}

/* メッセージパネルを再表示する */
void CODEC_ShowMesgPanel(CODEC_MESG_PANEL * panel)
{
  panel->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
}

void CODEC_CloseMesgPanel( CODEC_MESG_PANEL *panel )
{
  DG_DequeueDmapack(panel->dmapack);
  DG_FreeDmapack(panel->dmapack);
  codecFree( panel->packet );
  codecFree( panel->vram );    // NEED TO RETOUCH ほんとはよくない
}
