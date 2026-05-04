//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	sps_smoke.c
	スパス煙
	2000/06/30 S.Okajima
	$Id: sps_smoke.c,v 1.1.1.3 2002/11/19 11:50:37 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifndef KP_XBOX
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
#include <sys/types.h>
#endif


#include	"gameheader.h"
#include    "libutl.h"
#include	"libmt.h"
#include	"../../okajima/etc/ok_util.h"

#define	GRAVI_RAND_LEN	(3.0f)
#define	GRAVI_PARAM		(50.0f)
#define	NEAR_LIMIT		(50.0f)

#define	SMOKE_INIT_LEN	(800.0f)
#define	SHIFT_SMOKE		(400.0f)

#define	RGB_SMOKE		(128)
#define	ALPHA_MIN		(4.0f)
#define	ALPHA_RND		(8.0f)
#define	ALPHA_SUB		(0.2f)

#define N_PRIMS		(2)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	SIZE			(200.0f)
#define	SIZE_RND		(100.0f)

#define	ZOOM_ADD		(5.0f)
#define	ANGLE_ADD		(PI * 0.002f)
#define	RAISE_SMOKE			(1000)

static FVECTOR sps_shift_mazzule = { 20.0f,-870.0f, 89.7f, 0.0f };

/*----------------------------------------------------------------*/
typedef	struct	{
	FVECTOR		vec;
	float		angle;
	float		size;
	float		alpha;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;

	FMATRIX		*world;
	FVECTOR		center;

	DG_PRIM2	*prim ;
	int			life;

	Unit		unit[N_PRIMS*N_VERTS];
} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	int		i;
	int		clock;
	int		count;
	Unit	*unit;

	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	unit = work->unit;
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	count = 0;
	for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		if(unit->alpha > ALPHA_SUB){
			unit->alpha -= ALPHA_SUB;
			uvrgbwh->a = (int)unit->alpha;
			_sceVu0AddVector( pos, pos_before, &unit->vec ) ;
			unit->size  += ZOOM_ADD;
			unit->angle += ANGLE_ADD;
			uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
			uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
		}else{
			unit->alpha = 0.0f;
			uvrgbwh->a = 0;
			count++;
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
		}

		unit++;
		pos    ++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}

//	if( work->life-- < -1 ) GV_DestroyActor( work ) ;
	if( count >= N_PRIMS*N_VERTS ) GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FMATRIX *world )
{
	FVECTOR		*pos;
	FVECTOR		unit_vec;
	FVECTOR		fvtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	Unit		*unit;
	int		i ;
	float	ftemp;

	unit_vec.vx = 0.0f;
	unit_vec.vy =-1.0f;
	unit_vec.vz = 0.0f;
	unit_vec.vw = 0.0f;
	DG_SetPos( world );
	DG_RotVector( &unit_vec, &unit_vec, 1 );

	unit       = work->unit;
	pos        = SCR_POS;
	uvrgbwh    = SCR_UVS;
	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE_SMOKE;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
//		ftemp = rnd();
		ftemp = (float)i / (float)(N_PRIMS*N_VERTS);
		_sceVu0ScaleVector( &fvtemp, &unit_vec, SMOKE_INIT_LEN*ftemp - SHIFT_SMOKE );
		_sceVu0AddVector( pos, (FVECTOR *)world->m[3], &fvtemp ) ;
		unit->angle = TPI*rnd();
		unit->size  = SIZE*ftemp + SIZE_RND*rnd();
		unit->alpha = ALPHA_MIN + ALPHA_RND*rnd();
		unit->vec.vx = 0.0f;
		unit->vec.vy = GRAVI_RAND_LEN*rnd();
		unit->vec.vz = 0.0f;

		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->w  = (int)(unit->size * cosf( unit->angle ));
		uvrgbwh->h  = (int)(unit->size * sinf( unit->angle ));
		uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = RGB_SMOKE ;
		uvrgbwh->a  = (int)( unit->alpha ) ;

		unit++;
		pos ++;
		uvrgbwh ++ ;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FMATRIX	fmtemp;

	DG_SetPos( work->world );
	DG_MovePos( &sps_shift_mazzule );
	DG_GetPos( &fmtemp );

	//------------------------------------------------------------
	/* 煙 */
	tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, /* 回転スプライト */
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, &fmtemp );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewSPS_Smoke( FMATRIX *hand )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->world = hand;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
