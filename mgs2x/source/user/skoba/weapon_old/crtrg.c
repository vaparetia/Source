//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	crtrg.c
	カートリッジ関連
	1999/11/04 S.Okajima
	$Id: crtrg.c,v 1.1.1.3 2002/11/19 11:50:33 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <libvu0.h>
#include <sifdev.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include    "../test/etc.h"


#if 1
#define		CARTRIDGE_NAME0	MDL_AKS_AMO // GV_StrCode( "aks_amo" ) 
#define		CARTRIDGE_NAME1	MDL_FAMAS_AMO // GV_StrCode( "fms_amo" ) 
#define		CARTRIDGE_NAME2	MDL_USP_AMO // GV_StrCode( "usp_amo" ) 
#define		CARTRIDGE_NAME3	MDL_P90_AMO // GV_StrCode( "p90_amo" ) 
#define		CARTRIDGE_NAME4	MDL_M4A_RAI_AMO // GV_StrCode( "m4a_rai_amo" ) 
#define		CARTRIDGE_NAME5	MDL_GLOCK_AMO // GV_StrCode( "glk_amo" ) 
#define		CARTRIDGE_NAME6	MDL_ABAKAN_AMO // GV_StrCode( "abk_amo" )  
#define		CARTRIDGE_NAME7	MDL_AKS_AMO // GV_StrCode( "aks_amo" ) 
#define     CARTRIDGE_NAME8	MDL_SOCOM_AMO // GV_StrCode( "scm_amo" ) 
#define     CARTRIDGE_NAME9	MDL_M4B_GRD_AMO	// GV_StrCode( "m4a_amo_htc" ) 
#define     CARTRIDGE_NAME10 MDL_RGB_AMO	// GV_StrCode( "rgb_amo.mdl") 
#define     CARTRIDGE_NAME11 MDL_SPS_AMO	// GV_StrCode( "sps_amo.mdl") 
#endif

#define		BOUND_RATIO	( 0.7f ) 
#define		BOUND_WIDTH	( 500.0f ) 

#define		MAX_PAT		( 12 ) 

static int strcode[ MAX_PAT ] = {
	CARTRIDGE_NAME0,
	CARTRIDGE_NAME1,
	CARTRIDGE_NAME2,
	CARTRIDGE_NAME3,
	CARTRIDGE_NAME4,
	CARTRIDGE_NAME5,
	CARTRIDGE_NAME6,
	CARTRIDGE_NAME7,
	CARTRIDGE_NAME8,
	CARTRIDGE_NAME9,
	CARTRIDGE_NAME10,
	CARTRIDGE_NAME11,
};
/*
0:敵兵ＡＫＳ
1:プレイヤーFAMAS
2:プレイヤーUSP
3:天狗兵 p90
4:ライデンM4A1
5:ファットマンGLK
6:アバカン
7:ライデンAKS
8:ライデンSocom
9:ハイテク兵m4a1
10:RGB6
11:SPS（ショットガン）
*/


extern	void *NewCartridgeFall(
			FMATRIX*    mat,		/* 呼ばれた時点でのマトリックス */
			int			pat,		/* 人体と武器の種類 */
			FMATRIX*    light,
			int			map
			);

static	ALIGN16_PRE int		ALIGN16_POST	joint_num_data[MAX_PAT*2]={
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10,
	6, 10
};

