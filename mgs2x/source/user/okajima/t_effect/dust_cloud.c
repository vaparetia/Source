//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dust_cloud.c 
	土煙 
	2000/10/18 S.Okajima
	$Id: dust_cloud.c,v 1.1.1.3 2002/11/19 11:47:43 Yoshizawa1 Exp $

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

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/*------------------------------------------------------------------*/

#define      PRIM_TYPE  (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define      N_PRIMS     1
#define      N_VERTS     5
#define      MEM_ADDR1   ((void *)( SCRPAD_ADDR + 0x0000 ))
#define      MEM_ADDR2   ((void *)( SCRPAD_ADDR + 0x0800 ))

typedef   struct {
    GV_ACT_EX        actor ;
   
    DG_PRIM2      *prim ;
    int           count ;
} Work ;

extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
static   void  Act( Work *work )
{
    DG_PRIM2            *prim ;
    DG_PRIM2_UVRGBWH   *uvrgbwh,*pre_uvrgbwh ;
    int                i ;

    /* バッファ交代 */
    prim = work->prim ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
    pre_uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
    DG_SwitchBuffPrim2( work->prim );

    work->count ++ ;
    if(work->count > 20)
    {
	GV_DestroyActor( work ) ;
	return ;
    }
    else if( work->count > 3)
    {
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	for(i = 0; i < N_PRIMS * N_VERTS; i++)
	{
	    uvrgbwh->w =  10 + pre_uvrgbwh->w ;
	    uvrgbwh->h =  10 + pre_uvrgbwh->h ;
	    if(pre_uvrgbwh->a > 3)
		uvrgbwh->a =  pre_uvrgbwh->a - 3 ;
	    else
		uvrgbwh->a = 0 ;

	    uvrgbwh ++ ;
	    pre_uvrgbwh ++ ;
	}
	return ;
    }
    else if(work->count > 1)
    {
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	for(i = 0; i < N_PRIMS * N_VERTS; i++)
	{
	    uvrgbwh->w = 50 + pre_uvrgbwh->w;
	    uvrgbwh->h = 50 + pre_uvrgbwh->h;
	    uvrgbwh ++ ;
	    pre_uvrgbwh ++ ;
	}
    }
}

/*-------------------------------------------------------------------*/
static   void  Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim ) ;
}
/*-------------------------------------------------------------------*/
static     void  InitScrPad(FVECTOR *center,DG_TEX *tex)
{
    int               i;
    FVECTOR           *pos ;
    DG_PRIM2_UVRGBWH  *uvrgbwh ;

    pos     = MEM_ADDR1 ;
    uvrgbwh = MEM_ADDR2 ;

    pos->vx = center->vx ;
    pos->vy = center->vy ;
    pos->vz = center->vz ;
    pos ++ ;
    for(i = 1; i < N_PRIMS * N_VERTS; i++)
    {
	pos->vx = center->vx - 150.0F + rnd() * 300.0F ;
	pos->vy = center->vy - 150.0F + rnd() * 300.0F ;
	pos->vz = center->vz - 150.0F + rnd() * 300.0F ;
	pos->vw = 1.0F ;
	pos ++ ;

	uvrgbwh->u0 = FTOI12(0.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh->v0 = FTOI12(0.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh->u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh->v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;
	/* WH値は整数なので注意 ! */
	uvrgbwh->w = uvrgbwh->h = 100 + irnd() % 100 ;
#if 1
	uvrgbwh->r = 48 + irnd() % 4 ;
	uvrgbwh->g = 48 + irnd() % 4 ;
	uvrgbwh->b = 48 + irnd() % 4 ;
	uvrgbwh->a = 48 ;	
#else
	r = irnd() % 8 ;
	uvrgbwh->r = 10 + r ;
	uvrgbwh->g = 96 + r ;
	uvrgbwh->b = 96 + r ;
	uvrgbwh->a = 48 ;
#endif
	uvrgbwh ++ ;
    }
 
}

static     int   GetResources( Work *work,FVECTOR *center )
{
    DG_TEX            *tex ;
    DG_PRIM2          *prim ;

    /*  ４角形ポリゴン実験 */
    /* ポリゴン＆ラインの最大頂点数は６４まで！！ */
    prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS ); 
    if(prim == NULL)
	return -1 ;

    /* テクスチャを取得 */
#if 0
    tex = DG_GetTexture( 4445730 /*"blood_2_msk"*/ ) ;
    if(tex == NULL)
	return -1 ;
    DG_ConfigPrim2Tex(prim, tex) ;
    DG_SetPrim2Alpha(prim, tex) ;
//    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;
#else
    tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );

    if(tex == NULL)
	return -1 ;
    DG_ConfigPrim2Tex(prim, tex) ;
    DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;
#endif
    /* スクラッチパット上で初期化したデータを実際のワークにコピーする */
    InitScrPad(center,tex) ;
    
    _BigMemCopy( prim->pos[ 0 ], MEM_ADDR1, sizeof(FVECTOR), N_PRIMS * N_VERTS) ;
    _BigMemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS) ;
    _BigMemCopy( prim->pos[ 1 ], MEM_ADDR1, sizeof(FVECTOR), N_PRIMS * N_VERTS) ;
    _BigMemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS) ;

    work->count = 0 ;

    return 0 ;

}


void *NewDustCloud( FVECTOR  *center )
{
	Work         *work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work,center ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
