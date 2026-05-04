//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   シナリオ制御関数

   シナリオは、コンバーターによって、テキストデータから
   バイナリに変換されたもの。

   1999/07/08 K.Uehara
	$Id: gcl_init.c,v 1.1.1.3 2002/11/19 11:42:42 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"libgv.h"
#include	"libgcl.h"

static int scenerio_code;

GCL_WORK_T gcl_work;

static int GCL_InitFunc( void *top, int id )
{
	if( ( id & 0x00FFFFFF ) == scenerio_code ){
printf( "LOAD SCRIPT %d\n", scenerio_code );
		GCL_LoadScript( top );
	}

	return 1;
}

void GCL_ChangeSenerioCode( int code )
{
	scenerio_code = code;
}

void GCL_Initialize( void )
{
	GCL_ParseInit();
	GCL_InitVar();
	GCL_ResetCommList();
	GCL_InitBasicCommands();
	GV_SetLoader( 'g', (GV_LOADFUNC) GCL_InitFunc );

	GCL_ChangeSenerioCode( GCL_SCENERIO );
}

void GCL_ResetSystem( void )
{
	// ステージリセット時に呼ばれる
	GCL_ResetLocalVar();
}


#ifdef GCL_DEBUG_MODE

/* for DEBUG */

void dump( char *top, int len )
{
	int i;
	unsigned char *p;

	p = ( unsigned char * )top;

	printf( "%08X: ", p );

	for( i = 0; i < len; i++ ){
		printf( "%02X ", *p );
		p++;
		if( i % 16 == 15 ){
			printf( "\n%08X: ", p );
		}
	}
	printf( "\n" );
}

int GCL_DEBUG_CurrentProcID;
int GCL_DEBUG_CurrentCommandID;

void GCL_Assert( char *file, int line )
{
	printf( "Assertion failed: %s line %d\n", file, line );
	printf( "GCL PROC %08X last COMMAND %08X\n"
			, GCL_DEBUG_CurrentProcID
			, GCL_DEBUG_CurrentCommandID );
	GCL_DEBUG_DumpLog();
	HANGUP();
}

#endif
