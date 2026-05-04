//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	auto_splush.c
	仮想球体と水面のインタラクションシミュレート
	ストリップ曲面形成

	2001/04/10 S.Okajima
	$Id: auto_splush.c,v 1.1.1.3 2002/11/19 11:47:17 Yoshizawa1 Exp $
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
extern int OK_PutSplush( FVECTOR *center, FVECTOR *force );
extern void *NewSplushTidalParts( FVECTOR *center, FVECTOR *force, float width );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		*pos;
	FVECTOR		before_pos;
	float		radius;
	float		before_radius;
	float		intense;

//	int			count;

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i;
	float	ftemp;
	float	len;
	float	data_radius=0.0f;
	FVECTOR	pos;
	FVECTOR	diff;
	FVECTOR	accel;
	FVECTOR	center;
	FVECTOR	fvtemp;
	FVECTOR	norm;
	FVECTOR	unit_circle;
	FVECTOR	local_circle;

/*
	FVECTOR	test_pos;
	float fff;

	if( GV_PadData[1].status & PAD_L1 ){
		GM_WaterLevel+= 10.0f;
	}else if( GV_PadData[1].status & PAD_L2 ){
		GM_WaterLevel-= 10.0f;
	}

	work->count++;
	fff = (float)work->count * 0.051f;
	work->pos = &test_pos;
	test_pos.vx = GM_PlayerPosition.vx + 2500.0f*sinf(fff*1.301f) + 1500.0f*sinf(fff*1.201f);
	test_pos.vy = GM_PlayerPosition.vy + 1000.0f*sinf(fff*1.011f) + 500.0f*sinf(fff*1.221f);
	test_pos.vz = GM_PlayerPosition.vz + 2500.0f*cosf(fff*1.101f) + 1500.0f*sinf(fff*1.241f);
AN_Test_Eye2( &test_pos, 2 );
*/

	DG_COPY_VEC( &pos, work->pos );
	if( (pos.vy              - work->radius > GM_WaterLevel)
	 && (work->before_pos.vy - work->radius > GM_WaterLevel) ){ // 過去現在とも水上
		DG_COPY_VEC( &work->before_pos, &pos );
		return;
	}
	if( (pos.vy              + work->radius < GM_WaterLevel)
	 && (work->before_pos.vy + work->radius < GM_WaterLevel) ){ // 過去現在とも水面下
		DG_COPY_VEC( &work->before_pos, &pos );
		return;
	}

	_sceVu0SubVector( &diff, &pos, &work->before_pos ) ;
	len = GV_VecLen3F( &diff );
	if( len < LEN_LIMIT ){
		DG_COPY_VEC( &work->before_pos, &pos );
		return;	// 移動量が少なすぎたら処理を止める
	}


