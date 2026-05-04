//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	単発銃のマズルフラッシュ 
	2000/1/11  H.Tanaka
	2000/10/18 S.Okajima
	$Id: mazzleflash_sub.c,v 1.1.1.3 2002/11/19 11:50:36 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include "libutl.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"


/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define   N_PRIMS	   5
#define   N_VERTS	   4
#define   N_TEX_DU	  2
#define   N_TEX_DV	  2
#define   N_TEX_PATERN  4
#define   PRIM_TYPE	 (DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF)
#define   MEM_POS	   ((void *)(SCRPAD_ADDR + 0x0000))
#define   MEM_UVRGBWH   ((void *)(SCRPAD_ADDR + 0x0500))
#define   MEM_GOMI	  ((void *)(SCRPAD_ADDR + 0x3000))

FVECTOR   big_submazzleflash[N_PRIMS] = {
	{ 0.0F, -40.0F, 0.0F, 1.0F },
	{ 0.0F, -90.0F, 0.0F, 1.0F },
	{ 0.0F, -150.0F, 0.0F, 1.0F },
	{ 0.0F, -210.0F, 0.0F, 1.0F }
} ;

typedef struct
{
	GV_ACT_EX	 actor ;

	DG_PRIM2   *prim ;
	int		count ;
} Work ;


extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;


static void  Act(Work *work)
{
// yano
//	DG_InvisiblePrim2Chanl( work->prim , 1 );

	if(work->count > 3)
	{
	DG_InvisiblePrim2(work->prim) ;
	GV_DestroyActor( work ) ;
	}
	else
	{
	work->count ++ ;
	}
}

static void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/*  -----------------------------------------------------------------  */
static void Init_ScrPad( FMATRIX  *world, DG_TEX *tex)
{
	SVECTOR	*ang ;
	FVECTOR	*pos ;
	int		i,j ;
	float	  r ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;

	ang = MEM_GOMI ;
	ang->vx = 800 ;
	ang->vy = irnd() % 100 ;
	ang->vz = 0 ;

	/* 位置決定 */
	pos = MEM_POS ;
	for( i = 0 ; i < N_PRIMS; i++)
	{
	r = rnd() * 10.0F ;
	for( j = 0; j < N_VERTS ; j++)
	{
		pos[j] = big_submazzleflash[j] ;
		pos[j].vy += r ; 
	}
	DG_SetPos2(&DG_ZeroVector,ang) ;
	ang->vy += 4096 / N_PRIMS ;
	DG_PutVector(pos,pos,N_VERTS) ;
	DG_SetPos(world) ;
	DG_PutVector(pos,pos,N_VERTS) ;

	pos += N_VERTS ;
	}

	/* uvrgbwh を決定 */
	uvrgbwh = MEM_UVRGBWH ;
	for(i = 0; i < N_PRIMS; i++)
	{
	float angle;
	/* サイズを先に決めておく 改造しました byこばよん
	uvrgbwh[0].w = uvrgbwh[0].h = 30 + irnd() % 10 ;
	uvrgbwh[1].w = uvrgbwh[1].h = 50 + irnd() % 10 ;
	uvrgbwh[2].w = uvrgbwh[2].h = 60 + irnd() % 10 ;
	uvrgbwh[3].w = uvrgbwh[3].h = 50 + irnd() % 10 ;
	*/
	for(j = 0; j < N_VERTS; j++)
	{
		angle = frnd() * PI;
		uvrgbwh->u0 = FTOI12(0.0F * tex->u_scale + tex->u_offset) ;
		uvrgbwh->v0 = FTOI12(0.0F * tex->v_scale + tex->v_offset) ;
		uvrgbwh->u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
		uvrgbwh->v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->r  = 46 + irnd() % 10 ;
		uvrgbwh->g  = 32 + irnd() % 10 ;
		uvrgbwh->b  = 16 + irnd() % 10 ;
		uvrgbwh->a  = 54 + irnd() % 10 - j * 15 ; // 54 -> 84 by kobayashi
		uvrgbwh->w = ( int )( ( rnd() * 70.f + 10.f ) * cosf( angle ) );
		uvrgbwh->h = ( int )( ( rnd() * 70.f + 10.f ) * sinf( angle ) );
		uvrgbwh++;
	}
	}
}

static  int  GetResources(Work *work, FMATRIX *world)
{
	DG_PRIM2		  *prim ;
	DG_TEX			*tex ;
 
	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL) return -1 ;

	tex = DG_GetTexture( GV_StrCode("blood_2bw_msk") ) ;
	if(tex == NULL)
	return -1 ;
	DG_ConfigPrim2Tex(prim, tex) ;
	DG_SetPrim2Alpha(prim,SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;

	Init_ScrPad(world,tex) ;

	/* メモリ転送 */
	_BigMemCopy( prim->pos[ 0 ], MEM_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS) ;
	_BigMemCopy( prim->uvrgb[ 0 ], MEM_UVRGBWH, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS ) ;
	_BigMemCopy( prim->pos[ 1 ], MEM_POS, sizeof(FVECTOR), N_PRIMS * N_VERTS) ;
	_BigMemCopy( prim->uvrgb[ 1 ], MEM_UVRGBWH, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS ) ;

	work->count = 0 ;

	return 0 ;
}

/*  -----------------------------------------------------------------  */
void *SubMazzleFlash(FMATRIX *world)
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;

}
