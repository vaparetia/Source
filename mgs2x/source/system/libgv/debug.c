//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	デバッグ関連ルーチン

	1999/03/23 K.Uehara
	$Id: debug.c,v 1.3 2002/12/15 11:20:31 takaki Exp $
*/
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifdef KP_WINDOWS
#include	<xtl.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"

#ifdef KP_WINDOWS
extern BOOL	RestoreWindowsPowerOffSetting(void) ;	// mainw.c
#endif

char *GV_DebugMes ="none";

void GV_Assert( char *file, int line )
{
#ifndef KP_WINDOWS
	printf( "Assertion failed: %s line %d\n", file, line );
#else
	ErrorLogPrintf("Assertion failed: %s line %d\n", file, line);
	ErrorLogFlush() ;

	/*-- Windows省電力設定を元に戻す ---------------------------*/

	RestoreWindowsPowerOffSetting() ;
	/*----------------------------------------------------------*/
#endif

	HANGUP();
}
