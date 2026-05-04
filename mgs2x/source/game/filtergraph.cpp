/*--------------------------------------------------------------*/
/*	filtergraph.cpp												*/
/*		フィルタグラグの管理									*/
/*						2002/12/07		Takaki Eiji 			*/
/*--------------------------------------------------------------*/
#define	__FILTERGRAPH_CPP__

#define STRICT
#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>

//#include <d3d8.h>
//#include <d3d8.h>

#include <dshow.h>
#include <qedit.h>
#include <streams.h>

#include "filtergraph.h"

//-----------------------------------------------------------------------------
// File: DShowTextures.h
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//-----------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//	CBufferRenderer	: バッファに出力するフィルタ
	//-------------------------------------------------------------------------
class CBufferRenderer : public CBaseVideoRenderer
{
public:
    CBufferRenderer(LPUNKNOWN pUnk,HRESULT *phr);
    ~CBufferRenderer();

public:
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample
    
	LONG m_lVidWidth;	// Video width
	LONG m_lVidHeight;	// Video Height
	LONG m_lVidPitch;	// Video Pitch
};

static	CBufferRenderer		*CreateBufferRender(void) ;
static	void				ReleaseBufferRender(void) ;
static	HRESULT				ConnectPin(char *filename, CBufferRenderer	*pCBR) ;

#define		g_pGB	(_man.g_pGraph)			// GraphBuilder
#define		g_pMC	(_man.g_pMediaControl)	// Media Control
#define		g_pMP	(_man.g_pMediaPosition)	// Media Postion
#define		g_pME	(_man.g_pEvent)			// Media Event

/*--------------------------------------------------------------*/
/*	define														*/
/*--------------------------------------------------------------*/
#define	SMP_GRABBER_NAME	L"SampleGrabber"

#define	POSITION_SYNC_RANGE_1ST	(10000000/6)	// 初期時間補正幅
#define	POSITION_SYNC_RANGE_MAX	(10000000/2)	// 最大時間補正幅

/*--------------------------------------------------------------*/
/*	macro														*/
/*--------------------------------------------------------------*/
#define	RELEASE(ptr_)	if( (ptr_) ){ (ptr_)->Release() ; (ptr_) = NULL ; }

#define	Msg(str_, hr_)	printf(str_, hr_)

/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
typedef	struct	MANAGER_
{
	BOOL	act ;	// 稼動中

	/*-- フィルタグラフマネージャ ------------------------------*/

	IGraphBuilder	*g_pGraph ;
	IMediaControl	*g_pMediaControl ;
	IMediaPosition	*g_pMediaPosition ;
	IMediaEvent		*g_pEvent ;
	IMediaSeeking	*g_pSeeking ;
	/*----------------------------------------------------------*/

	/*-- バッファレンダ ----------------------------------------*/

	CBufferRenderer	*g_pBufferRender ;
	/*----------------------------------------------------------*/

	/*-- ソースフィルタ ----------------------------------------*/

    IBaseFilter		*pFSrc;          // Source Filter
	/*----------------------------------------------------------*/

	/*-- 状態管理変数 ------------------------------------------*/

	#define		MAX_BUFFER_NUM		2
	#define		MAX_BUFFER_SIZE		((512 * 3) * 448)

	void		*cur_buffer ;
	void		*lock_buffer ;
	BOOL		buffer_rend ;
	DWORD		buffer_sw ;
	DWORD		buffer[MAX_BUFFER_NUM][(MAX_BUFFER_SIZE + (sizeof(int)-1))/sizeof(int)] ;

	BOOL		eom ;						// End Of Movie
	LONGLONG	stop_position ;				// 終了時間
	LONGLONG	position_sync_range ;		// 時間同期幅
	LONGLONG	pre_sync_position ;			// 前回の時間同期位置
	BOOL		pause ;
	/*----------------------------------------------------------*/
} MANAGER ;

/*--------------------------------------------------------------*/
/*	static														*/
/*--------------------------------------------------------------*/

static	MANAGER	_man ;

