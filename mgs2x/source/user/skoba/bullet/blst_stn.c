//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   blst_stn.c
   スタングレネード爆発
   
   2000/05/25 M.Sonoyama
   $Id: blst_stn.c,v 1.1.1.3 2002/11/19 11:49:59 Yoshizawa1 Exp $
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
#endif


#include	"gameheader.h"

/*------------------------------------------------------------*/

extern	void	*NewStanEffect( FVECTOR *pos, int mode ) ;

static	int		Active = 0 ;

#define	COUNT	(1)

/*------------------------------------------------------------*/

/*------------------------------------------------------------*/

typedef	struct	{
	GV_ACT		actor ;
	int			count ;
} Work ;

/*------------------------------------------------------------*/

static	void	Act( work )
Work			*work ;
{
	if ( -- work->count <= 0 ) GV_DestroyActor( work ) ;
}

static	void	Die( work )
Work			*work ;
{
	if ( -- Active <= 0 ) {
//		GM_ResetGameStatus( STATE_STUN ) ;
	}
}

/*------------------------------------------------------------*/

static	int	GetResources( work, world )
Work		*work ;
FMATRIX		*world ;
{
	FVECTOR		pos ;

	GV_MatToVec( world, &pos ) ;
	NewStanEffect( &pos, 0 ) ;
	work->count = COUNT ;
	return 0 ;
}

/*------------------------------------------------------------*/

void	*NewBlastStun( FMATRIX *world )
{
	Work		*work ;

	if ( !GM_CheckGameStatus( STATE_STUN ) ) Active = 0 ;
	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GetResources( work, world ) ;
//		GM_SetGameStatus( STATE_STUN ) ;
		Active ++ ;
	}
	return work ;
}
