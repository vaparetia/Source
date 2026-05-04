//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	water_surface.c
	沸き上がる水面
	2001/05/05 S.Okajima
	$Id: water_surface.c,v 1.1.1.3 2002/11/19 11:47:39 Yoshizawa1 Exp $

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

#define	COL_R	(128)
#define	COL_G	(128)
#define	COL_B	(128)
#define	COL_A	(64)

//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984


#define	N_SIDES0			(16)	/*  */
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

#define	SCROLL_DIV		(512)
#define	DIVISION		(4)
#define	SCROLL_INIT		(SCROLL_DIV/DIVISION)

#define	LIFE		(SCROLL_DIV - SCROLL_INIT)


//#define	RADIUS_PRESS	(0.25f)
#define	RADIUS_PRESS	(0.01f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	int			life;
	int			life_max;

	DG_PRIM2	*prim_add;
	DG_PRIM2	*prim_sub;

	FVECTOR		center;
	FMATRIX		world_add;
	FMATRIX		world_sub;

	float		radius;

	int			scroll_count;
	float		offset;
	float		shift_max;

	int			my_num;

} Work ;

static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkSin[N_SIDES1];
static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkCos[N_SIDES1];
static	int		OK_WS_Starata_Num = 0;
static	FVECTOR	OK_BaseVerts[N_TOTAL];

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

	base0   = OK_BaseVerts;
	base1   = base0;
	base1++;


	for ( i = 0 ; i < N_SIDES0 ; i++ ){
		//-------------------------------
#if 0
		angle  = PI*0.5f * (float)i / (float)(N_SIDES0+1);
		height = work->radius * cosf( angle );
		radius = work->radius * sinf( angle );
		latice_pos = SCR_POS;
		p_sin = OK_WS_WorkSin;
		p_cos = OK_WS_WorkCos;
		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			latice_pos->vx = radius * (*(p_sin++));
			latice_pos->vy = radius * (*(p_cos++));
			latice_pos->vz = height * RADIUS_PRESS;
			latice_pos++;
		}
#else
		radius = work->radius * (float)(i+N_SIDES0/8) / (float)(N_SIDES0+1);
		latice_pos = SCR_POS;
		p_sin = OK_WS_WorkSin;
		p_cos = OK_WS_WorkCos;
		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			latice_pos->vx = radius * (*(p_sin++));
			latice_pos->vy = radius * (*(p_cos++));
			latice_pos->vz = 0.0f;
			latice_pos++;
		}
#endif

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
}

