//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	smoke_strip.c
	ストリップ煙
	2001/08/09 S.Okajima
	$Id: smoke_strip.c,v 1.1.1.3 2002/11/19 11:47:36 Yoshizawa1 Exp $

*/



#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(64)
#define	N_PRIMS		(1)
#define	N_VERTS_HALF	(N_VERTS/2)
#define	N_DATA		(N_PRIMS*N_VERTS_HALF)

#define	VEC_SCALE	(0.5f)

typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		world;
	FMATRIX		root;
	FVECTOR		shift_pos;
	FVECTOR		shift_vec;
	float		radius0;
	float		radius1;
	float		radius_dest;
	float		angle_start;
	float		angle0;
	float		angle1;
	float		width_ratio0;
	float		width_ratio1;
	float		scale_x;
	float		vec_scale;
	float		slide_x;
	int			col;
	int			mode;
	int			life;
	int			life_max;

	int			seed;

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	FVECTOR			shift_pos;
	FVECTOR			shift_pos_diff;
	FVECTOR			fvtemp;
	DG_PRIM2_UVRGB	*uvrgb;
	float	vec_scale;
	float	radius_max;
	float	radius_diff;
	float	radius;
	float	angle_diff;
	float	angle;
	float	width_ratio_diff;
	float	width_ratio;
	float	time_ratio;
	float	alpha_max;
	float	slide_x;
	int		j ;
	int		clock;
	int		alpha;
	int		alpha_d;

	alpha_max = (float)(work->col&255);

	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	time_ratio = (float)work->life / (float)work->life_max;

	switch( work->mode ){
	  default:
	  case 0:
		DG_COPY_MAT( &work->root, &work->world );
		break;
	  case 1:
		DG_COPY_MAT( &work->root, &DG_Chanls->eye );
		DG_COPY_VEC( (FVECTOR *)work->root.m[3], (FVECTOR *)work->world.m[3] );
		break;
	}

	vec_scale = work->vec_scale*time_ratio;

	work->radius0 = work->radius0*0.995f + work->radius_dest*0.005f;
	radius_max  = work->radius0;
	radius_diff = ((work->radius1-work->radius0)*(1.0f - time_ratio*0.5f)) / (float)N_DATA;
	radius      = work->radius0;

	angle_diff  = (work->angle0 + work->angle1*(1.0f - time_ratio) ) / (float)N_DATA;
	angle       = work->angle_start;

	width_ratio_diff = (work->width_ratio1 - work->width_ratio0) / (float)N_DATA;
	width_ratio      = work->width_ratio0;

	DG_COPY_VEC( &shift_pos, &DG_ZeroVector );

	_sceVu0AddVector( &work->shift_pos, &work->shift_pos, &work->shift_vec );
	_sceVu0ScaleVector( &shift_pos_diff, &work->shift_pos, 1.0f/(float)N_DATA );

	slide_x = 0.0f;

	alpha = (int)( alpha_max * sinf( PI*time_ratio ) );
	pos0   = SCR_POS;
	pos1   = SCR_POS;
	pos1++;
	uvrgb  = prim->uvrgb[clock];
	for ( j = 0 ; j < N_DATA ; j++ ){
		pos0->vx = radius * sinf(angle);
		pos0->vy = radius * cosf(angle);
		pos0->vz = 0.0f;

		angle += angle_diff;
		radius+= radius_diff;

		_sceVu0ScaleVector( pos1, pos0, width_ratio );
		width_ratio += width_ratio_diff;

		_sceVu0AddVector( pos0, pos0, &shift_pos );
		_sceVu0AddVector( pos1, pos1, &shift_pos );

		_sceVu0ScaleVector( &fvtemp, &shift_pos_diff, radius * vec_scale );
		_sceVu0AddVector( &shift_pos, &shift_pos, &fvtemp );

		pos0->vx += slide_x;
		pos1->vx += slide_x;
		slide_x  += work->slide_x;

		alpha_d = alpha*j/N_DATA;
		(uvrgb++)->a = alpha_d;
		(uvrgb++)->a = alpha_d;

		pos0+=2;
		pos1+=2;
	}

	OK_Scr_Mem( prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);

	if( --work->life <= 0 ) GV_DestroyActor( work ) ;


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb;
	int		i,j ;
	int	col_r;
	int	col_g;
	int	col_b;
	int	side;

	col_r = ((work->col>>24)&255);
	col_g = ((work->col>>16)&255);
	col_b = ((work->col>> 8)&255);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	side = (GM_IRnd(&work->seed)>>8)&1;

	uvrgb = SCR_TMP;
	if( (GM_IRnd(&work->seed)>>8)&1 ){
		for ( i=0; i<N_PRIMS; i++ ){
			for ( j=0; j<N_VERTS; j++ ){
				uvrgb->u = FTOI12((float)(j/2) * 2.0f /(float)(N_VERTS) * tex->u_scale + tex->u_offset );
				uvrgb->v = ((side+j)&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
				uvrgb->q = 4096 ;
				uvrgb->f = (j<2)? 0x8fff: 0x0fff;
				uvrgb->r = col_r ;
				uvrgb->g = col_g ;
				uvrgb->b = col_b ;
				uvrgb->a = 0;
				uvrgb++;
			}
		}
	}else{
		for ( i=0; i<N_PRIMS; i++ ){
			for ( j=0; j<N_VERTS; j++ ){
				uvrgb->u = FTOI12((float)((N_VERTS - j)/2) * 2.0f /(float)(N_VERTS) * tex->u_scale + tex->u_offset );
				uvrgb->v = ((side+j)&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
				uvrgb->q = 4096 ;
				uvrgb->f = (j<2)? 0x8fff: 0x0fff;
				uvrgb->r = col_r ;
				uvrgb->g = col_g ;
				uvrgb->b = col_b ;
				uvrgb->a = 0;
				uvrgb++;
			}
		}
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	prim->root = &work->root;

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex=NULL ;

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 7744599 /*"hamon09_add_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	switch( (GM_IRnd(&work->seed)>>8)%5 ){
	  case 0:
		tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
		break;
	  case 1:
		tex = DG_GetTexture( 12020883 /*"chi02_alp"*/ );
		break;
	  case 2:
		tex = DG_GetTexture( 13069459 /*"chi03_alp"*/ );
		break;
	  case 3:
		tex = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
		break;
	  case 4:
		tex = DG_GetTexture( 15166611 /*"chi05_alp"*/ );
		break;
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( prim );

	return 0 ;
}
#if 0

// mode:0 マトリクスの方向をそのまま使用
// mode:1 カメラ方向に向ける
// side:0 左周り
// side:1 右周り

#endif

void *NewSmokeStrip( FMATRIX *world, float radius, int life, int col, int mode, int side, int seed )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->seed = seed;

		work->slide_x = radius * ( 0.02f + 0.05f*GM_Rnd(&work->seed) );

		work->col  = col;
		work->mode = mode;

		work->radius0     = radius*(1.0f + 0.1f*GM_FRnd(&work->seed));
		work->radius1     = work->radius0*(0.25f + 0.75f*GM_Rnd(&work->seed));
		work->radius_dest = work->radius0*(1.5f + 2.0f*GM_Rnd(&work->seed));

		work->angle_start = PI * (1.5f + GM_FRnd(&work->seed)*0.2f);
		work->angle0      = PI*(0.5f + 0.25f*rnd());
		work->angle1      = work->angle0 + PI*(0.25f + GM_Rnd(&work->seed)*2.0f);

		if(side){
			work->angle0 =-work->angle0;
			work->angle1 =-work->angle1;
		}

//		DG_COPY_VEC( &work->center, center );
		DG_COPY_MAT( &work->world, world );
		work->life     = life;
		work->life_max = life;
		work->width_ratio0 = GM_Rnd(&work->seed)*0.1f;
		work->width_ratio1 = 0.9f + GM_Rnd(&work->seed)*0.1f;

		work->scale_x = 1.0f + GM_Rnd(&work->seed);

		work->vec_scale = VEC_SCALE / radius;

		DG_COPY_VEC( &work->shift_pos, &DG_ZeroVector );
		work->shift_vec.vx = 100.0f;
		work->shift_vec.vy = 0.0f;
		work->shift_vec.vz = 0.0f;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

