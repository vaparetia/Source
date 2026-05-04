//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_thk.c
  NPCサンプル思考処理

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_rcg.c,v 1.1.1.3 2002/11/19 11:46:00 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"


static float CheckDistanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}

/* プレイヤーとの位置関係（距離,方向） */
static inline void CheckDirAndDist( Work *work )
{
    float x, z ;

    x = GM_PlayerPosition.vx - work->control.mov.vx ;
    z = GM_PlayerPosition.vz - work->control.mov.vz ;

    /* プレイヤーとの距離 */
    work->pl_dis = sceVu0Sqrt( x*x + z*z ) ;

    /* プレイヤーとの方向 */
    work->pl_dir = EMA_RecalcDir( (int)(2048.0f * atan2f( x, z ) / ( float )M_PI) ) ;
}

/* 位置をコピー */
static inline void SendPosition( Work *work )
{
    if ( work->ext_pos )
	_sceVu0CopyVector( work->ext_pos,  &work->npc.ctrl->mov ) ;
}

/* モーション番号をコピー */
static inline void SendMotionNum( Work *work )
{
    if ( work->ext_mtn )
	*work->ext_mtn = work->npc.action.set_pad ;
}

/* タイムをカウントする */
static inline void CountUpTime( Work *work )
{
    if ( ++work->tic == 6*5/TIME_BASE )
	work->time++, work->tic = 0 ;
}

/* レーダーの向きを変える */
static inline void RadarDirection( Work *work )
{
    work->npc.action.face_dir = work->control.rot.vy ;
}

/* 位置補正 */
static inline void AdjustPosition( Work *work )
{
    CONTROL *ctrl ;

    ctrl = &work->control ;
    if ( work->pos_adjust.vw > 0.0f )
    {
	work->pos_adjust.vw -= 1.0f ;
	_sceVu0AddVector( &ctrl->step, &ctrl->step, &work->pos_adjust ) ;
    }
    _sceVu0ScaleVector( &work->vel_adjust, &work->vel_adjust, 0.8f ) ;
    _sceVu0AddVector( &ctrl->step, &ctrl->step, &work->vel_adjust ) ;
}

/* 階段の上に居るのかどうか */
static inline void IsOnStairway( Work *work )
{
    if ( work->control.flr_atrs[0] & HZX_FLOOR_STEP ||
	 GM_PlayerControl->flr_atrs[0] & HZX_FLOOR_STEP ||
	 work->control.level[0]->p3.h < 30000.0f )
	EMA_SetFlag( EMA_F_IS_ON_STEP ) ;
    else
	EMA_ResetFlag( EMA_F_IS_ON_STEP ) ;
}

