//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	inter_line_blade.c
	毎フレーム更新される座標を補完しつつ繋ぐ
		→	dot で 修飾
	2001/03/26 S.Okajima
	$Id: inter_line_blade.c,v 1.1.1.3 2002/11/19 11:47:21 Yoshizawa1 Exp $
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
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"
#include	"utl_dma.h"

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x2000)
#define N_VERTS	(64)
#define N_POLYS	(N_VERTS/4)
#define	N_PRIMS	(16)

#define N_INTER	(N_POLYS)
#define N_INTER_DIV	(1.0f/(float)N_INTER)

#define	P_ALPHA_MAX		(255)
#define	SIZE			(16)

#define N_VERTS_DOT	(32)
#define	N_PRIMS_DOT	(16)
#define	N_PARTS_DOT	(N_PRIMS_DOT*N_VERTS_DOT)
#define	N_LOOPS_DOT	(8)

#define	N_INITS_DOT	(4)
#define	N_CYCLE_DOT	(N_LOOPS_DOT*N_PRIMS_DOT*N_VERTS_DOT / N_INITS_DOT)


#define	ANGLE_STEP	(TPI*0.00261f)
#define	ANGLE_STEP2	(TPI*0.00211f)
#define	ANGLE_STEP3	(TPI*0.0151f)
#define	RADIUS		(10.0f)



#define	FORCE_PARAM			(5000.0f)
#define	FORCE_LENGTH_MIN	(100.0f*100.0f)
#define	DECAY_VEC	(0.98f)

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

	int			prim_cycle;
	int			disp_f;
	SVECTOR		col;

	float		angle;
	float		angle2;
	float		angle3;

	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_dot ;

	FVECTOR		vec[N_LOOPS_DOT*N_PARTS_DOT];

} Work ;



#ifdef KP_XBOX	//// VC7 最適化バグ
#pragma optimize("", off)
#endif	

/* ---------------------------------------------------------------- */
static void CulcDot( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2 *prim;
	FVECTOR	*sc_pos_before;
	FVECTOR	*sc_pos;
	FVECTOR	*sc_vec;
	FVECTOR	*vec;
	FVECTOR	*pos;
	FVECTOR	diff;
	FVECTOR	fvtemp;
	FVECTOR	center;
	SVECTOR	svtemp;
	int		alpha;
	int		size;
	int		clock;
	int		i,j,n;
	float	len;
	float	ftemp;
	float	mul_scale;

	mul_scale = (float)work->disp_f;

	work->angle += ANGLE_STEP ;
	work->angle2+= ANGLE_STEP2;
	work->angle3+= ANGLE_STEP3;


	prim = work->prim_dot;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	sc_vec        = work->vec;
	sc_pos        = prim->pos[  clock];
	sc_pos_before = prim->pos[1-clock];
//	uvrgbwh       = prim->uvrgb[  clock ];

	_sceVu0SubVector( &diff, &work->before_pos1, &work->before_pos0 ) ;
	_sceVu0ScaleVector( &center, &diff, sinf(work->angle)*0.5f+0.5f );
	_sceVu0AddVector( &center, &work->before_pos0, &center ) ;

	OK_DirVecXY( &DG_ZeroVector, &diff, &svtemp );
	ftemp = RADIUS*sinf(work->angle3);
	fvtemp.vx = ftemp*sinf(work->angle2);
	fvtemp.vy = ftemp*cosf(work->angle2);
	fvtemp.vz = 0.0f;
	DG_SetPos2( &center, &svtemp );
	DG_PutVector( &fvtemp, &center, 1 );

	for( i=0; i<N_LOOPS_DOT; i++ ){
		OK_Mem_Scr( SCR_POS, sc_pos_before, sizeof(FVECTOR), N_PARTS_DOT) ;
		OK_Mem_Scr( SCR_VEC, sc_vec,        sizeof(FVECTOR), N_PARTS_DOT) ;
		pos = SCR_POS;
		vec = SCR_VEC;
		for( j=0; j<N_PARTS_DOT; j++ ){
			_sceVu0AddVector( pos, pos, vec ) ;
			_sceVu0SubVector( &fvtemp, &center, pos ) ;
//			fvtemp.vw = 0.0f;
			len = _sceVu0InnerProduct( &fvtemp, &fvtemp ) ;
			if( len < FORCE_LENGTH_MIN ){
//				DG_COPY_VEC( pos, &center );
				DG_COPY_VEC( vec, &DG_ZeroVector );
//				uvrgbwh->w = uvrgbwh->h = 0;

				_sceVu0ScaleVector( &fvtemp, &diff, rnd() );
				_sceVu0AddVector( pos, &work->before_pos0, &fvtemp ) ;
				pos->vx+= work->before_vec0.vx*mul_scale*frnd();
				pos->vy+= work->before_vec0.vy*mul_scale*frnd();
				pos->vz+= work->before_vec0.vz*mul_scale*frnd();
			}else{
				_sceVu0ScaleVector( vec, vec, DECAY_VEC );
				vec->vx += FORCE_PARAM * (fvtemp.vx) / len ;
				vec->vy += FORCE_PARAM * (fvtemp.vy) / len ;
				vec->vz += FORCE_PARAM * (fvtemp.vz) / len ;
//				uvrgbwh->w = uvrgbwh->h = (irnd()>>8)%SIZE + SIZE_MIN;
			}

			pos++;
			vec++;
//			uvrgbwh++;
		}
		OK_Scr_Mem( sc_pos, SCR_POS, sizeof(FVECTOR), N_PARTS_DOT) ;
		OK_Scr_Mem( sc_vec, SCR_VEC, sizeof(FVECTOR), N_PARTS_DOT) ;
		sc_pos_before += N_PARTS_DOT;
		sc_pos        += N_PARTS_DOT;
		sc_vec        += N_PARTS_DOT;
	}



//-------------
	n = N_CYCLE_DOT - work->prim_cycle + 1;
	uvrgbwh = prim->uvrgb[ clock ];
	for( i=0; i<N_CYCLE_DOT; i++ ){
		alpha = P_ALPHA_MAX * n / (N_CYCLE_DOT+1);
		size  = SIZE - (SIZE        * n / (N_CYCLE_DOT+1));
		n++;
		n = (n>N_CYCLE_DOT)? 0: n;
		for( j=0; j<N_INITS_DOT; j++ ){
			uvrgbwh->a = alpha;
			uvrgbwh->w = uvrgbwh->h = size;
			uvrgbwh++;
		}
	}



//-------------
	pos      = prim->pos[clock];
	vec      = work->vec;

	n = work->prim_cycle * N_INITS_DOT;
	pos+= n;
	vec+= n;

	work->prim_cycle++;
	if( work->prim_cycle >= N_CYCLE_DOT) work->prim_cycle = 0;

//	_sceVu0SubVector( &diff, &work->before_pos1, &work->before_pos0 ) ;
	for( i=0; i<N_INITS_DOT; i++ ){
		_sceVu0ScaleVector( &fvtemp, &diff, rnd() );
		_sceVu0AddVector( pos, &work->before_pos0, &fvtemp ) ;
		pos->vx+= work->before_vec0.vx*mul_scale*frnd();
		pos->vy+= work->before_vec0.vy*mul_scale*frnd();
		pos->vz+= work->before_vec0.vz*mul_scale*frnd();
#if 0
		vec->vx = 0.0f;
		vec->vy = 0.0f;
		vec->vz = 0.0f;
#else
//		_sceVu0ScaleVector( vec, &work->before_vec0, 0.5f );
//		_sceVu0ScaleVector( vec, &work->before_vec0, 1.0f );

		_sceVu0SubVector( vec, pos, &work->before_pos0 ) ;
		_sceVu0ScaleVector( vec, vec, 0.2f*rnd() );
#endif
		pos++;
		vec++;
	}






}

