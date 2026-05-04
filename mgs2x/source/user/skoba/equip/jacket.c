//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   jacket.c
   À…√∆•¡•Á•√•≠

   2000/06/02 M.Sonoyama
   $Id: jacket.c,v 1.1.1.3 2002/11/19 11:50:16 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT		actor ;
    CONTROL		*ctrl ;
    OBJECT		**body ;
    int			*unit ;
    int			*trigger ;
	int			time ;
} Work ;

/*------------------------------------------------------------------*/

static	void	Act( work )
Work			*work ;
{

}

static	void	Die( work )
Work			*work ;
{

}

/*------------------------------------------------------------------*/

void		*NewJacket( ctrl, body, unit, trigger )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		work->ctrl = ctrl ;
	}
    return work ;
}

