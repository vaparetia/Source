//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dbview.c
	デバッグ情報
	
	2000/01/11 Y.Korekado
	$Id: eyeview.c,v 1.1.1.3 2002/11/19 11:44:09 Yoshizawa1 Exp $
*/
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"enemy.h"

#define SIGHT_VIEW	0x00000001

#define		BULLER_VIEW (1)

#define 	DEV_C	1
#define		DIV_Y	2
#define		DIV_X	2
#define		cR	128/DEV_C
#define		cG	64/DEV_C
#define		cB	64/DEV_C

#ifdef BULLER_VIEW
#define		bR	64/DEV_C
#define		bG	64/DEV_C
#define		bB	128/DEV_C
#endif

enum {
	ST2_STATE_NONE,
	ST2_STATE_GLASSES,
	ST2_STATE_CLOSE,
} ;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		*center_world ;
	FVECTOR		base_tri[ (DIV_Y+DIV_X) * 2 * 3 ] ;
	FVECTOR		base_sq[ DIV_Y * DIV_X * 4 ] ;
#ifdef BULLER_VIEW
	FVECTOR		base_sq2[ DIV_Y * DIV_X * 4 ] ;
#endif

	FVECTOR		tri[ (DIV_Y+DIV_X) * 2 * 3 ] ;
	FVECTOR		sq[ DIV_Y * DIV_X * 4 ] ;
#ifdef BULLER_VIEW
	FVECTOR		sq2[ DIV_Y * DIV_X * 4 ] ;
#endif

	int			len,range,upper,under, glasses ;
	int			stat, *status2 ;
	COMMANDER 	*com ;
	int			*sw ;
} Work ;

