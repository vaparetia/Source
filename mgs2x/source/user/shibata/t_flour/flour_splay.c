//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

   メリケン粉の固まりの飛び散り
	2000/02/01 H.TANAKA
	2000/03/31 S.Okajima
	$Id: flour_splay.c,v 1.1.1.3 2002/11/19 11:48:51 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
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

/* ------------------------------------------------------------ */
#define	  FTOI12(_f)		 ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ------------------------------------------------------------- */
#define	  PRIM_TYPE	   ( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE)
#define	  N_PRIMS		(12)
#define	  N_VERTS		(4)
#define	  N_NUM			(N_PRIMS * N_VERTS)
#define	  MAX_ALPHA		(48)
#define	  MAX_RGB		(255)

#ifdef BP_PS2
#define	  MEM_POS		((void *)( SCRPAD_ADDR))
#define	  MEM_UVRGB		((void *)( MEM_POS + sizeof(FVECTOR) * N_NUM))
#define	  MEM_MOVE		((void *)(  MEM_UVRGB + sizeof(DG_PRIM2_UVRGB) * N_NUM))
#define	  MEM_TEMP		((void *)( MEM_MOVE + sizeof(FVECTOR) * N_NUM ))
#else
#define	  MEM_POS		((void *)( SCRPAD_ADDR))
#define	  MEM_UVRGB		((void *)( (char *)MEM_POS + sizeof(FVECTOR) * N_NUM))
#define	  MEM_MOVE		((void *)( (char *)MEM_UVRGB + sizeof(DG_PRIM2_UVRGB) * N_NUM))
#define	  MEM_TEMP		((void *)( (char *)MEM_MOVE + sizeof(FVECTOR) * N_NUM ))
#endif

#define	  MAX_COUNT		(30)
#define	  SCALE			(50)
#define	  GRAVITY		(-1.0F)

typedef   struct
{
	GV_ACT_EX	  actor ;

	DG_TEX	  *tex ;
	DG_PRIM2	*prim ;

	FMATRIX	 world ;
	FVECTOR	 move[N_NUM] ;
	int		 count ;
} Work ;

	
/* ----------------------------------------------------------------*/

extern  void  _BigScrCopy( void *dat, void *src, int size, int num ) ;
extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

/* ----------------------------------------------------------------*/
static  void  ActScrPAD(void)
{
	FVECTOR		  *pos ;
	FVECTOR		  *move ;
	int			  i,j ;
	
	pos = MEM_POS ;
	move  = MEM_MOVE ;

	for(i = 0; i < N_PRIMS; i++){
		for(j = 0; j < N_VERTS; j++){
			_sceVu0AddVector( pos,pos,move ) ;
			move->vy += GRAVITY ;
			pos ++ ;
			move ++ ;
		}
	}
}


static  void  Act(Work *work)
{
	DG_PRIM2   *prim ;
	u_short	color ;
	DG_PRIM2_UVRGB   *uvrgb ;
	int			  i,j ; 

	prim = work->prim ;
	if( work->count < 0){
		DG_InvisiblePrim2(prim) ;
		GV_DestroyActor( work ) ;
	}
	
	/*データ転送( データ保持 )*/
	_BigScrCopy(MEM_POS, prim->pos[prim->buffer_clock], sizeof(FVECTOR), N_NUM) ;
	_BigScrCopy(MEM_MOVE, work->move, sizeof(FVECTOR), N_NUM ) ;


	DG_SwitchBuffPrim2( prim ) ;

	/* データ構築 */
	ActScrPAD() ;
	_BigMemCopy(prim->pos[prim->buffer_clock],MEM_POS,sizeof(FVECTOR),N_NUM) ;
	_BigMemCopy(work->move,  MEM_MOVE,sizeof(FVECTOR),N_NUM) ;


	/* アルファ値のみ変更 */
	color = (u_short)( work->count * MAX_ALPHA / MAX_COUNT ) ;
	uvrgb = (DG_PRIM2_UVRGB *)prim->uvrgb[prim->buffer_clock] ;
 
	
	for(i = 0; i < N_PRIMS; i++){
		for(j = 0; j < N_VERTS; j++){
			uvrgb->a = color ; uvrgb ++ ;
		}
	}

	work->count --; 
}

static  void  Die(Work *work)
{
	if(work->prim != NULL) GM_FreePrim2(work->prim) ;
}

