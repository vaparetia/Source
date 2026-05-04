//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	c_box_parts.c
	ダンボールのフタのちぎれ落下
	2000/02/02 S.Okajima
	$Id: c_box_parts.c,v 1.1.1.3 2002/11/19 11:47:02 Yoshizawa1 Exp $
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

#define	MAX_ROTS	(4) /* 関節は最大４ */
#define	STEP_RATIO	(0.3f)
#define	BOX_GRAVITY	(1.0f)
#define	VEC_LIMIT	(-100.0f)
#define	FROM_UNDER_LIMIT	(-32000.0f)

extern void *NewCB_WetEffect( DG_OBJS *, int ) ;
extern void CB_WetEffectAdd( void *, int, FVECTOR *, float ) ;
extern int OK_BodySplashFlag;
extern int	OK_FloorOffFlag;

typedef	struct	{
	GV_ACT_EX	actor ;
	int			map;
	int			check_flag;
	int			model_name;
	int			wet_level;
	int			wet_level_max;
	float		under_limit;
	float		vec;
	SVECTOR		floor_rot;
	OBJECT		object ;
	FVECTOR		rots[MAX_ROTS];
	FMATRIX		lights[ 2 ] ;
	FMATRIX		world;
	void		*wet_work;
} Work ;


/*----------------------------------------------------------------*/
/* floor についての rotを得る */
static	void	CalcFloorRot( Work *work )
{
	FVECTOR	to;
	int	floor_flag;
	float		flr_height[2];
	HZX_FLR		flr[2];
	int			flr_atrs[2];

	floor_flag = HZX_LevelHazardCheck(
	                           GM_GetHzxGroupID( work->map ),
	                           (FVECTOR *)work->world.m[3],
	                           HZX_CHK_F_FLOOR,
	                           HZX_FLOOR_NO_PLAYER );
//printf("%d  :%f %f %f\n",GM_GetHzxGroupID( GM_CurrentStageMap ),work->world.m[3][0],work->world.m[3][1],work->world.m[3][2]);
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
		work->under_limit = flr_height[0] + 40.0f;
	}else{
		work->floor_rot.vx = 0;
		work->floor_rot.vy = 0;
		work->floor_rot.vz = 0;
		work->under_limit = FROM_UNDER_LIMIT;
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
	if( work->wet_work != NULL )CB_WetEffectAdd( work->wet_work, 0, (FVECTOR *)work->world.m[3], 300.0f ) ;

	work->vec -= BOX_GRAVITY;
	if( work->vec < VEC_LIMIT ) work->vec = VEC_LIMIT;
	work->world.m[3][1] += work->vec;
	if( work->world.m[3][1] < work->under_limit ) work->world.m[3][1] = work->under_limit;

}

static	void Die( Work *work )
{
	GM_FreeObject( &work->object ) ;
}

static	int GetResources( Work *work )
{

	work->check_flag = 0;

	CalcFloorRot( work );

	work->vec = 0.0f;

	GM_InitObject( &work->object, work->model_name, DG_FLAG_SHADE ) ;
	GM_ConfigObjectJoint( &work->object, work->rots );
	DG_GetLightMatrix( (FVECTOR *)work->world.m[3], work->lights );
	GM_ConfigObjectLight( &work->object, work->lights );

	if( OK_BodySplashFlag  &&  work->wet_level ){
		work->wet_work = NewCB_WetEffect( work->object.objs, work->model_name ) ;
	}




	return 0 ;
}

void *NewBoxPartsFall( FMATRIX *mat, FVECTOR *frots, int model_name, int map, int level, int max_level )
{
	Work		*work ;
	FVECTOR		fvtemp;
	int			i;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_MAT( &work->world, mat );
		DG_COPY_VEC( (FVECTOR *)work->world.m[3], &DG_ZeroVector );
		if( (irnd()>>8)&1 ){
			_sceVu0RotMatrixX( &work->world, &work->world, PI*(0.2f+0.1f*rnd()) );
		}else{
			_sceVu0RotMatrixX( &work->world, &work->world,-PI*(0.2f+0.1f*rnd()) );
		}
		DG_COPY_VEC( (FVECTOR *)work->world.m[3], (FVECTOR *)mat->m[3] );


		for( i=0; i<MAX_ROTS; i++ ){
			DG_COPY_VEC( &work->rots[i], &DG_ZeroVector );
		}
		DG_COPY_VEC( &fvtemp, frots );
		fvtemp.vx = -fvtemp.vx;
		fvtemp.vy = -fvtemp.vy;
		fvtemp.vz = -fvtemp.vz;
		DG_COPY_VEC( &work->rots[1], &fvtemp );	// 第一関節だけ
		work->model_name = model_name;
		work->map = map;
		work->wet_level     = level;
		work->wet_level_max = max_level;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
