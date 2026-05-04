//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga.c 
   オルガ

   1999/12/18 T.Morita
   $Id: orga.c,v 1.1.1.3 2002/11/19 11:46:20 Yoshizawa1 Exp $
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
#include "include/orga.h"

static void Act( Work *work )
{
    /* メッセージ処理（受け側） */
    ORG_Message( work ) ;    /* orga_msg.c */

    if ( !work->act_stop )
    {
	/* システムに必要なことをすべてする。 */
	ORG_PreAction( work ) ;  /* orga_act.c */

	/* work->flag により(*work->act)()の実行に必要な情報を集める */
	ORG_Recognition( work ) ;/* orga_rcg.c */

	/* 行動（ORG_Thinkルーチンから各行動のルーチンに派生する） */
	if ( !work->act )
	    work->act = ORG_ActionReset ;
	(**work->act)( work ) ;

	/* システムに必要なことを残りのすべてをする。 */
	ORG_PostAction( work ) ;  /* orga_act.c */

	/* 表示用の処理, ライフ値や間接補正などの計算をする */
	ORG_Display( work ) ;    /* orga_dsp.c */
    }
    else
	work->act_stop-- ;

#ifdef DEBUG_MODE
    /* デバッグ用 */
    ORG_Debug( work ) ;
#endif
}

static void Die( Work *work )
{
    /* kill all weapons */
    work->weap_usp.vw = -1.0f ;
    work->weap_ssk    = -1    ;
    work->weap_sgr.vw = -1.0f ;
    work->weap_amo    = -1    ;

    /* kill breast movements */
    if ( work->misc_breast )
	ExitEvmMMOrga( work->misc_breast ) ;

    /* Free radar controller */
    GM_FreeRadarControl( &work->radar ) ;

    /* kill system control */
    GM_FreeControl( &work->control ) ;
    GM_FreeObject( &work->body ) ;
    GM_RemoveGageSet( &work->gage ) ;

    GM_FreeTarget( work->target ) ;

    if ( GM_GameLevel < GM_LEVEL_EXTREME )
	GM_FreeHomingTrg( &work->homing ) ;
}

static int GetResources( Work *work, int name, int where )
{
    ORG_InitControl( work, name, where ) ;
    ORG_InitObject( work ) ;
    ORG_InitParam( work, name ) ;
    ORG_GetOptionValue( work, name ) ;
    ORG_InitPartAndTarget( work, where ) ;
    ORG_InitRadar( work ) ;
    ORG_InitVitalityGage( work ) ;

    ORG_InitMessages( work ) ;

    ORG_Recognition( work ) ;/* 思考の初期化のため */

    return 0 ;
}

void *NewOrga( int name, int where )
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
