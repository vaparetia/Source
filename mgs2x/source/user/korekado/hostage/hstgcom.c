/*
	hstgcom.c
	人質コマンダー

	2001/03/23 Y.Korekado
	$Id: hstgcom.c,v 1.1.1.3 2002/11/19 11:44:18 Yoshizawa1 Exp $
*/
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			hstg_num ;
} Work ;

/*----------------------------------------------------------------*/
int	HSTGCOM_SetHostage( )

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	
}
static	void	Die( Work *work )
{
}
/*----------------------------------------------------------------*/
static	int	GetResources( Work *work, int name, int where )
{
	work->hstg_num = 0 ;
}

void		*NewHostageCommander( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor(  GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
