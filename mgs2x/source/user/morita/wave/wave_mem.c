//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	memory.c
	    波メモリ制御用

	1999/11/10 T.Morita
	$Id: wave_mem.c,v 1.1.1.3 2002/11/19 11:46:40 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "wave.h"

static Work *w ;
int WAV_MemInit( Work *work )
{
    w = work ;
    w->mem.block   = (int)GV_Malloc( WAV_MEMORY_SIZE ) ;
    w->mem.current =  w->mem.block ;
    return w->mem.block ;
}

void *WAV_MemAlloc( int size )
{
    void *mem = (void *)w->mem.current ;

    *(char**)(&w->mem.current) += (size/16+1)*16 ;
    //printf( "mem get%x Left%d\n", w->mem.current, (int)(w->mem.block+WAV_MEMORY_SIZE-w->mem.current) );

    return mem ;
}

void WAV_MemDie()
{
    if ( w->mem.block )
	GV_DelayedFree( (u_int *)w->mem.block ) ;
}
