//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	flashlight_ini.c
	初期化部 
	2000/02/18   H.TANAKA
	2000/10/18 S.Okajima
	$Id: fl_light_act.c,v 1.1.1.3 2002/11/19 11:47:44 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
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
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

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
#include	"flashlight.h"
#include	"../etc/ok_util.h"


/*------------------------------------------*/
/*	  ガラス系							*/
/*------------------------------------------*/
int   FL_LIGHT_Gls_Act(Work *work)
{
#if 1
	GLASS	 *gls ;
	DG_PRIM2  *prim ;
	FVECTOR   *pos ;
	FVECTOR   *speed ;
	FVECTOR   *base ;
	SVECTOR   *rot,*const_rot ;
	int	   i ;

	gls = work->glass ;
	prim = gls->prim ;
	if(gls->glass_count <= 0){
		DG_InvisiblePrim2(prim) ;
		return -1 ;
	}

	speed = gls->speed ;
	base  = gls->base ;
	rot   = gls->rot ;
	const_rot = gls->const_rot ;

	DG_SwitchBuffPrim2(prim) ;

	/* 位置決定 */
	pos = prim->pos[prim->buffer_clock] ;
	for(i = 0; i < N_GLASS_NUM; i++){
		/* 位置決め */
		_sceVu0AddVector(base,base,speed) ;
		rot->vx += const_rot->vx ;
		if(rot->vx >= 4096){
			rot->vx -= 4096 ;
		}else if(rot->vx <= -4096){
			rot->vx += 4096 ;
		}

		rot->vy += const_rot->vy ;
		if(rot->vy <= 4096){
			rot->vy -= 4096 ;
		}else if(rot->vy <= -4096){
			rot->vy -= 4096 ;
		}
		DG_SetPos2(base,rot) ;
		DG_PutVector(FL_LIGHT_Triangle,pos,N_TRIANGL_VERTS) ;

		speed->vy -= 0.5F ;

		speed ++ ;
		base  ++ ;
		rot ++ ;
		const_rot ++ ;
		pos += N_TRIANGL_VERTS ;
	}
#endif
	return 0 ;
}

/*------------------------------------------*/
/*  破壊系								  */
/*------------------------------------------*/
int  FL_LIGHT_None_Act(Work *work)
{
	return 0 ;
}

int  FL_LIGHT_Disapp_Act(Work *work)
{
	DG_PRIM2		   *prim ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	int				alpha, i ;
	FLASHLIGHT		 *fl ;

	fl = &work->flashlight ;
	prim = fl->prim ;
	if(fl->break_count <= 0){
		DG_InvisiblePrim2(fl->prim) ;
		fl->prim        = OK_FreePrim2(fl->prim) ;
		fl->prim_hl_sub = OK_FreePrim2(fl->prim_hl_sub) ;
		fl->prim_hl     = OK_FreePrim2(fl->prim_hl) ;
		fl->prim_hl_add = OK_FreePrim2(fl->prim_hl_add) ;
		fl->act = (void *)FL_LIGHT_None_Act ;
		return 0 ;
	}

	DG_SwitchBuffPrim2(fl->prim) ;

	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	DG_SwitchBuffPrim2(fl->prim) ;

	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	alpha = 32 - 2 * (20 - fl->break_count) ;
	for(i = 0; i < N_MAIN_VERTS; i++){
		alpha -= 3 ;
		if(alpha < 0) alpha = 0 ;
		uvrgbwh->a = (u_short)alpha ;
		uvrgbwh ++ ;
	}

	fl->break_count -- ;
	return 0 ;
}

