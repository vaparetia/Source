//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	jimaku.c
		字幕簡易表示
	2000/01/05	K.Uehara
	$Id: jimaku.c,v 1.7 2002/12/13 02:55:45 takaki Exp $
*/
#endif

#include "gameheader.h"

#include "BP_BuildDefines.h"

#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Renderer.h"
#include "BP_Font.h"
#endif

#include "BP_UIAdjust.h"

#include "font.h"

#if BP_USE_NEW_FONT_SYSTEM()
#define JIMAKU_PACKET_SIZE ( sizeof( DG_DMAPACK_VIEWMAPPING ) + sizeof( DG_DMAPACK_TEXTURE_DYNAMIC ) + sizeof( DG_DMAPACK_ALPHA ) + sizeof( DG_DMAPACK_SPRT ) * BP_FONT_OUTLINED_DRAW_COUNT + sizeof( DG_DMAPACK_TAG ) )
#else
#define JIMAKU_PACKET_SIZE ( sizeof( DG_DMAPACK_TEX ) + sizeof( DG_DMAPACK_ALPHA ) + sizeof( DG_DMAPACK_SPRT ) * BP_FONT_OUTLINED_DRAW_COUNT + sizeof( DG_DMAPACK_TAG ) )
#endif

typedef struct jimaku_Work 
{
	GV_ACT actor;
	int name;
	int posy;
	int rate;
	int width;
	int disable;

   DG_DMAPACK dmapack;
	u_long128 packet[ NORM16( JIMAKU_PACKET_SIZE ) / sizeof( u_long128 ) ];

#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_VRAMINFO  bp_vinfo;
   SBP_BufferedTexture bp_BufferedTexture;
#else
   FONT_VRAMINFO vinfo;
   DG_TEX_LIN *tex;
	u_long128 clut[ sizeof( int ) * 16 / sizeof( u_long128 ) ];
	u_long128 vram[ 0 ];
#endif
} Work;

static Work *jimaku_work = NULL;

#define V_WIDTH			FONT_BUFFER_WIDTH( 21, 0 )
#define V_HEIGHT		( FONT_BUFFER_HEIGHT( 2, 12 ) )

#define BP_V_WIDTH	BP_FONT_BUFFER_WIDTH( 21, 0 )
#define BP_V_HEIGHT  BP_FONT_BUFFER_HEIGHT( 2, BP_RUBI_SIZE_H )

#define W_RATE			1
#define H_RATE			1

#define RATE_ONE		256

#define YTOP			DIRECT_SCREEN_Y( ((224-48)*2 + 6)-16 )       //BP JG - shifted up a bit to avoid safe zone issues.

#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-3)
#define CLUT_WIDTH	(64/64)

#define VRAM_SIZE		(V_WIDTH*V_HEIGHT*4/8)

static void Act( Work *work )
{
	GV_SleepActor( work, GV_CLASS_WAIT_USER );
}

static void Die( Work *work )
{
	DG_DequeueDmapack( &work->dmapack );
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FreeDynamicTexture_Buffered(&work->bp_BufferedTexture);
#else
	DG_FreeLinerTexture( work->tex );
#endif
	work->actor.free = GV_DelayedFree;
	jimaku_work = NULL;
}

