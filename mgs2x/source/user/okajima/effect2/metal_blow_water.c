//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	metal_blow_water.c
	メタルギア水発射

	2001/04/03 S.Okajima
	$Id: metal_blow_water.c,v 1.1.1.3 2002/11/19 11:47:22 Yoshizawa1 Exp $

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

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplush2( FVECTOR *center, SVECTOR *rot, float length );

/*----------------------------------------------------------------*/
#define	SIZE_MIN	(200.0f)
#define	SIZE_RND	(100.0f)

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	COL_A		(255)

#define	RAISE		(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define N_VERTS		(16)
#define N_PRIMS		(0x2000 / 16 / N_VERTS)
#define N_LOOPS		(8)
#define	TOTAL_VERTS		(N_LOOPS*N_PRIMS*N_VERTS)

#define N_PARTICLE_VERTS	(32)
#define N_PARTICLE_GROUP	(TOTAL_VERTS/N_PARTICLE_VERTS)

#define DECAY_SPEED		(0.995f)
#define VEC_RATIO		(0.1f)
#define NOZZLE_RATIO	(0.5f)

#define RAISE_TIME	(60)
#define MAIN_TIME	(300)
#define TOTAL_TIME	(RAISE_TIME + MAIN_TIME)

typedef	struct	{
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim ;

	FMATRIX		*world;
	float		length;
	float		radius;

	int			init_count;
	int			act_count;

	FVECTOR		vec[TOTAL_VERTS];
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*before_pos;
	FVECTOR	*pos;
	FVECTOR	*vec;
	FVECTOR	*sc_pos;
	FVECTOR	*sc_vec;
	FVECTOR	fvtemp;
	float	local_length;
	float	local_radius;
	float	local_angle;
	int		num;
	int		clock;
	int		i,j;
	DG_PRIM2			*prim ;


	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	before_pos = prim->pos[1-clock];
	pos        = prim->pos[  clock];
	vec = work->vec;
	for ( i=0; i < N_LOOPS; i++ ){
		OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;
		OK_Mem_Scr( SCR_TMP, vec,        sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;
		sc_pos = SCR_POS;
		sc_vec = SCR_TMP;
		for ( j=0; j < N_PRIMS*N_VERTS; j++ ){
			_sceVu0AddVector( sc_pos, sc_pos, sc_vec ) ;
			_sceVu0ScaleVector( sc_vec, sc_vec, DECAY_SPEED );
			if(j%N_PARTICLE_VERTS==0) sc_vec->vy+= P_GRAVITY;
			sc_pos++;
			sc_vec++;
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;
		OK_Scr_Mem( vec, SCR_TMP, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;
		before_pos+= N_PRIMS*N_VERTS;
		pos       += N_PRIMS*N_VERTS;
		vec       += N_PRIMS*N_VERTS;
	}

	local_length = ( work->act_count < RAISE_TIME )? work->length * (float)work->act_count / (float)RAISE_TIME: work->length;

	pos = prim->pos[clock];
	vec = work->vec;
	num = work->init_count*N_PARTICLE_VERTS;
	pos+= num;
	vec+= num;
	DG_SetPos( work->world );
	for ( i=0; i < N_PARTICLE_VERTS; i++ ){
		local_radius = work->radius * rnd();
		local_angle = TPI*rnd();
		fvtemp.vx = local_radius*sinf( local_angle );
		fvtemp.vy = local_radius*cosf( local_angle );
		fvtemp.vz = 0.0f;
		DG_PutVector( &fvtemp, pos, 1 );
		fvtemp.vx*= VEC_RATIO;
		fvtemp.vy*= VEC_RATIO;
		fvtemp.vz = local_length*VEC_RATIO*rnd();
		DG_RotVector( &fvtemp, vec, 1 );
		pos++;
		vec++;
	}

	work->act_count++;
	if( work->act_count > TOTAL_TIME ) GV_DestroyActor( work ) ;

	work->init_count = ( work->init_count >= N_PARTICLE_GROUP-1 )? 0: work->init_count+1;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR	center;
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	FVECTOR	*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	float	angle;
	float	size;

	prim->raise = RAISE;
	DG_ConfigPrim2Tex( prim, tex );

	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	DG_COPY_VEC( &center, (FVECTOR *)work->world->m[3] );
	pos0       = prim->pos[0];
	pos1       = prim->pos[1];
	vec        = work->vec;
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS*N_VERTS ; i++ ){
		DG_COPY_VEC( pos0, &center );
		DG_COPY_VEC( pos1, &center );
		DG_COPY_VEC( vec, &DG_ZeroVector );

		angle = TPI*rnd();
		size = work->radius*(0.25f + rnd()*0.25f);
		uvrgbwh1->w  = uvrgbwh0->w  = (int)(cosf( angle ) * size);
		uvrgbwh1->h  = uvrgbwh0->h  = (int)(sinf( angle ) * size);
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
		uvrgbwh1->r  = uvrgbwh0->r  = COL_R ;
		uvrgbwh1->g  = uvrgbwh0->g  = COL_G ;
		uvrgbwh1->b  = uvrgbwh0->b  = COL_B ;
		uvrgbwh1->a  = uvrgbwh0->a  = COL_A ;

		pos0++;
		pos1++;
		vec++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->init_count = 0;
	work->act_count = 0;


	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	if( !InitPacket2( work, prim, tex ) ) return -1;

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}


/* ---------------------------------------------------------------- */
void *NewMetalBlow( FMATRIX *world, float radius, float length )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->world  = world;
		work->radius = radius;
		work->length = length;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

