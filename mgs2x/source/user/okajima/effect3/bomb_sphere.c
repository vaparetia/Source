//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_sphere.c
	水中爆炎エフェクト：球断片爆縮
	2001/05/03 S.Okajima
	$Id: bomb_sphere.c,v 1.1.1.3 2002/11/19 11:47:28 Yoshizawa1 Exp $

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
#include <libvu0.h>
#include <libdev.h>
#include <sifdev.h>
#endif

#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"gameheader.h"
#include	"../etc/ok_util.h"

#define RAISE		 (0)

#define	SCR_POS		(SCRPAD_ADDR + 0x00)

#define COL_R_INIT (90)
#define COL_G_INIT (90)
#define COL_B_INIT (90)

#define COL_R_DEST (32)
#define COL_G_DEST (32)
#define COL_B_DEST (32)

#define COL_A (64)

//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984


//#define	N_SIDES			(32)	/* max:32 */
#define	N_SIDES			(16)
//#define	N_SIDES			(8)
#define	N_TOTAL_SIDES	(N_SIDES * N_SIDES)

#define	N_LOOP0		(N_SIDES - 1)
#define	N_LOOP1		(2 * N_SIDES)
#define	N_TOTAL		( N_LOOP0 * N_LOOP1 )

#define	N_VERTS		(N_LOOP1)
#define	N_PRIMS		(N_TOTAL/N_VERTS)
//#define	N_PRIMS		(N_TOTAL/N_VERTS + N_TOTAL%N_VERTS)

#define	LIFE		(8)

#define	ANGLE_INIT	( PI * 0.25f )
#define	ANGLE_WIDTH	( PI * 0.75f )

#define	RADIUS_MIN	(1000.0f)
#define	RADIUS_RND	(250.0f)

#define	RADIUS_DECLAINE_RATIO	(0.95f)

#define	SCALE_DOWN_RATIO	(0.75f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	int			life;
	int			life_max;

	DG_PRIM2	*prim;

	FMATRIX		world;
	float	rotation_x;
	float	rotation_y;

	float	radius_angle;
	float	radius_angle_step;
	float	radius;
	float	radius_min;

	float	angle_h_init;
	float	angle_h;

} Work ;


