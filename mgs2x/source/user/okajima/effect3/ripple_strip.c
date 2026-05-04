//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	ripple_strip.c
	ストリップ波紋
	2001/07/10 S.Okajima
	$Id: ripple_strip.c,v 1.1.1.3 2002/11/19 11:47:35 Yoshizawa1 Exp $

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
#define	MAX_NUM	(32)

#define	COL_R	(128)
#define	COL_G	(128)
#define	COL_B	(128)
#define	COL_A	(32)


#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(64)
#define	N_PRIMS		(1)
#define	N_VERTS_HALF	(N_VERTS/2)
#define	N_DATA		(N_PRIMS*N_VERTS_HALF)

#define	NEXT_COUNT_MIN	(10)
#define	NEXT_COUNT_RND	(30)

#define	FADE_OUT_TIME	(120)

#define	WIDTH_RATIO		(0.85f)

#define	SHIFT_UPPER	(200.0f)

#define	ANGLE	(TPI*1.5f)

typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		world;
	FVECTOR		center;
	FVECTOR		vec[N_DATA+1];
	float		radius;
	int			life;
	int			life_max;

	DG_PRIM2	*prim ;
} Work ;

static int OK_RippleStripNum;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	int		j ;
	int		clock;
	int		alpha;


	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	vec = work->vec;
	alpha = COL_A * work->life / work->life_max;
//	alpha = 255;
	OK_Mem_Scr( SCR_POS, prim->pos[1-clock],   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Mem_Scr( SCR_TMP, prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	pos0  = SCR_POS;
	pos1  = SCR_POS;
	pos1++;
	uvrgb = SCR_TMP;
	for ( j = 0 ; j < N_DATA ; j++ ){
		uvrgb->a = alpha;			uvrgb++;
		uvrgb->a = 0;				uvrgb++;

		_sceVu0AddVector( pos0, pos0, vec );
		_sceVu0ScaleVector( pos1, pos0, WIDTH_RATIO );
		pos0+=2;
		pos1+=2;
		vec++;
	}
	OK_Scr_Mem( prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[clock], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

	if( --work->life <= 0 ) GV_DestroyActor( work ) ;


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_RippleStripNum--;
	if( OK_RippleStripNum < 0 ) OK_RippleStripNum = 0;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	int				j, i ;
	float			angle_base;
	float			angle_width;
	float			angle;
	float			radius;
	float			radius_local;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	uvrgb = SCR_TMP;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_VERTS; j++ ){
			uvrgb->u = FTOI12((float)(j/2) * 2.0f /(float)(N_VERTS) * tex->u_scale + tex->u_offset );
			uvrgb->v = (j&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
			uvrgb->q = 4096 ;
			uvrgb->f = (j<2)? 0x8fff: 0x0fff;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = 0;
			uvrgb++;
		}
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

	radius = work->radius / (float)work->life_max;
	angle_base  = TPI*rnd();
	angle_width = TPI*( 0.25f + rnd()*0.5f );
	pos   = SCR_POS;
	vec = work->vec;
	for ( j=0; j<N_DATA; j++ ){
		angle = angle_base + angle_width * (float)j / (float)(N_DATA);
		radius_local = radius * (1.0f + frnd()*0.03f);
		vec->vx = radius_local * sinf( angle );
		vec->vy = 0.0f;
		vec->vz = radius_local * cosf( angle );
		vec++;

		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);

	DG_COPY_VEC( &work->vec[N_DATA].vx, &work->vec[0] );


	prim->root = &work->world;

}

static int GetResources( Work *work )
{
	SVECTOR		svtemp;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

#if 0
	DG_COPY_MAT( &work->world, &DG_UnitMatrix );
	DG_COPY_VEC( (FVECTOR *)work->world.m[3], &work->center );
#else
	svtemp.vx = 0;
	svtemp.vy = (irnd()>>8)&4095;
	svtemp.vz = 0;
	DG_SetPos2( &work->center, &svtemp );
	DG_GetPos( &work->world );
#endif

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 7744599 /*"hamon09_add_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	return 0 ;
}

void *NewRippleStrip( FVECTOR *center, float radius, int life )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_RippleStripNum > MAX_NUM ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->radius = radius;
		DG_COPY_VEC( &work->center, center );
		work->center.vy = GM_WaterLevel + 100.0f;
		work->life     = life;
		work->life_max = life;

		OK_RippleStripNum++;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

