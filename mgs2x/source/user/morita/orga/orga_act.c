//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_act.c 
   オルガ 行動処理関数群

   1999/12/18 T.Morita
   $Id: orga_act.c,v 1.1.1.3 2002/11/19 11:46:20 Yoshizawa1 Exp $
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

#include "include/orga.h"


/* 位置補正  pos_adj.vwが正か負かで補正方法が変わる */
static inline void ORG_ActPositionAdjust( Work *work )
{
    CONTROL *ctrl = &work->control ;
    _sceVu0ScaleVector( &ctrl->step, &ctrl->step, work->speed ) ;

    if ( work->pos_adj.vw > 0.0f )     /* pos_adj.vw の回数だけ pos_adj を位置にを足す */
    {
	work->pos_adj.vy = 0.0f ;
	work->pos_adj.vw -= 1.0f ;
	AddVector( &ctrl->step, &ctrl->step, &work->pos_adj ) ;
    }
    else if ( work->pos_adj.vw < 0.0f )/* pos_adj.vw の回数だけ pos_adj を半分にし位置に足す */
    {
	work->pos_adj.vw += 1.0f ;
	work->pos_adj.vy = 0.0f ;
	ScaleAndAddVector( &ctrl->step, &work->pos_adj, 0.5f ) ;
    }
}

static inline void ORG_ActMoveTarget( Work *work )
{
    int     i ;
    TARGET *t = work->target ;
    PART   *p = work->parts  ;

    /*ターゲットの位置更新*/
    GM_MoveTarget( t++, (FVECTOR *)&work->control.mov ) ;
    for ( i=ORGA_N_PARTS ; --i>=0 ; t++, p++ )
	GM_MoveTarget2( t, p->world ) ;
}

static inline void ORG_ActVibrateSensor( Work *work )
{
    int   i   ;
    float dis ;
    static u_char  DamageVib1L[] = { 176, 4, 0, 0 } ; /* by nakamura */

    /*ブルブルセンサー検知*/
    if( GM_Item == IT_VibSensor )
    {
	dis = GV_VecLen3F2( &GM_PlayerControl->mov, &work->control.mov ) ;

	if ( dis < 10000.0f )
	{
	    i = 16 + (dis > 3600.0f ? ((int)dis - 3600)/200 : 0) ;

	    if ( (int)fpu_Abs( (float)(GM_StagePlayTime - work->vib_time) ) > i )
	    {
		NewPadVibration( DamageVib1L, 2|VAR_FLAG_FORCE ) ;
		work->vib_time = GM_StagePlayTime ;
	    }
	}
    }
}

/* ＳＥ変換用 */
static inline void ORG_ActMotionSeTable( Work *work )
{
    MT_SetMotionSeTable( work->body.m_ctrl, GM_CurrentMap, 1/* SEテーブルID */, 
			 ( work->control.flr_atrs[0] & 0xf0000000 ) >> 28, 0 ) ;
}


/* モーション回転制御 */
static inline void ORG_ActMotionDirection( Work *work )
{
    if ( work->body.m_ctrl->rot_correct )
    {
	work->control.turn.vy = work->control.rot.vy =
	    (work->control.rot.vy + work->body.m_ctrl->rot_correct) & 4095 ;
	work->body.m_ctrl->rot_correct = 0;
    }
}