/* 地に足がついているか？ */
static inline void IsOnGround( Work *work )
{
    if ( work->control.grounded || !EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	work->ground = 0 ;
    else
	work->ground++   ;
}

/* 無敵なのか？ */
static inline void IsInvincible( Work *work )
{
    if ( work->non_damage > 0 )
	work->non_damage-- ;
}

/* ヴァンプが首締めをしているので壁を見ない */
static inline void IsHurtByVamp( Work *work ) 
{
    if ( EMA_Flag( EMA_F_HURT_BY_VMP ) )
	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
}

/* プレーヤーが自分をみているかどうか */
static inline void IsStaredByPlayer( Work *work ) 
{
    if ( !work->tic )
    {
	if ( EMA_CheckScreen( work ) )
	     work->stare_cnt++ ;
	else
	    work->stare_cnt = 0 ;
    }
}

/* プレーヤーが近くを狙ったかどうか */
static inline void PlayerNearShot( Work *work )
{
    float d ;
    FVECTOR aim ;

#if 0
    printf( "work->pose%d   %d %d %d\n", work->pose,
	    work->pose == EMA_TARGET_SIZE_STAND,
	    GM_WeaponFire,
	    GM_CheckPlayerStatus( PLAYER_WATCH ) ) ;
#endif

    if ( work->pose == EMA_TARGET_SIZE_STAND &&
	 !EMA_Flag( EMA_F_ENB_GO_DOWN|EMA_F_ENB_GO_UP ) &&
	 !work->non_damage &&
	 GM_CheckPlayerStatus( PLAYER_WATCH ) )
	if ( GM_WeaponFire == WP_Psg1 || GM_WeaponFire == WP_Psg1T ||
	     GM_WeaponFire == WP_Usp  || GM_WeaponFire == WP_m92   ||
	     GM_WeaponFire == WP_m4   || GM_WeaponFire == WP_Aks   ||
	     GM_WeaponFire == WP_Socom )
	{
	    _sceVu0AddVector( &aim, 
			      BODYPOS(&work->body, HUMAN21_MIGI_KAKATO),
			      BODYPOS(&work->body, HUMAN21_HIDARI_KAKATO) ) ;
	    _sceVu0ScaleVector( &aim, &aim, 0.5f ) ;

	    d = CheckDistanceLineAndPoint( (FVECTOR *)DG_Chanls->eye.m[W],
					   (FVECTOR *)DG_Chanls->eye.m[Z],
					   &aim ) ;
	    /*主観時にどれくらい近くを狙ってるか (軽いバージョン)*/
	    if ( d < EMA_ATTACK_RAD )
	    {
		EMA_SetFlag( EMA_F_NEAR_BY_PLY ) ;
		printf( "Emma: Hey!! Don't shoot near by me!!!%f\n", d ) ;
	    }
	}
}

/* プレーヤーがスプレーしたかどうか */
static inline void IsSprayed( Work *work )
{
    float d ;

    if ( work->cold_stare > 0 )
	if ( --work->cold_stare == 0 )
	    work->blink_tic = (GM_StagePlayTime+3) & 0xff ;/* 目を開ける処理 */

    if ( GM_WeaponFire == WP_ColdSpray && work->pl_dis < 2000.0f )
    {
	d = CheckDistanceLineAndPoint( (FVECTOR *)DG_Chanls->eye.m[W],
				       (FVECTOR *)DG_Chanls->eye.m[Z],
				       BODYPOS(&work->body, HUMAN21_ATAMA) ) ;
	/*主観時にどれくらい近くを狙ってるか (軽いバージョン)*/
	if ( d < 200.0f )
	{
	    work->cold_stare = 10*5/TIME_BASE ;

	    printf( "Emma: Hey!! I'm cold!!!%f %d %d\n", d, work->cold_count, 
		    work->npc.action.faint_count ) ;

	    if ( work->npc.action.faint_count > 1 )
	    {
		work->npc.action.faint_count -= 40*5/TIME_BASE ;
		if ( work->npc.action.faint_count < 1 )
		    work->npc.action.faint_count = 1 ;
	    }

	    else if ( work->cold_count < (600*5/TIME_BASE) )
	    {
		if ( work->cold_count == (600*5/TIME_BASE)-1 )
		{
		    if ( EMA_UtilStartStreamIdx( work, EMA_VC251 ) >= 0 )
			work->cold_count++ ;
		}
		else if ( !(++work->cold_count & 127) )
		{
		    work->voice_tim = 1 ;
		    work->voice_id = work->cold_count & 128 ?
			SD_V_EMAAFR01 : SD_V_EMAAFR02 ;
		}
	    }
	}
    }
}

#if 0 /* 蹴りは無しだそうです。 2001.08.12 */
#define MARGINE 800.0f
/* プレーヤーがエロ雑誌を置いた */
static inline void IsEroMagazine( Work *work )
{
    GM_BOMB *l ;
    GM_BOMB *result ;

    result = NULL ;
    for( l=GM_BombList.next ; l!=NULL ; l=l->next )
	if ( l->weapon == WP_Book )
	    if ( l->mov->vx > work->control.mov.vx - MARGINE &&
		 l->mov->vx < work->control.mov.vx + MARGINE &&
		 l->mov->vz > work->control.mov.vz - MARGINE &&
		 l->mov->vz < work->control.mov.vz + MARGINE )
	    {
		if ( work->book == NULL )
		    _sceVu0CopyVector( &work->book_pos, l->mov ) ;

		result = l ;
	    }
    work->book = result ;
}
#endif


/* 回復するかどうか */
static inline void Is_Recover( Work *work )
{
    if ( EMA_Flag( EMA_F_IS_RECOVER ) )
    {
	EMA_RecoverVitality( work, EMA_VITAL_UP ) ;
	if ( work->npc.action.life >= work->vital_max/2 )
	{
	    work->npc.action.life = work->vital_max/2 ;
	    EMA_ResetFlag( EMA_F_IS_RECOVER ) ;
	}
    }
}

/* スネークのステータスチェック */
static inline void IsSnakeInDanger( Work *work )
{
    if ( PL_GetPlayerWeapon() == WP_Mic )
	switch( GetNPCSnakeStatus() )
	{
	case NPCSNAKE_SCNSTATUS_DAMAGE_BY_RAIDEN:
	case NPCSNAKE_SCNSTATUS_SLEEP:
	case NPCSNAKE_SCNSTATUS_COUNTERATTACK:
	case NPCSNAKE_SCNSTATUS_FAINT:
	    if ( work->voice_flg == EMA_VCF_SNIPER_SNAKE_WISP )
		EMA_UtilStopStream( work ) ;
	}
}

#if DEBUG_MODE
static void DebugMenu( Work *work )
{
    if ( EMA_DbgInvincible == 1 )
	if ( work->non_damage < 2 )
	    work->non_damage = 2 ;
}
#endif

#define EMA_STAND_FPOS 1001.0f
#define EMA_SQUAT_FPOS 750.0f
#define EMA_DOWN_FPOS  200.0f
/* 見つけ位置計算（mar & motion_numの状況によって変える） */
static void FindPos( Work *work )
{
    FVECTOR v    ;
    int     pose ;

    work->findpos.vx = work->control.mov.vx ;
    work->findpos.vz = work->control.mov.vz ;
    work->findpos.vy = work->control.hzx_base ;

    switch( work->mar_mtn )
    {
    case MAR(EMA_BASE,NPC_SQUAT_IDLE):   case MAR(EMA_BASE,RAI_SQUAT_IDLE):
    case MAR(EMA_BASE,RAI_GETUP):        case MAR(EMA_BASE,PAL_SITDOWN):
    case MAR(EMA_BASE,PAL_SQUAT_IDLE):   case MAR(EMA_BASE,PAL_OHMY_GOD):
    case MAR(EMA_BASE,AFRAID_IDLE):      case MAR(EMA_BASE,DAMG_SQUAT):
    case MAR(EMA_BASE,SNIPE_SQUAT_IDLE): case MAR(EMA_BASE,SNIPE_AFRAID):
    case MAR(EMA_BASE,SNIPE_SITDOWN):    case MAR(EMA_BASE,SNIPE_SITAFRAID):
    case MAR(EMA_BASE,SNIPE_STANDUP):
	pose = EMA_TARGET_SIZE_SQUAT ;
	work->findpos.vy += EMA_SQUAT_FPOS ;
	break ;

    case MAR(EMA_BASE,DAMG_DOWN):        case MAR(EMA_BASE,DAMG_WAKE):
    case MAR(EMA_BASE,DAMG_FALL):        case MAR(EMA_BASE,DAMG_SLEEP):
    case MAR(EMA_BASE,DAMG_DOWN_OVER):   case MAR(EMA_BASE,DAMG_OVER):
    case MAR(EMA_BASE,DAMG_TUMBLE):      case MAR(EMA_BASE,DAMG_DROPOFF):
    case MAR(EMA_BASE,SNIPE_DOWN_FALL):  case MAR(EMA_CAPT,CAP_DOWN):
    case MAR(EMA_CAPT,CAP_RELEASE_DOWN): case MAR(EMA_DRAG,DRAG_BODY_START ):
    case MAR(EMA_DRAG,DRAG_BODY_IDLE ):  case MAR(EMA_DRAG,DRAG_BODY_WALK ):
    case MAR(EMA_DRAG,DRAG_BODY_END ):   case MAR(EMA_DRAG,DRAG_LEGF_START ):
    case MAR(EMA_DRAG,DRAG_LEGF_IDLE ):  case MAR(EMA_DRAG,DRAG_LEGF_WALK ):
    case MAR(EMA_DRAG,DRAG_LEGF_END ):   case MAR(EMA_DRAG,DRAG_LEGB_START ):
    case MAR(EMA_DRAG,DRAG_LEGB_IDLE ):  case MAR(EMA_DRAG,DRAG_LEGB_WALK ):
    case MAR(EMA_DRAG,DRAG_LEGB_END ):   case MAR(EMA_DRAG,DRAG_BODYB_START ):
	pose = EMA_TARGET_SIZE_DOWN ;
	work->findpos.vy += EMA_DOWN_FPOS ;
	/* ダウンの時だけ プレーヤーチェック */
	if (EMA_CheckControl( work, GM_PlayerControl, 500.0f*500.0f, &v ) )
		GM_SetPlayerStatus( PLAYER_ON_CORPSE ) ;
	break ;
    default:
	pose = EMA_TARGET_SIZE_STAND ;
	work->findpos.vy += EMA_STAND_FPOS ;
	break ;
    }

    work->pose = pose ;
    if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
	NPC_ChangeTargetSize( &work->npc, work->pose ) ;
    else
	NPC_ChangeTargetSize( &work->npc, 3 ) ;

    if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
    {
#if 0
	if( work->npc.action.current_mar != work->npc.base_mar ||
	    work->mar_mtn == RAI_WALK ||
	    work->mar_mtn == RAI_RUN ||
	     work->mar_mtn == AFRAID_WALK ||
	    work->pose == EMA_TARGET_SIZE_DOWN )
	    EMA_ResetFlag( EMA_F_ENB_IK ) ;
	else
	    EMA_SetFlag( EMA_F_ENB_IK ) ;
#else
	if( work->mar_mtn == MAR(EMA_BASE,RAI_IDLE) ||
	    work->mar_mtn == MAR(EMA_BASE,PAL_IDLE) )
	    EMA_SetFlag( EMA_F_ENB_IK ) ;
	else
	    EMA_ResetFlag( EMA_F_ENB_IK ) ;
#endif
    }
}

static void ResetVoice( Work *work )
{
    EMA_UtilStopStream( work ) ;
    work->voice_tim = 0 ;
}

static void SetVoice( Work *work )
{
    int  vol, pan, id ;
    float bp_angle;

    if ( work->voice_str >= 0 )
    {
        work->voice_tim = 0 ;
	if ( work->voice_flg == 0 )
	{
	    GM_SeGetVolPan( &work->control.mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
//	    GM_VoxStreamSetPan( work->voice_str, vol, pan ) ;
	    GM_VoxStreamSetParam( work->voice_str, &work->control.mov, work->control.addr, vol, pan, bp_angle ) ;
	}

        /* 再生が終っていれば ハンドラを使っていない状態に戻す*/
        if ( GM_StreamStatus( work->voice_str ) == GM_STREAM_STATE_END )
	{
	    if ( work->voice_proc )
	    {
		int i = 0 ;
		GCL_ARGS arg = { 1, &i } ;
		GCL_ExecProc( work->voice_proc, &arg ) ;
	    }

	    work->voice_str  = -1 ;
	    work->voice_code = 0 ;
	    work->voice_proc = 0 ;
	}
    }
    else
    {
	switch( work->mar_mtn )
	{
	    /* ねている声 */
	case MAR(EMA_CAPT,CAP_DOWN):
#if 0	  
	case MAR(EMA_DRAG,DRAG_BODY_START): case MAR(EMA_DRAG,DRAG_BODY_IDLE):
	case MAR(EMA_DRAG,DRAG_BODY_WALK):  case MAR(EMA_DRAG,DRAG_BODY_END):
	case MAR(EMA_DRAG,DRAG_LEGF_START): case MAR(EMA_DRAG,DRAG_LEGF_IDLE):
	case MAR(EMA_DRAG,DRAG_LEGF_WALK):  case MAR(EMA_DRAG,DRAG_LEGF_END):
	case MAR(EMA_DRAG,DRAG_LEGB_START): case MAR(EMA_DRAG,DRAG_LEGB_IDLE):
	case MAR(EMA_DRAG,DRAG_LEGB_WALK):  case MAR(EMA_DRAG,DRAG_LEGB_END):
	case MAR(EMA_DRAG,DRAG_BODYB_START):
#endif
	    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 15*5 ) )
		GM_SeSetMode( SD_V_EMASLP01,
			      BODYPOS( &work->body, HUMAN21_ATAMA ),
			      GM_SEMODE_NORMAL ) ;
	    else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 93*5 ) )
	    {
		GM_SeSetMode( SD_V_EMASLP02,
			      BODYPOS( &work->body, HUMAN21_ATAMA ),
			      GM_SEMODE_NORMAL ) ;
		if ( !(irnd() & 0x1011) && !work->cold_stare )
		{
		    id = GV_Time&1 ? EMA_VC221 : EMA_VC231 ;
		    if ( EMA_UtilStartStreamIdx( work, id ) >= 0 )
			work->voice_chk |= (1<<id) ;
		}
	    }
	    break ;

	case MAR(EMA_BASE,DAMG_TUMBLE):
	    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 10*5 ) )
		    work->voice_tim = 1, work->voice_id = SD_V_EMADWN01 ;
	    break ;

	case MAR(EMA_CAPT,CAP_RESIST):
	    if ( work->time < 4 && !EMA_Flag( EMA_F_EVENT_SNIPE ) )
		if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 1*5 ) )
		{
		    NewPadVibration2( 9217140 /*ema_single_iya_resist*/, 0 ) ;
		    work->voice_tim = 1, work->voice_id = SD_V_EMAGIV01 ;
		}
	    break ;

	case MAR(EMA_CAPT,CAP_ESCAPE):
	    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 24*5 ) )
		work->voice_tim = 1, work->voice_id = SD_V_EMAAFR01 ;
	    break ;

	case MAR(EMA_CAPT,CAP_THROW):
	    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
		    work->voice_tim = 1, work->voice_id = SD_V_EMAGIV01 ;
	    break ;
