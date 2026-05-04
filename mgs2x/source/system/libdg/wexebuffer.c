/*--------------------------------------------------------------*/
/*	wexebuffer.c												*/
/*					描画実行バッファ							*/
/*--------------------------------------------------------------*/
#define	__WEXEBUFFER_C__

#include <stdio.h>

#include "xtl.h"
#include "libdg.h"
#include "wexebuffer.h"

#define	__REQDONE_SYNC_BY_EVENT__	(TRUE)	// Request/Doneの同期はEventで行う
#define	__MARGE_CMD_DRAWPRIM__		(TRUE)	// DrawPrim系コマンドを可能ならば合体させる
#define	__MULTI_EXE_BUFFER__		(FALSE)	// Buffer多重化を行う

/*----------------------------------------------------------------------*/
/*	Define																*/
/*----------------------------------------------------------------------*/

#define	CMD_BUFFER_SIZE	(0x200000)

#if __MULTI_EXE_BUFFER__
#define	EXE_BUFFER_NUM	(2)
#endif
/*----------------------------------------------------------------------*/
/*	Typedef																*/
/*----------------------------------------------------------------------*/

typedef	struct	DG_EXEBUFF_CMD_HEADER_
{
	void	(*exe_func)(void *param) ;	// 実行関数
	DWORD	size ;
#ifdef DEBUG_MODE
	char	*func_name ;
#endif
} DG_EXEBUFF_CMD_HEADER ;

typedef	struct	DG_EXEBUFF_
{
	DWORD	*cmd_buff_top ;		// Command Buffer先頭
	int		cmd_buff_rem ;		// Command Buffer残量

	DWORD	*cmd_buff_exe ;			// Command Buffer実行先頭
	DWORD	*cmd_buff_exe_term ;	// Command Buffer実行終端

	DG_EXEBUFF_CMD_HEADER	*pre_cmd ;		// 前回のCommand

	DWORD					cmd_buff[CMD_BUFFER_SIZE>>2] ;
} DG_EXEBUFF ;

typedef	struct	DG_EXEBUFF_MAN_
{
#if __MULTI_EXE_BUFFER__
	int	sw ;							// 現在使用中のバッファ
	DG_EXEBUFF	buff[EXE_BUFFER_NUM] ;	// 多重化実行バッファ
#else
	DG_EXEBUFF	buff ;					// 実行バッファ
#endif

#if __REQDONE_SYNC_BY_EVENT__
	HANDLE	draw_req_handle ;	// 描画開始要求イベントハンドラ
	HANDLE	draw_done_handle ;	// 描画終了イベントハンドラ
#else
	volatile BOOL	draw_req ;	// 描画開始要求
	volatile BOOL	draw_done ;	// 描画終了
#endif
	HANDLE	draw_thread_handle ;
	DWORD	draw_thread_id ;
} DG_EXEBUFF_MAN ;


/*----------------------------------------------------------------------*/
/*	static																*/
/*----------------------------------------------------------------------*/

static	DG_EXEBUFF_MAN	_man ;

#if __EXEBUFFER_CALLFILE_DEBUG__
static	DWORD	_VS_Handle ;
#endif

/*----------------------------------------------------------------------*/
/*	ローカル関数プロトタイプ宣言										*/
/*----------------------------------------------------------------------*/

static	void	DG_ResetExeBufferDirect(DG_EXEBUFF *buff) ;

