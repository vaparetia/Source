/*
	memory.c
	    波メモリ制御用

	1999/11/10 T.Morita
	$Id: water_mem.c,v 1.1.1.3 2002/11/19 11:46:39 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "water.h"

static Work *w ;
int WTR_MemInit( Work *work )
{
    w = work ;
    w->mem.block   = (int)GV_Malloc( WATR_MEMORY_SIZE ) ;
    w->mem.current =  w->mem.block ;
    return w->mem.block ;
}

void *WTR_MemAlloc( int size )
{
    void *mem = (void *)w->mem.current ;

    (int)w->mem.current += (size/16+1)*16 ;
//    printf( "mem get%x Left%d\n", w->mem.current, (int)(w->mem.block+WATR_MEMORY_SIZE-w->mem.current) );

    return mem ;
}

void WTR_MemDie()
{
    if ( w->mem.block )
	GV_Free( (u_int *)w->mem.block ) ;
}
