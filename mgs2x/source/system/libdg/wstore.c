/*
	store.c
	イメージストア処理ユニット

	2000/02/01 K.Takabe
	$Id: wstore.c,v 1.6 2002/11/04 04:33:55 takaki Exp $

*/
/*

	void		DG_StoreChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	イメージをストアするＤＭＡパケットを接続する


	void DG_SetFrameStore( void *store_addr, int mode )
	void	*store_addr ;	データ書き出しアドレス
	int		mode ;			モード

	前のフレームの画像を指定したアドレスに１６ビットイメージとして書き出す

*/


#ifdef _XBOX
#include <xtl.h>
#include <xgraphics.h>
#else
#include <d3dx8.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

/*----------------------------------------------------------------*/
static void		*ImageStoreAddr = NULL ;
static int		ImageStoreMode = 0 ;
static int		RetryStoreFlag = 0 ;
static void		*RetryStoreAddr = NULL ;
static DG_USERTEX	*UserTexStoreAddr = NULL ;
static int			UserTexStoreMode  = 0 ;
static void			*UserTexRetryStoreAddr = NULL ;

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
static int MakeStoreImagePacket( int which )
{
#ifndef _WINDOWS
	/* データ転送パケット生成 */
	if ( ImageStoreMode == 0 ){	/* ＸＢＯＸでは区別がない */
	} else {
	}
	XGSetSurfaceHeader(
					   DISPLAY_WIDTH,
					   DISPLAY_HEIGHT,
					   D3DFMT_LIN_A8R8G8B8,
					   &StoreSurface,
					   0, 
					   DRAW_WIDTH * 4 );
	IDirect3DResource8_Register( &StoreSurface, ImageStoreAddr );
	{/* ＶＲＡＭの内容待避パケット生成 */
		DG_DMAPACK_PARAM	packet_top[2] ;
		void	*packet ;
		packet = packet_top ;
		packet = DG_SetDmapackBackupFrame( packet, 1|((int)&StoreSurface) );
		packet = DG_SetDmapackEnd( packet );
		DG_ExecAuto2DPrim( packet_top );
	}
#else
	DG_StoreImage_EB(ImageStoreAddr, 1, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0) ;
#endif
	return ( 0 );
}

static int MakeStoreUserTexPacket( int which )
{
	DG_BACKUPFRAME2SURFACE_PARAM	param ;

	param.src_page    = 1 ;
	param.dst_surface = UserTexStoreAddr->surface ;	// 転送先

	param.dst_x = 0.0f ;	// 転送先領域(0～1で正規化)
	param.dst_y = 0.0f ;
	param.dst_w = UserTexStoreAddr->texcoord_scale_w ;
	param.dst_h = UserTexStoreAddr->texcoord_scale_h ;

	DG_BackupFrame2Surface(&param) ;

	return ( 0 );
}

/*----------------------------------------------------------------*/

	/*
		イメージストア用ＤＭＡパケットを接続する
	*/
void DG_StoreChanl( DG_CHANL *cp, int which )
{
#ifndef _WINDOWS
	int			size ;

	if ( RetryStoreFlag && RetryStoreAddr != NULL ){
		ImageStoreAddr = RetryStoreAddr ;
	}
	RetryStoreAddr = NULL ;
	RetryStoreFlag = 0 ;
	if ( ImageStoreAddr == NULL ) return ;

	/* ＤＭＡバッファオープン(VIF1) */
	{/* パケット生成 */
		MakeStoreImagePacket( which );
	}
	/* ＤＭＡバッファクローズ */

	RetryStoreAddr = ImageStoreAddr ;
	ImageStoreAddr = NULL ;
#else
	if( RetryStoreFlag )
	{
		ImageStoreAddr   = RetryStoreAddr ;
		UserTexStoreAddr = UserTexRetryStoreAddr ;
	}
	RetryStoreAddr        = NULL ;
	UserTexRetryStoreAddr = NULL ;
	RetryStoreFlag = 0 ;


	/*-- Image処理 -----------------------------------------------*/

	if( ImageStoreAddr )
	{
		MakeStoreImagePacket( which );

		RetryStoreAddr = ImageStoreAddr ;
		ImageStoreAddr = NULL ;
	}
	/*------------------------------------------------------------*/

	/*-- UserTex処理 ---------------------------------------------*/

	if( UserTexStoreAddr )
	{
		MakeStoreUserTexPacket( which );

		UserTexRetryStoreAddr = UserTexStoreAddr ;
		UserTexStoreAddr      = NULL ;
	}
	/*------------------------------------------------------------*/
#endif
}
/*----------------------------------------------------------------*/
	/*
		イメージストアをリトライする
	*/
void DG_RetryStoreChanl( void )
{
	RetryStoreFlag = 1 ;
}
/*----------------------------------------------------------------*/
	/*
		前のフレームの画像を指定したアドレスに１６ビットイメージとして書き出す
	*/
void DG_SetFrameStore( void *store_addr, int mode )
{
printf("aaaaaaaa %p %d\n", store_addr, mode );
	ImageStoreAddr = store_addr ;
	ImageStoreMode = mode ;

	UserTexRetryStoreAddr = NULL ;	// UserTexのRetryを防ぐ
}

void DG_SetFrameStore2UserTex( DG_USERTEX *usertex, int mode )
{
	UserTexStoreAddr = usertex ;
	UserTexStoreMode = mode ;

	RetryStoreAddr = NULL ;	// ImageのRetryを防ぐ
}
