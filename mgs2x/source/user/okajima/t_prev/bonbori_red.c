//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bonbori_l1.c
	ボンボリ赤色表示プログラム 

	2000/01/20/  田中 秀幸
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
#define	  PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	  MEM_POS	  ((void *)(SCRPAD_ADDR + 0x0000))   /* ～0x0360 */
#define	  MEM_UVRGBWH  ((void *)(SCRPAD_ADDR + 0x1000))   /* ～0x1720 */
#define	  MAX_ALPHA	120
#define	WIDE		   10000.0F
#define	HEIGHT		 2000
#define	BORDER		 150000.0F
#define	MAX_LEN		220000.0F
#define	FIRST_COUNT	50
#define	SECOND_COUNT   100
#define	THIRD_COUNT	200
#define	MAX_COUNT	  300  

/* ------------------------------------------------------------------------*/
/* マンハッタンの1列目 */
#if 0
static FVECTOR  big_bonbori_l1_base[] = {
	{-130.0F,1654.0F,828.0F,1.0F},{-130.0F,1654.0F,402.0F,1.0F},
	{-224.0F,2028.0F,4938.0F,1.0F},  {-224.0F,2028.0F,4108.0F,1.0F},
	{-770.0F,1654.0F,-4122.0F,1.0F}, {-770.0F,1654.0F,-4548.0F,1.0F}
} ;

/* マンハッタンの2列目 */
FVECTOR  big_bonbori_l2_base[] = {
	{10.0F,2900.0F,-3900.0F,1.0F},{10.0F,2900.0F,-4600.0F,1.0F},
	{10.0F,2600.0F,2820.0F,1.0F},  {10.0F,2600.0F,3470.0F,1.0F},
	{10.0F,2350.0F,2000.0F,1.0F},  {10.0F,2350.0F,880.0F,1.0F},
	{10.0F,2230.0F,100.0F,1.0F},  {10.0F,2230.0F,-730.0F,1.0F}
} ;
#else
/* マンハッタンの1列目 */
static FVECTOR  big_bonbori_l1_base[] = {
	{-430.0F,1654.0F,828.0F,1.0F},{-430.0F,1654.0F,402.0F,1.0F},
	{-520.0F,2028.0F,4938.0F,1.0F},  {-520.0F,2028.0F,4108.0F,1.0F},
	{-1070.0F,1654.0F,-4122.0F,1.0F}, {-1070.0F,1654.0F,-4548.0F,1.0F}
} ;

/* マンハッタンの2列目 */
FVECTOR  big_bonbori_l2_base[] = {
	{10.0F,2900.0F,-3900.0F,1.0F},{10.0F,2900.0F,-4600.0F,1.0F},
	{10.0F,2600.0F,2820.0F,1.0F},  {10.0F,2600.0F,3470.0F,1.0F},
	{10.0F,2350.0F,2000.0F,1.0F},  {10.0F,2350.0F,880.0F,1.0F},
	{10.0F,2230.0F,100.0F,1.0F},  {10.0F,2230.0F,-730.0F,1.0F}
} ;
#endif


/* ブルックリンの2列目 */
FVECTOR  big_bonbori_r2_base[] = {
	{-10.0F,1110.0F,-3260.0F,1.0F},{-10.0F,1110.0F,-4090.0F,1.0F},
	{-10.0F,1180.0F,-1730.0F,1.0F},{-10.0F,1180.0F,-880.0F,1.0F},
	{-10.0F,1180.0F,2330.0F,1.0F}, {-10.0F,1180.0F,3300.0F,1.0F},
} ;

typedef struct
{
	GV_ACT_EX	 actor ;
	DG_PRIM2   *prim ;
	
	int		n_prims ;
	int		n_verts ;
	int		count[0] ;
} Work ;

Work  *BIG_BONBORI_L1_WORK = NULL ;
Work  *BIG_BONBORI_L2_WORK = NULL ;
Work  *BIG_BONBORI_R2_WORK = NULL ;

extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;


