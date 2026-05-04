//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   text_scn.c
	テキストスクリーン補助キャラ（メモリを食うので注意）

	2000/10/07	K.Takabe
	$Id: xtextscn.c,v 1.5 2002/11/23 12:16:41 Yoshizawa1 Exp $
*/

/* ---------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "xtextscn.h"

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"def_dma.h"
#include	"libdg.cnf"
#include	"dmapack.h"
#include	"sprite_2d.h"
#include "menu.h"

#include	"../codec/codecmem.h"

#include "BP_BuildDefines.h"

#include "BP_Renderer.h"

#if BP_USE_NEW_FONT_SYSTEM()
#include "BP_Font.h"
#else
#include	"font.h"
#endif



/* ---------------------------------------------------------------- */
/* プロトタイプ宣言 */
/* コールバック関数を設定する *///yano add 2002.05.15
void MENU_SetCallbackTextTexture( char *( *callback )( void *dr, char *now, int no ) );
/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work );
void MENU_ConvertTexture( void *work );
/* 説明文を表示する */
void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
void MENU_PutTextScreenF( void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col );
/* テキストテクスチャ管理デーモン */
void *NewTextScreenControl( void );
void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag );
/* ---------------------------------------------------------------- */
//#define TEXT_VRAM_WIDTH		( 768 )		/* フォント展開領域幅 */
//#define TEXT_VRAM_HEIGHT	( 512 )		/* フォント展開領域高さ */
#define TEXT_VRAM_WIDTH		( work->vram_width )		/* フォント展開領域幅 */
#define TEXT_VRAM_HEIGHT	( work->vram_height )		/* フォント展開領域高さ */
#define TEXT_VRAM_WIDTH_DEF		( 768 )		/* フォント展開領域幅 */
#define TEXT_VRAM_HEIGHT_DEF	( 512 )		/* フォント展開領域高さ */
#define TEXT_TEX_FMT		( SCE_GS_PSMT4 )		/* 説明文テクスチャフォーマット */
#define	TEXT_TEX_BASE	(TEXTURE_TOP_PAGE())	/* 説明文テクスチャベースアドレス */
#define	TEXT_TEX_WIDTH	( ( TEXT_VRAM_WIDTH  + 127 ) &0xff80 )	/* 説明文テクスチャVRAM幅 */
#define	TEXT_TEX_HEIGHT	( ( TEXT_VRAM_HEIGHT + 127 ) &0xff80 )	/* 説明文テクスチャVRAM高さ */
#define TEXT_CLUT_FMT	( SCE_GS_PSMCT32 )		/* 説明文ＣＬＵＴフォーマット */
#define	TEXT_CLUT_BASE	(TEXTURE_TOP_PAGE()+(TEXT_TEX_WIDTH/128)*(TEXT_TEX_HEIGHT/128)*32*64)	/* ＣＬＵＴベース */


/* ---------------------------------------------------------------- */
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))


/* ---------------------------------------------------------------- */
/* 符号なし８ビットから符号なし１６ビットへ拡張 */
#define PUBEXT(_d) ({ u_long128 __tmp ; asm ("pextlb %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* １６ビットから８ビットへパッキング */
#define PUBPAC(_d) ({ u_long128 __tmp ; asm ("ppacb %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* ３２ビットから１６ビットへパッキング */
#define PUHPAC(_d) ({ u_long128 __tmp ; asm ("ppacw %0,$0,%1":"=r"(__tmp):"r"(_d) ); __tmp ; })
/* １６ビット乗算（符号付き） */
#define PHMUL(_a,_b) ({ u_long128 __tmp ; asm ("pmulth $0,%1,%2;pmfhl.sh %0":"=r"(__tmp):"r"(_a),"r"(_b) ); __tmp ;})

/* ---------------------------------------------------------------- */

#include "text_scn.h"
#define Work TextScn_Work
#define PRIM_CONTROL TextScn_PRIM_CONTROL

//static Work	*work_ptr = NULL ;

/* フォント用ワーク */
static ALIGN16_PRE u_int	font_clut[32] ALIGN16_POST ;		/* フォントテクスチャ用CLUT */\
static char* MENU_CallbackFunc = NULL;/* コールバック関数 */

