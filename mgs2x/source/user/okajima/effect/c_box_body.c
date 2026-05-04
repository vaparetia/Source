//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	c_box_parts.c
	物体落下跳ね返り
	2000/02/02 S.Okajima
	$Id: c_box_body.c,v 1.1.1.3 2002/11/19 11:47:02 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../../shibata/util/ts_util.h"
#include	"../etc/ok_util.h"

#define	MAX_ROTS	(30)
#define	STEP_RATIO	(0.5f)
#define	BOX_GRAVITY	(4.0f)
#define	VEC_LIMIT	(-100.0f)
#define	FROM_UNDER_LIMIT	(-32000.0f)

#define	INIT_BREAK_COUNT	(300)

//#define	SHIFT_CENTER	(2.0f)
//#define	SHIFT_CENTER	(350.0f)
#define	SHIFT_CENTER		(220.0f)
#define	ADD_RATE	(0.0015f)

#define	HEIGHT		(800.0f)
#define	HEIGHT_CHILD	(350.0f)
#define	HEIGHT_CHILD2	(200.0f)
#define	WIDTH_0		(700.0f)
#define	WIDTH_1		(900.0f)

#define	SHIFT_BREAK	(30.0f)
#define	ADJUST_RATE	(TPI*0.002f)

extern void CB_WetEffectChangeObjs( void *_work, DG_OBJS *objs );
extern void *OK_cb_wet;		/* 濡れエフェクトワークへのポインタ */
extern int  FireBoxFlag;
extern int	OK_FloorOffFlag;

typedef	struct	{
	GV_ACT_EX	actor ;
	int			map;
	int			count;
	int			break_flag;
	int			body_name;
	int			joint_num;
	int			joint_flag[4];
	int			invisible_flags;
	int			check_flag;
    int			fire_count;
    int 		fire_flag;
	float		under_limit;
	float		crush_rate;
	float		vec;
	SVECTOR		floor_rot;
	OBJECT		object ;
	FVECTOR		fvmemory;
	FVECTOR		movs[MAX_ROTS];
	FVECTOR		rots[MAX_ROTS];
	FMATRIX		lights[ 2 ] ;
	FMATRIX		world;

} Work ;


/*----------------------------------------------------------------*/
/* floor についての rotを得る */
static	void	CalcFloorRot( Work *work )
{
	FVECTOR	to;
	FVECTOR	upper_shift;
	int	floor_flag;
	float		flr_height[2];
	HZX_FLR		flr[2];
	int			flr_atrs[2];

	DG_COPY_VEC( &upper_shift, (FVECTOR *)work->world.m[3] );
	upper_shift.vy += SHIFT_CENTER;

	floor_flag = HZX_LevelHazardCheck(
	                 GM_GetHzxGroupID( work->map ),
	                 &upper_shift,
	                 HZX_CHK_F_FLOOR,
	                 HZX_FLOOR_NO_PLAYER );
	if( floor_flag & 1 ){
		HZX_GetLevelHazard( flr, flr_atrs );
		HZX_GetLevelHeight( flr_height );
/*
		to.vx=flr->p1.h;
		to.vy=flr->p3.h;
		to.vz=flr->p2.h;
*/
		//法線を９０度回転させる
		to.vx=flr->p1.h;
		to.vy=flr->p2.h;
		to.vz=flr->p3.h;
		OK_DirVecXY( &DG_ZeroVector, &to, &work->floor_rot );
		work->under_limit = flr_height[0] + SHIFT_CENTER;
	}else{
		work->floor_rot.vx = 0;
		work->floor_rot.vy = 0;
		work->floor_rot.vz = 0;
		work->under_limit = FROM_UNDER_LIMIT;
	}
}


