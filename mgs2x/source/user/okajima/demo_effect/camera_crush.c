//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	camera_crush.c
	監視カメラ壊れエフェクト
	2000/10/13 S.Okajima
	$Id: camera_crush.c,v 1.1.1.3 2002/11/19 11:46:50 Yoshizawa1 Exp $

*/


#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"../etc/ok_util.h"

#define	JET_NUM		(4)
#define	MAX_VEC		(150.0f)

#define	N_PRIMS		(16)
#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/2)

#define	DUST_GRAVITY	( P_GRAVITY * 0.125f )

#define	ANGLE_STEP	(TPI * 0.0001f)
#define	RADIUS		(20.0f)

#define	SHIFT_HZD	(10.0f)

#define	SHIFT_UPPER	(10.0f)

#define	COL_R		(255)
#define	COL_G		(128)
#define	COL_B		(32)
#define	ALP_MAX		(255)

#define	FLASH_LENGTH	(2)
#define	FLASH_LENGTH_D	(FLASH_LENGTH * 5)

#define	LIFE_TIME			(60)
#define	FLOOR_CHECK_TIME	(15)

#define	VEC_SCALE		(0.001f)

#define	INIT_MULTIPLE_NUM	(2)
#define	ZOOM_RATIO			(1.012f)
#define	SEARCH_HZX			(1000)

#define	RAISE				(2000)

#define	SPD_MIN		(1.0f)
#define	SPD_MAX		(200.0f)
#define	DECAY_RATIO	(0.96f)

/*----------------------------------------------------------------*/
typedef	struct	{
	int			flag;
	float		limit;
} ATARI ;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			life;

	ATARI		flr;
	ATARI		sg0;
	ATARI		sg1;

	FVECTOR		vec[N_PRIMS*N_POLYS];
	DG_PRIM2	*prim ;
} Work ;

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
	r->vx = a->vx + b->vx ;
	r->vy = a->vy + b->vy ;
	r->vz = a->vz + b->vz ;
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
	output->vx = output->vx * input->vx ;
	output->vy = output->vy * input->vy ;
	output->vz = output->vz * input->vz ;
