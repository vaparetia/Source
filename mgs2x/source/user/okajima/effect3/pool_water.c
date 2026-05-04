//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pool_water.c
	泡立つプール（汚水処理場）

	2001/04/19 S.Okajima
	$Id: pool_water.c,v 1.1.1.3 2002/11/19 11:47:34 Yoshizawa1 Exp $
*/
//se_set

#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplush2( FVECTOR *center, SVECTOR *rot, float length );
extern void *NewVampDropSplash( FVECTOR *center, FVECTOR *vec, float radius );
extern void *NewWaterSurface( FVECTOR *center, float radius );
extern void *NewWaterSurface2( FVECTOR *center, float radius );
extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense, int flag );
extern void OK_PutKirariCross( FVECTOR *pos, float width_ratio );

extern int OK_EX0_Flag;
extern FVECTOR OK_EX0_Pos;

/*----------------------------------------------------------------*/


#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	COL_A		(32.0f)

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)

#define N_VERTS		(16)
#define N_PRIMS		(0x2000 / 16 / N_VERTS)
#define N_LOOPS		(1)
#define	TOTAL_VERTS		(N_LOOPS*N_PRIMS*N_VERTS)

#define SIZE_MIN	(300.0f)
#define SIZE_RND	(100.0f)

#define	SPEED_MIN		(20.0f)
#define	SPEED_RND		(30.0f)

#define	DEPTH			(200.0f)
#define	DEPTH_MERGINE	(400.0f)

#define	UNDER_ALPHA_LIMT	(0.001f)
//#define	STEP_ALPHA	(0.002f)
#define	STEP_ALPHA	(0.01f)

#define	STOCK_BACK	(16)

#define	RADIUS_MIN		(2000.0f)
#define	RADIUS_WIDTH	(1000.0f)
#define	RADIUS_HEIGHT	(500.0f)

#define	KIRARI_NUM	(32)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			chara_name;

	CONTROL		*control;
	FVECTOR		before_mov[STOCK_BACK];
	int			stock_count;
	int			before_flag;

	FVECTOR	bound0;
	FVECTOR	bound1;
	FVECTOR	bound_center;

	FVECTOR	center;
	FVECTOR	center_vec;
	FVECTOR diff;

	int		effect_count;

} Work ;

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
		  case 0:
		  case -1:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*pos;
	FVECTOR	fvtemp;
	FVECTOR	fvtemp1;
	float	ftemp;
	float	radius;
	int	i;
	int		path_flag;

	CheckMesgParam( work );

	if( (work->map & GM_CurrentStageMap) == 0 ){
		OK_EX0_Flag = 0;
		return;
	}

	OK_EX0_Flag = 1;
	ftemp = (float)(work->effect_count&4095)*0.01f;
	radius = ((irnd()>>8)&1)? RADIUS_WIDTH*sin( ftemp ):-RADIUS_WIDTH*sin( ftemp );
	radius = ((irnd()>>8)&1)? RADIUS_MIN+radius:-RADIUS_MIN+radius;
	ftemp = (float)(work->effect_count&4095)*0.025f;
	OK_EX0_Pos.vx = work->bound_center.vx + radius*sinf( ftemp );
	OK_EX0_Pos.vz = work->bound_center.vz + radius*cosf( ftemp );
//	ftemp = (float)(work->effect_count&4095)*0.021f;
//	OK_EX0_Pos.vy = GM_WaterLevel + RADIUS_HEIGHT*sinf( ftemp );
	OK_EX0_Pos.vy = GM_WaterLevel;

//	AN_Test_Eye2( &OK_EX0_Pos, 2 );


	work->control = NULL;
	if( work->chara_name != -1 ){
		work->control = GM_SearchWhere( work->chara_name );
	}


	if( (work->effect_count&255)==0 ){
		fvtemp.vx = work->bound_center.vx + RADIUS_MIN*frnd();
		fvtemp.vy = GM_WaterLevel;
		fvtemp.vz = work->bound_center.vz + RADIUS_MIN*frnd();
		NewWaterSurface( &fvtemp, 3000.0f+1000.0f*rnd() );
//		NewWaterSurface2( &fvtemp, 5000.0f+2500.0f*rnd() );
	}


#if 1
	if( (work->effect_count&1023)==0 ){
		fvtemp.vx = work->bound_center.vx + work->diff.vx*0.125f*frnd();
		fvtemp.vy = GM_WaterLevel;
		fvtemp.vz = work->bound_center.vz + work->diff.vz*0.125f*frnd();
//		NewWaterSurface( &fvtemp, 4000.0f+1500.0f*rnd() );
//		NewWaterSurface2( &fvtemp, 4000.0f+1500.0f*rnd() );
		NewWaterSurface2( &fvtemp, 5000.0f+2000.0f*rnd() );
	}
#endif

	for( i=0; i<KIRARI_NUM; i++ ){
		fvtemp.vx = work->bound_center.vx + work->diff.vx*0.5f*frnd();
		fvtemp.vy = GM_WaterLevel;
		fvtemp.vz = work->bound_center.vz + work->diff.vz*0.5f*frnd();
//		OK_PutKirariCross( &fvtemp, 0.7f );
//		AN_Test_Eye2( &fvtemp, 2 );
	}




	path_flag = 0;
	if( work->control == NULL ){
		path_flag = 1;
	}else if( work->control->mov.vy > GM_WaterLevel ){
		path_flag = 1;
	}else{
		work->center.vx = work->control->mov.vx;
		work->center.vz = work->control->mov.vz;


		if( ((irnd()>>8)&15)==0){
			extern void *NewBubbleMany( FVECTOR *center, int life, int col );
			OBJECT *object;
			int	col=0x80808080;
			object = (OBJECT *)(work->control + 1);
			if( object!=NULL ){
				NewBubbleMany( (FVECTOR *)object->objs->objs[(irnd()>>8)%21].world.m[3], 32, col );
			}
		}


	}

