//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	explosion_parts.c
	爆発の部品
	2001/07/20 S.Okajima
	$Id: explosion_parts.c,v 1.1.1.3 2002/11/19 11:47:31 Yoshizawa1 Exp $

	pack_trnall stage_effect.tri \
		effect/bakuha/bombsphere01_alp.bmp \
		effect/bakuha/bombsphere03_alp.bmp \
		effect/bakuha/bombsphere04_alp.bmp

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
#include	"utl_dma.h"

#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../etc/ok_util.h"
#include	"./explosion.h"

#define DECAY_VEC_RATIO	 (0.94f)




#define VEC_ANGLE_WIDTH	 (PI*0.5f)

//#define NORM_LIMIT	 (-0.1f)
#define NORM_LIMIT	 (0.0f)

#define VEC_DECAY_RATIO	 (0.95f)

#define RAISE		 (0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)

#define	SCROLL_DIV_MIN			(48.0f)
#define	SCROLL_DIV_RND			(32.0f)

#define SIZE_VEC_INIT_RATIO		 (0.5f)
#define	SCROLL_MIN_STEP			(0.1f)

#define	REBIRTH_COUNT_MIN		(15)
#define	REBIRTH_COUNT_RND		(10)

#define	MUL_NUM		(2)

#define	SCALE_MIN	(0.75f)
#define	SCALE_RND	(0.25f)

#define	SHIFT_WIDTH_U_RATIO	(0.5f)
#define	SHIFT_WIDTH_V_RATIO	(0.5f)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	ALIGN16_PRE DG_PRIM2_UVRGB	uvrgb[2][EXPLO_N_PRIMS*EXPLO_N_VERTS] ALIGN16_POST;

	DG_PRIM2	*prim_master;
	DG_PRIM2	*prim_m[MUL_NUM-1];

	FMATRIX		world[MUL_NUM];
	FVECTOR		pos[MUL_NUM];
	FVECTOR		vec[MUL_NUM];
	FVECTOR		fv_rot[MUL_NUM];
	FVECTOR		fv_rot_step[MUL_NUM];
	float		scale[MUL_NUM];

	float		shift_u[EXPLO_N_SIDES0*EXPLO_N_SIDES1];

	float		size;
	float		size_min;
	float		size_half;
	float		size_vec_init;
	float		size_vec;

	float		offset;
	float		shift_max;
	float		scroll_count;
	float		scroll_div;
	float		decay_param;

	float		alpha;

	int			col;
	int			rebirth_num;
	int			rebirth_count;
} Work ;

/* ---------------------------------------------------------------- */
extern FVECTOR	OK_EXPLO_WorkTemp0[];
extern int		OK_EXPLO_DESTROY_FLAG;
extern int		OK_EXPLO_SEED;

/* ---------------------------------------------------------------- */
void *NewExplosionParts( FVECTOR *center, FVECTOR *vec, float size, int col, float life_param, float decay_param, int rebirth_num );

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim_master;
	DG_PRIM2		*prim;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	FVECTOR		*base0;
	FVECTOR		*base1;
	FVECTOR		*fv_rot;
	FVECTOR		*fv_rot_step;
	FVECTOR		fvtemp;
	FMATRIX		*world;
	FMATRIX		unit_m;
	FMATRIX		eye_inv;
	FMATRIX		eye;
	FMATRIX		fmat0;
	int		clock;
	int		i,j;
	float	*scale;
	float	*shift_u;
	float	ratio;
	float	alpha;
	float	shift;
	float	ftemp1;


	work->size+= work->size_vec;
	work->size_vec*= work->decay_param;

	if( work->size_vec <= work->size_min
	 || OK_EXPLO_DESTROY_FLAG!=0 ){
		DG_InvisiblePrim2( work->prim_master );
		for( i=0; i<MUL_NUM-1; i++ ){
			prim = work->prim_m[i];
			DG_InvisiblePrim2( prim );
		}
		GV_DestroyActor( work ) ;
		return;
	}

	ratio = (work->size_vec-work->size_min) / (work->size_vec_init-work->size_min);
	alpha = work->alpha * sinf( PI*ratio );
	if( alpha < 0.0f ){
		alpha = 0.0f;
	}else if( alpha > 255.0f ){
		alpha = 255.0f;
	}

	prim_master = work->prim_master;
	DG_VisiblePrim2( prim_master );
	GM_GroupPrim2( prim_master, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_master );
	clock = prim_master->buffer_clock;



	for( i=0; i<MUL_NUM-1; i++ ){
		prim = work->prim_m[i];
		DG_VisiblePrim2( prim );
//		DG_InvisiblePrim2( prim );
		GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
		DG_SwitchBuffPrim2( prim );
	}




	shift = work->shift_max * work->scroll_count / work->scroll_div
	      + work->offset;


	shift_u = work->shift_u;
	uvrgb0  = prim_master->uvrgb[clock];
	uvrgb1  = uvrgb0;
	uvrgb1++;
	for ( i = 0 ; i < EXPLO_N_SIDES0 ; i++ ){
		ftemp1 = work->shift_max * (float)(i)/(float)EXPLO_N_SIDES0 + shift;
		if( i==0 ){
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				uvrgb0->a = 0;
				uvrgb0->u = FTOI12( ftemp1 + (*shift_u) );
				uvrgb0+= 2;
				shift_u++;
			}
		}else if( i==EXPLO_N_SIDES0-1 ){
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				uvrgb1->a = 0;
				uvrgb1->u = FTOI12( ftemp1 + (*shift_u) );
				uvrgb1+= 2;
				shift_u++;
			}
		}else{
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
#if 0
				uvrgb0->a = uvrgb1->a = (int)(alpha);
#else
				uvrgb0->a = uvrgb1->a = (int)(alpha*(float)((i+j)%EXPLO_N_SIDES1)/(float)EXPLO_N_SIDES1);
#endif
				uvrgb0->u = uvrgb1->u = FTOI12( ftemp1 + (*shift_u) );
				uvrgb0+= 2;
				uvrgb1+= 2;
				shift_u++;
			}
		}
	}



