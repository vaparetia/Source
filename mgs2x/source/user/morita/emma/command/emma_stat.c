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
  $Id: emma_stat.c,v 1.1.1.3 2002/11/19 11:46:02 Yoshizawa1 Exp $
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

void EMA_StatDie()
{
    EMA_Work = NULL ;
}

void EMA_StatBirth( void *ptr )
{
    Work *work = ptr ;

    EMA_Work = work ;
}


/*

  emmaの状態を取得する

*/
int EMA_CommandGetStatus()
{
    Work *work = EMA_Work ;
    int   state ;

    if ( work == NULL )
        return 0 ;
    state = EMA_F_DO_NOTHING ;
    if ( GM_IsGameOver() )
    {
	if ( EMA_Flag( EMA_F_GAMEOVER ) )
	{
	    if ( work->mar_mtn == MAR(EMA_CAPT,CAP_HANG) )
		state = EMA_F_ATTKED_BY_PLAYER ;
	    else if ( EMA_Flag( EMA_F_HURT_BY_PLY ) )
		state = EMA_F_ATTKED_BY_PLAYER ;
	    else
		state = EMA_F_ATTKED_BY_OTHERS ;
	}
    }
    else
	switch( work->mar_mtn )
	{
	case MAR(EMA_BASE,NPC_SQUAT_IDLE):   case MAR(EMA_BASE,RAI_SQUAT_IDLE):
	case MAR(EMA_BASE,PAL_SQUAT_IDLE):
	    state = EMA_F_I_SAT_DOWN ;
	    break ;

	case MAR(EMA_CAPT,CAP_HANG):
	    state = EMA_F_ATTKED_BY_PLAYER ;
	    break ;

	case MAR(EMA_BASE,DAMG_IDLE):        case MAR(EMA_BASE,DAMG_SQUAT):
	case MAR(EMA_BASE,DAMG_DOWN):        case MAR(EMA_BASE,DAMG_WAKE):
	case MAR(EMA_BASE,DAMG_FALL):        case MAR(EMA_BASE,DAMG_SLEEP):
	case MAR(EMA_BASE,DAMG_DOWN_OVER):   case MAR(EMA_BASE,DAMG_OVER):
	case MAR(EMA_BASE,DAMG_TUMBLE):      case MAR(EMA_BASE,DAMG_DROPOFF):
	    if ( EMA_Flag( EMA_F_HURT_BY_PLY ) )
		state = EMA_F_ATTKED_BY_PLAYER ;
	    else
		state = EMA_F_ATTKED_BY_OTHERS ;
	    break ;

	case MAR(EMA_BASE,PAL_DISLIKE):
	case MAR(EMA_BASE,PAL_SCREEM):
	    state = EMA_F_I_DISLIKE_WORMS ;
	    break ;

	case MAR(EMA_CAPT,CAP_THROW):        case MAR(EMA_CAPT,CAP_IDLE):
	case MAR(EMA_CAPT,CAP_WALK):         case MAR(EMA_CAPT,CAP_RELEASE):
	case MAR(EMA_CAPT,CAP_RELEASE_DOWN): case MAR(EMA_CAPT,CAP_RESIST):
	case MAR(EMA_CAPT,CAP_TIE):          case MAR(EMA_CAPT,CAP_ESCAPE):
	    state = EMA_F_I_AM_CAPTURED ;
	    break ;

	default:
	    if ( work->npc.action.faint_count > 0 )
	    {
		if ( work->npc.action.faint_mode )
		    state = EMA_F_I_AM_SLEEPING ;
		else
		    state = EMA_F_I_AM_FAINTED  ;
	    }
	    else if ( EMA_Flag( EMA_F_RUN_AWAY ) )
		state = EMA_F_I_DISLIKE_WORMS ;
 	    break ;
	}

    return state ;
}

int NewEMA_CommandGetState()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, EMA_CommandGetStatus() ) ;

    return 1 ;
}
