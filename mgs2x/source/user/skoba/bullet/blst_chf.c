//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   blst_chf.c
   チャフグレネード爆発
   
   2000/05/22 M.Sonoyama
   $Id: blst_chf.c,v 1.1.1.3 2002/11/19 11:49:59 Yoshizawa1 Exp $
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

extern	void	*NewChaffEffect( FVECTOR *pos, int mode ) ;

static	int		ChaffActive = 0 ;

#define	CHAFF_COUNT		(480)

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
	if ( -- ChaffActive <= 0 ) {
//		GM_ResetGameStatus( STATE_CHAFF ) ;
	}
}

/*------------------------------------------------------------*/

static	int	GetResources( work, world )
Work		*work ;
FMATRIX		*world ;
{
	FVECTOR		pos ;

	GV_MatToVec( world, &pos ) ;
	NewChaffEffect( &pos, 0 ) ;
	work->count = CHAFF_COUNT ;
	return 0 ;
}

/*------------------------------------------------------------*/

void	*NewBlastChaff( FMATRIX *world )
{
	Work		*work ;

	if ( !GM_CheckGameStatus( STATE_CHAFF ) ) ChaffActive = 0 ;
	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GetResources( work, world ) ;
//		GM_SetGameStatus( STATE_CHAFF ) ;
		ChaffActive ++ ;
	}
	return work ;
}
