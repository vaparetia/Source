//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_pre.c
  エマ 前処理

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_ene.c,v 1.1.1.3 2002/11/19 11:46:02 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../include/emma.h"

static Work *EMA_Work = NULL ;

void EMA_EneDie()
{
    EMA_Work = NULL ;
}

void EMA_EneBirth( void *ptr )
{
    Work *work = ptr ;

    EMA_Work = work ;
}


/*

コマンド化関数

*/

/* 位置の登録 */
static void EMA_ComMessageExecProc( int proc )
{
    u_int buffer[] = { EMA_M_EXEC_PROC, (u_int)proc } ;
    GV_MSG msg ;

    msg.address = EMA_Work->npc.ctrl->name ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

int EMA_CommandEneAttack( int name )
{
    int i ;
    int proc ;
    Work *work = EMA_Work ;

    if ( work )
	for ( i=EMA_MAX_ENEMY ; --i>=0 ; )
	{
	    if ( work->enemy[i] == name )
		break ;

	    else if ( !work->enemy[i] )
	    {
		/*危険状態プロック*/
		if ( !work->enemy_flg )
		    if ( (proc = work->proc[EMA_P_SNIPER_ATTACK]) )
		    {
			if ( work->proc_time == GV_Time )
			    EMA_ComMessageExecProc( proc ) ;
			else
			{
			    work->proc_time = GV_Time ;

			    GCL_ExecProc( proc, NULL ) ;
			}
			printf( "Emma : I'm in danger!!! Shit!!!\n" ) ;
		    }

		work->enemy[i] = name ;
		work->enemy_flg |= (1 << i) ;

		EMA_SetFlag( EMA_F_IS_ATTACKED ) ;
		return 1 ;
	    }
	}

    return 0 ;
}

int EMA_CommandEneDied( int name )
{
    int   i ;
    int   ret = 0 ;
    Work *work = EMA_Work ;
    int   proc ;

    if ( work )
	for ( i=EMA_MAX_ENEMY ; --i>=0 ; )
	    if ( work->enemy[i] == name )
	    {
		work->enemy[i] = 0 ;
		work->enemy_flg &= ~(1 << i) ;

		ret = 1 ;

		/*危険解除プロック*/
		if ( !work->enemy_flg )
		{
		    EMA_ResetFlag( EMA_F_IS_ATTACKED ) ;
		    if ( (proc = work->proc[EMA_P_SNIPER_NORMAL]) )
		    {
			if ( work->proc_time == GV_Time )
			    EMA_ComMessageExecProc( proc ) ;
			else
			{
			    work->proc_time = GV_Time ;

			    GCL_ExecProc( proc, NULL ) ;
			}
			printf( "Emma : I'm safe. Thanks (^^)\n" ) ;
		    }
		    break ;
		}
	    }

    return ret ;
}
