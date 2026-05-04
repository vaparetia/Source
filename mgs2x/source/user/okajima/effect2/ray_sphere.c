//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ray_sphere.c
	仮想球体と水面のインタラクションシミュレート
	ストリップ曲面形成

	2001/04/10 S.Okajima
	$Id: ray_sphere.c,v 1.1.1.3 2002/11/19 11:47:23 Yoshizawa1 Exp $
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

#include	"../../mode/demo/eft_con.h"

#define SPLUSH_NUM		(4)
#define LEN_LIMIT		(10.0f)
#define ADJUST_SCALE	(0.5f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern void *NewCrushParts( FVECTOR *center, FVECTOR *force, float width, float floor_height );
extern int	OK_PutFloorLight( FVECTOR *center, float floor_height, float radius, CVECTOR col );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	CVECTOR		col;

	FVECTOR		*pos;
	FVECTOR		before_pos;
	float		radius;
	float		before_radius;

	float		floor_height;

	int			count;

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i;
	float	ftemp;
	FVECTOR	fvtemp;
	FVECTOR	pos;
	FVECTOR	accel;
	FVECTOR	unit_circle;
	FVECTOR	local_circle;

	work->count++;

	if( work->count < 2 ) return;


	pos.vx = work->pos->vx + work->radius*frnd();
	pos.vy = work->pos->vy + work->radius*frnd();
	pos.vz = work->pos->vz + work->radius*frnd();


	unit_circle.vy  = 0.0f;
	local_circle.vy = work->floor_height;
	for( i=0; i<SPLUSH_NUM; i++ ){
		ftemp = TPI*rnd();
		unit_circle.vx = sinf(ftemp);
		unit_circle.vz = cosf(ftemp);

		local_circle.vx = unit_circle.vx*work->radius;
		local_circle.vz = unit_circle.vz*work->radius;

		// 発生点
		fvtemp.vx = work->pos->vx + local_circle.vx;
		fvtemp.vy = work->pos->vy;
		fvtemp.vz = work->pos->vz + local_circle.vz;

		// 加速度
		accel.vx = work->radius*unit_circle.vx;
		accel.vy = work->radius * (1.0f * rnd()) * 8.0f;
		accel.vz = work->radius*unit_circle.vz;
		_sceVu0ScaleVector( &accel, &accel, 0.5f );

		NewCrushParts( &fvtemp, &accel, work->radius*(0.25f+rnd()*0.75f), work->pos->vy );

//		OK_PutFloorLight( &fvtemp, work->radius*rnd(), work->radius, work->col );
	}



}


/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}


static int GetResources( Work *work )
{

	work->col.r = 255;
	work->col.g = 255;
	work->col.b = 255;
	work->col.cd= 255;

	work->count = 0;

	return 0 ;
}

/* ---------------------------------------------------------------- */
/*
・pos のポインタはずっと参照
・中心 pos 半径 radius の 球があるとして、水面の上下で自動的に水飛沫発生
・速度を考慮する
・使い方１：「水面に物体突入」の場合はそのものに付ける
・使い方２：「荒れる水面」の場合は pos を 水面を挟んで何度も往復させる
*/

void *NewRayCrush( FVECTOR *pos, float radius )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->pos    = pos;
		work->radius = radius;
		work->floor_height = pos->vy;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewRayCrush_EftCtrl( int con_name, float radius )
{
	EFTCONTROL	*ctrl;
	ctrl = DM_GetEftControl( con_name );
	if( !ctrl ) return NULL;
	return NewRayCrush( &ctrl->mov, radius );
}

