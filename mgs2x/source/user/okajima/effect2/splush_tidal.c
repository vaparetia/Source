//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_tidal.c
	落下物の水飛沫
	ストリップで曲面形成

	2001/01/29 S.Okajima
	$Id: splush_tidal.c,v 1.1.1.3 2002/11/19 11:47:25 Yoshizawa1 Exp $

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

#define	SCR_POS0	(SCRPAD_ADDR + 0x00)
#define	SCR_POS1	(SCRPAD_ADDR + 0x10)
#define	SCR_POS2	(SCRPAD_ADDR + 0x20)
#define	SCR_POS3	(SCRPAD_ADDR + 0x30)

#define COL_R (48)
#define COL_G (48)
#define COL_B (48)
#define COL_A (160)


#define	N_VERTS		(64)
#define	N_PRIMS		(16)
#define	N_TOTAL		(N_PRIMS*N_VERTS)

#define INIT_DIVIDE (N_PRIMS)
#define SIDE_DIVIDE ((N_VERTS/2)-1)

#define DECAY_PRIM_VEC (0.95f)

#define CHECK_NUM	(64)
#define CHECK_SKIP	(CHECK_NUM/INIT_DIVIDE)

#define SPLUSH_SCALE (0.75f)
#define DELAY_TIME	(64.0f)

#define SURFACE_SCALE	(0.95f)

#define DECAY_SPLUSH	(0.5f)

#define EX_SCALE_DOWN	(0.25f*0.125f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int OK_PutSplushSurface( FVECTOR *center, FVECTOR *force );
extern int OK_PutSplush( FVECTOR *center, FVECTOR *force );
extern void *NewSplushTidalParts( FVECTOR *center, FVECTOR *force, float width );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		*world;
	FVECTOR		shift0;
	FVECTOR		shift1;

	FVECTOR		before_pos[CHECK_NUM];
	FVECTOR		before_force[CHECK_NUM];

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR		pos0;
	FVECTOR		pos1;
	FVECTOR		center;
	FVECTOR		real_center;
	FVECTOR		diff;
	FVECTOR		diff2;
	FVECTOR		fvtemp;
	FVECTOR		direction;
	FVECTOR		*before;
	FVECTOR		*b_force;
	int		i;
	float	ratio;
	float	intense;
	float	width;
	float	len;
	float	ftemp;


	DG_SetPos( work->world );
	DG_PutVector( &work->shift0, &pos0, 1 );
	DG_PutVector( &work->shift1, &pos1, 1 );

	if( pos0.vy < pos1.vy ){
		_sceVu0SubVector( &direction, &pos0, &pos1 ) ;
	}else{
		_sceVu0SubVector( &direction, &pos1, &pos0 ) ;
	}
	direction.vy = 0.0f;
	_sceVu0Normalize( &direction, &direction );

//	AN_Test_Eye2( &pos0, 2 );
//	AN_Test_Eye2( &pos1, 2 );


//	DG_COPY_VEC( &pos0, work->pos0 );
//	DG_COPY_VEC( &pos1, work->pos1 );
	_sceVu0SubVector( &diff, &pos1, &pos0 ) ;
//	_sceVu0ScaleVector( &diff, &diff, 10.0f );
//	_sceVu0AddVector( &pos1, &pos0, &diff ) ;

	len = GV_VecLen3F( &diff )/(float)CHECK_NUM;