/*--------------------------------------------------------------*/
/*	GM_InitFilterGraphMan	管理初期化							*/
/*--------------------------------------------------------------*/
void	GM_InitFilterGraphMan(void)
{
	IGraphBuilder	*g_pGraph ;
	IMediaControl	*g_pMediaControl ;
	IMediaPosition	*g_pMediaPosition ;
	IMediaEvent		*g_pEvent ;
	IMediaSeeking	*g_pSeeking ;

	printf("GM_InitFilterGraphMan()\n") ;

	/*-- 稼動中判定 --------------------------------------------*/

	if( _man.act ){ return ; }	// 既に稼動中
	/*----------------------------------------------------------*/

	/*-- 初期化 ------------------------------------------------*/

	_man.act = TRUE ;	// 稼動中フラグOn
	_man.eom = FALSE ;	// 再生終了フラグOff
	/*----------------------------------------------------------*/

	/*-- フィルタグラフマネージャ準備 --------------------------*/

	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
					IID_IGraphBuilder, (void **)&g_pGraph) ;

	g_pGraph->QueryInterface(IID_IMediaControl, (void **)&g_pMediaControl) ;
	g_pGraph->QueryInterface(IID_IMediaPosition,(void **)&g_pMediaPosition) ;
	g_pGraph->QueryInterface(IID_IMediaEvent,   (void **)&g_pEvent) ;
	g_pGraph->QueryInterface(IID_IMediaSeeking,	(void **)&g_pSeeking) ;
	/*----------------------------------------------------------*/

	_man.g_pGraph         = g_pGraph ;
	_man.g_pMediaControl  = g_pMediaControl ;
	_man.g_pMediaPosition = g_pMediaPosition ;
	_man.g_pEvent         = g_pEvent ;
	_man.g_pSeeking       = g_pSeeking ;

	/*-- バッファレンダ作成 ------------------------------------*/

	CreateBufferRender() ;
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	GM_ReleaseFilterGraphMan	管理終了処理					*/
/*--------------------------------------------------------------*/
void	GM_ReleaseFilterGraphMan(void)
{
	printf("GM_ReleaseFilterGraphMan()\n") ;

	if( !_man.act ){ return ; }

	if( _man.g_pMediaControl ){ _man.g_pMediaControl->Stop() ; }

	/*-- フィルタ解放 ------------------------------------------*/

	RELEASE(_man.pFSrc) ;	// Source Filter
	/*----------------------------------------------------------*/

	/*-- フィルタグラフマネージャ解放 --------------------------*/

	RELEASE(_man.g_pSeeking) ;
	RELEASE(_man.g_pEvent) ;
	RELEASE(_man.g_pMediaPosition) ;
	RELEASE(_man.g_pMediaControl) ;
	RELEASE(_man.g_pGraph) ;
	/*----------------------------------------------------------*/

	/*-- バッファレンダ解放 ------------------------------------*/

	ReleaseBufferRender() ;
	/*----------------------------------------------------------*/

	_man.act = FALSE ;	// 稼動中フラグOff
}

/*--------------------------------------------------------------*/
/*	GM_CreateFilterGraph	フィルタグラフ作成					*/
/*--------------------------------------------------------------*/
HRESULT	GM_CreateFilterGraph(char *filename)
{
	HRESULT	hr ;

	printf("GM_CreateFilterGraph(%s)\n", filename) ;

	if( !_man.act ){ return(E_FAIL) ; }		// 稼動していません

	ZeroMemory(_man.buffer, sizeof(_man.buffer)) ;	// 前画像クリア

	hr = ConnectPin(filename, _man.g_pBufferRender) ;
	if( FAILED(hr) )
	{
#ifdef DEBUG_MODE
		ASSERT(0) ;
#endif
		return(hr) ;
	}

	_man.eom = FALSE ;	// 再生終了フラグOff

	_man.buffer_sw   = 0 ;
	_man.cur_buffer  = _man.buffer[_man.buffer_sw] ;
	_man.lock_buffer = NULL ;
	_man.buffer_rend = FALSE ;
	_man.pause       = FALSE ;

	_man.g_pMediaControl->Run() ;	// 実行開始

	_man.g_pSeeking->GetStopPosition(&_man.stop_position) ;	// 終了時間位置取得
	_man.position_sync_range = POSITION_SYNC_RANGE_1ST ;	// 初期時間同期幅
	_man.pre_sync_position   = 0 ;							// 前回の時間同期位置

	return(S_OK) ;
}

