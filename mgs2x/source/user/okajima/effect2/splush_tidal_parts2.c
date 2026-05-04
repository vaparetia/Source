//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_tidal_parts2.c
	落下物の水飛沫
	ストリップで曲面形成
	軽量化

	2001/04/09 S.Okajima
	$Id: splush_tidal_parts2.c,v 1.1.1.3 2002/11/19 11:47:25 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#define RAISE		 (-10000)

#define	SCR_POS0	(SCRPAD_ADDR + 0x00)
#define	SCR_POS1	(SCRPAD_ADDR + 0x10)
#define	SCR_POS2	(SCRPAD_ADDR + 0x20)
#define	SCR_POS3	(SCRPAD_ADDR + 0x30)

#define COL_R (255)
#define COL_G (255)
#define COL_B (255)
#define COL_A (64)


//#define	N_VERTS		(32)
#define	N_VERTS		(16)
#define	N_PRIMS		(1)
#define	N_TOTAL		(N_PRIMS*N_VERTS)

#define	N_STRIP		(N_PRIMS*N_VERTS/2-1)

#define	INIT_PHASE	(10)
#define	LIFE		(60)

#define	DECAY_VEC	(0.95f)

#define	MIN_VEC		(50.0f)

#define	SHAKE_RATIO	(0.10f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int OK_PutSplush( FVECTOR *center, FVECTOR *force );
extern void TS_MakeMatrix2( FMATRIX *out_mat, FVECTOR *cam_z, FVECTOR *vec, FVECTOR *pos );
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_PRIM2	*prim;
	int			init_count;
	int			life;

	float		width;
	FVECTOR		center;
	FVECTOR		top_vec;
	FVECTOR		pos[N_STRIP];
	FVECTOR		vec[N_STRIP];

} Work ;


static ALIGN16_PRE float ALIGN16_POST GravityParam[N_STRIP];

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR		*c_pos;
	FVECTOR		*c_vec;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		fvtemp;
	FVECTOR		before_pos;
	FVECTOR		diff;
	FVECTOR		cam;
	float	ftemp;
//	float	local_width;
	int		clock;
	int		j,k;
	float	*gravity;

	if( GV_VecLen3F( &work->top_vec ) < MIN_VEC ){
		DG_InvisiblePrim2( work->prim );
		GV_DestroyActor( work ) ;
		return;
	}
	DG_VisiblePrim2( work->prim );


	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	if( work->init_count < N_STRIP ){
		c_vec = work->vec;
		c_vec+= work->init_count;
		ftemp = (float)(N_STRIP-work->init_count)/(float)N_STRIP;
		c_vec->vx = work->top_vec.vx*ftemp*ftemp;
		c_vec->vy = work->top_vec.vy*ftemp;
		c_vec->vz = work->top_vec.vz*ftemp*ftemp;
		work->init_count++;
	}

	ftemp = (float)work->life/(float)LIFE;
	uvrgb = prim->uvrgb[ clock ];
	for ( k = 0 ; k < N_PRIMS*N_VERTS ; k++ ){
		uvrgb->a = (int)((float)COL_A*ftemp);
		uvrgb++;
	}

/*
	if( LIFE - work->life < INIT_PHASE ){
		local_width = work->width * (float)(LIFE - work->life) / (float)INIT_PHASE;
	}else{
		local_width = work->width;
	}
*/