/* ----------------------------------------------------------------*/
static  void  InitScrPad(Work *work,DG_TEX *tex)
{
	FVECTOR		  *pos ;			  
	DG_PRIM2_UVRGB   *uvrgb ;
	FVECTOR		  *move ;		/* 移動 */

	int			  i,j ; 
	FVECTOR		  *tex_data ;	 
	FVECTOR		  *center ;	   /* 発生位置 */
	FVECTOR		  *local_move ;   /* N_VERTS 個 保持する */ 
	float			speed ;		  /* 移動スピード */
	SVECTOR		  rot ;

	/* scrpadの格納場所決定 */
	pos = MEM_POS ;
	uvrgb = MEM_UVRGB ;
	move = MEM_MOVE ;

	/* ゴミデータを保持 */
	local_move = MEM_TEMP ;
#ifdef BP_PS2
	tex_data = MEM_TEMP + sizeof(FVECTOR) * N_VERTS ;
#else
	tex_data = (void *)((char *)MEM_TEMP + sizeof(FVECTOR) * N_VERTS) ;
#endif
	
   //BP - This line in invalidated 5 lines below.  Also, it makes no sense.
   //center = tex_data + sizeof(FVECTOR) ;
	
	/* テクスチャデータを前もって計算する( 高速化 ) */
	tex_data->vx = tex->u_offset ;
	tex_data->vy = tex->v_offset ;
	tex_data->vz = 1.0F * tex->u_scale + tex->u_offset ;
	tex_data->vw = 1.0F * tex->v_scale + tex->v_offset ;
	/* 中心位置 */
	center = (FVECTOR *)(work->world.m[3]) ;
	center->vy += 20.0F ;  /* ちょっと水増し */
	for( i = 0; i < N_PRIMS; i++)
	{
	/* 位置データ */
	for(j = 0; j < N_VERTS ; j++){
		DG_COPY_VEC(pos, center) ;
		pos ++ ;
	}
	/* スピードデータ */
	/* 基本位置をセット */
	speed = SCALE * (rnd() * 2.0F + 0.2F) ;

	local_move = MEM_TEMP ;
	local_move->vx = 0.0F ;
	local_move->vy = 0.0F ;
	local_move->vz = speed ;
	local_move ++ ;

	local_move->vx= SCALE * (rnd()/4.0f + 0.2f);
	local_move->vy= 0.0F;
	local_move->vz= speed / 2.0f;
	local_move ++ ;

	local_move->vx = -SCALE * (rnd()/4.0f + 0.2f);;
	local_move->vy = 0.0f;
	local_move->vz = speed/2.0f;
	local_move ++ ;

	local_move->vx = 0.0F;
	local_move->vy = 0.0F;
	local_move->vz = speed/8.0f;
	local_move ++ ;
	
	local_move = MEM_TEMP ;
	DG_SetPos(&work->world) ;
   rot.vx = - (640 + irnd() % 768) ;
	rot.vy = irnd() % 4096 ;
	rot.vz = 0 ;
	DG_RotatePos(&rot) ;
	DG_RotVector( local_move,  move, N_VERTS ) ;
	move += N_VERTS ;
	
	/* テクスチャデータ */
	uvrgb->u = FTOI12( tex_data->vx ) ;
	uvrgb->v = FTOI12( tex_data->vy ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x8fff ;
	uvrgb->r = MAX_RGB ;
	uvrgb->g = MAX_RGB ;
	uvrgb->b = MAX_RGB ;
	uvrgb->a = MAX_ALPHA ;
	uvrgb++;
	
	uvrgb->u = FTOI12( tex_data->vz ) ;
	uvrgb->v = FTOI12( tex_data->vy ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x8fff ;
	uvrgb->r = MAX_RGB ;
	uvrgb->g = MAX_RGB ;
	uvrgb->b = MAX_RGB ;
	uvrgb->a = MAX_ALPHA ;
	uvrgb++;

	uvrgb->u = FTOI12( tex_data->vx ) ;
	uvrgb->v = FTOI12( tex_data->vw ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x0fff ;
	uvrgb->r = MAX_RGB ;
	uvrgb->g = MAX_RGB ;
	uvrgb->b = MAX_RGB ;
	uvrgb->a = MAX_ALPHA ;
	uvrgb++;
	
	uvrgb->u = FTOI12( tex_data->vz ) ;
	uvrgb->v = FTOI12( tex_data->vw ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x0fff ;
	uvrgb->r = MAX_RGB ;
	uvrgb->g = MAX_RGB ;
	uvrgb->b = MAX_RGB ;
	uvrgb->a = MAX_ALPHA ;
	uvrgb++;
	}
}

static  int  GetResources(Work *work, FMATRIX *world)
{
	DG_PRIM2	 *prim ;
	DG_TEX	   *tex  ;

	/* プリミティブ */
	prim = work->prim = GM_MakePrim2( PRIM_TYPE, N_PRIMS, N_VERTS );
	if(prim == NULL){
		printf("null prim\n") ;
		return  -1 ;
	}

	/* テクスチャ */
	//tex = DG_GetTexture(GV_StrCode("powder02_alp")) ;
	tex = DG_GetTexture(9998494) ;

	if(tex == NULL)
	{
	printf(" not texture int flour_splay.c \n") ;
	return -1 ;
	}
	DG_ConfigPrim2Tex(prim, tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 )) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;

	
	work->world = *world ;
	work->count = MAX_COUNT ;
	InitScrPad(work, tex) ;

	_BigMemCopy(prim->pos[0],MEM_POS,sizeof(FVECTOR),N_NUM) ;
	_BigMemCopy(prim->uvrgb[0],MEM_UVRGB,sizeof(DG_PRIM2_UVRGB),N_NUM) ;
	_BigMemCopy(prim->pos[1],MEM_POS,sizeof(FVECTOR),N_NUM) ;
	_BigMemCopy(prim->uvrgb[1],MEM_UVRGB,sizeof(DG_PRIM2_UVRGB),N_NUM) ;
	_BigMemCopy(work->move,MEM_MOVE,sizeof(FVECTOR),N_NUM) ;
	
	return 0 ;
}

/* キャラ起動 */
void  *NewFlour_Splay(FMATRIX  *world)
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER,sizeof(Work)) ;
	if(work != NULL){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources(work, world) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
