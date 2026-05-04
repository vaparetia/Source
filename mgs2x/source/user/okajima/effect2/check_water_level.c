//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	check_water_level.c
	水面監視

	1999/10/23 S.Okajima
	$Id: check_water_level.c,v 1.1.1.3 2002/11/19 11:47:19 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#include "bp_matrix.h"

/* ---------------------------------------------------------------- */
extern float GM_WaterLevel;
extern void *NewDivingFlow( FVECTOR *center, float radius, float intense, int col, int life );
extern void *NewSplushRotateSplush( FVECTOR *pos, float radius, float intense, int col );
extern void *NewRippleStrip( FVECTOR *center, float radius, int life );
extern void *NewDeathDiveSplush( FVECTOR *center, float radius, float intense );
extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense, int flag );

void *NewSplushSurface2Man( int name, int map );
/* ---------------------------------------------------------------- */
static int OK_se_count;
/* ---------------------------------------------------------------- */
//========================Satoyoshi Modified 2001.10.3

#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_90	(0.0f)

static VOLUMECURVE water_norm = {
    2,
    SE_COS_75,
    SE_COS_90,
    {6000, 14000, -1, -1},
    {0x3f, 0x38, 0x10, 0, 0}, 
    {5000, 12000, -1, -1},
    {0x3f, 0x38, 0x10, 0, 0}, 
    1.0f
};
static VOLUMECURVE water_own = {
    2,
    SE_COS_60,
    SE_COS_90,
    {0, 10000, -1, -1},
    {0x3f, 0x3f, 0x10, 0, 0}, 
    {0, 9800, -1, -1},
    {0x3f, 0x3f, 0x10, 0, 0}, 
    1.0f
};
static VOLUMECURVE water_demo = {
    2,
    SE_COS_75,
    SE_COS_90,
    {6000, 14000, -1, -1},
    {0x3f, 0x38, 0x10, 0, 0}, 
    {5000, 12000, -1, -1},
    {0x3f, 0x38, 0x10, 0, 0}, 
    1.0f
};
VOLCURVES water_curves = {
    &water_norm,
    &water_own,
    &water_demo,
    &water_norm
};
//=====================================

#define SE_INTERVAL (4)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;

	int			chara_name;
	int			off_flag;
	FVECTOR		before_mov;

} Work ;


#define	IN_OUT_BOUNDARY	(20000.0f)
/* ---------------------------------------------------------------- */
static void MakkySE( int se_code, FVECTOR *pos )
{
	FVECTOR	fvtemp;

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

	if(
		DG_FABS( GM_PlayerPosition.vx - pos->vx ) < 1.0f
	 && DG_FABS( GM_PlayerPosition.vy - pos->vy ) < 1.0f
	 && DG_FABS( GM_PlayerPosition.vz - pos->vz ) < 1.0f
	){
		// プレイヤーだった

//printf(":::::::::::::::::プレイヤーです\n");

		// 室内
		GM_SeSetMode( se_code, &fvtemp, GM_SEMODE_BOMB ) ;
		return;
	}

//printf(":::::::::::::::::%f\n",GM_PlayerPosition.vy - GM_WaterLevel);

	if( GM_PlayerPosition.vy - GM_WaterLevel < IN_OUT_BOUNDARY ){
		// 室内
		GM_SeSetMode( se_code, &fvtemp, GM_SEMODE_BOMB ) ;
	}else{
		// 室外
		GM_SeSetFromVolCurve( se_code, &fvtemp, &water_curves);
	}
}

/* ---------------------------------------------------------------- */
void SetSplushSequence3( FVECTOR *pos )
{
	FVECTOR	fvtemp;

	if( OK_se_count <= 0 ){
		OK_se_count = SE_INTERVAL;
		MakkySE( SD_P_INWTERL1, pos );
	}

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

	NewSplushRotateSplush(   &fvtemp, 500.0f, 100.0f, 0x80808020 );
	NewDeathDiveSplush(      &fvtemp, 300.0f, 1000.0f );
	NewRippleStrip( &fvtemp, 30000.0f, 600 );

}

