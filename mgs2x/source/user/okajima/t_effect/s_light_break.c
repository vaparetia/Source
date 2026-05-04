//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	s_light_break.c
	ライト壊れ( 盾用 )   できれば、汎用性をもたせる 

	2000/01/13 H.TANAKA
	2000/10/18 S.Okajima
	$Id: s_light_break.c,v 1.1.1.3 2002/11/19 11:47:44 Yoshizawa1 Exp $

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
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

/* ----------------------------------------------------- */
#define	 N_PRIMS		 1
#define	 N_VERTS		 42		/* 3 * 10 */
#define	 N_TRIANGL_VERTS	   3
#define	 MAX_COUNT	 100
#define	 N_NUM	   ( N_PRIMS * N_VERTS / N_TRIANGL_VERTS )
#define	 PRIM_TYPE   (DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
 

typedef   struct
{
	GV_ACT_EX		  actor ;
	
	FMATRIX		 world ;
	DG_PRIM2		*prim ;
	FVECTOR		 base[N_NUM] ;
	FVECTOR		 speed[N_NUM] ;
	SVECTOR		 rot[N_NUM] ;
	SVECTOR		 const_rot[N_NUM] ;
	int			 count ;
	
} Work ;

static  FVECTOR  SL_Break_triangle[] = {
	{0.0F, 0.0F, 24.0F, 1.0F},
	{0.0F, 15.0F, -12.0F, 1.0F},
	{0.0F, -15.0F, -12.0F, 1.0F}
} ;

static  void Act(Work *work)
{
	DG_PRIM2	*prim ;
	FVECTOR	 *pos ;
	FVECTOR	 *speed ;
	FVECTOR	 *base ;
	SVECTOR	 *rot,*const_rot ;
	int		 i ;

	prim = work->prim ;
	if(work->count <= 0)
	{
	DG_InvisiblePrim2(prim) ;	
	return  ;
	}

	speed = work->speed ;
	base  = work->base ;
	rot   = work->rot ;
	const_rot = work->const_rot ;

	DG_SwitchBuffPrim2(prim) ;
		/* 位置決定 */
	pos = prim->pos[prim->buffer_clock] ;
	for(i = 0; i < N_NUM; i++)
	{
	/* 位置決め */
	_sceVu0AddVector(base,base,speed) ;
	rot->vz += const_rot->vz ;
	if(rot->vz >= 4096)
		rot->vz -= 4096 ;
	else if(rot->vz <= -4096)
		rot->vz += 4096 ;
		
	rot->vy += const_rot->vy ;
	if(rot->vy <= 4096)
		rot->vy -= 4096 ;
	else if(rot->vy <= -4096)
		rot->vy += 4096 ;
	
	DG_SetPos2(base,rot) ;
	DG_PutVector(SL_Break_triangle,pos,N_TRIANGL_VERTS) ;
	
	speed->vy -= 2.0F ;
	
	speed ++ ;
	base  ++ ;
	rot ++ ;
	const_rot ++ ;
	pos += N_TRIANGL_VERTS ;
	}
}

static   void Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/* ----------------------------------------------------------- */
static  void  Decide_Base(FVECTOR  *base, FVECTOR *speed, FMATRIX  *world)
{
	FVECTOR   *p_base, *p_speed ;
//	FVECTOR   center = {54.0F, -100.0F, 203.0F, 0.0F} ;
	FVECTOR   center = {54.0F, -100.0F, 203.0F, 0.0F} ;
	FVECTOR   fvtmp ;
	float	 range_vz, range_vy ;
	int	   i ;

	range_vz = 38.0F ;
	range_vy = 38.0F ;
	fvtmp.vx = center.vz + 24.0F ;
	fvtmp.vy = center.vy + 24.0F ;
	fvtmp.vz = center.vz - 24.0F ;
	fvtmp.vw = center.vy - 24.0F ;
	p_base = base ;
	p_speed = speed ;
	for(i = 0; i < N_NUM; i++)
	{
	base->vz = center.vz + range_vz * (2.0F * rnd() - 1.0F) ;
	base->vy = center.vy + range_vy * (2.0F * rnd() - 1.0F) ;
	base->vx = center.vx ;

	if(base->vz > fvtmp.vx)
	{
		speed->vz = rnd() * 10.0F ;
	}
	else if(base->vz < fvtmp.vz)
	{
		speed->vz = - rnd() * 10.0F ;
	}
	else
	{
		speed->vz = 0.0F ;
	}

	if(base->vy > fvtmp.vy)
	{
		speed->vy = rnd() * 10.0F ;
	}
	else if(base->vy < fvtmp.vw)
	{
		speed->vy = - rnd() * 5.0F ;
	}
	else
	{
		speed->vy = 0.0F ;
	}
	   
	speed->vx = 10.0F + rnd() * 10.0F ;

	speed ++ ;
	base ++ ;
	}

	base = p_base ;
	speed = p_speed ;
	DG_SetPos(world) ;
	DG_PutVector(base,base,N_NUM) ;
	DG_RotVector(speed,speed, N_NUM) ;   
}

