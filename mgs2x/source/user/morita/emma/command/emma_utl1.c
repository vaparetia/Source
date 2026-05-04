//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_utl1.c
  エマ 頻度の高いコマンド

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_utl1.c,v 1.1.1.3 2002/11/19 11:46:02 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../include/emma.h"

static Work *EMA_Work = NULL ;


void EMA_Util1Die()
{
    EMA_Work = NULL ;
}

void EMA_Util1Birth( void *ptr )
{
    Work *work = ptr ;

    EMA_Work = work ;
}


/*

コマンド化関数

*/
FVECTOR *EMA_CommandGetPosition()
{
    if ( EMA_Work )
	return &EMA_Work->control.mov ;
    return NULL ;
}

CONTROL *EMA_CommandGetControl()
{
    if ( EMA_Work )
	return &EMA_Work->control ;
    return NULL ;
}

