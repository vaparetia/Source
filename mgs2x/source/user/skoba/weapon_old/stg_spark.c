//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	stg_spark.c
	スティンガー火花
	2000/06/29 S.Okajima
	$Id: stg_spark.c,v 1.1.1.3 2002/11/19 11:50:38 Yoshizawa1 Exp $

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

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include	"gameheader.h"
#include	"libmt.h"

#define	N_PRIMS		(2)
#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/2)

#define	COL_R		(255)
#define	COL_G		(128)
#define	COL_B		(32)
#define	ALP_MIN		(220.0f)
#define	ALP_RND		(32.0f)
#define	SUB_ALPHA	(1.0f)

#define	VEC_MAX		(80.0f)
#define	VEC_RND		(10.0f)
#define	DECAY_RATIO	(0.95f)

typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		vec[N_PRIMS*N_POLYS];
	DG_PRIM2	*prim ;

	int			life;
	float		vec_len;

	FMATRIX		*world;
} Work ;


static	void Act( Work *work )
{
	int		i,j;
	int		clock;
	int		count;
	float	itemp;
	FVECTOR	*vec;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	DG_PRIM2_UVRGB	*uvrgb ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	vec        = work->vec;
	pos        = work->prim->pos[  clock];
	pos_before = work->prim->pos[1-clock];
	uvrgb      = work->prim->uvrgb[clock] ;
	pos_before++;
	count = 0;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			DG_COPY_VEC( pos, pos_before );
			vec->vw -= SUB_ALPHA;
			if( vec->vw <= 0.0f ){
				vec->vw  = 0.0f;
				count++;
			}
			itemp = (int)vec->vw;
			(uvrgb++)->a = itemp;
			(uvrgb++)->a = itemp;
			pos++;
			_sceVu0AddVector( pos, pos_before, vec );
			pos++;
			vec->vy += P_GRAVITY*0.5f;
			_sceVu0ScaleVector( vec, vec, DECAY_RATIO );
			pos_before+=2;
			vec++;
		}
	}

	if( count == N_PRIMS*N_POLYS ){
		GV_DestroyActor( work ) ;
	}
}

static	void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FMATRIX *world )
{
	int	i,j;
	FVECTOR	unit_vec;
	FVECTOR	center;
	SVECTOR	svtemp;
	FVECTOR	*vec;
	FVECTOR	*pos_0;
	FVECTOR	*pos_1;
	DG_PRIM2_UVRGB		*uvrgb_0 ;
	DG_PRIM2_UVRGB		*uvrgb_1 ;

	unit_vec.vx = 0.0f;
	unit_vec.vy = 1.0f;
	unit_vec.vz = 0.0f;
	unit_vec.vw = 0.0f;
	svtemp.vz = 0;

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	DG_COPY_VEC( &center, (FVECTOR *)world->m[3] );

	work->prim->buffer_clock = 0;
	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	pos_0   = prim->pos[ 0 ] ;
	pos_1   = prim->pos[ 1 ] ;
	vec     = work->vec;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			svtemp.vx = irnd()%128 + 512 + 256;
			svtemp.vy = irnd()%4096;
			svtemp.vz = 0;
			DG_SetPos( world );
			DG_RotatePos( &svtemp );
			DG_RotVector( &unit_vec, vec, 1 );
			_sceVu0ScaleVector( vec, vec, work->vec_len*rnd() );
			vec->vw  = ALP_MIN + ALP_RND*rnd();

			_sceVu0AddVector( pos_0, &center, vec );
			_sceVu0AddVector( pos_1, pos_0, vec );

			uvrgb_0->u = uvrgb_1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x8fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = 0 ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;


			DG_COPY_VEC( pos_0, &center );
			DG_COPY_VEC( pos_1, &center );
			uvrgb_0->u = uvrgb_1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x0fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = 0 ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;

			vec++;
		}
	}

}

static	int GetResources( Work *work )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;

	tex = DG_GetTexture( GV_StrCode("blood_1bw_msk") );
	prim  = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim == NULL){
		printf("null prim\n");
		return -1;
	}

	InitPacket( work, prim, tex, work->world );
	DG_VisiblePrim2( prim ) ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	return 0 ;
}

void *NewSTG_Spark( FMATRIX *hand, float vec_len )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->vec_len = vec_len;
		work->world = hand;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
