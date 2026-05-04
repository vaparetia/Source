//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xpic_scn.c
	画像イメージ表示補助ルーチン

	2002/03/25	K.Takabe
	$Id: xpic_scn.c,v 1.1.1.3 2002/11/19 11:45:15 Yoshizawa1 Exp $
*/
/*

	void *NewPictureDrawManager( int prio );
	int		prio ;		描画プライオリティ指定

	このマネージャーを起動した上で下の関数を呼び出すとイメージを描画することが出来る。


	void MENU_DrawPicture16( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );
	int		x ;			表示Ｘ座標（仮想座標）
	int		y ;			表示Ｙ座標（仮想座標）
	int		w ;			表示幅（仮想座標）
	int		h ;			表示高さ（仮想座標）
	int		color ;		スプライトカラー
	void	*image ;	１６ビットイメージデータへのポインタ（qword境界から始まること）
	int		image_w ;	イメージ幅
	int		image_h ;	イメージ高さ
	int		flag ;		フラグ（現在未使用）

		指定したベタイメージを画面に表示する（１フレーム中で８枚まで）
		イメージのサイズは最大２５６ＫＢ以下であること
		また、実際に画面に表示されるのは実際のイメージの0.5ドット内側なので注意すること


	void MENU_DrawPicture32( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );

		３２ビットカラー版


	void MENU_DrawPictureTex( int x, int y, int w, int h, int color, DG_TEX_LIN *tex, int flag );
	int		x ;			表示Ｘ座標（仮想座標）
	int		y ;			表示Ｙ座標（仮想座標）
	int		w ;			表示幅（仮想座標）
	int		h ;			表示高さ（仮想座標）
	int		color ;		スプライトカラー
	DG_TEX_LIN	*tex ;	線形テクスチャ
	int		flag ;		フラグ（現在未使用）

		指定した線形テクスチャを画面に表示する（１フレーム中で８枚まで）
		また、実際に画面に表示されるのは実際のイメージの0.5ドット内側なので注意すること

*/

#if 1 //#ifdef KP_XBOX //BP

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
#include	"menu.h"

#include "BP_Renderer.h"

/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */


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
	GV_ACT_EX	actor ;
	PRIM_CONTROL	prim_ctrl ;	/* パネル表示用 */
} Work ;

static Work	*work_ptr = NULL ;

/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

/* プリミティブコントロールの確保 */
static void MakePrimControl( PRIM_CONTROL *prim_ctrl, int buffer_size, int prio, int tri_id )
{
	DG_DMAPACK	*dmapack ;
	u_long128	*buffer ;

	/* プリミティブを確保（DG_DMAPACKを使用） */
	dmapack = prim_ctrl->dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, prio );
	DG_QueueDmapack( dmapack );
	/* プリミティブバッファの確保 */
	buffer = GV_Malloc( sizeof(u_long128) * buffer_size * 2 );
	prim_ctrl->buffer[0] = &buffer[0] ;
	prim_ctrl->buffer[1] = &buffer[buffer_size] ;
	///* テクスチャパケットの取得 */
	//prim_ctrl->tri_id = tri_id ;
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


/* ---------------------------------------------------------------- */
/*  */
void MENU_DrawPicture16( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag )
{
	PRIM_CONTROL		*prim_ctrl ;
	void				*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->prim_ctrl ;

	//prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, work->tex );
	//prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = ( x ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = ( y ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = ( x + w ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = ( y + h ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	//fu0 = ( ( (float)0 + 0.5f ) / (float)work->tex->width ) ;
	//fv0 = ( ( (float)0 + 0.5f ) / (float)work->tex->height ) ;
	//fu1 = ( ( (float)image_w - 0.5f ) / (float)work->tex->width ) ;
	//fv1 = ( ( (float)image_h - 0.5f ) / (float)work->tex->height ) ;

	//prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
	//											  fx0, fy0, fu0, fv0,
	//											  fx1, fy1, fu1, fv1, color );

	ClosePrimControl( prim_ctrl );

}

/*  */
void MENU_DrawPicture32( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag )
{
	PRIM_CONTROL		*prim_ctrl ;
	void				*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->prim_ctrl ;

	//prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, work->tex );
	//prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = ( x ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = ( y ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = ( x + w ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = ( y + h ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	//fu0 = ( ( (float)0 + 0.5f ) / (float)work->tex->width ) ;
	//fv0 = ( ( (float)0 + 0.5f ) / (float)work->tex->height ) ;
	//fu1 = ( ( (float)image_w - 0.5f ) / (float)work->tex->width ) ;
	//fv1 = ( ( (float)image_h - 0.5f ) / (float)work->tex->height ) ;

	//prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
	//											  fx0, fy0, fu0, fv0,
	//											  fx1, fy1, fu1, fv1, color );

	ClosePrimControl( prim_ctrl );

}

