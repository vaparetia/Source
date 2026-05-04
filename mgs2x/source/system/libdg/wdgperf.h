/*--------------------------------------------------------------*/
/*	wdgperf.h													*/
/*					DG関係パフォーマンス検査					*/
/*--------------------------------------------------------------*/
#ifndef	__WDGPERF_H__
#define	__WDGPERF_H__

#ifdef	__WDGPERF_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if TRUE
#ifdef DEBUG_MODE
#define	__DG_PERF_DEBUG__	(TRUE)
#else
#define	__DG_PERF_DEBUG__	(FALSE)
#endif

#else
#define	__DG_PERF_DEBUG__	(FALSE)
#endif

/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
#define	MAX_DGPERF_LOG_NUM_SHIFT	(5)
#define	MAX_DGPERF_LOG_NUM			(1 << MAX_DGPERF_LOG_NUM_SHIFT)

typedef	struct	DGPerf_
{
	LARGE_INTEGER	time ;
	LARGE_INTEGER	start ;

	DWORD	log_ptr ;
	DWORD	log[MAX_DGPERF_LOG_NUM] ;	// Log(下位4Byteのみで大丈夫でしょう....)
	DWORD	log_sum ;
} DGPerf ;

/*--------------------------------------------------------------*/
/*																*/
/*--------------------------------------------------------------*/
#undef	DGPERF_CTGR_EQU
#define	DGPERF_CTGR_EQU(ctgr_)	DGPERF_CTGR_ ## ctgr_,
enum
{
#include "wdgperf.h"
	MAX_DGPERF_CTGR,
} ;
#undef	DGPERF_CTGR_EQU

/*--------------------------------------------------------------*/
/*	extern														*/
/*--------------------------------------------------------------*/
EXTERN	DGPerf	_dg_perf[MAX_DGPERF_CTGR] ;

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/

extern	void	DG_PerfInit(void) ;
extern	void	DG_PerfReset(int id) ;
extern	void	DG_DrawPerf(void) ;
extern	void	DG_PerfStart(int id) ;
extern	void	DG_PerfEnd(int id) ;

extern	DWORD	DG_CalcSkipFrame(void) ;			// PROCESS_ALLとDRAW_ALLからSkip数計算

extern	BOOL	DG_CheckSkipFrameCancelOK(void) ;	// 今から描画すればFrameSkipを
													// キャンセルできるか検査

#if __DG_PERF_DEBUG__
extern	void	DG_PerfResetDrawDebug(void) ;

#define		DG_PerfStartD(id_)	DG_PerfStart(id_)
#define		DG_PerfEndD(id_)	DG_PerfEnd(id_)

#else
#define		DG_PerfResetDrawDebug()
#define		DG_PerfStartD(id_)
#define		DG_PerfEndD(id_)

#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* !defined(__WDGPERF_H__) */

/*--------------------------------------------------------------*/
/*	カテゴリテーブル											*/
/*--------------------------------------------------------------*/
#ifdef	DGPERF_CTGR_EQU

	/*--			file		--*/
	DGPERF_CTGR_EQU(	PLUGIN			)
	DGPERF_CTGR_EQU(	PLUGIN_END		)
	DGPERF_CTGR_EQU(	FRAME_START		)
	DGPERF_CTGR_EQU(	SCREEN			)
	DGPERF_CTGR_EQU(	CHAIN2			)
	DGPERF_CTGR_EQU(	CHAIN2_LATTER	)
	DGPERF_CTGR_EQU(	CHAIN_EVM		)
	DGPERF_CTGR_EQU(	CHAIN_EVM_LATTER)
	DGPERF_CTGR_EQU(	CHAIN			)
	DGPERF_CTGR_EQU(	SHADOW			)
	DGPERF_CTGR_EQU(	SPOT			)
	DGPERF_CTGR_EQU(	PRIM2			)
	DGPERF_CTGR_EQU(	SORT_CHAIN		)
	DGPERF_CTGR_EQU(	DRAWALL			)
	DGPERF_CTGR_EQU(	PROCESS_ALL		)
	DGPERF_CTGR_EQU(	PRESENT			)

#endif /* defined(DGPERF_CTGR_EQU) */


/*-- End Of File --*/
