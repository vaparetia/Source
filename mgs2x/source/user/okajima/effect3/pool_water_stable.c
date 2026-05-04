//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pool_water_stable.c
	泡立つプール（汚水処理場）

	2001/04/27 S.Okajima
	$Id: pool_water_stable.c,v 1.1.1.3 2002/11/19 11:47:34 Yoshizawa1 Exp $
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
extern int	OK_PutSplush2( FVECTOR *center, SVECTOR *rot, float length );

/*----------------------------------------------------------------*/

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	COL_A_MIN	(16)
#define	COL_A_STEP	(16)


#define	RAISE			(-10000)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x2000 / 16 / N_VERTS)
//#define N_PRIMS		(0x1000 / 16 / N_VERTS)
#define N_LOOPS		(1)
#define	TOTAL_VERTS		(N_LOOPS*N_PRIMS*N_VERTS)

#define SHIFT_SURFACE	(750.0f)

#define SIZE_MIN	(800.0f)
#define SIZE_RND	(100.0f)

#define	VEC_MIN		(16.0f)
#define	VEC_RND		(16.0f)

#define	STRATA_HEIGHT		(200.0f)
#define	STRATA_HEIGHT_STEP	(STRATA_HEIGHT / (N_PRIMS*N_VERTS))

typedef	struct	{
	GV_ACT_EX		actor ;
	int		map;

	FVECTOR	bound0;
	FVECTOR	bound1;
	FVECTOR	diff;

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
	int	clock;
	int	i,j;
	DG_PRIM2			*prim ;
	float	step;

	if( (work->map & GM_CurrentStageMap) == 0 ) return;

	prim = work->prim;

	sc_pos = SCR_POS;
	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		sc_pos->vx = 0.0f;
		sc_pos->vy = 0.0f;
		sc_pos->vz = 1.0f;
		DG_SetPos( &DG_Chanls->eye );
		DG_RotVector( sc_pos, sc_pos, 1 );
//if(GV_Time%120==0)printf("%f\n",sc_pos->vy);
		if( sc_pos->vy > 0.3f ){
			DG_InvisiblePrim2( prim );
			return;
		}
	}
	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );

//	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	pos        = prim->pos[  clock];
	pos_before = prim->pos[1-clock];
	vec = work->vec;
	for ( j=0 ; j < N_LOOPS ; j++ ){
		OK_Mem_Scr( SCR_POS, pos_before, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		sc_pos = SCR_POS;
		step = GM_WaterLevel-SHIFT_SURFACE;
		for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
			sc_pos->vx += vec->vx;
			sc_pos->vy  = step;
			sc_pos->vz += vec->vz;
			step-= STRATA_HEIGHT_STEP;
			if( sc_pos->vx < work->bound0.vx ){
				sc_pos->vx+= work->diff.vx;
			}else if( sc_pos->vx > work->bound1.vx ){
				sc_pos->vx-= work->diff.vx;
			}
			if( sc_pos->vz < work->bound0.vz ){
				sc_pos->vz+= work->diff.vz;
			}else if( sc_pos->vz > work->bound1.vz ){
				sc_pos->vz-= work->diff.vz;
			}
			vec++;
			sc_pos++;
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		pos        += N_VERTS*N_PRIMS;
		pos_before += N_VERTS*N_PRIMS;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos0;
	FVECTOR				*pos1;
	FVECTOR				*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i,j;
	float	angle;
	float	size;
	float	len;

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	vec = work->vec;
	pos0       = prim->pos[0];
	pos1       = prim->pos[1];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( j=0 ; j < N_LOOPS ; j++ ){
		for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
			angle = TPI*rnd();
			len   = VEC_MIN + VEC_RND*rnd();
			vec->vx = len * sinf(angle);
			vec->vz = len * cosf(angle);

			pos1->vx = pos0->vx = work->bound0.vx + work->diff.vx*rnd();
			pos1->vz = pos0->vz = work->bound0.vz + work->diff.vz*rnd();

			angle = TPI*rnd();
			size = SIZE_MIN + SIZE_RND*rnd();
			uvrgbwh1->w  = uvrgbwh0->w  = (int)(cosf( angle ) * size);
			uvrgbwh1->h  = uvrgbwh0->h  = (int)(sinf( angle ) * size);
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
			uvrgbwh1->a  = uvrgbwh0->a  = COL_A_MIN + COL_A_STEP * (N_PRIMS*N_VERTS - i) / (N_PRIMS*N_VERTS) ;

			vec++;
			pos0++;
			pos1++;
			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}


	return 1;
}

static int GetResources( Work *work, int map )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	_sceVu0SubVector( &work->diff, &work->bound1, &work->bound0 );


//	tex = DG_GetTexture( 15638432 /*"blood_1bw_msk"*/ );
	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	GM_GroupPrim2( prim, map ) ;

	return 0 ;
}

void *NewPoolWaterStable( FVECTOR *bound0, FVECTOR *bound1, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->map = map;

		if( bound0->vx < bound1->vx ){
			work->bound0.vx = bound0->vx;
			work->bound1.vx = bound1->vx;
		}else{
			work->bound0.vx = bound1->vx;
			work->bound1.vx = bound0->vx;
		}
		if( bound0->vz < bound1->vz ){
			work->bound0.vz = bound0->vz;
			work->bound1.vz = bound1->vz;
		}else{
			work->bound0.vz = bound1->vz;
			work->bound1.vz = bound0->vz;
		}

		if ( GetResources( work, map ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
