//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_big.c
	指定点posを中心とした半径radiusの円周上から水飛沫を発生

	2000/03/13 S.Okajima
	$Id: splush_big.c,v 1.1.1.3 2002/11/19 11:47:24 Yoshizawa1 Exp $
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
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"
#include	"utl_dma.h"

#define	SCR_POS0		(SCRPAD_ADDR)
#define	SCR_POS1		(SCRPAD_ADDR+0x10)
#define	SCR_POS2		(SCRPAD_ADDR+0x20)
#define	SCR_POS3		(SCRPAD_ADDR+0x30)

#define	LIFE		(8)
#define	NUM			(8)
#define	SCALE		(1.0f/16.0f)
#define	DECAY_RATIO	(0.995f)
//#define	DECAY_RATIO	(1.01f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplushSurface2( FVECTOR *center, FVECTOR *force, float intense );
extern int	OK_PutSplushSurface2_Alpha( FVECTOR *center, FVECTOR *force, float intense, int alpha );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	int			alpha;
	int			life;
	FVECTOR		center;
	float		radius;
	float		intense;

} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int	i;
	FVECTOR	pos0;
	FVECTOR	pos1;
	FVECTOR	pos2;
	float	radius;
	float	angle0;
	float	angle1;
	float	ftemp0;
	float	ftemp1;
	float	intense;


	radius = work->radius;
	DG_COPY_VEC( &pos0, &work->center );

	i = NUM;
	while( --i>=0 ){
		angle0 = TPI*rnd();
		pos1.vx = pos0.vx + radius*sinf( angle0 );
		pos1.vy = pos0.vy;
		pos1.vz = pos0.vz + radius*cosf( angle0 );

		angle1 = PI*(0.5f - 0.125f*rnd());
		ftemp0 = cosf( angle1 )*0.5f;
		ftemp1 = sinf( angle1 );
		pos2.vx = (pos1.vx - pos0.vx)*ftemp0;
		pos2.vy = radius         *ftemp1;
		pos2.vz = (pos1.vz - pos0.vz)*ftemp0;
		pos2.vw = 1.0f;
		_sceVu0Normalize( &pos2, &pos2);
		intense = work->intense*( 0.5f + 0.5f*rnd() );
		_sceVu0ScaleVector( &pos2, &pos2, intense );
		OK_PutSplushSurface2_Alpha( &pos1, &pos2, intense, work->alpha&255 );

		work->intense*= DECAY_RATIO;
	}

	if( work->life-- < 0 ) GV_DestroyActor( work ) ;
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	work->life = LIFE;

	return (0);
}

/* ---------------------------------------------------------------- */
// 指定点posを中心とした半径radiusの円周上から水飛沫を発生
void *NewSphereSplush( FVECTOR *pos, float radius, float intense, int alpha )
{
	Work		*work ;


//printf(":::::::::::::::::::::::%f %f %d\n",radius, intense, alpha);

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
//printf("drop_bdy_splush::::::::::::::::::::::::::::::work_size:%x\n",sizeof( Work ));
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		work->radius = radius;
		work->intense = intense;
		work->alpha = alpha;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

