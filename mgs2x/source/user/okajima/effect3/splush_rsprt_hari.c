//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_rsprt.c
	回転スプライトで水飛沫

	2001/07/04 S.Okajima
	$Id: splush_rsprt_hari.c,v 1.1.1.3 2002/11/19 11:47:37 Yoshizawa1 Exp $
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
extern float GM_WaterLevel;
/*----------------------------------------------------------------*/
#define	RAISE		(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x0400 / 16 / N_VERTS)
#define N_LOOPS		(1)
#define N_TOTAL		(N_LOOPS*N_PRIMS*N_VERTS)

//#define SIZE_RATIO		(0.75f)
#define SIZE_RATIO		(1.5f)
#define SIZE_STEP_RATIO	(0.01f)

#define DECAY_RATIO	(0.995f)

//#define LIFE	(300)
#define LIFE	(40)

#define VEC_MIN	(100.0f)

#define SIDE_VEC_RATIO	(0.01f)

static	int OK_Sp2Num;

typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		center;
	FVECTOR		vec[N_TOTAL];
	float		size;
	float		radius;
	float		intense;
	int			col;
	int			life;

	DG_PRIM2	*prim ;
} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGBWH	*uvrgb;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR	*sc_pos;
	FVECTOR	*vec;
	FVECTOR	*sc_vec;
//	FVECTOR	fv_keep;
	FVECTOR	base;
	FVECTOR	vec_base;
	FVECTOR	unit_upper_vec;
	SVECTOR	rot;
	int	clock;
	int	i,j;
	int		num;
	int		alpha;
	int		count;
	float	bottom;
	float	angle;
	float	size;
	float	rs;
	float	rc;

	bottom = GM_WaterLevel + work->size;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
	DG_VisiblePrim2( prim );


	alpha = (work->col&255) * (LIFE - work->life) / LIFE;

	uvrgb        = prim->uvrgb[  clock];
	pos          = prim->pos[  clock];
	pos_before   = prim->pos[1-clock];
	vec          = work->vec;

//	fv_keep.vw = -1.0f;
	num = (work->life >= N_LOOPS)? N_LOOPS: work->life;
	count = N_LOOPS*N_PRIMS*N_VERTS;
	for ( j=0; j<num; j++ ){
		OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		OK_Mem_Scr( SCR_TMP, vec,        sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;

		sc_pos = SCR_POS;
		sc_vec = SCR_TMP;
		for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
			if( sc_pos->vy < bottom ){
				count--;
				uvrgb->a = 0;
				uvrgb->w = uvrgb->h = 0;
			}else{
				sc_pos->vx += sc_vec->vx;
				sc_pos->vy += sc_vec->vy;
				sc_pos->vz += sc_vec->vz;
				sc_vec->vy += P_GRAVITY;
				uvrgb->a = alpha;
				uvrgb->w+= uvrgb->pad0;
				uvrgb->h+= uvrgb->pad1;
			}

			sc_vec++;
			sc_pos++;
			uvrgb++;
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		OK_Scr_Mem( vec, SCR_TMP, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		pos_before+= N_VERTS*N_PRIMS;
		pos       += N_VERTS*N_PRIMS;
		vec       += N_VERTS*N_PRIMS;
	}

	if( work->life < N_LOOPS){
		unit_upper_vec.vx = 0.0f;
		unit_upper_vec.vy = 1.0f;
		unit_upper_vec.vz = 0.0f;

		DG_COPY_VEC( &base,     &work->center );
		base.vy = GM_WaterLevel + work->size;
		DG_COPY_VEC( &vec_base, &work->center );

		vec_base.vx = 0.0f;
		vec_base.vz = 0.0f;
		rot.vz = 0;

		num  = N_PRIMS*N_VERTS*work->life;
		vec  = work->vec;
		pos  = prim->pos[clock];
		vec += num;
		pos += num;
		for ( i=0; i<N_PRIMS*N_VERTS; i++ ){
			angle = TPI*rnd();
			size = work->radius*rnd() * (1.0f + (float)work->life*2.0f/(float)LIFE) ;
			rs = sinf( angle );
			rc = cosf( angle );

			pos->vx = rs * size;
			pos->vy = 0.0f;
			pos->vz = rc * size;

			vec->vx = pos->vx * SIDE_VEC_RATIO;;
			vec->vy = work->intense*(0.1f + rnd()*0.9f);
			vec->vz = pos->vz * SIDE_VEC_RATIO;

			_sceVu0AddVector( pos, &base, pos );

			vec++;
			pos++;
		}
	}

//printf("count:%d\n",count);
	if( work->life++ >= LIFE || count<=0 ){
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_Sp2Num--;
}


static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	int		col_r;
	int		col_g;
	int		col_b;
	int		col_a;
	float	angle;
	float	size;
	float	rs;
	float	rc;

	col_r = ((work->col>>24)&255);
	col_g = ((work->col>>16)&255);
	col_b = ((work->col>> 8)&255);
	col_a = ((work->col>> 0)&255);

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	uvrgbwh0   = prim->uvrgb[0];
	uvrgbwh1   = prim->uvrgb[1];
	for ( i=0; i<N_LOOPS*N_PRIMS*N_VERTS; i++ ){
		angle = TPI*rnd();
		size = work->size*(0.8f + rnd()*0.2f);
		rs = sinf( angle ) * size;
		rc = cosf( angle ) * size;
		uvrgbwh1->w    = uvrgbwh0->w    = (int)(rc);
		uvrgbwh1->h    = uvrgbwh0->h    = (int)(rs);
		uvrgbwh1->pad0 = uvrgbwh0->pad0 = (int)(rc*SIZE_STEP_RATIO);
		uvrgbwh1->pad1 = uvrgbwh0->pad1 = (int)(rs*SIZE_STEP_RATIO);
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
		uvrgbwh1->r  = uvrgbwh0->r  = col_r;
		uvrgbwh1->g  = uvrgbwh0->g  = col_g;
		uvrgbwh1->b  = uvrgbwh0->b  = col_b;
		uvrgbwh1->a  = uvrgbwh0->a  = 0;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->life = 0;
	work->size = work->radius*SIZE_RATIO;

	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( prim );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);


#if 1
	{	// 固定生成波紋
		extern void *NewRippleStrip( FVECTOR *center, float radius, int life );
		NewRippleStrip( &work->center, 30000.0f, 200 );
	}
#endif

	return 0 ;
}

void *NewSplushRotateSplush2( FVECTOR *pos, float radius, float intense, int col )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_Sp2Num > 8 ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		work->radius  = radius;
		work->intense = intense;
		work->col = col;

		OK_Sp2Num++;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
