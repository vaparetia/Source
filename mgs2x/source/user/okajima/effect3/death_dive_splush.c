//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	death_dive_splush.c
	ゲームオーバー水飛沫
	2001/07/11 S.Okajima
	$Id: death_dive_splush.c,v 1.1.1.3 2002/11/19 11:47:29 Yoshizawa1 Exp $

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
#include	"utl_dma.h"
#include	"gameheader.h"

#include	"../etc/ok_util.h"


#define RAISE		 (-1000)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	COL_R	(255)
#define	COL_G	(255)
#define	COL_B	(255)
#define	COL_A	(16)

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

#define	N_VERTS		(64)
#define	N_PRIMS		(N_TOTAL/N_VERTS)

#define	LIFE		(60)

//#define	MARGINE_RATIO	(0.1f)
#define	MARGINE_RATIO	(0.0f)

//#define	MIN_RATIO		(0.3f)
#define	MIN_RATIO		(0.0f)
#define	SPREAD_RATIO	(4.0f)


#define	CHILD_SPLUSH_NUM	(4)
#define	CHILD_SPLUSH_MIN	(100.0f)
#define	CHILD_SPLUSH_RND	(200.0f)


/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplush( FVECTOR *center, FVECTOR *force );
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_PRIM2	*prim;

	FVECTOR		center;
	FMATRIX		world;

	float		height;
	float		height_vec;
	float		radius_min;
	float		radius_add;
	float		radius_stp;
	float		offset;
	float		shift_max;

	int			life;
	int			life_max;
	int			fade_sw;
} Work ;

static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkSin[N_SIDES1];
static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkCos[N_SIDES1];

/* ---------------------------------------------------------------- */
static void CalcBaseVerts( Work *work, int clock )
{
	FVECTOR		*base0;
	FVECTOR		*base1;
	FVECTOR		*latice_pos;
	int		i,j,k;
	float	height;
	float	ratio;
	float	ratio1;
	float	radius;
	float	radius1;
	float	angle;
	float	*p_sin;
	float	*p_cos;

	work->radius_add+= work->radius_stp;

	base0   = SCR_TMP;
	base1   = base0;
	base1++;
	for ( i = 0 ; i < N_SIDES0 ; i++ ){
		ratio = (float)i     / (float)(N_SIDES0+1);
		ratio1= (float)(i+1) / (float)(N_SIDES0+1);
		radius = work->radius_add * ratio  + work->radius_min;
		radius1= work->radius_add * ratio1 + work->radius_min;
		latice_pos = SCR_POS;
		p_sin = OK_WS_WorkSin;
		p_cos = OK_WS_WorkCos;

		height =  work->height * (1.0f - ratio*ratio) ;

		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			angle = TPI * (float)k / (float)(N_SIDES1-1);
			latice_pos->vx = radius * (*(p_sin++));
			latice_pos->vy = height * (1.0f + 0.5f*sinf(angle));
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
	OK_Scr_Mem( work->prim->pos[clock], SCR_TMP, sizeof(FVECTOR), N_PRIMS*N_VERTS );




}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		clock;
	int		i,j;
	int		alpha;
	int		flag;
	float	alpha_base;
	float	shift;

	flag = work->life_max - work->life;

	prim     = work->prim;

	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	flag = 0;
	if( work->height > 0.0f ) flag = 1;
	work->height+= work->height_vec;
	if( work->height > 0.0f ){
		work->height_vec+= P_GRAVITY*2.0f;
		if(!flag ) work->height_vec*= 0.75f;
	}else{
		work->height_vec-= P_GRAVITY*8.0f;
		if( flag ) work->height_vec*= 0.75f;
	}

	if( work->height_vec < 0.0f ) work->fade_sw = 1;
//	work->height_vec*= 0.99f;

	CalcBaseVerts( work, clock );

	shift = work->shift_max + work->offset;

	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = uvrgb0;
	uvrgb1++;

	alpha_base = (float)(COL_A * work->life) / (float)work->life_max;

	for ( i = 0 ; i < N_SIDES0 ; i++ ){
		alpha = (int)(alpha_base * sinf( PI * 0.5f * (float)(i+1) / (float)(N_SIDES0+1) ) ) ;
		if( alpha < 0 ) alpha = 0;

		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = 0;
				uvrgb0+= 2;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb1->a = 0;
				uvrgb1+= 2;
			}
		}else{
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = uvrgb1->a = alpha;
				uvrgb0+= 2;
				uvrgb1+= 2;
			}
		}
	}


	if( work->fade_sw ){
		work->life-=2;
	}else{
		work->life--;
	}
	if( work->life <= 0){
		extern void *NewRippleStrip( FVECTOR *center, float radius, int life );
		NewRippleStrip( &work->center, 30000.0f, 600 );
		GV_DestroyActor( work ) ;
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim     = OK_FreePrim2( work->prim );
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

	margine_offset = tex->v_offset;
	margine_scale  = tex->v_scale;

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

static int GetResources( Work *work, FVECTOR *center, float radius, float intense )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR 	svtemp;
	FVECTOR		fvtemp1;
	int	ftemp;
	float	flr_height[2];

	work->fade_sw = 0;

	fvtemp1.vx = center->vx;
	fvtemp1.vy = GM_WaterLevel;
	fvtemp1.vz = center->vz;
	if( HZX_LevelHazardCheck(
			GM_GetHzxGroupID( GM_CurrentStageMap ),
			&fvtemp1,
			HZX_CHK_ALL,
			0 )
		 & 2 ){
		HZX_GetLevelHeight( flr_height );
		ftemp = (flr_height[1]-GM_WaterLevel)*0.5f;
		if( intense > ftemp ) intense = ftemp;
	}




	DG_COPY_VEC( &work->center, center );
	work->radius_min = radius*MIN_RATIO;
	work->radius_add = radius - work->radius_min;
	work->radius_stp = radius*SPREAD_RATIO / (float)LIFE;
	work->height     = intense;
	work->height_vec = intense*0.125f;


	work->life_max = work->life = LIFE;


	svtemp.vx = 0;
	svtemp.vy = (irnd()>>8)&4095;
	svtemp.vz = 0;
	DG_SetPos2( center, &svtemp );
	DG_GetPos( &work->world );
	work->world.m[3][1] = GM_WaterLevel;
	work->world.m[0][0]*= 0.95f;
	work->world.m[1][1]*= 0.95f;
	work->world.m[2][2]*= 0.95f;


//	tex = DG_GetTexture( 8781729 /*"splash07_alp"*/ );
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 8038630 /*"wave12_alp_ovl"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("mo:null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->raise = RAISE;
	prim->root = &work->world;

	DG_InvisiblePrim2( prim );

	return 0 ;
}

void *NewDeathDiveSplush( FVECTOR *center, float radius, float intense )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );

		if ( GetResources( work, center, radius, intense ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

