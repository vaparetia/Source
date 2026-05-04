//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	water_reflect.c
	水面の照り返しが天井に映り込み
	2001/05/23 S.Okajima
	$Id: water_reflect_wall.c,v 1.1.1.3 2002/11/19 11:47:39 Yoshizawa1 Exp $

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
#define	COL_G	(196)
#define	COL_B	(255)
#define	COL_A	(255)

//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984


#define	N_SIDES0			(16)
#define	N_SIDES1			(16)

//#define	N_SIDES0			(8)
//#define	N_SIDES1			(8)

#define	N_TOTAL_SIDES	(N_SIDES0 * N_SIDES1)

#define	N_LOOP0		(N_SIDES0 - 1)
#define	N_LOOP1		(2 * N_SIDES1)
#define	N_TOTAL		( N_LOOP0 * N_LOOP1 )

#define	N_VERTS		(N_LOOP1)
#define	N_PRIMS		(N_TOTAL/N_VERTS)

//#define	CYCLE_TIME	(128)
#define	CYCLE_TIME	(64)

#define	RAISE_ADD	(1000)
#define	RAISE_SUB	(0)

#define	MASK				(0xfffffffc)

#define	NOBI				(4.0f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	int			name;
	int			where;

	int			origin_count0;
	int			origin_count1;
	int			scroll_count;
	int			rot_count[N_TOTAL_SIDES*2];

	DG_PRIM2	*prim_add;
	DG_PRIM2	*prim_sub;

	float		radius;
	float		offset;
	float		shift_max;

	FMATRIX		world;
	FMATRIX		world_sub;

	float		rot_shift[CYCLE_TIME];
	FVECTOR		calc;
	FVECTOR		w_verts[4];
	u_char		alpha[CYCLE_TIME];

	int			min_alpha;
	int			add_alpha;
	int			mode;

	FVECTOR		base_verts0[N_TOTAL];
	FVECTOR		base_verts1[N_TOTAL];
} Work ;


/* ---------------------------------------------------------------- */
static void UpdateCount( Work *work )
{
	int		i;
	int		*p_i;

	p_i = work->rot_count;
	for( i=0; i<N_TOTAL_SIDES*2; i++ ){
		(*p_i)--;
		if( (*p_i) < 0 ) (*p_i) = CYCLE_TIME-1;
		p_i++;
	}

}





/* ---------------------------------------------------------------- */
static void CalcBaseVertsPart( Work *work, FVECTOR *base, int num )
{
	FVECTOR		*base0;
	FVECTOR		*base1;
	FVECTOR		*latice_pos;
	FVECTOR		edge;
	FVECTOR		width;
	int		i,j,k;
	int		*p_i;
	float	*p_f;
	float	f0;
	float	f1;

	edge.vx = -work->calc.vx;
	edge.vy = -work->calc.vy;
	edge.vz = 0;

	width.vx = work->calc.vx*2.0f;
	width.vy = work->calc.vy*2.0f;
	width.vz = work->calc.vz*2.0f;

	base0   = base;
	base1   = base0;
	base1++;
	p_i = SCR_TMP;
	OK_Mem_Scr( p_i,                         &work->rot_count[num],  sizeof(int), N_TOTAL_SIDES*2 - num );
	OK_Mem_Scr( &p_i[N_TOTAL_SIDES*2 - num],  work->rot_count,       sizeof(int), num                   );
	for ( i = 0 ; i < N_SIDES0 ; i++ ){
		latice_pos = SCR_POS;
		f1 = (float)i/(float)N_SIDES0;
		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			f0 = (float)k/(float)N_SIDES1;
			p_f = work->rot_shift;
			p_f+= (*(p_i++));
			latice_pos->vx = edge.vx + width.vx*f0 + (*p_f);
			p_f = work->rot_shift;
			p_f+= (*(p_i++));
			latice_pos->vy = edge.vy + width.vy*f1 + (*p_f);
			latice_pos->vz = 0.0f;
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
}

/* ---------------------------------------------------------------- */
static void CalcBaseVerts( Work *work )
{
	CalcBaseVertsPart( work, work->base_verts0, work->origin_count0 );
	CalcBaseVertsPart( work, work->base_verts1, work->origin_count1 );
}

/* ---------------------------------------------------------------- */
static void CalcAdaptVerts( Work *work, int clock )
{
	FVECTOR		*pos;

	pos = SCR_POS;
	OK_Mem_Scr( SCR_POS, work->base_verts0, sizeof(FVECTOR), N_PRIMS*N_VERTS );
	DG_SetPos( &work->world );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim_add->pos[clock], pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );


	OK_Mem_Scr( SCR_POS, work->base_verts1, sizeof(FVECTOR), N_PRIMS*N_VERTS );
//	DG_SetPos( &work->world_sub );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );
	OK_Scr_Mem( work->prim_sub->pos[clock], pos, sizeof(FVECTOR), N_PRIMS*N_VERTS );

}