/* ---------------------------------------------------------------- */
static void CalcAdaptVerts( Work *work )
{
	FVECTOR		*pos;

	pos = SCR_POS;
	OK_Mem_Scr( SCR_POS, OK_BaseVerts, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	DG_SetPos( &work->world_add );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim_add->pos[0], pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim_add->pos[1], pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );


	OK_Mem_Scr( SCR_POS, OK_BaseVerts, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	DG_SetPos( &work->world_sub );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim_sub->pos[0], pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim_sub->pos[1], pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim_add;
	DG_PRIM2		*prim_sub;
	DG_PRIM2_UVRGB	*uvrgb0_add;
	DG_PRIM2_UVRGB	*uvrgb1_add;
	DG_PRIM2_UVRGB	*uvrgb0_sub;
	DG_PRIM2_UVRGB	*uvrgb1_sub;
	int		clock;
	int		i,j;
	float	alpha_base;
	int		alpha;
	float	shift;
	float	ftemp1;
	float	shift_y0;
	float	shift_y1;
	FVECTOR	*sc_pos;

	prim_add = work->prim_add;
	prim_sub = work->prim_sub;

	sc_pos = SCR_POS;
	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		sc_pos->vx = 0.0f;
		sc_pos->vy = 0.0f;
		sc_pos->vz = 1.0f;
		DG_SetPos( &DG_Chanls->eye );
		DG_RotVector( sc_pos, sc_pos, 1 );
//if(GV_Time%120==0)printf("%f\n",sc_pos->vy);
		if( sc_pos->vy > 0.3f ){
			DG_InvisiblePrim2( prim_add );
			DG_InvisiblePrim2( prim_sub );
			return;
		}
	}

	shift_y0 = SHIFT_WIDTH*(float)work->my_num;
	shift_y1 = shift_y0 + SHIFT_WIDTH*0.5f;


//AN_Test_Eye2( &work->center, 2 );

	DG_VisiblePrim2( prim_add );
	GM_GroupPrim2( prim_add, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_add );

	DG_VisiblePrim2( prim_sub );
	GM_GroupPrim2( prim_sub, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_sub );

	clock = prim_sub->buffer_clock;

	shift = work->shift_max * (float)(SCROLL_DIV - work->scroll_count) / (float)SCROLL_DIV
	      + work->offset;

	uvrgb0_add = prim_add->uvrgb[clock];
	uvrgb1_add = uvrgb0_add;
	uvrgb1_add++;

	uvrgb0_sub = prim_sub->uvrgb[clock];
	uvrgb1_sub = uvrgb0_sub;
	uvrgb1_sub++;


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
				uvrgb0_add->a = uvrgb0_sub->a = 0;
				uvrgb0_add->u = uvrgb0_sub->u = FTOI12( ftemp1 );
				uvrgb0_add+= 2;
				uvrgb0_sub+= 2;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb1_add->a = uvrgb1_sub->a = 0;
				uvrgb1_add->u = uvrgb1_sub->u = FTOI12( ftemp1 );
				uvrgb1_add+= 2;
				uvrgb1_sub+= 2;
			}
		}else if( i < N_SIDES0/2 ){
			alpha = alpha * i / (N_SIDES0/2);
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0_add->a = uvrgb1_add->a = uvrgb0_sub->a = uvrgb1_sub->a = alpha;
				uvrgb0_add->u = uvrgb1_add->u = uvrgb0_sub->u = uvrgb1_sub->u = FTOI12( ftemp1 );
				uvrgb0_add+= 2;
				uvrgb0_sub+= 2;
				uvrgb1_add+= 2;
				uvrgb1_sub+= 2;
			}
		}else{
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0_add->a = uvrgb1_add->a = uvrgb0_sub->a = uvrgb1_sub->a = alpha;
				uvrgb0_add->u = uvrgb1_add->u = uvrgb0_sub->u = uvrgb1_sub->u = FTOI12( ftemp1 );
				uvrgb0_add+= 2;
				uvrgb0_sub+= 2;
				uvrgb1_add+= 2;
				uvrgb1_sub+= 2;
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
	work->prim_add = OK_FreePrim2( work->prim_add );
	work->prim_sub = OK_FreePrim2( work->prim_sub );

	OK_WS_Starata_Num = 0;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i, k ;
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

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];

	ftemp0 = tex->v_scale * 2.0f / (float)(N_LOOP1-2);
	for ( i = 0 ; i < N_LOOP0 ; i++ ){
		ftemp1 = tex->u_scale * (float)(i  )/(float)N_LOOP0;
		ftemp2 = tex->u_scale * (float)(i+1)/(float)N_LOOP0;
		for ( k = 0 ; k < N_LOOP1 ; k++ ){
			if( k&1 ){
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp2 + tex->u_offset );
			}else{
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp1 + tex->u_offset );
			}
			uvrgb1->v = uvrgb0->v = FTOI12((float)(k>>1) * ftemp0 + tex->v_offset );
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

static int GetResources( Work *work )
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
	DG_GetPos( &work->world_add );

//	svtemp.vy+= ((irnd()>>8)&2047)+1024;
	svtemp.vy+= 32;
	DG_SetPos2( &work->center, &svtemp );
	DG_GetPos( &work->world_sub );

	work->life_max = work->life = LIFE;


//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

//	tex = DG_GetTexture( 6400230 /*"wave09_alp_ovl"*/ );

//	tex = DG_GetTexture( 7907558 /*"wave10_alp_ovl"*/ );
//	tex = DG_GetTexture( 7973094 /*"wave11_alp_ovl"*/ );
//	tex = DG_GetTexture( 5022022 /*"wave_r"*/ );

	tex = DG_GetTexture( 8038630 /*"wave12_alp_ovl"*/ );
//	tex = DG_GetTexture( 3932448 /*"wave12hrz_alp_ovl"*/ );

	prim = work->prim_add = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	InitPacket2( work, prim, tex );
	prim->raise = -16383 + RAISE_WIDTH*work->my_num;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	prim = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	InitPacket2( work, prim, tex );
	prim->raise = -16383 + RAISE_WIDTH*work->my_num + RAISE_WIDTH/2;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );


	CalcBaseVerts( work );
	CalcAdaptVerts( work );

	return 0 ;
}

void *NewWaterSurface( FVECTOR *center, float radius )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->radius     = radius;

		DG_COPY_VEC( &work->center, center );
		work->center.vy = GM_WaterLevel - work->radius*RADIUS_PRESS;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

