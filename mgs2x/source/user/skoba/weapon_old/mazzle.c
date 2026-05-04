//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	武器のマズルフラッシュ 
	2000/01/11   H.TANAKA
	2000/10/18 S.Okajima
	2001/03/30 S.Kobayashi	
	$Id: mazzle.c,v 1.1.1.3 2002/11/19 11:50:35 Yoshizawa1 Exp $
*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"
#include	"def_dma.h"

#include	"big_weapon.h"

extern void *NewClaySmokeNormal( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeGray( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeFire( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeFire2( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeWhite( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
//extern void *NewSK_Easy_Smoke2Normal( FMATRIX *world, FVECTOR *shift, char *status ); 
int	OK_togle_switch = 1;

enum {
	EFFECT_FLAGS_AMO    = (0x01),
	EFFECT_FLAGS_LINE   = (0x02),
	EFFECT_FLAGS_REC    = (0x04),
	EFFECT_FLAGS_ON_TMP	= (0x08),
	EFFECT_FLAGS_ON_SPARK = (0x10),
	EFFECT_FLAGS_SAA_LEFT = (0x20),
	EFFECT_FLAGS_NO_MZL = (0x40), // 仮 
};

enum {
	EFFECT_MAIN_VIEW = 0x100,
};

#if 1	//kore 02.4.24
#define R_RANGE	1250.f
#define E_RANGE	2500.0f
#define L_COLOR	(80 | 55 << 8 | 40 <<16)
#else
#define R_RANGE	780.0f
#define E_RANGE	1560.0f
#define L_COLOR	(0xc0 | 0xa0 << 8 | 0x50 <<16)
#endif


/* ------------------------ */
/* M92 */
void MazzleM92( FMATRIX *world, int silence, int light_mode )
{
}

// 海兵 
void MazzleM92_Sea( FMATRIX *world, int silence, int light_mode )
{
	static FVECTOR  shift[] = {
		{18.0F,-303.0F,60.0F,1.0F},
		{18.0F,-403.0F,60.0F,1.0F},
		{18.0F,-458.0F,60.0F,1.0F},
		{18.0F,-488.0F,60.0F,1.0F}
	} ;
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	static float  size[] = {150.0F,100.0F,75.0F,50.0F} ;
	static u_char bright[] = {55,30,20,10} ;

	FMATRIX	 mat ;
	
	/* マズルフラッシュ 単発 */
	MazzleFlash(4,world,shift,size,bright) ;
	
	/* 火花 と テンプライト*/
	DG_SetPos( world ) ;
	DG_MovePos( &shift[0] ) ;
	DG_GetPos( &mat ) ;

	{
	if ( light_mode & EFFECT_FLAGS_ON_SPARK ){
		SVECTOR  rot ;
		SVECTOR  rot_wide ;
		FVECTOR  color ;

		rot.vx = 256 ;
		rot.vy = 0 ;
		rot.vz = 0 ;
		rot_wide.vx = 1536 ;
		rot_wide.vy = 4096 ;
		rot_wide.vz = 0 ;
		color.vx = 50.0F ;
		color.vy = 25.0F ;
		color.vz = 25.0F ;
		color.vw = 100.0F ;
		NewSpark2(4,&mat,36.0F,16.0F,0.5F,&rot,&rot_wide,&color,1.0F,6) ;
	}
	}
	
	{
		FVECTOR   smoke_shift ;
		FVECTOR   smoke_pos,smoke_speed ;

		smoke_shift.vx = 18.0F ;
		smoke_shift.vy = -303.0F ;
		smoke_shift.vz = 60.0F ;
		smoke_shift.vw = 1.0F ;
		DG_SetPos( world ) ;
		DG_PutVector(&smoke_shift,&smoke_pos,1) ;
		DG_MovePos( &smoke_shift ) ;
		DG_GetPos( &mat ) ;
		smoke_shift.vx = 0.0F ;
		smoke_shift.vy = rnd();
		smoke_shift.vz = -30.0F + rnd() * 5.0F ;
		smoke_shift.vw = 1.0F ;
		DG_RotVector(&smoke_shift,&smoke_speed,1) ;

		smoke_shift.vx = 18.0F ;
		smoke_shift.vy = -303.0F ;
		smoke_shift.vz = 60.0F ;
		smoke_shift.vw = 1.0F ;
		NewSmokeNormal( 20 , world, &smoke_shift, &smoke_speed , 10 , 200.0f , &ftmp , 4 );
		//	NewGunSmoke(&smoke_pos,&smoke_speed,150.0F,0) ; 
	}
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)mat.m[ 3 ], GM_SEMODE_NORMAL );	

	{
		FVECTOR   smoke_shift ;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 18.0F ;
			smoke_shift.vy = -500.0F ;
			smoke_shift.vz = 60.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &mat );

#if 0 
/// by yano
			DG_SetTmpLight2(
				    (FVECTOR *)mat.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
#endif
		}
	}
}

void AN_MazzleM92_Sea(FMATRIX *world , int silence , int mode )
{
	MazzleM92_Sea( world, silence , mode );
}

/* -------------------------- */
/* USP(サイレンサー無) */
void MazzleUSP( FMATRIX *world, int silence, int light_mode )
{
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	static FVECTOR  shift[] = {
		{17.5F,-275.0F,60.0F,1.0F},
		{17.5F,-375.0F,60.0F,1.0F},
		{17.5F,-430.0F,60.0F,1.0F},
		{17.5F,-460.0F,60.0F,1.0F}
	} ;
	FVECTOR       Usp_shift_mazzle;
	int           tmp;
	static float  size[] = {150.0F,100.0F,75.0F,50.0F} ;
	static u_char bright[] = {56,20,10,6} ;

	FMATRIX	 mat ;

	if ( silence & EFFECT_FLAGS_NO_MZL ){
		Usp_shift_mazzle.vx = 17.5f;
		Usp_shift_mazzle.vy = -466.5f;
		Usp_shift_mazzle.vz = 70.5f;
		Usp_shift_mazzle.vw = 1.0f;
	} else {
		Usp_shift_mazzle.vx = 17.5f;
		Usp_shift_mazzle.vy = -275.0f;
		Usp_shift_mazzle.vz = 60.0f;
		Usp_shift_mazzle.vw = 1.0f;

		tmp = -( irnd() % 20 ) - 60;
		/* マズルフラッシュ 単発 */
		MazzleFlash(4,world,shift,size,bright) ;
		MazzleFlash2( 4 , world , &Usp_shift_mazzle , size , bright , ( float )tmp );
	}
	
	/* 火花 と テンプライト*/
	DG_SetPos( world ) ;
	DG_MovePos( &shift[0] ) ;
	DG_GetPos( &mat ) ;

	if ( !( silence & EFFECT_FLAGS_NO_MZL ) ){
	{
		SVECTOR  rot ;
		SVECTOR  rot_wide ;
		FVECTOR  color ;

		rot.vx = 256 ;
		rot.vy = 0 ;
		rot.vz = 0 ;
		rot_wide.vx = 1536 ;
		rot_wide.vy = 4096 ;
		rot_wide.vz = 0 ;
		color.vx = 50.0F ;
		color.vy = 25.0F ;
		color.vz = 25.0F ;
		color.vw = 100.0F ;
		NewSpark2(4,&mat,36.0F,16.0F,0.5F,&rot,&rot_wide,&color,1.0F,6) ;
	}
	}

	{
		FVECTOR   smoke_shift ;
		FVECTOR   power;

		smoke_shift.vx = 17.5F ;
		smoke_shift.vy = -360.0F ;
		smoke_shift.vz = 20.0F ;
		smoke_shift.vw = 1.0F ;
		power.vx = 0.F;
		power.vy = 0.f;
		power.vz = -( 40.0F + rnd() * 5.0F );
		power.vw = 1.0F ;
//		NewSmokeNormal( 40 , world, &smoke_shift , &smoke_speed , 10 , 160.0f , &ftmp , 4 ); 
		if ( silence & EFFECT_MAIN_VIEW ) { // 主観  
			NewSmokeNormal( 40 , world, &smoke_shift, &power , 10 , 160.0f , &ftmp , 4 );
			// 後にさがる煙 
			power.vz = ( rnd() * 60.0f + 110.0f );
			NewSmokeNormal( 10 , world, &smoke_shift, &power , 10 , 320.0f , &ftmp , 4 );
		} else {
			NewSmokeNormal( 40 , world, &smoke_shift, &power , 10 , 160.0f , &ftmp , 4 );
		}

	}
	{
		FVECTOR   smoke_shift ;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 17.5F ;
			smoke_shift.vy = -560.0F ;
			smoke_shift.vz = 60.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &mat );
			DG_SetTmpLight2(
				    (FVECTOR *)mat.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

/* 昔の奴 */
void AN_MazzleUSP(FMATRIX *world, int  silence)
{
	MazzleUSP(world,0,0) ;
	return ;
}

// Socom 
void MazzleSOCOM( FMATRIX *world, int silence, int light_mode )
{
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	static float  size[] = {170.0F,120.0F,95.0F,70.0F} ;
	static u_char bright[] = {62,26,16,12} ;
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	FVECTOR         Socom_shift_mazzle;
	float  tmp;

	power.vx = 0;
	power.vy = rnd() * 2.0f;
	power.vz = -( rnd() * 20.0f + 30.0f );
	power.vw = 0;

	if ( silence & EFFECT_FLAGS_NO_MZL ){
		Socom_shift_mazzle.vx = 17.5f;
		Socom_shift_mazzle.vy = -501.0f;
		Socom_shift_mazzle.vz = 78.7f;
		Socom_shift_mazzle.vw = 1.0f;
	} else {
		Socom_shift_mazzle.vx = 7.5f;
		Socom_shift_mazzle.vy = -314.0f;
		Socom_shift_mazzle.vz = 78.7f;
		Socom_shift_mazzle.vw = 1.0f;
		tmp = -(BP_PS2_rand() % 20) - 60;
		/* マズルフラッシュ 単発 */
		MazzleFlash2( 4 , world , &Socom_shift_mazzle , size , bright , ( float )tmp );
	}
	if ( silence & EFFECT_MAIN_VIEW ) { // 主観 
		NewSmokeNormal( 40 , world, &Socom_shift_mazzle, &power , 10 , 160.0f , &ftmp , 4 );
		//  
		power.vz = ( rnd() * 60.0f + 110.0f );
		NewSmokeNormal( 20 , world, &Socom_shift_mazzle, &power , 10 , 320.0f , &ftmp , 4 );
	} else {
		NewSmokeNormal( 40 , world, &Socom_shift_mazzle, &power , 10 , 160.0f , &ftmp , 4 );
	}
	fwork = DG_ZeroVector;
	fwork.vy = 10.0f;
	_sceVu0AddVector( &fwork , &fwork , &Socom_shift_mazzle );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 17.5F ;
			smoke_shift.vy = -560.0F ;
			smoke_shift.vz = 60.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleSOCOM(FMATRIX *world , int silence , int mode )
{
	MazzleSOCOM( world, silence , mode );
}

/* ------------------------------ */
/* FAMAS */
void MazzleFAMAS( FMATRIX *world, int silence, int light_mode )
{
	
	static FVECTOR  shift[] = {
	{19.5F,-537.0F,72.0F,1.0F},
	{19.5F,-587.0F,72.0F,1.0F},
	{19.5F,-637.0F,72.0F,1.0F},
	{19.5F,-687.0F,72.0F,1.0F},
	{19.5F,-737.0F,72.0F,1.0F}
	} ;

	static  float  size[] = {80.0F,75.0F,75.0F,70.0F,70.0F} ;
	static u_char bright[] = {36,36,36,30,24} ;

	FMATRIX   mat ;

	FVECTOR	sub_shift ;

	/* マズルフラッシュ 連発*/
	MazzleFlash(5,world, shift,size,bright) ;
	
	/* サブマズルフラッシュ */
	sub_shift.vx = 19.5F ;
	sub_shift.vy = -515.7F ;
	sub_shift.vz = 72.0F ;
	DG_SetPos( world ) ;
	DG_MovePos( &sub_shift ) ;
	DG_GetPos( &mat ) ;
//	SubMazzleAKS(&mat) ; 
	SubMazzleFlash(&mat) ;

	{
		FMATRIX fmtmp;
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 19.5F ;
			smoke_shift.vy = -715.0F ;
			smoke_shift.vz = 72.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

/* マカロフ(サイレンサー無) */
void MazzleMKR( FMATRIX *world, int silence, int light_mode )
{
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	static FVECTOR  shift[] = {
	{20.0F,-232.0F,54.5F,1.0F},
	{20.0F,-310.0F,54.5F,1.0F},
	{20.0F,-390.0F,54.5F,1.0F},
	{20.0F,-410.0F,54.5F,1.0F}
	} ;

	static float  size[] = {125.0F,75.0F,50.0F,25.0F} ;
	static u_char bright[] = {48,20,12,4} ;
	FMATRIX	 mat ;

	/* マズルフラッシュ 単発 */
	MazzleFlash(4,world,shift,size,bright) ;
	/* 火花 */
	DG_SetPos( world ) ;
	DG_MovePos( &shift[0] ) ;
	DG_GetPos( &mat ) ;

	{
	SVECTOR	 rot ;
	SVECTOR	 rot_wide ;
	FVECTOR	 color ;
	FMATRIX	 fmtmp ;
	
	rot.vx = 256 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	rot_wide.vx = 1536 ;
	rot_wide.vy = 4096 ;
	rot_wide.vz = 0 ;
	color.vx = 50.0F ;
	color.vy = 25.0F ;
	color.vz = 25.0F ;
	color.vw = 100.0F ;
	NewSpark2(4,&mat,24.0F,12.0F,0.5F,&rot,&rot_wide,&color,1.0F,6) ;

	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0F ;
			smoke_shift.vy = -432.0F ;
			smoke_shift.vz = 54.5F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}

	}
	
	{
		FVECTOR smoke_shift;
		FVECTOR power;
	
		smoke_shift.vx = 20.0F ;
		smoke_shift.vy = -310.0F ;
		smoke_shift.vz = 54.5F ;
		smoke_shift.vw = 1.0F ;
		power.vx = 0.F;
		power.vy = 0.f;
		power.vz = -( 40.0F + rnd() * 5.0F );
		power.vw = 1.0F ;
		NewSmokeNormal( 40 , world, &smoke_shift, &power , 10 , 140.0f , &ftmp , 4 );
	}

}

void	AN_MazzleMKR(FMATRIX *world,int silence,int mode)
{
	MazzleMKR( world, silence,mode )  ;
}

// AKS ゴルル 
#define AKS_MAZZELE_MAX (8) 
void MazzleAKS( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	FVECTOR  AKS_shift_mazzule;
	static FVECTOR  ftmp = { 40.f , 40.f , 60.f , 0.f };
	u_char bright[ AKS_MAZZELE_MAX ] = { 20 , 20 , 20 , 20 , 15 , 15 , 15 , 10 };
	float  size[ AKS_MAZZELE_MAX ];
	float  tmp;
	int      i;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );

	if ( silence & 0x100 ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 4;
		smoke_num = 4;
		for ( i = 0 ; i < mazzle_max ; i++ ){
			bright[ i ] /= 2;
			size[ i ] /= 2;
		}
		tmp = -rnd() * 20.f;
	} else {
		mazzle_max = 8;
		smoke_num = 10;
		tmp = -rnd() * 40.f - 15.f;
	}
	// 発射場所をずらす 
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	if ( silence ){
		AKS_shift_mazzule.vx = 19.5F;
		AKS_shift_mazzule.vy = -730.0f;
		AKS_shift_mazzule.vz = 74.0f;
		AKS_shift_mazzule.vw = 1.0f;
		NewSmokeNormal( smoke_num , world, &AKS_shift_mazzule, &power , 10 , 200.0f , &ftmp , 6 );
		fwork = DG_ZeroVector;
		fwork.vy += 10;
		DG_SetPos( world );
		DG_MovePos( &fwork );
		DG_GetPos( &fmtmp );
	} else {
		AKS_shift_mazzule.vx = 19.5f;;
		AKS_shift_mazzule.vy = -560.0f;
		AKS_shift_mazzule.vz = 72.0f;
		AKS_shift_mazzule.vw = 1.0f;
		_sceVu0AddVector( &fwork , &fwork , &AKS_shift_mazzule );
		NewSmokeNormal( smoke_num , world, &AKS_shift_mazzule, &power , 10 , 200.0f , &ftmp , 6 );
		MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
		/* サブマズルフラッシュ */
		fwork = DG_ZeroVector;
		fwork.vy += 10;
		_sceVu0AddVector( &fwork , &fwork , &AKS_shift_mazzule );
		DG_SetPos( world );
		DG_MovePos( &fwork );
		DG_GetPos( &fmtmp );
		SubMazzleFlash2( &fmtmp ); 
	}
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL ); 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -930.0f;
			smoke_shift.vz = 89.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[3],
					R_RANGE,
					E_RANGE,
					L_COLOR,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

/* マズルフラッシュ  */
void AN_MazzleAKS(FMATRIX *world,int silence,int mode)
{
	MazzleAKS(world, silence, mode ) ;

	return ;
}

/* P90 */
void MazzleP90( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	static FVECTOR ftmp = { 100.f , 50.f , 100.f , 0.f };
	static FVECTOR p90_shift_mazzule = { 17.5f, -395.f, 57.5f , 0.0f };
	static u_char bright[] = {60,25,15,8 , 5} ;
	float  size[5];
	int    tmp;

	power.vx = 0;
	power.vy = -( BP_PS2_rand() % 5 + 2 );
	power.vz = -( BP_PS2_rand() % 30 + 10 );
	power.vw = 0;
	size[0] = BP_PS2_rand() % 20 + 160;
	size[1] = BP_PS2_rand() % 20 + 135;
	size[2] = BP_PS2_rand() % 20 + 110;
	size[3] = BP_PS2_rand() % 20 + 85;
	size[4] = BP_PS2_rand() % 20 + 65;
	tmp = -(BP_PS2_rand() % 20) - 100;
	NewSmokeNormal( 30 , world, &p90_shift_mazzule, &power , 4 , 250.0f , &ftmp , 4 );
	MazzleFlash2( 5 , world , &p90_shift_mazzule , size , bright , ( float )tmp ); // フラッシュ 
	DG_SetPos( world );
	DG_MovePos( &p90_shift_mazzule );
	DG_GetPos( &fmtemp );
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL ); 

	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 17.5F ;
			smoke_shift.vy = -595.0F ;
			smoke_shift.vz = 57.5F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

/* マズルフラッシュ  */
void AN_MazzleP90(FMATRIX *world,int silence,int mode)
{
	MazzleP90(world, silence, mode ) ;

	return ;
}

// ソリダス 
void MazzleP90_Solidas( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	static FVECTOR ftmp = { 100.f , 50.f , 100.f , 0.f };
	static FVECTOR p90_shift_mazzule = { 17.0f , -321.0f , 60.0f , 0.0f };
	static u_char bright[] = { 60 , 25 , 15 , 8 , 5} ;
	float  size[5];
	int    tmp;

	power.vx = 0;
	power.vy = -( BP_PS2_rand() % 5 + 2 );
	power.vz = -( BP_PS2_rand() % 30 + 10 );
	power.vw = 0;
	size[0] = BP_PS2_rand() % 20 + 160;
	size[1] = BP_PS2_rand() % 20 + 135;
	size[2] = BP_PS2_rand() % 20 + 110;
	size[3] = BP_PS2_rand() % 20 + 85;
	size[4] = BP_PS2_rand() % 20 + 65;
	tmp = -(BP_PS2_rand() % 20) - 100;
	NewSmokeNormal( 30 , world, &p90_shift_mazzule, &power , 4 , 250.0f , &ftmp , 4 );
	MazzleFlash2( 5 , world , &p90_shift_mazzule , size , bright , ( float )tmp ); // フラッシュ 
	DG_SetPos( world );
	DG_MovePos( &p90_shift_mazzule );
	DG_GetPos( &fmtemp );
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL ); 

	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 17.5F ;
			smoke_shift.vy = -605.0F ;
			smoke_shift.vz = 57.5F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

/* マズルフラッシュ  */
void AN_MazzleP90_Solidas(FMATRIX *world,int silence,int mode)
{
	MazzleP90_Solidas(world, silence, mode ) ;

	return ;
}

// GLK(グロック) 
void MazzleGLK( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	static FVECTOR  GLK_shift_mazzule = { 9.f , -320.f , 71.f , 0.0f };
	static FVECTOR  ftmp = { 100.f , 50.f , 100.f , 0.f };
	static u_char   bright[] = { 50 , 30 , 10 , 5 , 5 };
	float  size[5];
	int    tmp;

	power.vx = 0;
	power.vy = -( BP_PS2_rand() % 25 + 2 );
	power.vz = -( BP_PS2_rand() % 10 + 10 );
	power.vw = 0;
	size[0] = BP_PS2_rand() % 20 + 120;
	size[1] = BP_PS2_rand() % 20 + 95;
	size[2] = BP_PS2_rand() % 20 + 80;
	size[3] = BP_PS2_rand() % 20 + 55;
	size[4] = BP_PS2_rand() % 20 + 25;
	tmp = -(BP_PS2_rand() % 20) - 60;
	NewSmokeNormal( 30 , world, &GLK_shift_mazzule, &power , 4 , 200.0f , &ftmp , 4 );
	MazzleFlash2( 5 , world , &GLK_shift_mazzule , size , bright , ( float )tmp ); // フラッシュ 
	DG_SetPos( world );
	DG_MovePos( &GLK_shift_mazzule );
	DG_GetPos( &fmtemp );
	/* GLK発砲ＳＥ */
	GM_SeSetMode( SD_E_GLOCKF00 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_BOMB );	
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 9.0F ;
			smoke_shift.vy = -520.0F ;
			smoke_shift.vz = 71.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
	}
	}
}

void	AN_MazzleGLK(FMATRIX *world , int silence , int mode )
{
	MazzleGLK( world, silence , mode );
}

// M4A1(ライデン用) silenceに0x100を入れると主観モード 
#define M4A1_MAZZELE_MAX (8) 
void MazzleM4A1( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  M4A1_shift_mazzule = { 20.f , -660.f , 92.f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ M4A1_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ M4A1_MAZZELE_MAX ];
	float  tmp;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	/* マズルフラッシュ 連発*/
	if ( silence & 0x100 ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 4;
		smoke_num = 8;
		tmp = -rnd() * 20.f;
		fwork = DG_ZeroVector;
		fwork.vx = frnd() * 5;
		fwork.vy -= 40;
		_sceVu0AddVector( &fwork , &fwork , &M4A1_shift_mazzule );
		MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	} else {
		mazzle_max = 8;
		smoke_num = 12;
		tmp = -rnd() * 40.f - 15.f;
		fwork = DG_ZeroVector;
		fwork.vx = frnd() * 5;
		fwork.vy -= 40;
		_sceVu0AddVector( &fwork , &fwork , &M4A1_shift_mazzule );
		MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	}
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &M4A1_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	SubMazzleFlash2( &fmtmp ); 
	SubMazzleFlash2( &fmtmp ); 
	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	NewSmokeNormal( smoke_num , world, &M4A1_shift_mazzule, &power , 10 , 240.0f , &ftmp , 5 );

#if 0
	{	
		FVECTOR  ftmp2 = { 100.f , 150.f , 100.f , 0.f };

		status = 0;
		power.vx = 0;
		power.vy = -rnd() * 100.0f;
		power.vz = rnd() * 50.0f;
		power.vw = 0;

		NewSK_Easy_Smoke2Normal( world, &M4A1_shift_mazzule, &status );
//		NewSK_Smoke2Normal( 16 , world, &M4A1_shift_mazzule , &power , 512.0f , &ftmp2 , 4.0f , &status ); 
//		NewConnect_smoke_Mng( world, &M4A1_shift_mazzule , &power , &ftmp2 , 16 , 4.0f , 512.0f , &status ); 
	}
#endif
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL ); 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
 			smoke_shift.vx = 20.0F ;
			smoke_shift.vy = -860.0F ;
			smoke_shift.vz = 92.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleM4A1(FMATRIX *world , int silence , int mode )
{
	MazzleM4A1( world, silence , mode );
}

// M4A1(snake用) silenceに0x100を入れると主観モード 
#define M4A1_SNK_MAZZELE_MAX (8) 
void MazzleM4A1_Snk( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  M4A1_Snk_shift_mazzule = { 20.f , -670.f , 82.f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ M4A1_SNK_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ M4A1_SNK_MAZZELE_MAX ];
	float  tmp;
	int      i;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	/* マズルフラッシュ 連発*/
	if ( silence & 0x100 ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 4;
		smoke_num = 4;
		for ( i = 0 ; i < mazzle_max ; i++ ){
			bright[ i ] /= 2;
			size[ i ] /= 2;
		}
		tmp = -rnd() * 20.f;
	} else {
		mazzle_max = 8;
		smoke_num = 12;
		tmp = -rnd() * 40.f - 15.f;
	}
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , &M4A1_Snk_shift_mazzule );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	NewSmokeNormal( smoke_num , world, &M4A1_Snk_shift_mazzule, &power , 10 , 240.0f , &ftmp , 5 );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &M4A1_Snk_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	SubMazzleFlash2( &fmtmp ); 
	SubMazzleFlash2( &fmtmp ); 
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL ); 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
 			smoke_shift.vx = 20.0F ;
			smoke_shift.vy = -860.0F ;
			smoke_shift.vz = 92.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleM4A1_Snk(FMATRIX *world , int silence , int mode )
{
	MazzleM4A1_Snk( world, silence , mode );
}

// M4A1(ハイテク兵用) 
#define M4A1_HI_MAZZELE_MAX (8) 
void MazzleM4A1_HI( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  M4A1_HI_shift_mazzule = { 20.0f, -675.0f, 80.0f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ M4A1_HI_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ M4A1_HI_MAZZELE_MAX ];
	float  tmp;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	/* マズルフラッシュ 連発*/
	mazzle_max = 8;
	smoke_num = 12;
	tmp = -rnd() * 40.f - 15.f;
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , &M4A1_HI_shift_mazzule );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &M4A1_HI_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	SubMazzleFlash2( &fmtmp ); 
	SubMazzleFlash2( &fmtmp ); 
	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	NewSmokeNormal( smoke_num , world, &M4A1_HI_shift_mazzule, &power , 10 , 240.0f , &ftmp , 5 );
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL ); 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
 			smoke_shift.vx = 20.0F ;
			smoke_shift.vy = -860.0F ;
			smoke_shift.vz = 92.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[3],
					R_RANGE,
					E_RANGE,
					L_COLOR,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleM4A1_HI(FMATRIX *world , int silence , int mode )
{
	MazzleM4A1_HI( world, silence , mode );
}

// アバカン 
#define ABK_MAZZELE_MAX (8) 
void MazzleABK( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	static FVECTOR ftmp = { 50.f , 50.f , 50.f , 0.f };
	static FVECTOR ABK_shift_mazzule = { 19.5f, -730.0f, 77.0f , 0.0f };
	static u_char bright[] = { 40 , 40 , 30 , 25 , 20 } ;
	float  size[5];
	int    tmp;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 30.0f + 10.0f );
	power.vw = 0;
	size[0] = BP_PS2_rand() % 20 + 110;
	size[1] = BP_PS2_rand() % 20 + 95;
	size[2] = BP_PS2_rand() % 20 + 70;
	size[3] = BP_PS2_rand() % 20 + 55;
	size[4] = BP_PS2_rand() % 20 + 50;
	tmp = -(BP_PS2_rand() % 20) - 40;
	NewSmokeNormal( 60 , world, &ABK_shift_mazzule, &power , 4 , 180.0f , &ftmp , 9 );
	MazzleFlash2( 5 , world , &ABK_shift_mazzule , size , bright , ( float )tmp ); // フラッシュ 
	DG_SetPos( world );
	DG_MovePos( &ABK_shift_mazzule );
	DG_GetPos( &fmtemp );

	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL );
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
 			smoke_shift.vx = 19.5F ;
			smoke_shift.vy = -1000.0F ;
			smoke_shift.vz = 77.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					R_RANGE,
					E_RANGE,
					L_COLOR,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleABK(FMATRIX *world , int silence , int mode )
{
	MazzleABK( world, silence , mode );
}

