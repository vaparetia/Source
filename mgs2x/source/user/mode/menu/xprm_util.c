//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	prm_util.c
	汎用簡易メニュープリミティブ管理デーモン

	2001/06/24	K.Takabe
	$Id: xprm_util.c,v 1.1.1.3 2002/11/19 11:45:15 Yoshizawa1 Exp $
*/
/*

	void *NewMenuPrimUtil( int dmapack_phase, int prio, int tri_id, int buffer_size );
	int		dmapack_phase ;		DG_DMAPACK使用フェーズ
	int		prio ;				DG_DMAPACKプライオリティ
	int		buffer_size ;		確保するプリミティブバッファサイズ（128bit単位）

		プリミティブ表示デーモンを起動


	int MENU_PrimUtil_GetUseBufferSize( void *work_ptr );
	void	*work_ptr ;			表示デーモンワーク

		現在使用しているバッファサイズを得る（128bit単位）


	void MENU_PrimUtil_PutSprite( void *work_ptr, int x, int y, int w, int h, int col, DG_TEX *tex );
	void	*work_ptr ;			表示デーモンワーク
	int		x ;					左上表示座標（仮想座標）
	int		y ;					左上表示座標（仮想座標）
	int		w ;					表示サイズ（仮想座標）
	int		h ;					表示サイズ（仮想座標）
	int		col ;				表示色
	DG_TEX	*tex ;				テクスチャ

		スプライトを表示させる


	void MENU_PrimUtil_PutBox( void *work_ptr, int x1, int y1, int x2, int y2, int col );
	void	*work_ptr ;			表示デーモンワーク
	int		x1 ;				左上表示座標（仮想座標）
	int		y1 ;				左上表示座標（仮想座標）
	int		x2 ;				右下表示座標（仮想座標）
	int		y2 ;				右下表示座標（仮想座標）
	int		col ;				表示色

		テクスチャなしスプライトを表示させる


	void MENU_PrimUtil_PutLine( void *work_ptr, int x1, int y1, int x2, int y2, int col );
	void	*work_ptr ;			表示デーモンワーク
	int		x1 ;				始点表示座標（仮想座標）
	int		y1 ;				始点表示座標（仮想座標）
	int		x2 ;				終点表示座標（仮想座標）
	int		y2 ;				終点表示座標（仮想座標）
	int		col ;				表示色

		フラットシェードラインを表示させる



*/
/*
プロトタイプ宣言
extern void *NewMenuPrimUtil( int dmapack_phase, int prio, int tri_id, int buffer_size );
extern int MENU_PrimUtil_GetUseBufferSize( void *work_ptr );
extern void MENU_PrimUtil_PutSprite( void *work_ptr, int x, int y, int w, int h, int col, DG_TEX *tex );
extern void MENU_PrimUtil_PutBox( void *work_ptr, int x1, int y1, int x2, int y2, int col );
extern void MENU_PrimUtil_PutLine( void *work_ptr, int x1, int y1, int x2, int y2, int col );
*/

#if 1
//#ifdef KP_XBOX //BP
#undef PSX2
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
#include "menu.h"

/* ---------------------------------------------------------------- */
/* 仮想座標から実座標への変換用 */
#define SCALE_X( _x )	((_x)*DRAW_WIDTH/512)
#define SCALE_Y( _y )	((_y)*DRAW_HEIGHT/384)

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
/* 単一のテクスチャ設定付きスプライト描画プリミティブ */
typedef struct {
	DG_DMATAG		dmatag_tex ;	/* テクスチャ設定パケット転送タグ */
	DG_DMATAG		dmatag_prim ;	/* スプライト本体転送タグ */
	DG_GIFTAG		gif_tag ;		/* スプライトプリミティブ用GIFタグ */
	DG_MENU_SPRT	sprt ;			/* スプライトプリミティブ */
} PRIM_SPRITE ;

typedef struct {
	DG_DMATAG		dmatag_prim ;	/* スプライト本体転送タグ */
	DG_GIFTAG		gif_tag ;		/* スプライトプリミティブ用GIFタグ */
	DG_MENU_LINE	line ;			/* スプライトプリミティブ */
} PRIM_LINE ;


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