/*----------------------------------------------------------------*/
static void MakePos( Work *work, FMATRIX *world, int len, int buller, int range, int upper,int under )
{
	SVECTOR	rot[DIV_X+1] ;
	FVECTOR	base_pos[DIV_Y+1] ;
	FVECTOR	pos[DIV_X+1][DIV_Y+1] ;
	int		i, j, r ;

	r = (range*2)/DIV_X ;
	for ( j=0; j<DIV_X+1; j++ ) {
		rot[ j ].vx = 0 ;
		rot[ j ].vy = -range + (j*r) ;
		rot[ j ].vz = 0 ;
	}

	r = (upper - under) / DIV_Y ;
	for ( i=0; i<DIV_Y+1; i++ ) {
		base_pos[i].vx = 0.0F ;
		base_pos[i].vy = len*_RsinF( upper - (r*i) ) ;
		base_pos[i].vz = len*_RcosF( upper - (r*i) ) ;
	}

	for ( j=0; j<DIV_X+1; j++ ) {
		DG_SetPos( &DG_UnitMatrix ) ;
		DG_RotatePos( &rot[j] ) ;
		DG_PutVector( &base_pos[0], &pos[j][0], DIV_Y+1 ) ;
	}

	for ( i=0; i<DIV_Y; i++ ) {
		/* 左側面三角 */
		work->base_tri[ (i*3) ] = DG_ZeroVector ;
		work->base_tri[ (i*3)+1 ] = pos[0][ i ] ;
		work->base_tri[ (i*3)+2 ] = pos[0][ i+1 ] ;

		/* 右側面三角 */
		work->base_tri[ (DIV_Y*3) + (i*3) ] = DG_ZeroVector ;
		work->base_tri[ (DIV_Y*3) + (i*3)+1 ] = pos[ DIV_X ][ i ] ;
		work->base_tri[ (DIV_Y*3) + (i*3)+2 ] = pos[ DIV_X ][ i+1 ] ;
	}
	for ( j=0; j<DIV_X; j++ ) {
		/* 上三角 */
		work->base_tri[ (DIV_Y*2*3) + (j*3) ] = DG_ZeroVector ;
		work->base_tri[ (DIV_Y*2*3) + (j*3)+1 ] = pos[j][ 0 ] ;
		work->base_tri[ (DIV_Y*2*3) + (j*3)+2 ] = pos[j+1][ 0 ] ;
		/* 下三角 */
		work->base_tri[ (DIV_Y*2*3) + (DIV_X*3) + (j*3) ] = DG_ZeroVector ;
		work->base_tri[ (DIV_Y*2*3) + (DIV_X*3) + (j*3)+1 ] = pos[j][ DIV_Y ] ;
		work->base_tri[ (DIV_Y*2*3) + (DIV_X*3) + (j*3)+2 ] = pos[j+1][ DIV_Y ] ;
	}

	for ( j=0; j<DIV_X; j++ ) {
		for ( i=0; i<DIV_Y; i++ ) {
			work->base_sq[ (((j*DIV_Y)+i)*4) ] = pos[ j ][ i ] ;
			work->base_sq[ (((j*DIV_Y)+i)*4) + 1 ] = pos[ j+1 ][ i ] ;
			work->base_sq[ (((j*DIV_Y)+i)*4) + 3 ] = pos[ j ][ i+1 ] ;
			work->base_sq[ (((j*DIV_Y)+i)*4) + 2 ] = pos[ j+1 ][ i+1 ] ;
		}
	}

#ifdef BULLER_VIEW
	r = (upper - under) / DIV_Y ;
	for ( i=0; i<DIV_Y+1; i++ ) {
		base_pos[i].vx = 0.0F ;
		base_pos[i].vy = buller*_RsinF( upper - (r*i) ) ;
		base_pos[i].vz = buller*_RcosF( upper - (r*i) ) ;
	}

	for ( j=0; j<DIV_X+1; j++ ) {
		DG_SetPos( &DG_UnitMatrix ) ;
		DG_RotatePos( &rot[j] ) ;
		DG_PutVector( &base_pos[0], &pos[j][0], DIV_Y+1 ) ;
	}

	for ( j=0; j<DIV_X; j++ ) {
		for ( i=0; i<DIV_Y; i++ ) {
			work->base_sq2[ (((j*DIV_Y)+i)*4) ] = pos[ j ][ i ] ;
			work->base_sq2[ (((j*DIV_Y)+i)*4) + 1 ] = pos[ j+1 ][ i ] ;
			work->base_sq2[ (((j*DIV_Y)+i)*4) + 3 ] = pos[ j ][ i+1 ] ;
			work->base_sq2[ (((j*DIV_Y)+i)*4) + 2 ] = pos[ j+1 ][ i+1 ] ;
		}
	}
#endif

}
/*----------------------------------------------------------------*/
static void DebugEyeViewer( work )
Work	*work ;
{
	DG_SetPos( work->center_world ) ;
	DG_PutVector( &work->base_tri[0], &work->tri[0], (DIV_Y+DIV_X) * 2 * 3 ) ;
	DG_PutVector( &work->base_sq[0], &work->sq[0], DIV_Y * DIV_X * 4 ) ;
	NewTriangleView( &work->tri[0], (DIV_Y+DIV_X) * 2, cR, cG, cB ) ;
	NewSquareView( &work->sq[0], DIV_Y * DIV_X, cR, cG, cB )	;
#ifdef BULLER_VIEW
	DG_PutVector( &work->base_sq2[0], &work->sq2[0], DIV_Y * DIV_X * 4 ) ;
	NewSquareView( &work->sq2[0], DIV_Y * DIV_X, bR, bG, bB )	;
#endif
}
static void StateCheck( work )
Work	*work ;
{
	int len, buller, range, upper, under ;

	
	if ( (work->status2 != NULL) && (*(work->status2) & ENE_STATUS_GLASSES) ) {
		if ( work->stat != ST2_STATE_GLASSES ) {
			work->stat = ST2_STATE_GLASSES ;
			len = work->len + work->glasses ;
			buller = len ;
			range = DEF_GLASSES_RANGE ;
			upper = -DEF_GLASSES_RANGE ;
			under = DEF_GLASSES_RANGE ;
			MakePos( work, work->center_world, len, buller, range, upper, under ) ;
		}
	} else if ( work->stat != ST2_STATE_NONE ) {
		work->stat = ST2_STATE_NONE ;
		len = work->len ;
		buller = len*3/2 ;
		range = work->range ;
		upper = work->upper ;
		under = work->under ;
		MakePos( work, work->center_world, len, buller, range, upper, under ) ;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	if ( work->com != NULL ) {
		if ( work->com->status & CMST_ENEMY_SIGHT_VIEW ) {
			StateCheck( work ) ;
			DebugEyeViewer( work ) ;
		}
	} else if ( work->sw != NULL ) {
		if ( *(work->sw) & SIGHT_VIEW ) {
			DebugEyeViewer( work ) ;
		}
	} else {
		DebugEyeViewer( work ) ;
	}
}

static	void	Die( work )
Work		*work ;
{

}

/*----------------------------------------------------------------*/
static	int	GetResources( work, world, len, range, upper, under, status2 )
Work	*work ;
FMATRIX	*world ;
int		len ;
int		range ;
int		upper ;
int 	under ;
int 	*status2 ;
{

	MakePos( work, world, len, (len*3/2), range, upper, under ) ;

	work->status2 = status2 ;
	work->len = len ;
	work->range = range ;
	work->upper = upper ;
	work->under = under ;
	work->center_world = world ;
	work->stat = 0 ;
	work->glasses = DEF_GLASSES_DIS ;
	work->sw = NULL ;

	return 0 ;
}

/*----------------------------------------------------------------*/
void *NewEyeView( FMATRIX *world, int len, int range, int upper, int under, COMMANDER *com, int *status2 )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), COMMANDER_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, len, range, upper, under, status2 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->com = com ;
		GV_ActorEX( &( work->actor ) ) ;
	}
	return (void *)work ;
}

