//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_dash_light.c
    ソリダスダッシュ炎用テンプライト設置キャラ
    2001/06/28 Yuuta Kunibe	
    $Id: solidus_dash_light.c,v 1.1.1.3 2002/11/19 11:44:49 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define LIGHT_MAX_LENGTH	(10000.0f)
#define	FIRE_MAX_LENGTH		(500.0f)

#define	LIGHT_MAX_LENGTH2	(LIGHT_MAX_LENGTH*LIGHT_MAX_LENGTH)
#define	FIRE_MAX_LENGTH2	(FIRE_MAX_LENGTH*FIRE_MAX_LENGTH)


typedef struct {
    FVECTOR	pos;	// 位置
    float	diff2;	// 距離
    float	str;	// 強さ
} LIGHT_PARAM;


typedef	struct	{

    GV_ACT_EX		actor;

    LIGHT_PARAM		sol_light;
    LIGHT_PARAM		rai_light;

    int			lit_set_flag;

    TARGET	 	offense;
    POWER_TARGET 	pt_offense;

    int			bodyfire_flag;
    int			bodyfire_count;

    int mDamageTick;
    
} Work;


static Work *SolidusDashLightWork = NULL;
int SolidusDashFireNum = 0;

extern	int		PL_LiftupNow( void ) ;


/* プレイヤー消火特殊処理メッセージ受信関数 */
static void RecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;


    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( GM_PLAYER_CHAR_BODY_FLAME, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:			/* fade_in, fade_out */
	    work->bodyfire_count = 0;
	    printf("solidus_dash_light-------------extra_extinguish!!\n");
	    break;
	}
	msg++;
    }

}



void SetSolDashFireLightPos( FVECTOR *sol_lit_pos, FVECTOR *rai_lit_pos,
			     float sol_lit_len2, float rai_lit_len2, float sol_lit_str, float rai_lit_str,
			     int bodyfire_flag )
{

    Work *work;

    if ( ( work = SolidusDashLightWork ) == NULL ) {
	return;
    }

    if ( work->lit_set_flag == 0 ) {

	DG_COPY_VEC( &work->sol_light.pos, sol_lit_pos );
	work->sol_light.diff2 = sol_lit_len2;
	work->sol_light.str   = sol_lit_str; 

	DG_COPY_VEC( &work->rai_light.pos, sol_lit_pos );
	work->rai_light.diff2 = rai_lit_len2;
	work->rai_light.str   = rai_lit_str; 

	work->lit_set_flag = 1;

	work->bodyfire_flag = bodyfire_flag;
    }
    else {

	if ( sol_lit_len2 < work->sol_light.diff2 ) {
	    DG_COPY_VEC( &work->sol_light.pos, sol_lit_pos );
	    work->sol_light.diff2 = sol_lit_len2;
	    work->sol_light.str   = sol_lit_str; 
	}
    
	if ( rai_lit_len2 < work->rai_light.diff2 ) {
	    DG_COPY_VEC( &work->rai_light.pos, rai_lit_pos );
	    work->rai_light.diff2 = rai_lit_len2;
	    work->rai_light.str   = rai_lit_str; 
	    work->bodyfire_flag = bodyfire_flag;
	}

    }
    
}


static void Act( Work *work )
{

    int i;
    
    if ( work->lit_set_flag ) {

	/* ソリダス用テンプライト接地 */
	if ( work->sol_light.diff2 < LIGHT_MAX_LENGTH2 ) {

	    //AN_Test_Eye2( &work->sol_lit_pos, 500.f );
	    DG_SetTmpLight2 (
			     &work->sol_light.pos,
			     ( 4000.0f + 1000.f * frnd() ) * work->sol_light.str,
			     ( 8000.0f + 2000.f * frnd() ) * work->sol_light.str,
			     0xff | 0x4f<<8 | 0x08<<16,
			     LIT_FLAG_CHARAONLY );
	}


	/* ライデン用テンプライト接地 */
	if ( work->rai_light.diff2 < LIGHT_MAX_LENGTH2 ) {

	    //AN_Test_Eye2( &work->rai_lit_pos, 500.f );
	    DG_SetTmpLight2 (
			     &work->rai_light.pos,
			     ( 4000.0f + 1000.f * frnd() ) * work->rai_light.str,
			     ( 8000.0f + 2000.f * frnd() ) * work->rai_light.str,
			     0xff | 0x4f<<8 | 0x08<<16,
			     LIT_FLAG_CHARAONLY );

	    /* 体燃え炎呼び出し */	    
	    if ( work->bodyfire_count == 0 ) {
		if ( work->bodyfire_flag ) {
		    if ( work->rai_light.diff2 < FIRE_MAX_LENGTH2 ) {
			if ( !( GM_PlayerStatus & ( PLAYER_ROLLING ) ) && 
				 !PL_LiftupNow() ) {
			    extern void *NewBodyFire( OBJECT *body, int part );
			    for ( i = 0 ; i < 5 ; i++ ) {
				NewBodyFire( GM_PlayerBody, i );
			    }
			    /* ダメージモーション発動 */
			    GM_MoveTarget( &work->offense, &GM_PlayerPosition );
			    GM_PutTarget( &work->offense );
			    work->bodyfire_count = 300;/* body_fireの寿命と合わせる */
			}
		    }
		}
	    }

	}

	work->bodyfire_flag = 0;
	work->lit_set_flag = 0;

    }	


    if ( work->bodyfire_count > 0 ) {
	/* 体燃え中体力減らす -> 体燃えに移すこと！ */
	if ( !(work->mDamageTick & 0xf) ) {
	    GM_VitalityAdjust = -1;
	}

   ++work->mDamageTick;

	work->bodyfire_count--;
	RecieveMessage( work );
    }
    

    if ( SolidusDashFireNum <= 0 ) {
	GV_DestroyActor( work );
    }
	
    
}


static void Die( Work *work )
{
    SolidusDashLightWork = NULL;
}



/*--- ダメージモーション発動用ターゲット初期化 ---*/
static int InitTarget( Work *work )
{

    GM_SetTarget( &work->offense, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector );
    GM_SetPowerTarget( &work->offense, &work->pt_offense, POWER_ONCE, 255, 0, 1, &DG_ZeroVector );
    GM_SetTargetWeaponType( &work->offense, WP_BULLET | WP_NO_BLOOD );
    //GM_TargetSetDirectAttack( &work->offense, GM_PlayerTarget );
    GM_MoveTarget( &work->offense, &DG_ZeroVector );

    return 0;
    
}



static int GetResources( Work *work )
{

    work->lit_set_flag = 0;

    work->bodyfire_flag = 0;
    work->bodyfire_count = 0;

    InitTarget( work );
    
    return 0;
}


void *NewSolidusDashFireLight( void )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	SolidusDashLightWork = work;
	    
	return (void *)work ;

}