#ifdef KP_XBOX	//// VC7 最適化バグ
#pragma optimize("", on)
#endif


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2 *prim;
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

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;


//AN_Test_Eye2( work->pos0, 2 );

	sc_before_pos = prim->pos[ 1 - clock ];
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

	OK_Scr_Mem( prim->pos[ clock ],  SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;


	//====================================================================

	CulcDot( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim     = OK_FreePrim2( work->prim );
	work->prim_dot = OK_FreePrim2( work->prim_dot );
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


	DG_COPY_VEC( &work->before_pos0, work->pos0 );
	DG_COPY_VEC( &work->before_pos1, work->pos1 );
	DG_COPY_VEC( &work->before_vec0, &DG_ZeroVector );
	DG_COPY_VEC( &work->before_vec1, &DG_ZeroVector );


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	pos_0  = prim->pos[ 0 ] ;
	pos_1  = prim->pos[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos_0, work->pos0 );
			DG_COPY_VEC( pos_1, work->pos0 );
			pos_0++ ;
			pos_1++ ;
		}
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

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS_DOT*N_LOOPS_DOT*N_VERTS_DOT ; j++ ){
		for ( k = 0 ; k < N_VERTS_DOT ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			uvrgbwh1->w = uvrgbwh0->w = SIZE;
			uvrgbwh1->h = uvrgbwh0->h = SIZE;

			uvrgbwh1->r = uvrgbwh0->r = work->col.vx ;
			uvrgbwh1->g = uvrgbwh0->g = work->col.vy ;
			uvrgbwh1->b = uvrgbwh0->b = work->col.vz ;
			uvrgbwh1->a = uvrgbwh0->a = P_ALPHA_MAX ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
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

	work->prim_cycle = 0;

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL) return -1;
	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim );

	DG_COPY_VEC( &work->before_pos0, work->pos0 );
	DG_COPY_VEC( &work->before_pos1, work->pos1 );
	DG_COPY_VEC( &work->before_vec0, &DG_ZeroVector );
	DG_COPY_VEC( &work->before_vec1, &DG_ZeroVector );


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




	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim_dot = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_DOT*N_LOOPS_DOT, N_VERTS_DOT );
	if(prim==NULL) return -1;
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( prim );

	return (0);
}

/* ---------------------------------------------------------------- */
/*
*pos0  :[ポインタ参照する]残像生成端点１
*pos1  :[ポインタ参照する]残像生成端点２
disp_f :保存フレーム数（新規点を含めた数、最大１６）
col    :ＲＧＢα（pad に αを！！）
*/
void *NewInterLineBlade( FVECTOR *pos0, FVECTOR *pos1, int disp_f, SVECTOR *col )
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

