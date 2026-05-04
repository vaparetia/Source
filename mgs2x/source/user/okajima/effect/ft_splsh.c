//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ft_splsh.c
	足元水飛沫
	1999/11/06 S.Okajima
	$Id: ft_splsh.c,v 1.1.1.3 2002/11/19 11:47:06 Yoshizawa1 Exp $
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
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/
#define	PLAYER_NAME0	(10163495)	/* GV_StrCode( "ライデン" ) */
#define	PLAYER_NAME1	(2677990)	/* GV_StrCode( "スネーク" ) */

//#define	GRAVITY			(8.0f)
#define	SCR_LENGTH		( 0x4000 )

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(1)

#define	BASE_RGB		(255)
#define	MAX_ALPHA		(128)

#define	LIFE_TIME		(20)

#define	HEIGHT_BOUNDARY	(100.0f)

#define	SCALE			(100.0f)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	N_FOOT			(2)
static int foot_joint[N_FOOT] = { 16, 20 };

extern int OK_foot_splash_flag;

typedef	struct	{
	DG_PRIM2	*prim ;
	FMATRIX		*world;
	int			flag;
	int			life;
	FVECTOR		pos[N_VERTS] ;
	FVECTOR		vec[N_VERTS] ;
} Splash ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			*flag;			//オンオフフラグ

	Splash		splash[N_FOOT];

	OBJECT		*body;
	CONTROL		*control;

} Work ;

static inline void _MulVector( FVECTOR *output, FVECTOR *input )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2			vf8,0x00(%0)
	lqc2			vf9,0x00(%1)
	vmul.xyz		vf8,vf8,vf9
	sqc2			vf8,0x00(%0)
	":  : "r" (output) ,"r"(input) :"memory" );
#else

	output->x = output->x * input->x ;
	output->y = output->y * input->y ;
	output->z = output->z * input->z ;


#endif
	return;
}

/* 座標計算データを初期化する（１回目） */
static	void	InitVectors( Work *work )
{
	int	i;
	int	keep;
	int	scale;
	float	high[N_FOOT];
	float	min;
	float	len;
	float	upper;
	SVECTOR	rot;
	FVECTOR	*center;
	FVECTOR	*pos;
	FVECTOR	*vec;
	FVECTOR	fvcalc[4];
	FVECTOR	fvtemp;

	keep=-1;
	min=0.0f;
	for( i=0; i<N_FOOT; i++ ){
		high[i] = work->splash[i].world->m[3][1] - work->control->levels[ 0 ] - HEIGHT_BOUNDARY;
		/* バンダリの上に来た */
		if( high[i] > 0.0f ){
			work->splash[i].flag = 0;
		}
		if( work->splash[i].flag == 0 && min > high[i] ){
			work->splash[i].flag = 1;
			min = high[i];
			keep=i;
		}
	}

	if(keep==-1) return;
//	DG_VisiblePrim2( work->splash[keep].prim ) ;

	work->splash[keep].flag = 1;
	work->splash[keep].life = LIFE_TIME;

	center = (FVECTOR *)(&work->splash[keep].world->m[3][0]);

	pos = work->splash[keep].pos;
	vec = work->splash[keep].vec;
	rot.vx = work->control->rot.vx;
	rot.vz = work->control->rot.vz;
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;


	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		scale = SCALE;
	}else{
		scale = SCALE * 0.5f;
	}

	for( i=0; i<N_POLYS; i++ ){
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;

		len   = scale * (rnd() + 0.1f);
		upper = scale * (rnd() + 0.3f);
		fvcalc[0].vx=0.0f;
		fvcalc[0].vy=upper;
		fvcalc[0].vz=len;

		fvcalc[1].vx= scale * (rnd()*0.25f + 0.2f);
		fvcalc[1].vy=upper*0.5f;
		fvcalc[1].vz=len*0.5f;

		fvcalc[2].vx=-scale * (rnd()*0.25f + 0.2f);;
		fvcalc[2].vy=upper*0.5f;
		fvcalc[2].vz=len*0.5f;

		fvcalc[3].vx=0.0f;
		fvcalc[3].vy=upper*0.0625f;
		fvcalc[3].vz=len*0.0625f;

		rot.vy = work->control->rot.vy + (2048/N_POLYS) * (i - N_POLYS / 2);
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( fvcalc, vec, 4 );
//printf("%f %f %f \n",vec->vx,vec->vy,vec->vz);
		vec+=4;
	}

}


