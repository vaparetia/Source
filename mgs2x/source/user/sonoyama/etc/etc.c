//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   etc.c
   そのたもろもろの
   
   2000/08/08 M.Sonoyama
   $Id: etc.c,v 1.1.1.3 2002/11/19 11:50:42 Yoshizawa1 Exp $
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

/* 
   プロックの連続実行
*/

typedef struct {
    GV_ACT 			actor ;
	GCL_ARGS		args ;
	int				buf[ 2 ] ;
	int				proc ;
	int				exec ;
	int				end_proc ;
	int				end_exec ;
	int				time ;
	int				name ;
} Work ;

static	int		CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	if ( n_msg ) {
		while( -- n_msg >= 0 ) {
			if ( msg->message[ 0 ] == 0 ) {
				return 1 ; 
			}
			msg ++ ;
		}
	}
	return 0 ;
}

static 	void 	Act( work )
Work		*work ;
{
	if ( CheckMessage( work ) ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	work->buf[ 0 ] = work->time - 1 ;
	if ( work->proc != 0 ) {
		GM_ExecProc( work->proc, &work->args ) ;
	} else if ( work->exec != 0 ) {
		GM_ExecBlock( ( char * )work->exec, &work->args ) ;
	}
	if ( work->time > 0 && -- work->time <= 0 ) {
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( work )
Work		*work ;
{
	if ( work->end_proc != 0 ) {
		GM_ExecProc( work->end_proc, NULL ) ;
	} else if ( work->end_exec != 0 ) {
		GM_ExecBlock( ( char * )work->end_exec, NULL ) ;
	}
}

void 	*NewExecProcContinual( int name, int where )
{
    Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		work->proc = GCL_GetOptionValue( 'p', 0 ) ;
		work->exec = GCL_GetOptionValue( 'e', 0 ) ;
		work->end_proc = GCL_GetOptionValue( 'R', 0 ) ;
		work->end_exec = GCL_GetOptionValue( 'X', 0 ) ;
		work->time = GCL_GetOptionValue( 't', 0 ) ;
		work->args.argc = 1 ;
		work->args.argv = work->buf ;
		work->name = name ;
    }
    return ( void * )work ;
}

/*--------------------------------------------------------------------------*/