//	OK_Scr_Mem( work->prim_master->pos[clock], SCR_POS, sizeof(FVECTOR), EXPLO_N_PRIMS*EXPLO_N_VERTS );

	//-----------------------------------------------------計算用マトリクス
	// ワールドに置いてからカメラ前に変換（回転のみ使用前提）
	// Ｚ軸を潰し、ワールドに戻す
	// 頂点座標計算用
	DG_COPY_MAT( &unit_m, &DG_UnitMatrix );
//	unit_m.m[2][2] = 0.25f;						// Ｚ軸方向に潰す
	unit_m.m[2][2] = 0.1f;						// Ｚ軸方向に潰す
//	unit_m.m[2][2] = 8.0f;						// Ｚ軸方向に潰す



	//-----------------------------------------------------計算用マトリクス
	// ワールドに置いてからカメラ前に変換（回転のみ使用前提）
	DG_COPY_MAT( &eye_inv, &DG_Chanls->eye_inv );
	DG_COPY_VEC( (FVECTOR *)eye_inv.m[3], &DG_ZeroVector );

	DG_COPY_MAT( &eye, &DG_Chanls->eye );

	scale = work->scale;
	fv_rot      = work->fv_rot;
	fv_rot_step = work->fv_rot_step;
	base0 = work->pos;
	base1 = work->vec;
	world = work->world;
	for( i=0; i<MUL_NUM; i++ ){
#if 0
		_sceVu0Normalize( &fvtemp, base1 );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, work->size_vec );
		_sceVu0AddVector( base0, base0, &fvtemp ) ;
#else
		_sceVu0AddVector( base0, base0, base1 ) ;
		base1->vy-= P_GRAVITY*0.5f * (float)i/(float)MUL_NUM;
#endif
		base0->vw = 1.0f;	// 必用なのだ
		_sceVu0ScaleVector( base1, base1, DECAY_VEC_RATIO );

#if 1
		// 微妙に回転させる
		fv_rot->vx+= fv_rot_step->vx;
		if( fv_rot->vx > PI ){
			fv_rot->vx-= TPI;
		}else if( fv_rot->vx <-PI ){
			fv_rot->vx+= TPI;
		}
		fv_rot->vy+= fv_rot_step->vy;
		if( fv_rot->vy > PI ){
			fv_rot->vy-= TPI;
		}else if( fv_rot->vy <-PI ){
			fv_rot->vy+= TPI;
		}
