//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ray_missile_shower.c
	レイのミサイルシャワー
	2001/06/20 S.Okajima
	$Id: ray_missile_shower.c,v 1.1.1.3 2002/11/19 11:47:35 Yoshizawa1 Exp $
*/



#ifdef PSX2 ///
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
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	RAISE				(0)

#define	USE_MEM		(0x2000)
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVR		(SCRPAD_ADDR + USE_MEM)

#define	N_VERTS		(64)
#define	N_LINES		(1)
#define	N_POLYS		(N_VERTS/N_LINES)
#define	N_PRIMS		( USE_MEM/16/N_VERTS )

extern int DM_FrameSkip ;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	int			r_seed;

	FVECTOR		vec[N_PRIMS*N_LINES];
	DG_PRIM2	*prim ;
	int			life;
	CVECTOR		col;
	float		intense;
} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR		fvtemp;
	FVECTOR		*sc_tmp;
	FVECTOR		*sc_pos;
	FVECTOR		*vec;
	DG_PRIM2	*prim;
	int			clock;
	int			j,k;
	float		intense;


	intense = work->intense;

#if 1

	prim = work->prim;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	 //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_PRIMS * N_VERTS);
	sc_pos = SCR_POS;
	sc_tmp = SCR_POS;
	sc_tmp++;
	vec = work->vec;
	for ( j=0; j<N_PRIMS*N_LINES; j++ ){
		for ( k=0; k<N_POLYS-1; k++ ){
			DG_COPY_VEC( sc_pos++, sc_tmp++ ) ;
		}
		vec->vy+= P_GRAVITY;
		_sceVu0Normalize( &fvtemp, vec );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, intense );
		_sceVu0AddVector( vec, vec, &fvtemp );
		_sceVu0AddVector( sc_pos, sc_pos, vec++ ) ;

//AN_Test_Eye2( sc_pos, 2 );

		sc_pos++;
		sc_tmp++;
	}
	OK_Scr_Mem( prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS);


#endif

	/* 初期値が -1 なら 自殺しない */
	if( work->life >= 0 ){
		work->life -= DM_FrameSkip ;
		work->life--;
		if( work->life < 0 ){
			GV_DestroyActor( work ) ;
		}
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FMATRIX *world, float radius, float angle, int col )
{
	FVECTOR				cul_back ;
	FVECTOR				fvtemp ;
	FVECTOR				*sc_tmp ;
	FVECTOR				*sc_pos ;
	FVECTOR				*vec ;
	DG_PRIM2_UVRGB		*uvrgb ;
	float				ftemp;
	float				ftemp1;
	float				intense;
	int					j, k ;
	int					col_r;
	int					col_g;
	int					col_b;
	int					col_a;

	intense = work->intense;

	ftemp = PI*0.5f - angle;
	if( ftemp < 0.01f ) ftemp = 0.01f;
	cul_back.vx = 0.0f;
	cul_back.vy = 0.0f;
	cul_back.vz =-radius * sinf( ftemp ) / cosf( ftemp );

	DG_SetPos( world );
	DG_PutVector( &cul_back, &cul_back, 1 );


	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	col_r = ((col>>24)&255);
	col_g = ((col>>16)&255);
	col_b = ((col>> 8)&255);
	col_a = ((col>> 0)&255);
	//-------------------------------
	sc_pos = SCR_POS;
	sc_tmp = SCR_POS;
	vec    = work->vec;
	uvrgb = SCR_UVR ;
	for ( j=0; j<N_PRIMS*N_LINES; j++ ){
		ftemp1 = radius*GM_Rnd( &work->r_seed );
		ftemp  = TPI*GM_Rnd( &work->r_seed );
		sc_tmp->vx = ftemp1*sinf( ftemp );
		sc_tmp->vy = ftemp1*cosf( ftemp );
		sc_tmp->vz = 0.0f;
		DG_PutVector( sc_tmp, sc_tmp, 1 );
		for ( k=0; k<N_POLYS; k++ ){
			DG_COPY_VEC( sc_pos++, sc_tmp );

			uvrgb->u = FTOI12( (float)k / (float)N_POLYS * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( (float)k / (float)N_POLYS * tex->v_scale + tex->v_offset ) ;

			if ( k==0 ){
				uvrgb->f = 0x8fff ;
			} else {
				uvrgb->f = 0x0fff ;
			}
			uvrgb->q = 4096 ;

			if( k==N_POLYS-1 ){
				uvrgb->r = 255;
				uvrgb->g = 255;
				uvrgb->b = 255;
				uvrgb->a = 255;
			}else{
				uvrgb->r = col_r * k / N_POLYS;
				uvrgb->g = col_g * k / N_POLYS;
				uvrgb->b = col_b * k / N_POLYS;
				uvrgb->a = col_a * k / N_POLYS ;
			}

			uvrgb++ ;
		}

		_sceVu0SubVector( &fvtemp, sc_tmp, &cul_back ) ;
		_sceVu0Normalize( &fvtemp, &fvtemp );
		_sceVu0ScaleVector( &fvtemp, &fvtemp, intense );
		DG_COPY_VEC( vec++,    &fvtemp );

	}
	OK_Scr_Mem( prim->pos[0],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[1],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[0], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[1], SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

}

static int GetResources( Work *work, FMATRIX *world, float radius, float angle, int col )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

//	prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );

	InitPacket2( work, prim, tex, world, radius, angle, col );
	DG_VisiblePrim2( prim );

	return (0);
}
/* ---------------------------------------------------------------- */
void *NewRayMissileShower(
		FMATRIX *world,
		float	radius,
		float	angle,
		float	intense,
		int		col,
		int		life
		 )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->intense = intense;
		work->life = DIRECT_TICK( life );

		if ( GM_CheckGameStatus( STATE_DEMO ) ){
			work->r_seed = 1;
		}else{
			work->r_seed = irnd();
		}

		if ( GetResources( work, world, radius, angle, col ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

