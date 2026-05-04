//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_msg.c
  ヴァンプ行動

  2001/03/23 T.Morita
  $Id: vamp_msg.c,v 1.1.1.3 2002/11/19 11:46:35 Yoshizawa1 Exp $
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


/* 位置の登録 */
void VMPS_Message( Work *work )
{
    GV_MSG  *msg   ;
    int i ;

    for ( i=GV_ReceiveMessage( work->control.name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case VMPS_M_INVISBLE_INACT:
	    VMPS_SetFlag( VMPS_F_NON_ACTION ) ;
	    /* オブジェクトも消す */
	    DG_InvisibleObjs( work->body.objs ) ;
	    work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	    break ;

	case VMPS_M_VISBLE_ACT:
	    VMPS_ResetFlag( VMPS_F_NON_ACTION ) ;
	    /* オブジェクトは自動的につく */
	    break ;

	case VMPS_M_CAPTURE_EMMA:
	    VMPS_CaptureEmma( work ) ;
	    break ;

	default:
	    break ;
	}
}


/* ヴァンプ顔アニメ モーション再生開始 */
void VMPS_SendMessageFaceAnimePlayMotion( int motion )
{
    if ( VMPS_Work )
	if ( motion >= 0 && motion < 2 )
	{
	    int buffer[] = { 0, motion } ;
	    GV_MSG msg ;

	    msg.address = VMPS_FACEANIME_CHARA ;
	    msg.message = buffer ;
	    msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	    GV_SendMessage( &msg ) ;
	}
}

/* ヴァンプ顔アニメ 視線制御 */
void VMPS_SendMessageFaceAnimeSightControl( int motion, int type, int time, FVECTOR *pos )
{
    if ( VMPS_Work )
    {
	int buffer[] = { 1, type, time, (int)pos->vx, (int)pos->vy, (int)pos->vz } ;
	GV_MSG msg ;

	msg.address = VMPS_FACEANIME_CHARA ;
	msg.message = buffer ;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }
}


/*

コマンド化関数

*/
FVECTOR *VMPS_CommandGetPosition()
{
    if ( VMPS_Work )
	return &VMPS_Work->control.mov ;
    return NULL ;
}

OBJECT *VMPS_CommandGetBody()
{
    if ( VMPS_Work )
	return &VMPS_Work->body ;
    return NULL ;
}

CONTROL *VMPS_CommandGetControl()
{
    if ( VMPS_Work )
	return &VMPS_Work->control ;
    return NULL ;
}

int VMPS_CommandGetLife()
{
    if ( VMPS_Work )
	return VMPS_Work->npc.action.life ;
    return -1 ;
}

int VMPS_CommandGetMaxLife()
{
    if ( VMPS_Work )
	return VMPS_Work->vital_max ;
    return -1 ;
}
