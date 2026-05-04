//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	cjimaku.c
		無線機用字幕表示
	2000/07/18	K.Uehara
	2000/09/08      Y.Kira

	$Id: cjimaku.c,v 1.5 2002/12/05 18:42:03 takaki Exp $
*/
#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include <stdio.h>

#include "gameheader.h"
#include "font.h"
#include "dmapack.h"
#include "def_dma.h"

#include "cjimaku.h"
#include "codec_config.h"
#include "codecmem.h"

#include "BP_Renderer.h"
#include "libfs.h"

#include "BP_UIAdjust.h"

#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)

#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)
#define DSIZ(_type)   ((sizeof(_type) + 7) / 8)

#if BP_USE_NEW_FONT_SYSTEM()
#define JIMAKU_PACKET_SIZE ( sizeof( DG_DMAPACK_VIEWMAPPING ) + sizeof( DG_DMAPACK_TEXTURE_DYNAMIC ) + sizeof( DG_DMAPACK_ALPHA ) + sizeof( DG_DMAPACK_SPRT ) * 1 + sizeof( DG_DMAPACK_TAG ) )
#else
#define JIMAKU_PACKET_SIZE ( sizeof( DG_DMAPACK_TEX ) + sizeof( DG_DMAPACK_ALPHA ) + sizeof( DG_DMAPACK_SPRT ) * 1 + sizeof( DG_DMAPACK_TAG ) )
#endif

/* #define NO_CLUT */

#define NO_SIDE     /* ウィンドウ両サイドのぼかしを表現するコードが入るまでは、
		       このマクロを定義しておく。 */

//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
static int euonly_max_length = 0;
//#endif /* PAL */

#if 1 //BP_XBOX def KP_XBOX
static void set_pos( CODEC_MESG_PANEL *work, float posx, float posy, float posw, float posh, float text_width, int use_posx )
{
	// packet設定
	void *pack = ( void * )work->packet;

#if BP_USE_NEW_FONT_SYSTEM()
   pack = DG_SetDmapackViewMapping(pack, 0.0f, 0.0f, BP_REAL_SCREEN_X, BP_REAL_SCREEN_Y);
   work->bp_packTextureLocation = pack;
   pack = DG_SetDmapackTextureDynamic01( pack, NULL, work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex] );
#else
	pack = DG_SetDmapackTexLin( pack, work->tex );
#endif
	pack = DG_SetDmapackAlpha( pack, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
	{
		unsigned int rgba;
		float x, y, w, h;
      float uOff = 0.0f;
      float vOff = 0.0f;

		rgba = 0x80808080;

#if !BP_USE_NEW_FONT_SYSTEM()
		x = ( float )( posx );
		y = ( float )( posy );

		w = ( float )posw;
		h = ( float )posh;
#else
      {
         int const textureWidth = BP_FONT_CONVERT_ORIGINAL_X(work->width);
         int const textureHeight = BP_FONT_CONVERT_ORIGINAL_Y(work->height);
         
         w = textureWidth;
         h = textureHeight;

         uOff = 0.5f / textureWidth;
         vOff = 0.5f / textureHeight;
      }

      if ( use_posx )
         x = posx;
      else
         x = (int)(BP_REAL_SCREEN_X - BP_FONT_CONVERT_ORIGINAL_X(text_width)) / 2;

      y = (int)BP_ADJUST_SCREEN_Y(posy);


#endif

		pack = DG_SetDmapackSprt( pack, 
                                x, y, 
                                0.0f + uOff, 0.0f + vOff, 
                                x + w, y + h, 
                                1.0f + uOff, 1.0f + vOff, 
                                DG_MakeDmaPackColorFromInt(rgba) );
	}
	pack = DG_SetDmapackEnd( pack );
}

#if !BP_USE_NEW_FONT_SYSTEM()
static void send_draw_pattern( CODEC_MESG_PANEL *work )
{
	/* work->vramに展開された4ビットフォントを16ビットテクスチャとして展開する */
	int x, y;
	unsigned char *s = ( unsigned char * )work->vram;
	unsigned short *p = ( unsigned short * )work->tex->image;
	int *clut = ( int * )work->clut;

	for( y = 0; y < work->height; y++ ){
		for( x = 0; x < work->width; x += 2 ){
			p[ 0 ] = clut[ *s & 0xF ];
			p[ 1 ] = clut[ *s >> 4 ];
			p += 2;
			s ++;
		}
	}

	DG_LinerTextureSetImageDirty(work->tex) ;		// image変更内容を適用させる
}
#endif

#endif

/*
 * �テキストビットマップを転送するための DMA パケットを生成
 */
static int codec_Create_DMA_PacketForTextTrans(CODEC_MESG_PANEL * panel,
						int x, int y)
{
  int dx, dy, w, h, ofs;
  DG_DMAPACK * dmapack;

#if 1 //BP_XBOX def KP_XBOX
  int *packet;

  packet = codecMalloc( JIMAKU_PACKET_SIZE );

  if(NULL == (dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
				       DG_DMAPACK_PHASE_LAST)))
    {
      codecFree(packet);
      return -1;
    }

  dmapack->autopacket = packet;