static void set_pos( Work *work )
{
	// packet設定
	int i;
	int posx, posy;

	void *pack = ( void * )work->packet;

#if BP_USE_NEW_FONT_SYSTEM()
   pack = DG_SetDmapackViewMapping(pack, 0.0f, 0.0f, BP_REAL_SCREEN_X, BP_REAL_SCREEN_Y);
   pack = DG_SetDmapackTextureDynamic01(pack, NULL, work->bp_BufferedTexture.aTextureHandle[work->bp_BufferedTexture.currIndex]);

   posx = BP_REAL_SCREEN_X / 2 - ( ( work->width / 2 ) * W_RATE ) * work->rate / RATE_ONE;
   posy = (int)BP_ADJUST_SCREEN_Y(work->posy + FONT_SIZE_H / 2 - ( FONT_SIZE_H * work->rate / RATE_ONE ) / 2);

#else
	pack = DG_SetDmapackTexLin( pack, work->tex );

   posx = DRAW_WIDTH / 2 - ( ( work->width / 2 ) * W_RATE ) * work->rate / RATE_ONE;
   posy = work->posy + FONT_SIZE_H / 2 - ( FONT_SIZE_H * work->rate / RATE_ONE ) / 2;
#endif


	pack = DG_SetDmapackAlpha( pack, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	for( i = 0; i < BP_FONT_OUTLINED_DRAW_COUNT; i++ )
   {
		unsigned int rgba;
		float x, y, w, h;
      float uOff = 0.0f, vOff = 0.0f;
      int xOff, yOff;

      if( BP_GetOutlineOffsets(i, &xOff, &yOff) )
         rgba = 0x80808080;
      else 
         rgba = 0x80101010;

		x = ( float )( posx + xOff );
		y = ( float )( posy + yOff );

#if BP_USE_NEW_FONT_SYSTEM()
      w = ( float )BP_V_WIDTH * W_RATE * work->rate / RATE_ONE;
      h = ( float )BP_V_HEIGHT * H_RATE * work->rate / RATE_ONE;

      uOff = 0.5f / BP_V_WIDTH;
      vOff = 0.5f / BP_V_HEIGHT;
#else
		w = ( float )work->width * W_RATE * work->rate / RATE_ONE;
		h = ( float )V_HEIGHT * H_RATE * work->rate / RATE_ONE;
#endif

		pack = DG_SetDmapackSprt(pack, x, y, 0.0F + uOff, 0.0F + vOff, x + w, y + h, 1.0f + uOff, 1.0F + vOff, DG_MakeDmaPackColorFromInt(rgba));
	}
	pack = DG_SetDmapackEnd( pack );
}

#if !BP_USE_NEW_FONT_SYSTEM()
static void send_draw_pattern( Work *work )
{
	/* work->vramに展開された4ビットフォントを16ビットテクスチャとして展開する */
	int x, y;
	unsigned char *s = ( unsigned char * )work->vram;
	unsigned short *p = ( unsigned short * )work->tex->image;
	int *clut = ( int * )work->clut;

	for( y = 0; y < V_HEIGHT; y++ ){
		for( x = 0; x < V_WIDTH; x += 2 ){
			p[ 0 ] = clut[ *s & 0xF ];
			p[ 1 ] = clut[ *s >> 4 ];
			p += 2;
			s ++;
		}
	}
	DG_LinerTextureSetImageDirty(work->tex) ;	// Image変更の適用
}
#endif

static int GetResources( Work *work )
{
	DG_DMAPACK *pack;
	unsigned int *clut;
   int l_skip;

	/*
		プリミティブの生成
	*/

	pack = &( work->dmapack );
	pack->flag = DG_DMAPACK_MENU | DG_DMAPACK_INVISIBLEMENU;
	pack->flag |= DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3;
	pack->phase = DG_DMAPACK_PHASE_AFTER;
	pack->priority = 250;	/* default */

	DG_QueueDmapack( pack );

#if BP_USE_NEW_FONT_SYSTEM()
   BP_AllocDynamicTexture_Buffered(BP_V_WIDTH, BP_V_HEIGHT, 1, &work->bp_BufferedTexture);
   // keep subtitles out of safe zone - Japanese doesn't have descenders, but it needs more space for rubi chars
   l_skip = GM_Language == GM_LANG_JAPANESE ? BP_RUBI_SIZE_H : 3 * BP_RUBI_SIZE_H / 4;
   BP_font_set_vraminfo_texture_buffered(&work->bp_vinfo, &work->bp_BufferedTexture, 0, l_skip, FONT_NO_KINSOKU);
#else
	/* VRAM の設定 */
	font_set_vraminfo( &work->vinfo,
					   work->vram, V_WIDTH, V_HEIGHT, 0, 12, FONT_NO_KINSOKU );

	clut = ( unsigned int * )work->clut;

	/* パレットの設定 */
	font_set_clut4( clut, 0, FONT_RGB( 240, 240, 240 ), FONT_RGB( 0, 0, 0 ) );
	clut[ 0 ] = 0;

	memset( ( char * )work->vram, 0x00, VRAM_SIZE );

	work->tex = DG_MakeLinerTexture( V_WIDTH, V_HEIGHT, DG_TEXLIN_FORMAT_A1R5G5B5 );
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
		
		CLUT *clut = ( CLUT * )work->clut;
		for( i = 0; i < 16; i++ ) {
			unsigned int v;

			v = ( ( ( clut->r >> 3 ) << 10 )
				 | ( ( clut->g >> 3 ) << 5 )
				 | ( ( clut->b ) >> 3 ) << 0 ) | 0x8000;
			clut->value = v;
			clut ++;
		}
		( ( CLUT * )work->clut )[ 0 ].value = 0;
	}
#endif
   work->posy = YTOP;
   work->rate = RATE_ONE;
   work->width = 0;

   pack->autopacket = work->packet;

	set_pos( work );
	
	return 0;
}

