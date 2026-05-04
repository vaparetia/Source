//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  ema_guncam.c
  エマ ガンカメラ用パラメータ

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_guncam.c,v 1.1.1.3 2002/11/19 11:46:02 Yoshizawa1 Exp $
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

int EMA_GunCameraBrokenNum = 0 ;

void EMA_GunCamDie()
{
    EMA_GunCameraBrokenNum = 0 ;
    EMA_Work = NULL ;
}

void EMA_GunCamBirth( void *ptr )
{
    EMA_Work = (Work *)ptr ;
}


int EMA_GunCameraBroken()
{
    if ( EMA_Work == NULL )
        return 0 ;

    EMA_GunCameraBrokenNum++ ;
    return 1 ;
}

int EMA_GetGunCameraBrokenNum()
{
    return EMA_GunCameraBrokenNum ;
}
