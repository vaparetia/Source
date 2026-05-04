//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sam_slashed.c
   蛇手ミサイル斬られた後

   2001/06/11	M.Sonoyama
   $Id: sam_slashed.c,v 1.1.1.3 2002/11/19 11:51:06 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"rand.h"
#include	"sol_common.h"

/*----------------------------------------------------------------*/

extern	void 	*NewRisingSmoke( FVECTOR* pos, float size ) ;
extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;
extern void *NewSolidusMissileBarn( FMATRIX *mat );

/*----------------------------------------------------------------*/

#define	BODY_NAME_R		GV_StrCode( "sol_msl_r" )
#define	BODY_NAME_L		GV_StrCode( "sol_msl_l" )

#define	BODY_FLAG		(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

typedef	struct	{
	GV_ACT_EX			actor ;
	OBJECT				body ;
	FMATRIX				lights[ 2 ] ;
	FVECTOR				pos ;
	FVECTOR				step ;
	SVECTOR				rot ;
	SVECTOR				turn ;
	int					count ;
	int					time ;
	int					flag ;
	int					which ;
	int					smoke_flag ;
} Work ;

enum {
	SAM_RIGHT = 0,
	SAM_LEFT,
} ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	FVECTOR		p_mov ;
	int			dir ;

	DG_COPY_VEC( &p_mov, &work->pos ) ;

	dir = ( work->which == SAM_RIGHT ) ? 1 : -1 ;
	work->turn.vx = work->turn.vx - irnd() % 8 ;
	work->turn.vy = work->turn.vy + irnd() % 8 * dir ;
	if ( GV_DiffDirS( work->turn.vx, 1000 + 1024 ) < 0 ) {
		work->turn.vx = 1000 + 1024 ;
	}
	GV_NearExp4PV( &work->rot, &work->turn, 3 ) ;
	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_MovePos( &work->step ) ;

	GM_ActObject( &work->body ) ;

	GV_MatToVec( &work->body.objs->world, &work->pos ) ;
	
	if ( -- work->count <= 0 ||
		 HZX_OnlineHazardCheck( HZX_AllMapID, &p_mov, &work->pos, HZX_CHK_ALL,
							    0, 0 ) ) {
		NewBlast( &p_mov, NO_SIDE, 100, 200, 0, 0, WP_C4Bomb ) ; /* 壊れ物に反応する場合は変更 */
		work->smoke_flag = 1;
		GV_DestroyActor( work ) ;
	}

	
}

static	void	Die( Work *work )
{
	GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work, FMATRIX *world, FVECTOR *step, int map, int which )
{
	OBJECT		*body ;
	int			model ;

	GM_SetCurrentMap( map ) ;
	body = &work->body ;
	if ( which == SAM_RIGHT ) model = BODY_NAME_R ;
	else					  model = BODY_NAME_L ;
	GM_InitObject( body, model, BODY_FLAG ) ;
	if ( body->objs == NULL ) return -1 ;
	GM_ConfigObjectLight( body, work->lights ) ;
	DG_SetPos( world ) ;
	GM_ActObject( body ) ;

	GV_MatToVec( world, &work->pos ) ;

	GV_SetVec3( &work->step, 0.0F, 96.0F, 0.0F ) ;

	GV_VecToRot( step, &work->rot ) ; 
	work->rot.vx += 1024 ;	/* モデルがＹ上の為 */
	work->turn = work->rot ;

	work->which = which ;

	//GV_SetActorChild( work, NewRisingSmoke( &work->pos, 150.0F ) ) ;
	//GV_SetActorChild( work, NewSolidusMissileFire( &work->pos, &work->step ) ) ;
	/*NewSolidusMissileSmoke( &work->pos, 
				&work->step,
				100.0F, &work->smoke_flag );*/
	
	work->count = 2 * ( 300 / TIME_BASE ) + irnd() % 32 ;
	GV_SetActorChild( work, NewSolidusMissileBarn( &work->body.objs->world ) );

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewSnakeArmMissileSlashed( FMATRIX *world, FVECTOR *step, int map, int which )
{
	Work				*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, world, step, map, which ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

