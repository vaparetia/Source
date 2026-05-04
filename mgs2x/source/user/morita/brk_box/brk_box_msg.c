/*
   brk_box_msg.c
   じゃがいも壊れ

   2000/01/15 T. Morita
   $Id: brk_box_msg.c,v 1.1.1.3 2002/11/19 11:45:23 Yoshizawa1 Exp $
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

#include "brk_box.h"



void BRK_BOX_RegistFoot( int name, OBJECT *body )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトに足のポインタを登録する。*/
    buffer[0] = BRK_BOX_M_REGIST_FOOT ;
    buffer[1] = (u_int)body ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_BOX_GetBoxVitality( int name, int **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;


    /* 壊れポテトの箱の寿命を取得する。*/
    buffer[0] = BRK_BOX_M_GET_BOX_VITALITY ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_BOX_GetPotatoNum( int name, int **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトの数を取得する。*/
    buffer[0] = BRK_BOX_M_GET_POTATO_NUM ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_BOX_GetPotatoPos( int name, FVECTOR **v, int n )
{
    GV_MSG msg ;
    u_int buffer[3] ;

    /* 壊れポテトの位置を取得する。*/
    buffer[0] = BRK_BOX_M_GET_POTATO_POS ;
    buffer[1] = (u_int)v ;
    buffer[2] = (u_int)n ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 3 ;
    GV_SendMessage( &msg ) ;
}


void BRK_BOX_GetBoxPos( int name, FVECTOR **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトの箱の位置を取得する。*/
    buffer[0] = BRK_BOX_M_GET_POTATO_POS ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}



static inline void BRK_BOX_SetFoot( Work *work, OBJECT *body )
{
    work->n_foot-- ;
    work->foot->parts.foot = (FVECTOR *)&body->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[W] ;
    work->foot->target.size.vx = 200 ;
    work->foot->act = BRK_BOX_ActOnFoot ;
    work->foot++ ;
    work->foot->parts.foot = (FVECTOR *)&body->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[W] ;
    work->foot->target.size.vx = 200 ;
    work->foot->act = BRK_BOX_ActOnFoot ;
    work->foot++ ;
    work->n_box += 2 ;
}

void BRK_BOX_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;
    extern OBJECT *GM_PlayerBody ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case BRK_BOX_M_REGIST_FOOT:
	    if ( work->n_foot )
		BRK_BOX_SetFoot( work, msg->message[1] ? (OBJECT*)msg->message[1] : GM_PlayerBody ) ;
	    break ;
	    
	case BRK_BOX_M_GET_BOX_VITALITY:
	    *((int **)msg->message[1]) = &work->box.vitality ;
	    break ;

	case BRK_BOX_M_GET_POTATO_POS:
	    *((FVECTOR **)msg->message[1]) = &work->box[msg->message[2]].pos ;
	    break ;

	case BRK_BOX_M_GET_POTATO_NUM:
	    *((int **)msg->message[1]) = &work->n_box ;
	    break ;

	case BRK_BOX_M_GET_BOX_POS:
	    *((FVECTOR **)msg->message[1]) = &work->box.pos ;
	    break ;
	}
}