void *GM_JimakuDaemonStart( void )
{
	Work *work;

#if BP_USE_NEW_FONT_SYSTEM()
   int const additionalSize = 0;
#else
   int const additionalSize = VRAM_SIZE;
#endif

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_SYSTEM, sizeof( Work ) + additionalSize, 0 );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}

	jimaku_work = work;
	return work;
}

/* ---------------------------------------------------------------------- */
/*
	そとから触るための関数
*/

static int current_stream_id = -1;

void GM_JimakuShow( int stream_id, char *mes )
{
   Work *work = jimaku_work;

   if( work->disable )
      return;

   {
#if BP_USE_NEW_FONT_SYSTEM()
      BP_FONT_DRAWINFO dr;

      BP_font_begin_render_texture(&work->bp_vinfo, 1);

      BP_font_open_drawinfo(&dr, &work->bp_vinfo);
      BP_font_set_color(&dr, 240, 240, 240, 128);
      BP_font_draw_string(&dr, mes);

      BP_font_end_render_texture(&work->bp_vinfo);
      // set_pos will correctly update us to the new buffered texture handle

      work->width = BP_font_get_draw_width(&dr);
#else
      FONT_DRAWINFO dr;

	   memset( ( char * )work->vram, 0x00, VRAM_SIZE );

 	   font_open_drawinfo( &dr, &work->vinfo );
	   font_draw_string( &dr, mes );

	   work->width = font_get_draw_width( &dr );

	   send_draw_pattern( work );
#endif
   }

	set_pos( work );

	current_stream_id = stream_id;

	work->dmapack.flag &= ~DG_DMAPACK_INVISIBLEMENU;
}

void GM_JimakuHide( void )
{
	current_stream_id = -1;

#ifndef KP_WINDOWS
	jimaku_work->dmapack.flag |= DG_DMAPACK_INVISIBLEMENU;
#else
	if( jimaku_work ){ jimaku_work->dmapack.flag |= DG_DMAPACK_INVISIBLEMENU; }
#endif
}

void GM_JimakuSetPosY( int posy )
{
	Work *work = jimaku_work;

	if( posy < 0 ){
		work->posy = YTOP;
	} else {
		work->posy = posy;
	}

	set_pos( work );
}

void GM_JimakuSetZoom( int rate )
{
	Work *work = jimaku_work;

	work->rate = rate;

	set_pos( work );
}

int GM_JimakuGetStreamID( void )
{
	return current_stream_id;
}

void *NewJimaku( int name, int map )
{
	char *mes;
	mes = GCL_GetNextString();

	GM_JimakuShow( 0, mes );
	
	return ( void * )1;
}

void GM_JimakuDisable( void )
{
	Work *work = jimaku_work;
	work->disable = 1;
	GM_JimakuHide();
}
