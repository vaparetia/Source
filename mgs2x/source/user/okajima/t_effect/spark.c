//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   spark.c  火花(汎用性)
   
   2000/01/03
   田中 秀幸 
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
/* ---------------------------------------------------------------- */

#define	SCR_LENGTH   ( 0x3000 )	/* 本来は 0x4000 */
#define N_VERTS	  2  
#define MEM_ADDR1	((void *)( SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2	((void *)( SCRPAD_ADDR + 0x2000))
//#define PRIM_TYPE	( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define PRIM_TYPE	( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA|DG_PRIM2_NOMSAA)

typedef struct
{
	GV_ACT_EX	 actor ;
	DG_PRIM2   *prim ;

	int			n_prims ;	/* 火花の数 */
	float		length ;	/* 長さの割合 */
	float		gravity ;	/* 重力 */
	int			count ;
	int			count_max ;

	FVECTOR	color ;			   /* 色 */
	FVECTOR	speed[0] ;			/* スピード */
} Work ;

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* メインメモリからスクラッチパッドへの転送 */
extern void  _BigScrCopy( void *dst, void *src, int size, int num) ;

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

/*-------------------------------------------------------------------*/

static   void  Act_Scr(Work *work)
{
	FVECTOR   *pos ;
	FVECTOR   *speed ;
	int	   n_prims ;
	int	   i ;
	float	 length ;

	n_prims = work->n_prims ;
	pos = MEM_ADDR1 ;
	speed = work->speed ;
	length = work->length * 2.0f;

	for(i = 0; i < n_prims ; i++){
		speed->vy -= work->gravity ;
		_sceVu0AddVector(&pos[0],&pos[0],speed) ;
		pos[1].vx = pos[0].vx - speed->vx * length;
		pos[1].vy = pos[0].vy - speed->vy * length;
		pos[1].vz = pos[0].vz - speed->vz * length;
		pos[0].vw = 1.0F ;
		pos[1].vw = 1.0F ;
		speed ++ ;
		pos   += 2 ;
	}
}

static   void  Act(Work *work)
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int				num ;
	int				count ;
	float			bright ;
	SVECTOR			color ;
	int				i ;
	
	if((count = -- work->count) <= 0){
		DG_InvisiblePrim2(work->prim) ;
		GV_DestroyActor(work) ;
		return ;
	}
	DG_VisiblePrim2(work->prim) ;

	prim = work->prim ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	num = work->n_prims * N_VERTS ;
	/* 位置とスピードを更新する */
	_BigScrCopy( MEM_ADDR1, prim->pos[prim->buffer_clock], sizeof(FVECTOR), num) ; 

	DG_SwitchBuffPrim2( prim ) ;

	Act_Scr(work) ;
	_BigMemCopy( prim->pos[prim->buffer_clock], MEM_ADDR1, sizeof(FVECTOR), num) ;

	bright = (float)count / (float)work->count_max;
	color.vx = (u_short)(work->color.vx * bright) ;
	color.vy = (u_short)(work->color.vy * bright) ;
	color.vz = (u_short)(work->color.vz * bright) ;

	uvrgb = prim->uvrgb[prim->buffer_clock] ;
	for(i = 0 ; i < work->n_prims; i++){
		/* 先頭のみ更新 */
		uvrgb->r = color.vx ;
		uvrgb->g = color.vy ;
		uvrgb->b = color.vz ;
		uvrgb += N_VERTS ;
	}
	
}

static   void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/*--------------------------------------------------------------------------*/
static void Init_Tex(Work *work, DG_TEX *tex)
{
	DG_PRIM2_UVRGB	*uvrgb ;
	int			   n_prims ;
	int			   i ;

	n_prims = work->n_prims ;
	uvrgb = MEM_ADDR1 ;

	for(i = 0; i < n_prims; i++){
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = (u_short)work->color.vx ;
		uvrgb->g = (u_short)work->color.vy ;
		uvrgb->b = (u_short)work->color.vz ;
		uvrgb->a = 128 ;/* アンチエイリアスを使用するときには１２８にする */

		uvrgb[1].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[1].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[1].q = 4096 ;
		uvrgb[1].f = 0x0fff ;
		uvrgb[1].r = 0 ;
		uvrgb[1].g = 0 ;
		uvrgb[1].b = 0 ;
		uvrgb[1].a = 128 ;/* アンチエイリアスを使用するときには１２８にする */
		uvrgb += N_VERTS ;
	}
}

