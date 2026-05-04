//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	explosion_controler.c
	爆発コントローラ
	2001/07/22 S.Okajima
	$Id: explosion_controler.c,v 1.1.1.3 2002/11/19 11:47:30 Yoshizawa1 Exp $

*/


#ifdef PSX2	///
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
#include	"utl_dma.h"
#include	"gameheader.h"

#include	"../etc/ok_util.h"
#include	"./explosion.h"


#define	LIFE			(120)
#define	BIRTH_INTERVAL	(6)
#define	MAX_NUM			(2)

/* ---------------------------------------------------------------- */
extern void *NewExplosionParts( FVECTOR *center, FVECTOR *vec, float size, int col, float life_param, float decay_param, int rebirth_num );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			birth_num;
	int			interval;
	int			interval_max;
	int			life;
	int			life_max;
	int			alpha;

	float		size;
	FVECTOR		dummy;
	FVECTOR		*center;
} Work ;

int		OK_EXPLO_SEED = 1;
int		OK_EXPLO_DESTROY_FLAG = 0;
FVECTOR	OK_EXPLO_WorkTemp0[EXPLO_N_PRIMS*EXPLO_N_VERTS];
static	Work	*OK_EXPLO_WORK = NULL ;

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
			work->interval     =
			work->interval_max =        msg->message[1];
			work->birth_num    =        msg->message[2];
			work->size         = (float)msg->message[3] / (float)REBIRTH_NUM;
			work->alpha        =        msg->message[4];
			break;
		  case 1:
			OK_EXPLO_DESTROY_FLAG = 1;
			break;
		  case 2:
			OK_EXPLO_SEED = msg->message[1];
			break;
		  default:
			break;
		}
		msg--;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	fvtemp0;
	FVECTOR	center;
	FVECTOR	vec;
	float	angle0;
	float	angle1;
	float	radius0;
	float	radius1;
	float	life_ratio;
	int		i;


	OK_EXPLO_DESTROY_FLAG = 0;

	if( GM_CheckGameStatus( STATE_DEMO ) ) {
		CheckMesgParam( work );
		if( work->interval <= 0 ) return;
		life_ratio = GM_Rnd( &OK_EXPLO_SEED );
	}else{
		life_ratio = (float)work->life/(float)work->life_max;
		work->life--;
		if( work->life <= 0){
			GV_DestroyActor( work ) ;
			return;
		}
	}

	work->interval--;
	if( work->interval <= 0 ){
		work->interval = work->interval_max;
		for( i=0; i<work->birth_num; i++ ){
			angle0     =  PI*0.5f*GM_FRnd( &OK_EXPLO_SEED );
			angle1     = TPI*GM_Rnd( &OK_EXPLO_SEED );
			radius0    = work->size * (0.5f + GM_Rnd( &OK_EXPLO_SEED )*0.5f);
			radius1    = radius0 * cosf( angle0 );
			fvtemp0.vx = radius1 * sinf( angle1 );
			fvtemp0.vy = radius0 * sinf( angle0 );
			fvtemp0.vz = radius1 * cosf( angle1 );

			_sceVu0AddVector( &center, work->center, &fvtemp0 ) ;
			_sceVu0ScaleVector( &vec, &fvtemp0, 0.005f*GM_Rnd( &OK_EXPLO_SEED ) );

			NewExplosionParts(
				&center,
				&vec,
				work->size*(0.75f + 0.25f*life_ratio),
				(0xffffff<<8)|(work->alpha&255),
				0.05f+0.1f*GM_Rnd( &OK_EXPLO_SEED ),
				0.98f,
				REBIRTH_NUM );
		}
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
//	OK_EXPLO_DESTROY_FLAG = 0;
	OK_EXPLO_WORK = NULL ;
}

static void MakeVerts( FVECTOR *pos, float height, float radius, int div )
{
	float angle;

	angle = TPI * (float)div / (float)(EXPLO_N_SIDES1-1);
	pos->vx = radius * sinf( angle );
	pos->vy = height;
	pos->vz = radius * cosf( angle );
}

static int GetResources( Work *work )
{
	FVECTOR		*base0;
	FVECTOR		*base1;
	float	height;
	float	ratio;
	float	radius;
	int		i,j;

	work->life = work->life_max = LIFE;
	work->interval     = 0;
	work->interval_max = BIRTH_INTERVAL;
	work->birth_num    = MAX_NUM;

	// 共通形状データ作成
	base0   = OK_EXPLO_WorkTemp0;
	base1   = base0;
	base1++;
	for ( i = 0 ; i < EXPLO_N_SIDES0 ; i++ ){
		ratio = PI*(ANGLE_START + ANGLE_DIFF*(float)i/ (float)(EXPLO_N_SIDES0-1));
		radius = sinf( ratio );
		height =-cosf( ratio );
		if( i==0 ){
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				MakeVerts( base0, height, radius, j );
				base0+=2;
			}
		}else if( i==EXPLO_N_SIDES0-1 ){
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				MakeVerts( base1, height, radius, j );
				base1+=2;
			}
		}else{
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				MakeVerts( base0, height, radius, j );
				DG_COPY_VEC( base1, base0 );
				base0+=2;
				base1+=2;
			}
		}
	}

	return 0 ;
}

void *NewExplosionControl( FVECTOR *pos, float size, int alpha )
{
	Work		*work ;

	OPERATOR() ;

	if( GM_CheckGameStatus( STATE_DEMO ) ) {
		if( OK_EXPLO_WORK != NULL ){
			return NULL;
		}
	}

	OK_EXPLO_SEED = (irnd()>>8);

//	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->dummy, pos );
		work->name = -1;
		work->center = &work->dummy;
		work->size   = size / (float)REBIRTH_NUM;
		work->alpha  = alpha;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		OK_EXPLO_WORK = work ;

	}
	return (void *)work ;
}

void *NewExplosionControl_Demo( int name, FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	if( GM_CheckGameStatus( STATE_DEMO ) ) {
		if( OK_EXPLO_WORK != NULL ){
			return NULL;
		}
	}

//	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->center = (FVECTOR *)world->m[3];
		work->size   = 0.0f;
		work->alpha  = 0xff;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		OK_EXPLO_WORK = work ;

	}
	return (void *)work ;
}

