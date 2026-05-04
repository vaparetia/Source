//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shield_smoke.c
	煙突の煙

	2000/01/13 H.TANAKA
	2000/10/18 S.Okajima
	$Id: shield_smoke.c,v 1.1.1.3 2002/11/19 11:47:44 Yoshizawa1 Exp $

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

/* ------------------------------------------------------------- */
#define  FTOI12(_f)   ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ------------------------------------------------------------- */
#define	 SCR_LENGTH		( 0x4000 )

#define   N_PRIMS	1
#define   N_VERTS	5
#define   PRIM_TYPE   (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

#define   MAX_COUNT  60
#define	MEM_ADDR1   ((void *)( SCRPAD_ADDR + 0x0000 ))
#define	MEM_ADDR2   ((void *)( SCRPAD_ADDR + 0x0800 ))
#define	MAX_PATERN  30

typedef   struct
{
	GV_ACT_EX	actor ;
 
	DG_TEX	*tex ;
	DG_PRIM2   *prim ;

	FMATRIX   world; 
	int	   count ;

	int	   patern[N_PRIMS * N_VERTS] ;
	float	 pic_scale_u,pic_scale_v ;
	float	 scale_u,scale_v ;
} Work ;

/* ------------------------------------------------------------------- */
				  /* 
					  プログラム使用サブルーチン
				  */
/* メインメモリからスクラッチパッドへの転送 */
extern void  _BigScrCopy( void *dst, void *src, int size, int num) ;

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;


/* ---------------------------------------------------------------------*/
static  void Act_Scr(Work *work, DG_TEX *tex)
{
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	int				*patern ;
	float			  offset_u,offset_v ;
	float				x,y ;
	int				i ;

	uvrgbwh = MEM_ADDR2 ;

	if(work->count > MAX_COUNT - 48)
	{
	for(i = 0; i < N_PRIMS * N_VERTS; i++)
	{
		if(uvrgbwh->a < 1)
		{
		uvrgbwh->a = 0 ;
		}
		else
		{
		uvrgbwh->a -= 1 ;
		}
		uvrgbwh->w += 10 ;
		uvrgbwh->h += 10 ;

		uvrgbwh ++ ;
	}
	}
	else if(work->count > 0)
	{
	for(i = 0; i < N_PRIMS * N_VERTS; i++)
	{
		uvrgbwh->w += 40 ;
		uvrgbwh->h += 40 ;
		
		uvrgbwh ++ ;
	}
	}

	/* パターンチェック */
	if((work->count % 3) == 0)
	{
	uvrgbwh = MEM_ADDR2 ;
	patern  = work->patern ;
	for(i = 0; i < N_PRIMS * N_VERTS; i++)
	{
		*patern = (*patern + 1) % 30 ;
		/* テクスチャ決定 */
		x = (float)(*patern % 8) ;
		y = (float)(*patern / 8) ;
		offset_u = tex->u_offset + x * (32.0F * work->pic_scale_u) ;
		offset_v = tex->v_offset + y * (64.0F * work->pic_scale_v) ;
		uvrgbwh->u0 = FTOI12(offset_u) ;
		uvrgbwh->v0 = FTOI12(offset_v) ;
		uvrgbwh->u1 = FTOI12( work->scale_u + offset_u ) ;
		uvrgbwh->v1 = FTOI12( work->scale_v + offset_v ) ;
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		patern ++ ;
		uvrgbwh ++ ;
	}
	}
}

static  void Act(Work *work)
{
	DG_PRIM2	 *prim ;

	if(work->count > MAX_COUNT)
	{
	GV_DestroyActor( work ) ;
	return ;
	}

	prim = work->prim ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	_BigScrCopy( MEM_ADDR2,prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH),N_PRIMS* N_VERTS) ;
	/* バッファ交代 */
	DG_SwitchBuffPrim2(work->prim) ;

	Act_Scr(work,work->tex) ;
	_BigMemCopy( prim->uvrgb[prim->buffer_clock],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS* N_VERTS ) ;

	work->count ++ ;
}

/* ---------------------------------------------------------------------*/
static  void Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/* ---------------------------------------------------------------------*/
static   void  InitScrPad(Work *work,DG_TEX *tex,FMATRIX *world)
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	int				*patern ;
	int				i ;
	float				x,y ;
	float			  offset_u,offset_v ;

	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	patern = work->patern ;

	for(i = 0; i < N_PRIMS* N_VERTS; i++)
	{
	/* 位置情報(数値は縦の位置) */
	pos->vx = 33.5F + rnd() * 7.5F ;
	pos->vy = -360.0F + rnd() * 720.0F ;
	pos->vz = -207.5f + rnd() * 420.0F ;
	pos->vw = 1.0F ;
	
	*patern = irnd() % MAX_PATERN ;

	/* uvrgwhを決定 */
	x = (float)(*patern % 8) ;
	y = (float)(*patern / 8) ;
	offset_u = tex->u_offset + x * (32.0F * work->pic_scale_u) ;
	offset_v = tex->v_offset + y * (64.0F * work->pic_scale_v) ;

	uvrgbwh->u0 = FTOI12(offset_u) ;
	uvrgbwh->v0 = FTOI12(offset_v) ;
	uvrgbwh->u1 = FTOI12( work->scale_u + offset_u ) ;
	uvrgbwh->v1 = FTOI12( work->scale_v + offset_v ) ;
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;

	/* WH値を整数なので注意 ! */
	uvrgbwh->w = uvrgbwh->h = 100 + irnd() % 100 ;
	uvrgbwh->r = 24 + irnd() % 4;
	uvrgbwh->g = 24 + irnd() % 4;
	uvrgbwh->b = 24 + irnd() % 4;
	uvrgbwh->a = 48;

	pos ++ ;
	uvrgbwh ++ ;
	patern ++ ;
	}

	pos = MEM_ADDR1 ;
	DG_SetPos(world) ;
	DG_PutVector(pos,pos,N_PRIMS* N_VERTS) ;
	
}

static   int   GetResources(Work *work,FMATRIX  *world)
{
	DG_PRIM2		 *prim ;
	DG_TEX		   *tex ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS);
	if(prim == NULL) return -1 ;

	tex = work->tex = DG_GetTexture( 16238439 /*"smoke_msk"*/) ;
	if(tex == NULL)
	return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;
	work->pic_scale_u = tex->u_scale / 255.0F ;
	work->pic_scale_v = tex->v_scale / 255.0F ;
	work->scale_u	 = work->pic_scale_u * 31.0F ;
	work->scale_v	 = work->pic_scale_v * 63.0F ;
	work->count = 0 ;

	/* スクラッチパット上で初期化したデータを実際のワークにコピーする */
	InitScrPad(work,tex,world) ;
	_BigMemCopy( prim->pos[0], MEM_ADDR1,sizeof(FVECTOR),N_PRIMS * N_VERTS ) ;
	_BigMemCopy( prim->uvrgb[0],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS * N_VERTS) ;
	_BigMemCopy( prim->pos[1], MEM_ADDR1,sizeof(FVECTOR),N_PRIMS * N_VERTS) ;
	_BigMemCopy( prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS * N_VERTS) ;

	return 0 ;
}


/* ---------------------------------------------------------------------*/
void *NewShield_Smoke(FMATRIX  *world)
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER,sizeof(Work)) ;
	if(work != NULL){
		work->world = *world ;
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		if(GetResources(work,world) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
