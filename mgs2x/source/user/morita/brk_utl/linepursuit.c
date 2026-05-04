//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_mgz_clb.c
   雑誌壊れ コールバック関数

   2000/06/20 T. Morita
   $Id: linepursuit.c,v 1.1.1.3 2002/11/19 11:45:52 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "../include/util.h"

#define N_VERTS 30

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

//for DEBUG
#include "../brk_utl/brk_utl.x"

typedef struct work_t
{
    GV_ACT    actor ;

    int       name  ;
    int       where ;

    DG_PRIM2 *line ;
    FVECTOR  *pos  ;

    int      count ;
} Work ;


static void Die( Work *work )
{
    if ( work->line )
	GM_FreePrim2( work->line ) ;
}

static void Act( Work *work )
{
    int i ;
    FVECTOR  *pos ;

    if ( work->count < 1  )
	for( i=N_VERTS ; --i>=0 ; )
	{
	    _sceVu0CopyVector( &work->line->pos[0][i], work->pos ) ;
	    _sceVu0CopyVector( &work->line->pos[1][i], work->pos ) ;
	}

    if ( work->count < 3  )
	printf( "[%x] %.0f %.0f %.0f(%d)\n",
		work, work->pos->vx,work->pos->vy,work->pos->vz,
		work->count ) ;

    DG_SwitchBuffPrim2( work->line ) ;
    pos = work->line->pos[work->line->buffer_clock] ;

    for( i=N_VERTS-1 ; --i>=0 ; pos++ )
	_sceVu0CopyVector( pos, pos+1 ) ;
    _sceVu0CopyVector( pos, work->pos ) ;
    work->count++ ;
}

static int GetResources( Work *work, FVECTOR *pos )
{
    int  i ;


    work->line = BRK_UTL_MakeLINE2( 1, N_VERTS,  0,
				    SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ),
				    0x7f7f7f7f, 0x007f7f7f ) ;
    if ( work->line == NULL )
	PERROR( "Line Prim Fail\n" ) ;

    work->pos = pos ;
    for( i=N_VERTS ; --i>=0 ; )
    {
	_sceVu0CopyVector( &work->line->pos[0][i], work->pos ) ;
	_sceVu0CopyVector( &work->line->pos[1][i], work->pos ) ;
    }

    printf( "start[%x] %.0f %.0f %.0f\n",
	    work, work->pos->vx,work->pos->vy,work->pos->vz ) ;

    return 0 ;
}

void *NewLinePursuit( FVECTOR *pos )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if ( GetResources( work, pos ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
