//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	crush_dust.c
	壊れ物粉吹雪
	2000/04/13 S.Okajima
	$Id: crush_dust.c,v 1.1.1.3 2002/11/19 11:47:04 Yoshizawa1 Exp $

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

#define	COL_R			(128)
#define	COL_G			(128)
#define	COL_B			(128)
#define	ALPHA			(64)

#define N_PRIMS		(4)
#define N_VERTS		(8)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

//#define	LIFE_TIME		(64)
#define	LIFE_TIME		(32)
#define	SIZE			(1000.0f)

#define	INIT_MULTIPLE_NUM	(2)
#define	SEARCH_HZX			(700)

#define	RAISE				(0)

#define	VEC_MIN				(10.0f)
#define	VEC_RAND			(200.0f)
#define	VEC_DECAY_RATIO		(0.97f)

extern void *NewFlyingSmoke( FVECTOR *pos, FVECTOR *vector, int life );

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	DG_PRIM2	*prim ;
	FVECTOR		center;
	FVECTOR		vec[ N_PRIMS * N_VERTS ];
	int			mode;
	int			life;
	int			life_max;
	float		length;
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
	SVECTOR	rot;


	alpha = ALPHA * work->life / work->life_max;
	if( alpha < 0 ) alpha=0;


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
	fvtemp.vx  = 0.0f;
	fvtemp.vz  = 0.0f;
	rot.vz = 0;
	for( i=0; i<N_PRIMS; i++ ){
		local_count=irnd()%INIT_MULTIPLE_NUM+1;
		for( j=0; j<N_VERTS; j++ ){
			if( vec->vw == 0.0f  &&  local_count >= 0 ){
				vec->vw  = 1.0f;
				local_count--;
				DG_COPY_VEC( pos, &work->center );
			}else{
				vec->vy += P_GRAVITY*0.66f;
				vec->vx *= VEC_DECAY_RATIO;
				vec->vy *= VEC_DECAY_RATIO;
				vec->vz *= VEC_DECAY_RATIO;
				pos->vx  = pos_before->vx + vec->vx;
				pos->vy  = pos_before->vy + vec->vy;
				pos->vz  = pos_before->vz + vec->vz;
			}

			uvrgbwh->a = alpha;
			if( alpha == 0 ) uvrgbwh->h = uvrgbwh->w = 0;

			pos    ++;
			pos_before++;
			uvrgbwh++;
			vec++;
		}
	}


	work->life--;
	if( work->life < -1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, SVECTOR *pole_rot, float size, float speed, int color )
{
	FVECTOR		*pos;
	FVECTOR		*vec;
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	float		angle;
	float		ftemp;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec       = work->vec;
	fvtemp.vy = 0.0f;
	svtemp.vy = 0;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, center );

			ftemp       = speed*rnd();
			angle       = rnd()*TPI*0.06f;
			fvtemp.vz   = ftemp * cosf( angle );
			fvtemp.vx   = ftemp * sinf( angle );
			svtemp.vz   = irnd()%4096;
			DG_SetPos2( &DG_ZeroVector, &svtemp );
			DG_RotVector( &fvtemp, vec, 1 );

			angle       = rnd()*TPI;
			uvrgbwh->w  = (int)(size * cosf( angle ));
			uvrgbwh->h  = (int)(size * sinf( angle ));


			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->r  = (color >>  0)&0xff ;
			uvrgbwh->g  = (color >>  8)&0xff ;
			uvrgbwh->b  = (color >> 16)&0xff ;
			uvrgbwh->a  = (color >> 24)&0xff ;


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
static int GetResources( Work *work, FVECTOR *pos, FVECTOR *force, int mode )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR		svtemp;

	DG_COPY_VEC( &work->center, pos );
	work->length = GV_VecLen3F( force );
	if( work->length != 0.0f ){
		OK_DirVecXY( &DG_ZeroVector, force, &svtemp );
	}else{
		svtemp.vx = 0;
		svtemp.vy = 0;
		svtemp.vz = 0;
	}

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	work->mode = mode;
	switch( work->mode ){
		case 0:	/* かぼちゃ */
		default:
			tex = DG_GetTexture( 12162657 /*"pump01_alp"*/ );
			work->life_max = work->life = 30;
			InitPacket( work, prim, tex, pos, &svtemp,100.0f, 60.0f, 0x40808080 );
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
			break;
		case 1:	/* グラス */
			tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
			work->life_max = work->life = 30;
			InitPacket( work, prim, tex, pos, &svtemp,190.0f, 100.0f, 0x40806070 );
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
			break;
		case 2:	/* ワイン跳ねる */
			tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
			work->life_max = work->life = 10;
			InitPacket( work, prim, tex, pos, &svtemp,120.0f, 50.0f, 0x700a0040 );
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
			break;
		case 3:	/* りんご汁 */
			tex = DG_GetTexture( 15052494 /*"apple_alp"*/ );
			work->life_max = work->life = 25 ;
			InitPacket( work, prim, tex, pos, &svtemp,120.0f, 50.0f, 0x40808080 );
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
			break;
		case 4:	/* スイカ汁 */
			tex = DG_GetTexture( 8336485 /*watermelon01_alp*/ );
			work->life_max = work->life = 60;
			InitPacket( work, prim, tex, pos, &svtemp,200.0f, -180.0f, 0x40808080 );
			DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
			break;
	}

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}


// [modeの種類]
//	case 0:	/* かぼちゃ */
//	case 1:	/* グラス */
void *NewCrushDust( FVECTOR *pos, FVECTOR *force, int mode )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, force, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

