//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_act.c 
   フォーチュン 行動処理関数群

   1999/12/18 T.Morita
   $Id: fort_act.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "include/fort.h"


static inline void VibrationSensor( Work *work )
{
    float dis ;
    int   i   ;

    if( GM_Item == IT_VibSensor )
    {
	dis = GV_VecLen3F2( &GM_PlayerControl->mov, &work->control.mov ) ;
	if ( dis < 10000.0f )
	{
	    i = 16 + (dis > 3600.0f ? ((int)dis - 3600)/200 : 0) ;
	    
	    if ( (int)fpu_Abs((float)(GV_Time - work->vib_time)) > i )
	    {
		static u_char  DamageVib1L[] = { 176, 4, 0, 0 } ; /* by nakamura */
		
		NewPadVibration( DamageVib1L, 2|VAR_FLAG_FORCE ) ;
		work->vib_time = GV_Time ;
	    }
	}
    }
}

static inline void MoveTargets( Work *work )
{
    GM_MoveTarget( &work->target, &work->control.mov ) ;
}

static inline void PositionAdustment( Work *work )
{
    if ( work->pos_adj.vw > 0.0f )     /* pos_adj.vw の回数だけ pos_adj を位置にを足す */
    {
	work->pos_adj.vw -= 1.0f ;
	AddVector( &work->control.mov, &work->control.mov, &work->pos_adj ) ;
    }
    else if ( work->pos_adj.vw < 0.0f )/* pos_adj.vw の回数だけ pos_adj を半分にし位置に足す */
    {
	work->pos_adj.vw += 1.0f ;
	ScaleAndAddVector( &work->control.mov, &work->pos_adj, 0.5f ) ;
    }
}

static inline void StreamAndSeControl( Work *work )
{
    int  vol, pan ;
    float bp_angle;

    if ( work->str_hdl > 0 )
    {
	work->voice_tim = 0 ;
	GM_SeGetVolPan( &work->control.mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
	if ( work->flag & FRT_F_DEMO_MOVIE )
	    if ( !(work->voice_vox>>12) || (work->voice_vox>>12)==FRT_N_STREAM-1 )
		if ( vol <= 63 )
		    vol = 63 ;
//	GM_VoxStreamSetPan( work->str_hdl, vol, pan ) ;
	GM_VoxStreamSetParam( work->str_hdl, &work->control.mov, work->control.addr, vol, pan, bp_angle ) ;
	/* 再生が終っていれば ハンドラを使っていない状態に戻す*/
	if ( GM_StreamStatus( work->str_hdl ) == GM_STREAM_STATE_END )
	    FRT_RecogStopStream( work ) ;
    }

    /* ストリーム再生がなければ 声を出す */
    else if ( work->voice_tim )
	if ( !--work->voice_tim && work->voice )
	    GM_JimakuSeSetMode( work->voice, &work->control.mov, GM_SEMODE_BOMB ) ;
}

/***

  Functions for Pre and Post action
  Most for System

  ***/
void FRT_PreAction( Work *work )
{
    GM_ActMotion( &work->body ) ;
    GM_ActControl( &work->control ) ;
    GM_ActObject2( &work->body ) ;
    DG_GetLightMatrix( &work->control.mov, work->lights ) ;

    /* ＳＥ変換用 */
    MT_SetMotionSeTable( work->body.m_ctrl, GM_CurrentMap, 1/* SEテーブルID */, 
			 ( work->control.flr_atrs[0] & 0xf0000000 ) >> 28, 0 ) ;

    work->control.height = work->body.height ;
    work->control.step.vy -= 30.0f ;

    /* 位置補正  pos_adj.vwが正か負かで補正方法が変わる */
    PositionAdustment( work ) ;

    /*ターゲットの位置更新*/
    MoveTargets( work ) ;

    /*ブルブルセンサー検知*/
    VibrationSensor( work ) ;
}


void FRT_PostAction( Work *work )
{
    /* モーション回転制御 */
    if ( work->body.m_ctrl->rot_correct )
    {
	work->control.turn.vy = work->control.rot.vy =
	    (work->control.rot.vy + work->body.m_ctrl->rot_correct) & 4095 ;
	work->body.m_ctrl->rot_correct = 0;
    }

    /*ストリーム再生終了チェック*/
    StreamAndSeControl( work ) ;    
}

/***

  Primary Thinking Action

  ***/
void FRT_Action( Work *work )
{
    if ( !(work->flag & FRT_F_GAME_END) )
    {
	if ( !work->act )
	    work->act = FRT_ActionReset ;
	(**work->act)( work ) ;   /* fort_act.c and action.h */
    }
}

/* So called ACTs are included */
#include "action/fort_act.h"

#include "action/fort_wait.h" /* 待ちアクトセット */
#include "action/fort_move.h" /* 移動アクトセット 転びアクションセットを含む*/
#include "action/fort_fire.h" /* 攻撃アクトセット */
#include "action/fort_stll.h" /* 静止アクトセット   */

/* So called ACTIONs are included */
#include "action/fort_list.h" /* アクションセット */

#if DEBUG_MODE
#include "action/fort_dbug.h" /* デバッグ用データ */
#endif

#include "action/fort_thk_attk.h" /* 思考ルーチン 攻撃 */
#include "action/fort_thk_move.h" /* 思考ルーチン 移動 */

#if DEBUG_MODE
#include "action/fort_dbglst.org"   /*デバッグ用関数エントリーリスト*/
#endif

/*

  基本の思考は,モードとレベルによって思考のルーチンを用意する。しかし
ながら,基本的に思考を支配するのはモードである。モードには,攻撃と移動の
２つに分けられる。それぞれのモードでの条件分岐により,現在のアクション
をwork->actに登録する。

 */
void FRT_ActThink( Work *work )
{
    static void (*think[])( Work *work ) = {
	FRT_ActThinkToMove,
	FRT_ActThinkToAttack } ;

    (*think[ work->flag & FRT_F_MODE_MSK ])( work ) ;
}
