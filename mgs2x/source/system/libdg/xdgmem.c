//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xdgmem.c
	ＤＧ用メモリ管理

	2002/05/09	K.Takabe
	$Id: xdgmem.c,v 1.4 2002/11/23 11:36:55 Yoshizawa1 Exp $
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


/* ---------------------------------------------------------------------- */
#define USE_MEMORY_ID	(GV_NORMAL_MEMORY)

void		*DG_PacketMemory ;			/* 共通パケットメモリ */
void		*DG_PacketMemoryTop[2] ;		/* カレントパケットメモリ先頭アドレス */
void		*DG_PacketMemoryEnd[2] ;		/* カレントパケットメモリ最終アドレス */
int			DG_PacketMemoryFree ;			/* カレントパケットメモリの空き領域 */
int			DG_PacketMemorySize ;			/* パケットメモリの最大サイズ */
void		*DG_PacketFreeTop ;				/* フリー領域先頭アドレス */
void		*DG_PacketFreeEnd ;				/* フリー領域最終アドレス */
int			DG_UsePacketTop ;				/* 先頭の使用サイズ */
int			DG_UsePacketBottom ;			/* 後ろの使用サイズ */
int			DG_OldUsePacketTop ;			/* 前フレームでの先頭の使用サイズ */
int			DG_OldUsePacketBottom ;			/* 前フレームでの後ろの使用サイズ */
int			DG_PacketOverFlag ;				/* そのフレーム内でのバッファーオーバー通知用 */


void DG_ResetMemorySystem( int which );


/* ＤＧ用のメモリを初期化する */
void DG_InitMemorySystem( int static_size, int dynamic_size, int push_buff_size )
{
	int		total_size ;
	void	*mem ;

	total_size = dynamic_size * 2 + push_buff_size * 2 ;
	DG_PacketMemorySize = dynamic_size + push_buff_size ;

	/* メモリの確保 */
#if 0 //BP_RENDER
	mem = XPhysicalAlloc( total_size, MAXULONG_PTR, 4096, PAGE_READWRITE|PAGE_WRITECOMBINE );
#else
   mem = malloc(total_size);
#endif
	//mem = XPhysicalAlloc( total_size, MAXULONG_PTR, 4096, PAGE_READWRITE );
	printf("libdg memory init addr:%p size:%08x(%08x,%08x,%08x)\n",
		   mem, total_size, static_size, dynamic_size, push_buff_size );
	ASSERT( mem != NULL );
	DG_PacketMemory = mem ;
	GV_ZeroMemory( mem, total_size );

	/* スタティックローカルメモリ領域の作成 */
	DG_PacketMemoryTop[0] = (void*)( (char*)DG_PacketMemory );
	DG_PacketMemoryEnd[0] = (void*)( (char*)DG_PacketMemoryTop[0] + DG_PacketMemorySize );
	DG_PacketMemoryTop[1] = (void*)( (char*)DG_PacketMemoryEnd[0] );
	DG_PacketMemoryEnd[1] = (void*)( (char*)DG_PacketMemoryTop[1] + DG_PacketMemorySize );
	/* その他変数の初期化 */
	DG_PacketFreeTop = DG_PacketMemoryTop[ 0 ];
	DG_PacketFreeEnd = DG_PacketMemoryEnd[ 0 ];
	DG_UsePacketTop = 0 ;
	DG_UsePacketBottom = 0 ;
	DG_OldUsePacketTop = 0 ;
	DG_OldUsePacketBottom = 0 ;
	DG_PacketMemoryFree = DG_PacketMemorySize ;
	DG_PacketOverFlag = 0 ;

	//DG_ResetLocalVideoMemorySystem();
	DG_InitDynamicVertexBufferSystem();

	DG_ResetMemorySystem( 0 );
}

/* フレームの先頭でＤＧ用メモリをリセットする */
void DG_ResetMemorySystem( which )
{
#if 0
printf("packet mem free:%d/%d\n", DG_PacketMemoryFree, (int)DG_PacketMemoryEnd[ 0 ] - (int)DG_PacketMemoryTop[ 0 ] );
#endif
	DG_PacketFreeTop = DG_PacketMemoryTop[ which ];
	DG_PacketFreeEnd = DG_PacketMemoryEnd[ which ];
	DG_PacketMemoryFree = (int)DG_PacketFreeEnd - (int)DG_PacketFreeTop ;
	DG_OldUsePacketTop = DG_UsePacketTop ;
	DG_OldUsePacketBottom = DG_UsePacketBottom ;
	DG_UsePacketTop = 0 ;
	DG_UsePacketBottom = 0 ;
	DG_PacketOverFlag = 0 ;
}

/* ---------------------------------------------------------------- */
/* 指定したサイズの動的（使い捨て用）頂点バッファメモリを確保 */
void *DG_AllocDynamicVertexBuffer( int size, int num )
{
	void	*addr ;
	int		total_size ;

	total_size = size * num ;

	total_size = ( total_size + 0x1f ) & ~0x1f ;
	if ( total_size > DG_PacketMemoryFree ){
#ifdef DEBUG_MODE
		printf("few packet memory of dynamic packet memory!\n");
		ASSERT( 0 );
#endif
		return ( NULL );
	}
	/* 頂点バッファアドレスの取得 */
	addr = (void*)( (char*)DG_PacketFreeEnd - total_size );
	DG_PacketMemoryFree -= total_size ;
	DG_UsePacketBottom += total_size ;
	DG_PacketFreeEnd = addr ;

	return ( addr );
}

