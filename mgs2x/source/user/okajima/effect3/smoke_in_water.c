//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	smoke_in_water.c
	水中煙
	2001/04/25 S.Okajima
	$Id: smoke_in_water.c,v 1.1.1.3 2002/11/19 11:47:36 Yoshizawa1 Exp $

*/
//asm

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
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/


#define	RAISE			(-10000)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x4000 / 16 / N_VERTS)
#define N_LOOPS		(2)
#define	TOTAL_VERTS		(N_LOOPS*N_PRIMS*N_VERTS)

#define	COL_R_MIN	(64)
#define	COL_G_MIN	(64)
#define	COL_B_MIN	(64)

#define	COL_R_ADD	(64/N_LOOPS)
#define	COL_G_ADD	(64/N_LOOPS)
#define	COL_B_ADD	(64/N_LOOPS)

#define	COL_A	(32)


#define	VEC_MIN		(0.01f)
#define	VEC_RND		(32.0f)
#define	VEC_SCALE	(0.97f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	FVECTOR		center;
	float		size;
	int			size_add;
	int			life;
	int			life_max;

	FVECTOR	vec[TOTAL_VERTS];

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR	*sc_pos;
	FVECTOR	*vec;
	FVECTOR	scale;
	int	clock;
	int	alpha;
	int	num;
	int	i,j;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	 //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	alpha = COL_A * work->life / work->life_max;

	scale.vx = VEC_SCALE;
	scale.vy = VEC_SCALE;
	scale.vz = VEC_SCALE;
	scale.vw = VEC_SCALE;

#ifdef BP_PSX2_ASM ///

	asm volatile ("
		lqc2		vf4,0x00(%0)
		"::"r"(&scale):"memory"
	);
#endif

	pos        = prim->pos[  clock];
	pos_before = prim->pos[1-clock];
	uvrgbwh    = prim->uvrgb[clock];
	vec = work->vec;
	for ( j=0 ; j < N_LOOPS ; j++ ){
		OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		sc_pos = SCR_POS;
		for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
			num = i>>2;
			if(alpha-num < 0){
				uvrgbwh->a = 0;
				uvrgbwh->h = uvrgbwh->w = 0;
			}else{
				uvrgbwh->a = alpha-num;
				uvrgbwh->w+= work->size_add;
				uvrgbwh->h+= work->size_add;
#if 0
				sc_pos->vx += vec->vx;
				sc_pos->vy += vec->vy;
				sc_pos->vz += vec->vz;
#else
#ifdef BP_PSX2_ASM ///
				asm volatile ("
					lqc2		vf5,0x00(%0)
					lqc2		vf6,0x00(%1)
					vadd.xyz	vf5,vf5,vf6
					vmul.xyz	vf6,vf6,vf4
					sqc2		vf5,0x00(%0)
					sqc2		vf6,0x00(%1)
				"::"r"(sc_pos),"r"(vec):"memory"
				);
#else

//	asm volatile ("
//		lqc2		vf4,0x00(%0)
//		"::"r"(&scale):"memory"
//	);
				sc_pos->vx += vec->vx;
				sc_pos->vy += vec->vy;
				sc_pos->vz += vec->vz;

				vec->vx *= scale.vx;
				vec->vy *= scale.vy;
				vec->vz *= scale.vz;

#endif
#endif
			}
			vec++;
			sc_pos++;
			uvrgbwh++;
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		pos        += N_VERTS*N_PRIMS;
		pos_before += N_VERTS*N_PRIMS;
	}

	if( work->life-- < 0 ) GV_DestroyActor( work ) ;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos0;
	FVECTOR				*pos1;
	FVECTOR				*sc_pos;
	FVECTOR				*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i,j,k;
	float	angle0;
	float	angle1;
	float	base_radius;
	float	radius;
	float	base_size;
	float	size;
	float	vec_len;
	int		col_r;
	int		col_g;
	int		col_b;

	FVECTOR	center_step;
	FVECTOR	center_vec;
	SVECTOR	svtemp;

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	base_radius = work->size*0.75f;

	vec_len   = work->size * 8.0f / (float)work->life;
	base_size = work->size * 0.5f;

	DG_COPY_VEC( &center_step, &work->center );
	center_vec.vx = 0.0f;
	center_vec.vy = 0.0f;
	center_vec.vz = base_radius;


	work->size_add = work->size*0.25f / (float)work->life;

	col_r = COL_R_MIN;
	col_g = COL_G_MIN;
	col_b = COL_B_MIN;

	vec = work->vec;
	pos0       = prim->pos[0];
	pos1       = prim->pos[1];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i=0; i<N_LOOPS; i++ ){
		sc_pos = SCR_POS;
		for ( j=0; j<N_PRIMS; j++ ){
			center_vec.vz = base_radius * (rnd() + 0.75f);
			svtemp.vx = ((irnd()>>8)&2047) - 1024;
			svtemp.vy = ((irnd()>>8)&4095);
			svtemp.vz = 0;
			DG_SetPos2( &work->center, &svtemp );
			DG_PutVector( &center_vec, &center_step, 1 );
			for ( k=0; k<N_VERTS; k++ ){
				angle0 =  PI*rnd();
				angle1 = TPI*rnd();

				radius = sinf( angle0 );
				sc_pos->vx = radius * sinf( angle1 );
				sc_pos->vy = radius * cosf( angle1 );
				sc_pos->vz =          cosf( angle0 );

				_sceVu0ScaleVector( vec,    sc_pos,  vec_len*(rnd()*0.75f + 0.25f) );
				_sceVu0ScaleVector( sc_pos, sc_pos,  -base_radius );
				_sceVu0AddVector( sc_pos, &center_step, sc_pos );


				angle0 = PI*rnd();
				size   = base_size * (rnd()*0.75f + 0.25f);
				uvrgbwh1->w  = uvrgbwh0->w  = (int)(cosf( angle0 ) * size);
				uvrgbwh1->h  = uvrgbwh0->h  = (int)(sinf( angle0 ) * size);
				uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
				uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
				uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
				uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

				uvrgbwh1->r  = uvrgbwh0->r  = col_r ;
				uvrgbwh1->g  = uvrgbwh0->g  = col_g ;
				uvrgbwh1->b  = uvrgbwh0->b  = col_b ;
				uvrgbwh1->a  = uvrgbwh0->a  = 0 ;

				vec++;
				sc_pos++;
				uvrgbwh0 ++ ;
				uvrgbwh1 ++ ;
			}
		}
		col_r+= COL_R_ADD;
		col_g+= COL_G_ADD;
		col_b+= COL_B_ADD;
		OK_Scr_Mem( pos0, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		OK_Scr_Mem( pos1, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		pos0+= N_VERTS*N_PRIMS;
		pos1+= N_VERTS*N_PRIMS;
	}


	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

//	tex = DG_GetTexture( 15638432 /*"blood_1bw_msk"*/ );
//	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	if( !InitPacket2( work, prim, tex ) ) return -1;


	return 0 ;
}


/*
pos    :発生位置（へのポインタ）
vector :飛翔方向ベクトル（へのポインタ）
life   :消滅までのフレーム数
*/
void *NewSmokeInWater( FVECTOR *pos, float size, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		work->size = size;
		work->life = work->life_max = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
