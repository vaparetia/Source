//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breath.c
	息
	2000/10/18 S.Okajima
	$Id: breath.c,v 1.1.1.3 2002/11/19 11:47:43 Yoshizawa1 Exp $
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

#define	FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define	N_PRIMS	 5
#define	N_VERTS	 1
#define	PRIM_TYPE   (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	MAX_PATERN  30
#define	MEM_ADDR1   ((void *)( SCRPAD_ADDR + 0x0000 ))
#define	MEM_ADDR2   ((void *)( SCRPAD_ADDR + 0x0800 ))
#define	MEM_ADDR3   ((void *)( SCRPAD_ADDR + 0x1200 )) 
#define	WIDE		20
#define	MAX_PATERN  30

#define	COL_A  28

extern  FVECTOR G_wind ;
extern  int	 G_wind_intense;
extern  int	 G_wind_sw ;

typedef struct
{
	FVECTOR   speed ;
	int	   count ;
	int	   patern ;
	int	   pad0 ;
	int	   pad1 ;
} DATA ; 

typedef struct
{
	GV_ACT_EX	 actor ;
	DG_PRIM2   *prim ;

	DG_TEX	 *tex ;
	DATA	   data[N_PRIMS] ;
	FMATRIX	world ;
	FVECTOR	move ;
	float	  pic_scale_u,pic_scale_v ;
	float	  scale_u, scale_v ;
	int		all_count ;
	int		   *sw ;
} Work ;

/* ------------------------------------------------------------------- */
				  /* 
					  プログラム使用サブルーチン
				  */
/* メインメモリからスクラッチパッドへの転送 */
extern void  _BigScrCopy( void *dst, void *src, int size, int num) ;

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

/* ------------------------------------------------------------------- */
static  void Act_Scr( Work *work,DG_TEX *tex )
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	DATA			   *data ;
	int				i ;
	FVECTOR			wind ;
	int				step_color ;
	int				bright ;
	float			  offset_u,offset_v ;
	int				x,y ;
	FVECTOR			shift ;

	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	data	= MEM_ADDR3 ;
	for(i = 0; i < N_PRIMS; i++){
		if(data->count > 0){
			/* 位置決定 */
			_sceVu0AddVector(pos,pos,&data->speed) ;
			if(G_wind_intense > 40.0F){
				wind.vx = G_wind.vx / G_wind_intense * 2.0F ;
				wind.vy = G_wind.vy / G_wind_intense * 2.0F ;
				wind.vz = G_wind.vz / G_wind_intense * 2.0F ;
			}else{
				wind.vx = G_wind.vx / 20.0F ;
				wind.vy = G_wind.vy / 20.0F ;
				wind.vz = G_wind.vz / 20.0F ;
			}

			/* 速度決定 */
			_sceVu0AddVector(&data->speed,&data->speed,&wind) ;

			/* 幅、高さ決定 */
			uvrgbwh->w += 10.0F ;
			uvrgbwh->h += 10.0F ;

			/* 色決定 */
			step_color = G_wind_intense / 100 + 1;
			bright = (int)uvrgbwh->r - step_color ;
			if(bright < 0) bright = 0 ;
			uvrgbwh->r = (u_short)bright ;
			uvrgbwh->g = uvrgbwh->r ;
			uvrgbwh->b = uvrgbwh->r ;
			uvrgbwh->a = COL_A ;

			/* テクスチャ決定 */
			x = (float)(data->patern % 8) ;
			y = (float)(data->patern / 8) ;
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
		}else if(data->count == 0){
			/* 位置決定 */
			shift.vx = work->move.vx ;
			shift.vy = work->move.vy ;
			shift.vz = work->move.vz ;

			DG_SetPos(&work->world) ;
			DG_PutVector(&shift,pos,1) ;

			/* サイズ決定 */
			uvrgbwh->w = WIDE ;
			uvrgbwh->h = WIDE ;
			/* 色決定 */
			uvrgbwh->r = 32 ;
			uvrgbwh->g = 32 ;
			uvrgbwh->b = 32 ;
			uvrgbwh->a = COL_A ;
			/* テクスチャパターン */
			data->patern = irnd() % MAX_PATERN ;
			x = data->patern % 8 ;
			y = data->patern / 8 ;
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
	
			/* スピード決定 */
			DG_MovePos(&shift) ;
			shift.vy = 0.0F ;
			shift.vz = 10.0F ;
			DG_RotVector(&shift,&data->speed,1) ;
		}
		data->count ++ ;
		pos ++ ;
		uvrgbwh ++ ;
		data ++ ;
	}
 
}

