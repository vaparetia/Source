/*--------------------------------------------------------------*/
/*	wdgperf.c													*/
/*					DG関係パフォーマンス検査					*/
/*--------------------------------------------------------------*/
#define	__WDGPERF_C__

#include <stdio.h>

#include "xtl.h"
#include "game.h"
#include "libdg.h"
#include "wdgperf.h"

#define	__DG_PERF_USE_ASM__	(FALSE)

#ifndef PAL
#define	FPS	(60)	// 60Hz
#else
#define	FPS	(50)	// 50Hz
#endif
/*----------------------------------------------------------------------*/
/*	カテゴリ名テーブル作成												*/
/*----------------------------------------------------------------------*/
#undef	DGPERF_CTGR_EQU
#define	DGPERF_CTGR_EQU(ctgr_)	#ctgr_,
static	char	*_ctgr_name_tbl[] =
{
#include "wdgperf.h"
	NULL,
} ;
#undef	DGPERF_CTGR_EQU

/*----------------------------------------------------------------------*/
/*	static																*/
/*----------------------------------------------------------------------*/
static LARGE_INTEGER	_time_freq ;


/*----------------------------------------------------------------------*/
/*	DG_PerfInit		パフォーマンス検査初期化							*/
/*----------------------------------------------------------------------*/
void	DG_PerfInit(void)
{
	ZeroMemory(_dg_perf, sizeof(_dg_perf)) ;

	/*-- 時間分解能力取得 ----------------------------------------------*/

	QueryPerformanceFrequency(&_time_freq) ;
	/*------------------------------------------------------------------*/
}

/*----------------------------------------------------------------------*/
/*	DG_PerfReset		パフォーマンス検査リセット						*/
/*----------------------------------------------------------------------*/
void	DG_PerfReset(int id)
{
	DGPerf	*perf ;
	int		log_ptr ;
	DWORD	*cur_log ;

	perf = &_dg_perf[id] ;

	log_ptr = perf->log_ptr ;

	cur_log = &perf->log[log_ptr] ;
	perf->log_sum -= *cur_log ;			// 総加算数から、上書きされる分を消去

	*cur_log = perf->time.LowPart ;
	perf->log_sum += *cur_log ;

	perf->time.QuadPart = 0 ;

	/*-- 次のLogへ... --------------------------------------------------*/

	log_ptr-- ;	// 次のLogへ...
	if( log_ptr < 0 ){ log_ptr = MAX_DGPERF_LOG_NUM - 1 ; }
	perf->log_ptr = log_ptr ;
	/*------------------------------------------------------------------*/
}

/*----------------------------------------------------------------------*/
/*	DG_PerfResetDrawDebug	描画Debug用パフォーマンス検査リセット		*/
/*----------------------------------------------------------------------*/
#if __DG_PERF_DEBUG__
void	DG_PerfResetDrawDebug(void)
{
	DG_PerfReset( DGPERF_CTGR_PLUGIN ) ;
	DG_PerfReset( DGPERF_CTGR_PLUGIN_END ) ;
	DG_PerfReset( DGPERF_CTGR_FRAME_START ) ;
	DG_PerfReset( DGPERF_CTGR_SCREEN ) ;
	DG_PerfReset( DGPERF_CTGR_CHAIN2 ) ;
	DG_PerfReset( DGPERF_CTGR_CHAIN2_LATTER ) ;
	DG_PerfReset( DGPERF_CTGR_CHAIN_EVM ) ;
	DG_PerfReset( DGPERF_CTGR_CHAIN_EVM_LATTER) ;
	DG_PerfReset( DGPERF_CTGR_CHAIN ) ;
	DG_PerfReset( DGPERF_CTGR_SHADOW ) ;
	DG_PerfReset( DGPERF_CTGR_SPOT ) ;
	DG_PerfReset( DGPERF_CTGR_PRIM2 ) ;
	DG_PerfReset( DGPERF_CTGR_SORT_CHAIN ) ;
	DG_PerfReset( DGPERF_CTGR_PRESENT ) ;
}
#endif	// __DG_PERF_DEBUG__

/*----------------------------------------------------------------------*/
/*	DG_DrawPerft		パフォーマンス表示								*/
/*----------------------------------------------------------------------*/
static	DWORD			_draw_cntr = 0 ; ;

void	DG_DrawPerf(void)
{
	int		i ;
	DGPerf	*perf ;
	float	val ;
	float	avval ;
	char	**name_ptr ;
	float	ffreq ;

	/*-- 各Perf内容表示 ------------------------------------------------*/

	DEBUG_Locate( 64, 0, 0 );

	ffreq = (float)_time_freq.QuadPart ;

	name_ptr = _ctgr_name_tbl ;
	perf     = _dg_perf ;
	for(i=MAX_DGPERF_CTGR; i>0; i--, perf++, name_ptr++)
	{
		if( (perf->time.QuadPart > 0) || perf->log_sum )
		{
			/*-- 現在値 ------------------------------------------------*/

			val = ((float)perf->time.QuadPart * (100.0f * (float)FPS)) / ffreq ;
			/*----------------------------------------------------------*/

			/*-- 平均値 ------------------------------------------------*/
			avval = ((float)(perf->log_sum >> MAX_DGPERF_LOG_NUM_SHIFT)
					* (100.0f * (float)FPS)) / ffreq ;
			/*----------------------------------------------------------*/

			DEBUG_Printf( "%24s:%6.2f%%(AVE:%6.2f%(%08X)%)\n",
							*name_ptr, val, avval,
							(perf->log_sum >> MAX_DGPERF_LOG_NUM_SHIFT)) ;
		}
	}
	/*------------------------------------------------------------------*/
}

