//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
  photo_panel.c
  写真転送端末の文字表示パネル

  2001/05/16  Y.Kira
  $Id: photo_panel.c,v 1.4 2002/11/23 12:28:38 Yoshizawa1 Exp $
*/
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gameheader.h"
#include "g_struct.h"
#include "g_define.h"
#include "libdg.h"
#include "font.h"
#include "libfs.h"
#include "dmapack.h"
#include "def_dma.h"

#include "../codec/codecmem.h"

#define _photo_panel_c_
#include "photo_config.h"
#include "photo_panel.h"


#ifdef DEBUG
#define DBG   printf
#else
#define DBG
#endif /* DEBUG */

#include "BP_BuildDefines.h"
#include "BP_Renderer.h"

#define RUBI_HEIGHT     0
#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)

#define FONT_R  100
#define FONT_G  100
#define FONT_B  100

#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)

static ALIGN16_PRE unsigned int  ALIGN16_POST _text_clut[32];

#if !BP_USE_NEW_FONT_SYSTEM()
static void send_draw_pattern( PHOTO_MESG_PANEL *work )
{
	/* work->vramに展開された4ビットフォントを16ビットテクスチャとして展開する */
	int x, y;
	unsigned char *s = ( unsigned char * )work->vram;
	unsigned short *p = ( unsigned short * )work->tex->image;
//	int *clut = ( int * )work->clut;

	for( y = 0; y < work->height; y++ ){
		for( x = 0; x < work->width; x += 2 ){
			p[ 0 ] = _text_clut[ *s & 0xF ];
			p[ 1 ] = _text_clut[ *s >> 4 ];
			p += 2;
			s ++;
		}
	}
	DG_LinerTextureSetImageDirty(work->tex) ;	// Image変更の適用
}
#endif

/*
 * テキストビットマップ転送パケット生成
 */
static int photo_Create_DMA_PacketForText(PHOTO_MESG_PANEL * panel,
					  int x, int y, int aw, int ah)
{
  int dx, dy, w, h, ofs;
  DG_DMAPACK * dmapack;

  /*
    テクスチャのパラメタは以下の通り

    転送先バッファベースポイント   : TEX_BASE
    転送先バッファ幅               : TEX_WIDTH
    転送先ピクセル格納フォーマット : SCE_GS_PSMT4HH
  */


  /* CLUT のマスタを作成 */
  font_set_clut4( _text_clut, 0, FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) ) ;

  
  /* 座標等を設定 */
  ofs = DG_CurrentField ? 0 : 8;
  dx = /*DRAW_WIDTH/2  - DG_Chanls[4].width  / 2 +*/ x * DG_Chanls[4].width  / 512 ;
  dy = /*DRAW_HEIGHT/2 - DG_Chanls[4].height / 2 +*/ y * DG_Chanls[4].height / 384 ;

  w = aw * DG_Chanls[4].width  / 512 * 24 / 24;
  h = ah * DG_Chanls[4].height / 384 * 24 / 24;
  panel->panel_dx = dx;
  panel->panel_dy = dy;
  panel->panel_w  = w;
  panel->panel_h  = h;

  printf("PHOTO PANEL: (%d, %d), w:%d  h:%d\n", dx, dy, w, h);

  /* DMAPACK として登録 */
  /* このパケットは、メニューチャネル以外で転送を行わない */
  if(NULL == (dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
				       DG_DMAPACK_PHASE_LAST)))
    {
      return -1;
    }

  panel->dmapack = dmapack;
  DG_QueueDmapack(dmapack);

  if ( !(panel->packet_mem = codecMalloc( 256 )) ) {
	  printf("ERR!! MALLOC photo_panel.c!!\n");
	  return -1 ;
  }

#if !BP_USE_NEW_FONT_SYSTEM()
  panel->tex = DG_MakeLinerTexture( panel->width, panel->height, DG_TEXLIN_FORMAT_A1R5G5B5 );

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
      
      CLUT *clut = ( CLUT * )_text_clut;
      for( i = 0; i < 16; i++ ) {
		  unsigned int v;

		  v = ( ( ( clut->r >> 3 ) << 10 )
			   | ( ( clut->g >> 3 ) << 5 )
			   | ( ( clut->b ) >> 3 ) << 0 ) | 0x8000;
		  clut->value = v;
		  clut ++;
      }
      ( ( CLUT * )_text_clut )[ 0 ].value = 0;
  }
#endif

  panel->prim = panel->dmapack->autopacket = panel->packet_mem ;
  DG_SetDmapackEnd( panel->prim ) ;

  return 0;
}


