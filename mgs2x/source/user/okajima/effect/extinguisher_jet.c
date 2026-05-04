//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	extinguisher_jet.c
	消火器本体からの噴射
	2000/11/02 S.Okajima
	$Id: extinguisher_jet.c,v 1.1.1.3 2002/11/19 11:47:04 Yoshizawa1 Exp $
*/


#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern int BP_AdjustTick(int);

/*----------------------------------------------------------------*/
#define	SPEED			(60.0f)
#define	SPEED_RAND		(20.0f)
#define	ANGLE_RAND		(128)

#define N_PRIMS		(2)
#define N_VERTS		(16)

#define	COLOR_R		(255)
#define	COLOR_G		(255)
#define	COLOR_B		(255)
#define	MAX_ALPHA	(32)

#define	LIFE_TIME	( 60*8 )
#define	REFLESH_INTERVAL	(N_PRIMS * N_VERTS)

#define	SIZE_LARGE		( 50.0f )
#define	SIZE_ADD_LARGE	( 12.0f )
#define	SIZE_MAX_LARGE	( 800.0f)


typedef	struct	{
	GV_ACT_EX	actor ;
	int			map ;

	DG_PRIM2	*prim ;

	FMATRIX		world;
	FVECTOR		center;
	FVECTOR		force;
	FVECTOR		vec[N_PRIMS*N_VERTS];
	int			prim_count[N_PRIMS*N_VERTS];
	int			count;

	int			life;
	int			prim_num;

	int			se_count;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	FVECTOR		*vec;
	FVECTOR		*pos;
	FVECTOR		*pos_back;
	int	i,j;
	int	clock;
	int	count = 0;
	int	number;
	int	*prim_count;
	int	size;
	int	size_add;
	int	size_max;

//	GM_SeSetMode( SD_A_SYOKAK01, &work->center, GM_SEMODE_NORMAL );	/* 消火器通常噴出 */

	size     = SIZE_LARGE;
	size_add = SIZE_ADD_LARGE;
	size_max = SIZE_MAX_LARGE;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   // AS MCampbell - Instead of returning we need to execute any logic not related to the prims. This fixes issues 
   // where the sound effects would not play at the right speed or last too long.
   // Bug: MGSTWO-2950
   if( DG_SwitchBuffPrim2( work->prim ) )
   {
	   clock = work->prim->buffer_clock;

	   vec      = work->vec;
	   pos      = work->prim->pos[clock];
	   pos_back = work->prim->pos[1-clock];
	   uvrgbwh      = work->prim->uvrgb[clock];
	   count = 0;
	   number = 0;
	   prim_count = work->prim_count;
	   for( i=0; i<N_PRIMS; i++ ){
		   for( j=0; j<N_VERTS; j++ ){
			   if( vec->vw == 0.0f ){
				   if( number < work->life ){
					   vec->vw = size + size*rnd();
					   pos->vx = work->center.vx + vec->vx;
					   pos->vy = work->center.vy + vec->vy;
					   pos->vz = work->center.vz + vec->vz;
					   pos->vw = TPI * rnd();
					   uvrgbwh->a = MAX_ALPHA;
					   (*prim_count) = REFLESH_INTERVAL;
				   }
			   }else{
				   if( (*prim_count) < 0 ){
					   count++;
					   vec->vw = size + size*rnd();
					   pos->vx = work->center.vx + vec->vx;
					   pos->vy = work->center.vy + vec->vy;
					   pos->vz = work->center.vz + vec->vz;
					   pos->vw = TPI * rnd();
					   if( work->life < LIFE_TIME ){
						   (*prim_count) = REFLESH_INTERVAL;
						   uvrgbwh->a = MAX_ALPHA;
					   }else{
						   (*prim_count) = REFLESH_INTERVAL - ( work->life - LIFE_TIME );
						   if( (*prim_count) > 0 ){
							   uvrgbwh->a = MAX_ALPHA * (*prim_count) / REFLESH_INTERVAL;
						   }else{
							   uvrgbwh->a = 0;
						   }
					   }
				   }else{
					   vec->vw += size_add;
					   if( vec->vw > size_max ) vec->vw = size_max;
					   pos->vx = pos_back->vx + vec->vx;
					   pos->vy = pos_back->vy + vec->vy;
					   pos->vz = pos_back->vz + vec->vz;
					   if(j%2){
						   pos->vw = pos_back->vw + TPI * 0.1f * rnd();
					   }else{
						   pos->vw = pos_back->vw - TPI * 0.1f * rnd();
					   }
					   uvrgbwh->a = MAX_ALPHA * (*prim_count) / REFLESH_INTERVAL;
				   }

			   }
			   uvrgbwh->w = (int)(cosf( pos->vw ) * vec->vw );
			   uvrgbwh->h = (int)(sinf( pos->vw ) * vec->vw );
			   (*prim_count)--;

			   prim_count++;
			   number++;
			   pos++;
			   pos_back++;
			   vec ++;
			   uvrgbwh++;
		   }
	   }

      // AS MCampbell - Only increment life if we actually updated the particles, else there won't be a constant stream
	  // of particles.
      work->life++;
   }

   if( work->se_count++ > BP_AdjustTick(12) )
   {
		work->se_count = 0;
		GM_SeSetMode( SD_A_SYOKAK03 , &work->center, GM_SEMODE_NORMAL );	/* 消火器連続噴出 */
	}
    if( count >= N_PRIMS*N_VERTS-1 ){
         GV_DestroyActor( work ) ;
      }
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0;
	DG_PRIM2_UVRGBWH	*uvrgbwh1;
	int	i,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_PRIMS; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = 0 ;
			uvrgbwh0->h = uvrgbwh1->h = 0 ;
			uvrgbwh0->r = uvrgbwh1->r = COLOR_R ;
			uvrgbwh0->g = uvrgbwh1->g = COLOR_G ;
			uvrgbwh0->b = uvrgbwh1->b = COLOR_B ;
			uvrgbwh0->a = uvrgbwh1->a = 0 ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

}

static inline float NormalizeInnerProduct( FVECTOR *a, FVECTOR *b)
{
	FVECTOR	local_a;
	FVECTOR	local_b;

	_sceVu0Normalize( &local_a, a );
	_sceVu0Normalize( &local_b, b );
    return _sceVu0InnerProduct( &local_a, &local_b );
}


/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	FVECTOR		*vec;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			*prim_count;

	work->se_count = 0;

	tex = DG_GetTexture( 9998494 /*"powder02_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );


	vec = work->vec;
	pos0 = work->prim->pos[0];
	pos1 = work->prim->pos[1];
	prim_count = work->prim_count;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos0, &work->center );
			DG_COPY_VEC( pos1, pos0 );

			pos0->vw = pos1->vw = TPI * rnd();
			_sceVu0ScaleVector( vec, &work->force, -(SPEED + rnd() * SPEED_RAND) );
			vec->vw = 0.0f;		/* フラグ兼サイズ */
			(*prim_count) = REFLESH_INTERVAL;

			prim_count++;
			pos0++;
			pos1++;
			vec ++;
		}
	}

	work->life = 0;
	work->count=0;

	return 0 ;
}

/*-------------------------------------------------*/
void *NewExtinguisherJet( FVECTOR *pos, FVECTOR *force )
{
	Work		*work ;
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		_sceVu0Normalize( &work->force, force );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


