//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_kasu.c
	爆炎破片飛翔
	2000/03/11 S.Okajima
	$Id: bomb_kasu.c,v 1.1.1.3 2002/11/19 11:47:01 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define	COL_R			(255)
#define	COL_G			(255)
#define	COL_B			(255)
//#define	ALPHA			(48)
#define	ALPHA			(64)

#define N_PRIMS		(2)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	LIFE_TIME		(32)
//#define	SIZE			(1000.0f)
#define	SIZE			(1200.0f)

#define	INIT_MULTIPLE_NUM	(2)
#define	SEARCH_HZX			(700)

//#define	RAISE				(4095)
#define	RAISE				(1000)
//#define	RAISE				(0)
//#define	RAISE				(-3000)

#define	VEC_MIN				(10.0f)
#define	VEC_RAND			(200.0f)
#define	VEC_DECAY_RATIO		(0.97f)

extern float	OK_slow_param;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	DG_PRIM2	*prim ;
	FVECTOR		center;
	FVECTOR		vec[ N_PRIMS * N_VERTS ];
	float		life;
} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int	i,j;
	int	local_count;
	int		clock;
	int		alpha;
	FVECTOR	*pos;
	FVECTOR	*vec;
	FVECTOR	*pos_before;
	FVECTOR	fvtemp;


	alpha = ALPHA * (int)work->life / LIFE_TIME;
	if( alpha < 0 ) alpha=0;


	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;


	vec            = work->vec;
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	for( i=0; i<N_PRIMS; i++ ){
		local_count=irnd()%INIT_MULTIPLE_NUM+1;
		for( j=0; j<N_VERTS; j++ ){
			if( vec->vw == 0.0f  &&  local_count >= 0 ){
				vec->vw  = 1.0f;
				local_count--;
				DG_COPY_VEC( pos, &work->center );
				_sceVu0ScaleVector( &fvtemp, vec, 4.0f );
				pos->vx += fvtemp.vx;
				pos->vy += fvtemp.vy;
				pos->vz += fvtemp.vz;
			}else{
				vec->vy += P_GRAVITY*0.66f*OK_slow_param;
				vec->vx *= VEC_DECAY_RATIO;
				vec->vy *= VEC_DECAY_RATIO;
				vec->vz *= VEC_DECAY_RATIO;
				_sceVu0ScaleVector( &fvtemp, vec, OK_slow_param );
				pos->vx  = pos_before->vx + fvtemp.vx;
				pos->vy  = pos_before->vy + fvtemp.vy;
				pos->vz  = pos_before->vz + fvtemp.vz;
			}

			uvrgbwh->a = alpha;

			pos    ++;
			pos_before++;
			uvrgbwh++;
			vec++;
		}
	}


	work->life-= OK_slow_param;
	if( work->life < 0.0f ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, SVECTOR *pole_rot )
{
	FVECTOR		*pos;
	FVECTOR		*vec;
	FVECTOR		fvtemp;
	FVECTOR		fvtemp1;
	SVECTOR		svtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	float		angle;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec       = work->vec;
	fvtemp.vx = 0.0f;
	fvtemp.vz = 0.0f;
	svtemp.vz = 0;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, center );

			fvtemp.vy   = VEC_MIN + VEC_RAND*rnd();
			svtemp.vx   = irnd()%512;
			svtemp.vy   = irnd()%4096;
			DG_SetPos2( &DG_ZeroVector, &svtemp );
			DG_RotVector( &fvtemp, vec, 1 );

#if 1
			_sceVu0ScaleVector( &fvtemp1, vec, 4.0f );
			pos->vx += fvtemp1.vx;
			pos->vy += fvtemp1.vy;
			pos->vz += fvtemp1.vz;
#else
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
#endif

//			fvtemp.vy   = SIZE + ((VEC_MIN + VEC_RAND) - fvtemp.vy) * 0.3f;	/* 速い時小さい */
			fvtemp.vy   = SIZE;
			angle       = rnd()*TPI;
			uvrgbwh->w  = (int)(fvtemp.vy * cosf( angle ));
			uvrgbwh->h  = (int)(fvtemp.vy * sinf( angle ));


			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->r  = COL_R ;
			uvrgbwh->g  = COL_G ;
			uvrgbwh->b  = COL_B ;
			uvrgbwh->a  = 0 ;


			pos ++;
			vec ++;
			uvrgbwh ++ ;
		}
	}

	DG_SetPos2( &DG_ZeroVector, pole_rot );
	DG_RotVector( work->vec, work->vec, N_PRIMS*N_VERTS );


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, SVECTOR *pole_rot )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR		svtemp;

	DG_COPY_VEC( &work->center, pos );

	tex = DG_GetTexture( 6324186 /*"bombpowder7_msk"*/ );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	
	svtemp.vx = pole_rot->vx + irnd()%512-256;
	svtemp.vy = pole_rot->vy + irnd()%512-256;
	svtemp.vz = pole_rot->vz;
	InitPacket( work, prim, tex, pos, &svtemp );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	work->life = (float)LIFE_TIME;

	return 0 ;
}

void *NewBombKasu( FVECTOR *pos, SVECTOR *pole_rot )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, pole_rot ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
