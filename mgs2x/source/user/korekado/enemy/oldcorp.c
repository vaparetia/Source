//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	oldcorp.c
	腐った死体

	1999/09/13 Y.Korekado
	$Id: oldcorp.c,v 1.3 2002/11/23 12:46:54 Yoshizawa1 Exp $

*/
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"enemy.h"

/*----------------------------------------------------------------*/
//#define NO_DELETE (1)	//死体消さない
/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	int			count ;
#ifdef KP_WINDOWS
	int			blink_count ;
#endif
	FVECTOR		pos ;
	SVECTOR		rot ;

	OLDCORP		oldcorp ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
#ifndef NO_DELETE
#if 0
	FVECTOR shift ;
	shift = DG_ZeroVector ;
	if( work->count & 0x1 ) {
		DG_SetPos2( &work->pos, &work->rot ) ;
		shift.vx = 114.0f * (float)work->count ;
		shift.vy = 114.0f * (float)work->count ;
		shift.vz = 114.0f * (float)work->count ;
		shift.vy = 1140.0f ;
		DG_MovePos( &shift ) ;
		DG_PutObjs( work->body.objs ) ;
//		DG_InvisibleObjs( work->body.objs );
	} else {
		DG_SetPos2( &work->pos, &work->rot ) ;
		shift.vx = -114.0f * (float)work->count ;
		shift.vy = -114.0f * (float)work->count ;
		shift.vz = -114.0f * (float)work->count ;
		shift.vy = -1140.0f ;
		DG_MovePos( &shift ) ;
		DG_PutObjs( work->body.objs ) ;
//		DG_VisibleObjs( work->body.objs );
	}
#else
#ifndef KP_WINDOWS
	if( work->count & 0x1 ) {
		DG_InvisibleObjs( work->body.objs );
	} else {
		DG_VisibleObjs( work->body.objs );
	}

#else
	if( !DG_CurrentFrameDrawSkip() ){ work->blink_count++ ; }	// Frame Skip考慮
	if( work->blink_count & 0x1 ) {
		DG_InvisibleObjs( work->body.objs );
	} else {
		DG_VisibleObjs( work->body.objs );
	}
#endif
#endif

	if( work->count > COUNT_VMODE(0x10 * 2) )  {
		GV_DestroyActor( work ) ;
	}

	work->count ++ ;
#else
	if( work->body.objs->bound_mode & 2 ) {
		GV_DestroyActor( work ) ;
	}
#endif
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &work->body ) ;
    CP_FreeOldCorp( &work->oldcorp ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, model, ctrl, body, light, wp )
Work	*work ;
int		model ;
CONTROL	*ctrl ;
OBJECT	*body ;
FMATRIX	*light ;
int		wp ;
{
	work->pos = ctrl->mov ;
	work->rot = ctrl->rot ;

	GM_InitObject( &work->body, model,  BODY_FLAG );

    GM_GroupObjs( work->body.objs, body->map_name  ) ;
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_PutObjs( work->body.objs ) ;
	_CopyObjectWorld( body->objs, work->body.objs ) ;

	work->lights[0] = *light ;
	light ++ ;
	work->lights[1] = *light ;
	DG_SetLightMatrix( work->body.objs, work->lights );

	CP_SetOldCorp( &work->oldcorp, &ctrl->mov ) ;
	CP_PutOldCorp( &work->oldcorp ) ;

if ( model == GV_StrCode("gbs_def") ) {
extern void *NewEneEquip( int name, void *parent, OBJECT *body, int flag, int wpname, int left_wp ) ;
/*
#define	EQUIP_HLST		0x0001
#define	EQUIP_KNIF		0x0002
#define	EQUIP_KNIFCASE	0x0004
#define	EQUIP_MAG		0x0008
#define	EQUIP_BACKPACK	0x0010
#define	EQUIP_RADIO		0x0020
#define	EQUIP_NSIGHT	0x0040
#define	EQUIP_WEAPON	0x0080
#define	EQUIP_WPFREE	0x0100
*/

	if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
		GV_SetActorChild( work, NewEneEquip( 0, work, &work->body, 0x13b, wp, 0 ) ) ;
	}
}

	work->count = 0 ;
#ifdef KP_WINDOWS
	work->blink_count = 0 ;
#endif
	return 0 ;
}

void		*NewOldCorp( model, ctrl, body, light, wp )
int			model ;
CONTROL		*ctrl ;
OBJECT		*body ;
FMATRIX		*light ;
int			wp ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, model, ctrl, body, light, wp ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
