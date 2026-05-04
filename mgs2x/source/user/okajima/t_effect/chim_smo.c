//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	chim_smo.c
		煙突の煙 
	2000/10/18 S.Okajima
	$Id: chim_smo.c,v 1.4 2002/11/23 12:16:43 Yoshizawa1 Exp $
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
#define  N_VERTS	  15
#define  PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define  DIV_X		 8 
#define  DIV_Y		 4 
#define  MAX_PATERN	30
#define  WIDE		  3000.0F
#define  HEIGHT		3000.0F
#define  GRAVITY	   (-0.0F)
#define  MAX_COUNT	 180  
#define  CHANGE_COUNT  120
#define  UNIT_MAX	  5
#define  MEM_ADDR1	 ((void *)(SCRPAD_ADDR + 0x0000))
#define  MEM_ADDR2	 ((void *)(SCRPAD_ADDR + 0x1200))

/* ---------------------------------------------------------------------------*/

extern  FVECTOR  G_wind ;
extern	int G_wind_sw;

typedef   struct
{
	FVECTOR speed ;				 /* スピード */
	int	 count ;				 /* カウント数*/
	int	 patern ;				/* テクスチャパターン */
} DATA ;

typedef   struct
{
	FVECTOR	start_pos ;   /* スタート位置 */
	float	  pre_wind_vy ; /* 前の風 */
	DATA	   data[N_VERTS] ;
	u_short	 r,g,b ;		  /* 全体の色 */
}UNIT ;

typedef   struct
{
	GV_ACT_EX		actor ;

	DG_PRIM2	  *prim ;
	DG_TEX		*tex ;
	float		 pic_scale_u,pic_scale_v ;
	float		 scale_u,scale_v ;

	int		   unit_num ;	  /* ユニット数 */
	UNIT		  unit[0] ;
} Work ;

/* ---------------------------------------------------------------------------*/
				  /* 
					  プログラム使用サブルーチン
				  */
/* メインメモリからスクラッチパッドへの転送 */
extern void  _BigScrCopy( void *dst, void *src, int size, int num) ;

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

/* ---------------------------------------------------------------------------*/
static  void  Act_Scr(Work *work, DG_TEX *tex)
{
	UNIT			  *unit ;
	DATA			  *data ;
	FVECTOR		   *pos ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	int			   i,j ;
	int			   x,y ;
	float			  offset_u,offset_v ; 

	unit = work->unit ;
#ifndef KP_WINDOWS
	pos  = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
#else
	{
		DG_PRIM2   *prim ;
		prim = work->prim ;
		pos     = prim->pos[prim->buffer_clock] ;
		uvrgbwh = prim->uvrgb[prim->buffer_clock] ;	// 直接展開
	}
#endif

	for(i = 0; i < work->unit_num; i ++)
	{
	data = unit->data ;
	unit->pre_wind_vy = G_wind.vy;
	for(j = 0; j < N_VERTS; j++)
	{
		if(data->count > 0)
		{
		if(data->count > CHANGE_COUNT)
		{
			uvrgbwh->w += 50 ;
			uvrgbwh->h += 50 ;
			if((data->count % 3) == 0)
			{
			if(uvrgbwh->a <= 1)
			{
				uvrgbwh->a = 0 ;
			}
			else
			{
				uvrgbwh->a -= 1 ;
			}
			}
		}
		else
		{
			uvrgbwh->w += 10 ;
			uvrgbwh->h += 10 ;
		}
		
		data->speed.vx = (G_wind.vx + rnd() * 10.0F - 5.0F) / 10.0F;
		data->speed.vy += (GRAVITY + (G_wind.vy - unit->pre_wind_vy) / 10.0F) / 10.0F ;
		data->speed.vz = (G_wind.vz + rnd() * 10.0F - 5.0F) / 10.0F;
		
		/* 位置決定 */
		_sceVu0AddVector(pos,pos,&data->speed) ;
		
		data->count ++ ;
		if(data->count >= MAX_COUNT)
			data->count = 0 ;
		if((data->count % 4) == 0)
		{
			data->patern ++ ;
			if(data->patern >= MAX_PATERN)
			data->patern = 0 ;

			/* テクスチャ決定 */
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
		}
		}
		else if(data->count == 0)
		{
		/* 仮ポジション */
		_sceVu0CopyVector(pos,&unit->start_pos) ;
		/* uvrgbwh情報 */		
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

		uvrgbwh->w = WIDE *0.5f ;
		uvrgbwh->h = HEIGHT *0.5f ;
		uvrgbwh->r = unit->r ;
		uvrgbwh->g = unit->g ;
		uvrgbwh->b = unit->b ;
		uvrgbwh->a = 24 ;
		data->count ++ ;
		}
		else if(data->count < 0)
		{
		data->count ++ ;
		}

		pos ++ ;
		uvrgbwh ++ ;
		data ++ ;
	}
	unit ++ ;
	}
	
}

static  void  Act(Work *work)
{
	DG_PRIM2   *prim ;
	int	  num ;

	prim = work->prim ;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	num = work->unit_num * N_VERTS ;
#ifndef KP_WINDOWS
	_BigScrCopy( MEM_ADDR1,prim->pos[prim->buffer_clock],sizeof(FVECTOR),num ) ;
	_BigScrCopy( MEM_ADDR2,prim->uvrgb[prim->buffer_clock],sizeof(DG_PRIM2_UVRGBWH),num) ;
#else
	memcpy( prim->pos[prim->buffer_clock^1], prim->pos[prim->buffer_clock],
				sizeof(FVECTOR) * num ) ;
	memcpy( prim->uvrgb[prim->buffer_clock^1], prim->uvrgb[prim->buffer_clock],
				sizeof(DG_PRIM2_UVRGBWH) * num) ;
#endif
	/* バッファの交代 */
	DG_SwitchBuffPrim2(prim) ;
	Act_Scr(work,work->tex) ;
#ifndef KP_WINDOWS
	_BigMemCopy( prim->pos[prim->buffer_clock],MEM_ADDR1,sizeof(FVECTOR),num ) ;
	_BigMemCopy( prim->uvrgb[prim->buffer_clock],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),num ) ;
