//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  mob_panel.c
  携帯端末の文字表示パネル

  2001/07/13  Y.Kira
  $Id: mob_panel.c,v 1.1.1.3 2002/11/19 11:45:15 Yoshizawa1 Exp $
*/
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

#define _mob_panel_c_
#include "mobact.h"
#include "mobile.h"

#include "mob_panel.h"

#ifdef DEBUG
#define DBG   printf
#else
#define DBG
#endif /* DEBUG */



#define RUBI_HEIGHT     0
#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)

#define ALPHA_MAIN      128
#define ALPHA_SHADOW    100

#include "BP_Renderer.h"

static ALIGN16_PRE unsigned int ALIGN16_POST _main_clut[32];
static ALIGN16_PRE unsigned int ALIGN16_POST _shadow_clut[32];


static void set_clut4_alpha(int * clut, int col, int low, int high)
{
  int base, idx, i;

  base = col * 4;
  for(i = 0; i < 4; i++)
    {
      idx = base + i;
      clut[idx] = (clut[idx] & 0x00ffffff) |
	(((((high - low) * i) / 3) + low) << 24);
    }
}

#if !BP_USE_NEW_FONT_SYSTEM()
static void send_draw_pattern( MOBILE_MESG_PANEL *work )
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
}
#endif


/*
 * テキストビットマップ転送パケット生成
 */
static int mobile_Create_DMA_PacketForText(MOBILE_MESG_PANEL * panel,
					  int x, int y, int w, int h)
{
  int dx, dy, ofs;
  DG_DMAPACK * dmapack;

  /*
    テクスチャのパラメタは以下の通り

    転送先バッファベースポイント   : TEX_BASE
    転送先バッファ幅               : TEX_WIDTH
    転送先ピクセル格納フォーマット : SCE_GS_PSMT4HH
  */


  /* CLUT のマスタを作成 */
  font_set_clut4( _main_clut, 0, 
		  FONT_RGB( 128, 128, 128 ), FONT_RGB( 0, 0, 0 ) );

  /* アルファの設定が必要なので、設定する */
  set_clut4_alpha(_main_clut, 0, 0, 128);
  
  /* 座標等を設定 */
  ofs = DG_CurrentField ? 0 : 8;
  dx = DRAW_WIDTH/2  - DG_Chanls[4].width  / 2 + x;
  dy = DRAW_HEIGHT/2 - DG_Chanls[4].height / 2 + y;
  /*
    w = panel->width;
    h = panel->height;
  */
  panel->panel_dx = dx;
  panel->panel_dy = dy;
  panel->panel_w  = w;
  panel->panel_h  = h;


  /* DMAPACK として登録 */
  /* このパケットは、メニューチャネル以外で転送を行わない */
  if(NULL == (dmapack = DG_MakeDmapack(DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE,
				       DG_DMAPACK_PHASE_LAST))) {
      return -1;
  }
  DG_QueueDmapack(dmapack);
  panel->dmapack = dmapack;


  if ( !(panel->packet_mem = codecMalloc( 4350 + sizeof(DG_DMAPACK_TEXTURE_DYNAMIC))) ) {
	  printf("ERR!! MALLOC mobile.c!!\n");
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
  }
#endif

  panel->prim = panel->dmapack->autopacket = panel->packet_mem ;
  DG_SetDmapackEnd( panel->prim ) ;

  return 0;
}


