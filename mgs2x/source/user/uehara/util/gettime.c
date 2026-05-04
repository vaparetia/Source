//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gclutil.c
		gclユーティリティ関数

	2000/07/14 K.Uehara
	$Id: gettime.c,v 1.1.1.3 2002/11/19 11:51:36 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "libgv.h"

#include "BP_Misc.h"


int NewGclGetRTC( void )
{
	int hour, minute;

   BP_GetLocalTime(NULL, NULL, NULL, &hour, &minute, NULL);

printf( "=---------------------\n" );
printf( "RTC %02d %02d\n", hour, minute );
printf( "=---------------------\n" );
	return ( hour * 60 ) + minute;
}
