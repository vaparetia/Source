//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_thk.c
  ヴァンプ思考処理

  2001/03/23 T.Morita
  $Id: vamp_thk.c,v 1.1.1.3 2002/11/19 11:46:35 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "include/vamp.h"


static inline void VMPS_ThinkGameover( Work *work )
{
    if ( work->npc.action.act_end )/* モーションが終ったら呼ぶ */
    {
	/* ゲームエンド プロックがあれば呼ぶ */
	if ( work->proc[VMPS_P_GAMEEND] )
	    GCL_ExecProc( work->proc[VMPS_P_GAMEEND], NULL ) ;
	work->npc.action.pad = HANG_IDLE ;
    }
}

/*----- 思考処理メイン --------------------------------------------------------*/
void VMPS_Think( Work *work )
{
    work->npc.action.pad = work->npc.action.set_pad ;

    /* ゲームオーバー時の処理 */
    if ( VMPS_Flag( VMPS_F_GAMEOVER ) )/* ゲームエンドになったら こっち */
	VMPS_ThinkGameover( work ) ;

    else if ( !GM_IsGameOver() )
    {
	/* 取り敢えずフラグをセット */
	VMPS_ResetSetFlag( VMPS_F_IS_SQUAT, VMPS_F_NONE ) ;

	if ( VMPS_GameOverCheck( work, 0 ) )
	    if ( !(work->npc.action.status & (NPC_ACT_STATUS_DAMAGE| NPC_ACT_STATUS_NPCSYS_DAM)) )
	    {
		switch( work->npc.action.set_pad )
		{
		case HANG_KILL:
		    if ( work->npc.action.act_end )
			work->npc.action.pad = HANG_IDLE ;
		    break ;

		case HANG_TIEUP:
		    if ( work->npc.action.act_end )
		    {
			if ( EMA_CommandGiveDamage( 5 ) == 0 )
			{
			    work->capture.capture->flag |= CAPTURE_BREAK ;
			    work->npc.action.pad = HANG_KILL ;
			}
			else
			    work->npc.action.pad = HANG_IDLE ;
		    }
		    break ;

		case HANG_IDLE:
		    if ( work->time > work->hang_tim )
		    {
			if ( !work->capture.capture )
			    VMPS_CaptureEmma( work ) ;
			work->npc.action.pad = HANG_TIEUP ;
			work->capture.capture->flag |= CAPTURE_HANG ;
		    }
		    break ;

		default:
		    work->npc.action.pad = HANG_IDLE ;
		    break ;
		}
		/* パッドが変わった時点でも time をクリアする */
		if ( work->npc.action.pad != work->npc.action.set_pad )
		    work->time = 0 ;
	    }
    }
    //printf( "Vamp: %d\n", work->npc.action.pad ) ;
}
