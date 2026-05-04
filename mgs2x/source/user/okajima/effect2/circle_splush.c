//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	circle_splush.c
	水飛沫発生点
	ストリップで曲面形成
	沢山呼んで円形にする

	2001/04/10 S.Okajima
	$Id: circle_splush.c,v 1.1.1.3 2002/11/19 11:47:19 Yoshizawa1 Exp $
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
	float		intense;
	float		angle;

	int			num;
	int			life;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i;
	float	angle;
	FVECTOR	center;
	FVECTOR	fvtemp;
	SVECTOR	svtemp;

	DG_COPY_VEC( &center, (FVECTOR *)work->world->m[3] );

	fvtemp.vx = 1.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 0.0f;
	fvtemp.vw = 1.0f;
	DG_SetPos( work->world );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	OK_DirVecXY( &DG_ZeroVector, &fvtemp, &svtemp );


	for( i=0; i<work->num; i++ ){
		angle = work->angle*(1.0f + frnd()*0.1f);
		fvtemp.vx = 0.0f;
		fvtemp.vy = work->intense*cosf(angle);
		fvtemp.vz = work->intense*sinf(angle);
		fvtemp.vw = 1.0f;

		svtemp.vz = (irnd()>>8)&4095;
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( &fvtemp, &fvtemp, 1 );

		NewSplushTidalParts2( &center, &fvtemp, work->intense*0.1f*(0.75f+rnd()*0.25f) );
	}


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
・world はずっと参照する！！他は直値
・world の X方向を軸として intense の強さの水飛沫を 出す。
・angle は 球座標系の角度で、0.0f でＸ方向のみ、1.0f で最大開放
*/
void *NewCircleSplush( FMATRIX *world, float intense, float angle, int num, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->world = world;
		work->intense = intense;

		if( angle < 0.0f ){
			work->angle = 0.0f;
		}else if( angle > 1.0f ){
			work->angle = PI*0.5f;
		}else{
			work->angle = angle*PI*0.5f;
		}

		work->num = num;
		work->life = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

