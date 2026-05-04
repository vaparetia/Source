//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_msg.c 
   オルガ メッセージ 関数群

   2000/01/06 T.Morita
   $Id: orga_msg.c,v 1.1.1.3 2002/11/19 11:46:21 Yoshizawa1 Exp $
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

#include "include/orga.h"


void ORG_Message( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
    {
        switch ( msg->message[0] )
	{
	case ORGA_M_PLYR_HIDING:
	    //printf( "ORGA_M_PLYR_HIDING\n" ) ;
	    if ( (work->ply_hide & ORGA_F_PLYR_HIDEXP) == ORGA_F_PLYR_HIDEXP )
	    {
		/* 隠れながら場所を移していない */
		work->trgt_hid = *ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
		(*(int*)&work->trgt_hid.vw) = msg->message[1] << 6 ;
	    }
	    //work->ply_hide &=  ORGA_F_PLYR_HIDCLR ;  /* 移動すれば時間クリア */
	    work->ply_hide |=  ORGA_F_PLYR_HIDING ;
	    work->ply_hide &= ~ORGA_F_PLYR_HIDEXP ;  /* 隠れたので expose はクリアする */
	    work->ply_hide &= ~ORGA_F_PLYR_HIDPOS ;  /* ハイドのIDをクリア */
	    work->ply_hide |= msg->message[1] << 6 ; /* ハイドのIDを入れる */
	    break ;

        case ORGA_M_PLYR_GOTOUT:
	    //printf( "ORGA_M_PLYR_GOTOUT\n" ) ;
	    work->ply_hide &= ~ORGA_F_PLYR_HIDING ;
	    break ;

        case ORGA_M_HOLO_GONE:
	    if ( work->procs[ORGA_P_HOLO_GONE] && !(work->flag & ORGA_F_DEMO_MOVIE) )
		GCL_ExecProc( work->procs[ORGA_P_HOLO_GONE], NULL ) ;/*プロック実行*/
	    if ( work->procs[ORGA_P_HOLO_MUST] )
		GCL_ExecProc( work->procs[ORGA_P_HOLO_MUST], NULL ) ;/*プロック実行*/
	    work->flag |= ORGA_F_DMG_HOROATTK ;
	    work->misc_holo_stat = NULL ;
	    break ;

        case ORGA_M_SNA_BARR_IN:
	    printf( "Err: No more ORGA_M_SNA_BARR_IN\n" ) ;
	    break ;
        case ORGA_M_SNA_BARR_OUT:
	    printf( "Err: No more ORGA_M_SNA_BARR_OUT\n" ) ;
	    break ;

	case ORGA_M_STOPSTILL_END:
	    printf( "ORGA_M_STOPSTILL_END\n" ) ;
	    work->flag &= ~ORGA_F_STOP_STILL ;
	    break ;

	case ORGA_M_STOPSTILL_START:
	    work->act = ORG_ActionStopStill ;
	    work->stll_mtn = msg->message[1] ;
	    work->control.rot.vy = work->control.turn.vy = msg->message[2] ;
	    break ;

	case ORGA_M_DSP_MUL_WEIGHT:
	    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	    DG_InvisibleObjs( work->body.objs ) ;
	    break ;
	case ORGA_M_DSP_SGL_WEIGHT:
	    work->body.evmobj->flag |=  DG_EVMOBJ_INVISIBLE ;
	    DG_VisibleObjs( work->body.objs ) ;
	    break ;

        case ORGA_M_SNA_HIDE_TARGET:
	    work->hide_trgt = (TARGET *)msg->message[1] ;
	    break ;

        case ORGA_M_MOVE_DESTINATE:
	    work->act_flg = ORGA_F_MOVE_ATTK ;/* 他の特別行動を止めさせるため代入 */
	    work->act     = ORG_ActionReset  ;
	    work->trgt_pos = &work->trgt_frc ;
	    work->trgt_frc.vx = (float)msg->message[1] ;
	    work->trgt_frc.vy = (float)msg->message[2] ;
	    work->trgt_frc.vz = (float)msg->message[3] ;
#if DEBUG_MODE
	    {
		extern int ORG_DebugFlag ;
		ORG_DebugFlag |= 2 ;
	    }
#endif
	    break ;

        case ORGA_M_MOVE_TELEPORT:
	    work->control.mov.vx = (float)msg->message[1] ;
	    work->control.mov.vy = (float)msg->message[2] ;
	    work->control.mov.vz = (float)msg->message[3] ;
	    work->control.rot.vy = work->control.turn.vy = msg->message[4] ;
	    break ;

        case ORGA_M_AVOID_CANCEL:
	    work->avoid_flg = 0 ;
	    work->flag &= ~ORGA_F_ENB_AVOID ;
	    work->flag |=  ORGA_F_NVR_RESET ;
	    break ;

        case ORGA_M_AVOID_ENABLE:
	    work->flag &= ~ORGA_F_NVR_RESET ;
	    break ;

        case ORGA_M_STOP_ALLACT:
	    work->act_stop = msg->message[1] ;
	    break ;

        case ORGA_M_DEMO_SKIP:
	    work->flag |= ORGA_F_DEMO_SKIP ;
printf( "DEmo SKIPPED %x\n", work->flag ) ;
	    break ;

        case ORGA_M_DEMO_START:
	    work->flag |=  ORGA_F_DEMO_MOVIE ;
	    work->head_mark = HMK2_TYPE_KILL ;/* 頭の上のマークは消す */
	    break ;
        case ORGA_M_DEMO_END:
	    work->flag &= ~ORGA_F_DEMO_MOVIE ;
	    break ;

        case ORGA_M_FLARED_END:
	    if ( work->procs[ORGA_P_FLARED_END] )
		GCL_ExecProc( work->procs[ORGA_P_FLARED_END], NULL ) ;/*プロック実行*/
	    work->misc_spot_stat = NULL ;
	    break ;

	case ORGA_M_FLARED_CHANGE:
	    printf( "%x->", work->act_flg ) ;

	    if ( work->act_flg & ORGA_F_SPOTLGT_ATTK )
		work->act_flg |= ORGA_F_CHNGLGT_ATTK ;

	    printf( "%x Recieved Change Light\n", work->act_flg ) ;
	    break ;

#if DEBUG_MODE
	case ORGA_M_DBG_ACTION:
	{
	    extern int ORG_DbgAction ;
	    ORG_DbgAction = msg->message[1] ;
	    break ;
	}
#endif
	}
    }
}