/* ---------------------------------------------------------------- */
static void Calc_Alpha( Work *work, int clock )
{
	int	i,j;
	DG_PRIM2_UVRGB	*uvrgb0_0;
	DG_PRIM2_UVRGB	*uvrgb0_1;
	DG_PRIM2_UVRGB	*uvrgb1_0;
	DG_PRIM2_UVRGB	*uvrgb1_1;
	int		*p_i;
	u_char		*p_uc;

#if 0
	p_i = work->rot_count;
	uvrgb0_0 = work->prim_add->uvrgb[clock];
	uvrgb0_1 = uvrgb0_0;
	uvrgb0_1++;
	uvrgb0_0+= N_SIDES1*2;
	for( i=1; i<N_SIDES0-1; i++ ){
		uvrgb0_0+= 2;
		uvrgb0_1+= 2;
		for( j=1; j<N_SIDES1-1; j++ ){
			p_uc = work->alpha;
			p_uc+= (*(p_i++));
			uvrgb0_0->a =
			uvrgb0_1->a = (*p_uc);
			uvrgb0_0+= 2;
			uvrgb0_1+= 2;
		}
		uvrgb0_0+= 2;
		uvrgb0_1+= 2;
	}

	p_i = &work->rot_count[N_SIDES0*N_SIDES1];
	uvrgb1_0 = work->prim_sub->uvrgb[clock];
	uvrgb1_1 = uvrgb1_0;
	uvrgb1_1++;
	uvrgb1_0+= N_SIDES1*2;
	for( i=1; i<N_SIDES0-1; i++ ){
		uvrgb1_0+= 2;
		uvrgb1_1+= 2;
		for( j=1; j<N_SIDES1-1; j++ ){
			p_uc = work->alpha;
			p_uc+= (*(p_i--));
			uvrgb1_0->a =
			uvrgb1_1->a = (*p_uc);
			uvrgb1_0+= 2;
			uvrgb1_1+= 2;
		}
		uvrgb1_0+= 2;
		uvrgb1_1+= 2;
	}
#else

	p_i = work->rot_count;
	uvrgb0_0 = work->prim_add->uvrgb[clock];
	uvrgb0_1 = uvrgb0_0;
	uvrgb0_1++;
	uvrgb0_0+= N_SIDES1*2;
	uvrgb1_0 = work->prim_sub->uvrgb[clock];
	uvrgb1_1 = uvrgb1_0;
	uvrgb1_1++;
	uvrgb1_0+= N_SIDES1*2;
	if( work->mode==0 ){
		for( i=1; i<N_SIDES0-1; i++ ){
			uvrgb0_0+= 2;
			uvrgb0_1+= 2;
			uvrgb1_0+= 2;
			uvrgb1_1+= 2;
			for( j=1; j<N_SIDES1-1; j++ ){
				p_uc = work->alpha;
				p_uc+= (*(p_i++));
				uvrgb0_0->a =
				uvrgb0_1->a =
				uvrgb1_0->a =
				uvrgb1_1->a = (*p_uc);
				uvrgb0_0+= 2;
				uvrgb0_1+= 2;
				uvrgb1_0+= 2;
				uvrgb1_1+= 2;
			}
			uvrgb0_0+= 2;
			uvrgb0_1+= 2;
			uvrgb1_0+= 2;
			uvrgb1_1+= 2;
		}
	}else{
		for( i=1; i<N_SIDES0-1; i++ ){
			uvrgb0_0+= 2;
			uvrgb0_1+= 2;
			uvrgb1_0+= 2;
			uvrgb1_1+= 2;
			for( j=1; j<N_SIDES1-1; j++ ){
				p_uc = work->alpha;
				p_uc+= (*(p_i++));
				uvrgb0_0->a =
				uvrgb0_1->a =
				uvrgb1_0->a =
				uvrgb1_1->a = work->min_alpha + (*p_uc) * (N_SIDES0-i) / N_SIDES0;
				uvrgb0_0+= 2;
				uvrgb0_1+= 2;
				uvrgb1_0+= 2;
				uvrgb1_1+= 2;
			}
			uvrgb0_0+= 2;
			uvrgb0_1+= 2;
			uvrgb1_0+= 2;
			uvrgb1_1+= 2;
		}
	}

#endif

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim_add;
	DG_PRIM2		*prim_sub;
	int		clock;
	int		i,j;
	FVECTOR	*sc_pos;

	prim_add = work->prim_add;
	prim_sub = work->prim_sub;

//	AN_Test_Eye2( &work->w_verts[0], 2 );
//	AN_Test_Eye2( &work->w_verts[1], 2 );
//	AN_Test_Eye2( &work->w_verts[2], 2 );
//	AN_Test_Eye2( &work->w_verts[3], 2 );

	/* 座標を透視変換する */
	sc_pos = SCR_POS;
	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( work->w_verts, sc_pos, 4 );
	i = 0;
	for ( j = 0 ; j < 4 ; j++ ){
		if( sc_pos->vz > sc_pos->vw ){
			i++;
		}
		sc_pos++;
	}
	if( i>=4 ){
		DG_InvisiblePrim2( prim_add );
		DG_InvisiblePrim2( prim_sub );
		return;
	}

	DG_VisiblePrim2( prim_add );
	GM_GroupPrim2( prim_add, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_add );

	DG_VisiblePrim2( prim_sub );
	GM_GroupPrim2( prim_sub, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_sub );

	clock = prim_sub->buffer_clock;

	CalcBaseVerts( work );
	CalcAdaptVerts( work, clock );
	Calc_Alpha( work, clock );
	UpdateCount( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim_add = OK_FreePrim2( work->prim_add );
	work->prim_sub = OK_FreePrim2( work->prim_sub );
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
	float	tex_scale;

	work->offset    = tex->u_offset;
	work->shift_max = tex->u_scale;

	DG_ConfigPrim2Tex( prim, tex );

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	ftemp0 = tex->v_scale * 2.0f / (float)(N_LOOP1-2);
	tex_scale = tex->u_scale / NOBI;
	for ( i = 0 ; i < N_LOOP0 ; i++ ){
		ftemp1 = tex_scale * (float)(i  )/(float)N_LOOP0;
		ftemp2 = tex_scale * (float)(i+1)/(float)N_LOOP0;
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
	SVECTOR		rot;
	FVECTOR		pos;
	float		ave;
	float		*p_f;
	int			*p_i;
	int			i;
	int			itemp0;
	int			itemp1;
	u_char		*p_uc;


	itemp1 = COL_A;
	if ( GCL_GetOption( 'a' ) != NULL ){
		itemp1 = GCL_GetNextInt();
	}
	itemp0 = 0;
	if ( GCL_GetOption( 'b' ) != NULL ){
		itemp0 = GCL_GetNextInt();
	}

	work->min_alpha = itemp0;
	work->add_alpha = itemp1 - itemp0;


	work->mode = 0;
	if ( GCL_GetOption( 'm' ) != NULL ){
		work->mode = GCL_GetNextInt();
	}




	if ( GCL_GetOption( 'r' ) != NULL ){
		rot.vx = GCL_GetNextInt();
		rot.vy = GCL_GetNextInt();
		rot.vz = GCL_GetNextInt();
	}else{
		printf("water_reflect.c::r\n");
		ASSERT(0)
	}


	itemp0 = (rot.vx+1024) & 4095;
	if(       itemp0 <= 1024 ){
		itemp1 = itemp0;
	}else if( itemp0 <= 2048 ){
		itemp1 = 2048 - itemp0;
	}else if( itemp0 <= 3096 ){
		itemp1 = itemp0 - 2048;
	}else{
		itemp1 = 4096 - itemp0;
	}

	if( itemp1 > 800 ) itemp1 = 800;
	ave = (float)itemp1 * TPI / 4096.0f;

	if ( GCL_GetOption( 'p' ) != NULL ){
		pos.vx = (float)GCL_GetNextInt();
		pos.vy = (float)GCL_GetNextInt();
		pos.vz = (float)GCL_GetNextInt();
	}else{
		printf("water_reflect.c::p\n");
		ASSERT(0)
	}

	work->calc.vz = 0.0f;
	if ( GCL_GetOption( 'w' ) != NULL ){
		work->calc.vx  = (float)GCL_GetNextInt();
	}else{
		printf("water_reflect.c::w\n");
		ASSERT(0)
	}
	if ( GCL_GetOption( 'h' ) != NULL ){
		work->calc.vy = (float)GCL_GetNextInt();
	}else{
		printf("water_reflect.c::h\n");
		ASSERT(0)
	}

	work->w_verts[0].vx = -work->calc.vx;
	work->w_verts[0].vy = -work->calc.vy;
	work->w_verts[0].vz = 0.0f;
	work->w_verts[1].vx =  work->calc.vx;
	work->w_verts[1].vy = -work->calc.vy;
	work->w_verts[1].vz = 0.0f;
	work->w_verts[2].vx = -work->calc.vx;
	work->w_verts[2].vy =  work->calc.vy;
	work->w_verts[2].vz = 0.0f;
	work->w_verts[3].vx =  work->calc.vx;
	work->w_verts[3].vy =  work->calc.vy;
	work->w_verts[3].vz = 0.0f;
	DG_PutVector( work->w_verts, work->w_verts, 4 );


	ave = (work->calc.vx + work->calc.vy) * 0.25f / (float)N_SIDES1;

	work->scroll_count = 0;

	svtemp.vx =-1024;
	svtemp.vy = (irnd()>>8)&4095;
	svtemp.vz = 0;

	DG_SetPos2( &pos, &rot );
	DG_GetPos( &work->world );

#if 0
	rot.vz+= 1024;
	DG_SetPos2( &pos, &rot );
	DG_GetPos( &work->world_sub );
/*
	DG_COPY_MAT( &work->world_sub, &work->world );
	work->world_sub.m[3][1]+= 100.0f;
*/
#endif

	tex = DG_GetTexture( 6122702 /*"water_reflection_alp_01"*/ );
//	tex = DG_GetTexture( 8038630 /*"wave12_alp_ovl"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
	prim = work->prim_add = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	InitPacket2( work, prim, tex );
	prim->raise = RAISE_ADD;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	tex = DG_GetTexture( 6122703 /*"water_reflection_alp_02"*/ );
	prim = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim );
	InitPacket2( work, prim, tex );
	prim->raise = RAISE_SUB;
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	p_uc = work->alpha;
	p_f  = work->rot_shift;
	for( i=0; i<CYCLE_TIME; i++ ){
		(*(p_f++)) = ave * sinf( TPI*(float)i/(float)CYCLE_TIME );
		(*(p_uc++)) = (u_char)( (float)work->add_alpha * (sinf( TPI*(float)i/(float)CYCLE_TIME ) + 1.0f) * 0.5f );
	}

	p_i = work->rot_count;
	for( i=0; i<N_TOTAL_SIDES*2; i++ ){
#if 1
		(*(p_i++)) = (irnd()>>8)%CYCLE_TIME;
#else
		(*p_i) = i%CYCLE_TIME;
		printf("::::%x %d\n",p_i,(*p_i) );
		p_i++;
#endif

	}

	work->origin_count0 = ((irnd()>>8)%(N_TOTAL_SIDES*2))&MASK;	// ＸとＹで2倍、転送の都合上ＭＡＳＫする
	work->origin_count1 = ((irnd()>>8)%(N_TOTAL_SIDES*2))&MASK;	// ＸとＹで2倍、転送の都合上ＭＡＳＫする

	return 0 ;
}

void *NewWaterReflectWall( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name  = name;
		work->where = where;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