/* 火花部分 */
static int FL_LIGHT_Spark(Work *work)
{
	FVECTOR	 pos ;
	SVECTOR	 local_rot,rot_wide ;
	int		 n ;
	FVECTOR	 color ;
	FLASHLIGHT  *fl ;
	
	fl = &work->flashlight ;
	DG_SetPos2(&work->pos, &work->rot) ;
	if(fl->fire_count <= 0)
	{
	/*  位置 */
	pos.vx = - 50.0F + rnd() * 100.0F ;
	pos.vy = 50.0F + rnd() * 100.0F ;
	pos.vz = -100.0F + rnd() * 200.0F ;
	DG_PutVector(&pos,&pos,1) ;
	local_rot.vx = - (irnd() % 1024) ;
	local_rot.vy = irnd() % 4096 ;
	local_rot.vz = 0 ;
	
	rot_wide.vx = 512 ;
	rot_wide.vy = 512 ;
	rot_wide.vz = 0 ;
	
	color.vx = 255.0F ;
	color.vy = 200.0F ;
	color.vz = 128.0F ;
	color.vw = 50.0F ;
	
	n = 16 + irnd() % 8 ;

	NewSpark1(n,&pos,36.0F,36.0F,(rnd() * 3.0F), &local_rot, &rot_wide, &color, 0.5F,10) ;

	GM_SeSetMode( SD_A_HIBANA01 , &pos, GM_SEMODE_NORMAL ) ;
	

//	Big_TmpLight2(&pos,150.0F,300.0F, 0x407080,3) ;
	DG_SetTmpLight2(
		&pos,
		150.0f,
		300.0f,
		80 | 70<<8 | 40 <<16,
		LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ) ;

	   
	FL_LIGHT_SetSound(SD_A_HIBANA01,&pos,GM_SEMODE_NORMAL) ;
	
	fl->fire_count = irnd() % 14 + 6 ;
	return 1 ;
	}

	return 0 ;
}

/* ランプを点滅させる */
static  void  FL_LIGHT_OnAndOff(Work *work,int flag)
{
	DG_PRIM2		   *prim ;
	DG_PRIM2_UVRGBWH   *uvrgbwh ;
	int				i ;
	int				alpha ;
	FLASHLIGHT		 *fl ;

	fl = &work->flashlight ;
	prim = fl->prim ;	
	DG_SwitchBuffPrim2(prim) ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	if(flag){
		alpha = 24 + irnd() % 6;
		for(i = 0; i < N_MAIN_VERTS; i ++){
			alpha -= 3 ;
			if(alpha < 0) alpha = 0 ;
			uvrgbwh->a = (u_short)alpha ;
			uvrgbwh ++ ;
		}
	}else{
		for(i = 0; i < N_MAIN_VERTS; i++){
			uvrgbwh->a = 0 ;
			uvrgbwh ++ ;
		}
	}
}

/* 破壊時のエフェクト */
static  void  Goto_Disapp_Act(FLASHLIGHT *fl)
{
	DG_PRIM2		  *prim ;
	DG_PRIM2_UVRGBWH  *uvrgbwh ;
	int			   alpha ;
	int			   i ;

	fl->act = (void *)FL_LIGHT_Disapp_Act ;
	/* ボンボリを初期化 */
	prim = fl->prim ;
	DG_SwitchBuffPrim2(prim) ;
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	alpha   = 32 ;
	for(i = 0; i < N_MAIN_VERTS; i++){
		alpha -= 3 ;
		if(alpha < 0) alpha = 0 ;
		uvrgbwh->a = (u_short)alpha ;
		uvrgbwh ++ ;
	}
	fl->break_count = 20 ;
}

int  FL_LIGHT_Break_Act(Work *work)
{
	FLASHLIGHT		 *fl ;
	int				flag ;

	fl = &work->flashlight ;
	if(fl->break_count <= 0){
		Goto_Disapp_Act(fl) ;
		return 0 ;
	}

	/* 火花を散らす */
	flag = 0 ;

	flag = FL_LIGHT_Spark(work) ;
	/* ランプを点滅させる */
	FL_LIGHT_OnAndOff(work,flag) ;

	fl->break_count -- ;
	fl->fire_count -- ;

	return 0 ;
}