#if DEBUG_MODE
void ORG_SendDebugDumpMessage()
{
    GV_MSG msg ;
    int buffer = 0 ;

    msg.address = GV_StrCode( "debug_mode_for_orga" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}
#endif




void ORG_SendOrgaExitHoloMessage()
{
    GV_MSG msg ;
    int buffer = ORGA_M_HOLO_GONE ;

    printf( "Send a message to HOLO\n" ) ;

    msg.address = GV_StrCode( "オルガ" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendHoloMessage( int buffer )
{
    GV_MSG msg ;
    /* buffer == 1 ホロの紐をオルガが切るメッセージ   */
    /* buffer == 2 ホロの右側をたくし上げるメッセージ */
    /* buffer == 3 ホロの左側をたくし上げるメッセージ */
    /* buffer == 4 ホロを初期化するメッセージ         */

    printf( "Send a message to HOLO\n" ) ;

    msg.address = GV_StrCode( "ホロ" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendHoloGetFlagAddrMessage( int **flag )
{
    int buffer[2] ={ 0, (int)flag } ;/*ホロのフラグのポインタを得るメッセージ*/
    GV_MSG msg ;

    msg.address = GV_StrCode( "ホロ" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendSpotLightGetFlagAddrMessage( int **flag )
{
    int buffer[2] ={ 0, (int)flag } ;/*投光器のフラグのポインタを得るメッセージ*/
    GV_MSG msg ;

    msg.address = GV_StrCode( "投光器" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendSpotLightFlareMessage()
{
    int buffer[2] ={ 1 } ;/*投光器のフレアを表示するメッセージ*/
    GV_MSG msg ;

    msg.address = GV_StrCode( "投光器" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendGetNearBoxMessage( FVECTOR *from, FVECTOR *to, FVECTOR *r )
{
    int buffer[4] = { 0, (int)from, (int)to, (int)r } ; /* from toを結ぶ線に最も近い箱を取る */
    GV_MSG msg ;

    printf( "Send a message to Box\n" ) ;

    msg.address = GV_StrCode( "小物" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendWallScarRandomSEMessage()
{
    int buffer[1] = { 2 } ; /* 弾痕にランダムの音を鳴らすメッセージを送る */
    GV_MSG msg ;

    msg.address = GV_StrCode( "痕" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}


void ORG_SendOrgaExitLightMessage()
{
    GV_MSG msg ;
    int buffer = ORGA_M_FLARED_END ;

    printf( "Exit Light\n" ) ;

    msg.address = GV_StrCode( "オルガ" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendOrgaChangeLightMessage()
{
    GV_MSG msg ;
    int buffer = ORGA_M_FLARED_CHANGE ;

    printf( "Change Light\n" ) ;

    msg.address = GV_StrCode( "オルガ" ) ;
    msg.message = &buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}