void *NewEyeView2( FMATRIX *world, int len, int range, int upper, int under )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), COMMANDER_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, len, range, upper, under, NULL ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->com = NULL ;
		GV_ActorEX( &( work->actor ) ) ;
	}
	return (void *)work ;
}

void *NewEyeView3( FMATRIX *world, int len, int range, int upper, int under, COMMANDER *com, int *status2, int glasses )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), COMMANDER_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, len, range, upper, under, status2 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->com = com ;
		work->glasses = glasses ;
		GV_ActorEX( &( work->actor ) ) ;
	}
	return (void *)work ;
}

void *NewEyeView4( FMATRIX *world, int len, int range, int upper, int under, int *sw )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), COMMANDER_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, len, range, upper, under, NULL ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->com = NULL ;
		work->sw = sw ;
		GV_ActorEX( &( work->actor ) ) ;
	}
	return (void *)work ;
}

void DBG_ViewZone( zone, grpid, r,g,b )
int 	zone ;
int		grpid ;
int		r,g,b ;
{
	HZX_ZON		*z ;
	FVECTOR	vec[4] ;
	
	z = HZX_GetZone( grpid, zone ) ;

	vec[0].vx = z->x - z->w ;
	vec[0].vy = z->y ;
	vec[0].vz = z->z - z->h ;

	vec[1].vx = z->x + z->w ;
	vec[1].vy = z->y ;
	vec[1].vz = z->z - z->h ;

	vec[2].vx = z->x + z->w ;
	vec[2].vy = z->y ;
	vec[2].vz = z->z + z->h ;

	vec[3].vx = z->x - z->w ;
	vec[3].vy = z->y ;
	vec[3].vz = z->z + z->h ;

	NewSquareView( &vec[0], 1, r,g,b )	;
}

void DBG_ViewAllZone( hzx_id, r,g,b )
{
	HZX_GRP	*grp ;
    HZX_ZON	*zone ;
	int i ;

	grp = HZX_GetGroup( hzx_id ) ;
	zone = grp->zones ;
	for( i=0; i<grp->n_zones; i++ ) {
		DBG_ViewZone( i, hzx_id, r,g,b ) ;
		zone++ ;
	}
}