// Singre Action Army 右 
void MazzleSAA( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	SVECTOR         rot , rot_wide;
	FVECTOR         color;
	FVECTOR         SAA_shift_mazzule;
	static FVECTOR ftmp = { 25.f , 25.f , 35.f , 0.f };
	static u_char bright[] = { 30 , 30 , 20 , 15 , 10 } ;
	float  size[5];
	int    tmp;

	if ( silence & EFFECT_FLAGS_SAA_LEFT ){
		SAA_shift_mazzule.vx = -22.5f;
		SAA_shift_mazzule.vy = -322.0f;
		SAA_shift_mazzule.vz = 68.0f;
		SAA_shift_mazzule.vw = 1.0f;
	} else { 
		SAA_shift_mazzule.vx = 17.5f;
		SAA_shift_mazzule.vy = -342.0f;
		SAA_shift_mazzule.vz = 67.7f;
		SAA_shift_mazzule.vw = 1.0f;
	}
	DG_SetPos( world );
	DG_MovePos( &SAA_shift_mazzule );
	DG_GetPos( &fmtemp );
	GV_VecToRot( (FVECTOR *)fmtemp.m[ 1 ], &rot );
	rot.vx -= 2112;
	rot_wide.vx = 406;
	rot_wide.vy = 376;
	rot_wide.vz = 0;

	power.vx = 0;
	power.vy = -( BP_PS2_rand() % 4 + 1 );
	power.vz = -( BP_PS2_rand() % 55 + 15 );
	power.vw = 0;
	size[0] = BP_PS2_rand() % 20 + 200;
	size[1] = BP_PS2_rand() % 20 + 185;
	size[2] = BP_PS2_rand() % 20 + 160;
	size[3] = BP_PS2_rand() % 20 + 145;
	size[4] = BP_PS2_rand() % 20 + 140;
	color.vx = 255.0F;
	color.vy = 64.0F;
	color.vz = 64.0F;
	color.vw = 50.0F;
	tmp = -(BP_PS2_rand() % 20) - 60;
	NewSmokeDemo( 30 , world, &SAA_shift_mazzule, &power , 8 , 200.0f , &ftmp , 4 );
	MazzleFlash2( 5 , world , &SAA_shift_mazzule , size , bright , ( float )tmp ); // フラッシュ 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			if ( silence & EFFECT_FLAGS_SAA_LEFT ){
				smoke_shift.vx = -22.5f;
				smoke_shift.vy = -522.0f;
				smoke_shift.vz = 68.0f;
				smoke_shift.vw = 1.0f;
			} else { 
				smoke_shift.vx = 17.5f;
				smoke_shift.vy = -542.0f;
				smoke_shift.vz = 67.7f;
				smoke_shift.vw = 1.0f;
			}
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
	if ( light_mode & EFFECT_FLAGS_ON_SPARK ){
		NewSpark1( 4 , (FVECTOR *)fmtemp.m[3] , 54.0F , 20.0F , 0.0F , &rot , &rot_wide , &color , 0.9F , 6);
	}
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL );	
}

