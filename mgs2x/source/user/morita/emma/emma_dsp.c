//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_dsp.c 
  エマ 表示用関数群

  2000/02/26 T.Morita
  $Id: emma_dsp.c,v 1.1.1.3 2002/11/19 11:45:59 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include	"include/emma.h"


#if 0
/* 顔のモーションのストリーミング再生 */
static inline void EMA_DisplayMouth( Work *work )
{
    DEMO_MOTION *mtn ;
    int       i ;
    FMATRIX  *skel_mats, *mats ;
    EVM_SKEL *skel ;
    FVECTOR  *rot, *trans, vec ;
    extern DEMO_MOTION * GM_StreamGetMotion( int id ) ;


    if ( work->voice_str < 0 )
	return ;
    //if ( work->body.evmobj->flag & DG_EVMOBJ_INVISIBLE )
	//return ;

    /* 現在のフレームのモーションを得る */
    if( (mtn = GM_StreamGetMotion( GV_StrCode( "エマ" ) ) ) )
    {
	/* 顔の全関節の値を設定する */
	skel_mats = work->body.evmobj->matrix[work->body.evmobj->use_buffer^1] ; 
	skel = work->body.evmobj->def->skeleton + 21 ;
	vec.vw = 1.0F;
	for( i=0 ; i<32 ; i++, skel++ )
	{
	    mats = &skel_mats[i + 21] ;

	    /* クォータニオンとトランスレーションの取得 */
	    rot   = &mtn->motion[i*2  ] ;
	    trans = &mtn->motion[i*2+1] ;
	    /* ローカルマトリックスを作成 */
	    MT_QuatToMat( mats, rot ) ;
	    mats->m[W][X] = skel->rt_tx + trans->vx ;
	    mats->m[W][Y] = skel->rt_ty + trans->vy ;
	    mats->m[W][Z] = skel->rt_tz + trans->vz ;
	    /* 親を掛け ワールドマトリックスに変換 */
	    _sceVu0MulMatrix( mats, &skel_mats[ skel->parent ], mats ) ;

	    vec.vx = -skel->rt_tx;
	    vec.vy = -skel->rt_ty;
	    vec.vz = -skel->rt_tz;
	    _sceVu0ApplyMatrix( (FVECTOR *)mats->m[W], mats, &vec ) ;
	}
    }
}
#endif

/* 方向を見る */
static void EMA_DispUnfaceToPlayer( Work *work, FVECTOR *dir )
{
    FVECTOR  d ;
    int    turn_x, turn_y ;

    /* 左右は腰と胸 */
    _sceVu0SubVector( &d, &work->control.mov,
		      BODYPOS( GM_PlayerBody, HUMAN21_MUNE ) ) ;
    turn_y = RAD2ANG( atan2f( d.vx, d.vz ) ) ;
    turn_y -= work->control.rot.vy ;
    turn_y = EMA_RecalcDir( turn_y ) ;

    if ( work->diff_turn < 0 )
	turn_y += turn_y<-512 ? 4096 : 0 ;
    else if ( work->diff_turn > 0 )
	turn_y -= turn_y> 512 ? 4096 : 0 ;
    turn_x = 0 ;

    dir->vy = (float)ANG2RAD(turn_y) * 0.25f ;
    dir->vx = (float)ANG2RAD(turn_x) * 0.25f ;
    if ( work->body.m_ctrl->flag & MT_FLAG_REVERSAL1 )
	dir->vy *= -1 ;
    /* 一定以上曲がらないようクランプする */
    _sceVu0ClampVector( dir, dir, -M_PI/8, M_PI/8 ) ;
}


/* 方向を見る */
static void EMA_DispFaceToPlayer( Work *work, FVECTOR *dir )
{
    FVECTOR  d ;
    FVECTOR *aim ;
    int    turn_x, turn_y ;

    /*  */
    if ( work->ext_trg ? *work->ext_trg : 0 )
	aim = BODYPOS(GM_PlayerBody, HUMAN21_MIGI_TE ) ;
    else
	aim = BODYPOS(GM_PlayerBody, HUMAN21_MUNE    ) ;

    /* 左右は腰と胸 */
    _sceVu0SubVector( &d, aim, &work->control.mov ) ;
    turn_y = RAD2ANG( atan2f( d.vx, d.vz ) ) ;
    turn_y -= work->control.rot.vy ;
    turn_y = EMA_RecalcDir( turn_y ) ;

    /* 移動し始めた時,クルリンバグをなくすためのコード */
    if ( !EMA_Flag(EMA_F_FIRST_LINK) )
    {
	if ( work->diff_turn < 0 )
	    turn_y += turn_y<-512 ? 4096 : 0 ;
	else if ( work->diff_turn > 0 )
	    turn_y -= turn_y> 512 ? 4096 : 0 ;
    }

    if ( /*EMA_Flag(EMA_F_ENB_EYESIGHT)*/ 0 )
    {
        turn_x = -RAD2ANG( atan2f( d.vy, bp_sqrtf( d.vx*d.vx + d.vz*d.vz )) ) ;  //BP_MATH - emulate PS2 sqrtf
        turn_x -= DEG2RAD( 30.0f ) ;
    }
    else
        turn_x = 0 ;

    dir->vy = (float)ANG2RAD(turn_y) * 0.25f ;
    dir->vx = (float)ANG2RAD(turn_x) * 0.25f ;
    if ( work->body.m_ctrl->flag & MT_FLAG_REVERSAL1 )
	dir->vy *= -1 ;

    /* 一定以上曲がらないようクランプする */
    _sceVu0ClampVector( dir, dir, -M_PI/6, M_PI/6 ) ;
}

