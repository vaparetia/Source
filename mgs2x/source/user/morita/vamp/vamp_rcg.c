//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_rcg.c
  ヴァンプ 認識処理

  2001/03/23 T.Morita
  $Id: vamp_rcg.c,v 1.1.1.3 2002/11/19 11:46:35 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/vamp.h"

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
    if ( work->pos_adjust.vw > 0.0f )
    {
	work->pos_adjust.vw -= 1.0f ;
	AddVector( &work->control.mov, &work->control.mov, &work->pos_adjust ) ;
    }
}

/* エマの床 */
#define VMPS_FLOOR_HEIGHT 2000
#define VMPS_FLOOR_WIDTH  2000
void VMPS_MoveFloor( Work *work )
{
    FVECTOR v ;
    IVECTOR hzx_pos[4] ;

    _sceVu0CopyVector( &v, &work->control.mov ) ;
    v.vy  = work->control.levels[0] ;
    v.vx += VMPS_FLOOR_WIDTH   ;
    v.vz += VMPS_FLOOR_HEIGHT  ;
    _sceVu0FTOI0Vector( &hzx_pos[0], &v ) ;
    v.vx -= VMPS_FLOOR_WIDTH*2  ;
    _sceVu0FTOI0Vector( &hzx_pos[1], &v ) ;
    v.vz -= VMPS_FLOOR_HEIGHT*2 ;
    _sceVu0FTOI0Vector( &hzx_pos[2], &v ) ;
    v.vx += VMPS_FLOOR_WIDTH*2  ;
    _sceVu0FTOI0Vector( &hzx_pos[3], &v ) ;
    HZX_MoveDynamicFloor( work->d_floor, &hzx_pos[0], &hzx_pos[1], &hzx_pos[2], &hzx_pos[3] );
    //HZX_ViewDynamicFloor( work->d_floor, 4 ) ;
}


static void SetVoice( Work *work )
{
    if ( work->voice_tim > 0 )
	if ( ! --work->voice_tim )
	    GM_SeSetMode( work->voice_id, &work->control.mov, GM_SEMODE_BOMB ) ;
}

/*----- 認知処理メイン --------------------------------------------------------*/
void VMPS_Recognize( Work *work )
{
    /* ゲームオーバーじゃなければ 認知する */
    if ( !GM_IsGameOver() )
    {
	CountUpTime( work ) ;
	AdjustPosition( work ) ;
	RadarDirection( work ) ;
	SetVoice( work ) ;
    }
    VMPS_MoveFloor( work ) ;
}