#endif
		DG_COPY_MAT( &fmat0, &DG_UnitMatrix );
		fmat0.m[0][0]*= (*scale);
		fmat0.m[1][1]*= (*scale);
		fmat0.m[2][2]*= (*scale);

		_sceVu0RotMatrixX( &fmat0, &fmat0, fv_rot->vx ) ;
		_sceVu0RotMatrixY( &fmat0, &fmat0, fv_rot->vy ) ;

		_sceVu0MulMatrix( &fmat0, &eye_inv, &fmat0 );

		_sceVu0MulMatrix( world, &unit_m, &fmat0 );	// fmat0:カメラ前座標系（ゼロ原点）
		DG_COPY_VEC( (FVECTOR *)eye.m[3], base0 );
		_sceVu0MulMatrix( world, &eye, world );	// ワールドに戻す

		scale++;
		base0++;
		base1++;
		fv_rot++;
		fv_rot_step++;
		world++;
	}


	work->scroll_count+= (work->size_vec/work->size_vec_init + SCROLL_MIN_STEP);
	if( work->scroll_count > work->scroll_div ) work->scroll_count = 0.0f;

	if( work->rebirth_num > 0 ){
		if( work->rebirth_count > 0 ){
			work->rebirth_count--;
			if( work->rebirth_count<= 0 ){
				int	col;
				work->rebirth_num--;
#if 1
				i = (GM_IRnd( &OK_EXPLO_SEED )>>8)&255;
				col = (i<<24)|(i<<16)|(i<<8)|(int)(work->alpha);
#else
				col = 0xffffffff;
#endif
				i = (GM_IRnd( &OK_EXPLO_SEED )>>8)%MUL_NUM;
				DG_COPY_VEC( &fvtemp, &work->pos[i] );
				fvtemp.vy+= work->size_half;

				NewExplosionParts(
					&fvtemp,
					&work->vec[i],
					work->size_half,
					col,
					0.15f+0.15f*GM_Rnd( &OK_EXPLO_SEED ),
					work->decay_param-0.01f, 
					work->rebirth_num );
			}
		}
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int	i;

	for( i=0; i<MUL_NUM-1; i++ ){
		work->prim_m[i] = OK_FreePrim2( work->prim_m[i] );
	}
	work->prim_master = OK_FreePrim2( work->prim_master );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	DG_PRIM2_UVRGB	*uvrgb0_s;
	DG_PRIM2_UVRGB	*uvrgb1_s;
	int		i, j, k ;
	int		col_r;
	int		col_g;
	int		col_b;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	*shift_u;
	float	shift_width_max_u;
	float	shift_width_max_v;

	col_r = (work->col>>24)&255;
	col_g = (work->col>>16)&255;
	col_b = (work->col>> 8)&255;


	work->offset    = tex->u_offset;
	work->shift_max = tex->u_scale*0.5f;

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	ftemp0 = tex->v_scale * 2.0f / (float)(EXPLO_N_LOOP1-2);
	for ( i = 0 ; i < EXPLO_N_LOOP0 ; i++ ){
		ftemp1 = tex->u_scale * (float)(i  )/(float)EXPLO_N_LOOP0;
		ftemp2 = tex->u_scale * (float)(i+1)/(float)EXPLO_N_LOOP0;
		for ( k = 0 ; k < EXPLO_N_LOOP1 ; k++ ){
/*
			if( k&1 ){
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp2 + tex->u_offset );
			}else{
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp1 + tex->u_offset );
			}
*/
//			uvrgb1->v = uvrgb0->v = FTOI12((float)(k>>1) * ftemp0 + tex->v_offset );

			uvrgb1->q = uvrgb0->q = 4096 ;
//			uvrgb1->q = uvrgb0->q = 4096 + (GM_IRnd( &OK_EXPLO_SEED )>>8)%256-128;
			uvrgb1->f = uvrgb0->f = (k==0)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = col_r;
			uvrgb1->g = uvrgb0->g = col_g;
			uvrgb1->b = uvrgb0->b = col_b;
			uvrgb1->a = uvrgb0->a = 0;
			uvrgb0++;
			uvrgb1++;
		}
	}




	shift_u = work->shift_u;
	shift_width_max_u = tex->u_scale * SHIFT_WIDTH_U_RATIO /(float)EXPLO_N_SIDES0;
	shift_width_max_v = tex->v_scale * SHIFT_WIDTH_V_RATIO /(float)EXPLO_N_SIDES1;
	ftemp0 = tex->v_scale / (float)(EXPLO_N_SIDES1-1);
	uvrgb0   = prim->uvrgb[ 0 ];
	uvrgb1   = uvrgb0;
	uvrgb1++;
	uvrgb0_s = prim->uvrgb[ 1 ];
	uvrgb1_s = uvrgb0_s;
	uvrgb1_s++;
	for ( i = 0 ; i < EXPLO_N_SIDES0 ; i++ ){
		if( i==0 ){
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				uvrgb0->v = uvrgb0_s->v = FTOI12( (float)(j)*ftemp0 + tex->v_offset + shift_width_max_v*GM_FRnd( &OK_EXPLO_SEED ) );
				uvrgb0  += 2;
				uvrgb0_s+= 2;
				(*shift_u) = shift_width_max_u*GM_Rnd( &OK_EXPLO_SEED );
				shift_u++;
			}
		}else if( i==EXPLO_N_SIDES0-1 ){
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				uvrgb1->v = uvrgb1_s->v = FTOI12( (float)(j)*ftemp0 + tex->v_offset + shift_width_max_v*GM_Rnd( &OK_EXPLO_SEED ) );
				uvrgb1  += 2;
				uvrgb1_s+= 2;
				(*shift_u) = shift_width_max_u*GM_Rnd( &OK_EXPLO_SEED );
				shift_u++;
			}
		}else{
			for ( j = 0 ; j < EXPLO_N_SIDES1 ; j++ ){
				uvrgb0->v = uvrgb1->v = uvrgb0_s->v = uvrgb1_s->v = FTOI12( (float)(j)*ftemp0 + tex->v_offset + shift_width_max_v*GM_Rnd( &OK_EXPLO_SEED ) );
				uvrgb0  += 2;
				uvrgb1  += 2;
				uvrgb0_s+= 2;
				uvrgb1_s+= 2;
				(*shift_u) = shift_width_max_u*GM_Rnd( &OK_EXPLO_SEED );
				shift_u++;
			}
		}
	}







}