/*------------------------------------------------------------------*/
/* プリミティブコントロールの確保 */
static void MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id, int dmapack_flag,
							 int phase)	// phase を追加 2002/08/26 M.Kobayashi
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU|dmapack_flag, phase, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	/* テクスチャパケットの取得 */
	prim_ctrl->tri_id = tri_id ;
	//prim_ctrl->tex_list = DG_GetTextureList( tri_id );
#if 0
	/* DMAPACKにバッファを設定 */
	dmapack->packet[0] = prim_ctrl->buffer[0] ;
	dmapack->packet[1] = prim_ctrl->buffer[1] ;
#endif
	dmapack->autopacket = buffer ;

	/* その他初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->end_offset = buffer_size ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
}

/* プリミティブコントロールの開放 */
static void FreePrimControl( PRIM_CONTROL *prim_ctrl )
{
	DG_DequeueDmapack( prim_ctrl->dmapack );
	DG_FreeDmapack( prim_ctrl->dmapack );
	GV_DelayedFree( prim_ctrl->buffer[0] );
}

/* 終端タグの書き込み */
static void ClosePrimControl( PRIM_CONTROL *prim_ctrl )
{
	DG_SetDmapackEnd( prim_ctrl->current_buffer );
	/* 追加書き込みをする為終端後アドレスは記録しない（次に上書きできるように） */
}

/* プリミティブコントロールの初期化 */
static void InitPrimControl( PRIM_CONTROL *prim_ctrl )
{
	int		size ;

	if ( prim_ctrl->current_offset >= ( prim_ctrl->end_offset - 1 ) ){
		printf("%s: prim buffer over error!!!\n", __FILE__);
	}
	/* 書き込み位置の初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
	/* 終端タグの書き込み */
	ClosePrimControl( prim_ctrl );
}

/* ---------------------------------------------------------------- */
static void Font2Texture( DG_TEX_LIN *tex, char *font_vram, unsigned short *pal )
{
#if 0
	unsigned short	*dst ;
	unsigned char	*src ;
	int				i, j, index ;
	dst = tex->image ;
	src = font_vram ;
	for ( j = 0 ; j < tex->height ; j++ ){
		for ( i = 0 ; i < tex->width ; i += 2 ){
			index = *src & 0x0f ;
			*dst++ = pal[ index ];
			index = ( *src >> 4 ) & 0x0f ;
			*dst++ = pal[ index ];
			src++ ;
		}
	}
#else
	/* ８ビットアルファ */
	unsigned char	*dst ;
	unsigned char	*src ;
	static unsigned char	pal2alpha[4] = {0,255*1/3,255*2/3,255};
	int				i, j, index ;
	dst = tex->image ;
	src = font_vram ;
	for ( j = 0 ; j < tex->height ; j++ ){
		for ( i = 0 ; i < tex->width ; i += 2 ){
			index = *src & 0x0f ;
			*dst++ = pal2alpha[ index ];
			index = ( *src >> 4 ) & 0x0f ;
			*dst++ = pal2alpha[ index ];
			src++ ;
		}
	}
#endif

#if 1 //BP def KP_WINDOWS
	DG_LinerTextureSetImageDirty(tex) ;	// image変更内容を適用させる
#endif
}
/* �テクスチャが明るくなり過ぎないように色に補正を掛ける */
static u_int ScaleColor( u_int col )
{
	u_int	r, g, b, a ;
	r = ( ( col & 0x00ff0000 ) * 192 / 256 ) & 0x00ff0000 ;
	g = ( ( col & 0x0000ff00 ) * 192 / 256 ) & 0x0000ff00 ;
	b = ( ( col & 0x000000ff ) * 192 / 256 ) & 0x000000ff ;
	col &= 0xff000000 ;
	col |= r ;
	col |= g ;
	col |= b ;
	return ( col );
}

/* コールバック関数を設定する *///yano add 2002.05.15
void MENU_SetCallbackTextTexture( char *( *callback )( void *dr, char *now, int no ) ){
	MENU_CallbackFunc = (char*)callback;
}

