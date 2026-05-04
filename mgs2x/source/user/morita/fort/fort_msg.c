//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_msg.c 
   フォーチュン メッセージ 関数群

   2000/01/06 T.Morita
   $Id: fort_msg.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "include/fort.h"


void FRT_Message( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
    {
        switch ( msg->message[0] )
	{
	case FRT_M_PLYR_HIDING:
	    //printf( "FRT_M_PLYR_HIDING\n" ) ;
	    if ( (work->ply_hide & FRT_F_PLYR_HIDEXP) == FRT_F_PLYR_HIDEXP )
	    {
		/* 隠れながら場所を移していない */
		_sceVu0CopyVectorXYZ( &work->trgt_hid, FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ) ;
		(*(int*)&work->trgt_hid.vw) = msg->message[1] << 6 ;
	    }
	    //work->ply_hide &=  FRT_F_PLYR_HIDCLR ;  /* 移動すれば時間クリア */
	    work->ply_hide |=  FRT_F_PLYR_HIDING ;
	    work->ply_hide &= ~FRT_F_PLYR_HIDEXP ;  /* 隠れたので expose はクリアする */
	    work->ply_hide &= ~FRT_F_PLYR_HIDPOS ;  /* ハイドのIDをクリア */
	    work->ply_hide |= msg->message[1] << 6 ; /* ハイドのIDを入れる */
	    break ;

        case FRT_M_PLYR_GOTOUT:
	    //printf( "FRT_M_PLYR_GOTOUT\n" ) ;
	    work->ply_hide &= ~FRT_F_PLYR_HIDING ;
	    break ;

	case FRT_M_STOPSTILL_END:
	    printf( "FRT_M_STOPSTILL_END\n" ) ;
	    work->flag &= ~FRT_F_STOP_STILL ;
	    break ;

	case FRT_M_STOPSTILL_START:
	    work->act = FRT_ActionStopStill ;
	    work->stll_mtn = msg->message[1] ;
	    work->control.rot.vy = work->control.turn.vy = msg->message[2] ;
	    break ;

	case FRT_M_DSP_MUL_WEIGHT:
	    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	    DG_InvisibleObjs( work->body.objs ) ;
	    break ;
	case FRT_M_DSP_SGL_WEIGHT:
	    work->body.evmobj->flag |=  DG_EVMOBJ_INVISIBLE ;
	    DG_VisibleObjs( work->body.objs ) ;
	    break ;

        case FRT_M_MOVE_DESTINATE:
	    work->act_flg  = FRT_F_MOVE_ATTK ;
	    work->act      = FRT_ActionReset  ;
	    work->trgt_pos = &work->trgt_frc ;
	    work->trgt_frc.vx = (float)msg->message[1] ;
	    work->trgt_frc.vy = (float)msg->message[2] ;
	    work->trgt_frc.vz = (float)msg->message[3] ;
	    break ;

        case FRT_M_MOVE_TELEPORT:
	    work->control.mov.vx = (float)msg->message[1] ;
	    work->control.mov.vy = (float)msg->message[2] ;
	    work->control.mov.vz = (float)msg->message[3] ;
	    work->control.rot.vy = work->control.turn.vy = msg->message[4] ;
	    break ;

        case FRT_M_STOP_ALLACT:
	    work->act_stop = msg->message[1] ;
	    break ;

        case FRT_M_DEMO_START:
	    work->flag |=  FRT_F_DEMO_MOVIE ;
	    break ;
        case FRT_M_DEMO_END:
	    work->flag &= ~FRT_F_DEMO_MOVIE ;
	    break ;

	case FRT_M_SLOWMTN_START:
	    MT_SetMotionSpeed( work->body.m_ctrl,  (float)TIME_BASE/5.0f ) ;
	    break;
	case FRT_M_SLOWMTN_END:
	    MT_SetMotionSpeed( work->body.m_ctrl, -1.0f ) ;
	    break;

#if DEBUG_MODE
	case FRT_M_DBG_ACTION:
	{
	    extern int FRT_DbgAction ;
	    FRT_DbgAction = msg->message[1] ;
	    break ;
	}
#endif
	}
    }
}

void FRT_SendButtonMessage( int updown )
{
    GV_MSG msg ;
    int buffer = updown ;

    msg.address = GV_StrCode( "ボタン" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}


#define	FRT_RGB_R  (16)
#define	FRT_RGB_G  (8)
#define	FRT_RGB_B  (4)
void FRT_SendLightOffMessage( Work *work )
{
    GV_MSG msg ;
    int buffer = 0 ;

    OK_FogColorBase.r = FRT_RGB_R ;
    OK_FogColorBase.g = FRT_RGB_G ;
    OK_FogColorBase.b = FRT_RGB_B ;
    OK_FogNear        = 0 ;
    OK_FogFar         = 16000 ;
    NewFogSet_Demo(
	OK_FogColorBase.r,
	OK_FogColorBase.g,
	OK_FogColorBase.b,
	OK_FogNear, OK_FogFar,
	30*5/TIME_BASE ) ;

    msg.address = 7637503/*fort_lgt*/ ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;

    /* 消えていることを表す */
    work->lgt_on = -2 ;
}

void FRT_SendLightOnMessage( Work *work )
{
    GV_MSG msg ;
    int buffer = 1 ;
    extern void *NewFogSet_Demo( int col_r, int col_g, int col_b,
				 float near, float far,
				 int time ) ;

    OK_FogColorBase.r = FRT_FogColor.r ;
    OK_FogColorBase.g = FRT_FogColor.g ;
    OK_FogColorBase.b = FRT_FogColor.b ;
    OK_FogNear        = FRT_FogNear ;
    OK_FogFar         = FRT_FogFar  ;
    NewFogSet_Demo(
	OK_FogColorBase.r,
	OK_FogColorBase.g,
	OK_FogColorBase.b,
	OK_FogNear, OK_FogFar,
	60*5/TIME_BASE ) ;

    msg.address = 7637503/*fort_lgt*/ ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
    work->lgt_on = -1 ;
}


#if DEBUG_MODE
void FRT_SendDebugDumpMessage()
{
    GV_MSG msg ;
    int buffer = 0 ;

    msg.address = GV_StrCode( "debug_mode_for_fort" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}
#endif
