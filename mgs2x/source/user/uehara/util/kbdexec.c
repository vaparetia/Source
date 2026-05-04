//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	kbdexec.c
		キーボードで指定したコマンドを実行

	1999/11/11 K.Uehara
	$Id: kbdexec.c,v 1.1.1.3 2002/11/19 11:51:37 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#endif

#include "gameheader.h"

typedef struct {
	int code;
	void *block;
} PARAM;

typedef struct {
	GV_ACT	actor;
	int num;
	PARAM param[ 0 ];
} Work;

static void Act( Work *work )
{
	if( IS_ANYKEY_PRESS() ){
		int i;
		PARAM *p;

		p = work->param;

		for( i = 0; i < work->num; i++, p++ ){
			if( KEY_PRESS( p->code ) ){
				GCL_ExecBlock( p->block, NULL );
			}
		}
	}
}

void *NewKbdExec( int name, int map )
{
	Work *work;
	char *top;
	int i, n;

	top = GCL_NextStr();

	for( n = 0;; n++ ){
		int opt;
		opt = GCL_GetNextOption();
		if( opt == 0 ) break;
		if( opt != 'e' ){
			printf( "NewKbdExec:ERROR option\n" );
		}
	}
	if( n == 0 ) return NULL;

	if( ( work = (Work *)GV_NewActor( GV_ACTOR_AFTER
									  , sizeof( Work ) + sizeof( PARAM ) * n ) ) != NULL ){
		GCL_GetOption( 'e' );	// 最初のeオプション
		for( i = 0; i < n; i++ ){
			work->param[ i ].code = GCL_GetNextInt();
			work->param[ i ].block = ( void * )GCL_GetNextInt();
			if( GCL_GetNextOption() != 'e' ){
				break;
			}
		}
		work->num = n;
		GV_SetActor( work, Act, NULL );
	}
	return work;
}