/*ストリーム再生チェック*/
static inline void ORG_ActPlayStream( Work *work )
{
    if ( work->str_hdl > 0 )
    {
	int  vol, pan ;
   float bp_angle;

	work->voice_tim = 0 ;
	GM_SeGetVolPan( &work->control.mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
	if ( work->flag & ORGA_F_DEMO_MOVIE )
	    if ( !(work->voice_vox>>12) || (work->voice_vox>>12)==ORGA_N_STREAM-1 )
		if ( vol <= 63 )
		    vol = 63 ;
//	GM_VoxStreamSetPan( work->str_hdl, vol, pan ) ;
	GM_VoxStreamSetParam( work->str_hdl, &work->control.mov, work->control.addr, vol, pan, bp_angle ) ;
	/* 再生が終っていれば ハンドラを使っていない状態に戻す*/
	if ( GM_StreamStatus( work->str_hdl ) == GM_STREAM_STATE_END )
	    ORG_RecogStopStream( work ) ;
    }
    /* 声を出す */
    else if ( work->voice_tim )
	if ( !--work->voice_tim && work->voice )
	    GM_JimakuSeSetMode( work->voice, &work->control.mov, GM_SEMODE_BOMB ) ;
}

/***

  Functions for Pre and Post action
  Most for System

  ***/
void ORG_PreAction( Work *work )
{
    GM_ActMotion( &work->body ) ;

    ORG_ActPositionAdjust( work ) ;

    GM_ActControl( &work->control ) ;

    GM_ActObject2( &work->body ) ;

    DG_GetLightMatrix( (FVECTOR *)&work->control.mov, work->lights ) ;

    work->control.height = work->body.height ;
    work->control.step.vy -= 16.0f ;

    ORG_ActMoveTarget( work ) ;
    ORG_ActVibrateSensor( work ) ;
}


void ORG_PostAction( Work *work )
{
    ORG_ActMotionDirection( work ) ;
    ORG_ActPlayStream( work );
}


/***

  Primary Thinking Action

  ***/
/* So called ACTs are included */
#include "action/orga_act.h"

#include "action/orga_wait.h" /* 待ちアクトセット */
#include "action/orga_move.h" /* 移動アクトセット 転びアクションセットを含む*/
#include "action/orga_fire.h" /* 攻撃アクトセット */
#include "action/orga_peep.h" /* 覗きアクトセット */
#include "action/orga_teaz.h" /* 茶化しアクトセット */
#include "action/orga_stll.h" /* 静止アクトセット   */
#include "action/orga_damg.h" /* ダメージアクトセット */

/* So called ACTIONs are included */
#include "action/orga_list.h" /* アクションセット */

#if DEBUG_MODE
#include "action/orga_dbug.h" /* デバッグ用データ */
#endif

#include "action/orga_thk_care.h" /* 思考ルーチン 警戒 */
#include "action/orga_thk_attk.h" /* 思考ルーチン 攻撃 */
#include "action/orga_thk_move.h" /* 思考ルーチン 移動 */

#if DEBUG_MODE
#include "action/orga_dbglst.org"   /*デバッグ用関数エントリーリスト*/
#endif

/*

  基本の思考は,モードとレベルによって思考のルーチンを用意する。しかし
ながら,基本的に思考を支配するのはモードである。モードには,攻撃,移動と
警戒の３つに分けられる。それぞれのモードでの条件分岐により,現在取れる
アクションをa_listに登録する。

反射的運動でない場合に(*think)()は,１を返し,そこからランダムに選び出し
アクションを実行する。しかし,アクションの履歴として ORGA_N_ACT_HIST 個
のヒストリーを持っているので同じ行動は避けられる。

反射的運動である場合に(*think)()は,0を返し,無条件にa_listの最初に登録
されたアクションが選ばれる。このときヒストリーには残らない。弾がなくなっ
た時のリロードなどが反射的運動に当たる。

 */
void ORG_ActThink( Work *work )
{
    static int (*think[])( Work *work, void (***list)( Work * ) ) = {
	ORG_ActThinkToMove      , ORG_ActThinkToAttack    ,
	ORG_ActThinkToBeCarefull, ORG_ActThinkToBeCarefull,
    } ;
    void (**a_list[32])( Work * ) ;

    /* オルガが勝って,ゲームオーバー時 */
    if ( GM_IsGameOver() && work->vitality > 0 && work->vitality_m9 > 0 )
	FaceAtoB( work,
		  ORG_RecogPlayerPos( HUMAN21_KUBI ),
		  &work->control.mov ) ;

    /* その他の場合 */
    else if ( (*think[ work->flag & ORGA_F_MODE_MSK ])( work, a_list ) )
    {
	void (***list     )( Work * ) ;
	int i, min = ORGA_N_ACT_HIST ;
	u_int found=0, mask = 1 ;

	/*
	  a_list に登録された行動をランダムで選び出す。a_listは,NULLで終了して
	  いなければならず,u_intでマスクを取っているため32個以上は登録できない。
	 */
	for ( list=a_list ; *list!=NULL ; list++, mask <<= 1 )
	{
	    for ( i=ORGA_N_ACT_HIST ; --i>=0 ; )
		if ( work->act_hist[i] == (u_int)*list )
		{
		    i -= work->act_h_idx ;
		    if ( (i &= (ORGA_N_ACT_HIST-1)) < min )/* 全部履歴にあった時のためにアクトに登録 */
			work->act = *list, min = i ;
		    goto next_hist ;/* 履歴にあるアクトは,飛ばす */
		}
	    found |= mask ;/* 履歴にないものを登録する */
	next_hist:
		;
	}
//printf( "found[%x]", found ) ;
	if ( found ) /* 見つけた中からランダムにアクトを選ぶ */
	{
	    for ( mask=found ; (mask &= irnd()) ; found=mask ) ;
//printf( "choosed[%x]", found ) ;
	    for ( list=a_list ; !(found&1) ; list++, found >>= 1 ) ;
	    work->act_hist[(int)work->act_h_idx] = (u_int)(work->act = *list) ;
	    work->act_h_idx = (work->act_h_idx+1) & (ORGA_N_ACT_HIST-1) ;
	}
//printf( "act[%x]\n", work->act ) ;
    }
    else
	work->act = a_list[0] ;/* 反射的行動（履歴にも残らない） */

#if DEBUG_MODE
    printf( (work->flag & ORGA_F_MODIST  ? "STLTIM%d MODIST " :
	     work->flag & ORGA_F_OFFENCE ? "STLTIM%d OFFENCE " : "STLTIM%d MOVE "), work->stll_tim ) ;
    printf( work->flag & ORGA_F_DANGR_STAND ? "STAND " : "" ) ;
    printf( work->flag & ORGA_F_DANGR_LEFT  ? "LEFT  " :
	    work->flag & ORGA_F_DANGR_RIGHT ? "RIGHT " : " " ) ;
    printf( work->stll_mtn & ORGA_F_IS_BEHIND ? "Behind " : "Normal " ) ;
    printf( work->stll_mtn & ORGA_F_IS_SQUAT  ? "Squat\n" : "Stand\n" ) ;
#endif
}
