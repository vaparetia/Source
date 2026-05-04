//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gll_achoo.c
	くしゃみ飛沫（実部）
	2000/03/31 S.Okajima
	をぱくって
	2002/08/16 Y.Korekado
	$Id: gll_achoo.c,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
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

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/*----------------------------------------------------------------*/

#define	SCR_LENGTH		( 0x4000 )

#define	N_VERTS		(16)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(1)

#define	BASE_RGB		(24)
#define	MAX_ALPHA		(64)

#define	LIFE_TIME		(16)

#if 1
#define	SCALE			(800.0f)
#define	WIDTH			(100.0f)
#define	UPPER			(100.0f)
#else
#define	SCALE			(80.0f)
#define	WIDTH			(10.0f)
#define	UPPER			(10.0f)
#endif

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	ACHUU_GRAVITY	(-0.5f)

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
	FMATRIX		*world;
	int			life;
	FVECTOR		pos[N_VERTS] ;
	FVECTOR		vec[N_VERTS] ;

} Work ;

/* Ｚ軸方向が０方向、Ｚからの方向を求める */
static void DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot )
{
	SVECTOR	temp_rot ;
	FVECTOR	vec ;
	float tmp ;

	_sceVu0SubVector( &vec, to, from ) ;
//	_sceVu0Normalize( &vec, &vec );

	tmp = atan2f( vec.vx, vec.vz ) ;
	rot->vy = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */
	temp_rot.vx = 0.0f;
	temp_rot.vy = -rot->vy;
	temp_rot.vz = 0.0f;

	DG_SetPos2( &DG_ZeroVector, &temp_rot );
	DG_PutVector( &vec, &vec, 1 );

	tmp = -atan2f( vec.vy, vec.vz ) ;
	rot->vx = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */

	rot->vz = 0 ;
}


/* メインメモリからスクラッチパッドへ転送 */
static void Mem_Scr( void *dst, void *src, int size, int num )
{
	UTL_StartMemToSpr( dst, src, size * num / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}

/* スクラッチパッドからメインメモリへ転送 */
static void Scr_Mem( void *dst, void *src, int size, int num )
{
	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}


/* 座標計算データを初期化する */
static	void	InitVectors( Work *work )
{
	int	i;
	float	len;
	float	upper;
	SVECTOR	rot;
	FVECTOR	*center;
	FVECTOR	*pos;
	FVECTOR	*vec;
	FVECTOR	fvcalc[4];
	FVECTOR	fvtemp;

	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = 1.0f;
	DG_SetPos( work->world );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	DirVecXY( &DG_ZeroVector, &fvtemp, &rot );

	center = (FVECTOR *)work->world->m[3];

	pos = work->pos;
	vec = work->vec;
	for( i=0; i<N_POLYS; i++ ){
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;
		DG_COPY_VEC( pos, center );	pos++;

		len   = SCALE * (rnd()*0.9f + 0.1f);
		upper = UPPER *  rnd();
		fvcalc[0].vx=0.0f;
		fvcalc[0].vy=upper;
		fvcalc[0].vz=len;

		fvcalc[1].vx= WIDTH;
		fvcalc[1].vy=upper*0.5f;
		fvcalc[1].vz=len*0.75f;

		fvcalc[2].vx=-WIDTH;
		fvcalc[2].vy=upper*0.5f;
		fvcalc[2].vz=len*0.75f;

		fvcalc[3].vx=0.0f;
		fvcalc[3].vy=upper*0.0625f;
		fvcalc[3].vz=len*0.50f;

		rot.vz = irnd()%4096;
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( fvcalc, vec, 4 );
//printf("%f %f %f \n",vec->vx,vec->vy,vec->vz);
		vec+=4;
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
	Mem_Scr( SCR_POS, work->pos,                sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Mem_Scr( SCR_VEC, work->vec,                sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Mem_Scr( SCR_UVS, work->prim->uvrgb[clock], sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
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
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += ACHUU_GRAVITY;
//AN_Test_Eye2(pos,2);
		pos++;
		vec++;
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += ACHUU_GRAVITY;
//AN_Test_Eye2(pos,2);
		pos++;
		vec++;
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += ACHUU_GRAVITY;
//AN_Test_Eye2(pos,2);
		pos++;
		vec++;
		pos->vx += vec->vx;
		pos->vy += vec->vy;
		pos->vz += vec->vz;
		vec->vy += ACHUU_GRAVITY;
//AN_Test_Eye2(pos,2);
		pos++;
		vec++;
	}
	Scr_Mem( work->pos,                SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Scr_Mem( work->vec,                SCR_VEC, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Scr_Mem( work->prim->pos[clock],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Scr_Mem( work->prim->uvrgb[clock], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	if( work->life > 0 ){
		UpdateVectors( work ) ;
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static DG_PRIM2 *FreePrim2( DG_PRIM2 *prim )
{
	if ( prim != NULL ) {
		DG_DequeuePrim2( prim ) ;
		DG_FreePrim2( prim ) ;
		prim = NULL;
	}
	return prim;
}
static void Die( Work *work )
{
	work->prim = FreePrim2( work->prim );
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
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

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

	Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

static int GetResources( Work *work, FMATRIX *world, int mode )
{
	DG_PRIM2		*prim ;
	DG_TEX		*tex ;

	if( world==NULL ) return -1;
	work->world = world;
	switch( mode ){
	  case 0:
		work->life = LIFE_TIME;
		break;
	  case 1:
		work->life = LIFE_TIME/2;
		break;
	}

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 13872561 /*"blood_1e_msk"*/ );
	tex = DG_GetTexture( 4587425 /*"splash03_alp"*/ );


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

void *NewGllAchoo( FMATRIX *world, int mode )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