/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work )
{
	Work	*work_ptr = work ;

	if ( work_ptr == NULL ) return ;

#if BP_USE_NEW_FONT_SYSTEM()
   BP_font_clear_texture(&work_ptr->bp_vinfo);
#else
	GV_ZeroMemory( work_ptr->vram, work_ptr->vram_size );
#endif
}
/* VRAM領域をライン単位で部分的にクリアする */
void MENU_ClearPartTextTexture( void *work_ptr, int start_line, int height )
{
	Work	*work = work_ptr ;

	if ( work_ptr == NULL ) 
      return ;

	/* 念のためフォント展開領域のチェックを行う */
	if ( start_line < 0 ) 
      start_line = 0 ;

	if ( ( start_line + height ) > TEXT_VRAM_HEIGHT ) 
      height = TEXT_VRAM_HEIGHT - start_line - 1 ;

#if BP_USE_NEW_FONT_SYSTEM()
   {
      int y0 = BP_FONT_CONVERT_ORIGINAL_Y(start_line);
      int y1 = BP_FONT_CONVERT_ORIGINAL_Y(start_line + height);
      BP_font_clear_texture_range(&work->bp_vinfo, y0, y1);
   }
#else
	GV_ZeroMemory( (void *)((int)(work->vram)+start_line*(TEXT_VRAM_WIDTH/2)),
				   height*(TEXT_VRAM_WIDTH/2) );
#endif
}

void MENU_SetTextSpace( void *work_ptr, int space )
{
	// CreateTextTextureのspace設定がバグっていたので、
	// 無理やり設定するための関数

	Work	*work = work_ptr ;

#if BP_USE_NEW_FONT_SYSTEM()
   work->bp_vinfo.y_step = BP_FONT_CONVERT_ORIGINAL_Y(24 + space);
#else
	work->vinfo.y_step = FONT_SIZE_H + space;
#endif
}

/* 説明分テキストを展開する */
int MENU_CreateTextTexture( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message )
{
	int result = MENU_CreateTextTextureNoConvert( work_ptr, x, y, width, height, pitch, space, max_lines, message );
	MENU_ConvertTexture( work_ptr );
	return result;
}

int MENU_CreateTextTextureGetLines( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message )
{
   int result = MENU_CreateTextTextureNoConvertGetLines( work_ptr, x, y, width, height, pitch, space, max_lines, message );
   MENU_ConvertTexture( work_ptr );
   return result;
}

/* 説明分テキストを展開する
   XBOX 用追加：テクスチャ用展開作業をやらない
   ADD M.Kobayashi 2002/04/24
 */
int MENU_CreateTextTextureNoConvert_VariableResult( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message, int getResultType )
{
	Work	*work = work_ptr ;
#if BP_USE_NEW_FONT_SYSTEM()
   BP_FONT_DRAWINFO dr;
#else
	FONT_DRAWINFO dr;
#endif
  
	if ( work_ptr == NULL ) return ( 0 );
	/* 重複変更チェック */
	//if ( work->last_message == message ) return ( 0 );
	//work->last_message = message ;

	/* 念のためフォント展開領域のチェックを行う */
	if ( x < 0 ) x = 0 ;
	if ( y < 0 ) y = 0 ;
	if ( ( x + width ) > TEXT_VRAM_WIDTH ) width = TEXT_VRAM_WIDTH - x - 1 ;
	if ( ( y + height ) > TEXT_VRAM_HEIGHT ) height = TEXT_VRAM_HEIGHT - y - 1 ;

#if BP_USE_NEW_FONT_SYSTEM()
   BP_font_begin_render_texture(&work->bp_vinfo, 0);

   work->bp_vinfo.c_skip = BP_FONT_CONVERT_ORIGINAL_X(pitch);
   work->bp_vinfo.l_skip = BP_FONT_CONVERT_ORIGINAL_Y(space);
   work->bp_vinfo.width = BP_FONT_CONVERT_ORIGINAL_X(x + width);
   work->bp_vinfo.height = BP_FONT_CONVERT_ORIGINAL_Y(y + height);
   work->bp_vinfo.max_lines = max_lines;

   BP_font_open_drawinfo( &dr, &work->bp_vinfo );
   dr.xtop = BP_FONT_CONVERT_ORIGINAL_X(x);
   dr.ytop = BP_FONT_CONVERT_ORIGINAL_Y(y);
   BP_font_set_draw_callback( &dr, MENU_CallbackFunc );/* コールバック関数設定 */
   BP_font_set_color(&dr, 255, 255, 255, 128);
   BP_font_draw_string( &dr, message );
   
   BP_font_end_render_texture(&work->bp_vinfo);

   if( getResultType == 0 )
      return BP_FONT_CONVERT_NEW_X( work->bp_vinfo.max_width );
   else
      return dr.yc;
#else
	work->vinfo.c_skip = pitch ;
	work->vinfo.l_skip = space ;
	work->vinfo.width = x + width ;
	work->vinfo.height = y + height ;

	font_open_drawinfo( &dr, &work->vinfo );
	dr.xtop = x ;
	dr.ytop = y ;
	font_set_draw_callback( &dr, MENU_CallbackFunc );/* コールバック関数設定 */
	font_draw_string( &dr, message );

   if( getResultType == 0 )
	   return ( work->vinfo.max_width );
   else
      return dr.yc;
#endif
}