/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work )
{
	int	i,j;
	FVECTOR	*pos;
	FVECTOR	*vec;
	DG_PRIM2_UVRGB	*uvrgb ;
	u_char	col;
	int	clock;


	for( i=0; i<N_FOOT; i++ ){
		DG_SwitchBuffPrim2( work->splash[i].prim );
		clock = work->splash[i].prim->buffer_clock;
		if( work->splash[i].life > 0 ){
			DG_VisiblePrim2( work->splash[i].prim ) ;
			col=(u_char)( work->splash[i].life * MAX_ALPHA/LIFE_TIME );

//			col=(u_char)(MAX_ALPHA);

//printf("%d:%d\n",i,col);
			OK_Mem_Scr( SCR_POS, work->splash[i].pos,                sizeof(FVECTOR),        N_PRIMS * N_VERTS);
			OK_Mem_Scr( SCR_VEC, work->splash[i].vec,                sizeof(FVECTOR),        N_PRIMS * N_VERTS);
			OK_Mem_Scr( SCR_UVS, work->splash[i].prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
			pos   = SCR_POS;
			vec   = SCR_VEC;
			uvrgb = SCR_UVS;
			for( j=0; j<N_POLYS; j++ ){
				/* α */
				uvrgb->a = col;	uvrgb++;
				uvrgb->a = col;	uvrgb++;
				uvrgb->a = col;	uvrgb++;
				uvrgb->a = col;	uvrgb++;

				/* 座標更新 */
				_sceVu0AddVector( pos, pos, vec );
				vec->vy += P_GRAVITY;
				pos++;
				vec++;
				_sceVu0AddVector( pos, pos, vec );
				vec->vy += P_GRAVITY;
				pos++;
				vec++;
				_sceVu0AddVector( pos, pos, vec );
				vec->vy += P_GRAVITY;
				pos++;
				vec++;
				_sceVu0AddVector( pos, pos, vec );
				vec->vy += P_GRAVITY;
				pos++;
				vec++;
			}
			work->splash[i].life--;
			OK_Scr_Mem( work->splash[i].pos,                SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
			OK_Scr_Mem( work->splash[i].vec,                SCR_VEC, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
			OK_Scr_Mem( work->splash[i].prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
			OK_Scr_Mem( work->splash[i].prim->uvrgb[clock], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
		}else{
			DG_InvisiblePrim2( work->splash[i].prim ) ;
		}
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i;
	int	real_flag;

	if( work->control == NULL
	 || work->body == NULL ){
		GV_DestroyActor( work ) ;
		return;
	}

	if( work->flag==NULL ){
		real_flag = 1;
	}else{
		if( *work->flag==0 ){
			real_flag = 0;
		}else{
			real_flag = 1;
		}
	}

	UpdateVectors( work ) ;

	if( (     (work->control->level_found & 2)
	       || GM_CheckPlayerStatus(PLAYER_GROUND)
	       || (!real_flag)  )
	    && !GM_CheckGameStatus( STATE_DEMO ) ){
		OK_foot_splash_flag = 0;
		for( i=0; i<N_FOOT; i++ ){
			DG_InvisiblePrim2( work->splash[i].prim ) ;
		}
		return;
	}else{
		OK_foot_splash_flag = 1;
	}

	if( work->control->grounded & 1 ){
		InitVectors( work );
	}
}

static void Die( Work *work )
{

	work->splash[0].prim = OK_FreePrim2( work->splash[0].prim );
	work->splash[1].prim = OK_FreePrim2( work->splash[1].prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   = SCR_POS ;
	uvrgb = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = MAX_ALPHA ;
			uvrgb++;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

static int GetResources( Work *work, OBJECT *body, CONTROL *control )
{
	DG_PRIM2		*prim ;
	DG_TEX		*tex ;

	if( control == NULL
	 || body == NULL ){
		GV_DestroyActor( work ) ;
		return -1;
	}
	work->control = control;
	work->body = body;

//	tex = DG_GetTexture( 6697084 /*"splash05_msk"*/ );
	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );


//	tex = DG_GetTexture( 5648508 /*"splash04_msk"*/ );
//	tex = DG_GetTexture( 4599932 /*"splash03_msk"*/ );
//	tex = DG_GetTexture( 7745660 /*"splash06_msk"*/ );

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );



	prim = work->splash[0].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	work->splash[0].flag=0;
	work->splash[0].world = &body->objs->objs[ foot_joint[0] ].world;

	prim = work->splash[1].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	work->splash[1].flag=0;
	work->splash[1].world = &body->objs->objs[ foot_joint[1] ].world;

	if( !GM_CheckGameStatus( STATE_DEMO )
	 && ( (control->name == PLAYER_NAME0) || (control->name == PLAYER_NAME1) ) ){
		extern void *NewSplashMotion_Demo(
			DG_OBJS *objs,		/* [ポインタ保存参照]対象オブジェ */
			float	step_limit,
			int life			/* 寿命。フレーム指定。初期値が -1 なら 自殺しない */
			 );
		void	*p0;
		p0 = NewSplashMotion_Demo(
			body->objs,		/* [ポインタ保存参照]対象オブジェ */
			200.0f,
			-1			/* 寿命。フレーム指定。初期値が -1 なら 自殺しない */
			 );
		if(p0) GV_SetActorChild(work,p0);
	}

	return 0 ;
}

void *NewFootSplash( OBJECT *body, CONTROL *control )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = NULL;

		if ( GetResources( work, body, control ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void *NewFootSplash_AddedFlag( OBJECT *body, CONTROL *control, int *flag )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = flag;

		if ( GetResources( work, body, control ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
