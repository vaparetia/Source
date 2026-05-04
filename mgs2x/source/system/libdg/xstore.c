//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	store.c
	イメージストア処理ユニット

	2000/02/01 K.Takabe
	$Id: xstore.c,v 1.1.1.3 2002/11/19 11:42:38 Yoshizawa1 Exp $

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


#ifdef KP_XBOX //BP

#ifdef KP_XBOX
#include <xtl.h>
//BP_RENDER #include <xgraphics.h>
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
#if 0 //BP_RENDER
static D3DSurface	StoreSurface ;
#endif
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
static int MakeStoreImagePacket( int which )
{
#if 0 //BP_RENDER
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
#endif

	return ( 0 );
}

/*----------------------------------------------------------------*/

	/*
		イメージストア用ＤＭＡパケットを接続する
	*/
void DG_StoreChanl( DG_CHANL *cp, int which )
{
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
}

#endif