#ifdef DEBUG_MODE
#define	DG_AllocCmdExeBuffer(func_, size_)	\
						_DG_AllocCmdExeBuffer((void *)func_, size_, #func_)

static inline	DWORD	*_DG_AllocCmdExeBuffe(void *func, DWORD size, char *func_name) ;
#else
#define	DG_AllocCmdExeBuffer(func_, size_) \
						_DG_AllocCmdExeBuffer((void *)func_, size_)

static inline	DWORD	*_DG_AllocCmdExeBuffer(void *func, DWORD size) ;
#endif


static DWORD WINAPI	DG_DrawExeBufferThread(LPVOID arg) ;


/*----------------------------------------------------------------------*/
/*	DG_InitExeBuffer	実行バッファ管理初期化							*/
/*----------------------------------------------------------------------*/
void	DG_InitExeBuffer(void)
{
#if __MULTI_EXE_BUFFER__
	int			i ;
#endif
	DG_EXEBUFF	*buff ;

	ZeroMemory(&_man, sizeof(_man)) ;

	/*-- バッファ作成 --------------------------------------------------*/

#if __MULTI_EXE_BUFFER__
	buff = _man.buff ;
	for(i=EXE_BUFFER_NUM; i>0; i--, buff++)
	{
		/*-- 内容初期化 ------------------------------------------------*/

		DG_ResetExeBufferDirect(buff) ;
		/*--------------------------------------------------------------*/
	}
#else
	buff = &_man.buff ;
	DG_ResetExeBufferDirect(buff) ;
#endif

	/*------------------------------------------------------------------*/

	/*-- 描画制御用イベント作成 ----------------------------------------*/

#if __REQDONE_SYNC_BY_EVENT__
	_man.draw_req_handle  = CreateEvent(NULL, TRUE, FALSE, NULL) ;
	_man.draw_done_handle = CreateEvent(NULL, TRUE,  TRUE, NULL) ;
#else
	_man.draw_req  = FALSE ;
	_man.draw_done = TRUE ;
#endif
	/*------------------------------------------------------------------*/

	/*-- 描画スレッド作成 ----------------------------------------------*/

	_man.draw_thread_handle = CreateThread( 0, 0, &DG_DrawExeBufferThread,
										0, 0, &_man.draw_thread_id) ;
	/*------------------------------------------------------------------*/
}

/*----------------------------------------------------------------------*/
/*	DG_ReleaseExeBuffer	実行バッファ管理解放処理						*/
/*----------------------------------------------------------------------*/
void	DG_ReleaseExeBuffer(void)
{
	/*-- 描画Thread終了 ------------------------------------------------*/

	if( _man.draw_thread_handle )
	{
		DG_ExeBufferSync(FALSE) ;
		CloseHandle(_man.draw_thread_handle) ;
	}
	/*------------------------------------------------------------------*/

	/*-- 描画制御用イベントClose ---------------------------------------*/

#if __REQDONE_SYNC_BY_EVENT__
	if( _man.draw_req_handle  ){ CloseHandle(_man.draw_req_handle) ; }
	if( _man.draw_done_handle ){ CloseHandle(_man.draw_done_handle) ; }
#endif
	/*------------------------------------------------------------------*/

	ZeroMemory(&_man, sizeof(_man)) ;
#if !__REQDONE_SYNC_BY_EVENT__
	_man.draw_done = TRUE ;
#endif
}

/*----------------------------------------------------------------------*/
/*	DG_RunExeBuffer	実行バッファ実行									*/
/*----------------------------------------------------------------------*/
extern volatile	DWORD	_dipswitch ;
void	DG_RunExeBuffer(void)
{
	DG_EXEBUFF				*buff ;
	DWORD					*term ;
	DWORD					*cmd ;
	DG_EXEBUFF_CMD_HEADER	*cmd_header ;

	SetThreadPriority(_man.draw_thread_handle, THREAD_PRIORITY_HIGHEST) ;

#if __MULTI_EXE_BUFFER__
	buff = &_man.buff[_man.sw] ;
#else
	buff = &_man.buff ;
#endif
	term = buff->cmd_buff_exe_term ;	// 終端
	cmd  = buff->cmd_buff_exe ;
	while( cmd < term )
	{
		cmd_header = (void *)cmd ;

		/*-- 実行 ------------------------------------------------------*/

#ifdef DEBUG_MODE
#if TRUE
		if( _dipswitch & (1 << 4) )
		{
			int		i ;
			DWORD	*ptr ;

			printf("%s(", cmd_header->func_name) ;
			ptr = (DWORD *)(cmd_header+1) ;
			for(i=(cmd_header->size - sizeof(DG_EXEBUFF_CMD_HEADER))>>2; i>0; i--, ptr++)
			{
				printf("%d,", *ptr) ;
			}
			printf(")\n") ;
		}
#endif
#endif
		ASSERT( cmd_header->exe_func ) ;
		cmd_header->exe_func(cmd_header) ;
		/*--------------------------------------------------------------*/

		/*-- 次のコマンドへ --------------------------------------------*/

		(DWORD)cmd += cmd_header->size ;
		/*--------------------------------------------------------------*/
	}
	buff->cmd_buff_exe = cmd ;

	SetThreadPriority(_man.draw_thread_handle, THREAD_PRIORITY_NORMAL) ;
	Sleep(0) ;
}


/*----------------------------------------------------------------------*/
/*	DG_ExeBufferSync	実行バッファ終了Sync							*/
/*----------------------------------------------------------------------*/
BOOL	DG_ExeBufferSync(BOOL imm)
{
#if __REQDONE_SYNC_BY_EVENT__
	if( !_man.draw_done_handle ){ return(TRUE) ; }
#endif

	if( !imm )
	{
		/*-- 終了までLockする ------------------------------------------*/

#if __REQDONE_SYNC_BY_EVENT__
		WaitForSingleObject(_man.draw_done_handle, INFINITE) ;
#else
		while( !_man.draw_done ){ Sleep(0) ; }
#endif
		return(TRUE) ;
		/*--------------------------------------------------------------*/
	}
	else
	{
#if __REQDONE_SYNC_BY_EVENT__
		DWORD	ret ;

		/*-- 現在の状態を即時に返す ------------------------------------*/

		ret = WaitForSingleObject(_man.draw_done_handle, 0) ;
		return( ret == WAIT_OBJECT_0 ) ;
		/*--------------------------------------------------------------*/
#else
		return(_man.draw_done) ;
#endif
	}
}

/*----------------------------------------------------------------------*/
/*	DG_FlipExeBuffer	実行バッファFlip								*/
/*----------------------------------------------------------------------*/
void	DG_FlipExeBuffer(void)
{
	DG_EXEBUFF				*buff ;

	/*-- 溜まっている物があったらとりあえず吐き出す --------------------*/

#if __MULTI_EXE_BUFFER__
	buff = &_man.buff[_man.sw] ;
#else
	buff = &_man.buff ;
#endif
	if( buff->cmd_buff_exe_term < buff->cmd_buff_top ){ DG_KickExeBuffer() ; }
	/*------------------------------------------------------------------*/

	/*-- 描画終了待ち --------------------------------------------------*/

	DG_ExeBufferSync(FALSE) ;
	/*------------------------------------------------------------------*/

	DG_PerfReset(DGPERF_CTGR_DRAWALL) ;

	/*-- Buffer切り替え ------------------------------------------------*/

#if __MULTI_EXE_BUFFER__
	_man.sw-- ;
	if( _man.sw < 0 ){ _man.sw = EXE_BUFFER_NUM - 1 ; }
#endif
	DG_ResetExeBuffer() ;

	DG_RewindDynamicVertexBuffer() ;
	DG_RewindDynamicIndexBuffer() ;
	/*------------------------------------------------------------------*/
}

/*----------------------------------------------------------------------*/
/*	DG_ResetExeBufferDirect	実行バッファReset							*/
/*----------------------------------------------------------------------*/
static	void	DG_ResetExeBufferDirect(DG_EXEBUFF *buff)
{
	buff->cmd_buff_top = buff->cmd_buff ;		// Command Buffer先頭
	buff->cmd_buff_rem = CMD_BUFFER_SIZE ;		// Command Buffer残量

	buff->cmd_buff_exe      = buff->cmd_buff_top ;	// Command Buffer実行先頭
	buff->cmd_buff_exe_term = buff->cmd_buff_top ;	// Command Buffer実行終端

	buff->pre_cmd = NULL ;
}

/*----------------------------------------------------------------------*/
/*	DG_ResetExeBuffer	実行バッファReset								*/
/*----------------------------------------------------------------------*/
void	DG_ResetExeBuffer(void)
{
#if __MULTI_EXE_BUFFER__
	DG_ResetExeBufferDirect(&_man.buff[_man.sw]) ;
#else
	DG_ResetExeBufferDirect(&_man.buff) ;
#endif
}

/*----------------------------------------------------------------------*/
/*	DG_KickExeBuffer	描画開始										*/
/*----------------------------------------------------------------------*/
void	DG_KickExeBuffer(void)
{
	DG_EXEBUFF				*buff ;

	/*-- 実行終端設定 --------------------------------------------------*/

#if __MULTI_EXE_BUFFER__
	buff = &_man.buff[_man.sw] ;
#else
	buff = &_man.buff ;
#endif
	buff->cmd_buff_exe_term = buff->cmd_buff_top ;	// 現在登録されている物まで実行
	buff->pre_cmd = NULL ;
	/*------------------------------------------------------------------*/

#if __REQDONE_SYNC_BY_EVENT__
	ResetEvent(_man.draw_done_handle) ;
	SetEvent(_man.draw_req_handle) ;
#else
	_man.draw_done = FALSE ;
	_man.draw_req  = TRUE ;
#endif
}

/*----------------------------------------------------------------------*/
/*	DG_AllocCmdExeBuffer	実行コマンド領域確保						*/
/*----------------------------------------------------------------------*/
#ifdef DEBUG_MODE
static inline	DWORD	*_DG_AllocCmdExeBuffer(void *func, DWORD size, char *func_name)
#else
static inline	DWORD	*_DG_AllocCmdExeBuffer(void *func, DWORD size)
#endif
{
	DG_EXEBUFF				*buff ;
	DG_EXEBUFF_CMD_HEADER	*cmd_header ;

	ASSERT( !(size & 3) ) ;

#if __MULTI_EXE_BUFFER__
	buff = &_man.buff[_man.sw] ;
#else
	buff = &_man.buff ;
#endif

	if( buff->cmd_buff_rem < (int)size ){ return(NULL) ; }

	cmd_header = (void *)buff->cmd_buff_top ;

	/*-- ヘッダ作成 ----------------------------------------------------*/

	cmd_header->exe_func  = func ;
	cmd_header->size      = size ;
#ifdef DEBUG_MODE
	cmd_header->func_name = func_name ;
#endif
	/*------------------------------------------------------------------*/

	/*-- コマンドバッファを進める --------------------------------------*/

	(BYTE *)buff->cmd_buff_top += size ;
	buff->cmd_buff_rem         -= size ;
	/*------------------------------------------------------------------*/

	buff->pre_cmd = cmd_header ;	// 前回のCommand

	return((void *)cmd_header) ;
}

/*----------------------------------------------------------------------*/
/*	DG_DrawExeBufferThread	実行バッファ描画Thread						*/
/*----------------------------------------------------------------------*/
static DWORD WINAPI	DG_DrawExeBufferThread(LPVOID arg)
{
	HRESULT	hr ;

	while(TRUE)
	{
		/*-- 描画要求待ち ----------------------------------------------*/

#if __REQDONE_SYNC_BY_EVENT__
		WaitForSingleObject(_man.draw_req_handle, INFINITE) ;
		ResetEvent(_man.draw_done_handle) ;
#else
		while( !_man.draw_req ){ Sleep(0) ; }
		_man.draw_done = FALSE ;
#endif
		/*--------------------------------------------------------------*/

		/*-- 描画実行 --------------------------------------------------*/

		DG_PerfStart(DGPERF_CTGR_DRAWALL) ;		// 描画時間総計計測開始

		DG_SetDrawStartMark();
		hr = IDirect3DDevice8_BeginScene(g_pd3dDevice) ;
		ASSERT( !FAILED(hr) ) ;
#if __REQDONE_SYNC_BY_EVENT__
		while( WaitForSingleObject(_man.draw_req_handle, 0) == WAIT_OBJECT_0 )
#else
		while( _man.draw_req )
#endif
		{
#if __REQDONE_SYNC_BY_EVENT__
			ResetEvent(_man.draw_req_handle) ;
#else
			_man.draw_req = FALSE ;
#endif
			DG_RunExeBuffer() ;
		}
		hr = IDirect3DDevice8_EndScene(g_pd3dDevice) ;
		ASSERT( !FAILED(hr) ) ;
		DG_SetDrawEndMark();

		DG_PerfEnd(DGPERF_CTGR_DRAWALL) ;		// 描画時間総計計測開始
		/*--------------------------------------------------------------*/

		/*-- 終了 ------------------------------------------------------*/

#if __REQDONE_SYNC_BY_EVENT__
		SetEvent(_man.draw_done_handle) ;
#else
		_man.draw_done = TRUE ;
#endif
		/*--------------------------------------------------------------*/
	}
}

/*----------------------------------------------------------------------*/
/*	DG_Clear_EB															*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_CLEAR_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD		Count ;
	DWORD		Flags ;
	D3DCOLOR	Color ;
	float		Z ;
	DWORD		Stencil ;
} EXECMD_CLEAR ;

static void	DG_Clear_Exe(EXECMD_CLEAR *cmd) ;

void	DG_Clear_EB(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
			  D3DCOLOR Color, float Z, DWORD Stencil)
{
	EXECMD_CLEAR	*cmd ;
	DWORD			size ;

	size = sizeof(EXECMD_CLEAR) + (sizeof(D3DRECT) * Count) ;
	cmd = (void *)DG_AllocCmdExeBuffer(DG_Clear_Exe, size) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Count   = Count ;
	cmd->Flags   = Flags ;
	cmd->Color   = Color ;
	cmd->Z       = Z ;
	cmd->Stencil = Stencil ;

	if( Count ){ memcpy((void * )(cmd + 1), pRects, sizeof(D3DRECT) * Count) ; }
}

static void	DG_Clear_Exe(EXECMD_CLEAR *cmd)
{
	HRESULT	hr ;
	D3DRECT	*pRect ;

	if( cmd->Count > 0 ){ pRect = (D3DRECT *)(cmd + 1) ; }
	else{ pRect = NULL ; }

	hr = IDirect3DDevice8_Clear( g_pd3dDevice,
							cmd->Count,
							pRect,
							cmd->Flags,
							cmd->Color,
							cmd->Z,
							cmd->Stencil) ;
	ASSERT( !FAILED(hr) ) ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetRenderState_EB												*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETRENDERSTATE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	D3DRENDERSTATETYPE	State ;
	DWORD				Value ;
} EXECMD_SETRENDERSTATE ;

static void	DG_SetRenderState_Exe(EXECMD_SETRENDERSTATE *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetRenderState( g_pd3dDevice,
									cmd->State,
									cmd->Value) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetRenderState_EB(D3DRENDERSTATETYPE State, DWORD Value)
{
	EXECMD_SETRENDERSTATE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetRenderState_Exe, sizeof(EXECMD_SETRENDERSTATE)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->State = State ;
	cmd->Value = Value ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetTransform_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETTRANSFORM_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	D3DTRANSFORMSTATETYPE	State ;
	D3DMATRIX				Matrix ;
} EXECMD_SETTRANSFORM ;

static void	DG_SetTransform_Exe(EXECMD_SETTRANSFORM *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetTransform( g_pd3dDevice,
									cmd->State,
									&cmd->Matrix) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetTransform_EB( D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	EXECMD_SETTRANSFORM	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetTransform_Exe, sizeof(EXECMD_SETTRANSFORM)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->State  = State ;
	cmd->Matrix = *pMatrix ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetRenderTarget_EB												*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETRENDERTARGET_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	IDirect3DSurface8* pRenderTarget ;
	IDirect3DSurface8* pNewZStencil ;
} EXECMD_SETRENDERTARGET ;

static void	DG_SetRenderTarget_Exe(EXECMD_SETRENDERTARGET *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetRenderTarget( g_pd3dDevice,
									cmd->pRenderTarget,
									cmd->pNewZStencil) ;
	ASSERT( !FAILED(hr) ) ;
}

extern void DG_SetRenderTarget_EB(IDirect3DSurface8* pRenderTarget,
							IDirect3DSurface8* pNewZStencil)
{
	EXECMD_SETRENDERTARGET	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetRenderTarget_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->pRenderTarget = pRenderTarget ;
	cmd->pNewZStencil  = pNewZStencil ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetTexture_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETTEXTURE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD				Stage ;
	LPDIRECT3DTEXTURE8	pTexture ;
} EXECMD_SETTEXTURE ;

static void	DG_SetTexture_Exe(EXECMD_SETTEXTURE *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetTexture( g_pd3dDevice,
									cmd->Stage,
									cmd->pTexture) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetTexture_EB(DWORD Stage, LPDIRECT3DTEXTURE8 pTexture)
{
	EXECMD_SETTEXTURE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetTexture_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Stage		= Stage ;
	cmd->pTexture	= pTexture ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetTextureStageState_EB											*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETTEXTURESTAGESTATE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD						Stage ;
	D3DTEXTURESTAGESTATETYPE	Type ;
	DWORD 						Value ;
} EXECMD_SETTEXTURESTAGESTATE ;

static void	DG_SetTextureStageState_Exe(EXECMD_SETTEXTURESTAGESTATE *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetTextureStageState( g_pd3dDevice,
									cmd->Stage,
									cmd->Type,
									cmd->Value) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetTextureStageState_EB(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	EXECMD_SETTEXTURESTAGESTATE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetTextureStageState_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Stage = Stage ;
	cmd->Type  = Type;
	cmd->Value = Value ;
}

/*----------------------------------------------------------------------*/
/*	DG_DrawPrimitive_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_DRAWPRIMITIVE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	D3DPRIMITIVETYPE	PrimitiveType ;
	UINT				StartVertex ;
	UINT				PrimitiveCount ;
#if	__EXEBUFFER_CALLFILE_DEBUG__
	char				*filename ;
	DWORD				line ;
#endif
} EXECMD_DRAWPRIMITIVE ;

static void	DG_DrawPrimitive_Exe(EXECMD_DRAWPRIMITIVE *cmd)
{
	HRESULT	hr ;
	hr = IDirect3DDevice8_DrawPrimitive( g_pd3dDevice,
									cmd->PrimitiveType,
									cmd->StartVertex,
									cmd->PrimitiveCount) ;
#if	!__EXEBUFFER_CALLFILE_DEBUG__
	ASSERT( !FAILED(hr) ) ;
#else
	if( FAILED(hr) )
	{
		printf("[ERROR] DG_DrawPrimitive in %s(%d)\n", cmd->filename, cmd->line) ;
		printf("\tShader: %d(%08X)\n", _VS_Handle, _VS_Handle) ;
	}
#endif
}

#if	!__EXEBUFFER_CALLFILE_DEBUG__
void DG_DrawPrimitive_EB(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
						UINT PrimitiveCount)
#else
void _DG_DrawPrimitive_EB(D3DPRIMITIVETYPE PrimitiveType,
								UINT StartVertex, UINT PrimitiveCount,
								char *filename, DWORD line)
#endif
{
	EXECMD_DRAWPRIMITIVE	*cmd ;

	/*-- 連結検査 ------------------------------------------------------*/

#if __MARGE_CMD_DRAWPRIM__
	if(  (PrimitiveType == D3DPT_LINELIST)
	  || (PrimitiveType == D3DPT_TRIANGLELIST) )
	{
		EXECMD_DRAWPRIMITIVE	*pre_cmd ;

		pre_cmd =(void *)_man.buff.pre_cmd ;
		if( pre_cmd )
		{
			if(  (pre_cmd->header.exe_func == DG_DrawPrimitive_Exe)
			  && (pre_cmd->PrimitiveType == PrimitiveType)
			  && ((pre_cmd->PrimitiveCount + PrimitiveCount)
				   < DG_WinApp.d3d_cap.MaxPrimitiveCount) )
			{
				if( PrimitiveType == D3DPT_LINELIST )
				{
					if( StartVertex
							== (pre_cmd->StartVertex + (pre_cmd->PrimitiveCount << 1)) )
					{
						pre_cmd->PrimitiveCount += PrimitiveCount ;
						//printf("D3DPT_LINELIST: Hit!!\n") ;
						return ;
					}
					
				}
				else //if( PrimitiveType == D3DPT_TRIANGLELIST )
				{
					if( StartVertex
							== (pre_cmd->StartVertex + (pre_cmd->PrimitiveCount * 3)) )
					{
						pre_cmd->PrimitiveCount += PrimitiveCount ;
						return ;
					}
				}
			}
		}
	}
#endif
	/*------------------------------------------------------------------*/

	cmd = (void *)DG_AllocCmdExeBuffer(DG_DrawPrimitive_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->PrimitiveType  = PrimitiveType ;
	cmd->StartVertex    = StartVertex ;
	cmd->PrimitiveCount = PrimitiveCount ;
#if	__EXEBUFFER_CALLFILE_DEBUG__
	cmd->filename       = filename ;
	cmd->line           = line ;
#endif
}

/*----------------------------------------------------------------------*/
/*	DG_DrawIndexedPrimitive_EB											*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_DRAWINDEXEDPRIMITIVE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	D3DPRIMITIVETYPE	Type ;
	UINT				MinIndex ;
	UINT				NumVertices ;
	UINT				StartIndex ;
	UINT				PrimitiveCount;
} EXECMD_DRAWINDEXEDPRIMITIVE ;

static void	DG_DrawIndexedPrimitive_Exe(EXECMD_DRAWINDEXEDPRIMITIVE *cmd)
{
	HRESULT	hr ;
	hr = IDirect3DDevice8_DrawIndexedPrimitive( g_pd3dDevice,
									cmd->Type,
									cmd->MinIndex,
									cmd->NumVertices,
									cmd->StartIndex,
									cmd->PrimitiveCount) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_DrawIndexedPrimitive_EB(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices,
							 UINT StartIndex, UINT PrimitiveCount)
{
	EXECMD_DRAWINDEXEDPRIMITIVE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_DrawIndexedPrimitive_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Type           = Type ;
	cmd->MinIndex       = MinIndex ;
	cmd->NumVertices    = NumVertices ;
	cmd->StartIndex     = StartIndex ;
	cmd->PrimitiveCount = PrimitiveCount ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetStreamSource_EB												*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETSTREAMSOURCE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	UINT					StreamNumber ;
	IDirect3DVertexBuffer8	*pStreamData ;
	UINT					Stride ;
} EXECMD_SETSTREAMSOURCE ;

static void	DG_SetStreamSource_Exe(EXECMD_SETSTREAMSOURCE *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetStreamSource( g_pd3dDevice,
									cmd->StreamNumber,
									cmd->pStreamData,
									cmd->Stride) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetStreamSource_EB(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData,
						UINT Stride)
{
	EXECMD_SETSTREAMSOURCE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetStreamSource_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->StreamNumber	= StreamNumber ;
	cmd->pStreamData	= pStreamData ;
	cmd->Stride			= Stride ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetIndices_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETINDICES_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	IDirect3DIndexBuffer8	*pIndexData ;
	UINT					BaseVertexIndex ;
} EXECMD_SETINDICES ;

static void	DG_SetIndices_Exe(EXECMD_SETINDICES *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetIndices( g_pd3dDevice,
									cmd->pIndexData,
									cmd->BaseVertexIndex) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetIndices_EB(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
	EXECMD_SETINDICES	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetIndices_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->pIndexData			= pIndexData ;
	cmd->BaseVertexIndex	= BaseVertexIndex ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetVertexShader_EB												*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETVERTEXSHADER_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD					Handle ;
} EXECMD_SETVERTEXSHADER ;

static void	DG_SetVertexShader_Exe(EXECMD_SETVERTEXSHADER *cmd)
{
	HRESULT	hr ;

#if __EXEBUFFER_CALLFILE_DEBUG__
	_VS_Handle = cmd->Handle;
#endif
	hr = IDirect3DDevice8_SetVertexShader( g_pd3dDevice,
									cmd->Handle) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetVertexShader_EB(DWORD Handle)
{
	EXECMD_SETVERTEXSHADER	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetVertexShader_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Handle	= Handle ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetPixelShader_EB												*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETPIXELSHADER_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD					Handle ;
} EXECMD_SETPIXELSHADER ;

static void	DG_SetPixelShader_Exe(EXECMD_SETPIXELSHADER *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetPixelShader( g_pd3dDevice,
									cmd->Handle) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetPixelShader_EB(DWORD Handle)
{
	EXECMD_SETPIXELSHADER	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetPixelShader_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Handle	= Handle ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetVertexShaderConstant_EB										*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETVERTEXSHADERCONSTANT_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD	Register ;
	DWORD	ConstantCount ;
} EXECMD_SETVERTEXSHADERCONSTANT ;

static void	DG_SetVertexShaderConstant_Exe(EXECMD_SETVERTEXSHADERCONSTANT *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetVertexShaderConstant(g_pd3dDevice,
									cmd->Register,
									(void *)(cmd + 1),
									cmd->ConstantCount) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetVertexShaderConstant_EB(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
	DWORD							size ;
	EXECMD_SETVERTEXSHADERCONSTANT	*cmd ;

	size = sizeof(EXECMD_SETVERTEXSHADERCONSTANT) + (sizeof(FVECTOR) * ConstantCount) ;
	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetVertexShaderConstant_Exe, size) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Register		= Register ;
	cmd->ConstantCount	= ConstantCount ;
	memcpy((void *)(cmd + 1), pConstantData, sizeof(FVECTOR) * ConstantCount) ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetPixelShaderConstant_EB										*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETPIXELSHADERCONSTANT_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD	Register ;
	DWORD	ConstantCount ;
} EXECMD_SETPIXELSHADERCONSTANT ;

static void	DG_SetPixelShaderConstant_Exe(EXECMD_SETPIXELSHADERCONSTANT *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetPixelShaderConstant(g_pd3dDevice,
									cmd->Register,
									(void *)(cmd + 1),
									cmd->ConstantCount) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_SetPixelShaderConstant_EB(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
	DWORD							size ;
	EXECMD_SETPIXELSHADERCONSTANT	*cmd ;

	size = sizeof(EXECMD_SETPIXELSHADERCONSTANT) + (sizeof(FVECTOR) * ConstantCount) ;
	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetPixelShaderConstant_Exe, size) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Register		= Register ;
	cmd->ConstantCount	= ConstantCount ;
	memcpy((void *)(cmd + 1), pConstantData, sizeof(FVECTOR) * ConstantCount) ;
}

/*----------------------------------------------------------------------*/
/*	DG_SetViewport_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETVIEWPORT_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	D3DVIEWPORT8			viewport ;
} EXECMD_SETVIEWPORT ;

static void	DG_SetViewport_Exe(EXECMD_SETVIEWPORT *cmd)
{
	HRESULT	hr ;

#if FALSE
	if( DG_CheckUseVertexShader() )
	{
		hr = IDirect3DDevice8_SetViewport(g_pd3dDevice,
										&cmd->viewport) ;
		ASSERT( !FAILED(hr) ) ;
	}
	else
	{
		/* BeginScene～EndSceneの間でViewport変更はまずいらしい.... */
		hr = IDirect3DDevice8_EndScene(g_pd3dDevice) ;
		ASSERT( !FAILED(hr) ) ;

		hr = IDirect3DDevice8_SetViewport(g_pd3dDevice,
										&cmd->viewport) ;
		ASSERT( !FAILED(hr) ) ;

		hr = IDirect3DDevice8_BeginScene(g_pd3dDevice) ;
		ASSERT( !FAILED(hr) ) ;
	}
