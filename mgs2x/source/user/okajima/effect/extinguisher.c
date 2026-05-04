//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	extinguisher.c
	消化器と噴出物
	2000/04/11 S.Okajima
	$Id: extinguisher.c,v 1.1.1.3 2002/11/19 11:47:04 Yoshizawa1 Exp $
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
#include	"libhzx.h"
#include	"../etc/ok_util.h"

extern void DG_FreePreshade( DG_OBJS * );
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
extern void *NewFlour_Gas3( FVECTOR *center );
extern void *NewExtinguisherJet( FVECTOR *pos, FVECTOR *force );
extern void *NewFlour_Fall( int map, int name, FVECTOR *hit ,FVECTOR *force);
extern int OK_IRS_OnFlag;


#define MAX_JOINTS (4)
#define LEN_JOINTS (1000.0f / (float)MAX_JOINTS)
#define SPEED		(30.0f)
#define MAX_SIZE	(700.0f)
#define SIZE_MUL	(1.03f)
#define	COLOR_R		(255)
#define	COLOR_G		(255)
#define	COLOR_B		(255)

#define N_PRIMS		(8)
#define N_VERTS		(16)
#define	MAX_ALPHA	(32.0f)
#define	DIM_ALPHA	(0.5f)

#ifdef KP_XBOX
#define	EDGE_0		(111)
#define	EDGE_1		(32)
#else
#define	EDGE_0		(68)
#define	EDGE_1		(27)
#endif

#define	SHIFT_CONE	(280.0f)

#define	REFLECT_DECAY		(0.6f)
#define	FRICTION_RATIO		(0.3f)
#define	TEX_MARGIN_RATIO	(0.7f)
#define	DECAY_SPEED			(0.99f)

#define	CONT_TIME			(60*4)

#define TARGET_SIZE (5000.0f)

typedef struct holowork_t
{
	GV_ACT_EX	actor ;
	int		name;
	int		where;

	FVECTOR	center;


	int		debug;

	FVECTOR	bound_min;
	FVECTOR	bound_max;

	float	max_alpha;
	int		prim_count;
	DG_PRIM2	*prim ;
	FVECTOR		vec[N_PRIMS*N_VERTS];

	int		v_num;
	int		v_anim_count_max;
	float	width;
	float	calc_width;
	CV2_DEF *cdef ;
	FVECTOR	pos0;
	FVECTOR	pos1;
	FVECTOR	pos2;
	FVECTOR	pos3;
	FVECTOR	nrm0;
	FVECTOR	nrm1;

	int		count;
	int		dir_flag;
	int		mode;
	int	   calc_key   ; /* キーフレーム */
	int	   key   ; /* キーフレーム */
	int	   n_key ;
	DG_OBJS  *objs  ;
	FMATRIX		lights[2] ;
	VERTEX_ANIME_WORK *anime ;

	DG_OBJS  *objs_cone;
	FMATRIX		cone_world;
	FMATRIX		lights_cone[ 2 ] ;

	TARGET tgt;
	POWER_TARGET pow;
	TARGET tgt_mini;
	POWER_TARGET pow_mini;

	TARGET target_at;
	POWER_TARGET power_at;

	int		se_flag;
	int		se_count;
	int		irs_on;

	int			proc_id;

} Work ;