static	ALIGN16_PRE FVECTOR	ALIGN16_POST	fv_shift_data[MAX_PAT*4]={
	{  20.0f, -264.0f, -27.0f, 0.0f },	//銃 aks 
	{ -17.5f, -102.5f, -12.5f, 0.0f },	//左手 
	{  17.5f, -102.5f, -12.5f, 0.0f },	//右手 
	{  17.5f, -102.5f, -12.5f, 0.0f },	//左手[パターン２] 

	{  19.5f,   23.0f, -31.0f, 0.0f },	//銃 famas 
	{ -15.5f, -103.0f,  -2.0f, 0.0f },	//左手 
	{  19.5f, -103.0f,  -2.0f, 0.0f },	//右手 
	{  19.5f, -103.0f,  -2.0f, 0.0f },	//左手[パターン２] 

	{  17.5f, -115.9f,  -1.2f, 0.0f },	//銃 usp 
	{ -17.5f, -110.0f, -11.2f, 0.0f },	//左手 
	{  17.5f, -115.9f,  -1.2f, 0.0f },	//右手 
	{ -67.5f, -100.0f,   0.0f, 0.0f },	//左手[パターン２] 

	{  17.5f, -112.5f,  85.0f, 0.0f },	//銃 p90 
	{ -25.0f, -102.0f, -70.0f, 0.0f },	//左手 
	{  17.5f, -112.5f,   85.f, 0.0f },	//右手 
	{ -67.5f, -100.0f,   0.0f, 0.0f },	//左手[パターン２] 

	{  20.0f, -225.0f,   0.0f, 0.0f },	//銃 m4a1 
	{ -17.0f,  -90.0f,   0.0f, 0.0f },	//左手 
	{  20.0f, -225.0f,   0.0f, 0.0f },	//右手 
	{ -17.0f,  -90.0f,   0.0f, 0.0f },	//左手[パターン２] 

	{   9.0f, -125.0f,  -10.f, 0.0f },	//銃 glk 
	{  -9.0f, -125.0f,  1.0f, 0.0f },	//左手 
	{   9.0f, -125.0f,  -10.f, 0.0f },	//右手 
	{  -9.0f, -125.0f,  1.0f, 0.0f },	//左手 

	{  20.0f, -264.0f, -27.0f, 0.0f },	//銃 abakan 
	{ -17.5f, -102.5f, -12.5f, 0.0f },	//左手 
	{  17.5f, -102.5f, -12.5f, 0.0f },	//右手 
	{  17.5f, -102.5f, -12.5f, 0.0f },	//左手[パターン２] 

	{  20.0f, -247.0f, -13.0f, 0.0f },	//銃 ライデンAKS 
	{ -18.0f,  -75.0f, -15.0f, 0.0f },	//左手 
	{ -17.5f, -102.5f, -12.5f, 0.0f },	//右手 
	{  17.5f, -102.5f, -12.5f, 0.0f },	//左手 

	{  17.5f,  -86.0f,  14.0f, 0.0f },	//銃 ライデンSocom 
	{ -17.0f,  -95.0f,   0.0f, 0.0f },	//左手 
//	{ -22.0f,  -95.0f,   0.0f, 0.0f },	//左手 
	{   0.0f,    0.0f,   0.0f, 0.0f },	//右手 
	{ -22.0f, -117.0f,  -1.0f, 0.0f },	//左手 

	{  20.0f, -245.0f, -10.0f, 0.0f },	//銃 ハイテク兵 m4a1 
	{ -19.0f, -110.0f,  -8.0f, 0.0f },	//左手 
	{   0.0f,    0.0f,   0.0f, 0.0f },	//右手 
	{ -19.0f, -110.0f,  -8.0f, 0.0f },	//左手 

	{   0.0f,    0.0f,   0.0f, 0.0f },	//銃 ハイテク兵 rgb6 
	{   0.0f,    0.0f,   0.0f, 0.0f },	//左手 
	{   0.0f,    0.0f,   0.0f, 0.0f },	//右手 
	{   0.0f,    0.0f,   0.0f, 0.0f },	//左手 

	{   155.0f,  0.0f,   -124.0f, 0.0f },	//銃 SPS(ショットガン) 
	{   -9.65f,  -40.0f,  -130.0f, 0.0f },	//左手 
	{   0.0f,    0.0f,   0.0f, 0.0f },	//右手 
	{   -9.65f,  -40.0f,  -130.0f, 0.0f },	//左手 

};
//	
// 	{  17.5f, -115.9f,  -1.2f, 0.0f },	//右手 
static	ALIGN16_PRE SVECTOR	ALIGN16_POST	sv_rotate_data[MAX_PAT*4]={
	{   0,   0,   0 }, // aks
	{   0,   0,   0 },
	{   0,   0,   0 },
	{   0,   0,   0 },

	{   0,   0,   0 }, // famas
	{   0,   0,   0 },
	{   0,   0,   0 },
	{   0,   0,   0 },

	{   0,    0,  0 }, // usp
	{   0,    0,  0 },
	{   0,    0,  0 },
	{   0,-1024,  0 },

	{   0,    0,     0 }, // p90
	{   0,-1024, -1024 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{  20,    0,     0 }, // m4a1
	{  20,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{   0,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{   0,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{   0,    0,     0 },
	{ 341,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{   0,    0,     0 },
	{ 171,    0,     0 },
	{   0,    0,     0 },
	{   0,-1024,     0 },

	{  30,    0,     0 }, // ハイテク兵 m4a1 
	{  30,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{   0,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },
	{   0,    0,     0 },

	{   3807,    3823,     4000 },	//ショットガン 
	{   3413,    0,     0 },
	{   0,    0,     0 },
	{   3413,    0,     0 }

} ;

//	{-512,   0,   0 },


static	inline	void	SetWeaponWorld( DG_OBJS *weapon_objs )
{
	if( weapon_objs->root != NULL ){
		DG_SetPos( weapon_objs->root );
	}else{
		DG_SetPos( &weapon_objs->world );
	}
}



typedef	struct	{
	GV_ACT_EX		actor ;

	int			*control_num;

	OBJECT		*weapon ;
	OBJECT		*body ;			/* 敵兵へのポインタ */

	/* カートリッジ */
	DG_OBJS		*objs ;
	/* 同時表示用カートリッジ1 */
	DG_OBJS		*objs_plus ;
	int			pat;
} Work ;


static	void Act( Work *work )
{
	int		pat1;
	int		pat2;

	if( work->control_num==NULL ||
		work->body       ==NULL ||
		work->objs       ==NULL ||
		work->objs_plus  ==NULL
	    ){
			GV_DestroyActor( work ) ;
			return;
	}
	// boss用 
	if( work->body->evmobj != NULL ){
		if( ( work->body->evmobj->flag & DG_FLAG_INVISIBLE0 ) && !GM_CheckGameStatus( STATE_DEMO ) ){
			work->objs->flag      |= DG_FLAG_INVISIBLE0;
			work->objs_plus->flag |= DG_FLAG_INVISIBLE0;
			/* 映り込み対策 */
			work->objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			work->objs_plus->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			//return;
		}
	} else {
		if((work->body->objs->flag & DG_FLAG_INVISIBLE0) && !GM_CheckGameStatus( STATE_DEMO ) ){
			work->objs->flag      |= DG_FLAG_INVISIBLE0;
			work->objs_plus->flag |= DG_FLAG_INVISIBLE0;
			work->objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			work->objs_plus->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			/* 映り込み対策 */
			work->objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			work->objs_plus->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			//return;
		}
	}
	
	// map 対応by koba4 2001/08/01
	work->weapon->map_name = work->body->map_name;
	GM_GroupObjs( work->objs , work->weapon->map_name );
	GM_GroupObjs( work->objs_plus , work->weapon->map_name );

	pat1=work->pat*2;
	pat2=work->pat*4;

	/* 一個目 */
//printf("crtrg %d : ",((*work->control_num) & 15));
	//	  printf("%d\n" ,(*work->control_num) & 15);
	switch( (*work->control_num) & 15 ){
	  case 0:	/* 非表示 */
		work->objs->flag      |= DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE ;
		SetWeaponWorld( work->weapon->objs );
		break;
	  case 1:	/* 銃に接続されている（銃は右手に付いていると仮定） */
		work->objs->flag      &= ~DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		SetWeaponWorld( work->weapon->objs );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2]) );
		DG_RotatePos( &sv_rotate_data[pat2] );
		break;
	  case 2:	/* 左手にもっている */
		work->objs->flag      &= ~DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		DG_SetPos( &work->body->objs->objs[joint_num_data[pat1 + 1]].world );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2 + 1]) );
		DG_RotatePos( &sv_rotate_data[pat2 + 1] );
		break;
	  case 3:	/* 右手にもっている */
		work->objs->flag      &= ~DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		SetWeaponWorld( work->weapon->objs );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2 + 2]) );
		DG_RotatePos( &sv_rotate_data[pat2 + 2] );
		break;
	  case 4:	/* その場落下開始 */