#if !BP_USE_NEW_FONT_SYSTEM()
  /* CLUT のマスタを作成 */
  font_set_clut4( panel->clut, 0, 
		  FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) );
  /* 非選択時の色 */
  font_set_clut4( panel->clut, 1, 
		 FONT_RGB( 70, 70, 70 ), FONT_RGB( 0, 0, 0 ) );
  
  /* 選択時の色 */
  font_set_clut4( panel->clut, 2, 
		  FONT_RGB( GM_FOCUS_COLOR_R * 2, GM_FOCUS_COLOR_G * 2, GM_FOCUS_COLOR_B * 2 )
		 , FONT_RGB( 0, 0, 0 ) );

  panel->tex = DG_MakeLinerTexture( panel->width, panel->height, DG_TEXLIN_FORMAT_A1R5G5B5 );
  {
      // CLUTを16ビット形式に変換
      int i;
      typedef union {
	  struct {
#if BPE_IS_ENDIAN_LITTLE()
	      unsigned char r;
	      unsigned char g;
	      unsigned char b;
	      unsigned char a;
#else
        unsigned char a;
        unsigned char b;
        unsigned char g;
        unsigned char r;
#endif
	  };
	  unsigned int value;
      } CLUT;
      
     CLUT *clut = ( CLUT * )panel->clut;
      for( i = 0; i < 16; i++ ) {
	  unsigned int v;

	  v = ( ( ( clut->r >> 3 ) << 10 )
	       | ( ( clut->g >> 3 ) << 5 )
	       | ( ( clut->b ) >> 3 ) << 0 ) | 0x8000;
	  clut->value = v;
	  clut ++;
      }
      ( ( CLUT * )panel->clut )[ 0 ].value = 0;
      ( ( CLUT * )panel->clut )[ 4 ].value = 0;
      ( ( CLUT * )panel->clut )[ 8 ].value = 0;
  }
#endif
  panel->packet = packet;
  panel->dmapack = dmapack;

  DG_QueueDmapack(dmapack);

  panel->panel_dx = x;
  panel->panel_dy = y;
  panel->panel_w = panel->width;
  panel->panel_h = panel->height;

  set_pos( panel, 0, 0, panel->width, panel->height, 0, 0 );
#endif

  return 0;
}


static void codec_centering( CODEC_MESG_PANEL * panel, float tw)
{
  // センタリングと転送を同時に行う
  int x, y, w, h;

  x = ( DRAW_WIDTH - tw ) / 2;
  y = panel->panel_dy;
  w = panel->panel_w;
  h = panel->panel_h;

  // accurate x
  x = (int)(BP_REAL_SCREEN_X - BP_FONT_CONVERT_ORIGINAL_X(tw)) / 2;
#if BP_VITA
  x+= 12;   //BP JG - shift the text over a little. kp request. AAARRRGGGG!
#else
  x+= 16;   //BP JG - shift the text over a little. kp request.
#endif

  set_pos( panel, x, y, w, h, tw, 1 );

#if !BP_USE_NEW_FONT_SYSTEM()
  send_draw_pattern( panel );
#endif
}

void CODEC_DrawMesgPanel( CODEC_MESG_PANEL *panel, int x, int y )
{
  x = x * DRAW_WIDTH / 512;
  y = y * DRAW_HEIGHT / 448;  /* PAL でも位置が合うように */
  codec_Create_DMA_PacketForTextTrans(panel, x, y);
}

void CODEC_DrawMessage( CODEC_MESG_PANEL *panel, char *mesg )
{
   int w;

#if BP_USE_NEW_FONT_SYSTEM()

   BP_font_begin_render_texture(&panel->bp_vinfo, 1);

   if( mesg != NULL )
   {
      BP_FONT_DRAWINFO dr;
      BP_font_open_drawinfo(&dr, &panel->bp_vinfo);
      BP_font_set_color(&dr, 200, 200, 200, 128);
      BP_font_set_locate(&dr, 0, BP_FONT_CONVERT_ORIGINAL_Y(RUBI_HEIGHT));
      BP_font_draw_string(&dr, mesg);

      w = BP_FONT_CONVERT_NEW_X(BP_font_get_draw_width(&dr));
      codec_centering(panel, w);
   }
   else
   {
//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
      euonly_max_length = 0;
//#endif
   }

   BP_font_end_render_texture(&panel->bp_vinfo);
   if( panel->bp_packTextureLocation )
      DG_SetDmapackTextureDynamic01( panel->bp_packTextureLocation, NULL, panel->pbp_BufferedTexture->aTextureHandle[panel->pbp_BufferedTexture->currIndex] );

#else
   memset( ( char * )panel->vram, 0x00, panel->vramsize );

   if( mesg != NULL )
   {
      int w;
      FONT_DRAWINFO dr;
      font_open_drawinfo( &dr, &panel->vinfo );
      font_set_locate(&dr, 0, RUBI_HEIGHT);
      font_set_color(&dr, 0);
      font_draw_string( &dr, mesg );
      w = font_get_draw_width(&dr);
      printf("cjimaku.c: w = %d\n", w);
      codec_centering(panel, w);
      printf("cjimaku.c: w = %d success\n", w);
   }
   else
   {
//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
      euonly_max_length = 0;
//#endif
      send_draw_pattern( panel );
   }
#endif
}