static void SetAttackTarget( Work *work )
{
	FVECTOR	size;
	FVECTOR	center;
	FVECTOR	bound_min;
	FVECTOR	bound_max;

	bound_min.vx = work->center.vx - TARGET_SIZE;
	bound_min.vy = work->center.vy - TARGET_SIZE;
	bound_min.vz = work->center.vz - TARGET_SIZE;
	bound_max.vx = work->center.vx + TARGET_SIZE;
	bound_max.vy = work->center.vy + TARGET_SIZE;
	bound_max.vz = work->center.vz + TARGET_SIZE;

	bound_min.vx = ( bound_min.vx > work->bound_min.vx )? bound_min.vx: work->bound_min.vx;
	bound_max.vx = ( bound_max.vx < work->bound_max.vx )? bound_max.vx: work->bound_max.vx;
	bound_min.vy = ( bound_min.vy > work->bound_min.vy )? bound_min.vy: work->bound_min.vy;
	bound_max.vy = ( bound_max.vy < work->bound_max.vy )? bound_max.vy: work->bound_max.vy;
	bound_min.vz = ( bound_min.vz > work->bound_min.vz )? bound_min.vz: work->bound_min.vz;
	bound_max.vz = ( bound_max.vz < work->bound_max.vz )? bound_max.vz: work->bound_max.vz;

//	AN_Test_Eye2( &bound_min, 2 );
//	AN_Test_Eye2( &bound_max, 2 );
//	NewBoundingBoxView( &bound_min, &bound_max, 64, 128, 255 ) ;

	size.vx   = (bound_max.vx - bound_min.vx)*0.5f;
	size.vy   = (bound_max.vy - bound_min.vy)*0.5f;
	size.vz   = (bound_max.vz - bound_min.vz)*0.5f;
	center.vx = bound_min.vx + size.vx;
	center.vy = bound_min.vy + size.vy;
	center.vz = bound_min.vz + size.vz;

	GM_SetTarget( &work->target_at,
	              TARGET_OFFENSE,
	              work->where,
	              BOTH_SIDE,
	              &size,
	              &DG_ZeroVector);
	GM_SetTargetWeaponType( &work->target_at, WP_NOBLOOD|WP_EXTINGUISHER );
	GM_MoveTargetMap( &work->target_at, &center, work->where );

	GM_SetPowerTarget( &work->target_at,
	                   &work->power_at,
	                   POWER_ONCE,
	                   255,
	                   0,
	                   0,
	                   &DG_ZeroVector);
#ifdef DEBUG_MODE
	if( work->debug ) NewTargetView( &work->target_at, 0, 255, 0 ) ;
#endif
}