typedef struct {
	GV_ACT_EX		actor ;
	PRIM_CONTROL	prim_ctrl ;	/* パネル表示用 */
} Work ;

/*------------------------------------------------------------------*/
static void ConvertPos( IVECTOR *res, int x1, int y1, int x2, int y2 )
{
#ifdef PSX2
	static FVECTOR	scale = { (float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT, 
								(float)DRAW_WIDTH/VR_WIDTH, (float)DRAW_HEIGHT/VR_HEIGHT };
	static FVECTOR	offset = { 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2,
								 2048.0f-DRAW_WIDTH/2, 2048.0f-DRAW_HEIGHT/2 };
	res->vx = x1 ; res->vy = y1 ; res->vz = x2 ; res->vw = y2 ;
	asm volatile("
		lqc2			vf1,0x00(%0)	# 
		lqc2			vf2,0x00(%1)	# 
		lqc2			vf3,0x00(%2)	# 
		vitof0.xyzw		vf1,vf1			# 整数から浮動小数点へ
		vmula.xyzw		ACC,vf2,vf1		# スケール乗算
		vmaddw.xyzw		vf1,vf3,vf0		# オフセット加算
		vftoi4.xyzw		vf1,vf1			# 浮動小数点から整数へ
		sqc2			vf1,0x00(%0)	# 
	"::"r"(res),"r"(&scale),"r"(&offset):"memory" );
	//DG_SET_XY1( sprt, tmp.vx, tmp.vy );
	//DG_SET_XY2( sprt, tmp.vz, tmp.vw );
#else
	res->vx = ( (float)DRAW_WIDTH/VR_WIDTH * x1 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	res->vy = ( (float)DRAW_HEIGHT/VR_HEIGHT * y1 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	res->vz = ( (float)DRAW_WIDTH/VR_WIDTH * x2 + 2048.0f - DRAW_WIDTH / 2 ) * 16.0f ;
	res->vw = ( (float)DRAW_HEIGHT/VR_HEIGHT * y2 + 2048.0f - DRAW_HEIGHT / 2 ) * 16.0f ;
	//DG_SET_XY1( sprt, x1, y1 );
	//DG_SET_XY2( sprt, x2, y2 );
#endif
}
/*------------------------------------------------------------------*/

/* プリミティブコントロールの確保 */
static void MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int dmapack_phase, int prio, int tri_id )
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, dmapack_phase, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	/* テクスチャパケットの取得 */
	if ( tri_id != 0 ){
		prim_ctrl->tri_id = tri_id ;
		prim_ctrl->tex_list = DG_GetTextureList( tri_id );
	}
	/* DMAPACKにバッファを設定 */
	//dmapack->packet[0] = prim_ctrl->buffer[0] ;
	//dmapack->packet[1] = prim_ctrl->buffer[1] ;
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

/* ---------------------------------------------------------------- */
#if 0
/* バッファから指定したサイズのメモリを確保 */
static void *GetPrimBuffer( PRIM_CONTROL *prim_ctrl, int size )
{
	void	*buff_addr ;
	if ( ( prim_ctrl->current_offset + size ) > prim_ctrl->end_offset ) return ( NULL );
	buff_addr = prim_ctrl->current_buffer ;
	prim_ctrl->current_buffer += size ;
	prim_ctrl->current_offset += size ;
	/* 先に終端コードの書き込みを行う */
	ClosePrimControl( prim_ctrl );
	return ( buff_addr );
}
#endif

/* ---------------------------------------------------------------- */
/* プリミティブコントロールの初期化 */
static void InitPrimControl( PRIM_CONTROL *prim_ctrl, int clock )
{
	int		size ;

	if ( prim_ctrl->current_offset >= ( prim_ctrl->end_offset - 1 ) ){
		printf("menu_sub.c: prim buffer over error!!!\n");
	}
	/* 書き込み位置の初期化 */
	prim_ctrl->current_offset = 0 ;
	prim_ctrl->current_buffer = prim_ctrl->buffer[ clock ] ;
	/* 終端タグの書き込み */
	ClosePrimControl( prim_ctrl );
}

/* スプライト描画プリミティブの設定 */
/* 初期設定 */
static void SetSprite( PRIM_SPRITE *sprt )
{
#if 0
	/* DMAタグ＆GIFタグの設定 */
	sprt->dmatag_prim.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DG_MENU_SPRT)+SIZEOF_QWORD(DG_GIFTAG) );
	sprt->dmatag_prim.vifcode[0] = SCE_VIF1_SET_NOP(0);
	sprt->dmatag_prim.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_MENU_SPRT)+SIZEOF_QWORD(DG_GIFTAG) ,0 );
	sprt->gif_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_SPRITE ;
	*(u_short*)&sprt->gif_tag |= 1 ;	/* GIFパケット転送データを１に設定 */
	sprt->sprt.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 );
