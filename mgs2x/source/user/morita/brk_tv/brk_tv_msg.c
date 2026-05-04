//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tv_msg.c
   壊れテレビ メッセージ

   2000/04/25 T. Morita
   $Id: brk_tv_msg.c,v 1.1.1.3 2002/11/19 11:45:50 Yoshizawa1 Exp $
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

#include "brk_tv.h"

void BRK_TV_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;
    static float screen_uv[] = { 0.5f, 0.5f, 0.5f, 0.5f } ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    if ( !work->count )
	    {
		BRK_TV_InitPrimitive( &work->wipe,
				      DG_GetTexture( GV_StrCode( BRK_TV_WIPE ) ),
				      1,
				      SCE_GS_SET_ALPHA(2,0,0,1,0),
				      75.0f, screen_uv,
				      work->def->lz, work->def->uz, work->def->ly, work->def->uy,
				      work->def->lx ) ;
		work->count = -1 ;
	    }
	    break ;
	}
}
