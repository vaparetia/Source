//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   lodctrl.c
   ＬｏＤ制御
   
   2000/05/10 M.Sonoyama
   $Id: lodctrl.c,v 1.1.1.3 2002/11/19 11:50:43 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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
#include	"camera.h" 

#define	MAX_LOD_MODELS	(4)

typedef	struct _lodctrl_Work {
    GV_ACT_EX	actor ;
	OBJECT		bodys[ MAX_LOD_MODELS ] ;
	int			values[ MAX_LOD_MODELS ] ;
	int			values2[ MAX_LOD_MODELS ] ;
	CONTROL		*ctrl ;
    OBJECT		*body ;
    int			flag ;
	int			type ;
	int			n_models ;
} Work ;

typedef	struct _lodctrl_Work2 {
	GV_ACT		actor ;
	OBJECT		*body ;
	int			*flag ;
} Work2 ;

/*---------------------------------------------------------*/

static	void	_lodctrl_CopyMatrix( DG_OBJS * new, DG_OBJS * org )
{
	int			i ;

	new->group_id = org->group_id ;
	DG_COPY_MAT( &new->world, &org->world ) ;
	new->root = &org->world ;
	for ( i = 0; i < org->n_models; i ++ ) {
		DG_COPY_MAT( &new->objs[ i ].world, &org->objs[ i ].world ) ;
	}
}

static	void	CopyObjectFlag( new, org )
DG_OBJS			*new, *org ;
{
	int			i ;

	for ( i = 0; i < org->n_models; i ++ ) {
		new->objs[ i ].flag = org->objs[ i ].flag ;
	}
}

/*---------------------------------------------------------*/

static	void	Act( work )
Work			*work ;
{
	int				i, flag, *values ;
	int				isplayer, no_visible ;
	float			len ;
	GM_CameraSet	*cam ;

	for ( i = 0; i < work->n_models; i ++ ) {
		DG_InvisibleObjs( work->bodys[ i ].objs ) ;
	}
	isplayer =  ( work->body == GM_PlayerBody ) ? 1 : 0 ;
	if ( isplayer ) GM_ResetPlayerStatusEX( I64(0), PLAYER2_LOD_HIGH ) ;

	no_visible = 0 ;
    work->flag = flag = ( work->body->objs->flag & DG_FLAG_INVISIBLE ) ;
	if ( ( flag & DG_FLAG_INVISIBLE0 ) && ( flag & DG_FLAG_INVISIBLE1 ) ) {
		if ( isplayer )	no_visible = 1 ;	/* LOD_HIGHフラグが必要 */
		else		    return ;
	}

	/* ステルスのときはＬＯＤしない */
	if ( isplayer && GM_CheckPlayerStatus( PLAYER_STEALTH ) ) return ;

	if ( work->type == 0 || work->type == 2 ) {
		values = work->values ;
		for ( i = 0; i < work->n_models; i ++ ) {
			if ( isplayer && GM_CheckPlayerStatus( HZX_PlayerStateMask[ values[ i ] ] ) ) {
				if ( no_visible == 0 ) {
					//DG_VisibleObjs( work->bodys[ i ].objs ) ;
					work->bodys[ i ].objs->flag &= ~DG_FLAG_INVISIBLE ;
					work->bodys[ i ].objs->flag |= flag ;
					DG_InvisibleObjs( work->body->objs ) ;
					_lodctrl_CopyMatrix( work->bodys[ i ].objs, work->body->objs ) ;
					CopyObjectFlag( work->bodys[ i ].objs, work->body->objs ) ;
					PL_ActionBlurObjs = work->bodys[ i ].objs ;
					PL_ActionBlurParent = work ;
				}
				GM_SetPlayerStatusEX( I64(0), PLAYER2_LOD_HIGH ) ;
				return ;				
			}
		}
	} 

	if ( work->type == 0 ) return ;

	values = ( work->type == 1 ) ? work->values : work->values2 ;

	cam = GM_GetCurrentCamera( 0 ) ;
	len = GV_VecLen3F2( &cam->position, &work->ctrl->mov ) ;
	for ( i = 0; i < work->n_models; i ++ ) {
		if ( len < ( float )values[ i ] ) {
			if ( no_visible == 0 ) {
				//DG_VisibleObjs( work->bodys[ i ].objs ) ;
				work->bodys[ i ].objs->flag &= ~DG_FLAG_INVISIBLE ;
				work->bodys[ i ].objs->flag |= flag ;
				DG_InvisibleObjs( work->body->objs ) ;
				_lodctrl_CopyMatrix( work->bodys[ i ].objs, work->body->objs ) ;
				CopyObjectFlag( work->bodys[ i ].objs, work->body->objs ) ;
			}
			if ( isplayer ) {
				PL_ActionBlurObjs = work->bodys[ i ].objs ;
				PL_ActionBlurParent = work ;
				GM_SetPlayerStatusEX( I64(0), PLAYER2_LOD_HIGH ) ;
			}
			return ;
		}
	}
}

