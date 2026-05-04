//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wall_tidal.c
	壁から海水
	ストリップで曲面形成

	2001/04/09 S.Okajima
	$Id: wall_tidal.c,v 1.1.1.3 2002/11/19 11:47:26 Yoshizawa1 Exp $
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

#define SET_NUM	(2)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int OK_PutSplushSurface( FVECTOR *center, FVECTOR *force );
extern int OK_PutSplush( FVECTOR *center, FVECTOR *force );
extern void *NewSplushTidalParts4( FVECTOR *center, FVECTOR *force, float width );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		pos0;
	FVECTOR		pos1;
	FVECTOR		diff;
	FVECTOR		direction;
	float		intense;
	float		width;

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i;
	float	ftemp;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;

	for( i=0; i<SET_NUM; i++ ){
		ftemp = rnd();
		_sceVu0ScaleVector( &fvtemp0, &work->diff, ftemp );
		_sceVu0AddVector( &fvtemp0, &work->pos0, &fvtemp0 ) ;
		_sceVu0ScaleVector( &fvtemp1, &work->direction, rnd()*0.75f + ftemp*0.25f );
		if(i==0)OK_PutSplush( &fvtemp0, &fvtemp1 );
		NewSplushTidalParts4( &fvtemp0, &fvtemp1, work->width*(0.75f+rnd()*0.25f) );
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
・全部初期化時のみ参照
・pos0 pos1 の線分上からdirection 方向に width の 厚さで intense の強さの水が出る
*/
void *NewWallTidal( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *direction, float intense, float width )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		if( pos0->vy < pos1->vy ){
			DG_COPY_VEC( &work->pos0, pos0 );
			DG_COPY_VEC( &work->pos1, pos1 );
		}else{
			DG_COPY_VEC( &work->pos0, pos1 );
			DG_COPY_VEC( &work->pos1, pos0 );
		}

		_sceVu0SubVector( &work->diff, &work->pos1, &work->pos0 ) ;

		_sceVu0Normalize( &work->direction, direction );
		_sceVu0ScaleVector( &work->direction, &work->direction, intense );

		work->intense = intense;
		work->width   = width;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