void	AN_MazzleSAA(FMATRIX *world , int silence , int mode )
{
	MazzleSAA( world, silence , mode );
}

// AKS(ライデン用) silenceに0x100を入れると主観モード 
#define AKS_RAI_MAZZELE_MAX (8) 

void MazzleAKS_RAI( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	FVECTOR  AKS_RAI_shift_mazzule;
	static FVECTOR  ftmp = { 40.f , 40.f , 60.f , 0.f };
	u_char bright[ AKS_RAI_MAZZELE_MAX ] = { 20 , 20 , 20 , 20 , 15 , 15 , 15 , 10 };
	float  size[ AKS_RAI_MAZZELE_MAX ];
	float  tmp;
	int      i;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );

	if ( silence & EFFECT_MAIN_VIEW ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 6;
		smoke_num = 4;
		for ( i = 0 ; i < mazzle_max ; i++ ){
			bright[ i ] -= 4;
			size[ i ] -= 4;
		}
		tmp = -rnd() * 20.f;
	} else {
		mazzle_max = 8;
		smoke_num = 10;
		tmp = -rnd() * 40.f - 15.f;
	}
	// 発射場所をずらす 
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	if ( silence & EFFECT_FLAGS_NO_MZL ){
		AKS_RAI_shift_mazzule.vx = 20.0f;
		AKS_RAI_shift_mazzule.vy = -730.0f;
		AKS_RAI_shift_mazzule.vz = 89.0f;
		AKS_RAI_shift_mazzule.vw = 1.0f;
		NewSmokeNormal( smoke_num , world, &AKS_RAI_shift_mazzule, &power , 10 , 200.0f , &ftmp , 6 );
		fwork = DG_ZeroVector;
		fwork.vy += 10;
		DG_SetPos( world );
		DG_MovePos( &fwork );
		DG_GetPos( &fmtmp );
	} else {
		AKS_RAI_shift_mazzule.vx = 20.0f;
		AKS_RAI_shift_mazzule.vy = -560.0f;
		AKS_RAI_shift_mazzule.vz = 89.0f;
		AKS_RAI_shift_mazzule.vw = 1.0f;
		_sceVu0AddVector( &fwork , &fwork , &AKS_RAI_shift_mazzule );
		NewSmokeNormal( smoke_num , world, &AKS_RAI_shift_mazzule, &power , 10 , 200.0f , &ftmp , 6 );
		MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
		/* サブマズルフラッシュ */
		fwork = DG_ZeroVector;
		fwork.vy += 10;
		_sceVu0AddVector( &fwork , &fwork , &AKS_RAI_shift_mazzule );
		DG_SetPos( world );
		DG_MovePos( &fwork );
		DG_GetPos( &fmtmp );
   		SubMazzleFlash2( &fmtmp ); 
	}
	/* ショットガン発砲ＳＥ */
	//GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL );	 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -930.0f;
			smoke_shift.vz = 89.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleAKS_RAI(FMATRIX *world , int silence , int mode )
{
	MazzleAKS_RAI( world, silence , mode );
}

