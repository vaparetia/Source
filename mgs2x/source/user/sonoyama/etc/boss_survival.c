//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   boss_survival.c
   ボスサバイバル用のキャラ、コマンドはここに
   つくろう。

   2001/10/01	M.Sonoyama
   $Id: boss_survival.c,v 1.1.1.3 2002/11/19 11:50:41 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*----------------------------------------------------------------*/

/* ボスサバイバルステートをセット */
int			GM_COM_SetBossSurvivalStatus( void )
{
	GM_SetGameStatus( STATE_BOSS_SURVIVAL ) ;
	return 0 ;
}


