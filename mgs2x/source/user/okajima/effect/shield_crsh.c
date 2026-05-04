//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	shield_crsh.c
	盾こわれエフェクト
	1999/12/07 S.Okajima
	$Id: shield_crsh.c,v 1.1.1.3 2002/11/19 11:47:13 Yoshizawa1 Exp $

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
#include	"../etc/ok_util.h"

/* 発生破片数 */
#define		N_FRAGMENT	(64)

#define		FRAGMENT_MODEL	(2656239 /*"shl_frg1"*/)
//#define		FRAGMENT_MODEL	(2656240 /*"shl_frg2"*/)
//#define		FRAGMENT_MODEL	(2656241 /*"shl_frg3"*/)
#define		MAX_MODEL_PATTERN	(3)

#define		MAX_SPEED	(50.0f)

#define		MAX_ALPHA	(128.0f)

#define		SHIELD_HALF_X	( 90.0f)
#define		SHIELD_HALF_Y	(400.0f)
#define		SHIELD_HALF_Z	(250.0f)
#define		MAX_X			((SHIELD_HALF_Y*SHIELD_HALF_Y + SHIELD_HALF_Z*SHIELD_HALF_Z)*4.0f)

#define		FRAGMENT_LIFE	(60 * 3)
#define		FRAGMENT_DECAY_TIME	(FRAGMENT_LIFE/4)

extern int  DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

typedef	struct	{
	GV_ACT_EX		actor ;

	FMATRIX		light[2];

	int			flr_flag[N_FRAGMENT];
	float		flr_height[N_FRAGMENT];
	FVECTOR		boundary[N_FRAGMENT] ;	/* 破片の数だけ持つ */

	DG_COMDL	*comdl ;
	FVECTOR		vec[N_FRAGMENT];
	FVECTOR		pos[N_FRAGMENT];
	SVECTOR		rot[N_FRAGMENT];

	FVECTOR		col_amb_max;
	FVECTOR		col_pll_max;

	FVECTOR		color;

	int			life;

	HZX_GROUP_ID	map_id;

} Work ;

static	void Act( Work *work )
{
	DG_COMDL_POS	*comdl_pos;
	FVECTOR fvtemp0;
	float	f_abs;
	FVECTOR		*boundary;

	FVECTOR		*vec;
	FVECTOR		*pos;
	SVECTOR		*rot;
	int			i;
	int			color_a;
	HZX_FLR		flr[2];
	int			flr_atrs[2];
	float		*height;
	float		flr_height[2];

	work->comdl->group_id = GM_GetDGGroupID( GM_CurrentStageMap );

	if(work->life < FRAGMENT_DECAY_TIME){
		color_a = (int)( MAX_ALPHA * (float)work->life / (float)FRAGMENT_DECAY_TIME );
	}else{
		color_a = (int)( MAX_ALPHA );
	}

//printf("%d %d %d\n",color_r,color_g,color_b);

	height   = work->flr_height;
	boundary=work->boundary;
	vec=work->vec;
	pos=work->pos;
	rot=work->rot;
	comdl_pos=work->comdl->pos;
	for( i=0; i<N_FRAGMENT; i++ ){
		rot->vx += 128;
		rot->vy += 128;
		rot->vz += 128;

		vec->vy += P_GRAVITY;
		_sceVu0AddVector( &fvtemp0, pos, vec ) ;

		if( pos->vx < boundary->vx
		 || pos->vz < boundary->vy
		 || pos->vx > boundary->vz
		 || pos->vz > boundary->vw ){
			if( HZX_LevelHazardCheck( GM_CurrentStageMap, pos, HZX_CHK_FIX, HZX_FLOOR_NO_PLAYER ) & 1 ){
				HZX_GetLevelHazard( flr, flr_atrs );
				HZX_GetLevelHeight( flr_height );
				OK_FloorToBoundary( flr, boundary );
				(*height) = flr_height[0];
				if( fvtemp0.vy < (*height) ){	/* 新たな床は下に無かった */
					OK_NoFloorInit( boundary, flr_height );
				}
			}
		}
		if( fvtemp0.vy < (*height) ){	/* 床に当たった */
			vec->vx *= 0.8f;
			vec->vx *= 0.8f;
			vec->vx += rnd()*vec->vy*0.5f - vec->vy*0.25f;
			vec->vz += rnd()*vec->vy*0.5f - vec->vy*0.25f;
			if(irnd()%4==0){
				vec->vy *= -0.9f;
			}else{
				vec->vy *= -0.6f;
			}
			f_abs=( vec->vy > 0 )?vec->vy:-vec->vy;
//			if( f_abs < 10.0f ){
			if( f_abs < DG_FABS(P_GRAVITY)*2.0f ){
				pos->vy = (*height) + 15.0f;
				rot->vx = 0;
				rot->vy = (short)(4096.0f*rnd());
				rot->vz = 1024;
			}else{
				rot->vx += 128;
				rot->vy += 128;
				rot->vz += 128;
			}
		}else{	/* 当たらなかった */
			DG_COPY_VEC( pos, &fvtemp0 );
		}
		DG_SetPos2( pos, rot );
		DG_GetPos( &comdl_pos->world );

		comdl_pos->color.vw = color_a ;

		height++;
		boundary++;
		comdl_pos++;
		vec++;
		pos++;
		rot++;
	}


	work->life--;
	if( work->life < 0 ){
//printf("destroy\n");
		work->life=0;
		GV_DestroyActor( work ) ;
	}
}

