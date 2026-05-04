//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shake_camera.c
	爆破時カメラ揺らし
	2000/03/10 S.Okajima
	$Id: shake_camera.c,v 1.1.1.3 2002/11/19 11:47:42 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define		FAR_DISTANCE	(10000.0f)
#define		NEAR_DISTANCE	(1000.0f)

#define		GLL_FAR_DISTANCE	(50000.0f)
#define		GLL_NEAR_DISTANCE	(5000.0f)

extern void GM_SetCameraAdjust( int	chanl, FVECTOR	*adj );

typedef	struct	{
	GV_ACT_EX		actor ;

	float		intense_max;
	float		life_max;
	int			life;
	int			chanl;
	FVECTOR		adj;

} Work ;

static	void Act( Work *work )
{
	float	angle;
	float	ftemp;

	if( work->life == work->life_max ){
		angle = rnd() * TPI;
		work->adj.vx = sinf(angle)*work->intense_max;
		work->adj.vy = cosf(angle)*work->intense_max;

	}else if( work->life > 0 ){
		angle = rnd() * TPI;
		ftemp = (float)work->life / work->life_max;
		work->adj.vx = sinf(angle)*rnd()*work->intense_max * ftemp;
		work->adj.vy = cosf(angle)*rnd()*work->intense_max * ftemp;
	}else{
		GV_DestroyActor( work ) ;
	}

	GM_SetCameraAdjust( work->chanl, &work->adj );
	work->life--;
}

static	void Die( Work *work )
{
//	GM_SetCameraAdjust( work->chanl, &DG_ZeroVector );
}

static	int GetResources( Work *work, int chanl, float intense, int time )
{
	work->chanl       = chanl;
	work->intense_max = intense;
	work->life        = time;
	work->life_max    = (float)time;
	DG_COPY_VEC( &work->adj, &DG_ZeroVector );

	return 0;
}

void *NewShakeCamera( int chanl, int intense, int time )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, chanl, (float)intense, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos )
{
	Work		*work ;
	float		ftemp;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		ftemp = GV_VecLen3F2( (FVECTOR *)DG_Chanls[chanl].eye.m[3], pos );
		if( ftemp > FAR_DISTANCE ){
			ftemp = FAR_DISTANCE;
		}else if( ftemp < NEAR_DISTANCE ){
			ftemp = NEAR_DISTANCE;
		}
		ftemp  = (float)intense * (1.0f - (ftemp-NEAR_DISTANCE) / (FAR_DISTANCE-NEAR_DISTANCE));

		if ( GetResources( work, chanl, ftemp, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewShakeCameraGLL( int chanl, int intense, int time, FVECTOR *pos )
{
	Work		*work ;
	float		ftemp;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		ftemp = GV_VecLen3F2( (FVECTOR *)DG_Chanls[chanl].eye.m[3], pos );
		if( ftemp > GLL_FAR_DISTANCE ){
			ftemp = GLL_FAR_DISTANCE;
		}else if( ftemp < GLL_NEAR_DISTANCE ){
			ftemp = GLL_NEAR_DISTANCE;
		}
		ftemp  = (float)intense * (1.0f - (ftemp-GLL_NEAR_DISTANCE) / (GLL_FAR_DISTANCE-GLL_NEAR_DISTANCE));

		if ( GetResources( work, chanl, ftemp, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