/* 動作部分 */
static  void Act(Work *work)
{
	DG_PRIM2		   *prim ;

	if(work->all_count > 60)
	GV_DestroyActor( work ) ;

	prim = work->prim ;

	if ( *( work->sw ) == 0 ) DG_InvisiblePrim2( prim ) ;
	else					  DG_VisiblePrim2( prim ) ;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	_BigScrCopy( MEM_ADDR1,prim->pos[prim->buffer_clock],sizeof(FVECTOR),N_PRIMS ) ;
	_BigScrCopy( MEM_ADDR2,prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH),N_PRIMS) ;
	_BigScrCopy( MEM_ADDR3,work->data,sizeof(DATA),N_PRIMS) ;

	/* バッファ交代 */
	DG_SwitchBuffPrim2( work->prim ) ;

	Act_Scr(work,work->tex) ; 

	_BigMemCopy( prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),N_PRIMS ) ;
	_BigMemCopy( prim->uvrgb[prim->buffer_clock],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS ) ;
	_BigMemCopy( work->data,MEM_ADDR3,sizeof(DATA),N_PRIMS) ;

	work->all_count ++ ;
}

static  void Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/* ------------------------------------------------------------------ */
static void InitScrPad(Work *work,DG_TEX *tex)
{
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	DATA			  *data ;
	int			   i ;
	int			   x,y ;
	float			 offset_u,offset_v ;

	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	data  = MEM_ADDR3 ;
	for(i = 0; i < N_PRIMS; i++){
		/* 仮ポジション決定 */
		pos->vx = 0.0F ;
		pos->vy = 0.0F ;
		pos->vz = 0.0F ;
		pos->vw = 1.0F ;

		/* データを決定 */
		data->speed.vx = 0.0F ;
		data->speed.vy = 0.0F ;
		data->speed.vz = 0.0F ;
		data->speed.vw = 1.0F ;
		data->count	= - i * 5 ;
		data->patern   = irnd() % MAX_PATERN ;

		/* uvrgbwhを決定 */
		x = data->patern % 8 ;
		y = data->patern / 8 ;
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

		/* WH値は整数なので注意 ! */
		uvrgbwh->w = 0 ;
		uvrgbwh->h = 0 ;
		uvrgbwh->r = 0 ;
		uvrgbwh->g = 0 ;
		uvrgbwh->b = 0 ;
		uvrgbwh->a = 0 ;

		pos ++ ;
		uvrgbwh ++ ;
		data ++ ;
	}
}

/* 初期化部分 */
static  int  GetResources(Work  *work)
{
	DG_PRIM2		*prim ;
	DG_TEX		  *tex ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL)
	return -1 ;
	
	tex = work->tex = DG_GetTexture( 16238439 /*"smoke_msk"*/) ;
	if(tex == NULL)
	return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim,SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;

	/* スクラッチパット上で初期化したデータを実際のワークにコピーする */
	work->pic_scale_u = tex->u_scale / 255.0F ;
	work->pic_scale_v = tex->v_scale / 255.0F ;
	work->scale_u	 = work->pic_scale_u * 31.0F ;
	work->scale_v	 = work->pic_scale_v * 63.0F ;
	work->all_count   = 0 ;

	InitScrPad(work,tex) ;

	/* メモリーを転送する */
	_BigMemCopy( prim->pos[0], MEM_ADDR1,sizeof(FVECTOR),N_PRIMS ) ;
	_BigMemCopy( prim->uvrgb[0],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS) ;
	_BigMemCopy( prim->pos[1], MEM_ADDR1,sizeof(FVECTOR),N_PRIMS ) ;
	_BigMemCopy( prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),N_PRIMS) ;
	_BigMemCopy( work->data, MEM_ADDR3,sizeof(DATA),N_PRIMS) ;
	 
	return 0 ;
}

/* -------------------------------------------------------------------- */
/* 息 */
void  *NewBreath( FMATRIX *world, FVECTOR *move, int *sw )
{
	Work	  *work ;

	/* ワークのサイズを確保 */
	work = (Work *)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		DG_COPY_MAT( &work->world, world );
		DG_COPY_VEC( &work->move, move );
		work->sw = sw ;

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