/* ---------------------------------------------------------------- */
void SetSplushSequence4( FVECTOR *pos, float step )
{
	FVECTOR	fvtemp;

	if( OK_se_count <= 0 ){
		OK_se_count = SE_INTERVAL;
		MakkySE( SD_P_INWTERL1, pos );
	}

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

	step = DG_FABS( step );
	if( step < 200.0f ) step = 200.0f;
	if( step > 400.0f ) step = 400.0f;

	step = DG_FABS( step ) * 0.25f;

	NewDivingFlow(           &fvtemp, 1000.0f, step*1.5f, 0xa0a0a0ff, 10 );
	NewSplushRotateSplush(   &fvtemp, 500.0f,  step*2.0f, 0x80808020 );
	NewRippleStrip(          &fvtemp, 15000.0f, (int)(step)*2 );
	NewDeathDiveSplush(      &fvtemp, step*3.0f, step*18.0f );

}

/* ---------------------------------------------------------------- */
void SetSplushSequence5( FVECTOR *pos, float step )
{
	FVECTOR	fvtemp;

	MakkySE( SD_A_INWTRG01, pos );

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

	step = DG_FABS( step );
	if( step < 200.0f ) step = 200.0f;
	if( step > 500.0f ) step = 500.0f;

	step = DG_FABS( step ) * 0.25f;

	NewDivingFlow(           &fvtemp, 1000.0f, step*1.5f, 0xa0a0a0ff, 10 );
	NewSplushRotateSplush(   &fvtemp, 500.0f,  step*2.0f, 0x80808020 );
	NewRippleStrip(          &fvtemp, 15000.0f, (int)(step)*2 );
	NewDeathDiveSplush(      &fvtemp, step*3.0f, step*18.0f );

}

/* ---------------------------------------------------------------- */
void SetSplushSequence2( FVECTOR *pos )
{
	FVECTOR	fvtemp;

	if( OK_se_count <= 0 ){
		OK_se_count = SE_INTERVAL*4;
		MakkySE( SD_P_INWTERL1, pos );
	}

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

	NewDivingFlow(           &fvtemp, 1000.0f, 300.0f, 0xa0a0a0ff, 30 );
	NewSplushRotateSplush(   &fvtemp, 500.0f, 200.0f, 0x80808020 );
	NewRippleStrip(          &fvtemp, 30000.0f, 600 );
	NewDeathDiveSplush(      &fvtemp, 500.0f, 3000.0f );

}

/* ---------------------------------------------------------------- */
void SetSplushSequenceWeapon( FVECTOR *pos, float width, float intense, int flag )
{
	FVECTOR	fvtemp;

	if( OK_se_count <= 0 ){
		OK_se_count = SE_INTERVAL;
		MakkySE( SD_P_INWTERL1, pos );
	}

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

//	GM_SeSetMode( SD_A_INWTRG01, &fvtemp, GM_SEMODE_NORMAL ) ;
	NewSplushRotateSplush( &fvtemp, width, intense, 0x80808040 );
	NewRippleStrip( &fvtemp, 3000.0f, 60 );

}

/* ---------------------------------------------------------------- */
void SetSplushSequence( FVECTOR *pos, float width, float intense, int flag )
{
	FVECTOR	fvtemp;

	if( OK_se_count <= 0 ){
		OK_se_count = SE_INTERVAL;
		MakkySE( SD_P_INWTERL1, pos );
	}

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

//	GM_SeSetMode( SD_A_INWTRG01, &fvtemp, GM_SEMODE_NORMAL ) ;
	NewSplushRotateSplush( &fvtemp, width, intense, 0x80808040 );
	NewRippleStrip( &fvtemp, 3000.0f, 60 );

}

/* ---------------------------------------------------------------- */
void SetSplushSequenceBullet( FVECTOR *pos, float width, float intense, int flag )
{
	FVECTOR	fvtemp;
	FVECTOR	cam;


	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;
	NewSplushRotateSplush( &fvtemp, width, intense, 0x80808040 );
	NewRippleStrip( &fvtemp, 3000.0f, 60 );

	// マキムー監修
	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );
	_sceVu0SubVector( &fvtemp, &fvtemp, &cam );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 0.85f );
	_sceVu0AddVector( &fvtemp, &fvtemp, &cam );

//	AN_Test_Eye2( &fvtemp, 2 );

//printf("ここっすか？\n");
	GM_SeSetMode( SD_A_INWTRG01, &fvtemp, GM_SEMODE_NORMAL ) ;
