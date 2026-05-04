//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wave.c
	    波メイン

	1999/08/10 T.Morita
	$Id: wave.c,v 1.1.1.3 2002/11/19 11:46:39 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "wave.h"


static void Act( Work *work )
{
    if ( WAV_EyeControl( work ) )
	WAV_ActAllWave( work ) ;
}

static int GetResources( Work *work, int name )
{
    work->name = name ;
    if ( WAV_GetOptionValue( work, WAVE_WAVE_COL, WAVE_WAVE_ROW ) < 0 )
	return -1 ;
    return 0 ;
}

void *NewOceanWave( int name, int where )
{
    Work *work ;
    
    if ( (work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) )) != NULL )
    {
	GV_SetActor( &work->actor, Act, NULL ) ;
	if ( GetResources( work, name ) )
	    GV_DestroyActor( work ), work = NULL ;
    }
    return (void *)work ;
}