static void Die( Work *work )
{
	if ( work->anime ) ExitVertexAnimation( work->anime ) ;
	if ( work->objs ){
//		DG_FreePreshade( work->objs );
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
	if ( work->objs_cone ){
		DG_DequeueObjs( work->objs_cone );
		DG_FreeObjs( work->objs_cone );
	}

	GM_ClearTargetDamage( &work->tgt ) ;
	GM_FreeTarget( &work->tgt ) ;
	GM_ClearTargetDamage( &work->tgt_mini ) ;
	GM_FreeTarget( &work->tgt_mini ) ;
	GM_ClearTargetDamage( &work->target_at ) ;
	GM_FreeTarget( &work->target_at ) ;

	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0;
	DG_PRIM2_UVRGBWH	*uvrgbwh1;
	int	i,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_PRIMS; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = 0 ;
			uvrgbwh0->h = uvrgbwh1->h = 0 ;
			uvrgbwh0->r = uvrgbwh1->r = COLOR_R ;
			uvrgbwh0->g = uvrgbwh1->g = COLOR_G ;
			uvrgbwh0->b = uvrgbwh1->b = COLOR_B ;
			uvrgbwh0->a = uvrgbwh1->a = 0 ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

}

static void EdgeReflect( FVECTOR *vec, int mode )
{
	FVECTOR	abs_vec;
	float	ftemp;

	abs_vec.vx = (vec->vx > 0.0f)? vec->vx: -vec->vx;
	abs_vec.vy = (vec->vy > 0.0f)? vec->vy: -vec->vy;
	abs_vec.vz = (vec->vz > 0.0f)? vec->vz: -vec->vz;
	switch( mode ){
	  case 0:
		vec->vx *= -REFLECT_DECAY;
		ftemp = abs_vec.vx * FRICTION_RATIO;
		vec->vy += (ftemp+ftemp*0.1f*frnd()) * vec->vy / ( abs_vec.vy + abs_vec.vz );
		vec->vz += (ftemp+ftemp*0.1f*frnd()) * vec->vz / ( abs_vec.vy + abs_vec.vz );
		break;
	  case 1:
		vec->vy *= -REFLECT_DECAY;
		ftemp = abs_vec.vy * FRICTION_RATIO;
		vec->vx += (ftemp+ftemp*0.1f*frnd()) * vec->vx / ( abs_vec.vx + abs_vec.vz );
		vec->vz += (ftemp+ftemp*0.1f*frnd()) * vec->vz / ( abs_vec.vx + abs_vec.vz );
		break;
	  case 2:
		vec->vz *= -REFLECT_DECAY;
		ftemp = abs_vec.vz * FRICTION_RATIO;
		vec->vy += (ftemp+ftemp*0.1f*frnd()) * vec->vy / ( abs_vec.vy + abs_vec.vx );
		vec->vx += (ftemp+ftemp*0.1f*frnd()) * vec->vx / ( abs_vec.vy + abs_vec.vx );
		break;
	}
}

static int HoseDance( Work *work, int key )
{
	int	i;
	float	ftemp0;
	float	ftemp1;
	SVECTOR svtemp0;
	FVECTOR fvtemp0;
	FVECTOR fvtemp2;
	FVECTOR shift;
	FVECTOR	*vec;
	FVECTOR	*pos = NULL;
	FVECTOR	*pos_back;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_back;
	int		clock;




	SimpleVertexAnimation( work->anime ) ;
	if( work->anime->count == 0 ){
		work->se_flag++;
		work->anime->p[work->calc_key] = 0.0f ;
		if( (irnd()>>8)%4==0 ){
			work->dir_flag = 1-work->dir_flag;
		}
		if( work->dir_flag ){
			work->key++;
		}else{
			work->key--;
		}
		if( work->key > 9 ){
			work->key = 8;
			work->dir_flag = 0;
		}else if( work->key < 2 ){
			work->key = 3;
			work->dir_flag = 1;
		}
		if( (irnd()>>8)%4==0 ){
			work->calc_key = work->key + 8;
		}else{
			work->calc_key = work->key;
		}

		if( key >= 0 ){
			if( work->calc_key == key ){
				work->anime->count = 0 ;
			}else{
				work->anime->count = 20 ;
			}
			work->calc_key = key;
		}else{
			work->anime->count = (irnd()>>8)%8+6 ;
		}
		work->v_anim_count_max = work->anime->count;
		work->anime->p[work->calc_key] = 1.0f ;

		DG_COPY_VEC( &work->pos0, &work->pos1 );
		DG_COPY_VEC( &work->pos1, &work->cdef->models[work->calc_key].verts[EDGE_0] );
		DG_COPY_VEC( &work->pos2, &work->pos3 );
		DG_COPY_VEC( &work->pos3, &work->cdef->models[work->calc_key].verts[EDGE_1] );
		DG_COPY_VEC( &work->nrm0, &work->nrm1 );
		DG_COPY_VEC( &work->nrm1, &work->cdef->models[work->calc_key].norms[EDGE_0] );

		_sceVu0SubVector( &fvtemp0, &work->pos0, &work->pos1 ) ;
		ftemp0 = GV_VecLen3F( &fvtemp0 );
		work->width = ftemp0 * 3.0f / (float)work->v_anim_count_max;
	}

	fvtemp0.vx = work->pos1.vx + (work->pos0.vx - work->pos1.vx) * (float)work->anime->count / (float)work->v_anim_count_max;
	fvtemp0.vy = work->pos1.vy + (work->pos0.vy - work->pos1.vy) * (float)work->anime->count / (float)work->v_anim_count_max;
	fvtemp0.vz = work->pos1.vz + (work->pos0.vz - work->pos1.vz) * (float)work->anime->count / (float)work->v_anim_count_max;
	fvtemp0.vw = 1.0f;

	fvtemp2.vx = work->pos3.vx + (work->pos2.vx - work->pos3.vx) * (float)work->anime->count / (float)work->v_anim_count_max;
	fvtemp2.vy = work->pos3.vy + (work->pos2.vy - work->pos3.vy) * (float)work->anime->count / (float)work->v_anim_count_max;
	fvtemp2.vz = work->pos3.vz + (work->pos2.vz - work->pos3.vz) * (float)work->anime->count / (float)work->v_anim_count_max;
	fvtemp2.vw = 1.0f;


	DG_SetPos( &work->objs->world );
	DG_PutVector( &fvtemp0, &fvtemp0, 1 );
	DG_PutVector( &fvtemp2, &fvtemp2, 1 );
//	AN_Test_Eye2( &fvtemp0, 3 );
//	AN_Test_Eye2( &fvtemp2, 3 );
	// fvtemp0:先端位置

	// コーン
	DG_GetLightMatrix( &fvtemp0, work->lights_cone );
	OK_DirVecXY_Y( &fvtemp0, &fvtemp2, &svtemp0 );
	DG_SetPos2( &fvtemp0, &svtemp0 );
	DG_GetPos( &work->cone_world );
	DG_PutObjs( work->objs_cone );

	shift.vx = 0.0f;
	shift.vy = SHIFT_CONE;
	shift.vz = 0.0f;
	DG_RotVector( &shift, &shift, 1 );


	// ガス
	GM_GroupPrim2( work->prim, work->where ) ;
   
   //AR_PARTICLE_HALF
   // AS MCampbell - Instead of returning we need to execute any logic not related to the prims. This fixes issues 
   // where the sound effects would not play at the right speed or last too long.
   // Bug: MGSTWO-2950
   if( DG_SwitchBuffPrim2( work->prim ) )
   {
	   clock = work->prim->buffer_clock;
	   vec      = work->vec;
	   pos      = work->prim->pos[clock];
	   pos_back = work->prim->pos[1-clock];
	   uvrgbwh      = work->prim->uvrgb[clock];
	   uvrgbwh_back = work->prim->uvrgb[1-clock];
	   for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		   if( i < work->prim_count ){
			   uvrgbwh->a = (int)(work->max_alpha * (float)(N_PRIMS*N_VERTS - work->prim_count + i) / (float)(N_PRIMS*N_VERTS));
		   }else{
			   uvrgbwh->a = (int)(work->max_alpha * (float)(i-work->prim_count) / (float)(N_PRIMS*N_VERTS));
		   }
   //		uvrgbwh->a = uvrgbwh_back->a - 1;
		   vec->vw = ( vec->vw < MAX_SIZE )? vec->vw*SIZE_MUL: MAX_SIZE;
		   uvrgbwh->w = (int)(cosf( pos->vw ) * vec->vw );
		   uvrgbwh->h = (int)(sinf( pos->vw ) * vec->vw );

		   vec->vx *= DECAY_SPEED;
		   vec->vy *= DECAY_SPEED;
		   vec->vz *= DECAY_SPEED;
		   vec->vy += P_GRAVITY*0.15f;

		   ftemp0 = vec->vw*TEX_MARGIN_RATIO;
		   pos->vx = pos_back->vx + vec->vx;
		   pos->vy = pos_back->vy + vec->vy;
		   pos->vz = pos_back->vz + vec->vz;
		   if( pos->vx  <  work->bound_min.vx + ftemp0 ){
			   EdgeReflect( vec, 0 );
			   vec->vx = ( vec->vx > 0.0f )? vec->vx: -vec->vx;
			   pos->vx = work->bound_min.vx + ftemp0 + vec->vx;
		   }else if( pos->vx  >  work->bound_max.vx - ftemp0 ){
			   EdgeReflect( vec, 0 );
			   vec->vx = ( vec->vx < 0.0f )? vec->vx: -vec->vx;
			   pos->vx = work->bound_max.vx - ftemp0 + vec->vx;
		   }

		   if( pos->vy  <  work->bound_min.vy + ftemp0 ){
			   EdgeReflect( vec, 1 );
   //			vec->vy = ( vec->vy > 0.0f )? vec->vy: -vec->vy;
			   pos->vy = work->bound_min.vy + ftemp0 + vec->vy;
		   }else if( pos->vy  >  work->bound_max.vy - ftemp0 ){
			   EdgeReflect( vec, 1 );
			   vec->vy = ( vec->vy < 0.0f )? vec->vy: -vec->vy;
			   pos->vy = work->bound_max.vy - ftemp0 + vec->vy;
		   }

		   if( pos->vz  <  work->bound_min.vz + ftemp0 ){
			   EdgeReflect( vec, 2 );
			   vec->vz = ( vec->vy > 0.0f )? vec->vz: -vec->vz;
			   pos->vz = work->bound_min.vz + ftemp0 + vec->vz;
		   }else if( pos->vz  >  work->bound_max.vz - ftemp0 ){
			   EdgeReflect( vec, 2 );
			   vec->vz = ( vec->vy < 0.0f )? vec->vz: -vec->vz;
			   pos->vz = work->bound_max.vz - ftemp0 + vec->vz;
		   }
   //		AN_Test_Eye2( pos, 2 );

		   pos++;
		   pos_back++;
		   uvrgbwh++;
		   uvrgbwh_back++;
		   vec ++;
	   }

	   work->prim_count++;
	   if( work->prim_count >= N_PRIMS*N_VERTS ) work->prim_count = 0;
	   vec      = &work->vec[work->prim_count];
	   pos      = &work->prim->pos[clock][work->prim_count];
	   pos_back = &work->prim->pos[1-clock][work->prim_count];
	   uvrgbwh  = work->prim->uvrgb[clock];
	   uvrgbwh += work->prim_count;
	   _sceVu0AddVector( pos, &shift, &fvtemp0 );
	   pos->vw = pos_back->vw = ftemp0 = TPI * rnd();
	   _sceVu0Normalize( &shift, &shift );
	   _sceVu0ScaleVector( vec, &shift, SPEED );

	   work->calc_width = work->calc_width*0.95f + work->width*0.05f;
	   vec->vw = ftemp1 = work->calc_width + work->calc_width*rnd()*0.5f;
	   uvrgbwh->w = 0;
	   uvrgbwh->h = 0;
   }


#if 1
//	if( work->se_flag>=2  &&  key<2 ){
	if( work->se_flag>=1  &&  key<2 ){
		work->se_flag=0;

      if (pos) // AS MCampbell - pos is only valid if the particles got updated.
		   GM_SeSetMode( SD_A_SYOKAK02 , pos, GM_SEMODE_NORMAL );	/* 消火器くねくね噴出 */
	}
#endif

	if( vu0_CheckBoundingBox( &GM_PlayerPosition, &work->bound_min, &work->bound_max ) ){
		GM_SetPlayerStatusEX( I64(0), PLAYER2_POWDER ) ;
	}

	if( work->anime->count==1  &&  key>=0  &&  work->calc_key == key ){
		return -1;
	}
	return 0;
}