#if 0
	case MAR(EMA_CAPT,CAP_HANG):
	    if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 16*5 ) )
		work->voice_tim = 1, work->voice_id = SD_V_EMAOUH02 ;
	    break ;
#endif

	case MAR(EMA_CAPT,CAP_TIE):
	    if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
		if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5*5 ) )
		     work->voice_tim = 1, work->voice_id = SD_V_EMAHNG01 ;
	    break ;
	}

	if ( work->voice_tim >= 0 )
	    if ( --work->voice_tim == 0 )
		GM_SeSetMode( work->voice_id, &work->control.mov,
			      GM_SEMODE_BOMB ) ;
    }
}

#if 0

SD_V_EMAAFR01,	//エマ脅え１「ひっ！」//emaafr02 688
SD_V_EMAAFR02,	//エマ脅え２「あぁ…ぁぁ…」//emaafr03 689
SD_V_EMABTH01,	//エマ小息継ぎ「はーひー」//emabth01 690
SD_V_EMABTH02,	//エマ大息継ぎ「ぶっはぁひぃ」//emabth03 691
SD_V_EMADMG01,	//エマダメージ１「ヤッ！」//emadmg11 692
SD_V_EMADMG02,	//エマダメージ２「アッ！」//emadmg12 693
SD_V_EMADMW01,	//エマ水中ダメージ１「ブッ！」//emadmw01 694
SD_V_EMADMW02,	//エマ水中ダメージ２「プァッ！」//emadmw02 695
SD_V_EMADWN01,	//エマ転ぶ１「あはっ！」//emadwn01 696
//SD_V_EMADWN02,	//エマ転ぶ２「えぁっ！」//emadwn02 697
SD_V_EMAEXP01,	//エマ狙われる予感１「あっ！？」//emaexp01 698
SD_V_EMAEXP02,	//エマ狙われる予感２「はっ！？」//emaexp02 699
SD_V_EMAGIV01,	//エマ気絶うめき１「ううーっ」//emagiv01 700
//SD_V_EMAGIV02,	//エマ気絶うめき２「うぅふぅ」//emagiv02 701
SD_V_EMAOUT01,	//エマアウト「キャーッ！」//emaout11 702
SD_V_EMAOUW01,	//エマ水中アウト「アァァァ」//emaouw01 703
SD_V_EMASCR01,	//エマ悲鳴１「キャッ！」//emascr01 704
SD_V_EMASCR02,	//エマ悲鳴２「イヤー！」//emascr02 705
SD_V_EMAHNG01,	//エマ首絞められ「んあっ」//emahng01 706
SD_V_EMAOUH02,	//エマ首絞めアウト//emaouh01 707
SD_A_EMFOOTL1,	//エマ走り足音（左）//emfootl1 708
SD_A_EMFOOTR1,	//エマ走り足音（右）//emfootr1 709
SD_A_EMFOOTL2,	//エマ歩き足音（左）//emfootl2 710
SD_A_EMFOOTR2,	//エマ歩き足音（右）//emfootr2 711
SD_A_EMSITDW1,	//エマしゃがむ//emsitdw1 712
SD_A_EMSTAND1,	//エマ立つ//emstand1 713
SD_A_MEGAEQU1,	//エマ眼鏡かける//megaequ1 714
SD_A_MEGAOFF1,	//エマ眼鏡外す//megaoff1 715

