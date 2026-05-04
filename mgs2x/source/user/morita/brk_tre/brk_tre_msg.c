//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pot_msg.c
   皿壊れ メッセージ

   2000/04/25 T. Morita
   $Id: brk_tre_msg.c,v 1.1.1.3 2002/11/19 11:45:48 Yoshizawa1 Exp $
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

#include "brk_tree.h"

void BRK_TRE_AlreadyMessy( Work *work )
{
    int     i ;
    FVECTOR pos ;

    for ( i=BRK_N_LEAVES ; --i>=0 ; )
    {
	pos.vx = work->objs->world.m[W][X] + frnd()*800.0f ;
	pos.vy = work->floor[0] + 240.0f ;
	pos.vz = work->objs->world.m[W][Z] + frnd()*800.0f ;
	RotateMatrixXY( &work->leaf->pos[i].world, &DG_UnitMatrix, 0, irnd()&4095 ) ;
	TransMatrix( &work->leaf->pos[i].world, &pos ) ;
	work->leaf->pos[i].color.vw = 128 ;
    }
}

void BRK_TRE_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{	    
	case 1:
	    BRK_TRE_AlreadyMessy( work ) ;
	    break ;
	}
}