// M4_gun_demo silenceに0x100を入れると主観モード 海兵隊用 m4a_nm, m4b_gl, m4c_sc 
#define M4_DEMO_MAZZELE_MAX (8) 
void MazzleM4demo_gun( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  M4_gun_demo_shift_mazzule = { 20.0f, -654.0f, 92.0f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ M4_DEMO_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ M4_DEMO_MAZZELE_MAX ];
	float  tmp;
	int      i;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -( rnd() * 5.0f + 3.0f );
	power.vz = -( rnd() * 45.0f + 15.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	/* マズルフラッシュ 連発*/
	if ( silence & 0x100 ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 4;
		smoke_num = 4;
		for ( i = 0 ; i < mazzle_max ; i++ ){
			bright[ i ] /= 2;
			size[ i ] /= 2;
		}
		tmp = -rnd() * 20.f;
	} else {
		mazzle_max = 8;
		smoke_num = 26;
		tmp = -rnd() * 40.f - 15.f;
	}
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , &M4_gun_demo_shift_mazzule );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	NewSmokeNormal( smoke_num , world, &M4_gun_demo_shift_mazzule, &power , 10 , 240.0f , &ftmp , 3 );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &M4_gun_demo_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	SubMazzleFlash2( &fmtmp ); 
	SubMazzleFlash2( &fmtmp ); 
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL );	

	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -654.0f;
			smoke_shift.vz = 92.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[ 3 ],
					1080.0f,
					2200.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleM4demo_gun(FMATRIX *world , int silence , int mode )
{
	MazzleM4demo_gun( world, silence , mode );
}

