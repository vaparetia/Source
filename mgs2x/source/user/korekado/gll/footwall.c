//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	footwall.c
	足壁設置

	2002/07/01 Y.Korekado
	$Id: footwall.c,v 1.2 2002/11/25 01:54:46 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define WALL_MODEL	(5172135)	/*box_koreB*/

static FVECTOR WallPos[4] = {
	{ -4000.0, 0.0, -22000.0 },
	{ -4000.0, 0.0, -21000.0 },
	{ 3000.0, 0.0, -20000.0 },
	{ 3000.0, 0.0, -19000.0 },
} ;

/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT_EX	actor;
	OBJECT		body[4] ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
}

static	void	Die( Work *work )
{
	int	i ;

	for( i=0; i<4; i++ ) {
	    GM_FreeObject( &work->body[i] ) ;
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	extern void *NewMakeObjectBoundHazard( HZX_GROUP_ID	hzx_id, DG_OBJS *objs, u_int seg_atr, u_int flr_atr ) ;
	int	i ;

	for( i=0; i<4; i++ ) {
		GM_InitObject( &work->body[i], WALL_MODEL, BODY_FLAG );

		DG_SetPos2( &WallPos[i], &DG_ZeroSVector ) ;
		DG_PutObjs( work->body[i].objs ) ;
		NewMakeObjectBoundHazard( GM_GetHzxGroupID( GM_CurrentMap ), 
			work->body[i].objs,
			HZX_SEG_NO_BULLETHOLE|HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE,
			HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_BULLET|HZX_FLOOR_NO_MISSILE ) ;
		work->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
	}

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewFootWall( void )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
