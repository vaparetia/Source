//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ice_msg.c
   皿壊れ メッセージ

   2000/04/25 T. Morita
   $Id: brk_ice_msg.c,v 1.1.1.3 2002/11/19 11:45:33 Yoshizawa1 Exp $
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

#include "brk_icebox.h"
#include "../brk_utl/brk_utl.x"
#include "../brk_hzd/brk_hazard.h"

void BRK_ICE_AlreadyMessy( Work *work )
{
    BOX *b = &work->box ;
    FVECTOR size ;

    /*氷のメモリを確保できそうか？*/
    if ( BRK_ICE_InitIce( work ) < 0 )
	BRK_ICE_FreeIce( work ) ;
    if ( BRK_ICE_InitOpenBox( work ) < 0 )
	BRK_ICE_FreeIce( work ) ;
    else
    {
	b->mov.rot_x  = 1024 ;
	b->mov.rot_y  = (irnd()&0x0100)-128 ;
	b->mov.rot_vx = b->mov.rot_vy = 0 ;
	b->mov.pos_v.vx = 0.0f ;
	b->mov.pos_v.vy = -500.0f ;
	b->mov.pos_v.vz = 0.0f ;
	b->ice_fount = 0 ;
	b->flag = 0 ;
	RotateMatrixXY( &b->objs->world, &DG_UnitMatrix, b->mov.rot_x, b->mov.rot_y ) ;
	BRK_UTL_SizeOfBound( b->objs, &b->objs->world, &size ) ;
	BRK_CheckHazard( work->hzd, &b->mov.pos, &b->mov.pos_v, &BRK_HZD_NoBounce, &size ) ;
	TransMatrix( &b->objs->world, &b->mov.pos ) ;
    }
}

void BRK_ICE_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    BRK_ICE_AlreadyMessy( work ) ;
	    break ;
	}
}