#endif
	return;
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	int		clock;
	int		alpha;
	int			map_id;
	FVECTOR		ratio;
	FVECTOR	*vec;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR	step;
	DG_PRIM2_UVRGB		*uvrgb ;


	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	ratio.vx = DECAY_RATIO;
	ratio.vy = DECAY_RATIO;
	ratio.vz = DECAY_RATIO;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	vec          = work->vec;
	pos          = work->prim->pos[  clock];
	pos_before   = work->prim->pos[1-clock];
	pos_before++;
	uvrgb        = work->prim->uvrgb[  clock];
	alpha        = ALP_MAX * work->life / LIFE_TIME;
	if( alpha < 0 ) alpha = 0;
	if( LIFE_TIME - work->life < FLOOR_CHECK_TIME ){
		for ( i=0; i<N_PRIMS; i++ ){
			if( i<N_PRIMS/2 ){
				vec->vw += ANGLE_STEP;
			}else{
				vec->vw -= ANGLE_STEP;
			}
			step.vx  = RADIUS * cosf( vec->vw );
			step.vy  = RADIUS * sinf( vec->vw );
			step.vz  = RADIUS * sinf( vec->vw );
			for ( j=0; j<N_POLYS; j++ ){
				DG_COPY_VEC( pos++, pos_before );
				MulVector( vec, &ratio );
				vec->vy += (P_GRAVITY*0.125f);
				AddVector( pos, pos_before, vec  );
				AddVector( pos, pos,        &step  );
				if( work->sg0.flag==1 ){
					if( pos->vx > work->sg0.limit ){
						pos->vx = work->sg0.limit - SHIFT_HZD;
						vec->vx =-vec->vx ;
					}
				}else if( work->sg0.flag==2 ){
					if( pos->vx < work->sg0.limit ){
						pos->vx = work->sg0.limit + SHIFT_HZD;
						vec->vx =-vec->vx ;
					}
				}
				if( work->sg1.flag==1 ){
					if( pos->vz > work->sg1.limit ){
						pos->vz = work->sg1.limit - SHIFT_HZD;
						vec->vz =-vec->vz ;
					}
				}else if( work->sg1.flag==2 ){
					if( pos->vz < work->sg1.limit ){
						pos->vz = work->sg1.limit + SHIFT_HZD;
						vec->vz =-vec->vz ;
					}
				}
				if( vec->vy < 0.0f ){	/* 落下中 */
					if( work->flr.flag ){
						if( work->flr.limit >= pos->vy ){
							pos->vy = work->flr.limit + SHIFT_HZD;
							vec->vy =-vec->vy ;
						}
					}
				}

				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha;

				pos++;
				pos_before+=2;
				vec++;
			}
		}
	}else{
		for ( i=0; i<N_PRIMS; i++ ){
			if( i<N_PRIMS/2 ){
				vec->vw += ANGLE_STEP;
			}else{
				vec->vw -= ANGLE_STEP;
			}
			step.vx  = RADIUS * cosf( vec->vw );
			step.vy  = RADIUS * sinf( vec->vw );
			step.vz  = RADIUS * sinf( vec->vw );

			step.vx  = 0.0f;
			step.vy  = 0.0f;
			step.vz  = 0.0f;
			for ( j=0; j<N_POLYS; j++ ){
				DG_COPY_VEC( pos++, pos_before );
				MulVector( vec, &ratio );
				vec->vy += (DUST_GRAVITY);
				AddVector( pos, pos_before, vec );
				AddVector( pos, pos,        &step  );

				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha;

				pos++;
				pos_before+=2;
				vec++;
			}
		}
	}

	work->life--;
	if( work->life < -1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}




static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	int	i,j;
	FVECTOR	fvtemp;
	FVECTOR	*pos_0;
	FVECTOR	*pos_1;
	DG_PRIM2_UVRGB		*uvrgb_0 ;
	DG_PRIM2_UVRGB		*uvrgb_1 ;

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	DG_COPY_VEC( &fvtemp, center );
	fvtemp.vy += SHIFT_UPPER;

	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	pos_0   = prim->pos[ 0 ] ;
	pos_1   = prim->pos[ 1 ] ;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			DG_COPY_VEC( pos_0, &fvtemp );
			DG_COPY_VEC( pos_1, &fvtemp );
			uvrgb_0->u = uvrgb_1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x8fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = ALP_MAX ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;


			DG_COPY_VEC( pos_0, center );
			DG_COPY_VEC( pos_1, center );
			uvrgb_0->u = uvrgb_1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x0fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = ALP_MAX ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;
		}
	}
}


/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos )
{
	int	i,j;
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	HZX_GROUP_ID	map_id;
	float		flr_height[2];
	float		rad;
	float		ftemp;
	FVECTOR		fvtemp;
	FVECTOR		*vec;
	int			flr_flag;
	int			seg_num;
	int			atr[2] ;
	HZX_SEG		seg[2] ;
	SVECTOR		pole_rot;
	SVECTOR		svtemp;

	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	prim  = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim == NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, pos );
	DG_VisiblePrim2( prim ) ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;


	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	svtemp.vz = 0;
	vec = work->vec;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			fvtemp.vz = SPD_MIN + SPD_MAX*rnd();
			svtemp.vx = irnd()%4096;
			svtemp.vy = irnd()%2048;
			DG_SetPos2( &DG_ZeroVector, &svtemp );
			DG_RotVector( &fvtemp, vec, 1 );
			vec->vw = TPI * rnd();
			vec++;
		}
	}


	//----------------------------------
	work->flr.flag = 0;
	work->sg0.flag = 0;
	work->sg1.flag = 0;

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	flr_flag = HZX_LevelHazardCheck( map_id, pos, HZX_CHK_ALL, 0 );
	if( flr_flag & 1 ){
		HZX_GetLevelHeight( flr_height );
		work->flr.flag  = 1;
		work->flr.limit = flr_height[0];
	}

	seg_num = HZX_NearHazardCheck( map_id,
					pos,
					SEARCH_HZX,
					HZX_CHK_RECOIL_TYPE_ONLY,0,
					SEARCH_HZX);