static void photo_centering( PHOTO_MESG_PANEL * panel, int tw)
{
  int dx, dy, w, h;

  tw = tw * DRAW_WIDTH / 640;
  w  = panel->panel_w;
  h  = panel->panel_h;

#if !BP_USE_NEW_FONT_SYSTEM()
  send_draw_pattern( panel ) ;
#endif

  dx = DRAW_WIDTH/2 - tw / 2 + 30;
  dy = panel->panel_dy;

  panel->prim = panel->packet_mem ;
#if !BP_USE_NEW_FONT_SYSTEM()
  panel->prim = DG_SetDmapackTexLin( panel->prim, panel->tex );
#else
  panel->prim = DG_SetDmapackTextureDynamic01(panel->prim, NULL, panel->pbp_BufferedTexture->aTextureHandle[panel->pbp_BufferedTexture->currIndex]);
#endif

  panel->prim = DG_SetDmapackAlpha( panel->prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
  panel->prim = DG_SetDmapackSprt( panel->prim,
								   dx  ,dy  , 0.5f/panel->width,                        0.5f/panel->height,
								   dx+w,dy+h, (panel->tex_width_xbox-0.5f)/panel->width,(panel->height-0.5f)/panel->height,
								   DG_MakeDmaPackColorFromInt(SCE_GS_SET_RGBAQ(FONT_R, FONT_G, FONT_B, 0x80, 0)) ) ;
  DG_SetDmapackEnd( panel->prim ) ;

  printf(" photo panel.c!!: needed %d\n", (int)panel->prim - (int)panel->packet_mem ) ;
}


void PHOTO_DrawMesgPanel( PHOTO_MESG_PANEL *panel, int x, int y, int w, int h )
{
  photo_Create_DMA_PacketForText(panel, x, y, w, h);
}

void PHOTO_DrawMessage( PHOTO_MESG_PANEL *panel, char *mesg )
{
   int w;

   DBG("PHOTO_DrawMessage(): mesg = %p\n", mesg);

#if !BP_USE_NEW_FONT_SYSTEM()
   memset( ( char * )panel->vram, 0x00, panel->vramsize );

   if( mesg != NULL )
   {
      FONT_DRAWINFO dr;
      font_open_drawinfo( &dr, &panel->vinfo );
      font_set_locate(&dr, 0, RUBI_HEIGHT);
      font_draw_string( &dr, mesg );
      w = font_get_draw_width(&dr);
      photo_centering(panel, w);
   }
#else

   BP_font_begin_render_texture(&panel->bp_vinfo, 1);

   if( mesg != NULL )
   {
      BP_FONT_DRAWINFO dr;
      BP_font_open_drawinfo( &dr, &panel->bp_vinfo );
      BP_font_set_locate(&dr, 0, BP_FONT_CONVERT_ORIGINAL_Y(RUBI_HEIGHT));
      BP_font_set_color(&dr, 200, 200, 200, 128);
      BP_font_draw_string( &dr, mesg );
      w = BP_FONT_CONVERT_NEW_X(BP_font_get_draw_width(&dr));
      photo_centering(panel, w);
   }

   BP_font_end_render_texture(&panel->bp_vinfo);

#endif

   DBG("PHOTO_DrawMessage(): finish\n");
}

void PHOTO_OpenMesgPanel( PHOTO_MESG_PANEL *panel, int width, int height )
{
  int vramsize;

  /* 英語/ヨーロッパ言語圏版は少し幅が広い */
  /* 日本語もおなじようにしました。2002.07.09yano */
  width += 48;
  if(width > 512) width = 512; 

#if 1 //def KP_XBOX //ＸＢＯＸは横幅が64の倍数である必要がある
  panel->tex_width_xbox = width;/* 実際の横幅はテクスチャを貼り付ける際使う */
  width = ((width+63) / 64) * 64 ;
#endif

  panel->width = width;
  panel->height = height;

#if !BP_USE_NEW_FONT_SYSTEM()
  panel->vramsize = width * height * 4 / 8;
  panel->vram = codecMalloc( panel->vramsize );
  ASSERT( panel->vram != NULL );
  
  printf("vram = %p (size = %d) \n", panel->vram, panel->vramsize);
#else
  panel->pbp_BufferedTexture = BP_AllocDynamicTexture_Buffered_Pointer(BP_FONT_CONVERT_ORIGINAL_X(width), BP_FONT_CONVERT_ORIGINAL_Y(height), 1);
#endif

  {
	  /* VRAM の設定 */
	  int line_pitch, flag;
	  if( GM_Language == GM_LANG_JAPANESE )
     {
		  /* 日本語 */
		  flag = 0;
		  line_pitch = 12;
	  } 
     else 
     {
		  /* 日本語以外 */
		  flag = FONT_NO_KINSOKU;
		  line_pitch = 4;
	  }

#if !BP_USE_NEW_FONT_SYSTEM()
	  font_set_vraminfo( &panel->vinfo, panel->vram, width, height, 0, line_pitch, flag );
#else
     BP_font_set_vraminfo_texture_buffered(&panel->bp_vinfo, panel->pbp_BufferedTexture, 0, BP_FONT_CONVERT_ORIGINAL_Y(line_pitch), flag);
#endif
  }
}

/* メッセージパネルを隠す */
void PHOTO_HideMesgPanel( PHOTO_MESG_PANEL * panel)
{
  panel->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
}


/* メッセージパネルを再表示する */
void PHOTO_ShowMesgPanel(PHOTO_MESG_PANEL * panel)
{
  panel->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
}

void PHOTO_CloseMesgPanel( PHOTO_MESG_PANEL *panel )
{
#if !BP_USE_NEW_FONT_SYSTEM()
	/*フォントテクスチャーの開放*/
	if ( panel->tex ) 
      DG_FreeLinerTexture( panel->tex ) ;

   codecDelayedFree( panel->vram );    // NEED TO RETOUCH ほんとはよくない
#else
   if( panel->pbp_BufferedTexture )
   {
      BP_FreeDynamicTexture_Buffered_Pointer(panel->pbp_BufferedTexture);
      panel->pbp_BufferedTexture = NULL;
   }
#endif
   /*DMAパケットの開放*/
	if ( panel->packet_mem ) codecDelayedFree( panel->packet_mem ) ;

  DG_DequeueDmapack(panel->dmapack);
  DG_FreeDmapack(panel->dmapack);
}