//	GM_SeSetMode( SD_A_INWTRG01, &fvtemp, GM_SEMODE_BOMB ) ;

}

/* ---------------------------------------------------------------- */
void SetSplushSequenceSmallObject( FVECTOR *pos )
{
	FVECTOR	fvtemp;
	FVECTOR	cam;

	fvtemp.vx = pos->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = pos->vz;

	// マキムー監修
	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );
	_sceVu0SubVector( &fvtemp, &fvtemp, &cam );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 0.85f );
	_sceVu0AddVector( &fvtemp, &fvtemp, &cam );

//printf("ここっす\n");
	GM_SeSetMode( SD_A_INWTRG01, &fvtemp, GM_SEMODE_NORMAL ) ;
	NewSplushRotateSplush( &fvtemp, 50.0f, 50.0f, 0x80808040 );
	NewRippleStrip( &fvtemp, 3000.0f, 60 );
}


enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
//printf("work->off_flag = 1\n");
			work->off_flag = 1;
			break;
		  case REQ_ON:
//printf("work->off_flag = 0\n");
			work->off_flag = 0;
			break;
		  case REQ_KILL:
		  case -1:
			GV_DestroyActor( work ) ;
			return;
			break;
		  default:
			break;
		}
		msg--;
	}
}
/*
MDL_CHAFF_PIN_RAI
MDL_CHAFF_LEVER_RAI
MDL_STUN_PIN_RAI
MDL_STUN_LEVER_RAI
MDL_GRENADE_PIN_RAI
MDL_GRENADE_LEVER_RAI
*/
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR		fvtemp;
//	CONTROL		*ctrl;
	CONTROL		**list ;
	int			i, n ;
//	int			name;

	OK_se_count--;

	CheckMesgParam( work );

	if( work->off_flag ) return;

#if 1
	list = GM_WhereList ;
	n = GM_N_WhereList ;
	for ( i = n ; i > 0 ; -- i, list++ ) {
#if 0
		name = (*list)->name;
		if( ( name == MDL_CHAFF_PIN_RAI )
		 || ( name == MDL_CHAFF_LEVER_RAI )
		 || ( name == MDL_STUN_PIN_RAI )
		 || ( name == MDL_STUN_LEVER_RAI )
		 || ( name == MDL_GRENADE_PIN_RAI )
		 || ( name == MDL_GRENADE_LEVER_RAI )
		  ) continue;
#endif
//		printf("%d:%f %f %f:%f %f %f\n",i,(*list)->mov.vx,(*list)->mov.vy,(*list)->mov.vz,(*list)->step.vx,(*list)->step.vy,(*list)->step.vz);
		DG_COPY_VEC( &fvtemp, &(*list)->mov );
		if( fvtemp.vy < GM_WaterLevel ){
//printf("%x::%f:%f:%f\n",(*list),fvtemp.vy,(*list)->step.vy,GM_WaterLevel);
			if( fvtemp.vy - (*list)->step.vy > GM_WaterLevel ){
//printf("WATER IN!!!!!!!!%6.6f:%6.6f:%6.6f\n",fvtemp.vy,(*list)->step.vy,GM_WaterLevel);
				SetSplushSequence4( &fvtemp, (*list)->step.vy );
				GM_SetNoise( NOISE_MM, &fvtemp, (*list)->map ) ;
			}
		}
	}
#else
	ctrl = GM_SearchWhere(work->chara_name);
	if( ctrl==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}
	if( ( work->before_mov.vy > GM_WaterLevel  &&  ctrl->mov.vy < GM_WaterLevel )
	 || ( work->before_mov.vy < GM_WaterLevel  &&  ctrl->mov.vy > GM_WaterLevel ) ){

	}
	DG_COPY_VEC( &work->before_mov, &ctrl->mov );
#endif

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

static int GetResourcesScn( Work *work )
{
//	CONTROL		*ctrl;

	work->off_flag = 0;

#if 0
	work->chara_name = -1;
	if ( GCL_GetOption( 'n' ) != NULL ){
		work->chara_name = GCL_GetNextInt() ;
		ctrl = GM_SearchWhere(work->chara_name);
		if( ctrl==NULL ) return -1;
	}else{
		return -1;
	}
	DG_COPY_VEC( &work->before_mov, &ctrl->mov );
#endif

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewCheckWaterLevel( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;

		if ( GetResourcesScn( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
