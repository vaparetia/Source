//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_ceil.c 
   昇降機

   2001/01/16 T.Morita
   $Id: efct_vib.c,v 1.1.1.3 2002/11/19 11:46:11 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;
    int       high  ;
    int       low   ;
    int       base  ;
} Work ;

static void Act( Work *work )
{
    if ( work->high )
    {
	work->high-- ;
	GM_SetVibration1( 0, 1 ) ;
    }
    if ( work->low )
    {
	work->low-- ;
	GM_SetVibration2( 0, work->base + (irnd()&63) ) ;
    }

    if ( work->high == 0 && work->low == 0 )
	GV_DestroyActor( work ) ;
}

void *NewFortVibrate( int base, int high, int low )
{
    Work *work ;

    if ( high <= 0 && low <= 0)
	return NULL ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        GV_ActorEX( &work->actor ) ;
	work->high = high ;
	work->low  = low ;
	work->base = base ;
    }
    return (void *)work ;
}
