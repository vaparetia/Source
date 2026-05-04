//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_chaff_screen.c
	チャフスクリーンエフェクト
	2000/05/18 S.Okajima
	$Id: bomb_chaff_screen.c,v 1.1.1.3 2002/11/19 11:47:01 Yoshizawa1 Exp $

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

#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"../etc/ok_util.h"

#define	N_PARTS		(256)
#define	COMDL_RGB_MIN	(140)
#define	COMDL_RGB_RND	(8)
#define	COMDL_SPD_MIN	(1.0f)
#define	COMDL_SPD_RND	(20.0f)

#define	COMDL_SPEED_UNDER_LIMIT	(-500.0f)

#define	LIFE_TIME_PARAM0	( 1.0f )
#define	LIFE_TIME_PARAM1	( 0.2f )
#define	LIFE_TIME_PARAM2	( 0.8f )

#define	CENTER_DISTANCE		(1500.0f)
#define	BOUNDARY_WIDTH		(1000.0f)


#define	DUST_GRAVITY	( P_GRAVITY * 0.05f )

#define	ANGLE_STEP	(TPI * 0.0001f)
#define	RADIUS		(20.0f)

#define	SHIFT_HZD	(10.0f)

#define	SHIFT_UPPER	(10.0f)

#define	COL_R		(255)
#define	COL_G		(0)
#define	COL_B		(0)
#define	ALP_MAX		(255)

#define	FLASH_LENGTH	(2)
#define	FLASH_LENGTH_D	(FLASH_LENGTH * 5)

#define	VEC_SCALE		(0.001f)

#define	INIT_MULTIPLE_NUM	(2)
#define	ZOOM_RATIO			(1.012f)
#define	SEARCH_HZX			(1000)

#define	SPD_MIN		(1.0f)
#define	SPD_RND		(300.0f)
#define	DECAY_RATIO	(0.97f)

#define	WIN_MIN		(100.0f)

extern int BP_AdjustTick(int);
#define	SE_INTERVAL	(BP_AdjustTick(40))

#define	ANGLE_NEAR	(3)
#define	ANGLE_FAR	(10)

extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
extern float	OK_slow_param;
extern int big_thund_flash_flag ;
extern	int		G_wind_intense;
extern	SVECTOR	G_wind_rot;
extern	SVECTOR	OK_rain_fall_rot;
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	int			life;
	int			life_param0;
	int			life_param1;
	int			life_param2;

	FVECTOR		center;

	DG_COMDL	*comdl ;
	FVECTOR		comdl_vec[N_PARTS];
	SVECTOR		comdl_rot[N_PARTS];

	SVECTOR		wind_rot;

	int			before_count;
	int			se_count;
} Work ;

static	Work	*CHAFF_SCREEN_WORK = NULL;

/*----------------------------------------------------------------*/
static inline void AddVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2		vf1, 0(%0)
	lqc2		vf2, 0(%1)
	vadd.xyz	vf1, vf2, vf1
	sqc2		vf1, 0(%2)
	" : : "r"(a), "r"(b), "r"(r) ) ;

#else
		BP_Vec3_AddVec( r, a, b ); 
#endif
}