//printf("come----------------------\n");

	if( pos.vy < work->before_pos.vy ){	// 下降
		if( pos.vy > GM_WaterLevel ){
			ftemp = pos.vy - GM_WaterLevel;
			data_radius = fpu_Sqrt( work->radius*work->radius - ftemp*ftemp );
			DG_COPY_VEC( &center, &pos );
		}else if( (pos.vy+work->radius) > GM_WaterLevel ){
			if( work->before_pos.vy > GM_WaterLevel ){
				data_radius = work->radius;
			}else if( (work->before_pos.vy+work->radius) > GM_WaterLevel ){
				ftemp = GM_WaterLevel - pos.vy;
				data_radius = fpu_Sqrt( work->radius*work->radius - ftemp*ftemp );
			}
			DG_COPY_VEC( &center, &pos );
		}else{
			if( work->before_pos.vy < GM_WaterLevel ){
				DG_COPY_VEC( &work->before_pos, &pos );
				return;	// 連続水没だった
			}
			data_radius = work->radius;
			_sceVu0ScaleVector( &fvtemp, &diff, (work->before_pos.vy - GM_WaterLevel)/(work->before_pos.vy - pos.vy) );
			_sceVu0AddVector( &center, &work->before_pos, &fvtemp ) ;

#if 0
			fvtemp.vx = center.vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = center.vz;
AN_Test_Eye2( &fvtemp, 2 );
#endif

		}
	}else if( pos.vy > work->before_pos.vy ){	// 上昇
		if( pos.vy < GM_WaterLevel ){
			ftemp = GM_WaterLevel - pos.vy;
			data_radius = fpu_Sqrt( work->radius*work->radius - ftemp*ftemp );
			DG_COPY_VEC( &center, &pos );
		}else if( (pos.vy-work->radius) < GM_WaterLevel ){
			if( work->before_pos.vy < GM_WaterLevel ){
				data_radius = work->radius;
			}else if( (work->before_pos.vy-work->radius) < GM_WaterLevel ){
				ftemp = pos.vy - GM_WaterLevel;
				data_radius = fpu_Sqrt( work->radius*work->radius - ftemp*ftemp );
			}
			DG_COPY_VEC( &center, &pos );
		}else{
			if( work->before_pos.vy > GM_WaterLevel ){
				DG_COPY_VEC( &work->before_pos, &pos );
				return;	// 連続水上だった
			}
			data_radius = work->radius;
			_sceVu0ScaleVector( &fvtemp, &diff, (GM_WaterLevel - work->before_pos.vy)/(pos.vy - work->before_pos.vy) );
			_sceVu0AddVector( &center, &work->before_pos, &fvtemp ) ;

#if 0
			fvtemp.vx = center.vx;
			fvtemp.vy = GM_WaterLevel;
			fvtemp.vz = center.vz;
AN_Test_Eye2( &fvtemp, 2 );
#endif
		}
	}


	unit_circle.vy  = 0.0f;
	local_circle.vy = GM_WaterLevel;
	for( i=0; i<SPLUSH_NUM; i++ ){
		ftemp = TPI*rnd();
		unit_circle.vx = sinf(ftemp);
		unit_circle.vz = cosf(ftemp);

		local_circle.vx = unit_circle.vx*data_radius;
		local_circle.vz = unit_circle.vz*data_radius;

		// 発生点の法線を求める（上昇最大）
		norm.vx =-unit_circle.vz;
		norm.vy = 0.0f;
		norm.vz = unit_circle.vx;
		_sceVu0SubVector( &fvtemp, &local_circle, &center ) ;
		_sceVu0Normalize( &fvtemp, &fvtemp );
		_sceVu0OuterProduct(&norm,&norm,&fvtemp);
		if( norm.vy < 0.0f ){
			norm.vx =-norm.vx;
			norm.vy =-norm.vy;
			norm.vz =-norm.vz;
		}
//		_sceVu0ScaleVector( &norm, &norm, len*0.50f );
		_sceVu0ScaleVector( &norm, &norm, len*0.75f );
//		_sceVu0ScaleVector( &norm, &norm, len );

		// 発生点
		fvtemp.vx = center.vx + local_circle.vx;
		fvtemp.vy = GM_WaterLevel;
		fvtemp.vz = center.vz + local_circle.vz;

		// 加速度
		accel.vx = len*unit_circle.vx + diff.vx  + norm.vx;
		accel.vy =              DG_FABS(diff.vy) + norm.vy;
		accel.vz = len*unit_circle.vz + diff.vz  + norm.vz;
		_sceVu0ScaleVector( &accel, &accel, 0.5f*work->intense );

		NewSplushTidalParts( &fvtemp, &accel, len*(0.25f+rnd()*0.75f) );
		OK_PutSplush( &fvtemp, &accel );
	}

	DG_COPY_VEC( &work->before_pos, &pos );
}


/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}


static int GetResources( Work *work )
{

//	work->count = (irnd()>>8)&4095;
	DG_COPY_VEC( &work->before_pos, work->pos );

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


void *NewAutoSplush( FVECTOR *pos, float radius )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->pos    = pos;
		work->radius = radius;

		work->intense = 1.0f;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

// intense: エフェクトの大きさ。1.0f で 通常
void *NewAutoSplush2( FVECTOR *pos, float radius, float intense )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->pos    = pos;
		work->radius = radius;

		work->intense = intense;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewAutoSplush_EftCtrl( int con_name, float radius )
{
	EFTCONTROL	*ctrl;
	ctrl = DM_GetEftControl( con_name );
	if( !ctrl ) return NULL;
	return NewAutoSplush( &ctrl->mov, radius );
}

