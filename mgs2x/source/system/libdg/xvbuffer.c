//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xvbuffer.c
	動的頂点バッファ管理ルーチン

	2002/02/27 K.Takabe
	$Id: xvbuffer.c,v 1.4 2002/11/23 11:36:56 Yoshizawa1 Exp $

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

#include "libgv.h"
#include "libdg.h"

//#include	"gameheader.h"

#include "dmapack.h"
#include	"def_dma.h"

/* ---------------------------------------------------------------- */
int			DG_MaxVertexBufferSize ;	/* バッファ割り当てサイズ（xdgmem.cで初期化） */
void		*DG_VertexBuffer[2] ;		/* バッファ先頭アドレス（xdgmem.cで初期化） */
void		*DG_VertexBufferEnd[2] ;	/* バッファ終了アドレス（xdgmem.cで初期化） */
int			DG_LastUseVertexBufferSize ;	/* １フレーム前の最大使用量 */

static int		DG_UseVertexBufferSize ;
static void		*DG_TopVertexBuffer ;
static void		*DG_CurrentVertexBuffer ;
#define USE_VB_RINGBUFF	(8)
static IDirect3DVertexBuffer9	DG_D3DVBuffer8[USE_VB_RINGBUFF][2] ;
static int		DG_VBufferClock[USE_VB_RINGBUFF] ;

DG_VERTEXSTREAM	DG_VertexStream ;

/* ---------------------------------------------------------------- */
/* 動的頂点バッファの初期化 */
void DG_InitDynamicVertexBufferSystem( void )
{
	int		i ;

	for ( i = 0 ; i < USE_VB_RINGBUFF ; i++ ) DG_VBufferClock[i] = 0 ;

}

void DG_SetVertexBuffer( int stream, void *addr, int size )
{
#if 0 //BP_RENDER
	LPDIRECT3DVERTEXBUFFER8	lpVBuffer8 ;

	if ( addr == NULL ){
		DG_SetStreamSource( stream, NULL, 0 );
		return ;
	}

	/* 頂点バッファインターフェイスの初期化 */
#if 0
	lpVBuffer8 = ( DG_VBufferClock ) ? &DG_D3DVBuffer8[ 0 ] : &DG_D3DVBuffer8[ 1 ] ;
	DG_VBufferClock ^= 1 ;
#else
	lpVBuffer8 = &DG_D3DVBuffer8[ stream ][ DG_VBufferClock[ stream ] ] ;
	DG_VBufferClock[ stream ] = ( DG_VBufferClock[ stream ] + 1 ) & 1 ;
#endif

#ifndef KP_WINDOWS
	lpVBuffer8->Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER ;
	lpVBuffer8->Data = (int)addr & 0x0fffffff ;
	lpVBuffer8->Lock = 0 ;
#endif

	/* ストリームとして登録する */
	//DG_SetStreamSource( stream, lpVBuffer8, size );

#ifndef KP_WINDOWS
	DG_VertexStream.input[ stream ].VertexBuffer = lpVBuffer8 ;
	DG_VertexStream.input[ stream ].Stride = size ;
#endif
	DG_VertexStream.max_stream = stream + 1 ;
	/* 指定したストリーム番号が最大入力数になるので */
	/* 複数のストリームをセットする場合はストリーム番号の小さい順に行うこと！ */
#endif
}

/* 指定したサイズの動的（使い捨て）頂点バッファを確保＆ストリーム０に設定 */
void *DG_NewDynamicVertexBuffer( int size, int num )
{
	void	*addr ;
#if 0 //BP_RENDER
	addr = DG_AllocDynamicVertexBuffer( size, num );
	if ( addr == NULL ) return ( NULL );
	DG_SetVertexBuffer( 0, addr, size );
#else
   addr = 0;
#endif
	return ( addr );
}

#endif
