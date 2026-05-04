//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash_ripple.c
	デモ用風紋状水飛沫（呼び出し口）
	2000/04/01 S.Okajima
	$Id: d_splash_ripple.c,v 1.1.1.3 2002/11/19 11:46:55 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


#define	SIZE_MIN	(10.0f)

#define	ARC_LIFE_MAX	(60)

/*----------------------------------------------------------------*/
extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense );

typedef	struct	{
	GV_ACT_EX	actor ;
	FVECTOR		center;
	FVECTOR		origin;

	float		radius;
	float		direction;
	float		angle;
	float		size;
	int			multiple;
 	int			life;

	float		arc_radius;
	float		arc_direction;
	float		arc_angle;
	int			arc_life;

} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	SVECTOR	rot;
	float	rad;
	float	radius;
	float	ftemp0;
	float	ftemp1;
	int		i;

	if( work->life > 0 ){

		if( work->arc_life <= 0 ){
			work->arc_life = irnd()%ARC_LIFE_MAX/2 + ARC_LIFE_MAX/2;
			work->arc_radius    = work->radius*0.4f + work->radius*0.2f*rnd();

			work->arc_direction = (work->direction - work->angle*0.5f) + (work->angle*rnd());
			ftemp0 = (work->direction + work->angle) - work->arc_direction;
			ftemp1 = work->arc_direction - (work->direction - work->angle);
			ftemp0 = (ftemp0 < ftemp1)?ftemp0:ftemp1;
			work->arc_angle = ftemp0*rnd()*0.1f + ftemp0*0.9f;
		}

		OK_FloatSmoother( &work->arc_radius, &work->radius, 0.8f );

//printf("%f\n",work->arc_radius);

		for( i=0; i<work->multiple; i++ ){
			rad = work->arc_direction + work->arc_angle*frnd();
			radius = work->arc_radius;
			fvtemp0.vx = work->center.vx + sinf( rad ) * radius;
			fvtemp0.vy = work->center.vy;
			fvtemp0.vz = work->center.vz + cosf( rad ) * radius;


			DG_SetPos( &DG_Chanls->eye_pers );
			DG_PutVector( &fvtemp0, &fvtemp1, 1 );
			if( (fvtemp1.vz > fvtemp1.vw) ){
				continue;
			}else{
				if(fvtemp1.vw < 0.0f) fvtemp1.vw = -fvtemp1.vw;
//				fvtemp1.vw *= 2.0f;
				if( fvtemp1.vx < -fvtemp1.vw ){
					continue;
				}else if( (fvtemp1.vx >  fvtemp1.vw) ){
					continue;
				}else if( (fvtemp1.vy < -fvtemp1.vw) ){
					continue;
				}else if( (fvtemp1.vy >  fvtemp1.vw) ){
					continue;
				}
			}

			OK_DirVecXY( &work->origin, &fvtemp0, &rot );
			NewSplashParts_Demo( &fvtemp0, &rot, work->size );
		}

		work->life--;
	}else if( work->arc_life <= 0 ){
		GV_DestroyActor( work ) ;
	}
	work->arc_life--;
}

static void Die( Work *work )
{
}


static int GetResources( Work *work, FVECTOR *center, float radius, float direction, float angle, float size, int multiple, int life )
{
	DG_COPY_VEC( &work->center, center );
	work->radius    = radius;
	if( work->radius < 0.0f ) work->radius = 0.0f;
	work->direction = direction;
	work->angle     = angle;
	work->size      = size;
	if( work->size < SIZE_MIN ) work->size = SIZE_MIN;
	work->multiple  = multiple;
	if( work->multiple < 1 ) work->multiple = 1;
	work->life      = life;
	if( work->life < 1 ) work->life = 1;

	work->arc_life = 0;


	work->origin.vx = work->center.vx;
	work->origin.vy = work->center.vy - work->radius*0.3f;
	work->origin.vz = work->center.vz;


	return 0 ;
}

/*
[ポインタ保存必要なし]
FVECTOR *center   :発生円の中心（高さＹの水平面のみから発生する）
float radius      :発生円の最大半径
float direction   :Ｙ軸回転角度（ラジアン単位）。
float angle       :direction からの＋－
float size        :水飛沫自体の大きさ
int multiple      :発生させる円弧内の水飛沫の数
int life          :発生させ続けるフレーム数（その後自殺する）
*/
void *NewSplashRipple_Demo( FVECTOR *center, float radius, float direction, float angle, float size, int multiple, int life )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, center, radius, direction, angle, size, multiple, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
