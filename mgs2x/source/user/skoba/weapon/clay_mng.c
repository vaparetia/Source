//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	clay_eff.c
	クレイモア爆発（内容物エフェクト）
	2001/05/03 S.Kobayashi
	$Id: clay_mng.c,v 1.1.1.3 2002/11/19 11:50:27 Yoshizawa1 Exp $

*/
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"
#include	"../test/etc.h"
#include    "../test/etc.h"


#define	VIBLATION_LENGTH_1		(300)
#define	VIBLATION_LENGTH_2		(30)
/*----------------------------------------------------------------*/
typedef	struct _work {
	GV_ACT_EX	  actor ;

	FMATRIX		  *pWorld;
	FVECTOR	      shift;
	FVECTOR       power;

	int			  life;
	int           alpha; // ベース
	int           num;
	int           flag;
	u_char        vibration_frame_high[ 6 ];
	u_char        vibration_frame_low[ 6 ];

	void (*act)( struct _work * );
} Work ;

// 外部変数
extern	FVECTOR	G_wind ;				/* 風速（単位フレームに移動する距離） */
extern	SVECTOR	G_wind_rot ;			/* 風向 */
extern	int		G_wind_intense ;		/* 強さ */
extern	int		G_wind_intense_max ;	/* 強さ */
extern	int		G_wind_sw ;				/* 突風発生中に立つ */
extern	FMATRIX	G_wind_matrix ;

extern void *NewClayBomb( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , long64 );
extern void *NewClaySmokeNormal( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeGray( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeFire( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );
extern void *NewClaySmokeFire2( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , FVECTOR * , float , int );

static int before_frame = 0; // いみはない

static u_char VibrationHigh[] = { 128 , 6 , 64 , 6 , 48 , 6 , 0 , 0 };
static u_char VibrationLow[] = { 128 , 3 , 0 , 0 , 0, 0 /* BP - Added extra ,0 */};
static u_char VibrationHigh_work[ 6 ];
static u_char VibrationLow_work[ 6 ];

enum {
	SK_VIBE_HIGH_ON = 0x1 ,
	SK_VIBE_LOW_ON = 0x2 ,
};

extern 	void	NewPadVibration( u_char *, int ) ;   

static void NormalAct( Work *pWork )
{
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		if ( pWork->flag & SK_VIBE_HIGH_ON ){
			NewPadVibration( VibrationLow_work , 1 | VAR_FLAG_FORCE ) ;
			pWork->flag &= ~SK_VIBE_HIGH_ON;
		}
		if ( pWork->flag & SK_VIBE_LOW_ON ){
			NewPadVibration( VibrationHigh_work , 2 | VAR_FLAG_FORCE ) ;
			pWork->flag &= ~SK_VIBE_LOW_ON;
		}
	}
	if ( !( pWork->flag & SK_VIBE_LOW_ON ) && !( pWork->flag & SK_VIBE_HIGH_ON ) ){
		GV_DestroyActor( pWork );
	}
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
}

/*----------------------------------------------------------------*/
static int GetResources( Work *pWork , int raute , long64 flag )
{
	static FVECTOR  sftmp = { 100.f , 200.f , 200.f , 0.f };
	static FVECTOR  shift = { -800.0f , 0.0f , 0.0f , 0.0f };
	FMATRIX world;
	FVECTOR speed;
	FVECTOR fvtmp;
	SVECTOR rot;
	float alpha;
	float ftmp;
	float log;
	int   i;

	world = *pWork->pWorld;
	ftmp = 1.0f / ( ( float )raute );
	alpha = ( rnd() * 32.0f + 56.0f); // 24.0f
	// 方向の変更
	rot.vz = 0; // 横方向
	rot.vy = ( short )2176;   // たて方向
	rot.vx = 0;
	DG_SetPos2( &DG_ZeroVector , &rot );
	DG_RotVector( &pWork->power , &speed , 1 ); 
	// 登録
	if ( NewClayBomb( 24 , pWork->pWorld , &pWork->shift , &speed , pWork->life , alpha , flag ) == NULL ){
		SK_Err("clay Bomb\0");
		return ( -1 );
	}
	if ( NewClaySmokeNormal( 5 , &world , &shift, &speed , 6 , 1000.0f , &sftmp , 12 , 20 ) == NULL ){
		SK_Err("clay normal\0");
		return ( -1 );
	}
	if ( NewClaySmokeGray( 5 , &world , &shift, &speed , 6 , 1000.0f , &sftmp , 20 , 10 ) == NULL ){
		SK_Err("clay gray\0");
		return ( -1 );
	}
	if ( NewClaySmokeFire( 4 , &world , &shift, &speed , 6 , 200.0f , &sftmp , 164 , 10 ) == NULL ){
		SK_Err("clay fire\0");
		return ( -1 );
	}
	if ( NewClaySmokeFire2( 4 , &world , &shift, &speed , 6 , 300.0f , &sftmp , 104 , 10 ) == NULL ){
		SK_Err("clay fire2\0");
		return ( -1 );
	}

	// vibe
	// 距離
	_sceVu0SubVector( &fvtmp , ( FVECTOR * )GM_PlayerBody->objs->objs[ 0 ].world.m[ 3 ] , ( FVECTOR * )world.m[ 3 ] );
	log = fpu_Sqrt( _sceVu0InnerProduct( &fvtmp , &fvtmp ) );
	if ( log <= 2000.0f ){
		ftmp = 1.0f;
	} else if ( log <= 5000.0f ){
		ftmp = 0.6f;
	} else {
		ftmp = 0.3f;
	}
	for ( i = 0 ; i < 6 ; i++ ){
		if ( i & 0x1 ){ // % 2  と同じ
			VibrationHigh_work[ i ] = VibrationHigh[ i ] * ftmp;
			VibrationLow_work[ i ] = VibrationLow[ i ] * ftmp;
		} else {
			VibrationHigh_work[ i ] = VibrationHigh[ i ];
			VibrationLow_work[ i ] = VibrationLow[ i ];
		}
	}
	pWork->flag = SK_VIBE_HIGH_ON | SK_VIBE_LOW_ON;

	return 0 ;
}

static void *NewClayBombMng( int num , FMATRIX *world, FVECTOR *shift , FVECTOR *pPower , int life, float alpha , long64 flag )
{
	Work		*pWork ;
	int          raute;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->pWorld = world;
		DG_COPY_VEC( &pWork->shift, shift );
		DG_COPY_VEC( &pWork->power , pPower );
		pWork->num = num;
		pWork->life = life;
		pWork->alpha = alpha;
		pWork->act = ( void * )NormalAct;
		if ( GV_Time - before_frame <= 1 ){
			raute = 6;
		} else {
			raute = 1;
		}
		before_frame = GV_Time;
		if ( GetResources( pWork , raute , flag ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewClayBombNormal( int num , FMATRIX *world , FVECTOR *shift , FVECTOR *pPower , int life , float alpha )
{
	return NewClayBombMng( num , world, shift , pPower , life, alpha , 0 );
}

void *NewClayBombFlag( int num , FMATRIX *world , FVECTOR *shift , FVECTOR *pPower , int life , float alpha , long64 flag )
{
	return NewClayBombMng( num , world, shift , pPower , life, alpha , flag );
}