ALIGN16_PRE float	ALIGN16_POST	OK_WorkSin[N_SIDES];
ALIGN16_PRE float	ALIGN16_POST	OK_WorkCos[N_SIDES];


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	FMATRIX		*sc_mat;
	FVECTOR		*latice_pos;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		fvtemp;
	int		clock;
	int		i,j,k;
	int		alpha;
	int		col_r;
	int		col_g;
	int		col_b;
	float	angle1;
	float	height;
	float	base_radius;
	float	radius;
	float	*p_sin;
	float	*p_cos;

	radius = cosf( work->radius_angle );
	radius*= radius;
	radius*= radius;
	base_radius = work->radius*(1.0f - SCALE_DOWN_RATIO)*radius
	            + work->radius*SCALE_DOWN_RATIO;
	work->radius_angle+= work->radius_angle_step;

	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;


	sc_mat = SCR_POS;
	DG_COPY_MAT( sc_mat, &work->world );
	DG_COPY_VEC( &fvtemp, (FVECTOR *)sc_mat->m[3] );
	DG_COPY_VEC( (FVECTOR *)sc_mat->m[3], &DG_ZeroVector );
	_sceVu0RotMatrixX( sc_mat, sc_mat, work->rotation_x );
	_sceVu0RotMatrixY( sc_mat, sc_mat, work->rotation_y );
	DG_COPY_VEC( (FVECTOR *)sc_mat->m[3], &fvtemp );
	DG_SetPos( sc_mat );
	DG_COPY_MAT( &work->world, sc_mat );


	alpha = COL_A * work->life / work->life_max;
	col_r =(COL_R_INIT - COL_R_DEST) * work->life / work->life_max + COL_R_DEST;
	col_g =(COL_G_INIT - COL_G_DEST) * work->life / work->life_max + COL_G_DEST;
	col_b =(COL_B_INIT - COL_B_DEST) * work->life / work->life_max + COL_B_DEST;

	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = uvrgb0;
	uvrgb1++;
	pos0   = prim->pos[clock];
	pos1   = pos0;
	pos1++;
	for ( i = 0 ; i < N_SIDES ; i++ ){
		//-------------------------------
		angle1 = work->angle_h  * (float)i / (float)N_SIDES + work->angle_h_init;
		height = base_radius * cosf( angle1 );
		radius = base_radius * sinf( angle1 );
		latice_pos = SCR_POS;
		p_sin = OK_WorkSin;
		p_cos = OK_WorkCos;
		for ( k = 0 ; k < N_SIDES ; k++ ){
			latice_pos->vx = radius * (*(p_sin++));
			latice_pos->vy = radius * (*(p_cos++));
			latice_pos->vz = height;
			latice_pos++;
		}
		latice_pos = SCR_POS;
		DG_PutVector( latice_pos, latice_pos, N_SIDES );

		if( i==0 ){
			for ( j = 0 ; j < N_SIDES ; j++ ){
				DG_COPY_VEC( pos0, latice_pos );	// 上段
				uvrgb0->r = col_r;
				uvrgb0->g = col_g;
				uvrgb0->b = col_b;
				uvrgb0->a = alpha;

				pos0+=2;
				latice_pos++;
				uvrgb0+= 2;
			}
		}else if( i==N_SIDES-1 ){
			for ( j = 0 ; j < N_SIDES ; j++ ){
				DG_COPY_VEC( pos1, latice_pos );	// 下段
				uvrgb1->r = col_r;
				uvrgb1->g = col_g;
				uvrgb1->b = col_b;
				uvrgb1->a = alpha;

				pos1+=2;
				latice_pos++;
				uvrgb1+= 2;
			}
		}else{
			for ( j = 0 ; j < N_SIDES ; j++ ){
				DG_COPY_VEC( pos1, latice_pos );	// 下段
				DG_COPY_VEC( pos0, latice_pos );	// 上段
				uvrgb0->r = uvrgb1->r = col_r;
				uvrgb0->g = uvrgb1->g = col_g;
				uvrgb0->b = uvrgb1->b = col_b;
				uvrgb0->a = uvrgb1->a = alpha;

				pos1+=2;
				pos0+=2;
				latice_pos++;
				uvrgb0+= 2;
				uvrgb1+= 2;
			}
		}
	}

	if( work->life-- <= 0) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i, k ;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	angle;
	float	*p_sin;
	float	*p_cos;


	work->angle_h_init = ANGLE_INIT  * rnd();
	work->angle_h      = ANGLE_WIDTH * (0.80f + rnd()*0.20f);


	p_sin = OK_WorkSin;
	p_cos = OK_WorkCos;
	for ( k = 0 ; k < N_SIDES ; k++ ){
//		angle = ANGLE_WIDTH * 2.0f * (float)k / (float)N_SIDES;
		angle = PI * (float)k / (float)N_SIDES;
		(*p_sin++) = sinf( angle );
		(*p_cos++) = cosf( angle );
	}

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	pos0   = prim->pos[ 0 ];
	pos1   = prim->pos[ 1 ];
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];

	if( (irnd()>>8)&1 ){
		ftemp0 = tex->v_scale * 2.0f / (float)(N_LOOP1-2);
		for ( i = 0 ; i < N_LOOP0 ; i++ ){
			ftemp1 = tex->u_scale * (float)(i  )/(float)N_LOOP0;
			ftemp2 = tex->u_scale * (float)(i+1)/(float)N_LOOP0;
			for ( k = 0 ; k < N_LOOP1 ; k++ ){
				if( k&1 ){
					uvrgb1->u = uvrgb0->u = FTOI12( ftemp2 + tex->u_offset );
				}else{
					uvrgb1->u = uvrgb0->u = FTOI12( ftemp1 + tex->u_offset );
				}
				uvrgb1->v = uvrgb0->v = FTOI12((float)(k>>1) * ftemp0 + tex->v_offset );
				uvrgb1->q = uvrgb0->q = 4096 ;
				uvrgb1->f = uvrgb0->f = (k==0)? 0x8fff: 0x0fff;
				uvrgb1->r = uvrgb0->r = 0;
				uvrgb1->g = uvrgb0->g = 0;
				uvrgb1->b = uvrgb0->b = 0;
				uvrgb1->a = uvrgb0->a = 0;
				uvrgb0++;
				uvrgb1++;
			}
		}
	}else{
		ftemp0 = tex->u_scale * 2.0f / (float)(N_LOOP1-2);
		for ( i = 0 ; i < N_LOOP0 ; i++ ){
			ftemp1 = tex->v_scale * (float)(i  )/(float)N_LOOP0;
			ftemp2 = tex->v_scale * (float)(i+1)/(float)N_LOOP0;
			for ( k = 0 ; k < N_LOOP1 ; k++ ){
				if( k&1 ){
					uvrgb1->v = uvrgb0->v = FTOI12( ftemp2 + tex->v_offset );
				}else{
					uvrgb1->v = uvrgb0->v = FTOI12( ftemp1 + tex->v_offset );
				}
				uvrgb1->u = uvrgb0->u = FTOI12((float)(k>>1) * ftemp0 + tex->u_offset );
				uvrgb1->q = uvrgb0->q = 4096 ;
				uvrgb1->f = uvrgb0->f = (k==0)? 0x8fff: 0x0fff;
				uvrgb1->r = uvrgb0->r = 0;
				uvrgb1->g = uvrgb0->g = 0;
				uvrgb1->b = uvrgb0->b = 0;
				uvrgb1->a = uvrgb0->a = 0;
				uvrgb0++;
				uvrgb1++;
			}
		}
	}



}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->life_max = work->life = LIFE;

	work->radius_angle      =-PI*0.5f*0.25f;
	work->radius_angle_step =(PI*0.5f-work->radius_angle) / (float)(LIFE+2);

//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	DG_InvisiblePrim2( work->prim );


#if 1
	work->rotation_x = PI*0.01f*frnd();
	work->rotation_y = PI*0.01f*frnd();
#else
	work->rotation_x = PI*0.04f*frnd();
	work->rotation_y = PI*0.04f*frnd();
#endif

	return 0 ;
}

void *NewBombSphere( FMATRIX *world, float radius )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_MAT( &work->world, world );
		work->radius     = radius;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