static void Act( Work *work )
{
	switch( work->mode ){
	  case 0:
		DG_InvisiblePrim2( work->prim );
		DG_SetPos( &work->cone_world );
		DG_PutObjs( work->objs_cone );
		break;
	  case 1:
		GM_SeSetMode( SD_A_SYOKAK01, &work->center, GM_SEMODE_NORMAL );	/* 消火器着弾 */
		work->mode = 2;
		work->count = CONT_TIME;
		work->max_alpha = MAX_ALPHA;
		GM_PutTarget( &work->target_at ) ;
		if( work->proc_id > 0 ){
			GCL_ExecProc( work->proc_id, NULL );
		}

		//攻撃を受けるターゲットの開放
		GM_ClearTargetDamage( &work->tgt ) ;
		GM_FreeTarget( &work->tgt ) ;
		GM_ClearTargetDamage( &work->tgt_mini ) ;
		GM_FreeTarget( &work->tgt_mini ) ;

	  case 2:
		DG_VisiblePrim2( work->prim );
		HoseDance( work, -1 );
		if(work->count-- < 0){
			work->mode = 3;
		}
		break;
	  case 3:
		DG_VisiblePrim2( work->prim );
		work->max_alpha -= DIM_ALPHA;
		if( work->max_alpha <= 0.0f ){
			work->max_alpha  = 0.0f;
			if( HoseDance( work, 1 ) == -1 ){
				work->mode = 4;
			}
		}else{
			HoseDance( work, -1 );
		}
		break;
	  case 4:
		DG_InvisiblePrim2( work->prim );
		DG_SetPos( &work->cone_world );
		DG_PutObjs( work->objs_cone );
		break;
	}

}

