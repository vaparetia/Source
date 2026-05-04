//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	point_tidal.c
	水飛沫発生点
	ストリップで曲面形成

	2001/04/09 S.Okajima
	$Id: point_tidal.c,v 1.1.1.3 2002/11/19 11:47:22 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int OK_PutSplushSurface( FVECTOR *center, FVECTOR *force );
extern int OK_PutSplush( FVECTOR *center, FVECTOR *force );
extern void *NewSplushTidalParts( FVECTOR *center, FVECTOR *force, float width );
extern void *NewSplushTidalParts2( FVECTOR *center, FVECTOR *force, float width );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		*world;
	float		*intense;
	float		*width;

	int			life;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	center;
	FVECTOR	fvtemp;

	DG_COPY_VEC( &center, (FVECTOR *)work->world->m[3] );


	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = (*work->intense);
	fvtemp.vw = 1.0f;

	DG_SetPos( work->world );
	DG_RotVector( &fvtemp, &fvtemp, 1 );

	//OK_PutSplush( &center, &fvtemp1 );
	NewSplushTidalParts2( &center, &fvtemp, (*work->width)*(0.75f+rnd()*0.25f) );

	if( work->life >= 0 ){
		work->life--;
		if( work->life <= 0 ) GV_DestroyActor( work ) ;
	}

}


/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}


static int GetResources( Work *work )
{
	return 0 ;
}

/* ---------------------------------------------------------------- */
/*
・全てずっと参照する！！
・world の 位置・方向（Ｚ方向）に width の 厚さで intense の強さの水が出る
*/
void *NewPointTidal( FMATRIX *world, float *intense, float *width, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->world = world;
		work->intense = intense;
		work->width = width;
		work->life = DIRECT_TICK( life );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

