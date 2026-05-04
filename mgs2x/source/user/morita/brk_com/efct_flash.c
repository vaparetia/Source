//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_blink.c
  点滅ボタン

  2001/06/12 T. Morita
  $Id: efct_flash.c,v 1.1.1.3 2002/11/19 11:45:26 Yoshizawa1 Exp $
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



enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

typedef struct work_t
{
    GV_ACT_EX actor ;

    DG_PRIM2 *blink ;
    int       count ;
    int       alpha ;
} Work ;


static void Die( Work *work )
{
    if ( work->blink )
	GM_FreePrim2( work->blink ) ;
}

static void Act( Work *work )
{
    DG_PRIM2_UVRGB *nxt_bu, *prv_bu ;
    int i, alpha ;

    prv_bu = work->blink->uvrgb[work->blink->buffer_clock] ;
    DG_SwitchBuffPrim2( work->blink ) ;
    nxt_bu = work->blink->uvrgb[work->blink->buffer_clock] ;

    /* 点滅 */
    if ( (alpha = prv_bu->a) )
	alpha -= alpha>work->count ? work->count : alpha ;
    else
	alpha = work->alpha ;
    for( i=4 ; --i>=0 ; nxt_bu++ )
	nxt_bu->a = alpha ;
}

static int GetResources( Work *work, FVECTOR *pos, int rgba, int count )
{
    int  i ;

    if ( !(work->blink = BRK_UTL_MakePOLY( 1,
					   13225953,/* light10_msk */
					   SCE_GS_SET_ALPHA(0,2,0,1,0),
					   rgba )) )
        PERROR( "No Prim(no memory) : NewBreakComputerBlink\n" ) ;

    work->count = count>=0 ? count : 0 ;
    work->alpha = (rgba >> 24) & 0xff ; 
    for( i=4 ; --i>=0 ; )
    {
	_sceVu0CopyVector( &work->blink->pos[0][i], &pos[i] ) ;
	_sceVu0CopyVector( &work->blink->pos[1][i], &pos[i] ) ;
    }
    work->blink->pos[0][0].vw = 1.0f ;
    work->blink->pos[1][0].vw = 1.0f ;

    return 0 ;
}

void *NewBreakComputerSquareButton( FVECTOR *pos, int rgba, int life )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, pos, rgba, life ) < 0 )
	{
	    GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