static void TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work        *work ;
	FVECTOR		fvtemp;

	work = (Work *)ptr ;
	if ( def->damaged & TARGET_POWER ){
		if( &work->tgt == def  &&  (def->weapon_type & (WP_BULLET|WP_M92)) ){
			if( work->irs_on ){
				OK_IRS_OnFlag = 1;
			}

			if( ( off->class & TARGET_POWER ) && ( off->power != NULL ) ) {
				DG_COPY_VEC( &fvtemp, &off->power->force );
				fvtemp.vy *= 2.0f;
				NewExtinguisherJet( &def->hit, &fvtemp );
			}

			NewFlour_Gas3( &def->hit );
			NewFlour_Fall( work->where, work->name, &def->hit ,&off->power->force);

			def->class &= ~(TARGET_POWER);
			if( work->mode==0 ){
				work->mode=1;
			}
		}
		GM_ClearTargetDamage( def ) ;
	}

	{
		extern void BRK_UTL_CallOffenceWhenThrough( TARGET *off, TARGET *def );
		BRK_UTL_CallOffenceWhenThrough( off, def );
	}

}

// 壁の向こうの爆発に反応しないように爆発専用の小さいターゲットを置く
static void TargetCallBackForBomb( TARGET *off, TARGET *def, void *ptr )
{
	Work        *work ;
	FVECTOR		fvtemp;

	work = (Work *)ptr ;
	if ( def->damaged & TARGET_POWER ){
		if( &work->tgt_mini == def  &&  (def->weapon_type & (WP_BLAST)) ){
			if( work->irs_on ){
				OK_IRS_OnFlag = 1;
			}

			if( ( off->class & TARGET_POWER ) && ( off->power != NULL ) ) {
				DG_COPY_VEC( &fvtemp, &off->power->force );
				fvtemp.vy *= 2.0f;
				NewExtinguisherJet( &def->hit, &fvtemp );
			}

			NewFlour_Gas3( &def->hit );
			NewFlour_Fall( work->where, work->name, &def->hit ,&off->power->force);

			def->class &= ~(TARGET_THROUGH|TARGET_POWER);
			if( work->mode==0 ){
				work->mode=1;
			}
		}
		GM_ClearTargetDamage( def ) ;
	}
}

