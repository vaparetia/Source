/*--------------------------------------------------------------*/
/*	filtergraph.h												*/
/*		フィルタグラグの管理									*/
/*						2002/12/08		Takaki Eiji 			*/
/*--------------------------------------------------------------*/
#ifndef	__FILTERGRAPH_H__
#define	__FILTERGRAPH_H__


#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
typedef	struct	GM_FG_BUFFER_INFO_
{
	DWORD	width ;
	DWORD	height ;
} GM_FG_BUFFER_INFO ;

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/
extern	void	GM_InitFilterGraphMan(void) ;
extern	void	GM_ReleaseFilterGraphMan(void) ;

extern	HRESULT	GM_CreateFilterGraph(char * filename) ;
extern	HRESULT	GM_ReleaseFilterGraph(void) ;
extern	HRESULT	GM_FilterGraphSyncTime(int tick) ;
extern	HRESULT	GM_FilterGraphPauseOn(void) ;
extern	HRESULT	GM_FilterGraphPauseOff(void) ;
extern	BOOL	GM_FilterGraphGetPauseState(void) ;

extern	void	*GM_FilterGraphGetCurrentBuffer(void) ;
extern	BOOL	GM_FilterGraphGetCurrentBufferInfo(GM_FG_BUFFER_INFO *info) ;
extern	void	GM_FilterGraphUnLockBuffer(void) ;

extern	BOOL	GM_FilterGraphIsEnd(void) ;

#ifdef __cplusplus
}
#endif
#endif	/* !defined(__FILTERGRAPH_H__) */
