//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	text_scn.c
	テキストスクリーン補助キャラ（メモリを食うので注意）

	2000/10/07	K.Takabe
	$Id: text_scn.c,v 1.1.1.3 2002/11/19 11:45:14 Yoshizawa1 Exp $
*/

/* ---------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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
#include	"font.h"
#include	"sprite_2d.h"
#include "menu.h"

#include	"../codec/codecmem.h"


/* ---------------------------------------------------------------- */
/* プロトタイプ宣言 */
/* コールバック関数を設定する *///yano add 2002.05.15
void MENU_SetCallbackTextTexture( char *( *callback )( FONT_DRAWINFO *dr, char *now, int no ) );
/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work );
/* 説明分テキストを展開する */
int MENU_CreateTextTexture( void *work, int x, int y, int width, int height, int pitch, int space, char *message );
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

/* テキスト描画用プリミティブ */
typedef struct {
	DG_DMATAG		dmatag ;		/* 本体転送タグ */
	DG_GIFTAG		gif_tag ;		/* ＧＳ設定用GIFタグ */
	struct _text_setup{
		DG_GSREG	texflush ;		/* テクスチャキャッシュのフラッシュ */
		//DG_GSREG	tex1 ;			/* テクスチャ設定 */
#ifndef HIGHRESO_FFI
		DG_GSREG	xyoffset ;		/* ハイレゾ化のためのオフセット設定 */
#endif
		DG_GSREG	tex0 ;			/* テクスチャ設定 */
		DG_GSREG	clamp ;			/* クランプ設定 */
		DG_GSREG	alpha ;			/* アルファ設定 */
	} data ;
	/* テクスチャ描画 */
	DG_GIFTAG		sprt_tag ;		/* スプライトプリミティブ用GIFタグ */
	DG_MENU_SPRT	mes_sprt ;		/* スプライトプリミティブ */
} PRIM_TEXT ;




/* プリミティブ表示コントロール構造体 */
typedef struct {
	DG_DMAPACK		*dmapack ;
	int				tri_id ;			/* 使用TRI */
	DG_TEXTURE_LIST	*tex_list ;			/* TRIデータポインタ */
	int				current_offset ;	/* 書き込みデータ最終位置オフセット（1qword単位） */
	int				end_offset ;		/* 書き込みデータ限界位置オフセット（1qword単位） */
	u_long128		*buffer[2] ;		/* 転送バッファ */
	u_long128		*current_buffer ;	/* 最終書き込み位置アドレス */
} PRIM_CONTROL ;


typedef struct _text_scn_Work {
	GV_ACT_EX	actor ;
	PRIM_CONTROL	text_ctrl ;	/* 説明分表示用（テクスチャ転送パケットなども含む） */
	FONT_VRAMINFO	vinfo;			/* フォント展開ワーク */
	int				vram_width ;	/* フォント展開領域幅 */
	int				vram_height ;	/* フォント展開領域高さ */
	void			*vram ;			/* フォント展開領域 */
	int				vram_size ;
	char			*last_message ;	/* フォント展開領域に書き込まれているメッセージへのポインタ */
	int				flag ;			/* 各種フラグ */
} Work ;

//static Work	*work_ptr = NULL ;

/* フォント用ワーク */
static ALIGN16_PRE u_int	font_clut[32] ALIGN16_POST ;		/* フォントテクスチャ用CLUT */
static void* MENU_CallbackFunc = NULL;/* コールバック関数 */

/*------------------------------------------------------------------*/
/* プリミティブコントロールの確保 */
static void MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id, int dmapack_flag )
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU|dmapack_flag, DG_DMAPACK_PHASE_AFTER, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	/* テクスチャパケットの取得 */
	prim_ctrl->tri_id = tri_id ;
	prim_ctrl->tex_list = DG_GetTextureList( tri_id );
	/* DMAPACKにバッファを設定 */
	dmapack->packet[0] = prim_ctrl->buffer[0] ;
	dmapack->packet[1] = prim_ctrl->buffer[1] ;
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
	const DG_DMATAG	endtag = { DMATAG_SET_QWC( DMATAG_ID_RET, 0 ), NULL,
								 {SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ) } } ;
	DG_DMATAG	*tag ;
	tag = (DG_DMATAG*)prim_ctrl->current_buffer ;
	//tag = GV_UNCACHEA( tag );
	*tag = endtag ;
}

