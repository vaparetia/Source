//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_plane.c
	板状水飛沫

	2001/01/05 S.Okajima
	$Id: splush_plane.c,v 1.1.1.3 2002/11/19 11:47:25 Yoshizawa1 Exp $
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
#define	RAISE			(0)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
//#define	N_PRIMS		(8)
#define	N_PRIMS		(1)
//以上でスクラッチパッドぎりぎり

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	SUB_ALPHA	(2)				/* 寿命に影響 */
#define	MAX_ALPHA	(SUB_ALPHA*32)	/* 寿命に影響 */

#define	CLINE	(0.3f)		/* 傾き割合 */

#define	LIFE	(32)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	FVECTOR		pos0;
	FVECTOR		pos1;
	float		height;
	int			side;

	int			life;


	FVECTOR		vec[N_PRIMS*N_POLYS];	// 頂点速度は随時計算
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*vec;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		j, k ;
	int		clock;


	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	uvrgb = prim->uvrgb[1-clock];
	if( uvrgb->a == 0 ){
		GV_DestroyActor( work ) ;
		return;
	}else{
		OK_Mem_Scr( SCR_POS, prim->pos[1-clock],   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Mem_Scr( SCR_VEC, work->vec,            sizeof(FVECTOR),        N_PRIMS*N_POLYS);
		OK_Mem_Scr( SCR_UVR, prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		pos     = SCR_POS;
		vec     = SCR_VEC;
		uvrgb   = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS ; j++ ){
			for ( k = 0 ; k < N_POLYS ; k++ ){
				if( uvrgb->a != 0 ){
					(uvrgb++)->a--;
					(uvrgb++)->a--;
					(uvrgb++)->a--;
					(uvrgb++)->a--;

					pos+=2;
					_sceVu0AddVector( pos, pos, vec );
					pos++;
					_sceVu0AddVector( pos, pos, vec );
					pos++;

					vec->vy += P_GRAVITY;
					vec++;
				}
			}
		}
		OK_Scr_Mem( prim->pos[  clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( work->vec,            SCR_VEC, sizeof(FVECTOR),        N_PRIMS*N_POLYS);
		OK_Scr_Mem( prim->uvrgb[  clock], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			diff0;
	FVECTOR			diff1;
	FVECTOR			fvtemp0;
	FVECTOR			fvtemp1;
	FVECTOR			fvtemp2;
	FVECTOR			fvside;
	FVECTOR			outer;
	float			rwidth;
	int		j, k ;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	rwidth = work->height * CLINE * 0.8f;

	_sceVu0SubVector( &diff0, &work->pos1, &work->pos0 ) ;
	_sceVu0Normalize( &fvtemp1, &diff0 );
	fvtemp1.vw = 1.0f;

	fvtemp0.vx = 0.0f;
	fvtemp0.vy = 1.0f;
	fvtemp0.vz = 0.0f;
	fvtemp0.vw = 1.0f;
	_sceVu0OuterProduct( &outer, &fvtemp0, &fvtemp1 ) ;
	if( work->side ){
		_sceVu0ScaleVector( &fvside, &outer,  CLINE*work->height );
		_sceVu0ScaleVector( &outer, &outer, work->height );
	}else{
		_sceVu0ScaleVector( &fvside, &outer, -CLINE*work->height );
		_sceVu0ScaleVector( &outer, &outer, -work->height );
	}
//printf("%f %f %f\n",outer.vx,outer.vy,outer.vz);
	_sceVu0ScaleVector( &diff0, &diff0, 1.0f/(float)(N_PRIMS*N_POLYS) );

	_sceVu0SubVector( &fvtemp0, &work->pos0, &diff0 ) ;

	_sceVu0ScaleVector( &diff1, &diff0, 2.0f );
//	DG_COPY_VEC( &fvtemp0, &work->pos0 );	//下点スタート地点
	DG_COPY_VEC( &fvtemp1, &fvtemp0 );
	fvtemp1.vy += work->height;
	_sceVu0AddVector( &fvside, &fvside, &fvtemp1 );	//上点スタート地点

	pos   = SCR_POS;
	vec   = SCR_VEC;
	uvrgb = SCR_UVR;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			_sceVu0SubVector( vec, &fvside, &fvtemp0 );
			vec->vy+= frnd()*rwidth;
			_sceVu0ScaleVector( &fvtemp1, &diff0, frnd()*0.25f );
			_sceVu0AddVector( vec, vec, &fvtemp1 );
			_sceVu0ScaleVector( vec, vec, 0.5f );
			vec++;


			_sceVu0SubVector( pos, &fvtemp0, &diff0 );
			_sceVu0ScaleVector( &fvtemp2, &outer, frnd() );
			_sceVu0AddVector( pos, pos, &fvtemp2 );
			pos++;
			_sceVu0AddVector( pos, &fvtemp0, &diff0 );
			_sceVu0ScaleVector( &fvtemp2, &outer, frnd() );
			_sceVu0AddVector( pos, pos, &fvtemp2 );
			pos++;
			_sceVu0AddVector( &fvtemp0, &fvtemp0, &diff0 );

			_sceVu0SubVector( pos, &fvside, &diff1 );
			_sceVu0ScaleVector( &fvtemp2, &outer, frnd() );
			_sceVu0AddVector( pos, pos, &fvtemp2 );
			pos++;
			_sceVu0AddVector( pos, &fvside, &diff1 );
			_sceVu0ScaleVector( &fvtemp2, &outer, frnd() );
			_sceVu0AddVector( pos, pos, &fvtemp2 );
			pos++;
			_sceVu0AddVector( &fvside, &fvside, &diff0 );


			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COL_R ;
			uvrgb->g = COL_G ;
			uvrgb->b = COL_B ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;
		}
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( work->vec,        SCR_VEC, sizeof(FVECTOR),        N_PRIMS*N_POLYS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->life = DIRECT_TICK( LIFE ) ;

	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	return 0 ;
}

void *NewSplushPlane( FVECTOR *pos0, FVECTOR *pos1, float height, int side )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos0, pos0 );
		DG_COPY_VEC( &work->pos1, pos1 );
		work->height = height;
		work->side   = side;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}