/* ------------------------------------------------------------------------*/
static  void CalcAlpha_Size(
DG_PRIM2_UVRGBWH *uvrgbwh,
FVECTOR *cam, 
int *count,
int n_prims,
int n_verts
 )
{
	float			 range ;
	float			 len ;
	int			   i,j ;
	FVECTOR		   tmp ;
	FVECTOR		   *pos ;
	float			 alpha ;

	range = MAX_LEN - BORDER ;
	pos   = MEM_POS ;
	for(i = 0; i < n_prims; i++)
	{
	for(j = 0; j < n_verts; j++)
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

		/* 最大値を決定 */
		if(*count < FIRST_COUNT)
		{
		alpha = alpha * (float)(*count) / 50.0F ;
		}
		else if(*count < SECOND_COUNT)
		{
		}
		else if(*count < THIRD_COUNT)
		{
		alpha = alpha * (float)(THIRD_COUNT - *count) / 100.0F ;
		}
		else
		{
		alpha = 0.0F ;
		}
		uvrgbwh->a = (u_short)alpha ;

		pos ++ ;
		count ++ ;
		uvrgbwh ++ ;
	}
	}

}

static void PutLight(Work *work, FMATRIX *world, FVECTOR *cam, FVECTOR *shift)
{
	int			   i ;
	float			 scale ;
	float			 center ;
	float			 tmp ;
	FVECTOR		   *pos ;
	DG_PRIM2		  *prim ;
	int			   n_prims,n_verts,num ;
  
   
	n_prims = work->n_prims ;
	n_verts = work->n_verts ;
	num = n_prims * n_verts ;
	/* 位置決定 */
	scale = world->m[0][0] ;
	center = world->m[3][2] ;
	pos	= MEM_POS ;
	tmp = world->m[3][2] ;
	for(i = 0; i < n_prims; i++)
	{
	world->m[3][2] = center + scale * WIDE * (i - 3) ;
	/* 位置決め */
	DG_SetPos(world) ;
	DG_PutVector(shift,pos,n_verts) ;
	pos += n_verts ;
	}
	world->m[3][2] = tmp ;
	
	prim = work->prim ;
	
	/* 切り替え */
	DG_SwitchBuffPrim2(prim );

	CalcAlpha_Size(prim->uvrgb[prim->buffer_clock],cam,work->count,n_prims,n_verts) ;
	/*ポジションを転送 */
	_BigMemCopy(prim->pos[prim->buffer_clock],MEM_POS,sizeof(FVECTOR), num) ;
	
}

/* ---------------------------------------------------------------- */
void Big_PutManHatLight_L1(FMATRIX *world, FVECTOR *cam)
{
	Work  *work ;
	FVECTOR *shift ;

	if(BIG_BONBORI_L1_WORK == NULL) return ;

	work = BIG_BONBORI_L1_WORK ;
	shift = big_bonbori_l1_base ;
	PutLight(work, world, cam, shift) ;
}

void Big_PutManHatLight_L2(FMATRIX *world, FVECTOR *cam)
{
	Work  *work ;
	FVECTOR *shift ;

	if(BIG_BONBORI_L2_WORK == NULL) return ;

	work = BIG_BONBORI_L2_WORK ;
	shift = big_bonbori_l2_base ;
	PutLight(work, world, cam, shift) ;
}

void Big_PutManHatLight_R2(FMATRIX *world, FVECTOR *cam)
{
	Work  *work ;
	FVECTOR *shift ;

	if(BIG_BONBORI_R2_WORK == NULL) return ;

	work = BIG_BONBORI_R2_WORK ;
	shift = big_bonbori_r2_base ;
	PutLight(work, world, cam, shift) ;
}

/* ---------------------------------------------------------------- */
static  void	Act( Work *work)
{
	int   num ;
	int   *count ;
	int   i ;

	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	num = work->n_prims * work->n_verts ;
	count = work->count ;
	for(i = 0 ; i < num; i++){
		*count += 1 ;
		if(*count > MAX_COUNT) *count = 0 ;
		count ++ ;
	}
}