static  void  Init_Glass(Work *work, DG_TEX *tex)
{
	DG_PRIM2		*prim ;
	FVECTOR		 *base ;
	FVECTOR		 *speed ;
	SVECTOR		 *rot,*const_rot ;	
	DG_PRIM2_UVRGB  *uvrgb_a, *uvrgb_b ;	 /* uvrgb */
	FVECTOR		 *pos_a, *pos_b ;		 /* 位置 */
	int			 alpha ;
	int				 i ;

	/* 基準点と速度を決定 */
	base = work->base ;
	speed = work->speed ;
	Decide_Base(base,speed,&work->world) ;
	
	/* 回転度 */
	prim = work->prim ;
	alpha = 36 + irnd() % 48 ;
	
	rot = work->rot ;
	const_rot = work->const_rot ;
	pos_a = prim->uvrgb[0] ;
	pos_b = prim->uvrgb[1] ;
	uvrgb_a = prim->uvrgb[0] ;
	uvrgb_b = prim->uvrgb[1] ;
	for(i = 0; i < N_NUM; i++)
	{
	rot->vz = irnd() % 512 ;
	if(rot->vz % 2)
		rot->vz *= -1 ;
	const_rot->vz = rot->vz ;
	rot->vy = irnd() % 512 ;
	if(rot->vy % 2)
		rot->vy *= -1 ;
	const_rot->vy = rot->vy ;
	rot->vx = irnd() % 4096 ;
	const_rot->vx = 0 ;

	DG_SetPos2(base,rot) ;
	DG_PutVector(SL_Break_triangle,pos_a,N_TRIANGL_VERTS) ;
 	*pos_b = *pos_a ; pos_b ++ ; pos_a ++ ;
 	*pos_b = *pos_a ; pos_b ++ ; pos_a ++ ;
 	*pos_b = *pos_a ; pos_b ++ ; pos_a ++ ;

	uvrgb_a->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb_a->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb_a->q = 4096 ;
	uvrgb_a->f = 0x8fff ;
	uvrgb_a->r = 128 ;
	uvrgb_a->g = 128 ;
	uvrgb_a->b = 128 ;
	uvrgb_a->a = alpha ;
	*uvrgb_b = *uvrgb_a ;
	uvrgb_a ++ ; uvrgb_b ++ ;
	uvrgb_a->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb_a->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb_a->q = 4096 ;
	uvrgb_a->f = 0x8fff ;
	uvrgb_a->r = 128 ;
	uvrgb_a->g = 128 ;
	uvrgb_a->b = 128 ;
	uvrgb_a->a = alpha ;
	*uvrgb_b = *uvrgb_a ;
	uvrgb_a ++ ; uvrgb_b ++ ;
	uvrgb_a->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb_a->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb_a->q = 4096 ;
	uvrgb_a->f = 0x0fff ;
	uvrgb_a->r = 128 ;
	uvrgb_a->g = 128 ;
	uvrgb_a->b = 128 ;
	uvrgb_a->a = alpha ;
	*uvrgb_b = *uvrgb_a ;
	uvrgb_a ++ ; uvrgb_b ++ ;
	
	base  ++ ;
	rot   ++ ;
	const_rot ++ ;
	}
}	

/* -------------------------------------------------------- */
static   int  GetResources(Work  *work, FMATRIX *world)
{
	/* work */
	DG_PRIM2  *prim ;
	DG_TEX	*tex ;

	work->world = *world ;
	prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS) ;
	if(prim == NULL)
	return 1 ;

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

	tex = DG_GetTexture(8617368 /*"col128_add"*/) ;
	if(tex == NULL)
	{
	printf("not texuter\n") ;
	return -1 ;
	}
	DG_ConfigPrim2Tex(prim,tex) ;

	Init_Glass(work,tex) ;
	
	work->count = MAX_COUNT ;

//	printf("ccccccc\n")  ;
	return 0 ;

}

void  *NewShieldFlashLight(FMATRIX  *world)
{
	Work   *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL ){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );

		if( GetResources( work, world ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;
}

