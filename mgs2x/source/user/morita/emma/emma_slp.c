//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_thk.c
  NPCサンプル思考処理

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_slp.c,v 1.1.1.3 2002/11/19 11:46:00 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"



static inline void AbortStopStill( Work *work )
{
    if ( EMA_Flag( EMA_F_NON_MOVE ) )
    {
	printf( "Emma : %d\n", work->route->flag ) ;
	if ( work->route->flag==5 || work->route->flag==11 )
	{
	    printf( "tasareru\n",
		    !GM_CheckMenuStatus( MENU_STREAM_CH_0 ),
		    !GM_CheckMenuStatus( MENU_STREAM_CH_1 ) ) ;
	    if ( !GM_CheckMenuStatus( MENU_STREAM_CH_0 ) &&
		 !GM_CheckMenuStatus( MENU_STREAM_CH_1 ) )
	    {
		printf( "Emma : AbortCount\n" ) ;
		work->stop_time++ ;
	    }
	}

	if ( (work->route->flag==5  &&  work->stop_time > 180*2*5/TIME_BASE) ||
	     (work->route->flag==11 &&  work->stop_time > 240*2*5/TIME_BASE) )
	{
	    printf( "Emma : Abort Stopping Emma\n" ) ;
	    EMA_ResetFlag( EMA_F_NON_MOVE ) ;
	}
    }
    else
	work->stop_time = 0 ;
}






/*----- 停止処理メイン --------------------------------------------------------*/
void EMA_Sleep( Work *work )
{
    /* ゲームオーバーじゃなければ 認知する */
    if ( !GM_IsGameOver() )
	if ( EMA_Flag( EMA_F_EVENT_SNIPE ) )
	{
	    AbortStopStill( work ) ;
	}
}
