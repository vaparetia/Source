//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gw_lines.c
	ジョージワシントン橋のワイヤー保護ライン
	2000/08/10 S.Okajima
	$Id: gw_lines.c,v 1.1.1.3 2002/11/19 11:46:56 Yoshizawa1 Exp $
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

#define N_VERTS		(36)
#define	N_PRIMS		(2)
#define	N_POLYS		(N_PRIMS*N_VERTS/2)


#define	P_ALPHA_MAX		(128)
#define	P_RGB_R_MAX		(255)
#define	P_RGB_G_MAX		(255)
#define	P_RGB_B_MAX		(255)

#define	SIZE			(50)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		clock;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

//static FVECTOR terminal0  = { -730000.0f, 167100.0f, 14250.0f, 0.0f };
//static FVECTOR terminal1  = { -930000.0f,  74000.0f, 14250.0f, 0.0f };
static FVECTOR terminal0  = {   10000.0f, 170000.0f, 14250.0f, 0.0f };
static FVECTOR terminal1  = { -710000.0f, 170000.0f, 14250.0f, 0.0f };
static float floor_height = 66000.0f;
/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	float				ftemp;
	FVECTOR				fvtemp0 ;
	FVECTOR				fvtemp1 ;
	FVECTOR				*pos_0 ;
	FVECTOR				*pos_1 ;
	DG_PRIM2_UVRGB	*uvrgb_0 ;	/* スプライト用 */
	DG_PRIM2_UVRGB	*uvrgb_1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	fvtemp0.vx = terminal1.vx - terminal0.vx;
	fvtemp0.vy = terminal1.vy - terminal0.vy;
	fvtemp0.vz = terminal1.vz - terminal0.vz;

	//-------------------------------
	pos_0  = prim->pos[ 0 ] ;
	pos_1  = prim->pos[ 1 ] ;
	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	for ( k = 0 ; k < N_POLYS ; k++ ){
		ftemp = (float)k;
		pos_1->vx = pos_0->vx = terminal0.vx + fvtemp0.vx*ftemp / (float)N_POLYS;
		pos_1->vy = pos_0->vy = floor_height;
		pos_1->vz = pos_0->vz = terminal0.vz + fvtemp0.vz*ftemp / (float)N_POLYS;
		pos_0++ ;
		pos_1++ ;
		pos_1->vx = pos_0->vx = terminal0.vx + fvtemp0.vx*ftemp / (float)N_POLYS;
		pos_1->vy = pos_0->vy = terminal0.vy + fvtemp0.vy*ftemp / (float)N_POLYS;
		pos_1->vz = pos_0->vz = terminal0.vz + fvtemp0.vz*ftemp / (float)N_POLYS;
		pos_0++ ;
		pos_1++ ;

		uvrgb_0->u = uvrgb_1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgb_0->v = uvrgb_1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgb_0->q = uvrgb_1->q = 4096 ;
		uvrgb_0->f = uvrgb_1->f = 0x8fff ;
		uvrgb_0->r = uvrgb_1->r = P_RGB_R_MAX ;
		uvrgb_0->g = uvrgb_1->g = P_RGB_G_MAX ;
		uvrgb_0->b = uvrgb_1->b = P_RGB_B_MAX ;
		uvrgb_0->a = uvrgb_1->a = P_ALPHA_MAX ;
		uvrgb_0++ ;
		uvrgb_1++ ;

		uvrgb_0->u = uvrgb_1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgb_0->v = uvrgb_1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgb_0->q = uvrgb_1->q = 4096 ;
		uvrgb_0->f = uvrgb_1->f = 0x0fff ;
		uvrgb_0->r = uvrgb_1->r = P_RGB_R_MAX ;
		uvrgb_0->g = uvrgb_1->g = P_RGB_G_MAX ;
		uvrgb_0->b = uvrgb_1->b = P_RGB_B_MAX ;
		uvrgb_0->a = uvrgb_1->a = P_ALPHA_MAX ;
		uvrgb_0++ ;
		uvrgb_1++ ;
	}

}

static int GetResources( Work *work )
{
	FVECTOR	fvtemp;
	float	len;
	FVECTOR	*node;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 0 );
	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim );

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewGWB_Lines( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

