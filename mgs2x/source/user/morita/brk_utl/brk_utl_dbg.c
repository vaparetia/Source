//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_ini.c
   壊れ用 汎用初期化ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_dbg.c,v 1.1.1.3 2002/11/19 11:45:51 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_utl.h"

/*

  壊れ物用のデバッグキャラ


*/

typedef struct worknum_t
{
    GV_ACT_EX actor ;
	FVECTOR  *pos ;
    int       num ;
} WorkNum ;


void BRK_UTL_PosNum( FVECTOR *pos, int  num )
{
    int  x, y ;
    FVECTOR  v ;

    _sceVu0ApplyMatrix( &v, &DG_Chanls->eye_pers, pos ) ;
    x = (int)((v.vx / v.vw) * (float)(DRAW_WIDTH >>1)) + (DRAW_WIDTH >>1) ;
    y = (int)((v.vy / v.vw) * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1) ;
    //printf( "(%d,%d) %d\n", x, y, num ) ;
    if ( x>=0 && x<512 && y>=0 && y<512 )
    {
	DEBUG_Locate( x, y, 0 ) ;
	DEBUG_Printf( "%d\n", num ) ;
    }
}

static  void  ActNum( WorkNum *work )
{
	BRK_UTL_PosNum( work->pos, work->num ) ;
}

void *NewNameNum( DG_OBJS *body, int i )
{
    WorkNum *work ;

    work = (WorkNum *)GV_NewEffect( GV_ACTOR_PREV2, sizeof( WorkNum ) ) ;
    if( work != NULL && body )
    {
	GV_SetActor( &work->actor, ActNum, NULL ) ;
	GV_ActorEX( &work->actor ) ;

	work->pos = (FVECTOR *)body->world.m[W] ;
	work->num = i ;
    }
    return (void *)work ;
}

void *NewWorldNum( FMATRIX *world, int i )
{
    WorkNum *work ;

    work = (WorkNum *)GV_NewEffect( GV_ACTOR_PREV2, sizeof( WorkNum ) ) ;
    if( work != NULL && world )
    {
	GV_SetActor( &work->actor, ActNum, NULL ) ;
	GV_ActorEX( &work->actor ) ;

	work->pos = (FVECTOR *)world->m[W] ;
	work->num = i ;
    }
    return (void *)work ;
}


void *NewPosNum( FVECTOR *world, int i )
{
    WorkNum *work ;

    work = (WorkNum *)GV_NewEffect( GV_ACTOR_PREV2, sizeof( WorkNum ) ) ;
    if( work != NULL && world )
    {
	GV_SetActor( &work->actor, ActNum, NULL ) ;
	GV_ActorEX( &work->actor ) ;

	work->pos = world ;
	work->num = i     ;
    }
    return (void *)work ;
}
