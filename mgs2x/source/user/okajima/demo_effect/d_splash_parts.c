//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash_parts.c
	デモ用水飛沫（実部）
	2000/04/01 S.Okajima
	$Id: d_splash_parts.c,v 1.1.1.3 2002/11/19 11:46:55 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libmt.h"
#include        "libutl.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/

#define	SCR_LENGTH		( 0x4000 )

#define	N_VERTS		(16)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(1)

#define	BASE_RGB		(255)
#define	MAX_ALPHA		(128)

#define	LIFE_TIME		(16)

#define	SCALE			(80.0f)
#define	WIDTH			(5.0f)
#define	UPPER			(10.0f)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	SCR_W0			(SCRPAD_ADDR + 0x3f00)
#define	SCR_W1			(SCRPAD_ADDR + 0x3f10)
#define	SCR_W2			(SCRPAD_ADDR + 0x3f20)
#define	SCR_W3			(SCRPAD_ADDR + 0x3f30)
#define	SCR_W4			(SCRPAD_ADDR + 0x3f40)

#define scw_fvtemp0     ((FVECTOR *)SCR_W0)
#define scw_fvcalc0     ((FVECTOR *)SCR_W1)

#define	ACHUU_GRAVITY	(-0.1f)

typedef	struct	{
	GV_ACT_EX		actor ;

	float	intense;
	DG_PRIM2	*prim ;
	FVECTOR		center;
	FVECTOR		rot;

	int			life;
	FVECTOR		pos[N_VERTS] ;
	FVECTOR		vec[N_VERTS] ;

} Work ;

/* 座標計算データを初期化する */
static	void	InitVectors( Work *work )
{
	int	i;
	float	len;
	SVECTOR	rot;
	FVECTOR	*center;
	FVECTOR	*pos;
	FVECTOR	*vec;

	rot.vx = work->rot.vx;
	rot.vy = work->rot.vy;
	center = &work->center;

	pos = work->pos;
	vec = work->vec;
	for( i=0; i<N_POLYS; i++ ){
#if 0
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
#else
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf12,0x00(%1)

			vmove.xyz	vf8 ,vf12
			vmove.xyz	vf9 ,vf12
			vmove.xyz	vf10,vf8
			vmove.xyz	vf11,vf9

			sqc2		vf8 ,0x00(%0)
			sqc2		vf9 ,0x10(%0)
			sqc2		vf10,0x20(%0)
			sqc2		vf11,0x30(%0)
			"::"r"(pos),"r"(center):"memory"
		);
		pos += 4;
#else
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
#endif
#endif

#if 0
		len   = work->intense * (rnd()*0.9f + 0.1f);
		upper = len *  rnd() * 0.1f;
		width = len * 0.12f;
		fvcalc[0].vx=0.0f;
		fvcalc[0].vy=upper;
		fvcalc[0].vz=len;

		fvcalc[1].vx= width;
		fvcalc[1].vy=upper*0.5f;
		fvcalc[1].vz=len*0.75f;

		fvcalc[2].vx=-width;
		fvcalc[2].vy=upper*0.5f;
		fvcalc[2].vz=len*0.75f;

		fvcalc[3].vx=0.0f;
		fvcalc[3].vy=upper*0.0625f;
		fvcalc[3].vz=len*0.50f;
#else
		len   = work->intense * (rnd()*0.9f + 0.1f);
		scw_fvtemp0->vx = len;
		scw_fvtemp0->vy = len * 0.75f;
		scw_fvtemp0->vz = len * 0.12f;
		scw_fvtemp0->vw = len * 0.50f;
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf12,0x00(%1)

			vmove.xy	vf8 ,vf0
			vaddx.z		vf8 ,vf0 ,vf12

			vaddz.x		vf9 ,vf0 ,vf12
			vmove.y		vf9 ,vf0
			vaddy.z		vf9 ,vf0 ,vf12

			vsubz.x		vf10,vf0,vf12
			vmove.y		vf10,vf0
			vaddy.z		vf10,vf0,vf12

			vmove.xy	vf11,vf0
			vaddw.z		vf11,vf0,vf12

			sqc2		vf8 ,0x00(%0)
			sqc2		vf9 ,0x10(%0)
			sqc2		vf10,0x20(%0)
			sqc2		vf11,0x30(%0)
			"::"r"(scw_fvcalc0),"r"(scw_fvtemp0):"memory"
		);
#else
			scw_fvcalc0[0].vx = 0.0f ;
			scw_fvcalc0[0].vy = 0.0f ;
			scw_fvcalc0[0].vz = scw_fvtemp0->vx ;

			scw_fvcalc0[1].vx = scw_fvtemp0->vz ;
			scw_fvcalc0[1].vy = 0.0f ;
			scw_fvcalc0[1].vz = scw_fvtemp0->vy ;

			scw_fvcalc0[2].vx = 0.0f - scw_fvtemp0->vz ;
			scw_fvcalc0[2].vy = 0.0f ;
			scw_fvcalc0[2].vz = scw_fvtemp0->vy ;

			scw_fvcalc0[3].vx = 0.0f;
			scw_fvcalc0[3].vy = 0.0f;
			scw_fvcalc0[3].vz = scw_fvtemp0->vw ;