#ifdef PSX2
#define PERROR(...) { printf( __VA_ARGS__ ) ; return -1 ; }
#else
#define PERROR(...) 	 { printf( __VA_ARGS__ ) ; return -1 ; } 
#endif

static int GetResources( Work *work )
{
	int	  i, id, buf[4] ;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	SVECTOR	svtemp0;
	FMATRIX *mtx ;
	CV2_DEF *cdef ;
	DG_DEF  *def  ;
	LIT_DEF *lit = NULL ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->proc_id = -1;
	if ( GCL_GetOption( 'e' ) != NULL ){
		work->proc_id = GCL_GetNextInt();
	}

	work->irs_on = 0;
	if ( GCL_GetOption( 'i' ) != NULL ){
		work->irs_on = GCL_GetNextInt() ;
	}

	if( GCL_GetOption( 'd' ) != NULL ){
		work->debug = GCL_GetNextInt();
	}else{
		work->debug = 0;
	}

	id = 9713826 /*"hose_new2"*/;
	if ( !(cdef = work->cdef = GV_GetCache( GV_CacheID( id, 'c' ) )) ) PERROR( "No CV2 found in data.cnf!!\n" ) ;
	if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) ) PERROR( "No KMS found in data.cnf!!\n" ) ;
	def->n_models = def->n_x_models = 1 ;

	def->ux = 3000.0f;
	def->uy = 3000.0f;
	def->uz = 3000.0f;
	def->lx =-3000.0f;
	def->ly =-3000.0f;
	def->lz =-3000.0f;

	def->models[0].ux = 3000.0f;
	def->models[0].uy = 3000.0f;
	def->models[0].uz = 3000.0f;
	def->models[0].lx =-3000.0f;
	def->models[0].ly =-3000.0f;
	def->models[0].lz =-3000.0f;