/*----------------------------------------------------------------------*/
/*	DG_PerfStart		パフォーマンス計測開始							*/
/*----------------------------------------------------------------------*/
void	DG_PerfStart(int id)
{
	DGPerf	*perf ;
#if __DG_PERF_USE_ASM__
	LARGE_INTEGER	time ;
#endif

	perf = &_dg_perf[id] ;
#if __DG_PERF_USE_ASM__
	__asm {
		rdtsc;
		mov time.LowPart,eax;
		mov time.HighPart,edx;
	}
	perf->start = time ;
#else
	QueryPerformanceCounter(&perf->start);
#endif
	
}

/*----------------------------------------------------------------------*/
/*	DG_PerfEnd		パフォーマンス計測終了								*/
/*----------------------------------------------------------------------*/
void	DG_PerfEnd(int id)
{
	DGPerf	*perf ;
	LARGE_INTEGER	time ;

	perf = &_dg_perf[id] ;
#if __DG_PERF_USE_ASM__
	__asm {
		rdtsc;
		mov time.LowPart,eax;
		mov time.HighPart,edx;
	}
#else
	QueryPerformanceCounter(&time);
#endif
	perf->time.QuadPart = perf->time.QuadPart
						+ (time.QuadPart - perf->start.QuadPart) ;
}

/*----------------------------------------------------------------------*/
/*	DG_CalcSkipFrame		PROCESS_ALLとDRAWALLからSkip数計算			*/
/*----------------------------------------------------------------------*/
DWORD	DG_CalcSkipFrame(void)
{
	DGPerf	*proc_perf ;
	DGPerf	*draw_perf ;
	float	coef ;
	DWORD	skip ;
#if __DG_DRAW_IN_EXEBUFFER__
	DWORD	proc_skip ;
#endif
	LARGE_INTEGER	past_time ;

	DG_GetVSyncPastTime(&past_time) ;		// 前回のVSyncからの経過時間取得
	if( past_time.HighPart ){ return(0) ; }	// 経過しすぎ...

	proc_perf = &_dg_perf[DGPERF_CTGR_PROCESS_ALL] ;
	draw_perf = &_dg_perf[DGPERF_CTGR_DRAWALL] ;

#if !__DG_DRAW_IN_EXEBUFFER__
	coef = ((float)_time_freq.LowPart
			* ((float)MAX_DGPERF_LOG_NUM / (float)FPS))
			- (float)proc_perf->log_sum ;
#if FALSE
	if( coef <= 0.0f ){ return(0x7fffffff) ; }	// Skip計算不可能
#else
	if( coef <= 0.0f ){ return(0) ; }			// Skip計算不可能
#endif

	skip = (DWORD)(((float)draw_perf->log_sum
				+ ((float)past_time.LowPart * (float)MAX_DGPERF_LOG_NUM)) / coef) ;

#else

	/*-- 描画時間考慮スキップ ------------------------------------------*/

	coef = ((float)draw_perf->log_sum
		+ ((float)past_time.LowPart * (float)MAX_DGPERF_LOG_NUM)) ;
	coef /= (float)_time_freq.LowPart * ((float)MAX_DGPERF_LOG_NUM / (float)FPS) ;
	skip = (DWORD)coef ;
	/*------------------------------------------------------------------*/

	/*-- Process時間考慮スキップ ---------------------------------------*/
#if FALSE
	if( skip
      && (proc_perf->log_sum > ((_time_freq.LowPart * MAX_DGPERF_LOG_NUM)/FPS)) )
	{
		if( skip < 1 )
		{
			coef = (float)_time_freq.LowPart * ((float)MAX_DGPERF_LOG_NUM / (float)FPS) ;
		}
		else
		{
			coef = ((float)_time_freq.LowPart * (float)MAX_DGPERF_LOG_NUM
					* (float)(skip + 1)) / (float)FPS ;
		}
		coef -= ((float)past_time.LowPart * (float)MAX_DGPERF_LOG_NUM) ;	// 使用可能時間計	算

		coef /= (float)proc_perf->log_sum ;
		proc_skip = (DWORD)coef ;

		if( proc_skip < skip ){ skip = proc_skip ; }
	}
#endif
	/*------------------------------------------------------------------*/
#endif

	return(skip) ;
}

/*----------------------------------------------------------------------*/
/*	DG_CalcSkipFrame		FrameSkipキャンセル判定						*/
/*----------------------------------------------------------------------*/
BOOL	DG_CheckSkipFrameCancelOK(void)
{
#if __DG_DRAW_IN_EXEBUFFER__
	return( FALSE ) ;
#else
	DWORD	next_vsync ;
	DGPerf	*draw_perf ;
	DWORD	draw_time ;

	draw_perf = &_dg_perf[DGPERF_CTGR_DRAWALL] ;
	draw_time = draw_perf->log_sum >> MAX_DGPERF_LOG_NUM_SHIFT ;

	if( draw_time > (_time_freq.LowPart/ FPS) ){ return(FALSE) ; }

	next_vsync = DG_GetNextVSyncTime() ;	// 次のVSYNCまでの時間
	if( !next_vsync ){ return(FALSE) ; }

	if( draw_time > next_vsync )
	{
		return(FALSE) ;	// 今描画するとVSyncをまたいでしまうので不可
	}

	return(TRUE) ;
#endif
}

/*-- End Of File --*/