static int GetResources( Work *work, FVECTOR *center, FVECTOR *master_vec, float life_param )
{
	DG_PRIM2	*prim_master ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex = NULL ;
	FVECTOR		fvtemp0;
	FVECTOR		*pos;
	FVECTOR		*vec;
	FVECTOR		*fv_rot;
	FVECTOR		*fv_rot_step;
	DG_PRIM2_UVRGB	*uvrgb ;
	int	i,j;
	float	angle0;
	float	angle1;
	float	radius0;
	float	radius1;
	float	*scale;

	work->alpha = (float)(work->col&255) * (0.75f + 0.25f*GM_Rnd( &OK_EXPLO_SEED ));
	work->scroll_div = SCROLL_DIV_MIN + SCROLL_DIV_RND*((float)(REBIRTH_NUM - work->rebirth_num) / (float)REBIRTH_NUM)*GM_Rnd( &OK_EXPLO_SEED );

	work->rebirth_count = REBIRTH_COUNT_MIN + (GM_IRnd( &OK_EXPLO_SEED )>>8)%REBIRTH_COUNT_RND;
	work->size_half = work->size*(0.5f + 0.5f*GM_Rnd( &OK_EXPLO_SEED ));
	work->size_min  = work->size*(0.02f + 0.03f*GM_Rnd( &OK_EXPLO_SEED )) ;	// 終了条件

//	work->size_vec = work->size_vec_init = work->size * SIZE_VEC_INIT_RATIO;
	work->size_vec = work->size_vec_init = work->size * life_param;

	work->scroll_count = 0;

	fv_rot      = work->fv_rot;
	fv_rot_step = work->fv_rot_step;
	pos = work->pos;
	vec = work->vec;
	scale = work->scale;
	for( i=0; i<MUL_NUM; i++ ){
#if 1
		fv_rot->vx = atan2f( master_vec->vx, master_vec->vz ) + VEC_ANGLE_WIDTH*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot->vy = atan2f( master_vec->vy, bp_sqrtf( master_vec->vx*master_vec->vx + master_vec->vz*master_vec->vz ) ) + VEC_ANGLE_WIDTH*GM_Rnd( &OK_EXPLO_SEED );   //BP_MATH - emulate PS2 sqrtf
		fv_rot->vz = 0.0f;
		fv_rot_step->vx = PI*0.001f*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot_step->vy = PI*0.001f*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot_step->vz = 0.0f;
#else
		fv_rot->vx = TPI*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot->vy = TPI*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot->vz = 0.0f;
		fv_rot_step->vx = PI*0.001f*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot_step->vy = PI*0.001f*GM_Rnd( &OK_EXPLO_SEED );
		fv_rot_step->vz = 0.0f;
#endif

		angle0     =  PI*0.5f*GM_Rnd( &OK_EXPLO_SEED );
		angle1     = TPI*GM_Rnd( &OK_EXPLO_SEED );
		radius0    = work->size * (0.1f + 0.1f*GM_Rnd( &OK_EXPLO_SEED ));
		radius1    = radius0 * cosf( angle0 );
		fvtemp0.vx = radius1 * sinf( angle1 );
		fvtemp0.vy = radius0 * sinf( angle0 );
		fvtemp0.vz = radius1 * cosf( angle1 );

		angle0 = GM_Rnd( &OK_EXPLO_SEED );
		_sceVu0AddVector( pos, center, &fvtemp0 ) ;
		_sceVu0AddVector( vec, master_vec, &fvtemp0 ) ;
		_sceVu0ScaleVector( vec, vec, 0.5f + 0.25f*angle0 );
		(*scale) = work->size*(SCALE_MIN + SCALE_RND*angle0 );

		scale++;
		pos++;
		vec++;
		fv_rot++;
		fv_rot_step++;
	}





//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 6324186 /*"bombpowder7_msk"*/ );
//	tex = DG_GetTexture( 8038630 /*"wave12_alp_ovl"*/ );
//	tex = DG_GetTexture( 8819182 /*"alpha03_alp_ovl"*/ );
//	tex = DG_GetTexture( 8688110 /*"alpha01_alp_ovl"*/ );

#if 0
	if( GV_PadData[ 1 ].status & PAD_U ){
		tex = DG_GetTexture( 4856333 /*"bombsphere01_alp"*/ );
	}else if( GV_PadData[ 1 ].status & PAD_L ){
		tex = DG_GetTexture( 5904909 /*"bombsphere02_alp"*/ );
	}else if( GV_PadData[ 1 ].status & PAD_D ){
		tex = DG_GetTexture( 6953485 /*"bombsphere03_alp"*/ );
	}else{
		tex = DG_GetTexture( 8002061 /*"bombsphere04_alp"*/ );
	}
#else
	switch( (GM_IRnd( &OK_EXPLO_SEED )>>8)%3 ){
	  case 0:
		tex = DG_GetTexture( 4856333 /*"bombsphere01_alp"*/ );
		break;
	  case 1:
		tex = DG_GetTexture( 6953485 /*"bombsphere03_alp"*/ );
		break;
	  case 2:
		tex = DG_GetTexture( 8002061 /*"bombsphere04_alp"*/ );
		break;
	}
#endif


	prim_master = work->prim_master = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_NOBUFFER, EXPLO_N_PRIMS, EXPLO_N_VERTS );
	if(prim_master==NULL){
		printf("mo:null prim\n");
		return -1;
	}

	/* パケットデータの初期化 */
