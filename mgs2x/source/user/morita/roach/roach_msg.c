/*
   rch_msg.c 
   ゴキブリ メッセージ 関数群

   2000/04/23 T.Morita
   $Id: roach_msg.c,v 1.1.1.3 2002/11/19 11:46:33 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "gameheader.h"
#include "libutl.h"

#include "roach.h"


void RCH_Message( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch ( msg->message[0] )
	{
	}
}
