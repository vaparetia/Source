//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
?/*
  ema_pre.c
  エマ 前処理

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_msg.c,v 1.2 2002/12/09 12:56:29 takaki Exp $
*/
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"



/* 位置の登録 */
void EMA_Message( Work *work )
{
    GV_MSG  *msg   ;
    int i ;
    CONTROL *ctrl ;

    /* 特別にメッセージ受ける前にアンセットしてしまう */
    if ( GM_CheckPlayerStatus( PLAYER_FORCE ) )
	if ( work->ext_trg )
	    *work->ext_trg = 0 ;

    for ( i=GV_ReceiveMessage( work->control.name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case EMA_M_REGIST_POSTION:
	    work->ext_pos = (FVECTOR *)msg->message[1] ;
	    break ;

	case EMA_M_REGIST_TRIGGER:
	    work->ext_trg = (int *)msg->message[1] ;
	    break ;

	case EMA_M_REGIST_MOTION_NUM:
	    work->ext_mtn = (int *)msg->message[1] ;
	    break ;

	case EMA_M_SET_TRIGGER:
	    if ( work->ext_trg )
		*work->ext_trg = msg->message[1] ;
	    break ;

	case EMA_M_INVISBLE_INACT:
	    /* トラップからフラッシュしてクリアする */
	    HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;

	    EMA_SetFlag( EMA_F_NON_ACTION ) ;
	    /* オブジェクトも消す */
	    DG_InvisibleObjs( work->body.objs ) ;
	    work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	    GM_InvisibleGage( &work->gage ) ;

	    work->rctrl.flag &= ~RADAR_VISIBLE ;
	    work->deftrg.class |= TARGET_SKIP ;/* ターゲット無効 */
	    break ;

	case EMA_M_VISBLE_ACT:
	    EMA_ResetFlag( EMA_F_NON_ACTION ) ;
	    /* オブジェクトは自動的につく */
	    GM_VisibleGage( &work->gage ) ;

	    work->rctrl.flag |= RADAR_VISIBLE ;
	    work->deftrg.class &= ~TARGET_SKIP ;/* ターゲット有効 */
	    break ;

	case EMA_M_INVINCIBLE_ON:
	    EMA_SetFlag( EMA_F_NON_DAMAGE ) ;
	    break ;
	case EMA_M_INVINCIBLE_OFF:
	    EMA_ResetFlag( EMA_F_NON_DAMAGE ) ;
	    break ;

	case EMA_M_GIVE_DAMAGE:
	    if ( !EMA_Flag( EMA_F_NON_DAMAGE ) )
	    {
		int  id ;
		printf( "Emma : EMA_M_GIVE_DAMAGE%d\n",
			EMA_Flag(EMA_F_HURT_BY_VMP) ) ;

		if ( EMA_Flag(EMA_F_HURT_BY_VMP) && work->voice_cnt < 3 )
		{
		    id = (work->voice_cnt==0 ? EMA_VC711 :
			  work->voice_cnt==1 ? EMA_VC721 :
			  EMA_VC731) ;
		    EMA_UtilStartStreamIdx( work, id ) ;
		}
		EMA_DamageVitality( work, msg->message[1], DAMG_OVER ) ;
	    }
	    break ;

	case EMA_M_FORCE_MOVE:
	    EMA_UtilForceMove( work,
			       msg->message[1],
			       msg->message[2],
			       msg->message[3] ) ;
	    break ;

	case EMA_M_FORCE_UNLINK:
	    PLY_EE_ResetGraspModeAndTrigger() ;
	    break ;

	case EMA_M_ENABLE_RECOVER:
	    EMA_SetFlag( EMA_F_ENB_RECOVER ) ;
	    break ;
	case EMA_M_DISABLE_RECOVER:
	    EMA_ResetFlag( EMA_F_ENB_RECOVER ) ;
	    break ;

	case EMA_M_ENABLE_STUNGRENADE:
	    EMA_SetFlag( EMA_F_ENB_STUNGRE ) ;
	    break ;
	case EMA_M_DISABLE_STUNGRENADE:
	    EMA_ResetFlag( EMA_F_ENB_STUNGRE ) ;
	    break ;

	    /* エマを強制的に起きる */
	case EMA_M_FORCE_WAKEUP:
	    work->npc.action.faint_count = 0 ;
	    break ;

	case EMA_M_FORCE_LINK:
	    if ( work->ext_trg )
		*work->ext_trg = 3 ;
	    break ;

	case EMA_M_FORCE_HIDE:
	{
	    float min ;
	    int   idx ;

	    if ( (idx = EMA_GetNearestHide( work, &min )) >= 0 )
	    {
		ctrl = &work->control ;
		/* ステップをリセット */
		_sceVu0CopyVector( &ctrl->step, &DG_ZeroVector ) ;

		/* コントロールの位置を更新し,HZXIDを更新させる */
		GM_ResetControlPositionAndGroup( ctrl, &work->hide[idx], 0 ) ;

		/* システムで高さを自動的にリセットさせる */
		if ( HZX_LevelHazardCheck( ctrl->hzx_id,
					   &ctrl->mov,
					   HZX_CHK_ALL, HZX_FLOOR_ALL ) )
		    ctrl->mov.vy = HZX_GetFloorLevel() ;
		GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy ) ;

	    }
	    break ;
	}

	case EMA_M_SET_LIFE: /* ライフをセットする */
	    work->npc.action.life = msg->message[1] ;
	    /* ゲージのアニメーションはなし */
	    work->gage.value = work->gage.prev = work->gage.dec =
		msg->message[1] ;
	    break ;

	case EMA_M_FORCE_DIR:
	    work->control.turn.vy = work->control.rot.vy = msg->message[1] ;
	    break ;

	case EMA_M_EXEC_PROC:
	    if ( msg->message[1] )
		GCL_ExecProc( msg->message[1], NULL ) ;
	    break ;

	case EMA_M_VIBRATE_ON:
	    EMA_SetFlag( EMA_F_ENB_VIBRATE ) ;
	    break ;
	case EMA_M_VIBRATE_OFF:
	    EMA_ResetFlag( EMA_F_ENB_VIBRATE ) ;
	    break ;



	case EMA_M_INVISBLE_SNIPE_INACT:
	    /* トラップからフラッシュしてクリアする */
	    HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;

	    EMA_SetFlag( EMA_F_NON_MOVE ) ;
	    /* オブジェクトも消す */
	    DG_InvisibleObjs( work->body.objs ) ;
	    work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	    break ;

	case EMA_M_VISBLE_SNIPE_ACT:
	    EMA_ResetFlag( EMA_F_NON_MOVE ) ;
	    /* オブジェクトは自動的につく */
	    break ;

	case EMA_M_DISABLE_LINK:
	    PLY_EE_ResetGraspModeAndTrigger() ;
	    EMA_ResetFlag( EMA_F_ENB_LINK ) ;
	    break ;
	case EMA_M_ENABLE_LINK:
	    EMA_SetFlag( EMA_F_ENB_LINK ) ;
	    break ;

	case EMA_M_RELEASE_C4:
	    work->capture.flag &= ~CAPTURE_C4EXIST ;
	    break ;

	case EMA_M_DISABLE_WANDER:
	    EMA_ResetFlag( EMA_F_ENB_WANDER ) ;
	    break ;

	case EMA_M_ENABLE_WANDER:
	    EMA_SetFlag( EMA_F_ENB_WANDER ) ;
	    break ;

	default:
	    break ;
	}
}


