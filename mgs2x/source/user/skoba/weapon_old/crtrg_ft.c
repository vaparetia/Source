//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	crtrg_ft.c
	カートリッジ落下跳ね返り 
	1999/11/05 S.Okajima
	$Id: crtrg_ft.c,v 1.1.1.3 2002/11/19 11:50:33 Yoshizawa1 Exp $

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

#include	"gameheader.h"
#include	"libmt.h"

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

extern int	OK_FloorOffFlag;

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

#else
#define		CARTRIDGE_NAME0	6368044 // GV_StrCode( "aks_amo" ) 
#define		CARTRIDGE_NAME1	6368368 // GV_StrCode( "fms_amo" ) 
#define		CARTRIDGE_NAME2	3223612 // GV_StrCode( "usp_amo" ) 
#define		CARTRIDGE_NAME3	3223172 // GV_StrCode( "p90_amo" ) 
#define		CARTRIDGE_NAME4	10823954 // GV_StrCode( "m4a_rai_amo" ) 
#define		CARTRIDGE_NAME5	14757037 // GV_StrCode( "glk_amo" ) 
#define		CARTRIDGE_NAME6	(14756633) // GV_StrCode( "abk_amo" )  
#define		CARTRIDGE_NAME7	 (6368044) // GV_StrCode( "aks_amo" ) 
#define     CARTRIDGE_NAME8	   (77724) // GV_StrCode( "scm_amo" ) 
#define     CARTRIDGE_NAME9	 (344511) // GV_StrCode( "m4a_amo_htc" ) 
#endif

#define		MAX_PAT		( 12 ) 

#define		VEC_LIMIT			(-250.0f)

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



//#define		GRAVITY		( 6.0f ) 
#define		FALL_TIME	( 60*60*1 ) 

#define		MAX_MAG		( 32 ) 

#define		DECAY_RATIO	( 0.6f ) 

int	ok_magagine_num=0;


typedef	struct	{
	GV_ACT_EX		actor ;

	/* カートリッジ */
	DG_OBJS		*objs ;
	FMATRIX		mat ;
	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos ;
	FVECTOR		velocity ;
	SVECTOR		rot ;
	int			count;
	int			pat;

	int			se_flag;
	int			mag;
	int			flag;
	int			check_flag;

} Work ;


