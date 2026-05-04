//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_inter_poly.c
	毎フレーム更新される座標を補完しつつ繋ぐ
	2000/04/18 S.Okajima
	$Id: d_inter_poly.c,v 1.1.1.3 2002/11/19 11:46:53 Yoshizawa1 Exp $
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

#define	SCR_POS		(SCRPAD_ADDR)
#define N_VERTS	(64)
#define N_POLYS	(N_VERTS/4)
#define	N_PRIMS	(16)

#define N_INTER	(N_POLYS)
#define N_INTER_DIV	(1.0f/(float)( N_INTER-1) )

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		*pos0;
	FVECTOR		*pos1;

	FVECTOR		before_pos0;
	FVECTOR		before_pos1;
	FVECTOR		before_vec0;
	FVECTOR		before_vec1;
	float		inter_ratio[N_INTER];

	int			disp_f;
	SVECTOR		col;

	DG_PRIM2	*prim ;

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	*sc_before_pos;
	FVECTOR	*sc_calc_pos;
	FVECTOR	before_vec;
	FVECTOR	diff;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	int		clock;
	int		i;
	float	len_base;
	float	len_local;
	float	ftemp0;
	float	ftemp1;
	float	*inter_ratio;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;


//AN_Test_Eye2( work->pos0, 2 );

	sc_before_pos = work->prim->pos[ 1 - clock ];
	sc_before_pos+=N_VERTS;
	OK_Mem_Scr( SCR_POS, sc_before_pos,  sizeof(FVECTOR), (N_PRIMS-1)*N_VERTS ) ;
	//====================================================================
	sc_before_pos = SCR_POS;
	sc_before_pos+=(N_PRIMS-1)*N_VERTS;
	sc_calc_pos   = sc_before_pos;
	sc_calc_pos  +=2;
	DG_COPY_VEC( sc_before_pos, &work->before_pos0 );
	DG_COPY_VEC( &fvtemp1,      &work->before_pos0 );
	DG_COPY_VEC( &before_vec,   &work->before_vec0 );
	fvtemp0.vx = (work->pos0->vx - sc_before_pos->vx) * N_INTER_DIV;
	fvtemp0.vy = (work->pos0->vy - sc_before_pos->vy) * N_INTER_DIV;
	fvtemp0.vz = (work->pos0->vz - sc_before_pos->vz) * N_INTER_DIV;
	len_base = GV_VecLen3F( &fvtemp0 ); /* 目標点へのベクトル（分割） */
	inter_ratio = work->inter_ratio;
	for( i=0; i<N_INTER; i++ ){
		/* 目標点へのベクトル正規化 */
		diff.vx = work->pos0->vx - fvtemp1.vx;
		diff.vy = work->pos0->vy - fvtemp1.vy;
		diff.vz = work->pos0->vz - fvtemp1.vz;
		len_local = GV_VecLen3F( &diff );
		if( len_local == 0.0f ){
			len_local = N_INTER_DIV;
		}else{
			len_local = len_base / len_local;
		}
		diff.vx *= len_local;
		diff.vy *= len_local;
		diff.vz *= len_local;

		/* ローカルベクトル正規化 */
		len_local = GV_VecLen3F( &before_vec );
		if( len_local == 0.0f ){
			DG_COPY_VEC( &before_vec, &fvtemp0 );
		}else{
			len_local = len_base / len_local;
			before_vec.vx *= len_local;
			before_vec.vy *= len_local;
			before_vec.vz *= len_local;
		}

		/* 内挿 */
		ftemp0 = (*inter_ratio);
		ftemp1 = 1.0f - ftemp0;
		before_vec.vx = before_vec.vx * ftemp0 + diff.vx * ftemp1;
		before_vec.vy = before_vec.vy * ftemp0 + diff.vy * ftemp1;
		before_vec.vz = before_vec.vz * ftemp0 + diff.vz * ftemp1;

		DG_COPY_VEC( sc_calc_pos, &fvtemp1 );
		sc_before_pos->vx = sc_calc_pos->vx + before_vec.vx;
		sc_before_pos->vy = sc_calc_pos->vy + before_vec.vy;
		sc_before_pos->vz = sc_calc_pos->vz + before_vec.vz;

		DG_COPY_VEC( &fvtemp1, sc_before_pos );
//if(i%2==0)AN_Test_Eye3( &fvtemp1 );
//AN_Test_Eye3( &fvtemp1 );
		sc_calc_pos   +=4;
		sc_before_pos +=4;

		inter_ratio++;
	}
	DG_COPY_VEC( &work->before_pos0, &fvtemp1 );
	DG_COPY_VEC( &work->before_vec0, &before_vec );