/* 階段での処理 */
static inline void EMA_DispFaceToPlayerOnStep( Work *work, FVECTOR *dir )
{
    float   diff ;
    float   turn_x ;

    turn_x = 0.0f ;
    diff = GM_PlayerControl->mov.vy - work->control.mov.vy ;
    if ( diff > 300.0f )
    {
	if ( work->control.level[0]->p3.h < 30000.0f ||
	     work->control.flr_atrs[0] & HZX_FLOOR_STEP )
	    turn_x += -20.0f*M_PI/180.0f ;
	if ( GM_PlayerControl->level[0]->p3.h < 30000.0f ||
	    GM_PlayerControl->flr_atrs[0] & HZX_FLOOR_STEP )
	    turn_x += -20.0f*M_PI/180.0f ;
    }
    else if ( diff < -300.0f )
	turn_x = 20.0f*M_PI/180.0f ;
    dir->vx = turn_x*0.5f ;

    /* 一定以上曲がらないようクランプする */
    _sceVu0ClampVector( dir, dir, -M_PI/6, M_PI/6 ) ;
}

/* 間接の線形補間 */
static void EMA_DispFaceToInterpolation( Work *work,
					 FVECTOR *org, FVECTOR *quat,
					 int id1, int id2 )
{
    FVECTOR d ;
    static int reversal_flag ;

    if ( (work->body.m_ctrl->flag & MT_FLAG_REVERSAL1) ^ reversal_flag ) /* Reverse のON/OFF */
	org->vy *= -1 ;
    _sceVu0SubVector( &d, quat, org ) ;
    _sceVu0ScaleVector( &d, &d, 0.15f/*0.25f*/ ) ;/*補間処理 大きければ大きいほど速い(0.0f<x<1.0f) */
    if ( org->vx != 0.0f || org->vy != 0.0f || org->vz != 0.0f ||
	 d.vx    != 0.0f || d.vy    != 0.0f || d.vz    != 0.0f )
    {
	_sceVu0AddVector( org, org, &d ) ;

	work->body.m_ctrl->adjust_flag |= (1 << id1)|(1 << id2) ;
	MT_EulerToQuatXYZ( &work->body.m_ctrl->adjust[id1], org ) ;
	MT_EulerToQuatXYZ( &work->body.m_ctrl->adjust[id2], org ) ;
    }
    reversal_flag = (work->body.m_ctrl->flag & MT_FLAG_REVERSAL1) ;
}

/*  顔や体を向ける方向 */
static inline void EMA_DisplayDir( Work *work )
{
    FVECTOR  direction ;
    _sceVu0CopyVector( &direction, &DG_ZeroVector ) ;
    work->body.m_ctrl->adjust_flag = 0 ;

    if ( work->cold_stare > 0 )
	EMA_DispUnfaceToPlayer( work, &direction ) ;
    else if ( !EMA_Flag( EMA_F_NON_FACING ) )
	if ( !EMA_Flag( EMA_F_IS_ON_STEP ) )
	    EMA_DispFaceToPlayer( work, &direction ) ;

    EMA_DispFaceToInterpolation( work, &work->head_dir, &direction,
				 HUMAN21_ATAMA, HUMAN21_KUBI ) ;

    if ( EMA_Flag(EMA_F_ENB_EYESIGHT) || work->cold_stare > 0 )
	return ;

    if ( !EMA_Flag( EMA_F_NON_FACING ) )
    {
	if ( EMA_Flag( EMA_F_EVENT_SNIPE ) )
	    EMA_DispFaceToPlayer( work, &direction ) ;
	else if ( EMA_Flag( EMA_F_IS_ON_STEP ) )
	    EMA_DispFaceToPlayerOnStep( work, &direction ) ;
    }
    EMA_DispFaceToInterpolation( work, &work->breast_dir, &direction,
				 HUMAN21_ONAKA, HUMAN21_MUNE ) ;
}






