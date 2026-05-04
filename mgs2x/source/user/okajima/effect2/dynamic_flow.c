//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dynamic_flow.c
	巻き込みガスなど
	2000/12/12 S.Okajima
	$Id: dynamic_flow.c,v 1.1.1.3 2002/11/19 11:47:20 Yoshizawa1 Exp $
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
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#define		N_PRIMS		(128)
#define		N_VERTS		(16)
#define		MAX_PARTICLE	(N_PRIMS*N_VERTS)

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	ALPHA		(8)

#define	RAISE		(0)

#define	VEC_MIN				(10.0f)
#define	VEC_RND				(20.0f)
#define	VEC_AVE				( VEC_MIN + VEC_RND*0.5f )
#define	VEC_AVE_POW2		( VEC_AVE * VEC_AVE )
#define	DECAY_RATIO			(0.995f)
#define	ACCEL_RATIO			(0.001f)
#define	LIMIT_LENGTH		(1500.0f)
#define	LIMIT_LENGTH_POW2	(LIMIT_LENGTH * LIMIT_LENGTH)
#define	ACCEL_MAX			(10.0f)
#define	MINUS_CONST_DIV		(8.0f)	/* (1.0f/MINUS_CONST_DIV) で ＭＡＸ */
#define	LIMIT_ACCEL_MAX		(LIMIT_LENGTH_POW2 / MINUS_CONST_DIV)
#define	MINUS_CONST			(LIMIT_ACCEL_MAX * ACCEL_MAX)


