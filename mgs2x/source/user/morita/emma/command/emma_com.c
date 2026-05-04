//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_scn.c
  エマ シナリオ用コマンド

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_com.c,v 1.1.1.3 2002/11/19 11:46:01 Yoshizawa1 Exp $
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


void EMA_InitCommandWork( void *ptr )
{
    extern void EMA_LinkBirth( void *ptr ) ;
    extern void EMA_SnipeBirth( void *ptr ) ;
    extern void EMA_StatBirth( void *ptr ) ;
    extern void EMA_EneBirth( void *ptr ) ;
    extern void EMA_Util1Birth( void *ptr ) ;
    extern void EMA_Util2Birth( void *ptr ) ;
    extern void EMA_GunCamBirth( void *ptr ) ;
    
    EMA_LinkBirth( ptr ) ;
    EMA_SnipeBirth( ptr ) ;
    EMA_StatBirth( ptr ) ;
    EMA_EneBirth( ptr ) ;
    EMA_Util1Birth( ptr ) ;
    EMA_Util2Birth( ptr ) ;
    EMA_GunCamBirth( ptr ) ;
}

void EMA_FreeCommandWork()
{
    extern void EMA_LinkDie() ;
    extern void EMA_SnipeDie() ;
    extern void EMA_StatDie() ;
    extern void EMA_EneDie() ;
    extern void EMA_Util1Die() ;
    extern void EMA_Util2Die() ;
    extern void EMA_GunCamDie() ;

    EMA_LinkDie() ;
    EMA_SnipeDie() ;
    EMA_StatDie() ;
    EMA_Util1Die() ;
    EMA_Util2Die() ;
    EMA_GunCamDie() ;
}