/*
 * 指定された文字列を, cnt 項目中の num 番目の項目として表示する
 */
void CODEC_DrawSelect( CODEC_MESG_PANEL * panel, int sel,
		       int cnt, int num, char * mesg)
{
   int w;
   int y;
   int colorIdx;

#if 1 //BP_XBOX def KP_XBOX
   colorIdx = 1 + sel;
#else
   colorIdx = 1 - sel;
#endif

   //Added FONT_SIZE_H to compensate for the increased size (MGSTWO-3376)
   y = (panel->height - (RUBI_SIZE_H + FONT_SIZE_H * 2)) * num / cnt;

   if( mesg != NULL )
   {
#if BP_USE_NEW_FONT_SYSTEM()
      BP_FONT_DRAWINFO dr;

      BP_font_begin_render_texture(&panel->bp_vinfo, 0);
      BP_font_open_drawinfo(&dr, &panel->bp_vinfo);
      BP_font_set_locate(&dr, 0, BP_FONT_CONVERT_ORIGINAL_Y(y));
      
      switch(colorIdx)
      {
      case 0:
         BP_font_set_color(&dr, 200, 200, 200, 128);
         break;
      case 1:
         BP_font_set_color(&dr, 70, 70, 70, 128);
         break;
      case 2:
         BP_font_set_color(&dr, GM_FOCUS_COLOR_R * 2, GM_FOCUS_COLOR_R * 2, GM_FOCUS_COLOR_R * 2, 128);
         break;
      }
      BP_font_draw_string(&dr, mesg);
      BP_font_end_render_texture(&panel->bp_vinfo);
      if( panel->bp_packTextureLocation )
         DG_SetDmapackTextureDynamic01( panel->bp_packTextureLocation, NULL, panel->pbp_BufferedTexture->aTextureHandle[panel->pbp_BufferedTexture->currIndex] );

      w = BP_FONT_CONVERT_NEW_X(BP_font_get_draw_width(&dr));
#else
      FONT_DRAWINFO dr;

      font_open_drawinfo( &dr, &panel->vinfo );
      font_set_locate(&dr, 0, y);
      font_set_color(&dr, colorIdx);
      font_draw_string( &dr, mesg );
      w = font_get_draw_width(&dr);
#endif

      if ( BP_Area_EU() )
      {
//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
         if(w > euonly_max_length)
            euonly_max_length = w;
         codec_centering(panel, euonly_max_length);
      }
      else
      {
//#else
         codec_centering(panel, w);
      }
//#endif
   }
}


void CODEC_OpenMesgPanel( CODEC_MESG_PANEL *panel, int width, int height )
{
   int line_pitch, flag;
   int vramsize;

   if ( !BP_Area_JP() )
   {
//#ifndef JAPANESE_BP_IGNORE()
     width += 48;   /* 英語/ヨーロッパ言語圏版は幅が広い */
     if(width > 512) width = 512;
     printf("English Version: text panel width = %d\n", width);
//#endif
   }

  /* VRAM の設定 */
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

  panel->width = width;
  panel->height = height;

#if BP_USE_NEW_FONT_SYSTEM()
  panel->pbp_BufferedTexture = BP_AllocDynamicTexture_Buffered_Pointer(BP_FONT_CONVERT_ORIGINAL_X(panel->width), BP_FONT_CONVERT_ORIGINAL_Y(panel->height), 1);
  BP_font_set_vraminfo_texture_buffered(&panel->bp_vinfo, panel->pbp_BufferedTexture, 0, BP_FONT_CONVERT_ORIGINAL_Y(line_pitch), flag);
#else

  vramsize = width * height * 4 / 8;
  
  panel->vramsize = vramsize;
  
  panel->vram = codecMalloc( vramsize );
  ASSERT( panel->vram != NULL );

  font_set_vraminfo( &panel->vinfo, panel->vram, width, height, 0, line_pitch, flag );
#endif
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
  codecDelayedFree( panel->packet );
#if BP_USE_NEW_FONT_SYSTEM()
  if( panel->pbp_BufferedTexture )
  {
     BP_FreeDynamicTexture_Buffered_Pointer(panel->pbp_BufferedTexture);
     panel->pbp_BufferedTexture = NULL;
  }
#else
  codecDelayedFree( panel->vram );    // NEED TO RETOUCH ほんとはよくない
  DG_FreeLinerTexture( panel->tex );
#endif
}
