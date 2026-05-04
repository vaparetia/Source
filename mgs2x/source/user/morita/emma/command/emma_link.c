//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_pre.c
  エマ 前処理

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_link.c,v 1.1.1.3 2002/11/19 11:46:02 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../include/emma.h"

static Work *EMA_Work = NULL ;

void EMA_LinkDie()
{
    EMA_Work = NULL ;
}

void EMA_LinkBirth( void *ptr )
{
    Work *work = ptr ;

    EMA_Work = work ;
}





/* 位置の登録 */
void EMA_SendMessageGetPosition( FVECTOR *pos )
{
    u_int buffer[] = { EMA_M_REGIST_POSTION, (u_int)pos } ;
    GV_MSG msg ;

    msg.address = EMA_Work->npc.ctrl->name ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

/* トリガーの登録 */
void EMA_SendMessageSetTrigger( int *trigger )
{
    u_int buffer[] = { EMA_M_REGIST_TRIGGER, (u_int)trigger } ;
    GV_MSG msg ;

    msg.address = EMA_Work->npc.ctrl->name ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}

/* モーション番号取得の登録 */
void EMA_SendMessageGetMotionNum( int *motion )
{
    u_int buffer[] = { EMA_M_REGIST_MOTION_NUM, (u_int)motion } ;
    GV_MSG msg ;

    msg.address = EMA_Work->npc.ctrl->name ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
    GV_SendMessage( &msg ) ;
}


/*

コマンド化関数

*/

int EMA_CommandIsEnableLinkMove()
{
    Work *work = EMA_Work ;
    int   dir ;

    if ( work )
    {
	/*
	  最初だけは,角度がある程度同じでないと駄目
	 */
	if ( EMA_Flag(EMA_F_FIRST_WALL) )
	    return 1 ;
	if ( EMA_Flag(EMA_F_FIRST_LINK) )
	{
	    dir = work->control.rot.vy - GM_PlayerControl->rot.vy ;
	    dir = EMA_RecalcDir( dir ) ;
	    if ( dir > 256 || dir < -256 )
		if ( !EMA_UtilDestinateToWall( work, NULL ) )
		    return  0 ;
	}

	/* ある程度近付いているか エマが近付いた判断をした場合,
	   動き出せる */
	switch( work->mar_mtn )
	{
	case MAR(EMA_BASE,RAI_RUN):
	case MAR(EMA_BASE,RAI_WALK):
	    if ( EMA_Flag(EMA_F_FIRST_LINK) )
		return 0 ;
	case MAR(EMA_BASE,RAI_IDLE):
	    return 1 ;
	}
    }
    return  0 ;
}


int EMA_CommandIsEnableLink()
{
    Work *work = EMA_Work ;
    float cs ;

    if ( work )
    {
	if ( !EMA_Flag( EMA_F_ENB_LINK ) )
	    return 0 ;
	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )
	    return 0 ;

	cs = work->pl_dis / (GM_PlayerPosition.vy - work->control.mov.vy) ;
	if ( (work->pose==EMA_TARGET_SIZE_STAND && cs>-0.5f && cs<1.0f) ||
	     (work->pose==EMA_TARGET_SIZE_SQUAT && cs>-0.5f && cs<0.1f) )
	    return 0 ;

	if ( !(HZX_OnlineHazardCheck( work->control.hzx_id,
				      &work->control.mov,
				      &GM_PlayerControl->mov,
				      HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT, 
				      HZX_SEG_ALL, HZX_FLOOR_ALL ) & 1 ) )
	{
	    return 1 ;
	}
    }
    return 0 ;
}






int NewEMA_CommandIsLink()
{
    if ( EMA_Work == NULL )
        return 0 ;

    return  (EMA_Work->ext_trg ? *EMA_Work->ext_trg : 0) ;
}

int NewEMA_CommandGetLife()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( EMA_Work == NULL )
        return 0 ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, EMA_Work->npc.action.life ) ;

    return 1 ;
}

int NewEMA_CommandGetPosition()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( EMA_Work == NULL )
        return 0 ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)EMA_Work->control.mov.vx ) ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)EMA_Work->control.mov.vy ) ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)EMA_Work->control.mov.vz ) ;

    return 1 ;
}

int NewEMA_CommandGetFloor()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( EMA_Work == NULL )
        return 0 ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)EMA_Work->control.mov.vx ) ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)EMA_Work->control.levels[0] ) ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)EMA_Work->control.mov.vz ) ;

    return 0 ;
}

int NewEMA_CommandGetLink()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( EMA_Work == NULL )
        return 0 ;
    if ( EMA_Work->ext_trg == NULL )
        return 0 ;

    if ( GCL_NextStr() == NULL )
        return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)*EMA_Work->ext_trg ) ;

    return 1 ;
}