int MENU_CreateTextTextureNoConvert( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message )
{
   return MENU_CreateTextTextureNoConvert_VariableResult(work_ptr, x, y, width, height, pitch, space, max_lines, message, 0);
}

int MENU_CreateTextTextureNoConvertGetLines( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message )
{
   return MENU_CreateTextTextureNoConvert_VariableResult(work_ptr, x, y, width, height, pitch, space, max_lines, message, 1);
}

void MENU_ConvertTexture( void* work_ptr )
{
#if !BP_USE_NEW_FONT_SYSTEM()
   /* XBOX用テクスチャ用に展開 */
	Work	*work = work_ptr ;
	unsigned short	pal[4] = {
		0x0ccc, 0x4ccc, 0xaccc, 0xfccc
	};
	Font2Texture( work->tex, work->vram, pal );
#endif
}



/* 説明文を表示する */
void MENU_PutTextScreen( void *work_ptr, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col )
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	void			*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;

	/* テキスト描画プリミティブ設定 */
	prim_ctrl = &work->text_ctrl ;

#if BP_USE_NEW_FONT_SYSTEM()
   {
      unsigned int BPCurrentTexture = work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex];
      prim_ctrl->current_buffer = DG_SetDmapackTextureDynamic01( prim_ctrl->current_buffer, NULL, BPCurrentTexture );

      u1 = BP_FONT_CONVERT_ORIGINAL_X(u1);
      v1 = BP_FONT_CONVERT_ORIGINAL_Y(v1);
      u2 = BP_FONT_CONVERT_ORIGINAL_X(u2);
      v2 = BP_FONT_CONVERT_ORIGINAL_Y(v2);

      fu0 = ( (float)u1 + 0.5f ) / BP_GetDynamicTextureWidth(BPCurrentTexture);
      fv0 = ( (float)v1 + 0.5f ) / BP_GetDynamicTextureHeight(BPCurrentTexture);
      fu1 = ( (float)u2 + 0.5f ) / BP_GetDynamicTextureWidth(BPCurrentTexture) ;
      fv1 = ( (float)v2 + 0.5f ) / BP_GetDynamicTextureHeight(BPCurrentTexture);
   }
#else
	prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, work->tex );

   fu0 = ( ( (float)u1 + 0.5f ) / (float)work->tex->width ) ;
   fv0 = ( ( (float)v1 + 0.5f ) / (float)work->tex->height ) ;
   fu1 = ( ( (float)u2 - 0.5f ) / (float)work->tex->width ) ;
   fv1 = ( ( (float)v2 - 0.5f ) / (float)work->tex->height ) ;

#endif

   prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = x1 * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = y1 * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = x2 * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = y2 * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
												  fx0, fy0, fu0, fv0,
                                      fx1, fy1, fu1, fv1, DG_MakeDmaPackColorFromInt(ScaleColor( col )) );

	/* 終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );

}
/* 説明文を表示する */
void MENU_PutTextScreenF( void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col )
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	void			*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;

	/* テキスト描画プリミティブ設定 */
	prim_ctrl = &work->text_ctrl ;

