/*
	xvbuffer.c
	動的頂点バッファ管理ルーチン

	2002/02/27 K.Takabe
	$Id: wvbuffer.c,v 1.20 2002/12/26 06:15:05 takaki Exp $

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

#include "libgv.h"
#include "libdg.h"

//#include	"gameheader.h"

#include "dmapack.h"
#include	"def_dma.h"

#define	__DYNAMICVBUFF_EXEBUFF_KICK_ALL__	(TRUE)	// Buffer Over時、 実行バッファ内容を全部吐き出し

#define	__DYNAMICVBUFF_SET_PRIORITY__	(FALSE)

/*------------------------------------------------------------------------------*/
/*		Define																	*/
/*------------------------------------------------------------------------------*/
#define	VTXBUFFER_NUM			(4)
#define	VTXBUFFER_MAX			(0x80000)
#define	VTXBUFFER_MIN			(0x80000)

#define	INDBUFFER_VTXCNT_MAX	(0x10000)	// Index Buffer 最大頂点数

#define	VTXBUFFER_LOCKFLAG		(D3DLOCK_NOSYSLOCK)
#define	INDBUFFER_LOCKFLAG		(D3DLOCK_NOSYSLOCK)

/*------------------------------------------------------------------------------*/
/*		Tyepdef																	*/
/*------------------------------------------------------------------------------*/
typedef	struct	DynaVtxBufferSys_
{
#if !__DG_DRAW_IN_EXEBUFFER__
	LPDIRECT3DVERTEXBUFFER8	vtx_buff ;		// 動的確保用Vertex Buffer
#else
	LPDIRECT3DVERTEXBUFFER8	vtx_buff[VTXBUFFER_NUM] ;	// 動的確保用Vertex Buffer
	LPDIRECT3DVERTEXBUFFER8	cur_vtx_buff ;
	DWORD					vtx_buff_cntr ;

	HANDLE					vtx_buffer_use_event[VTXBUFFER_NUM] ;	// 描画使用中判定
#endif

	DWORD			max ;	// Buffer Size

	DWORD	stream_size ;	// 現在のストリームの頂点サイズ(0で未設定)
	DWORD	used_ofs ;		// オフセット
} DynaVtxBufferSys ;

typedef	struct	DynaIndBufferSys_
{
	LPDIRECT3DINDEXBUFFER8	ind_buff ;		// 動的確保用Vertex Buffer
	DWORD					max ;

	BOOL	set ;		// SetIndices済み
	DWORD	used_ofs ;	// オフセット
} DynaIndBufferSys ;


/*------------------------------------------------------------------------------*/
/*		static																	*/
/*------------------------------------------------------------------------------*/
static	DynaVtxBufferSys	_dvtx_buff_sys ;
static	DynaIndBufferSys	_dind_buff_sys ;

/*------------------------------------------------------------------------------*/
/*		ローカル関数プロトタイプ宣言											*/
/*------------------------------------------------------------------------------*/
static	HRESULT	DG_InitDynamicVertexBuffer(void) ;
static	HRESULT	DG_ReleaseDynamicVertexBuffer(void) ;
#define			DG_ResetStreamDynamicVertexBuffer() { _dvtx_buff_sys.stream_size = 0 ; }

static	HRESULT	DG_InitDynamicIndexBuffer(void) ;
static	HRESULT	DG_ReleaseDynamicIndexBuffer(void) ;

static	HRESULT	DG_InitDynamicVertexBufferUnit(DG_DYNAMIC_VBUFF_UNIT *unit, DWORD size) ;
static	HRESULT	DG_ReleaseDynamicVertexBufferUnit(DG_DYNAMIC_VBUFF_UNIT *unit) ;
static	void	DG_ResetDynamicVertexBufferUnit(DG_DYNAMIC_VBUFF_UNIT *unit) ;