#endif
}

/* ---------------------------------------------------------------------------*/

static  void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ------------------------------------------------------------------- */
static void GetOptionValue(Work *work)
{
	int  buf[3] ;
	UNIT *unit ;
	int  i ;

	unit = work->unit ;

	/* 位置を初期化 */
	if(GCL_GetOption('c') != NULL){
		for(i  = work->unit_num; i > 0; i --){
			GCL_GetIV(GCL_NextStr(),buf) ;
			unit->r = (u_short)buf[0] ;
			unit->g = (u_short)buf[1] ;
			unit->b = (u_short)buf[2] ;
			unit ++ ;
		}
	}else{
		for(i = work->unit_num ; i > 0; i--){
			unit->r = 16 ;
			unit->g = 16 ;
			unit->b = 16 ;
			unit ++ ;
		}
	}

	unit = work->unit ;
	if(GCL_GetOption('p') != NULL){
		for(i = work->unit_num; i> 0; i --){
			GCL_GetIV(GCL_NextStr(),buf) ;
			unit->start_pos.vx = (float)buf[0] ;
			unit->start_pos.vy = (float)buf[1] + HEIGHT / 3.0F;
			unit->start_pos.vz = (float)buf[2] ;
			unit->start_pos.vw = 1.0F ;
			unit ++ ;
		}
	}else{
		for(i = work->unit_num ; i > 0;  i--){
			unit->start_pos.vx = 0.0F ;
			unit->start_pos.vy = 0.0F ;
			unit->start_pos.vz = 0.0F ;
			unit->start_pos.vw = 1.0F ;
			unit ++;
		}
	}
	
}

static int InitScrPad(Work *work, DG_TEX *tex)
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	UNIT			   *unit ;
	DATA			   *data ;
	int				i,j ;
	int				x,y ;
	float			  offset_u,offset_v ;

	unit = work->unit ;
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	
	
	for(i = 0; i < work->unit_num; i++)
	{
	unit->pre_wind_vy = G_wind.vy ;
	data	= unit->data ;
	for(j = 0; j < N_VERTS; j++)
	{
		/* 直代入 */
		/* データを初期化 */
		data->patern = irnd() % MAX_PATERN ;
		data->count  = j * (MAX_COUNT / N_VERTS) - 1;
		data->speed.vx = (G_wind.vx + rnd() * 10.0F - 5.0F) / 10.0F ;
		data->speed.vy = (15.0F + G_wind.vy / 10.0F) / 10.0F ;
		data->speed.vz = (G_wind.vz + rnd() * 10.0F - 5.0F) / 10.0F ;
		
		/* ----------------------------------------------*/
		/* スクラッチパッドのデータ */
		/* 仮ポジション */
		_sceVu0CopyVector(pos,&unit->start_pos) ;
		/* uvrgbwh情報 */
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

		uvrgbwh->w = WIDE *0.5f ;
		uvrgbwh->h = HEIGHT *0.5f ;
		uvrgbwh->r = unit->r ;
		uvrgbwh->g = unit->g ;
		uvrgbwh->b = unit->b ;
		uvrgbwh->a = 0 ;

		pos ++ ;
		uvrgbwh ++ ;
		data ++ ;
	}
	unit ++ ;
	}
	
	return 0 ;
}

static int GetResources( Work *work )
{
	DG_PRIM2 *prim;
	DG_TEX   *tex ;
	int	  num ;
	
	prim = work->prim = GM_MakePrim2(PRIM_TYPE,work->unit_num,N_VERTS) ;
	if(prim == NULL)
	return -1 ;

	tex= work->tex = DG_GetTexture( 16238439 /*"smoke_msk"*/) ;
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
   
	GetOptionValue(work) ;
	InitScrPad(work,tex) ;
	
	num = work->unit_num * N_VERTS ;
	_BigMemCopy( prim->pos[0], MEM_ADDR1,sizeof(FVECTOR),num) ;
	_BigMemCopy( prim->uvrgb[0],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),num) ;
	_BigMemCopy( prim->pos[1], MEM_ADDR1,sizeof(FVECTOR),num ) ;
	_BigMemCopy( prim->uvrgb[1],MEM_ADDR2,sizeof(DG_PRIM2_UVRGBWH),num) ;
	 
	return 0 ;
}

void  *New_ChimnySmoke( void )
{
	Work	  *work ;
	int	   buf_size ;
	int	   unit_num ;
	int	   max_num ;

	/* ユニット数決定 */
	if( GCL_GetOption('n') != NULL){
		unit_num = GCL_GetNextInt() ;
		max_num = UNIT_MAX ;
		if(unit_num > max_num){
			printf("MAX_UNIT  %d\n",max_num);
			unit_num = max_num;
		}
	}else{
		unit_num = 1 ;
	}

	/* ワークのサイズを確保 */
	buf_size = sizeof(Work) + sizeof(UNIT) * unit_num ;
	work = (Work *)GV_NewEffect(GV_ACTOR_USER, buf_size);
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );

		work->unit_num = unit_num ;				/* ユニット数 */

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