//		work->objs->flag      |= DG_FLAG_INVISIBLE;
		work->objs->flag &= ~DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		/* 非表示だったら落下しないようにしてみる。
		   （プレイヤーが主観用と客観用を起動しているため、
		   そうしないと、二つマガジンが落ちる）2001/05/02 M.Sonoyama */
		/* 2001/07/18 改訂。DG_FLAG_INVISIBLE0 でチェック */
		if ( !( work->weapon->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
			GV_SetActorChild( work, NewCartridgeFall( &work->objs->world, work->pat, 
													 work->weapon->objs->light, work->body->map_name ) );
		}
		*work->control_num &= ~0x4 ;
		break;
	  case 5:	/* デストロイ */
		work->objs->flag      |= DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE ;
		GV_DestroyActor( work ) ;
		break;
	  case 6:	/* 左手にもっている[パターン２] */
		work->objs_plus->flag &= ~DG_FLAG_INVISIBLE;
		work->objs->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		DG_SetPos( &work->body->objs->objs[joint_num_data[pat1 + 1]].world );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2 + 3]) );
		DG_RotatePos( &sv_rotate_data[pat2 + 3] );
		break;
	}

	DG_PutObjs( work->objs );

	/* 二個目 */
//printf("%d\n",((*work->control_num)<<4) & 15);
	switch( ((*work->control_num)>>4) & 15 ){
	  case 0:	/* 非表示 */
		work->objs_plus->flag |= DG_FLAG_INVISIBLE;
		work->objs_plus->objs[ 0 ].flag |= DG_FLAG_INVISIBLE ;
		SetWeaponWorld( work->weapon->objs );
		break;
	  case 1:	/* 銃に接続されている（銃は右手に付いていると仮定） */
		work->objs_plus->flag &= ~DG_FLAG_INVISIBLE;
		work->objs_plus->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		SetWeaponWorld( work->weapon->objs );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2]) );
		DG_RotatePos( &sv_rotate_data[pat2] );
		break;
	  case 2:	/* 左手にもっている */
		work->objs_plus->flag &= ~DG_FLAG_INVISIBLE;
		work->objs_plus->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		DG_SetPos( &work->body->objs->objs[joint_num_data[pat1 + 1]].world );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2 + 1]) );
		DG_RotatePos( &sv_rotate_data[pat2 + 1] );
		break;
	  case 3:	/* 右手にもっている */
		work->objs_plus->flag &= ~DG_FLAG_INVISIBLE;
		work->objs_plus->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		SetWeaponWorld( work->weapon->objs );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2 + 2]) );
		DG_RotatePos( &sv_rotate_data[pat2 + 2] );
		break;
	  case 4:	/* その場落下開始 */