#endif
#endif

		rot.vz = irnd()%4096;
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( scw_fvcalc0, vec, 4 );
		(vec++)->vw = P_GRAVITY;	/* 後で使用 */
		(vec++)->vw = P_GRAVITY;	/* 後で使用 */
		(vec++)->vw = P_GRAVITY;	/* 後で使用 */
		(vec++)->vw = P_GRAVITY;	/* 後で使用 */
//printf("%f %f %f \n",vec->vx,vec->vy,vec->vz);
	}

}


/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work )
{
	int	j;
	FVECTOR	*pos;
	FVECTOR	*vec;
	DG_PRIM2_UVRGB	*uvrgb ;
	u_char	col;
	int	clock;


	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	col=(u_char)( work->life * MAX_ALPHA/LIFE_TIME );
	OK_Mem_Scr( SCR_POS, work->pos,                sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Mem_Scr( SCR_VEC, work->vec,                sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Mem_Scr( SCR_UVS, work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	pos   = SCR_POS;
	vec   = SCR_VEC;
	uvrgb = SCR_UVS;
	for( j=0; j<N_POLYS; j++ ){
		/* α */
		uvrgb->a = col;	uvrgb++;
		uvrgb->a = col;	uvrgb++;
		uvrgb->a = col;	uvrgb++;
		uvrgb->a = col;	uvrgb++;

#if 0
		/* 座標更新 */
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += P_GRAVITY;
		pos++;
		vec++;
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += P_GRAVITY;
		pos++;
		vec++;
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += P_GRAVITY;
		pos++;
		vec++;
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += P_GRAVITY;
		pos++;
		vec++;
#else
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2		vf8 ,0x00(%0)
			lqc2		vf9 ,0x10(%0)
			lqc2		vf10,0x20(%0)
			lqc2		vf11,0x30(%0)
			lqc2		vf12,0x00(%1)
			lqc2		vf13,0x10(%1)
			lqc2		vf14,0x20(%1)
			lqc2		vf15,0x30(%1)

			vadd.xyz	vf8, vf8, vf12
			vadd.xyz	vf9, vf9, vf13
			vadd.xyz	vf10,vf10,vf14
			vadd.xyz	vf11,vf11,vf15

			vaddw.y		vf12,vf12,vf12
			vaddw.y		vf13,vf13,vf13
			vaddw.y		vf14,vf14,vf14
			vaddw.y		vf15,vf15,vf15

			sqc2		vf8 ,0x00(%0)
			sqc2		vf9 ,0x10(%0)
			sqc2		vf10,0x20(%0)
			sqc2		vf11,0x30(%0)
			sqc2		vf12,0x00(%1)
			sqc2		vf13,0x10(%1)
			sqc2		vf14,0x20(%1)
			sqc2		vf15,0x30(%1)
			"::"r"(pos),"r"(vec):"memory"
		);
#else
		{
			_sceVu0AddVector(&pos[0],&pos[0],&vec[0]) ;
			_sceVu0AddVector(&pos[1],&pos[1],&vec[1]) ;
			_sceVu0AddVector(&pos[2],&pos[2],&vec[2]) ;
			_sceVu0AddVector(&pos[3],&pos[3],&vec[3]) ;
			vec[0].vy += vec[0].vw ;
			vec[1].vy += vec[1].vw ;
			vec[2].vy += vec[2].vw ;
			vec[3].vy += vec[3].vw ;
		}
#endif
		pos += 4;
		vec += 4;
#endif

	}
	OK_Scr_Mem( work->pos,                SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( work->vec,                SCR_VEC, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( work->prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( work->prim->uvrgb[clock], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	if( work->life > 0 ){
		GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
		UpdateVectors( work ) ;
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

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
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

static int GetResources( Work *work, FVECTOR *center, SVECTOR *rot, float intense )
{
	DG_PRIM2		*prim ;
	DG_TEX		*tex ;

//AN_Test_Eye2( center, 2 );

	work->life = LIFE_TIME;

	DG_COPY_VEC( &work->center, center );
	work->rot.vx = rot->vx;
	work->rot.vy = rot->vy;
	work->rot.vz = rot->vz;
	work->intense = intense;

	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	InitVectors( work );
	DG_VisiblePrim2( work->prim ) ;

	return 0 ;
}

/*
FVECTOR *center:発生場所
SVECTOR *rot   :発生方向
float intense  :強さ（大きさ）
*/
void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense )
{
	Work		*work ;
#ifndef PSX2
	scw_fvtemp0	= SCR_W0 ;
	scw_fvcalc0	= SCR_W1 ;
#endif
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, center, rot, intense ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