#endif
}
/* テクスチャ＆ＵＶ設定 */
static void SetSpriteTex( PRIM_SPRITE *sprt, DG_TEX *tex )
{
#if 0
	/* テクスチャ設定 */
	sprt->dmatag_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	sprt->dmatag_tex.addr = &tex->tex_trans ;
	sprt->dmatag_tex.vifcode[0] = SCE_VIF1_SET_NOP(0);
	sprt->dmatag_tex.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_TEX_TRANS) ,0 );
#endif
}
/* 座標設定 */
static void SetSpritePos( DG_MENU_SPRT *sprt, int x1, int y1, int x2, int y2 )
{
#if 0
	IVECTOR	pos ;
	ConvertPos( &pos, x1, y1, x2, y2 );
	DG_SET_XY1( sprt, pos.vx, pos.vy );
	DG_SET_XY2( sprt, pos.vz, pos.vw );
#endif
}
/* ＵＶ設定 */
static void SetSpriteUv( DG_MENU_SPRT *sprt, int u1, int v1, int u2, int v2 )
{
#if 0
	/* UV設定 */
	DG_SET_UV1( sprt, u1, v1 );
	DG_SET_UV2( sprt, u2, v2 );
#endif
}
/* 色設定 */
inline static void SetSpriteCol( DG_MENU_SPRT *sprt, int col )
{
#if 0
	//DG_SET_RGBA1( sprt, r, g, b, a );
	*(int*)&sprt->rgba1.r = col ;
#endif
}

/* ---------------------------------------------------------------- */
/* テクスチャなしスプライト描画プリミティブの設定 */
/* 初期設定 */
static void SetBox( PRIM_LINE *prim )
{
#if 0
	/* DMAタグ＆GIFタグの設定 */
	prim->dmatag_prim.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DG_MENU_LINE)+SIZEOF_QWORD(DG_GIFTAG) );
	prim->dmatag_prim.vifcode[0] = SCE_VIF1_SET_NOP(0);
	prim->dmatag_prim.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_MENU_LINE)+SIZEOF_QWORD(DG_GIFTAG) ,0 );
	prim->gif_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_LINE ;
	*(u_short*)&prim->gif_tag |= 1 ;	/* GIFパケット転送データを１に設定 */
	prim->line.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0 );
#endif
}
/* 初期設定 */
static void SetLine( PRIM_LINE *prim )
{
#if 0
	/* DMAタグ＆GIFタグの設定 */
	prim->dmatag_prim.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DG_MENU_LINE)+SIZEOF_QWORD(DG_GIFTAG) );
	prim->dmatag_prim.vifcode[0] = SCE_VIF1_SET_NOP(0);
	prim->dmatag_prim.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_MENU_LINE)+SIZEOF_QWORD(DG_GIFTAG) ,0 );
	prim->gif_tag = *(DG_GIFTAG*)&DG_GIFTAG_MENU_LINE ;
	*(u_short*)&prim->gif_tag |= 1 ;	/* GIFパケット転送データを１に設定 */
	prim->line.prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_LINE, 0, 0, 0, 1, 0, 1, 0, 0 );
#endif
}
/* 座標設定 */
static void SetLinePos( DG_MENU_LINE *line, int x1, int y1, int x2, int y2 )
{
#if 0
	IVECTOR	pos ;
	ConvertPos( &pos, x1, y1, x2, y2 );
	DG_SET_XY1( line, pos.vx, pos.vy );
	DG_SET_XY2( line, pos.vz, pos.vw );
#endif
}
/* 色設定 */
inline static void SetLineCol( DG_MENU_LINE *line, int col )
{
#if 0
	//DG_SET_RGBA1( sprt, r, g, b, a );
	*(int*)&line->rgba1.r = col ;
#endif
}

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