// M4_gun_demo silenceに0x100を入れると主観モード 海兵隊用 demo_m4_grenade_usm 
void MazzleM4demo_gun_grenade( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  M4_gun_demo_shift_mazzule = { 20.0f, -557.0f, 99.0f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ M4_DEMO_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ M4_DEMO_MAZZELE_MAX ];
	float  tmp;
	int      i;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -( rnd() * 5.0f + 3.0f );
	power.vz = -( rnd() * 45.0f + 15.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	/* マズルフラッシュ 連発*/
	if ( silence & 0x100 ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 4;
		smoke_num = 4;
		for ( i = 0 ; i < mazzle_max ; i++ ){
			bright[ i ] /= 2;
			size[ i ] /= 2;
		}
		tmp = -rnd() * 20.f;
	} else {
		mazzle_max = 8;
		smoke_num = 26;
		tmp = -rnd() * 40.f - 15.f;
	}
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , &M4_gun_demo_shift_mazzule );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	NewSmokeNormal( smoke_num , world, &M4_gun_demo_shift_mazzule, &power , 10 , 240.0f , &ftmp , 3 );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &M4_gun_demo_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	SubMazzleFlash2( &fmtmp ); 
	SubMazzleFlash2( &fmtmp ); 
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL );	

	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -527.0f;
			smoke_shift.vz = 99.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[ 3 ],
					1080.0f,
					2200.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleM4demo_gun_grenade(FMATRIX *world , int silence , int mode )
{
	MazzleM4demo_gun_grenade( world, silence , mode );
}

