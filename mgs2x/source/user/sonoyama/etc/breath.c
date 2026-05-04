//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   breath.c
   白息
   
   2000/04/21 M.Sonoyama
   $Id: breath.c,v 1.1.1.3 2002/11/19 11:50:41 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"

extern	void  *NewBreath( FMATRIX  *world, FVECTOR *move, int * ) ;

typedef	struct	{
    GV_ACT		actor ;
	FVECTOR		shift ;

	int			name ;
    OBJECT		*body ;
	int			sw ;

    int			flag ;
	int			objNo ;
	int			count ;
	int			interval ;
	
	float		motion_rate ;
} Work ;

static	void	Act( work )
Work		*work ;
{
    int		n ;
	GV_MSG	*msg ;

	n = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n >= 0 ) {
		if ( msg->message[ 0 ] == 1 ) {
			/* 非表示 */
			work->flag = 0 ;
			work->sw = 0 ;
		} else {
			/* 表示 */
			work->flag = 1 ;
			work->sw = 1 ;
		}
		msg ++ ;
	}
	if ( work->flag == 0 ) return ;
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->sw = 0 ;
		return ;
	} 
	work->sw = 1 ;
	if ( work->body == GM_PlayerBody ) {
		PlayerWork	*pWork ;
		int			act ;
		float		rate ;

		if ( GM_Vitality <= 0 ) {
			work->sw = 0 ;
			return ;
		}

		pWork = GM_PlayerWork ;
		act = pWork->act_name ;
		if ( act == STAND_STILL || 
			 act == SQUAT_STILL ||
			 act == GROUND_STILL ||
			 act == STAND_CAUTION_STILL ||
			 act == SQUAT_CAUTION_STILL ) {
			rate = pWork->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
				/ pWork->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
			if ( work->motion_rate < 0.75F && rate >= 0.75F ) {
				NewBreath( &work->body->objs->objs[ work->objNo ].world, &work->shift, &work->sw ) ;
			}
			work->motion_rate = rate ;
			work->count = 0 ;
		} else {
			if ( ++ work->count == work->interval ) {
				NewBreath( &work->body->objs->objs[ work->objNo ].world, &work->shift, &work->sw ) ;
				work->count = 0 ;
			}
			work->motion_rate = 0.0F ;
		}
	} 
#if 0
	if ( ++ work->count == work->interval ) {
		NewBreath( &work->body->objs->objs[ work->objNo ].world, &work->shift, &work->sw ) ;
		work->count = 0 ;
	}
#endif
}

static	int	GetResources( work, name, object )
Work		*work ;
int			name ;
OBJECT		*object ;
{
	IVECTOR		iv ;

	work->name = name ;
    GCL_GetOption( 'o' ) ;
	work->objNo = GCL_GetNextInt() ;
	GCL_GetOption( 's' ) ;
	GCL_GetNextIV( ( int * )&iv ) ;
	GV_IVtoFV( &iv, &work->shift, 3 ) ;
	work->interval = GCL_GetOptionValue( 'i', 0 ) ;
	work->interval = DIRECT_TICK( work->interval ) ;
    work->body = object ;
    work->flag = 1 ; /* 表示 */
	work->count = 0 ;
    return 0 ;
}

void 	*NewCharaBreath( name, where )
int		name, where ;
{
    Work		*work ;
    CONTROL		*ctrl ;
    OBJECT		*object ;

    GCL_GetOption( 'n' ) ;
    ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
    ASSERT( ctrl != NULL ) ;
    object = ctrl->object ;
    ASSERT( object != NULL ) ;
    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) )  ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, NULL ) ;
		if ( GetResources( work, name, object ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}