/* レーダー表示*/
static inline void EMA_DisplayRadar( Work *work )
{
#if 0    /* 取り敢えずサイト表示は要らない */
    float(*m)[4] = BODYWORLD( &work->body, HUMAN21_ATAMA ).m ;
    float sb = m[X][Y] ;
    float ry ;

    /* 頭の向きを計算する(マトリックスからオイラー角を計算する) */
    if ( sb <= -1.0f || sb >= 1.0f )
        ry = atan2f(  m[Z][X], m[Z][Z] ) ;
    else
	ry = atan2f( -m[X][Z], m[X][X] ) ;
    ry = ry*2048.0f/M_PI ;

    /* レーダーサイト */
    GM_RadarSetSight( &work->rctrl, (int)ry, /*512*/0, /*8128.0f*/0.0f,
		      GM_AlertMode != ALERT_MODE_ALERT ? RADAR_COLOR_BLUE : RADAR_COLOR_YELOW ) ;
#endif
    work->rctrl.flag &= ~RADAR_SIGHT ;
}







/* ゲージを更新 */
static inline void EMA_DisplayGage( Work *work )
{
    work->gage.value = work->npc.action.life ;

#if 0 /* とりあえず警告色を止める */
    /* ゲージを警告色にするかどうか */
    if ( work->npc.action.life < work->vital_max/4 )
	work->gage.flag |=  GM_GAGE_WARNING ;
    else
	work->gage.flag &= ~GM_GAGE_WARNING ;
#endif
}







/* 顔アニメ表示*/
static inline void EMA_DisplayFace( Work *work )
{
    NPCACT *act ;

    act = &work->npc.action ;
    if ( act->face_anime )
    {
	printf( "Emma: face motion%d requested by NPC\n", act->face_anime ) ;
	EMA_SendMessageFaceAnimePlayMotion( work, act->face_anime ) ;
    }
    else
    {
	
	/* 目閉じ */
	if ( act->status & NPC_ACT_STATUS_EYE_CLOSE ||
	     work->mar_mtn == MAR( EMA_CAPT,CAP_DOWN) ||
	     work->cold_stare > 7 )
	    EMA_SendMessageFaceAnimePlayMotion( work, 3 ) ;

	/* 痛い顔 */
	else if ( work->npc.action.current_mar == work->npc.capture_mar )
	    EMA_SendMessageFaceAnimePlayMotion( work, 2 ) ;

	/* 目パチ制御 */
	else
	{
	    int tic = GM_StagePlayTime & 0xff ;

	    if ( tic == work->blink_tic )
	    {
		work->blink_tic = irnd()&255 ;
		work->blink_tic = work->blink_tic<30 ? 30 : work->blink_tic ;
		work->blink_tic = (tic + work->blink_tic) & 255 ;
		EMA_SendMessageFaceAnimePlayMotion( work, 4 ) ;
	    }
	}

    }
}






/* 視線制御 */
static inline void EMA_DisplayEye( Work *work )
{
    if ( work->npc.action.status & NPC_ACT_STATUS_EYE_CLOSE )
	EMA_SendMessageFaceAnimeSightControl( work, -1, 0, NULL ) ;
    else if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
    {
	EMA_SendMessageFaceAnimeSightControl( work, 2, 10, &work->eye_pos ) ;
    }
    else if ( EMA_Flag( EMA_F_ENB_EYESIGHT ) )
    {
	EMA_SendMessageFaceAnimeSightControl( work, 2, 200, &work->eye_pos ) ;
    }
    else if ( EMA_PFlag( EMA_F_ENB_EYESIGHT ) )
	EMA_SendMessageFaceAnimeSightControl( work, -1, 0, NULL ) ;
}






/*

  Prepareing for display

*/
void EMA_Display( Work *work )
{
    EMA_DisplayDir( work ) ;    /* 顔向け */

    /* 腕をIK */
    if ( work->arm_ik )
	EMA_ActPuppetIK( work->arm_ik, EMA_Flag( EMA_F_ENB_ARM_IK ) ) ;

    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	EMA_DisplayFace( work ) ;   /* 顔アニメ表示 */

#if 0
    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	EMA_DisplayMouth( work ) ;  /* 口パク表示   */
#else
    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	if ( work->face_h )
	    EMA_ActFaceAnimation( work->face_h ) ;
    
#endif
    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
    {
	EMA_ActMirrorObj( work->hair_obj  ) ;
	EMA_ActMirrorObj( work->glass_obj ) ;
    }

    EMA_DisplayGage( work ) ;   /* ゲージを更新 */
    EMA_DisplayRadar( work ) ;  /* レーダー表示 */
    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	EMA_DisplayEye( work ) ;    /* 視線制御 */

#if DEBUG_MODE
    EMA_DbgDispStatus( work ) ;
#endif

}