//printf("0:%f %f %f\n",work->before_pos0.vx,work->before_pos0.vy,work->before_pos0.vz);


//printf("______________\n");

	//====================================================================
	sc_before_pos = SCR_POS;
	sc_before_pos+=(N_PRIMS-1)*N_VERTS;
	sc_calc_pos   = sc_before_pos;
	sc_before_pos ++;
	sc_calc_pos   +=3;
	DG_COPY_VEC( sc_before_pos, &work->before_pos1 );
	DG_COPY_VEC( &fvtemp1,      &work->before_pos1 );
	DG_COPY_VEC( &before_vec,   &work->before_vec1 );
	fvtemp0.vx = (work->pos1->vx - sc_before_pos->vx) * N_INTER_DIV;
	fvtemp0.vy = (work->pos1->vy - sc_before_pos->vy) * N_INTER_DIV;
	fvtemp0.vz = (work->pos1->vz - sc_before_pos->vz) * N_INTER_DIV;
	len_base = GV_VecLen3F( &fvtemp0 ); /* 目標点へのベクトル（分割） */
	inter_ratio = work->inter_ratio;
	for( i=0; i<N_INTER; i++ ){
		/* 目標点へのベクトル正規化 */
		diff.vx = work->pos1->vx - fvtemp1.vx;
		diff.vy = work->pos1->vy - fvtemp1.vy;
		diff.vz = work->pos1->vz - fvtemp1.vz;
		len_local = GV_VecLen3F( &diff );
		if( len_local == 0.0f ){
			len_local = N_INTER_DIV;
		}else{
			len_local = len_base / len_local;
		}
		diff.vx *= len_local;
		diff.vy *= len_local;
		diff.vz *= len_local;

		/* ローカルベクトル正規化 */
		len_local = GV_VecLen3F( &before_vec );
		if( len_local == 0.0f ){
			DG_COPY_VEC( &before_vec, &fvtemp0 );
		}else{
			len_local = len_base / len_local;
			before_vec.vx *= len_local;
			before_vec.vy *= len_local;
			before_vec.vz *= len_local;
		}

		/* 内挿 */
		ftemp0 = (*inter_ratio);
		ftemp1 = 1.0f - ftemp0;
		before_vec.vx = before_vec.vx * ftemp0 + diff.vx * ftemp1;
		before_vec.vy = before_vec.vy * ftemp0 + diff.vy * ftemp1;
		before_vec.vz = before_vec.vz * ftemp0 + diff.vz * ftemp1;

		DG_COPY_VEC( sc_calc_pos, &fvtemp1 );
		sc_before_pos->vx = sc_calc_pos->vx + before_vec.vx;
		sc_before_pos->vy = sc_calc_pos->vy + before_vec.vy;
		sc_before_pos->vz = sc_calc_pos->vz + before_vec.vz;

		DG_COPY_VEC( &fvtemp1, sc_before_pos );
//if(i%2==0)AN_Test_Eye3( &fvtemp1 );
//AN_Test_Eye3( &fvtemp1 );
		sc_calc_pos   +=4;
		sc_before_pos +=4;

		inter_ratio++;
	}
	DG_COPY_VEC( &work->before_pos1, &fvtemp1 );
	DG_COPY_VEC( &work->before_vec1, &before_vec );

	//====================================================================
	OK_Scr_Mem( work->prim->pos[ clock ],  SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos_0 ;
	FVECTOR				*pos_1 ;
	DG_PRIM2_UVRGB		*uvrgb_0 ;
	DG_PRIM2_UVRGB		*uvrgb_1 ;
	int		j, k ;
	int		count ;
	int		limit ;
	int		alpha;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	pos_0  = prim->pos[ 0 ] ;
	pos_1  = prim->pos[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS*N_VERTS ; j++ ){
		DG_COPY_VEC( pos_0, work->pos0 );
		DG_COPY_VEC( pos_1, work->pos0 );
		pos_0++ ;
		pos_1++ ;
	}

	//-------------------------------
	limit = work->disp_f * N_POLYS;
	count = 0;
	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			alpha = work->col.pad * count / limit;
			uvrgb_1->u = uvrgb_0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x8fff ;
			uvrgb_1->r = uvrgb_0->r = work->col.vx ;
			uvrgb_1->g = uvrgb_0->g = work->col.vy ;
			uvrgb_1->b = uvrgb_0->b = work->col.vz ;
			uvrgb_1->a = uvrgb_0->a = alpha ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x8fff ;
			uvrgb_1->r = uvrgb_0->r = work->col.vx ;
			uvrgb_1->g = uvrgb_0->g = work->col.vy ;
			uvrgb_1->b = uvrgb_0->b = work->col.vz ;
			uvrgb_1->a = uvrgb_0->a = alpha ;
			uvrgb_0++;
			uvrgb_1++;

			if( j > N_PRIMS - work->disp_f ){
				count++;
				if( count > limit ) count = limit;
			}
			alpha = work->col.pad * count / limit;
			uvrgb_1->u = uvrgb_0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x0fff ;
			uvrgb_1->r = uvrgb_0->r = work->col.vx ;
			uvrgb_1->g = uvrgb_0->g = work->col.vy ;
			uvrgb_1->b = uvrgb_0->b = work->col.vz ;
			uvrgb_1->a = uvrgb_0->a = alpha ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x0fff ;
			uvrgb_1->r = uvrgb_0->r = work->col.vx ;
			uvrgb_1->g = uvrgb_0->g = work->col.vy ;
			uvrgb_1->b = uvrgb_0->b = work->col.vz ;
			uvrgb_1->a = uvrgb_0->a = alpha ;
			uvrgb_0++;
			uvrgb_1++;

		}
	}
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i ;
	float	ftemp0;
	float	ftemp1;

	DG_COPY_VEC( &work->before_pos0, work->pos0 );
	DG_COPY_VEC( &work->before_pos1, work->pos1 );
	DG_COPY_VEC( &work->before_vec0, &DG_ZeroVector );
	DG_COPY_VEC( &work->before_vec1, &DG_ZeroVector );

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL) return -1;
	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim );



	for( i=0; i<N_INTER; i++ ){
		ftemp0 = (float)(i+1) * N_INTER_DIV;
		ftemp1 = 1.0f - ftemp0;
		/* 初期は線形に、後半は円弧に */
		work->inter_ratio[i] = (ftemp1*ftemp0 + cosf( PI * 0.5f * ftemp0 )*ftemp1);
//		work->inter_ratio[i] = cosf( PI * 0.5f * ftemp0 );
//		ftemp0 = cosf( PI * 0.5f * ftemp0 );
//		work->inter_ratio[i] = ftemp0*ftemp0*ftemp0*ftemp0;
//printf("%f\n",work->inter_ratio[i]);
	}

	return (0);
}

/* ---------------------------------------------------------------- */
/*
*pos0  :[ポインタ参照する]残像生成端点１
*pos1  :[ポインタ参照する]残像生成端点２
disp_f :保存フレーム数（新規点を含めた数、最大１６）
col    :ＲＧＢα（pad に αを！！）
*/
void *NewInterPoly_Demo( FVECTOR *pos0, FVECTOR *pos1, int disp_f, SVECTOR *col )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pos0   = pos0;
		work->pos1   = pos1;
		work->disp_f = disp_f;
		if( work->disp_f < 1 ) work->disp_f = 1;
		if( work->disp_f > N_PRIMS ) work->disp_f = N_PRIMS;
		work->col.vx  = col->vx;
		work->col.vy  = col->vy;
		work->col.vz  = col->vz;
		work->col.pad = col->pad;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

