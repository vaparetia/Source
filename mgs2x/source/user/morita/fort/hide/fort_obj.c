//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_obj.c 
   フォーチュン戦 ライデン隠れ専用プットオブジェ

   2000/12/14 T.Morita
   $Id: fort_obj.c,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"

#define __MAIN_FILE__
#include "fort_obj.h"

void FRT_OBJ_FreeHzxHzd( HIDE *h, int target )
{
    int i ;

    for ( i=3 ; --i>=0 ; )
	if ( h->hzd[i] )
	{
	    BRK_FreeHazard( h->hzd[i] ) ;
	    h->hzd[i] = NULL ;
	}
    /*フロアとセグメントの解放*/
    FRT_OBJ_FreeHzxSegAndFloor( h ) ;

    /*狙いシステムの解放*/
    FRT_AIM_RemoveAimSpot( h ) ;

    if ( target )
    {
	if ( h->shadow )
	{
	    GV_DestroyOtherActor( h->shadow ) ;
	    h->shadow = NULL ;
	}
	/*ターゲットの解放*/
	GM_FreeTarget( &h->target ) ;
    }
}

void FRT_OBJ_FreeObject( HIDE *h, int target )
{
    if ( h->work )
	GV_CallChildSignalFunc( h->work, FRT_OBJ_FreeObjs, (int)h ) ;
    if ( h->objs )
    {

#if MAKING
	if ( h->wireframe )
	    GV_DestroyOtherActor( h->wireframe ), h->wireframe = NULL ;
#endif

	DG_DequeueObjs( h->objs ) ;
	DG_FreeObjs( h->objs );
	h->objs = NULL ;
    }
    if ( h->shadow )
    {
	GV_DestroyOtherActor( h->shadow ) ;
	h->shadow = NULL ;
    }

    FRT_OBJ_FreeHzxHzd( h, target ) ;
}

static void Die( Work *work )
{
    int i ;
    HIDE *h = work->hide ;

    for ( i=0 ; i<FRT_MAX_SNAHIDE ; i++, h++ )
    {
	FRT_OBJ_FreeObject( h, 1 ) ;

	if ( i==6 || i==8 )
	    FRT_OBJ_TypePool[i] = FRT_TYP_WOODBOX0_BRK ;/* */
	else if ( i==7 || i==5 || i==10 )
	    FRT_OBJ_TypePool[i] = FRT_TYP_IRONBOX1M3_D ;/* */
	else if ( FRT_OBJ_GetTypeMasked(h) == FRT_TYP_IRONBOX1M1_I )
	    FRT_OBJ_TypePool[i] = FRT_TYP_IRONBOX1M0_H ;
	else
	    FRT_OBJ_TypePool[i] = h->type ;/* タイプを保存する */
    }
    FRT_OBJ_Work = NULL ;
}

static void Act( Work *work )
{
    int status ;

    status = FRT_OBJ_ActCheckHide( work ) ;
    FRT_OBJ_SendFortuneHideMessage( work, status ) ;
    work->status = status ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;

    if ( FRT_OBJ_InitHazard( work ) )
	return -1 ;
    if ( FRT_OBJ_GetOptions( work, name, where ) )
	return -1 ;
    FRT_OBJ_Work = work ;

    return 0 ;
}

void *NewFortHideObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) )
	    GV_SetActor( &work->actor, NULL, Die ) ;
	else
	    GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;

        if ( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
	if ( work->flag & 1 )
	    GV_SetActor( &work->actor, NULL, Die ) ;/* やっぱりアクトは要らない */
    }
    return work ;
}
