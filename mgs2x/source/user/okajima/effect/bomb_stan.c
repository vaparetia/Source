//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_stan.c
	スタングレネードエフェクト
	2000/05/15 S.Okajima
	$Id: bomb_stan.c,v 1.1.1.3 2002/11/19 11:47:01 Yoshizawa1 Exp $

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
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../effect/bomb_vibration.h"

#define	SCREEN_NEAR	( 51.0f )

#define	JET_NUM		(4)
#define	MAX_VEC		(150.0f)

#define	N_PRIMS		(1)
#define	N_LIGHT		(2)
#define	N_VERTS		(4*N_LIGHT)
#define	N_POLYS		(N_LIGHT)

#define	DUST_GRAVITY	( P_GRAVITY * 0.125f )

#define	ANGLE_STEP	(TPI * 0.0001f)
#define	RADIUS		(20.0f)

#define	SHIFT_HZD	(10.0f)

#define	SHIFT_UPPER	(10.0f)

#define	COL_R_SUB	(4)
#define	COL_G_SUB	(4)
#define	COL_B_SUB	(4)
#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)

#define	COL_R_HOLA	(32)
#define	COL_G_HOLA	(32)
#define	COL_B_HOLA	(32)
#define	ALP_MAX		(64)

#define	SIZE_MIN	(1.0f)
// AS(JM) - Undeffing size max before redefining it!
#ifdef SIZE_MAX
#  undef SIZE_MAX
#endif
#define	SIZE_MAX	(30.0f)

#define	LENGTH_MIN	(1000.0f)
#define	LENGTH_MAX	(60000.0f)

#define	FLASH_LENGTH	(2)
#define	FLASH_LENGTH_D	(FLASH_LENGTH * 5)

#define	LIFE_TIME		(60 * 4)
#define	FLOOR_CHECK_TIME	(15)

#define	VEC_SCALE		(0.001f)

#define	INIT_MULTIPLE_NUM	(2)
#define	ZOOM_RATIO			(1.012f)
#define	SEARCH_HZX			(1000)

extern float OK_PutBombVibration( FVECTOR *pos, float ratio );
extern void *NewShakeCamera( int chanl, int intense, int time );
extern void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos );
extern void *NewFlyingSmoke2( FVECTOR *pos, FVECTOR *vector, int life );
extern void *NewBombKasu( FVECTOR *pos, SVECTOR *pole_rot );
extern void *NewFadeInOut_Demo( int col_r, int col_g, int col_b, int col_a, int time );
extern void *NewFadeInOutForce_Demo( int ini_r, int ini_g, int ini_b, int ini_a,
                                     int col_r, int col_g, int col_b, int col_a, int time );
extern void *NewContrastForce_Demo( int ini_r, int ini_g, int ini_b, int ini_a, 
	                                int col_r, int col_g, int col_b, int col_a, int nega_posi_flag, int time );
extern void *NewFlushOnCamera( int life );