static	HRESULT	DG_InitDynamicIndexBufferUnit(DG_DYNAMIC_IBUFF_UNIT *unit, DWORD size) ;
static	HRESULT	DG_ReleaseDynamicIndexBufferUnit(DG_DYNAMIC_IBUFF_UNIT *unit) ;
static	void	DG_ResetDynamicIndexBufferUnit(DG_DYNAMIC_IBUFF_UNIT *unit) ;


/* ---------------------------------------------------------------- */
int			DG_MaxVertexBufferSize ;	/* バッファ割り当てサイズ（xdgmem.cで初期化） */
void		*DG_VertexBuffer[2] ;		/* バッファ先頭アドレス（xdgmem.cで初期化） */
void		*DG_VertexBufferEnd[2] ;	/* バッファ終了アドレス（xdgmem.cで初期化） */
int			DG_LastUseVertexBufferSize ;	/* １フレーム前の最大使用量 */

static int		DG_UseVertexBufferSize ;
static void		*DG_TopVertexBuffer ;
static void		*DG_CurrentVertexBuffer ;
#define USE_VB_RINGBUFF	(8)
static IDirect3DVertexBuffer8	DG_D3DVBuffer8[USE_VB_RINGBUFF][2] ;
static int		DG_VBufferClock[USE_VB_RINGBUFF] ;

DG_VERTEXSTREAM	DG_VertexStream ;

/* ---------------------------------------------------------------- */
/* 動的頂点バッファの初期化 */
void DG_InitDynamicVertexBufferSystem( void )
{
#if FALSE
	int		i ;

	for ( i = 0 ; i < USE_VB_RINGBUFF ; i++ ) DG_VBufferClock[i] = 0 ;
#endif

	DG_InitDynamicVertexBuffer() ;
	DG_InitDynamicIndexBuffer() ;
}

void DG_ReleaseDynamicVertexBufferSystem( void )
{
	DG_ExeBufferSync(FALSE) ;	// 安全策

	DG_ReleaseDynamicIndexBuffer() ;
	DG_ReleaseDynamicVertexBuffer() ;
}


void DG_SetVertexBuffer( int stream, void *addr, int size )
{
	//LPDIRECT3DVERTEXBUFFER8	lpVBuffer8 ;

	if( stream == 0 ){ DG_ResetStreamDynamicVertexBuffer() ; }// Dynamicの方との整合性を取る

	if ( addr == NULL ){
		DG_SetStreamSource( stream, NULL, 0 );
		return ;
	}
#ifndef _WINDOWS

	/* 頂点バッファインターフェイスの初期化 */
#if 0
	lpVBuffer8 = ( DG_VBufferClock ) ? &DG_D3DVBuffer8[ 0 ] : &DG_D3DVBuffer8[ 1 ] ;
	DG_VBufferClock ^= 1 ;
#else
	lpVBuffer8 = &DG_D3DVBuffer8[ stream ][ DG_VBufferClock[ stream ] ] ;
	DG_VBufferClock[ stream ] = ( DG_VBufferClock[ stream ] + 1 ) & 1 ;
#endif

#ifndef _WINDOWS
	lpVBuffer8->Common = 1 | D3DCOMMON_TYPE_VERTEXBUFFER ;
	lpVBuffer8->Data = (int)addr & 0x0fffffff ;
	lpVBuffer8->Lock = 0 ;
#endif

	/* ストリームとして登録する */
	//DG_SetStreamSource( stream, lpVBuffer8, size );

#ifndef _WINDOWS
	DG_VertexStream.input[ stream ].VertexBuffer = lpVBuffer8 ;
	DG_VertexStream.input[ stream ].Stride = size ;
#endif
	DG_VertexStream.max_stream = stream + 1 ;
	/* 指定したストリーム番号が最大入力数になるので */
	/* 複数のストリームをセットする場合はストリーム番号の小さい順に行うこと！ */

#else	//---- for Windows

#endif	// _WINDOWS
}

/* 指定したサイズの動的（使い捨て）頂点バッファを確保＆ストリーム０に設定 */
void *DG_NewDynamicVertexBuffer( int size, int num )
{
#if TRUE
	void	*addr ;
	addr = DG_AllocDynamicVertexBuffer( size, num );
	if ( addr == NULL ) return ( NULL );
	DG_SetVertexBuffer( 0, addr, size );
	return ( addr );
#else
	return(NULL) ;
#endif
}