//	DG_SetPrim2Buffer( prim_master, OK_EXPLO_WorkTemp0, OK_EXPLO_WorkTemp0, work->uvrgb[0], work->uvrgb[1] );
	{
		DG_PRIM2_PACKET		*packet ;
		prim_master->pos[0] = OK_EXPLO_WorkTemp0;
		prim_master->pos[1] = OK_EXPLO_WorkTemp0;
		prim_master->uvrgb[0] = work->uvrgb[0];
		prim_master->uvrgb[1] = work->uvrgb[1];
		for ( i=0; i<2; i++ ){
			packet = prim_master->packet[ i ] ;
			pos    = prim_master->pos[ i ] ;
			uvrgb  = prim_master->uvrgb[ i ] ;
			for ( j=0; j<EXPLO_N_PRIMS; j++ ){
				packet->prim       = prim_master;
				packet->type       = prim_master->type ;
				packet->flag       = prim_master->flag & 0xffff ;
				packet->n_verts    = EXPLO_N_VERTS ;
				packet->pos_addr   = pos ;
				packet->uvrgb_addr = uvrgb ;
				packet++ ;
				pos += EXPLO_N_VERTS ;
				uvrgb = (void*)( (int)uvrgb + sizeof(DG_PRIM2_UVRGB) * EXPLO_N_VERTS ) ;
			}
		}
	}

	InitPacket2( work, prim_master, tex );
	DG_ConfigPrim2Tex( prim_master, tex );
	prim_master->raise = RAISE;
	DG_SetPrim2Alpha( prim_master, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim_master->root = &work->world[0];
	DG_InvisiblePrim2( prim_master );

	for( i=0; i<MUL_NUM-1; i++ ){
		prim = work->prim_m[i] = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_NOBUFFER, EXPLO_N_PRIMS, EXPLO_N_VERTS );
		if(prim==NULL){
			printf("mo:null prim\n");
			return -1;
		}
		DG_SetPrim2Buffer( prim, OK_EXPLO_WorkTemp0, OK_EXPLO_WorkTemp0, prim_master->uvrgb[0], prim_master->uvrgb[1] );
//		InitPacket2( work, prim, tex );

		DG_ConfigPrim2Tex( prim, tex );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		prim->raise = RAISE;
		prim->root = &work->world[i+1];
		DG_InvisiblePrim2( prim );
	}



	return 0 ;
}

void *NewExplosionParts( FVECTOR *center, FVECTOR *vec, float size, int col, float life_param, float decay_param, int rebirth_num )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), 1 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->size = size;
		work->col  = col;
		work->decay_param = decay_param;
		work->rebirth_num = rebirth_num;

		if ( GetResources( work, center, vec, life_param ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

