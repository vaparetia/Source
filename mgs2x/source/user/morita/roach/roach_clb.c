/*
  rch_clb.c
  ゴキブリ コールバック関数

  2000/04/23 T. Morita
  $Id: roach_clb.c,v 1.1.1.3 2002/11/19 11:46:33 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "roach.h"



void RCH_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;

    if ( def->damaged )
    {
	
	/* clear damage */
	GM_ClearTargetDamage( def ) ;
    }
}