/* --------------------------------------------------- */
/*  ノーマル時 */
/* --------------------------------------------------- */
static  void  FL_LIGHT_ActHalation( Work *work )
{
	FLASHLIGHT		*fl ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		   *pos ;
	FVECTOR		   center ;
	FVECTOR		   fvtemp0 ;
	FVECTOR		fvtemp1;
	FMATRIX		mat;
	int			clock;
	int			alpha;
	float		size;
	float		len;
	float		inner;

	fl = &work->flashlight ;
	/* 動く可能性が無ければ初期化時に行列を作るほうがいい */
	DG_SetPos2( &work->pos, &work->rot );
	fvtemp0.vx = SHIFT_X;
	fvtemp0.vy = SHIFT_Y;
	fvtemp0.vz = SHIFT_Z;
	DG_MovePos( &fvtemp0 );
	DG_GetPos( &mat );
	DG_COPY_VEC( &center, (FVECTOR *)mat.m[3] );

	/* カメラからの距離を求める */
	_sceVu0SubVector( &fvtemp0, &center, (FVECTOR *)DG_Chanls->eye.m[3] );
	len=GV_VecLen3F( &fvtemp0 );
	/* 懐中電灯の光線の方向ベクトルを生成 */
	fvtemp0.vx = 0.0f;
	fvtemp0.vy = 0.0f;
	fvtemp0.vz = 1.0f;
//  DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_RotVector( &fvtemp0, &fvtemp0, 1 );

	/* カメラからライトへのベクトルと、ライトのベクトルの内積 */
	_sceVu0SubVector( &fvtemp1, &center, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	_sceVu0Normalize( &fvtemp1, &fvtemp1 );
	inner = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );

	/* 懐中電灯の光線の方向ベクトルを */
	/* カメラ行列と乗算（乗算結果のＺ成分がライト方向のカメラ法線への成分となる） */
	DG_SetPos( &DG_Chanls->eye_inv );
	DG_RotVector( &fvtemp0, &fvtemp0, 1 );

	inner *= fvtemp0.vz;

	/* 光源の透視変換点を求める */
	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( &center, &fvtemp1, 1 );	/* ずらす */

	if( inner < LIMIT_ANGLE	/* 負だと光線の方向が逆向き */
		|| fvtemp1.vz > fvtemp1.vw		/* カメラの後ろに光源があった */
		|| len		> LIMIT_RANGE	/* 遠すぎた */
		|| GM_CheckPlayerStatus( PLAYER_LOCKER ) ){
//printf("invisible\n") ;	
		DG_InvisiblePrim2( fl->prim_hl_sub ) ;
		DG_InvisiblePrim2( fl->prim_hl ) ;
		DG_InvisiblePrim2( fl->prim_hl_add ) ;
	} else {
//printf("visible\n") ;
	inner = (inner - LIMIT_ANGLE) / (1.0f - LIMIT_ANGLE);	/* 比に変換 */
	len = (LIMIT_RANGE - len) / LIMIT_RANGE;	/* 比に変換 */
	DG_VisiblePrim2( fl->prim_hl_sub ) ;
	DG_VisiblePrim2( fl->prim_hl ) ;
	DG_VisiblePrim2( fl->prim_hl_add ) ;
	fvtemp1.vw = (fvtemp1.vw  > 0.0f)? fvtemp1.vw: -fvtemp1.vw;
	fvtemp1.vx = SCREEN_NEAR / (ASPECT_X()) / DG_Chanls->screen * fvtemp1.vx / fvtemp1.vw;
	fvtemp1.vy = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen * fvtemp1.vy / fvtemp1.vw;
	alpha = (int)(ALPHA_MAX_SC * inner * len);
	size  = SIZE_MAX_SC  * inner * len;
	
	DG_SwitchBuffPrim2( fl->prim_hl_sub );
	clock = fl->prim_hl_sub->buffer_clock;
	uvrgb = fl->prim_hl_sub->uvrgb[clock];
	pos   = fl->prim_hl_sub->pos[clock];

	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx - size;
	pos->vy = fvtemp1.vy - size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx + size;
	pos->vy = fvtemp1.vy - size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx - size;
	pos->vy = fvtemp1.vy + size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx + size;
	pos->vy = fvtemp1.vy + size;
//			pos++;
	
	DG_SwitchBuffPrim2( fl->prim_hl );
	clock = fl->prim_hl->buffer_clock;
	uvrgb = fl->prim_hl->uvrgb[clock];
	pos   = fl->prim_hl->pos[clock];
	
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx - size;
	pos->vy = fvtemp1.vy - size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx + size;
	pos->vy = fvtemp1.vy - size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx - size;
	pos->vy = fvtemp1.vy + size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx + size;
	pos->vy = fvtemp1.vy + size;
//		pos++;

	DG_SwitchBuffPrim2( fl->prim_hl_add );
	clock = fl->prim_hl_add->buffer_clock;
	uvrgb = fl->prim_hl_add->uvrgb[clock];
	pos   = fl->prim_hl_add->pos[clock];

	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	uvrgb->a = alpha;	uvrgb++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx - size;
	pos->vy = fvtemp1.vy - size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx + size;
	pos->vy = fvtemp1.vy - size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx - size;
	pos->vy = fvtemp1.vy + size;
	pos++;
	pos->vz = SCREEN_NEAR;
	pos->vx = fvtemp1.vx + size;
	pos->vy = fvtemp1.vy + size;
//		pos++;
	}
}

/* 通常動作 */
int  FL_LIGHT_NormalAct(Work *work)
{
	/* メインランプは動かない */
	
	/* ハレーションは動く */
	FL_LIGHT_ActHalation(work) ;

	return 0 ;
}