/* ---------------------------------------------------------------- */
/* 以降、Windows用に新規追加  */


/*------------------------------------------------------------------------------*/
/*		DG_InitDynamicVertexBuffer												*/
/*			ダイナミック頂点バッファ初期化										*/
/*------------------------------------------------------------------------------*/
static	HRESULT	DG_InitDynamicVertexBuffer(void)
{
	BYTE	*dst_vtx ;
	DWORD	size ;
#if __DG_DRAW_IN_EXEBUFFER__
	int 					i ;
	LPDIRECT3DVERTEXBUFFER8	*vbuff ;
	HANDLE					*use_event ;
#endif

	memset(&_dvtx_buff_sys, 0x00, sizeof(_dvtx_buff_sys)) ;

	/*-- Vertex Bufferの確保 ---------------------------------------------------*/

	size = VTXBUFFER_MAX ;

#if __DG_DRAW_IN_EXEBUFFER__
	vbuff     = _dvtx_buff_sys.vtx_buff ;
	use_event = _dvtx_buff_sys.vtx_buffer_use_event ;
	for(i=VTXBUFFER_NUM; i>0; i--, vbuff++, use_event++)
	{
		DG_CreateVertexBuffer(size,
							DG_WinApp.vbuff_usage | D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
							0, D3DPOOL_DEFAULT, vbuff) ;
#if ! __DYNAMICVBUFF_EXEBUFF_KICK_ALL__
		*use_event = CreateEvent(NULL, TRUE, TRUE, NULL) ;
		ASSERT( *use_event ) ;
#endif
	}
	_dvtx_buff_sys.vtx_buff_cntr = 0 ;
	_dvtx_buff_sys.cur_vtx_buff  = _dvtx_buff_sys.vtx_buff[0] ;

#if __DYNAMICVBUFF_SET_PRIORITY__
	IDirect3DVertexBuffer8_SetPriority(_dvtx_buff_sys.vtx_buff[0], 1) ;	// 優先順位を上げる(TOPのみ)
#endif

#else
	DG_CreateVertexBuffer( size,
						D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
						0, D3DPOOL_DEFAULT, &_dvtx_buff_sys.vtx_buff) ;
#if __DYNAMICVBUFF_SET_PRIORITY__
	IDirect3DVertexBuffer8_SetPriority(_dvtx_buff_sys.vtx_buff, 1) ;	// 優先順位を上げる
#endif
#endif
	_dvtx_buff_sys.stream_size = 0 ;
	_dvtx_buff_sys.used_ofs    = 0 ;
	_dvtx_buff_sys.max         = size ;
	/*--------------------------------------------------------------------------*/

	/*-- Memory Clear ----------------------------------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	IDirect3DVertexBuffer8_Lock(_dvtx_buff_sys.cur_vtx_buff,
							0, size, (BYTE **)&dst_vtx,
							D3DLOCK_DISCARD) ;
	ZeroMemory(dst_vtx, size) ;
	IDirect3DVertexBuffer8_Unlock(_dvtx_buff_sys.cur_vtx_buff) ;
#else
	IDirect3DVertexBuffer8_Lock(_dvtx_buff_sys.vtx_buff,
							0, size, (BYTE **)&dst_vtx,
							D3DLOCK_DISCARD) ;
	ZeroMemory(dst_vtx, size) ;
	IDirect3DVertexBuffer8_Unlock(_dvtx_buff_sys.vtx_buff) ;
#endif
	/*--------------------------------------------------------------------------*/

	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_ReleaseDynamicVertexBuffer											*/
/*			ダイナミック頂点バッファ終了処理									*/
/*------------------------------------------------------------------------------*/
static	HRESULT	DG_ReleaseDynamicVertexBuffer(void)
{
#if __DG_DRAW_IN_EXEBUFFER__
	int 					i ;
	LPDIRECT3DVERTEXBUFFER8	*vbuff ;
	HANDLE					*use_event ;

	vbuff = _dvtx_buff_sys.vtx_buff ;
	use_event = _dvtx_buff_sys.vtx_buffer_use_event ;
	for(i=VTXBUFFER_NUM; i>0; i--, vbuff++, use_event++)
	{
		if( *vbuff )
		{
			DG_ReleaseD3DVertexBuffer(*vbuff) ;
			*vbuff = NULL ;
		}

#if ! __DYNAMICVBUFF_EXEBUFF_KICK_ALL__
		if( *use_event )
		{
			CloseHandle(*use_event) ;
			*use_event = NULL ;
		}
#endif
	}
#else
	if( _dvtx_buff_sys.vtx_buff )
	{
		DG_ReleaseD3DVertexBuffer(_dvtx_buff_sys.vtx_buff) ;
		_dvtx_buff_sys.vtx_buff = NULL ;
	}
#endif
	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_RewindDynamicVertexBuffer											*/
/*			ダイナミック頂点バッファReset										*/
/*------------------------------------------------------------------------------*/
void	DG_RewindDynamicVertexBuffer(void)
{
	BYTE	*dst_vtx ;

	_dvtx_buff_sys.stream_size = 0 ;
	_dvtx_buff_sys.used_ofs    = 0 ;

#if __DG_DRAW_IN_EXEBUFFER__
	_dvtx_buff_sys.vtx_buff_cntr     = 0 ;
	_dvtx_buff_sys.cur_vtx_buff      = _dvtx_buff_sys.vtx_buff[0] ;

	if( _dvtx_buff_sys.cur_vtx_buff )
	{
		IDirect3DVertexBuffer8_Lock(_dvtx_buff_sys.cur_vtx_buff,
								0, _dvtx_buff_sys.max, (BYTE **)&dst_vtx,
								D3DLOCK_DISCARD) ;
		IDirect3DVertexBuffer8_Unlock(_dvtx_buff_sys.cur_vtx_buff) ;
	}
#else
	if( _dvtx_buff_sys.vtx_buff )
	{
		IDirect3DVertexBuffer8_Lock(_dvtx_buff_sys.vtx_buff,
								0, _dvtx_buff_sys.max, (BYTE **)&dst_vtx,
								D3DLOCK_DISCARD) ;					// とりあえずLockだけしておく
		IDirect3DVertexBuffer8_Unlock(_dvtx_buff_sys.vtx_buff) ;
	}
#endif
}

/*------------------------------------------------------------------------------*/
/*		DG_SetDynamicVertexBuffer												*/
/*			ダイナミック頂点バッファに頂点を設定								*/
/*------------------------------------------------------------------------------*/
static	void	DG_SetDynamicVertexBuffer_Callback(DWORD arg)
{
#if ! __DYNAMICVBUFF_EXEBUFF_KICK_ALL__
	SetEvent(_dvtx_buff_sys.vtx_buffer_use_event[arg]) ;
#endif
}

HRESULT	DG_SetDynamicVertexBuffer(void *vtx, DWORD size, int num, DWORD *start_ofs)
{
	LPDIRECT3DVERTEXBUFFER8	vtx_buff ;
	void					*dst_vtx ;
	DWORD					total_size ;
	DWORD					ofs ;
	DWORD					used_ofs ;
	DWORD					lock_flag ;
#if __DG_DRAW_IN_EXEBUFFER__
	BOOL					set_stream ;
#if ! __DYNAMICVBUFF_EXEBUFF_KICK_ALL__
	BOOL					change_buff ;
	DWORD					vtx_buff_ocntr ;
#endif
	DWORD					cntr ;
#endif

	total_size  = (size * (DWORD)num) ;
	used_ofs    = _dvtx_buff_sys.used_ofs ;
#if __DG_DRAW_IN_EXEBUFFER__
	vtx_buff    = _dvtx_buff_sys.cur_vtx_buff ;
	set_stream  = FALSE ;
#if !__DYNAMICVBUFF_EXEBUFF_KICK_ALL__
	change_buff = FALSE ;
#endif
#else
	vtx_buff    = _dvtx_buff_sys.vtx_buff ;
#endif
	lock_flag   = D3DLOCK_NOOVERWRITE | VTXBUFFER_LOCKFLAG ;

	/*-- Vertexバッファより大きければやめる ------------------------------------*/

	if( total_size > _dvtx_buff_sys.max ){ ASSERT(0) ; return(S_OK) ; }
	/*--------------------------------------------------------------------------*/

	/*-- 頂点Stream再設定 ------------------------------------------------------*/

	if( _dvtx_buff_sys.stream_size != size )
	{		
#if !__DG_DRAW_IN_EXEBUFFER__
		DG_SetStreamSource(0, vtx_buff, size) ;
#else
		set_stream = TRUE ;	// SetStreamが必要
#endif
		_dvtx_buff_sys.stream_size = size ;

		used_ofs = ((used_ofs + size - 1) / size) * size ;	// 再設定
	}
	/*--------------------------------------------------------------------------*/

	/*-- 頂点開始位置計算 ------------------------------------------------------*/

	ofs       = used_ofs ;
	used_ofs += total_size ;

	if( used_ofs >= _dvtx_buff_sys.max )	// 限界を超えたら巻き戻し
	{
#if !__DG_DRAW_IN_EXEBUFFER__
		ofs = 0 ;
		_dvtx_buff_sys.used_ofs = total_size ;

		lock_flag = D3DLOCK_DISCARD | VTXBUFFER_LOCKFLAG ;
#else

#if __DYNAMICVBUFF_EXEBUFF_KICK_ALL__
		cntr = _dvtx_buff_sys.vtx_buff_cntr ;
#else
		/*-- Callbackの為の情報保存 --------------------------------------------*/

		change_buff = TRUE ;

		cntr           = _dvtx_buff_sys.vtx_buff_cntr ;
		vtx_buff_ocntr = cntr ;
		/*----------------------------------------------------------------------*/
#endif
		/*-- Buffer切り替え ----------------------------------------------------*/

		lock_flag = D3DLOCK_DISCARD | VTXBUFFER_LOCKFLAG ;

		ofs = 0 ;
		_dvtx_buff_sys.used_ofs = total_size ;

		cntr++ ;
#if !__DYNAMICVBUFF_EXEBUFF_KICK_ALL__
		if( cntr >= VTXBUFFER_NUM ){ cntr = 0 ; }
#else
		if( cntr >= VTXBUFFER_NUM )
		{
			cntr = 0 ;
			DG_KickExeBuffer() ;		// 現在の実行バッファ内容を全部吐き出す
			DG_ExeBufferSync(FALSE) ;
		}
#endif

		_dvtx_buff_sys.vtx_buff_cntr = cntr ;

		vtx_buff = _dvtx_buff_sys.vtx_buff[cntr] ;
		_dvtx_buff_sys.cur_vtx_buff = vtx_buff ;
		/*----------------------------------------------------------------------*/

		/*-- VertexBufferが使用中の場合はLock ----------------------------------*/

#if !__DYNAMICVBUFF_EXEBUFF_KICK_ALL__
		WaitForSingleObject(_dvtx_buff_sys.vtx_buffer_use_event[cntr], INFINITE) ;
#endif
		/*----------------------------------------------------------------------*/

		set_stream = TRUE ;	// SetStreamが必要

#endif
	}
	else
	{
		_dvtx_buff_sys.used_ofs = used_ofs ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- SetStream -------------------------------------------------------------*/
#if __DG_DRAW_IN_EXEBUFFER__
	if( set_stream )
	{
		DG_SetStreamSource(0, vtx_buff, size) ;

#if !__DYNAMICVBUFF_EXEBUFF_KICK_ALL__
		if( change_buff )
		{
			/*-- Callback ------------------------------------------------------*/

			cntr = vtx_buff_ocntr ;

			ResetEvent(_dvtx_buff_sys.vtx_buffer_use_event[cntr]) ; // 使用中判定
			DG_ExeBufferCallback((void *)DG_SetDynamicVertexBuffer_Callback, cntr) ;
			/*------------------------------------------------------------------*/

			/*-- ここまでの実行バッファを実行 ----------------------------------*/

			DG_KickExeBuffer() ;
			/*------------------------------------------------------------------*/
		}
#endif
	}
#endif
	/*--------------------------------------------------------------------------*/

	/*-- Vertex Bufferに複製 ---------------------------------------------------*/

	
	ASSERT(total_size) ;
	IDirect3DVertexBuffer8_Lock(vtx_buff, ofs, total_size, (BYTE **)&dst_vtx, lock_flag) ;
	memcpy(dst_vtx, vtx, total_size) ;
	IDirect3DVertexBuffer8_Unlock(vtx_buff) ;
	/*--------------------------------------------------------------------------*/

	/*-- 頂点開始位置Offset計算 ------------------------------------------------*/

	*start_ofs = (ofs / size) ;
	/*--------------------------------------------------------------------------*/

	return( S_OK ) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_SetDynamicVertexBufferZeroOfs										*/
/*			ダイナミック頂点バッファに頂点を設定(0番目に強制設定)				*/
/*------------------------------------------------------------------------------*/
HRESULT	DG_SetDynamicVertexBufferZeroOfs(void *vtx, DWORD size, int num)
{
	LPDIRECT3DVERTEXBUFFER8	vtx_buff ;
	void					*dst_vtx ;
	DWORD					total_size ;

	ASSERT(0) ;	// Buffer切り替えに対応していないので、とりあえず止める

	total_size  = (size * (DWORD)num) ;
#if __DG_DRAW_IN_EXEBUFFER__
	vtx_buff    = _dvtx_buff_sys.cur_vtx_buff ;
#else
	vtx_buff    = _dvtx_buff_sys.vtx_buff ;
#endif
	/*-- Vertexバッファより大きければやめる ------------------------------------*/

	if( total_size > _dvtx_buff_sys.max ){ ASSERT(0) ; return(S_OK) ; }
	/*--------------------------------------------------------------------------*/

	/*-- 頂点Stream再設定 ------------------------------------------------------*/

	if( _dvtx_buff_sys.stream_size != size )
	{		
		DG_SetStreamSource(0, vtx_buff, size) ;
		_dvtx_buff_sys.stream_size = size ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 頂点開始位置計算 ------------------------------------------------------*/

	_dvtx_buff_sys.used_ofs = total_size ;
	/*--------------------------------------------------------------------------*/

	/*-- Vertex Bufferに複製 ---------------------------------------------------*/

	IDirect3DVertexBuffer8_Lock(vtx_buff, 0, total_size, (BYTE **)&dst_vtx, D3DLOCK_DISCARD) ;
	memcpy(dst_vtx, vtx, total_size) ;
	IDirect3DVertexBuffer8_Unlock(vtx_buff) ;
	/*--------------------------------------------------------------------------*/

	return( S_OK ) ;
}



/*------------------------------------------------------------------------------*/
/*		DG_InitDynamicIndexBuffer												*/
/*			ダイナミックインデックスバッファ初期化								*/
/*------------------------------------------------------------------------------*/
static	HRESULT	DG_InitDynamicIndexBuffer(void)
{
	DWORD	max ;
	BYTE	*dst ;

	/*-- インデックスバッファ機能実装判定 --------------------------------------*/

	if( !DG_CheckIndexPrimitiveUseable() ){ return(S_OK) ; }
	/*--------------------------------------------------------------------------*/


	memset(&_dind_buff_sys, 0x00, sizeof(_dind_buff_sys)) ;

	/*-- Index Buffer 作成 -----------------------------------------------------*/

	max = DG_WinApp.d3d_cap.MaxVertexIndex / sizeof(WORD) ;
	if( max > INDBUFFER_VTXCNT_MAX ){ max = INDBUFFER_VTXCNT_MAX ; }

	_dind_buff_sys.max = max ;
	DG_CreateIndexBuffer(max * sizeof(WORD),
						DG_WinApp.vbuff_usage | D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
						D3DFMT_INDEX16, D3DPOOL_DEFAULT,
						&_dind_buff_sys.ind_buff) ;
#if __DYNAMICVBUFF_SET_PRIORITY__
	IDirect3DIndexBuffer8_SetPriority(_dind_buff_sys.ind_buff, 1) ;	// 優先順位を上げる
#endif
	/*--------------------------------------------------------------------------*/

	/*-- Memory Clear ----------------------------------------------------------*/

	IDirect3DIndexBuffer8_Lock(_dind_buff_sys.ind_buff,
							0, max * sizeof(WORD), (BYTE **)&dst,
							D3DLOCK_DISCARD) ;
	ZeroMemory(dst, max * sizeof(WORD)) ;
	IDirect3DIndexBuffer8_Unlock(_dind_buff_sys.ind_buff) ;
	/*--------------------------------------------------------------------------*/

	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_ReleaseDynamicIndexBuffer											*/
/*			ダイナミックインデックスバッファ終了処理							*/
/*------------------------------------------------------------------------------*/
static	HRESULT	DG_ReleaseDynamicIndexBuffer(void)
{
	if( _dind_buff_sys.ind_buff )
	{
		DG_ReleaseD3DIndexBuffer(_dind_buff_sys.ind_buff) ;
		_dind_buff_sys.ind_buff = NULL ;
	}

	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_RewindDynamicIndexBuffer												*/
/*			ダイナミックインデックスバッファReset								*/
/*------------------------------------------------------------------------------*/
void	DG_RewindDynamicIndexBuffer(void)
{
	BYTE	*dst ;

	_dind_buff_sys.used_ofs = 0 ;
	if( _dind_buff_sys.ind_buff )
	{
		IDirect3DIndexBuffer8_Lock(_dind_buff_sys.ind_buff,
								0,_dind_buff_sys.max * sizeof(WORD), (BYTE **)&dst,
								D3DLOCK_DISCARD) ;
		IDirect3DIndexBuffer8_Unlock(_dind_buff_sys.ind_buff) ;
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_SetDynamicIndexBuffer												*/
/*			ダイナミックインデックスバッファ設定								*/
/*------------------------------------------------------------------------------*/
HRESULT	DG_SetDynamicIndexBuffer(WORD *idx, int num, DWORD base_idxofs, DWORD *start_ofs)
{
	LPDIRECT3DINDEXBUFFER8	ind_buff ;
	DWORD					ofs ;
	DWORD					used_ofs ;
	DWORD					max ;
	DWORD					lock_flag ;

	max       = _dind_buff_sys.max ;
	lock_flag = D3DLOCK_NOOVERWRITE | INDBUFFER_LOCKFLAG;

	/*-- インデックスバッファ機能実装判定 --------------------------------------*/
#ifdef DEBUG_MODE
	if( !DG_CheckIndexPrimitiveUseable() ){ return(S_OK) ; }
#endif
	/*--------------------------------------------------------------------------*/

	/*-- バッファサイズが足らない場合は終了 ------------------------------------*/

	if( num > max ){ ASSERT(0) ; return(S_OK) ; }
	/*--------------------------------------------------------------------------*/

	/*-- インデックスバッファ設定 ----------------------------------------------*/

	ofs      = _dind_buff_sys.used_ofs ;
	ind_buff = _dind_buff_sys.ind_buff ;
	DG_SetIndices(ind_buff, base_idxofs) ;
	/*--------------------------------------------------------------------------*/

	/*-- 確保場所を決定 --------------------------------------------------------*/

	used_ofs = ofs + num ;
	if( used_ofs > max )
	{
#if !__DG_DRAW_IN_EXEBUFFER__
		ofs = 0 ;							// 使用量オーバーの際には巻き戻し
		_dind_buff_sys.used_ofs = num ;

		lock_flag = D3DLOCK_DISCARD | INDBUFFER_LOCKFLAG ;
#else

		/*-- 現在のバッファを全部吐き出し --------------------------------------*/

		DG_KickExeBuffer() ;
		DG_ExeBufferSync(FALSE) ;
		/*----------------------------------------------------------------------*/

		/*-- 使用量オーバーの際には巻き戻し ------------------------------------*/

		DG_RewindDynamicVertexBuffer() ;

		ofs = 0 ;
		_dind_buff_sys.used_ofs = num ;
		/*----------------------------------------------------------------------*/
#endif
	}
	else
	{
		_dind_buff_sys.used_ofs = used_ofs ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- Index設定 -------------------------------------------------------------*/

	{
		WORD	*dst_idx ;
		DWORD	size ;

		size = num << 1 ;

		IDirect3DIndexBuffer8_Lock(ind_buff, ofs << 1, size, (BYTE**)&dst_idx, lock_flag) ;
		memcpy(dst_idx, idx, size) ;
		IDirect3DIndexBuffer8_Unlock(ind_buff) ;
	}
	/*--------------------------------------------------------------------------*/

	*start_ofs = ofs ;
	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_SetStaticVertexBuffer												*/
/*			静的頂点バッファ設定												*/
/*------------------------------------------------------------------------------*/
HRESULT	DG_SetStaticVertexBuffer(int stream, LPDIRECT3DVERTEXBUFFER8 vbuff, DWORD stride)
{
	if( stream == 0 ){ DG_ResetStreamDynamicVertexBuffer() ; }// Dynamicの方との整合性を取る

	DG_SetStreamSource(stream, vbuff, stride) ;

	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_SetStaticIndexBuffer													*/
/*			静的インデックスバッファ設定										*/
/*------------------------------------------------------------------------------*/
HRESULT	DG_SetStaticIndexBuffer(LPDIRECT3DINDEXBUFFER8 ibuff, DWORD base_idxofs)
{
	/*-- インデックスバッファ機能実装判定 --------------------------------------*/
#ifdef DEBUG_MODE
	if( !DG_CheckIndexPrimitiveUseable() ){ return(S_OK) ; }
#endif
	/*--------------------------------------------------------------------------*/

	DG_SetIndices(ibuff, base_idxofs) ;

	return(S_OK) ;
}

/*------------------------------------------------------------------------------*/
/*		DG_ArrangeVertexByIndex													*/
/*			頂点バッファをIndex情報により並び替える								*/
/*------------------------------------------------------------------------------*/
static void	DG_ArrangeVertexByIndex4B(DWORD *dst, DWORD *src, DWORD stride,
									WORD *index, DWORD num) ;

void	DG_ArrangeVertexByIndex(BYTE *dst, BYTE *src, DWORD stride,
									WORD *index, DWORD num)
{
	DWORD	i, j ;
	BYTE	*src_ptr ;

	if( !(stride & 3) && !((DWORD)dst & 3) && !((DWORD)src & 3) )
	{
		DG_ArrangeVertexByIndex4B((DWORD *)dst, (DWORD *)src,stride,
									index, num) ;
		return ;
	}

	for(i=num; i>0; i--)
	{
		src_ptr = src + (stride * (*index)) ;
		for(j=stride; j>0; j--)
		{
			*dst = *src_ptr ;
			dst++ ;
			src_ptr++ ;
		}

		index++ ;
	}
}

/*------------------------------------------------------------------------------*/
/*		DG_ArrangeVertexByIndex4B												*/
/*			DG_ArrangeVertexByIndexの4Byte Align版								*/
/*------------------------------------------------------------------------------*/
void	DG_ArrangeVertexByIndex4B(DWORD *dst, DWORD *src, DWORD stride,
									WORD *index, DWORD num)
{
	DWORD	i, j ;
	DWORD	*src_ptr ;

	stride >>= 2 ;

	for(i=num; i>0; i--)
	{
		src_ptr = src + (stride * (*index)) ;
		for(j=stride; j>0; j--)
		{
			*dst = *src_ptr ;
			dst++ ;
			src_ptr++ ;
		}

		index++ ;
	}
}