static int CheckBoxBoundaryLine( Work *work, int num )
{
	int	flag;
	FVECTOR	fvtemp[2];

	switch( num ){
	  case 0:
		fvtemp[0].vx =  SHIFT_BREAK;
		fvtemp[0].vy =  HEIGHT;
		fvtemp[0].vz =  WIDTH_1*0.5f;
		fvtemp[1].vx =  SHIFT_BREAK;
		fvtemp[1].vy =  HEIGHT;
		fvtemp[1].vz = -WIDTH_1*0.5f;
		DG_SetPos( &work->object.objs->objs[5].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	  case 1:
		fvtemp[0].vx = -SHIFT_BREAK;
		fvtemp[0].vy =  HEIGHT;
		fvtemp[0].vz =  WIDTH_1*0.5f;
		fvtemp[1].vx = -SHIFT_BREAK;
		fvtemp[1].vy =  HEIGHT;
		fvtemp[1].vz = -WIDTH_1*0.5f;
		DG_SetPos( &work->object.objs->objs[7].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	  case 2:
		fvtemp[0].vx =  WIDTH_0*0.5f;
		fvtemp[0].vy =  HEIGHT;
		fvtemp[0].vz =  SHIFT_BREAK;
		fvtemp[1].vx = -WIDTH_0*0.5f;
		fvtemp[1].vy =  HEIGHT;
		fvtemp[1].vz =  SHIFT_BREAK;
		DG_SetPos( &work->object.objs->objs[9].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	  case 3:
		fvtemp[0].vx =  WIDTH_0*0.5f;
		fvtemp[0].vy =  HEIGHT;
		fvtemp[0].vz = -SHIFT_BREAK;
		fvtemp[1].vx = -WIDTH_0*0.5f;
		fvtemp[1].vy =  HEIGHT;
		fvtemp[1].vz = -SHIFT_BREAK;
		DG_SetPos( &work->object.objs->objs[11].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;



	  case 4:
		fvtemp[0].vx = -HEIGHT_CHILD;
		fvtemp[0].vy =  SHIFT_BREAK;
		fvtemp[0].vz =  WIDTH_1*0.5f;
		fvtemp[1].vx = -HEIGHT_CHILD;
		fvtemp[1].vy =  SHIFT_BREAK;
		fvtemp[1].vz = -WIDTH_1*0.5f;
		DG_SetPos( &work->object.objs->objs[6].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	  case 5:
		fvtemp[0].vx =  HEIGHT_CHILD;
		fvtemp[0].vy =  SHIFT_BREAK;
		fvtemp[0].vz =  WIDTH_1*0.5f;
		fvtemp[1].vx =  HEIGHT_CHILD;
		fvtemp[1].vy =  SHIFT_BREAK;
		fvtemp[1].vz = -WIDTH_1*0.5f;
		DG_SetPos( &work->object.objs->objs[8].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	  case 6:
		fvtemp[0].vx =  WIDTH_0*0.5f;
		fvtemp[0].vy =  SHIFT_BREAK;
		fvtemp[0].vz = -HEIGHT_CHILD2;
		fvtemp[1].vx = -WIDTH_0*0.5f;
		fvtemp[1].vy =  SHIFT_BREAK;
		fvtemp[1].vz = -HEIGHT_CHILD2;
		DG_SetPos( &work->object.objs->objs[10].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	  case 7:
		fvtemp[0].vx =  WIDTH_0*0.5f;
		fvtemp[0].vy =  SHIFT_BREAK;
		fvtemp[0].vz =  HEIGHT_CHILD2;
		fvtemp[1].vx = -WIDTH_0*0.5f;
		fvtemp[1].vy =  SHIFT_BREAK;
		fvtemp[1].vz =  HEIGHT_CHILD2;
		DG_SetPos( &work->object.objs->objs[12].world );
		DG_PutVector( fvtemp, fvtemp, 2 );
		break;
	}

	flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
		GM_GetHzxGroupID( work->map ),
		&fvtemp[0],
		&fvtemp[1],
		HZX_CHK_F_SEGMENT,
		HZX_SEG_NO_PLAYER,
		HZX_FLOOR_NO_PLAYER );

	return flag;
}

static void CrushBoard( Work *work )
{
	float	rate0, rate1;

	work->crush_rate += ADD_RATE;

	rate0 = work->crush_rate;
	rate1 = 1.0f - rate0;
	if( !CheckBoxBoundaryLine( work, 0 ) ){
		work->rots[ 5].vz = work->rots[ 5].vz*rate1 - TPI*0.24f*rate0;
		if( !CheckBoxBoundaryLine( work, 4 ) ){
			if( !work->joint_flag[0] ){
				work->rots[ 6].vz = work->rots[ 6].vz*rate1 - TPI*0.24f*rate0;
			}
		}else{
			work->joint_flag[0] = 1;
			work->rots[ 6].vz += ADJUST_RATE;
		}
	}else{
		work->rots[ 6].vz = work->rots[ 6].vz*rate1 + TPI*0.125f*rate0;
	}

	rate0 = work->crush_rate;
	rate1 = 1.0f - rate0;
	if( !CheckBoxBoundaryLine( work, 1 ) ){
		work->rots[ 7].vz = work->rots[ 7].vz*rate1 + TPI*0.24f*rate0;
		if( !CheckBoxBoundaryLine( work, 5 ) ){
			if( !work->joint_flag[1] ){
				work->rots[ 8].vz = work->rots[ 8].vz*rate1 + TPI*0.24f*rate0;
			}
		}else{
			work->joint_flag[1] = 1;
			work->rots[ 8].vz -= ADJUST_RATE;
		}
	}else{
		work->rots[ 8].vz = work->rots[ 8].vz*rate1 - TPI*0.125f*rate0;
	}

	rate0 = work->crush_rate*0.7f;
	rate1 = 1.0f - rate0;
	if( !CheckBoxBoundaryLine( work, 2 ) ){
		work->rots[ 9].vx = work->rots[ 9].vx*rate1 + TPI*0.24f*rate0;
		if( !CheckBoxBoundaryLine( work, 6 ) ){
			if( !work->joint_flag[2] ){
				work->rots[10].vx = work->rots[10].vx*rate1 + TPI*0.24f*rate0;
			}
		}else{
			work->joint_flag[2] = 1;
			work->rots[10].vx -= ADJUST_RATE;
		}
	}else{
		work->rots[10].vx = work->rots[10].vx*rate1 - TPI*0.125f*rate0;
	}

	rate0 = work->crush_rate*0.7f;
	rate1 = 1.0f - rate0;
	if( !CheckBoxBoundaryLine( work, 3 ) ){
		work->rots[11].vx = work->rots[11].vx*rate1 - TPI*0.24f*rate0;
		if( !CheckBoxBoundaryLine( work, 7 ) ){
			if( !work->joint_flag[3] ){
				work->rots[12].vx = work->rots[12].vx*rate1 - TPI*0.24f*rate0;
			}
		}else{
			work->joint_flag[3] = 1;
			work->rots[12].vx += ADJUST_RATE;
		}
	}else{
		work->rots[12].vx = work->rots[12].vx*rate1 + TPI*0.125f*rate0;
	}

}

static	void Act( Work *work )
{
	SVECTOR	svtemp;
	SVECTOR	svdest;

	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		if( OK_FloorOffFlag ){
			work->check_flag = 1;
		}
	}

	if( work->check_flag ){
		if( UTL_EFT_CheckBound( (FVECTOR *)work->world.m[3] ) >= 0 ){
			GV_DestroyActor( work ) ;
			return;
		}
	}

	switch( work->break_flag ){
		case 0:
			work->vec -= BOX_GRAVITY;
			if( work->vec < VEC_LIMIT ) work->vec = VEC_LIMIT;
			work->world.m[3][1] += work->vec;
			if( work->world.m[3][1] < work->under_limit ){
			    work->break_flag = 1;	//分解開始
				work->world.m[3][1] = work->under_limit;

			    // 電撃床燃えフラグ確保しておく
			    //work->fire_flag = FireBoxFlag;
			    // 電撃床燃えフラグたっていたら炎呼び出し
			    if ( work->fire_flag ) {
					extern void *NewBoxFire( FMATRIX *mat, FVECTOR *bound_max, FVECTOR *bound_min,
											 int *break_flag, int se_flag );
					int	i;
					
					for( i=5; i<13; i++  ){
						if ( i==11 ) {
							NewBoxFire( &work->object.objs->objs[i].world,
										&work->object.objs->objs[i].bound_max, 
										&work->object.objs->objs[i].bound_min,
										&work->break_flag,
										1 );
						}
						else {
							NewBoxFire( &work->object.objs->objs[i].world,
										&work->object.objs->objs[i].bound_max, 
										&work->object.objs->objs[i].bound_min,
										&work->break_flag,
										0 );
						}
					}
			    }
			    
			}

			DG_SetPos( &work->world );
			GM_ActObject( &work->object );

			TS_MatToRot( &svtemp, &work->world );
			svdest.vx = work->floor_rot.vx;
			svdest.vy = work->floor_rot.vy;
			svdest.vz = work->floor_rot.vz;
			OK_DirectionSmoother( &svdest, &svtemp, STEP_RATIO );
			svdest.vx =-svdest.vx ;
			svdest.vy = 0;
			svdest.vz =-svdest.vz;
			DG_RotatePos( &svdest );
			DG_GetPos( &work->world );

			work->count = INIT_BREAK_COUNT;
			work->crush_rate = 0.0f;
			DG_COPY_VEC( &work->fvmemory, (FVECTOR *)work->world.m[3] );
			break;
		case 1:
			if( work->count > 0 ){

				if ( work->fire_flag && work->count == INIT_BREAK_COUNT-5 ) {
					DG_InvisibleObjs( work->object.objs );					
				}
				
				work->count--;

				CrushBoard( work );
			}else{
			    work->break_flag = 2;	//定常状態

				   if ( work->fire_flag ) {
					   GV_DestroyActor( work );			      
				   }

			}


			DG_SetPos( &work->world );
			GM_ActObject( &work->object );

			TS_MatToRot( &svtemp, &work->world );
			svdest.vx = work->floor_rot.vx;
			svdest.vy = work->floor_rot.vy;
			svdest.vz = work->floor_rot.vz;
			OK_DirectionSmoother( &svdest, &svtemp, STEP_RATIO );
			svdest.vx =-svdest.vx ;
			svdest.vy = 0;
			svdest.vz =-svdest.vz;
			DG_RotatePos( &svdest );
			DG_GetPos( &work->world );

			break;
		case 2:
		default:
			GM_GroupObjs( work->object.objs, GM_CurrentStageMap );

#if 0
			{
				extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
				int	i;
				for( i=5; i<13; i++  ){
					NewBoundingBoxView_1(
						&work->object.objs->objs[i].world,
						(float *)&work->object.objs->objs[i].bound_max,
						(float *)&work->object.objs->objs[i].bound_min,
						0x00ff0000 );
					
				}
			}
#endif
			break;
	}
}

static	void Die( Work *work )
{
	GM_FreeObject( &work->object ) ;

//ダンボール本体がデストロイするのでここではしない
/*
	if( OK_cb_wet != NULL ){
		if( !(GV_IsStageDestroy( work )) ){
			GV_DestroyActorQuick( OK_cb_wet ) ;
		}
	}
*/
}

static	int GetResources( Work *work )
{
	int	i;
	int	flag;
	DG_OBJ		*obj;

	work->check_flag = 0;

	work->joint_flag[0] = 0;
	work->joint_flag[1] = 0;
	work->joint_flag[2] = 0;
	work->joint_flag[3] = 0;

	work->break_flag = 0;

	CalcFloorRot( work );

	work->vec = 0.0f;

	GM_InitObject( &work->object, work->body_name, DG_FLAG_SHADE ) ;
	if( OK_cb_wet != NULL ) CB_WetEffectChangeObjs( OK_cb_wet, work->object.objs );	// 水濡れコピー
	GM_ConfigObjectJoint( &work->object, work->rots );
	DG_GetLightMatrix( (FVECTOR *)work->world.m[3], work->lights );
	GM_ConfigObjectLight( &work->object, work->lights );

	//同一モデル内での関節のＯＮ・ＯＦＦ
	flag = work->invisible_flags;
	obj = work->object.objs->objs;
	for( i=0; i<work->joint_num; i++ ){
		if( !(flag & 1) ){
			obj->flag |= DG_FLAG_INVISIBLE;
		}else{
			obj->flag &=~DG_FLAG_INVISIBLE;
		}
		flag >>= 1;
		obj++;
	}

	work->fire_flag = FireBoxFlag;
	FireBoxFlag = 0;

	return 0 ;
}

void *NewBreakBoxBody( int body_name, FMATRIX *world, FVECTOR *rots, int joint_num, int invisible_flags, int map )
{
	Work		*work ;
	int			i;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->map = map;
		DG_COPY_MAT( &work->world, world );
		work->body_name = body_name;
		work->joint_num = joint_num;
		work->invisible_flags = invisible_flags;
		for( i=0; i<work->joint_num; i++ ){
			DG_COPY_VEC( &work->rots[i], &rots[i] );
		}

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