//		work->objs_plus->flag |= DG_FLAG_INVISIBLE;
		work->objs_plus->flag &= ~DG_FLAG_INVISIBLE;
		work->objs_plus->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		/* 非表示だったら落下しないようにしてみる。
		   （プレイヤーが主観用と客観用を起動しているため、
		   そうしないと、二つマガジンが落ちる）2001/05/02 M.Sonoyama */
		/* 2001/07/18 改訂。DG_FLAG_INVISIBLE0 でチェック */
		if ( !( work->weapon->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
			GV_SetActorChild( work, NewCartridgeFall( &work->objs->world, work->pat, 
													 work->weapon->objs->light, work->body->map_name ) );
		}
		*work->control_num &= ~0x40 ;
		break;
	  case 6:	/* 左手にもっている[パターン２] */
		work->objs_plus->flag &= ~DG_FLAG_INVISIBLE;
		work->objs_plus->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		DG_SetPos( &work->body->objs->objs[joint_num_data[pat1 + 1]].world );
		DG_MovePos( (FVECTOR *)(&fv_shift_data[pat2 + 3]) );
		DG_RotatePos( &sv_rotate_data[pat2 + 3] );
		break;
	}
	DG_PutObjs( work->objs_plus );

/*sigeno 2001.03.14 武器本体のフラグ反映 */
#if 0
//2001.04.19
//非表示フラグ立てても、クリアされてしまうぞ。これでは、、 
	work->objs->flag &= ~(DG_FLAG_INVISIBLE);
	work->objs->flag |= (work->weapon->objs->flag & DG_FLAG_INVISIBLE);	

	work->objs_plus->flag &= ~(DG_FLAG_INVISIBLE);
	work->objs_plus->flag |= (work->weapon->objs->flag & DG_FLAG_INVISIBLE);	
#else
//sigeno debug 
	/*親が非表示なら問答無用に非表示 */
	if(work->weapon->objs->flag & DG_FLAG_INVISIBLE){
		/* 親の非表示フラグをかぶせる */
		work->objs->flag |= ( work->weapon->objs->flag & DG_FLAG_INVISIBLE ) ;
		work->objs_plus->flag |= ( work->weapon->objs->flag & DG_FLAG_INVISIBLE ) ;
	}

	/* 写り込み、影対策（プレイヤー専用） */
	if ( !GM_CheckGameStatus( STATE_DEMO ) &&
		 work->body == GM_PlayerBody ) {
		if ( work->body->objs->objs[ HUMAN21_MIGI_TE ].flag & DG_FLAG_INVISIBLE0 ) {
			work->objs->flag |= DG_FLAG_INVISIBLE0 ;
			work->objs_plus->flag |= DG_FLAG_INVISIBLE0 ;
			work->objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
			work->objs_plus->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
		} 
		if ( !( work->body->objs->flag & DG_FLAG_SHADOWMAKE ) ||
			 ( PL_GetPlayerItem() == IT_Stealth && 
			   !GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ) ) {
			/* ステルス中、影縛り中はSHADOWMAKE倒す */
			work->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
			work->objs_plus->flag &= ~DG_FLAG_SHADOWMAKE ;
		} else {
			work->objs->flag |= DG_FLAG_SHADOWMAKE ;
			work->objs_plus->flag |= DG_FLAG_SHADOWMAKE ;
		}
	}
