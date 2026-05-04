//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   syscllbk.c
   システムコールバック

   2000/05/29 M.Sonoyama
   $Id: syscllbk.c,v 1.1.1.3 2002/11/19 11:41:56 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

static	int	GM_SystemCallbackProc[ GM_CALLBACK_MAX ] ;

/* コールバックのリセット */
void	GM_ResetCallbackProc( void )
{
	int			i ;

	for ( i = 0; i < GM_CALLBACK_MAX; i ++ ) {
		GM_SystemCallbackProc[ i ] = 0 ;
	}
}

/* コールバックのコール */
void	GM_CallCallbackProc( which )
int		which ;
{
	printf("Callback[%d]\n",which);
	GM_CallCallbackProc2( which, NULL ) ;
}

void	GM_CallCallbackProc2( int which, void *args )
{
	int		proc ;

	ASSERT( which >= 0 && which < GM_CALLBACK_MAX ) ;
	proc = GM_SystemCallbackProc[ which ] ;
	printf("Callback proc[%d]\n",proc);
	if ( proc > 0 ) GM_ForceExecProc( proc, ( GCL_ARGS * )args ) ;
}

/* コールバック登録 */
int		NewSystemCallback( void )
{
	if ( GCL_GetOption( 'l' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_LOADING ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'r' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_RESTART ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'g' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_GAMEOVER ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'c' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_CONTINUE ] = GCL_GetNextInt() ;
	}	
	if ( GCL_GetOption( 'i' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_ITEM ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'w' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_WEAPON ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'e' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_EXIT ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'd' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_PLAYER_DEAD ] = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'a' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_ACTIVE_MAP_CHANGE ] = GCL_GetNextInt() ;
	}	
	if ( GCL_GetOption( 's' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_ALERT_MODE_ENTER ] = GCL_GetNextInt() ;
	}	
	if ( GCL_GetOption( 'q' ) != NULL ) {
		GM_SystemCallbackProc[ GM_CALLBACK_ALERT_MODE_QUIT ] = GCL_GetNextInt() ;
	}	
	return 0 ;
}