static void Act( Work *work )
{
	/* 書き込み位置の初期化のみ行う */
	//InitPrimControl( &work->prim_ctrl, DG_Clock );
	InitPrimControl( &work->prim_ctrl, 0 );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	FreePrimControl( &work->prim_ctrl );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int dmapack_phase, int prio, int tri_id, int buffer_size )
{
	/* プリミティブメモリ確保 */
	MakePrimControl( &work->prim_ctrl,
					SIZEOF_QWORD(u_long128) * ( buffer_size + 16 ),
					dmapack_phase, prio, tri_id );
	InitPrimControl( &work->prim_ctrl, 0 );
	InitPrimControl( &work->prim_ctrl, 1 );

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		汎用簡易メニュープリミティブ管理デーモン
	*/
void *NewMenuPrimUtil( int dmapack_phase, int prio, int tri_id, int buffer_size )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, dmapack_phase, prio, tri_id, buffer_size ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/* ---------------------------------------------------------------- */
	/*
		外部インターフェイス
	*/
int MENU_PrimUtil_GetUseBufferSize( void *work_ptr )
{
	Work			*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;

	if ( work_ptr == NULL ) return ( 0 );
	prim_ctrl = &work->prim_ctrl ;

	//return ( work->prim_ctrl.current_offset );
	return ( (int)prim_ctrl->current_buffer - (int)prim_ctrl->buffer[ 0 ] );

}

/**/
void MENU_PrimUtil_PutSprite( void *work_ptr, int x, int y, int w, int h, int col, DG_TEX *tex )
{
	Work			*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	int				tx, ty, tw, th ;
	float			x0, y0, x1, y1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work->prim_ctrl ;

	/* 背景ウィンドウスプライトの設定 */
	x0 = SCALE_X( x );
	y0 = SCALE_Y( y );
	x1 = SCALE_X( x + w );
	y1 = SCALE_Y( y + h );
	prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
	prim_ctrl->current_buffer = DG_SetDmapackSprtTex( prim_ctrl->current_buffer,
													 x0, y0, 0.0f, 0.0f,
													 x1, y1, 1.0f, 1.0f,
													 DG_MakeDmaPackColorFromInt(col),
													 tex );

	ClosePrimControl( prim_ctrl );

}

void MENU_PrimUtil_PutBox( void *work_ptr, int x1, int y1, int x2, int y2, int col )
{
	Work			*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	float			fx0, fy0, fx1, fy1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work->prim_ctrl ;

	fx0 = SCALE_X( x1 );
	fy0 = SCALE_Y( y1 );
	fx1 = SCALE_X( x2 );
	fy1 = SCALE_Y( y2 );
	if ( ( col & 0xff000000 ) == 0x80000000 ){
		prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, 0 );
	} else {
		prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
	}
	prim_ctrl->current_buffer = DG_SetDmapackBox( prim_ctrl->current_buffer,
												 fx0, fy0, fx1, fy1, DG_MakeDmaPackColorFromInt(col) );

	ClosePrimControl( prim_ctrl );

}

void MENU_PrimUtil_PutLine( void *work_ptr, int x1, int y1, int x2, int y2, int col )
{
	Work			*work = work_ptr ;
	PRIM_CONTROL	*prim_ctrl ;
	float			fx0, fy0, fx1, fy1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work->prim_ctrl ;

	fx0 = SCALE_X( x1 );
	fy0 = SCALE_Y( y1 );
	fx1 = SCALE_X( x2 );
	fy1 = SCALE_Y( y2 );
	prim_ctrl->current_buffer = DG_SetDmapackLine( prim_ctrl->current_buffer,
												  fx0, fy0, DG_MakeDmaPackColorFromInt(col), fx1, fy1, DG_MakeDmaPackColorFromInt(col) );

	ClosePrimControl( prim_ctrl );

}

#endif