#else
	/* BeginScene～EndSceneの間でViewport変更はまずいらしい.... */
	hr = IDirect3DDevice8_EndScene(g_pd3dDevice) ;
	ASSERT( !FAILED(hr) ) ;

	hr = IDirect3DDevice8_SetViewport(g_pd3dDevice,
									&cmd->viewport) ;
	ASSERT( !FAILED(hr) ) ;

	hr = IDirect3DDevice8_BeginScene(g_pd3dDevice) ;
	ASSERT( !FAILED(hr) ) ;
#endif
}

void DG_SetViewport_EB(D3DVIEWPORT8 *viewport)
{
	EXECMD_SETVIEWPORT	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetViewport_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->viewport	= *viewport ;
}

/*----------------------------------------------------------------------*/
/*	DG_StoreImage_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_STOREIMAGE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	void	*addr ;
	int		which ;
	int		x ;
	int		y ;
	int		w ;
	int		h ;
	int		mode ;
} EXECMD_STOREIMAGE ;

static void	DG_StoreImage_Exe(EXECMD_STOREIMAGE *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_EndScene(g_pd3dDevice) ;
	ASSERT( !FAILED(hr) ) ;

	DG_StoreImagePacket(cmd->addr,
					cmd->which,
					cmd->x,
					cmd->y,
					cmd->w,
					cmd->h,
					cmd->mode) ;


	hr = IDirect3DDevice8_BeginScene(g_pd3dDevice) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_StoreImage_EB(void *addr, int which, int x, int y, int w, int h, int mode)
{
	EXECMD_STOREIMAGE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_StoreImage_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->addr	= addr ;
	cmd->which	= which ;
	cmd->x		= x ;
	cmd->y		= y ;
	cmd->w		= w ;
	cmd->h		= h ;
	cmd->mode	= mode ;
}

/*----------------------------------------------------------------------*/
/*	DG_ExeBufferCallback												*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_CALLBACK_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	void 	(*func)(DWORD) ;
	DWORD	arg ;
} EXECMD_CALLBACK ;

static void	DG_ExeBufferCallback_Exe(EXECMD_CALLBACK *cmd)
{
	cmd->func(cmd->arg) ;
}

void DG_ExeBufferCallback(void (*func)(DWORD), DWORD arg)
{
	EXECMD_CALLBACK	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_ExeBufferCallback_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->func	= func ;
	cmd->arg	= arg ;
}


/*----------------------------------------------------------------------*/
/*	DG_SetLight_EB														*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_SETLIGHT_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD		Index ;
	D3DLIGHT8	Light ;
} EXECMD_SETLIGHT ;

static void DG_SetLight_Exe(EXECMD_SETLIGHT *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetLight(g_pd3dDevice, cmd->Index, &cmd->Light) ;
#ifdef DEBUG_MODE
	if( FAILED(hr) )
	{
		printf("SetLight:%d\n", cmd->Index) ;
		printf("\tDirection:(%7.3f, %7.3f, %7.3f)\n",
				cmd->Light.Direction.x, cmd->Light.Direction.y, cmd->Light.Direction.z) ;
		printf("\tDiffuse:(%7.3f, %7.3f, %7.3f)\n",
				cmd->Light.Diffuse.r, cmd->Light.Diffuse.g, cmd->Light.Diffuse.b) ;
		ASSERT(0) ;
	}
#else
	ASSERT( !FAILED(hr) ) ;
#endif
}

void DG_SetLight_EB(DWORD Index, D3DLIGHT8 *pLight)
{
	EXECMD_SETLIGHT	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_SetLight_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Index	= Index ;
	cmd->Light	= *pLight ;
}

/*----------------------------------------------------------------------*/
/*	DG_LightEnable_EB													*/
/*----------------------------------------------------------------------*/
typedef	struct	EXECMD_LIGHTENABLE_
{
	DG_EXEBUFF_CMD_HEADER	header ;

	DWORD	Index ;
	BOOL 	bEnable ;
} EXECMD_LIGHTENABLE ;

static void DG_LightEnable_Exe(EXECMD_LIGHTENABLE *cmd)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_LightEnable(g_pd3dDevice, cmd->Index, cmd->bEnable) ;
	ASSERT( !FAILED(hr) ) ;
}

void DG_LightEnable_EB(DWORD Index, BOOL bEnable)
{
	EXECMD_LIGHTENABLE	*cmd ;

	cmd = (void *)DG_AllocCmdExeBuffer(DG_LightEnable_Exe, sizeof(*cmd)) ;
	if( !cmd ){ ASSERT(0) ; return ; }

	cmd->Index		= Index ;
	cmd->bEnable	= bEnable ;
}