// M4_gun_demo silenceに0x100を入れると主観モード シールズ用 demo_m4_sel 
void MazzleM4demo_gun_sel( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  M4_gun_demo_shift_mazzule = { 5.0f, -690.0f, 79.0f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ M4_DEMO_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ M4_DEMO_MAZZELE_MAX ];
	float  tmp;
	int      i;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -( rnd() * 5.0f + 3.0f );
	power.vz = -( rnd() * 45.0f + 15.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	/* マズルフラッシュ 連発*/
	if ( silence & 0x100 ){ // 主観モードだったら明るさや数を抑える 
		mazzle_max = 4;
		smoke_num = 4;
		for ( i = 0 ; i < mazzle_max ; i++ ){
			bright[ i ] /= 2;
			size[ i ] /= 2;
		}
		tmp = -rnd() * 20.f;
	} else {
		mazzle_max = 8;
		smoke_num = 26;
		tmp = -rnd() * 40.f - 15.f;
	}
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , &M4_gun_demo_shift_mazzule );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	NewSmokeNormal( smoke_num , world, &M4_gun_demo_shift_mazzule, &power , 10 , 240.0f , &ftmp , 3 );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &M4_gun_demo_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	SubMazzleFlash2( &fmtmp ); 
	SubMazzleFlash2( &fmtmp ); 
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL );	

	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -527.0f;
			smoke_shift.vz = 99.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[ 3 ],
					1080.0f,
					2200.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzleM4demo_gun_sel(FMATRIX *world , int silence , int mode )
{
	MazzleM4demo_gun_sel( world, silence , mode );
}

/* demo_M4_grenade_usm 海兵隊 */
void MazzleM4demo( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	FVECTOR M4_shift_mazzule = { 20.0f, -530.0f, 44.0f , 1.0f };
	static FVECTOR  ftmp = { 100.f , 50.f , 100.f , 0.f };
	SVECTOR         rot_wide;
	SVECTOR         rot;
	FVECTOR         tmp;
	FVECTOR         color;
	int                 i;

	tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	DG_SetPos( world );
	DG_MovePos( &M4_shift_mazzule );
	DG_GetPos( &fmtemp );
	DG_RotVector( &tmp , &tmp, 1 );
	GV_VecToRot( &tmp, &rot );
	rot.vx -= 1900;
	rot_wide.vx = 376;
	rot_wide.vy = 376;
	rot_wide.vz = 0;
	color.vx = 255.0F;
	color.vy = 64.0F;
	color.vz = 64.0F;
	color.vw = 50.0F;

	for ( i = 0 ; i < 3 ; i++ ){
		power.vx = 0;
		power.vy = -( BP_PS2_rand() % 5 + 2 );
		power.vz = -( BP_PS2_rand() % 10 + 20 );
		power.vw = 0;
		NewSmokeNormal( 80 , world, &M4_shift_mazzule, &power , 6 , 300.0f , &ftmp , 4 );
	}
	NewSpark1( 4 , (FVECTOR *)fmtemp.m[3] , 54.0F , 20.0F , 0.0F , &rot , &rot_wide , &color , 0.8F , 4);
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL );

	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -730.0f;
			smoke_shift.vz = 44.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void AN_MazzleM4demo(FMATRIX *world,int silence,int mode)
{
	MazzleM4demo( world, silence,mode )  ;
}

/* demo_M4_grenade_usm ハイテク兵 */
void MazzleM4demo_hi( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	FVECTOR M4_shift_mazzule = { 20.0f , -653.0f , 24.0f , 1.0f };
	static FVECTOR  ftmp = { 100.f , 50.f , 100.f , 0.f };
	SVECTOR         rot_wide;
	SVECTOR         rot;
	FVECTOR         tmp;
	FVECTOR         color;
	int                 i;

	tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	DG_SetPos( world );
	DG_MovePos( &M4_shift_mazzule );
	DG_GetPos( &fmtemp );
	DG_RotVector( &tmp , &tmp, 1 );
	GV_VecToRot( &tmp, &rot );
	rot.vx -= 1900;
	rot_wide.vx = 376;
	rot_wide.vy = 376;
	rot_wide.vz = 0;
	color.vx = 255.0F;
	color.vy = 64.0F;
	color.vz = 64.0F;
	color.vw = 50.0F;

	for ( i = 0 ; i < 3 ; i++ ){
		power.vx = 0;
		power.vy = -( BP_PS2_rand() % 5 + 2 );
		power.vz = -( BP_PS2_rand() % 10 + 20 );
		power.vw = 0;
		NewSmokeNormal( 80 , world, &M4_shift_mazzule, &power , 6 , 300.0f , &ftmp , 4 );
	}
	NewSpark1( 4 , (FVECTOR *)fmtemp.m[3] , 54.0F , 20.0F , 0.0F , &rot , &rot_wide , &color , 0.8F , 4);
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL );
// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -730.0f;
			smoke_shift.vz = 44.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void AN_MazzleM4demo_hi(FMATRIX *world,int silence,int mode)
{
	MazzleM4demo_hi( world, silence,mode )  ;
}

// demo_m4_sel  シールズ用 
void MazzleM4demo_sel( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	FVECTOR M4_shift_mazzule = { 5.0f , -670.0f , 24.0f , 1.0f };
	static FVECTOR  ftmp = { 100.f , 50.f , 100.f , 0.f };
	SVECTOR         rot_wide;
	SVECTOR         rot;
	FVECTOR         tmp;
	FVECTOR         color;
	int                 i;

	tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	DG_SetPos( world );
	DG_MovePos( &M4_shift_mazzule );
	DG_GetPos( &fmtemp );
	DG_RotVector( &tmp , &tmp, 1 );
	GV_VecToRot( &tmp, &rot );
	rot.vx -= 1900;
	rot_wide.vx = 376;
	rot_wide.vy = 376;
	rot_wide.vz = 0;
	color.vx = 255.0F;
	color.vy = 64.0F;
	color.vz = 64.0F;
	color.vw = 50.0F;

	for ( i = 0 ; i < 3 ; i++ ){
		power.vx = 0;
		power.vy = -( BP_PS2_rand() % 5 + 2 );
		power.vz = -( BP_PS2_rand() % 10 + 20 );
		power.vw = 0;
		NewSmokeNormal( 80 , world, &M4_shift_mazzule, &power , 6 , 300.0f , &ftmp , 4 );
	}
	NewSpark1( 4 , (FVECTOR *)fmtemp.m[3] , 54.0F , 20.0F , 0.0F , &rot , &rot_wide , &color , 0.8F , 4);
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL ); 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -730.0f;
			smoke_shift.vz = 44.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void AN_MazzleM4demo_sel(FMATRIX *world,int silence,int mode)
{
	MazzleM4demo_sel( world, silence,mode )  ;
}