#if BP_USE_NEW_FONT_SYSTEM()
   {
      unsigned int BPCurrentTexture = work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex];
      prim_ctrl->current_buffer = DG_SetDmapackTextureDynamic01( prim_ctrl->current_buffer, NULL, BPCurrentTexture );

      u1 = BP_FONT_CONVERT_ORIGINAL_X(u1);
      v1 = BP_FONT_CONVERT_ORIGINAL_Y(v1);
      u2 = BP_FONT_CONVERT_ORIGINAL_X(u2);
      v2 = BP_FONT_CONVERT_ORIGINAL_Y(v2);

      fu0 = ( (float)u1 + 0.5f ) / BP_GetDynamicTextureWidth(BPCurrentTexture);
      fv0 = ( (float)v1 + 0.5f ) / BP_GetDynamicTextureHeight(BPCurrentTexture);
      fu1 = ( (float)u2 + 0.5f ) / BP_GetDynamicTextureWidth(BPCurrentTexture) ;
      fv1 = ( (float)v2 + 0.5f ) / BP_GetDynamicTextureHeight(BPCurrentTexture);
   }
#else
   prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, work->tex );

   fu0 = ( ( (float)u1 + 0.5f ) / (float)work->tex->width ) ;
   fv0 = ( ( (float)v1 + 0.5f ) / (float)work->tex->height ) ;
   fu1 = ( ( (float)u2 - 0.5f ) / (float)work->tex->width ) ;
   fv1 = ( ( (float)v2 - 0.5f ) / (float)work->tex->height ) ;
#endif

	prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = x1 * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = y1 * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = x2 * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = y2 * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
												  fx0, fy0, fu0, fv0,
												  fx1, fy1, fu1, fv1, DG_MakeDmaPackColorFromInt(ScaleColor( col )) );

	/* 終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );
}

/* テキスト用テクスチャをVRAMへ転送 */
void MENU_TransTextTexture(void *work_ptr)
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;

	if ( work_ptr == NULL ) return ;

}

/* 説明文を表示する */
void MENU_PutTextScreenNoTR( void *work_ptr, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col )
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	void			*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;

	/* テキスト描画プリミティブ設定 */
	prim_ctrl = &work->text_ctrl ;

#if BP_USE_NEW_FONT_SYSTEM()
   {
      unsigned int BPCurrentTexture = work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex];
      prim_ctrl->current_buffer = DG_SetDmapackTextureDynamic01( prim_ctrl->current_buffer, NULL, BPCurrentTexture);

      u1 = BP_FONT_CONVERT_ORIGINAL_X(u1);
      v1 = BP_FONT_CONVERT_ORIGINAL_Y(v1);
      u2 = BP_FONT_CONVERT_ORIGINAL_X(u2);
      v2 = BP_FONT_CONVERT_ORIGINAL_Y(v2);

      fu0 = ( (float)u1 + 0.5f ) / BP_GetDynamicTextureWidth(BPCurrentTexture);
      fv0 = ( (float)v1 + 0.5f ) / BP_GetDynamicTextureHeight(BPCurrentTexture);
      fu1 = ( (float)u2 + 0.5f ) / BP_GetDynamicTextureWidth(BPCurrentTexture) ;
      fv1 = ( (float)v2 + 0.5f ) / BP_GetDynamicTextureHeight(BPCurrentTexture);
   }
#else
   prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, work->tex );

   fu0 = ( ( (float)u1 + 0.5f ) / (float)work->tex->width ) ;
   fv0 = ( ( (float)v1 + 0.5f ) / (float)work->tex->height ) ;
   fu1 = ( ( (float)u2 - 0.5f ) / (float)work->tex->width ) ;
   fv1 = ( ( (float)v2 - 0.5f ) / (float)work->tex->height ) ;
#endif

	prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = x1 * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = y1 * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = x2 * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = y2 * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
												  fx0, fy0, fu0, fv0,
												  fx1, fy1, fu1, fv1, DG_MakeDmaPackColorFromInt(ScaleColor( col )) );

	/* 終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );
}


/* ---------------------------------------------------------------- */