int	ok_bomb_stun_num=0;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			viblation_count_h;
	int			viblation_count_l;
	int			my_num;
	int			off_flag;
	int			life;

	float		rad;

	FVECTOR		pos;

	int			mode;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	if( work->my_num == ok_bomb_stun_num ){
		switch( work->off_flag ){
		  case 0:
//printf("ON :%d::::::%d\n",work->my_num,ok_bomb_stun_num);
			work->off_flag = 1;
			if(work->mode==0){
				GM_SetGameStatus( STATE_STUN );
			}
			break;
		  case 1:
//printf("OFF:%d::::::%d\n",work->my_num,ok_bomb_stun_num);
			work->off_flag = 2;
			GM_ResetGameStatus( STATE_STUN );
			break;
		}
	}else{
		work->my_num = -1;
	}


	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		if( work->viblation_count_h > 0 ){
			work->viblation_count_h--;
			GM_SetVibration1( 0, irnd()%2 );
		}
		if( work->viblation_count_l > 0 ){
			work->viblation_count_l--;
			GM_SetVibration2( 0, (int)( (float)(240 + irnd()%16)*OK_PutBombVibration( &work->pos, 1.0f ) ) );
		}
	}

	 work->life--;
	if( work->viblation_count_h <=0
	 && work->viblation_count_l <=0
	 && work->life < 0 ){
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	ok_bomb_stun_num--;
	if( ok_bomb_stun_num <= 0 ){
		ok_bomb_stun_num  = 0;
	}

	// 念の為
	GM_ResetGameStatus( STATE_STUN );
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, int mode, int life, int alpha )
{
	int	i;
	HZX_GROUP_ID	map_id;
	float		flr_height[2];
	float		rad;
	float		ftemp;
	FVECTOR		fvtemp;
	int			seg_num;
	int			atr[2] ;
	HZX_SEG		seg[2] ;
	SVECTOR		pole_rot;
	SVECTOR		svtemp;

	work->mode = mode;
	work->life = life;

	DG_COPY_VEC( &work->pos, pos );

	//----------------------------------
	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	if( HZX_LevelHazardCheck( map_id, pos, HZX_CHK_ALL, 0 ) & 1 ){
		HZX_GetLevelHeight( flr_height );
	}

	seg_num = HZX_NearHazardCheck( map_id,
					pos,
					SEARCH_HZX,
					HZX_CHK_ALL,0,
					SEARCH_HZX);

	/* 反射ベクトル */
	pole_rot.vy = 0;
	pole_rot.vz = 0;
	rad = 0.0f;	/* Ｘ回転の合計を計算する */
	if( seg_num ){
		HZX_GetNearHazard( seg, atr ) ;
		HZX_GetReactVector( &fvtemp );

		ftemp = atan2f( fvtemp.vx, fvtemp.vz ) ;
		pole_rot.vy = 4095 & ( short )( ( ftemp * 2048.0f / PI ) + 0.5f );

		rad = PI * 0.5f;
	}
	pole_rot.vx = 4095 & ( short )( ( rad * 1024.0f / PI ) + 0.5f );

	NewShakeCamera2( 0, 512, 10, pos );
	NewBombKasu( pos, &pole_rot );

//	if( ok_bomb_stun_num==1 ){
	if( ok_bomb_stun_num < 3 ){
		work->viblation_count_h = VIBLATION_LENGTH_1_H;
		work->viblation_count_l = VIBLATION_LENGTH_1_L;
	}else{
		work->viblation_count_h = VIBLATION_LENGTH_2_H;
		work->viblation_count_l = VIBLATION_LENGTH_2_L;
	}

	for(i=0; i<JET_NUM; i++){
		fvtemp.vx = 0.0f;
		fvtemp.vy = rnd()*MAX_VEC*0.5f + MAX_VEC*0.5f;
		fvtemp.vz = 0.0f;
		svtemp.vx = irnd()%2048;
		svtemp.vy = irnd()%4096;
		svtemp.vz = 0;
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( &fvtemp, &fvtemp, 1 );
		NewFlyingSmoke2( pos, &fvtemp, 20 );
	}

//	NewFadeInOutForce_Demo( 255, 255, 255, 32,  255, 255, 255, 0, life );
//	NewFadeInOutForce_Demo( 255, 255, 255, 96,  255, 255, 255, 96, life );
	NewFadeInOutForce_Demo( 255, 255, 255, alpha,  255, 255, 255, 0, life );

//	NewContrastForce_Demo( 32, 32, 32, 255,     0,  0,  0, 128, 0, life/2 );

	NewFlushOnCamera( 15 );
//	NewFlushOnCamera( 60 );

	return 0 ;
}

/*
mode:0:フラグを立てる
mode:1:フラグを立てない
*/
void *NewStanEffect( FVECTOR *pos, int mode )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		ok_bomb_stun_num++;
		work->my_num = ok_bomb_stun_num;
		work->off_flag = 0;

//		if ( GetResources( work, pos, mode, LIFE_TIME, 48 ) < 0 ) {
		if ( GetResources( work, pos, mode, LIFE_TIME, 64 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*
mode:0:フラグを立てる
mode:1:フラグを立てない
life:画面効果持続時間（スタンフラグではない）現在は LIFE_TIME で 指定されている。
*/
void *NewStanEffect_LifeVariance( FVECTOR *pos, int mode, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		ok_bomb_stun_num++;
		work->my_num = ok_bomb_stun_num;
		work->off_flag = 0;

		if ( GetResources( work, pos, mode, life, 128 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