static	void Act( Work *work )
{
	FVECTOR	prev_pos;
//	FVECTOR	diff ;
	FVECTOR fvtemp0;
	FVECTOR fvtemp1;
//	FVECTOR fvtemp2;
	float	f_abs;
	int		flag;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	int			atr[2] ;
	int			map_id;

	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		if( OK_FloorOffFlag ){
			work->check_flag = 1;
		}
	}

	if( work->check_flag ){
		if( UTL_EFT_CheckBound( &work->pos ) >= 0 ){
			GV_DestroyActor( work ) ;
			return;
		}
	}


	if( work->count-- < 0
	 || work->mag == ok_magagine_num){
		GV_DestroyActor( work ) ;
		return;
	}

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	DG_COPY_VEC( &prev_pos, &work->pos ) ;

	if( work->flag==0 ){
		work->rot.vx += 128;
		work->rot.vy += 128;
//		work->rot.vz += 128;

		work->velocity.vy += P_GRAVITY;
		if( work->velocity.vy < VEC_LIMIT ){
			DG_InvisibleObjs( work->objs ) ;
			GV_DestroyActor( work ) ;
			return;
		}

		_sceVu0AddVector( &fvtemp0, &work->pos, &work->velocity ) ;
		if( work->velocity.vy < 0.0f ){
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
				map_id,
				&work->pos,
				&fvtemp0,
				HZX_CHK_FIX,
				HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE );
//				HZX_SEG_NO_PLAYER,
//				HZX_FLOOR_NO_PLAYER );
		}else{
			flag = HZX_OnlineHazardCheck( /* ハザードチェック：床は見ない */
				map_id,
				&work->pos,
				&fvtemp0,
				HZX_CHK_F_SEGMENT,
				HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE );
//				HZX_SEG_NO_PLAYER,
//				HZX_FLOOR_NO_PLAYER );
		}

		if( flag==1 ){	/* 壁 */
			GM_SeSetMode( SD_W_MAGAZI01 , &work->pos, GM_SEMODE_NORMAL ) ;
			HZX_GetOnlinePoint( &fvtemp1 );
			HZX_GetOnlineHazard( flr, atr ) ;
			seg=(HZX_SEG *)flr;
			fvtemp0.vx = seg->p2.z - seg->p1.z ;
			fvtemp0.vy = 0.0f ;
			fvtemp0.vz = seg->p1.x - seg->p2.x ;
			DG_ReflectVector( &fvtemp0, &work->velocity, &work->velocity );
			work->velocity.vx *= DECAY_RATIO;
			work->velocity.vy *= DECAY_RATIO;
			work->velocity.vz *= DECAY_RATIO;
			_sceVu0AddVector( &work->pos, &fvtemp1, &work->velocity ) ;
			work->rot.vx += 128;
			work->rot.vy += 128;
			work->rot.vz += 128;
		}else if( flag==2 ){	/* 床 */
			if(work->se_flag==0 || work->se_flag==1){	/* 床は2回だけなる */
				GM_SeSetMode( SD_W_MAGAZI01 , &work->pos, GM_SEMODE_NORMAL ) ;
				work->se_flag++;
			}
			HZX_GetOnlinePoint( &fvtemp1 );
			HZX_GetOnlineHazard( flr, atr ) ;
			work->velocity.vx = rnd()*work->velocity.vy - work->velocity.vy*0.5f;
			work->velocity.vz = rnd()*work->velocity.vy - work->velocity.vy*0.5f;
			work->velocity.vy *= -DECAY_RATIO;
			_sceVu0AddVector( &work->pos, &fvtemp1, &work->velocity ) ;
			f_abs=( work->velocity.vy > 0 )?work->velocity.vy:-work->velocity.vy;
//			if( f_abs < 10.0f ){
			if( f_abs < DG_FABS(P_GRAVITY)*2.0f ){
				work->pos.vy += 15.0f;
				work->rot.vx = 0;
				work->rot.vy = (short)(4096.0f*rnd());
				work->rot.vz = 1024;
				work->flag=1;
				if( atr[0] & HZX_FLOOR_NO_OBJECT ){	// 停止後の可視決定 
					GV_DestroyActor( work ) ;
					return;
				}
			}else{
				work->rot.vx += 128;
				work->rot.vy += 128;
				work->rot.vz += 128;
			}
			DG_GetLightMatrix( &work->pos, work->lights );
		}else{	/* 当たらなかった */
			DG_COPY_VEC( &work->pos, &fvtemp0 );
		}
		DG_SetPos2( &work->pos, &work->rot );
		DG_GetPos( &work->mat );
	}else{
		DG_SetPos( &work->mat );
	}
	DG_PutObjs( work->objs );

    /* 水飛沫 */
    if ( prev_pos.vy > GM_WaterLevel && work->pos.vy <= GM_WaterLevel ) {
		extern void SetSplushSequenceSmallObject( FVECTOR *pos );
		SetSplushSequenceSmallObject( &work->pos );
//		_sceVu0SubVector( &diff, &work->pos, &prev_pos ) ;
//		PL_CheckBulletSplash( &prev_pos, &diff, 25.0F, 75.0F ) ;
	}
}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}

static	int GetResources(
			Work*       work,
			FMATRIX*    mat,
			int			pat,			/* 人体と武器の種類 */
			FMATRIX*    light,
			int			map
			){

	DG_DEF		*def ;

	work->check_flag = 0;

	work->se_flag=0;

	work->pos.vx = mat->m[3][0];
	work->pos.vy = mat->m[3][1];
	work->pos.vz = mat->m[3][2];
	if( !(pat < MAX_PAT) ) return -1;

	work->flag=0;


	work->pat=pat;

	if ( work->pat > MAX_PAT ){
		return -1;
	}
	def = (DG_DEF*)GV_GetCache( GV_CacheID( strcode[ work->pat ] , 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );
	if( work->objs == NULL ) return -1;
	DG_QueueObjs( work->objs );
	GM_GroupObjs( work->objs, map );
	DG_VisibleObjs( work->objs ) ;

	work->count=FALL_TIME;
	work->velocity.vx = rnd()*2.0f;
	work->velocity.vy = 0.0f;
	work->velocity.vz = rnd()*2.0f;
	work->rot.vx = 0;
	work->rot.vy = 0;
	work->rot.vz = 0;

	DG_COPY_MAT( &work->lights[0], &light[0] );
	DG_COPY_MAT( &work->lights[1], &light[1] );
	DG_SetLightMatrix( work->objs, work->lights );

	work->mag=ok_magagine_num;
	ok_magagine_num++;
	if( ok_magagine_num >= MAX_MAG ) ok_magagine_num=0;

	DG_SetPos2( &work->pos, &work->rot );
	DG_PutObjs( work->objs );

	return 0 ;
}

void *NewCartridgeFall(
			FMATRIX*    mat,		/* 呼ばれた時点でのマトリックス */
			int			pat,		/* 人体と武器の種類 */
			FMATRIX*    light,		/* 銃のライトマトリックス */
			int			map			/* 銃のマップ */
			){
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, mat, pat, light, map ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