#define	SCR_FVEC0	(SCRPAD_ADDR)
#define	SCR_FVEC1	(SCRPAD_ADDR+16*1)
#define	SCR_FVEC2	(SCRPAD_ADDR+16*2)
#define	SCR_FVEC3	(SCRPAD_ADDR+16*3)
#define	SCR_FVEC4	(SCRPAD_ADDR+16*4)
#define	SCR_WORK0	(SCRPAD_ADDR+1024)
#define	SCR_WORK1	(SCRPAD_ADDR+2048)
#define	SCR_WORK2	(SCRPAD_ADDR+3072)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor;
	int			name;
	int			map;

	DG_PRIM2	*prim ;

	FVECTOR		*center_plus;
	FVECTOR		*center_minus;

	float		size;
	FVECTOR		vec[MAX_PARTICLE];
	FVECTOR		bound_min;
	FVECTOR		bound_max;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR		*sc_min;
	FVECTOR		*sc_max;
	FVECTOR		*sc_plus;
	FVECTOR		*sc_minus;
	FVECTOR		*sc_temp0;
	FVECTOR		*vec;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*prim_pos0;
	FVECTOR		*prim_pos1;
	FVECTOR		*prim_vec;
	int		i, k, clock ;
	DG_PRIM2	*prim ;


	work->center_plus  = &GM_PlayerPosition;
	work->center_minus = &GM_PlayerPosition;

	sc_min   = SCR_FVEC0;
	sc_max   = SCR_FVEC1;
	sc_plus  = SCR_FVEC2;
	sc_minus = SCR_FVEC3;
	sc_temp0 = SCR_FVEC4;
	DG_COPY_VEC( sc_min,   &work->bound_min );
	DG_COPY_VEC( sc_max,   &work->bound_max );
	DG_COPY_VEC( sc_plus,  work->center_plus );
	DG_COPY_VEC( sc_minus, work->center_minus );

	prim = work->prim;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	prim_pos0 = prim->pos[1-clock];
	prim_pos1 = prim->pos[  clock];
	prim_vec  = work->vec;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		pos0 = SCR_WORK0;
		pos1 = SCR_WORK1;
		vec  = SCR_WORK2;
		OK_Mem_Scr( SCR_WORK1, prim_pos0, sizeof(FVECTOR), N_VERTS);
		OK_Mem_Scr( SCR_WORK2, prim_vec,  sizeof(FVECTOR), N_VERTS);
		for ( k = 0 ; k < N_VERTS ; k++ ){
			// 斥力
			_sceVu0SubVector( sc_temp0, pos1, sc_minus ) ;
			sc_min->vw = _sceVu0InnerProduct( sc_temp0, sc_temp0 ) ;
			if( sc_min->vw < LIMIT_LENGTH_POW2 ){	// 一定距離内
				if( sc_min->vw < LIMIT_ACCEL_MAX ){
					sc_min->vw = ACCEL_MAX;
				}else{
					sc_min->vw = MINUS_CONST/sc_min->vw;	// 力
				}
				sc_temp0->vw = 0.0f;
				_sceVu0Normalize( sc_temp0, sc_temp0 );
				_sceVu0ScaleVector( sc_temp0, sc_temp0, sc_min->vw );
				_sceVu0AddVector( vec, vec, sc_temp0 ) ;
			}

			_sceVu0AddVector( pos0, pos1, vec ) ;

			// Ｘバンダリチェック
			if( pos0->vx > sc_max->vx  ||  pos0->vx < sc_min->vx ){
				pos0->vx = pos1->vx;
				vec->vx  = -vec->vx;
			}

			// Ｙバンダリチェック
			if( pos0->vy > sc_max->vy ){
				sc_max->vw = pos0->vy - sc_max->vy;
				if( sc_max->vw  >  0.0f ){
					vec->vy -= sc_max->vw*ACCEL_RATIO;
				}
			}else if( pos0->vy < sc_min->vy ){
				pos0->vy = pos1->vy;
				vec->vy  = -vec->vy;
			}

			// Ｚバンダリチェック
			if( pos0->vz > sc_max->vz  ||  pos0->vz < sc_min->vz ){
				pos0->vz = pos1->vz;
				vec->vz  = -vec->vz;
			}

			if( _sceVu0InnerProduct( vec, vec ) > VEC_AVE_POW2 ){
				_sceVu0ScaleVector( vec, vec, DECAY_RATIO );
			}

//			if(i==0 && k==0) AN_Test_Eye2( pos0, 2 );

			vec++;
			pos0++;
			pos1++;
		}
		OK_Scr_Mem( prim_pos1, SCR_WORK0, sizeof(FVECTOR), N_VERTS);
		OK_Scr_Mem( prim_vec,  SCR_WORK2, sizeof(FVECTOR), N_VERTS);
		prim_pos0 += N_VERTS;
		prim_pos1 += N_VERTS;
		prim_vec  += N_VERTS;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	SVECTOR		svtemp0;
	FVECTOR		*vec;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i, k ;
	float		angle;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	_sceVu0SubVector( &fvtemp0, &work->bound_max, &work->bound_min ) ;

	svtemp0.vz = 0;
	fvtemp1.vx = 0.0f;
	fvtemp1.vy = 0.0f;
	vec  = work->vec;
	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgbwh0 = prim->uvrgb[ 0 ];
	uvrgbwh1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			svtemp0.vx = irnd()%2048-1024;
			svtemp0.vy = irnd()%4096;
			fvtemp1.vz = VEC_MIN + VEC_RND*rnd();
			DG_SetPos2( &DG_ZeroVector, &svtemp0 );
			DG_RotVector( &fvtemp1, vec, 1 );

			pos0->vx = work->bound_min.vx + fvtemp0.vx*rnd();
			pos0->vy = work->bound_min.vy + fvtemp0.vy*rnd();
			pos0->vz = work->bound_min.vz + fvtemp0.vz*rnd();
			DG_COPY_VEC( pos1, pos0 );

			angle        = rnd()*TPI;
			uvrgbwh1->w  = uvrgbwh0->w  = (int)(work->size * cosf( angle ));
			uvrgbwh1->h  = uvrgbwh0->h  = (int)(work->size * sinf( angle ));

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
			uvrgbwh1->a  = uvrgbwh0->a  = ALPHA ;

			vec++;
			pos0++;
			pos1++;
			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}
	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;

	if ( GCL_GetOption( 'b' ) != NULL ){
		fvtemp0.vx = (float)GCL_GetNextInt() ;
		fvtemp0.vy = (float)GCL_GetNextInt() ;
		fvtemp0.vz = (float)GCL_GetNextInt() ;
		fvtemp1.vx = (float)GCL_GetNextInt() ;
		fvtemp1.vy = (float)GCL_GetNextInt() ;
		fvtemp1.vz = (float)GCL_GetNextInt() ;

		work->bound_min.vx = (fvtemp0.vx < fvtemp1.vx)?fvtemp0.vx:fvtemp1.vx ;
		work->bound_min.vy = (fvtemp0.vy < fvtemp1.vy)?fvtemp0.vy:fvtemp1.vy ;
		work->bound_min.vz = (fvtemp0.vz < fvtemp1.vz)?fvtemp0.vz:fvtemp1.vz ;
		work->bound_max.vx = (fvtemp0.vx > fvtemp1.vx)?fvtemp0.vx:fvtemp1.vx ;
		work->bound_max.vy = (fvtemp0.vy > fvtemp1.vy)?fvtemp0.vy:fvtemp1.vy ;
		work->bound_max.vz = (fvtemp0.vz > fvtemp1.vz)?fvtemp0.vz:fvtemp1.vz ;
	}else{
		printf("ERR:dynamic_flow.c: -b option unseted");
		return -1;
	}

	work->size = 1000.0f;
	if ( GCL_GetOption( 's' ) != NULL ){
		work->size = (float)GCL_GetNextInt() ;
	}


	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
	InitPacket( work, prim, tex );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewDynamicFlow( int name, int map )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map  = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

static int GetResources_Demo( Work *work, FVECTOR *bound0, FVECTOR *bound1, float size )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	//FVECTOR		fvtemp0;
	//FVECTOR		fvtemp1;

	work->bound_min.vx = (bound0->vx < bound1->vx)?bound0->vx:bound1->vx ;
	work->bound_min.vy = (bound0->vy < bound1->vy)?bound0->vy:bound1->vy ;
	work->bound_min.vz = (bound0->vz < bound1->vz)?bound0->vz:bound1->vz ;
	work->bound_max.vx = (bound0->vx > bound1->vx)?bound0->vx:bound1->vx ;
	work->bound_max.vy = (bound0->vy > bound1->vy)?bound0->vy:bound1->vy ;
	work->bound_max.vz = (bound0->vz > bound1->vz)?bound0->vz:bound1->vz ;
	
	work->size = size;


	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
	InitPacket( work, prim, tex );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewDynamicFlow_Demo( FVECTOR *bound0, FVECTOR *bound1, float size )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		//work->name = name;
		//work->map  = map;

		if ( GetResources_Demo( work, bound0, bound1, size ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
