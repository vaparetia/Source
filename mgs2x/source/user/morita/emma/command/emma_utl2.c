//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_utl2.c
  エマ 前処理

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_utl2.c,v 1.1.1.3 2002/11/19 11:46:03 Yoshizawa1 Exp $
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

void EMA_Util2Die()
{
    EMA_Work = NULL ;
}

void EMA_Util2Birth( void *ptr )
{
    Work *work = ptr ;

    EMA_Work = work ;
}


/*

コマンド化関数

*/
FVECTOR *EMA_CommandGetFindPos()
{
    if ( EMA_Work )
	return &EMA_Work->findpos ;
    return NULL ;
}

OBJECT *EMA_CommandGetBody()
{
    if ( EMA_Work )
	return &EMA_Work->body ;
    return NULL ;
}

int EMA_CommandGetLife()
{
    if ( EMA_Work )
	return EMA_Work->npc.action.life ;
    return -1 ;
}

int EMA_CommandGetMaxLife()
{
    if ( EMA_Work )
	return EMA_Work->vital_max ;
    return -1 ;
}

