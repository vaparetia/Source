//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_mgz_msg.c
   雑誌壊れ

   2000/01/15 T. Morita
   $Id: put_bok_msg.c,v 1.1.1.3 2002/11/19 11:46:30 Yoshizawa1 Exp $
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

#include "put_book.h"


void PUT_BOK_ReceiveMessage( Work *work )
{
    int       i    ;
    GV_MSG   *msg  ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    break ;
	}
}