#endif


static inline void SetFootPrint( Work *work )
{
    /*flag:0x00:何も起こらない
      flag:0x01:右足の真下に足跡
      flag:0x02:左足の真下に足跡
      flag:0x03:両足の真下に足跡*/
    if ( work->pose == EMA_TARGET_SIZE_STAND )
    {
	FMATRIX *mat ;

	work->foot_flag = 0 ;
	mat = &work->body.objs->objs[HUMAN21_MIGI_TSUMASAKI].world ;
	if ( fpu_Abs(work->control.levels[0] - mat->m[W][Y]) < 5.0f ) 
	{
	    if ( !(work->foot_prev & 1) )
		work->foot_flag |= 1 ;
	    work->foot_prev |= 1 ;
	}
	else
	    work->foot_prev &= ~1 ;

	mat = &work->body.objs->objs[HUMAN21_HIDARI_TSUMASAKI].world ;
	if ( fpu_Abs(work->control.levels[0] - mat->m[W][Y]) < 5.0f ) 
	{
	    if ( !(work->foot_prev & 2) )
		work->foot_flag |= 2 ;
	    work->foot_prev |= 2 ;
	}
	else
	    work->foot_prev &= ~2 ;
    }
    //printf( "%d %d\n", work->foot_flag, work->foot_prev ) ;
}