int NewEMA_CommandForceMove()
{
    float x, y, z ;

    if ( EMA_Work )
	if ( GCL_NextStr() )
	{
	    x = (float)GCL_GetNextInt() ;
	    if ( GCL_NextStr() )
	    {
		y = (float)GCL_GetNextInt() ;
		if ( GCL_NextStr() )
		{
		    z = (float)GCL_GetNextInt() ;
		    EMA_UtilForceMove( EMA_Work, x, y, z ) ;
		    return 1 ;
		}
	    }
	}

    return 0 ;
}




#if 0
/*
  プレーヤがエマに視線を追わせる

  一度 目を逸したら EMA_NONSTARE_CNT だけ目を合わせない
  一度 目を合わせたら EMA_STARE_CNT だけ目を合わせ続ける
*/
void EMA_PlayerEyeControl( void )
{
    FVECTOR d ;
    int     rotx, roty ;
    Work *work = EMA_Work ;

    if ( work->ply_stare_cnt > -EMA_NONSTARE_CNT )
        work->ply_stare_cnt-- ;
    if ( work->ply_stare_cnt > 0 || work->ply_stare_cnt <= -EMA_NONSTARE_CNT )
    {
        /* 角度計算（ライデン頭からエマ頭へ） */
        _sceVu0SubVector( &d,
                          BODYPOS(&work->body  , HUMAN21_ATAMA),
                          BODYPOS(GM_PlayerBody, HUMAN21_ATAMA) ) ;

        /* 角度アジャストワーク（目標）横回転 */
        roty = 2048.0f / M_PI * atan2f( d.vx, d.vz ) ;
        roty = EMA_RecalcDir( roty - GM_PlayerControl->turn.vy ) ;
        roty = roty>1408 ? 1408 : roty<-1408 ? -1408 : roty ;

        /* 角度アジャストワーク（目標）縦回転 */
        rotx = 2048.0f / M_PI * atan2f( d.vy, bp_sqrtf( d.vx*d.vx + d.vz*d.vz )) ;  //BP_MATH - emulate PS2 sqrtf
        rotx = EMA_RecalcDir( rotx - GM_PlayerControl->turn.vx ) ;
        rotx = rotx>610 ? 610 : rotx<-256 ? -256 : rotx ;

        /* 角度を更新する条件(これでブルブル首を振らなくなる) */
        if ( (work->ply_stare_cnt < 0 ) ||
             (!(work->ply_stare_roty/1024) && !(roty/1024)) ||
             (work->ply_stare_roty>0 && roty>0) ||
             (work->ply_stare_roty<0 && roty<0) )
            work->ply_stare_rotx = rotx, work->ply_stare_roty = roty ;

        /* ライデンに反映させる条件 */
        if ( d.vx*d.vx+d.vz*d.vz > 750.0f*750.0f || !(roty/1408) )
            work->ply_stare_cnt = 2*60*TIME_BASE/5 ;
    }

    if ( work->ply_stare_cnt > 0 )
    {
        /* 腰から顔までがY軸回転してエマを追う */
        GM_PlayerWork->turn_adjusts[HUMAN21_ONAKA].vy =
            GM_PlayerWork->turn_adjusts[HUMAN21_MUNE].vy  =
            GM_PlayerWork->turn_adjusts[HUMAN21_KUBI].vy  =
            GM_PlayerWork->turn_adjusts[HUMAN21_ATAMA].vy = work->ply_stare_roty/4 ;

        /* 首と頭で縦回転をしてエマを追う */
        GM_PlayerWork->turn_adjusts[HUMAN21_KUBI].vx  =
            GM_PlayerWork->turn_adjusts[HUMAN21_ATAMA].vx = -work->ply_stare_rotx/2 ;
        GM_PlayerWork->turn_adjusts[HUMAN21_KUBI].vz  =
            GM_PlayerWork->turn_adjusts[HUMAN21_ATAMA].vz = -work->ply_stare_rotx/2 ;

        /* 腕は回転した分だけ元の位置に戻ろうとしている */
        GM_PlayerWork->turn_adjusts[HUMAN21_MIGI_KATA].vy =
            GM_PlayerWork->turn_adjusts[HUMAN21_MIGI_UDE2].vy = -work->ply_stare_roty/4 ;
        GM_PlayerWork->turn_adjusts[HUMAN21_MIGI_KATA].vx =
            GM_PlayerWork->turn_adjusts[HUMAN21_MIGI_UDE2].vx = -work->ply_stare_roty/12 ;
        GM_PlayerWork->turn_adjusts[HUMAN21_MIGI_UDE1].vx = work->ply_stare_roty/8 ;

        /* 補正が掛かるようにフラグを立てる */
        GM_PlayerBody->m_ctrl->adjust_flag |= ((1<<HUMAN21_MIGI_KATA)|
                                               (1<<HUMAN21_MIGI_UDE2)|
                                               (1<<HUMAN21_MIGI_UDE1)|
                                               (1<<HUMAN21_ONAKA)|
                                               (1<<HUMAN21_MUNE )|
                                               (1<<HUMAN21_KUBI )|
                                               (1<<HUMAN21_ATAMA)) ;
    }
}
#endif
