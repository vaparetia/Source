//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	foreach.c
		繰返し実行
	2000/01/13	K.Uehara
	$Id: foreach.c,v 1.1.1.3 2002/11/19 11:51:36 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef KP_XBOX
#include <eekernel.h>
#endif

#include "mgs_type.h"
#include "libgcl.h"

int NewForeach( void )
{
	int argc, repeat;

	if( GCL_GetOption( 'a' ) == NULL ){
		printf( "Foreach:Error\n" );
		HANGUP();
	}
	argc = GCL_GetNextInt();
	if( GCL_GetOption( 'r' ) == NULL ){
		printf( "Foreach:Erorr\n" );
		HANGUP();
	}
	repeat = GCL_GetNextInt();

	{
		int *buffer ;
		int i;
		GCL_ARGS arg;
		char *block;
		char *ap;

		buffer = GV_Malloc( sizeof(int) * argc ) ;
		if ( buffer == NULL ) {
			return 0 ;
		}
		arg.argc = argc;
		arg.argv = buffer;
		if( GCL_GetOption( 'e' ) == NULL ){
			GV_Free( buffer ) ;
			return 0;
		}
		block = ( char * )GCL_GetNextInt();

		if( GCL_GetOption( 'd' ) == NULL ){
			printf( "Foreach:Error\n" );
			HANGUP();
		}
		ap = GCL_NextStr();

		for( i = 0; i < repeat; i++ ){
			int j;
			for( j = 0; j < argc; j++ ){
				if( ap == NULL ){
					printf( "Foreach: Not Enough args\n" );
					HANGUP();
				}
				buffer[ j ] = GCL_GetInt( ap );
				ap = GCL_NextStr();
			}
			GCL_ExecBlock( block, &arg );
		}
		GV_Free( buffer ) ;
	}

	return 0;
}