static  void	Die( Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;

	if( BIG_BONBORI_L1_WORK == work  ) BIG_BONBORI_L1_WORK = NULL;
	if( BIG_BONBORI_L2_WORK == work  ) BIG_BONBORI_L2_WORK = NULL;
	if( BIG_BONBORI_R2_WORK == work  ) BIG_BONBORI_R2_WORK = NULL;
}

/* -------------------------------------------------------------------  */
/* データを初期化 */
static void InitScrPad(Work *work,DG_TEX *tex)
{
	FVECTOR			  *pos ;
	DG_PRIM2_UVRGBWH	 *uvrgbwh ;
	int				  i, j ;
	int				  *count ;
	int				  n_prims,n_verts ;

	n_prims = work->n_prims ;
	n_verts = work->n_verts ;
	pos	 = MEM_POS ;
	uvrgbwh = MEM_UVRGBWH ;
	/* ついでにカウントも構築 */
	count = work->count ;
	/* データ構築 */
	for(i = 0; i < n_prims; i++)
	{
	for(j = 0; j < n_verts; j++)
	{
		*pos = DG_ZeroVector ;
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

		/* カウントをセット */
		if(j % 2)
		count[j] = count[j - 1] ;
		else
		count[j] = irnd() % 400 ;
	}
	count += n_verts ;
	}
}

static   int  GetResources( Work *work,int n_prims, int n_verts)
{
	DG_TEX   *tex ;
	DG_PRIM2 *prim ;

	work->n_prims = n_prims ;
	work->n_verts = n_verts ;
	prim = work->prim = GM_MakePrim2(PRIM_TYPE,n_prims,n_verts) ;
	if(prim == NULL)
	return -1 ;
	
	tex = DG_GetTexture(3908076 /*"light08_msk"*/) ;
	if(tex == NULL)
	return -1 ;
	/* テクスチャセット */
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;
	
	/* すくらっちパットで初期化したデータを送る */
	InitScrPad(work,tex) ;
	_BigMemCopy(prim->pos[0],MEM_POS,sizeof(FVECTOR),n_prims * n_verts) ;
	_BigMemCopy(prim->uvrgb[0],MEM_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),n_prims * n_verts) ;
	_BigMemCopy(prim->pos[1],MEM_POS,sizeof(FVECTOR),n_prims * n_verts) ;
	_BigMemCopy(prim->uvrgb[1],MEM_UVRGBWH,sizeof(DG_PRIM2_UVRGBWH),n_prims * n_verts) ;

	return 0 ;
}

/* ---------------------------------------------------------------------- */
/* マンハッタン1列目 */
void *NewManhatLight_L1(void)
{
	Work		*work ;
	int		 data_size ;
	int		 n_prims,n_verts ;

	n_prims = 6 ;
	n_verts = 6 ;
	data_size = sizeof(Work) + sizeof(int) * n_prims * n_verts ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, data_size ) ;
	if ( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work,n_prims,n_verts ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		BIG_BONBORI_L1_WORK = work ;
	}
	return (void *)work ;
}

/* マンハッタン2列目 */
void *NewManhatLight_L2(void)
{
	Work		*work ;
	int		 data_size ;
	int		 n_prims,n_verts ;
	
	n_prims = 6 ;
	n_verts = 8 ;
	data_size = sizeof(Work) + sizeof(int) * n_prims * n_verts ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, data_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work,n_prims,n_verts ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		BIG_BONBORI_L2_WORK = work ;
	}
	return (void *)work ;
}


void *NewBrooklyn_R2(void)
{
	Work		*work ;

	int		 data_size ;
	int		 n_prims,n_verts ;
	
	n_prims = 6 ;
	n_verts = 6 ;
	data_size = sizeof(Work) + sizeof(int) * n_prims * n_verts ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, data_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work,n_prims,n_verts ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		BIG_BONBORI_R2_WORK = work ;
	}
	return (void *)work ;
}
