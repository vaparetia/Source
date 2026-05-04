//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   cig.c
   ¥¿¥Ð¥³

   1999/12/14 M.Sonoyama
   $Id: cig.c,v 1.1.1.3 2002/11/19 11:50:14 Yoshizawa1 Exp $
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

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
			 DG_FLAG_ONEPIECE)
#define	BODY_NAME	(104839)	/* cig.kms */

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT		actor ;
    OBJECT		equip ;

    CONTROL		*ctrl ;
    OBJECT		**body ;
    int			*unit ;
    u_int		*trigger ;
} Work ;

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    u_int	trg ;
    FMATRIX	*root ;

    GM_ConfigObjectRoot( &( work->equip ), 
			 *( work->body ), *( work->unit ) ) ;
    trg = *( work->trigger ) ;
}

static	void	Die( work )
Work		*work ;
{
    if ( GM_PlayerBody != NULL ) {
	DG_DisconnectObjs( GM_PlayerBody->objs, work->equip.objs ) ;
    }
    GM_FreeObject( &( work->equip ) ) ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, body, unit )
Work		*work ;
OBJECT		**body ;
int		*unit ;
{
    OBJECT	*equip ;

    equip = &( work->equip ) ;
    GM_InitObject( equip, BODY_NAME, BODY_FLAG ) ;
    GM_ConfigObjectRoot( equip, *body, *unit ) ;
    DG_ConnectObjs( GM_PlayerBody->objs, work->equip.objs ) ;
    return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewCigar( ctrl, body, unit, trigger )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	if ( GetResources( work, body, unit ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
	work->ctrl = ctrl ;
	work->body = body ;
	work->unit = unit ;
	work->trigger = trigger ;
    }
    return work ;
}