//	if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) ) PERROR( "Can't Create DG_OBJS(Maybe no memory)!!\n" ) ;
	if ( !(id = GCL_GetOptionValue( 'l', 0 )) ) PERROR( "No LIT found in scn\n" ) ;
	lit = GV_GetCache( GV_CacheID( id, 'l' ) ) ;
	if ( lit ){
		work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
//		DG_MakePreshade( work->objs, lit ) ; /* ライトの設定 */
	}else{
		PERROR( "ERROR\n" ) ;
	}
	GM_GroupObjs( work->objs, work->where );
	DG_QueueObjs( work->objs ) ;

	work->objs->world = DG_UnitMatrix ;
	mtx = &work->objs->world ;

	if ( GCL_GetOption( 'r' ) != NULL ){
		int r ;
		r = GCL_GetNextInt() & 0x0fff ;
		_sceVu0RotMatrixX( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
		r = GCL_GetNextInt() & 0x0fff ;
		_sceVu0RotMatrixY( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
		r = GCL_GetNextInt() & 0x0fff ;
		_sceVu0RotMatrixZ( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	}
	if ( GCL_GetOption( 'p' ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->center ) ;
		DG_COPY_VEC( (FVECTOR *)mtx->m[3], &work->center );
		mtx->m[3][3] = 1.0f ;
	}else{
		PERROR( "No position!! :: NewPutVanimeObject\n" ) ;
	}

	fvtemp0.vx = 90.0f;
	fvtemp0.vy = 350.0f;
	fvtemp0.vz = 90.0f;
	fvtemp1.vx = work->center.vx +   0.0f;
	fvtemp1.vy = work->center.vy + 600.0f;
	fvtemp1.vz = work->center.vz +   0.0f;
	GM_SetTarget( &work->tgt, (TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH), work->where, ENEMY_SIDE, &fvtemp0, &fvtemp1 ) ;
	GM_SetPowerTarget( &work->tgt, &work->pow, POWER_CONST, 255, 0, 0, &fvtemp1 ) ;
	GM_SetTargetCallBack( &work->tgt, TargetCallBack, work ) ;
	GM_PutTarget( &work->tgt ) ;
#ifdef DEBUG_MODE
	if( work->debug ) NewTargetView( &work->tgt, 255, 0, 0 ) ;
#endif

	// 爆発用、ミニミニ
	fvtemp0.vx = 10.0f;
	fvtemp0.vy = 10.0f;
	fvtemp0.vz = 10.0f;
	fvtemp1.vx = work->center.vx +    0.0f;
	fvtemp1.vy = work->center.vy + 1000.0f;
	fvtemp1.vz = work->center.vz +    0.0f;
	GM_SetTarget( &work->tgt_mini, (TARGET_DEFENSE|TARGET_POWER), work->where, ENEMY_SIDE, &fvtemp0, &fvtemp1 ) ;
	GM_SetPowerTarget( &work->tgt_mini, &work->pow_mini, POWER_CONST, 255, 0, 0, &fvtemp1 ) ;
	GM_SetTargetCallBack( &work->tgt_mini, TargetCallBackForBomb, work ) ;
	GM_PutTarget( &work->tgt_mini ) ;
#ifdef DEBUG_MODE
	if( work->debug ) NewTargetView( &work->tgt_mini, 255, 128, 64 ) ;
#endif


//	DG_GetLightMatrixFix( &fvtemp1, work->lights ) ;
	DG_GetLightMatrix( &fvtemp1, work->lights ) ;
	DG_SetLightMatrix( work->objs, work->lights ) ;



	/* アニメーション初期化 */
	if ( !(work->anime = InitVertexAnimation( work->objs->objs,
						  cdef->models,
						  DG_VANIME_VERTS|DG_VANIME_NORMS,
//						  DG_VANIME_VERTS,
						  cdef->n_models ) ) )
	PERROR( "InitVertexAnimation() returns NULL!! :: NewPutVanimeObject\n" ) ;

	for ( i=0 ; i<cdef->n_models ; i++ ){
		if ( cdef->models[i].n_verts == cdef->models[0].n_verts ){
			work->anime->key[i] = &cdef->models[i], work->anime->p[i] = 0.0f ;
		}else{
			PERROR( "Animation Vertexies are not same!! :: NewPutVanimeObject\n" ) ;
		}
	}


	work->anime->p[0] = 1.0f ;
	work->key      = 0 ;
	work->calc_key = 0 ;

	work->n_key = cdef->n_models ;

	work->mode = 0;
	work->count = 0;

	work->dir_flag = 0;

	if ( GCL_GetOption( 'b' ) != NULL ){
		fvtemp0.vx = (float)GCL_GetNextInt() ;
		fvtemp0.vy = (float)GCL_GetNextInt() ;
		fvtemp0.vz = (float)GCL_GetNextInt() ;
		fvtemp1.vx = (float)GCL_GetNextInt() ;
		fvtemp1.vy = (float)GCL_GetNextInt() ;
		fvtemp1.vz = (float)GCL_GetNextInt() ;

		work->bound_min.vx = (fvtemp0.vx < fvtemp1.vx)?fvtemp0.vx:fvtemp1.vx ;
		work->bound_min.vy = (fvtemp0.vy < fvtemp1.vy)?fvtemp0.vy:fvtemp1.vy ;
		work->bound_min.vz = (fvtemp0.vz < fvtemp1.vz)?fvtemp0.vz:fvtemp1.vz ;
		work->bound_max.vx = (fvtemp0.vx > fvtemp1.vx)?fvtemp0.vx:fvtemp1.vx ;
		work->bound_max.vy = (fvtemp0.vy > fvtemp1.vy)?fvtemp0.vy:fvtemp1.vy ;
		work->bound_max.vz = (fvtemp0.vz > fvtemp1.vz)?fvtemp0.vz:fvtemp1.vz ;
	}else{
		printf( "消化器：バウンディングボックス指定が無いのでデフォルトを設定\n" ) ;
		work->bound_min.vx = work->center.vx - 5000.0f ;
		work->bound_min.vy = work->center.vy - 5000.0f ;
		work->bound_min.vz = work->center.vz - 5000.0f ;
		work->bound_max.vx = work->center.vx + 5000.0f ;
		work->bound_max.vy = work->center.vy + 5000.0f ;
		work->bound_max.vz = work->center.vz + 5000.0f ;
	}
	SetAttackTarget( work );



	tex = DG_GetTexture( 9998494 /*"powder02_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
//	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	work->prim_count=0;
	work->width      = 0.0f;
	work->calc_width = 0.0f;


	id = 9363381 /*"hose_cone"*/;
	if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) ) PERROR( "No KMS found in data.cnf!!\n" ) ;
	work->objs_cone = DG_MakeObjs( def, DG_FLAG_SHADE | DG_FLAG_ONEPIECE, 0 );
	if( work->objs_cone == NULL ) return -1;
	GM_GroupObjs( work->objs_cone, work->where );
	DG_QueueObjs( work->objs_cone );
	DG_SetLightMatrix( work->objs_cone, work->lights_cone );





#if 0
	{
		float	ftemp0;
		float	ftemp1;
		int		mem;
		ftemp1 = 1000.0f;
		mem = 0;
		for ( i=0 ; i<cdef->models[1].n_verts ; i++ ){
			ftemp0 = work->cdef->models[1].verts[i].vy;
			printf("%d:%f::::%f:%f:%f\n",i,ftemp0,work->cdef->models[1].norms[i].vx,work->cdef->models[1].norms[i].vy,work->cdef->models[1].norms[i].vz);
			if( ftemp0 < ftemp1 ){
				ftemp1 = ftemp0;
				mem = i;
			}
		}
		printf("mem:%d\n",mem);
	}
#endif
	
	/* EDGE_0とEDGE_1が決め打ちでさらにPS2とXBOXでは違うので注意！！ yano 2002.08.21 */
	DG_COPY_VEC( &work->pos1, &work->cdef->models[work->calc_key].verts[EDGE_0] );
	DG_COPY_VEC( &work->pos3, &work->cdef->models[work->calc_key].verts[EDGE_1] );

	DG_SetPos( &work->objs->world );
	DG_PutVector( &work->pos1, &fvtemp0, 1 );
	DG_PutVector( &work->pos3, &fvtemp1, 1 );
	DG_GetLightMatrix( &fvtemp0, work->lights_cone );
	OK_DirVecXY_Y( &fvtemp0, &fvtemp1, &svtemp0 );
	DG_SetPos2( &fvtemp0, &svtemp0 );
	DG_GetPos( &work->cone_world );
	DG_PutObjs( work->objs_cone );

	work->se_flag = 2;
	work->se_count  = 0;


	return 0 ;
}


/*------------------------------------------------------------------*/
void *NewExtinguisher( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;
		work->where = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}