static   int  GetResources( Work *work )
{
	DG_PRIM2	  *prim ;
	DG_TEX		*tex ;
	int		   n_prims ;
	int		   num ;

	n_prims = work->n_prims ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,n_prims,N_VERTS) ;
	if(prim == NULL)
	return -1 ;

	DG_InvisiblePrim2(work->prim) ;

	/* 位置と速度情報を移す */
	num = N_VERTS * n_prims ;
	_BigMemCopy( prim->pos[0], MEM_ADDR1, sizeof(FVECTOR), num) ;
	_BigMemCopy( prim->pos[1], MEM_ADDR1, sizeof(FVECTOR), num) ;
	_BigMemCopy( work->speed, MEM_ADDR2, sizeof(FVECTOR), n_prims) ;

	/* テクスチャ関係の初期化 */
	tex = DG_GetTexture( 8617368 /*"col128_add"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

	DG_ConfigPrim2Tex(prim, tex) ;
	DG_SetPrim2Alpha(prim,SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
	Init_Tex(work,tex) ;
	_BigMemCopy( prim->uvrgb[0],MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), num) ;
	_BigMemCopy( prim->uvrgb[1],MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), num) ;

	return 0;
}

/* --------------------------------------------------------*/

static  void Init_Scr1(
Work	*work ,
float   min_speed,
float   speed_wide,
FVECTOR *center,
SVECTOR *rot,
SVECTOR *rot_wide
)
{
	SVECTOR		 local_rot ;
	FVECTOR		 *pos ;
	FVECTOR		 *speed ;
	int			 i ;
	int			 n_prims ;
	float		   length ;

	pos = MEM_ADDR1 ;
	speed = MEM_ADDR2 ;

	n_prims = work->n_prims ;
	length  = work->length ;

	for(i = 0; i < n_prims; i++)
	{
// yano
/* 0除算回避のため-->なんでPS2ではOKだったのか不明 */
   //BP - code to avoid divide by zero
#if 0 //BP def PSX2
	local_rot.vx = rot->vx + irnd() % rot_wide->vx ;
	local_rot.vy = rot->vy + irnd() % rot_wide->vy ;
	local_rot.vz = rot->vz + irnd() % rot_wide->vz ;
#else
	if( rot_wide->vx == 0 ){
		rot_wide->vx = 1;
	}
	local_rot.vx = rot->vx + irnd() % rot_wide->vx ;
	if( rot_wide->vy == 0 ){
		rot_wide->vy = 1;
	}
	local_rot.vy = rot->vy + irnd() % rot_wide->vy ;
	if( rot_wide->vz == 0 ){
		rot_wide->vz = 1;
	}
	local_rot.vz = rot->vz + irnd() % rot_wide->vz ;
#endif
	DG_SetPos2(&DG_ZeroVector,&local_rot) ;
	speed->vx = 0.0F ;
	speed->vy = 0.0F ;
	speed->vz = min_speed + rnd() * speed_wide ;
	speed->vw = 1.0F ;
	DG_RotVector(speed,speed,1) ;
	
	/* 位置決定 */
	_sceVu0AddVector(&pos[0],speed,center) ;
	pos[0].vw = 1.0F ;
	pos[1].vx = pos[0].vx - speed->vx * length ;
	pos[1].vy = pos[0].vy - speed->vy * length ;
	pos[1].vz = pos[0].vz - speed->vz * length ;
	pos[1].vw = 1.0F ;
	
	pos += 2 ;
	speed ++ ;
	}
	
}

void *NewSpark1(
int		  n_prims,			 /* 火花の数  */
FVECTOR	  *center,				  /* 位置	  */
float		min_speed,			/* 初期最小スピード  */
float		speed_wide,			/* 幅スピード  */
float		gravity,			   /* 重力 */
SVECTOR	  *rot,				  /* 向き */  
SVECTOR	  *rot_wide,			 /* ワイド */
FVECTOR	  *color,				 /* 色 */
float		length,				 /* スピードに対する火の長さの割合 */
int		  count				  /* 表示時間 */ 
)
{
	Work *work ;
	int  data_size ;
	int  max_num ;

	data_size = sizeof(FVECTOR) * 3 ;

	max_num = SCR_LENGTH / data_size ;
	if(n_prims > max_num){
//		printf("max  %d\n",max_num) ;
		n_prims = max_num ;
	}

	data_size = sizeof(Work) + sizeof(FVECTOR) * n_prims ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, data_size ) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->n_prims = n_prims ;
		work->count_max= work->count    = count ;
		work->length   = length;
		work->color.vx = color->vx ;
		work->color.vy = color->vy ;
		work->color.vz = color->vz ;
		work->color.vw = color->vw ;
		work->gravity  = gravity ;
		Init_Scr1(work,min_speed,speed_wide,center,rot,rot_wide) ;
		/* ワークの情報を初期化 */	
		if(GetResources(work) < 0){
			printf("spark.c : NewSpark1 destroy\n") ;
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return work ;
}

/* ---------------------------------------------------------------- */

static  void Init_Scr2(
Work	*work ,
float   min_speed,
float   speed_wide,
FMATRIX *world,
SVECTOR *rot,
SVECTOR *rot_wide
)
{
	SVECTOR		 local_rot ;
	FVECTOR		 *pos ;
	FVECTOR		 *speed ;
	int			 i ;
	int			 n_prims ;
	float		   length ;

	pos = MEM_ADDR1 ;
	speed = MEM_ADDR2 ;

	n_prims = work->n_prims ;
	length  = work->length ;

	for(i = 0; i < n_prims; i++)
	{
// yano
/* 0除算回避のため-->なんでPS2ではOKだったのか不明 */
#if 0 //BP_PS2 def PSX2
	local_rot.vx = rot->vx + irnd() % rot_wide->vx ;
	local_rot.vy = rot->vy + irnd() % rot_wide->vy ;
	local_rot.vz = rot->vz + irnd() % rot_wide->vz ;
#else
	if( rot_wide->vx == 0 ){
		rot_wide->vx = 1;
	}
	local_rot.vx = rot->vx + irnd() % rot_wide->vx ;
	if( rot_wide->vy == 0 ){
		rot_wide->vy = 1;
	}
	local_rot.vy = rot->vy + irnd() % rot_wide->vy ;
	if( rot_wide->vz == 0 ){
//printf("VZ=0 \n");
		rot_wide->vz = 1;
	}
	local_rot.vz = rot->vz + irnd() % rot_wide->vz ;
#endif
	DG_SetPos(world) ;
	DG_RotatePos(&local_rot) ;
	speed->vx = 0.0F ;
	speed->vy = 0.0F ;
	speed->vz = min_speed + rnd() * speed_wide ;
	speed->vw = 1.0F ;
	DG_RotVector(speed,speed,1) ;
	
	/* 位置決定 */
	_sceVu0AddVector(&pos[0],speed,(FVECTOR *)world->m[3]) ;
	pos[0].vw = 1.0F ;
	pos[1].vx = pos[0].vx - speed->vx * length ;
	pos[1].vy = pos[0].vy - speed->vy * length ;
	pos[1].vz = pos[0].vz - speed->vz * length ;
	pos[1].vw = 1.0F ;
	
	pos += 2 ;
	speed ++ ;
	}
	
}

void *NewSpark2(
int		  n_prims,		 /* 火花の数 */
FMATRIX	  *world,		   /* マトリックス*/
float		min_speed,		/* 最小スピード */
float		speed_wide,		/* 幅スピード */
float		gravity,		   /* 重力 */
SVECTOR	  *rot,			  /* 回転 */
SVECTOR	  *rot_wide,		 /* 幅 */
FVECTOR	  *color,			/* 色 */ 
float		length,			/* スピードに対する火の長さの割合 */		
int		  count			  /* カウント */ 
)
{
	Work *work ;
	int  data_size ;
	int  max_num ;

	data_size = sizeof(FVECTOR) * 3 ;
	max_num = SCR_LENGTH / data_size ;
	if(n_prims > max_num){
//		printf("max  %d\n",max_num) ;
		n_prims = max_num ;
	}

	data_size = sizeof(Work) + sizeof(FVECTOR) * n_prims ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, data_size ) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->n_prims = n_prims ;
		work->count_max= work->count    = count ;
		work->length   = length ;
		work->color.vx = color->vx ;
		work->color.vy = color->vy ;
		work->color.vz = color->vz ;
		work->color.vw = color->vw ;
		work->gravity  = gravity ;
		Init_Scr2(work,min_speed,speed_wide,world,rot,rot_wide) ;
		/* ワークの情報を初期化 */	
		if(GetResources(work) < 0){
//			printf("spark.c : NewSpark2 destroy\n") ;
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return work ;
}