static	void Die( Work *work )
{
	DG_DequeueComdlObjs( work->comdl );
	DG_FreeComdl( work->comdl );
}

static	int GetResources( Work *work, OBJECT *shield )
{
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	FVECTOR		*boundary;
	FVECTOR		*vec;
	FVECTOR		*pos;
	SVECTOR		*rot;
	DG_DEF		*def ;
	DG_COMDL	*comdl ;
	DG_COMDL_POS	*comdl_pos;
	float		ftemp;
	float		*height;
	float		flr_height[2];
	int			i;
	HZX_FLR		flr[2];
	int			flr_atrs[2];
	int			init_flr_flag;
	int			*flr_flag;

	work->life=FRAGMENT_LIFE;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( FRAGMENT_MODEL, 'k' ) ) ;
	comdl = work->comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, N_FRAGMENT, 0 );
	if( comdl==NULL ) return -1;
	DG_QueueComdlObjs( comdl );

	svtemp.vz=0;
	fvtemp.vx=0.0f;
	fvtemp.vy=0.0f;
	vec=work->vec;
	pos=work->pos;
	rot=work->rot;
	for( i=0; i<N_FRAGMENT; i++ ){
		pos->vx = -SHIELD_HALF_X + rnd() * SHIELD_HALF_X * 2.0f;
		pos->vy = -SHIELD_HALF_Y + rnd() * SHIELD_HALF_Y * 2.0f;
		pos->vz = -SHIELD_HALF_Z + rnd() * SHIELD_HALF_Z * 2.0f;

		svtemp.vx=irnd()%2048+2048;
		svtemp.vy=irnd()%4096;

		fvtemp.vz=rnd() * MAX_SPEED;

		DG_SetPos2( &DG_ZeroVector, &svtemp ) ;
		DG_PutVector( &fvtemp, vec, 1 ) ;

		rot->vx = irnd()%4096;
		rot->vy = irnd()%4096;
		rot->vz = irnd()%4096;

		vec++;
		pos++;
		rot++;
	}
	DG_SetPos( &shield->objs->world ) ;
	DG_PutVector( work->pos, work->pos, N_FRAGMENT ) ;

	work->color.vx = (shield->objs->light[1].m[0][0] + shield->objs->light[1].m[3][0]) *0.5f;
	work->color.vy = (shield->objs->light[1].m[0][1] + shield->objs->light[1].m[3][1]) *0.5f;
	work->color.vz = (shield->objs->light[1].m[0][2] + shield->objs->light[1].m[3][2]) *0.5f;

	// けばいライトに反応しないようにする
	ftemp = (work->color.vx + work->color.vy + work->color.vz)/3.0f;
	work->color.vx = (work->color.vx + ftemp)*0.5f;
	work->color.vy = (work->color.vy + ftemp)*0.5f;
	work->color.vz = (work->color.vz + ftemp)*0.5f;

	work->color.vx = (work->color.vx + ftemp)*0.5f;
	work->color.vy = (work->color.vy + ftemp)*0.5f;
	work->color.vz = (work->color.vz + ftemp)*0.5f;

//	work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
//	work->map_id = GM_GetHzxGroupID( GM_CurrentMap );
//	work->map_id = GM_GetHzxGroupID( shield->objs->group_id );
	work->map_id = shield->objs->group_id;

	init_flr_flag = HZX_LevelHazardCheck(
	                               GM_CurrentStageMap,
	                               (FVECTOR *)shield->objs->world.m[3],
	                               HZX_CHK_F_FLOOR,
	                               HZX_FLOOR_NO_PLAYER );
	if( init_flr_flag & 1 ){
		HZX_GetLevelHazard( flr, flr_atrs );
		OK_FloorToBoundary( flr, &fvtemp );

		HZX_GetLevelHeight( flr_height );
	}else{
		OK_NoFloorInit( &fvtemp, flr_height );
	}

	height   = work->flr_height;
	flr_flag = work->flr_flag;
	boundary=work->boundary;
	vec=work->vec;
	pos=work->pos;
	rot=work->rot;
	comdl_pos=comdl->pos;
	for( i=0; i<N_FRAGMENT; i++ ){
		DG_SetPos2( pos, rot ) ;
		DG_GetPos( &comdl_pos->world );

		/* 整数型なので注意！ */
		comdl_pos->color.vx = (int)work->color.vx ;
		comdl_pos->color.vy = (int)work->color.vy ;
		comdl_pos->color.vz = (int)work->color.vz ;
		comdl_pos->color.vw = 128 ;

		*flr_flag = init_flr_flag;
		*height   = flr_height[0];
		DG_COPY_VEC( boundary, &fvtemp );

		height++;
		boundary++;
		flr_flag++;
		comdl_pos++;
		pos++;
		rot++;
	}

	{
		extern void *NewShield_Smoke(FMATRIX  *world);
		NewShield_Smoke( &shield->objs->world );
	}

	return 0 ;
}

void *NewCrushShield( OBJECT *shield )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, shield ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
