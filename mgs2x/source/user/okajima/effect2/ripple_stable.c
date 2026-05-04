//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ripple_stable.c
	指定点から波紋を発生

	2001/04/13 S.Okajima
	$Id: ripple_stable.c,v 1.1.1.3 2002/11/19 11:47:23 Yoshizawa1 Exp $
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

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern float GM_WaterLevel;
extern int OK_PutRipple( FVECTOR *center );

#define RADIUS (30.0f)
#define RIPPLE_RATIO (15)
// RIPPLE_RATIO ＆をとる
// RIPPLE_RATIO フレームに一回波紋を呼ぶ

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			num;

	FVECTOR		pos[0];
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*fvtemp;
	FVECTOR	center;
	float	ftemp;
	float	radius;
	int	i;



	fvtemp = work->pos;
	for( i=0; i<work->num; i++ ){
		if( ((irnd()>>8)&RIPPLE_RATIO)==0 ){
			ftemp  = TPI*rnd();
			radius = RADIUS*rnd();
			center.vx = fvtemp->vx + radius*sinf(ftemp);
			center.vy = GM_WaterLevel;
			center.vz = fvtemp->vz + radius*cosf(ftemp);
			OK_PutRipple( &center );
		}
		fvtemp++;
	}

}

static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	int	i;
	FVECTOR	*fvtemp;

	if ( GCL_GetOption( 'p' ) != NULL ) {
		fvtemp = work->pos;
		for( i=0; i<work->num; i++ ){
			fvtemp->vx = (float)GCL_GetNextInt() ;
			fvtemp->vy = 0.0f ;
			fvtemp->vz = (float)GCL_GetNextInt() ;
			fvtemp++;
		}
	}else{
		printf("波紋のポジションがありません");
		return -1;
	}

	return 0 ;
}

void *NewRippleStable( int name, int map )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;

	OPERATOR() ;

	unit_num=0;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		unit_num = GCL_GetNextInt() ;
	}

	buf_size = sizeof( Work ) + sizeof( FVECTOR ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->num = unit_num;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

