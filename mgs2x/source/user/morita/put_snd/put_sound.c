//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_sound.c
   音声鳴らし君

   2000/9/31 T. Morita
   $Id: put_sound.c,v 1.1.1.3 2002/11/19 11:46:31 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"
#include "../include/util.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define MAX_SOUND 4

typedef struct work_t
{
    GV_ACT_EX actor ;

    FVECTOR   pos ;

    int       se[MAX_SOUND] ;
    int       n_se ;

    int       count ;
    int       inter ;
    int       mode  ;
} Work ;



static void Act( Work *work )
{
    int se ;

    if ( work->count++ == work->inter )
    {
	switch( work->mode )
	{
	case 0:
	    se = irnd() % work->n_se ;
	    GM_SeSetMode( work->se[se], &work->pos, GM_SEMODE_NORMAL ) ;
	    break ;

	default:
	    GM_SeSetMode( SD_A_FLAGBT12, &work->pos, GM_SEMODE_BOMB ) ;
	}
	work->count = 0 ;
    }
}

static int GetResources( Work *work, int name, int where )
{
    int     i   ;
    IVECTOR buf ;

    work->count = 0 ;
    work->n_se  = 0 ;
    if ( GCL_GetOption( 's' ) )
	for ( i=MAX_SOUND ; GCL_NextStr() && --i>=0 ; work->n_se++ )
	    GCL_GetNextInt() ;
    else
    {
	work->se[work->n_se++] = SD_A_FLAGBT12 ;
	work->se[work->n_se++] = SD_A_FLAGBT13 ;
    }

    if ( !GCL_GetOption( 'p' ) )
	PERROR( "No -pos option found\n" ) ;
    GCL_GetNextIV( (int *)&buf ) ;
    vu0_IV0toFV( &buf, &work->pos ) ;

    work->mode  = GCL_GetOptionValue( 'm', 0  ) ;
    work->inter = GCL_GetOptionValue( 'i', 24 ) ;
    work->inter = work->inter*5/TIME_BASE ;

    return 0 ;
}

void *NewPutSeSound( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
