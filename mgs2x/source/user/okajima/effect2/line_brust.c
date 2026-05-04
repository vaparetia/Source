//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	line_brust.c
	指向性壊れエフェクト
	2001/01/24 S.Okajima
	$Id: line_brust.c,v 1.1.1.3 2002/11/19 11:47:21 Yoshizawa1 Exp $

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

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define	N_PRIMS		(2)
#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/2)

#define	DUST_GRAVITY	( P_GRAVITY * 0.125f )

#define	ANGLE_STEP	(TPI * 0.0001f)
#define	RADIUS		(20.0f)

#define	SHIFT_HZD	(10.0f)

#define	SHIFT_UPPER	(10.0f)

#define	COL_R		(128)
#define	COL_G		(64)
#define	COL_B		(16)
#define	ALP_MAX		(255)

#define	FLASH_LENGTH	(2)
#define	FLASH_LENGTH_D	(FLASH_LENGTH * 5)

#define	LIFE_TIME			(32)

#define	VEC_SCALE		(0.001f)

#define	SEARCH_HZX			(1000)

#define	RAISE				(2000)

#define	SPD_MIN		(1.0f)
#define	SPD_MAX		(200.0f)
#define	DECAY_RATIO	(0.9f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			life;

	FVECTOR		vec[N_PRIMS*N_POLYS];
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static inline void AddVector( FVECTOR *r, FVECTOR *a, FVECTOR *b  )
{
#ifdef BP_PSX2_ASM	
	asm volatile ("
	lqc2		vf1, 0(%0)
	lqc2		vf2, 0(%1)
	vadd.xyz	vf1, vf2, vf1
	sqc2		vf1, 0(%2)
	" : : "r"(a), "r"(b), "r"(r) ) ;
#else
	BP_Vec3_AddVec(r, a, b);
#endif	
}

static inline void MulVector( FVECTOR *output, FVECTOR *input )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2			vf8,0x00(%0)
	lqc2			vf9,0x00(%1)
	vmul.xyz		vf8,vf8,vf9
	sqc2			vf8,0x00(%0)
	":  : "r" (output) ,"r"(input) :"memory" );
#else
	output->vx *= input->vx;
	output->vy *= input->vy;
	output->vz *= input->vz;
#endif	
	return;
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	int		clock;
	int		alpha;
	FVECTOR		ratio;
	FVECTOR	*vec;
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	DG_PRIM2_UVRGB		*uvrgb ;

	ratio.vx = DECAY_RATIO;
	ratio.vy = DECAY_RATIO;
	ratio.vz = DECAY_RATIO;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	vec          = work->vec;
	pos          = work->prim->pos[  clock];
	pos_before   = work->prim->pos[1-clock];
	pos_before++;
	uvrgb        = work->prim->uvrgb[  clock];
	alpha        = ALP_MAX * work->life / LIFE_TIME;
	if( alpha < 0 ) alpha = 0;

	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			DG_COPY_VEC( pos++, pos_before );
			MulVector( vec, &ratio );
			vec->vy += P_GRAVITY;
			AddVector( pos, pos_before, vec );

			(uvrgb++)->a = alpha;
			uvrgb++;
			pos++;
			pos_before+=2;
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




static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, FVECTOR *force, float radius )
{
	int	i,j;
	SVECTOR	svtemp;
	FVECTOR	diff;
	FVECTOR	fvtemp;
	FVECTOR	keep;
	FVECTOR	*pos_0;
	FVECTOR	*pos_1;
	FVECTOR	*vec;
	DG_PRIM2_UVRGB		*uvrgb_0 ;
	DG_PRIM2_UVRGB		*uvrgb_1 ;

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	_sceVu0AddVector( &diff, center, force ) ;

	svtemp.vz = 0;
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = radius;

	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	pos_0   = prim->pos[ 0 ] ;
	pos_1   = prim->pos[ 1 ] ;
	vec = work->vec;
	for ( i=0; i<N_PRIMS; i++ ){
		for ( j=0; j<N_POLYS; j++ ){
			svtemp.vx = ((irnd()>>8)&2047)-1024;
			svtemp.vy = ((irnd()>>8)&4095);
			DG_SetPos2( center, &svtemp );
			DG_PutVector( &fvtemp, &keep, 1 ) ;

			_sceVu0SubVector( vec, &diff, &keep ) ;
			_sceVu0ScaleVector( vec, vec, rnd()+0.001f );
			vec++;

			DG_COPY_VEC( pos_0, &keep );
			DG_COPY_VEC( pos_1, pos_0 );
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


			DG_COPY_VEC( pos_0, &keep );
			DG_COPY_VEC( pos_1, pos_0 );
			uvrgb_0->u = uvrgb_1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_0->v = uvrgb_1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_0->f = uvrgb_1->f = 0x0fff ;
			uvrgb_0->q = uvrgb_1->q = 4096 ;
			uvrgb_0->r = uvrgb_1->r = COL_R ;
			uvrgb_0->g = uvrgb_1->g = COL_G ;
			uvrgb_0->b = uvrgb_1->b = COL_B ;
			uvrgb_0->a = uvrgb_1->a = ALP_MAX ;
			uvrgb_0++;
			uvrgb_1++;
			pos_0++;
			pos_1++;

		}
	}
}


/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, FVECTOR *force, float radius )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;

	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	prim  = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim == NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, pos, force, radius );
	DG_VisiblePrim2( prim ) ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	work->life = LIFE_TIME;


	return 0 ;
}


/* 飛び散り */
void *NewCrushLine( FVECTOR *pos, FVECTOR *force, float radius )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, pos, force, radius ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