/* ---------------------------------------------------------------- */
/* スタティックプッシュバッファにメモリを割り当てる（この間動的頂点バッファメモリは使用不可能なので注意） */
static D3DPushBuffer *CurrentPushBuffer ;
void *DG_OpenPushBufferMemory( D3DPushBuffer *pushbuffer )
{
#if 0 //BP_RENDER
	XGSetPushBufferHeader( DG_PacketMemoryFree, FALSE, pushbuffer, 0 );
	IDirect3DResource8_Register( pushbuffer, DG_PacketFreeTop );
   CurrentPushBuffer = pushbuffer ;
#endif

	return ( DG_PacketFreeTop );
}
/* スタティックプッシュバッファのメモリを確定し、使用メモリ量を確定させる */
int DG_ClosePushBufferMemory( D3DPushBuffer *pushbuffer )
{
	int		size ;
#if 0 //BP_RENDER
	IDirect3DPushBuffer8_GetSize( pushbuffer, &size );
	size = ( size + 1023 ) & ~1023 ;			/* 適度にアラインを行っておく（そうしないと落ちる！） */
#ifndef KP_WINDOWS
	pushbuffer->AllocationSize = size ;			/* これは必要ないかも */
#endif
	DG_PacketMemoryFree -= size ;
	DG_UsePacketTop += size ;
	DG_PacketFreeTop = (void*)( (char*)DG_PacketFreeTop + size );
	CurrentPushBuffer = NULL ;
#else
   size = 0;
#endif
	return ( size );
}
/* 現在のプッシュバッファメモリの空きを調べる */
int DG_GetPushBufferFreeSpace( void )
{
#if 0 //BP_RENDER
	int		size, alloc_size ;

	//IDirect3DPushBuffer8_GetSize( CurrentPushBuffer, &size );
	IDirect3DDevice8_GetPushBufferOffset( g_pd3dDevice, &size );
	//alloc_size = CurrentPushBuffer->AllocationSize ;
	alloc_size = DG_PacketMemoryFree ;	/* プッシュバッファの作成と同時に動的頂点を作成する可能性があるため */
	return ( alloc_size - size );
#else
   return 0;
#endif
}

/* ---------------------------------------------------------------- */
/* 静的頂点及びテクスチャデータ用メモリの確保 */
void *DG_AllocLocalVideoMemory( int size )
{
	return ( GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size, 16 ) );
}
/* 静的頂点及びテクスチャデータ用メモリの確保（アライメント指定付き） */
void *DG_AllocLocalVideoMemoryAlign( int size, int align )
{
	return ( GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size, align ) );
}
/* 静的頂点及びテクスチャデータ用メモリの開放 */
void DG_FreeLocalVideoMemory( void *addr )
{
	GV_FreeMemory( GV_NORMAL_MEMORY, addr );
}
/* 静的頂点及びテクスチャデータ用メモリの開放（１フレーム遅れ開放） */
void DG_DelayedFreeLocalVideoMemory( void *addr )
{
	GV_FreeMemory2( GV_NORMAL_MEMORY, addr );
}


/* ---------------------------------------------------------------------- */
/*
	PS2互換用
*/

#include "libgv.cnf"

void *DG_ResizePacketMemory( int size )
{
	// DMA描画終了待ちを行う。

	int packet_size;

	DG_DrawSync();

#if 0 //BP_RENDER
	IDirect3DDevice8_BlockUntilIdle( g_pd3dDevice );
#endif
	if( size == 0 ){
		/* 元に戻す */
		packet_size = DG_PacketMemorySize ;
	} else {
		packet_size = DG_PacketMemorySize - size / 2 ;
	}

	DG_PacketMemoryTop[0] = (void*)( (char*)DG_PacketMemory );
	DG_PacketMemoryEnd[0] = (void*)( (char*)DG_PacketMemoryTop[0] + packet_size );
	DG_PacketMemoryTop[1] = (void*)( (char*)DG_PacketMemoryEnd[0] );
	DG_PacketMemoryEnd[1] = (void*)( (char*)DG_PacketMemoryTop[1] + packet_size );

	DG_DmaClear( DG_Clock );
	DG_ResetMemorySystem( DG_Clock );

#if 0 //BP_RENDER
	__asm {/* キャッシュの強制クリア */
		WBINVD
	}

	if( size == 0 ){
		XPhysicalProtect( DG_PacketMemory, DG_PacketMemorySize * 2
						 , PAGE_READWRITE | PAGE_WRITECOMBINE );
		return NULL;
	} else {
		/* その領域をキャッシュONにする */
		XPhysicalProtect( DG_PacketMemoryEnd[1], size, PAGE_READWRITE );
		return ( void * )DG_PacketMemoryEnd[1];
	}
#else
   return NULL;
#endif
}


/* デバッグ用 */
void DG_EnablePacketMemoryCached( void )
{
#if 0 //BP_RENDER
   __asm {/* キャッシュの強制クリア */
		WBINVD
	}
	XPhysicalProtect( DG_PacketMemoryTop[DG_Clock], DG_PacketMemorySize, PAGE_READWRITE );
#endif
}
void DG_DisablePacketMemoryCached( void )
{
#if 0 //BP_RENDER
	__asm {/* キャッシュの強制クリア */
		WBINVD
	}
	XPhysicalProtect( DG_PacketMemoryTop[DG_Clock], DG_PacketMemorySize, PAGE_READWRITE | PAGE_WRITECOMBINE );
#endif
}


#endif