static void Act( Work *work )
{
	/* 書き込み位置の初期化のみ行う */
	InitPrimControl( &work->text_ctrl );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	FreePrimControl( &work->text_ctrl );
#if BP_USE_NEW_FONT_SYSTEM()
   if( work->pbp_BufferedTexture )
   {
      BP_FreeDynamicTexture_Buffered_Pointer(work->pbp_BufferedTexture);
      work->pbp_BufferedTexture = NULL;
   }
#else
	GV_DelayedFree( work->vram );
   DG_FreeLinerTexture( work->tex );
#endif


	//work_ptr = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	int		ruby_height = 12 ;
	int		add_flag = 0 ;

	if ( work->flag & 0x0001 )	ruby_height = 0 ;
	if ( work->flag & 0x0002 )	add_flag = DG_DMAPACK_PRIVILEGE ;
	if ( work->flag & 0x0004 )	ruby_height = 4 ;

	/* プリミティブメモリ確保 */
	MakePrimControl( &work->text_ctrl,
					 1024,
					 (work->flag & 0x0008) ? 255 : 240,
					 MENU_TRI, add_flag,
					 (work->flag & 0x0008) ? DG_DMAPACK_PHASE_LAST : DG_DMAPACK_PHASE_AFTER );

   /* フォント展開テクスチャ確保 */
	/* フォント展開領域確保 */
#if BP_USE_NEW_FONT_SYSTEM()
   if( work->bufferedTextFlag )
   {
      work->pbp_BufferedTexture = BP_AllocDynamicTexture_Buffered_Pointer(BP_FONT_CONVERT_ORIGINAL_X(TEXT_VRAM_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(TEXT_VRAM_HEIGHT), 1);
      BP_font_set_vraminfo_texture_buffered(&work->bp_vinfo, work->pbp_BufferedTexture, 0, BP_FONT_CONVERT_ORIGINAL_Y(ruby_height), FONT_NO_KINSOKU);
   }
   else
   {
      work->pbp_BufferedTexture = BP_AllocDynamicTexture_SingleBuffered_Pointer(BP_FONT_CONVERT_ORIGINAL_X(TEXT_VRAM_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(TEXT_VRAM_HEIGHT), 1);
      BP_font_set_vraminfo_texture(&work->bp_vinfo, work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex], 0, BP_FONT_CONVERT_ORIGINAL_Y(ruby_height), FONT_NO_KINSOKU);
   }

#else
   work->tex = DG_MakeLinerTexture( TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT, DG_TEXLIN_FORMAT_A8 );

   work->vram_size = TEXT_VRAM_WIDTH * TEXT_VRAM_HEIGHT / 2 ;
	work->vram = GV_Malloc( work->vram_size );
	if ( work->vram == NULL ){
		printf("text_scn.c: memory alloc error!!\n");
		return ( -1 );
	}
	GV_ZeroMemory( work->vram, work->vram_size );
	/* フォント展開領域設定 */
	font_set_vraminfo( &work->vinfo,
					  work->vram, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
					  0, ruby_height, FONT_NO_KINSOKU );
#endif

	/* フォントＣＬＵＴ作成 */
	//font_set_clut4( font_clut, 0, 
	//			   FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) );
	font_clut[0] = 0x00c0c0c0 ;
	font_clut[1] = 0x2ac0c0c0 ;
	font_clut[2] = 0x54c0c0c0 ;
	font_clut[3] = 0x80c0c0c0 ;

	Act( work );

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		テキストテクスチャ管理デーモン
	*/
void *NewTextScreenControl( void )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX(&(work->actor));

		work->vram_width = TEXT_VRAM_WIDTH_DEF ;
		work->vram_height = TEXT_VRAM_HEIGHT_DEF ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

	/*
		テキストテクスチャ管理デーモン
	*/
/*
	int		text_vram_width ;		フォント展開ＶＲＡＭ幅
	int		text_vram_height ;		フォント展開ＶＲＡＭ高さ
	int		prio ;					DG_DMAPACKのプライオリティ（現在未使用：=240）
	int		flag ;					起動フラグ
									0x0001	ルビ表示領域なしモード
									0x0002	特権モード

									0x0008	最後 ( PHASE_AFTER, prio =255 ) に描く
*/
void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX(&(work->actor));

		work->vram_width = text_vram_width ;
		work->vram_height = text_vram_height ;
		work->flag = flag ;
      work->bufferedTextFlag = bufferedTextFlag;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


/* ---------------------------------------------------------------- */
static int GetResourcesForTitle( Work *work )
{
	/* プリミティブメモリ確保 */
	MakePrimControl( &work->text_ctrl,
					2048,
					240, MENU_TRI, 0, DG_DMAPACK_PHASE_AFTER);

#if BP_USE_NEW_FONT_SYSTEM()   
   if( work->bufferedTextFlag )
   {
      work->pbp_BufferedTexture = BP_AllocDynamicTexture_Buffered_Pointer( BP_FONT_CONVERT_ORIGINAL_X(TEXT_VRAM_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(TEXT_VRAM_HEIGHT), 1);
      BP_font_set_vraminfo_texture_buffered(&work->bp_vinfo, work->pbp_BufferedTexture, 0, BP_FONT_CONVERT_ORIGINAL_Y(12), FONT_NO_KINSOKU);
   }
   else
   {
      work->pbp_BufferedTexture = BP_AllocDynamicTexture_SingleBuffered_Pointer( BP_FONT_CONVERT_ORIGINAL_X(TEXT_VRAM_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(TEXT_VRAM_HEIGHT), 1);
      BP_font_set_vraminfo_texture(&work->bp_vinfo, work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex], 0, BP_FONT_CONVERT_ORIGINAL_Y(12), FONT_NO_KINSOKU);
   }
#else
   /* フォント展開テクスチャ確保 */
   work->tex = DG_MakeLinerTexture( TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT, DG_TEXLIN_FORMAT_A8 );
	/* フォント展開領域確保 */
	work->vram_size = TEXT_VRAM_WIDTH * TEXT_VRAM_HEIGHT / 2 ;
	work->vram = GV_Malloc( work->vram_size );
	if ( work->vram == NULL ){
		printf("text_scn.c: memory alloc error!!\n");
		return ( 1 );
	}
	GV_ZeroMemory( work->vram, work->vram_size );
	/* フォント展開領域設定 */
	font_set_vraminfo( &work->vinfo, work->vram, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
					   0,12, FONT_NO_KINSOKU );

#endif

	/* フォントＣＬＵＴ作成 */
	//font_set_clut4( font_clut, 0, 
	//			   FONT_RGB( 200, 200, 200 ), FONT_RGB( 0, 0, 0 ) );
	font_clut[0] = 0x00c0c0c0 ;
	font_clut[1] = 0x2ac0c0c0 ;
	font_clut[2] = 0x54c0c0c0 ;
	font_clut[3] = 0x80c0c0c0 ;

	Act( work );

	return ( 0 );
}

/* ---------------------------------------------------------------- */

#define TEXT_VRAM_HEIGHT_FOR_TITLE		768


	/*
		テキストテクスチャ管理デーモン
	*/
void *NewTextScreenControlForTitle( void )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX(&(work->actor));

		work->vram_width = TEXT_VRAM_WIDTH_DEF ;
		work->vram_height = TEXT_VRAM_HEIGHT_FOR_TITLE ;
		if ( GetResourcesForTitle( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


/*------------------------------------------------------------------*/
/* プリミティブコントロールの確保 */
static void MakePrimControlForCodec( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id )
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack
		= DG_MakeDmapack2( DG_DMAPACK_MENU|DG_DMAPACK_PRIVILEGE, DG_DMAPACK_PHASE_AFTER, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = codecMalloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	/* テクスチャパケットの取得 */
	prim_ctrl->tri_id = tri_id ;
	//prim_ctrl->tex_list = DG_GetTextureList( tri_id );
#if 0
	/* DMAPACKにバッファを設定 */
	dmapack->packet[0] = prim_ctrl->buffer[0] ;
	dmapack->packet[1] = prim_ctrl->buffer[1] ;
#endif
	dmapack->autopacket = buffer ;

	/* その他初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->end_offset = buffer_size ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ 0 ] ;
}

/* プリミティブコントロールの開放 */
static void FreePrimControlForCodec( PRIM_CONTROL *prim_ctrl )
{
	DG_DequeueDmapack( prim_ctrl->dmapack );
	DG_FreeDmapack( prim_ctrl->dmapack );
	codecDelayedFree( prim_ctrl->buffer[0] );
}

/* ---------------------------------------------------------------- */

static void DieForCodec( Work *work )
{
	FreePrimControlForCodec( &work->text_ctrl );
#if BP_USE_NEW_FONT_SYSTEM()
   if( work->pbp_BufferedTexture )
   {
      BP_FreeDynamicTexture_Buffered_Pointer(work->pbp_BufferedTexture);
      work->pbp_BufferedTexture = NULL;
   }
#else
   codecDelayedFree( work->vram );
#ifdef KP_WINDOWS
	DG_FreeLinerTexture2( work->tex );
#endif
	// Add by K.Uehara
	codecDelayedFree( work->imagetop );
#endif
	//work_ptr = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResourcesForCodec( Work *work )
{
	/* プリミティブメモリ確保 */
	MakePrimControlForCodec( &work->text_ctrl,
							 2048,
							 240, MENU_TRI );

#if BP_USE_NEW_FONT_SYSTEM()
   if( work->bufferedTextFlag )
   {
      work->pbp_BufferedTexture = BP_AllocDynamicTexture_Buffered_Pointer(BP_FONT_CONVERT_ORIGINAL_X(TEXT_VRAM_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(TEXT_VRAM_HEIGHT), 1);
      BP_font_set_vraminfo_texture_buffered(&work->bp_vinfo, work->pbp_BufferedTexture, 0, BP_FONT_CONVERT_ORIGINAL_Y(12), FONT_NO_KINSOKU);
   }
   else
   {
      work->pbp_BufferedTexture = BP_AllocDynamicTexture_SingleBuffered_Pointer(BP_FONT_CONVERT_ORIGINAL_X(TEXT_VRAM_WIDTH), BP_FONT_CONVERT_ORIGINAL_Y(TEXT_VRAM_HEIGHT), 1);
      BP_font_set_vraminfo_texture(&work->bp_vinfo, work->pbp_BufferedTexture->aTextureHandle[work->pbp_BufferedTexture->currIndex], 0, BP_FONT_CONVERT_ORIGINAL_Y(12), FONT_NO_KINSOKU);
   }
#else
	/* フォント展開領域確保 */
	work->vram_size = TEXT_VRAM_WIDTH * TEXT_VRAM_HEIGHT / 2 ;
	work->vram = codecMalloc( work->vram_size );
	if ( work->vram == NULL ){
		printf("text_scn.c: memory alloc error!!\n");
		return ( 1 );
	}
	/* フォント展開領域設定 */
	font_set_vraminfo( &work->vinfo, work->vram, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
					   0,12, FONT_NO_KINSOKU );

   GV_ZeroMemory( work->vram, work->vram_size );

   /* フォント展開テクスチャ確保 */
   // ADD K.Uehara
   work->imagetop = codecMalloc( TEXT_VRAM_WIDTH * TEXT_VRAM_HEIGHT * sizeof( char )
      + sizeof( DG_TEX_LIN ) + 128 );
   {
      unsigned int image = ( unsigned int )work->imagetop;
      DG_TEX_LIN *texbody;
      texbody = ( DG_TEX_LIN * )image;
      GV_ZeroMemory( texbody, sizeof( *texbody ) );
      image += sizeof( DG_TEX_LIN );
      image = ( image + 127 ) & ~0x7F;
      work->tex = texbody;
      //DG_MakeLinerTexture2( texbody, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT
      //					 , DG_TEXLIN_FORMAT_A4R4G4B4, ( void * )image );
      DG_MakeLinerTexture2( texbody, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT
         , DG_TEXLIN_FORMAT_A8, ( void * )image );
   }
#endif

   /* フォントＣＬＵＴ作成 */
	font_clut[0] = 0x00c0c0c0 ;
	font_clut[1] = 0x2ac0c0c0 ;
	font_clut[2] = 0x54c0c0c0 ;
	font_clut[3] = 0x80c0c0c0 ;
	

	Act( work );

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		テキストテクスチャ管理デーモン
	*/
void *NewTextScreenControlForCodec( void )
{
	Work			*work ;

	work=codecMalloc(sizeof(Work));

	if(work!=NULL){
		/* アクター起動 */
		GV_ZeroMemory(work,sizeof(Work));
		GV_SetActorFreeFunc(work,codecFree);
		GV_SetActorClass(work,GV_CLASS_OBJECT);
		GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
		GV_InsertActorPriority(GV_ACTOR_DAEMON,work,0x1FF);

		GV_SetActor(work,Act,DieForCodec);
		GV_ActorEX(&(work->actor));

		work->vram_width = TEXT_VRAM_WIDTH_DEF ;
		work->vram_height = TEXT_VRAM_HEIGHT_FOR_TITLE ;
		if ( GetResourcesForCodec( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;
}
