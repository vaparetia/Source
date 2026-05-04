//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort.c 
   フォーチュン

   1999/12/18 T.Morita
   $Id: fort.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#define __MAIN_FILE__
#include "include/fort.h"

static void Act( Work *work )
{
    /* メッセージ処理（受け側） */
    FRT_Message( work ) ;    /* fort_msg.c */

#ifdef DEBUG_MODE
    if ( FRT_DbgOffense )
#endif
    {
	if ( !work->act_stop )
	{
	    /* システムに必要なことをすべてする。 */
	    FRT_PreAction( work ) ;  /* fort_act.c */

	    /* work->flag により(*work->act)()の実行に必要な情報を集める */
	    FRT_Recognition( work ) ;/* fort_rcg.c */

	    /* 行動（FRT_Think????ルーチンから各行動のルーチンに派生する） */
	    FRT_Action( work ) ; /* fort_act.c */

	    /* システムに必要なことを残りのすべてをする。 */
	    FRT_PostAction( work ) ;  /* fort_act.c */

	    /* 表示用の処理, 間接補正などの計算をする */
	    FRT_Display( work ) ;    /* fort_dsp.c */
	}
	else if ( work->act_stop > 0 )
	    work->act_stop-- ;
    }

#ifdef DEBUG_MODE
    /* デバッグ用 */
    FRT_Debug( work ) ;
#endif
}

static void Die( Work *work )
{
    /* kill all weapons */
    work->weap_lnr.vw = -1.0f ;
    work->weap_amo    = -1    ;

    /* Free radar controller */
    GM_FreeRadarControl( &work->radar ) ;

    /* kill system control */
    GM_FreeControl( &work->control ) ;
    GM_FreeObject( &work->body ) ;

    GM_RemoveGageSet( &work->gage ) ;

    GM_FreeTarget( &work->target ) ;
    GM_FreeHomingTrg( &work->homing ) ;

    FRT_FreeCommandWork() ;
#if DEBUG_MODE
    FRT_FreeDebugWork() ;
#endif
}

static int GetResources( Work *work, int name, int where )
{
    FRT_InitCommandWork( work ) ;
#if DEBUG_MODE
    FRT_InitDebugWork( work ) ;
#endif

    if ( FRT_InitControl( work, name, where ) )
	return -1 ;
    if ( FRT_InitObject( work ) )
	return -1 ;
    if ( FRT_InitParam( work, name ) )
	return -1 ;
    if ( FRT_GetOptionValue( work, name, where ) )
	return -1 ;
    FRT_InitTarget( work, where ) ;
    FRT_InitRadar( work ) ;
    FRT_InitVitalityGage( work ) ;
    FRT_InitHomingTarget( work ) ;
    FRT_LaunchExternalChara( work, name, where ) ;

    FRT_InitMessages( work ) ;

    /* 狙いシステムを初期化 */
    FRT_AIM_InitAimSpot() ;

    return 0 ;
}

void *NewFortune( int name, int where )
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	if ( GetResources( work, name, where ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
