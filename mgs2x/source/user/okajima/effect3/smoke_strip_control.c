//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	smoke_strip_control.c
	ストリップ煙の発生源
	2001/08/09 S.Okajima
	$Id: smoke_strip_control.c,v 1.1.1.3 2002/11/19 11:47:36 Yoshizawa1 Exp $

*/

#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>


#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern void *NewSmokeStrip( FMATRIX *world, float radius, int life, int col, int mode, int side, int seed );
extern void *NewSmoke2Strip( FMATRIX *world, float width, int life, int col, int mode, int seed );
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		pos0;
	FVECTOR		pos1;
	FVECTOR		diff;
	float		size;
	int			num;
	int			col;

	int			seed;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i;
	FVECTOR	fvtemp;
	SVECTOR	svtemp;
	FMATRIX	fmat;

	svtemp.vz = 0;
	for( i=0; i<work->num; i++ ){
		if( (GM_IRnd(&work->seed)>>8)&1 ){
			svtemp.vx = (GM_IRnd(&work->seed)>>8)&4095;
			svtemp.vy = (GM_IRnd(&work->seed)>>8)&4095;
			_sceVu0ScaleVector( &fvtemp, &work->diff, GM_Rnd(&work->seed) );
			_sceVu0AddVector( &fvtemp, &work->pos0, &fvtemp ) ;
			fvtemp.vx+= work->size*GM_FRnd(&work->seed);
			fvtemp.vy+= work->size*GM_FRnd(&work->seed);
			fvtemp.vz+= work->size*GM_FRnd(&work->seed);
			DG_SetPos2( &fvtemp, &svtemp );
			DG_GetPos( &fmat );
			NewSmokeStrip(
				&fmat,
				work->size*(1.0f+GM_Rnd(&work->seed)),
				(GM_IRnd(&work->seed)>>8)%64+32,
				work->col,
				(GM_IRnd(&work->seed)>>8)&1,
				(GM_IRnd(&work->seed)>>8)&1,
				GM_IRnd(&work->seed)
				 );
		}else{
			svtemp.vx = (GM_IRnd(&work->seed)>>8)&4095;
			svtemp.vy = (GM_IRnd(&work->seed)>>8)&4095;
			_sceVu0ScaleVector( &fvtemp, &work->diff, GM_Rnd(&work->seed) );
			_sceVu0AddVector( &fvtemp, &work->pos0, &fvtemp ) ;
			fvtemp.vx+= work->size*GM_FRnd(&work->seed);
			fvtemp.vy+= work->size*GM_FRnd(&work->seed);
			fvtemp.vz+= work->size*GM_FRnd(&work->seed);
			DG_SetPos2( &fvtemp, &svtemp );
			DG_GetPos( &fmat );
			NewSmoke2Strip(
				&fmat,
				work->size*(3.0f + 5.0f*GM_Rnd(&work->seed)),
				(GM_IRnd(&work->seed)>>8)%64+32,
				work->col,
				(GM_IRnd(&work->seed)>>8)&1,
				GM_IRnd(&work->seed)
				 );
		}
	}

}

static void Die( Work *work )
{
}


static int GetResources( Work *work )
{
	_sceVu0SubVector( &work->diff, &work->pos1, &work->pos0 ) ;
	return 0 ;
}


void *NewSmokeStripControl( FVECTOR *pos0, FVECTOR *pos1, float size, int num, int col, int seed )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos0, pos0 );
		DG_COPY_VEC( &work->pos1, pos1 );
		work->size = size;
		work->num = num;
		work->col = col;
		work->seed = seed;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

