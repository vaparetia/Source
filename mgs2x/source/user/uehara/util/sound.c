//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sound.c
		command サウンド設定 name -x hoge
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#include "sd_ee.h"
#endif

#include "gameheader.h"

#define SD_LOADFLAG	0x80000007

extern int sceGsSyncV(int mode);

int NewSound( void )
{
	printf( "NewSound\n" );
	if( GCL_GetOption( 'x' ) != NULL ){
		int code;
		code = GCL_GetNextInt();
printf( "SOUND CODE = %X\n", code );
		sd_set_cli( code );
	}
	if( GCL_GetOption( 'w' ) != NULL ){
		int		*ss ;

		ss = sd_status() ;
		while( (*(ss+1) & SD_LOADFLAG) ) sceGsSyncV(0) ;
	}
	printf( "NewSound end\n" );

	return 0;
}
