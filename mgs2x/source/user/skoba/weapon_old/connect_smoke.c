//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	connect_smoke.c
	ポインタ参照先に接続する煙 
	2000/07/05 S.Okajima
	$Id: connect_smoke.c,v 1.1.1.3 2002/11/19 11:50:33 Yoshizawa1 Exp $

*/

#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"


#define	MULTIPLE	(4.0f)

#define	GRAVI_RAND_LEN	(3.0f)

#ifdef PSX2
#define N_PRIMS		(16)
#else //KP_XBOX /* XBOXではひきつり過ぎてゲーム性が崩れるため2002.06.25yano */
#define N_PRIMS		(4)
#endif
#define N_VERTS		(16)

#define	RGB_SMOKE		(255)
#define	ALPHA_MIN		(6.0f)
#define	ALPHA_RND		(6.0f)
#define	ALPHA_SUB		( (float)(ALPHA_MIN+ALPHA_RND)/(float)(N_PRIMS*N_VERTS)*MULTIPLE )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	SIZE_MIN		(4.0f)
#define	SIZE_RND		(16.0f)

#define	ZOOM_ADD		(0.7f)
#define	ANGLE_ADD		(PI * 0.004f)
#define	RAISE_SMOKE			(1000)

/*----------------------------------------------------------------*/
typedef	struct	{
	int			count;
	FVECTOR		vec;
	float		angle;
	float		size;
	float		alpha;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;

	FMATRIX		*world;
	FVECTOR		shift;
	FVECTOR		before_pos;

	DG_PRIM2	*prim ;
	int			life;

	float		size_rnd;

	Unit		unit[N_PRIMS*N_VERTS];
} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	now_pos;
	FVECTOR	dif_pos;
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FMATRIX	fmtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	int	i;
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

	DG_SetPos( work->world );
	DG_MovePos( &work->shift );
	DG_GetPos( &fmtemp );
	DG_COPY_VEC( &now_pos, (FVECTOR *)fmtemp.m[3] );

	_sceVu0SubVector( &dif_pos, &now_pos, &work->before_pos ) ;

	unit = work->unit;
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	count = 0;
	for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		if(unit->count-- > 0){
		}else if(unit->alpha > ALPHA_SUB){
			unit->alpha -= ALPHA_SUB;
			uvrgbwh->a = (int)unit->alpha;
			_sceVu0AddVector( pos, pos_before, &unit->vec ) ;
			unit->size  += ZOOM_ADD;
			unit->angle += ANGLE_ADD;
			uvrgbwh->w = (int)(unit->size * cosf( unit->angle ));
			uvrgbwh->h = (int)(unit->size * sinf( unit->angle ));
		}else if( work->life > 0 ){
			DG_COPY_VEC( pos, &now_pos );

			_sceVu0ScaleVector( &fvtemp, &dif_pos, rnd() );
			_sceVu0AddVector( pos, &work->before_pos, &fvtemp ) ;
			unit->angle  = TPI*rnd();
			unit->size   = SIZE_MIN  + work->size_rnd *rnd();
			unit->alpha  = ALPHA_MIN + ALPHA_RND*rnd();
			unit->vec.vx = 0.0f;
			unit->vec.vy = GRAVI_RAND_LEN*rnd();
			unit->vec.vz = 0.0f;
		}else{
			count++;
		}

		unit++;
		pos    ++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}
	if( count >= N_PRIMS*N_VERTS ) GV_DestroyActor( work ) ;

	work->life--;

	DG_COPY_VEC( &work->before_pos, &now_pos );


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FMATRIX *world )
{
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	FVECTOR		unit_vec;
	FMATRIX		fmtemp;
	Unit		*unit;
	int		i ;

	DG_SetPos( work->world );
	DG_MovePos( &work->shift );
	DG_GetPos( &fmtemp );

	DG_COPY_VEC( &work->before_pos, (FVECTOR *)fmtemp.m[3] );


	unit_vec.vx = 0.0f;
	unit_vec.vy =-1.0f;
	unit_vec.vz = 0.0f;
	unit_vec.vw = 0.0f;
	DG_SetPos( world );
	DG_RotVector( &unit_vec, &unit_vec, 1 );

	unit       = work->unit;
	pos0       = prim->pos[ 0 ];
	pos1       = prim->pos[ 1 ];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE_SMOKE;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		unit->count  = i/(int)MULTIPLE;

		uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;
		uvrgbwh0->w  = uvrgbwh1->w  = 0;
		uvrgbwh0->h  = uvrgbwh1->h  = 0;
		uvrgbwh0->a  = uvrgbwh1->a  = 0 ;
		uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = 
		uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_SMOKE ;

		DG_COPY_VEC( pos0, &DG_ZeroVector );
		DG_COPY_VEC( pos1, &DG_ZeroVector );

		unit++;
		pos0 ++;
		pos1 ++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

//	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
//	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
//	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
//	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

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
	InitPacket( work, prim, tex, work->world );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewConnectSmoke( FMATRIX *world, FVECTOR *shift, int life, float size_rnd )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;
		DG_COPY_VEC( &work->shift, shift );
		work->life = life;
		work->size_rnd = size_rnd;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
