//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	liner_gun_init.c
	リニアガン起動エフェクト
	2001/04/16 S.Okajima
	$Id: liner_gun_init.c,v 1.1.1.3 2002/11/19 11:47:21 Yoshizawa1 Exp $
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

#define	PLASMA_NUM	(8)
#define	LIFE		(300)
#define	PLASMA_LIFE	(120)
#define	PLASMA_LINE_PHASE	(200)

//#define	WIDTH		(10.0f)
#define	WIDTH		(30.0f)
//#define	HEIGHT		(80.0f)
#define	HEIGHT		(270.0f)
#define	RATIO_SPEED	(1.5f / HEIGHT)

#define	EFFECT_RADIUS_MIN	(50.0f)
#define	EFFECT_RADIUS_RND	(1000.0f)


/* ---------------------------------------------------------------- */
int OK_LinerFlag;

//static FVECTOR Shift = { 18.0f, -1080.0f, 102.5f, 1.0f } ;
static FVECTOR Shift = { 18.0f, -900.0f, 102.5f, 1.0f } ;

/* ---------------------------------------------------------------- */
extern void *NewLinerPlasmaMini( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2, int width, int life, float *ratio );
extern void *NewLinerGunPlasmaFlush( FVECTOR *pos0, FVECTOR *pos1, int life );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	int			life;
	int			life_max;

	int			effect_count;

	FMATRIX		*world;

	float		pos_ratio[PLASMA_NUM];
	FVECTOR		pos0[PLASMA_NUM] ;
	FVECTOR		pos1[PLASMA_NUM] ;
	FVECTOR		pos2[PLASMA_NUM] ;

} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i;
	float	*pos_ratio;
	SVECTOR svtemp ;
	FVECTOR base ;
	FVECTOR width ;
	FVECTOR height ;
	FVECTOR local_pos0 ;
	FVECTOR local_pos1 ;
	FVECTOR local_pos2 ;
	FVECTOR fvtemp ;
	FVECTOR fvtemp2 ;
	FVECTOR *pos0 ;
	FVECTOR *pos1 ;
	FVECTOR *pos2 ;


	DG_SetPos( work->world );
	DG_PutVector( &Shift, &base, 1 );

	if(DIRECT_TICK( GV_Time )%8==0){
		GM_SeSetMode( SD_E_RGSPARK2, &base, GM_SEMODE_NORMAL ) ;// ブブブブ
	}

	if ( GM_CheckGameStatus( STATE_DEMO ) ) {
		OK_LinerFlag=1;
	}

	width.vx = 0.0f;
	width.vy = 0.0f;
	width.vz = WIDTH;
	DG_RotVector( &width, &width, 1 );

	_sceVu0AddVector( &local_pos0, &base, &width ) ;
	_sceVu0SubVector( &local_pos1, &base, &width ) ;
	DG_COPY_VEC( &local_pos2, &base ) ;

	height.vx = 0.0f;
	height.vy =-HEIGHT;
	height.vz = 0.0f;
	DG_RotVector( &height, &height, 1 );

	_sceVu0AddVector( &fvtemp2, &base, &height ) ;

	if( OK_LinerFlag !=0 ){
		if( LIFE - work->life < PLASMA_LINE_PHASE ){
			if( ((irnd()>>8)&3)==0 ){
				svtemp.vx = ((irnd()>>8)&2047)-1024;
				svtemp.vy = ((irnd()>>8)&4095);
				svtemp.vz = 0;
				fvtemp.vx = 0.0f;
				fvtemp.vy = 0.0f;
				fvtemp.vz = EFFECT_RADIUS_MIN + EFFECT_RADIUS_RND*rnd() * (float)(PLASMA_LINE_PHASE - LIFE + work->life) / (float)PLASMA_LINE_PHASE;
				DG_SetPos2( &base, &svtemp );
				DG_PutVector( &fvtemp, &fvtemp, 1 );
				NewLinerGunPlasmaFlush( &fvtemp2, &fvtemp, 4 );
			}else if( ((irnd()>>8)&7)==0 ){
				GM_SeSetMode( SD_E_RGSPARK1, &base, GM_SEMODE_NORMAL ) ;// スパーク
			}
		}
	}



	pos_ratio = work->pos_ratio;
	pos0   = work->pos0;
	pos1   = work->pos1;
	pos2   = work->pos2;
	for( i=0; i<PLASMA_NUM; i++ ){
		(*pos_ratio)+=RATIO_SPEED;

		if( (*pos_ratio) > 1.0f ){
			if( work->life > PLASMA_LIFE  &&  ((irnd()>>8)&31)==0 ){
				(*pos_ratio) = rnd();
				DG_COPY_VEC( pos0, &local_pos0 );
				DG_COPY_VEC( pos1, &local_pos1 );
				_sceVu0AddVector( pos2, pos0, pos1 ) ;
				_sceVu0ScaleVector( pos2, pos2, 0.5f );
				if( OK_LinerFlag !=0 ){
					NewLinerPlasmaMini( pos0, pos1, pos2, 4, (int)((float)PLASMA_LIFE*(*pos_ratio)), pos_ratio );
				}
			}else{
				(*pos_ratio) = 1.0f;
			}
		}

		_sceVu0ScaleVector( &fvtemp, &height, (*pos_ratio) );
		_sceVu0AddVector( pos0, &local_pos0, &fvtemp ) ;

		_sceVu0ScaleVector( &fvtemp, &height, (*pos_ratio) );
		_sceVu0AddVector( pos1, &local_pos1, &fvtemp ) ;

		_sceVu0ScaleVector( &fvtemp, &height, (*pos_ratio)*2.0f );
		_sceVu0AddVector( pos2, &local_pos2, &fvtemp ) ;

		pos0++;
		pos1++;
		pos2++;
		pos_ratio++;
	}

	if( work->life > 0 ){
//		work->life--;
	}else{
//		GV_DestroyActor( work ) ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}


static int GetResources( Work *work )
{
	int	i;
	float	*pos_ratio;

	pos_ratio = work->pos_ratio;
	for( i=0; i<PLASMA_NUM; i++ ){
		(*pos_ratio) = rnd();
		pos_ratio++;
	}

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewLinerGunInitEffect( FMATRIX *world )
{
	Work		*work ;
	int			life;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;

		life = DIRECT_TICK( LIFE );
		work->life     = life;
		work->life_max = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