//	if( path_flag!=0 ){
	if( 1 ){
		if( ((irnd()>>8)&31)==0 || work->before_flag==0 ){
			work->center.vx = work->bound0.vx + work->diff.vx*rnd();
			work->center.vz = work->bound0.vz + work->diff.vz*rnd();
			work->center_vec.vy = 0.0f;
		}

		work->center.vx+= work->center_vec.vx*frnd();
		work->center.vz+= work->center_vec.vz*frnd();
		work->center.vx+= work->center_vec.vx;
		work->center.vz+= work->center_vec.vz;
		if( work->center.vx < work->bound0.vx ){
			work->center.vx = work->bound0.vx;
		}else if( work->center.vx > work->bound1.vx ){
			work->center.vx = work->bound1.vx;
		}
		if( work->center.vz < work->bound0.vz ){
			work->center.vz = work->bound0.vz;
		}else if( work->center.vz > work->bound1.vz ){
			work->center.vz = work->bound1.vz;
		}
	}


	if( work->control != NULL ){
		CONTROL	*ctrl = work->control;
		pos = work->before_mov;
		pos+= work->stock_count;
		if( work->before_flag != path_flag
		 && work->effect_count > 30 ){
			_sceVu0SubVector( &fvtemp, &ctrl->mov, pos ) ;

#if 1 ///
//se_set
			if( fvtemp.vy < 0.0f ){
				GM_SeSetMode( SD_E_MIZU_IN2 , &ctrl->mov, GM_SEMODE_BOMB ); //ヴァンププール上がり飛び出し
			}else{
				GM_SeSetMode( SD_E_MIZU_OT1 , &ctrl->mov, GM_SEMODE_BOMB ); //ヴァンププール飛び込み着水
			}
#endif

			fvtemp.vy = DG_FABS( fvtemp.vy );
			NewVampDropSplash( &ctrl->mov, &fvtemp, 500.0f );

			ftemp = fvtemp.vy*0.5f;
			if( !GM_CheckGameStatus( STATE_DEMO ) ){
				fvtemp1.vx = ctrl->mov.vx;
				fvtemp1.vy = GM_WaterLevel;
				fvtemp1.vz = ctrl->mov.vz;
			}
			NewWaterSurfaceMountain( &fvtemp1, 500.0f, ftemp, 0x03 );

		}

		DG_COPY_VEC( pos, &ctrl->mov );
		work->stock_count++;
		if( work->stock_count >= STOCK_BACK ) work->stock_count = 0;
	}
	work->before_flag = path_flag;


	work->effect_count++;

}

static void Die( Work *work )
{
}

static int GetResources( Work *work, int map )
{
	int	i;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	float		angle;
	float		len;

	work->effect_count = 0;

	if ( GCL_GetOption( 'b' ) != NULL ){
		for( i = 0; GCL_NextStr() != NULL; i++ ){
			fvtemp0.vx = (float)GCL_GetNextInt() ;
			fvtemp0.vy = GM_WaterLevel ;
			fvtemp0.vz = (float)GCL_GetNextInt() ;
			fvtemp1.vx = (float)GCL_GetNextInt() ;
			fvtemp1.vy = GM_WaterLevel ;
			fvtemp1.vz = (float)GCL_GetNextInt() ;
		}
	}else{
		return -1;
	}

	work->chara_name = -1;
	work->control = NULL;
	if ( GCL_GetOption( 'n' ) != NULL ){
		work->chara_name = GCL_GetNextInt() ;
		work->control = GM_SearchWhere( work->chara_name );
	}

	if( work->control!=NULL ){
		for( i=0; i<STOCK_BACK; i++ ){
			DG_COPY_VEC( &work->before_mov[i], &work->control->mov );
		}
	}

	if( fvtemp0.vx < fvtemp1.vx ){
		work->bound0.vx = fvtemp0.vx;
		work->bound1.vx = fvtemp1.vx;
	}else{
		work->bound0.vx = fvtemp1.vx;
		work->bound1.vx = fvtemp0.vx;
	}
	if( fvtemp0.vz < fvtemp1.vz ){
		work->bound0.vz = fvtemp0.vz;
		work->bound1.vz = fvtemp1.vz;
	}else{
		work->bound0.vz = fvtemp1.vz;
		work->bound1.vz = fvtemp0.vz;
	}

	_sceVu0SubVector( &work->diff, &work->bound1, &work->bound0 );
	_sceVu0ScaleVector( &fvtemp0, &work->diff, 0.5f );
	_sceVu0AddVector( &work->bound_center, &work->bound0, &fvtemp0 );


	work->center.vx = work->bound0.vx + work->diff.vx*rnd();
	work->center.vz = work->bound0.vz + work->diff.vz*rnd();

	angle = TPI*rnd();
	len   = SPEED_MIN + rnd()*SPEED_RND;
	work->center_vec.vx = len * sinf( angle );
	work->center_vec.vz = len * cosf( angle );


	{
		extern void *NewPoolWaterStable( FVECTOR *bound0, FVECTOR *bound1, int map );
		NewPoolWaterStable( &work->bound0, &work->bound1, map );
	}

	return 0 ;
}

void *NewPoolWaterComplex( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX-16 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map = map;

		if ( GetResources( work, map ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
