//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	water_surface2.c
	沸き上がる水面
	2001/05/05 S.Okajima
	$Id: water_surface2.c,v 1.1.1.3 2002/11/19 11:47:40 Yoshizawa1 Exp $

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
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


#define STRATA_NUM	 (16)
#define RAISE_WIDTH	 (8192)
#define SHIFT_WIDTH	 (20.0f)


#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	COL_R	(255)
#define	COL_G	(255)
#define	COL_B	(255)
#define	COL_A	(32)

//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984


#define	N_SIDES0			(8)	/*  */
#define	N_SIDES1			(32)	/* 円周分割数 */
#define	N_TOTAL_SIDES	(N_SIDES0 * N_SIDES1)

#define	N_LOOP0		(N_SIDES0 - 1)
#define	N_LOOP1		(2 * N_SIDES1)
#define	N_TOTAL		( N_LOOP0 * N_LOOP1 )

#if 0
#define	N_VERTS		(N_LOOP1)
#define	N_PRIMS		(N_TOTAL/N_VERTS)
#else
#define	N_VERTS		(64)
#define	N_PRIMS		(N_TOTAL/N_VERTS)
#endif

#define	SCROLL_DIV		(1024)
#define	DIVISION		(4)
#define	SCROLL_INIT		(SCROLL_DIV/DIVISION)

#define	LIFE		(SCROLL_DIV - SCROLL_INIT)


//#define	RADIUS_PRESS	(0.25f)
#define	RADIUS_PRESS	(0.01f)

#define	MARGINE_RATIO	(0.1f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	int			life;
	int			life_max;

	DG_PRIM2	*prim;

	FVECTOR		center;
	FMATRIX		world;

	float		radius;

	int			scroll_count;
	float		offset;
	float		shift_max;

	int			my_num;

} Work ;

static	int		OK_WS_Starata_Num = 0;
static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkSin[N_SIDES1];
static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkCos[N_SIDES1];

/* ---------------------------------------------------------------- */
static void CalcBaseVerts( Work *work )
{
	FVECTOR		*base0;
	FVECTOR		*base1;
	FVECTOR		*latice_pos;
	int		i,j,k;
//	float	angle;
//	float	height;
	float	radius;
	float	*p_sin;
	float	*p_cos;

	base0   = SCR_TMP;
	base1   = base0;
	base1++;


	for ( i = 0 ; i < N_SIDES0 ; i++ ){
		radius = work->radius * (float)i / (float)(N_SIDES0+1);
		latice_pos = SCR_POS;
		p_sin = OK_WS_WorkSin;
		p_cos = OK_WS_WorkCos;
		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			latice_pos->vx = radius * (*(p_sin++));
			latice_pos->vy = 0.0f;
			latice_pos->vz = radius * (*(p_cos++));
			latice_pos++;
		}

		latice_pos = SCR_POS;
		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base0, latice_pos );
				base0+=2;
				latice_pos++;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base1, latice_pos );
				base1+=2;
				latice_pos++;
			}
		}else{
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base0, latice_pos );
				DG_COPY_VEC( base1, latice_pos );
				base0+=2;
				base1+=2;
				latice_pos++;
			}
		}
	}

	OK_Scr_Mem( work->prim->pos[0], SCR_TMP, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim->pos[1], SCR_TMP, sizeof(FVECTOR), N_PRIMS*N_VERTS );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim;

	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;

	int		clock;
	int		i,j;
	float	alpha_base;
	int		alpha;
	float	shift;
	float	ftemp1;
	float	shift_y0;
	float	shift_y1;
	FVECTOR	*sc_pos;


	prim = work->prim;

	sc_pos = SCR_POS;
	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		sc_pos->vx = 0.0f;
		sc_pos->vy = 0.0f;
		sc_pos->vz = 1.0f;
		DG_SetPos( &DG_Chanls->eye );
		DG_RotVector( sc_pos, sc_pos, 1 );
//if(GV_Time%120==0)printf("%f\n",sc_pos->vy);
		if( sc_pos->vy > 0.3f ){
			DG_InvisiblePrim2( prim );
			return;
		}
	}

	shift_y0 = SHIFT_WIDTH*(float)work->my_num;
	shift_y1 = shift_y0 + SHIFT_WIDTH*0.5f;


//AN_Test_Eye2( &work->center, 2 );

	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	shift = work->shift_max * (float)(SCROLL_DIV - work->scroll_count) / (float)SCROLL_DIV
	      + work->offset;

	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = uvrgb0;
	uvrgb1++;

	alpha_base = (float)COL_A * sinf( PI * (float)work->life / (float)work->life_max ) ;
