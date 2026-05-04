//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bonbori_yw.c
	ボンボリ黄色表示プログラム 

	2000/01/20  田中 秀幸 

*/


#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
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
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"def_dma.h"
#endif
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"

extern int ok_flush_status;

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */
#define	N_PRIMS	  6
#define	N_VERTS	  5
#define	PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	MEM_POS	  ((void *)(SCRPAD_ADDR + 0x0000))   /* ～0x0360 */
#define	MEM_UVRGBWH  ((void *)(SCRPAD_ADDR + 0x1000))   /* ～0x1720 */
#define	WIDE		10000.0F
#define	PRIM_TYPE   (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	MAX_ALPHA   64
#define	HEIGHT	  3000
#define	BORDER	  150000.0F
#define	MAX_LEN	 200000.0F   

FVECTOR  big_bonbori_yw_base[] = {
	{1400.0F,122.0F,-1898.0F,1.0F},{788.0F,122.0F,4477.0F,1.0F},
	{788.0F,122.0F,3889.0F,1.0F},  {1402.0F,122.0F,-715.0F,1.0F},
	{1686.0F,122.0F,-360.0F,1.0F}
} ;

typedef struct 
{
	GV_ACT_EX   actor ;

	DG_PRIM2 *prim ;
} Work ;

Work   *BIG_BONBORI_YW_WORK = NULL ;
extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

static  void CalcAlpha_Size(DG_PRIM2_UVRGBWH *uvrgbwh,FVECTOR *cam)
{
	float			 range ;
	float			 len ;
	int			   i,j ;
	FVECTOR		   tmp ;
	FVECTOR		   *pos ;
	float			 alpha ;

	range = MAX_LEN - BORDER ;
	pos   = MEM_POS ;
	for(i = 0; i < N_PRIMS; i++)
	{
	for(j = 0; j < N_VERTS; j++)
	{
		_sceVu0SubVector(&tmp,pos,cam) ;
		len = GV_VecLen3F(&tmp) ;
	
		/* フェードさせる */
		if(len >= MAX_LEN)
		alpha = 0.0F;
		else if(len > BORDER)
		alpha = (float)MAX_ALPHA * (MAX_LEN - len) / range ;
		else
		alpha = (float)MAX_ALPHA ;

		uvrgbwh->a = (u_short)alpha ;
		uvrgbwh->w = HEIGHT ;
		uvrgbwh->h = HEIGHT ;
		pos ++ ;
		uvrgbwh ++ ;
	}
	}
}

static void PutLight(Work *work, FMATRIX *world, FVECTOR *cam, FVECTOR *shift)
{
	int				i ;
	float			scale ;
	float			center ;
	float			tmp ;
	FVECTOR			*pos ;
	DG_PRIM2		*prim ;
	int				num ;

	num = N_PRIMS * N_VERTS ;
	/* 位置決定 */
	scale  = world->m[0][0] ;
	center = world->m[3][2] ;
	pos    = MEM_POS ;
	tmp    = world->m[3][2] ;
	for(i = 0; i < N_PRIMS; i++){
		world->m[3][2] = center + scale * WIDE * (i - 3) ;
		/* 位置決め */
		DG_SetPos(world) ;
		DG_PutVector(shift,pos,N_VERTS) ;
		pos += N_VERTS ;
	}

	world->m[3][2] = tmp ;
	prim = work->prim ;
	/* 切り替え */
	DG_SwitchBuffPrim2(prim );
	CalcAlpha_Size(prim->uvrgb[prim->buffer_clock],cam) ;
	/*ポジションを転送 */
	_BigMemCopy(prim->pos[prim->buffer_clock],MEM_POS,sizeof(FVECTOR), num) ;
}

void Big_PutManHatLight_Yw(FMATRIX *world, FVECTOR *cam)
{
	Work  *work ;
	FVECTOR *shift ;

	if(BIG_BONBORI_YW_WORK == NULL) return ;

	work = BIG_BONBORI_YW_WORK ;
	shift = big_bonbori_yw_base ;
	PutLight(work, world, cam, shift) ;

}

/*----------------------------------------------------------------------*/
static  void  Act(Work *work)
{
	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}
}

static  void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
	BIG_BONBORI_YW_WORK = NULL ;
}

/*----------------------------------------------------------------------*/
/* データを初期化 */
static void InitScrPad(Work *work,DG_TEX *tex)
{
	FVECTOR			  *pos ;
	DG_PRIM2_UVRGBWH	 *uvrgbwh ;
	int				  i, j ;

	pos	 = MEM_POS ;
	uvrgbwh = MEM_UVRGBWH ;
	/* ついでにカウントも構築 */
	/* データ構築 */
	for(i = 0; i < N_PRIMS; i++){
		for(j = 0; j < N_VERTS; j++){
//@			*pos = DG_ZeroVector ;
			pos->x = DG_ZeroVector.x ;
			pos->y = DG_ZeroVector.y ;
			pos->z = DG_ZeroVector.z ;
			pos->w = DG_ZeroVector.w ;

			pos ++ ;

			uvrgbwh->u0 = FTOI12(tex->u_offset) ;
			uvrgbwh->v0 = FTOI12(tex->v_offset) ;
			uvrgbwh->u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
			uvrgbwh->v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			/* WH値は整数なので注意 ! */
			uvrgbwh->w = HEIGHT ;
			uvrgbwh->h = HEIGHT ;
			uvrgbwh->r = 100 ;
			uvrgbwh->g = 100 ;
			uvrgbwh->b = 100 ;
			uvrgbwh->a = 0 ;
			uvrgbwh ++ ;
		}
	}
}


static   int   GetResources(Work *work)
{
	DG_TEX		 *tex ;
	DG_PRIM2		*prim ;
	int			 num ;

	num = N_PRIMS * N_VERTS ;
	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL)
	return -1 ;
	
	tex = DG_GetTexture(15442413 /*"light13_msk"*/) ;
	if(tex == NULL)
	return -1 ;
	/* テクスチャセット */
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
	
	/* すくらっちパットで初期化したデータを送る */
	InitScrPad(work,tex) ;
	_BigMemCopy(prim->pos[0],MEM_POS,sizeof(FVECTOR),num) ;
	_BigMemCopy(prim->uvrgb[0],MEM_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),num) ;
	_BigMemCopy(prim->pos[1],MEM_POS,sizeof(FVECTOR),num) ;
	_BigMemCopy(prim->uvrgb[1],MEM_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),num) ;	
	return 0 ;
}

/**************************************************************************/
void *NewManhatLight(void)
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		BIG_BONBORI_YW_WORK = work ;
	}
	return (void *)work ;
}