/*

  手繋ぎイベントで使用されるメッセージ関数
  エマ本体が使う関数

*/
/* 位置の登録 */
void EMA_SendMessageExecProc( Work *work, int proc )
{
    u_int buffer[] = { EMA_M_EXEC_PROC, (u_int)proc } ;
    GV_MSG msg ;

    msg.address = work->npc.ctrl->name ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

/* ダメージを与える */
void EMA_SendMessageGiveDamage( Work *work, int damage )
{
    u_int buffer[] = { EMA_M_GIVE_DAMAGE, (u_int)damage } ;
    GV_MSG msg ;

    msg.address = work->npc.ctrl->name ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}


/* エマ顔アニメ モーション再生開始 */
void EMA_SendMessageFaceAnimePlayMotion( Work *work, int motion )
{
#if 0 //BP
   //#ifndef KP_WINDOWS
    int buffer[] = { 0, motion } ;
#else
    int buffer[6] = { 0, motion } ;	// Windowsではこうしないと死ぬ…
#endif

    if ( motion >= 0 && motion < 2+3 )
    {
#if 0
	GV_MSG msg ;
	msg.address = EMA_FACEANIME_CHARA ;
	msg.message = buffer ;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
#else
	EMA_ActFaceCommand( work->face_h, buffer ) ;
#endif
    }
}

/* エマ濡れ髪の毛 非表示 */
void EMA_SendMessageInvisbleWetHair( Work *work )
{
    int buffer[] = { 0, 0 } ;
    GV_MSG msg ;

    msg.address = GV_StrCode("エマぬれ髪の毛") ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

void EMA_SendMessageVisbleWetHair( Work *work )
{
    int buffer[] = { 0, 1 } ;
    GV_MSG msg ;

    msg.address = GV_StrCode("エマぬれ髪の毛") ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

/* エマ顔アニメ 視線制御 */
void EMA_SendMessageFaceAnimeSightControl( Work *work, int type,
					   int time, FVECTOR *pos )
{
#if 0 //BP
   //#ifndef KP_WINDOWS
    int buffer[5] = { 1, type, time } ;
#else
    int buffer[6] = { 1, type, time } ;		// こっちが正しいはず
#endif

    if ( pos )
	buffer[3] = pos->vx, buffer[4] = pos->vy, buffer[5] = pos->vz ;
    else
	buffer[3] = buffer[4] = buffer[5] = 0 ;

#if 0
    {
	GV_MSG msg ;
	msg.address = EMA_FACEANIME_CHARA ;
	msg.message = buffer ;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }
#else
    EMA_ActFaceCommand( work->face_h, buffer ) ;
#endif

}


/* エマ濡れ髪の毛 消すメッセージ */
void EMA_SendMessageWetHairInvisible( Work *work, int buffer )
{
    GV_MSG msg ;

    msg.address = GV_StrCode( "エマぬれ髪の毛" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

void EMA_SendMessageDryHairInvisible( Work *work, int buffer )
{
    GV_MSG msg ;

    msg.address = GV_StrCode( "エマ髪の毛" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}
