//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	searchlt.c
	ごるるごんサーチライトエフェクト

	2002/07/01 Y.Korekado
	$Id: searchlt.c,v 1.1.1.3 2002/11/19 11:44:14 Yoshizawa1 Exp $
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

extern void AN_Test_Eye2( FVECTOR *mov, int i ) ; /* for test */

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	DG_SPOT		spot ;
	DG_SPOT		spot2 ;
	FMATRIX		*root ;
	float		umbra ;
	float		penumbra ;
	float		range ;
	float		angle ;
	int			*color ;
	int			sw ;
} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
//printf( "*work->color[%d]\n",*work->color);
	if ( *work->color ) {
		if ( !(work->sw) ) {
			work->sw = 1 ;
			DG_QueueSpotObjs( &work->spot );
			DG_QueueSpotObjs( &work->spot2 );
		}
		work->spot.world = work->spot2.world = *( work->root ) ;
		work->spot.range = work->spot2.range = work->range ;
		work->spot.angle = work->spot2.angle = cosf( work->angle ) / sinf( work->angle ) ;

		work->spot.color = work->spot2.color = *work->color ;

		DG_SetTmpSpotLight( (FVECTOR*)work->spot.world.m[3], (FVECTOR*)work->spot.world.m[2],
					   work->range, 1.0f, cosf( work->angle ), work->spot.color, LIT_FLAG_CHARAONLY );
	} else {
		if ( work->sw ) {
			work->sw = 0 ;
			DG_DequeueSpotObjs( &work->spot );
			DG_DequeueSpotObjs( &work->spot2 );
		}
	}

if(0){
FVECTOR s ;
FMATRIX m ;

DG_SetPos( &work->spot.world ) ;
s.vx = s.vy = 0.0 ;
s.vz = work->range ;
DG_MovePos( &s ) ;
DG_GetPos( &m ) ;

AN_Test_Eye2((FVECTOR*)work->spot.world.m[3],2);
AN_Test_Eye2((FVECTOR*)m.m[3],2);
}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if ( work->sw ) {
		DG_DequeueSpotObjs( &work->spot );
		DG_DequeueSpotObjs( &work->spot2 );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *root, float umbra, float penumbra, float range, int *color )
{

	work->sw = 1 ;
	work->root = root ;
	work->umbra = umbra ;
	work->penumbra = penumbra ;
	work->range = range ;
	work->angle = penumbra ;
	work->color = color ;

	work->spot.world = *root ;
	DG_QueueSpotObjs( &work->spot );

	work->spot.range = range ;
	work->spot.angle = cosf( work->angle ) / sinf( work->angle ) ;
	work->spot.color = *work->color ;
	work->spot.shadow_id = 355453/* "投影モデル" */ ;

	work->spot2.world = *root ;
	DG_QueueSpotObjs( &work->spot2 );

	work->spot2.range = range ;
	work->spot2.angle = cosf( work->angle ) / sinf( work->angle ) ;
	work->spot2.color = *work->color ;
	work->spot2.shadow_id = 11668919 ; /* 投影モデル箱 */

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewGllSearchLight( FMATRIX *mat, float umbra, float penumbra, float range, int *color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, mat, umbra, penumbra, range, color ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