/* プリミティブコントロールの初期化 */
static void InitPrimControl( PRIM_CONTROL *prim_ctrl )
{
	//int		size ;

	if ( prim_ctrl->current_offset >= ( prim_ctrl->end_offset - 1 ) ){
		printf("%s: prim buffer over error!!! %d %d\n", __FILE__,
			   prim_ctrl->current_offset,prim_ctrl->end_offset);
	}
	/* 書き込み位置の初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ DG_Clock ] ;
#if 0
	/* テクスチャ転送パケットの設定 */
	size = DG_WriteTextureChangePacks( prim_ctrl->current_buffer,
									  (void*)&prim_ctrl->tex_list->tex_packet[ DG_Clock ] );
	prim_ctrl->current_offset += size ;
	prim_ctrl->current_buffer += size ;
#endif
	/* 終端タグの書き込み */
	ClosePrimControl( prim_ctrl );
}

/* 座標設定 */
static void SetSpritePos( DG_MENU_SPRT *sprt, int x1, int y1, int x2, int y2 )
{
#ifdef BP_PSX2_ASM
	static FVECTOR	scale = { (float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT, 
								(float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT };
	static FVECTOR	offset = { 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2,
								 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2 };
	IVECTOR			tmp ;
	tmp.vx = x1 ;
	tmp.vy = y1 ;
	tmp.vz = x2 ;
	tmp.vw = y2 ;
	asm volatile("
		lqc2			vf1,0x00(%0)	# 
		lqc2			vf2,0x00(%1)	# 
		lqc2			vf3,0x00(%2)	# 
		vitof0.xyzw		vf1,vf1			# 整数から浮動小数点へ
		vmula.xyzw		ACC,vf2,vf1		# スケール乗算
		vmaddw.xyzw		vf1,vf3,vf0		# オフセット加算
		vftoi4.xyzw		vf1,vf1			# 浮動小数点から整数へ
		sqc2			vf1,0x00(%0)	# 
	"::"r"(&tmp),"r"(&scale),"r"(&offset):"memory" );
	DG_SET_XY1( sprt, tmp.vx, tmp.vy );
	DG_SET_XY2( sprt, tmp.vz, tmp.vw );
#else
	x1 = ( (float)DRAW_WIDTH/VR_WIDTH * x1 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y1 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y1 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	x2 = ( (float)DRAW_WIDTH/VR_WIDTH * x2 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y2 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y2 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	DG_SET_XY1( sprt, x1, y1 );
	DG_SET_XY2( sprt, x2, y2 );
#endif
}
static void SetSpritePosF( DG_MENU_SPRT *sprt, float x1, float y1, float x2, float y2 )
{
#ifdef BP_PSX2_ASM
	static FVECTOR	scale = { (float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT, 
								(float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT };
	static FVECTOR	offset = { 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2,
								 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2 };
	IVECTOR			tmp ;
	tmp.vx = x1 ;
	tmp.vy = y1 ;
	tmp.vz = x2 ;
	tmp.vw = y2 ;
	asm volatile("
		lqc2			vf1,0x00(%0)	# 
		lqc2			vf2,0x00(%1)	# 
		lqc2			vf3,0x00(%2)	# 
		vitof0.xyzw		vf1,vf1			# 整数から浮動小数点へ
		vmula.xyzw		ACC,vf2,vf1		# スケール乗算
		vmaddw.xyzw		vf1,vf3,vf0		# オフセット加算
		vftoi4.xyzw		vf1,vf1			# 浮動小数点から整数へ
		sqc2			vf1,0x00(%0)	# 
	"::"r"(&tmp),"r"(&scale),"r"(&offset):"memory" );
	DG_SET_XY1( sprt, tmp.vx, tmp.vy );
	DG_SET_XY2( sprt, tmp.vz, tmp.vw );
#else
	x1 = ( (float)DRAW_WIDTH/VR_WIDTH * x1 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y1 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y1 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	x2 = ( (float)DRAW_WIDTH/VR_WIDTH * x2 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	y2 = ( (float)DRAW_HEIGHT/VR_HEIGHT * y2 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	DG_SET_XY1( sprt, x1, y1 );
	DG_SET_XY2( sprt, x2, y2 );
#endif
}
/* ＵＶ設定 */
static void SetSpriteUv( DG_MENU_SPRT *sprt, int u1, int v1, int u2, int v2 )
{
	/* UV設定 */
	DG_SET_UV1( sprt, u1, v1 );
	DG_SET_UV2( sprt, u2, v2 );
}
/* 色設定 */
inline static void SetSpriteCol( DG_MENU_SPRT *sprt, int col )
{
	//DG_SET_RGBA1( sprt, r, g, b, a );
	*(int*)&sprt->rgba1.r = col ;
}

/* ---------------------------------------------------------------- */
/* コールバック関数を設定する *///yano add 2002.05.15
void MENU_SetCallbackTextTexture( char *( *callback )( FONT_DRAWINFO *dr, char *now, int no ) ){
	MENU_CallbackFunc = callback;
}

/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work )
{
	Work	*work_ptr = work ;

	if ( work_ptr == NULL ) return ;
	GV_ZeroMemory( work_ptr->vram, work_ptr->vram_size );
}
/* VRAM領域をライン単位で部分的にクリアする */
void MENU_ClearPartTextTexture( void *work_ptr, int start_line, int height )
{
	Work	*work = work_ptr ;

	if ( work_ptr == NULL ) return ;

	/* 念のためフォント展開領域のチェックを行う */
	if ( start_line < 0 ) start_line = 0 ;
	if ( ( start_line + height ) > TEXT_VRAM_HEIGHT ) height = TEXT_VRAM_HEIGHT - start_line - 1 ;

	GV_ZeroMemory( (void *)((int)(work->vram)+start_line*(TEXT_VRAM_WIDTH/2)),
				   height*(TEXT_VRAM_WIDTH/2) );
}

/* 説明分テキストを展開する */
static int CreateTextTexture( void *work_ptr, int x, int y, int width, int height, int pitch, int space, char *message )
{
	Work	*work = work_ptr ;
	FONT_DRAWINFO dr;
  
	/* 重複変更チェック */
	//if ( work->last_message == message ) return ( 0 );
	//work->last_message = message ;

	/* 念のためフォント展開領域のチェックを行う */
	if ( x < 0 ) x = 0 ;
	if ( y < 0 ) y = 0 ;
	if ( ( x + width ) > TEXT_VRAM_WIDTH ) width = TEXT_VRAM_WIDTH - x - 1 ;
	if ( ( y + height ) > TEXT_VRAM_HEIGHT ) height = TEXT_VRAM_HEIGHT - y - 1 ;

	//GV_ZeroMemory( work->vram, work->vram_size );
	work->vinfo.c_skip = pitch ;
	work->vinfo.l_skip = space ;
	work->vinfo.width = x + width ;
	work->vinfo.height = y + height ;

	font_open_drawinfo( &dr, &work->vinfo );
	dr.xtop = x ;
	dr.ytop = y ;
	font_set_draw_callback( &dr, MENU_CallbackFunc );/* コールバック関数設定 */
	font_draw_string( &dr, message );

	return ( dr.yc );
}

void MENU_SetTextSpace( void *work_ptr, int space )
{
	// CreateTextTextureのspace設定がバグっていたので、
	// 無理やり設定するための関数

	Work	*work = work_ptr ;
	work->vinfo.y_step = FONT_SIZE_H + space;
}

/* 説明分テキストを展開する */
int MENU_CreateTextTexture( void *work_ptr, int x, int y, int width, int height, int pitch, int space, char *message )
{
	Work	*work = work_ptr ;

	if ( work_ptr == NULL ) return ( 0 );

	CreateTextTexture( work_ptr, x, y, width, height, pitch, space, message );

	return ( work->vinfo.max_width );
}

int MENU_CreateTextTextureGetLines( void *work_ptr, int x, int y, int width, int height, int pitch, int space, char *message )
{	// ADD M.Kobayashi 2002/06/21
	// 何行書いたかを得る
	if ( work_ptr == NULL ) return ( 0 );

	return CreateTextTexture( work_ptr, x, y, width, height, pitch, space, message );
}

/* 説明文を表示する */
void MENU_PutTextScreen( void *work_ptr, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col )
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_TEXT		*prim ;

	if ( work_ptr == NULL ) return ;

	/* プリミティブを１枚分確保 */
	prim_ctrl = &work->text_ctrl ;
	/* テクスチャ転送パケット生成 */
	prim_ctrl->current_buffer =
	  DG_MakeLoadImagePacket( prim_ctrl->current_buffer,
						 TEXT_TEX_FMT, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
						 TEXT_TEX_BASE, TEXT_TEX_WIDTH, work->vram );
	prim_ctrl->current_offset = ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[DG_Clock] ) / 16 ;/* 補正 */
	/* ＣＬＵＴ転送パケット生成 */
	prim_ctrl->current_buffer =
	  DG_MakeLoadImagePacket( prim_ctrl->current_buffer,
						 TEXT_CLUT_FMT, 16, 2,
						 TEXT_CLUT_BASE, 1, font_clut );
	prim_ctrl->current_offset = ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[DG_Clock] ) / 16 ;/* 補正 */

	/* テキスト描画プリミティブ設定 */
	prim = (PRIM_TEXT*)prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += SIZEOF_QWORD(PRIM_TEXT) ;
	prim_ctrl->current_offset += SIZEOF_QWORD(PRIM_TEXT) ;

	/*
		テキスト描画プリミティブの初期化
	*/
	/* 先頭ＤＭＡタグ初期化 */
	prim->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PRIM_TEXT) - 1 );
	prim->dmatag.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	prim->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PRIM_TEXT) - 1, 0 );
	/* ＧＳ設定転送用ＧＩＦタグ初期化 */
	prim->gif_tag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(struct _text_setup), 1, 0, 0, 0, 1 ) ;
	prim->gif_tag.regs = 0x0e ;
	/* ＧＳ設定パケット設定 */
	prim->data.texflush.data = 0 ;
	prim->data.texflush.reg = SCE_GS_TEXFLUSH ;
	//prim->data.tex1.data = SCE_GS_SET_TEX1(1, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0) ;
	//prim->data.tex1.reg = SCE_GS_TEX1_1 ;
#ifndef HIGHRESO_FFI
	prim->data.xyoffset.data = SCE_GS_SET_XYOFFSET( (2048-DRAW_WIDTH/2)*16, (2048-DRAW_HEIGHT/2)*16 + ( DG_CurrentField ? 8 : 0 ) ) ;
	prim->data.xyoffset.reg = SCE_GS_XYOFFSET_1 ;
#endif
	prim->data.tex0.data = SCE_GS_SET_TEX0( TEXT_TEX_BASE/64, TEXT_TEX_WIDTH/64, TEXT_TEX_FMT,
										   10,10, 1, 0,
										   TEXT_CLUT_BASE/64, TEXT_CLUT_FMT, 0, 0, 4 );
	prim->data.tex0.reg = SCE_GS_TEX0_1 ;
	prim->data.clamp.data = SCE_GS_SET_CLAMP( 2, 2, 1, TEXT_VRAM_WIDTH-2, 1, TEXT_VRAM_HEIGHT-2 ); ;
	prim->data.clamp.reg = SCE_GS_CLAMP_1 ;
	prim->data.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ;
	prim->data.alpha.reg = SCE_GS_ALPHA_1 ;
	/* スプライトプリミティブ初期化 */
	prim->sprt_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&prim->sprt_tag |= 1 ;	/* GIFパケット転送データを2に設定 */

	/* テキストテクスチャ用スプライト設定 */
	prim->mes_sprt.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
	/* ＵＶ設定 */
	SetSpriteUv( &prim->mes_sprt, ((u1)<<4)+8, ((v1)<<4)+8, ((u2)<<4)-8, ((v2)<<4)-8 );
	/* 座標設定 */
	SetSpritePos( &prim->mes_sprt, x1, y1, x2, y2 );
	/* 色設定 */
	SetSpriteCol( &prim->mes_sprt, col );
	

	/* 終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );

}
/* 説明文を表示する */
void MENU_PutTextScreenF( void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col )
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_TEXT		*prim ;

	if ( work_ptr == NULL ) return ;

	/* プリミティブを１枚分確保 */
	prim_ctrl = &work->text_ctrl ;
	/* テクスチャ転送パケット生成 */
	prim_ctrl->current_buffer =
	  DG_MakeLoadImagePacket( prim_ctrl->current_buffer,
						 TEXT_TEX_FMT, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
						 TEXT_TEX_BASE, TEXT_TEX_WIDTH, work->vram );
	prim_ctrl->current_offset = ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[DG_Clock] ) / 16 ;/* 補正 */
	/* ＣＬＵＴ転送パケット生成 */
	prim_ctrl->current_buffer =
	  DG_MakeLoadImagePacket( prim_ctrl->current_buffer,
						 TEXT_CLUT_FMT, 16, 2,
						 TEXT_CLUT_BASE, 1, font_clut );
	prim_ctrl->current_offset = ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[DG_Clock] ) / 16 ;/* 補正 */

	/* テキスト描画プリミティブ設定 */
	prim = (PRIM_TEXT*)prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += SIZEOF_QWORD(PRIM_TEXT) ;
	prim_ctrl->current_offset += SIZEOF_QWORD(PRIM_TEXT) ;

	/*
		テキスト描画プリミティブの初期化
	*/
	/* 先頭ＤＭＡタグ初期化 */
	prim->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PRIM_TEXT) - 1 );
	prim->dmatag.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	prim->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PRIM_TEXT) - 1, 0 );
	/* ＧＳ設定転送用ＧＩＦタグ初期化 */
	prim->gif_tag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(struct _text_setup), 1, 0, 0, 0, 1 ) ;
	prim->gif_tag.regs = 0x0e ;
	/* ＧＳ設定パケット設定 */
	prim->data.texflush.data = 0 ;
	prim->data.texflush.reg = SCE_GS_TEXFLUSH ;
	//prim->data.tex1.data = SCE_GS_SET_TEX1(1, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0) ;
	//prim->data.tex1.reg = SCE_GS_TEX1_1 ;