// demo_M4_grenade_usm -snake 
void MazzleM4demo_Snk( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	FVECTOR M4_SNK_shift_mazzule = { 20.0f ,-653.0f, 27.0f , 1.0f };
	static FVECTOR  ftmp = { 100.f , 50.f , 100.f , 0.f };
	SVECTOR         rot_wide;
	SVECTOR         rot;
	FVECTOR         tmp;
	FVECTOR         color;
	int                 i;

	tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	DG_SetPos( world );
	DG_MovePos( &M4_SNK_shift_mazzule );
	DG_GetPos( &fmtemp );
	DG_RotVector( &tmp , &tmp, 1 );
	GV_VecToRot( &tmp, &rot );
	rot.vx -= 1900;
	rot_wide.vx = 376;
	rot_wide.vy = 376;
	rot_wide.vz = 0;
	color.vx = 255.0F;
	color.vy = 64.0F;
	color.vz = 64.0F;
	color.vw = 50.0F;

	for ( i = 0 ; i < 3 ; i++ ){
		power.vx = 0;
		power.vy = -( BP_PS2_rand() % 5 + 2 );
		power.vz = -( BP_PS2_rand() % 10 + 20 );
		power.vw = 0;
		NewSmokeNormal( 80 , world, &M4_SNK_shift_mazzule, &power , 6 , 300.0f , &ftmp , 4 );
	}
	NewSpark1( 4 , (FVECTOR *)fmtemp.m[3] , 54.0F , 20.0F , 0.0F , &rot , &rot_wide , &color , 0.8F , 4);
	/* ショットガン発砲ＳＥ */
//	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_NORMAL ); 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -730.0f;
			smoke_shift.vz = 44.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void AN_MazzleM4demo_Snk(FMATRIX *world,int silence,int mode)
{
	MazzleM4demo_Snk( world, silence,mode )  ;
}

// SPS enemy 
void MazzleSPS( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtemp;
	FVECTOR 		power;
	FVECTOR SPS_shift_mazzule = { 20.0f, -880.0f , 89.7f , 1.0f };
	static FVECTOR  ftmp = { 100.f , 50.f , 150.f , 0.f };
	static FVECTOR  base_speed = { 0.0f , 0.0f , -100.0f , 0.0f };	
	SVECTOR         rot_wide;
	SVECTOR         rot;
	FVECTOR         tmp;
	FVECTOR         color;

	tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	DG_SetPos( world );
	DG_MovePos( &SPS_shift_mazzule );
	DG_GetPos( &fmtemp );
	DG_RotVector( &tmp , &tmp, 1 );
	GV_VecToRot( &tmp, &rot );
	rot.vx -= 1900;
	rot_wide.vx = 376;
	rot_wide.vy = 376;
	rot_wide.vz = 0;
	color.vx = 255.0F;
	color.vy = 64.0F;
	color.vz = 64.0F;
	color.vw = 50.0F;

	{
		float raute;

		DG_SetPos( &fmtemp );
		raute = ( rnd() * 2.0f + 1.5f);
		_sceVu0ScaleVector( &power , &base_speed , raute );
		DG_RotVector( &power , &power , 1 );
		NewClaySmokeGray( 6 , &fmtemp , &DG_ZeroVector , &power , 10 , 220.0f , &ftmp , 16 , 4 );
		NewClaySmokeWhite( 4 , &fmtemp , &DG_ZeroVector , &power , 10 , 540.0f , &ftmp , 16 , 8 );
		raute = 1.2f;
		_sceVu0ScaleVector( &power , &power , raute );
		NewClaySmokeFire( 5 , &fmtemp , &DG_ZeroVector , &power , 20 , 100.0f , &ftmp , 128 , 4 );
		NewClaySmokeFire2( 4 , &fmtemp , &DG_ZeroVector , &power , 20 , 82.0f , &ftmp , 84 , 4 );
	}
//	NewSpark1( 4 , (FVECTOR *)fmtemp.m[3] , 54.0F , 20.0F , 0.0F , &rot , &rot_wide , &color , 0.8F , 4); 
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtemp.m[3], GM_SEMODE_BOMB );
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 20.0f;
			smoke_shift.vy = -730.0f;
			smoke_shift.vz = 44.0f;
			smoke_shift.vw = 1.0f;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtemp );
#if 0
/// yano
			DG_SetTmpLight2(
					(FVECTOR *)fmtemp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
#endif
		}
	}
}

void AN_MazzleSPS(FMATRIX *world,int silence,int mode)
{
	MazzleSPS( world, silence,mode )  ;
}