//printf("seg_num:%d\n",seg_num);

	/* 反射ベクトル */
	pole_rot.vy = 0;
	pole_rot.vz = 0;
	rad = 0.0f;	/* Ｘ回転の合計を計算する */
	if( seg_num ){
		HZX_GetNearHazard( seg, atr ) ;
		HZX_GetReactVector( &fvtemp );

		if( seg[0].p1.x - seg[0].p2.x == 0.0f ){
			if( !((seg[0].p1.z > pos->vz) && (seg[0].p2.z > pos->vz))
			 && !((seg[0].p1.z < pos->vz) && (seg[0].p2.z < pos->vz)) ){
				if( pos->vx < seg[0].p1.x ){
					work->sg0.flag  = 1;
				}else{
					work->sg0.flag  = 2;
				}
				work->sg0.limit = seg[0].p1.x;
			}
		}else if( (seg_num >= 2) && ( seg[1].p1.x - seg[1].p2.x == 0.0f) ){
			if( !((seg[1].p1.z > pos->vz) && (seg[1].p2.z > pos->vz))
			 && !((seg[1].p1.z < pos->vz) && (seg[1].p2.z < pos->vz)) ){
				if( pos->vx < seg[1].p1.x ){
					work->sg0.flag  = 1;
				}else{
					work->sg0.flag  = 2;
				}
				work->sg0.limit = seg[1].p1.x;
			}
		}
		if( seg[0].p1.z - seg[0].p2.z == 0.0f ){
			if( !((seg[0].p1.x > pos->vx) && (seg[0].p2.x > pos->vx))
			 && !((seg[0].p1.x < pos->vx) && (seg[0].p2.x < pos->vx)) ){
				if( pos->vz < seg[0].p1.z ){
					work->sg1.flag  = 1;
				}else{
					work->sg1.flag  = 2;
				}
				work->sg1.limit = seg[0].p1.z;
			}
		}else if( (seg_num >= 2) && ( seg[1].p1.z - seg[1].p2.z == 0.0f) ){
			if( !((seg[1].p1.x > pos->vx) && (seg[1].p2.x > pos->vx))
			 && !((seg[1].p1.x < pos->vx) && (seg[1].p2.x < pos->vx)) ){
				if( pos->vz < seg[1].p1.z ){
					work->sg1.flag  = 1;
				}else{
					work->sg1.flag  = 2;
				}
				work->sg1.limit = seg[1].p1.z;
			}
		}

		ftemp = atan2f( fvtemp.vx, fvtemp.vz ) ;
		pole_rot.vy = 4095 & ( short )( ( ftemp * 2048.0f / PI ) + 0.5f );

		rad = PI * 0.5f;
		if( flr_flag & 1 ){
			ftemp = pos->vy - flr_height[0];
			if( ftemp > 0 ){
				rad += asinf( ftemp * VEC_SCALE );
			}
		}
	}
	pole_rot.vx = 4095 & ( short )( ( rad * 1024.0f / PI ) + 0.5f );


//printf("f0:%d:%f\n",work->flr.flag,work->flr.limit);
//printf("s0:%d:%f\n",work->sg0.flag,work->sg0.limit);
//printf("s1:%d:%f\n",work->sg1.flag,work->sg1.limit);


	work->life = LIFE_TIME;

	return 0 ;
}


/* 飛び散り */
void *NewCrushWithForce( FVECTOR *pos )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
