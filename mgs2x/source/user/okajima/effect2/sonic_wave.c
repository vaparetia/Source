//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sonic_wave.c
	衝撃波

	2001/03/02 S.Okajima
	$Id: sonic_wave.c,v 1.1.1.3 2002/11/19 11:47:24 Yoshizawa1 Exp $
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
#include	"./bubble.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/*----------------------------------------------------------------*/

#define	RAISE			(-60000)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(1)


#define	ADD_SIZE	(5.0f)

#define	WIDTH_RATIO_DEMO	(0.85f)
#define	WIDTH_RATIO_GAME	(0.15f)

#define	LIFE		(16)

typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		center;
	FVECTOR		vec[N_PRIMS*N_POLYS+1];
	SVECTOR		rot;
	float		radius;
	CVECTOR		col;

	int			life;

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	FVECTOR			*vec0;
	FVECTOR			*vec1;
	FVECTOR			fvtemp;
	DG_PRIM2_UVRGB	*uvrgb;
	int		j ;
	int		clock;
	int		alpha;
	float	ratio;


	ratio = WIDTH_RATIO_GAME;
	if( GM_CheckGameStatus( STATE_DEMO ) ){
		ratio = WIDTH_RATIO_DEMO;
	}

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	vec0 = work->vec;
	vec1 = vec0;
	vec1++;

	alpha = work->col.cd * work->life / LIFE;

	OK_Mem_Scr( SCR_POS, prim->pos[1-clock],   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Mem_Scr( SCR_UVR, prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	pos   = SCR_POS;
	uvrgb = SCR_UVR;
	for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
		uvrgb->a = alpha;			uvrgb++;
		uvrgb->a = alpha;			uvrgb+= 3;

		_sceVu0AddVector( pos, pos, vec0 );
		pos++;

		_sceVu0AddVector( pos, pos, vec1 );
		pos++;

		_sceVu0ScaleVector( &fvtemp, vec0, ratio );
		_sceVu0AddVector( pos, pos, &fvtemp );
		pos++;

		_sceVu0ScaleVector( &fvtemp, vec1, ratio );
		_sceVu0AddVector( pos, pos, &fvtemp );
		pos++;

		vec0++;
		vec1++;
	}
	OK_Scr_Mem( prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[clock], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

	if( --work->life <= 0 ) GV_DestroyActor( work ) ;


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			fvtemp;
	SVECTOR			svtemp;
	int				j ;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	svtemp.vx = 0;
	svtemp.vy = 0;
	fvtemp.vx = work->radius / (float)LIFE;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 0.0f;

	pos   = SCR_POS;
	uvrgb = SCR_UVR;
	for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
		svtemp.vz = 4096 * j / (N_PRIMS*N_POLYS);
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_PutVector( &fvtemp, &work->vec[j], 1 );

		DG_COPY_VEC( pos, &work->center );	pos++;
		DG_COPY_VEC( pos, &work->center );	pos++;
		DG_COPY_VEC( pos, &work->center );	pos++;
		DG_COPY_VEC( pos, &work->center );	pos++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = work->col.r ;
		uvrgb->g = work->col.g ;
		uvrgb->b = work->col.b ;
		uvrgb->a = work->col.cd ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = work->col.r ;
		uvrgb->g = work->col.g ;
		uvrgb->b = work->col.b ;
		uvrgb->a = work->col.cd ;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = work->col.r ;
		uvrgb->g = work->col.g ;
		uvrgb->b = work->col.b ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = work->col.r ;
		uvrgb->g = work->col.g ;
		uvrgb->b = work->col.b ;
		uvrgb->a = 0 ;
		uvrgb++;
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_PutVector( work->vec, work->vec, N_PRIMS*N_POLYS);
	work->vec[N_PRIMS*N_POLYS].vx = work->vec[0].vx;
	work->vec[N_PRIMS*N_POLYS].vy = work->vec[0].vy;
	work->vec[N_PRIMS*N_POLYS].vz = work->vec[0].vz;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 7744599 /*"hamon09_add_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->life = LIFE;

	return 0 ;
}

/*
*pos:			衝撃波中心座標
*nrm:			衝撃波の法線
float radius:	衝撃波の半径
CVECTOR col:	衝撃波の色（r,g,b,cd）ｃｄがαにあたる

*/
void *NewSonicWave( FVECTOR *pos, FVECTOR *nrm, float radius, CVECTOR col )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		OK_DirVecXY( &DG_ZeroVector, nrm, &work->rot );
		work->radius = radius;
		work->col = col;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}