//	alpha = COL_A * work->life / work->life_max;

	for ( i = 0 ; i < N_SIDES0 ; i++ ){

		ftemp1 = work->shift_max / (float)DIVISION * (float)(i  )/(float)N_LOOP0 + shift;
		if( ftemp1 > work->shift_max + work->offset ){
			ftemp1 = work->shift_max + work->offset;
		}else if( ftemp1 < work->offset ){
			ftemp1 = work->offset;
		}
//		alpha = (int)(alpha_base * sinf( PI * (float)(i+1) / (float)(N_SIDES0+1) ) ) ;
		alpha = (int)(alpha_base * cosf( PI * 0.5f * (float)(i+1) / (float)(N_SIDES0+1) ) ) ;


		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = 0;
				uvrgb0->u = FTOI12( ftemp1 );
				uvrgb0+= 2;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb1->a = 0;
				uvrgb1->u = FTOI12( ftemp1 );
				uvrgb1+= 2;
			}
		}else{
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = uvrgb1->a = alpha;
				uvrgb0->u = uvrgb1->u = FTOI12( ftemp1 );
				uvrgb0+= 2;
				uvrgb1+= 2;
			}
		}

	}

	work->scroll_count++;
	if( work->scroll_count > SCROLL_DIV ) work->scroll_count = SCROLL_INIT;

	if( work->life-- <= 0) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );

	OK_WS_Starata_Num = 0;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i, k ;
	float	margine_offset;
	float	margine_scale;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	angle;
	float	*p_sin;
	float	*p_cos;

	work->offset    = tex->u_offset;
	work->shift_max = tex->u_scale;


	p_sin = OK_WS_WorkSin;
	p_cos = OK_WS_WorkCos;
	for ( k = 0 ; k < N_SIDES1 ; k++ ){
		angle = TPI * (float)k / (float)(N_SIDES1-1);
		(*p_sin++) = sinf( angle );
		(*p_cos++) = cosf( angle );
	}


	DG_ConfigPrim2Tex( prim, tex );

	margine_offset = tex->v_scale * MARGINE_RATIO + tex->v_offset;
	margine_scale  = tex->v_scale * (1.0f - MARGINE_RATIO*2.0f);

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	ftemp0 = margine_scale * 2.0f / (float)(N_LOOP1-2);
	for ( i = 0 ; i < N_LOOP0 ; i++ ){
		ftemp1 = tex->u_scale * (float)(i  )/(float)N_LOOP0;
		ftemp2 = tex->u_scale * (float)(i+1)/(float)N_LOOP0;
		for ( k = 0 ; k < N_LOOP1 ; k++ ){
			if( k&1 ){
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp2 + tex->u_offset );
			}else{
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp1 + tex->u_offset );
			}
			uvrgb1->v = uvrgb0->v = FTOI12((float)(k>>1) * ftemp0 + margine_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (k==0)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = COL_R;
			uvrgb1->g = uvrgb0->g = COL_G;
			uvrgb1->b = uvrgb0->b = COL_B;
			uvrgb1->a = uvrgb0->a = 0;
			uvrgb0++;
			uvrgb1++;
		}
	}

}

static int GetResources( Work *work, FVECTOR *center )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR		svtemp;

	work->my_num = OK_WS_Starata_Num;

	OK_WS_Starata_Num++;
	if( OK_WS_Starata_Num >= STRATA_NUM ) OK_WS_Starata_Num = 0;

	work->scroll_count = SCROLL_INIT;

	svtemp.vx =-1024;
	svtemp.vy = (irnd()>>8)&4095;
	svtemp.vz = 0;

	DG_SetPos2( &work->center, &svtemp );
	DG_GetPos( &work->world );

	work->life_max = work->life = LIFE;


//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

//	tex = DG_GetTexture( 6400230 /*"wave09_alp_ovl"*/ );
//	tex = DG_GetTexture( 7907558 /*"wave10_alp_ovl"*/ );
//	tex = DG_GetTexture( 7973094 /*"wave11_alp_ovl"*/ );
//	tex = DG_GetTexture( 5022022 /*"wave_r"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	InitPacket2( work, prim, tex );
	prim->raise = -16383 + RAISE_WIDTH*work->my_num;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	svtemp.vx = 0;
	svtemp.vy = (irnd()>>8)&4095;
	svtemp.vz = 0;
	DG_SetPos2( center, &svtemp );
	DG_GetPos( &work->world );
	work->world.m[3][1] = GM_WaterLevel;
	prim->root = &work->world;

	CalcBaseVerts( work );

	return 0 ;
}

void *NewWaterSurface2( FVECTOR *center, float radius )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->radius     = radius;

		if ( GetResources( work, center ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