// PSG(共用) 
#define PSG_MAZZELE_MAX (8) 
void MazzlePSG( FMATRIX *world, int silence, int light_mode )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  PSG_shift_mazzule = { 17.5f , -954.0f , 89.7f , 0.0f };
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ PSG_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ PSG_MAZZELE_MAX ];
	float  tmp;
	int   smoke_num;
	int  mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	mazzle_max = 8;
	smoke_num = 12;
	tmp = -rnd() * 40.f - 15.f;
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , &PSG_shift_mazzule );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , &PSG_shift_mazzule );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	NewSmokeNormal( smoke_num , world, &PSG_shift_mazzule, &power , 10 , 240.0f , &ftmp , 5 );
	/* ショットガン発砲ＳＥ */
	//GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL );	 
	// テンプライト 
	{
		FVECTOR smoke_shift;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
 			smoke_shift.vx = 20.0F ;
			smoke_shift.vy = -860.0F ;
			smoke_shift.vz = 92.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &fmtmp );
			DG_SetTmpLight2(
					(FVECTOR *)fmtmp.m[3],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

void	AN_MazzlePSG(FMATRIX *world , int silence , int mode )
{
	MazzlePSG( world, silence , mode );
}


#define OFFSET_DSE (-80.0f)

/* デザートイーグル(サイレンサー無) */
void MazzleDSE( FMATRIX *world, int silence, int light_mode )
{
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	static FVECTOR  shift[] = {
		{17.5F,-275.0F + OFFSET_DSE,60.0F,1.0F},
		{17.5F,-375.0F + OFFSET_DSE,60.0F,1.0F},
		{17.5F,-430.0F + OFFSET_DSE,60.0F,1.0F},
		{17.5F,-460.0F + OFFSET_DSE,60.0F,1.0F}
	} ;
	FVECTOR       Dse_shift_mazzle;
	int           tmp;
	static float  size[] = {150.0F,100.0F,75.0F,50.0F} ;
	static u_char bright[] = {56,20,10,6} ;

	FMATRIX	 mat ;

	if ( silence & EFFECT_FLAGS_NO_MZL ){
		Dse_shift_mazzle.vx = 17.5f;
		Dse_shift_mazzle.vy = -466.5f + OFFSET_DSE;
		Dse_shift_mazzle.vz = 70.5f;
		Dse_shift_mazzle.vw = 1.0f;
	} else {
		Dse_shift_mazzle.vx = 17.5f;
		Dse_shift_mazzle.vy = -275.0f + OFFSET_DSE;
		Dse_shift_mazzle.vz = 60.0f;
		Dse_shift_mazzle.vw = 1.0f;

		tmp = -( irnd() % 20 ) - 60;
		/* マズルフラッシュ 単発 */
		MazzleFlash(4,world,shift,size,bright) ;
		MazzleFlash2( 4 , world , &Dse_shift_mazzle , size , bright , ( float )tmp );
	}
	
	/* 火花 と テンプライト*/
	DG_SetPos( world ) ;
	DG_MovePos( &shift[0] ) ;
	DG_GetPos( &mat ) ;

	if ( !( silence & EFFECT_FLAGS_NO_MZL ) ){
	{
		SVECTOR  rot ;
		SVECTOR  rot_wide ;
		FVECTOR  color ;

		rot.vx = 256 ;
		rot.vy = 0 ;
		rot.vz = 0 ;
		rot_wide.vx = 1536 ;
		rot_wide.vy = 4096 ;
		rot_wide.vz = 0 ;
		color.vx = 50.0F ;
		color.vy = 25.0F ;
		color.vz = 25.0F ;
		color.vw = 100.0F ;
		NewSpark2(4,&mat,36.0F,16.0F,0.5F,&rot,&rot_wide,&color,1.0F,6) ;
	}
	}

	{
		FVECTOR   smoke_shift ;
		FVECTOR   power;

		smoke_shift.vx = 17.5F ;
		smoke_shift.vy = -360.0F + OFFSET_DSE ;
		smoke_shift.vz = 20.0F ;
		smoke_shift.vw = 1.0F ;
		power.vx = 0.F;
		power.vy = 0.f;
		power.vz = -( 40.0F + rnd() * 5.0F );
		power.vw = 1.0F ;
//		NewSmokeNormal( 40 , world, &smoke_shift , &smoke_speed , 10 , 160.0f , &ftmp , 4 ); 
		if ( silence & EFFECT_MAIN_VIEW ) { // 主観  
			NewSmokeNormal( 40 , world, &smoke_shift, &power , 10 , 160.0f , &ftmp , 4 );
			// 後にさがる煙 
			power.vz = ( rnd() * 60.0f + 110.0f );
			NewSmokeNormal( 10 , world, &smoke_shift, &power , 10 , 320.0f , &ftmp , 4 );
		} else {
			NewSmokeNormal( 40 , world, &smoke_shift, &power , 10 , 160.0f , &ftmp , 4 );
		}

	}
	{
		FVECTOR   smoke_shift ;
		if ( light_mode & EFFECT_FLAGS_ON_TMP ){
			smoke_shift.vx = 17.5F ;
			smoke_shift.vy = -560.0F + OFFSET_DSE ;
			smoke_shift.vz = 60.0F ;
			smoke_shift.vw = 1.0F ;
			DG_SetPos( world ) ;
			DG_MovePos( &smoke_shift ) ;
			DG_GetPos( &mat );
			DG_SetTmpLight2(
				    (FVECTOR *)mat.m[ 3 ],
					780.0f,
					1560.0f,
					0xc0 | 0xa0 << 8 | 0x50 <<16,
					LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
	}
}

/* デザートイーグルのマズル */
void AN_MazzleDSE(FMATRIX *world, int  silence)
{
	MazzleDSE(world,0,0) ;
	return ;
}


// cypher 
#define CYPHER_MAZZELE_MAX (8) 
void MazzleMeca( FMATRIX *world, FVECTOR *shift )
{
	FMATRIX			fmtmp;
	FVECTOR 		power;
	FVECTOR         fwork;
	static FVECTOR  ftmp = { 50.f , 50.f , 50.f , 0.f };
	u_char bright[ CYPHER_MAZZELE_MAX ] = { 25 , 25 , 25 , 25 , 20 , 20 , 20 , 15 };
	float  size[ CYPHER_MAZZELE_MAX ];
	float  tmp;
	int     smoke_num;
	int    mazzle_max;

	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	size[0] = ( float )( irnd() % 40 + 60 );
	size[1] = ( float )( irnd() % 40 + 60 );
	size[2] = ( float )( irnd() % 40 + 60 );
	size[3] = ( float )( irnd() % 40 + 60 );
	size[4] = ( float )( irnd() % 40 + 55 );
	size[5] = ( float )( irnd() % 40 + 55 );
	size[6] = ( float )( irnd() % 40 + 50 );
	size[7] = ( float )( irnd() % 40 + 50 );
	mazzle_max = 8;
	smoke_num = 12;
	tmp = -rnd() * 40.f - 15.f;
	fwork = DG_ZeroVector;
	fwork.vx = frnd() * 5;
	fwork.vy -= 40;
	_sceVu0AddVector( &fwork , &fwork , shift );
	MazzleFlash2( mazzle_max , world , &fwork , size , bright , tmp );
	/* サブマズルフラッシュ */
	fwork = DG_ZeroVector;
	fwork.vy += 10;
	_sceVu0AddVector( &fwork , &fwork , shift );
	DG_SetPos( world );
	DG_MovePos( &fwork );
	DG_GetPos( &fmtmp );
	power.vx = 0;
	power.vy = -rnd();
	power.vz = -( rnd() * 40.0f + 10.0f );
	power.vw = 0;
	NewSmokeNormal( smoke_num , world, shift, &power , 10 , 240.0f , &ftmp , 5 );
#if 0
	/* ショットガン発砲ＳＥ */
	GM_SeSetMode( SD_E_SHTFRE01 , (FVECTOR *)fmtmp.m[3], GM_SEMODE_NORMAL );	
	// テンプライト 
	{
		FVECTOR smoke_shift;

		smoke_shift.vx = 20.0F ;
		smoke_shift.vy = -860.0F ;
		smoke_shift.vz = 92.0F ;
		smoke_shift.vw = 1.0F ;
		DG_SetPos( world ) ;
		DG_MovePos( &smoke_shift ) ;
		DG_GetPos( &fmtmp );
		DG_SetTmpLight2(
				(FVECTOR *)fmtmp.m[3],
				780.0f,
				1560.0f,
				0xc0 | 0xa0 << 8 | 0x50 <<16,
				LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
	}
#endif
}

void	AN_MazzleMeca( FMATRIX *world , FVECTOR *shift )
{
	MazzleMeca( world , shift );
}

enum {
	MAZ_ID_M92 = 0,
	MAZ_ID_USP,
	MAZ_ID_FMS,
	MAZ_ID_MKR,
	MAZ_ID_AKS,
	MAZ_ID_P90,
	MAZ_ID_GLK,
	MAZ_ID_M4A1,
	MAZ_ID_ABK,
	MAZ_ID_AKS_RAI,
	MAZ_ID_SOCOM,
	MAZ_ID_PSG,
	MAZ_ID_M4A1_SNK,
};

void NewCallMazzle( int maz_id, FMATRIX *world, int silence, int light_mode )
{
	switch( maz_id ){
	case MAZ_ID_M92:
		MazzleM92( world, silence, light_mode );
		break;
	case MAZ_ID_USP:
		MazzleUSP( world, silence, light_mode );
		break;
	case MAZ_ID_FMS:
		MazzleFAMAS( world, silence, light_mode );
		break;
	case MAZ_ID_MKR:
		MazzleMKR( world, silence, light_mode );
		break;
	case MAZ_ID_AKS:
		MazzleAKS( world, silence, light_mode );
		break;
	case MAZ_ID_P90:
		MazzleP90( world, silence, light_mode );
		break;
	case MAZ_ID_GLK:
		MazzleGLK( world, silence, light_mode );
		break;
	case MAZ_ID_M4A1:
		MazzleM4A1( world, silence, light_mode );
		break;
	case MAZ_ID_ABK:
		MazzleABK( world, silence, light_mode );
		break;
	case MAZ_ID_AKS_RAI:
		MazzleAKS_RAI( world, silence, light_mode );
		break;
	case MAZ_ID_SOCOM:
		MazzleSOCOM( world, silence, light_mode );
		break;
	case MAZ_ID_PSG:
		MazzlePSG( world, silence, light_mode );
		break;
	case MAZ_ID_M4A1_SNK:
		MazzleM4demo_Snk( world, silence, light_mode );
		break;
	default : break;
	}
}

