//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash_motion.c
	モーション連動水飛沫（呼び出し口）
	2000/04/09 S.Okajima
	$Id: d_splash_motion.c,v 1.1.1.3 2002/11/19 11:46:55 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
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

#define		JOINT_NUM	(22)

#define		STEP_LIMIT	(100.0f)

extern void *NewSplashPartsSlow_Demo( FVECTOR *center, SVECTOR *rot, float intense );

//extern int OK_rain_on_off_flag;
extern int OK_foot_splash_flag;

static	int	joint_joint[JOINT_NUM] = {
	0,	/* 00 */
	0,	/* 01 */
	1,	/* 02 */
	2,	/* 03 */
	3,	/* 04 */
	4,	/* 05 */
	5,	/* 06 */
	2,	/* 07 */
	7,	/* 08 */
	8,	/* 09 */
	9,	/* 10 */
	2,	/* 11 */
	11,	/* 12 */
	0,	/* 13 */
	13,	/* 14 */
	14,	/* 15 */
	15,	/* 16 */
	0,	/* 17 */
	17,	/* 18 */
	18,	/* 19 */
	19,	/* 20 */
	0	/* 21 */
};

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		new_vec[ JOINT_NUM ];
	FVECTOR		old_vec[ JOINT_NUM ];
	DG_OBJS		*objs;
	int			n_models;
	int			life;
	float		step_limit;
} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i ;
	int		*joint_num;
	DG_OBJ	*obj;
	FVECTOR	*fpvec0;
	FVECTOR	*fpvec1;
	FVECTOR	fvtemp;
	SVECTOR	rot;
	float	len;

	if( work->objs==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}

	if( OK_foot_splash_flag==0 && !GM_CheckGameStatus( STATE_DEMO ) ){
		obj    = work->objs->objs;
		fpvec0 = work->old_vec;
		for( i=0; i<work->n_models; i++ ){
			DG_COPY_VEC( fpvec0, (FVECTOR *)obj->world.m[3] );
			fpvec0++;
			obj++;
		}
		return;
	}

	obj    = work->objs->objs;
	fpvec0 = work->new_vec;
	for( i=0; i<work->n_models; i++ ){
		DG_COPY_VEC( fpvec0, (FVECTOR *)obj->world.m[3] );
		fpvec0++;
		obj++;
	}

	fpvec0 = work->new_vec;
	fpvec1 = work->old_vec;
	joint_num = joint_joint;
	for( i=0; i<work->n_models; i++ ){
		if(
		     i==5
		  || i==6
		  || i==9
		  || i==10
		  || i==14
		  || i==16
		  || i==18
		  || i==20
		){
			fvtemp.vx = fpvec0->vx - fpvec1->vx;
			fvtemp.vy = fpvec0->vy - fpvec1->vy;
			fvtemp.vz = fpvec0->vz - fpvec1->vz;
			len = GV_VecLen3F( &fvtemp );
//printf("%f:::%f\n",len,work->step_limit);
			if( len > work->step_limit ){ 
//printf("ok:a:%d:%f\n",i,len);
//printf("ok:b:%f:%f:%f\n",fpvec0->vx,fpvec0->vy,fpvec0->vz);
//printf("ok:c:%f:%f:%f\n",fpvec1->vx,fpvec1->vy,fpvec1->vz);

				OK_DirVecXY( &fpvec0[ (*joint_num) ], fpvec0, &rot );
//printf("ok:%f:::%f:%f:%f\n",len,fpvec0->vx,fpvec0->vy,fpvec0->vz);
				NewSplashPartsSlow_Demo( fpvec0, &rot, len*2.0f );
/*
				fvtemp.vx = fvtemp.vx * 0.5f + fpvec1->vx;
				fvtemp.vy = fvtemp.vy * 0.5f + fpvec1->vy;
				fvtemp.vz = fvtemp.vz * 0.5f + fpvec1->vz;
				NewSplashPartsSlow_Demo( &fvtemp, &rot, len );
//				AN_Test_Eye2( &fvtemp, 2 );
*/
			}
		}
		DG_COPY_VEC( fpvec1, fpvec0 );
//		AN_Test_Eye3( fpvec );
		fpvec0++;
		fpvec1++;
		obj++;
		joint_num++;
	}

	/* 初期値が -1 なら 自殺しない */
	if( work->life >= 0 ){
		work->life--;
		if( work->life < 0 ){
			GV_DestroyActor( work ) ;
		}
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

static int GetResources( Work *work, DG_OBJS *objs, float step_limit, int life )
{
	int		i ;
	DG_OBJ	*obj;
	FVECTOR	*fpvec;

	work->objs=objs;
	work->life = life;
	work->step_limit = step_limit;

	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 

	obj   = work->objs->objs;
	fpvec = work->old_vec;
	for( i=0; i<work->n_models; i++ ){
		DG_COPY_VEC( fpvec, (FVECTOR *)obj->world.m[3] );
//		AN_Test_Eye3( fpvec );
		fpvec++;
		obj++;
	}



	return (0);
}
/* ---------------------------------------------------------------- */
void *NewSplashMotion_Demo(
		DG_OBJS *objs,		/* [ポインタ保存参照]対象オブジェ */
		float	step_limit,
		int life			/* 寿命。フレーム指定。初期値が -1 なら 自殺しない */
		 )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, objs, step_limit, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