static void mobile_centering( MOBILE_MESG_PANEL * panel, int tw)
{
  int dx, dy, w, h;

  dx = panel->panel_dx;
  dy = panel->panel_dy;
  w  = panel->panel_w;
  h  = panel->panel_h;

#if !BP_USE_NEW_FONT_SYSTEM()
  send_draw_pattern( panel ) ;
  DG_LinerTextureSetImageDirty(panel->tex);
#endif


  panel->prim = panel->packet_mem ;
#if BP_USE_NEW_FONT_SYSTEM()
  panel->prim = DG_SetDmapackTextureDynamic01(panel->prim, NULL, panel->bp_tex);
#else
  panel->prim = DG_SetDmapackTexLin( panel->prim, panel->tex );
#endif
  panel->prim = DG_SetDmapackAlpha( panel->prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
  panel->prim = DG_SetDmapackSprt( panel->prim,
								  dx,dy+4, 0.0f, 0.0f,
								  dx+w,dy+h, 1.0f, 1.0f,
								 DG_MakeDmaPackColorFromInt(SCE_GS_SET_RGBAQ(0, 72, 70, 100, 0)) ) ;
  panel->prim = DG_SetDmapackSprt( panel->prim,
								  dx,dy, 0.0f, 0.0f,
								  dx+w,dy+h, 1.0f, 1.0f,
								  DG_MakeDmaPackColorFromInt(SCE_GS_SET_RGBAQ(0, 24, 24, 128, 0)) ) ;
  DG_SetDmapackEnd( panel->prim ) ;

  printf(" mobile.c!!: needed %d\n", (int)panel->prim - (int)panel->packet_mem ) ;
}

void MOBILE_DrawMesgPanel( MOBILE_MESG_PANEL *panel,
			   int x, int y, int w, int h )
{
  mobile_Create_DMA_PacketForText(panel, x, y, w, h);
}

void MOBILE_DrawMessage( MOBILE_MESG_PANEL *panel, char *mesg )
{
   int w;

#if !BP_USE_NEW_FONT_SYSTEM()
   memset( ( char * )panel->vram, 0x00, panel->vramsize );

   if( mesg != NULL )
   {
      FONT_DRAWINFO dr;
      font_open_drawinfo( &dr, &panel->vinfo );
      font_set_locate(&dr, 0, RUBI_HEIGHT);
      font_draw_string( &dr, mesg );
      w = font_get_draw_width(&dr);
      mobile_centering(panel, w);
   }
#else
   BP_font_begin_render_texture(&panel->bp_vinfo, 1);
   if( mesg != NULL )
   {
      BP_FONT_DRAWINFO dr;
      BP_font_open_drawinfo(&dr, &panel->bp_vinfo);
      BP_font_set_color(&dr, 128, 128, 128, 128);
      BP_font_set_locate(&dr, 0, 0/*RUBI_HEIGHT*/);
      BP_font_draw_string(&dr, mesg);
      w = BP_FONT_CONVERT_NEW_X(BP_font_get_draw_width(&dr));
      mobile_centering(panel, w);
   }
   BP_font_end_render_texture(&panel->bp_vinfo);
#endif
}

void MOBILE_OpenMesgPanel( MOBILE_MESG_PANEL *panel, int width, int height )
{
  int vramsize;
  int w = width * 8 / 6 + 24;

#if 0 //BP_XBOX ndef PSX2
  w = (w/64 + 1) * 64 ;
#endif
  vramsize = w * height * 4 / 8;
  
  panel->width = w;
  panel->height = height;

#if !BP_USE_NEW_FONT_SYSTEM()
  panel->vramsize = vramsize;
  panel->vram = codecMalloc( vramsize );
  ASSERT( panel->vram != NULL );
  
  printf("vram = %p (size = %d) \n", panel->vram, panel->vramsize);

  /* VRAM の設定 */
  font_set_vraminfo( &panel->vinfo,
		     panel->vram,
		     w, height, 0, 4, FONT_NO_KINSOKU );
#else
  panel->bp_tex = BP_AllocDynamicTexture(BP_FONT_CONVERT_ORIGINAL_X(panel->width), BP_FONT_CONVERT_ORIGINAL_Y(panel->height), 1);
  BP_font_set_vraminfo_texture(&panel->bp_vinfo, panel->bp_tex, 0, BP_FONT_CONVERT_ORIGINAL_Y(4), FONT_NO_KINSOKU);
#endif
}

/* メッセージパネルを隠す */
void MOBILE_HideMesgPanel( MOBILE_MESG_PANEL * panel)
{
  panel->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
}

/* メッセージパネルを再表示する */
void MOBILE_ShowMesgPanel(MOBILE_MESG_PANEL * panel)
{
  panel->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
}

void MOBILE_CloseMesgPanel( MOBILE_MESG_PANEL *panel )
{
#if !BP_USE_NEW_FONT_SYSTEM()
	/*フォントテクスチャーの開放*/
	if ( panel->tex ) 
      DG_FreeLinerTexture( panel->tex ) ;

   codecDelayedFree( panel->vram );
#else
   if(panel->bp_tex)
      BP_FreeDynamicTexture(panel->bp_tex);
#endif
   /*DMAパケットの開放*/
	if ( panel->packet_mem ) 
      codecDelayedFree( panel->packet_mem ) ;

	DG_DequeueDmapack(panel->dmapack);
	DG_FreeDmapack(panel->dmapack);
}