//	_sceVu0AddVector( &before_pos, work->pos, work->vec ) ;
	_sceVu0SubVector( &before_pos, work->pos, work->vec ) ;
	OK_Mem_Scr( SCR_POS0, prim->pos[1-clock], sizeof(FVECTOR), N_PRIMS*N_VERTS );
	pos0 = SCR_POS0;
	pos1 = SCR_POS1;
	c_pos = work->pos;
	c_vec = work->vec;
	gravity = GravityParam;




	//カメラに平行な幅を生成
	_sceVu0SubVector( &diff, c_pos, &cam ) ;
	_sceVu0SubVector( &fvtemp, &before_pos, c_pos ) ;
	_sceVu0OuterProduct( &fvtemp, &diff, &fvtemp );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, work->width*(1.0f-ftemp) );




	for( j=0; j<N_STRIP; j++ ){
		_sceVu0AddVector( c_pos, c_pos, c_vec );
		c_vec->vy+= (*gravity);


		if( fvtemp.vy > 0.0f ){
			_sceVu0AddVector( pos0, c_pos, &fvtemp ) ;
			_sceVu0SubVector( pos1, c_pos, &fvtemp ) ;
		}else{
			_sceVu0AddVector( pos1, c_pos, &fvtemp ) ;
			_sceVu0SubVector( pos0, c_pos, &fvtemp ) ;
		}

		pos0+= 2;
		pos1+= 2;
		c_pos++;
		c_vec++;
		gravity++;
	}

	pos0 = SCR_POS0;
	pos1 = SCR_POS1;
	pos0+= N_STRIP*2;
	pos1+= N_STRIP*2;
	if( fvtemp.vy > 0.0f ){
		_sceVu0AddVector( pos0, &work->center, &fvtemp ) ;
		_sceVu0SubVector( pos1, &work->center, &fvtemp ) ;
	}else{
		_sceVu0AddVector( pos1, &work->center, &fvtemp ) ;
		_sceVu0SubVector( pos0, &work->center, &fvtemp ) ;
	}

	OK_Scr_Mem( prim->pos[  clock], SCR_POS0, sizeof(FVECTOR), N_PRIMS*N_VERTS );

	if( work->life-- <= 0) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	FVECTOR			fvtemp;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		k, l ;
	int		side ;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 0, 0, 1, 0x00 ) );

	DG_COPY_VEC( &fvtemp, &work->center );

	pos0   = prim->pos[ 0 ];
	pos1   = prim->pos[ 1 ];
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];

	side = (irnd()>>8)&1;
	if( (irnd()>>8)&1 ){
		for ( k = 0 ; k < N_PRIMS*N_VERTS ; k++ ){
			l = k+side;
			DG_COPY_VEC( pos0++, &fvtemp );
			DG_COPY_VEC( pos1++, &fvtemp );

			uvrgb1->u = uvrgb0->u = FTOI12((float)(k/2) * 2.0f/(float)(N_VERTS-2) * tex->u_scale + tex->u_offset );
			uvrgb1->v = uvrgb0->v = (l&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->f = uvrgb0->f = (k<2)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
		}
	}else{
		for ( k = 0 ; k < N_PRIMS*N_VERTS ; k++ ){
			l = k+side;
			DG_COPY_VEC( pos0++, &fvtemp );
			DG_COPY_VEC( pos1++, &fvtemp );

			uvrgb1->v = uvrgb0->v = FTOI12((float)(k/2) * 2.0f/(float)(N_VERTS-2) * tex->v_scale + tex->v_offset );
			uvrgb1->u = uvrgb0->u = (l&1)?FTOI12( 1.0f * tex->u_scale + tex->u_offset ):FTOI12( tex->u_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->f = uvrgb0->f = (k<2)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
		}
	}
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		*c_pos;
	FVECTOR		*c_vec;
	int	i;
	float	ftemp;

	work->life = LIFE;

	work->init_count = 0;

//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	c_pos = work->pos;
	c_vec = work->vec;
	for( i=0; i<N_STRIP; i++ ){
		DG_COPY_VEC( c_pos, &work->center );
		DG_COPY_VEC( c_vec, &DG_ZeroVector );
		c_pos++;
		c_vec++;
	}

	DG_InvisiblePrim2( work->prim );


	for( i=0; i<N_STRIP; i++ ){
		ftemp = (float)(N_STRIP-i)/(float)N_STRIP;
		GravityParam[i] = P_GRAVITY*ftemp*ftemp;
	}


	return 0 ;
}

/* ---------------------------------------------------------------- */
/*
座標のポインタ先を毎フレーム参照
*/
void *NewSplushTidalParts2( FVECTOR *center, FVECTOR *force, float width )
{
	Work		*work ;
	SVECTOR		svtemp;
	FVECTOR		fvtemp;
	float		len;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		DG_COPY_VEC( &work->center,  center );


		len = GV_VecLen3F( force );
		_sceVu0Normalize( &fvtemp, force );
		fvtemp.vx+= SHAKE_RATIO*frnd();
		fvtemp.vy+= SHAKE_RATIO*frnd();
		fvtemp.vz+= SHAKE_RATIO*frnd();
		OK_DirVecXY( &DG_ZeroVector, &fvtemp, &svtemp );
		fvtemp.vx = 0.0f;
		fvtemp.vy = 0.0f;
		fvtemp.vz = len;

		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_PutVector( &fvtemp, &work->top_vec, 1 );
		_sceVu0ScaleVector( &work->top_vec, &work->top_vec, 0.5f );
		work->width = width*8.0f;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