void MENU_DrawPictureTex( int x, int y, int w, int h, int color, DG_TEX_LIN *tex, int flag )
{
	PRIM_CONTROL		*prim_ctrl ;
	void				*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->prim_ctrl ;

	prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, tex );
	prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = ( x ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = ( y ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = ( x + w ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = ( y + h ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fu0 = ( ( (float)0 + 0.5f ) / (float)tex->width ) ;
	fv0 = ( ( (float)0 + 0.5f ) / (float)tex->height ) ;
	fu1 = ( ( (float)tex->width - 0.5f ) / (float)tex->width ) ;
	fv1 = ( ( (float)tex->height - 0.5f ) / (float)tex->height ) ;

	prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
												  fx0, fy0, fu0, fv0,
												  fx1, fy1, fu1, fv1, DG_MakeDmaPackColorFromInt(color) );

	ClosePrimControl( prim_ctrl );

}

void MENU_DrawPictureTexEX( int x, int y, int w, int h, int color,
						   DG_TEX_LIN *tex, int u0, int v0, int u1, int v1, int flag )
{
	PRIM_CONTROL		*prim_ctrl ;
	void				*prim ;
	float			fx0, fy0, fx1, fy1 ;
	float			fu0, fv0, fu1, fv1 ;

	if ( work_ptr == NULL ) return ;
	/* プリミティブを１枚分確保 */
	prim_ctrl = &work_ptr->prim_ctrl ;

	prim_ctrl->current_buffer = DG_SetDmapackTexLin( prim_ctrl->current_buffer, tex );
	prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	fx0 = ( x ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy0 = ( y ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fx1 = ( x + w ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
	fy1 = ( y + h ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
	fu0 = ( ( (float)u0 + 0.5f ) / (float)tex->width ) ;
	fv0 = ( ( (float)v0 + 0.5f ) / (float)tex->height ) ;
	fu1 = ( ( (float)u1 - 0.5f ) / (float)tex->width ) ;
	fv1 = ( ( (float)v1 - 0.5f ) / (float)tex->height ) ;

	prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
												  fx0, fy0, fu0, fv0,
												  fx1, fy1, fu1, fv1, DG_MakeDmaPackColorFromInt(color) );

	ClosePrimControl( prim_ctrl );

}

void BP_MENU_DrawPictureTexEX( int x, int y, int w, int h, int color,
                           int photoNum, int u0, int v0, int u1, int v1, int flag )
{
   PRIM_CONTROL * prim_ctrl ;
   void * prim ;
   float fx0, fy0, fx1, fy1 ;
   float fu0, fv0, fu1, fv1 ;

   if ( work_ptr == NULL ) return ;
   /* プリミティブを１枚分確保 */
   prim_ctrl = &work_ptr->prim_ctrl ;

   // prim_ctrl->current_buffer = DG_SetDmapackTex( prim_ctrl->current_buffer, tex );
   prim_ctrl->current_buffer = DG_SetDmapackTextureDynamic01( prim_ctrl->current_buffer, NULL, BP_GetRenderTarget_c(kRT_TankerSS0+photoNum) );
   prim_ctrl->current_buffer = DG_SetDmapackAlpha( prim_ctrl->current_buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

   fx0 = ( x ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
   fy0 = ( y ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
   fx1 = ( x + w ) * ( (float)DRAW_WIDTH / VR_WIDTH ) ;
   fy1 = ( y + h ) * ( (float)DRAW_HEIGHT / VR_HEIGHT ) ;
   fu0 = ( ( (float)u0 + 0.5f ) / (float)1280 ) ;
   fv0 = ( ( (float)v0 + 0.5f ) / (float)720 ) ;
   fu1 = ( ( (float)u1 - 0.5f ) / (float)1280 ) ;
   fv1 = ( ( (float)v1 - 0.5f ) / (float)720 ) ;

   prim_ctrl->current_buffer = DG_SetDmapackSprt( prim_ctrl->current_buffer,
      fx0, fy0, fu0, fv0,
      fx1, fy1, fu1, fv1, DG_MakeDmaPackColorFromInt(color) );

   ClosePrimControl( prim_ctrl );
}

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

static void Act( Work *work )
{
	/* 書き込み位置の初期化のみ行う */
	InitPrimControl( &work->prim_ctrl );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	FreePrimControl( &work->prim_ctrl );
	work_ptr = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int prio )
{
	/* プリミティブメモリ確保 */
	MakePrimControl( &work->prim_ctrl,
					48*16 + 32,
					prio, 0 );
	Act( work );


	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		アイテムメニュー用プリミティブ管理デーモン
	*/
void *NewPictureDrawManager( int prio )
{
	Work			*work ;

	if ( work_ptr != NULL ) return (NULL);

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, prio ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	work_ptr = work ;
	return work ;
}


#if 0
/* 動作テスト用サンプルルーチン */
	{
		extern void MENU_DrawPicture16( int x, int y, int w, int h, int color
									 , void *image, int image_w, int image_h, int flag );

		static u_long128		dummy[ 32 * 32 * 2 / 16 ];
		int		i ;
		u_short	*data ;
		data = (u_short*)dummy ;
		for ( i = 0 ; i < 32*32 ; i++, data++ ){
			*data = 0xfc00 | i ;
		}
		MENU_DrawPicture16( 256, 256, 96, 96, 0x80808080, dummy, 32, 32, 0 );
	}
#endif

#endif
