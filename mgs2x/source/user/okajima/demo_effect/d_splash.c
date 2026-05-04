//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash.c
	デモ用水飛沫（呼び出し口）
	2000/04/01 S.Okajima
	$Id: d_splash.c,v 1.1.1.3 2002/11/19 11:46:54 Yoshizawa1 Exp $
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
#include        "libutl.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


#define	SIZE_MIN	(10.0f)


/*----------------------------------------------------------------*/
extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense );

typedef	struct	{
	GV_ACT_EX	actor ;
	FVECTOR		origin;
	FVECTOR		center;
	float		radius;
	float		size;
	int			multiple;
 	int			life;
} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	fvtemp0;
	SVECTOR	rot;
	float	rad;
	float	radius;
	int		i;

	if( work->life > 0 ){
		for( i=0; i<work->multiple; i++ ){
			rad = TPI*rnd();
			radius = work->radius * rnd();
			fvtemp0.vx = work->center.vx + sinf( rad ) * radius;
			fvtemp0.vy = work->center.vy;
			fvtemp0.vz = work->center.vz + cosf( rad ) * radius;

			OK_DirVecXY( &work->origin, &fvtemp0, &rot );
			NewSplashParts_Demo( &fvtemp0, &rot, work->size );
		}
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
}


static int GetResources( Work *work, FVECTOR *origin, FVECTOR *center, float radius, float size, int multiple, int life )
{
	DG_COPY_VEC( &work->origin, origin );
	DG_COPY_VEC( &work->center, center );
	work->radius   = radius;
	if( work->radius < 0.0f ) work->radius = 0.0f;
	work->size     = size;
	if( work->size < SIZE_MIN ) work->size = SIZE_MIN;
	work->multiple = multiple;
	if( work->multiple < 1 ) work->multiple = 1;
	work->life     = life;
	if( work->life < 1 ) work->life = 1;

	return 0 ;
}

/*
[ポインタ保存必要なし]
FVECTOR *origin:方向計算用（起源点）
FVECTOR *center:発生円の中心（高さＹの水平面のみから発生する）
float radius   :発生円の半径
float size     :水飛沫自体の大きさ
int multiple   :１フレームに発生させる水飛沫のパーツの個数
int life       :発生させ続けるフレーム数（その後自殺する）
*/
void *NewSplash_Demo( FVECTOR *origin, FVECTOR *center, float radius, float size, int multiple, int life )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, origin, center, radius, size, multiple, life  ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