//	AN_Test_Eye2( &pos0, 2 );
//	AN_Test_Eye2( &pos1, 2 );

	before = work->before_pos;
	b_force = work->before_force;
	for( i=0; i<CHECK_NUM; i++ ){
		ratio = (float)i/(float)CHECK_NUM;
		_sceVu0ScaleVector( &fvtemp, &diff, ratio );
		_sceVu0AddVector( &real_center, &pos0, &fvtemp) ;

		ratio = DG_FABS(ratio-0.5f)*2.0f;

		width   = intense = (before->vy - GM_WaterLevel);
		intense*= SPLUSH_SCALE;
		width  *= 0.1f; 

		if( before->vy      > GM_WaterLevel
		 && real_center.vy  < GM_WaterLevel ){

			_sceVu0SubVector( &diff2, &real_center, before ) ;
			_sceVu0ScaleVector( &fvtemp, &diff2, DG_FABS( (before->vy - GM_WaterLevel)/diff2.vy ) );
			_sceVu0AddVector( &center, before, &fvtemp ) ;
			center.vy = GM_WaterLevel;

			fvtemp.vx =-diff.vz;
			fvtemp.vy = 0.0f;
			fvtemp.vz = diff.vx;
			fvtemp.vw = 0.0f;
			_sceVu0Normalize( &diff2, &fvtemp );

			_sceVu0ScaleVector( &fvtemp, &diff2, intense*(0.5f+0.5f*rnd()) );
			b_force->vx = fvtemp.vx;
			b_force->vz = fvtemp.vz;
			b_force->vy = intense*(0.5f+0.5f*rnd());
			before->vx = center.vx;
			before->vy = center.vy;
			before->vz = center.vz;
			before->vw = DELAY_TIME;


			ftemp = GV_VecLen3F( b_force )*ratio;

			diff2.vx = b_force->vx*2.0f*(1.0f-ratio) + direction.vx*ftemp;
			diff2.vy = b_force->vy*1.0;
			diff2.vz = b_force->vz*2.0f*(1.0f-ratio) + direction.vz*ftemp;
			_sceVu0ScaleVector( &diff2, &diff2, EX_SCALE_DOWN );
			fvtemp.vx = before->vx + diff2.vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = before->vz + diff2.vz;
			NewSplushTidalParts( &fvtemp, &diff2, len*0.5f );

			diff2.vx =-b_force->vx*2.0f*(1.0f-ratio) + direction.vx*ftemp;
			diff2.vy = b_force->vy*1.0;
			diff2.vz =-b_force->vz*2.0f*(1.0f-ratio) + direction.vz*ftemp;
			_sceVu0ScaleVector( &diff2, &diff2, EX_SCALE_DOWN );
			fvtemp.vx = before->vx + diff2.vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = before->vz + diff2.vz;
			NewSplushTidalParts( &fvtemp, &diff2, len*0.5f );
		}else{
			before->vx = real_center.vx;
			before->vy = real_center.vy;
			before->vz = real_center.vz;
			before->vw = (before->vw > 1.0f)? before->vw-1.0f: 0.0f;
		}
		if( before->vw > 0.0f ){
			fvtemp.vx = before->vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = before->vz;

			ftemp = GV_VecLen3F( b_force )*ratio;

			diff2.vx = b_force->vx*(1.0f-ratio) + direction.vx*ftemp;
			diff2.vy = b_force->vy;
			diff2.vz = b_force->vz*(1.0f-ratio) + direction.vz*ftemp;
			_sceVu0ScaleVector( &diff2, &diff2, EX_SCALE_DOWN );
			fvtemp.vx = before->vx + diff2.vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = before->vz + diff2.vz;
			OK_PutSplush( &fvtemp, &diff2 );

			diff2.vx =-b_force->vx*(1.0f-ratio) + direction.vx*ftemp;
			diff2.vy = b_force->vy;
			diff2.vz =-b_force->vz*(1.0f-ratio) + direction.vz*ftemp;
			_sceVu0ScaleVector( &diff2, &diff2, EX_SCALE_DOWN );
			fvtemp.vx = before->vx + diff2.vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = before->vz + diff2.vz;
			OK_PutSplush( &fvtemp, &diff2 );
		}

		before++;
		b_force++;
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
・座標のポインタ先を毎フレーム参照
・world で計算された 線分shift0,shift1 が水面を上から下に横切った時に水飛沫発生
・加速度判定する
*/
void *NewSplushTidal( FMATRIX *world, FVECTOR *shift0, FVECTOR *shift1 )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->world = world;
		DG_COPY_VEC( &work->shift0, shift0 );
		DG_COPY_VEC( &work->shift1, shift1 );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

