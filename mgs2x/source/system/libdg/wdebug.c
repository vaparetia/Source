/*
	xdebug.c
	デバッグ関連ルーチン

	2002/02/04 K.Takabe
	$Id: wdebug.c,v 1.1 2002/08/09 00:45:08 takaki Exp $

*/
/*

	----------------------------------------------------------------

	----------------------------------------------------------------
	----------------------------------------------------------------

*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.cnf"
#include "libdg.h"
#include "def_dma.h"

#include "d3d8.h"

static void		*TopPushBuffer ;
static void		*EndPushBuffer ;

/* ---------------------------------------------------------------- */
#ifndef _WINDOWS
void *DG_GetPushBufferAddress( void )
{
	void	*ptr ;
	IDirect3DDevice8_BeginPush( g_pd3dDevice, 0, &ptr );
	IDirect3DDevice8_EndPush( g_pd3dDevice, ptr );
	return ( ptr );
}

/* プッシュバッファの解析開始 */
void DG_StartPushBufferAnalyze( void )
{
	IDirect3DDevice8_BeginPush( g_pd3dDevice, 0, &TopPushBuffer );
	IDirect3DDevice8_EndPush( g_pd3dDevice, TopPushBuffer );
}

/* プッシュバッファのダンプ */
void DG_EndPushBufferAnalyze( void )
{
	int			i, size, offset ;
	unsigned int	*data ;

	IDirect3DDevice8_BeginPush( g_pd3dDevice, 0, &EndPushBuffer );
	IDirect3DDevice8_EndPush( g_pd3dDevice, EndPushBuffer );

	offset = 8 ;
	size = (int)EndPushBuffer - (int)TopPushBuffer ;
	size /= 4 ;
	data = (int*)TopPushBuffer ;
	printf("from %p, to %p\n", TopPushBuffer, EndPushBuffer );
	for ( ; size > 0 ; size-=offset ){
		if ( size >= offset ){
			printf("%08x %08x %08x %08x ", data[0], data[1], data[2], data[3] );
			printf("%08x %08x %08x %08x\n", data[4], data[5], data[6], data[7] );
		} else {
			for ( i = 0 ; i < size ; i++ ) printf("%08x ", data[ i ] );
			printf("\n");
		}
		data += offset ;
	}
}
#endif	// _WINDOWS

/* ---------------------------------------------------------------- */

/*----------------------------------------------------------------*/

	/*
		テクスチャ入れ替えＤＭＡパケット書き込み
	*/
int DG_WriteTextureChangePacks( void *tag_addr, void *tex_packet )
{
	/* オブジェクトパケットのＤＭＡデータを接続する */
	DG_DMATAG		*tag ;
	static u_long tex_flush_packet[6] ALIGN16 = { 
		SCE_GIF_SET_TAG(2, 1, 0, 0, 0, 1),
		GS_REGS_AD,
		0,
		SCE_GS_TEXFLUSH,/*0x3f*/
		SCE_GS_SET_TEX2( SCE_GS_PSMT8, 0, 0, 0, 0, 0x2 ),
		SCE_GS_TEX2_1
	};

	tag = (DG_DMATAG*)tag_addr ;
	/*  */
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
	tag->addr = tex_packet ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	tag++ ;
	tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 3 );
	tag->addr = tex_flush_packet ;
	tag->vifcode[0] = SCE_VIF1_SET_FLUSH(0) ;
	tag->vifcode[1] = SCE_VIF1_SET_DIRECT(3,0) ;
	tag++ ;

	return ( 2 );

}

	/*
		テクスチャ入れ替えＤＭＡパケット書き込み
		（テクスチャ部分差し替え対応版）
	*/
int DG_WriteTextureChangePacks2( void *tag_addr, DG_TEXTURE_LIST *tex_list, int which )
{
	/* オブジェクトパケットのＤＭＡデータを接続する */
	static u_long tex_flush_packet[6] ALIGN16 = { 
		SCE_GIF_SET_TAG(2, 1, 0, 0, 0, 1),
		GS_REGS_AD,
		0,
		SCE_GS_TEXFLUSH,/*0x3f*/
		SCE_GS_SET_TEX2( SCE_GS_PSMT8, 0, 0, 0, 0, 0x2 ),
		SCE_GS_TEX2_1
	};
	DG_DMATAG		*dmatag ;
	int				size = 2 ;
	void			*tex_packet ;

	dmatag = (DG_DMATAG*)tag_addr ;
	tex_packet = &tex_list->tex_packet[ which ] ;

	/* メインテクスチャ転送 */
    dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
	dmatag->addr = tex_packet ;
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	dmatag++ ;

	/* 差し替えパケットのチェック */
	if ( tex_list->move_link != NULL ){
		DG_TEX_MOVEREPLACE	*replace ;
		replace = tex_list->move_link ;
		while ( replace != NULL ){
			dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
			dmatag->addr = replace ;
			dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag++ ;
			size++ ;
			replace = replace->next ;
		}
	}
	if ( tex_list->load_link != NULL ){
		DG_TEX_LOADREPLACE	*replace ;
		replace = tex_list->load_link ;
		while ( replace != NULL ){
			dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
			dmatag->addr = replace ;
			dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
			dmatag++ ;
			size++ ;
			replace = replace->next ;
		}
	}

	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 3 );
	dmatag->addr = tex_flush_packet;
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSH(0) ;
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT(3,0) ;
	dmatag++ ;

	return ( size );

}

