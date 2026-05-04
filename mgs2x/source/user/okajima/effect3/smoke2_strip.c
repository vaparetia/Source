//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	smoke2_strip.c
	2001/08/15 S.Okajima
	$Id: smoke2_strip.c,v 1.1.1.3 2002/11/19 11:47:35 Yoshizawa1 Exp $

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
#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(64)
#define	N_PRIMS		(1)
#define	N_VERTS_HALF	(N_VERTS/2)
#define	N_DATA		(N_PRIMS*N_VERTS_HALF)

typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		world;
	FMATRIX		root;
	float		width0;
	float		width1;
	float		angle_start;
	float		angle_limit;
	float		scale_up;
	int			col;
	int			mode;
	int			life;
	int			life_max;

	int			seed;

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	float	angle;
	float	radius;
	float	width;
	float	width_diff;
	float	time_ratio;
	float	alpha_max;
	float	angle_start;
	float	angle_limit;
	float	scale_up;
	int		j ;
	int		clock;
	int		alpha;

	alpha_max = (float)(work->col&255);

	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	time_ratio = 1.0f - (float)work->life / (float)work->life_max;

	switch( work->mode ){
	  default:
	  case 0:
		DG_COPY_MAT( &work->root, &work->world );
		break;
	  case 1:
		DG_COPY_MAT( &work->root, &DG_Chanls->eye );
		DG_COPY_VEC( (FVECTOR *)work->root.m[3], (FVECTOR *)work->world.m[3] );
		break;
	}

	width_diff = (work->width0 + (work->width1 - work->width0)*time_ratio);
	width =-width_diff * 0.5f;
	width_diff*= 1.0f / (float)N_DATA;

	alpha = (int)( alpha_max * sinf( PI*time_ratio ) );

	radius = work->width0*0.25f;

	angle_start = work->angle_start;
	angle_limit = work->angle_limit;

	scale_up = work->scale_up * time_ratio;

	pos0  = SCR_POS;
	pos1  = SCR_POS;
	pos1++;
	uvrgb  = prim->uvrgb[clock];
	for ( j = 0 ; j < N_DATA ; j++ ){
		angle = angle_start + angle_limit * (float)j / (float)N_DATA;
		pos0->vx = width;
		pos0->vy = radius * sinf( angle ) * scale_up;
		pos0->vz = 0.0f;

		width+= width_diff;

		_sceVu0ScaleVector( pos1, pos0, 0.25f );

		(uvrgb++)->a = alpha;
		(uvrgb++)->a = alpha;

		pos0+=2;
		pos1+=2;
	}

	OK_Scr_Mem( prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);

	if( --work->life <= 0 ) GV_DestroyActor( work ) ;


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb;
	int		i,j ;
	int	col_r;
	int	col_g;
	int	col_b;
	int	side;

	col_r = ((work->col>>24)&255);
	col_g = ((work->col>>16)&255);
	col_b = ((work->col>> 8)&255);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	side = (GM_IRnd(&work->seed)>>8)&1;

	uvrgb = SCR_TMP;
	if( (GM_IRnd(&work->seed)>>8)&1 ){
		for ( i=0; i<N_PRIMS; i++ ){
			for ( j=0; j<N_VERTS; j++ ){
				uvrgb->u = FTOI12((float)(j/2) * 2.0f /(float)(N_VERTS) * tex->u_scale + tex->u_offset );
				uvrgb->v = ((side+j)&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
				uvrgb->q = 4096 ;
				uvrgb->f = (j<2)? 0x8fff: 0x0fff;
				uvrgb->r = col_r ;
				uvrgb->g = col_g ;
				uvrgb->b = col_b ;
				uvrgb->a = 0;
				uvrgb++;
			}
		}
	}else{
		for ( i=0; i<N_PRIMS; i++ ){
			for ( j=0; j<N_VERTS; j++ ){
				uvrgb->u = FTOI12((float)((N_VERTS - j)/2) * 2.0f /(float)(N_VERTS) * tex->u_scale + tex->u_offset );
				uvrgb->v = ((side+j)&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
				uvrgb->q = 4096 ;
				uvrgb->f = (j<2)? 0x8fff: 0x0fff;
				uvrgb->r = col_r ;
				uvrgb->g = col_g ;
				uvrgb->b = col_b ;
				uvrgb->a = 0;
				uvrgb++;
			}
		}
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_TMP, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	prim->root = &work->root;

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex=NULL ;


	switch( (GM_IRnd(&work->seed)>>8)%5 ){
	  case 0:
		tex = DG_GetTexture( 10972307  );
		break;
	  case 1:
		tex = DG_GetTexture( 12020883 );
		break;
	  case 2:
		tex = DG_GetTexture( 13069459 );
		break;
	  case 3:
		tex = DG_GetTexture( 14118035 );
		break;
	  case 4:
		tex = DG_GetTexture( 15166611  );
		break;
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( prim );

	work->angle_start = TPI*GM_Rnd(&work->seed);
	work->angle_limit = PI * (0.75 + 2.0f *GM_Rnd(&work->seed));

	work->scale_up = 1.0f + 1.0f*GM_Rnd(&work->seed);

	return 0 ;
}

void *NewSmoke2Strip( FMATRIX *world, float width, int life, int col, int mode, int seed )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->seed = seed;

		work->col  = col;
		work->mode = mode;

		work->width0  = width;
		work->width1  = width *(1.0f + GM_Rnd(&work->seed));

		DG_COPY_MAT( &work->world, world );
		work->life     = life;
		work->life_max = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

