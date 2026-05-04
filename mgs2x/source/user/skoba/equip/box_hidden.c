//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   box_hidden.c
   ダンボール同じ柄隠れエリアチェック

   2000/10/11	M.Sonoyama
   $Id: box_hidden.c,v 1.1.1.3 2002/11/19 11:50:14 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libutl.h>
#endif

#include	"gameheader.h"

typedef	struct	{
	int		item ;
	int		trap ;
} TRPDEF ;

typedef	struct	{
	GV_ACT			actor ;
	CONTROL			*ctrl ;
	int				n_trpdefs ;
	TRPDEF			def[ 4 ] ;
} Work ;

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int			i ;
	int			item, type ;

	item = PL_GetPlayerItem() ;
	type = GM_ItemTypes[ item ] ;
	if ( !( type & IT_TYPE_CBBOX ) ) return ;
	for ( i = 0; i < work->n_trpdefs; i ++ ) {
		if ( item == work->def[ i ].item && 
			 GM_CheckInsideTrap( work->ctrl, &work->def[ i ].trap, 1, 0 ) ) {
			GM_SetPlayerStatus( PLAYER_CB_BOX_HIDDEN ) ;
			break ;
		}
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			n ;

	if ( GCL_GetOption( 'n' ) == NULL ) return -1 ;
	work->ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	if ( work->ctrl == NULL ) return -1 ;
	GCL_GetOption( 't' ) ;
	n = 0 ;
	while( GCL_NextStr() != NULL ) {
		work->def[ n ].item = GCL_GetNextInt() ;
		work->def[ n ].trap = GCL_GetNextInt() ;
		if ( ++ n == 4 ) break ;
	}
	work->n_trpdefs = n ;
	return 0 ;
}

void		*NewCheckBoxHidden( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_CHECK_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, NULL ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}



