//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	water_drops.c
	水滴：detail

	2001/02/14 S.Okajima
	$Id: water_drops.c,v 1.1.1.3 2002/11/19 11:47:27 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
#define	SIZE_MIN			(10)
#define	SIZE_RND			(30)

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(32)
#define N_PRIMS		(0x2000 / 16 / N_VERTS)
#define N_LOOPS		(8)
#define	TOTAL_VERTS		(N_LOOPS*N_PRIMS*N_VERTS)

#define N_MUL		(2)
#define N_VERTS_MUL	(N_VERTS*N_MUL)
#define N_PRIMS_MUL	(N_PRIMS/N_MUL)

#define	COL_R	(255)
#define	COL_G	(255)
#define	COL_B	(255)
#define	COL_A	(255.0f)

#define	WIDTH_PARAM	(1.2f)

#define	RANDAM_VECTOR_RATIO	(0.5f)

#define	ALPHA_SUB	(1.0f / 32.0f)

typedef	struct	{
	GV_ACT_EX		actor ;

	int		now_num;

	FVECTOR		vec[TOTAL_VERTS+1];
	DG_PRIM2	*prim ;
} Work ;

static void *OK_WaterDropsWork=NULL;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR	*sc_pos;
	FVECTOR	*vec;
	FVECTOR	*sc_vec;
	FVECTOR	*sc_temp;
	int	clock;
	int	i,j,k;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGBWH	*uvrgb;
	int		num;
	int		alpha;
	SVECTOR svtemp;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	uvrgb      = prim->uvrgb[clock];
	pos        = prim->pos[  clock];
	pos_before = prim->pos[1-clock];
	vec = work->vec;
	for ( j=0 ; j < N_LOOPS ; j++ ){
		OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		OK_Mem_Scr( SCR_TMP, vec,        sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		sc_pos = SCR_POS;
		sc_vec = SCR_TMP;
		for ( i = 0 ; i < N_PRIMS_MUL ; i++ ){
			alpha = (int)(COL_A*sc_vec->vw);
			sc_vec->vw = (sc_vec->vw < ALPHA_SUB)? 0.0f: sc_vec->vw - ALPHA_SUB;
			if( sc_vec->vw > 0.0f ){
				num = (irnd()>>8)%N_VERTS_MUL;
				sc_temp = &sc_vec[num];
				svtemp.vx = ((irnd()>>8)&2047) - 1024;
				svtemp.vy = ((irnd()>>8)&4095);
				svtemp.vz = 0;
				sc_temp->vz = GV_VecLen3F( sc_temp )*0.25f;
				sc_temp->vx = 0.0f;
				sc_temp->vy = 0.0f;
				DG_SetPos2( &DG_ZeroVector, &svtemp );
				DG_RotVector( sc_temp, sc_temp, 1 );
				for ( k = 0 ; k < N_VERTS_MUL ; k++ ){
					sc_pos->vx += sc_vec->vx;
					sc_pos->vy += sc_vec->vy;
					sc_pos->vz += sc_vec->vz;

//					sc_vec->vy += P_GRAVITY*0.5f +  P_GRAVITY*0.5f * (float)(uvrgb->w) / (float)(SIZE_MIN+SIZE_RND);
					sc_vec->vy += P_GRAVITY;

					uvrgb->a = alpha;

					sc_vec++;
					sc_pos++;
					uvrgb++;
				}
			}else{
				sc_vec+= N_VERTS_MUL;
				sc_pos+= N_VERTS_MUL;
				for ( k = 0 ; k < N_VERTS_MUL ; k++ ){
					uvrgb->a = 0;
					uvrgb++;
				}
			}
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		OK_Scr_Mem( vec, SCR_TMP, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		pos        += N_VERTS*N_PRIMS;
		pos_before += N_VERTS*N_PRIMS;
		vec        += N_VERTS*N_PRIMS;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_WaterDropsWork = NULL;
}


static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i,j;

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );

	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS ; i++ ){
		for ( j = 0 ; j < N_VERTS ; j++ ){
			uvrgbwh1->h  = uvrgbwh0->h  = uvrgbwh1->w  = uvrgbwh0->w  = SIZE_MIN + (irnd()>>8)%SIZE_RND;
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
			uvrgbwh1->r  = uvrgbwh0->r  = COL_R ;
			uvrgbwh1->g  = uvrgbwh0->g  = COL_G ;
			uvrgbwh1->b  = uvrgbwh0->b  = COL_B ;
			uvrgbwh1->a  = uvrgbwh0->a  = 0 ;
			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);


	work->now_num = 0;



	return 0 ;
}


/* ---------------------------------------------------------------- */
static void RadiusShiftNode( FVECTOR *vec, int num )	//外周に寄せる
{
	float	max_length;
	float	ratio;

	max_length = vec[num].vz;
	while( --num > 0){
		ratio = 1.0f - vec->vz / max_length;
//printf("%d:%f %f %f\n",num,ratio,vec->vz,max_length);
		ratio*= ratio;
//		vec->vz*= (1.0f - ratio) * ( 1.0f + RANDAM_VECTOR_RATIO*frnd() );
		vec->vz*= (1.0f - ratio);
		vec++;
	}
}

/* ---------------------------------------------------------------- */
static void CalcInitNode( FVECTOR *vec, int node_num0, int node_num1 )
{
	FVECTOR	*fv0;
	FVECTOR	*fv1;
	FVECTOR	*fv2;
	int	node_num;
	float	th;

	node_num = node_num1 - node_num0;
	if( node_num <= 1 ) return;
	node_num = node_num0 + node_num/2;

	fv0  = fv1 = fv2 = vec;
	fv0 += node_num0;
	fv1 += node_num1;
	fv2 += node_num;

	fv0->vw = fv1->vy - fv0->vy;
	fv1->vw = fv1->vz - fv0->vz;
	fv2->vw = fv1->vw * WIDTH_PARAM * frnd();		/* len */

	th = asinf( fv0->vw / fv1->vw );
	th = (float)(((int)(th*4096.0f/TPI))&4095)*TPI/4096.0f;
	fv2->vx = fv0->vx + (fv1->vx - fv0->vx)*0.5f + fv2->vw * frnd();
	fv2->vy = fv0->vy + fv0->vw*0.5f + fv2->vw * cosf(th);
	fv2->vz = fv0->vz + fv1->vw*0.5f - fv2->vw * sinf(th);

	CalcInitNode( vec, node_num0, node_num  );
	CalcInitNode( vec, node_num,  node_num1 );
}


static void PutWaterDrops( Work *work, FVECTOR *data_pos, FVECTOR *data_vec )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	FVECTOR			*vec;
	FVECTOR			*sc_fv;
	SVECTOR			svtemp;
	int		clock;
	int		num;
	int		i;

	prim = work->prim;
	clock = prim->buffer_clock;

	num = work->now_num * N_VERTS_MUL;
	vec        = work->vec;
	pos        = prim->pos[clock];
	vec       += num;
	pos       += num;

	sc_fv = SCR_POS;
	sc_fv[ N_VERTS_MUL ].vz = GV_VecLen3F( data_vec );
//	sc_fv[ N_VERTS_MUL ].vz = (float)(SIZE_MIN+SIZE_RND) * ( 0.1f + rnd()*0.25f );
	CalcInitNode( sc_fv, 0, N_VERTS_MUL );	// 内部で一個多く処理してるので注意
	RadiusShiftNode( sc_fv, N_VERTS_MUL );
	OK_DirVecXY( &DG_ZeroVector, data_vec, &svtemp );
	DG_SetPos2( data_vec, &svtemp );
	DG_PutVector( sc_fv, vec, N_VERTS_MUL );
	vec->vw = 1.0f;	// 先頭だけデータセット

	for ( i = 0 ; i < N_VERTS_MUL ; i++ ){
		DG_COPY_VEC( pos, data_pos );
		pos++;
	}

	work->now_num++;
	if( work->now_num >= N_LOOPS*N_PRIMS_MUL ) work->now_num = 0;
}

void *NewWaterDrops( FVECTOR *pos, FVECTOR *vec )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_WaterDropsWork==NULL ){
		work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		OK_WaterDropsWork = work;
		return (void *)work ;
	}else{
		work = OK_WaterDropsWork;
	}

	PutWaterDrops( work, pos, vec );

	return (void *)(1) ;
}