static inline void MulVector( FVECTOR *output, FVECTOR *input )
{
#ifdef BP_PSX2_ASM

	asm volatile ("
	lqc2			vf8,0x00(%0)
	lqc2			vf9,0x00(%1)
	vmul.xyz		vf8,vf8,vf9
	sqc2			vf8,0x00(%0)
	":  : "r" (output) ,"r"(input) :"memory" );

#else

	output->x = output->x * input->x ;
	output->y = output->y * input->y ;
	output->z = output->z * input->z ;

#endif
	return;
}


/*----------------------------------------------------------------*/
static	int	BoundAndWaterCheck( FVECTOR *pos, FVECTOR *bound_min, FVECTOR *bound_max )
{
	if( pos->vy < GM_WaterLevel ) return 0;
	return vu0_CheckBoundingBox( pos, bound_min, bound_max );
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i;
	int		alpha;
	int		check_count_max;
	float	ftemp;
	FVECTOR	*pos;
	FVECTOR	bound_center;
	FVECTOR	bound_min;
	FVECTOR	bound_max;
	FVECTOR			*comdl_vec;
	DG_COMDL_POS	*comdl_pos;
	SVECTOR			*comdl_rot;
	FVECTOR wind;


	ftemp = DG_Chanls->screen - ANGLE_NEAR;
	if( ftemp < 0.0f ){
		alpha = 128;
	}else if( ftemp > ANGLE_FAR ){
		alpha = 0;
	}else{
		alpha = (int)(128.0f * (ANGLE_FAR - ftemp) / ANGLE_FAR);
	}



	wind.vx=0.0f;
	wind.vz=0.0f;
	if( G_wind_intense*0.05f > 0.0f){
		wind.vy = -G_wind_intense*0.05f;
	}else{
		wind.vy = 0.0f;
	}

	OK_DirectionSmoother( &work->wind_rot, &G_wind_rot, 0.99f - 0.04f*OK_slow_param );
	DG_SetPos2( &DG_ZeroVector, &work->wind_rot );
	DG_PutVector( &wind, &wind, 1 );

	bound_center.vx = 0.0f;
	bound_center.vy = 0.0f;
	bound_center.vz = CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &bound_center, &bound_center, 1 );

	if( work->se_count%SE_INTERVAL == 0  &&  work->life>120 ) GM_SeSetMode( SD_W_CHAF0003, &bound_center, GM_SEMODE_BOMB ) ;
	work->se_count++;


	bound_min.vx = bound_center.vx - BOUNDARY_WIDTH;
	bound_min.vy = bound_center.vy - BOUNDARY_WIDTH;
	bound_min.vz = bound_center.vz - BOUNDARY_WIDTH;
	bound_max.vx = bound_center.vx + BOUNDARY_WIDTH;
	bound_max.vy = bound_center.vy + BOUNDARY_WIDTH;
	bound_max.vz = bound_center.vz + BOUNDARY_WIDTH;

	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

	if( work->life == work->life_param0 && work->before_count!=0 ){ //前回未終了からの復帰
		comdl_pos = work->comdl->pos;
		comdl_rot = work->comdl_rot;
		comdl_pos += work->before_count;
		comdl_rot += work->before_count;
		for ( i=work->before_count; i<N_PARTS; i++ ){
			DG_SetPos2( &work->center, comdl_rot );
			DG_GetPos( &comdl_pos->world );
			comdl_pos++;
			comdl_rot++;
		}
	}


	comdl_vec = work->comdl_vec;
	comdl_pos = work->comdl->pos;
	comdl_rot = work->comdl_rot;
	if( work->life > work->life_param1 ){	// 画面前に表示を移行させるフェーズ
		check_count_max = N_PARTS *  (work->life_param0 - work->life) / (work->life_param0 - work->life_param1);
		for ( i=0; i<check_count_max; i++ ){
			comdl_pos->color.vw = alpha ;
			pos = (FVECTOR *)comdl_pos->world.m[3];
			if( !BoundAndWaterCheck( pos, &bound_min, &bound_max ) ){
				pos->vx = bound_center.vx + frnd() * BOUNDARY_WIDTH;
				pos->vy = bound_center.vy + frnd() * BOUNDARY_WIDTH;
				pos->vz = bound_center.vz + frnd() * BOUNDARY_WIDTH;
				comdl_vec->vy = 0;
				if( pos->vy < GM_WaterLevel ){
					pos->vy = bound_center.vy + BOUNDARY_WIDTH;
					comdl_pos->color.vw = 0 ;
				}
			}
			AddVector( pos, pos, comdl_vec );
			AddVector( pos, pos, &wind );
			comdl_vec->vy += (DUST_GRAVITY);
			comdl_rot->vz += 200;
			DG_SetPos2( pos, comdl_rot );
			DG_GetPos( &comdl_pos->world );

			comdl_pos++;
			comdl_vec++;
			comdl_rot++;
		}
		for ( i=check_count_max; i<N_PARTS; i++ ){
			pos = (FVECTOR *)comdl_pos->world.m[3];
			AddVector( pos, pos, comdl_vec );
			AddVector( pos, pos, &wind );
			comdl_vec->vy += (DUST_GRAVITY);
			comdl_rot->vz += 200;
			DG_SetPos2( pos, comdl_rot );
			DG_GetPos( &comdl_pos->world );

			comdl_pos->color.vw = alpha ;

			comdl_pos++;
			comdl_vec++;
			comdl_rot++;
		}
		work->before_count = 0;
	}else if( work->life < work->life_param0 - work->life_param2 ){	// 消すフェーズ
#if 0
		for ( i=0; i<check_count_max; i++ ){
			pos = (FVECTOR *)comdl_pos->world.m[3];
			if( !BoundAndWaterCheck( pos, &bound_min, &bound_max ) ){
				pos->vx = bound_center.vx + frnd() * BOUNDARY_WIDTH;
				pos->vy = bound_center.vy + frnd() * BOUNDARY_WIDTH;
				pos->vz = bound_center.vz + frnd() * BOUNDARY_WIDTH;
				comdl_vec->vy = 0;
				if( pos->vy < GM_WaterLevel ) pos->vy = bound_center.vy + BOUNDARY_WIDTH;
			}
			AddVector( pos, pos, comdl_vec );
			AddVector( pos, pos, &wind );
			comdl_vec->vy += (DUST_GRAVITY);
			comdl_rot->vz += 200;
			DG_SetPos2( pos, comdl_rot );
			DG_GetPos( &comdl_pos->world );
			comdl_pos++;
			comdl_vec++;
			comdl_rot++;
		}
		DG_COPY_MAT( &mat, &work->comdl->pos->world );
		for ( i=check_count_max; i<N_PARTS; i++ ){
			DG_COPY_MAT( &comdl_pos->world, &mat );
			comdl_pos++;
		}
		work->before_count = check_count_max;
#else
		for ( i=0; i<N_PARTS; i++ ){
			pos = (FVECTOR *)comdl_pos->world.m[3];
			if( !BoundAndWaterCheck( pos, &bound_min, &bound_max ) ){
				comdl_pos->color.vw = 0 ;
				if( pos->vy < GM_WaterLevel ) pos->vy = bound_center.vy + BOUNDARY_WIDTH;
			}
			AddVector( pos, pos, comdl_vec );
			AddVector( pos, pos, &wind );
			comdl_vec->vy += (DUST_GRAVITY);
			comdl_rot->vz += 200;
			DG_SetPos2( pos, comdl_rot );
			DG_GetPos( &comdl_pos->world );
			comdl_pos++;
			comdl_vec++;
			comdl_rot++;
		}
		work->before_count = 0;
#endif
	}else{
		for ( i=0; i<N_PARTS; i++ ){
			pos = (FVECTOR *)comdl_pos->world.m[3];
			if( !BoundAndWaterCheck( pos, &bound_min, &bound_max ) ){
				pos->vx = bound_center.vx + frnd() * BOUNDARY_WIDTH;
				pos->vy = bound_center.vy + frnd() * BOUNDARY_WIDTH;
				pos->vz = bound_center.vz + frnd() * BOUNDARY_WIDTH;
				comdl_vec->vy = 0;
				if( pos->vy < GM_WaterLevel ){
					pos->vy = bound_center.vy + BOUNDARY_WIDTH;
					comdl_pos->color.vw = 0 ;
				}
			}
			AddVector( pos, pos, comdl_vec );
			AddVector( pos, pos, &wind );
			comdl_vec->vy += (DUST_GRAVITY);
			comdl_rot->vz += 200;
			DG_SetPos2( pos, comdl_rot );
			DG_GetPos( &comdl_pos->world );
			comdl_pos++;
			comdl_vec++;
			comdl_rot++;
		}
		work->before_count = 0;
	}

	work->life--;
	if( work->life <= -1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	DG_DequeueComdlObjs( work->comdl );
	DG_FreeComdl( work->comdl );

	CHAFF_SCREEN_WORK = NULL;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos )
{
	int			i;
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	DG_DEF		*def ;
	DG_COMDL	*comdl ;
	DG_COMDL_POS	*comdl_pos;
	SVECTOR			*comdl_rot;
	FVECTOR			*comdl_vec;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( 14621904 /*"cgr_frg1_cm"*/, 'k' ) ) ;
	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, N_PARTS, 0 );
	if( comdl==NULL ) return -1;
	DG_QueueComdlObjs( comdl );

	comdl_pos = comdl->pos;
	comdl_rot = work->comdl_rot;
	comdl_vec = work->comdl_vec;
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	svtemp.vz = 0;
	for ( i=0; i<N_PARTS; i++ ){
		comdl_rot->vx = irnd()%4096;
		comdl_rot->vy = irnd()%2048;
		comdl_rot->vz = 0;
		DG_SetPos2( pos, comdl_rot ) ;
		DG_GetPos( &comdl_pos->world );
		comdl_pos->color.vx = irnd()%COMDL_RGB_RND + COMDL_RGB_MIN;
		comdl_pos->color.vy = irnd()%COMDL_RGB_RND + COMDL_RGB_MIN;
		comdl_pos->color.vz = irnd()%COMDL_RGB_RND + COMDL_RGB_MIN;
		comdl_pos->color.vw = 128 ;

		fvtemp.vz = COMDL_SPD_MIN + COMDL_SPD_RND*rnd();
		svtemp.vx = irnd()%4096;
		svtemp.vy = irnd()%2048;
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( &fvtemp, comdl_vec, 1 );
		comdl_vec->vw = TPI * rnd();

		comdl_vec++;
		comdl_pos++;
		comdl_rot++;
	}

	work->se_count = 0;

	return 0 ;
}

void *NewChaffEffectScreen( FVECTOR *pos, int life )
{
	Work		*work ;
	float		ftemp;

	OPERATOR() ;


	life += DIRECT_TICK( 180 );

	if( CHAFF_SCREEN_WORK != NULL ){
		CHAFF_SCREEN_WORK->life        = life;
		ftemp = (float)life;
		CHAFF_SCREEN_WORK->life_param0 = (int)(ftemp * LIFE_TIME_PARAM0);
		CHAFF_SCREEN_WORK->life_param1 = (int)(ftemp * LIFE_TIME_PARAM1);
		CHAFF_SCREEN_WORK->life_param2 = (int)(ftemp * LIFE_TIME_PARAM2);
		DG_COPY_VEC( &CHAFF_SCREEN_WORK->center, pos );
		return NULL ;
	}

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		CHAFF_SCREEN_WORK = work;
		work->life = life;
		ftemp = (float)life;
		work->life_param0 = (int)(ftemp * LIFE_TIME_PARAM0);
		work->life_param1 = (int)(ftemp * LIFE_TIME_PARAM1);
		work->life_param2 = (int)(ftemp * LIFE_TIME_PARAM2);

		work->before_count = 0;
		DG_COPY_VEC( &work->center, pos );

		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