#ifndef HIGHRESO_FFI
	prim->data.xyoffset.data = SCE_GS_SET_XYOFFSET( (2048-DRAW_WIDTH/2)*16, (2048-DRAW_HEIGHT/2)*16 + ( DG_CurrentField ? 8 : 0 ) ) ;
	prim->data.xyoffset.reg = SCE_GS_XYOFFSET_1 ;
#endif
	prim->data.tex0.data = SCE_GS_SET_TEX0( TEXT_TEX_BASE/64, TEXT_TEX_WIDTH/64, TEXT_TEX_FMT,
										   10,10, 1, 0,
										   TEXT_CLUT_BASE/64, TEXT_CLUT_FMT, 0, 0, 4 );
	prim->data.tex0.reg = SCE_GS_TEX0_1 ;
	prim->data.clamp.data = SCE_GS_SET_CLAMP( 2, 2, 1, TEXT_VRAM_WIDTH-2, 1, TEXT_VRAM_HEIGHT-2 ); ;
	prim->data.clamp.reg = SCE_GS_CLAMP_1 ;
	prim->data.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ;
	prim->data.alpha.reg = SCE_GS_ALPHA_1 ;
	/* スプライトプリミティブ初期化 */
	prim->sprt_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&prim->sprt_tag |= 1 ;	/* GIFパケット転送データを2に設定 */

	/* テキストテクスチャ用スプライト設定 */
	prim->mes_sprt.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
	/* ＵＶ設定 */
	SetSpriteUv( &prim->mes_sprt, ((u1)<<4)+8, ((v1)<<4)+8, ((u2)<<4)-8, ((v2)<<4)-8 );
	/* 座標設定 */
	SetSpritePosF( &prim->mes_sprt, x1, y1, x2, y2 );
	/* 色設定 */
	SetSpriteCol( &prim->mes_sprt, col );
	

	/* 終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );

}

/* テキスト用テクスチャをVRAMへ転送 */
void MENU_TransTextTexture(void *work_ptr)
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;

	if ( work_ptr == NULL ) return ;

	/* プリミティブを１枚分確保 */
	prim_ctrl = &work->text_ctrl ;

	/* テクスチャ転送パケット生成 */
	prim_ctrl->current_buffer =
	  DG_MakeLoadImagePacket( prim_ctrl->current_buffer,
						 TEXT_TEX_FMT, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
						 TEXT_TEX_BASE, TEXT_TEX_WIDTH, work->vram );
	prim_ctrl->current_offset = ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[DG_Clock] ) / 16 ;/* 補正 */

	/* ＣＬＵＴ転送パケット生成 */
	prim_ctrl->current_buffer =
	  DG_MakeLoadImagePacket( prim_ctrl->current_buffer,
						 TEXT_CLUT_FMT, 16, 2,
						 TEXT_CLUT_BASE, 1, font_clut );
	prim_ctrl->current_offset = ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[DG_Clock] ) / 16 ;/* 補正 */

	/* 終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );
}

/* 説明文を表示する */
void MENU_PutTextScreenNoTR( void *work_ptr, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col )
{
	Work	*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	PRIM_TEXT		*prim ;

	if ( work_ptr == NULL ) return ;

	/* テキスト描画プリミティブ設定 */
	prim_ctrl = &work->text_ctrl ;
	prim = (PRIM_TEXT*)prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += SIZEOF_QWORD(PRIM_TEXT) ;
	prim_ctrl->current_offset += SIZEOF_QWORD(PRIM_TEXT) ;

	/*
		テキスト描画プリミティブの初期化
	*/
	/* 先頭ＤＭＡタグ初期化 */
	prim->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PRIM_TEXT) - 1 );
	prim->dmatag.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	prim->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PRIM_TEXT) - 1, 0 );
	/* ＧＳ設定転送用ＧＩＦタグ初期化 */
	prim->gif_tag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(struct _text_setup), 1, 0, 0, 0, 1 ) ;
	prim->gif_tag.regs = 0x0e ;
	/* ＧＳ設定パケット設定 */
	prim->data.texflush.data = 0 ;
	prim->data.texflush.reg = SCE_GS_TEXFLUSH ;
	//prim->data.tex1.data = SCE_GS_SET_TEX1(1, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0) ;
	//prim->data.tex1.reg = SCE_GS_TEX1_1 ;