/*--------------------------------------------------------------*/
/*	GM_ReleaseFilterGraph	フィルタグラフ解放処理				*/
/*--------------------------------------------------------------*/
HRESULT	GM_ReleaseFilterGraph(void)
{
	printf("GM_ReleaseFilterGraph()\n") ;

	if( !_man.act ){ return(E_FAIL) ; }		// 稼動していません

	if( _man.g_pMediaControl ){ _man.g_pMediaControl->Stop() ; }

	/*-- フィルタ解放 ------------------------------------------*/

	RELEASE(_man.pFSrc) ;	// Source Filter
	/*----------------------------------------------------------*/

	return(S_OK) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphPauseOff	再生再開							*/
/*--------------------------------------------------------------*/
HRESULT	GM_FilterGraphPauseOff(void)
{
	if( !_man.act ){ return(E_FAIL) ; }		// 稼動していません

	if( !_man.pause ){ return(S_OK) ; }
	_man.pause = FALSE ;

	printf("GM_FilterGraphPauseOff()\n") ;
	return( _man.g_pMediaControl->Run() ) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphPauseOn	再生一時停止						*/
/*--------------------------------------------------------------*/
HRESULT	GM_FilterGraphPauseOn(void)
{
	if( !_man.act ){ return(E_FAIL) ; }		// 稼動していません

	if( _man.pause ){ return(S_OK) ; }
	_man.pause = TRUE ;

	printf("GM_FilterGraphPauseOn()\n") ;
	return( _man.g_pMediaControl->Pause() ) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphGetPauseState	一時停止状態取得				*/
/*--------------------------------------------------------------*/
BOOL	GM_FilterGraphGetPauseState(void)
{
	return(_man.pause) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphSyncTime		時間同期						*/
/*--------------------------------------------------------------*/
HRESULT	GM_FilterGraphSyncTime(int tick)
{
	LONGLONG	cur_pos ;
	LONGLONG	sync_pos ;
	LONGLONG	sync_pos2 ;
	LONGLONG	position_sync_range ;

	if( !_man.act ){ return(E_FAIL) ; }		// 稼動していません

	/*-- 同期時間計算 ------------------------------------------*/

	sync_pos = ((LONGLONG)tick * 100000)/3 ;

	if( _man.pre_sync_position == sync_pos ){ return(S_OK) ; }	// 前回と同じ同期位置
	/*----------------------------------------------------------*/

	/*-- 終端位置以降を指定されたら停止 ------------------------*/

	if( _man.stop_position < sync_pos )
	{
		_man.g_pMediaControl->Stop() ;	// 再生停止
		_man.eom = TRUE ;				// 終端判定ＯＮ
		return(S_OK) ;
	}
	/*----------------------------------------------------------*/

	/*-- 現在の再生時間位置を取得 ------------------------------*/

	_man.g_pSeeking->GetCurrentPosition(&cur_pos) ;
	/*----------------------------------------------------------*/

	/*-- 再生位置補正 ------------------------------------------*/

	position_sync_range = _man.position_sync_range ;
#if TRUE
//@03/01/20(月)@　試しに一枚目だけはスキップしない様に修正
	if( ( cur_pos != 0								  )
	  &&(((cur_pos + position_sync_range) < sync_pos)
	  || ((sync_pos + position_sync_range) < cur_pos)))
#else
	if( ((cur_pos + position_sync_range) < sync_pos)
	  || ((sync_pos + position_sync_range) < cur_pos) )
#endif
	{
#ifdef DEBUG_MODE
		printf("GetPositions(%ld)\n", cur_pos) ;
		printf("SetPositions(%ld)\n", sync_pos) ;
		printf("\tRange:%ld\n", position_sync_range) ;
#endif

		/*-- 開始位置再設定 ------------------------------------*/

		sync_pos2 = sync_pos + POSITION_SYNC_RANGE_1ST;	// シークが終わったころには同期ポイントが先に行っているので
														// ちょっと先にシークしておく 
		_man.g_pSeeking->SetPositions(&sync_pos2,
									AM_SEEKING_AbsolutePositioning|AM_SEEKING_SeekToKeyFrame,
									NULL,
									AM_SEEKING_NoPositioning) ;
		/*------------------------------------------------------*/

		/*-- 時間補正幅拡大(あまり補正が重なるとイヤなので) ----*/

		position_sync_range <<= 1 ;
		if( position_sync_range > POSITION_SYNC_RANGE_MAX )
		{
			position_sync_range = POSITION_SYNC_RANGE_MAX ;
		}
		_man.position_sync_range = position_sync_range ;
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/

	_man.pre_sync_position = sync_pos ;

	return(S_OK) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphGetCurrentBuffer	バッファを取得する			*/
/*--------------------------------------------------------------*/
void	*GM_FilterGraphGetCurrentBuffer(void)
{
	void	*buffer ;

	if( !_man.act         ){ return(NULL) ; }
	if( _man.lock_buffer  ){ return(_man.lock_buffer) ; }
	if( !_man.buffer_rend ){ return(NULL) ; }

	buffer = _man.cur_buffer ;

	/*-- 使用バッファの切り替え --------------------------------*/

	if( _man.buffer_sw ){ _man.buffer_sw-- ; }
	else{ _man.buffer_sw = MAX_BUFFER_NUM - 1 ; }

	_man.cur_buffer  = _man.buffer[_man.buffer_sw] ;
	_man.buffer_rend = FALSE ;
	/*----------------------------------------------------------*/

	_man.lock_buffer = buffer ;
	return(buffer) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphGetCurrentBufferInfo							*/
/*--------------------------------------------------------------*/
BOOL	GM_FilterGraphGetCurrentBufferInfo(GM_FG_BUFFER_INFO *info)
{
	CBufferRenderer	*pCBR ;

	if( !_man.act ){ return(FALSE) ; }

	pCBR = _man.g_pBufferRender ;

	if( !pCBR ){ return(FALSE) ; }

	info->width  = pCBR->m_lVidWidth;	// Video width
	info->height = pCBR->m_lVidHeight;	// Video Height

	return(TRUE) ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphUnLockBuffer	取得中バッファを解放			*/
/*--------------------------------------------------------------*/
void	GM_FilterGraphUnLockBuffer(void)
{
	_man.lock_buffer = NULL ;
}

/*--------------------------------------------------------------*/
/*	GM_FilterGraphIsEnd		再生終了判定						*/
/*--------------------------------------------------------------*/
BOOL	GM_FilterGraphIsEnd(void)
{
    long lEventCode;
    long lParam1;
	long lParam2;
	BOOL	eom ;	// End Of Movie
	HRESULT	hr ;

	if( !_man.act ){ return(TRUE) ; }
	if( _man.eom ){ return(TRUE) ; }	// 既に終了判定済み
	eom = FALSE ;

    // Check for completion events
    hr = g_pME->GetEvent(&lEventCode, (LONG_PTR *) &lParam1, (LONG_PTR *) &lParam2, 0);

    if (SUCCEEDED(hr))
    {
        if (EC_COMPLETE == lEventCode) 
        {
            hr = g_pMP->put_CurrentPosition(0);
			eom = TRUE ;
        }

        // Free any memory associated with this event
        hr = g_pME->FreeEventParams(lEventCode, lParam1, lParam2);
    }

	_man.eom = eom ;	// 判定結果保存
	return(eom) ;
}

/*--------------------------------------------------------------*/
/*	CreateBufferRender	BufferRenderフィルタの登録				*/
/*--------------------------------------------------------------*/
static	CBufferRenderer		*CreateBufferRender(void)
{
    IBaseFilter		*pFBR ;	// Buffer Renderer Filter
    CBufferRenderer	*pCBR ;
	HRESULT			hr ;

    // オブジェクト作成
	pCBR = new CBufferRenderer(NULL, &hr) ;
	if( !pCBR || FAILED(hr) )
	{
		/* メモリ不足？ */
		return(NULL) ;
	}
	_man.g_pBufferRender = pCBR ;

	// フィルタ登録
    pFBR = pCBR;
	if( FAILED(hr = g_pGB->AddFilter(pFBR, L"RENDER")) )
	{
		/* 登録出来ませんでした */
    	return(NULL) ;
	}

	// 正常終了
	return(pCBR) ;
}

/*--------------------------------------------------------------*/
/*	ReleaseBufferRender	BufferRenderフィルタ終了処理			*/
/*--------------------------------------------------------------*/
static	void	ReleaseBufferRender(void)
{
}

/*--------------------------------------------------------------*/
/*	ConnectPin													*/
/*--------------------------------------------------------------*/
static	HRESULT	ConnectPin(char *filename, CBufferRenderer	*pCBR)
{
	HRESULT			hr ;
    TCHAR			strFileName[MAX_PATH];
    WCHAR			wFileName[MAX_PATH];
    IBaseFilter		*pFBR ;			// Buffer Renderer Filter
    IPin			*pFBRPinIn;      // Texture Renderer Input Pin
    IBaseFilter		*pFSrc;          // Source Filter
    IPin			*pFSrcPinOut;    // Source Filter Output Pin   

	pFBR = pCBR ;

    // Determine the file to load based on DirectX Media path (from SDK)
    // Use a helper function included in DXUtils.cpp
#if FALSE
	lstrcpy( strFileName, DXUtil_GetDXSDKMediaPath() );
#else
	lstrcpy( strFileName, "" );
#endif
	lstrcat(strFileName, filename) ;

	#ifndef UNICODE
	MultiByteToWideChar(CP_ACP, 0, strFileName, -1, wFileName, MAX_PATH);
    #else
	lstrcpy(wFileName, strFileName);
	#endif

	// Add the source filter
	hr = g_pGB->AddSourceFilter(wFileName, L"SOURCE", &pFSrc) ;
	_man.pFSrc = pFSrc ;	// Source Filter

	if (FAILED(hr))
	{
		Msg(TEXT("Could not create source filter to graph!  hr=0x%x"), hr);
		return hr;
	}

	// Find the source's output pin and the renderer's input pin
	if (FAILED(hr = pFBR->FindPin(L"In", &pFBRPinIn)))
	{
		Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
		return hr;
	}

	if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut)))
	{
		Msg(TEXT("Could not find output pin!  hr=0x%x"), hr);
		return hr;
	}
    
	// Connect these two filters
	if (FAILED(hr = g_pGB->Connect(pFSrcPinOut, pFBRPinIn)))
	{
		Msg(TEXT("Could not connect pins!  hr=0x%x"), hr);
		return hr;
	}

	pFSrcPinOut->Release() ;
	pFBRPinIn->Release() ;

	/* 正常終了 */
	return(S_OK) ;
}

//-----------------------------------------------------------------------------
// File: DShowTextures.cpp
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
//#include "Textures.h"
//#include "DShowTextures.h"
//#include "DXUtil.h"

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------
#define SOURCE_FILE     TEXT("skiing.avi")

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
//#define REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Global DirectShow pointers
//-----------------------------------------------------------------------------
//D3DFORMAT               g_TextureFormat; // Texture format

//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT InitDShowTextureRenderer(void)
{
    HRESULT hr = S_OK;
    IBaseFilter	*pFTR;           // Texture Renderer Filter
    IPin		*pFTRPinIn;      // Texture Renderer Input Pin
    IBaseFilter	*pFSrc;          // Source Filter
    IPin		*pFSrcPinOut;    // Source Filter Output Pin   
    CBufferRenderer	*pCTR;          // DShow Texture renderer
    
#ifdef REGISTER_FILTERGRAPH
    // Register the graph in the Running Object Table (for debug purposes)
    AddToROT(g_pGB);
#endif
    
    // Create the Texture Renderer object
    pCTR = new CBufferRenderer(NULL, &hr);
    if (FAILED(hr))                                      
    {
        Msg(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
        return E_FAIL;
    }
    
    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    pFTR = pCTR;
    if (FAILED(hr = g_pGB->AddFilter(pFTR, L"TEXTURERENDERER")))
    {
        Msg(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
        return hr;
    }
    
    // Determine the file to load based on DirectX Media path (from SDK)
    // Use a helper function included in DXUtils.cpp
    TCHAR strFileName[MAX_PATH];
    WCHAR wFileName[MAX_PATH];
#if FALSE
    lstrcpy( strFileName, DXUtil_GetDXSDKMediaPath() );
#else
    lstrcpy( strFileName, "" );
#endif
    lstrcat( strFileName, SOURCE_FILE );

    #ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0, strFileName, -1, wFileName, MAX_PATH);
    #else
        lstrcpy(wFileName, strFileName);
    #endif

    // Add the source filter
    if (FAILED(hr = g_pGB->AddSourceFilter (wFileName, L"SOURCE", &pFSrc)))
    {
        Msg(TEXT("Could not create source filter to graph!  hr=0x%x"), hr);
        return hr;
    }

    // Find the source's output pin and the renderer's input pin
    if (FAILED(hr = pFTR->FindPin(L"In", &pFTRPinIn)))
    {
        Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
        return hr;
    }

    if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut)))
    {
        Msg(TEXT("Could not find output pin!  hr=0x%x"), hr);
        return hr;
    }
    
    // Connect these two filters
    if (FAILED(hr = g_pGB->Connect(pFSrcPinOut, pFTRPinIn)))
    {
        Msg(TEXT("Could not connect pins!  hr=0x%x"), hr);
        return hr;
    }
   
#if FALSE
    // Get the graph's media control, event & position interfaces
    g_pGB.QueryInterface(&g_pMC);
    g_pGB.QueryInterface(&g_pMP);
    g_pGB.QueryInterface(&g_pME);
    
    // Start the graph running;
    if (FAILED(hr = g_pMC->Run()))
    {
        Msg(TEXT("Could not run the DirectShow graph!  hr=0x%x"), hr);
        return hr;
    }
#endif
    return S_OK;
}


//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void CheckMovieStatus(void)
{
    long lEventCode;
    long lParam1;
    long lParam2;
    HRESULT hr;

    // Check for completion events
    hr = g_pME->GetEvent(&lEventCode, (LONG_PTR *) &lParam1, (LONG_PTR *) &lParam2, 0);
    if (SUCCEEDED(hr))
    {
        if (EC_COMPLETE == lEventCode) 
        {
            hr = g_pMP->put_CurrentPosition(0);
        }

        // Free any memory associated with this event
        hr = g_pME->FreeEventParams(lEventCode, lParam1, lParam2);
    }
}


//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void CleanupDShow(void)
{
#ifdef REGISTER_FILTERGRAPH
    // Pull graph from Running Object Table (Debug)
    RemoveFromROT();
#endif

}
    

//-----------------------------------------------------------------------------
// CBufferRenderer constructor
//-----------------------------------------------------------------------------
CBufferRenderer::CBufferRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                   : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                   NAME("Buffer Renderer"), pUnk, phr)
{
    // Store and AddRef the texture for our use.
    if( phr ){ *phr = S_OK; }
}


//-----------------------------------------------------------------------------
// CBufferRenderer destructor
//-----------------------------------------------------------------------------
CBufferRenderer::~CBufferRenderer()
{
    // Do nothing
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CBufferRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi;
    
    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }
    
    // Only accept RGB24
    pvi = (VIDEOINFO *)pmt->Format();
    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
    {
        hr = S_OK;
    }
    
    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CBufferRenderer::SetMediaType(const CMediaType *pmt)
{
    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();
    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
    m_lVidPitch = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24

    return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CBufferRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  *pBmpBuffer ;
    
    printf("Render %x\n",(_man.buffer_rend != 0));
    
	if( _man.buffer_rend ){ return(S_OK) ; }	// 処理時間稼ぎ

    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );

	if( (DWORD)pBmpBuffer & (sizeof(int)-1) )	// intのアライン検査
	{
		memcpy( _man.cur_buffer, pBmpBuffer, m_lVidWidth * m_lVidHeight * 3) ;
	}
	else
	{
		int		i  ;
		int		size ;
		int		*dst, *src ;

		/*-- 以下の方が本当に早いのか検査 --*/
		size = m_lVidWidth * m_lVidHeight * 3 ;

		dst = (int *)_man.cur_buffer ;
		src = (int *)pBmpBuffer ;

		for(i=size/sizeof(int); i>0; i--, src++, dst++){ *dst = *src ; }

		size &= (sizeof(int) - 1) ;
		if( size ){ memcpy(dst, src, size) ; }
	}

	_man.buffer_rend = TRUE ;

    return S_OK;
}


#ifdef REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Running Object Table functions: Used to debug. By registering the graph
// in the running object table, GraphEdit is able to connect to the running
// graph. This code should be removed before the application is shipped in
// order to avoid third parties from spying on your graph.
//-----------------------------------------------------------------------------
DWORD dwROTReg = 0xfedcba98;

HRESULT AddToROT(IUnknown *pUnkGraph) 
{
    IMoniker * pmk;
    IRunningObjectTable *pirot;
    if (FAILED(GetRunningObjectTable(0, &pirot))) {
        return E_FAIL;
    }
    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08x  pid %08x", (DWORD_PTR) 0, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
    if (SUCCEEDED(hr)) {
        hr = pirot->Register(0, pUnkGraph, pmk, &dwROTReg);
        pmk->Release();
    }
    pirot->Release();
    return hr;
}

void RemoveFromROT(void)
{
    IRunningObjectTable *pirot;
    if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) {
        pirot->Revoke(dwROTReg);
        pirot->Release();
    }
}

#endif


//-----------------------------------------------------------------------------
// Msg: Display an error message box if needed
//-----------------------------------------------------------------------------
#if FALSE
void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[512];

    va_list pArgs;
    va_start(pArgs, szFormat);
    _vstprintf(szBuffer, szFormat, pArgs);
    va_end(pArgs);

    MessageBox(NULL, szBuffer, TEXT("DirectShow Texture3D Sample"), 
               MB_OK | MB_ICONERROR);
}
#endif