#endif

/*
	if( ((*work->control_num) & 15)     == 4 ){
		*work->control_num = 0;
	}
*/
		     
}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		/* リスタート時は切断しない */
#if 0
		if ( !( ( ( GV_ACT * )work )->class & GV_CLASS_KILLALL ) GV_IsFollowDestroy( work ) ) {
			DG_DisconnectObjs( work->body->objs, work->objs ) ;
		}
#else
		//修正　柴田 
		if(!GV_IsStageDestroy( work )){
			DG_DisconnectObjs( work->body->objs, work->objs ) ;
		}
#endif
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
	if( work->objs_plus != NULL ){
		/* リスタート時は切断しない */
#if 0
		if ( !( ( ( GV_ACT * )work )->class & GV_CLASS_KILLALL ) ) {
			DG_DisconnectObjs( work->body->objs, work->objs_plus ) ;
		}
#else
		//修正　柴田 
		if(!GV_IsStageDestroy( work )){
			DG_DisconnectObjs( work->body->objs, work->objs_plus ) ;
		}
#endif
		DG_DequeueObjs( work->objs_plus );
		DG_FreeObjs( work->objs_plus );
	}
}

static	int GetResources(
			Work*       work,
			int*        control_num,	/* カートリッジ１の挙動制御 */
			OBJECT*     body,			/* 人体オブジェクト。ＮＵＬＬになるとこのアクターもデストロイする */
			OBJECT*		weapon,
			int			pat				/* 人体と武器の種類 */
			){

	DG_DEF		*def ;

	if( !(pat < MAX_PAT) ) return -1;

	work->control_num=control_num;
	work->body=body;
	work->weapon=weapon;

	work->pat=pat;

	if ( work->pat > MAX_PAT ){
		return -1;
	}
	def = (DG_DEF*)GV_GetCache( GV_CacheID( strcode[ work->pat ] , 'k' ) ) ;

	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );

	if( work->objs == NULL ) return -1;
	DG_QueueObjs( work->objs );
	DG_ConnectObjs( work->body->objs, work->objs );
	DG_SetLightMatrix( work->objs, work->weapon->objs->light );

	work->objs_plus = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );
	if( work->objs_plus == NULL ) return -1;
	DG_QueueObjs( work->objs_plus );
	DG_ConnectObjs( work->body->objs, work->objs_plus );
	DG_SetLightMatrix( work->objs_plus, work->weapon->objs->light );

	/* 親のいるSHADOWMAKEオブジェは、shadow_idをいじる */
	work->objs->shadow_id = 1 ;
	work->objs_plus->shadow_id = 1 ;

	return 0 ;
}


//[*control_num の 種類] 
//  case 0:	/* 非表示 */ 
//  case 1:	/* 銃に接続されている（銃は手に付いていると仮定） */ 
//  case 2:	/* 左手にもっている */
//  case 3:	/* 右手にもっている */
//  case 4:	/* その場落下開始 */
//  case 5:	/* デストロイ */
//------------------------------------------ 以上、従来どおり使える 
// 新規仕様として、上位下位４ビットでフラグを共有しカートリッジを同時に２個まで表示可。 
// 例）(*control_num) が 0x23 の時、両手にもっている 

// [pat]
// 0:敵兵ＡＫＳ 
// 1:プレイヤーFAMAS 
// 2:プレイヤーUSP 
// 3:P90 
// 4:ライデンM4A1 
// 5:ファットマン 
// 6:アバカン 
// 7:ライデンAKS 
// 8:ライデンSocom 
// 9:ハイテク兵M4A1 
// 11:SPS　ショットガン 

void *NewCartridgeControl(
			int		*control_num,	/* カートリッジの挙動制御 */
			OBJECT	*body,			/* 人体オブジェクト。ＮＵＬＬになるとこのアクターもデストロイする */
			OBJECT	*weapon,		/* 武器 */
			int		pat				/* 人体と武器の種類 */
			){
	Work		*work ;

//	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	work = ( Work * )GV_NewEffectPrio( GV_ACTOR_AFTER, sizeof( Work ), 0x70 );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, control_num, body, weapon, pat ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