#ifndef HIGHRESO_FFI
	prim->data.xyoffset.data = SCE_GS_SET_XYOFFSET( (2048-DRAW_WIDTH/2)*16, (2048-DRAW_HEIGHT/2)*16 + ( DG_CurrentField ? 8 : 0 ) ) ;
	prim->data.xyoffset.reg = SCE_GS_XYOFFSET_1 ;
#endif
	prim->data.tex0.data = SCE_GS_SET_TEX0( TEXT_TEX_BASE/64, TEXT_TEX_WIDTH/64, TEXT_TEX_FMT,
										   10,10, 1, 0,
										   TEXT_CLUT_BASE/64, TEXT_CLUT_FMT, 0, 0, 4 );
	prim->data.tex0.reg = SCE_GS_TEX0_1 ;
	prim->data.clamp.data = SCE_GS_SET_CLAMP( 2, 2, 1, TEXT_VRAM_WIDTH-2, 1, TEXT_VRAM_HEIGHT-2 ); ;
	prim->data.clamp.reg = SCE_GS_CLAMP_1 ;
	prim->data.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ;
	prim->data.alpha.reg = SCE_GS_ALPHA_1 ;
	/* スプライトプリミティブ初期化 */
	prim->sprt_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&prim->sprt_tag |= 1 ;	/* GIFパケット転送データを2に設定 */

	/* テキストテクスチャ用スプライト設定 */
	prim->mes_sprt.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
	/* ＵＶ設定 */
	SetSpriteUv( &prim->mes_sprt, ((u1)<<4)+8, ((v1)<<4)+8, ((u2)<<4)-8, ((v2)<<4)-8 );
	/* 座標設定 */
	SetSpritePos( &prim->mes_sprt, x1, y1, x2, y2 );
	/* 色設定 */
	SetSpriteCol( &prim->mes_sprt, col );
	

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
	GV_DelayedFree( work->vram );
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
					240, MENU_TRI, add_flag );

	/* フォント展開領域確保 */
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
*/
void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, 0/*bufferedTextFlag*/ )
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
					240, MENU_TRI, 0 );

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
	prim_ctrl->tex_list = DG_GetTextureList( tri_id );
	/* DMAPACKにバッファを設定 */
	dmapack->packet[0] = prim_ctrl->buffer[0] ;
	dmapack->packet[1] = prim_ctrl->buffer[1] ;
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
	codecDelayedFree( work->vram );
	//work_ptr = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResourcesForCodec( Work *work )
{
	/* プリミティブメモリ確保 */
	MakePrimControlForCodec( &work->text_ctrl,
							 2048,
							 240, MENU_TRI );

	/* フォント展開領域確保 */
	work->vram_size = TEXT_VRAM_WIDTH * TEXT_VRAM_HEIGHT / 2 ;
	work->vram = codecMalloc( work->vram_size );
	if ( work->vram == NULL ){
		printf("text_scn.c: memory alloc error!!\n");
		return ( 1 );
	}
	GV_ZeroMemory( work->vram, work->vram_size );
	/* フォント展開領域設定 */
	font_set_vraminfo( &work->vinfo, work->vram, TEXT_VRAM_WIDTH, TEXT_VRAM_HEIGHT,
					   0,12, FONT_NO_KINSOKU );
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