static	void	Die( work )
Work			*work ;
{
	int			i ;

	for ( i = 0; i < work->n_models; i ++ ) {
		GM_FreeObject( &work->bodys[ i ] ) ;
	}
}

static	void	Act2( work )
Work2			*work ;
{
    work->body->objs->flag &= ~DG_FLAG_INVISIBLE ;
    work->body->objs->flag |= *( work->flag ) ;
}

/*---------------------------------------------------------*/

static	int	InitObject( body, org, name )
OBJECT		*body, *org ;
int			name ;
{
	GM_InitObject( body, name, ( org->objs->flag & ~DG_FLAG_SHADOWMAKE ) ) ;
	GM_ConfigObjectLight( body, org->objs->light ) ;
	return 0 ;
}

static	int	GetResources( work, object )
Work		*work ;
OBJECT		*object ;
{
    int		*p ;
	OBJECT	*b ;

	b = work->bodys ;
    GCL_GetOption( 'm' ) ;
	while( GCL_NextStr() != NULL ) {
		InitObject( b, object, GCL_GetNextInt() ) ;
		DG_InvisibleObjs( b->objs ) ;
		work->n_models ++ ; b ++ ;
	}
	p = work->values ;
    GCL_GetOption( 'v' ) ;
	while( GCL_NextStr() != NULL ) {
		*( p ++ ) = GCL_GetNextInt() ;
	}
	p = work->values2 ;
    GCL_GetOption( 'A' ) ;
	while( GCL_NextStr() != NULL ) {
		*( p ++ ) = GCL_GetNextInt() ;
	}
	GCL_GetOption( 't' ) ;
	work->type = GCL_GetNextInt() ;
	work->flag = object->objs->flag & DG_FLAG_INVISIBLE ;
    return 0 ;
}

void 	*NewLoDControl( name, where )
int		name, where ;
{
    Work		*work ;
	Work2		*work2 ;
    CONTROL		*ctrl ;
    OBJECT		*object ;

    GCL_GetOption( 'n' ) ;
    ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
    ASSERT( ctrl != NULL ) ;
    object = ctrl->object ;
    ASSERT( object != NULL ) ;
    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, 
								  sizeof( Work ), PLAYER_LODCTRL_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->ctrl = ctrl ;
		work->body = object ;
		if ( GetResources( work, object ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }

    work2 = ( Work2 * )GV_CreateActor( GV_ACTOR_PREV2, GV_CLASS_OBJECT, 
									sizeof( Work2 ), PLAYER_LODCTRL_ACTOR2_PRIO ) ;
    if ( work2 != NULL ) {
		GV_SetActor( &( work2->actor ), Act2, NULL ) ;
		work2->flag = &work->flag ;
		work2->body = object ;
    }	
    return work ;
}