static inline void AbortStopStill( Work *work )
{
    if ( EMA_Flag( EMA_F_NON_MOVE ) )
    {
	if ( work->route->flag==5 || work->route->flag==11 )
	    if ( !GM_CheckMenuStatus( MENU_STREAM_CH_0 ) &&
		 !GM_CheckMenuStatus( MENU_STREAM_CH_1 ) )
		work->stop_time++ ;
	if ( (work->route->flag==5  &&  work->stop_time > 180*2*5/TIME_BASE) ||
	     (work->route->flag==11 &&  work->stop_time > 240*2*5/TIME_BASE) )
	    EMA_ResetFlag( EMA_F_NON_MOVE ) ;
    }
    else
	work->stop_time = 0 ;
}






/*----- 認知処理メイン --------------------------------------------------------*/
void EMA_Recognize( Work *work )
{
    /* ゲームオーバーじゃなければ 認知する */
    if ( !GM_IsGameOver() )
    {
#if DEBUG_MODE
	DebugMenu( work ) ;
#endif
	FindPos( work ) ;

	CountUpTime( work ) ;
	AdjustPosition( work ) ;

	IsInvincible( work ) ;

	if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
	{
	    CheckDirAndDist( work ) ;
	    //RadarDirection( work ) ;

	    SendPosition( work ) ;
	    SendMotionNum( work ) ;

	    SetFootPrint( work ) ;

	    IsOnStairway( work ) ;
	    IsStaredByPlayer( work ) ;
	    IsSprayed( work ) ;
	    //IsEroMagazine( work ) ; 駄目だそうです 2001.08.12

	}
	else
	{
	    IsOnGround( work ) ;
	    IsHurtByVamp( work ) ;
	    Is_Recover( work ) ;

	    IsSnakeInDanger( work ) ;

	    PlayerNearShot( work ) ;
	    AbortStopStill( work ) ;
	}

    }

    if ( !EMA_Flag( EMA_F_EVENT_SNIPE ) )
    {
	if ( work->pl_dis < 6000.0f )
	    SetVoice( work ) ;
	else
	    ResetVoice( work ) ;
    }
    else
	SetVoice( work ) ;
    /* フラグを とっておく */
    work->flag_prv = work->flag ;
}
